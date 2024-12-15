Crypto++: a C++ Class Library of Cryptographic Primitives
Version 1.1   10/27/1995

This version is basicly the same as version 1.0, but with rc4.h, rc4.cpp, 
rc5.h, rc5.cpp, rsa.h, rsa.cpp removed and calls to nonpublic functions in RSAREF disabled
at the request of RSADSI.  A few minor bugs were also fixed.

-------------- the original readme file follows ---------------------

This library includes:

MD5, SHS, DES, IDEA, WAKE, RC4, RC5,
Blowfish, Diamond, Diamond Lite, Sapphire, Luby-Rackoff, MDC, various 
modes (CFB, CBC, OFB, counter), DH, RSA, DSA, ElGamal, BBS, gzip 
compression, Shamir's secret sharing scheme, Rabin's information 
dispersal scheme, and zero-knowledge prover and verifier for
graph isomorphism.  There are also various miscellanous modules such 
as base 64 coding and 32-bit CRC.

It has been compiled and tested with Borland C++ 4.5, MSVC 2.1, 
and G++ 2.6.3 on MS-DOS, Windows NT, and a variety of Unix machines.
You are welcome to use it for any purpose without paying me, but see
license.txt for the fine print.

           
Some short instructions to compile this library:
(you probably need to modify this to suit your environment)


-- if want to use this library with RSAREF, then

1. get a copy of RSAREF

2. untar or unzip it into a directory below this one

3. type "gcc -c -I. *.c" (in the rsaref/source directory) to compile RSAREF

4. edit config.h

5. type "g++ -c -Irsaref/source -I. *.cpp" to compile this library

6. type "g++ *.o rsaref/source/*.o -lg++ -lm" to link the test driver

7. type "a.out" to run the test driver


-- if you DON'T want to use this library with RSAREF, then

1. edit config.h (make sure to comment out #define USE_RSAREF)

2. type "g++ -c *.cpp" to compile this library

3. type "g++ *.o -lg++ -lm" to link the test driver

4. type "a.out" to run the test driver


Finally, a note on object ownership:  If a constructor for A takes 
a pointer to an object B (except primitive types such as int and char), then 
A owns B and will delete B at A's destruction.  If a 
constructor for A takes a reference to an object B, then 
the caller retains ownership of B and should not destroy it until A no longer 
needs it.

Good luck, and feel free to e-mail me at weidai@eskimo.com if you have any 
problems.

Wei Dai
