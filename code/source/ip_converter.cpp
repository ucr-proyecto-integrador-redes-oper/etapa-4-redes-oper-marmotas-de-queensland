#include "ip_converter.h"

void IPConverter::printIP(const IpAddrType *addr){
    printf("int=%u\n", addr->intValue);
    printf("ipRecibida=%d.%d.%d.%d\n",addr->arrayIP[3], addr->arrayIP[2], addr->arrayIP[1], addr->arrayIP[0]);
    printf("ipInvertida=%d.%d.%d.%d\n",addr->arrayIP[0], addr->arrayIP[1], addr->arrayIP[2], addr->arrayIP[3]);
}

void IPConverter::cargarIPInvertido(const char *s, IpAddrType *addr){
    int i, d[4];
    sscanf(s, "%d.%d.%d.%d", &d[0], &d[1], &d[2], &d[3]);
    for(i=0; i<4; i++) { addr->arrayIP[i] = d[i]; }
}

void IPConverter::cargarIP(const char *s, IpAddrType *addr){
    int i, d[4];
    sscanf(s, "%d.%d.%d.%d", &d[3], &d[2], &d[1], &d[0]);
    for(i=0; i<4; i++) { addr->arrayIP[i] = d[i]; }
}
