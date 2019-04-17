#include "cache.h"
#include "sockets.h"

int main(int argc, char **argv){
    Sockets session;
    Sockets::userRequest clientRequest;
    Sockets::serverResponse response;
    fd_set master_fd_set, copy_fd_set;
    int listen_sock, max_fd, new_sock_fd, server_fd;
    if (argc != 2) {
      fprintf(stderr, "usage: %s <port>\n", argv[0]);
      exit(1);
    }
    listen_sock = session.create_proxy_address(atoi(argv[1]));
    max_fd = listen_sock;
    FD_ZERO (&master_fd_set);
    FD_SET (listen_sock, &master_fd_set);
    while(1){
        FD_ZERO (&copy_fd_set);
        memcpy(&copy_fd_set, &master_fd_set, sizeof(master_fd_set));
        cout << "Before select" << endl;
        if (select (max_fd + 1, &copy_fd_set, NULL, NULL, NULL) < 0){
          session.error ("ERROR on select");
        }
          //TO-DO: call update cache here
        for (int sock_fd = 0; sock_fd < max_fd + 1; sock_fd++){
          if (FD_ISSET (sock_fd, &copy_fd_set)){
            if (sock_fd == listen_sock){
                new_sock_fd = session.accept_new_connection(listen_sock);
                cout << "accepted a connection" << endl;
                if (new_sock_fd > 0){
                    FD_SET(new_sock_fd, &master_fd_set);
                    if (new_sock_fd > max_fd)
                        max_fd = new_sock_fd;
                }else{
                    session.error ("ERROR on accept");
                }
            }else{
                clientRequest = session.get_client_request(sock_fd);
                server_fd = session.connect_to_server(clientRequest.portno,
                                                      clientRequest.hostname);
                response = session.writeandread(server_fd, 
                                                      clientRequest.request);
                session.writetoclient(sock_fd, response);
                close (sock_fd);
                FD_CLR (sock_fd, &master_fd_set);
            }
          }
        }
    }
}