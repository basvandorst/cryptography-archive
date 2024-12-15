all: tran

tran: tran.c rnd.c
	cc -o tran -O rnd.c tran.c

test: tmp all
	tran < tmp > tmpo ; more tmpo ; mv tmpo tmp

tmp: foo
	cp foo tmp

clean:
	rm -f tmp tmpo tran tran.o rnd.o
