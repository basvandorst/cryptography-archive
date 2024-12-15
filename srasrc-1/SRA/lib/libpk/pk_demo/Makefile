all:	demo_client demo_server

CFLAGS += -I.. -I../../libdes/des -L.. -L../../libdes/des -L../../gmp-1.2

demo_client:	demo_client.o pk_util.o
	$(CC) $(CFLAGS) -o demo_client demo_client.o pk_util.o -lpk -ldes -lmp

demo_server:		demo_server.o pk_util.o
	$(CC) $(CFLAGS) -o demo_server demo_server.o pk_util.o -lpk -ldes -lmp
