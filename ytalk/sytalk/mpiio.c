/*	mpiio.c - C source code for multiprecision integer I/O routines.
	Implemented Nov 86 by Philip Zimmermann
	Last revised 13 Sep 91 by PRZ

	Boulder Software Engineering
	3021 Eleventh Street
	Boulder, CO 80304
	(303) 541-0140

	(c) Copyright 1986-92 by Philip Zimmermann.  All rights reserved.
	The author assumes no liability for damages resulting from the use 
	of this software, even if the damage results from defects in this 
	software.  No warranty is expressed or implied.  

	These routines are for multiprecision arithmetic I/O functions for
	number-theoretic cryptographic algorithms such as ElGamal,
	Diffie-Hellman, Rabin, or factoring studies for large composite
	numbers, as well as Rivest-Shamir-Adleman (RSA) public key
	cryptography.

	The external data representation for RSA messages and keys that
	some of these library routines assume is outlined in a paper by 
	Philip Zimmermann, "A Proposed Standard Format for RSA Cryptosystems",
	IEEE Computer, September 1986, Vol. 19 No. 9, pages 21-34.
	Some revisions to this data format have occurred since the paper
	was published.
*/

/* #define DEBUG */


#ifndef EMBEDDED	/* not EMBEDDED - not compiling for embedded target */
#include <stdio.h> 	/* for printf, etc. */
#else	/* EMBEDDED - compiling for embedded target */
#define NULL (void *)0
#endif

#include "mpilib.h"
#include "mpiio.h"
#include "pgp.h"

static void puthexbyte(byte b); /* Put out byte in ASCII hex via putchar. */
static
void puthexw16(word16 w); /* Put out 16-bit word in hex, high byte first. */
static
void putstr(string s); /* Put out null-terminated ASCII string via putchar. */

/*----------------- Following procedures relate to I/O ------------------*/

int string_length(char *s)
	/* Returns string length, just like strlen() from <string.h> */
{	int i;
	i = 0;
	if (s != NULL)
		while (*s++) i++;
	return (i);	
}	/* string_length */


#ifdef DEBUG
static int ctox(int c)
	/* Returns integer 0-15 if c is an ASCII hex digit, -1 otherwise. */
{	if ((c >= '0') && (c <= '9'))
		return(c - '0');
	if ((c >= 'a') && (c <= 'f'))
		return((c - 'a') + 10);
	if ((c >= 'A') && (c <= 'F'))
		return((c - 'A') + 10);
	return(-1);		/* error -- not a hex digit */
}	/* ctox */


int str2reg(unitptr reg,string digitstr)
	/* Converts a possibly-signed digit string into a large binary number.
	   Returns assumed radix, derived from suffix 'h','o',b','.' */
{	unit temp[MAX_UNIT_PRECISION],base[MAX_UNIT_PRECISION];
	int c,i;
	boolean minus = FALSE;
	short radix;	/* base 2-16 */

	mp_init(reg,0);
	
	i = string_length(digitstr);
	if (i==0) return(10);		/* empty string, assume radix 10 */
	c = digitstr[i-1];		/* get last char in string */
	
	switch (c)	/* classify radix select suffix character */
	{
	case '.':	radix = 10;
			break;
	case 'H':
	case 'h':	radix = 16;
			break;
	case 'O': 
	case 'o':	radix = 8;
			break;
	case 'B':
	case 'b':	radix = 2;	/* caution! 'b' is a hex digit! */
			break;
	default:	radix = 10;
	}

	mp_init(base,radix);
	if ((minus = (*digitstr == '-')) != 0) digitstr++;
	while ((c = *digitstr++) != 0)
	{	if (c==',') continue;	/* allow commas in number */
		c = ctox(c);
		if ((c < 0) || (c >= radix)) 
			break;	/* scan terminated by any non-digit */
		mp_mult(temp,reg,base);
		mp_move(reg,temp);
		mp_init(temp,c);
		mp_add(reg,temp);
	}
	if (minus) mp_neg(reg);
	return(radix);
} /* str2reg */

#endif	/* DEBUG */

/*	These I/O functions, such as putstr, puthexbyte, and puthexw16, 
	are provided here to avoid the need to link in printf from the 
	C I/O library.  This is handy in an embedded application.
	For embedded applications, use a customized putchar function, 
	separately compiled.
*/

