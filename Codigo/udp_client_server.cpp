#include "udp_client_server.h"

//CLIENT ROUTINES



//SERVER ROUTINES
Server::Server(int port){
  s_socket();
  s_bind(port);
}

Server::~Server(){
  close(sockfd);
}

ssize_t Server::send(char* msg){
  return sendto(sockfd, msg, sizeof(*msg),MSG_CONFIRM,(const struct sockaddr *) &cliaddr,len);
}

ssize_t Server::receive(char* msg, size_t buff_size){
  return recvfrom(sockfd, msg, buff_size,MSG_WAITALL,(struct sockaddr *) &cliaddr, &len);
}

void Server::s_socket(){
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
      perror("Server socket creation failed.");
      exit(EXIT_FAILURE);
    }
}

void Server::s_bind(int port) {
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);

    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) {
      perror("Server bind failed.");
      exit(EXIT_FAILURE);
    }
}
