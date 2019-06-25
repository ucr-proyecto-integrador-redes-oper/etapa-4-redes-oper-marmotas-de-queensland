#include <vector>
#include <queue>
#include <thread>
#include "paquetes.h"
#include "sudp.h"
#include <stdio.h>
#include <netdb.h>

#include <iostream>
#include <string.h>

//////////
#include "client.h"
#include "server.h"
////////////
using namespace std;
#ifndef NARANJA_H
#define NARANJA_H

class Naranja{
//https://www.lucidchart.com/documents/edit/bc90a84e-13b2-4c16-8461-54da604c8015/0

private:
	vector<uint32_t> ipsVecinos; //vector que contiene las ips de los demas naranjas
	queue<pack_solicitud> solicitudes;//cola de solicitudes de los azules.
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
	//falta 2 ports mios para naranjas y azules
	//SecureUDP* portAzul;
	//SecureUDP* portNaranjas;
	Server* server;
	Client* client;

	//paquetes
	struct pack_inicial inicial;
	struct pack_solicitud solicitud;
	struct pack_complete complete;
	struct pack_vacio vacio;

public:
	Naranja(int,short,char*,short,char*,short);
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

	void connectC(){
		client->connectClient("192.168.100.58",8080);
	}

	void connectS(){
		server->server_bind(8080);
		server->server_listen(1);
		server->server_accept();
	}
};


#endif //NARANJA_H
