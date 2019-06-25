#ifndef SERVER_H_
#define SERVER_H_
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <iostream>
#include <string.h>

using namespace std;

class Server{
private:
    int connfd;
    int sockfd;
    struct sockaddr_in servaddr;
    struct sockaddr_in client;

public:
    Server();
    ~Server();
    int server_bind(int);
    int server_listen(int);
    int server_read(char*,int);
    int server_write(char*,int);
    int server_accept();
    int close_server();
    void receivePackage(char*,int);
    int writeToServer(char*, int );
};

#endif
