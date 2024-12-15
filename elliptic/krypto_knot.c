/****** krypto_knot.c *****/
/********************************************************************************
*										*
*	The purpose of this file is to tie together all the subroutines from	*
*  the elliptic curve optimal normal basis suite to create a key hiding system.	*
*  The actual compression and encoding should be performed by standard 		*
*  algorithms such as arith-n (compression book) and blowfish (Dr. Dobbs's	*
*  Journal or Applied Cryptography.  The complete system would then violate ITAR*
*  and not be accessable, so you'll have to hack that yourself.			*
*										*
********************************************************************************/

#include <stdio.h>
#include "bigint.h"
#include "eliptic.h"
#include "eliptic_keys.h"

extern void null(BIGINT*);
extern void copy(BIGINT*, BIGINT*);
extern void fofx(BIGINT*, CURVE*, BIGINT*);
extern int gf_quadradic(BIGINT*, BIGINT*, BIGINT*);
extern void one(BIGINT*);
extern void Mother(unsigned long*);
extern void esum(POINT*, POINT*, POINT*, CURVE*);
extern void esub(POINT*, POINT*, POINT*, CURVE*);
extern void elptic_mul(BIGINT*, POINT*, POINT*, CURVE*);
extern void flaten(POINT*);
extern void public_key_gen(BIGINT*, PUBKEY*, INDEX);
extern void restore_pub_key( char*, PUBKEY*);
extern void print_pubkey( PUBKEY*);
extern void big_print(char*, BIGINT*);

extern unsigned long random_seed;

/*   encrypt a session key.  Enter with given session key to hide, public key
to hide it in, and storage block for result.  It is a waste of space to use a
PUBKEY for this, too bad.

session == pointer to key to be hidden
pk == pointer to public key block to use
ek == pointer to resultant key block.  ek->p holds kP, ek->q holds S+kQ
*/

void elptic_encrypt(BIGINT *session, PUBKEY *pk, PUBKEY *ek)
{
	INDEX	i;
	BIGINT	k, f, y[2];
	POINT	s, t;

/*  encode session key onto a random point using this public key.  */

	null(&k);
	Mother(&random_seed);
	copy(session, &k);
	k.b[STRTPOS] = random_seed & UPRMASK;

/*  note that this assumes session key < NUMBITS and that STRTPOS ELEMENT is free
to be clobbered.  For all reasonable encoding schemes this shouldn't be a problem.
*/

	fofx( &k, &pk->crv, &f);
	while (gf_quadradic(&k, &f, y)) {
	   k.b[STRTPOS]++;
	   fofx(&k, &pk->crv, &f);
	}
	copy( &k, &s.x);
	copy( &y[1],&s.y);	/*  use 1 just to be different, why not eh?  */
	one(&s.z);

/*  next generate a random multiplier k  */

	null(&k);
	SUMLOOP(i) {
	   Mother(&random_seed);
	   k.b[i] = random_seed;
	}
	k.b[STRTPOS] &= UPRMASK;

/*  do 2 multiplies, kp and kq  */

	elptic_mul(&k, &pk->p, &ek->p, &pk->crv);
	flaten( &ek->p);
	elptic_mul(&k, &pk->q, &t, &pk->crv);

/*  add s to kQ as final step  */

	esum(&s, &t, &ek->q, &pk->crv);
	flaten(&ek->q);
	ek->crv.form = pk->crv.form;
	copy( &pk->crv.a2, &ek->crv.a2);
	copy( &pk->crv.a6, &ek->crv.a6);
}

/*  decrypt session key from public and encrypted key.
	returns 0 if successful, -1 on failure (wrong pass phrase).
*/

int elptic_decrypt(BIGINT *session, PUBKEY *pk, PUBKEY *ek)
{
	INDEX	i;
	BIGINT	skey;
	POINT	check, t, s;

/*  first ensure you can generate secret key.  */

	public_key_gen(&skey, pk, 0);
	elptic_mul(&skey, &pk->p, &check, &pk->crv);
	flaten(&check);
	SUMLOOP(i) {
	   if (check.x.b[i] != pk->q.x.b[i]) {
	      printf("Invalid pass phrase.\n");
	      return(-1);
	   }
	}

/*  next compute T = aR and subtract from R' to get S  */

	elptic_mul(&skey, &ek->p, &t, &pk->crv);
	esub(&ek->q, &t, &s, &pk->crv);
	flaten(&s);

/*  clear out encoding garbage and return session key */

	copy( &s.x, session);
	session->b[STRTPOS] = 0;
	return(0);
}

main()
{
        char    file[256];
	BIGINT	session_key, recovered_key, secret_key;
	PUBKEY	public_key, hidden_key;
	INDEX	i;

        init_opt_math();
        init_rand();

/*  for this test, assume you have already generated a public key (and remember
the pass phrase!).  Read in key file from disk.  */

/*        printf("input file name for previous key: ");
        scanf("%s",&file);
        restore_pub_key(file, &public_key);
*/
	public_key_gen(&secret_key, &public_key, 1);
	print_pubkey(&public_key);
	big_print("secret key: ", &secret_key);

/*  generate a session key...  */

	printf("Generating session key and encryping it...\n");
	SUMLOOP(i) {
	   Mother(&random_seed);
	   session_key.b[i] = random_seed;
	}
	elptic_encrypt(&session_key, &public_key, &hidden_key);
	print_pubkey(&hidden_key);
	big_print("session_key: ",&session_key);
	elptic_decrypt(&recovered_key, &public_key, &hidden_key);
	big_print("recovered key: ",&recovered_key);

	close_rand();
}

