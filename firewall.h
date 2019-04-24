#ifndef FIREWALL_H_
#define FIREWALL_H_



#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <exception>
//#include <sockets.h>
#include <unordered_set>
#include <vector>
#include <fstream>

#define  LOG_FILE "FileLog.txt"

using namespace std;

class Firewall{
public: 
		Firewall();

		/**********************************
		 *Default destructor calls the destructor for the unordered set
		 *
		 *
		 **********************************/
		~Firewall();



		/**********************************
		 *FillTable populates the unordered set with elements
		 *from the unordered set
		 *
		 *
		 *
		 **********************************/
		void fillTable();



		/**********************************
		 *Lookup: queries if a hostname is in the unordered set
		 *Returns: True if the hostname is in the set and false otherwise
		 *
		 **********************************/
		bool lookup(string hostname);

		/**********************************
		 *addToBlackList: Function adds a hostname to the file and to the unordered set
		 *Returns: returns true if it successfully writes to the file and false if it was unable to
		 *
		 **********************************/
		bool addToBlackList(string newEntry);

		/**********************************
		 *Function clears the unordered set. 
		 *
		 *
		 **********************************/
		void clear();

private:
		/*unordered set holds a list of hostnames that the proxy should avoid*/
		unordered_set <string> RequestLog;





};
#endif