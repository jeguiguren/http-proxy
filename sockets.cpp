#include "sockets.h"
using namespace std;




int write_message(int sockfd, char *message, int messageSize, int append) {
    if (append) {
        message = strcat(message, "\r\n\r\n");
        messageSize += 4;    
    }
    cout << "Writing message of size " << messageSize << endl;
    int n = write(sockfd, message, messageSize);
    if (n < 0)
        throw runtime_error("ERROR writing to server");
    return n;
}

int read_message(int sockfd, char **message, int scale) {
    int n = 0, received = 0;
    char *buffer = (char*) malloc(scale);
    int bufSize = scale;
    while ((n = read(sockfd, (void *)(buffer + received), scale)) == scale) {
        received += n;
        bufSize += scale;
        buffer = (char*) realloc(buffer, bufSize);
    }
    received += n;
    if (n < 0) 
        throw runtime_error("ERROR reading from socket");
    *message = buffer;
    cout << "Read message of size " << received << endl;
    return received;
}

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
	cout << "Processing request" << endl;
    userRequest clientRequest;
    char *request = NULL;
	int bytes_read = read_message(client_fd, &request, REQUESTBUFSIZE);
    clientRequest.bytes_read = bytes_read;
    clientRequest.request = request;
    clientRequest.hostname = gethostname(request);
    clientRequest.portno = getportno(request);
    return clientRequest;
}

/*******************************************************************************
    Function: process_request
    Puroprse: writes a request to a server and return the server's response
*******************************************************************************/
Sockets::serverResponse Sockets::process_request(userRequest request) {
    //fprintf(stdout, "In proxy client with host -%s-\n", host);
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    struct hostent *server = gethostbyname(request.hostname);
    if (server == NULL)
        error("ERROR, no such host\n");

    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(request.portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    
    write_message(sockfd, request.request, request.bytes_read, 1);
    char *response = NULL;
    int response_size = read_message(sockfd, &response, RESPONSEBUFSIZE);
    close(sockfd); //TODO: hash connections

    serverResponse resp = {response_size, response};

    return resp;
}



/*******************************************************************************
	Function: writetoclient
	Puroprse: writes a response to the server
*******************************************************************************/
void Sockets::writetoclient(int sockfd,  serverResponse response){
    int n = write_message(sockfd, response.data, response.bytes_read, 0);
    cout << "Wrote to client " << n << endl;
}









