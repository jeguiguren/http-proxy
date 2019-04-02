#ifndef CACHE_H_
#define CACHE_H_

#include <functional>
#include <unordered_map>
#include <iostream>
#include <cstdlib>
#include <time.h> 
#include <locale>
#include<vector>
#include<string>


using namespace std;

//Confirm if dataSize if needed cause it can be just the size of the string

class Indexer{
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
					data: data to be chached
					TTL: time to stay in cache
		Returns: Nothing
		Puroprse: adds the server response to the cache if it doesn't exist or
				  updates it if it already exists
	******************************/
	void cacheElement(string name, string data, int TTL);

	/******************************
		Function: dataInCache
		Parameters: name: key of the element to be searched for
		Returns: true if the element exists and false otherwise
		Purpose: seraches for the specified element in the cache
	*******************************/
	bool dataInCache(string name);

	/******************************
		Function: getDataFromCache
		Parameters: name: key of the element to be returned
		Returns: data associated with HTTP/HTTPS request (name)
		Purpose: get an element from the cache
	*******************************/
	string getDataFromCache(string name);

	/******************************
		Function: getAllStaleData
		Parameters: none
		Returns: vector containing stale data
		Purpose: returns a vector containing all the data in the list that 
				 has gone stale
	*******************************/
	vector<string> getAllStaleData();



private:
	struct cacheNode {
		string name;
		string data;
		int timeStored, TTL;
		cacheNode(string key, string value, int size, int TS, int ttl): 
			name(key), data(value), timeStored(TS), TTL(ttl){}
	};

	unordered_map<string, cacheNode> table;

	static const int LEEWAYTIME = 5;
};
#endif