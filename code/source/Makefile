#To generate another binary add it to the binaries variable/list and write the associated target.
#Use make clean before pushing to the repository.

CC = g++
binaries = blue-node orange-node

b_node_flags = -pthread -std=c++17 -I ../headers -g -o
b_controller_flags = -std=c++17 -I ../headers -g -o
o_node_flags = -pthread -std=c++17 -I ../headers -g -o
sudp_test_flags = -pthread -std=c++17 -I ../headers -g -o

all: $(binaries)

#Blue nodes target
blue-node:	azul.cpp azul_instantiator.cpp sudp.cpp
	$(CC) $(b_node_flags) blue-node azul.cpp azul_instantiator.cpp sudp.cpp

#Orange nodes target
orange-node: orange.cpp orange_instantiator.cpp bitmap.cpp ip_converter.cpp orange_udp.cpp sudp.cpp
	$(CC)	$(o_node_flags) orange-node orange.cpp orange_instantiator.cpp bitmap.cpp ip_converter.cpp orange_udp.cpp sudp.cpp

clean:
	rm -v $(binaries) ../data/*
