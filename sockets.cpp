#include "sockets.h"
using namespace std;

/*******************************************************************************
	Function: constructor
	Puroprse: creates instance of this class
*******************************************************************************/
Sockets::Sockets(){
}

/*******************************************************************************
	Function: destructor
	Puroprse: deletes all memoer used on the heap
*******************************************************************************/
Sockets::~Sockets(){
}

/*******************************************************************************
	Function: error
	Puroprse: throw error message
*******************************************************************************/
void Sockets::error(string msg) {
	char message[msg.length() + 1];
	strcpy(message, msg.c_str());
  	perror(message);
 	exit(1);
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
        error("ERROR opening socket");
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
        error("ERROR on binding");
    if (listen(listen_sock, 10) < 0)
        error("ERROR on listen");
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
    return portno;
}

char *gethostname(char *request){
    char *holder;
    int i = 0;
    char *hostname = (char* )malloc(100);
    holder = strstr(request, "Host:") + 6;
    while (holder[i] != ':' && holder[i] != '\r'){
        hostname[i] = holder[i];
        i++;
    }
    hostname[i] = '\0';
	return hostname;
}


/*******************************************************************************
	Function: get_client_request
	Puroprse: reads and returns a client request;
*******************************************************************************/
Sockets::userRequest Sockets::get_client_request(int client_fd){
	//TO-DO: find out why there is empty space at the end of the request
	userRequest clientRequest;
	int bytes_read;
	char *request = (char *)malloc(REQUESTBUFSIZE);
	bzero(request, REQUESTBUFSIZE);
	bytes_read = read(client_fd, request, REQUESTBUFSIZE);
	if (bytes_read <= 0) 
	    throw runtime_error("ERROR reading from socket");
    clientRequest.request = request;
    clientRequest.hostname = gethostname(request);
    clientRequest.portno = getportno(request);
    cout << "got client request" << endl;
	return clientRequest;
}

/*******************************************************************************
	Function: connect_to_server
	Puroprse: connects to the server
*******************************************************************************/
int Sockets::connect_to_server(int portno, string host_address){
    //TO-DO: Figure out how to change the parameter to a pointer
	int sockfd;
	char hostname[host_address.length() + 1];
	strcpy(hostname, host_address.c_str());
    struct sockaddr_in serveraddr;
    struct hostent *server;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        throw runtime_error("ERROR opening socket");
    server = gethostbyname(hostname);
    if (server == NULL) {
        throw runtime_error("ERROR, no such host");
        exit(0);
    }
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
                         (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) <0) 
        throw runtime_error("ERROR connecting to server");
    cout << "Connected to server" << endl;
    return sockfd;
}


/*******************************************************************************
	Function: writeandread
	Puroprse: writes a request to a server and return the server's response
*******************************************************************************/
Sockets::serverResponse Sockets::writeandread(int sockfd, string request){
	int bytes_read = 0, i = 0, j = 0;
	serverResponse response;
	char serverResponse[RESPONSEBUFSIZE];
	char userRequest[request.length() + 1];
	strcpy(userRequest, request.c_str());
	int n = write(sockfd, userRequest, strlen(userRequest));
    if (n < 0)
    	throw runtime_error("ERROR writing to server");
    bzero(serverResponse, RESPONSEBUFSIZE);
    n = 1;
    char *temp = (char *)malloc(sizeof(char) * 0);
    cout << "Started reading" << endl;
    while ((n = read(sockfd, serverResponse, RESPONSEBUFSIZE))> 0){
    	bytes_read += n;
        temp = (char *)realloc(temp, sizeof(char) * bytes_read);
        while(i != bytes_read){
            temp[i] = serverResponse[j];
            i++;
            j++;
        }
        j = 0;
        bzero(serverResponse, RESPONSEBUFSIZE);
    }
    cout << "Done reading" << endl;
    if (n < 0)
        throw runtime_error("ERROR reading from client");
    response.bytes_read = bytes_read;
    response.data = temp;
    cout << "Wrote and read from server" << endl;
    return response;
}

/*******************************************************************************
	Function: writetoclient
	Puroprse: writes a response to the server
*******************************************************************************/
void Sockets::writetoclient(int sockfd,  serverResponse response){
	int n = write(sockfd, response.data, response.bytes_read);
    if (n < 0) 
        throw runtime_error("ERROR writing to client");
    cout << "Wrote to client" << endl;
}









