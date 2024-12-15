/* pgparmor.c */

/* Most of the low-level code is ripped out of armor.c from pgp
   and then I wrote a new high-level wrapper (pgp_create_armor and
   pgp_extract_armor) around it. Those low-level functions which
   accessed files were modified to access fifos instead. */

#include <stdio.h>
#include "usuals.h"
#include "fifo.h"
#include "pgparmor.h"

#define CRCBITS 24      /* may be 16, 24, or 32 */
#define LINELEN 48L     /* 48 input bytes = 64 ascii armor bytes */
/* #define maskcrc(crc) ((crcword)(crc)) */     /* if CRCBITS is 16 or 32 */
#define maskcrc(crc) ((crc) & 0xffffffL)        /* if CRCBITS is 24 */
#define CRCHIBIT ((crcword) (1L<<(CRCBITS-1))) /* 0x8000 if CRCBITS is 16 */
#define CRCSHIFTS (CRCBITS-8)

#define CCITTCRC 0x1021 /* CCITT's 16-bit CRC generator polynomial */
#define PRZCRC 0x864cfbL /* PRZ's 24-bit CRC generator polynomial */
#define CRCINIT 0xB704CEL       /* Init value for CRC accumulator */

/* ENC is the basic 1 character encoding function to
   make a char printing */

#define ENC(c) ((int)bintoasc[((c) & 077)])
#define PAD             '='

static void mk_crctbl(crcword poly);
static crcword crcbytes(byte *buf,unsigned len,register crcword accum);
static void outdec(byte *p,struct fifo *f,unsigned count);
static void outcrc (word32 crc,struct fifo *f);
static int dpem_buffer(char *inbuf, char *outbuf, unsigned *outlength);
static unsigned fifo_getarmor(struct fifo *f,byte *buf,
			      unsigned length,int *pad,int ignore);

/* Index this array by a 6 bit value to get the character corresponding
 * to that value.
 */
