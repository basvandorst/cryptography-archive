/*
 *  SecuDE Release 4.3 (GMD)
 */
/********************************************************************
 * Copyright (C) 1993, GMD. All rights reserved.                    *
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
 *	DSA - one block signature / verification
 *
 *	it depends on the calling algorithm (resp type of key), which
 *	method is used for computing the block.
 *
 */

#include "arithmetic.h"
#include  "dsa.h"

/***************************************************************
 *
 * Procedure ln_ggt
 *
 ***************************************************************/
#ifdef __STDC__

void ln_ggt(
	L_NUMBER	  ln1[],
	L_NUMBER	  ln2[],
	L_NUMBER	  ggt[]
)

#else

void ln_ggt(
	ln1,
	ln2,
	ggt
)
L_NUMBER	  ln1[];
L_NUMBER	  ln2[];
L_NUMBER	  ggt[];

#endif

{
	L_NUMBER   tmpln1[MAXLGTH], tmpln2[MAXLGTH], *tmpln3;

	trans(ln1, tmpln1);
	trans(ln2, tmpln2);

	do {
		div(tmpln1, tmpln2, tmpln1, tmpln1);
		if(!comp(tmpln1, lz_null)) {
			trans(tmpln2, ggt);
			return;
		}
		div(tmpln2, tmpln1, tmpln2, tmpln2);
	} while (comp(tmpln2, lz_null));

	trans(tmpln1, ggt);
	return;
}
/***************************************************************
 *
 * Procedure ln_inv
 *
 ***************************************************************/
#ifdef __STDC__

void ln_inv(
	L_NUMBER	  ln[],
	L_NUMBER	  modul[],
	L_NUMBER	  res[]
)

#else

void ln_inv(
	ln,
	modul,
	res
)
L_NUMBER	  ln[];
L_NUMBER	  modul[];
L_NUMBER	  res[];

#endif

{
	L_NUMBER   tmpln1[MAXLGTH], tmpln2[MAXLGTH], val1[MAXLGTH], val2[MAXLGTH], tmp[MAXLGTH];

	trans(ln, tmpln1);
	trans(modul, tmpln2);
	trans(lz_eins, val1);
	trans(lz_null, val2);

	while (comp(tmpln1, lz_eins)) {
		if(!comp(tmpln1, lz_null)) {
			res[0] = 0;
			return;
		}

		div(tmpln2, tmpln1, tmp, tmpln2);
		mmult(tmp, val1, tmp, modul);
		if(comp(val2,tmp)<0) add(val2, modul, val2);
		sub(val2, tmp, val2);

		if(!comp(tmpln2, lz_eins)) {
			trans(val2, res);
			return;
		}
		if(!comp(tmpln2, lz_null)) {
			res[0] = 0;
			return;
		}

		div(tmpln1, tmpln2, tmp, tmpln1);
		mmult(tmp, val2, tmp, modul);
		if(comp(val1,tmp)<0) add(val1, modul, val1);
		sub(val1, tmp, val1);
	}

	trans(val1, res);
	return;
}

/***************************************************************
 *
 * Procedure dsa_signblock
 *
 ***************************************************************/
#ifdef __STDC__

void dsa_signblock(
	register L_NUMBER	  in[],
	DSA_sig			 *sig,
	DSA_Skeys		 *key
)

#else

void dsa_signblock(
	in,
	sig,
	key
)
register L_NUMBER	  in[];
DSA_sig			 *sig;
DSA_Skeys		 *key;

#endif

{
	L_NUMBER   k[MAXLGTH], tmp[MAXLGTH], p_1[MAXLGTH];


	sub(key->p, lz_eins, p_1); 	/* p_1 = p-1 */

	do {
		rndm(key->q[0] * WLNG - WLNG + 1, k);

		ln_ggt(k, p_1, tmp);	/* tmp = ggt(k,p-1) */

	} while (comp(tmp, lz_eins));	

	mexp(key->g, k, sig->r, key->p);	/* r = g^k MOD p */
	div(sig->r, key->q, sig->r, sig->r);	/* r = r mod q = (g^k MOD p) MOD q */

	mmult(sig->r, key->x, sig->s, key->q);	/* s = rx MOD q */
	add(sig->s, in, sig->s);		/* s = rx+message MOD q */

	ln_inv(k, key->q, k);

	mmult(sig->s, k, sig->s, key->q);	/* s = k^(-1)*(rx+message) MOD q */


	return;
}


/***************************************************************
 *
 * Procedure dsa_verifyblock
 *
 ***************************************************************/
#ifdef __STDC__

int dsa_verifyblock(
	register L_NUMBER	  in[],
	DSA_sig			 *sig,
	DSA_Pkeys		 *key
)

#else

int dsa_verifyblock(
	in,
	sig,
	key
)
register L_NUMBER	  in[];
DSA_sig			 *sig;
DSA_Pkeys		 *key;

#endif

{
	L_NUMBER   y_exp[MAXLGTH], g_exp[MAXLGTH], gy[MAXLGTH], v[MAXLGTH], w[MAXLGTH], u1[MAXLGTH], u2[MAXLGTH];


	if(comp(sig->r, lz_null) <= 0 || comp(sig->r, key->q) >= 0 || comp(sig->s, lz_null) <= 0 || comp(sig->s, key->q) >= 0) return(-1);


	ln_inv(sig->s, key->q, w);
	mmult(w, in, u1, key->q);
	mmult(w, sig->r, u2, key->q);

	mexp(key->g, u1, g_exp, key->p);
	mexp(key->y, u2, y_exp, key->p);

	mmult(g_exp, y_exp, gy, key->p);
	div(gy, key->q, v, v);

	return(comp(v, sig->r));

}
