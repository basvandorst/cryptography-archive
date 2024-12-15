/*
 *  SecuDE Release 4.3 (GMD)
 */
/********************************************************************
 * Copyright (C) 1994, GMD. All rights reserved.                    *
 *                                                                  *
 *                                                                  *
 *                         NOTICE                                   *
 *                                                                  *
 *    Acquisition, use, and distribution of this module             *
 *    and related materials are subject to restrictions             *
 *    mentioned in each volume of the documentation.                *
 *                                                                  *
 ********************************************************************/


#include "secure.h"
#include "arithmetic.h"

#ifdef MAC
#include "Mac.h"
#endif /* MAC */


#include <memory.h>

/*-------------------------------------------------------------*/
/*   globale Variablen - Definitionen                          */
/*-------------------------------------------------------------*/


L_NUMBER  lz_null    [] = LZ_NULL   ;
L_NUMBER  lz_eins    [] = LZ_EINS   ;
L_NUMBER  lz_zwei    [] = LZ_ZWEI   ;
L_NUMBER  lz_fermat5 [] = LZ_FERMAT5;




#ifdef ARITHMETIC_TEST

FILE *arithmetic_trace = stdout;

int arithmetic_trace_counter = 0;

/***************************************************************
 *
 * Procedure arithmetic_trace_print
 *
 ***************************************************************/
#ifdef __STDC__

void arithmetic_trace_print(
	L_NUMBER	  ln[]
)

#else

void arithmetic_trace_print(
	ln
)
L_NUMBER	  ln[];

#endif

{
	int n = ln[0], m;
	char hex[9];

	if(!n) {
		fprintf(arithmetic_trace, "< 0 >\n");
		return;
	}
	
	fprintf(arithmetic_trace, "< %x", ln[n--]);

	while(n > 0) {

#ifdef WLNG32
		sprintf(hex, "%8x", ln[n--]);
		for(m = 0; hex[m] == ' '; m++) hex[m] = '0';
		fprintf(arithmetic_trace, " %s", hex);
#else
		sprintf(hex, "%4x", ln[n--]);
		for(m = 0; hex[m] == ' '; m++) hex[m] = '0';
		if(n & 1) fprintf(arithmetic_trace, " %s", hex);
		else fprintf(arithmetic_trace, "%s", hex);
#endif

	}

	fprintf(arithmetic_trace, " >\n");

	return;
}
#endif /* ARITHMETIC_TEST */




/* function intlog2() ==  int( log2() )
 */

/***************************************************************
 *
 * Procedure intlog2
 *
 ***************************************************************/
#ifdef __STDC__

int intlog2(
	unsigned int	  v
)

#else

int intlog2(
	v
)
unsigned int	  v;

#endif

{
	int     ret;
	for( ret = -1; v; v >>= 1, ret++) ;
	return ret;
}


/*
 * function lngtouse(modul) RETURNS BITLEN
 *
 * ::= mu(y: 2*2^y > modul)
 *
 */


/***************************************************************
 *
 * Procedure lngtouse
 *
 ***************************************************************/
#ifdef __STDC__

int lngtouse(
	L_NUMBER	  modul[]
)

#else

int lngtouse(
	modul
)
L_NUMBER	  modul[];

#endif

{
	register L_NUMBER	x;
	register int	i;
	
	if(!modul[0]) return(-1);
	i = (modul[0]-1) << SWBITS;
	x = modul[modul[0]];

	return i + intlog2(x);
}


/*
 *      lntoINTEGER ( LNUMBER, INTEGER )
 *      INTEGERtoln ( INTEGER, LNUMBER )
 *      lntoctets ( LNUMBER, OctetString, size ) append
 *      octetstoln ( OctetString, LNUMBER, offset, size )
 *      lntobits ( LNUMBER, BitString, size ) append
 *      bitstoln ( BitString, LNUMBER, offset, size )
 *
 */


/* get byte order routines ntohl, ntohs, htonl, htons from <netinet/in.h> */

#include <sys/types.h>
#include <netinet/in.h>



/***************************************************************
 *
 * Procedure octetstoln
 *
 ***************************************************************/
#ifdef __STDC__

void octetstoln(
	OctetString	 *octs,
	L_NUMBER	  lnum[],
	int		  offset,
	int		  size
)

