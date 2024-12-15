CC = gcc

#CFLAGS	= -I../

# use this line if you want libedit
CFLAGS	=	-I../ -DLIBEDIT

SRCS	= news_tcl.c
OBJS	= news_tcl.o

all: news_tcl

.c.o: $<
	$(CC) $(CFLAGS) -Wall -g -c $<

depend: $(SRCS)
	gcc -MM $(SRCS) > .depend

news_tcl: $(OBJS) ../libnews.a
	$(CC) -o news_tcl news_tcl.o -L../ -lnews -ltcl -ltk

news_tcl_edit: $(OBJS) ../libnews.a
	$(CC) -o news_tcl news_tcl.o -L../ -lnews -ltcl	-ltk -ledit

clean:
	rm -f *~ news_tcl *.o

#
# include a dependency file if one exists
#
ifeq (.depend,$(wildcard .depend))
include .depend
endif
