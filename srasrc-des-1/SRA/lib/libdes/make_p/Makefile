PROG=	make_p
OBJS=	make_p.o ../make_key_perm/misc.o
CFLAGS= -DKERBEROS -I../../kerberosIV/include -I../des

$(PROG):	$(OBJS)
	$(CC) -o $(PROG) $(CFLAGS) $(OBJS)

clean:
	rm -f *.o $(PROG)
