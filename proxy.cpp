//#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include "util.h"
#include "cache.h"
#define REQUEST_SIZE 512
using namespace std;


/* 
int REQUEST_SIZE = 512;
int RESPONSE_SIZE = 2e6; // 1MB



void write_message(int sockfd, char *request) {
	//request = strcat(request, "\r\n\r\n");
	int n = write(sockfd, request, strlen(request));
	//fprintf(stderr, "Writing message: %s", request);
	if (n < 0) 
		 error("ERROR writing to socket");
	//fprintf(stdout, "Request Sent\n");
}


int read_message(int sockfd, char **response) {
	//fprintf(stdout, "Reading from socket %d...\n", sockfd);
	int n = 0;
	int received = 0;
	int curr_size = RESPONSE_SIZE;
	int old_size = 0;
	char *buffer = malloc(curr_size);
	while ((n = read(sockfd, (void *)(buffer + old_size), RESPONSE_SIZE)) > 0) {
		received += n;
		old_size = curr_size; 
		curr_size = expand_buffer((void **) &buffer, old_size, RESPONSE_SIZE);
	}
	if (n < 0) 
		error("ERROR reading from socket");
	*response = buffer;
	return received;
}


int proxy_client(int port_num, char *host, char *request, char **response) {
	
	//fprintf(stdout, "In proxy client with host -%s-\n", host);
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");

	struct hostent *server = gethostbyname(host);
	if (server == NULL) {
		error("ERROR, no such host\n");
	}	
	struct sockaddr_in serv_addr;
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(port_num);
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		error("ERROR connecting");
	
	write_message(sockfd, request);
	int response_size = read_message(sockfd, response);
	close(sockfd);
	return response_size;
}




void serve_indefinitely(int sockfd) {	

	//cache a_cache = new_cache();
	void *a_cache = NULL;
	int n, cl_sockfd;
	struct sockaddr_in cl_addr;
	socklen_t cl_len = sizeof(cl_addr);
	char *request = malloc(REQUEST_SIZE);

	while (1) {
		memset(request, 0, REQUEST_SIZE);
	
		//fprintf(stdout, "\n-----\nListening for next client...\n");
		cl_sockfd = accept(sockfd, (struct sockaddr *) &cl_addr, &cl_len);
		if (cl_sockfd < 0) 
			error("ERROR on accept");

		n = read(cl_sockfd, request, REQUEST_SIZE);
		if (n < 0)
			error("ERROR on reading request");

		//fprintf(stdout, "Request:\n%s", request);

		char *get = get_substr(request, "GET ", "\r\n", 0);
		if (!get)
			error("Undefined Get request");
		char *host = get_substr(request, "Host: ", "\r\n", 6);
		if (!host)
			error("Undefined Host Name");

		int port = 80;
		char *portstr = get_substr(host, ":", NULL, 1);
		fprintf(stdout, "Original: %s\n", host);
		if (portstr != NULL){
			port = atoi(portstr);
			host = strtok(host, ":"); 
		}
		fprintf(stdout, "Host: %s, Port: %d\n", host, port);
		
		char *response = NULL;
		int cached = 1;
		//int response_size = proxy_client(80, host, request, &response);
		
		int response_size = cache_fetch(a_cache, port, get, &response);
		if (response_size < 0) {
			cached = 0;
			response_size = proxy_client(port, host, request, &response);
			cache_store(a_cache, port, get, response, response_size);
		} 
		//fprintf(stdout,"Writing Response of size: %d\n", response_size);
		n = write(cl_sockfd, response, response_size);
		if (n < 0) 
			error("ERROR writing to socket");
		
		if (cached)
			free(response);
		close(cl_sockfd);
	}
	free(request);
}

*/

void proxy_server(int port_num) {
	struct sockaddr_in se_addr;


	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sockfd < 0) 
		cout << "ERROR opening socket";

	se_addr.sin_family = AF_INET;
	se_addr.sin_addr.s_addr = INADDR_ANY;
	se_addr.sin_port = htons(port_num);

	if (::bind(sockfd, (struct sockaddr *) &se_addr, sizeof(se_addr)) < 0) 
		cout << "ERROR on binding";

	listen(sockfd, 5);
	serve_indefinitely(sockfd);
	close(sockfd);	
	
}


int main(int argc, char *argv[])
{
	if (argc != 2) {
	   fprintf(stderr,"usage: %s port\n", argv[0]);
	   exit(0);
	}
	
	proxy_server(atoi(argv[1]));
	return 0;
}


