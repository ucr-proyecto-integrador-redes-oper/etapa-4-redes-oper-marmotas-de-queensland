#include "orange.h"

/*
* @brief Constructor de la clase Naranja
* @param portNaranja: puerto de comunicacion con otros naranjas.
* @param cantidadAzules: cantidad total de azules que recibira el naranja.
* @param portAzul: puerto de comunicacion con los azules.
* @param pathcsv: nombre o ruta del csv con el grafo.
* @param ipDer: ip del nodo naranja derecha.
* @param portDer: puerto del nodo naranja derecha.
* @return None.
*/
Naranja::Naranja(int portNaranja,int cantidadAzules,short portAzul,string pathcsv,char* ipDer,short portDer){
  //Iniciar socket para naranjas
  udpNaranjas = new UDP(portNaranja);
  //Iniciar socket para azules
  sudpAzules = new SecureUDP(portAzul,WAIT_SUDP);

  this->cantidadAzules = cantidadAzules;
  this->continuar = true;
  //bandera para el token,0 es que no lo tengo 1 si ya Llego
  //se utiliza para el timer.
  this->banderaToken = 0;
  this->cantidadCompletos = 0;
  this->token = 0;

  this->ipDer = ipDer;
  this->portDer = portDer;
  //cout << "ip Derecha: " << this->ipDer << " puerto: " << portDer << endl;

  //se pregunta por la ip de mi pc///////////////////////////////
  string localIp;
  cout << "Ingrese la Ip local : " << endl;
  cin >> localIp;

  this->miIp = inet_addr(localIp.c_str());//pasar de string a int
  // cout << "Int de la Ip: "<< localIp <<" es: " << this->miIp << endl;
  //////////////////////////////////////////////////////////////
  //cantidad de nodos en el grafo, para luego iniciar los bitmap
  limpiarArchivoDatosAzul();
  this->pathcsv = pathcsv ;
  count_Lines(pathcsv);

  grafo = new BitMap(line_count); //CAMBIAR LUEGO: inicializar con n lineas CSV
  misAzules = new BitMap(line_count); //CAMBIAR LUEGO: inicializar con n lineas CSV
}

/*
* @brief Destructor del Naranja.
* @param None.
* @return None.
*/
Naranja::~Naranja(){
  delete udpNaranjas;
  delete sudpAzules;
  delete grafo;
  delete misAzules;
}

//funcionamiento interno:
/*
* @brief Metodo que cuenta la cantidad de nodos del grafo, utiliza el csv.
* @param path: Csv del grafo del cual cuenta los nodos.
* @return line_count: Cantidad de nodos en el grafo.
*/
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

/*
* @brief Inicia el funcionamiento inicial del naranja.
* @param None.
* @return None.
*/
void Naranja::iniciar(){
  this->enviarInicial();

  if(verificarNaranjas()) //si soy el mas bajo
    crearToken();

  verificarPaquetes();
}

//este metodo debe ser ejecutado con un thread para verificar el timeout
/*
* @brief Metodo de timer, se utiliza para esperar por el token en caso de que este se pierda.
* @param None.
* @return int 0.
*/
int Naranja::timeout(){
  std::unique_lock<std::mutex> mutex(m);
  if(cv.wait_for(mutex, 60s) == std::cv_status::timeout){
    cout << "Timeout: No se recibio el token."<< endl << "Reenviando token" << endl;
    enviarTokenVacio();
  }
}

/*
* @brief Verifica si mi ip es la menor entre todos los naranjas.
* @param None.
* @return int menor: 1 si mi ip es la menor, 0 si no es la menor.
*/
int Naranja::verificarNaranjas(){
  int menor = 1;
  for(int i = 0 ; i < ipsVecinos.size() && menor == 1 ;++i)
  if((unsigned int)ipsVecinos[i] < (unsigned int)miIp)
  menor = 0;

  return menor;
}


/*
* @brief guarda en la cola de solicitudes las solicitudes de los azules.
* @param nuevaSolicitud: solicitud del azul.
* @return None.
*/
void Naranja::guardarSolicitud(pack_solicitud nuevaSolicitud){
  //guardarlo en la cola
  solicitudes.push(nuevaSolicitud);
}

