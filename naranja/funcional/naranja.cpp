#include "naranja.h"
Naranja::Naranja(int portNaranja,short portAzul,string pathcsv,char* ipDer,short portDer){

  //Iniciar socket para naranjas
  udpNaranjas = new UDP(portNaranja);
  //Iniciar socket para azules
  udpAzules = new UDP(portAzul);

  this->banderaSolicitud = 0;
  this->cantidadCompletos = 0;
  this->token = 0;

  this->ipDer = ipDer;
  this->portDer = portDer;
  //cout << "ip Derecha: " << this->ipDer << " puerto: " << portDer << endl;
  //this->ipIzq = ipIzq;
  //this->portIzq = portIzq;

  //cout << "ip Izquierda: " << this->ipIzq << " puerto: " << portIzq << endl;

  //se pregunta por la ip de mi pc///////////////////////////////
  string localIp;
  cout << "Ingrese la Ip local invertida : " << endl;
  cin >> localIp;

  this->miIp = inet_addr(localIp.c_str());//pasar de string a int
  cout << "Int de la Ip: "<< localIp <<" es: " << this->miIp << endl;
  //////////////////////////////////////////////////////////////
  //cantidad de nodos en el grafo, para luego iniciar los bitmap
  count_Lines(pathcsv);
  cout << line_count << endl;
  // cout << "Tamanio inicial: " << sizeof(pack_inicial) << endl;
  // cout << "Tamanio solicitud: " << sizeof(pack_solicitud) << endl;
  // cout << "Tamanio complete: " << sizeof(complete) << endl;
  // cout << "Tamanio vacio: " << sizeof(vacio) << endl;

  // enviarInicial();
}

Naranja::~Naranja(){
  delete udpNaranjas;
  delete udpAzules;
}

//funcionamiento interno:
//arreglar esto
int Naranja::count_Lines(string &path){
  line_count = 0;
  ifstream csvFile;
	csvFile.open(path.c_str(), ifstream::in);

	if(!csvFile.is_open()){
		exit(EXIT_FAILURE);
	}
	char line[100];
	while(csvFile.getline(line,100,'\r') || csvFile.getline(line,100,'\n')){//esto esta asi porque es windows(\r), \n si es de linux
		line_count++;
	}
  csvFile.close(); //
	return line_count;
}

//inicia le funcionamiento del naranja
//debe enviarse en broadcast a izq y der.
void Naranja::iniciar(){
  this->enviarInicial();

  if(verificarNaranjas()) //si soy el mas bajo
    crearToken();

  verificarPaquetes();
}

