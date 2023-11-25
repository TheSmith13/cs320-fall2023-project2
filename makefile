CC = g++
CFLAGS = -std=c++11

all: cache

cache: cache-sim.cpp
	$(CC) $(CFLAGS) cache-sim.cpp -o cache-sim
	
clean:
	rm -f cache-sim
	
