CC=gcc 
all: server client 
server: server.o link_list_impl.o 
client: client.o 
clean: 	
	rm -f *.o server deliver 

server.o: server.c link_list_impl.c message.h link_list_impl.h
	$(CC) -c server.c link_list_impl.c

client.o: client.c messages.h
	$(CC) -c client.c

link_list_impl.o: link_list_impl.c link_list_impl.h
	$(CC) -c link_list_impl.c
