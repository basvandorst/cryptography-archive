
#   Make file for IDEA package

#   Everything will probably work OK without any changes below
#   this line.

#   Compiler flags

CC = cc
CFLAGS = -O

all:	idea.o

IDEAOBJS = idea.o

idea:	$(IDEAOBJS)
	$(CC) $(IDEAOBJS) -o idea

idea.o: idea.c idea.h

clean:
	rm -f core *.out *.o *.bak idea *.shar
