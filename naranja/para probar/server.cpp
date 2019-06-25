#include "server.h"

/*
*  Constructor for class Server
*/
Server::Server(){
  //crea socket, parametros: dominio, tipo, protocolo
  this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0){
    cout << "Socket creation failed...\n" << endl;
    exit(0);
  }
  else{
    cout << "Socket successfully created...\n" << endl;
  }
  bzero(&servaddr, sizeof(servaddr)); //limpia la direccion
}

/*
*  Destructor for class Server
*/
Server::~Server(){

}

/*
*   When a socket is created with, it exists in a name space
*   but has no address assigned to it.  bind() assigns
*   the address specified by addr to the socket referred to by sockfd.
*/
int Server::server_bind(int port){
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);
  return bind(sockfd,(sockaddr*)&servaddr,sizeof(servaddr));// -1 is returned on error
}

/*
*  marks the socket referred to by sockfd as a passive socket, that is,
*  as a socket that will be used to accept incoming connection requests.
*/
int Server::server_listen(int backlog){
  return listen(sockfd,backlog); // -1 is returned on error
}


/*
*   extracts the first connection request on the queue of pending connections
*   for the listening socket, sockfd, creates a new connected socket, and
*   returns a new file descriptor referring to that socket.  The newly
*   created socket is not in the listening state.
*/
int Server::server_accept(){
  socklen_t len = sizeof(client);
  this->connfd = accept(sockfd,(struct sockaddr *)&client,&len); // -1 is returned on error
  return this->connfd;
}

int Server::server_read(char* data, int dataSize){
  return read(connfd,data,dataSize);
}

int Server::server_write(char* data, int dataSize){
  char buf[dataSize];
  memcpy(buf,data,dataSize);
  return write(connfd,buf,dataSize);
}

/*
*   closes server, can be called after client shutdown
*/
int Server::close_server(){
  return close(sockfd);
}

/*
* Receive a message from client
*/
void Server::receivePackage(char* buff,int size){
  read(connfd, buff, size);
}

/*
* Write a message to client
*/
int Server::writeToServer(char *data, int dataSize){
  char buf[dataSize];
  memcpy(buf,data,dataSize);
  return write(connfd,buf,dataSize);
}
