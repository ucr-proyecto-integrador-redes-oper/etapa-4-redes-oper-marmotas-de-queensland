#include "azul.h"

bool continuar =  true;
bool soyParteArbol = false;

/**
* @brief Constructor de la clase azul, estos son los nodos azules.
* @param ipNaranja: IP del servidor Naranja al que realizara la solicitud.
* @param portNaranja: Puerto del servidor Naranja al que realizara la solicitud.
* @return None.
*/
Azul::Azul(char* ipNaranja, int portNaranja){
  this->sudp = new SecureUDP(2000);
  this->ipNaranja = ipNaranja;
  this->portNaranja = portNaranja;
  continuar = true;
  soyParteArbol = false;
  this->aceptado = false;
}

/**
* @brief Destructor de la clase azul.
* @param None.
* @return None.
*/
Azul::~Azul(){
  delete sudp;
}

/**
* @brief Metodo que comienza la ejecucion (funcionalidad) del nodo azul.
* @param None.
* @return None.
*/
void Azul::start(){
  this->joinTree();
  this->recibirVecinos();
  if(aceptado)
    this->startArbolGenerador();
}

/**
* @brief Metodo que envia solicitud al Naranja para formar parte del grafo.
* @param None.
* @return None.
*/
void Azul::joinTree(){
  f_join_graph join;
  join.type = 14;
  sudp->sendTo((char*)&join,sizeof(join),ipNaranja,portNaranja);
}

/**
* @brief Metodo que recibe mi Id en el grafo, los vecinos y envia Hello.
* Tambien verifica si fue aceptado en el grafo.
* @param None.
* @return None.
*/
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
  // cout << "Soy el nodo: " << miId << " con puerto: " << sudp->getPort() <<endl<< endl;

  //por si se rechazo la solicitud
  int idAceptado;
  memcpy((char*)&idAceptado,(char*)&vecino_info.node_id,4);
  if(vecino_info.type == 15 && idAceptado == -1){
    // cout << "No fui aceptado en el grafo." << endl<< endl;
    aceptado = false;
  }else{
    aceptado = true;
    while(vecino_info.type < 17 ){
      // cout << "Llego tipo: " << (int)vecino_info.type << endl;
      if(vecino_info.type == 15){ //el vecino no tiene info

        vecino_data.node_ip = 0;
        vecino_data.node_port = 0;
        //el id esta mal
        vecino_data.node_id = vecino_info.neighbor_id;
        vecinos[vecino_data.node_id] = vecino_data;

        // cout << "Recibiendo vecino no instanciado" << endl;
        // cout << "Nodo vecino id: "<< vecino_data.node_id << endl<< endl;

      }else if(vecino_info.type == 16){

        info_nodo.sin_addr.s_addr = (unsigned int)vecino_info.neighbor_ip;
        vecino_data.node_ip = inet_ntoa(info_nodo.sin_addr);

        vecino_data.node_port = vecino_info.neighbor_port;
        vecino_data.node_id = vecino_info.neighbor_id;
        vecinos[vecino_data.node_id] = vecino_data;

        // cout << "Recibiendo vecino instanciado" << endl;
        // cout << "Nodo vecino id: "<< vecino_data.node_id << endl;
        // cout << "Nodo vecino ip: " << vecino_data.node_ip << endl;
        // cout << "Nodo vecino port: "<< vecino_data.node_port << endl;
        enviarHello(vecino_data);

      }else if(vecino_info.type == 1){
        memcpy((char*)&hello,buffer,sizeof(hello));

        // cout << "Recibiendo Hello" << endl;

        vecino_data.node_ip = sender_data.first;
        vecino_data.node_port = sender_data.second;
        vecino_data.node_id = hello.node_id;
        vecinos[vecino_data.node_id] = vecino_data;

        // cout << "Nodo vecino id: "<< vecino_data.node_id << endl;
        // cout << "Nodo vecino ip: " << vecino_data.node_ip << endl;
        // cout << "Nodo vecino port: "<< vecino_data.node_port << endl<< endl;
      }
      sender_data = sudp->receive(buffer);
      memcpy((char*)&vecino_info,buffer,sizeof(vecino_info));
    }
    // cout << "Llego complete" << endl<< endl;
  }
}

