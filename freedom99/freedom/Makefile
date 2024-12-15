# Makefile for Freedom remailer
# 1995, jfleming@indiana.edu

# Define these for your system and your tastes
CC = gcc # cc
STRIP = strip # true
CFLAGS = -O3 # -g
LFLAGS =

########################################################################
# Things after this point probably don't need to be changed.
########################################################################

OBJS=remailer.o util1.o stats.o util2.o count.o util3.o util4.o

all:		remailer

remailer:	$(OBJS)
		$(CC) $(LFLAGS) $(OBJS) -o remailer
		$(STRIP) remailer
		chmod 711 remailer

remailer.o:	remailer.c config.h
		$(CC) $(CFLAGS) -c remailer.c -o remailer.o

util1.o:	util1.c config.h
		$(CC) $(CFLAGS) -c util1.c -o util1.o

stats.o:	stats.c config.h
		$(CC) $(CFLAGS) -c stats.c -o stats.o

util2.o:	util2.c config.h
		$(CC) $(CFLAGS) -c util2.c -o util2.o

count.o:	count.c config.h
		$(CC) $(CFLAGS) -c count.c -o count.o

util3.o:	util3.c config.h
		$(CC) $(CFLAGS) -c util3.c -o util3.o

util4.o:	util4.c config.h
		$(CC) $(CFLAGS) -c util4.c -o util4.o

clean:		
		rm -f $(OBJS) remailer *~