/*
* @brief crea el token, modificando el atributo token.
* @param None.
* @return None.
*/
void Naranja::crearToken(){
  cout << "Creando Token" << endl;
  this->token = 1;
  this->banderaToken = 1;
}

/*
* @brief Contiene un switch para analizar los diferentes paquetes.
* @param None.
* @return None.
*/
void Naranja::verificarPaquetes(){
  cout << "Recibiendo paquetes" << endl;
  char* ipAzulcorrecto;
  int id = 5;
  if(this->token)
    id = 4;

  //recibir paquetes
  while(this->continuar){
    //recibir paquete, se recibe con el tipo de paquete mas grande posible
    if(banderaToken==0){ // si no tengo el token
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
      in_addr ipEnInt;
      ipEnInt.s_addr = solicitud.ipAzul;
      ipAzulcorrecto = inet_ntoa(ipEnInt);
      this->agregarIPPuertoNodoAzul(solicitud.idNodo,ipAzulcorrecto,solicitud.portAzul);
      this->marcarNodoGrafo(solicitud.id);
      //enviar solicitud al siguiente
      this->udpNaranjas->sendTo((char*)&solicitud,sizeof(solicitud),ipDer,portDer);
      break;

      case 2://recibo complete
      cout << "Un servidor naranja completo sus azules." << endl;
      ++cantidadCompletos;
      //enviar al nodo derecho el complete
      this->enviarComplete();

      if(this->cantidadCompletos == 6)//si ya estan los complete
      this->enviarCompleteAzules();//avisar a los azules

      break;

      case 3://recibo token vacio
      cout << "Se recibio token vacio" << endl;

      cv.notify_one();//avisa al thread que recibio token

      this->banderaToken = 1;
      if(this->solicitudes.empty()){//si no hay solicitudes
        cout << "No hay solicitudes de nodo azul, enviando token vacio." << endl;
        this->enviarTokenVacio();//envie token vacio
        break;//se hace el break dentro del if para que pase al case 4
        //en caso de que hayan solicitudes
      }

      this->token = 1;

      case 4://cree el token y tengo solicitudes
      if(!this->solicitudes.empty()){
        cout << "Atendiendo solicitud de nodo azul." << endl;
        //atender una solicitud
        this->ocuparNodoGrafo();
        //enviarSolicitud(solicitud);
      }
      cout << "Enviando token vacio." << endl;
      this->enviarTokenVacio();//envie token vacio luego de realizar una solicitud


      break;
      default:
      cout << "Caso erroneo." << endl;
      break;
    }
  }
}

//paquetes:
//arreglar bug
//con el de Roger , el debe enviar primero
//con el de lucho yo debo enviar primero
/*
* @brief Envia el paquete inicial, recibe paquetes iniciales de otros naranjas.
* Cada inicial de otro naranja se almacena en el vector para luego ser comparados.
* @param None.
* @return None.
*/
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

/*
* @brief Envia solicitud a los otros naranjas.
* @param solicitud: la solicitud que se va a enviar.
* @return None.
*/
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

/*
* @brief Envia el token vacio a los otros naranjas.
* Ademas crea un thread para el timer, solamente si el naranja creo el token.
* @param None.
* @return None.
*/
void Naranja::enviarTokenVacio(){
  //enviar vacio
  vacio.id = 3;
  this->udpNaranjas->sendTo((char*)&vacio,sizeof(vacio),ipDer,portDer);
  this->banderaToken = 0;
  if(this->token ){
    thread (&Naranja::timeout,this).detach();//thread para el timer token
  }
}

/*
* @brief Envia un paquete de que ya el naranja completo sus azules.
* @param None.
* @return None.
*/
void Naranja::enviarComplete(){
  //enviar complete
  pack_complete complete;
  complete.id = 2;
  this->udpNaranjas->sendTo((char*)&complete,sizeof(pack_complete),ipDer,portDer);
}

