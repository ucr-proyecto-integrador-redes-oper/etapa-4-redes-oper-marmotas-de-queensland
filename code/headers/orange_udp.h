#ifndef ORANGE_UDP_H_
#define ORANGE_UDP_H_

#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>
#include <string.h>

using namespace std;

struct udp_ClieData{
  in_addr addr;
  uint16_t port;
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
    uint16_t getPortClient();
    char* getIpClient();


  private:
    struct udp_ClieData clientData;
    sockaddr_in servaddr;
    int sock_fd;
    uint16_t port;

    void setSocket();
};

#endif
