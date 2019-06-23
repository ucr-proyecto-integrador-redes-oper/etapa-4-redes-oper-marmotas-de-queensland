#include "sudp.h"
#include <cstdint>
#include <cstring>
#include <thread>
#include <chrono>
#include <iostream>



/*
  Constructor for the SecureUDP class
*/
SecureUDP::SecureUDP(int port,int wait_time){
  this->port = port;
  setSocket();
  sn = rand() % SN_CAP; //start sn with random number [0,2^16-1]
  this->wait_time = wait_time;
  start();
}

/*
  Destructor for the SecureUDP class
*/
SecureUDP::~SecureUDP(){

}



/*
  Encapsultates the data in a SecureUPD header and adds it to the send map.
*/
void SecureUDP::sendTo(char* data, char* r_ip, uint16_t r_port){
  sudp_frame* curr_frame = new sudp_frame();
  sudp_rdata* receiver_data = new sudp_rdata();
  smap_value* val = new smap_value();

  //sets de addr and port of the receiver in network order.
  inet_aton(r_ip,&receiver_data->addr);
  receiver_data->port = htons(r_port);

  //sets de sudp header and copies the data.
  curr_frame->type = 0;
  curr_frame->sn = sn;
  memcpy((char*) curr_frame->payload, data , PAYLOAD_CAP);
  //sets the map value and inserts it using the sn as key.
  val->frame = curr_frame;
  val->rdata = receiver_data;
  val->ack_received = false;
  s_map[sn] = val;

  sn++;
  sn = sn % SN_CAP;
}

/*


*/
void SecureUDP::receive(char* buffer){
  sudp_frame *curr_frame;
  bool empty = false;

  //caller waits until the queue isn't empty
  std::unique_lock<std::mutex> rq_lock(rq_m);
  //std::cout << "Lock aquired by receive caller." << std::endl;

  //std::cout << "Waiting on signal from receiver thread." << std::endl;
  rq_cv.wait(rq_lock,[&](){return !r_queue.empty();});
  //std::cout << "Siganl received." << std::endl;

  curr_frame = r_queue.front();
  r_queue.pop();
  //std::cout << "Queue operations working." << std::endl;

  //copies the payload.
  memcpy((char*) buffer, (char*) curr_frame->payload,PAYLOAD_CAP);
  std::cout << curr_frame->payload << std::endl;
  delete curr_frame;
}












/////////////////////////////////Private functions//////////////////////////////////////////

void SecureUDP::setSocket(){
  // Creating socket file descriptor , UDP
  if ( (sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
    perror("socket creation failed");
  }

  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));

  servaddr.sin_family    = AF_INET; // IPv4
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(port);
  bind(sock_fd,(struct sockaddr*) &servaddr,sizeof(sockaddr_in));

}

void SecureUDP::sender(){
  sudp_frame *curr_frame;
  sudp_rdata *rdata;
  bool ack_received;
  while(true){
    std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));

    for(auto itr = s_map.begin(); itr != s_map.end();itr++){
      if(!s_map.empty()){
        curr_frame = itr->second->frame;
        rdata = itr->second->rdata;
        ack_received = itr->second->ack_received;
        if(ack_received){ //message acked, remove from list
          delete curr_frame;
          delete rdata;
          delete itr->second;
          s_map.erase(itr);
        } else{ //send the message
           //sendto syscall, sets the client socket info before sending.
          cliaddr.sin_family = AF_INET;
          cliaddr.sin_port = rdata->port;
          cliaddr.sin_addr = rdata->addr;
          sendto(sock_fd, (char*) curr_frame, sizeof(sudp_frame),0,
          (sockaddr*) &cliaddr, sizeof(sockaddr_in));
        }
      }
    }
  }
}

void SecureUDP::receiver(){
  while(true){
    sudp_frame *curr_frame = new sudp_frame();
    //recvfrom syscall
    recvfrom(sock_fd,(char*) curr_frame, sizeof(sudp_frame),0,
    (sockaddr *) &servaddr,(unsigned int*)sizeof(sockaddr_in));

    if(curr_frame->type){ //ack
      //lock?
      if(s_map.count(curr_frame->sn)){ //sn matches with one key from s_map
        s_map[curr_frame->sn]->ack_received = true;
      }
    } else { //send-receive type
        struct sockaddr_in dest_addr;
        curr_frame->type = 1;

        //sendto syscall
        sendto(sock_fd, (char*) curr_frame, sizeof(sudp_frame),0,
        (sockaddr*) &servaddr, sizeof(sockaddr_in));

        std::unique_lock<std::mutex> rq_lock(rq_m);
        //std::cout << "Lock aquired by receiver thread." << std::endl;

        r_queue.push(curr_frame);
        rq_lock.unlock();
        //std::cout << "Realeased mutex from receiver thread." << std::endl;

        rq_cv.notify_one();
        //std::cout << "Notifying caller thread." << std::endl;
    }
  }
}

void SecureUDP::start(){
  std::thread s(&SecureUDP::sender,this);
  std::thread r(&SecureUDP::receiver,this);
  s.detach();
  r.detach();
}
