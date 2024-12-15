CC = g++
COPTS =  -DTEST -O2 -DUSE_LOCAL_BUFFER_DEF -DUSE_LOCAL_ERRORS
bf: bf.c bf.h bfdat.h
	$(CC) $(COPTS) -o bf bf.c
	strip bf
