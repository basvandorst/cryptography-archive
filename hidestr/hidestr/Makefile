# makefile for hidestr

# ================> customize here
CC		= cc
CFLAGS		= -O
SHAR		= shar2 -v
# ================> customize ends

STR1		= This execution should produce
STR2		= exactly two lines of output.
SHARlist	= README hidestr.1 hidestr.c Makefile

# default, make the program
hidestr:	hidestr.c
	$(CC) -o hidestr $(CFLAGS) hidestr.c

# make it and test it
test:	hidestr
	@./hidestr "$(STR1)" "$(STR2)" >test.c
	$(CC) -o test -DTEST_HIDE test.c
	./test
	@rm -f test test.c

# generate a shar file to share this
shar:	hidestr.shar

hidestr.shar:	$(SHARlist)
	$(SHAR) $(SHARlist) > hidestr.shar

tar:	hidestr.tar.Z

hidestr.tar.Z:	$(SHARlist)
	tar cvf - $(SHARlist) | compress -v > hidestr.tar.Z
