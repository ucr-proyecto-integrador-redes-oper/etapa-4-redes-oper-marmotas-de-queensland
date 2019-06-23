#ifndef SECURE_UDP_H_
#define SECURE_UDP_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>

//#include <thread>
#include <mutex>
#include <condition_variable>
#include <map>
#include <queue>

#define PAYLOAD_CAP 1024
#define SN_CAP 0xFFFF

/*
  Struct for the Secure Udp frame encapsulation.
  Types: 0 ---> sender/receiver
         1 ---> ack
*/
struct sudp_frame{
  uint8_t type; //1 byte type
  uint16_t sn; // 2 bytes sn
  char payload[PAYLOAD_CAP]; //1kb payload
};

/*
  Struct for the receiver data.
  Saves the receiver IP and port.
*/
struct sudp_rdata{
  in_addr addr;
  uint16_t port;
};

/*
  Struct for the map value associated with a sn key.
*/
struct smap_value{
  sudp_frame *frame;
  sudp_rdata *rdata;
  bool ack_received;
};


/*
  Class for a semi-reliable UDP communication.
  args: int port, int wait_time(ms)
*/
class SecureUDP{
  public:
  SecureUDP(int,int);
  ~SecureUDP();
  void sendTo(char*,char*,uint16_t);
  void receive(char*);


  private:
  struct sockaddr_in servaddr, cliaddr;
  int sock_fd;
  int wait_time;
  uint16_t port;
  uint16_t sn;

  std::mutex rq_m; //receive queue mutex
  //std::unique_lock<std::mutex> rq_lock(rq_m, std::defer_lock);
  std::condition_variable rq_cv; //receive queue cv
  std::queue<sudp_frame*> r_queue; //received queue
  std::map<uint16_t,smap_value*> s_map; //send map

  void setSocket();

  //Thread routines.
  void sender();
  void receiver();
  void start();
};

#endif
