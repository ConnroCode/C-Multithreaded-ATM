CCFLAGS = -ggdb -Wall -lpthread

server: server.c 
	gcc $(CCFLAGS) server.c -o server
	
client: client.c
	gcc $(CCFLAGS) client.c -o client
	
clean:
	rm -rf server client 
