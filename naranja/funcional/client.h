#ifndef CLIENT_H_
#define CLIENT_H_
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <strings.h>
#include <cstring>
#include <iostream>

class Client{
private:
  int sockfd;
  int bSize;
  struct sockaddr_in servaddr;
public:
  Client();
  ~Client();
  int connectClient(char*,int);
  int readFromServer(char*,int);
  int writeToServer(char*,int);
  int shutDownClient(int);
  int closeClient();
};
#endif