static
byte bintoasc[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ\
abcdefghijklmnopqrstuvwxyz0123456789+/";

/* Index this array by a 7 bit value to get the 6-bit binary field
 * corresponding to that value.  Any illegal characters return high bit set.
 */
static
byte asctobin[] = {
	0200,0200,0200,0200,0200,0200,0200,0200,
	0200,0200,0200,0200,0200,0200,0200,0200,
	0200,0200,0200,0200,0200,0200,0200,0200,
	0200,0200,0200,0200,0200,0200,0200,0200,
	0200,0200,0200,0200,0200,0200,0200,0200,
	0200,0200,0200,0076,0200,0200,0200,0077,
	0064,0065,0066,0067,0070,0071,0072,0073,
	0074,0075,0200,0200,0200,0200,0200,0200,
	0200,0000,0001,0002,0003,0004,0005,0006,
	0007,0010,0011,0012,0013,0014,0015,0016,
	0017,0020,0021,0022,0023,0024,0025,0026,
	0027,0030,0031,0200,0200,0200,0200,0200,
	0200,0032,0033,0034,0035,0036,0037,0040,
	0041,0042,0043,0044,0045,0046,0047,0050,
	0051,0052,0053,0054,0055,0056,0057,0060,
	0061,0062,0063,0200,0200,0200,0200,0200
};

static int crc_init=FALSE; /* To keep track of whether the CRC table
			      has been initialized yet */
							      
/* Ascii armors data up to end of i or num lines, does not destroy i */
/* If lines is zero, will go until end of i */
unsigned pgp_create_armor(struct fifo *i,struct fifo *o,unsigned lines)
{
byte buf[LINELEN];
byteptr bp;
unsigned j,n,numlines;
crcword crc;

if(!crc_init) {
  crc_init=TRUE;
  mk_crctbl(PRZCRC);
  }
numlines=0;
crc=CRCINIT;
while((n=fifo_aget(buf,LINELEN,i))) {
  crc=crcbytes(buf,n,crc);
  bp=buf;
  while(n>=3) {
    outdec(bp,o,3);
    bp+=3;
    n-=3;
    }
  if(n==1) {
    bp[1]='\0';
    bp[2]='\0';
    outdec(bp,o,1);
    }
  else if (n==2) {
    bp[2]='\0';
    outdec(bp,o,2);
    }
  fifo_put('\n',o);
  numlines++;
  if(lines&&(numlines==lines))
    break;
  }
outcrc(crc,o);
return(numlines);
}

/* Decodes ascii armor up to end of packet (crc) */
/* Returns TRUE if CRC good, FALSE if premature end,
   bad data, or bad CRC */
int pgp_extract_armor(struct fifo *i,struct fifo *o)
{
byte inbuf[LINELEN*4/3+1];
byte outbuf[LINELEN];
crcword crc;
crcword checkcrc;
unsigned n;
int pad;
if(!crc_init) {
  crc_init=TRUE;
  mk_crctbl(PRZCRC);
  }
crc=CRCINIT;
pad=FALSE;
while(!pad) {
  n=fifo_getarmor(i,inbuf,sizeof(inbuf)-1,&pad,FALSE);
  if(n==0)
    break; /* In case it ends right at the end of the line */
  if((n<sizeof(inbuf)-1)&&(!pad))
    return(FALSE); /* Premature end, no CRC */
  while((n%4)&&(n<sizeof(inbuf)-1)) {
    inbuf[n]='=';
    n++;
    }
  inbuf[n]='\0';
  if(dpem_buffer((char *)inbuf,(char *)outbuf,&n))
    return(FALSE); /* Error in ascii armor */
  fifo_aput(outbuf,n,o);  
  crc=crcbytes(outbuf,n,crc);
  }
n=fifo_getarmor(i,inbuf,4,&pad,TRUE);
/* Begin kludge */
if((n==2)&&(inbuf[0]=='3')&&(inbuf[1]=='D')) {
  do {
    n=fifo_getarmor(i,inbuf,6,&pad,TRUE);
    } while((n==2)&&(inbuf[0]=='3')&&(inbuf[1]=='D'));
  for(n=0;n<4;n++)
    inbuf[n]=inbuf[n+2];
  } else if((n==4)&&(inbuf[0]=='3')&&(inbuf[1]=='D')&&
	    (fifo_rget(0,i)>33)) {
    inbuf[0]=inbuf[2];
    inbuf[1]=inbuf[3];
    fifo_getarmor(i,inbuf+2,2,&pad,FALSE);
    } /* End kludge to get rid of MIME mangling ( = -> =3D ) */
inbuf[4]='\0';
dpem_buffer((char *)inbuf,(char *)outbuf,&n);
if(n!=3)
  return(FALSE); /* Couldn't read CRC */
checkcrc=outbuf[0];
checkcrc<<=8;
checkcrc|=outbuf[1];
checkcrc<<=8;
checkcrc|=outbuf[2];
if(crc==checkcrc)
  return(TRUE);
else
  return(FALSE);
}

/* Gets data from an ascii armored file. Gets rid of all whitespace
   characters. Sets *pad to true and ends if it runs into a pad
   character, except it ignores pads at the beginning if allpad is
   true. This allows reading of crc's */

static unsigned fifo_getarmor(struct fifo *f,byte *buf,
			      unsigned length,int *pad,int allpad)
{
int c;
unsigned left=length;
*pad=FALSE;
while(left) {
  c=fifo_get(f);
  if(c<0)
    break;
  if(c<33||c>127)
    continue;
  if(allpad) {
    if(c==PAD)
      continue;
    else
      allpad=FALSE;
    }
  if(c==PAD) {
    *pad=TRUE;
    break;
    }
  *(buf++)=c;
  left--;
  }
return(length-left);
}

     
/*      Notes on making a good 24-bit CRC--
	The primitive irreducible polynomial of degree 23 over GF(2),
	040435651 (octal), comes from Appendix C of "Error Correcting Codes,
	2nd edition" by Peterson and Weldon, page 490.  This polynomial was
	chosen for its uniform density of ones and zeros, which has better
	error detection properties than polynomials with a minimal number of
	nonzero terms.  Multiplying this primitive degree-23 polynomial by
	the polynomial x+1 yields the additional property of detecting any
	odd number of bits in error, which means it adds parity.  This 
	approach was recommended by Neal Glover.

	To multiply the polynomial 040435651 by x+1, shift it left 1 bit and
	bitwise add (xor) the unshifted version back in.  Dropping the unused 
	upper bit (bit 24) produces a CRC-24 generator bitmask of 041446373 
	octal, or 0x864cfb hex.  

	You can detect spurious leading zeros or framing errors in the 
	message by initializing the CRC accumulator to some agreed-upon 
	nonzero "random-like" value, but this is a bit nonstandard.  
*/

static
crcword crctable[256];          /* Table for speeding up CRC's */

/*      mk_crctbl derives a CRC lookup table from the CRC polynomial.
	The table is used later by the crcbytes function given below.
	mk_crctbl only needs to be called once at the dawn of time.

	The theory behind mk_crctbl is that table[i] is initialized
	with the CRC of i, and this is related to the CRC of i>>1,
	so the CRC of i>>1 (pointed to by p) can be used to derive
	the CRC of i (pointed to by q).
*/
static
void mk_crctbl(crcword poly)
{       int i;
	crcword t, *p, *q;
	p = q = crctable;
	*q++ = 0;
	*q++ = poly;
	for (i = 1; i < 128; i++)
	{       t = *++p;
		if (t & CRCHIBIT)
		{       t <<= 1;
			*q++ = t ^ poly;
			*q++ = t;
		}
		else
		{       t <<= 1;
			*q++ = t;
			*q++ = t ^ poly;
		}
	}
}

/*
 * Accumulate a buffer's worth of bytes into a CRC accumulator,
 * returning the new CRC value.
 */
static
crcword crcbytes(byte *buf, unsigned len, register crcword accum)
{
	do {
		accum = accum<<8 ^
		 crctable[(byte)(accum>>CRCSHIFTS) ^ *buf++];
	} while (--len);
	return maskcrc(accum);
} /* crcbytes */

/*
 * output one group of up to 3 bytes, pointed at by p, on fifo f.
 * if fewer than 3 are present, the 1 or two extras must be zeros.
 */
static void outdec(byte *p,struct fifo *f,unsigned count)
{
	int c1, c2, c3, c4;

	c1 = *p >> 2;
	c2 = ((*p << 4) & 060) | ((p[1] >> 4) & 017);
	c3 = ((p[1] << 2) & 074) | ((p[2] >> 6) & 03);
	c4 = p[2] & 077;
	fifo_put(ENC(c1), f);
	fifo_put(ENC(c2), f);
	if (count == 1)
	{       fifo_put(PAD, f);
		fifo_put(PAD, f);
	}
	else
	{       fifo_put(ENC(c3), f);
		if (count == 2)
			fifo_put(PAD, f);
		else
			fifo_put(ENC(c4), f);
	}
}       /* outdec */

/* Output the CRC value, MSB first per normal CRC conventions */
static void outcrc (word32 crc,struct fifo *f)
{       /* Output crc */
	char crcbuf[4];
	crcbuf[0] = (crc>>16) & 0xff;
	crcbuf[1] = (crc>>8) & 0xff;
	crcbuf[2] = (crc>>0) & 0xff;
	fifo_put(PAD,f);
	outdec((byte *)crcbuf,f,3);
	fifo_put('\n',f);
}       /* outcrc */

/* De-armors a buffer, returning -1 if error, 0 otherwise */
static
int dpem_buffer(char *inbuf, char *outbuf, unsigned *outlength)
{
	byte *bp;
	unsigned int length;
	unsigned int c1,c2,c3,c4;
	register int j;

	length = 0;
	bp = (byte *)inbuf;

	/* FOUR input characters go into each THREE output charcters */

	while (*bp!='\0')
	{       if (*bp&0x80 || (c1=asctobin[*bp])&0x80)
			return -1;
		++bp;
		if (*bp&0x80 || (c2=asctobin[*bp])&0x80)
			return -1;
		if (*++bp == PAD)
		{       c3 = c4 = 0;
			length += 1;
			if (c2 & 15)
				return -1;
			if (strcmp((char *)bp, "==") == 0)
				bp += 1;
			else if (strcmp((char *)bp, "=3D=3D") == 0)
				bp += 5;
			else
				return -1;
		}
		else if (*bp&0x80 || (c3=asctobin[*bp])&0x80)
				return -1;
		else
		{       if (*++bp == PAD)
			{       c4 = 0;
				length += 2;
				if (c3 & 3)
					return -1;
				if (strcmp((char *)bp, "=") == 0)
					; /* All is well */
				else if (strcmp((char *)bp, "=3D") == 0)
					bp += 2;
				else
					return -1;
			}
			else if (*bp&0x80 || (c4=asctobin[*bp])&0x80)
				return -1;
			else
				length += 3;
		}
		++bp;
		j = (c1 << 2) | (c2 >> 4);
		*outbuf++=j;
		j = (c2 << 4) | (c3 >> 2);
		*outbuf++=j;
		j = (c3 << 6) | c4;
		*outbuf++=j;
	}

	*outlength = length;
	return(0);      /* normal return */

}       /* dpem_buffer */
