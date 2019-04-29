#ifndef SOCKETS_H_
#define SOCKETS_H_

#include "cache.h"
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
#include <unordered_map> 
#include <fcntl.h>

using namespace std;
#define MSG_NOSIGNAL 0x2000 /* don't raise SIGPIPE */


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
		int isHttps;
	};

	struct serverResponse{
		int bytes_read;
		char *data;
		int bytes_last_read;
		int waitTime;
		int bytes_written;
		int last_written;
		bool done;
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
		Function: process_request
		Parameters: socket to read from and flag to set if request is https
		Returns: newly opened server socket
		Purpose: Reads from client and opens server socket; sets flag accordingly
	***************************************************************************/
	int process_request(int client_fd, int *isHttps);

	/***************************************************************************
		Function: transfer
		Parameters: sockfd: client file descriptor
					response: response from server
		Returns: num bytes transfered (0 if transfer done)
		Purpose: read from server and write to client (caches if http)
	***************************************************************************/
	int transfer(int serverSock, int clientSock);

	bool readWrite(int serverSock, int clientSock);

	char* getObjectname(char *request, int portnum);

private:
	int myPort;
	Cache sessionCache;
	unordered_map<int, userRequest> serverReq; //maps server sockets to Request
	unordered_map<int, userRequest>::iterator serverReqIter;

	unordered_map<int, int> httpsPairs; // double mapping of https-connected devices
	unordered_map<int, int>::iterator httpsPairsIter;
	
	unordered_map<int, serverResponse> serverResp; //maps server sockets to Responses (partial or complete)
	unordered_map<int, serverResponse>::iterator serverRespIter;


	/***************************************************************************
		Function: get_client_request
		Parameters: file descriptor to the socket data is to be read from
		Returns: the data (request) sent from a client
		Puroprse: reads and returns a client request;
	***************************************************************************/
	userRequest get_client_request(int client_fd);

	/***************************************************************************
		Function: connect_to_server
		Parameters: request: client's request as struct
		Returns: sockfd associated with the server
		Puroprse: opens up new socket for server
	***************************************************************************/
	int connect_to_server(userRequest request);



	void free_request(userRequest *req);
	static const int REQUESTBUFSIZE = 4096;
	//bandwidth in seconds
	static const int BANDWIDTHLIMIT = 4000;
	static const int RESPONSEBUFSIZE = 4096;
};
#endif