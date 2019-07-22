#include <thread>
#include <chrono>
#include <utility>
#include <sstream>
#include <cstring>
#include <fstream>
#include <iostream>

#include "blue.h"
using namespace std;
/**
* @brief BlueNode constructor
* @param server_ip char* with this node's associated orange controller IP.
* @param server_port uint16_t with this node's associated orange controller port.
* @param my_ip char* with the node's IP.
*/
BlueNode::BlueNode(char* server_ip,uint16_t server_port, char* my_ip){
  //sets server info.
  tree_member = false;
  received_complete = false;
  server_data.node_ip = server_ip;
  server_data.node_port = server_port;
  my_data.node_ip = my_ip;
  my_data.node_id = 0;
  files_path  = "../data/";
}

/**
* @brief BlueNode destructor
*/
BlueNode::~BlueNode(){

}


//////////////////////////////////////Utility functions///////////////////////////////////

/**
* @brief getter function for the node's IP.
* @return node_ip char* with the node's IP.
*/
char* BlueNode::getIP(){
  return my_data.node_ip;
}

/**
* @brief Returns the port that was binded by the node.
* @return port associated with this node's instance of secure udp.
*/
uint16_t BlueNode::getPort(){
  return sudp.getPort();
}

/**
* @brief This function returns the message type for a given char*
* The input must match one of the structs in 'frames.h' to get a meaningful type.
* @param data_source,char* to the data buffer
* @return uint8_t frame type.
*/
uint8_t BlueNode::getType(char* data_source){
  uint8_t type;
  memcpy(&type,data_source,1);
  return type;
}

/**
* @brief The joinGraph function requests a logical graph asignation from the orange server.
*/
void BlueNode::joinGraph(){
  f_join_graph frame;
  frame.type = 14;
  //sends the join graph frame to the orange controller.
  sudp.sendTo((char*)&frame,sizeof(f_join_graph),server_data.node_ip,server_data.node_port);
}


/**
* @brief The start function sets up the node by doing two things.
*        First, it guarantees that the node will join the graph when this is called.
*        Second, it guarantees that the first level of the spanning tree will be generated.
*/
void BlueNode::start(){
  my_data.node_port = getPort();
  joinGraph(); //sends request to join the graph.
  uint8_t type = 0;
  //do stuff
  //set neighbours, etc then instantiate the threads.
  joinTree(); //generates the first level of the tree, the root and its respective children.
  std::thread(&BlueNode::orangeRequests,this).detach();
  std::thread(&BlueNode::gbRequests,this).detach();
  receiver(); //caller acts as receiver thread.
}

///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////Orange comm related functions//////////////////////////////


/**
* @brief Receives node's assigned position and instantiated neighbours' ids
* @param Char* buffer containing packet w type 15 to have been received by node
* @return --
*/
void BlueNode::receivePos(char* buffer){
  f_graph_pos position;
  memcpy((char*)& position, buffer, sizeof(position));
  if((int)position.node_id != -1){
    if(my_data.node_id == 0){ // Si es el primer paquete que se recibe, guardar mi id.
      my_data.node_id = position.node_id;
    }
    node_data neighbour_data;
    neighbour_data.node_ip = 0;
    neighbour_data.node_port = 0;
    neighbour_data.node_id = position.neighbor_id;
    neighbours.emplace((uint16_t)position.neighbor_id, neighbour_data);
  }
  else{
    exit(0);
  }
}
/**
* @brief Receives node's assigned position and instantiated neighbours' ids, ip and port
* @param Char* buffer containing w type 15 to have been received by node
* @return --
*/
void BlueNode::receivePosWNeighbour(char* buffer){
  f_graph_pos_i position;
  memcpy((char*)& position, buffer, sizeof(position));
  if((int)position.node_id != -1){
    if(my_data.node_id == 0){ // Si es el primer paquete que se recibe, guardar mi id.
      my_data.node_id = position.node_id;
    }
    node_data neighbour_data;
    neighbour_data.node_ip = (char*)&position.neighbor_ip;
    neighbour_data.node_port = position.neighbor_port;
    neighbour_data.node_id = position.neighbor_id;
    neighbours.emplace((uint16_t)position.neighbor_id, neighbour_data);
    sendHello(my_data.node_id,neighbour_data); //Sends hello
  }
  else{
    exit(0);
  }
}

