CC=gcc 
all: server deliver remove_intermediate

server: server.o 
deliver: deliver.o 
remove_intermediate: 	
	rm -f *.o 
clean:
	rm -f *.o server deliver