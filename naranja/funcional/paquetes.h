#ifndef PAQUETES_H
#define PAQUETES_H
/*
* Este paquete se manda en broadcast para que los demas naranjas conozcan mi IP.
* cuando ya conte 5 paquetes mas, se verifican las demas ip, de esta forma se sabe quien
* quien crea el token.
*/
struct pack_inicial{
	char id; //id 0
	uint32_t ip; //ip de los demas naranjas
};

/*
* Se envia cuando tengo el token, informa a los demas naranjas sobre una solicitud de un azul.
*
*/
struct pack_solicitud{
	char id; //id 1
	short idNodo; //id del nodo(grafo) que se solicita
	uint32_t ipAzul; //ip del nodo azul solicitante
	short portAzul; //port del nodo azul solicitante
};

/*
* Con solo el id, los demas naranjas se dan cuenta que un naranja termino.
*/
struct pack_complete{
	char id; //id 2
};

/*
* Pasa el token vacio a otro naranja.
*/
struct pack_vacio{
	char id; //id 3
};

#endif //PAQUETES_H
