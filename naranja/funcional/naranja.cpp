#include "naranja.h"
Naranja::Naranja(int portNaranja,short portAzul,string pathcsv,char* ipDer,short portDer){

  //Iniciar socket para naranjas
  udpNaranjas = new UDP(portNaranja);
  //Iniciar socket para azules
  sudpAzules = new SecureUDP(portAzul,WAIT_SUDP);

  this->banderaSolicitud = 0;
  this->cantidadCompletos = 0;
  this->token = 0;

  this->ipDer = ipDer;
  this->portDer = portDer;
  //cout << "ip Derecha: " << this->ipDer << " puerto: " << portDer << endl;

  //se pregunta por la ip de mi pc///////////////////////////////
  string localIp;
  cout << "Ingrese la Ip local invertida : " << endl;
  cin >> localIp;

  grafo = new BitMap(15); //CAMBIAR LUEGO: inicializar con n lineas CSV
  misAzules = new BitMap(15); //CAMBIAR LUEGO: inicializar con n lineas CSV

  this->miIp = inet_addr(localIp.c_str());//pasar de string a int
  // cout << "Int de la Ip: "<< localIp <<" es: " << this->miIp << endl;
  //////////////////////////////////////////////////////////////
  //cantidad de nodos en el grafo, para luego iniciar los bitmap
  limpiarArchivoDatosAzul();
  this->pathcsv = pathcsv ;
  count_Lines(pathcsv);
}

Naranja::~Naranja(){
  delete udpNaranjas;
  delete sudpAzules;
  delete grafo;
  delete misAzules;
}

//funcionamiento interno:
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
    if((unsigned int)ipsVecinos[i] < (unsigned int)miIp)
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
  bool continuar = true;
  while(continuar){
    //recibir paquete, se recibe con el tipo de paquete mas grande posible
    if(token==0){ // si no tengo el token
    	cout << "Esperando por paquete" << endl;
    	this->udpNaranjas->receive((char*)&solicitud,sizeof(solicitud));
    	//se saca el id del paquete
    	memcpy((char*)&id,(char*)&solicitud.id,sizeof(char));
	cout << "id: " << id<<endl;
    }else{//si tengo el token
 	    id = 3;
    }
    switch (id) {
      case 0:
        break;
      case 1://recibo solicitud
        cout << "Se recibio una solicitud de otro nodo naranja." << endl;
      	cout << solicitud.id << endl;
      	cout << solicitud.idNodo << endl;
      	cout << solicitud.ipAzul << endl;
      	cout << solicitud.portAzul << endl;
        //guardar solicitud recibida
        //agregarIPPuertoNodoAzul(solicitud.idNodo,(char*)solicitud.ipAzul,portAzul);
        ////marcarNodoGrafo(solicitud.id);
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
          enviarSolicitud(solicitud);
        }
        cout << "Enviando token vacio." << endl;
        this->enviarTokenVacio();//envie token vacio luego de realizar una solicitud

        break;
      default:
        continuar = false;
        cout << "Terminando programa" << endl;
        break;
    }
  }
}

//paquetes:
//arreglar bug
//con el de Roger , el debe enviar primero
//con el de lucho yo debo enviar primero
void Naranja::enviarInicial(){
  //enviar inicial
  char id = 0;
  inicial.ip = this->miIp;
  inicial.id = 0;
  //enviar por der
  cout << "Enviando: " << inicial.ip << " id: " << inicial.id << " a derecha: "<<  portDer <<endl;
  this->udpNaranjas->sendTo((char*)&inicial,sizeof(inicial),ipDer,portDer);

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
    if(!miIp){
	    inicial.id= 0;
	    cout << "Enviando id: " <<inicial.id <<" ip: "<<inicial.ip << endl;
    	this->udpNaranjas->sendTo((char*)&inicial,sizeof(inicial),ipDer,portDer);
    }
    yaEsta = false;
    miIp = false;
  }
}

