#include "udp.h"

/*
  Constructor for the SecureUDP class
*/
UDP::UDP(int port){
  this->port = port;
  setSocket();
  //start();
}

/*
  Destructor for the SecureUDP class
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
  Encapsultates the data in a SecureUPD header and adds it to the send map.
*/
void UDP::sendTo(char* data,int size, char* ip, uint16_t port){
  udp_frame* curr_frame = new udp_frame();

  sockaddr_in dest_addr;
  struct in_addr addr;
  memset(&dest_addr, 0, sizeof(dest_addr));

  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(port);
  //dest_addr.sin_addr = ip;
  inet_aton(ip,&addr);
  dest_addr.sin_addr = addr;

  cout << "ip: " << ip << endl;

  //copies the data.
  memcpy((char*) curr_frame->payload, data , PAYLOADUDP_CAP);

  //sendto
  sendto(sock_fd, (char*) curr_frame, sizeof(udp_frame),0,
  (sockaddr*) &dest_addr, sizeof(sockaddr_in));

  delete curr_frame;
}

/*


*/
void UDP::receive(char* buffer,int size){

   sockaddr_in src_addr;
   socklen_t sz = sizeof(src_addr);

   udp_frame *curr_frame = new udp_frame();

   //recvfrom syscall
   recvfrom(sock_fd,(char*) curr_frame, sizeof(curr_frame),0,(sockaddr*)&src_addr,&sz);

   //sets de addr and port of the receiver in network order.
   //inet_aton(ip,&clientData->addr);
   clientData.addr = src_addr.sin_addr;
   clientData.port = ntohs(src_addr.sin_port);

   //copies the payload.
   memcpy(buffer, (char*)curr_frame->payload,size);
   delete curr_frame;
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
