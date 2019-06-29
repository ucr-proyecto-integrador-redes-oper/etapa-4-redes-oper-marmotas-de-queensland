#ifndef SECURE_UDP_H_
#define SECURE_UDP_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>
#include <stdio.h>

#include <mutex>
#include <condition_variable>
#include <map>
#include <queue>

#define PAYLOAD_CAP 1032
#define SN_CAP 0xFFFF

/*
  Struct for the Secure Udp frame encapsulation.
  Types: 0 ---> sender/receiver
         1 ---> ack
*/
//#pragma pack(push,1)
struct sudp_frame{
  uint8_t type; //1 byte type
  uint16_t sn; // 2 bytes sn
  char payload[PAYLOAD_CAP]; //1kb payload + blue header
}__attribute__((packed));
//#pragma pack(pop)

/*
  Struct for the receiver data.
  Saves the receiver IP and port.
*/
//#pragma pack(push,1)
struct sudp_rdata{
  in_addr addr;
  uint16_t port;
}__attribute__((packed));
//#pragma pack(pop)

/*
  Struct for the map value associated with a sn key.
*/
//#pragma pack(push,1)
struct smap_value{
  sudp_frame *frame;
  sudp_rdata *rdata;
  size_t msg_size;
}__attribute__((packed));
//#pragma pack(pop)


/*
  Class for a semi-reliable UDP communication.
  args: int port, int wait_time(ms)
*/
class SecureUDP{
  public:
    SecureUDP(int,int);
    ~SecureUDP();
    void sendTo(char*,size_t,char*,uint16_t);
    void receive(char*);


  private:
    sockaddr_in servaddr;
    int sock_fd;
    int wait_time;
    uint16_t port;
    uint16_t sn;

    std::mutex rq_m; //receive queue mutex
    std::mutex m_lock; //map mutex
    //std::unique_lock<std::mutex> rq_lock(rq_m, std::defer_lock);
    std::condition_variable rq_cv; //receive queue cv
    std::queue<sudp_frame*> r_queue; //received queue
    std::queue<sudp_frame*> del_queue;
    std::map<uint16_t,smap_value*> s_map; //send map

    void setSocket();

    //Thread routines.
    void sender();
    void receiver();
    void start();
};

#endif
