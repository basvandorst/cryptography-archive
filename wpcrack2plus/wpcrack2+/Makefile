
CFLAGS = -O2

all:	wpcrack.o
	$(CC) $(CFLAGS) wpcrack.o -o wpcrack

.c.o:	
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	$(RM) wpcrack *.o

