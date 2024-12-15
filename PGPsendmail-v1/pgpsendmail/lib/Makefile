DEFINES =

CC = cc
COPTIMISE = -O
CFLAGS = $(DEFINES) -I../include $(COPTIMISE)

TARGETS =	libpgpsendmail.a

all:	$(TARGETS) clean

OBJECTS =	conn_to_pgpd.o \
		header.o \
		keylist.o \
		memory.o \
		misc.o \
		options.o \
		spawn.o \
		strings.o \
		expression.o

libpgpsendmail.a:	$(OBJECTS)
	ar r libpgpsendmail.a $(OBJECTS)
	ranlib libpgpsendmail.a

clean:
	rm -f *.BAK *.log *.sed .#* *~ *.bak */*.bak */tmp* */*~ */*/*~
	-chmod u+rw,go-w * 2> /dev/null

# Dependencies
conn_to_pgpd.o:		../include/pgpsendmail.h	../include/karma.h
header.o:		../include/pgpsendmail.h	../include/karma.h
keylist.o:		../include/pgpsendmail.h	../include/karma.h
memory.o:		../include/pgpsendmail.h	../include/karma.h
misc.o:			../include/pgpsendmail.h	../include/karma.h
options.o:		../include/pgpsendmail.h	../include/karma.h
spawn.o:		../include/pgpsendmail.h	../include/karma.h
strings.o:		../include/pgpsendmail.h	../include/karma.h
