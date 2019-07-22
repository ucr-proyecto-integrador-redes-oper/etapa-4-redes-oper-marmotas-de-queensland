#include <thread>
#include <chrono>
#include <utility>
#include <sstream>
#include <cstring>
#include <string>
#include <map>
#include <queue>
#include <iostream>
#include <unistd.h>
#include <mutex>
#include <condition_variable>

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
  // unsigned int menorVecino;
  bool aceptado;
  map<uint16_t,node_data> vecinos;

  //Arbol:
  map<uint16_t,node_data> hijosArbol;

  //para el timer:
  std::mutex m;
  std::condition_variable cv;

public:
  Azul(char*, int);
  ~Azul();
  void start();
  //grafo:
  void joinTree();
  void recibirVecinos();
  void enviarHello(node_data);
  //Arbol:
  void startArbolGenerador();
  void enviarSolicitudesTree();
  void recibirSolicitudesTree();
  void agregarHijoArbol(node_data);
  void enviarIDo(pair<char*,uint16_t>);
  void enviarIDoNot(pair<char*,uint16_t>);
  void enviarJoinTree(node_data);
  void enviarDaddy(pair<char*,uint16_t>);
  void timeout();
  void imprimirHijos();
};

#endif
