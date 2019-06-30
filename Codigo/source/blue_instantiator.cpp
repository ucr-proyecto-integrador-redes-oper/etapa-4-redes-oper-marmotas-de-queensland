#include <iostream>
#include <string>
#include "blue.h"

int main(int argc, char* argv[]){
  char* serv_ip = argv[1];
  uint16_t serv_port = std::stoi(argv[2]);
  BlueNode b_node(serv_ip,serv_port);
  std::cout << "Server: " << serv_ip << " port " << serv_port << std::endl;
  std::cout << "Port for new blue node: " << b_node.getPort() << std::endl;
  while(true){
    //do stuff
  }
  return 0;
}
