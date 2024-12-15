# Makefile for portable C version under UNIX
certify: descert testdata
	./descert < testdata

descert: descert.o deskey.o desport.o dessp.o
	cc -o descert descert.o deskey.o desport.o dessp.o

descycle: descycle.o deskey.o desport.o dessp.o
	cc -o descycle descycle.o deskey.o desport.o dessp.o

destime: unixtime.o deskey.o desport.o dessp.o
	cc -o destime unixtime.o deskey.o desport.o dessp.o

dessp.c: gensp
	./gensp c > dessp.c

gensp: gensp.c
	cc -O -o gensp gensp.c

deskey.o: deskey.c des.h
desport.o: desport.c des.h
