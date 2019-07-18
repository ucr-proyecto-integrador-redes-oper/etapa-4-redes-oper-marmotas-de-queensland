#include "orange.h"
#include <string.h>
#include <iostream>

using namespace std;

//compilar:
//g++ -o naranja *.cpp -std=c++11 -pthread *.cc

int main(int argc, char* argv[]){
  if(argc < 7){
    cout << "Usage: portNaranja, cantidadNaranjas, portAzul, pathcsv, ipDer, portDer." << endl;
  }else{
    int portNaranja = atoi(argv[1]);
    int cantidadAzules = atoi(argv[2]);
    int portAzul = atoi(argv[3]);
    char* ipDer = argv[5];
    int portDer = atoi(argv[6]);
    char* ipIzq = argv[7];
    int portIzq = atoi(argv[8]);
    int a;
    //cout << "ingrese key" << endl;
    //cin >> a;
    Naranja naranja(portNaranja,cantidadAzules,portAzul,argv[4],ipDer,portDer);
    cout << "ingrese "<<endl;
    cin >> a;
    naranja.iniciar();
  }

  return 0;
}
