CC = g++
CFLAGS = -std=c++11

all: cache

cache: cache-sim.cpp Node.h node.cpp
	$(CC) $(CFLAGS) cache-sim.cpp node.cpp -o cache-sim
	
clean:
	rm -f cache-sim
	
