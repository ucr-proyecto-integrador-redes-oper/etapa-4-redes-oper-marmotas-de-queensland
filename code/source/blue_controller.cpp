#include <iostream>
#include <string>

/*
* The blue controller program gives the user a way to close specific blue nodes in an
* ordered manner according to the port they're binded to, or to kill all
* of them using pkill.
*/
int main(int argc,char* argv[]){

  std::string user_input;
  bool running = true;
  int option;
  int n_port;
  while(running){
    getline(std::cin,user_input);
    option = stoi(user_input);
    switch(option){
      case 1: //List blue nodes ports
        std::cout << "Listing nodes..." << std::endl;
        //List here.
        break;
      case 2: //Close blue node with a given port.
        getline(std::cin,user_input);
        n_port = stoi(user_input);
        std::cout << "Closing node with port: " << n_port << std::endl;
        //close here
        break;
      case 3: //Kill all blue nodes.
        system("pkill blue-node");
        break;
      case 4: //Exit the controller program.
        running = false;
        std::cout << "Closing the blue node controller program." << std::endl;
        break;
    }
  }
  return 0;
}
