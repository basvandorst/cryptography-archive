           README for the Additional Code directory

Algorithm Name:       Mars
Principle Submitter:  IBM
----------------------------------------------------------

Contents:

    sbox.c      - support code used to generate the sbox for MARS
    sha.c       - sha-1 code used by sbox.c
    sha.h       - header for sha.c
    index.c     - a supporting program used to the sbox extries
                  to be used during key generation
    makefile    - make file to make the sbox.exe program

    testsgcc.exe  - timing test code compiled using DJGPP.  This compiler
                    can be used for all test code and makes the timing tests
                    run about twice as fast as the Borland compiled code.

    Readme.txt  - this file

Compiling:

    To compile MARS and the tests with Borland C++ 5.0 (in a DOS window):

        bcc32 -Oi -6 -v -A -a4 -O2 -DKAT tests.c mars-opt.c

    -DKAT turns on the KAT tests. Similarly, -DMCT and -DIVT
    turn on their respective tests.  Note that -DIVT adds code to
    the MARS encryption and decryption code, and thus impacts speed.

    To compile MARS and the tests with DJGPP (in a DOS window):

       gcc -Wall -pedantic -O6 -fomit-frame-pointer -mcpu=pentiumpro
          -o tests -DKAT tests.c mars-opt.c

    The same -DMCT and DIVT apply.
