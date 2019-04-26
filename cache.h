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

	struct cacheResponse{
		char *data;
		int data_length;
	};

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

	/******************************
		Function: dataInCache
		Parameters: name: key of the element to be searched for
		Returns: true if the element exists and false otherwise
		Purpose: searches for the specified element in the cache
	*******************************/
	bool dataInCache(char *name);


	/******************************
		Function: getDataFromCache
		Parameters: name: key of the element to be returned
		Returns: data associated with HTTP/HTTPS request (name)
		Purpose: get an element from the cache
	*******************************/
	cacheResponse getDataFromCache(char *name);

private:
	enum class Event { CND, DD, RD};
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

	unordered_map<string, dataCacheNode> dataCache;

	/*data in the cache that has not been accessed in time >= OLDTIME is 
	  considered obselete*/
	static const int LEEWAY = 10;
	static const int OLDTIME = 7200;
	static const int MAXDATACACHESIZE = 3;

	/******************************
		Function: removeOldData
		Parameters: none
		Returns: nothing
		Purpose: deletes data that has not been accessed in a while;
	*******************************/
	void removeOldData();

	/******************************
		Function: logEvent
		Parameters: event: event to be logged
					cacheElement: element event happened to
		Returns: 
		Purpose: logs a cache event in cachelog.txt
	*******************************/
	void logEvent(Event event, string cacheElement);
	bool stale(string key);
	double getPriority(dataCacheNode data);
};
#endif