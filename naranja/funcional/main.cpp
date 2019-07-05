#include "naranja.h"
#include <string.h>
#include<iostream>

using namespace std;

//compilar:
//g++ -o naranja *.cpp -std=c++11 -pthread *.cc

int main(int argc, char* argv[]){
  if(argc < 6){
    cout << "Usage: portNaranja, portAzul, pathcsv, ipDer, portDer." << endl;
  }else{
    int portNaranja = atoi(argv[1]);
    int portAzul = atoi(argv[2]);
    char* ipDer = argv[4];
    int portDer = atoi(argv[5]);
    char* ipIzq = argv[6];
    int portIzq = atoi(argv[7]);
    int a;
    //cout << "ingrese key" << endl;
    //cin >> a;
    Naranja naranja(portNaranja,0,argv[3],ipDer,portDer);
    cout << "ingrese "<<endl;
    cin >> a;
    naranja.iniciar();
  }

  return 0;
}
