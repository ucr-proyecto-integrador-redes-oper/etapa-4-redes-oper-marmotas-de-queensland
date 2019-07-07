#include <iostream>
#include <string>
#include <fstream>
#include <sys/types.h>
#include <unistd.h>

#include "blue.h"

/*
* Creates a running instance of a blue node.
* Writes the node pid,port to the blude_data.txt file.
*/
int main(int argc, char* argv[]){
  char* serv_ip = argv[1];
  char* node_ip = argv[3];
  uint16_t serv_port = std::stoi(argv[2]);
  BlueNode b_node(serv_ip,serv_port,node_ip);
  std::ofstream blue_data("../data/blue_data.txt", std::ios::app);
  blue_data << getpid() << ',';
  blue_data << b_node.getPort() << '\n';
  blue_data.close();
  b_node.start();
  return 0;
}
