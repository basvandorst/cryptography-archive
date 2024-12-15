/* pgpkgen.h */

/* These functions are used to generate keys and write them out. There
   are also some more mp library functions here. Most of this stuff was
   chopped out of genprime.c and rsagen.c */

/* Generates a public key/secret key pair using pgp_randombyte for random
   numbers. Nbits specifies the number of bits in the modulus, ebits is
   the number of bits in e (PGP uses 5), and output will be called
   periodically to provide updates on what is happening.
   Output can be null to ignore. Output's prototype is:
void output(unsigned stage);
   Where stage gives the stage in progress:
   0=generating p 1=testing possible p
   2=generating q 3=testing possible q
   4=finishing key calculation (almost done) */
void pgpk_keygen(struct pgp_pubkey *pk,struct pgp_seckey *sk,
		 unsigned nbits,unsigned ebits,time_t timestamp,
		 word16 validity,void (*output)() );

/* Write a public key packet out to a fifo */
void pgpk_create_pk(struct fifo *f,struct pgp_pubkey *pk);

/* Write a secret key packet out to a fifo, encrypting it with ideakey
   or leaving it unencrypted if ideakey is NULL. This would be used to
   write a newly-created key, and also to change the passphrase on an
   existing key. */
void pgpk_create_sk(struct fifo *f,struct pgp_pubkey *pk,
	     struct pgp_seckey *sk,unsigned ideakey[16]);

/* These might be useful and so are not static */

/* Greatest common denominator */
void mp_gcd(unitptr result,unitptr a,unitptr n);

/* Multiplicative inverse - used for finding d */
void mp_inv(unitptr x,unitptr a,unitptr n);

/* Find a random prime of nbits length */
void randomprime(unitptr p,short nbits);
