
CC	= gcc # -Wall -ansi

# For GNU/Linux/BSD systems
CFLAGS	= -I. -O2	# -static
YACC	= bison -y
LDFLAGS = -static
LEX    = lex -l
RANLIB = ranlib

# MKLinux and Linux-ppc:
#CFLAGS  = -I. -O2       # -static
#YACC    = bison -y
#LDFLAGS =
#LEX     = lex -l
#RANLIB	= ranlib

# For the Cygnus Win32 environment
#CFLAGS	= -DL_ENDIAN -DGNU_WIN32 -I. -O
#YACC	= byacc
#LDFLAGS = 
#RANLIB = :

# For OS/2
#CFLAGS	= -DL_ENDIAN -DOS2 -I. -O2
#YACC	= yacc
#LEX	= flex -l
#LDFLAGS =
#RANLIB = :

# DEC Unix: compiled on OSF1 V4.0 386 alpha
#CC	= cc
#CFLAGS	= -I. -O
#YACC	= yacc
#LDFLAGS = -non_shared
#RANLIB = :

# Everything else
#CFLAGS	= -I. -O
#YACC	= yacc
#LDFLAGS = # -static
#RANLIB = :

# ----------------------------------------------------------------------
# You should not need to modify anything below here
#
OBJS	= morpher.o gram.o lex.o index.o \
		os2lx.o os2obj.o os2page.o \
		cp.o options.o \
		md5_calc.o md5_dgst.o misc.o \
		msgs.o \
		pef.o pidata.o \
		spans.o log.o

LIB	= libffy.a

all:	$(LIB)

$(LIB):	$(OBJS)
	ar ru $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

gram.o:	gram.y

lex.o:	lex.c y.tab.h

lex.c:	lex.l

lex.l:	y.tab.h

y.tab.h: gram.y
	$(YACC) -d gram.y

clean:
	-rm -f *.o
	-rm -f y.tab.h
	-rm -f y.tab.c
	-rm -f gram.c
	-rm -f lex.c
	-rm -f core
	-rm -f *.old
	-rm -f *.bak
	-rm -f $(LIB)
