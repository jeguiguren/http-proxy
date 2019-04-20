
#include "cache.h"
#include "sockets.h"
#include "util.h"
#include <unordered_map> 
#import <thread> 

int main(int argc, char **argv){

	unordered_map<int, int> serverClient;
	unordered_map<int, int>::iterator iter;

	int client_sock, server_sock;
	int myPort = atoi(argv[1]);
    Sockets session(myPort);
    Sockets::userRequest clientRequest;
    //Sockets::serverResponse response;
    fd_set master_fd_set, copy_fd_set;
    int listen_sock, max_fd, new_sock_fd;
    if (argc != 2) {
      fprintf(stderr, "usage: %s <port>\n", argv[0]);
      exit(1);
    }
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
          //TO-DO: call update cache here
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
                	// TODO: add socket function to see if cahed; else, proceed
                	
                	iter = serverClient.find(sock_fd);
                	if (iter == serverClient.end()) {
                		cout << "New client request\n";
                    	clientRequest = session.get_client_request(sock_fd);
                    	new_sock_fd = session.connect_and_write_to_server(clientRequest);
                    	//Write to server connect_and_write
                    	serverClient.insert(make_pair(new_sock_fd, sock_fd)); 
                    	FD_SET(new_sock_fd, &master_fd_set); 
                    	if (new_sock_fd > max_fd)
                        	max_fd = new_sock_fd;
                            cout << "New server on socket " << new_sock_fd << endl;
                	} 
                	// Server socket ready to write
                	else {
                		cout << "Server ready to write\n";
                		server_sock = iter->first;
                		client_sock = iter->second;
                		if (session.respond(server_sock, client_sock)) { // returns true if transfer completed
                			cout << "Closing client and server connections\n";
			                close (client_sock);
			                FD_CLR (client_sock, &master_fd_set);
			                close (server_sock);
			                FD_CLR (server_sock, &master_fd_set);
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

