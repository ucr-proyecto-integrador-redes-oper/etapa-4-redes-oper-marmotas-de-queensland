#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <cstring>
#include <sstream>
#include <functional>
#include "sudp.h"
#include "frames.h"



SecureUDP sudp(8888,1000);

void writer(){
  std::stringstream msg;
  std::string tmp;
  std::string ip = " ";
  std::string port = " ";
  int msg_size;
  while(std::strcmp(msg.str().c_str(  ),"exit")){
    std::cout << "Digite el mensaje: ";
    std::getline(std::cin,tmp);
    msg.str(tmp);
    std::cout << "\nDigite la IP: ";
    std::getline(std::cin,ip);
    std::cout << "\nDigite el puerto: ";
    std::getline(std::cin,port);
    msg.seekg(0,std::ios::end);
    msg_size = msg.tellg();
    std::cout << "Sending size " << msg_size << std::endl;
    sudp.sendTo(&msg.str()[0],msg_size,&ip[0],std::stoi(port));
  }
}

void reader(){
  char buffer[1032];
  char* ip;
  uint16_t port;
  std::pair<char*,uint16_t> sender_data;
  while(true){
    sender_data = sudp.receive(buffer);
    std::cout << "\nEn Reader: Mensaje leido: ";
    std::cout << buffer << std::endl;
    std::cout << "Desde IP,Puerto: " << sender_data.first << "," << sender_data.second << std::endl;
  }
}

int main(int argc, char* argv[]){
  int port = std::stoi(argv[1]);
  int timeout = std::stoi(argv[2]);

  std::thread writer_thread(writer);
  std::thread reader_thread(reader);
  reader_thread.detach();
  writer_thread.join();
  //reader_thread.join();
  return 0;
}
