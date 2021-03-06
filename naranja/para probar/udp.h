#ifndef UDP_H
#define UDP_H

#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>
#include <string.h>

#define PAYLOAD_CAP 1024

struct sudp_frame{
  char payload[PAYLOAD_CAP]; //1kb payload
};

/*
  Class for a semi-reliable UDP communication.
  args: int port, int wait_time(ms)
*/
class UDP{
  public:
    UDP(int);
    ~UDP();
    void sendTo(char*,int, char*, uint16_t);
    void receive(char*,int);
    

  private:
    sockaddr_in servaddr;
    int sock_fd;
    uint16_t port;

    void setSocket();
};

#endif
