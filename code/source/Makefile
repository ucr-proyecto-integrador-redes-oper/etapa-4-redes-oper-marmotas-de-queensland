#To generate another binary add it to the binaries variable/list and write the associated target.
#Use make clean before pushing to the repository.

CC = g++
binaries = test-sudp blue-node blue-controller orange-node

b_node_flags = -pthread -std=c++17 -I ../headers -g -o
b_controller_flags = -std=c++17 -I ../headers -g -o
o_node_flags = -pthread -std=c++17 -I ../headers -g -o
sudp_test_flags = -pthread -std=c++17 -I ../headers -g -o

all: $(binaries)

#Blue nodes target
blue-node:	blue.cpp blue_instantiator.cpp sudp.cpp
	$(CC) $(b_node_flags) blue-node blue.cpp blue_instantiator.cpp sudp.cpp

#Blue controller target
blue-controller: blue_controller.cpp
	$(CC) $(b_controller_flags) blue-controller  blue_controller.cpp

#Secure udp tester target
test-sudp: sudp.cpp sudp_tester.cpp
	$(CC) $(sudp_test_flags) test-sudp sudp.cpp sudp_tester.cpp

#Orange nodes target
orange-node: orange.cpp orange_instantiator.cpp bitmap.cpp ip_converter.cpp orange_udp.cpp sudp.cpp
	$(CC)	$(o_node_flags) orange-node orange.cpp orange_instantiator.cpp bitmap.cpp ip_converter.cpp orange_udp.cpp sudp.cpp

clean:
	rm -v $(binaries) ../data/*
