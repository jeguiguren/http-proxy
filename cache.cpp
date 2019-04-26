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

bool Cache::stale(string key){
	dataCacheNode element = dataCache.at(key);
	int currentTime = time(NULL);
	if ((currentTime - element.timeStored) >= (element.TTL - LEEWAY)){
		return true;
	}
	return false;
}

/***************************
	Function: cacheElement
	Puroprse: adds an element to the cache
******************************/
void Cache::cacheElement(char *name, char *userRequest, char *data, int TTL, int dataLength){
	string key = name;
	if (dataInCache(name)){
		if(!stale(key))
			return;
	}
	if (dataCache.size() >= MAXDATACACHESIZE)
		removeOldData();
	cout << "Caching :" << key << endl;
	Event event = Event::CND;
	int timeStored = time(NULL);
	int *lastAccesed = new int;
	*lastAccesed = timeStored;
	int *one = new int;
	*one = 1;
	dataCache[key] = dataCacheNode{name, userRequest, data, timeStored, TTL, lastAccesed, one, dataLength};
	logEvent(event, key);
}

/******************************
	Function: dataInCache
	Purpose: seraches for the specified element in the cache
*******************************/
bool Cache::dataInCache(char *name){
	string key = name;
	try{
		dataCache.at(key);
	}catch(...){
		return false;
	}
	if (stale(key))
		return false;
	return true;
}

/******************************
	Function: getDataFromCache
	Purpose: returns the data of the element associated with name in the cache
*******************************/
Cache::cacheResponse Cache::getDataFromCache(char *name){
	string key = name;
	cout << "Trying to get: " << key << "from the cache" << endl;
	Event event = Event::RD;
	dataCacheNode element = dataCache.at(key);
	char *data = element.data;
	*(element.hitRate) = *(element.hitRate) + 1;
	logEvent(event, key);
	return cacheResponse{data, element.dataLength};
}

struct datapriority{
	double priority;
	string datakey;
};

bool operator <(const datapriority& first, const datapriority& second){
	return first.priority < second.priority;
}

double Cache::getPriority(dataCacheNode data){
	//TO-DO: this is in seconds maybe cahnge it to minutes
	int currentTime = time(NULL);
	int timeIncache = currentTime - data.timeStored;
	return  ((*(data.hitRate) / timeIncache) * (1 / data.dataLength));
}

/******************************
	Function: removeOldData
	Purpose: removes data that has not been accessed in a while from the cache
*******************************/
void Cache::removeOldData(){
	Event event = Event::DD;
	priority_queue<datapriority, vector<datapriority>, less<datapriority>> pq;
	double priority;
	for (auto x: dataCache){
		priority = getPriority(x.second);
		pq.push(datapriority{priority, x.second.name});
	}
	datapriority toRemove;
	while (!(pq.empty())){
		toRemove = pq.top();
		pq.pop();
	}
	dataCache.erase(toRemove.datakey);
	logEvent(event, toRemove.datakey);
}

ofstream openFile(){
	ofstream outfile;
	outfile.open(FILENAME, std::ofstream::out | std::ofstream::app);
	if (!outfile.is_open())
	{
		throw runtime_error("Failed to open cachelog.txt");
	}
	return outfile;
}

string getTime(){
	string currentTime;
	time_t rawtime;
  	struct tm * timeinfo;
 	time(&rawtime);
  	timeinfo = localtime(&rawtime);
  	currentTime =  asctime(timeinfo);
  	return currentTime;
}

/******************************
	Function: logEvent
	Purpose: logs a cache event in cachelog.txt
*******************************/
void Cache::logEvent(Event event, string cacheElement){
	string currentTime = getTime();
	string toWrite;
	ofstream outfile;
	try{
		outfile = openFile();
	}catch(...){
		return;
	}
	switch (event){
		case Event::CND :
				toWrite = cacheElement + " was added to the data cache.";
				break;
		case Event::DD :
				toWrite = cacheElement + " was deleted from the data cache.";
				break;
		case Event::RD :
				toWrite = cacheElement + " was gotten from the data cache.";
				break;
		default: return; 
	}
	toWrite = currentTime.substr(0, currentTime.length() - 1) + " : " + toWrite;
	outfile << "________________________________________" <<
	           "________________________________________" << endl;
	outfile << toWrite << endl;
	outfile.close();
}