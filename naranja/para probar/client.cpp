#include "client.h"
/*
*  Constructor for class client
*/
Client::Client(){
  sockfd = socket(AF_INET,SOCK_STREAM,0);
  if(sockfd < 0){
    std::cout << "Socket creation failed." << std::endl;
  }
  bzero(&servaddr, sizeof(servaddr));
}

/*
* Destructor for class client
*/
Client::~Client(){

}


int Client::writeToServer(char *data, int dataSize){
  char buf[dataSize];
  memcpy(buf,data,dataSize);
  return write(sockfd,buf,dataSize);
}

int Client::readFromServer(char* data, int dataSize){
  return read(sockfd,data,dataSize);
}

int Client::connectClient(char* hostIp, int port){
  int status;
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(hostIp);
  servaddr.sin_port = htons(port);
  status = connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
  if(status < 0){
    std::cout << "Connection failed." << std::endl;
  }
  return status;
}

int Client::closeClient(){
  return close(sockfd);
}

int Client::shutDownClient(int mode){
  return shutdown(sockfd,mode);
}
