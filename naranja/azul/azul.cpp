#include "azul.h"

Azul::Azul(char* ipNaranja, int portNaranja){
  this->sudp = new SecureUDP(2000);
  this->ipNaranja = ipNaranja;
  this->portNaranja = portNaranja;
}

Azul::~Azul(){
  delete sudp;
}

void Azul::start(){
  this->joinTree();
  this->recibirVecinos();

}

void Azul::joinTree(){
  f_join_graph join;
  join.type = 14;
  sudp->sendTo((char*)&join,sizeof(join),ipNaranja,portNaranja);
}

void Azul::recibirVecinos(){
  f_graph_pos_i vecino_info;
  f_hello hello;

  node_data vecino_data;
  std::pair<char*,uint16_t> sender_data;
  sockaddr_in info_nodo;

  char* buffer = new char[F_PAYLOAD_CAP];
  sender_data = sudp->receive(buffer);
  memcpy((char*)&vecino_info,buffer,sizeof(vecino_info));

  miId = vecino_info.node_id;
  cout << "Soy el nodo: " << miId << " con puerto: " << sudp->getPort() <<endl<< endl;
  if(vecino_info.type == 15 && (int)vecino_info.node_id == -1){
    cout << "No fui aceptado en el grafo." << endl<< endl;
  }else{
    while(vecino_info.type < 17 ){
      cout << "Llego tipo: " << (int)vecino_info.type << endl;
      if(vecino_info.type == 15){ //el vecino no tiene info

        vecino_data.node_ip = 0;
        vecino_data.node_port = 0;
        //el id esta mal
        vecino_data.node_id = vecino_info.neighbor_id;
        vecinos[vecino_data.node_id] = vecino_data;
        cout << "Recibiendo vecino no instanciado" << endl;
        cout << "Nodo vecino id: "<< vecino_data.node_id << endl<< endl;

      }else if(vecino_info.type == 16){

        info_nodo.sin_addr.s_addr = (unsigned int)vecino_info.neighbor_ip;
        vecino_data.node_ip = inet_ntoa(info_nodo.sin_addr);

        vecino_data.node_port = vecino_info.neighbor_port;
        vecino_data.node_id = vecino_info.neighbor_id;
        vecinos[vecino_data.node_id] = vecino_data;

        cout << "Recibiendo vecino instanciado" << endl;
        cout << "Nodo vecino id: "<< vecino_data.node_id << endl;
        cout << "Nodo vecino ip: " << vecino_data.node_ip << endl;
        cout << "Nodo vecino port: "<< vecino_data.node_port << endl;
        enviarHello(vecino_data);

      }else if(vecino_info.type == 1){
        memcpy((char*)&hello,buffer,sizeof(hello));

        cout << "Recibiendo Hello" << endl;

        vecino_data.node_ip = sender_data.first;
        vecino_data.node_port = sender_data.second;
        vecino_data.node_id = hello.node_id;
        vecinos[vecino_data.node_id] = vecino_data;

        cout << "Nodo vecino id: "<< vecino_data.node_id << endl;
        cout << "Nodo vecino ip: " << vecino_data.node_ip << endl;
        cout << "Nodo vecino port: "<< vecino_data.node_port << endl<< endl;
      }
      sender_data = sudp->receive(buffer);
      memcpy((char*)&vecino_info,buffer,sizeof(vecino_info));
    }
    cout << "Llego complete" << endl<< endl;
  }
}

void Azul::enviarHello(node_data destino){
  f_hello hello;
  hello.type = 1;
  hello.node_id = miId;
  cout << "Enviando Hello a IP: "<< destino.node_ip << " port: "<< destino.node_port << endl;
  sudp->sendTo((char*)&hello,sizeof(hello),destino.node_ip,destino.node_port);
}
