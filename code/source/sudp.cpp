#include "sudp.h"

#include <cstdint>
#include <cstring>
#include <thread>
#include <chrono>
#include <iostream>
#include <string>


/**
*  @brief Constructor for the SecureUDP class that receives a specific port to bind
*  and a wait time in ms for the sender thread. It's the user's responsability to
*  send a valid port and wait time numbers.
*  @param port uint16_t with the port
*  @param wait_time uint32_t with the wait_time in ms.
*/
SecureUDP::SecureUDP(uint16_t port,uint32_t wait_time){
  this->port = port;
  setSocket(port);
  sn = rand() % SN_CAP; //start sn with random number [0,2^16-1]
  this->wait_time = wait_time;
  start();
}

/**
*  @brief Constructor for the SecureUDP class that receives a wait time in ms. Since
*  no port is specified the OS will search for any available port to bind.
*  It's the user's responsability to send a valid wait time as input.
*  @param wait_time uint32_t with the wait time in ms.
*/
SecureUDP::SecureUDP(uint32_t wait_time){
  setSocket();
  sn = rand() % SN_CAP; //start sn with random number [0,2^16-1]
  this->wait_time = wait_time;
  start();
}

/**
* @brief Destructor for the SecureUDP class
*/
SecureUDP::~SecureUDP(){

}



/*
* Encapsulates the data in a SecureUPD header and adds it to the send map.
* The data has a cap of 1032 bytes.
* args: char* with the data to send.
*       size_t size of the data about to send.
*       char* with the receiver IP address.
*       uint16_t with the receiver port.
* ret: --
*/
void SecureUDP::sendTo(char* data, size_t sz,char* r_ip, uint16_t r_port){
  sudp_frame* curr_frame = new sudp_frame();
  sudp_rdata* receiver_data = new sudp_rdata();
  smap_value* val = new smap_value();

  //sets de addr and port of the receiver in network order.
  inet_aton(r_ip,&receiver_data->addr);
  receiver_data->port = htons(r_port);

  //sets de sudp header and copies the data.
  curr_frame->type = 0;
  curr_frame->sn = sn;
  memcpy((char*) curr_frame->payload, data ,sz);

  //sets the map value and inserts it using the sn as key.
  val->frame = curr_frame;
  val->rdata = receiver_data;
  val->msg_size = sz + 3;
  //std::cout << "Message size: " << sz << std::endl;

  std::unique_lock<std::mutex> sm_lock(sm_m);
  s_map[sn] = val;
  sm_lock.unlock();

  sn++;
  sn = sn % SN_CAP;
}

/*
* Returns the port binded to the SecureUDP instance.
* args: --
* ret: uint16_t with the port number.
*/
uint16_t SecureUDP::getPort(){
  return port;
}

/*
* The receive function copies the memory into the buffer passed as parameter.
* Buffer size has a cap of 1032 bytes.
* args: char* where the sent data will be copied.
* ret: std::pair<char*,uint16_t> with the IP and port of the sender.
*/
std::pair<char*,uint16_t> SecureUDP::receive(char* buffer){
  std::pair<sudp_frame*,sudp_rdata*> frame_info;

  //caller blocks until the ready queue isn't empty
  std::unique_lock<std::mutex> rd_lock(rd_m);
  rd_cv.wait(rd_lock,[&]{return !ready_queue.empty();});

  ready_mux.lock();
  frame_info = ready_queue.front();
  ready_queue.pop();
  ready_mux.unlock();


  //sets the return values
  char* sender_ip = inet_ntoa(frame_info.second->addr);
  uint16_t sender_port = ntohs(frame_info.second->port);

  //copies the payload.
  memcpy((char*) buffer, (char*) frame_info.first->payload,PAYLOAD_CAP);
  delete frame_info.first;
  delete frame_info.second;
  return std::make_pair(sender_ip,sender_port);
}

/////////////////////////////////Private functions//////////////////////////////////////////

