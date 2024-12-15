CFLAGS = 

pwchkr: pwchkr.o subobjs
	cc -o fpwchkr fpwchkr.o fdes-init.o fdes-crypt.o fdes-xform.o

fpwchkr: fpwchkr.o subobjs
	cc -o fpwchkr fpwchkr.o fdes-init.o fdes-crypt.o fdes-xform.o

pwchkr.o: fpw.c
	cc -c -o fpwchkr.o -O fpw.c

fpwchkr.o: fpw.c
	cc -c -o fpwchkr.o -DFDES -O fpw.c

subobjs: fdes-init.o fdes-crypt.o fdes-xform.o
	cc -c -O fdes-init.c fdes-crypt.c

fdes-xform.o: fdes-xform.s fdes-xform.c
	cc -c -O fdes-xform.s

fdes-xform.s: fdes-xform.c fdes.sed
	cc -c -O -S fdes-xform.c
	sed -f fdes.sed < fdes-xform.s > tmp-fdes.s
	mv tmp-fdes.s fdes-xform.s

fdes.sed: fdes.sed.doc
	grep -v : fdes.sed.doc > fdes.sed

clean:
	rm -f *.o fdes-xform.s fdes.sed core
