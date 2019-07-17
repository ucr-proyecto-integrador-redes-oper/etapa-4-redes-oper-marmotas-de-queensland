#include "orange_udp.h"

/*
  Constructor for the UDP class
*/
UDP::UDP(int port){
  this->port = port;
  setSocket();
}

/*
  Destructor for the UDP class
*/
UDP::~UDP(){
   close(sock_fd);
}

/*
  Returns last client's port
*/
uint16_t UDP::getPortClient(){
  return this->clientData.port;
}

/*
  Returns last client's IP address
*/
char* UDP::getIpClient(){
  return inet_ntoa(this->clientData.addr);
}

/*
  Send msg buffer of size to ip and port
*/
void UDP::sendTo(char* data,int size, char* ip, uint16_t port){
  sockaddr_in dest_addr;
  struct in_addr addr;
  memset(&dest_addr, 0, sizeof(dest_addr));

  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(port);
  //dest_addr.sin_addr = ip;
  inet_aton(ip,&addr);
  dest_addr.sin_addr = addr;
  // cout << "ip: " << ip << endl;
  //sendto
  sendto(sock_fd,data, size,0,
  (sockaddr*) &dest_addr, sizeof(sockaddr_in));
}

/*
  Receive Msg and save it in buffer
*/
void UDP::receive(char* buffer,int size){
   sockaddr_in src_addr;
   socklen_t sz = sizeof(src_addr);
   //recvfrom syscall
   recvfrom(sock_fd,buffer, size,0,(sockaddr*)&src_addr,&sz);
   //sets de addr and port of the receiver in network order.
   //inet_aton(ip,&clientData->addr);
   clientData.addr = src_addr.sin_addr;
   clientData.port = ntohs(src_addr.sin_port);
}

/////////////////////////////////Private functions//////////////////////////////////////////

void UDP::setSocket(){
  // Creating socket file descriptor , UDP
  if ( (sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
    perror("socket creation failed");
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET; // IPv4
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(port);
  bind(sock_fd,(struct sockaddr*) &servaddr,sizeof(sockaddr_in));
}
