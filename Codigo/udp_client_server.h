#ifndef UDP_CLIENT_SERVER_H
#define UDP_CLIENT_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

class Client{
public:
  Client();
  ~Client();

private:

};


class Server{
public:
  Server(int port);
  ~Server();
  ssize_t send(char* msg);
  ssize_t receive(char* msg, size_t buff);

private:
  int sockfd;
  socklen_t len;
  struct sockaddr_in servaddr, cliaddr;
  void s_socket();
  void s_bind(int port);
};


#endif
