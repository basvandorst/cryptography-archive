/* ddes.h -
 *
 *	Headers and defines for ddes.c
 *	Graven Imagery, 1991.
 *
 * Copyright (c) 1988,1989,1990,1991 by Richard Outerbridge
 *	(GEnie : OUTER; CIS : [71755,204])
 */

#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

/* A useful alias on 68000-ish machines, but NOT USED HERE. */

typedef union {
	unsigned long blok[2];
	unsigned short word[4];
	unsigned char byte[8];
	} M68K;

#undef D2_DES		/* include double-length support */
		
#define EN0	0	/* MODE == encrypt (MUST be zero!) */
#define DE1	1	/* MODE == decrypt (MUST be != 0!) */

extern void deskey P((unsigned char *, int));
/*		      hexkey[8]     MODE
 * Sets the internal key register according to the hexadecimal
 * key contained in the 8 bytes of hexkey, according to the DES,
 * for encryption or decryption according to MODE.
 */

extern void usekey P((unsigned long *));
/*		    cookedkey[32]
 * Loads the internal key register with the data in cookedkey.
 */

extern void cpkey P((unsigned long *));
/*		   cookedkey[32]
 * Copies the contents of the internal key register into the storage
 * located at &cookedkey[0].
 */

extern void des P((unsigned char *, unsigned char *));
/*		    from[8]	      to[8]
 * Encrypts/Decrypts (according to the key currently loaded in the
 * internal key register) one block of eight bytes at address 'from'
 * into the block at address 'to'.  They can be the same.
 */

#ifdef D2_DES

extern void desDkey(unsigned char *, int);
/*		      hexkey[16]     MODE
 * Sets the internal key registerS according to the hexadecimal
 * keyS contained in the 16 bytes of hexkey, according to the DES,
 * for DOUBLE encryption or decryption according to MODE.
 */

extern void useDkey(unsigned long *);
/*		    cookedkey[64]
 * Loads the internal key registerS with the data in cookedkey.
 */

extern void cpDkey(unsigned long *);
/*		   cookedkey[64]
 * Copies the contents of the internal key registerS into the storage
 * located at &cookedkey[0].
 */

extern void Ddes(unsigned char *, unsigned char *);
/*		    from[8]	      to[8]
 * Encrypts/Decrypts (according to the keyS currently loaded in the
 * internal key registerS) one block of eight bytes at address 'from'
 * into the block at address 'to'.  They can be the same.
 */

extern void D2des(unsigned char *, unsigned char *);
/*		    from[16]	      to[16]
 * Encrypts/Decrypts (according to the keyS currently loaded in the
 * internal key registerS) one block of SIXTEEN bytes at address 'from'
 * into the block at address 'to'.  They can be the same.
 */

extern void makekey(char *, unsigned char *);
/*		*password,	single-length key[8]
 * With a double-length default key, this routine hashes a NULL-terminated
 * string into an eight-byte random-looking key, suitable for use with the
 * deskey() routine.
 */

extern void makeDkey(char *, unsigned char *);
/*		*password,	double-length key[16]
 * With a double-length default key, this routine hashes a NULL-terminated
 * string into a sixteen-byte random-looking key, suitable for use with the
 * desDkey() routine.
 */

#endif	/* D2_DES */

/* ddes V5.00 rwo 9105.20 09:00 Graven Imagery
 ********************************************************************/
