


#include "cache.h"

void error(string msg) {
  char message[msg.length() + 1];
  strcpy(message, msg.c_str());
    perror(message);
  exit(1);
}

int accept_new_connection(int listen_sock);
int create_proxy_address(int portno);

int main(int argc, char **argv){
  //TO-DO: uncomment this when adding back the time stuff
  //struct timeval timeout;
  fd_set master_fd_set, copy_fd_set;
  int listen_sock, max_fd, new_sock;
  if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    listen_sock = create_proxy_address(atoi(argv[1]));
    max_fd = listen_sock;
    while(1){
      //TO-DO: change the timeout to match what you need for updating cache
        //timeout.tv_sec = 5;
      FD_ZERO (&copy_fd_set);
      memcpy(&copy_fd_set, &master_fd_set, sizeof(master_fd_set));
      //TO-DO: switch the select call to select (max_fd + 1, &copy_fd_set, NULL, NULL, &timeout) when adding the time stuff
      if (select (max_fd + 1, &copy_fd_set, NULL, NULL, NULL) < 0){
        cout << "got past select" << endl;
        error ("ERROR on select");
      }
        //TO-DO: call update cache here
      for (int sock_fd = 0; sock_fd < max_fd + 1; sock_fd++){
        if (FD_ISSET (sock_fd, &copy_fd_set)){
          if (sock_fd == listen_sock){
            new_sock = accept_new_connection(listen_sock);
            if (new_sock > 0){
              FD_SET(new_sock, &master_fd_set);
              if (new_sock > max_fd)
                max_fd = new_sock;
            }else{
              error ("ERROR on accept");
            }
          }else{
            //TO-D0: handle client request here
                }
        }

      }
    }
}

int create_proxy_address(int portno){
  struct sockaddr_in serveraddr;
  int optval, listen_sock;
  listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock < 0) 
        error("ERROR opening socket");
    optval = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, 
                                     (const void *)&optval , sizeof(int));
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)portno);
    //TO-DO: Ask why the :: is needed
    if (::bind(listen_sock, (struct sockaddr *)&serveraddr, 
                              sizeof(serveraddr)) < 0)
        error("ERROR on binding");
    if (listen(listen_sock, 10) < 0)
        error("ERROR on listen");
    return listen_sock;
}

int accept_new_connection(int listen_sock){
  int clientlen;
  struct sockaddr_in clientaddr;
  clientlen = sizeof(clientaddr);
  return accept(listen_sock, (struct sockaddr *) &clientaddr, 
                            (socklen_t*)&clientlen);
}

