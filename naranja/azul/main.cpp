#include <iostream>
#include <string>
#include <fstream>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include "azul.h"
#include <thread>

using namespace std;

int main(int argc, char* argv[]){
  int a = 1;
  Azul azul(argv[1],atoi(argv[2]));
  azul.start();

  return 0;
}
