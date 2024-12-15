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


#include <stdio.h>
#include "af.h"
#include "arithmetic.h"

extern int primes[];    /* Die ersten 1000 Primzahlen          */


#define InitDH { if(sec_verbose) {fprintf(stderr,"DH Agreement Parameter generation"); fflush(stderr);} }
#define PrintRabinstest { if(sec_verbose) {fprintf(stderr,"\b\b\b\b%4d", cnt++); fflush(stderr);} }
#define EndGeneration { if(sec_verbose) {fprintf(stderr,"\n"); fflush(stderr);} }
#define Sorry { if(sec_verbose) {fprintf(stderr,"\nSorry: again."); fflush(stderr);} }
#define StartGeneration(s) { if(sec_verbose) {cnt=0;fprintf(stderr,"\nGenerating "); fprintf(stderr,s); fflush(stderr);} }


/***************************************************************
 *
 * Procedure parm2ln
 *
 * extract sequence of 2 integers from parm of algid
 *
 ***************************************************************/
#ifdef __STDC__

static RC parm2ln(
	AlgId		 *algid,
	L_NUMBER	 *p,
	L_NUMBER	 *g,
	int		 *l
)

#else

static RC parm2ln(
	algid,
	p,
	g,
	l
)
AlgId		 *algid;
L_NUMBER	 *p;
L_NUMBER	 *g;
int	 	 *l;

#endif

{
	char 	       *proc = "parm2ln";
	unsigned char  *octs;
	int		n;

	if(!g || !p || !l) {
		aux_add_error(EINVALID, "NULL parameter given", CNULL, 0, proc);
		return(-1);
	}
	if(!algid) {
		aux_add_error(EINVALID, "No Algorithm found", CNULL, 0, proc);
		return(-1);
	}
	if(aux_cmp_ObjId(algid->objid, dhKeyAgreement->objid) ) {
		aux_add_error(EALGID, "Algorithm is not dhKeyAgreement", (char *)algid, AlgId_n, proc);
		return(-1);
	}
	if(!algid->param
		|| !((KeyBits *)algid->param)->part1.noctets
		|| !((KeyBits *)algid->param)->part2.noctets
		|| ((KeyBits *)algid->param)->part3.noctets > 4) {
		aux_add_error(EALGID, "parameter of dhKeyAgreement algorithm is wrong", CNULL, 0, proc);
		return(-1);
	}


	INTEGERtoln(&((KeyBits *)algid->param)->part1, p);
	INTEGERtoln(&((KeyBits *)algid->param)->part2, g);

	*l = 0;
	if(((KeyBits *)algid->param)->part3.noctets) *l = aux_OctetString2int(&(((KeyBits *)algid->param)->part3));
	return(0);

}
/***************************************************************
 *
 * Procedure ln2parm
 *
 * store sequence of 3 integers into parm of algid
 *
 ***************************************************************/
#ifdef __STDC__

static KeyBits *ln2parm(
	L_NUMBER	 *p,
	L_NUMBER	 *g,
	int		  privateValueLength
)

#else

static KeyBits *ln2parm(
	p,
	g,
	privateValueLength
)
L_NUMBER	 *p;
L_NUMBER	 *g;
int		  privateValueLength;

#endif

