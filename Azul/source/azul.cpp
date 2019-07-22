#include "azul.h"

Azul::Azul(char* ipNaranja, int portNaranja){
  this->sudp = new SecureUDP(2000);
  this->ipNaranja = ipNaranja;
  this->portNaranja = portNaranja;
  files_path  = "../data/";
  num_of_chunks = 0;
}

Azul::~Azul(){
  delete sudp;
}

void Azul::start()
{
  this->joinGraph();
  this->recibirVecinos();
  this->joinTree();
  it_vecinos = vecinos.begin();
  cout<< "Soy el nodo: " << miId << "Mis vecinos en el arbol son: ";
  for(auto &itr : hijosArbol)
  {
    cout<<" "<<itr.second.node_id<<" -- ";
  }

  cout<<"Y mi padre es: "<<parent_data.node_id<< endl << endl;
}

void Azul::joinGraph(){
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////File handling routines////////////////////////////////////////////////////

/**
 * @brief Auxiliary method to alternate neighbours we need to send stuff to
 */
void Azul::neighbour_raffle(f_chunk* current_msg){
  node_data neighbour;
  //Check iterator to send to. If it is the end, begin anew.
  if(it_vecinos != vecinos.end()){
    neighbour=it_vecinos->second;
    it_vecinos++;
  }
  else{
    it_vecinos = vecinos.begin();
    neighbour=it_vecinos->second;
    it_vecinos++;
  }
  sudp->sendTo((char*)& current_msg,sizeof(current_msg), neighbour.node_ip, neighbour.node_port);
}

//AZUL-AZUL
/**
* @brief This function handles the behavior when a node receives a chunk from a file.
* @param current_msg fchunk* with the data about to be handled.
*/
  void Azul::PutChunkRequest(f_chunk* current_msg)
{
  uint24_t current_file_id = current_msg->file_id;
  std::stringstream file;
  file << files_path << current_file_id.data << ".dat";

  if(files.count(current_file_id)){ //Node already has chunks from file stored.
    if(num_of_chunks<TOTAL_CHUNK_CAP){
      map<uint24_t,uint8_t>::iterator it = files.find(current_file_id);
      if (it->second<FILE_CHUNK_CAP){
        it->second++;
        /*if (files[current_file_id]<FILE_CHUNK_CAP){
          files[current_file_id]++;*/                   //Implementacion original de este if
        //Generates the file and writes the data chunk.
        std::fstream current_file_stream;
        current_file_stream.open(file.str(), std::ios::in | std::ios::binary | std::ios::app);
        current_file_stream.write((char*)current_msg,sizeof(f_chunk));
        current_file_stream.close();
      }
    }
    else{ //Either or both caps have been reached
      //Go some place else
      neighbour_raffle(current_msg);
    }
  }
  else { //it's a chunk from a new file
    if(num_of_chunks<TOTAL_CHUNK_CAP){
      map<uint24_t,uint8_t>::iterator it = files.find(current_file_id);
      it->second = 1; //add chunk to map
      //Store chunk!
      std::fstream current_file_stream;
      current_file_stream.open(file.str(), std::ios::in | std::ios::binary | std::ios::app);
      current_file_stream.write((char*)current_msg,sizeof(f_chunk));
      current_file_stream.close();
    }
    else{ //Either or both caps have been reached
      //Go some place else
      neighbour_raffle(current_msg);
    }
  }
}

/**
* @brief This function handles checking and sending to parent Blue or green node wheter or not a chunk exists or passes the m3essage along the kids
* @param current_msg fchunk* exists message,
*/
void Azul::ExistsRequest(f_exists* current_msg, char* solicitingIP, uint16_t solicitingPort, uint16_t solicitingID){
  //Get file ID from current_msg
  uint24_t payload = current_msg->file_id;

  //Store asking node's info to know who to send a reply later
  node_data whosAsking;
  whosAsking.node_ip = solicitingIP;
  whosAsking.node_port = solicitingPort;
  whosAsking.node_id = solicitingID;


  //Assume green did not ask
  bool greenAsked=false;

  //Get payload bit by bit to later concatenate only group id and green id (first byte)
  unsigned* bits = (unsigned*)malloc(sizeof(unsigned) * 23);
  for(unsigned i = 0; i < 23; ++i){
    bits[i] = (payload.data >> i) & 1;
  }
  unsigned green_id;
  unsigned file_id;
  for(unsigned i = 0; i < 23; ++i){
    //welp. this is awkward.
  }

  //Check if id from file ID contained is the same as myGreenId
    //if true, myGreenAsks= true

  //If you can Open file from current message id
    //Prepare an exists answer package
    //If myDad myGreenAsks
      //Send it to my myGreenAsks
    //else
      //Send it to solicitng IP
  //else
    //Prepare an exists package and send to every node other in tree other than the soliciting one
    //Save a pending answer with exists file ID as key and the IP/Port,counter of soliciter
  free(bits);
}

/**
* @brief Recieves an answer for an exists and passes the answer on
* @param Answer package
*/
void Azul::ExistsAnswer(f_exists_ans* answerFromChild){
  //prepare answer package
  f_exists_ans existsAns;
  //If this pack is the last(N neighbors in map -1) || positive
    //If the green ID in pack is my greenAsked
      //Send to my green
    //else
      //Look for the FileID in map
      //if found,
        //Send answer to IP Port related to question
        //delete ID from map
  //else
    //If finds file in map
      // Add 1 to the counter
}

/**
* @brief Checks for chunks on tree and dictates whether an object
* can be reassembled or not
* @param  "complete?" package from green node
*/
void Azul::CompleteRequest(f_complete* completeReq){
  //Gets file ID from request
  //JUST SPREAD IT ALL OVER
}
/**
* @brief Generates an answer for each chunk found
*/
void Azul::CompleteAnswer(){

}

/**
* @brief
*/
void Azul::GetRequest(){

}
/**
* @brief
*/
void Azul::GetAnswer(){

}

/**
* @brief
*/
void Azul::LocateRequest(){

}
/**
* @brief
*/
void Azul::LocateAnswer(){

}

/**
* @brief if there is a chunk of the given file, it is deleted.
* @param Frame with delete package frame
*/

 void Azul::DeleteRequest(f_delete* toDelete)
{
  uint24_t file_to_delete = toDelete->file_id;
  if(files.count(file_to_delete)){ //Node has chunks from file in store.
    //Delete chunks!
    map<uint24_t,uint8_t>::iterator it;
    for(it = files.begin(); it != files.end(); it++){
    }
  }
  //Passes the file to every neighbor in the tree other than the one the gave the chunk
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////Spanning tree related functions////////////////////////////


/**
* @brief The joinGraph function generates the first level of the spanning tree. The other levels
* should be generated automatically by the green-blue handler thread. No other message type should
* be received during this tree generation phase.
*/

void Azul::joinTree()
{
  cout<<"Soy el nodo: "<< miId<< "Intendando entrar al arbol..." << endl << endl;
  bool allNeighborsBelong=false;
  int  numVecinos=0;
  int  numVecinosContados=0;
  //Arma paquete para enviar a
  f_join_tree join_msg;
  join_msg.type = 11;
  join_msg.node_id = miId; //my id
  size_t size = sizeof(f_join_tree);
  bool receiving = true;
  std::map<uint16_t,f_join_tree> ans_map;
  std::pair<char*,uint16_t> sender_data;
  uint8_t type;
  char recv_buffer[F_PAYLOAD_CAP];
  //Cuenta numero de vecinos que tengo
  for(auto &itr: vecinos)
  {
    numVecinos ++;
  }
  if(miId == 1)
  {
    ///If I am the first node, I already belong in graph.
    tree_member = true;
    cout << "Soy el nodo: " << miId<< "Soy el inicio del arbol "<< miId << endl << endl;
  }
 else
 {
    cout << "Soy el nodo: " << miId << "Intentando entrar arbol..."<< endl << endl;
    //While I'm not in tree
    while(!tree_member)
    {
      //Send join message to neighbours
      cout << "Soy el nodo: " << miId << "Enviando Join a vecinos..." << endl << endl;
      for(auto &itr: vecinos)
      {
        sudp->sendTo((char*)& join_msg,size, itr.second.node_ip,itr.second.node_port);
      }

      //While recieving messages, as I understand Join Requests arrive first, due to every blue sending messages first.
      while(receiving)
      {
        cout << "Soy el nodo: " << miId<< "Recibiendo Mensajes..."<<endl<<endl;
        //Creates a buffer with largest obsect to recieve pack
        f_join_tree buffer;
        //Recieves message and stores it
        sender_data = sudp->receive(recv_buffer);
        memcpy((char*)&buffer,recv_buffer,sizeof(f_join_tree));
        type = buffer.type;
        switch(type)
        {
          case 11: //join tree request from neighbour
            cout << "Soy el nodo: " << miId<< "Respondiendo I Do Not."<< endl<< endl;
            handleTreeRequest(sender_data.first,sender_data.second);
            break;
          case 12: //Recieved IDo IDoNot answers from requests.
          case 18:
            cout << "Soy el nodo: " << miId << "Revisando si llegaron todos los mensajes."<< endl<< endl;
            ans_map[buffer.node_id] = buffer; //adds the answer to the answer list(map).
            //Only when all answers have been recieved, this turns true. It also turns tree_member true if one IDo is recieved.
            receiving = handleTreeRequestAnswer(&ans_map);
            break;
          case 13: //parent notification, add sender to children list. This cannot happen here
              cout << "Soy el nodo: " << miId<< "Aceptación invalida a agregar hijo"<< endl << endl;
            //agregarHijo(sender_data.first,sender_data.second,buffer.node_id);
            break;
          default: // invalid type.
            cout << "Soy el nodo: " << miId<<"Mensaje invalido de tipo: "<<type << endl << endl;
            break;
        }
      } //end-while(receiving)
      if(!tree_member)//no neighbour is in spanning tree yet, wait and try again.
      {
        cout << "Soy el nodo: " << miId<< "No pude unirme al arbol. Durmiendo..."<< endl << endl;
        ans_map.clear();
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        cout << "Soy el nodo: " << miId<<"Despertando...Intentando volver a entrar al arbol"<< endl << endl;
      }
    } //end-while(!tree_member)
  }
  cout << "Soy el nodo: " << miId<< "Me he unido al arbol" << endl << endl;


  //Seguir recibiendo nodos hasta completar todos los vecinos estén en grafo
  while(!allNeighborsBelong)
  {
    //Send join message to neighbours
    cout << "Soy el nodo: " << miId << "Enviando Join a vecinos..."<< endl << endl;
    for(auto &itr: vecinos)
    {
      sudp->sendTo((char*)& join_msg,size, itr.second.node_ip,itr.second.node_port);
    }

    allNeighborsBelong=true;
    //While I have yet to get answers from all my neighbours
    cout << "Soy el nodo: " << miId<< "Recibiendo peticiones para entrar a arbol" << endl << endl;
    while(numVecinosContados != numVecinos)
    {
      f_join_tree buffer;
      //char* buffer;
      sender_data = sudp->receive(recv_buffer);
      memcpy((char*)&buffer,recv_buffer,sizeof(f_join_tree));
      type = buffer.type;
      switch(type)
      {
        case 11: //join tree request from neighbour
          cout << "Soy el nodo: " << miId << "Enviando a mi vecino IDO" << endl << endl;
          handleTreeRequest(sender_data.first,sender_data.second);
        break;
        case 12: //I do answer, no problem
          numVecinosContados++;
        break;
        case 18: //I do not answer, not all neighbors belong
          cout << "Soy el nodo: " << miId <<"Faltan vecinos por unirse" <<endl <<endl;
          numVecinosContados++;
          allNeighborsBelong=false;
        break;
        case 13: //parent notification, add sender to children list.
        cout << "Soy el nodo: " << miId <<"Tengo un nuevo hijo. Agregando a lista..."<< endl;
          agregarHijo(sender_data.first,sender_data.second,buffer.node_id);
        break;
        default: // invalid type.
          cout << "Soy el nodo: " << miId << "Mensaje invalido de tipo: "<<type << endl << endl;
        break;
      }
    }

    if(!allNeighborsBelong)
    {
      cout << "Soy el nodo: " << miId<<"Faltan vecinos de entrar a arbol" << endl <<endl;
      numVecinosContados=0;
    }
  }
cout << "Soy el nodo: " << miId<<" Todos mis vecinos pertenecen al arbol. Inicialización completa"<<endl<<endl;
}


/**
* @brief Adds a child to the node's list.
* @param ip char* with the node's ip.
* @param port uint16_t with the node's port.
* @param node_id uint16_t with the node's identifier.
*/
void Azul::agregarHijo(char* ip, uint16_t port, uint16_t node_id){
  node_data nodoHijo;
  nodoHijo.node_ip = ip;
  nodoHijo.node_port = port;
  nodoHijo.node_id = node_id;
  cout << "Agregando hijo con ID:"<<node_id<<" con IP: "<<ip<<" y puerto: "<<port<<endl<<endl;
  hijosArbol[node_id] = nodoHijo;
}


/**
* @brief This function handles the arrival of a join tree request(type 11).
* @param ip the IP of the sender node.
* @param port the port of the sender node.
* @return --
*/
void Azul::handleTreeRequest(char* ip, uint16_t port){
  size_t msg_size;
  if(tree_member)
  { //member of tree, affirmative anser.
    f_join_tree_y yes_ans;
    msg_size = sizeof(f_join_tree_y);
    yes_ans.type = 12;
    yes_ans.node_id = miId;
    sudp->sendTo((char*)&yes_ans,msg_size,ip,port);
  }
  else
  { //not root nor a part of tree yet,negative answer.
    f_join_tree_n no_ans;
    msg_size = sizeof(f_join_tree_n);
    no_ans.type = 18;
    no_ans.node_id = miId;
    sudp->sendTo((char*)&no_ans,msg_size,ip,port);
  }
}

/**
* @brief This function handles the arrival of an anwer to the join tree request(type 12 or 18).
* @param ans_map map<node_id,answer_frame> containing the answers from all neighbours.
* @return true if still receiving answer frames from neighbours, false if all answers have been received.
*/
bool Azul::handleTreeRequestAnswer(std::map<uint16_t,f_join_tree> *ans_map){
  bool receiving = true;
  if(vecinos.size() == ans_map->size())
  { //answer received from all graph neighbours, stop receiving.
    for(auto &itr : *ans_map)
    {
      if(itr.second.type == 12)
      { //found the parent node
        parent_data = vecinos[itr.second.node_id];
        //generate the parent frame and send it.
        f_parent parent_msg;
        parent_msg.type = 13;
        parent_msg.node_id = parent_data.node_id;
        sudp->sendTo((char*)&parent_msg,sizeof(f_parent),parent_data.node_ip,parent_data.node_port);
        tree_member = true;
        break; //found the parent with lowest id, stop iterating.
      }
    }
    receiving = false;
  }
  //else return true, still receiving answers from neighbours.
  return receiving;
}
