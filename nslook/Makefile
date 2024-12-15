CC = cc
CFLAGS = -O
# for most systems:
LIBS = -lresolv

# Interactive:
# LIBS = -linet

SOURCES = README nslook.1 nslook.c Makefile

nslook : nslook.c Makefile
	$(CC) $(CFLAGS) -o nslook nslook.c $(LIBS)