/**
* @brief Sends a hello to a given neghbor
* @param Blue Node ID to give neighbor, node_data with my neighbor IP/port
* @return --
*/
void BlueNode::sendHello(uint16_t myID,node_data neighbour){
  f_hello greeting;
  greeting.type=1;
  greeting.node_id=myID;
  //Si se tiene la ip, se envía
  sudp.sendTo((char*)& greeting, sizeof(greeting),neighbour.node_ip,neighbour.node_port);
}

/**
* @brief Sets complete flag on, allowing for the next phase to begin
* @param --
* @return --
*/
void BlueNode::recieveGraphComplete()
{
  received_complete = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////Spanning tree related functions////////////////////////////


/**
* @brief The joinTree function generates the first level of the spanning tree. The other levels
* should be generated automatically by the green-blue handler thread. No other message type should
* be received during this tree generation phase.
*/
void BlueNode::joinTree(){
  if(my_data.node_id == 1){ //already a part of tree
    tree_member = true;
  }
  else {
      f_join_tree join_msg;
      join_msg.type = 11;
      join_msg.node_id = my_data.node_id; //my id
      size_t size = sizeof(f_join_tree);
      bool receiving = true;
      std::map<uint16_t,f_join_tree> ans_map;
      std::pair<char*,uint16_t> sender_data;
      uint8_t type;
      char recv_buffer[F_PAYLOAD_CAP];
      //While not recieved a completed tree
      while(receiving)
      {
        //Send join message to neighbours //IF NOT TREE SEND TO NEIGHBOURS
        if(!tree_member)
        {
          for(auto &itr: neighbours)
          {
            sudp.sendTo((char*)& join_msg,size, itr.second.node_ip,itr.second.node_port);
          }
        }

        //While message
        while(receiving)
        {

          f_join_tree buffer;
          //char* buffer;
          sender_data = sudp.receive(recv_buffer);
          memcpy((char*)&buffer,recv_buffer,sizeof(f_join_tree));
          type = getType((char*)&buffer);
          switch(type)
          {
            case 11: //join tree request from neighbour
              handleTreeRequest(sender_data.first,sender_data.second);
              break;
            case 12: //answer to this node's request.
            case 18: //ONLY IF NOT TREE
              ans_map[buffer.node_id] = buffer; //adds the answer to the answer list(map).
              receiving = handleTreeRequestAnswer(&ans_map);
              break;
            case 13: //parent notification, add sender to children list.
              addChildNode(sender_data.first,sender_data.second,buffer.node_id);
              break;
            default: // invalid type.
              cout<<"Recieved invalid message type: "<<type;
              break;
          }
        } //end-while(receiving)
        if(!tree_member){ //no neighbour is in spanning tree yet, wait and try again.
          ans_map.clear();
          std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        }
      } //end-while(!tree_member)
  }
}


/**
* @brief Adds a child to the node's list.
* @param ip char* with the node's ip.
* @param port uint16_t with the node's port.
* @param node_id uint16_t with the node's identifier.
*/
void BlueNode::addChildNode(char* ip, uint16_t port, uint16_t node_id){
  node_data child_node;
  child_node.node_ip = ip;
  child_node.node_port = port;
  child_node.node_id = node_id;
  child_nodes[node_id] = child_node;
}


/**
* @brief This function handles the arrival of a join tree request(type 11).
* @param ip the IP of the sender node.
* @param port the port of the sender node.
* @return --
*/
void BlueNode::handleTreeRequest(char* ip, uint16_t port){
  size_t msg_size;
  if(tree_member)
  { //member of tree, affirmative anser.
    f_join_tree_y yes_ans;
    msg_size = sizeof(f_join_tree_y);
    yes_ans.type = 12;
    yes_ans.node_id = my_data.node_id;
    sudp.sendTo((char*)&yes_ans,msg_size,ip,port);
  }
  else
  { //not root nor a part of tree yet,negative answer.
    f_join_tree_n no_ans;
    msg_size = sizeof(f_join_tree_n);
    no_ans.type = 18;
    no_ans.node_id = my_data.node_id;
    sudp.sendTo((char*)&no_ans,msg_size,ip,port);
  }
}

/**
* @brief This function handles the arrival of an anwer to the join tree request(type 12 or 18).
* @param ans_map map<node_id,answer_frame> containing the answers from all neighbours.
* @return true if still receiving answer frames from neighbours, false if all answers have been received.
*/
bool BlueNode::handleTreeRequestAnswer(std::map<uint16_t,f_join_tree> *ans_map){
  bool receiving = true;
  if(neighbours.size() == ans_map->size()){ //answer received from all graph neighbours, stop receiving.
    for(auto &itr : *ans_map){
      if(itr.second.type == 12){ //found the parent node
        parent_data = neighbours[itr.second.node_id];
        //generate the parent frame and send it.
        f_parent parent_msg;
        parent_msg.type = 13;
        parent_msg.node_id = parent_data.node_id;
        sudp.sendTo((char*)&parent_msg,sizeof(f_parent),parent_data.node_ip,parent_data.node_port);
        tree_member = true;
        break; //found the parent with lowest id, stop iterating.
      }
    }
    receiving = false;
  }
  //else return true, still receiving answers from neighbours.
  return receiving;
}

///////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////File handling related functions////////////////////////
/**
* @brief This function handles recieving a hello, adding the neighbor IP,Port to map of neighbors
* @param char* neighborIp, uint16_t Port and ID to add
*/
void handleHello(char*neighborIP,uint16_t neighborPort,uint16_t neighborID)
{
      //Add to map the neighbor with key neighborID the data IP,Port

}


/**
* @brief This function handles the behavior when a node receives a chunk from a file.
* @param current_msg fchunk* with the data about to be handled.
*/
void BlueNode::handleChunkRequest(f_chunk* current_msg){
  uint24_t current_file_id = current_msg->file_id;
  std::stringstream file;
  file << files_path << current_file_id.data << ".dat";
  if(files.count(current_file_id)){ //Node already has chunks from file stored.
    //check for chunk cap
    //gen probabilities, etc.
  } else { //it's a chunk from a new file
    files[current_file_id] = 1; //add it to map

    //generates the file and writes the data chunk.
    std::fstream current_file_stream;
    current_file_stream.open(file.str(), std::ios::in | std::ios::binary | std::ios::app);
    current_file_stream.write((char*)current_msg,sizeof(f_chunk));
    current_file_stream.close();

    //send it to neighbour to copy the data.
    //sudp.sendTo();
  }
}

/**
* @brief This function handles checking and sending to parent Blue or green node wheter or not a chunk exists or passes the meessage along the kids
* @param current_msg fchunk* exists message,
*/
void BlueNode::handleExistsRequest(f_exists* current_msg, char* solicitingIP, uint16_t solicitingPort)
{
  //Get file ID from current_msg

  //Assume green did not ask
  bool greenAsked=false;
  //Check if id from file ID contained is the same as myGreenId
    //if true, myGreenAsks= true

  //If you can Open file from current message id
    //Prepare an exists answer package
    //If myGreenAsks
      //Send it to my myGreenAsks
    //else
      //Send it to solicitng IP
  //else
    //Prepare an exists package and send to every node other in tree other than the soliciting one
    //Save a pending answer with exists file ID as key and the IP/Port,counter of soliciter
}

/**
* @brief Recieves an asnwer for an exists and passes the asnwer on
* @param Answer package
*/
void BlueNode::handleExistsRequestAnswer(f_exists_ans* answerFromChild)
{
  //prepare answer package
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
* @brief
*/
void BlueNode::handleCompleteRequest(){}

/**
* @brief
*/
void BlueNode::handleCompleteRequestAnswer(){}

/**
* @brief
*/
void BlueNode::handleGetRequest(){}

/**
* @brief
*/
void BlueNode::handleGetRequestAnswer(){}

/**
* @brief
*/
void BlueNode::handleLocateRequest(){}

/**
* @brief
*/
void BlueNode::handleLocateRequestAnswer(){}

/**
* @brief if there is a chunk of the given file, it is deleted.
* @param Frame with delete package frame
*/
void BlueNode::handleDeleteRequest(f_delete toDelete)
{
  //If you can Open a file with chunk frames
    //Delete it
  //Passes the file to every neighbor in the tree other than the one the gave the chunk
}



///////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////Thread Functions////////////////////////////////////////////

/**
* @brief Function for the receiver thread. The receiver thread acts as a dispatcher
*        redirecting the messages it receives to the appropiate queue.
*/
void BlueNode::receiver(){
  uint8_t type;
  std::pair<char*,uint16_t> sender_data;
  while(true){
    char* buffer = new char[F_PAYLOAD_CAP];
    sender_data = sudp.receive(buffer);
    type = getType(buffer);
    switch(type){
      //Received from green/blue node.
      //green-blue thread will handle it.
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
      case 11:
      case 12:
      case 13:
      case 18:
        {
          std::unique_lock<std::mutex> gblck(gb_mtx); //lock the green-blue condition variable.

          gb_lock.lock(); //lock the green-blue queue.
          gb_queue.push(buffer);
          gb_lock.unlock(); //release the green-blue queue.

          gblck.unlock(); //release the cv.
          gb_cv.notify_one();
        }
        break;
      case 14:
      case 15: // Receive pos (no neighbor id and port)
      case 16: // Receive pos (w/ neighbor id and port)
      case 17:
        {
          std::unique_lock<std::mutex> olck(orange_mtx); //lock the orange condition variable.

          orange_lock.lock(); //lock orange queue.
          orange_queue.push(buffer);
          orange_lock.unlock(); //release orange queue.

          olck.unlock(); //release the orange cv.
          orange_cv.notify_one();
        }
        break;
      default:
        //Invalid frame type.
        //add message to log file.
        break;
    }
  }
}

/**
* @brief Function for the orange requests thread.
*        This thread is in charge of anything related with the orange controller.
*/
void BlueNode::orangeRequests(){
  char* buffer;
  uint8_t type;
  while(true){
    //thread blocks until the orange queue isn't empty
    std::unique_lock<std::mutex> olck(orange_mtx);
    orange_cv.wait(olck,[&]{return !orange_queue.empty();});

    orange_lock.lock(); //lock the queue
    buffer = orange_queue.front();
    orange_queue.pop();
    orange_lock.unlock(); //release the queue

    type = getType(buffer);
    switch(type){

      case 15:// Receive pos (no neighbor id and port)
        receivePos(buffer);
      break;

      case 16:// Receive pos (w/ neighbor id and port)
        receivePosWNeighbour(buffer);
      break;

      case 17: //Recieve a Graph complete
        recieveGraphComplete();
      break;

      default:
        //wrong type somehow
        //add message to log file.
        break;
    }
    delete buffer;
    buffer = nullptr;
  }
}

/**
* @brief Function for the green-blue requests thread.
*        This thread handles the requests/data of green and blue nodes.
*/
void BlueNode::gbRequests(){
  char* buffer;
  uint8_t type;
  node_data sender_data;

  while(true){
    //thread blocks until the green-blue queue isn't empty
    std::unique_lock<std::mutex> gblck(gb_mtx);
    gb_cv.wait(gblck,[&]{return !gb_queue.empty();});

    gb_lock.lock(); //lock the queue
    buffer = gb_queue.front();
    gb_queue.pop();
    gb_lock.unlock(); //release the queue

    type = getType(buffer);
    switch(type){
      case 0: //Chunk request from another node.
        {
          f_chunk* current_msg =  (f_chunk*) buffer;
          handleChunkRequest(current_msg);
        }
        break;
      case 2: //Exists request from another node.
        {
          f_exists* current_msg = (f_exists*) buffer;
        //  handleExistsRequest(current_msg);
        }
        break;
      case 3: //Answer to this node's exists request
        {
          f_exists_ans* current_msg = (f_exists_ans*) buffer;
        //  handleExistsRequestAnswer();
        }
        break;
      case 4: // Complete request from another node.
        {
          f_complete* current_msg = (f_complete*) buffer;
          handleCompleteRequest();
        }
        break;
      case 5: //Answer to this node's complete request.
        {
          f_complete_ans* current_msg = (f_complete_ans*) buffer;
          handleCompleteRequestAnswer();
        }
        break;
      case 6: //Get request from another node.
        {
          f_get* current_msg = (f_get*) buffer;
          handleGetRequest();
        }
        break;
      case 7: //Answer to this node's get request.
        {
          f_get_ans* current_msg = (f_get_ans*) buffer;
          handleGetRequestAnswer();
        }
        break;
      case 8: //Locate request from another node.
        {
          f_locate* current_msg = (f_locate*) buffer;
          handleLocateRequest();
        }
        break;
      case 9: //Answer to this node's locate request.
        {
          f_locate_ans* current_msg = (f_locate_ans*) buffer;
          handleLocateRequestAnswer();
        }
        break;
      case 10: //Delete request from another node.
        {
          f_delete* current_msg = (f_delete*) buffer;
          //handleDeleteRequest();
        }
        break;
      case 11: //request to join spanning tree from another node.
        {
          f_join_tree *current_msg = (f_join_tree*) buffer;
          sender_data = neighbours[current_msg->node_id];
          handleTreeRequest(sender_data.node_ip,sender_data.node_port);
        }
        break;
      case 13: //parent notification, add sender to children list.
        {
          f_join_tree *current_msg = (f_join_tree*) buffer;
          sender_data = neighbours[current_msg->node_id];
          addChildNode(sender_data.node_ip,sender_data.node_port,sender_data.node_id);
        }
        break;
      default:
        //wrong type somehow, add message to log file.
        break;
    }
    delete buffer;
    buffer = nullptr;
  }
}