/*
* setSocket receives a port as parameter and binds the socket to the specified port.
* If the socket or bind functions fail the program exits. It's the user's responsability
* to send a valid port number.
* agrs: uint16_t with the port to bind.
* ret: --
*/
void SecureUDP::setSocket(uint16_t port){
  // Creating socket file descriptor , UDP
  if ( (sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family    = AF_INET; // IPv4
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(port);

  //binds the socket
  if ( bind(sock_fd, (sockaddr*) &servaddr,sizeof(sockaddr_in)) < 0 ){
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

}

/*
* setSocket instantiates and binds the socket to any available port on the system.
* args: --
* ret: --
*/
void SecureUDP::setSocket(){
  // Creating socket file descriptor , UDP
  if ( (sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family    = AF_INET; // IPv4
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = 0; //OS will search for a free port to bind.

  //binds the socket
  if ( bind(sock_fd, (sockaddr*) &servaddr,sizeof(sockaddr_in)) < 0 ){
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  sockaddr_in addr;
  socklen_t socklen = sizeof(sockaddr_in);
  getsockname(sock_fd,(sockaddr*) &addr,&socklen);
  port = ntohs(addr.sin_port);
}


/*
* Function for the sender thread. The thread will loop and try to send
* the messages in map evey wait_time in ms. Thread execution will stop when
* the caller process finishes.
* args: --
* ret: --
*/
void SecureUDP::sender(){
  sudp_frame *curr_frame;
  sudp_rdata *dest_data;
  sockaddr_in dest_addr;
  memset(&dest_addr, 0, sizeof(dest_addr));

  while(true){
    std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));

    std::unique_lock<std::mutex> sm_lock(sm_m);
    for(auto itr = s_map.begin(); itr != s_map.end();itr++){
      if(!s_map.empty()){
        curr_frame = itr->second->frame;
        dest_data = itr->second->rdata;

        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = dest_data->port;
        dest_addr.sin_addr = dest_data->addr;

        //sendto
        sendto(sock_fd, (char*) curr_frame, itr->second->msg_size,0,
        (sockaddr*) &dest_addr, sizeof(sockaddr_in));
        }
      }
      sm_lock.unlock();
    }
  }


/*
* Function for the receiver thread.
*/
  void SecureUDP::receiver(){
    sockaddr_in src_addr;
    socklen_t sz = sizeof(src_addr);
    int size;
    while(true){
      sudp_frame *curr_frame = new sudp_frame();
      sudp_rdata *curr_data  = new sudp_rdata();

      //recvfrom syscall, blocking
      size = recvfrom(sock_fd,(char*) curr_frame, sizeof(sudp_frame),0,
      (sockaddr*)&src_addr,&sz);
      curr_data->addr = src_addr.sin_addr;
      curr_data->port = src_addr.sin_port;

      std::unique_lock<std::mutex> rq_lock(rq_m); //lock the cv

      received_mux.lock();
      received_queue.push(std::make_pair(curr_frame,curr_data));
      received_mux.unlock();

      //unlock the cv and notify waiting thread.
      rq_lock.unlock();
      rq_cv.notify_one();
    }
  }

/*
* Function for the processor thread.
* args: --
* ret: --
*/
void SecureUDP::processor(){
  sudp_frame *curr_frame;
  sudp_rdata *curr_data;
  std::pair<sudp_frame*,sudp_rdata*> frame_info;
  while(true){

    std::unique_lock<std::mutex> rq_lock(rq_m); //lock for the cv
    rq_cv.wait(rq_lock, [&]{return !received_queue.empty();}); //blocks until a message is ready to be processed

    received_mux.lock();
    frame_info = received_queue.front();
    received_queue.pop();

    received_mux.unlock(); //unlock the cv


    curr_frame = frame_info.first;
    curr_data = frame_info.second;

    if(curr_frame->type){ //ack type.
      //std::cout << "Ack received, removing from send map." << std::endl;
      std::unique_lock<std::mutex> m_lock(sm_m);
      if(s_map.count(curr_frame->sn)){ //sn matches with one key from s_map,remove element
        delete s_map[curr_frame->sn]->frame;
        delete s_map[curr_frame->sn]->rdata;
        delete s_map[curr_frame->sn];
        s_map.erase(curr_frame->sn);
      }
      m_lock.unlock();

    } else { //send-receive type, return an ack.
        curr_frame->type = 1;

        sockaddr_in dest_addr;
        memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_addr = curr_data->addr;
        dest_addr.sin_port = curr_data->port;

        //sendto syscall
        //std::cout << "Sending ack to IP: " << inet_ntoa(dest_addr.sin_addr) << " and port: " <<  ntohs(dest_addr.sin_port)  << std::endl;
        sendto(sock_fd, (char*) curr_frame, 3,0,
        (sockaddr*) &dest_addr, sizeof(sockaddr_in));

        //pushes data into ready queue.
        std::unique_lock<std::mutex> rd_lock(rd_m); //lock for the cv

        ready_mux.lock();
        ready_queue.push(frame_info);
        ready_mux.unlock();

        rd_lock.unlock(); //unlock the cv
        rd_cv.notify_one();

    }
  }
}

/*
* The start routine instiates the threads and detaches them from the parent
* process. The threads execution stops when the caller process does.
* args: --
* ret: --
*/
void SecureUDP::start(){
  std::thread s(&SecureUDP::sender,this);
  std::thread r(&SecureUDP::receiver,this);
  std::thread p(&SecureUDP::processor,this);
  s.detach();
  r.detach();
  p.detach();
}
