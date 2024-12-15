CFLAGS=-g -I../include -L../lib $(FLAGS)
LIB=skip_user

autoconfig load unload install uninstall:	
	@cd ..; make $@; cd skipd

all:	skipd skip_addkey skip_stat skip_attach skip_detach
	

skipd:	skipd.o com.o ../lib/lib$(LIB).a
	$(CC) $(CFLAGS) -o $@ skipd.o com.o -l$(LIB)

skip_addkey:	skip_addkey.o com.o ../lib/lib$(LIB).a
	$(CC) $(CFLAGS) -o $@ skip_addkey.o com.o -l$(LIB)

skip_attach:	skip_attach.c com.o ../lib/lib$(LIB).a
	$(CC) $(CFLAGS) -o $@ skip_attach.c com.o -l$(LIB)

skip_detach:	skip_detach.c com.o ../lib/lib$(LIB).a
	$(CC) $(CFLAGS) -o $@ skip_detach.c com.o -l$(LIB)

skip_stat:	skip_stat.c com.o
	$(CC) $(CFLAGS) -o $@ skip_stat.c com.o

clean:	
	rm -f skipd skip_addkey skip_attach skip_detach skip_stat *.o *~ Makefile.bak .depend
	sed -e '/^# Do not delete, dependencies are assumed below this line/q' >.Makefile Makefile; mv .Makefile Makefile

# Do not delete, dependencies are assumed below this line
