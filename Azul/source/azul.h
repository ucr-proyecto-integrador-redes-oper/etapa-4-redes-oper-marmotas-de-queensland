#ifndef AZUL_H
#define AZUL_H

#include <thread>
#include <chrono>
#include <utility>
#include <sstream>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "frames.h"
#include "sudp.h"
using namespace std;

#define TOTAL_CHUNK_CAP 40 //Max # of chunks that can be stored whithin a node
#define FILE_CHUNK_CAP 3 //Max # of chunks from the same file that can be stored within the same node

struct node_data{
  char* node_ip;
  uint16_t node_port;
  uint16_t node_id;
};

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

  //File handling routines
  void neighbour_raffle(f_chunk* current_msg);

  void PutChunkRequest(f_chunk*);

  void ExistsRequest(f_exists* current_msg, char* solicitingIP, uint16_t solicitingPort,uint16_t solicitingID);
  void ExistsAnswer(f_exists_ans* answerFromChild);

  void CompleteRequest(f_complete* completeReq);
  void CompleteAnswer();

  void GetRequest();
  void GetAnswer();

  void LocateRequest();
  void LocateAnswer();

  void DeleteRequest(f_delete* toDelete);

  //map with the ids of all 'owned' files and the # of chunks for each.
  //uint24_t& operator< (uint24_t x);
  //uint24_t& operator== (uint24_t y);
  

  map<const uint24_t,uint8_t,uint24_compare> files; //only working with one kind of overloading...
  string files_path;
  int num_of_chunks; //Number of chunks that have been stored on node
  map<uint16_t,node_data>::iterator it_vecinos;

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