//Cosas de azules:
/*
* @brief Obtiene los vecinos de un nodo del grafo a partir del csv.
* @param nodo: id del nodo a buscar en el csv.
* @param path: csv de donde buscara los vecinos del nodo.
* @return vector<int> vecinos_nodo: vector que contiene los vecinos.
*/
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
* @brief Recibe un mensaje de un azul por Secure udp y lo encola.
* @param None.
* @return None.
*/
void Naranja::recibirSolicitudAzul(){
  char buffer[7];
  pair<char*,uint16_t> info_client; //IP y puerto de azul solicitante
  info_client = this->sudpAzules->receive(buffer);
  if(!grafoCompleto && (strcmp(buffer, "14") == 0)) // Si el grafo esta incompleto y es una solicitud f_join_graph
  {
    //Se crea paquete
    pack_solicitud nuevaSolicitud;
    nuevaSolicitud.id = 1;
    ip_converter.cargarIP(info_client.first, &addrInvertido); //Funcion que corrige endianness de direccion de nodo
    //ip_converter.printIP(&addrInvertido);
    nuevaSolicitud.ipAzul = addrInvertido.intValue;
    nuevaSolicitud.portAzul = info_client.second;
    // Se encola paquete
    guardarSolicitud(nuevaSolicitud); //Se encola solicitud
  }
}

/*
* @brief Envia al azul su posición en el grafo. Cuando sus vecinos no necesariamente estan instanciados.
* Numero paquete = 15
* @param vector<int> vecinos_nodo: vector con los vecinos del nodo.
* @param ipEnvio: ip del nodo azul.
* @param puertoEnvio: puerto del nodo azul.
* @return None.
*/
void Naranja::enviarPosicion(vector<int> vecinos_nodo,char* ipEnvio, uint16_t puertoEnvio){
  int i = 1;
  while(vecinos_nodo[i] != -1){
    f_graph_pos pos_nodo;
    pos_nodo.node_id = (uint16_t)vecinos_nodo[0];
    pos_nodo.neighbor_id = (uint16_t)vecinos_nodo[i];
    //enviar paquete
    this->sudpAzules->sendTo((char*)&pos_nodo, sizeof(pos_nodo),ipEnvio,puertoEnvio);
    i++;
  }
}

/*
* @brief Envia al azul su posición en grafo y una lista de vecinos y sus direcciones y puertos
* si estan instanciados. Numero paquete = 16
* @param vector<int> vecinos_nodo: vector con los vecinos del nodo.
* @param ipEnvio: ip del nodo azul.
* @param puertoEnvio: puerto del nodo azul.
* @return None.
*/