{
	char 		*proc = "ln2parm";
	BitString 	*bits;
	KeyBits         *param;
	L_NUMBER	 l[2];

	l[0] = 1;
	l[1] = privateValueLength;

	if(!(param = (KeyBits *) calloc(1, sizeof(KeyBits)))) {
		aux_add_error(EMALLOC, "param", CNULL, 0, proc);
		return((KeyBits *)0);
	}


	if(!(param->part1.octets =  malloc(p[0] * WBYTES))) {
		aux_add_error(EMALLOC, "param->part1.octets", CNULL, 0, proc);
		free(param);
		return((KeyBits *)0);
	}
	lntoINTEGER(p, &(param->part1));


	if(!(param->part2.octets =  malloc(g[0] * WBYTES))) {
		aux_add_error(EMALLOC, "param->part2.octets", CNULL, 0, proc);
		free(param);
		free(param->part1.octets);
		return((KeyBits *)0);
	}
	lntoINTEGER(g, &(param->part2));


	if(privateValueLength) {
		if(!(param->part3.octets =  malloc(l[0] * WBYTES))) {
			aux_add_error(EMALLOC, "param->part3.octets", CNULL, 0, proc);
			free(param);
			free(param->part1.octets);
			free(param->part2.octets);
			return((KeyBits *)0);
		}
		lntoINTEGER(l, &(param->part3));
	}
	else {
		param->part3.octets = CNULL;
		param->part3.noctets = 0;
	}


	param->part4.octets = CNULL;
	param->part4.noctets = 0;

	return(param);

}

/***************************************************************
 *
 * Procedure test35711
 *
 * tests if a number is dividable by the first 19 primes,
 * is used to avoid a slower prime test in most cases
 *
 ***************************************************************/
#ifdef __STDC__

static int test35711(
	L_NUMBER	  p[]
)

#else

static int test35711(
	p
)
L_NUMBER	  p[];

#endif

{
	unsigned int 	n,
			m3 = 0,
			m5 = 0;
	L_NUMBER 	d[2];
	L_NUMBER 	r[2];
	L_NUMBER 	t[MAXGENL];

	if(p[0] == 1 && p[1] <= 71 && ( p[1] ==  3 || p[1] ==  5 || p[1] ==  7 || p[1] == 11 ||
					p[1] == 13 || p[1] == 17 || p[1] == 19 || p[1] == 23 ||
					p[1] == 29 || p[1] == 31 || p[1] == 37 || p[1] == 41 ||
					p[1] == 43 || p[1] == 47 || p[1] == 53 || p[1] == 59 ||
					p[1] == 61 || p[1] == 67 || p[1] == 71)) return(2);

	for(n = 1; n<=p[0]; n++) m3 += p[n] % 3;
	
	if(!(m3 % 3)) return(0);

	for(n = 1; n<=p[0]; n++) m5 += p[n] % 5;

	if(!(m5 % 5)) return(0);


	d[0] = 1;
	d[1] = 7*11*13*17*19*23*29;
	div(p,d,t,r);

	for(n=2;n<9; n++) 
		if(!(r[1] % primes[n])) return(0);
	

	d[1] = 31*37*41*43*47;
	div(p,d,t,r);

	for(n=9;n<14; n++) 
		if(!(r[1] % primes[n])) return(0);
	

	d[1] = 53*59*61*67*71;
	div(p,d,t,r);

	for(n=14;n<19; n++) 
		if(!(r[1] % primes[n])) return(0);
	


	return(1);
}

/***************************************************************
 *
 * Procedure create_p_g
 *
 * creates number p, g where
 *
 * p is a prime of DH_SIZE_p bits
 * p-1 is dividable by a prim q of DH_SIZE_q bits
 * g has order >= q (mod p)
 * 
 *
 ***************************************************************/
#ifdef __STDC__

static RC create_p_g(
	L_NUMBER	  p[],
	L_NUMBER	  g[],
	int	  	  size_of_p
)

#else

static RC create_p_g(
	p,
	g,
	size_of_p
)
L_NUMBER	  p[];
L_NUMBER	  g[];
int	  	  size_of_p;

#endif

