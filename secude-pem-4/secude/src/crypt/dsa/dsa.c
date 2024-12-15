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
 * DSA - modul fuer secure interface
 *
 * dsa_encrypt(), dsa_decrypt(), dsa_sign(), dsa_verify(), hash_sqmodn(),
 * dsa_set_key(), dsa_pkmap()
 */

#include "secure.h"
#include "arithmetic.h"

#include "dsa.h"

#define ERASEKEY bzero((char *)&K,sizeof(K))
/*---------------------------------------------------------------*
 * statische Variablen fuer restliche input Bloecke und DSA keys *
 *---------------------------------------------------------------*/
static DSA_keys  K;              /* DSA key fields */


#ifdef __STDC__
static void	bzero	(char *from, int length);
#else
static void 	bzero();
#endif



/*----------------------------------------------get_dsa_key---*/

/***************************************************************
 *
 * Procedure dsa_set_key
 *
 ***************************************************************/
#ifdef __STDC__

RC dsa_set_key(
	char	 *key,
	int	  keytype
)

#else

RC dsa_set_key(
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

        INTEGERtoln(&d_key->part1, K.pk.y);
	if(d_key->part2.noctets) {
       		INTEGERtoln(&d_key->part2, K.pk.p);
       		INTEGERtoln(&d_key->part3, K.pk.q);
        	INTEGERtoln(&d_key->part4, K.pk.g);
	}
	else {
		trans(dsa_public_part[keytype/64-8].p, K.pk.p)
		trans(dsa_public_part[keytype/64-8].q, K.pk.q)
		trans(dsa_public_part[keytype/64-8].g, K.pk.g)
	}

        /* destroy key conversion */

        if(keytype) {
           bzero(d_key->part1.octets, d_key->part1.noctets);
           bzero(d_key->part2.octets, d_key->part2.noctets);
           bzero(d_key->part3.octets, d_key->part3.noctets);
           bzero(d_key->part4.octets, d_key->part4.noctets);
           free(d_key->part1.octets);
           free(d_key->part2.octets);
           free(d_key->part3.octets);
           free(d_key->part4.octets);
           free(d_key);
        }
        return 0;
}




/*---------------------------------------------dsa_sign-------*/
/***************************************************************
 *
 * Procedure dsa_sign
 *
 ***************************************************************/
#ifdef __STDC__

RC dsa_sign(
	OctetString	 *hash,
	BitString	 *sign
)

#else

RC dsa_sign(
	hash,
	sign
)
OctetString	 *hash;
BitString	 *sign;

#endif

{
        L_NUMBER        L[MAXLGTH];
        L_NUMBER        S[MAXLGTH];
	DSA_sig	 	sig;
	KeyBits		keybits;
	char		p1[25], p2[25];
        BitString       *sigtmp;

	keybits.part1.octets = p1;
	keybits.part2.octets = p2;

        if(MAXLGTH * WLNG < hash->noctets * BYTEL) return(-1);

        sign->nbits = 0;

        INTEGERtoln(hash, L);

        dsa_signblock(L, &sig, &K.sk);

	lntoINTEGER(sig.s, &keybits.part1);
	lntoINTEGER(sig.r, &keybits.part2);
	keybits.part3.noctets = 0;
	keybits.part4.noctets = 0;

	sigtmp = e_KeyBits(&keybits);

	sign->bits = sigtmp->bits;
	sign->nbits = sigtmp->nbits;
	free(sigtmp);

        ERASEKEY;
        return 0;
}


/*---------------------------------------------dsa_verify-----*/
/***************************************************************
 *
 * Procedure dsa_verify
 *
 ***************************************************************/
#ifdef __STDC__

RC dsa_verify(
	OctetString	 *hash,
	BitString	 *sign
)

#else

RC dsa_verify(
	hash,
	sign
)
OctetString	 *hash;
BitString	 *sign;

#endif

{
        L_NUMBER        L[MAXLGTH];
        L_NUMBER        C[MAXLGTH];
	RC 		rc;
	KeyBits		*keybits;
	DSA_sig	 	sig;

        if(MAXLGTH * WLNG < sign->nbits) return(-1);
        if(MAXLGTH * WLNG < hash->noctets * BYTEL) return(-1);

	keybits = d_KeyBits(sign);

        INTEGERtoln(&keybits->part1, sig.s);
        INTEGERtoln(&keybits->part2, sig.r);

	aux_free_KeyBits(&keybits);

        INTEGERtoln(hash, L);

        rc = dsa_verifyblock(L, &sig, &K.pk);

        ERASEKEY;
        return(-rc*rc);
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
