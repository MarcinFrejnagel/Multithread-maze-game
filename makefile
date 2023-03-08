CC=gcc
CFLAGS= -pthread -lncurses

all: server client

server: server_main.o server_functions.o
    $(CC) $(CFLAGS) $^ -o $@

client: client_main.o client_functions.o
    $(CC) $(CFLAGS) $^ -o $@

server_main.o: server_main.c server_functions.h
    $(CC) $(CFLAGS) -c $< -o $@

server_functions.o: server_functions.c server_functions.h
    $(CC) $(CFLAGS) -c $< -o $@

client_main.o: client_main.c client_functions.h
    $(CC) $(CFLAGS) -c $< -o $@

client_functions.o: client_functions.c client_functions.h
    $(CC) $(CFLAGS) -c $< -o $@

clean:
    rm -f server client *.o
