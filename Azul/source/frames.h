#ifndef FRAMES_H_
#define FRAMES_H_

#define F_PAYLOAD_CAP 1024



/**
* @brief This struct represents a 24bit unsigned integer.
*/
struct uint24_t {
  unsigned int data : 24;
}__attribute__((packed));

struct uint24_compare
{
    bool operator() ( const uint24_t a, const uint24_t b ) const
    { return a.data > b.data; }
};
//////////////////////////////BLUE-GREEN NODES COMM FRAMES//////////////////////////////////////////////

/**
* @brief Struct for the data chunk request
*         Type 0
*/
struct f_chunk{
  uint8_t type; //type 0
  uint24_t file_id;
  uint32_t chunk_id;
  unsigned char payload[F_PAYLOAD_CAP];
}__attribute__((packed));


/**
* @brief Struct for the Hello request
*        Type 1
*/
struct f_hello{
  uint8_t type; //type 1
  uint16_t node_id;
}__attribute__((packed));


/**
* @brief Struct for the Exists request
*        Type 2
*/
struct f_exists{
  uint8_t type; //type 2
  uint24_t file_id;
}__attribute__((packed));

/**
* @brief Typedef for the answer to an Exists request
*        Type 3, same struct as 2
*/
typedef f_exists f_exists_ans; //type 3 - same as 2

/**
* @brief Typedef for the Complete request
*        Type 4, same struct as 2
*/
typedef f_exists f_complete; //type 4 - same as 2

/**
* @brief Struct for the answer to a complete request
*        Type 5
*/
struct f_complete_ans{
  uint8_t type; //type 5
  uint24_t file_id;
  uint32_t chunk_id;
}__attribute__((packed));


/**
* @brief Typedef for the Get request
*        Type 6, same struct as 2
*/
typedef f_exists f_get; //type 6 - same as 2

/**
* @brief Typedef for the answer to a Get request.
*        Type 7, same struct as 0
*/
typedef f_chunk f_get_ans; //type 7 - same as 0

/**
* @brief Typedef for the Locate request
*        Type 8, same struct as 2
*/
typedef f_exists f_locate; //type 8, same as 2

/**
* @brief Struct for the answer to a Locate request
*        Type 9
*/
struct f_locate_ans{
  uint8_t type; // type 9
  uint24_t file_id;
  uint16_t node_id;
}__attribute__((packed));

/**
* @brief Typedef for the Delete request
*        Type 10, same struct as 2
*/
typedef f_exists f_delete; //type 10 - same as 2

///////////////////////// SPANNING TREE FRAMES/////////////////////////////////////////////

/**
* @brief Typedef for the JoinTree request
*        Type 11, same struct as 1
*/
typedef f_hello f_join_tree; //type 11 - same as 1

/**
* @brief Typedef for the affirmative answer to a JoinTree request
*        Type 12, same struct as 1
*/
typedef f_hello f_join_tree_y; //type 12 - same as 1

/**
* @brief Typedef for the negative answer to a JoinTree request
*        Type 18, same struct as 1
*/
typedef f_hello f_join_tree_n; // type 18 - same as 1

/**
* @brief Typedef for Parent request.
*        Type 12, same struct as 1
*/
typedef f_hello f_parent; //type 13 - same as 1


/////////////////////////// ORANGE-BLUE NODES COMM FRAMES///////////////////////////////////

/**
* @brief Struct for the JoinGraph request
*        Type 14
*/
struct f_join_graph{
  uint8_t type; // type 14
}__attribute__((packed));

/**
* @brief Struct containing the data when the neighbour's IP/Port aren't known.
*        Type 15
*/
struct f_graph_pos{
  uint8_t type; //type 15
  uint16_t node_id;
  uint16_t neighbor_id;
}__attribute__((packed));

/**
* @brief Struct containing the data when the neighbour's IP/Port are known.
*        Type 16
*/
struct f_graph_pos_i{
  uint8_t type; //type 16
  uint16_t node_id;
  uint16_t neighbor_id;
  uint32_t neighbor_ip;
  uint16_t neighbor_port;
}__attribute__((packed));

/**
* @brief Typedef for the GrapComplete message.
*        Type 17, same struct as 15
*/
typedef f_join_graph f_graph_complete; //type 17 - same as 14


#endif
