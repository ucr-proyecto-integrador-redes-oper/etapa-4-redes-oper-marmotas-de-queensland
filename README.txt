CI-0123 Proyecto Integrador de Sistemas Operativos y Redes de Comunicación de Datos.
Grupo: Marmotas de Queensland.
Fase III.
Integrantes:
    Diana Arias Rojas
    Javier Padilla Avilés
    Julián Moya Zeledón
    Rodolfo Calvo Chacón


Para compilar:
  Irse a la carpeta code/source/ y correr el makefile


El ejecutable de los nodos naranja se llama orange-node, su main es orange_instantiator.cpp
Parámetros:

El ejecutable de los nodos azules se llama blue-node, su main es blue_instantiator.cpp
Parámetros:

El script openblues.sh permite instanciar n nodos azules.
Parámetros: Cantidad de nodos , IP del server, Puerto del server, IP del nodo siendo instanciado.
Este script también levanta un proceso blue-controller luego de levantar los n
nodos azules. Este proceso permite cerrar nodos azules individualmente o cerrarlos
todos de un solo.