#else

void octetstoln(
	octs,
	lnum,
	offset,
	size
)
OctetString	 *octs;
L_NUMBER	  lnum[];
int		  offset;
int		  size;

#endif

{
	int	length;		/* number of words */
	L_NUMBER *wp, c;
	char	*in;
	int     r;

	in = octs->octets + offset;
	length = (size + WBYTES-1)/WBYTES;
	lnum[0] = length;

	wp = lnum+length;
	r  = size%WBYTES;
	if ( r > 0 ) {
		c = 0;
		memcpy((char *)&c, in, r);      in += r;
		c = ntohl(c) >> ((WBYTES-r)*BYTEL);
		*wp-- = c;
	}

	while( wp > lnum ) {
		memcpy((char *)&c, in, WBYTES);
		*wp-- = ntohl(c);
		in += WBYTES;
	}

	normalize(lnum);
	return;
}


/***************************************************************
 *
 * Procedure lntoctets
 *
 ***************************************************************/
#ifdef __STDC__

void lntoctets(
	L_NUMBER	  lnum[],
	OctetString	 *octs,
	int		  size
)

#else

void lntoctets(
	lnum,
	octs,
	size
)
L_NUMBER	  lnum[];
OctetString	 *octs;
int		  size;

#endif

{
	int     nw, r;
	char	*out;
	L_NUMBER *wp, c;

	if (size == 0) {
		size = (lngtouse(lnum) + BYTEL)/BYTEL;
		out = octs->octets;
		octs->noctets = size;
	}
	else {
		out = octs->octets + octs->noctets;     /* append */
		octs->noctets += size;
	}
	nw = size/WBYTES;

	c = 0;
	for( ; nw > lngofln(lnum); nw-- ){      /* fill zero */
		memcpy(out, (char *)&c, WBYTES);
		out += WBYTES;
	}

	wp = lnum + nw;
	r = size%WBYTES;
	if (r>0) {
		if (nw < lngofln(lnum))
			c = htonl(*(wp+1));
		else    c = 0;
		memcpy(out, ((char*)&c)+WBYTES-r, r);       out += r;
	}

	while( wp > lnum )  { /* copy all except the length field */
		c = htonl( *wp-- );
		memcpy(out, (char*)&c, WBYTES);
		out += WBYTES;
	}

	return;
}

/***************************************************************
 *
 * Procedure bitstoln
 *
 ***************************************************************/
#ifdef __STDC__

void bitstoln(
	BitString	 *bits,
	L_NUMBER	  lnum[],
	int		  offset,
	int		  size
)

#else

void bitstoln(
	bits,
	lnum,
	offset,
	size
)
BitString	 *bits;
L_NUMBER	  lnum[];
int		  offset;
int		  size;

#endif

{
	OctetString     b;
	char    save;           /* 1. octet may be masked */
	int     r;

	b.noctets = (offset + size + BYTEL - 1)/BYTEL
		   - offset/BYTEL;
	b.octets = bits->bits + offset/BYTEL;

	r = offset%BYTEL;
	if (r>0) {
		save = *b.octets;
		*b.octets &= 0xFF>>r;
	}
	INTEGERtoln(&b,lnum);
	if(r>0) *b.octets = save;       /* restore */

	r = (offset + size)%BYTEL;      /* adjust */
	if (r>0) shift(lnum, r-BYTEL, lnum);
	return;
}

/***************************************************************
 *
 * Procedure lntobits
 *
 ***************************************************************/
#ifdef __STDC__

void lntobits(
	L_NUMBER	  lnum[],
	BitString	 *bits,
	int		  size
)

#else

void lntobits(
	lnum,
	bits,
	size
)
L_NUMBER	  lnum[];
BitString	 *bits;
int		  size;

#endif

{
	OctetString     b;
	int     r,c;
	char    save;

	b.noctets = 0;
	b.octets = bits->bits + bits->nbits/BYTEL;

	r = (bits->nbits + size)%BYTEL; /* adjust */
	if (r>0) shift(lnum, BYTEL-r, lnum);

	r = bits->nbits%BYTEL;
	if (r>0) save = *b.octets;

	c = (bits->nbits + size + BYTEL -1)/BYTEL - bits->nbits/BYTEL;
	lntoctets(lnum,&b,c);
	bits->nbits += size;
	if (r>0) *b.octets = (*b.octets & 0xFF>>r) | save;

	return;
}

