#include <iostream>
#include <string>
#include <fstream>
#include <sys/types.h>
#include <unistd.h>

#include "blue.h"

/*
* Creates a running instance of a blue node.
*/
int main(int argc, char* argv[]){
  char* serv_ip = argv[1];
  uint16_t serv_port = std::stoi(argv[2]);
  BlueNode b_node(serv_ip,serv_port);
  std::ofstream blue_ports("../data/blue_pids.txt", std::ios::app);
  blue_ports << getpid() << '\n';
  blue_ports.close();
  while(true){
    //do stuff
  }
  return 0;
}
