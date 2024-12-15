# OS/2 emx/gcc makefile

# Compiling for other than OS/2 should be pretty simple - take out all the
# -Z stuff and adjust the object extension as necessary.  Also, you won't
# need mess.def either.  Tell me how it goes.

CFLAGS	= -c -Zomf -O2 -Zmtd
OBJ	= obj
LDFLAGS	= -Zmtd -Zomf -O2 mess.def
OBJS	= copy.$(OBJ) mess.$(OBJ) header.$(OBJ)

%.$(OBJ):	%.c
	gcc $(CFLAGS) $<

mess:	$(OBJS)
	gcc -o $@ $(OBJS) $(LDFLAGS)
#	emxbind -b h:\emx\bin\emx.exe $@ $@.exe
#  You can use the emxbind command to make the program run under dos or os/2,
#  if you're using emx/gcc.