#ifdef MS_DOS
/* has need for ntohl(), htonl() functions */

#ifdef WLNG16
/***************************************************************
 *
 * Procedure ntohl
 *
 ***************************************************************/
#ifdef __STDC__

L_NUMBER ntohl(
	L_NUMBER	  x
)

#else

L_NUMBER ntohl(
	x
)
L_NUMBER	  x;

#endif

{
union {
	L_NUMBER i;
	unsigned char s[2];
}	v;
unsigned char	b;

	v.i = x;
	/* swap byte order */
	b = v.s[0];
	v.s[0] = v.s[1];
	v.s[1] = b;

	return v.i;
}
#endif /* WLNG16 */
#endif





/***************************************************************
 *
 * Procedure aux_LN2BitString
 *
 ***************************************************************/
#ifdef __STDC__

BitString *aux_LN2BitString(
	L_NUMBER	  lnum[],
	int		  size
)

#else

BitString *aux_LN2BitString(
	lnum,
	size
)
L_NUMBER	  lnum[];
int		  size;

#endif

{
	char      *proc = "aux_LN2BitString";
	BitString *bitstring;
	int        length = lngtouse(lnum) + 1;

	if(!size) size = length;
	else if(size < length) {
		aux_add_error(EINVALID, "number is too big", CNULL, 0, proc);
		return((BitString *)0);
	}
	if(!(bitstring = (BitString *) calloc(1, sizeof(BitString)))) {
		aux_add_error(EMALLOC, "bitstring", CNULL, 0, proc);
		return((BitString *)0);
	}
	if(!(bitstring->bits = malloc(size / BYTEL + 1))) {
		aux_add_error(EMALLOC, "bitstring->bits", CNULL, 0, proc);
		return((BitString *)0);
	}
	bitstring->nbits = 0;

	lntobits(lnum, bitstring, size);

	return(bitstring);
}
/***************************************************************
 *
 * Procedure aux_LN2OctetString
 *
 ***************************************************************/
#ifdef __STDC__

OctetString *aux_LN2OctetString(
	L_NUMBER	  lnum[],
	int		  size
)

#else

OctetString *aux_LN2OctetString(
	lnum,
	size
)
L_NUMBER	  lnum[];
int		  size;

#endif

{
	char        *proc = "aux_LN2OctetString";
	OctetString *octetstring;
	int          length = (lngtouse(lnum) + 8) / 8;

	if(!size) size = length;
	else if(size < length) {
		aux_add_error(EINVALID, "number is too big", CNULL, 0, proc);
		return((OctetString *)0);
	}
	if(!(octetstring = (OctetString *) calloc(1, sizeof(OctetString)))) {
		aux_add_error(EMALLOC, "octetstring", CNULL, 0, proc);
		return((OctetString *)0);
	}
	if(!(octetstring->octets = malloc(size))) {
		aux_add_error(EMALLOC, "octetstring->octets", CNULL, 0, proc);
		return((OctetString *)0);
	}
	octetstring->noctets = 0;

	lntoctets(lnum, octetstring, size);

	return(octetstring);
}
/***************************************************************
 *
 * Procedure aux_OctetString2LN
 *
 ***************************************************************/
#ifdef __STDC__

L_NUMBER *aux_OctetString2LN(
	OctetString	  *ostr
)

#else

L_NUMBER *aux_OctetString2LN(
	ostr
)
OctetString	  *ostr;

#endif

{
	char        *proc = "aux_OctetString2LN";
	L_NUMBER    *lnum;

	if(!ostr) {
		aux_add_error(EINVALID, "No OctetString given", CNULL, 0, proc);
		return((L_NUMBER *)0);
	}
	if(!(lnum = (L_NUMBER *)calloc(MAXGENL, sizeof(int)))) {
		aux_add_error(EMALLOC, "lnum", CNULL, 0, proc);
		return((L_NUMBER *)0);
	}
	octetstoln(ostr, lnum, 0, ostr->noctets);
	return(lnum);
}
/***************************************************************
 *
 * Procedure aux_BitString2LN
 *
 ***************************************************************/