static void putstr(string s)
	/* Put out null-terminated ASCII string via putchar. */
{	while (*s) putchar(*s++);
}	/* putstr */

static void puthexbyte(byte b)
	/* Put out byte in ASCII hex via putchar. */
{	static char *nibs = "0123456789ABCDEF";
	putchar(nibs[b >> 4]);
	putchar(nibs[b & 0x0F]);
}	/* puthexbyte */

static void puthexw16(word16 w)
	/* Put out 16-bit word in hex, high byte first. */
{	puthexbyte((byte)(w >> 8));
	puthexbyte((byte)(w & 0xFF));
}	/* puthexw16 */

#ifdef UNIT32
static void puthexw32(word32 lw)
	/* Puts out 32-bit word in hex, high byte first. */
{	puthexw16((word16)(lw>>16));
	puthexw16((word16)(lw & 0xFFFFL));
}	/* puthexw32 */
#endif	/* UNIT32 */


#ifdef UNIT8
#define puthexunit(u) puthexbyte(u)
#endif
#ifdef UNIT16
#define puthexunit(u) puthexw16(u)
#endif
#ifdef UNIT32
#define puthexunit(u) puthexw32(u)
#endif

#ifdef DEBUG
int display_in_base(string s,unitptr n,short radix)
	/* Display n in any base, such as base 10.  Returns number of digits. */
	/*	s is string to label the displayed register.
		n is multiprecision integer.
		radix is base, 2-16. 
	*/
{
	char buf[MAX_BIT_PRECISION + (MAX_BIT_PRECISION/8) + 2];
	unit r[MAX_UNIT_PRECISION],quotient[MAX_UNIT_PRECISION];
	word16 remainder;
	char *bp = buf;
	char minus = FALSE;
	int places = 0;
	int commaplaces;	/* put commas this many digits apart */
	int i;

	/*	If string s is just an ESC char, don't print it.
		It's just to inhibit the \n at the end of the number.
	*/
	if ((s[0] != '\033') || (s[1] != '\0'))
		putstr(s);

	if ( (radix < 2) || (radix > 16) )
	{	putstr("****\n");	/* radix out of range -- show error */
		return(-1);
	}
	commaplaces = (radix==10 ? 3 : (radix==16 ? 4 :
			(radix==2 ? 8 : (radix==8 ? 8 : 1))));
	mp_move(r,n);
	if ((radix == 10) && mp_tstminus(r))
	{	minus = TRUE;
		mp_neg(r);	/* make r positive */
	}

	*bp = '\0';
	do	/* build backwards number string */
	{	if (++places>1)
			if ((places % commaplaces)==1)
				*++bp = ',';	/* 000,000,000,000 */
		remainder = mp_shortdiv(quotient,r,radix);
		*++bp = "0123456789ABCDEF" [remainder]; /* Isn't C wonderful? */
		mp_move(r,quotient);
	} while (testne(r,0));
	if (minus)
		*++bp = '-';
	
	if (commaplaces!=1)
		while ((++places % commaplaces) != 1)
			*++bp = ' '; /* pad to line up commas */

	i = string_length(s);
	while (*bp)
	{	putchar(*bp);
		++i;
		if ((*bp == ',') || commaplaces==1)
			if (i > (72-commaplaces))
			{	putchar('\n'); 
				i=string_length(s);
				while (i--) putchar(' ');
				i = string_length(s);
			}
		bp--;
	}
	switch (radix)
	{	/* show suffix character to designate radix */
	case 10: /* decimal */
		putchar('.');
		break;
	case 16: /* hex */
		putchar('h');
		break;
	case 8: /* octal */
		putchar('o');
		break;
	case 2: /* binary */
		putchar('b');
		break;
	default: /* nonstandard radix */
		/* printf("(%d)",radix); */ ;	
	}

	if ((s[0] == '\033') && (s[1] == '\0'))
		putchar(' ');	/* supress newline */
	else putchar('\n');

	fill0((byteptr)buf,sizeof(buf));	/* burn the evidence on the stack...*/
	/* Note that local stack arrays r and quotient are now 0 */
	return(places);
}	/* display_in_base */

#endif	/* DEBUG */

