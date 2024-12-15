/*
* pgpRSAGlue.c - The interface between bignum math and RSA operations.
* This layer's primary reason for existence is to allow adaptation
* to other RSA math libraries for legal reasons.
*
* Written by Colin Plumb.
*
* Broken into two files by BAT. pgpRSAGlue1.c contains encryption and
* signing functions. pgpRSAGlue2.c contains decryption and verification
* functions. This was done for legal reasons.
*
* $Id: pgpRSAGlue1.c,v 1.1.2.1 1997/06/09 23:46:31 quark Exp $
*/

/*
* An alternative version of this module is used if RSAREF is needed. This
* entire source file is under the control of the following conditional:
*/
#if (!NO_RSA && !USE_RSA_LEGACY && !USE_RSAREF_FULL)
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpESK.h"
#include "pgpRSAKey.h"
#include "pgpRSAGlue.h"
#include "pgpKeyMisc.h"
#include "bn.h"
#include "pgpErr.h"
#include "pgpRndom.h"
#include "pgpUsuals.h"

/*
* This is handy for debugging, but VERY VERY DANGEROUS for production -
* it nicely prints every detail of your secret key!
*/
#define BNDEBUG 0

#if BNDEBUG
/* Some debugging hooks which have been left in for now. */
#include "bnprint.h"
#define bndPut(prompt, bn) (fputs(prompt, stdout), bnPrint(stdout, bn), \
				putchar('\n'))
#define bndPrintf printf
#else
#define bndPut(prompt, bn) ((void)(prompt),(void)(bn))
#if __GNUC__ > 1
/* Non-ANSI, but supresses warnings about expressions with no effect */
#define bndPrintf(arg...) (void)0
#else
/* ANSI-compliant - cast entire comma expression to void */
#define bndPrintf (void)
#endif
#endif

int
rsaPublicEncrypt(struct BigNum *bn, byte const *in, unsigned len,
	struct RSApub const *pub, struct PgpRandomContext const *rc)
{
	unsigned bytes = (bnBits(&pub->n)+7)/8;
	pgpPKCSPack(bn, in, len, PKCS_PAD_ENCRYPTED, bytes, rc);

bndPrintf("RSA encrypting.\n");
bndPut("plaintext = ", bn);
	return bnExpMod(bn, bn, &pub->e, &pub->n);
}


/*
* This does an RSA signing operation, which is very similar, except
* that the padding differs. The type is 1, and the padding is all 1's
* (hex 0xFF). In addition, if the data is a DER-padded MD5 hash, there's
* an option for encoding it with the old PGP 2.2 format, in which case
* that's all replaced by a 1 byte indicating MD5.
*
* When decrypting, distinguishing these is a bit trickier, since the
* second most significant byte is 1 in both cases, but in general,
* it could only cause confusion if the PGP hash were all 1's.
*
* To summarize, the formats are:
*
* Position	Value Function
* n-1	0	This is needed to ensure that the padded number
*		is less than the modulus.
* n-2	1	The padding type (all ones).
* n-3..len+1 255	All ones padding to ensure signatures are rare.
* len	0	Zero byte to mark the end of the padding
* len-1..x	ASN.1	The ASN.1 DER magic cookie (18 bytes)
* x-1..0	data	The payload MD5 hash (16 bytes).
*
*
* Position	Value
* n-1	0
* n-2	1	 "This is MD5"
* n-2..n-len-2 data		Supplied payload MD5 hash (len == 16).
* n-len-2 0		Zero byte to mark the end of the padding
* n-len-3..1 255	All ones padding.
* 0 1		The padding type (all ones).
*
*
* The reason for the all 1's padding is an extra consistency check.
* A randomly invented signature will not decrypt to have the long
* run of ones necessary for acceptance.
*
* Oh... the public key isn't needed to decrypt, but it's passed in
* because a different glue library may need it for some reason.
*
* TODO: Have the caller put on the PKCS wrapper. We can notice and
* strip it off if we're trying to be compatible.
*/
static const byte signedType = 1;

int
rsaPrivateEncrypt(struct BigNum *bn, byte const *in, unsigned len,
	struct RSAsec const *sec)
{
	unsigned bytes = (bnBits(&sec->n)+7)/8;

	pgpPKCSPack(bn, in, len, PKCS_PAD_SIGNED, bytes,
		(struct PgpRandomContext const *)NULL);

bndPrintf("RSA signing.\n");
bndPut("plaintext = ", bn);
	return bnExpModCRA(bn, &sec->d, &sec->p, &sec->q, &sec->u);
}


#endif

/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
