#ifndef BLUE_H_
#define BLUE_H_

#include <map>
#include "sudp.h"
#include "frames.h"


/*
* Keeps track of the IP and port of another node.
*/
struct n_data{
  char* n_ip;
  uint16_t n_port;
};


class BlueNode{
public:
  BlueNode(char *, uint16_t );
  ~BlueNode();
  char* getIP();
  uint16_t getPort();


private:
    n_data server_data;
    SecureUDP sudp = decltype(sudp)(1000);
    char* my_ip;
    uint16_t my_port;
    std::map<uint16_t,n_data> neighbours; //graph neighbours.
    std::map<uint16_t,n_data> t_neighbours; // spanning tree neighbours.


};

#endif