void mp_display(string s,unitptr r)
	/* Display register r in hex, with prefix string s. */
{	short precision;
	int i,j;
	putstr(s);
	normalize(r,precision);	/* strip off leading zeros */
	if (precision == 0)
	{	putstr(" 0\n");
		return;
	}
	make_msbptr(r,precision);
	i=0;
	while (precision--)
	{	if (!(i++ % (16/BYTES_PER_UNIT)))
		{	if (i>1)
			{	putchar('\n'); 
				j=string_length(s); 
				while (j--) putchar(' ');
			}
		}
		puthexunit(*r);
		putchar(' ');
		post_lowerunit(r);
	}
	putchar('\n');
}	/* mp_display */


word16 checksum(register byteptr buf, register word16 count)
	/* Returns checksum of buffer. */
{	word16 cs;
	cs = 0;
	while (count--) cs += *buf++;
	return(cs);
} /* checksum */


void cbc_xor(register unitptr dst, register unitptr src, word16 bytecount)
	/*	Performs the XOR necessary for RSA Cipher Block Chaining.
		The dst buffer ought to have 1 less byte of significance than 
		the src buffer.  Only the least significant part of the src 
		buffer is used.  bytecount is the size of a plaintext block.
	*/
{	short nunits;	/* units of precision */
	nunits = bytes2units(bytecount)-1;
	make_lsbptr(dst,global_precision);
	while (nunits--)
	{	*dst ^= *post_higherunit(src);
		post_higherunit(dst);
		bytecount -= units2bytes(1);
	}
	/* on the last unit, don't xor the excess top byte... */
	*dst ^= (*src & (power_of_2(bytecount<<3)-1));
}	/* cbc_xor */


void hiloswap(byteptr r1,short numbytes)
	/* Reverses the order of bytes in an array of bytes. */
{	byteptr r2;
	byte b;
	r2 = &(r1[numbytes-1]);
	while (r1 < r2)
	{	b = *r1; *r1++ = *r2; *r2-- = b;
	}
}	/* hiloswap */


#define byteglue(lo,hi) ((((word16) hi) << 8) + (word16) lo)

/****	The following functions must be changed if the external byteorder
	changes for integers in PGP packet data.
****/


word16 fetch_word16(byte *buf)
/*	Fetches a 16-bit word from where byte pointer is pointing.
	buf points to external-format byteorder array.
*/
{	word16 w0,w1;
/* Assume MSB external byte ordering */
	w1 = *buf++;
	w0 = *buf++;
	return(w0 + (w1<<8));
}	/* fetch_word16 */


byte *put_word16(word16 w, byte *buf)
/*	Puts a 16-bit word to where byte pointer is pointing, and 
	returns updated byte pointer.
	buf points to external-format byteorder array.
*/
{
/* Assume MSB external byte ordering */
	buf[1] = w & 0xff;
	w = w>>8;
	buf[0] = w & 0xff;
	return(buf+2);
}	/* put_word16 */


word32 fetch_word32(byte *buf)
/*	Fetches a 32-bit word from where byte pointer is pointing.
	buf points to external-format byteorder array.
*/
{	word32 w0,w1,w2,w3;
/* Assume MSB external byte ordering */
	w3 = *buf++;
	w2 = *buf++;
	w1 = *buf++;
	w0 = *buf++;
	return(w0 + (w1<<8) + (w2<<16) + (w3<<24));
}	/* fetch_word32 */


byte *put_word32(word32 w, byte *buf)
/*	Puts a 32-bit word to where byte pointer is pointing, and 
	returns updated byte pointer.
	buf points to external-format byteorder array.
*/
{
/* Assume MSB external byte ordering */
	buf[3] = w & 0xff;
	w = w>>8;
	buf[2] = w & 0xff;
	w = w>>8;
	buf[1] = w & 0xff;
	w = w>>8;
	buf[0] = w & 0xff;
	return(buf+4);
}	/* put_word32 */


/***	End of functions that must be changed if the external byteorder
	changes for integer fields in PGP packets.
***/




