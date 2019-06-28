#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <cstring>
#include <functional>

#include "sudp.h"
#include "frames.h"




std::mutex c_lock;
SecureUDP sudp(9999,1000);

void writer(){
  std::string msg = " ";
  std::string ip = " ";
  std::string port = " ";
  while(std::strcmp(msg.c_str(),"exit")){
    std::cout << "Digite el mensaje: ";
    std::getline(std::cin,msg);
    std::cout << "\nDigite la IP: ";
    std::getline(std::cin,ip);
    std::cout << "\nDigite el puerto: ";
    std::getline(std::cin,port);
    sudp.sendTo(&msg[0],&ip[0],std::stoi(port));
  }
}

void reader(){
  char buffer[0xFFFF];
  while(true){
    sudp.receive(buffer);
    std::cout << "\nEn Reader: Mensaje leido: ";
    std::cout << buffer << std::endl;
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
