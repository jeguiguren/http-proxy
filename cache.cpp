#include "cache.h"
using namespace std;

/***************************
	Function: constructor
	Puroprse: creates instance of this class
******************************/
Cache::Cache(){
}

/***************************
	Function: destructor
	Puroprse: deletes all memoer used on the heap
******************************/
Cache::~Cache(){
}

/***************************
	Function: cacheElement
	Puroprse: adds an element to the cache
******************************/
Cache::cacheElement(string name, string data, int TTL){
	int timeStored = time(NULL);
	table[name] = cacheNode(name, data, timeStored, TTL);
}

/******************************
	Function: dataInCache
	Purpose: seraches for the specified element in the cache
*******************************/
bool Cache::dataInCache(string name){
	try{
		table.at(name);
	}catch(...){
		return false;
	}
}

/******************************
	Function: getDataFromCache
	Purpose: returns the data of the element associated with name in the cache
*******************************/
string Cache::getDataFromCache(string name){
	cacheNode element = table.at(name);
	string data = element.data;
	int age = time(NULL) - element.timeStored;
	/*TO-DO: Add age to header and handle edge cases(age already exists in 
			 header from another cache)*/
	return data;
}

/******************************
	Function: getAllStaleData
	Parameters: none
	Returns: vector containing stale data
	Purpose: returns a vector containing all the data in the list that 
			 has gone stale
*******************************/
vector<string> Cache::getAllStaleData(){
	vector<string> listOfStale;
	(for auto x: table){
		if ((time(NULL) - x.second.timeStored) >= (x.second.TTL - LEEWAYTIME))
			listOfStale.push_back(x.second.data);
	}
	return listOfStale;
}


