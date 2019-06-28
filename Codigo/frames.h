#ifndef FRAMES_H_
#define FRAMES_H_

#define PAYLOAD_CAP2 1024


struct uint24_t {
  unsigned int data : 24;
}__attribute__((packed));



struct b_chunk{
  uint8_t type; //type 0
  uint24_t file_id;
  uint32_t chunk_id;
  unsigned char payload[PAYLOAD_CAP2];
}__attribute__((packed));



struct b_hello{
  uint8_t type; //type 1
  uint16_t node_id;
}__attribute__((packed));



struct b_exists{
  uint8_t type; //type 2
  uint24_t file_id;
}__attribute__((packed));


typedef b_exists b_exists_ans; //type 3
typedef b_exists b_complete; //type 4


struct b_complete_ans{
  uint8_t type; //type 5
  uint24_t file_id;
  uint32_t chunk_id;
}__attribute__((packed));


typedef b_exists b_get; //type 5
typedef b_chunk b_get_ans; //type 7





#endif
