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
#include <utility>
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
*  Class for a semi-reliable UDP communication. It guarantees message arrival but not
*  the order in which the message arrives.
*/
class SecureUDP{
  public:
    SecureUDP(uint16_t,uint32_t);
    SecureUDP(uint32_t);
    ~SecureUDP();
    void sendTo(char*,size_t,char*,uint16_t);
    std::pair<char*,uint16_t> receive(char*);
    uint16_t getPort();


  private:
    sockaddr_in servaddr;
    int sock_fd;
    int wait_time;
    uint16_t port;
    uint16_t sn;

    std::queue<std::pair<sudp_frame*,sudp_rdata*>> ready_queue; //ready queue
    std::queue<std::pair<sudp_frame*,sudp_rdata*>> received_queue; //received queue
    std::map<uint16_t,smap_value*> s_map; //send map

    std::mutex rq_m; //received queue mutex
    std::mutex received_mux;
    std::mutex ready_mux;


    std::condition_variable rq_cv; //received queue cv
    std::condition_variable rd_cv; //ready queue cv
    std::mutex rd_m; //ready queue mutex, for the cv
    std::mutex sm_m; //send map mutex, for the cv

    void setSocket(uint16_t);
    void setSocket();

    //Thread routines.
    void sender();
    void processor();
    void receiver();
    void start();
};

#endif
