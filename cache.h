#ifndef CACHE_H_
#define CACHE_H_

#include <queue>
#include <vector>
#include <string>
#include <time.h> 
#include <locale>
#include <stdio.h>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <exception>
#include <functional>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unordered_map>


using namespace std;

//Confirm if dataSize if needed cause it can be just the size of the string
const char FILENAME[13] = "cachelog.txt";
class Cache{
public:
	/***************************
		Function: constructor
		Parameters: none
		Returns: Nothing
		Puroprse: creates instance of this class
	******************************/
	Cache();

	/***************************
		Function: destructor
		Parameters: none
		Returns: Nothing
		Puroprse: deletes all memory used on the heap
	******************************/
	~Cache();

	/***************************
		Function: cacheElement
		Parameters: name: key of the element to be cached
					userRequest: the request gotten from the user
					data: data to be chached
					hostKey: key used to cache the TCP connection in the server
							 cache
					TTL: time to stay in cache
		Returns: Nothing
		Puroprse: adds the server response to the cache if it doesn't exist or
				  updates it if it already exists
	******************************/
	void cacheElement(char *name, char *userRequest, char *data, int TTL, int dataLength);


	/***************************
		Function: cacheConnection
		Parameters: key: key of the element to be cached (hostname concatenated 
						 with port number)
					sockfd: file descriptor for a connection
		Returns: Nothing
		Puroprse: adds a new open TCP connection to the connection cache
	******************************/
	//void cacheConnection(string key, int sockfd, sockaddr_in serveradd);
	void cacheConnection(string key, int sockfd);

	/******************************
		Function: dataInCache
		Parameters: name: key of the element to be searched for
		Returns: true if the element exists and false otherwise
		Purpose: seraches for the specified element in the cache
	*******************************/
	bool dataInCache(char *name);

	/******************************
		Function: availableConnection
		Parameters: key: key of the connection to be searched for
		Returns: true if the connection exists and false otherwise
		Purpose: seraches for the specified connection in the cache
	*******************************/
	bool availableConnection(string key);

	/******************************
		Function: getDataFromCache
		Parameters: name: key of the element to be returned
		Returns: data associated with HTTP/HTTPS request (name)
		Purpose: get an element from the cache
	*******************************/
	char* getDataFromCache(char *name);

	/******************************
		Function: getDataFromCache
		Parameters: name: key of the connection to be returned
		Returns: file descriptor of the connection
		Purpose: get an open tcp connection associated with the key if it exists
	*******************************/
	int getTcpConnection(string key);

	/******************************
		Function: upDatecache
		Parameters: 
		Returns: updates stale data in the cache
	*******************************/
	void upDatecache();

private:
	enum class Event { CND, DD, RD, UD, FUD, CNC, RC };
	//typedef struct sockaddr_in sockaddr_in;
	struct dataCacheNode {
		char *name;
		char *userRequest;
		char *data;
		int timeStored;
		int TTL;
		int *lastAccessed;
		int *hitRate;
		int dataLength;
	}; 

	struct tcpConnectionsNode{
		string key;
		int sockfd;
	};

	unordered_map<string, dataCacheNode> dataCache;
	unordered_map<string, tcpConnectionsNode> tcpConnections;

	/*data in the cache that has not been accessed in time >= OLDTIME is 
	  considered obselete*/
	static const int LEEWAY = 10;
	static const int OLDTIME = 7200;
	static const int MAXDATACACHESIZE = 50;

	/******************************
		Function: removeOldData
		Parameters: none
		Returns: nothing
		Purpose: deletes data that has not been accessed in a while;
	*******************************/
	void removeOldData();

	/******************************
		Function: existsInCahe
		Parameters: key: key to be searched for
					data: true if we are seraching the datacache, false otherwise
		Returns: bool
		Purpose: checks if the key exists in the specified cache
	*******************************/
	bool existsInCahe(string key, bool data);

	/******************************
		Function: logEvent
		Parameters: event: event to be logged
					cacheElement: element event happened to
		Returns: 
		Purpose: logs a cache event in cachelog.txt
	*******************************/
	void logEvent(Event event, string cacheElement);

	double getPriority(dataCacheNode data);
	void updateElement(string key);
};
#endif