short mpi2reg(register unitptr r,register byteptr buf)
/*	Converts a multiprecision integer from the externally-represented 
	form of a byte array with a 16-bit bitcount in a leading length 
	word to the internally-used representation as a unit array.
	Converts to INTERNAL byte order.
	The same buffer address may be used for both r and buf.
	Returns number of units in result, or returns -1 on error.
*/
{	byte buf2[MAX_BYTE_PRECISION];
	word16 bitcount, bytecount, unitcount, zero_bytes, i;

	/* First, extract 16-bit bitcount prefix from first 2 bytes... */
	bitcount = fetch_word16(buf);
	buf += 2;

	/* Convert bitcount to bytecount and unitcount... */
	bytecount = bits2bytes(bitcount);
	unitcount = bytes2units(bytecount);
	if (unitcount > global_precision)
	{	/* precision overflow during conversion. */
		return(-1);	/* precision overflow -- error return */
	}
	zero_bytes = units2bytes(global_precision) - bytecount;
/* Assume MSB external byte ordering */
	fill0(buf2,zero_bytes);  /* fill leading zero bytes */
	i = zero_bytes;	/* assumes MSB first */
	while (bytecount--) buf2[i++] = *buf++;

	mp_convert_order(buf2);	/* convert to INTERNAL byte order */
	mp_move(r,(unitptr)buf2);
	mp_burn((unitptr)buf2);	/* burn the evidence on the stack */
	return(unitcount);	/* returns unitcount of reg */
}	/* mpi2reg */


short reg2mpi(register byteptr buf,register unitptr r)
/*	Converts the multiprecision integer r from the internal form of 
	a unit array to the normalized externally-represented form of a
	byte array with a leading 16-bit bitcount word in buf[0] and buf[1].
	This bitcount length prefix is exact count, not rounded up.
	Converts to EXTERNAL byte order.
	The same buffer address may be used for both r and buf.
	Returns the number of bytes of the result, not counting length prefix.
*/
{	byte buf1[MAX_BYTE_PRECISION];
	byteptr buf2;
	short bytecount,bc;
	word16 bitcount;
	bitcount = countbits(r);
#ifdef DEBUG
	if (bitcount > MAX_BIT_PRECISION)
	{	fprintf(stderr, "reg2mpi: bitcount out of range (%d)\n", bitcount);
		return 0;
	}
#endif
	bytecount = bits2bytes(bitcount);
	bc = bytecount;	/* save bytecount for return */
	buf2 = buf1;
	mp_move((unitptr)buf2,r);
	mp_convert_order(buf2);	/* convert to EXTERNAL byteorder */
/* Assume MSB external byte ordering */
	buf2 += units2bytes(global_precision) - bytecount;
	buf = put_word16(bitcount, buf); /* store bitcount in external byteorder */

	while (bytecount--) *buf++ = *buf2++;

	mp_burn((unitptr)buf1);	/* burn the evidence on the stack */
	return(bc);		/* returns bytecount of mpi, not counting prefix */
}	/* reg2mpi */


#ifdef DEBUG

void dumpbuf(string s, byteptr buf, int bytecount)
	/* Dump buffer in hex, with string label prefix. */
{	putstr(s);
	while (bytecount--)
	{	puthexbyte(*buf++);
		putchar(' ');
		if ((bytecount & 0x0f)==0)
			putchar('\n');
	}
} /* dumpbuf */

void dump_unit_array(string s, unitptr r)
/*	Dump unit array r as a C array initializer, with string label prefix. 
	Array is dumped in native unit order.
*/
{	int unitcount;
	unitcount = global_precision;
	putstr(s);
	putstr("\n{ ");
	while (unitcount--)
	{	putstr("0x");
		puthexunit(*r++);
		putchar(',');
		if (unitcount && ((unitcount & 0x07)==0))
			putstr("\n  ");
	}
	putstr(" 0};\n");
} /* dump_unit_array */

#endif	/* ifdef DEBUG */


/* ASN encoding for RSA/MD5 for PKCS compatibility */
static unsigned char asn_array[] = {	/* PKCS 01 block type 01 data */
	0x30,0x20,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,
	0x02,0x05,0x05,0x00,0x04,0x10 };
/* Count from end for zero */
#define ASN_ZERO_END 3

/*
**	short preblock(outreg, inbuf, bytecount, modulus, randompad)
**
**	A plaintext message must be converted into an integer less than
**	the modulus n.  We do this by making it 1 byte shorter than the
**	normalized modulus n.  Short blocks are left justified and padded.
**
**	The padding used depends on whether randompad is NULL.  First a
**	0 byte is added beyond the data; then either 0xff's or values
**	from randompad are added.  Last is a byte which tells whether
**	we are preblocking a message digest or a conventional key; we
**	assume that random padding is only used for conventional keys.
**
*/

