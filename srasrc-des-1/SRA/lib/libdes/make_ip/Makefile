PROG=	make_ip
OBJS=	make_ip.o ../make_key_perm/misc.o
CFLAGS= -DKERBEROS  -I../../kerberosIV/include -I../des

$(PROG):	$(OBJS)
	$(CC) -o $(PROG) $(CFLAGS) $(OBJS)

clean:
	rm -f *.o $(PROG)
