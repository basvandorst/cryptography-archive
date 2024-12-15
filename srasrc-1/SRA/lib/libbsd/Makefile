LIB=	bsd
CFLAGS= -DLIBC_SCCS  -I../../include
OBJS=	getenv.o herror.o setenv.o strerror.o getpass.o

lib${LIB}.a: ${OBJS}
	echo building standard ${LIB} library 
	rm -f lib${LIB}.a 
	if [ -f /usr/bin/lorder ]; then \
		ar cq lib${LIB}.a `lorder ${OBJS} | tsort`  ; \
	else \
		ar cq lib${LIB}.a ${OBJS}; \
	fi;
	if [ -f /usr/bin/ranlib ]; then \
		ranlib lib${LIB}.a; \
	fi;

clean:
	rm -f *.o lib${LIB}.a
