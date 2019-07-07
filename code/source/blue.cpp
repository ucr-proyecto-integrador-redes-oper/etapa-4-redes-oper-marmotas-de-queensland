#include "blue.h"

BlueNode::BlueNode(char* server_ip,uint16_t server_port){
  //sets server info.
  server_data.n_ip = server_ip;
  server_data.n_port = server_port;
}


BlueNode::~BlueNode(){

}

char* BlueNode::getIP(){
  return nullptr;
}

uint16_t BlueNode::getPort(){
  return sudp.getPort();
}
/*
Effect: Sends hello message to neighbors
Requires: Map of Neighbors filled in.
Modifies: --
*/
void BlueNode::sendHello(uint16_t myID)
{
  std::map<uint16_t,n_data>::iterator iter = neighbours.begin();
  n_data neighbour;
  f_hello greeting;
  greeting.type=1;
  greeting.node_id=myID;
	// Iterate over the map of neighbors
	while (iter != neighbours.end())
	{
		// Se obtiene par
		neighbour = iter->second;
    //Si se tiene la ip, se envía
    if(strncmp(neighbour.n_ip,"",sizeof(neighbour.n_ip)) == 0)
    {
      //Envia Hello
      sudp.sendTo((char*)& greeting, sizeof(greeting),neighbour.n_ip,neighbour.n_port);
    }
		iter++;
	}
}
/*
Effect: Waits until a correct complete message arrives
Requires: --
Modifies: --
*/
void BlueNode::waitForComplete()
{ bool recievedComplete = false;
  f_graph_complete completePack;
  std::pair<char*,uint16_t> senderData;
  while(!recievedComplete)
  {
    senderData=sudp.receive((char*)&completePack);
    //Se acepta el paquete solo cuando el paquete es de tipo 17 y viene de la IP puerto registrada como mi naranja
    if(completePack.type == 17&&strncmp(senderData.first,server_data.n_ip,sizeof(server_data.n_ip)) && senderData.second ==server_data.n_port)
    {
      recievedComplete = true;
    }
  }
}
