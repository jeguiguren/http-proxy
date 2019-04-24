#include "firewall.h"
#include <iostream>
#include <cstdlib>
#include <fstream> 



using namespace std; 

int main()
{
	Firewall myFirewall;
	myFirewall.addToBlackList("facebook.com");
	myFirewall.addToBlackList("xvideos.com");
	myFirewall.addToBlackList("amazon.com");


	if(myFirewall.lookup("facebook.com"))
		cout << "successfully added 'facebook.com'\n";

	if(myFirewall.lookup("xvideos.com"))
		cout << "successfully added 'xvideos.com'\n";

	if(myFirewall.lookup("amazon.com"))
		cout << "successfully added 'amazon.com'\n";

	myFirewall.clear();
	myFirewall.fillTable();

	if(myFirewall.lookup("facebook.com"))
		cout << "successfully readded 'facebook.com'\n";

	if(myFirewall.lookup("xvideos.com"))
		cout << "successfully readded 'xvideos.com'\n";

	if(myFirewall.lookup("amazon.com"))
		cout << "successfully readded 'amazon.com'\n";



	return 0;

}