void Naranja::enviarSolicitud(pack_solicitud solicitud){
  //enviar solicitud
  //copia la solicitud en struct
  //memcpy((char*)&solicitud,(char*)&solicitudes.front(),sizeof(solicitud));
  //solicitud.id = 1; pasar a memcpy
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
  pack_complete complete;
  complete.id = 2;
  this->udpNaranjas->sendTo((char*)&complete,sizeof(pack_complete),ipDer,portDer);
}

//Cosas de azules:
vector<int> Naranja::obtener_vecinos(int nodo, string &path){
  vector <int> vecinos_nodo;
  ifstream csvFile;
  csvFile.open(path.c_str(), ifstream::in);

  if(!csvFile.is_open()){
    exit(EXIT_FAILURE);
  }
  char line[100];
  for(int i=0; i<100; i++){
    line[i] = -1;
  }
  while(csvFile.getline(line, 100, '\r') || csvFile.getline(line, 100, '\n')){
    if (line[0] == nodo){
      int j = 0;
      while(line[j] != -1){
        vecinos_nodo.push_back((int)line[j]);
        j++;
      }
    }
  }
  return vecinos_nodo;
}
/*
Efecto: Recibe un mensaje de un azul por Secure udp y lo encola
Requiere: Cola de solicitudes, socket abierto a solictudes
Modifica: Cola de solicitudes
*/
void Naranja::recibirSolicitudAzul(){
  char buffer[7];
  pair<char*,uint16_t> info_client;
  info_client = this->sudpAzules->receive(buffer);
  if(!grafoCompleto && (strcmp(buffer, "14") == 0))
  {
      //Se crea paquete
      pack_solicitud nuevaSolicitud;
      nuevaSolicitud.id = 1;
      ip_converter.cargarIP(info_client.first, &addrInvertido);
      //ip_converter.printIP(&addrInvertido);
      nuevaSolicitud.ipAzul = (uint32_t)addrInvertido.intValue;
      nuevaSolicitud.portAzul = info_client.second;
  		// Se encola paquete
  		guardarSolicitud(nuevaSolicitud);
  }
}

/*
Efecto: Envia al azul su posición en el grafo. Numero paquete = 15
Requiere: Número de nodo que se asignó
Modifica: El numero en grafo del azul al que se envía el paquete
*/
void Naranja::enviarPosicion(vector<int> vecinos_nodo,char* r_ip, uint16_t r_port){
  int i = 0;
  while(vecinos_nodo[i] != -1){
    f_graph_pos pos_nodo;
    pos_nodo.node_id = (uint16_t)vecinos_nodo[0];
    pos_nodo.neighbor_id = (uint16_t)vecinos_nodo[i];
    //enviar paquete
    this->sudpAzules->sendTo();
    i++;
  }
}

/*
Efecto: Envia al azul su posición en grafo y una lista de vecinos. Numero paquete = 16
Requiere: Número de nodo que se asignó, lista de vecinos
Modifica: El numero en grafo del azul al que se le envía el paquete y sus lista de vecinos
*/
void Naranja::enviarPosConVecino(vector<int> vecinos_nodo,char* r_ip, uint16_t r_port){
  int i = 0;
  while(vecinos_nodo[i] != -1){
    if(mapAzules.Test(vecinos_nodo[i])){ //vecino esta instanciado
      f_graph_pos_i pos_nodo_vecino;
      pos_nodo_vecino.node_id = (uint16_t)vecinos_nodo[0];
      pos_nodo_vecino.neighbor_id = (uint16_t)vecinos_nodo[i];
      pair<char*,uint16_t> infoAzul = obtenerIPPuertoNodoAzul(vecinos_nodo[i]);
      pos_nodo_vecino.neighbor_ip = infoAzul.first;
      pos_nodo_vecino.neighbor_port = infoAzul.second;
      //enviar paquete
      this->sudpAzules->sendTo(pos_nodo_vecino, sizeof(pos_nodo_vecino));
      delete infoAzul.first;
    }
    else{ // vecino no esta instanciado
      f_graph_pos pos_nodo;
      pos_nodo.node_id = (uint16_t)vecinos_nodo[0];
      pos_nodo.neighbor_id = (uint16_t)vecinos_nodo[i];
      //enviar
      this->sudpAzules->sendTo(pos_nodo, sizeof(pos_nodo),);
    }
    i++;
  }
}
/*
Efecto: Marca el nodo ocupado en bitmap grafo
Requiere: Un id recibido o asigado previamente
Modifica: El bitmap de grafo
 */