/**
* @brief Metodo que envia un pack Hello.
* @param destino: struct que contiene la ip y puerto a donde se envia el mensaje.
* @return None.
*/
void Azul::enviarHello(node_data destino){
  f_hello hello;
  hello.type = 1;
  hello.node_id = miId;
  // cout << "Enviando Hello a IP: "<< destino.node_ip << " port: "<< destino.node_port << endl;
  sudp->sendTo((char*)&hello,sizeof(hello),destino.node_ip,destino.node_port);
}

/**
* @brief Metodo que inicia la creacion del arbol generador.
* Crea dos threads, uno para enviar solicitudes al arbol y otra que recibe.
* @param None.
* @return None.
*/
void Azul::startArbolGenerador(){
  cout << "Iniciando arbol generador." << endl;
  thread receiver(&Azul::recibirSolicitudesTree,this);
  sleep(1);
  thread sender(&Azul::enviarSolicitudesTree,this);
  sender.join();
  receiver.join();
}

/**
* @brief Metodo que recibe solicitudes de otros azules.
* Y las responde con los paquetes establecidos.
* @param None.
* @return None.
*/
void Azul::recibirSolicitudesTree(){
  node_data hijo_data;
  f_hello hijo_info;

  pair<char*,uint16_t> sender_data;

  char* buffer = new char[F_PAYLOAD_CAP];
  sender_data = sudp->receive(buffer);
  memcpy((char*)&hijo_info,buffer,sizeof(hijo_info));

  //Se detiene cuando no recibe un pack dentro de un tiempo.
  thread (&Azul::timeout,this).detach();

  while( hijo_info.type && continuar){
    if(hijo_info.type == 11){ //Solicitud de JoinTree
      // cout << "Se recibio solicitud de JoinTree" << endl;
      if(soyParteArbol)
        enviarIDo(sender_data);
      else
        enviarIDoNot(sender_data);

    }else if(hijo_info.type == 13){//Me escogieron como padre
      hijo_data.node_ip = sender_data.first;
      hijo_data.node_port = sender_data.second;
      hijo_data.node_id = hijo_info.node_id;
      // cout << "El nodo: " << hijo_data.node_id << " escogio como padre a : " << miId << endl;
      agregarHijoArbol(hijo_data);

    }else if(!soyParteArbol && hijo_info.type == 12){
      //mi menor vecino pertenece al arbol.
      enviarDaddy(sender_data);

    }else if(hijo_info.type == 1){
      // cout << "Recibiendo Hello" << endl;

      hijo_data.node_ip = sender_data.first;
      hijo_data.node_port = sender_data.second;
      hijo_data.node_id = hijo_info.node_id;
      vecinos[hijo_data.node_id] = hijo_data;

      // cout << "Nodo vecino id: "<< hijo_data.node_id << endl;
      // cout << "Nodo vecino ip: " << hijo_data.node_ip << endl;
      // cout << "Nodo vecino port: "<< hijo_data.node_port << endl<< endl;
    }

    sender_data = sudp->receive(buffer);
    cv.notify_one(); // se notifica al timer.
    memcpy((char*)&hijo_info,buffer,sizeof(hijo_info));
    thread (&Azul::timeout,this).detach(); // se crea otro timer

  }
  continuar = false;
}

/**
* @brief Metodo que funciona como timer.
* @param None.
* @return None.
*/
void Azul::timeout(){
  std::unique_lock<std::mutex> mutex(m);
  if(cv.wait_for(mutex, 30s) == std::cv_status::timeout){
    // cout << "Timeout: Terminando el nodo: "<< this->miId << endl;
    this->imprimirVecinos();
    this->imprimirHijos();
    continuar = false;
    exit(0);
  }
}

