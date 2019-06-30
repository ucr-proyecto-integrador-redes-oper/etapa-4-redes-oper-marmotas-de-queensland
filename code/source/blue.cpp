#include "blue.h"

BlueNode::BlueNode(char* server_ip,uint16_t server_port){
  //sets server info.
  server_data.n_ip = server_ip;
  server_data.n_port = server_port;
}


BlueNode::~BlueNode(){

}

char* BlueNode::getIP(){
  return nullptr;
}

uint16_t BlueNode::getPort(){
  return sudp.getPort();
}
