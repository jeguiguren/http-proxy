#include "firewall.h"
#include <iostream>

using namespace std;



Firewall::Firewall() {
	
}

Firewall::~Firewall() {

}


void Firewall::fillTable()
{
	ifstream readFile;
	readFile.open(LOG_FILE);
	if(readFile.fail()) {
		cerr << "Unable to open Log file for reading\n";
		exit(1);
	}
	string newEntry;
	readFile >> newEntry;
	while(readFile.is_open()) {
		if(readFile.eof())
			break;
		if(RequestLog.find(newEntry) == RequestLog.end())
			RequestLog.insert(newEntry);

		readFile >> newEntry;
	}

	readFile.close();
}

bool Firewall::lookup(string hostname)
{
	if(RequestLog.find(hostname) != RequestLog.end())
		return true;
	else
		return false;	
}

bool Firewall::addToBlackList(string newEntry)
{
	RequestLog.insert(newEntry);
	ofstream writeFile;

	writeFile.open(LOG_FILE, ofstream::app);
	if(!writeFile.is_open())
		return false; 

	writeFile << newEntry << '\n';
	writeFile.close(); 

	return true; 
}



void Firewall::clear()
{
	RequestLog.clear();
	return;
}