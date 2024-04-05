all: client.exe server.exe address.exe

C: client.exe

client.exe: client.o CS_TCP.o
	gcc -o client.exe client.o CS_TCP.o

client.o: client.c CS_TCP.h
	gcc -c client.c

S: server.exe

server.exe: server.o CS_TCP.o
	gcc -o server.exe server.o CS_TCP.o

server.o: server.c CS_TCP.h
	gcc -c server.c

A: address.exe

address.exe: getaddr.o CS_TCP.o
	gcc -o address.exe getaddr.o CS_TCP.o

getaddr.o: getaddr.c CS_TCP.h
	gcc -c getaddr.c
	
CS_TCP.o: CS_TCP.c CS_TCP.h
	gcc -c CS_TCP.c

F: file-demo.exe

file-demo.exe: file-demo.c
	gcc -o file-demo.exe file-demo.c

D: string-demo.exe

string-demo.exe: string-demo.c
	gcc -o string-demo.exe string-demo.c
	
clean:
	rm *.o *.exe
