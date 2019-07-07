#ifndef IP_CONVERTER_H_
#define IP_CONVERTER_H_

#include <string.h>
#include <iostream>

using namespace std;

typedef union{
    unsigned char arrayIP[4]; //Struc guarda en forma de red (al revés)
    uint32_t intValue; //El valor lo hace aunque esté guardado al revés correctamente
}IpAddrType;

class IPConverter{
    public:
    IPConverter() {};
    ~IPConverter() {};
    void printIP(const IpAddrType *addr);
    void cargarIPInvertido(const char *s, IpAddrType *addr);
    void cargarIP(const char *s, IpAddrType *addr);

};

#endif //IP_CONVERTER_H
