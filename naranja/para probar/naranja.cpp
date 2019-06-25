#include "naranja.h"

Naranja::Naranja(int portNaranja,short portAzul,char* ipDer,short portDer,char* ipIzq,short portIzq){

  this->banderaSolicitud = 0;
  this->cantidadCompletos = 0;
  this->token = 0;

  this->ipDer = ipDer;
  this->portDer = portDer;

  this->ipIzq = ipIzq;
  this->portIzq = portIzq;

  //solo asi pude sacar la ip de mi pc
  hostent * localhost = gethostbyname("localhost");
  in_addr * address = (in_addr * )localhost->h_addr;
  string ip_address = inet_ntoa(* address);
  this->miIp = inet_addr(ip_address.c_str());//pasar de string a int

  inicial.id = 0;
  solicitud.id = 1;
  complete.id = 2;
  vacio.id = 3;

  //Iniciar socket para naranjas
	//this->portNaranjas = new SecureUDP(portNaranja,0);

  //Iniciar socket para azules
  //this->portAzul = new SecureUDP(portAzul,0);

  server = new Server();
  client = new Client();

  connectS();
  connectC();

}

Naranja::~Naranja(){
  server->close_server();
  client->closeClient();

  delete server;
  delete client;
  // delete portNaranjas;
  //delete portAzul;
}

//funcionamiento interno:

//inicia le funcionamiento del naranja
void Naranja::iniciar(){
  this->enviarInicial();
  //recibir 5 paquetes de los otros naranjas
  for(int i = 0; i < 5;++i){
    server->receivePackage((char*)&inicial,sizeof(inicial));
    //portNaranjas->receive((char*)&inicial);
    cout << "Ip: " << inicial.ip << "id: " << inicial.id << endl;
    //y guardar las ips
    ipsVecinos.push_back(inicial.ip);
  }

  if(verificarNaranjas()) //si soy el mas bajo
    crearToken();

  verificarPaquetes();
}

//este metodo debe ser ejecutado con un thread para verificar el timeout
int Naranja::timeout(){

}

//verifica si mi ip es la mas baja y retorna un valor. 0 si no, 1 si si
int Naranja::verificarNaranjas(){
  int menor = 1;
  for(int i = 0 ; i < ipsVecinos.size() && menor == 1 ;++i)
    if(ipsVecinos[i] < miIp)
      menor = 0;

  return menor;
}

//guarda en la cola de solicitudes las solicitudes de los azules.
void Naranja::guardarSolicitud(pack_solicitud nuevaSolicitud){
  //guardarlo en la cola
  solicitudes.push(nuevaSolicitud);
}

//crea el token, modificando el atributo token
void Naranja::crearToken(){
  cout << "Creando Token" << endl;
  this->token = 1;
}

//contiene switch con los diferentes id de packs
void Naranja::verificarPaquetes(){
  cout << "Recibiendo paquetes" << endl;
  //recibir paquetes
  int id = -1;
  while(true){
    //verificar paquetes
    /*
    switch (id) {
      case 1:

        break;
      case 2:

        break;
      case 3:

        break;
      default:

        break;
    }*/
  }
}

//para el sig metodo ocupo algo para almacenar los nodos ocupados
//al recibir una solicitud, la escribo en este metodo
void Naranja::ocuparNodoGrafo(){

}

//paquetes:
void Naranja::enviarInicial(){
  //enviar inicial
  memcpy((char*)&inicial.ip,(char*)&this->miIp,4);
  client->writeToServer((char*)&inicial,sizeof(inicial));
  //portNaranjas->sendTo((char*)&inicial,ipDer,portDer);
}

void Naranja::enviarSolicitud(){
  //enviar solicitud
}

void Naranja::enviarTokenVacio(){
  //enviar vacio
}

void Naranja::enviarComplete(){
  //enviar complete
}

void Naranja::enviarCompleteAzules(){

}
