#include <cstdlib>
#include <string>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/wait.h>
#include <netdb.h>
#include <cstring>
#include <unistd.h>
#include <iostream>

#define MAX_BUFSIZE 5000

using namespace std;




int make_server_socket(char* address, char* port);

void write_server(const char* buff_to_server,int sockfd,int buff_length);

void write_clientSock(const char* buff_to_server,int sockfd,int buff_length);

void write_client (int Clientfd, int Serverfd);

string hostname_parser(string buffer, char** server_portno);

void read_client(void* sockid);

int main (int argc, char *argv[]) 
{

	int sockfd, newsockfd;

	struct sockaddr_in serv_addr; 
	struct sockaddr cli_addr;


	if (argc < 2) 
  	{
  		cerr << "usage: \n\t "<< argv[0] << " <portno>\n";
  		exit(1);
  	}


  	sockfd = socket(AF_INET, SOCK_STREAM, 0);   

  	if (sockfd < 0) {
  		cerr << "ERROR while creating a socket\n";
  		exit(1);
  	}

  	memset(&serv_addr,0,sizeof serv_addr);

  	int  portno = atoi(argv[1]);        
 	serv_addr.sin_family = AF_INET;    
  	serv_addr.sin_addr.s_addr = INADDR_ANY;  
 	serv_addr.sin_port = htons(portno); 


 	int n = bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

 	if (n < 0 ) {
 		cerr << "ERROR on binding\n";
  		exit(1);
 	}
  	
  	listen(sockfd, 50); 

  	int clilen = sizeof(struct sockaddr);
  	int maxsockfd;
  	int engagement;
  	fd_set rfds;
  	fd_set copyrfds;
  	FD_ZERO(&rfds);
  	FD_SET(sockfd, &rfds);

  	maxsockfd = sockfd;
  	int sd;

  	while(true) {
  		
  		
  		FD_ZERO(&copyrfds);
  		engagement = select(maxsockfd + 1, &copyrfds, NULL, NULL, NULL);

  		if (engagement == 0) {
  			cerr << "ERROR on select\n";
  			exit(1);
  			
  		}

  		for(sd = 0; sd < maxsockfd + 1; sd++) {
  			if(FD_ISSET(sd, &copyrfds)) {
  				if(sd == maxsockfd) {
  		
			  		newsockfd = accept(sockfd,&cli_addr, (socklen_t*) &clilen); 

			  		if (newsockfd < 0){
			  			cerr << "ERROR on accept call\n";
			 		}
			 		if (newsockfd > 0) {
			 			FD_SET(newsockfd, &rfds);
			 			if(newsockfd > maxsockfd) {
			 				maxsockfd = newsockfd;
			 			}
			 		}

		 		}

		 		else {
		 			read_client((void*)&sd);

		 		}

 			}
 		}

 	}

 	close(sockfd);


	return 0;
}


int make_server_socket(char* address, char* port)
{
	struct addrinfo ahints;
  	struct addrinfo *paRes;

  	int iSockfd;

  	memset(&ahints, 0, sizeof(ahints));
  	ahints.ai_family = AF_UNSPEC;
  	ahints.ai_socktype = SOCK_STREAM;
  	if (getaddrinfo(address, port, &ahints, &paRes) != 0) {
  	 		cerr << "ERROR in server address format\n";
			exit (1);
	}

	  /* Create and connect */
	if ((iSockfd = socket(paRes->ai_family, paRes->ai_socktype, paRes->ai_protocol)) < 0) {
	    cerr << "ERROR in creating socket\n";
		exit (1);
	}
	if (connect(iSockfd, paRes->ai_addr, paRes->ai_addrlen) < 0) {
		cerr << "ERROR on connecting to server\n";
		exit (1);
	}

	  /* Free paRes, which was dynamically allocated by getaddrinfo */
	freeaddrinfo(paRes);

  	return iSockfd;
}

void write_server(const char* buff_to_server,int sockfd,int buff_length)
{
	string temp;

	temp.append(buff_to_server);

	int total_bytes_sent = 0;

	int current_bytes_sent;

	while (total_bytes_sent < buff_length) {
		if ((current_bytes_sent = send(sockfd, (void *) (buff_to_server + total_bytes_sent), buff_length - total_bytes_sent, 0)) < 0) {
			cerr << "ERROR on sending to server\n";
				exit (1);
		}
		total_bytes_sent += current_bytes_sent;

	}	

}

void write_clientSock(const char* buff_to_server,int sockfd,int buff_length)
{

	string temp;

	temp.append(buff_to_server);

	int total_bytes_sent = 0;

	int current_bytes_sent;

	while (total_bytes_sent < buff_length) {
		if ((current_bytes_sent = send(sockfd, (void *) (buff_to_server + total_bytes_sent), buff_length - total_bytes_sent, 0)) < 0) {
			cerr << "ERROR on sending to server\n";
				exit (1);
		}
		total_bytes_sent = total_bytes_sent + current_bytes_sent;

	}	

}


void write_client (int Clientfd, int Serverfd) {
	

	int receiver;
	char buf[MAX_BUFSIZE];

	while ((receiver = recv(Serverfd, buf, MAX_BUFSIZE, 0)) > 0) {
	      write_clientSock(buf, Clientfd,receiver);         // writing to client	    
		memset(buf,0,sizeof buf);	
	}      

	if (receiver < 0) {
		cerr << "ERROR while receiving from the server\n";
	  exit (1);
	}
}


void read_client(void* sockid)
{
	
	int MAX_BUFFER_SIZE = 5000;

	char buf[MAX_BUFFER_SIZE];

	int newsockfd = *((int*)sockid);

	char *request_message;  // Get message from URL

	request_message = (char *) malloc(MAX_BUFFER_SIZE); 

	if (request_message == NULL) {
		cerr << "ERROR with memory allocation\n";
		exit (1);
	}	

	request_message[0] = '\0';

	int total_received_bytes = 0;

	while (strstr(request_message, "\r\n\r\n") == NULL) {  // determines end of request

	  int recvd = recv(newsockfd, buf, MAX_BUFFER_SIZE, 0) ;

	  if(recvd < 0 ){
	  	cerr << "ERROR on receive call\n";
		exit (1);
	  				
	  }else if(recvd == 0) {
	  		break;
	  } else {

	  	total_received_bytes = total_received_bytes + recvd;

	  	/* if total message size greater than our string size,double the string size */

	  	buf[recvd] = '\0';
	  	if (total_received_bytes > MAX_BUFFER_SIZE) {
			MAX_BUFFER_SIZE *= 2;
			request_message = (char *) realloc(request_message, MAX_BUFFER_SIZE);
			if (request_message == NULL) {
				cerr << "ERROR in memory allocation\n";
				exit(1);
			}
		}


	  }

	  strcat(request_message, buf);

	}
	
		
	int iServerfd;

	char * PARSED_HOSTNAME = "after parsing, hostname goes here\n";
	char * PARSED_PORTNUM = "after parsing, portnum goes here\n";

	iServerfd = make_server_socket(PARSED_HOSTNAME, PARSED_PORTNUM);
	

	write_server(browser_req, iServerfd, total_received_bytes);
	write_client(newsockfd, iServerfd);

		
	close(newsockfd);   
	close(iServerfd);


}


