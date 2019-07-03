#ifndef PAQUETES_H
#define PAQUETES_H
/*
* Este paquete se manda en broadcast para que los demas naranjas conozcan mi IP.
* cuando ya conte 5 paquetes mas, se verifican las demas ip, de esta forma se sabe quien
* quien crea el token.
*/
struct pack_inicial{
	unsigned char id; //id 0
	uint32_t ip; //ip de los demas naranjas
}__attribute__((packed));

/*
* Se envia cuando tengo el token, informa a los demas naranjas sobre una solicitud de un azul.
*
*/
struct pack_solicitud{
	unsigned char id; //id 1
	uint16_t idNodo; //id del nodo(grafo) que se solicita
	uint32_t ipAzul; //ip del nodo azul solicitante
	uint16_t portAzul; //port del nodo azul solicitante
}__attribute__((packed));

/*
* Con solo el id, los demas naranjas se dan cuenta que un naranja termino.
*/
struct pack_complete{
	unsigned char id; //id 2
}__attribute__((packed));

/*
* Pasa el token vacio a otro naranja.
*/
struct pack_vacio{
	unsigned char id; //id 3
}__attribute__((packed));

#endif //PAQUETES_H
