#include <thread>
#include <chrono>
#include <utility>
#include <sstream>
#include <cstring>
#include <map>
#include <queue>
#include <iostream>

#include "frames.h"
#include "sudp.h"
using namespace std;

struct node_data{
  char* node_ip;
  uint16_t node_port;
  uint16_t node_id;
};

#ifndef AZUL_H
#define AZUL_H

class Azul{
private:
  SecureUDP* sudp;
  char* ipNaranja;
  int portNaranja;
  int miId;
  std::map<uint16_t,node_data> vecinos;
public:
  Azul(char*, int);
  ~Azul();
  void start();
  void joinTree();
  void recibirVecinos();
  void enviarHello(node_data);

};

#endif