#ifdef PKCS_COMPAT
/* Version for compatibility with PKCS */

short preblock(unitptr outreg, byteptr inbuf, short bytecount,
	unitptr modulus, byteptr randompad)
/*	Converts plaintext block into form suitable for RSA encryption.
	inbuf contains the pointer to the data to be blocked.  To the
	extent that it can be said to have a byte order, it should be
	prepared MSB first.
	Converts to an MPI in INTERNAL byte order.
	Returns # of bytes remaining to process.  Note that the same buffer 
	address may be used for both outreg and inbuf.
	randompad is a pointer to a buffer of random pad bytes to use for 
	padding material, or NULL iff we want to use constant padding.
*/
{	byte out[MAX_BYTE_PRECISION];
	short byte_precision,leading_zeros,remaining,blocksize,padding;
	boolean mic_flag = (randompad==0);
	int i,j;

	/* Our strategy is to fill the buffer in MSB-first order, then
	 * to call convert_order which will make it proper for an MPI.
	 */
	byte_precision = units2bytes(global_precision);
	leading_zeros = byte_precision - countbytes(modulus) + 1;
	blocksize = byte_precision - leading_zeros;
	/* note that blocksize includes data plus pad bytes, if any */

	padding = blocksize - 2 - (mic_flag?sizeof(asn_array):0) - bytecount;

	/* Remaining is # of bytes we can't do.  If it's negative it's the
	 * number more we could have done.
	 */
	remaining = 1 - padding;	/* Padding must be >= 1 */

	if (remaining>0)		/* Just do a part of it */
	{	bytecount -= remaining;
		padding = 1;
	}

	/* Start filling array, MSB first */
	i = 0;

	while (leading_zeros--)
		out[i++] = 0;

	/* Now we've gotten to the "mpi" part of the number */

	/* Now the type byte */
	out[i++] = mic_flag ? MD_ENCRYPTED_BYTE : CK_ENCRYPTED_BYTE;

	/* Now padding: use either 0xff or values from randompad */
	while (padding--)
		out[i++] = mic_flag ? 0xff : *randompad++;

	/* A zero to separate things */
	out [i++] = 0;

	/* Now ASN stuff if MIC (signature) */
	if (mic_flag)
		for (j=0; j<sizeof(asn_array); )
			out[i++] = asn_array[j++];

	/* Now user data */
	while (bytecount--)
		out[i++] = *inbuf++;

	/* End of blocking logic */

	mp_move(outreg,(unitptr)out);
	mp_burn((unitptr)out); /* burn the evidence on the stack */
	mp_convert_order((byte *)outreg);	/* convert outreg to INTERNAL byte order */
	return(remaining);	/* less than 0 if there was padding */
}	/* preblock */

#else /* ! PKCS_COMPAT */

short preblock(unitptr outreg, byteptr inbuf, short bytecount,
	unitptr modulus, byteptr randompad)
/*	Converts plaintext block into form suitable for RSA encryption.
	Converts to INTERNAL byte order.
	Returns # of bytes remaining to process.  Note that the same buffer 
	address may be used for both outreg and inbuf.
	randompad is a pointer to a buffer of random pad bytes to use for 
	padding material, or NULL iff we want to use constant padding.
*/
{	byte out[MAX_BYTE_PRECISION];
	short byte_precision,leading_zeros,remaining,blocksize;
	int i;

	byte_precision = units2bytes(global_precision);
	leading_zeros = byte_precision - countbytes(modulus) + 1;
	blocksize = byte_precision - leading_zeros;
	/* note that blocksize includes data plus pad bytes, if any */

	remaining = bytecount - blocksize;
	if (remaining>=0)
		bytecount = blocksize;
	i = 0;

/* Assume MSB external byte ordering */
	while (leading_zeros--)	/* assumes MSB first */
		out[i++] = 0;

	while (bytecount--)		/* copy user data */
		out[i++] = *inbuf++;

	out	[i++] = 0;			/* Always start with a 0 for padding */

/* Assume MSB external byte ordering */
	/* Pad with either 0xff or values from randompad */
	while (i < byte_precision - 1)
		out[i++] = randompad ? *randompad++ : 0xff;

	/* End with type byte, which we deduce from randompad */
	out[i++] = randompad ? CK_ENCRYPTED_BYTE : MD_ENCRYPTED_BYTE;

	/* End of padding logic */

	mp_move(outreg,(unitptr)out);
	mp_burn((unitptr)out); /* burn the evidence on the stack */
	mp_convert_order((byte *)outreg);	/* convert outreg to INTERNAL byte order */
	return(remaining);	/* less than 0 if there was padding */
}	/* preblock */
#endif /* PKCS_COMPAT */


