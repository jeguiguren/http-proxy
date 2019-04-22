#include "sockets.h"
using namespace std;


/*
For select:
Jorge requests google (select identifies Jorge); open up socket
with Google, and return the socket to main loop (and add it to slect)
When Google is ready to write, select will identify it;
Keep a list or dict where socket can index into "receiver" so that when google
writes we know what client to send it to


For HTTPS:
Read for "Connect" key word in header, and then
open up a tunnel between them
*/

int write_message(int sockfd, char *message, int messageSize) {
	cout << "Writing\n";
	int n = write(sockfd, message, messageSize);
	if (n < 0)
		throw runtime_error("Error on write");
	//cout << "done\n";
	return n;
}

int read_message(int sockfd, char **message, int scale) {
	cout << "reading\n";
	char *buffer = (char*) malloc(scale);
	int received = read(sockfd, (void *)buffer, scale);
	if (received < 0) 
		throw runtime_error("ERROR reading from socket");
	*message = buffer;
	return received;
}

void Sockets::free_request(userRequest *req) {
	if (req != NULL) {
		if (req->request != NULL)
			free(req->request);
		free(req);
	}
}

/*******************************************************************************
	Function: constructor
	Puroprse: creates instance of this class
*******************************************************************************/
Sockets::Sockets(int port){
	myPort = port;
}

/*******************************************************************************
	Function: destructor
	Puroprse: deletes all memory used on the heap
*******************************************************************************/
Sockets::~Sockets(){
}


/*******************************************************************************
	Function: create_proxy_address
	Puroprse: creates a proxy socket
*******************************************************************************/
int Sockets::create_proxy_address(int portno){
	struct sockaddr_in serveraddr;
	int optval, listen_sock;
	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock < 0) 
		throw runtime_error("ERROR opening socket");
	optval = 1;
	setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, 
										   (const void *)&optval , sizeof(int));
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short)portno);
	//TO-DO: Ask why the :: is needed
	if (::bind(listen_sock, (struct sockaddr *)&serveraddr, 
											sizeof(serveraddr)) < 0)
		throw runtime_error("ERROR on binding");
	if (listen(listen_sock, 10) < 0)
		throw runtime_error("ERROR on listen");
	cout << "Created the proxy" << endl;
	return listen_sock;
}

/*******************************************************************************
	Function: accept_new_connection
	Puroprse: accepts a new user connection
*******************************************************************************/
int Sockets::accept_new_connection(int listen_sock){
	int clientlen;
	struct sockaddr_in clientaddr;
	clientlen = sizeof(clientaddr);
	int sock_fd = accept(listen_sock, (struct sockaddr *) &clientaddr, 
														(socklen_t*)&clientlen);
	//cout << "Accepted connection to: " << sock_fd << endl;
	return sock_fd;
}


//TO-DO: change to use jorge's util file
int getportno(char *request){
	char *holder;
	int portno = 0;
	holder = strstr(request, "Host:") + 6;
	if (strchr(holder, ':'))
		portno = atoi(strchr(holder, ':') + 1);
	if (portno == 0)
		portno = 80;
	cout << portno << endl;
	return portno;
}

string get_host_and_port(char *request, int *port){
	request = strstr(request, "Host: ") + 6; 
	char *end =  strstr(request, "\r\n");
	string host(request, end - request);
	string sport = "80";
	int len = host.length();
	for (int i = 0; i < len; i++) {
		if (host[i] == ':') {
			sport = host.substr(i + 1, len - 1);
			host = host.substr(0, i);
		}
	}
	(*port) = stoi(sport);
	cout << host << ", " << sport << endl; 
	return host;
}

int isHttps(string req) {

	size_t index = req.find("CONNECT");
	return index != string::npos;
}


/*******************************************************************************
	Function: get_client_request
	Puroprse: reads and returns a client request;
*******************************************************************************/
Sockets::userRequest Sockets::get_client_request(int client_fd){

	userRequest clientRequest;
	char *request = NULL;
	int bytes_read = read_message(client_fd, &request, REQUESTBUFSIZE);
	
	cout << request;
	clientRequest.bytes_read = bytes_read;
	clientRequest.request = request;
	clientRequest.hostname = get_host_and_port(request, &clientRequest.portno);
	clientRequest.isHttps = isHttps(request);
	if ((clientRequest.hostname == "localhost") and 
		clientRequest.portno == myPort)
		throw runtime_error("I am only a proxy");

	cout << "request" << endl;
	return clientRequest;
}


