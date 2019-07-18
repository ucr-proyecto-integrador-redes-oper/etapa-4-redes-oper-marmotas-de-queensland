#ifndef FRAMES_H_
#define FRAMES_H_

#define F_PAYLOAD_CAP 1024



/**
* @brief This struct represents a 24bit unsigned integer.
*
*/
struct uint24_t {
  unsigned int data : 24;
}__attribute__((packed));


//////////////////////////////BLUE-GREEN NODES COMM FRAMES//////////////////////////////////////////////


struct f_chunk{
  uint8_t type; //type 0
  uint24_t file_id;
  uint32_t chunk_id;
  unsigned char payload[F_PAYLOAD_CAP];
}__attribute__((packed));



struct f_hello{
  uint8_t type; //type 1
  uint16_t node_id;
}__attribute__((packed));



struct f_exists{
  uint8_t type; //type 2
  uint24_t file_id;
}__attribute__((packed));


typedef f_exists f_exists_ans; //type 3 - same as 2
typedef f_exists f_complete; //type 4 - same as 2


struct f_complete_ans{
  uint8_t type; //type 5
  uint24_t file_id;
  uint32_t chunk_id;
}__attribute__((packed));


typedef f_exists f_get; //type 6 - same as 2
typedef f_chunk f_get_ans; //type 7 - same as 0
typedef f_exists f_locate; //type 8, same as 2

struct f_locate_ans{
  uint8_t type; // type 9
  uint24_t file_id;
  uint16_t node_id;
}__attribute__((packed));

typedef f_exists f_delete; //type 10 - same as 2

///////////////////////// SPANNING TREE FRAMES/////////////////////////////////////////////

typedef f_hello f_join_tree; //type 11 - same as 1
typedef f_hello f_join_tree_y; //type 12 - same as 1
typedef f_hello f_join_tree_n; // type 18 - same as 1
typedef f_hello f_parent; //type 13 - same as 1


/////////////////////////// ORANGE-BLUE NODES COMM FRAMES///////////////////////////////////

struct f_join_graph{
  uint8_t type; // type 14
}__attribute__((packed));

struct f_graph_pos{
  uint8_t type; //type 15
  uint16_t node_id;
  uint16_t neighbor_id;
}__attribute__((packed));

struct f_graph_pos_i{
  uint8_t type; //type 16
  uint16_t node_id;
  uint16_t neighbor_id;
  uint32_t neighbor_ip;
  uint16_t neighbor_port;
}__attribute__((packed));

typedef f_join_graph f_graph_complete; //type 17 - same as 14


#endif
