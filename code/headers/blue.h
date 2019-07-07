#ifndef BLUE_H_
#define BLUE_H_

#include <map>
#include <string.h>
#include <queue>
#include "sudp.h"


/*
* Keeps track of the IP and port of a node.
*/
struct n_data{
  char* n_ip;
  uint16_t n_port;
};


class BlueNode{
public:
  BlueNode(char *, uint16_t , char*);
  ~BlueNode();

  char* getIP();
  uint16_t getPort();
  void start();

private:
    n_data my_data;
    n_data server_data;
    uint16_t n_id; // this node's id.

    SecureUDP sudp = decltype(sudp)(1000);
    std::map<uint16_t,n_data> neighbours; //graph neighbours.
    std::map<uint16_t,n_data> t_neighbours; // spanning tree neighbours.
    void joinGraph();
    uint8_t getType(char*);

    void sendHello(uint16_t myID);
    void waitForComplete();

    //Thread routines.
    void receiver();
    void orangeRequests();
    void bgRequests(); //blue-green requests.


};

#endif