short postunblock(byteptr outbuf, unitptr inreg, unitptr modulus)
/*	Converts a just-decrypted RSA block back into unblocked plaintext form.
	Converts to EXTERNAL byte order.
	See the notes on preblocking in the preblock routine above.
	Note that outbuf must be at least as large as inreg.
	The same buffer address may be used for both outbuf and inreg.
	Returns positive bytecount of plaintext, or negative error status.
	-1 is bad packet; -2 is unrecognized digest algorithm.
*/
{	short i,j,byte_precision,leading_zeros,bytecount;
	short	blocksize;
	boolean constpad;

	byte_precision = units2bytes(global_precision);
	leading_zeros = byte_precision - countbytes(modulus) + 1;
	blocksize = byte_precision - leading_zeros;
	/* note that blocksize includes data plus pad bytes, if any */

	mp_move((unitptr)outbuf,inreg);
	mp_convert_order(outbuf);	/* convert to EXTERNAL byte order */

	/* Determine if it is the PKCS format or the older format.
	 * PKCS keys must be >= 48 bytes (384 bits).  DEK keys have a 2
	 * as the MSB.  Pre-PKCS keys and PKCS MICs have 1 there.  PKCS
	 * MICs have a 0 at a certain point in the ASN string, a point
	 * where non-PKCS strings have non-zero padding.  That's how we
	 * tell.
	 */
	if (byte_precision < 48  ||
		(outbuf[leading_zeros] == 1  &&
		 (outbuf[byte_precision-1]==1 || outbuf[byte_precision-1]==2) &&
		 outbuf[byte_precision-16-ASN_ZERO_END] != 0))
	{ /* Pre-2.3 format */
	/* Check high byte, make sure it's legal, figure out padding type */
/* Assume MSB external byte ordering */
	i = byte_precision - 1;
	if (outbuf[i] == MD_ENCRYPTED_BYTE)
		constpad = 1;
	else if (outbuf[i] == CK_ENCRYPTED_BYTE)
		constpad = 0;
	else
			return(-2);

	/* Scan down for the 0 byte that ends padding */
	while (--i > 0  &&  outbuf[i])
		if (constpad && outbuf[i] != 0xff)
			return(-1);

/* Assume MSB external byte ordering */
	bytecount = i - leading_zeros;
	if (leading_zeros)
		for (i = 0; i < bytecount; ++i)
			outbuf[i] = outbuf[i+leading_zeros];

	/* Zero out high part of buffer to make it look nice */
	while (i < byte_precision)
		outbuf[i++] = 0;
	}
	else
	{ /* PKCS compatible format */
		/* Determine which type it is */
		i = leading_zeros;
		if (outbuf[i] == MD_ENCRYPTED_BYTE)
			constpad = 1;
		else if (outbuf[i] == CK_ENCRYPTED_BYTE)
			constpad = 0;
		else
			return(-1);

		/* Scan for the 0 byte that ends padding */
		while (++i<byte_precision  &&  outbuf[i])
			if (constpad && outbuf[i] != 0xff)
				return(-1);

		i++;

		/* Verify ASN material for MICs */
		if (constpad)
		{	if (memcmp(outbuf+i, asn_array, sizeof(asn_array)) != 0)
				return(-2);
			i += sizeof(asn_array);
		}

		/* Copy remaining material down to the front of the buffer */
		bytecount = byte_precision - i;
		for (j=0; j<bytecount; )
			outbuf[j++] = outbuf[i++];

		/* Zero out high part of buffer to make it look nice */
		while (j < byte_precision)
			outbuf[j++] = 0;
	}

	return(bytecount);	/* normal return */
}	/* postunblock */

/************ end of multiprecision integer I/O library *****************/

