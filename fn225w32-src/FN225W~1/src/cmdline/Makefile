
COMMON	= ../common
INC	= -I. -I$(COMMON)

CC	= gcc # -Wall

# For GNU/Linux systems
CFLAGS	= $(INC) -O2	# -static
LDFLAGS = -static

# MKLinux and Linux-ppc:
#CFLAGS  = $(INC) -O2       # -static
#LDFLAGS =

# For the Cygnus Win32 environment
#CFLAGS	= -DL_ENDIAN -DGNU_WIN32 $(INC) -O
#LDFLAGS = 

# For OS/2
#CFLAGS	= -DL_ENDIAN -DOS2 $(INC) -O2
#LDFLAGS = 

# DEC Unix: compiled on OSF1 V4.0 386 alpha
#CC	= cc
#CFLAGS	= $(INC) -O
#LDFLAGS = -non_shared

# SunOS 4.1.3
#CFLAGS	= $(INC) -O -DSYS_ERRLIST
#LDFLAGS = # -static

# Everything else
#CFLAGS	= $(INC) -O
#LDFLAGS = # -static

# ----------------------------------------------------------------------
# You should not need to modify anything below here
#
LIB	= $(COMMON)/libffy.a

all:	fortify md5

fortify: fortify.o callback.o $(LIB)
	$(CC) $(CFLAGS) $(LDFLAGS) fortify.o callback.o $(LIB) -o $@

md5:    md5.o callback.o $(LIB)
	$(CC) $(CFLAGS) $(LDFLAGS) md5.o callback.o $(LIB) -o $@


clean:
	-rm -f *.o
	-rm -f core
	-rm -f *.old
	-rm -f *.bak
