/* rsaglue.h - RSA encryption and decryption */

/*
 * If this is defined, and the appropriate changes are made in the makefile,
 * a version of PGP that uses RSAREF will be generated.  Note, however, that
 * RSAREF is distributed under a licence which does not permit access to the
 * functions that PGP uses, so this is not legal in any country.  Stick with
 * the non-RSAREF version, which is faster and only illegal in the U.S.
 * (Negotiations to get permission to use the extra RSAREF functions are in
 * progress, but an agreement has not been reached as of the time of release.)
 */
#undef RSAREF

/* Declarations */
int
rsa_public_encrypt(unitptr outbuf, byteptr inbuf, short bytes,
	 unitptr E, unitptr N);
int
rsa_private_encrypt(unitptr outbuf, byteptr inbuf, short bytes,
	 unitptr E, unitptr D, unitptr P, unitptr Q, unitptr U, unitptr N);
int
rsa_public_decrypt(byteptr outbuf, unitptr inbuf,
	unitptr E, unitptr N);
int
rsa_private_decrypt(byteptr outbuf, unitptr inbuf,
	 unitptr E, unitptr D, unitptr P, unitptr Q, unitptr U, unitptr N);
