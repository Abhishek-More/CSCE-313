#makefile

all: client

1PNC.o: 1PNC.cpp
	g++ -g -w -std=c++11 -c 1PNC.cpp

client: 1PNC.cpp 
	g++ -g -w -std=c++11 -o client 1PNC.cpp -lpthread

clean:
	rm -rf *.o fifo* server client 
