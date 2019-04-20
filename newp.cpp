
#include <stdio.h>  
#include <string.h>   //strlen  
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>   //close  
#include <arpa/inet.h>    //close  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros  
#include <sys/time.h>
#include "cache.h"
#include "util.h"
#include <netdb.h>

#define PORT 9051
#define MAX_CLIENTS 50
#define REQSIZE 5000
#define RESPONSEBUFSIZE 1000


int write_message(int sockfd, char *message, int messageSize) {
    cout << "Writing message of size " << messageSize << endl;
    int n = write(sockfd, message, messageSize);
    if (n < 0)
        throw runtime_error("ERROR writing to server");
    cout << "out\n";
    return n;
}

int read_message(int sockfd, char **message, int scale) {
    cout << "Reading\n";
    int n = 0, received = 0;
    char *buffer = (char*) malloc(scale);
    int bufSize = scale;
    while ((n = read(sockfd, (void *)(buffer + received), scale)) > 0) {
        cout << "loop\n";
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


void process_request(char *req, int reqSize) {

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        throw runtime_error("ERROR opening socket");

    struct hostent *server = gethostbyname(gethostname(req));
    if (server == NULL)
        throw runtime_error("ERROR, no such host\n");

    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(getportno(req));
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        throw runtime_error("ERROR connecting");
    
    write_message(sockfd, req, reqSize);
    char *response = NULL;
    int response_size = read_message(sockfd, &response, RESPONSEBUFSIZE);
    close(sockfd); //TODO: hash connections

}


void serve_indefinitely() {

    int opt = true;   
    int master_socket , addrlen , new_socket , client_socket[MAX_CLIENTS] ,  
        activity, i , valread , sd;   
    int max_sd;   
    struct sockaddr_in address;   
         
         
    //set of socket descriptors  
    fd_set readfds;    
     
    //initialise all client_socket[] to 0 so not checked  
    for (i = 0; i < MAX_CLIENTS; i++)   
        client_socket[i] = 0;   
         
    //create a master socket  
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)   
        error("socket failed");   
     
    //set master socket to allow multiple connections ,  
    //this is just a good habit, it will work without this  
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,  
          sizeof(opt)) < 0 )
        error("setsockopt");   
        
     
    //type of socket created  
    address.sin_family = AF_INET;   
    address.sin_addr.s_addr = INADDR_ANY;   
    address.sin_port = htons( PORT );   
         
    //bind the socket to localhost port 8888  
    if (::bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)   
    {   
        perror("bind failed");   
        exit(EXIT_FAILURE);   
    }   
    printf("Listener on port %d \n", PORT);   
         
    //try to specify maximum of 3 pending connections for the master socket  
    if (listen(master_socket, 3) < 0)   
    {   
        perror("listen");   
        exit(EXIT_FAILURE);   
    }   
         
    //accept the incoming connection  
    addrlen = sizeof(address);   
    puts("Waiting for connections ...");  

    while(1)   
    {   
        //clear the socket set  
        FD_ZERO(&readfds);   
     
        //add master socket to set  
        FD_SET(master_socket, &readfds);   
        max_sd = master_socket;   
             
        //add child sockets to set  
        for ( i = 0 ; i < MAX_CLIENTS ; i++)   
        {   
            //socket descriptor  
            sd = client_socket[i];   
                 
            //if valid socket descriptor then add to read list  
            if(sd > 0)   
                FD_SET( sd , &readfds);   
                 
            //highest file descriptor number, need it for the select function  
            if(sd > max_sd)   
                max_sd = sd;   
        }   
     
        //wait for an activity on one of the sockets , timeout is NULL ,  
        //so wait indefinitely  
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);   
       
        if ((activity < 0) && (errno!=EINTR))   
        {   
            printf("select error");   
        }   
             
        //If something happened on the master socket ,  
        //then its an incoming connection  
        if (FD_ISSET(master_socket, &readfds))   
        {   
            if ((new_socket = accept(master_socket,  
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)   
            {   
                perror("accept");   
                exit(EXIT_FAILURE);   
            }   
             
            //inform user of socket number - used in send and receive commands  
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , 
                    new_socket , inet_ntoa(address.sin_addr) , ntohs 
                  (address.sin_port));   
                            
            //add new socket to array of sockets  
            for (i = 0; i < MAX_CLIENTS; i++)   
            {   
                //if position is empty  
                if( client_socket[i] == 0 )   
                {   
                    client_socket[i] = new_socket;   
                    printf("Adding to list of sockets as %d\n" , i);   
                         
                    break;   
                }   
            }   
        }   
             
        //else its some IO operation on some other socket 
        for (i = 0; i < MAX_CLIENTS; i++)   
        {   
            sd = client_socket[i];   
                 
            if (FD_ISSET( sd , &readfds))   
            {   
                //Check if it was for closing , and also read the  
                //incoming message  
                cout << "processing request for .." << i << "\n";

                char *req = (char *) malloc(REQSIZE);
                valread = read(sd, req, REQSIZE);
                if (valread < 0)
                    perror("Error on read");
                else if (valread == 0) {
                    //Close the socket and mark as 0 in list for reuse  
                    close( sd );   
                    client_socket[i] = 0;   
                }   
                else {
                    process_request(req, valread);
                }
            }   
        }   
    }   

}




int main(){
    serve_indefinitely();
}

