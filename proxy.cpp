#include "sockets.h"
#include <signal.h>
#include "util.h"
#include <unordered_map> 
#import <thread> 

//Treat https clients as servers

int main(int argc, char **argv){

    if (argc != 3) {
      fprintf(stderr, "usage: %s <port> <maxBPS>\n", argv[0]);
      exit(1);
    }

	unordered_map<int, int> senderReceiver;
	unordered_map<int, int>::iterator iter;

	int sender_sock, receiver_sock;
	int myPort = atoi(argv[1]);
    int maxBPS = atoi(argv[2]) * 1000;
    
    Sockets session(myPort, maxBPS);
    int isHttps = 0;
    
    fd_set master_fd_set, copy_fd_set;
    int listen_sock, max_fd, new_sock_fd;
    
    signal(SIGPIPE, SIG_IGN); // Prevent SIG_PIPE
    listen_sock = session.create_proxy_address(myPort);
    max_fd = listen_sock;
    FD_ZERO (&master_fd_set);
    FD_SET (listen_sock, &master_fd_set);
    while(1){
        FD_ZERO (&copy_fd_set);
        memcpy(&copy_fd_set, &master_fd_set, sizeof(master_fd_set));
        cout << "Waiting.............................." << endl;
        if (select (max_fd + 1, &copy_fd_set, NULL, NULL, NULL) < 0){
          	error("ERROR on select");
        }
        cout << "GOT PASSED SELECT" << endl;
        //TO-DO: call update cache here?
        for (int sock_fd = 0; sock_fd < max_fd + 1; sock_fd++){
        	if (FD_ISSET (sock_fd, &copy_fd_set)){
            	if (sock_fd == listen_sock){
                	new_sock_fd = session.accept_new_connection(listen_sock);
                	if (new_sock_fd > 0){
                    	FD_SET(new_sock_fd, &master_fd_set);
                    	if (new_sock_fd > max_fd)
                        	max_fd = new_sock_fd;
                    cout << "New client on socket " << new_sock_fd << endl;
                	}else{
                    	error("ERROR on accept");
                	}
            }else{
            	cout << "Socket ready " << sock_fd << endl;
                try{
                	//New client request
                	// TODO: add socket function to see if cached; else, proceed
                	
                	iter = senderReceiver.find(sock_fd);
                	if (iter == senderReceiver.end()) {
                		cout << "New client request\n";

                        //Returns newly oppened server socket
                        new_sock_fd = session.process_request(sock_fd, &isHttps); 
                        senderReceiver.insert(make_pair(new_sock_fd, sock_fd)); 
                        FD_SET(new_sock_fd, &master_fd_set); 
                        if (new_sock_fd > max_fd)
                            max_fd = new_sock_fd;
                        cout << "New server on socket " << new_sock_fd << endl;

                        if (isHttps)
                            senderReceiver.insert(make_pair(sock_fd, new_sock_fd));
                	} 
                	// Server socket ready to write
                	else {
                		cout << "New Transfer\n";
                		sender_sock = iter->first;
                		receiver_sock = iter->second;
                		if (session.transfer(sender_sock, receiver_sock) == 0) { // Transfer completed
                			cout << "Closing client and server connections\n";
			                close (sender_sock);
			                FD_CLR (sender_sock, &master_fd_set);
                            senderReceiver.erase(sender_sock);
			                close (receiver_sock);
			                FD_CLR (receiver_sock, &master_fd_set);
                            senderReceiver.erase(receiver_sock);
                		}
                	}
                    
                }catch(const std::exception &exc){
                	cerr << exc.what();
                    cout << "Closed connection: " << sock_fd <<endl;
                    close (sock_fd);
                    FD_CLR (sock_fd, &master_fd_set);
                }
            }
          }
        }
    }
}

