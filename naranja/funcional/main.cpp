#include "naranja.h"
#include <string.h>
#include<iostream>

using namespace std;
int main(int argc, char* argv[]){
  int port = atoi(argv[1]);
  int portAzul = atoi(argv[2]);

  Naranja naranja(port,0,0,0,0,0);
  naranja.iniciar();



  return 0;
}
