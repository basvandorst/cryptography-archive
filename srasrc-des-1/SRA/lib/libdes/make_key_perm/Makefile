PROG=	make_key_perm
OBJS=	make_key_perm.o misc.o
CFLAGS= -DKERBEROS -I../../kerberosIV/include -I../des

$(PROG):	$(OBJS)
	$(CC) -o $(PROG) $(CFLAGS) $(OBJS)

clean:
	rm -f *.o $(PROG)
