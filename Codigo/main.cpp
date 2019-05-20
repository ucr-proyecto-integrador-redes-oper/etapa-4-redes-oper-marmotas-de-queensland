#include <iostream>
#include "udp_client_server.h"

int main(int argc, char* argv[]){
  Server serv(9090);
  size_t buff_size = 100;
  char msg_received[buff_size];
  std::string ans = "Message received.";
  while(true){
    serv.receive(msg_received,buff_size);
    std::cout << msg_received << std::endl;
    serv.send(&ans[0]);
  }
  return 0;
}
