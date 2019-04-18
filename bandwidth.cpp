#include "bandwidth.h"
using namespace std; 
  

#define CHUNK_SIZE 512



Bandwidth::Bandwidth(){
	cout << "Bandwidth starting\n";
}

Bandwidth::~Bandwidth(){
	cout << "Bandwidth cleanup\n";
}




/* Private */
int Bandwidth::get_next_priority() {
	int n = PRIORITYQ.front();
	if (n > 0)
		PRIORITYQ.pop();
	return n;
}


int Bandwidth::get_next_secondary() {
	int n = SECONDARYQ.front();
	if (n > 0)
		SECONDARYQ.pop();
	return n;
}


/*
For each new request, creates a new thread and process it.

void process_request(char *req, int socket);
void response(char *response, int socket);


// A dummy function 
void foo(int reps, int sleep, int name) 
{ 
	int utime = sleep * (10E5);
	for (int i = 0; i < reps; i++) { 
		cout << "Thread " << name << " sleeping " << sleep << "seconds\n";
		usleep(utime);
	} 
} 
 */
