# shade
POLY = o.c
#POLY = o.a68k

#DEFS =
DEFS = -DCHECK
#CHECK =
CHECK = check.o

FILES = shade.h make_prime.c shade.c    \
        poly.c check.c poly.a68k        \
        make_table.c project.c uucode.c \
		Makefile test.in test.orig

all: poly_prime shade test

clean:
	-rm *.o o.* test.out *.bak

spotless: clean
	-rm shade make_table make_prime poly_prime table.c

test: shade poly_prime
	shade -m -u <test.in
	diff test.orig test.out

make_table: make_table.c $(CHECK) $(POLY)
	gcc $(DEFS) -o make_table make_table.c $(CHECK) poly.o

make_prime: make_prime.c $(CHECK) $(POLY) table.o
	gcc -o make_prime make_prime.c $(CHECK) poly.o table.o

shade: shade.c uucode.o project.o $(CHECK) $(POLY) table.o
	gcc -o shade shade.c project.o uucode.o $(CHECK) poly.o table.o

poly_prime: make_prime
	make_prime

table.c: make_table
	make_table

table.o: table.c
	gcc -o table.o -c table.c

make_prime.o: make_prime.c
	gcc $(DEFS) -o make_prime.o -c make_prime.c

check.o: check.c
	gcc $(DEFS) -o check.o -c check.c

project.o: project.c
	gcc $(DEFS) -o project.o -c project.c

uucode.o: uucode.c
	gcc $(DEFS) -o uucode.o -c uucode.c

o.a68k: poly.a68k
	a68k poly.a68k
	-rm o.*
	touch o.a68k

o.c: poly.c
	gcc $(DEFS) -o poly.o -c poly.c
	-rm o.*
	touch o.c


