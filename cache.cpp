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
void Cache::cacheElement(string name, string userRequest, 
										  string data, string hostKey, int TTL){
	Event event = Event::CND;
	int timeStored = time(NULL);
	int lastAccesed = timeStored;
	dataCache[name] = dataCacheNode{name, userRequest, data, hostKey, 
					       					   timeStored, TTL, lastAccesed, 1};
	logEvent(event, name);
	if (dataCache.size() >= MAXDATACACHESIZE)
		removeOldData();
}

/***************************
	Function: cacheConnection
	Puroprse: adds a new open TCP connection to the connection cache
******************************/
void Cache::cacheConnection(string key, int sockfd){
	//Test to see if this, especially the serveradd, works perfectly
	Event event = Event::CNC;
	tcpConnections[key] = tcpConnectionsNode{key, sockfd};
	logEvent(event, key);
}

/******************************
	Function: existsInCahe
	Purpose: checks if the key exists in the specified cache
*******************************/
bool Cache::existsInCahe(string key, bool data){
	if (data){
		try{
			dataCache.at(key);
		}catch(...){
			return false;
		}
		return true;
	}else{
		try{
			tcpConnections.at(key);
		}catch(...){
			return false;
		}
		return true;
	}
}

/******************************
	Function: dataInCache
	Purpose: seraches for the specified element in the cache
*******************************/
bool Cache::dataInCache(string name){
	return existsInCahe(name, true);
}

/******************************
	Function: availableConnection
	Parameters: key: key of the connection to be searched for
	Returns: true if the connection exists and false otherwise
	Purpose: seraches for the specified connection in the cache
*******************************/
bool Cache::availableConnection(string key){
	return existsInCahe(key, false);
}

/******************************
	Function: getDataFromCache
	Purpose: returns the data of the element associated with name in the cache
*******************************/
string Cache::getDataFromCache(string name){
	Event event = Event::RD;
	dataCacheNode element = dataCache.at(name);
	string data = element.data;
	dataCache.erase(name);
	//TO-DO: increase hitrate by 1
	cacheElement(name, element.userRequest, data, element.hostKey, element.TTL);
	logEvent(event, name);
	return data;
}

/******************************
	Function: getDataFromCache
	Purpose: get an open tcp connection associated with the key if it exists
*******************************/
int Cache::getTcpConnection(string key){
	Event event = Event::RC;
	logEvent(event, key);
	return tcpConnections.at(key).sockfd;
}

/******************************
	Function: removeOldData
	Purpose: removes data that has not been accessed in a while from the cache
*******************************/
void Cache::removeOldData(){
	Event event = Event::DD;
	int currentTime = time(NULL);
	for (auto x: dataCache){
		if (currentTime - x.second.lastAccessed >= OLDTIME){
			dataCache.erase(x.first);
			logEvent(event, x.first);
		}
	}
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
				toWrite = cacheElement + "was added to the data cache.";
				break;
		case Event::DD :
				toWrite = cacheElement + "was deleted from the data cache.";
				break;
		case Event::RD :
				toWrite = cacheElement + "was gotten from the data cache.";
				break;
		case Event::UD :
				toWrite = cacheElement + "was updated in the data cache.";
				break;
		case Event::FUD :
				toWrite = "Failed to update "+ cacheElement + 
				          "in the data cache.";
				break;
		case Event::CNC :
				toWrite = cacheElement + "connection was added to" + 
				                         " the connection cache.";
			    break;
		case Event::RC :
				toWrite = cacheElement + "connection was deleted from" + 
				                         " the connection cache.";
			    break;
		default: return; 
	}
	toWrite = currentTime.substr(0, currentTime.length() - 1) + " : " + toWrite;
	outfile << "________________________________________" <<
	           "________________________________________" << endl;
	outfile << toWrite << endl;
	outfile.close();
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
	return  ((data.hitRate / timeIncache) * (1 / data.data.length()));
}

void Cache::updateElement(string key){
	dataCacheNode data = dataCache.at(key);
	string request = data.userRequest;
	char charRequest[request.length() + 1];
	strcpy(charRequest, request.c_str());
	//int connection = tcpConnections.at(data.hostKey).sockfd;
	//TO-DO: complete this when you figure out SSL
}

/******************************
	Function: upDatecache 
	Returns: updates stale data in the cache
*******************************/
void Cache::upDatecache(){
	double priority;
	Event event = Event::UD;
	Event event2 = Event::FUD;
	int currentTime = time(NULL), count = 0;
	priority_queue<datapriority, vector<datapriority>, less<datapriority>> pq;
	for (auto x: dataCache){
		if ((currentTime - x.second.timeStored) >= (x.second.TTL - LEEWAY)){
			priority = getPriority(x.second);
			pq.push(datapriority{priority, x.second.name});
		}
	}
	while (!(pq.empty()) && count < 10){
		try{
			updateElement(pq.top().datakey);
			logEvent(event, pq.top().datakey);
		}catch(...){
			logEvent(event2, pq.top().datakey);
		}
		count++;
	}
}