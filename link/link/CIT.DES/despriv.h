/*
 *  despriv.h - specifies private defs for the DES encryption library.
 *
 *	RCS ID: $Header: despriv.h,v 2.1 90/07/18 10:57:34 lpb Exp $
 *
 *  Author: Lawrence Brown <lpb@cs.adfa.oz.au>      Dec 1987
 *      Computer Science, UC UNSW, Australian Defence Force Academy,
 *          Canberra, ACT 2600, Australia.
 */

#if defined(LITTLE_ENDIAN) && !defined(STRICT_ALIGN)
/*
 *	bswap(b) - exchanged bytes in each longword of a 64-bit data block
 *			on little-endian machines where byte order is reversed
 */

#define bswap(cb) {				\
	register char	c;			\
	c = cb[0]; cb[0] = cb[3]; cb[3] = c;	\
	c = cb[1]; cb[1] = cb[2]; cb[2] = c;	\
	c = cb[4]; cb[4] = cb[7]; cb[7] = c;	\
	c = cb[5]; cb[5] = cb[6]; cb[6] = c;	\
}
#endif

