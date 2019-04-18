#ifndef BANDWIDTH_H_
#define BANDWIDTH_H_

#include <iostream> 
#include <thread> 
#include <unistd.h>
#include <queue>

using namespace std;

/*
Read & Write in fixed-size chunks
Proxy -> Cache -> Bandwidth limit (socket) -> Request -> Apply delay to response ->
If not cached, proxy directs request to bandwidth module;


Fair Queueing implementation based on Package Content-Length (included in response header)

Bandwidth maintains 3 threads:
- 1 processes priority requests
- 2 processes secondary requests
- 3 adds new requests to appropriate queue
	- Computes client rate
	- 
Note: Bandwidth only applies when retrieving uncached elements
*/


class Bandwidth{

public:
	/*
	Construct new instance
	*/
	Bandwidth();
	
	/*
	Free heap-allocated memory and close threads
	*/
	~Bandwidth();

	/*
	Hashes socket-reqp in map, computes socket's rate, and adds it to appropriate queue
	*/
	void process_request(void *reqp, int socket);

	/*

	*/
	int get_response(void *resp);
	
private:
	int get_next_priority();
	int get_next_secondary();
	int header_size = 64;
	int pack_size = 512;

	//thread th_priority(foo, 3, 2, 1);
	//thread th_secondary(foo, 3, 3, 2);
	//thread th_requests(foo, 3, 3, 2);
	queue <int> PRIORITYQ;
	queue <int> SECONDARYQ;
};
#endif

