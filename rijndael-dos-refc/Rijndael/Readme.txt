Rijndael
Joan Daemen

Reference Implementation (+ KATs and MCTs)  v2.0
-----------------------------------------------------------

This archive contains the following files:

Makefile.bcc:	    makefile for use with Borland compiler.
Makefile.gcc:	    makefile for use with GCC-based compilers.
Makefile.Visualc:   makefile for use with Visual C compiler.
README: 	    This file.
boxes-ref.dat:	    Tables that are needed by the reference implementation.
		    The tables implement the S-box and its inverse, and also
		    some temporary tables needed for multiplying in the finite
		    field GF(2^8).
rijndael-alg-ref.c:
rijndael-alg-ref.h:
		    Algorithm implementation.
rijndael-api-ref.c:
rijndael-api-ref.h:
		    Interface to the C API.
rijndaeltest-ref.c:
		    Implementation of the KAT and MCT.
table.128:
table.192:
table.256:
		Files needed for the KAT (for the Table Known Answer Test).



Instructions for the KAT and MCT software:

1) Compile the C code and put the executable in the same directory as the
   table.??? files.
2) Run the executable. It generates all the tables in the NIST format.
3) Compare the generated tables with the original provided tables, e.g.
   in Unix, with `diff'. 
   
   
Changes with respect to v1.0 (= round 1 submission)
---------------------------------------------------

1) Removed the parameter blockLen from makeKey() and cipherInit().
   The parameter is still present in the structures keyInstance and
   cipherInstance.
   
   
