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

/*
 * RSA - modul fuer secure interface
 *
 * rsa_encrypt(), rsa_decrypt(), rsa_sign(), rsa_verify(), hash_sqmodn(),
 * rsa_set_key(), rsa_pkmap()
 */


#include "secure.h"
#include <stdio.h>
#include "arithmetic.h"
#include "rsa.h"

#define N(x)    ((x)->part1)    /* the modulus */
#define E(x)    ((x)->part2)    /* the exponent */
#define ERASEKEY bzero((char *)&K,sizeof(K))
/*---------------------------------------------------------------*
 * statische Variablen fuer restliche input Bloecke und RSA keys *
 *---------------------------------------------------------------*/

static RSAkeys  K;              /* RSA key fields */
static L_NUMBER ACC[MAXLGTH];   /* residual value */
static int      freesize;       /* free bitlen in ACC */

#ifdef __STDC__

static void	unpk	(char *buf, L_NUMBER L[], int len);
static void	bzero	(char *from, int length);
#else
static void	unpk	();
static void	bzero	();
#endif

/*----------------------------------------------get_rsa_key---*/



/***************************************************************
 *
 * Procedure rsa_set_key
 *
 ***************************************************************/
#ifdef __STDC__

RC rsa_set_key(
	char	 *key,
	int	  keytype
)

#else

RC rsa_set_key(
	key,
	keytype
)
char	 *key;
int	  keytype;

#endif

{
        /* initialize static storage */
        KeyBits        *d_key;

        if(keytype) {
                d_key = d_KeyBits((BitString *)key);
                if (d_key == 0)
                        return -1;
        }
        else d_key = (KeyBits *)key;

        INTEGERtoln(&N(d_key), K.pk.n);
        INTEGERtoln(&E(d_key), K.pk.e);

        inttoln(0, K.sk.u);
        freesize = 0;

        /* destroy key conversion */

        if(keytype) {
           bzero(N(d_key).octets, N(d_key).noctets);
           bzero(E(d_key).octets, E(d_key).noctets);
           free(N(d_key).octets);
           free(E(d_key).octets);
           free(d_key);
        }
        return 0;
}
#if !defined(MAC) && !defined(__HP__) && !defined(__STDC__)
#undef N(x)
#undef E(x)
#else
#undef N
#undef E
#endif


/*----------------------------------------------rsa_encrypt---*/
/***************************************************************
 *
 * Procedure rsa_encrypt
 *
 ***************************************************************/
#ifdef __STDC__

RC rsa_encrypt(
	OctetString	 *in,
	BitString	 *out,
	More		  more,
	int		  keysize
)

#else

RC rsa_encrypt(
	in,
	out,
	more,
	keysize
)
OctetString	 *in;
BitString	 *out;
More		  more;
int		  keysize;

#endif

{
        extern L_NUMBER lz_eins[];
        L_NUMBER        L[MAXLGTH], X[MAXLGTH];
        int             r, off, blocksize;
        int             ret = out->nbits;

        blocksize = (keysize - 1) / BYTEL;
        if (freesize)
                r = freesize / BYTEL;
        else
                r = blocksize;
        off = r;

        if (in && (in->noctets > 0)) {
                if (r > in->noctets)
                        r = in->noctets;
                octetstoln(in, L, 0, r);
                if (off > r) {  /* incomplete block */
                        shift(L, (off - r) * BYTEL, L);
                        if (freesize)
                                add(ACC, L, L);
                        if (more == SEC_MORE) {
                                freesize = (off - r) * BYTEL;
                                trans(L, ACC);
                                return 0;
                        }
                } else if (freesize)
                        add(ACC, L, L);
        } else {                /* no input, but check ACC */
                if (freesize && (more == SEC_END))
                        trans(ACC, L)
                else
                        goto finish;
        }
        freesize = 0;

        r = blocksize;
        do {
#ifdef   PAD
                if (comp(L, lz_eins) <= 0) {    /* pad values 0 and 1 */
                        inttoln(1, X);
                        shift(X, lngtouse(K.pk.n), X);
                        add(X, L, L);
                }
#endif
                rsa_encblock(L, X, &K.pk);
                lntobits(X, out, keysize);
                /* load next block, if possible */
                if (off + r > in->noctets)
                        r = in->noctets - off;
                if (r > 0)
                        octetstoln(in, L, off, r);
                off += blocksize;
        } while (off <= in->noctets);

        /* work on rest block in L */
        if (r > 0) {
                shift(L, (blocksize - r) * BYTEL, L);   /* pads 0's */
                if (more == SEC_MORE) {
                        freesize = (blocksize - r) * BYTEL;
                        trans(L, ACC);
                        return out->nbits - ret;
                }
#ifdef   PAD
                if (comp(L, lz_eins) <= 0) {    /* pad values 0 and 1 */
                        inttoln(1, X);
                        shift(X, lngtouse(K.pk.n), X);
                        add(X, L, L);
                }
#endif
                rsa_encblock(L, X, &K.pk);
                lntobits(X, out, keysize);
        }
finish:
        if (more == SEC_END)
                ERASEKEY;
        return out->nbits - ret;
}


