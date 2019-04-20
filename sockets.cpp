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
    cout << "Writing message of size " << messageSize << endl;
    int n = write(sockfd, message, messageSize);
    if (n < 0)
        throw runtime_error("Error on write");
    //cout << "done\n";
    return n;
}


int read_message(int sockfd, char **message, int scale) {
    cout << "reading\n";
    int n = 0, received = 0;
    char *buffer = (char*) malloc(scale);
    int bufSize = scale;
    while ((n = read(sockfd, (void *)(buffer + received), scale)) > 0) {
        received += n;
        if (received >= scale){
            *message = buffer;
            return received;
        }
        bufSize += scale;
        buffer = (char*) realloc(buffer, bufSize);
    }
    received += n;
    if (n < 0) 
        throw runtime_error("ERROR reading from socket");
    *message = buffer;
    //cout << "Read message of size " << received << endl;
    return received;
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
    cout << "Receiving request" << endl;
    userRequest clientRequest;
    char *request = (char *)malloc(REQUESTBUFSIZE);
    int bytes_read = read(client_fd, request, REQUESTBUFSIZE);
    cout << "Request of size: " << bytes_read << endl;
    if (bytes_read <= 0)
        throw runtime_error("ERROR reading from client\n");
    clientRequest.bytes_read = bytes_read;
    clientRequest.request = request;
    clientRequest.hostname = gethostname(request);
    clientRequest.portno = getportno(request);

    if ((strcmp(clientRequest.hostname, "localhost") == 0) and 
        clientRequest.portno == myPort)
        throw runtime_error("I am only a proxy");

    return clientRequest;
}


/*******************************************************************************
    Function: connect_and_write_to_server
    Puroprse: connects and writes to the server
*******************************************************************************/
int Sockets::connect_and_write_to_server(userRequest request) {
    //TO-DO: Figure out how to change the parameter to a pointer
    int sockfd;
    string host_name = request.hostname;
    char hostname[host_name.length() + 1];
    strcpy(hostname, host_name.c_str());
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
    serveraddr.sin_port = htons(request.portno);
    if (connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) <0) 
        throw runtime_error("ERROR connecting to server");
    cout << "Connected to server" << endl;
    write_message(sockfd, request.request, request.bytes_read);
    cout << "Wrote to server" << endl;
    return sockfd;
}


/*******************************************************************************
    Function: process_request
    Puroprse: writes a request to a server and return the server's response
*******************************************************************************/
Sockets::serverResponse Sockets::process_request(userRequest request) {
    cout << "Processing request\n"; 
    //fprintf(stdout, "In proxy client with host -%s-\n", host);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        throw runtime_error("ERROR opening socket");

    struct timeval tv;
    tv.tv_sec = 20;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    cout << "Host: "<< request.hostname << ", Port: " << request.portno << endl;

    struct hostent *server = gethostbyname(request.hostname);
    if (server == NULL)
        throw runtime_error("ERROR, no such host\n");

    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(request.portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        throw runtime_error("ERROR connecting");

    write_message(sockfd, request.request, request.bytes_read);

    char *response = NULL;
    int response_size = read_message(sockfd, &response, RESPONSEBUFSIZE);
    //close(sockfd); //TODO: hash connections

    serverResponse resp;
    resp.bytes_read = response_size;
    resp.data = response;
    return resp;
}



/*******************************************************************************
    Function: respond
    Puroprse: reads from Server and writes to client in chunks of at least RESPONSEBUFSIZE (except last chunk)
    Returns: returns true if done transferring data
    TODO must cache it
*******************************************************************************/
int Sockets::respond(int serverSock, int clientSock){
    char *message = NULL;
    int received = read_message(serverSock, &message, RESPONSEBUFSIZE);
    if (received > 0) {
        cout << "Read message of size " << received << endl;
        //int size = read_message(serverSock, &message, RESPONSEBUFSIZE);
        int size = write_message(clientSock, message, received);
        cout << "Wrote " << size << " from " << serverSock << " to " << clientSock << endl;
    }
    free(message);        
    return received < RESPONSEBUFSIZE;
}









