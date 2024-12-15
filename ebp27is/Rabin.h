/* Rabin.h - Glue routines for RSA encryption and decryption */

extern char signon_legalese[];

/* Declarations */
int
rabin_public_encrypt(unitptr outbuf, byteptr inbuf, short bytes,
	 unitptr E, unitptr N);
int
rabin_private_encrypt(unitptr outbuf, byteptr inbuf, short bytes,
	 unitptr P, unitptr Q, unitptr N, unitptr A, unitptr B);
int
rabin_public_decrypt(byteptr outbuf, unitptr inbuf,
	unitptr E, unitptr N);
int
rabin_private_decrypt(byteptr outbuf, unitptr inbuf,
	 unitptr P, unitptr Q, unitptr N, unitptr A, unitptr B);