/*----------------------------------------------rsa_decrypt---*/
/***************************************************************
 *
 * Procedure rsa_decrypt
 *
 ***************************************************************/
#ifdef __STDC__

RC rsa_decrypt(
	BitString	 *in,
	OctetString	 *out,
	More		  more,
	int		  keysize
)

#else

RC rsa_decrypt(
	in,
	out,
	more,
	keysize
)
BitString	 *in;
OctetString	 *out;
More		  more;
int		  keysize;

#endif

{
        L_NUMBER        L[MAXLGTH], X[MAXLGTH];
        int             r, off, blocksize;
        int             ret = out->noctets;

        blocksize = (keysize - 1) / BYTEL;
        if (freesize)
                r = freesize;
        else
                r = keysize;
        off = r;

        if (in && (in->nbits > 0)) {
                if (r > in->nbits)
                        r = in->nbits;
                bitstoln(in, L, 0, r);
                if (off > r) {  /* incomplete block */
                        shift(L, (off - r), L);
                        if (freesize)
                                add(ACC, L, L);
                        if (more == SEC_MORE) {
                                freesize = (off - r);
                                trans(L, ACC);
                                return 0;
                        }
                } else if (freesize)
                        add(ACC, L, L);
        } else {                /* no input, but check ACC */
                if (freesize && (more == SEC_END))
                        trans(ACC, L)
                else
                        goto finish;
        }
        freesize = 0;

        r = keysize;
        do {
                rsa_decblock(L, X, &K.sk);
                lntoctets(X, out, blocksize);
                /* load next block, if possible */
                if (off + r > in->nbits)
                        r = in->nbits - off;
                if (r > 0)
                        bitstoln(in, L, off, r);
                off += keysize;
        } while (off <= in->nbits);

        /* work on rest block in L */
        if (r > 0) {
                shift(L, (keysize - r), L);     /* pads 0's */
                if (more == SEC_MORE) {
                        freesize = (keysize - r);
                        trans(L, ACC);
                        return out->noctets - ret;
                }
                rsa_decblock(L, X, &K.sk);
                lntoctets(X, out, blocksize);
        }
finish:
        if (more == SEC_END)
                ERASEKEY;
        return out->noctets - ret;
}



/***************************************************************
 *
 * Procedure unpk
 *
 ***************************************************************/
#ifdef __STDC__

static void unpk(
	char		 *buf,
	L_NUMBER	  L[],
	int		  len
)

#else

static void unpk(
	buf,
	L,
	len
)
char		 *buf;
L_NUMBER	  L[];
int		  len;

#endif

{
        char            hash[MAXLGTH * WBYTES];
        char           *out;
        char             b;
        int             cnt = len;
        OctetString     ostr;

        out = hash;
        for (; len > 0; len--) {
                b = *buf++;
                *out = (b >> 4) | 0xF0;
                 out++;
                *out = (b & 0x0F) | 0xF0;
                 out++;
        }
        ostr.noctets = cnt << 1;
        ostr.octets = hash;
        INTEGERtoln(&ostr, L);
        return;
}


/***************************************************************
 *
 * Procedure hash_sqmodn
 *
 ***************************************************************/
#ifdef __STDC__

RC hash_sqmodn(
	OctetString	 *in,
	OctetString	 *sum,
	More		  more,
	int		  keysize
)

#else

RC hash_sqmodn(
	in,
	sum,
	more,
	keysize
)
OctetString	 *in;
OctetString	 *sum;
More		  more;
int		  keysize;

#endif