/**
* @brief Metodo que imprime la lista de hijos, que conforman el arbol.
* @param None.
* @return None.
*/
void Azul::imprimirHijos(){
  stringstream hijos;
  hijos << "Hijos de nodo " << this->miId << " son: ";
  // cout << "Hijos de nodo " << this->miId << " son: " << endl;
  for (std::map<uint16_t,node_data>::iterator it=hijosArbol.begin(); it!=hijosArbol.end(); ++it)
    hijos << it->second.node_id << " , ";

  cout<< hijos.str() << endl;
}

void Azul::imprimirVecinos(){
  stringstream hijos;
  hijos << "Vecinos del nodo " << this->miId << " son: ";
  // cout << "Hijos de nodo " << this->miId << " son: " << endl;
  for (std::map<uint16_t,node_data>::iterator it=vecinos.begin(); it!=vecinos.end(); ++it)
    hijos << it->second.node_id << " , ";

  cout<< hijos.str() << endl;
}

/**
* @brief Metodo que envia solicitudes a los vecinos azules.
* @param None.
* @return None.
*/
void Azul::enviarSolicitudesTree(){
  if(miId == 1)
    soyParteArbol = true;

  // cout << "Enviando JoinTree a vecinos." << endl;
  while(continuar && !soyParteArbol){
    for (std::map<uint16_t,node_data>::iterator it=vecinos.begin(); it!=vecinos.end(); ++it){
      if(it->second.node_port)
        enviarJoinTree(it->second); // se envia el node_data
    }
    sleep(1); //duerma por 3 seg
  }
}

/**
* @brief Metodo que agrega un hijo al mapa de mis hijos.
* @param hijo_data: struct que contiene IP/Puerto y nodoId que estoy agregando.
* @return None.
*/
void Azul::agregarHijoArbol(node_data hijo_data){
  hijosArbol[hijo_data.node_id] = hijo_data;
}

/**
* @brief Metodo que envia un pack IDo al nodo que solicito joinTree.
* Se envia si pertenezco al azul.
* @param destino: Par de IP/Puerto donde se envia el mensaje.
* @return None.
*/
void Azul::enviarIDo(pair<char*,uint16_t> destino){
  f_hello IDo;
  IDo.type = 12;
  IDo.node_id = miId;
  // cout << "Enviando IDo a IP: "<< destino.first << " port: "<< destino.second << endl;
  sudp->sendTo((char*)&IDo,sizeof(IDo),destino.first,destino.second);
}

/**
* @brief Metodo que envia un pack IDoNot al nodo que solicito joinTree.
* Se envia si no pertenezco al azul.
* @param destino: Par de IP/Puerto donde se envia el mensaje.
* @return None.
*/
void Azul::enviarIDoNot(pair<char*,uint16_t> destino){
  f_hello IDoNot;
  IDoNot.type = 18;
  IDoNot.node_id = miId;
  // cout << "Enviando IDoNot a IP: "<< destino.first << " port: "<< destino.second << endl;
  sudp->sendTo((char*)&IDoNot,sizeof(IDoNot),destino.first,destino.second);
}

/**
* @brief Metodo que envia una solicitud JoinTree a la direccion por parametro.
* @param destino: Info IP/Puerto donde se envia el mensaje.
* @return None.
*/
void Azul::enviarJoinTree(node_data destino){
  f_hello JoinTree;
  JoinTree.type = 11;
  JoinTree.node_id = miId;
  // cout << "Enviando JoinTree a IP: "<< destino.node_ip << " port: "<< destino.node_port << endl;
  sudp->sendTo((char*)&JoinTree,sizeof(JoinTree),destino.node_ip,destino.node_port);
}

/**
* @brief Metodo que envia un pack Daddy al nodo que pertenece al arbol.
* @param destino: Par de IP/Puerto donde se envia el mensaje.
* @return None.
*/
void Azul::enviarDaddy(pair<char*,uint16_t> destino){
  f_hello Daddy;
  soyParteArbol = true;
  Daddy.type = 13;
  Daddy.node_id = miId;
  // cout << "Enviando Daddy a IP: "<< destino.first << " port: "<< destino.second << endl;
  sudp->sendTo((char*)&Daddy,sizeof(Daddy),destino.first,destino.second);
}
