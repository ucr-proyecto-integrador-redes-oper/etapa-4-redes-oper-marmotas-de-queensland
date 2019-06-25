// #include "naranja.h"
#include "server.h"
//#include "client.h"
#include <string.h>
#include<iostream>
#include "paquetes.h"
using namespace std;
int main(int argc, char* argv[]){
	cout << sizeof(short);
  cout<< argv[1] << endl;
  Server* server = new Server();
  server->server_bind(atoi(argv[1]));
  server->server_listen(1);
  server->server_accept();

    struct pack_inicial inicial;
    struct pack_solicitud solicitud;
    struct pack_complete complete;
    struct pack_vacio vacio;

    vacio.id = 3;
    complete.id =2;
    solicitud.id = 1;
    inicial.id = 0;

    int a = 1;
    while(a){
      if(a == 1){ //recibir pack inicial
        cout << "recibiendo inicial" << endl;
        server->receivePackage((char*)&inicial,sizeof(inicial));
        cout << inicial. ip << endl;

      }else if(a == 2){ //enviar inicial
        cout << "ingrese ip de inicial" << endl;
        cin >> inicial.ip;
        server->writeToServer((char*)&inicial,sizeof(inicial));

      }else if(a == 3){//recibir solicitud
        server->receivePackage((char*)&solicitud,sizeof(solicitud));
        cout << "id : "<< solicitud.id << "id nodo" << solicitud.idNodo << "ipA" << solicitud.ipAzul << "port" << solicitud.portAzul << endl;

      }else if(a==4){//enviar solicitud
        cout << "ingrese idNodo,ip azul y puerto" << endl;
        solicitud.id = 1;
        cin >>solicitud.idNodo;
        cin >>solicitud.ipAzul;
        cin >>solicitud.portAzul;
        server->writeToServer((char*)&solicitud,sizeof(solicitud));

      }else if(a==5){//enviar complete
        //complete.id = 2;
        cout << "enviando complete" << endl;
        server->writeToServer((char*)&complete,sizeof(complete));

      }else if(a==6){//recibir complete
        server->receivePackage((char*)&complete,sizeof(complete));
        cout << "id: " << complete.id << endl;

      }else if(a==7){//enviar vacio
        // vacio.id = 3;
        cout << "enviando vacio" << endl;
        server->writeToServer((char*)&vacio,sizeof(vacio));

      }else if(a==8){//recibir vacio
        server->receivePackage((char*)&vacio,sizeof(vacio));
        cout << "id " << vacio.id << endl;
      }
      cout << "ingrese a" << endl;
      cin >> a;
    }

    delete server;



  return 0;
}
