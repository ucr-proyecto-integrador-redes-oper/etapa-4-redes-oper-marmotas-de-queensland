#ifndef ORANGE_H_
#define ORANGE_H_

#include <vector>
#include <queue>
#include <iterator>
#include <map>
#include <thread>
#include <utility> //pares
#include <stdio.h>
#include <netdb.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include <fstream>
#include <thread>
#include <chrono>
#include <unistd.h>

#include <mutex>
#include <condition_variable>

//////////
#include "orange_udp.h"
#include "bitmap.h"
#include "sudp.h" //CAMBIAR AL UNIR TODO
#include "frames.h"//CAMBIAR AL UNIR TODO
#include "orange_frames.h"
#include "ip_converter.h"
////////////
#define WAIT_SUDP 2000

using namespace std;

class Naranja{
//https://www.lucidchart.com/documents/edit/bc90a84e-13b2-4c16-8461-54da604c8015/0

private:
	vector<uint32_t> ipsVecinos; //vector que contiene las ips de los demas naranjas
	int banderaToken; //bandera de cuando se envia el token de solicitud, para que espere por dicho token
	int cantidadCompletos; //cuando llega a la cantidad de naranjas(6), manda mensajes a los azules.
	int token; //0 que no tengo el token, 1 que el token me pertenece.

	//vecino der
	char* ipDer;
	short portDer;

	//cantidad de naranjas en el grafo.
	int cantidadNaranjas;
	bool banderaCompleteAzules;
	bool continuar;
	bool banderaComplete;

	std::mutex m;
  std::condition_variable cv;

	uint32_t miIp;

	//ports para naranjas y azules
	UDP* udpNaranjas;
	SecureUDP* sudpAzules;
	
	//paquetes
	struct pack_inicial inicial;
	struct pack_solicitud solicitud;
	struct pack_complete complete;
	struct pack_vacio vacio;

	//azules
	queue<pack_solicitud> solicitudes;//cola de solicitudes de los azules.
	IPConverter ip_converter;
	IpAddrType addrInvertido;
	//file csv
	string pathcsv;
	int line_count;
	int count_Lines(string &path);
	vector<int> obtener_vecinos(int nodo, string &path);

	//bitmap para todos los azules (de todos los naranjas) que se estan ocupando
//revisar en le bitmap los vecinos del nodo que estoy pidiendo, por si ya estan siendo ocupados
	//enviarles la ip y port al nuevo azul
	BitMap* grafo;
	//bitmap para saber cuales son mis azules.
	BitMap* misAzules;
	//mapita (nodoAzul,(ip, port))
	bool grafoCompleto;
	map<uint16_t,pair <uint32_t, uint16_t>> mapAzules;

public:
	Naranja(int,int,short,string,char*,short);
	~Naranja();

	//funcionamiento interno:
	void iniciar(); //inicia le funcionamiento del naranja
	int timeout();//este metodo debe ser ejecutado con un thread para verificar el timeout
	int verificarNaranjas(); //verifica si mi ip es la mas baja y retorna un valor. 0 si no, 1 si si
	void guardarSolicitud(pack_solicitud); //guarda en la cola de solicitudes las solicitudes de los azules.
	void crearToken(); //crea el token, modificando el atributo token
	void verificarPaquetes(); //contiene switch con los diferentes id de packs

	//paquetes:
	void enviarInicial(); //envia un paquete inicial para saber la ip
	void enviarSolicitud(pack_solicitud solicitud); //envia solicitud al vecino naranja derecho
	void enviarTokenVacio(); //envia un paquete indicando que el token esta libre
	void enviarComplete();//envia pack indicando complete

	//Cosas Azules
	void recibirSolicitudAzul(); // Recibe solicitud de un azul para unirse 14
	void enviarPosicion(vector<int> vecinos_nodo,char* ipEnvio, uint16_t puertoEnvio);//Envia al azul su posición en el grafo 15
	void enviarPosConVecino(vector<int> vecinos_nodo,char* ipEnvio, uint16_t puertoEnvio);//Envia al azul su posición en grafo y una lista de vecinos 16
	//para el sig metodo ocupo algo para almacenar los nodos ocupados
	void marcarNodoGrafo(int id);//Marca en el bitmap el nodo id
	void ocuparNodoGrafo(); //al recibir una solicitud, la escribo en este metodo
	void enviarAsignado();
	int enviarCompleteAzules();//envia un pack a los azules para que empiecen 17
	//Metodos para almacenar y obtener datos azules:
	void limpiarArchivoDatosAzul() ;//Crea/limpia un archivo con datos del azules
	void agregarIPPuertoNodoAzul(int id, char* ip, uint16_t puerto); //Agrega la IP a un archivo
	pair<char*,uint16_t>  obtenerIPPuertoNodoAzul(int id);// Retorna un par con la ip e int con ip-puerto. Se debe hacer delete al .first del par que recibe.



};
#endif
