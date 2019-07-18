#ifndef BLUE_H_
#define BLUE_H_

#include <map>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string.h>

#include "frames.h"
#include "sudp.h"

#define MY_UINT16_MAX (UINT16_MAX-1)

/*
* Keeps track of the IP and port of a node.
*/
struct node_data{
  char* node_ip;
  uint16_t node_port;
  uint16_t node_id;
};


class BlueNode{
public:
  BlueNode(char *, uint16_t , char*);
  ~BlueNode();


  //Utility routines
  char* getIP();
  uint16_t getPort();
  void start();

private:
    node_data my_data;
    node_data server_data;
    bool tree_member;
    bool received_complete;

    SecureUDP sudp = decltype(sudp)(1000);
    std::mutex orange_mtx; //for the cv.
    std::mutex gb_mtx; //for the cv.
    std::mutex orange_lock; //lock for the orange buffer.
    std::mutex gb_lock; //lock for the green-blue buffer.
    std::condition_variable orange_cv;
    std::condition_variable gb_cv;


    std::map<uint16_t,node_data> neighbours; //graph neighbours.
    node_data parent_data; //parent data for the spanning tree.
    std::map<uint16_t,node_data> child_nodes; // spanning tree child nodes.
    std::queue<char*> orange_queue; //orange buffer.
    std::queue<char*> gb_queue; //green-blue buffer.

    //map with the ids of all 'owned' files and the # of chunks for each.
    std::map<uint24_t,uint8_t> files;

    //Utility routines.
    uint8_t getType(char*);

    //Orange-related routines.
    void sendHello(uint16_t myID,node_data neighbour);
    void waitForComplete();
    void joinGraph();
    void receivePos(char* buffer);
    void receivePosWNeighbour(char* buffer);
    void recieveGraphComplete();

    //Spanning Tree routines
    void joinTree();
    void addChildNode(char*,uint16_t,uint16_t);
    void handleTreeRequest(char*,uint16_t);
    bool handleTreeRequestAnswer(std::map<uint16_t,f_join_tree>*);

    //File handling routines
    void handleChunkRequest(f_chunk*);
    void handleExistsRequest(f_exists*);
    void handleExistsRequestAnswer();
    void handleCompleteRequest();
    void handleCompleteRequestAnswer();
    void handleGetRequest();
    void handleGetRequestAnswer();
    void handleLocateRequest();
    void handleLocateRequestAnswer();
    void handleDeleteRequest();

    //Thread routines.
    void receiver();
    void orangeRequests();
    void gbRequests(); //blue-green requests.
};

#endif
