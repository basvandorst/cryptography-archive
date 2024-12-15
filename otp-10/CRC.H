

/*                                          Mon  05-08-1989  13:33:19
                                            
    This program demonstrates 3 methods of data integrity.

        1. Checksum method.  (Fast, but not reliable)

        2. 16 bit cyclical redundancy check (CRC). Almost
           as fast as Checksum, but more reliable.

        3. 32 bit cyclical redundancy check (CRC). Slow,
           but very reliable.  (Used by C.Forsberg and P.Katz)

    This demo will compile with Turbo C, but porting to
    other compilers will be simple.

           Usage:
                  crc [d:][path][/1|/3|/c|/a] filename

                     /1 - CRC16      /3 - CRC32
                     /c - Checksum   /a - All

                                            Tony Tortorelli
*/


/*
    Sources:
*/

/*
                          **** CRC32 ****


   Copyright (C) 1986 Gary S. Brown.  You may use this program, or
   code or tables extracted from it, as desired without restriction.

   Demonstration program to compute the 32-bit CRC used as the frame
   check sequence in ADCCP (ANSI X3.66, also known as FIPS PUB 71
   and FED-STD-1003, the U.S. versions of CCITT's X.25 link-level
   protocol).  The 32-bit FCS was added via the Federal Register,
   1 June 1982, p.23798.  I presume but don't know for certain that
   this polynomial is or will be included in CCITT V.41, which
   defines the 16-bit CRC (often called CRC-CCITT) polynomial.  FIPS
   PUB 78 says that the 32-bit FCS reduces otherwise undetected
   errors by a factor of 10^-5 over 16-bit FCS.

   First, the polynomial itself and its table of feedback terms.  The
   polynomial is
   X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0
   Note that we take it "backwards" and put the highest-order term in
   the lowest-order bit.  The X^32 term is "implied"; the LSB is the
   X^31 term, etc.  The X^0 term (usually shown as "+1") results in
   the MSB being 1.

   Note that the usual hardware shift register implementation, which
   is what we're using (we're merely optimizing it by doing eight-bit
   chunks at a time) shifts bits into the lowest-order term.  In our
   implementation, that means shifting towards the right.  Why do we
   do it this way?  Because the calculated CRC must be transmitted in
   order from highest-order term to lowest-order term.  UARTs transmit
   characters in order from LSB to MSB.  By storing the CRC this way,
   we hand it to the UART in the order low-byte to high-byte; the UART
   sends each low-bit to hight-bit; and the result is transmission bit
   by bit from highest- to lowest-order term without requiring any bit
   shuffling on our part.  Reception works similarly.
*/

/*
                          **** CRC16 ****


   CRC16 macro derived from article Copyright (C) 1986 Stephen Satchell.
    NOTE: First srgument must be in range 0 to 255.
          Second argument is referenced twice.

   Programmers may incorporate any or all code into their programs,
   giving proper credit within the source. Publication of the
   source routines is permitted so long as proper credit is given
   to Stephen Satchell, Satchell Evaluations and Chuck Forsberg,
   Omen Technology.
*/



#define YES 1
#define NO 0



/*
CRC32 Macro: [ i.e. x = UPDC32(c, x); , Then, x = ~x in final calculation ]
*/

#define UPDC32(c, crc) (crc32tab[((int)(crc) ^ (c)) & 0xff] ^ (((crc) >> 8) & 0x00FFFFFFL))











