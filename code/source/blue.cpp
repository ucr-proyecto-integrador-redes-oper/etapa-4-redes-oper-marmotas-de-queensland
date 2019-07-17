#include <utility>
#include <thread>
#include <chrono>
#include "blue.h"



BlueNode::BlueNode(char* server_ip,uint16_t server_port, char* my_ip){
  //sets server info.
  tree_member = false;
  server_data.node_ip = server_ip;
  server_data.node_port = server_port;
  my_data.node_ip = my_ip;
}

BlueNode::~BlueNode(){

}


//////////////////////////////////////Utility functions///////////////////////////////////

/**
*
*/
char* BlueNode::getIP(){
  return my_data.node_ip;
}

/**
* @brief Returns the port that was binded by the node.
* @param --
* @return --
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
* The joinGraph function requests a logical graph asignation from the orange server.
* args: --
* ret: --
*/
void BlueNode::joinGraph(){
  f_join_graph frame;
  frame.type = 14;
  //sends the join graph frame to the orange controller.
  sudp.sendTo((char*)&frame,sizeof(f_join_graph),server_data.node_ip,server_data.node_port);
}


/*
*
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



/*
Effect: Sends hello message to neighbors
Requires: Map of Neighbors filled in.
Modifies: --
*/
void BlueNode::sendHello(uint16_t myID,node_data neighbour){
  f_hello greeting;
  greeting.type=1;
  greeting.node_id=myID;
  //Si se tiene la ip, se envía
  sudp.sendTo((char*)& greeting, sizeof(greeting),neighbour.node_ip,neighbour.node_port);
}

/*
NOTA: Qué pasa con los otros frames que recibe?.
Effect: Waits until a correct complete message arrives
Requires: --
Modifies: --

void BlueNode::waitForComplete(){
  bool recievedComplete = false;
  f_graph_complete completePack;
  std::pair<char*,uint16_t> senderData;
  while(!recievedComplete){
    senderData=sudp.receive((char*)&completePack);
    //Se acepta el paquete solo cuando el paquete es de tipo 17 y viene de la IP puerto registrada como mi naranja
    if(completePack.type == 17&&strncmp(senderData.first,server_data.node_ip,sizeof(server_data.node_ip))
    && senderData.second ==server_data.node_port){
      recievedComplete = true;
    }
  }
}
*/

/////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////Spanning tree related functions////////////////////////////


/**
* @brief The joinTree function generates the first level of the spanning tree. The other levels
* should be generated automatically by the green-blue handler thread. No other message type should
* be received during this tree generation phase.
* @param --
* @return --
*/
void BlueNode::joinTree(){
  f_join_tree join_msg;
  join_msg.type = 11;
  join_msg.node_id = my_data.node_id; //my id
  size_t size = sizeof(f_join_tree);
  bool receiving = true;
  std::map<uint16_t,f_join_tree> ans_map;
  std::pair<char*,uint16_t> sender_data;
  uint8_t type;

  while(!tree_member){
    //sends join message to all neighbours
    for(auto &itr: neighbours){
      sudp.sendTo((char*)& join_msg,size, itr.second.node_ip,itr.second.node_port);
    }

    while(receiving){
      f_join_tree buffer;
      sender_data = sudp.receive((char*)&buffer);
      type = getType((char*)&buffer);
      switch(type){
        case 11: //join tree request from neighbour
          handleTreeRequest(sender_data.first,sender_data.second);
          break;
        case 12: //answer to this node's request.
        case 18:
          ans_map[buffer.node_id] = buffer; //adds the answer to the answer list(map).
          receiving = handleTreeRequestAnswer(&ans_map);
          break;
        case 13: //parent notification, add sender to children list.
          addChildNode(sender_data.first,sender_data.second,buffer.node_id);
          break;
        default: // invalid type.
          break;
      }
    } //end-while(receiving)
    if(!tree_member){ //no neighbour is in spanning tree yet, wait and try again.
      std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    }
  } //end-while(!tree_member)
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
* @brief
* @param
* @param
* @return --
*/
void BlueNode::handleTreeRequest(char* receiver_ip, uint16_t receiver_port){
  size_t msg_size;
  if(my_data.node_id == 0 || tree_member){ //root node or member of tree, affirmative anser.
    f_join_tree_y yes_ans;
    msg_size = sizeof(f_join_tree_y);
    yes_ans.type = 12;
    yes_ans.node_id = my_data.node_id;
    sudp.sendTo((char*)&yes_ans,msg_size,receiver_ip,receiver_port);
  } else { //not root nor a part of tree yet,negative answer.
    f_join_tree_n no_ans;
    msg_size = sizeof(f_join_tree_n);
    no_ans.type = 18;
    no_ans.node_id = my_data.node_id;
    sudp.sendTo((char*)&no_ans,msg_size,receiver_ip,receiver_port);
  }
}

/**
*
*
*/
bool BlueNode::handleTreeRequestAnswer(std::map<uint16_t,f_join_tree> *ans_map){
  if(neighbours.size() == ans_map->size()){ //answer received from all graph neighbours, stop receiving.
    for(auto itr = ans_map->begin(); itr != ans_map->end(); itr++){
      /*
      if(getType(itr->second.type) == 12){ //found the parent node
        //set parent data.
        tree_member = true;
      }
      */

    }
    return false;
  }
  //else return true, still receiving answers from neighbours.
  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////Thread Functions////////////////////////////////////////////

/*
* The receiver thread acts as a dispatcher redirecting the messages it
* receives to the appropiate queue.
*/
void BlueNode::receiver(){
  uint8_t type;
  std::pair<char*,uint16_t> sender_data;
  char buffer[F_PAYLOAD_CAP];
  while(true){
    sender_data = sudp.receive(buffer);
    type = getType(buffer);
    switch(type){
      //Received from green/blue node.
      //Blue-green thread will handle it.
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
        //push message intro blue-green queue.
        //Let blue-green request thread handle.
        break;
      case 14:
      case 15:
      case 16:
      case 17:
        //push message into orange queue.
        //Let the orange requests thread handle.
        break;

      default:
        //Invalid frame type.
        break;
    }
  }
}

/*
* Function for the orange handler thread.
* This thread is in charge of anything related with the orange controller.
*/
void BlueNode::orangeRequests(){
  while(true){

  }
}

/*
* Blue-Green requests thread.
* This thread handles the requests/data of green and blue nodes.
*/
void BlueNode::gbRequests(){
  while(true){

  }
}
