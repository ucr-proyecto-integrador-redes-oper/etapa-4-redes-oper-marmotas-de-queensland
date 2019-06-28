#include "udp.h"
#include <string.h>
#include <stdio.h>
#include<iostream>
#include "paquetes.h"
using namespace std;
int main(int argc, char* argv[]){
	//cout << sizeof(short);
	//cout<< argv[1] << endl;
	UDP udp(atoi(argv[1]));

    struct pack_inicial inicial;
    struct pack_solicitud solicitud;
    struct pack_complete complete;
    struct pack_vacio vacio;

    vacio.id = 3;
    complete.id =2;
    solicitud.id = 1;
    inicial.id = 0;

    int a = 1;
    int port;
    uint16_t ports = 0;
    while(a){
      if(a == 1){ //recibir pack inicial
        cout << "recibiendo inicial" << endl;
        udp.receive((char*)&inicial,sizeof(inicial));
        cout << inicial.id << " ip: " << inicial.ip << endl;

      }else if(a == 2){ //enviar inicial
        cout << "ingrese ip de inicial y port a enviar" << endl;
        cin >> inicial.ip;
	cin >> port;
        udp.sendTo((char*)&inicial,sizeof(inicial),"10.1.137.131",port);

      }else if(a == 3){//recibir solicitud
        udp.receive((char*)&solicitud,sizeof(solicitud));
	memcpy((char*)ports,(char*)&solicitud.portAzul,2);
        cout << "id : "<< solicitud.id << "id nodo" << solicitud.idNodo << "ipA" << solicitud.ipAzul << "port" << ports << endl;

      }else if(a==4){//enviar solicitud
        cout << "ingrese idNodo,ip azul y puerto,port" << endl;
        solicitud.id = 1;
        cin >>solicitud.idNodo;
        cin >>solicitud.ipAzul;
        cin >> solicitud.portAzul;
	cin >> port;

        cout << solicitud.idNodo;
        cout << solicitud.ipAzul;
        cout << solicitud.portAzul;
        udp.sendTo((char*)&solicitud,sizeof(solicitud),"10.1.137.131",port);

      }else if(a==5){//enviar complete
        complete.id = 2;
	cout << "enviando complete, ingrese puerto" << endl;
	cin >> port;
        udp.sendTo((char*)&complete,sizeof(complete),"10.1.137.131",port);

      }else if(a==6){//recibir complete
        udp.receive((char*)&complete,sizeof(complete));
        cout << "id: " << complete.id << endl;

      }else if(a==7){//enviar vacio
        vacio.id = 3;
        cout << "enviando vacio, ingrese puerto" << endl;
	cin >> port;
        udp.sendTo((char*)&vacio,sizeof(vacio),"10.1.137.131",port);

      }else if(a==8){//recibir vacio
        udp.receive((char*)&vacio,sizeof(vacio));
        cout << "id " << vacio.id << endl;
      }
      cout << "ingrese a" << endl;
      cin >> a;
    }

//    delete server;



  return 0;
}
