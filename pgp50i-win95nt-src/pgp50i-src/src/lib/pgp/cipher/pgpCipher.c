/*
* pgpCipher.c - header file for generic block ciphers
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpCipher.c,v 1.3.2.1 1997/06/07 09:49:51 mhw Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpCipher.h"
#include "pgpDES3.h"
#include "pgpIDEA.h"
#include "pgpCAST5.h"
/*#include "nullciph.h"*/
#include "pgpMem.h"
#include "pgpEnv.h"

static struct PgpCipher const * const cipherList[] = {
			NULL, /* &cipherNULL, */
			&cipherIDEA,
			&cipher3DES,
			&cipherCAST5
	};

struct PgpCipher const *
pgpCipherByNumber (unsigned num)
	{
			if (num >= sizeof (cipherList) / sizeof (cipherList[0]))
				 return 0;
			return cipherList[num];
	}

/* Look up a cipher by name (of any use? Anyway, I wrote it. */
struct PgpCipher const *
pgpCipherByName (char const *name, size_t namelen)
	{
			unsigned i;
			struct PgpCipher const *cipher;

			for (i = 0; i < sizeof (cipherList) / sizeof (cipherList[0]); i++) {
					cipher = cipherList[i];
					if (cipher && !memcmp (name, cipher->name, namelen) &&
					cipher->name[namelen] == '\0')
							return cipher; /* That's it. */
			}
			return 0;				 /* not found */
	}

	void
pgpCipherWipe(struct PgpCipherContext *cc)
{
	memset(cc->priv, 0, cc->cipher->context_size);
}

struct PgpCipherContext *
pgpCipherCreate(struct PgpCipher const *cipher)
	{
			struct PgpCipherContext *cc;
			void *priv;

priv = pgpMemAlloc(cipher->context_size);
if (!priv)
	return 0;
cc = (struct PgpCipherContext *)pgpMemAlloc(sizeof(*cc));
if (!cc) {
	pgpMemFree(priv);
	return 0;
}
memset (priv, 0, cipher->context_size);
cc->cipher = cipher;
cc->priv = priv;
return cc;
}

struct PgpCipherContext *
pgpCipherCopy (struct PgpCipherContext const *cc)
{
			struct PgpCipherContext *newcc;

newcc = pgpCipherCreate (cc->cipher);
if (!newcc)
	return NULL;

memcpy (newcc->priv, cc->priv, cc->cipher->context_size);
return newcc;
}

void
pgpCipherDestroy(struct PgpCipherContext *context)
	{
			if (context) {
					pgpCipherWipe(context);
					pgpMemFree(context->priv);
					pgpMemFree(context);
			}
	}

/*
* "Bulk" refers to bulk data. "Key" refers to other
* keys and similar small things. You might want to
* make different speed/security tradeoffs for the two.
*/
struct PgpCipher const *
pgpCipherDefaultBulk(struct PgpEnv const *env)
	{
			return pgpCipherByNumber (pgpenvGetInt (env, PGPENV_CIPHER,
				 		 		 NULL, NULL));
	}

/*
* Choose cipher for key encryption. Not used for V2 compatible keys.
* Override pgpenv defaults with our own default.
*/
struct PgpCipher const *
pgpCipherDefaultKey(struct PgpEnv const *env)
{
int ciphernum;
int cipherpri;

ciphernum = pgpenvGetInt (env, PGPENV_CIPHER, &cipherpri, NULL);
if (cipherpri < PGPENV_PRI_CONFIG)
	ciphernum = PGP_CIPHER_CAST5;
return pgpCipherByNumber (ciphernum);
}

/* Use this for old pre-PGP3 key encryption; default to backwards compat */
struct PgpCipher const *
pgpCipherDefaultKeyV2(struct PgpEnv const *env)
{
		int ciphernum;
		int cipherpri;

		ciphernum = pgpenvGetInt (env, PGPENV_CIPHER, &cipherpri, NULL);
		if (cipherpri < PGPENV_PRI_CONFIG)
			ciphernum = PGP_CIPHER_IDEA;
		return pgpCipherByNumber (ciphernum);
}
