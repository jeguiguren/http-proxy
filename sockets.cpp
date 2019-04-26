#include "sockets.h"
using namespace std;


/*
*/

int write_message(int sockfd, char *message, int messageSize) {
	cout << "Writing\n";
	int n = send(sockfd, message, messageSize, MSG_NOSIGNAL);
	if (n < 0)
		throw runtime_error("Error on write");
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
		if (req->hostname != NULL)
			free(req->hostname);
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
	cout << "Accepted connection to: " << sock_fd << endl;
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

char *gethostname(char *request){
	char *holder;
	int i = 0;
	cout << "before malloc\n";
	char *hostname = (char* )malloc(100);
	cout << "after malloc\n";
	holder = strstr(request, "Host:") + 6;
	while (holder[i] != ':' && holder[i] != '\r'){
		hostname[i] = holder[i];
		i++;
	}
	hostname[i] = '\0';
	cout << "host " << hostname << endl;
	return hostname;
}

char* Sockets::getObjectname(char *request, int portnum){
    char *name = (char* )malloc(1000);
    int bytesUsed = 1000;
    char *holder;
    char intString[6];
    int i = 0, j = 0;
    holder = strstr(request, "GET ") + 4;
    while(holder[i] != ' '){
        name[i] = holder[i];
        i++;
        if (i == (bytesUsed - 1)){
        	name = (char* )realloc(name, bytesUsed + 1000);
        	bytesUsed += 1000;
        }
    }
    name[i] = ':';
    i++;
    sprintf(intString,"%d",portnum);
    while(intString[j] != '\0'){
        name[i] = intString[j];
        i++;
        j++;
    }
    name[i] = '\0';
    return name;
}

int isHttps(string req) {
	size_t index = req.find("CONNECT");
	return index != string::npos;
}

int getTimeToLive(char *response){
    char *holder;
    int timeToLive = 0;
    holder = strstr(response, "max-age=");
    if (holder != NULL){
        if((holder + 8)[0] == '0'){
            //timeToLive = 0;
            return 0;
        }
        timeToLive = atoi(holder + 8);
    }
    if((timeToLive == 0) || (holder == NULL)){
    	//TO-DO: change this to have to deal with minutes
        timeToLive = 3600;
    }
    return timeToLive;

}


/*******************************************************************************
	Function: get_client_request
	Puroprse: reads and returns a client request;
*******************************************************************************/
Sockets::userRequest Sockets::get_client_request(int client_fd){

	userRequest clientRequest;
	char *request = NULL;
	int bytes_read = read_message(client_fd, &request, REQUESTBUFSIZE);

	if (bytes_read > 0){
		cout << request;
		clientRequest.bytes_read = bytes_read;
		clientRequest.request = request;
		clientRequest.hostname = gethostname(request);
		clientRequest.portno = getportno(request);
		clientRequest.isHttps = isHttps(request);
		cout << "got request info" << endl;
		if ((strcmp(clientRequest.hostname, "localhost") == 0) and 
			clientRequest.portno == myPort)
			throw runtime_error("I am only a proxy");
	}
	else {
		throw runtime_error("Empty request");
	}
	return clientRequest;
}


/*******************************************************************************
	Function: connect_and_write_to_server
	Puroprse: connects and writes to the server
*******************************************************************************/
int Sockets::connect_to_server(userRequest request) {
	//TO-DO: Figure out how to change the parameter to a pointer
	cout << "in connect\n";
	int sockfd;
	string host_name = request.hostname;
	char hostname[host_name.length() + 1];
	strcpy(hostname, host_name.c_str());

	struct sockaddr_in serveraddr;
	struct hostent *server;
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		throw runtime_error("ERROR opening socket");

	long arg = fcntl(sockfd, F_GETFL, NULL); 
  	arg |= O_NONBLOCK; 
  	fcntl(sockfd, F_SETFL, arg); 

	server = gethostbyname(hostname);
	if (server == NULL) 
		throw runtime_error("ERROR, no such host");
		
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, 
						 (char *)&serveraddr.sin_addr.s_addr, server->h_length);
	serveraddr.sin_port = htons(request.portno);

	if (connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
		fd_set fdset;
	    struct timeval tv;
		FD_ZERO(&fdset);
	    FD_SET(sockfd, &fdset);
	    tv.tv_sec = 1;             /* 1 second timeout */
	    tv.tv_usec = 0;
	    if (select(sockfd + 1, NULL, &fdset, NULL, &tv) <= 0)
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
	if (!request.isHttps){
		char * requestName = getObjectname(request.request, request.portno);
		if (sessionCache.dataInCache(requestName)){
			Cache::cacheResponse response = sessionCache.getDataFromCache(requestName);
			write_message(client_fd, response.data, response.data_length);
			throw runtime_error("Wrote from cache");
		}
	}

	int server_fd;
	try{        
		server_fd = connect_to_server(request);
	}catch(const std::exception &exc){
		free_request(&request);
		throw runtime_error("ERROR connecting to server");
	}
                

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
	int erase = 1;

	httpsPairsIter = httpsPairs.find(serverSock);
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
				serverResponse response { received, message };
				serverResp.insert(make_pair(serverSock, response)); 
				erase = 0;
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
		if (httpsPairsIter == httpsPairs.end() and serverRespIter != serverResp.end()) { // HTTP transfer, Need to cache complete
			cout << "Caching complete transfer\n";
			serverResponse response = serverRespIter->second;
			serverReqIter = serverReq.find(clientSock);
			userRequest request = serverReqIter->second; 
			(void) request;
			char * requestName = getObjectname(request.request, request.portno);
			int TTL = getTimeToLive(response.data);
			if (TTL > 0){
				cout << "Caching request of size " << request.bytes_read <<  " with response of size " << response.bytes_read << "; cache handles request parsing for GET object?\n";
				sessionCache.cacheElement(requestName, request.request, response.data, TTL, response.bytes_read);
			}
		}
		else {
			cout << "Removing https b/c complete transfer\n";
			httpsPairs.erase(clientSock);
			httpsPairs.erase(serverSock);
		}
	}
	if (message != NULL and erase)
		free(message);
	return received;
}