{
        extern L_NUMBER lz_eins[];
        L_NUMBER        L[MAXLGTH], X[MAXLGTH];
        int             r, off, blocksize;

        blocksize = (keysize - 1) / (BYTEL << 1);
        if (freesize)
                r = freesize / (BYTEL << 1);
        else
                r = blocksize;
        off = r;

        if (in && (in->noctets > 0)) {
                if (r > in->noctets)
                        r = in->noctets;
                unpk(in->octets, L, r);

                if (off > r) {  /* incomplete block */
                        shift(L, (off - r) * (BYTEL << 1), L);
                        if (freesize)
                                add(ACC, L, L);
                        if (more == SEC_MORE) {
                                freesize = (off - r) * (BYTEL << 1);
                                trans(L, ACC);
                                return 0;
                        } else {/* pad with 1's */
                                inttoln(1, X);
                                shift(X, (off - r) * (BYTEL << 1), X);
                                sub(X, lz_eins, X);
                                add(X, L, L);
                        }
                } else if (freesize)
                        add(ACC, L, L);
        } else {                /* no input, but check ACC */
                if (!freesize || (more == SEC_MORE))
                        goto finish2;
                trans(ACC, L);
                if (more == SEC_END) {      /* needs padding */
                        inttoln(1, X);
                        shift(X, freesize, X);
                        sub(X, lz_eins, X);
                        add(X, L, L);
                }
        }
        freesize = 0;

        INTEGERtoln(sum, X);    /* load last result */
        r = blocksize;
        do {
                xor(L, X, X);   /* add ... */
                mmult(X, X, X, K.pk.n); /* ... and square */
                /* load next block, if possible */
                if (off + r > in->noctets)
                        r = in->noctets - off;
                if (r > 0)
                        unpk(in->octets + off, L, r);
                off += blocksize;
        } while (off <= in->noctets);

        /* work on rest block in L */
        if (r > 0) {
                shift(L, (blocksize - r) * (BYTEL << 1), L);    /* pads 0's */
                if (more == SEC_MORE) {
                        freesize = (blocksize - r) * (BYTEL << 1);
                        trans(L, ACC);
                        goto finish;
                } else {        /* pad 1's */
                        inttoln(1, ACC);
                        shift(ACC, (blocksize - r) * (BYTEL << 1), ACC);
                        sub(ACC, lz_eins, ACC);
                        add(ACC, L, L);
                }
                xor(L, X, X);   /* add ... */
                mmult(X, X, X, K.pk.n); /* ... and square */
        }
finish:
        lntoINTEGER(X, sum);

finish2:
        if (more == SEC_END)
                ERASEKEY;
        return 0;
}


/*---------------------------------------------rsa_sign-------*/
/***************************************************************
 *
 * Procedure rsa_sign
 *
 ***************************************************************/
#ifdef __STDC__

RC rsa_sign(
	OctetString	 *hash,
	BitString	 *sign
)

#else

RC rsa_sign(
	hash,
	sign
)
OctetString	 *hash;
BitString	 *sign;

#endif

{
        L_NUMBER        L[MAXLGTH];
        L_NUMBER        S[MAXLGTH];

        if(MAXLGTH * WLNG < hash->noctets * BYTEL) return(-1);

        sign->nbits = 0;
        INTEGERtoln(hash, L);
        rsa_decblock(L, S, &K.sk);
        lntobits(S, sign, (lngtouse(S) + BYTEL) & (~(BYTEL - 1)));
        ERASEKEY;
        return 0;
}


/*---------------------------------------------rsa_verify-----*/
#include <stdio.h>
/***************************************************************
 *
 * Procedure rsa_verify
 *
 ***************************************************************/
#ifdef __STDC__

RC rsa_verify(
	OctetString	 *hash,
	BitString	 *sign
)

#else

RC rsa_verify(
	hash,
	sign
)
OctetString	 *hash;
BitString	 *sign;

#endif

{
        L_NUMBER        L[MAXLGTH];
        L_NUMBER        C[MAXLGTH];
	OctetString     *ostr;


        if(MAXLGTH * WLNG < sign->nbits) return(-1);
        if(MAXLGTH * WLNG < hash->noctets * BYTEL) return(-1);

        bitstoln(sign, L, 0, sign->nbits);
        rsa_encblock(L, C, &K.pk);
        INTEGERtoln(hash, L);
        ERASEKEY;
	if(sec_verbose) {
		ostr = (OctetString *)malloc(sizeof(OctetString));
		ostr->octets = (char *)malloc(512);
		lntoctets(C, ostr, 0);
		aux_fprint_OctetString(stderr, ostr);
		aux_free_OctetString(&ostr);
	}
        if (comp(L, C))
                return -1;
        else
                return 0;
}

/*---------------------------------------------rsa_encblock2OctetString-----*/
/***************************************************************
 *
 * Procedure rsa_encblock2OctetString
 *
 ***************************************************************/
#ifdef __STDC__

RC rsa_encblock2OctetString(
	BitString	 *sign,
	OctetString	 *hash
)

#else

RC rsa_encblock2OctetString(
	sign,
	hash
)
BitString	 *sign;
OctetString	 *hash;

#endif

{
        L_NUMBER        L[MAXLGTH];
        L_NUMBER        C[MAXLGTH];

        if(MAXLGTH * WLNG < sign->nbits) return(-1);
        if(MAXLGTH * WLNG < hash->noctets * BYTEL) return(-1);

        bitstoln(sign, L, 0, sign->nbits);
        rsa_encblock(L, C, &K.pk);
        lntoINTEGER(C, hash);
        return 0;
}

/***************************************************************
 *
 * Procedure bzero
 *
 ***************************************************************/
#ifdef __STDC__

static void bzero(
	char	 *from,
	int	  length
)

#else

static void bzero(
	from,
	length
)
char	 *from;
int	  length;

#endif

{
   int i;
   for (i=0; i<length; i++)
    *from++ = 0x00;

}
