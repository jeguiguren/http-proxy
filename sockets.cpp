#include "sockets.h"
using namespace std;


/*
*/

int write_message(int sockfd, char *message, int messageSize) {
	//cout << "Writing\n";
	int n = send(sockfd, message, messageSize, MSG_NOSIGNAL);
	if (n < 0){
		//cout << "The error was error " << n << endl;
		throw runtime_error("Error on write");
	}
	return n;
}

int read_message(int sockfd, char **message, int scale) {
	//cout << "reading\n";
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
Sockets::Sockets(int port, int bps){
	myPort = port;
	MAX_BPS = bps;
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
	//cout << "Created the proxy" << endl;
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
int getportno(char *request, int isConnect){
	char *holder;
	int portno = 0;
	if (isConnect)
    	holder = strstr(request, "CONNECT ") + 8;
    else
    	holder = strstr(request, "Host:") + 6;
    
	if (strchr(holder, ':'))
		portno = atoi(strchr(holder, ':') + 1);
	if (portno == 0)
		portno = 80;
	//cout << portno << endl;
	return portno;
}

char *gethostname(char *request, int isConnect){
	char *hostname = (char* )malloc(100);
	char *holder;
	
    if (isConnect)
    	holder = strstr(request, "CONNECT ") + 8;
    else
    	holder = strstr(request, "Host:") + 6;
    int i = 0;
	while (holder[i] != ':' && holder[i] != '\r'){
		hostname[i] = holder[i];
		i++;
	}
	hostname[i] = '\0';
	//cout << "host " << hostname << endl;
	return hostname;
}

char* Sockets::getObjectname(char *request, int portnum){
    char *name = (char* )malloc(100);
    int bytesUsed = 100;
    char *holder;
    char intString[6];
    int i = 0;
    holder = strstr(request, "GET ") + 4;
    while(holder[i] != ' '){
        name[i] = holder[i];
        i++;
        if (i == (bytesUsed - 1)){
        	name = (char* )realloc(name, bytesUsed + 100);
        	bytesUsed += 100;
        }
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
		clientRequest.bytes_read = bytes_read;
		clientRequest.request = request;
		clientRequest.isHttps = isHttps(request);
		clientRequest.hostname = gethostname(request, clientRequest.isHttps);
		clientRequest.portno = getportno(request, clientRequest.isHttps);
		if ((strcmp(clientRequest.hostname, "localhost") == 0) and 
			clientRequest.portno == myPort)
			throw runtime_error("I am only a proxy");
	}
	else {
		throw runtime_error("Empty request\n");
	}
	return clientRequest;
}


/*******************************************************************************
	Function: connect_and_write_to_server
	Puroprse: connects and writes to the server
*******************************************************************************/
int Sockets::connect_to_server(userRequest request) {
	//cout << "in connect\n";
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
	//cout << "Connected to server" << endl;
	return sockfd;
}



/***************************************************************************
	Function: process_request
	Parameters: socket to read from and flag to set if request is https
	Returns: newly opened server socket
	Purpose: Reads from client and opens server socket; sets flag accordingly
***************************************************************************/
int Sockets::process_request(int client_fd, int *isHttps) {

	userRequest request = get_client_request(client_fd);
	if (!request.isHttps){
		char * requestName = getObjectname(request.request, request.portno);
		cout << "Req Name: " << requestName << endl;
		if (sessionCache.dataInCache(requestName)){
			cout << "\n*** writing from cache ***\n";
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
		httpsPairs.insert(make_pair(client_fd, server_fd)); 
		httpsPairs.insert(make_pair(server_fd, client_fd)); 
		char *ok200 = (char *)"HTTP/1.0 200 Connection established\r\n\r\n";
		write_message(client_fd, ok200, strlen(ok200));
		(*isHttps) = 1;
		free_request(&request);
	}
	else {
		cout << "adding server req\n";
		write_message(server_fd, request.request, request.bytes_read);
		serverReq.insert(make_pair(server_fd, request));
		(*isHttps) = 0;
	}
	return server_fd;
}


// Returns True if client's download rate > MAX_BPS
int Sockets::bandwidth_exceeded(int clientSock) {
	
	clientBPSIter = clientBPSMap.find(clientSock);
	if (clientBPSIter == clientBPSMap.end())
		return 0;

	struct clientBPS c = clientBPSIter->second;

	int now = time(NULL);
	int elapsed = now - c.start;

	if (elapsed == 0)
		return 0;

	int bps = c.bytes_read / elapsed;
	return bps > MAX_BPS;

}

/*******************************************************************************
	Function: respond
	Puroprse: reads from Server and writes to client in chunks of at least RESPONSEBUFSIZE (except last chunk)
	Returns: returns amount transfered
	TODO must cache it
*******************************************************************************/
int Sockets::transfer(int serverSock, int clientSock){

	if (bandwidth_exceeded(clientSock)) {
		cout << "\n\n ******* EXCEEDED *******\n\n";
		return 1;
	}

	char *message = NULL;
	int received = read_message(serverSock, &message, RESPONSEBUFSIZE);
	int erase = 1;

	int now = time(NULL);
	struct clientBPS new_bps = {now, received};			

	httpsPairsIter = httpsPairs.find(serverSock);
	serverRespIter = serverResp.find(serverSock);
	clientBPSIter = clientBPSMap.find(clientSock);
	
	// Transfer in progress
	if (received > 0) {
		//cout << "Read message of size " << received << endl;
		write_message(clientSock, message, received);
		//cout << "Wrote " << size << " from " << serverSock << " to " << clientSock << endl;


		if (clientBPSIter == clientBPSMap.end())
			clientBPSMap.insert(make_pair(clientSock, new_bps));
		else {
			//cout << "updating!\n";
			(clientBPSIter->second).bytes_read += received;
		}

				

		if (httpsPairsIter == httpsPairs.end()) { // HTTP transfer, Need to save partial read
			//cout << "\n***** Store to later cache ********\n";
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
			//cout << "At the end" << endl;
		}
	}
	//Transfer done
	else {
		cout << "^^^^^TRANSFER COMPLETE^^^^^^" << endl;
		if (httpsPairsIter == httpsPairs.end() and serverRespIter != serverResp.end()) { // HTTP transfer, Need to cache complete
			//cout << "Caching complete transfer\n";
			serverResponse response = serverRespIter->second;
			serverReqIter = serverReq.find(serverSock);
			if (serverReqIter != serverReq.end()) {
				userRequest request = serverReqIter->second; 
				char * requestName = getObjectname(request.request, request.portno);
				cout << "Req namee: " << requestName << endl;
				int TTL = getTimeToLive(response.data);
				if (TTL > 0 or true){
					//cout << "caching: " << requestName << endl;
					cout << "Caching request of size " << request.bytes_read <<  " with response of size " << response.bytes_read << "; cache handles request parsing for GET object?\n";
					sessionCache.cacheElement(requestName, request.request, response.data, TTL, response.bytes_read);
				}
			}
		}
		//cout << "Removing keys b/c complete transfer\n";
		httpsPairs.erase(clientSock);
		httpsPairs.erase(serverSock);
		clientBPSMap.erase(clientSock);
		serverReq.erase(serverSock);
		serverResp.erase(serverSock);
	}
	if (message != NULL and erase)
		free(message);
	return received;
}