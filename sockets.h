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
		Parameters: port where master socket is running
		Returns: Nothing
		Puroprse: creates instance of this class
	******************************/
	Sockets(int port);

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
		int bytes_read;
	};

	struct serverResponse{
		int bytes_read;
		char *data;
	};

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
		Function: connect_and_write_to_server
		Parameters: request: client's request as struct
		Returns: sockfd associated with the server
		Puroprse: connects to the server and writes request to server
	***************************************************************************/
	int connect_and_write_to_server(userRequest request);

	/***************************************************************************
		Function: process_request
		Parameters: request: request made to sever
		Returns: data read from server
		Puroprse: writes a request to a server and return the server's response
	***************************************************************************/
	serverResponse process_request(userRequest request);

	/***************************************************************************
		Function: respond
		Parameters: sockfd: client file descriptor
					response: response from server
		Returns: nothing
		Puroprse: writes a response to the server
	***************************************************************************/
	int respond(int serverSock, int clientSock);
private:
	int myPort;
	static const int REQUESTBUFSIZE = 5000;
	static const int RESPONSEBUFSIZE = 16384;
};
#endif