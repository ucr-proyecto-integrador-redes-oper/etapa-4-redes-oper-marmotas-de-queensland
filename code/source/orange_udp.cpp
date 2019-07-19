#include "orange_udp.h"

/**
* @brief Constructor para la clase UDP, inicializa un socket llamando al metodo setSocket.
* @param int port: puerto a ser inicialiado.
* @return None.
*/
UDP::UDP(int port){
  this->port = port;
  setSocket();
}

/**
* @brief Destructor de la clase UDP. Realiza close al socket creado.
* @param None.
* @return None.
*/
UDP::~UDP(){
   close(sock_fd);
}

/**
* @brief Metodo que retorna el puerto del ultimo cliente del cual recibio un paquete.
* @param None.
* @return uint16_t: Puerto, de 16bits de largo, del ultimo cliente.
*/
uint16_t UDP::getPortClient(){
  return this->clientData.port;
}

/**
* @brief Metodo que retorna la del ultimo cliente del cual recibio un paquete.
* @param None.
* @return Char*: IP del cliente en formato char.
*/
char* UDP::getIpClient(){
  return inet_ntoa(this->clientData.addr);
}

/*
  Send msg buffer of size to ip and port
*/
/**
* @brief Metodo que envia un mensaje a otro socket.
* @param data: un puntero que indica el inicio del data a ser enviado.
* @param size: la cantidad de bytes del data.
* @param ip: la IP destino del paquete.
* @param port: el puerto destino del paquete.
* @return None.
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
/**
* @brief Metodo que recibe un paquete, es almacenado en buffer.
* @param buffer: puntero que indica el inicio de memoria donde se comienzan a escribir los datos.
* @param size: cantidad de bytes a ser escritos en memoria.
* @return None.
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
/**
* @brief Metodo que crea y hace bind a un socket con el puerto definido como atributo.
* @param None.
* @return None.
*/
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
