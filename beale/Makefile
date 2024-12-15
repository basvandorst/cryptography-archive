all:	code modulo use

code:	code.c getwrd.c loadkey.c beale.h
	cc code.c -o code

modulo:	modulo.c  beale.h
	cc modulo.c -o modulo -lm

use:	use.c getwrd.c beale.h
	cc use.c -o use

beale.h:	1 2 3

clean:
	rm -f code modulo use
