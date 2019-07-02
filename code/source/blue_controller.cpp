#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>





/*
* Empties the file given by the input path.
* args: string file_path with the path to the file.
* ret: --
*/
void emptyFile(std::string file_path){
  std::ofstream file_clear(file_path,std::ios::trunc);
  file_clear.close();
}


/*
* The blue controller program gives the user a way to close specific blue nodes in an
* ordered manner according to the port they're binded to, or to kill all
* of them using pkill.
*/
int main(int argc,char* argv[]){
  std::string user_input;
  std::string file_path = "../data/blue_pids.txt";
  bool running = true;
  int option;
  int n_port;

  //Sets the blue pid list.
  std::vector<std::string> blue_pids;
  std::fstream pids_file(file_path,std::ios::in);
  std::string current_line;
  while(getline(pids_file,current_line)){
    blue_pids.push_back(current_line);
  }
  pids_file.close();
  emptyFile(file_path); //empties the file in case more blue nodes are created.




  while(running){
    std::cout << "1-List blue node pids. 2-Close node.3-Kill all blue nodes. 4-Exit." << std::endl;
    std::cout << "Select an option(1,2,3,4):";
    getline(std::cin,user_input);
    option = stoi(user_input);
    switch(option){
      case 1: //List blue nodes pids
        std::cout << "Listing nodes..." << std::endl;
        for(int i = 0; i < blue_pids.size(); i++){
          std::cout << "Blue node pid: " << blue_pids[i] << std::endl;
        }
        break;
      case 2: //Close blue node with a given pid.
      {
        std::cout << "Node process id: ";
        getline(std::cin,current_line);
        auto pos = std::find(blue_pids.begin(),blue_pids.end(),current_line);
        if(pos != blue_pids.end()){
          user_input = "kill ";
          user_input += current_line;
          system(&user_input[0]);
          blue_pids.erase(pos);
          std::cout << "The node with pid " << current_line << " has been closed." << std::endl;
        } else {
          std::cout << "Invalid pid/input." << std::endl;
        }
      }
        break;
      case 3: //Kill all blue nodes.
        system("pkill blue-node");
        blue_pids.clear();
        std::cout << "Blue node processes closed." << std::endl;
        break;
      case 4: //Exit the controller program.
        running = false;
        std::cout << "Closing the blue node controller program." << std::endl;
        break;
      default:
        std::cout << "That's not a valid option." << std::endl;
        break;
    }
  }

  return 0;
}
