# Makefile for GNU 386 assembler version under UNIX
certify: descerta testdata
	./descerta < testdata

descerta: descert.o deskey.o desgnu.o desspa.o
	cc -o descerta descert.o deskey.o desgnu.o desspa.o

descyclea: descycle.o deskey.o desgnu.o desspa.o
	cc -o descyclea descycle.o deskey.o desgnu.o desspa.o

destimea: unixtime.o deskey.o desgnu.o desspa.o
	cc -o destimea unixtime.o deskey.o desgnu.o desspa.o

# The SP boxes for the assembler version are left-rotated 3 bits
desspa.c: gensp
	./gensp a > desspa.c

gensp: gensp.c
	cc -O -o gensp gensp.c

# GNU as seems confused by what should be legal comments. Double quotes
# in comments apparently start strings that extend past the end of line,
# and the "# file 1" lines emitted by cpp also seem to confuse it. So
# we strip out all the offending material.
desgnu.o: desgnu.s
	cpp desgnu.s | tr -d '"' | sed -e '/^#/d' | as -o desgnu.o

	
