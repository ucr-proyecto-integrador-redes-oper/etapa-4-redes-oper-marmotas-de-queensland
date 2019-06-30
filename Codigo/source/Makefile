#To generate another binary add it to the binaries variable/list and write the associated target.
#Use make clean before pushing to the repository.

CC = g++
CFLAGS = -lpthread -std=c++17 -I ../headers -g -o
binaries = testSudp blueNode

all: $(binaries)

#Blue nodes target
blueNode:	blue.cpp blue_instantiator.cpp sudp.cpp
	$(CC) $(CFLAGS) blueNode blue.cpp blue_instantiator.cpp sudp.cpp

#Secure udp tester target
testSudp: sudp.cpp sudp_tester.cpp
	$(CC) $(CFLAGS) testSudp sudp.cpp sudp_tester.cpp

clean:
	rm $(binaries)
