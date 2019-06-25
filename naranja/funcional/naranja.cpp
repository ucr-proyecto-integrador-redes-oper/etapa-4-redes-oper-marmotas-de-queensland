#include "naranja.h"

int Naranja::count_Lines(string &path){
	ifstream csvFile;
	csvFile.open(path.c_str());

	if(!csvFile.is_open()){
		exit(EXIT_FAILURE);
	}

	string line;
	while(getline(csvFile, line)){
		if(line.empty()){
			continue;
		}
		else{
			line_count++;
		}
	}

	return line_count;
}
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
  line_count = 0;
  //Iniciar socket para naranjas
	//this->portNaranjas = new SecureUDP(portNaranja,0);

  //Iniciar socket para azules
  //this->portAzul = new SecureUDP(portAzul,0);

  client = new Client();
  connectC(portNaranja);

}

Naranja::~Naranja(){
  client->closeClient();

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
    client->readFromServer((char*)&inicial,sizeof(inicial));
    //portNaranjas->receive((char*)&inicial);
    cout << "Ip: " << inicial.ip << "id: " << inicial.id << endl;
    //y guardar las ips
    ipsVecinos.push_back(inicial.ip);
  }

  //el paquete que yo envie, vuelve a mi
  //se espera por el pero no se hace nada
  client->readFromServer((char*)&inicial,sizeof(inicial));

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
  int id = 5;
  if(this->token)
    id = 4;

  //recibir paquetes
  while(id){
    //recibir paquete, se recibe con el tipo de paquete mas grande posible
    cout << "Esperando por paquete" << endl;
    client->readFromServer((char*)&solicitud,sizeof(solicitud));
    //se saca el id del paquete
    memcpy((char*)&id,(char*)&solicitud.id,sizeof(char));

    switch (id) {
      case 1://recibo solicitud
        cout << "Se recibio una solicitud de nodo naranja." << endl;
        //guardar solicitud recibida
        //ocuparNodoGrafo();
        //enviar solicitud al sig
        this->client->writeToServer((char*)&solicitud,sizeof(solicitud));
        break;

      case 2://recibo complete
        cout << "Un servidor naranja completo sus azules." << endl;
        ++cantidadCompletos;
        //enviar al nodo derecho el complete
        this->enviarComplete();

        if(this->cantidadCompletos == 6)//si ya estan los complete
          enviarCompleteAzules();//avisar a los azules

        break;

      case 3://recibo token vacio
        cout << "Se recibio token vacio" << endl;

        if(this->solicitudes.empty()){//si no hay solicitudes
          cout << "No hay solicitudes de nodo azul." << endl;
          this->enviarTokenVacio();//envie token vacio
          break;//se hace el break dentro del if para que pase al case 4
          //en caso de que hayan solicitudes
        }

        this->token = 1;

      case 4://cree el token y tengo solicitudes
        if(!this->solicitudes.empty()){
          cout << "Atendiendo solicitud de nodo azul." << endl;
//atender una solicitud
          enviarSolicitud();

          //falta recibirla se recibe del izq.
          this->client->readFromServer((char*)&solicitud,sizeof(solicitud));
        }
        this->enviarTokenVacio();//envie token vacio luego de realizar una solicitud

        break;
      default:
        id = 0;
        cout << "Terminando programa" << endl;
        break;
    }
  }
}

//paquetes:
void Naranja::enviarInicial(){
  //enviar inicial
  memcpy((char*)&inicial.ip,(char*)&this->miIp,4);
  this->client->writeToServer((char*)&inicial,sizeof(inicial));
  //portNaranjas->sendTo((char*)&inicial,ipDer,portDer);
}

void Naranja::enviarSolicitud(){
  //enviar solicitud
  //copia la solicitud en struct
  memcpy((char*)&solicitud,(char*)&solicitudes.front(),sizeof(solicitud));
  this->solicitudes.pop();//saco la solicitud de la cola
  //se envia solicitud al nodo naranja derecho
  this->client->writeToServer((char*)&solicitud,sizeof(solicitud));
}

void Naranja::enviarTokenVacio(){
  //enviar vacio
  this->client->writeToServer((char*)&vacio,sizeof(vacio));
  this->token = 0;
}

void Naranja::enviarComplete(){
  //enviar complete
  this->client->writeToServer((char*)&complete,sizeof(pack_complete));
}

//Cosas de azules:
void Naranja::enviarCompleteAzules(){

}

//para el sig metodo ocupo algo para almacenar los nodos ocupados
//al recibir una solicitud, la escribo en este metodo
void Naranja::ocuparNodoGrafo(){

}