void Naranja::enviarPosConVecino(vector<int> vecinos_nodo,char* ipEnvio, uint16_t puertoEnvio){
  IPConverter ip_converter;
  int i = 1;
  while(vecinos_nodo[i] != -1){
    if(misAzules->Test(vecinos_nodo[i])){ //vecino esta instanciado
      f_graph_pos_i pos_nodo_vecino;
      pos_nodo_vecino.node_id = (uint16_t)vecinos_nodo[0];
      pos_nodo_vecino.neighbor_id = (uint16_t)vecinos_nodo[i];
      pair<char*,uint16_t> infoAzul = obtenerIPPuertoNodoAzul(vecinos_nodo[i]);
      ip_converter.cargarIP(infoAzul.first, &addrInvertido);
      pos_nodo_vecino.neighbor_ip = addrInvertido.intValue;
      pos_nodo_vecino.neighbor_port = infoAzul.second;
      //enviar paquete
      this->sudpAzules->sendTo((char*)&pos_nodo_vecino, sizeof(pos_nodo_vecino),ipEnvio,puertoEnvio);
      delete infoAzul.first;
    }
    else{ // vecino no esta instanciado
      f_graph_pos pos_nodo;
      pos_nodo.node_id = (uint16_t)vecinos_nodo[0];
      pos_nodo.neighbor_id = (uint16_t)vecinos_nodo[i];
      //enviar
      this->sudpAzules->sendTo((char*)&pos_nodo, sizeof(pos_nodo),ipEnvio,puertoEnvio);
    }
    i++;
  }
}
/*
* @brief Marca el nodo ocupado en bitmap grafo.
* @param id: recibido o asigado previamente.
* @return None.
*/
void Naranja::marcarNodoGrafo(int id){
  //Asigna en mapa de bits
  grafo->Mark(id);
  //Se acepta solicitud
}
/*
* @brief Asigna un número en el grafo a un nodo azul.
* @param None.
* @return None.
*/
//para el sig metodo ocupo algo para almacenar los nodos ocupados
//al recibir una solicitud, la escribo en este metodo
void Naranja::ocuparNodoGrafo(){
  //Desencola de lista de solicitudes
  pack_solicitud nuevaSolicitud = solicitudes.front();
  solicitudes.pop();
  //Se obtiene el ip a partir del uint32_t

  in_addr ipEnInt;
  ipEnInt.s_addr = nuevaSolicitud.ipAzul;
  char* ipAzul = inet_ntoa(ipEnInt);
  //Asuma que el paquete es rechazado
  bool aceptado = false;
  //Si el grafo está incompleto
  if(!grafoCompleto){
    //Asigna en mapa de bits
    int id = grafo->Find();
    marcarNodoGrafo(id);
    misAzules->Mark(id);
    //Se acepta solicitud
    nuevaSolicitud.idNodo = id;
    this->agregarIPPuertoNodoAzul(nuevaSolicitud.idNodo,ipAzul,nuevaSolicitud.portAzul);
    //Envia paquete a los naranjas por el anillo
    enviarSolicitud(nuevaSolicitud);
  }
  //Si mi paquete fue aceptado
  if(aceptado){
    vector <int> vecinos = obtener_vecinos(nuevaSolicitud.idNodo, pathcsv);
    //Si tengo ips de Vecinos
    if(vecinos[1] == -1){//Si no tiene vecinos
      enviarPosicion(vecinos,ipAzul,nuevaSolicitud.portAzul);
    }
    else{//Si tiene vecinos
      //Crear paquete y enviar
      enviarPosConVecino(vecinos,ipAzul,nuevaSolicitud.portAzul);
    }
  }
  else{
    //Devuelve solicitud a cola
    solicitudes.push(nuevaSolicitud);
  }
  delete ipAzul;
}

/*
* @brief Envia paquete indicando completitud del grafo. Numero paquete = 17
* @param None.
* @return None.
*/
void Naranja::enviarCompleteAzules(){
  if(grafoCompleto){
    //Para cada nodo azul de mi lista
    f_graph_complete grafoCompleto;
    grafoCompleto.type = 17;
    for(int i = 0; i<this->misAzules->numBits;i++)
    {
      if(misAzules->Test(i))
      {
        pair<char*,uint16_t> infoAzul = obtenerIPPuertoNodoAzul(i);
        //Envio paquete de complete.
        this->sudpAzules->sendTo((char*)& grafoCompleto, sizeof(grafoCompleto),infoAzul.first,infoAzul.second);
        delete infoAzul.first;
      }
    }
  }
}

///Funciones para guardar datos de los azules
/*
* @brief Limpia el archivo listaIPAzules.csv o lo crea si no existe
* @param None.
* @return None.
*/
void Naranja::limpiarArchivoDatosAzul()
{
  ofstream myfile;
  myfile.open ("listaIPSAzules.csv");
  myfile<<"";
  myfile.close();
}
/*
* @brief Apende una nueva linea al archivo de listaIPSAzules,
* Agrega una nueva linea al archivo con las ips y puertos.
* @param id: Una id única para nodo.
* @param ip: ip del nodo.
* @param puerto: puerto del nodo.
*/
void Naranja::agregarIPPuertoNodoAzul(int id, char* ip, uint16_t puerto)
{
  ofstream myfile;
  myfile.open ("listaIPSAzules.csv",std::fstream::app);
  myfile <<id<<","<<ip<<","<<puerto<<"\n";
  myfile.close();
}
/*
* @brief Obtiene la ip de un nodo agregado a la listaIPSAzules
* @param id pasada por parámetro exista en el archivo y eliminar el puntero con delete al dejar de usarse afuera.
* @return None.
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
  //Abre el archivo con la listas de ip
  if(!myfile.is_open())
  {
    cout<<"ERROR AL ABRIR LISTA IPS"<<endl;
  }
  else
  {
    //Itera por el archivo buscando el nodo y lo devuelve
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
