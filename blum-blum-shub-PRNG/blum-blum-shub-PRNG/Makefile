#
#
#
compile:	Reblock
		echo "compiling C program"
#
#
#
clean:
		chmod 444 README Example_?file Sample_Primes *_bc
		chmod 755 Generate_Bits Create_*e*
		rm -i Reblock Example_sfile reblock.o *ugh*
#
#
#
Reblock:	reblock.o
		cc reblock.o -o Reblock
#
#
#
Example:	Reblock
		Create_Seed Example_pfile Example_qfile Example_ifile \
		> Example_sfile
		Generate_Bits   -n 300   Example_pfile  Example_qfile \
		  Example_sfile
#
#
