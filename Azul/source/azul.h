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
  bool tree_member;
  std::map<uint16_t,node_data> vecinos;
  node_data parent_data;
  std::map<uint16_t,node_data> hijosArbol;
public:
  Azul(char*, int);
  ~Azul();
  void start();
  void joinGraph();
  void recibirVecinos();
  void enviarHello(node_data);

  //Spanning Tree routines
  void joinTree();
  void agregarHijo(char*,uint16_t,uint16_t);
  void handleTreeRequest(char*,uint16_t);
  bool handleTreeRequestAnswer(std::map<uint16_t,f_join_tree>*);

};

#endif