#ifdef __STDC__

L_NUMBER *aux_BitString2LN(
	BitString	  *bstr
)

#else

L_NUMBER *aux_BitString2LN(
	bstr
)
BitString	  *bstr;

#endif

{
	char        *proc = "aux_BitString2LN";
	L_NUMBER    *lnum;

	if(!bstr) {
		aux_add_error(EINVALID, "No BitString given", CNULL, 0, proc);
		return((L_NUMBER *)0);
	}
	if(!(lnum = (L_NUMBER *)calloc(MAXGENL, sizeof(int)))) {
		aux_add_error(EMALLOC, "lnum", CNULL, 0, proc);
		return((L_NUMBER *)0);
	}
	bitstoln(bstr, lnum, 0, bstr->nbits);
	return(lnum);
}













/***************************************************************
 *
 * Procedure aux_LN2BitString2
 *
 ***************************************************************/
#ifdef __STDC__

RC aux_LN2BitString2(
	BitString	 *bitstring,
	L_NUMBER	  lnum[],
	int		  size
)

#else

RC aux_LN2BitString2(
	bitstring,
	lnum,
	size
)
BitString	 *bitstring;
L_NUMBER	  lnum[];
int		  size;

#endif

{
	char      *proc = "aux_LN2BitString2";
	int        length = lngtouse(lnum) + 1;

	if(!size) size = length;
	else if(size < length) {
		aux_add_error(EINVALID, "number is too big", CNULL, 0, proc);
		return(-1);
	}
	if(!(bitstring->bits = malloc(size / BYTEL + 1))) {
		aux_add_error(EMALLOC, "bitstring->bits", CNULL, 0, proc);
		return(-1);
	}
	bitstring->nbits = 0;

	lntobits(lnum, bitstring, size);

	return(0);
}
/***************************************************************
 *
 * Procedure aux_LN2OctetString2
 *
 ***************************************************************/
#ifdef __STDC__

RC aux_LN2OctetString2(
	OctetString	 *octetstring,
	L_NUMBER	  lnum[],
	int		  size
)

#else

RC aux_LN2OctetString2(
	octetstring,
	lnum,
	size
)
OctetString	 *octetstring;
L_NUMBER	  lnum[];
int		  size;

#endif

{
	char        *proc = "aux_LN2OctetString2";
	int          length = (lngtouse(lnum) + 8) / 8;

	if(!size) size = length;
	else if(size < length) {
		aux_add_error(EINVALID, "number is too big", CNULL, 0, proc);
		return(-1);
	}
	if(!(octetstring->octets = malloc(size))) {
		aux_add_error(EMALLOC, "octetstring->octets", CNULL, 0, proc);
		return(-1);
	}
	octetstring->noctets = 0;

	lntoctets(lnum, octetstring, size);

	return(0);
}
/***************************************************************
 *
 * Procedure aux_OctetString2LN
 *
 ***************************************************************/
#ifdef __STDC__

RC aux_OctetString2LN2(
	L_NUMBER    	  *lnum,
	OctetString	  *ostr
)

#else

RC aux_OctetString2LN2(
	lnum,
	ostr
)
L_NUMBER    	  *lnum;
OctetString	  *ostr;

#endif

{
	char        *proc = "aux_OctetString2LN2";

	if(!ostr) {
		aux_add_error(EINVALID, "No OctetString given", CNULL, 0, proc);
		return(-1);
	}
	octetstoln(ostr, lnum, 0, ostr->noctets);
	return(0);
}
/***************************************************************
 *
 * Procedure aux_BitString2LN
 *
 ***************************************************************/
#ifdef __STDC__

RC aux_BitString2LN2(
	L_NUMBER    	  *lnum,
	BitString	  *bstr
)

#else

RC aux_BitString2LN2(
	lnum,
	bstr
)
BitString	  *bstr;
L_NUMBER    	  *lnum;

#endif

{
	char        *proc = "aux_BitString2LN2";

	if(!bstr) {
		aux_add_error(EINVALID, "No BitString given", CNULL, 0, proc);
		return(-1);
	}
	bitstoln(bstr, lnum, 0, bstr->nbits);
	return(0);
}
