#To generate another binary add it to the binaries variable/list and write the associated target.
#Use make clean before pushing to the repository.

CC = g++
CFLAGS = -lpthread -std=c++17 -I ../headers -g -o
binaries = test-sudp blue-node blue-controller

all: $(binaries)

#Blue nodes target
blue-node:	blue.cpp blue_instantiator.cpp sudp.cpp
	$(CC) $(CFLAGS) blue-node blue.cpp blue_instantiator.cpp sudp.cpp

#Blue controller target
blue-controller: blue_controller.cpp
	$(CC) $(CFLAGS) blue-controller  blue_controller.cpp

#Secure udp tester target
test-sudp: sudp.cpp sudp_tester.cpp
	$(CC) $(CFLAGS) test-sudp sudp.cpp sudp_tester.cpp

clean:
	rm $(binaries) ../data/*