/*******************************************************************************
	Function: connect_and_write_to_server
	Puroprse: connects and writes to the server
*******************************************************************************/
int Sockets::connect_to_server(userRequest request) {
	//TO-DO: Figure out how to change the parameter to a pointer
	int sockfd;
	string host_name = request.hostname;
	char hostname[host_name.length() + 1];
	strcpy(hostname, host_name.c_str());

	struct sockaddr_in serveraddr;
	struct hostent *server;

	cout << "a\n";
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		throw runtime_error("ERROR opening socket");
	
	server = gethostbyname(hostname);
	if (server == NULL) 
		throw runtime_error("ERROR, no such host");
	cout << "b\n";
		
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, 
						 (char *)&serveraddr.sin_addr.s_addr, server->h_length);
	serveraddr.sin_port = htons(request.portno);
	cout << "c\n";
	if (connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
		cout << "d\n";
		throw runtime_error("ERROR connecting to server");
	}
	cout << "Connected to server" << endl;
	return sockfd;
}



/***************************************************************************
	Function: process_request
	Parameters: socket to read from and flag to set if request is https
	Returns: newly opened server socket
	Purpose: Reads from client and opens server socket; sets flag accordingly
***************************************************************************/
int Sockets::process_request(int client_fd, int *isHttps) {

	cout << "Processing request\n";

	userRequest request = get_client_request(client_fd);
	int server_fd = connect_to_server(request);

	if (request.isHttps) {
		cout << "Received HTTPS request" << endl;
		httpsPairs.insert(make_pair(client_fd, server_fd)); 
		httpsPairs.insert(make_pair(server_fd, client_fd)); 
		char *ok200 = (char *)"HTTP/1.0 200 Connection established\r\n\r\n";
		write_message(client_fd, ok200, strlen(ok200));
		(*isHttps) = 1;
		free_request(&request);
	}
	else {
		cout << "Received HTTP request" << endl;
		write_message(server_fd, request.request, request.bytes_read);
		cout << "Wrote to server" << endl;
		serverReq.insert(make_pair(client_fd, request));
		(*isHttps) = 0;
	}
	return server_fd;
}



/*******************************************************************************
	Function: respond
	Puroprse: reads from Server and writes to client in chunks of at least RESPONSEBUFSIZE (except last chunk)
	Returns: returns amount transfered
	TODO must cache it
*******************************************************************************/
int Sockets::transfer(int serverSock, int clientSock){

	char *message = NULL;
	int received = read_message(serverSock, &message, RESPONSEBUFSIZE);

	httpsPairsIter = httpsPairs.find(clientSock);
	serverRespIter = serverResp.find(serverSock);
		
	// Transfer in progress
	if (received > 0) {
		cout << "Read message of size " << received << endl;
		int size = write_message(clientSock, message, received);
		cout << "Wrote " << size << " from " << serverSock << " to " << clientSock << endl;

		if (httpsPairsIter == httpsPairs.end()) { // HTTP transfer, Need to save partial read
			cout << "\n***** Store to later cache ********\n";
			// First read
			if (serverRespIter == serverResp.end()) {
				char *merged = (char *) malloc(received);
				memcpy(merged, message, received);
				serverResponse response { received, merged };
				serverResp.insert(make_pair(serverSock, response)); 
			} 
			//Subsequent reads
			else { 
				serverResponse response = serverRespIter->second;
				char *merged = (char *)realloc(response.data, response.bytes_read + received);
				memcpy(merged + response.bytes_read, message, received);
				response.bytes_read += received;
				response.data = merged;
				serverRespIter->second = response;
			}
		}
	}
	//Transfer done
	else {
		if (httpsPairsIter == httpsPairs.end()) { // HTTP transfer, Need to cache complete
			cout << "Caching complete transfer\n";
			serverResponse response = serverRespIter->second;
			serverReqIter = serverReq.find(clientSock);
			userRequest request = serverReqIter->second; 
			(void) request;
			cout << "Caching request of size " << request.bytes_read <<  " with response of size " << response.bytes_read << "; cache handles request parsing for GET object?\n";
		}
		else {
			cout << "Removing https b/c complete transfer\n";
			httpsPairs.erase(clientSock);
			httpsPairs.erase(serverSock);
		}
	}
	free(message);
	return received;
}









