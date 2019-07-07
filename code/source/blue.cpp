#include <utility>
#include <thread>
#include "blue.h"
#include "frames.h"



BlueNode::BlueNode(char* server_ip,uint16_t server_port, char* my_ip){
  //sets server info.
  server_data.n_ip = server_ip;
  server_data.n_port = server_port;
  my_data.n_ip = my_ip;
}

BlueNode::~BlueNode(){

}

char* BlueNode::getIP(){
  return my_data.n_ip;
}

/*
* Returns the port that was binded by the node.
* args: --
* ret: --
*/
uint16_t BlueNode::getPort(){
  return sudp.getPort();
}

/*
* This function returns the message type for a given char*
* The input must match one of the structs in frames.h to get a meaningful type.
* args: char* to the data buffer
* ret: uint8_t frame type.
*/
uint8_t BlueNode::getType(char* data_source){
  uint8_t type;
  memcpy(&type,data_source,1);
  return type;
}

/*
* The joinGraph function requests a logical graph asignation from the orange server.
* args: --
* ret: --
*/
void BlueNode::joinGraph(){
  f_join_graph frame;
  frame.type = 14;
  //sends the join graph frame to the orange controller.
  sudp.sendTo((char*)&frame,sizeof(f_join_graph),server_data.n_ip,server_data.n_port);
}


/*
Effect: Sends hello message to neighbors
Requires: Map of Neighbors filled in.
Modifies: --
*/
void BlueNode::sendHello(uint16_t myID,n_data neighbour){
  f_hello greeting;
  greeting.type=1;
  greeting.node_id=myID;
  //Si se tiene la ip, se envía
  sudp.sendTo((char*)& greeting, sizeof(greeting),neighbour.n_ip,neighbour.n_port);
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
    if(completePack.type == 17&&strncmp(senderData.first,server_data.n_ip,sizeof(server_data.n_ip))
    && senderData.second ==server_data.n_port){
      recievedComplete = true;
    }
  }
}
*/

/*
*
*/
void BlueNode::start(){
  my_data.n_port = getPort();
  joinGraph(); //sends request to join the graph.
  uint8_t type = 0;
  //do stuff
  //set neighbours, etc then instantiate the threads.

  std::thread(&BlueNode::orangeRequests,this).detach();
  std::thread(&BlueNode::bgRequests,this).detach();
  receiver(); //caller acts as receiver thread.
}


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

}

/*
* Blue-Green requests thread.
* This thread handles the requests/data of green and blue nodes.
*/
void BlueNode::bgRequests(){

}