{
	L_NUMBER	q[MAXGENL],
			test[MAXGENL],
			n[MAXGENL];

	int 		cnt, size_q;

	if(size_of_p <= 23) size_of_p = 24;

	size_q = size_of_p - 16;

	InitDH;

	p[0] = 0;

	do {

		if(p[0]) Sorry;

	/* generate random q */
	
		StartGeneration("q:     ");PrintRabinstest;
	
		rndm(size_q-1, q);
	
	
	
	/* increment q by 2 until q is prime */
	
		while (!test35711(q) || rabinstest(q)<0) {
			PrintRabinstest;
			add(q, lz_zwei, q);
		}
	
	
	/* generate random n such that p = q*n+1 hat the right size */
	
		StartGeneration("p:     ");PrintRabinstest;
	
		rndm(size_of_p - size_q, n);
		add(n, lz_eins, n);
	
	
		mult(q, n, p);
		add(p, lz_eins, p);
	
	/* increment p by 2q and n by 2 until p = q*n+1 is prime */
	
		while ((!test35711(p) || rabinstest(p)<0) && lngtouse(p)<size_of_p) {
			PrintRabinstest;
			add(p, q, p);
			add(p, q, p);
			add(n, lz_zwei, n);
		}
	
	} while(lngtouse(p) >= size_of_p);

/* generate random g */

	StartGeneration("g:     "); PrintRabinstest;

	rndm(size_of_p - 1, g);


	mexp(g, n, test, p);

/* increment g until g has order >= q   (mod p)*/

	while(!comp(test, lz_eins)) {
		PrintRabinstest;
		add(g, lz_eins, g);
		mexp(g, n, test, p);
	}
	div(g, p, g, g);
	EndGeneration;

	return(0);
}

/***************************************************************
 *
 * Procedure sec_DH_init
 *
 * Generates the prime modulus p and the base g for DH key
 * agreement.
 *
 * Returns a AlgId with OID dhWithCommonModulus and KeyBits
 * parameter which comprises the three components p, g and 
 * private_value_length.
 *
 ***************************************************************/
#ifdef __STDC__

AlgId	*sec_DH_init(
	int	  size_of_p,
	int	  private_value_length
)

#else

AlgId	*sec_DH_init(
	size_of_p,
	private_value_length
)
int	  size_of_p;
int	  private_value_length;

#endif

{
	char 	 	*proc = "sec_DH_init";

	L_NUMBER 	 p[MAXGENL];
	L_NUMBER 	 g[MAXGENL];
	AlgId		*dhparam;


	/* Generate p and g */

	create_p_g(p, g, size_of_p);

	if(!(dhparam = (AlgId *) calloc(1, sizeof(AlgId)))) {
		aux_add_error(EMALLOC, "dhparam", CNULL, 0, proc);
		return((AlgId *)0);
	}
	dhparam->objid = aux_cpy_ObjId(dhKeyAgreement->objid);
	if(!(dhparam->param = (char *)ln2parm(p, g, private_value_length))) {
		AUX_ADD_ERROR;
		aux_free_AlgId(&dhparam);
		return((AlgId *) 0);
	}

	return(dhparam);
}


/***************************************************************
 *
 * Procedure sec_DH_phase1
 * 
 * Generates a random private key X from the given (p,g)
 * and stores it according to key_x.
 *
 * Calculates Y from g, X and p.
 * 
 * Returns keyinfo with Y in keyinfo->subjectkey and
 * keyinfo->subjectAI = dhWithCommonModulus, if with_pg is FALSE, 
 * or
 * keyinfo->subjectAI = dhKeyAgreement, if with_pg is TRUE. 
 *
 ***************************************************************/
#ifdef __STDC__

KeyInfo *sec_DH_phase1(
	AlgId	 *dhparam,
	Key	 *key_x,
	Boolean	  with_pg
)

#else

KeyInfo *sec_DH_phase1(
	dhparam,
	key_x,
	with_pg
)
AlgId	 *dhparam;
Key	 *key_x;
Boolean	  with_pg;

#endif

