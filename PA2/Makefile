all: shell

main.o : shell.cpp 
	g++ -c -g shell.cpp

shell: shell.o
	g++ -g -o shell shell.o

clean:
	rm *.o
