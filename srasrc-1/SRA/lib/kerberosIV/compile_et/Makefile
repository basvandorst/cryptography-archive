PROG=	compile_et
CFLAGS += -I. 
OBJS=	compile_et.o error_message.o et_name.o init_et.o perror.o
OBJS +=	error_table.o
LDADD=	-ll
CLEANFILES=et_lex.lex.c y.tab.c y.tab.h error_table.c

$(PROG):	$(OBJS)
	$(CC) -o $(PROG) $(CFLAGS) $(OBJS) $(LDADD)

error_table.c: et_lex.lex.c

clean:
	rm -f *.o $(CLEANFILES)