void Naranja::marcarNodoGrafo(int id){
    //Asigna en mapa de bits
   grafo->Mark(id);
    //Se acepta solicitud
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
  pack_solicitud nuevaSolicitud = solicitudes.front();
  solicitudes.pop();
  //Asuma que el paquete es rechazado
  nuevaSolicitud.idNodo = -1;
  //Si el grafo está incompleto
  if(!grafoCompleto){
    //Asigna en mapa de bits
    int id = grafo->Find();
    marcarNodoGrafo(id);
    misAzules->Mark(id);
    //Se acepta solicitud
    nuevaSolicitud.idNodo = id;
    //Envia paquete a los naranjas por el anillo
    enviarSolicitud(nuevaSolicitud); //CAMBIAR
  }
  //Si mi paquete fue aceptado
  if(nuevaSolicitud.idNodo > -1 ){
    vector <int> vecinos = obtener_vecinos(nuevaSolicitud.idNodo, pathcsv);
    //Si tengo ips de Vecinos
    if(vecinos[1] == -1){
      enviarPosicion(vecinos);
    }
    else{
      //While existan vecinos
	//Crear paquete y enviar
      enviarPosConVecino(vecinos);
    }
  }
  else{
    //Devuelve solicitud a cola
    solicitudes.push(nuevaSolicitud);
  }
}

/*
Efecto: Envia paquete indicando completitud del grafo. Numero paquete = 17
Requiere: Lista de nodos azules del naranja
Modifica: Los nodos azules se activan
*/
void Naranja::enviarCompleteAzules(){
  if(grafoCompleto){
  //Para cada nodo azul de mi lista
  pack_complete grafoCompleto;
    //envio paquete de complete.
  }
}

///Funciones para guardar datos de los azules
/*
Efecto: Limpia el archivo listaIPAzules.csv o lo crea si no existe
Requiere: Nada
Modifica: El directorio del programa se creará un archivo llamado listaIPSAzules en blanco
*/
void Naranja::limpiarArchivoDatosAzul()
{
  ofstream myfile;
  myfile.open ("listaIPSAzules.csv");
  myfile<<"";
  myfile.close();
}
/*
Efecto: Apende una nueva linea al archivo de listaIPSAzules
Requiere:Una id única para nodo, ip y puerto
Modifica: Agrega una nueva linea al archivo con las ips y puertos
*/
void Naranja::agregarIPPuertoNodoAzul(int id, char* ip, uint16_t puerto)
{
  ofstream myfile;
  myfile.open ("listaIPSAzules.csv",std::fstream::app);
  myfile <<id<<","<<ip<<","<<puerto<<"\n";
  myfile.close();
}
/*
Efecto: Obtiene la ip de un nodo agregado a la listaIPSAzules
Requiere: Que la id pasada por parámetro exista en el archivo y eliminar el puntero con delete al dejar de usarse afuera.
Modifica: Nada.
*/
pair<char*,uint16_t>  Naranja::obtenerIPPuertoNodoAzul(int id)
{
  bool found=false;
  std::pair<char*,uint16_t> nodoInfo;
  char* ipChar = new char[32];
  string idObtenida;
  string ip;
  string puerto;
  fstream myfile("listaIPSAzules.csv");
  if(!myfile.is_open())
  {
    cout<<"ERROR AL ABRIR LISTA IPS"<<endl;
  }
  else
  {
    while(myfile.good() && !found)
    {
      getline(myfile,idObtenida,',');
      getline(myfile,ip,',');
      getline(myfile,puerto,'\n');
      if(id==stoi(idObtenida))
      {
        found=true;
      }
    }
      if(found)
      {
        strcpy(ipChar, ip.c_str());
      }
  }
  myfile.close();
  return  make_pair(ipChar,stoi(puerto));
}
