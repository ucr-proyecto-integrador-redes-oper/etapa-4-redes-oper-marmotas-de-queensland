#include "sudp.h"
#include <cstdint>
#include <cstring>
#include <thread>
#include <chrono>
#include <iostream>



/*
  Constructor for the SecureUDP class
*/
SecureUDP::SecureUDP(int port,int wait_time){
  this->port = port;
  setSocket();
  start();
}

/*
  Destructor for the SecureUDP class
*/
SecureUDP::~SecureUDP(){

}


/////////////////////////////////Private functions//////////////////////////////////////////

void SecureUDP::setSocket(){
  // Creating socket file descriptor , UDP
  if ( (sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
    perror("socket creation failed");
  }

  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));

  servaddr.sin_family    = AF_INET; // IPv4
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(port);
  bind(sock_fd,(struct sockaddr*) &servaddr,sizeof(sockaddr_in));

}

void SecureUDP::send(){

}

void SecureUDP::receive(){
}

void SecureUDP::start(){
  std::thread s(&SecureUDP::sender,this);
  std::thread r(&SecureUDP::receiver,this);
  s.detach();
  r.detach();
}