{
	char 	 	*proc = "sec_DH_phase1";

	L_NUMBER 	 p[MAXGENL];
	L_NUMBER 	 g[MAXGENL];
	L_NUMBER 	 X[MAXGENL];
	L_NUMBER 	 Y[MAXGENL];
	KeyInfo  	*keyinfo_x,
			*keyinfo_y;
	int		 private_value_length, length_of_p, no_of_octets;

/* get (p,g) from dhparam */
	if(parm2ln(dhparam, p, g, &private_value_length) < 0) {
		AUX_ADD_ERROR;
		return((KeyInfo *)0);
	}
	length_of_p = lngtouse(p) + 1;
	no_of_octets = length_of_p / BYTEL + 1;


/* generate random X */
	rndm((private_value_length ? private_value_length - 1 : length_of_p - 1), X);

/* calculate Y */
	mexp(g, X, Y, p);


/* put X into a keyinfo */
	if(!(keyinfo_x = (KeyInfo *)calloc(1, sizeof(KeyInfo)))) {
		aux_add_error(EMALLOC, "calloc KeyInfo", CNULL, 0, proc);
		return((KeyInfo *)0);
	}

/* give it an AlgId according to with_pg */
	if(with_pg) keyinfo_x->subjectAI = aux_cpy_AlgId(dhparam);
	else keyinfo_x->subjectAI = aux_cpy_AlgId(dhWithCommonModulus);

/* convert X into a BitString */
	keyinfo_x->subjectkey.nbits = 0;
	if(!(keyinfo_x->subjectkey.bits = malloc(no_of_octets))) {
		aux_add_error(EMALLOC, "malloc BitString", CNULL, 0, proc);
		aux_free_KeyInfo(&keyinfo_x);
		return((KeyInfo *)0);
	}


	if(aux_LN2BitString2(&keyinfo_x->subjectkey, X, 0) < 0) {
		AUX_ADD_ERROR;
		return((KeyInfo *)0);
	}

	bzero(X,MAXGENL*WBYTES);


/* store the keyinfo for X */
	if(put_keyinfo_according_to_key(keyinfo_x, key_x, DHprivate_OID) < 0) {
		aux_add_error(LASTERROR, "Can't put keyinfo", CNULL, 0, proc);
		aux_free_KeyInfo(&keyinfo_x);
		return((KeyInfo *)0);
	}
	bzero(keyinfo_x->subjectkey.bits, no_of_octets);

	aux_free_KeyInfo(&keyinfo_x);


/* put Y into a keyinfo */
	if(!(keyinfo_y = (KeyInfo *)calloc(1, sizeof(KeyInfo)))) {
		aux_add_error(EMALLOC, "calloc KeyInfo", CNULL, 0, proc);
		return((KeyInfo *)0);
	}

/* give it an AlgId according to with_pg */
	if(with_pg) keyinfo_y->subjectAI = aux_cpy_AlgId(dhparam);
	else keyinfo_y->subjectAI = aux_cpy_AlgId(dhWithCommonModulus);


/* convert Y into a BitString */
	if(aux_LN2BitString2(&keyinfo_y->subjectkey, Y, 0) < 0) {
		AUX_ADD_ERROR;
		return((KeyInfo *)0);
	}

	return(keyinfo_y);
}



/***************************************************************
 *
 * Procedure sec_DH_phase2
 *
 * calculates the session key from (p,g), the own X and the peer Y,
 * and returns it in a BitString
 *
 ***************************************************************/
#ifdef __STDC__

BitString *sec_DH_phase2(
	AlgId    *dhparam,
	Key	 *key_x,
	KeyInfo	 *peer_y
)

#else

BitString *sec_DH_phase2(
	dhparam,
	key_x,
	peer_y
)
AlgId    *dhparam;
Key	 *key_x;
KeyInfo	 *peer_y;

#endif