//este metodo debe ser ejecutado con un thread para verificar el timeout
int Naranja::timeout(){
  return 0;
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
    if(token==0){ // si no tengo el token
    	cout << "Esperando por paquete" << endl;
    	this->udpNaranjas->receive((char*)&solicitud,sizeof(solicitud));
    	//se saca el id del paquete
    	memcpy((char*)&id,(char*)&solicitud.id,sizeof(char));
    }else{//si tengo el token
 	    id = 3;
    }
    switch (id) {
      case 1://recibo solicitud
        cout << "Se recibio una solicitud de otro nodo naranja." << endl;
      	cout << solicitud.id << endl;
      	cout << solicitud.idNodo << endl;
      	cout << solicitud.ipAzul << endl;
      	cout << solicitud.portAzul << endl;
        //guardar solicitud recibida
        //ocuparNodoGrafo();
        //enviar solicitud al sig
        this->udpNaranjas->sendTo((char*)&solicitud,sizeof(solicitud),ipDer,portDer);
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
          cout << "No hay solicitudes de nodo azul, enviando token vacio." << endl;
          this->enviarTokenVacio();//envie token vacio
	  this->token = 0 ;
          break;//se hace el break dentro del if para que pase al case 4
          //en caso de que hayan solicitudes
        }

        this->token = 1;

      case 4://cree el token y tengo solicitudes
        if(!this->solicitudes.empty()){
          cout << "Atendiendo solicitud de nodo azul." << endl;
          //atender una solicitud
          enviarSolicitud();
        }
        cout << "Enviando token vacio." << endl;
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
//arreglar bug
void Naranja::enviarInicial(){
  //enviar inicial
  queue<pack_inicial> paquetesIniciales;
  char id = '0';
  inicial.ip = this->miIp;
  inicial.id = (char)0;
  memcpy((char*)&inicial.id,&id,1);
  //enviar por der e izq
  cout << "Enviando: " << inicial.ip << " id: " << inicial.id << " a derecha: "<<  portDer <<endl;
  this->udpNaranjas->sendTo((char*)&inicial,sizeof(inicial),ipDer,portDer);
  //cout << "Enviando: " <<inicial.ip << " id: " << inicial.id << " a izquierda: "<<  portIzq <<endl;
  //this->udpNaranjas->sendTo((char*)&inicial,sizeof(inicial),ipIzq,portIzq);

  int otroIp = 0;
  bool miIp = false;
  int misPaquetes = 0;
  bool yaEsta = false;

  //recibir paquetes hasta que haya llegado mis otros 2
  while(misPaquetes < 1){
    this->udpNaranjas->receive((char*)&inicial,sizeof(inicial));
    memcpy((char*)&otroIp,(char*)&inicial.ip,4);
    //cout << "Se recibio id: " << inicial.id << "con ip: " << inicial.ip << endl;
    if(inicial.ip == this->miIp){
      cout << "Llego un pack mio" << endl;
      miIp = true;
      yaEsta = true;
      ++misPaquetes;
    }
    for(int index = 0; index < ipsVecinos.size() && !yaEsta; ++index){
      //recibir paquetes y guardarlos(si ya los tengo solo los envio), y luego se envian
      //se reciben hasta que sean mis paquetes,
      if(ipsVecinos[index] == otroIp)
        yaEsta = true;
    }
    if(!yaEsta){
      ipsVecinos.push_back(otroIp);
      cout << "Agregando Ip: " << otroIp << endl;
    }
    if(!miIp)
    	this->udpNaranjas->sendTo((char*)&inicial,sizeof(inicial),ipDer,portDer);

    yaEsta = false;
    miIp = false;
  }
  // for(int i = 0 ; i < ipsVecinos.size();++i)
  //   cout << ipsVecinos[i] << endl;
  cout << "Finalizo la fase inicial." << endl;
}

void Naranja::enviarSolicitud(){
  //enviar solicitud
  //copia la solicitud en struct
  memcpy((char*)&solicitud,(char*)&solicitudes.front(),sizeof(solicitud));
  solicitud.id = 1;

  this->solicitudes.pop();//saco la solicitud de la cola
  //se envia solicitud al nodo naranja derecho
  this->udpNaranjas->sendTo((char*)&solicitud,sizeof(solicitud),ipDer,portDer);
  //falta recibirla se recibe del izq.
  this->udpNaranjas->receive((char*)&solicitud,sizeof(solicitud));
}

void Naranja::enviarTokenVacio(){
  //enviar vacio
  vacio.id = 3;
  this->udpNaranjas->sendTo((char*)&vacio,sizeof(vacio),ipDer,portDer);
  this->token = 0;
}

void Naranja::enviarComplete(){
  //enviar complete
  complete.id = 2;
  this->udpNaranjas->sendTo((char*)&complete,sizeof(pack_complete),ipDer,portDer);
}

//Cosas de azules:
/*
Efecto: Recibe un mensaje de un azul por Secure udp y lo encola
Requiere: Cola de solicitudes, socket abierto a solictudes
Modifica: Cola de solicitudes
*/
void recibirSolicitudAzul(){
  //Si el token ring está listo
    //Escucha por Azules
    //Pasa a cola al llegar algo
}

/*
Efecto: Envia al azul su posición en el grafo. Numero paquete = 15
Requiere: Número de nodo que se asignó
Modifica: El numero en grafo del azul al que se envía el paquete
*/
void enviarPosicion(){
  //Recibe parametros para crear el paquete posición
  //envia paquete al azul
}

/*
Efecto: Envia al azul su posición en grafo y una lista de vecinos. Numero paquete = 16
Requiere: Número de nodo que se asignó, lista de vecinos
Modifica: El numero en grafo del azul al que se le envía el paquete y sus lista de vecinos
*/
void enviarPosConVecino(){
  //Recibe parametros para crear el paquete posición.
  //Itera por una lista de vecinos, agregandolos al paquetes
  //Envia Paquete al azul
}

/*
Efecto: Asigna un número en el grafo a un nodo azul.
Requiere: Numero nodo de petición, mapa de bits de nodos azules,
Modifica: Mapa de bits de nodos ocupaos local y en red. Asigna un numero a un  nodo azul.
*/
//para el sig metodo ocupo algo para almacenar los nodos ocupados
//al recibir una solicitud, la escribo en este metodo
void Naranja::ocuparNodoGrafo(){
  //Desencola de lista de solicitudes
  //Asuma que el paquete es rechazado
  //Si el grafo está incompleto
    //Se acepta solicitud
    //Asigna en mapa de bits
    //Envia paquete a los naranjas por el anillo
    //Asigna variables para paquete
  //Si mi paquete fue aceptado
    //Si tengo ips de Vecinos
      //Envie paquete 16(Posición+Vecinos)
    //else
      //Envie Paquete 15 (Posición)
  //else
    //envie rechazo

}

/*
Efecto: Envia paquete indicando completitud del grafo. Numero paquete = 17
Requiere: Lista de nodos azules del naranja
Modifica: Los nodos azules se activan
*/
void Naranja::enviarCompleteAzules(){
  //Para cada nodo azul de mi lista
    //envio paquete de complete.
}
