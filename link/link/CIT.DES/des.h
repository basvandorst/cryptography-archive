/*
 *  des.h - specifies the interface to the DES encryption library.
 *      This library provides routines to expand a key, encrypt and
 *      decrypt 64-bit data blocks.  The DES Data Encryption Algorithm
 *      is a block cipher which ensures that its output is a complex
 *      function of its input and the key.  A description of the
 *      algorithm may be found in:
 *
 *      Australian Standard AS2805.5-1985   The Data Encryption Algorithm
 *      ANSI Standard X3.92-1981            The Data Encryption Algorithm
 *      ANSI Standard X3.106-1983           DEA - Modes of Operation
 *
 *	RCS ID: $Header: des.h,v 2.1 90/07/18 10:53:53 lpb Exp $
 *
 *  Author: Lawrence Brown <lpb@cs.adfa.oz.au>      Dec 1987
 *      Computer Science, UC UNSW, Australian Defence Force Academy,
 *          Canberra, ACT 2600, Australia.
 *
 *      The user interface and some code has been inspired by the PD
 *      programs of:    Phil Karn, KA9Q <karn@flash.bellcore.com> 1986
 *        and       Jim Gillogly 1977.
 *      and by      Michael Barr-David's honours DES program 1987
 *
 *  Version:
 *      v4.0 - of des64.o is current 7/89
 *
 *  Copyright 1988 by Lawrence Brown and CITRAD. All rights reserved.
 *      This program may not be sold or used as inducement to buy a
 *      product without the written permission of the author.
 *
 *  Description:
 *  The routines provided by the library are:
 *
 *  keyinit(key)	    - expands a key into subkey, for use in 
 *    char  key[8];             encryption and decryption operations.
 *
 *  endes(b)		    - main DES encryption routine, this routine 
 *    char  b[8];               encrypts one 64-bit block b with subkey
 *
 *  dedes(b)		    - main DES decryption routine, this routine 
 *    char  b[8];               decrypts one 64-bit block b with subkey
 *
 *  desinit(mode)	    - init DES library, NULL fn for compatibility
 *    int   mode;		should be 0
 *
 *  desdone()		    - close DES library, NULL fn for compatibility
 *
 *  The 64-bit data blocks used in the algorithm are specified as eight
 *      unsigned chars
 *      For the purposes of implementing the DES algorithm, the bits
 *      are numbered as follows:
 *          [1...8] [9...16] [17...24] ... [57...64]
 *      in  b[0]  b[1]  b[2]  b[3]  b[4]  b[5]  b[6]  b[7]
 *  
 *  The key is passed as a 64-bit value, of which the 56 non-parity 
 *      bits are used. The parity bits are in DES bits
 *      8, 16, 24, 32, 40, 48, 56, 64 (nb: these do NOT correspond to
 *      the parity bits in ascii chars when packed in the usual way).
 */

#define DESBLK	8		/* No of bytes in a DES data-block           */
#define ROUNDS	16		/* No of DES rounds             	     */

typedef unsigned long   Long;   /* type specification for aligned DES blocks */

extern Long des64_subkeys[ROUNDS][DESBLK/4];	/* des subkeys               */
extern Long dea_IV[2];	/* Initialization Vector for chaining        */
extern char *des_lib_ver;       /* String with version no. & copyright       */

#ifdef ansi			/* declare prototypes for library functions  */
extern void endes(char b[8]);
extern void dedes(char b[8]);
extern void keyinit(char key[8]);
extern void setIV(char iv[8]);
extern void desinit(int mode);
extern void desdone(void);
#else				/* else just declare library functions extern */
extern void endes(), dedes(), keyinit(), setIV(), desinit(), desdone();
#endif ansi

#ifdef sysv
#define index strchr
#endif

#ifdef vax
#define index strchr
#endif
