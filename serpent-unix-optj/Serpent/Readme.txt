README

Algorithm:	SERPENT
Submitter:	Ross J. Anderson, Eli Biham, Lars Knudsen
==========================================================

This directory contains the following files:

Serpent.jar		.jar file for the SERPENT algorithm

classes:
   NIST:
	KAT.class
	MCT.class
	test.class

   Serpent:
	gen_plains.class
	Serpent_Algorithm.class
	Serpent_BitSlice.class
	Serpent_Properties.class
	Serpent_Standard.class

src:
   NIST:
	KAT.java
	MCT.java
	test.java

   Serpent:
	gen_plains.java
	Serpent_Algorithm.java
	Serpent_BitSlice.java
	Serpent_Properties.java
	Serpent_Standard.java



=============
This Java(TM) code is based on Cryptix code, but has been further optimised and
the format in which values are printed has been corrected.

There are actually two implementations: Serpent_Standard and
Serpent_Bitslice. The latter is much faster. You can select one or the
other by setting a constant in Serpent_Algorithm. Choose Serpent_Bitslice
unless you need debugging (since only Serpent_Standard outputs debugging
information).

There is code to produce the KAT and MCT text files. The original KAT/MCT
from Cryptix printed numbers the wrong way round with respect to Serpent's
conventions.

The NIST.test program helps measure the speed of the AES ciphers (Usage:
"java NIST.test Serpent").

We also supply the Serpent.gen_plains program, which generates the
plaintexts for the tables tests. (Usage: "java Serpent.gen_plains Serpent";
this will produce ecb_tables.txt.) It generates the plaintexts using
special functions (in Serpent_Standard), and then encrypts and decrypts the
results to print out the values and check its own correctness. Note that
the actual encryption/decryption can be done by either Serpent_Standard or
Serpent_Bitslice, depending on your selection in Serpent_Algorithm.

This bitslice code is almost 14 times faster than the latest cryptix code,
and about 35 times faster than their original code.

We are very grateful to the Cryptix group for making the Java code
available.
					Serpent authors

-----------------------------------------------------------------
TM: Java and Java-based marks are trademarks or registered 
trademarks of Sun Microsystems, Inc. in the United States and 
other countries.