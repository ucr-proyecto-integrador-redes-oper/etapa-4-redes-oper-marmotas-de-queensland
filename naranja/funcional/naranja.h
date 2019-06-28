#include <vector>
#include <queue>
#include <iterator>
#include <map>

#include <thread>
#include <utility> //pares
#include "paquetes.h"

#include <stdio.h>
#include <netdb.h>

#include <iostream>
#include <sstream>
#include <string.h>
#include <fstream>

//////////
#include "udp.h"
#include "bitmap.h"
////////////

using namespace std;
#ifndef NARANJA_H
#define NARANJA_H

class Naranja{
//https://www.lucidchart.com/documents/edit/bc90a84e-13b2-4c16-8461-54da604c8015/0

private:
	vector<uint32_t> ipsVecinos; //vector que contiene las ips de los demas naranjas
	int banderaSolicitud; //bandera de cuando se envia el token de solicitud, para que espere por dicho token
	int cantidadCompletos; //cuando llega a la cantidad de naranjas(6), manda mensajes a los azules.
	int token; //0 que no tengo el token, 1 que el token me pertenece.

	//vecino der
	char* ipDer;
	short portDer;
	//vecino izq
	char* ipIzq;
	short portIzq;

	uint32_t miIp;

	//ports para naranjas y azules
	UDP* udpNaranjas;
	UDP* udpAzules;

	//paquetes
	struct pack_inicial inicial;
	struct pack_solicitud solicitud;
	struct pack_complete complete;
	struct pack_vacio vacio;

	//azules
	queue<pack_solicitud> solicitudes;//cola de solicitudes de los azules.
	//file csv
	int line_count;
	int count_Lines(string &path);

	//bitmap para todos los azules (de todos los naranjas) que se estan ocupando
//revisar en le bitmap los vecinos del nodo que estoy pidiendo, por si ya estan siendo ocupados
	//enviarles la ip y port al nuevo azul
	BitMap* grafo;
	//bitmap para saber cuales son mis azules.
	BitMap* misAzules;
	//mapita (nodoAzul,(ip, port))
	map<uint16_t,pair <uint32_t, uint16_t>> mapAzules;

public:
	Naranja(int,short,string,char*,short,char*,short);
	~Naranja();

	//funcionamiento interno:
	void iniciar(); //inicia le funcionamiento del naranja
	int timeout();//este metodo debe ser ejecutado con un thread para verificar el timeout
	int verificarNaranjas(); //verifica si mi ip es la mas baja y retorna un valor. 0 si no, 1 si si
	void guardarSolicitud(pack_solicitud); //guarda en la cola de solicitudes las solicitudes de los azules.
	void crearToken(); //crea el token, modificando el atributo token
	void verificarPaquetes(); //contiene switch con los diferentes id de packs

	//para el sig metodo ocupo algo para almacenar los nodos ocupados
	void ocuparNodoGrafo(); //al recibir una solicitud, la escribo en este metodo

	//paquetes:
	void enviarInicial(); //envia un paquete inicial para saber la ip
	void enviarSolicitud(); //envia solicitud al vecino naranja derecho
	void enviarTokenVacio(); //envia un paquete indicando que el token esta libre
	void enviarComplete();//envia pack indicando complete
	void enviarCompleteAzules();//envia un pack a los azules para que empiecen
};


#endif //NARANJA_H