{
	char 	 	*proc = "sec_DH_phase2";

	L_NUMBER 	 p[MAXGENL];
	L_NUMBER 	 g[MAXGENL];
	L_NUMBER 	 X[MAXGENL];
	L_NUMBER 	 Y[MAXGENL];
	L_NUMBER   	 K[MAXGENL];

	KeyInfo  	*own_x;
	BitString 	*agreed;
	AlgId           *dhparam_in_use;
	int		 private_value_length, length_of_p, no_of_octets;

/* get keyinfo for X from PSE, keypool or memory */
	if(!(own_x = get_keyinfo_from_key(key_x))) {
		AUX_ADD_ERROR;
		return((BitString *)0);
	}

/* determine DH parameters to be used for phase 2 */

	if(aux_cmp_ObjId(peer_y->subjectAI->objid, dhWithCommonModulus->objid) == 0) {

		/* peer_y has no DH parameter. 
                   use either DH parameter from own_x, if present, or from dhparam */

		if(aux_cmp_ObjId(own_x->subjectAI->objid, dhWithCommonModulus->objid) == 0) dhparam_in_use = dhparam;
		else if(aux_cmp_ObjId(own_x->subjectAI->objid, dhKeyAgreement->objid) == 0) dhparam_in_use = own_x->subjectAI;
		else {
			aux_add_error(EALGID, "own_x has wrong algid", (char *)own_x->subjectAI, AlgId_n, proc);
			aux_free_KeyInfo(&own_x);
			return((BitString *)0);
		}
	}
	else if(aux_cmp_ObjId(peer_y->subjectAI->objid, dhKeyAgreement->objid) == 0) {

		/* peer_y has DH parameter. Check whether it fits to own_x */

		if(aux_cmp_ObjId(own_x->subjectAI->objid, dhKeyAgreement->objid) == 0) {

			/* own_x has DH parameter. Compare with DH parameter of peer_y */

			if(aux_cmp_AlgId(peer_y->subjectAI, own_x->subjectAI)) {

				/* doesn't fit */

				aux_add_error(EINVALID, "DH parameter from peer_y doesn't fit to own_x", CNULL, 0, proc);
				aux_free_KeyInfo(&own_x);
				return((BitString *)0);
			}
		}
		else if(aux_cmp_ObjId(own_x->subjectAI->objid, dhWithCommonModulus->objid) == 0) {
			if(aux_cmp_AlgId(peer_y->subjectAI, dhparam)) {

				/* doesn't fit */

				aux_add_error(EINVALID, "DH parameter from peer_y doesn't fit to default parameter", CNULL, 0, proc);
				aux_free_KeyInfo(&own_x);
				return((BitString *)0);
			}
		}
		else {
			aux_add_error(EALGID, "own_x has wrong algid", CNULL, 0, proc);
			aux_free_KeyInfo(&own_x);
			return((BitString *)0);
		}
		dhparam_in_use = peer_y->subjectAI;
	}
	else {
		aux_add_error(EALGID, "peer_y has wrong algid", CNULL, 0, proc);
		aux_free_KeyInfo(&own_x);
		return((BitString *)0);
	}

/* get X from KeyInfo *own_x */

	if(aux_BitString2LN2(X, &own_x->subjectkey) < 0) {
		AUX_ADD_ERROR;
		return((BitString *)0);
	}

	bzero(own_x->subjectkey.bits, (own_x->subjectkey.nbits + 7) / 8 );

/* get Y from KeyInfo *peer_y */

	if(aux_BitString2LN2(Y, &peer_y->subjectkey) < 0) {
		AUX_ADD_ERROR;
		return((BitString *)0);
	}

/* get p, g and l from dhparam_in_use */

	if(parm2ln(dhparam_in_use, p, g, &private_value_length) < 0) {
		AUX_ADD_ERROR;
		aux_free_KeyInfo(&own_x);
		return((BitString *)0);
	}
	aux_free_KeyInfo(&own_x);

	length_of_p = lngtouse(p) + 1;
	no_of_octets = length_of_p / BYTEL + 1;


/* calculate agreed key */

	mexp(Y, X, K, p);

	bzero(X, MAXGENL*WBYTES);

/* store agreed key in a Bitstring */


	if(!(agreed = aux_LN2BitString(K, length_of_p))) {
		AUX_ADD_ERROR;
		return((BitString *)0);
	}

	bzero(K, MAXGENL*WBYTES);

	return(agreed);
}



