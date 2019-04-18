#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <netdb.h>
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <exception>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;


class Sockets{
public:

	/***************************
		Function: constructor
		Parameters: none
		Returns: Nothing
		Puroprse: creates instance of this class
	******************************/
	Sockets();

	/***************************
		Function: destructor
		Parameters: none
		Returns: Nothing
		Puroprse: deletes all memory used on the heap
	******************************/
	~Sockets();

	struct userRequest{
		char *request;
		char *hostname;
		int portno;
	};

	struct serverResponse{
		int bytes_read;
		char *data;
	};

	void error(string msg);

	/***************************************************************************
		Function: create_proxy_address
		Parameters: portno: port number the proxy is meant to serve from
		Returns: the file descriptor for the socket
		Puroprse: creates a proxy socket
	***************************************************************************/
	int create_proxy_address(int portno);

	/***************************************************************************
		Function: accept_new_connection
		Parameters: file descriptor that the connection is to be accepted from
		Returns: the new connection
		Puroprse: accepts a new user connection
	***************************************************************************/
	int accept_new_connection(int listen_sock);

	/***************************************************************************
		Function: get_client_request
		Parameters: file descriptor to the socket data is to be read from
		Returns: the data (request) sent from a client
		Puroprse: reads and returns a client request;
	***************************************************************************/
	userRequest get_client_request(int client_fd);

	/***************************************************************************
		Function: connect_to_server
		Parameters: portno: portno server is on
					host_address: server address
		Returns: sockfd associated with the server
		Puroprse: connects to the server
	***************************************************************************/
	int connect_to_server(int portno, string host_address);

	/***************************************************************************
		Function: writeandread
		Parameters: sockfd: server file descriptor
					request: request made to sever
		Returns: data read from server
		Puroprse: writes a request to a server and return the server's response
	***************************************************************************/
	serverResponse writeandread(int sockfd, string request);

	/***************************************************************************
		Function: writetoclient
		Parameters: sockfd: client file descriptor
					response: response from server
		Returns: nothing
		Puroprse: writes a response to the server
	***************************************************************************/
	void writetoclient(int sockfd, serverResponse response);
private:

	static const int REQUESTBUFSIZE = 1024;
	static const int RESPONSEBUFSIZE = 1000;

};
#endif