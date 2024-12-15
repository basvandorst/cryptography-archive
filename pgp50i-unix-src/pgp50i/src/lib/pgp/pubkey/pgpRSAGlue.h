/*
* pgpRSAGlue.h - RSA encryption and decryption
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* This is a PRIVATE header file, for use only within the PGP Library.
* You should not be using these functions in an application.
*
* $Id: pgpRSAGlue.h,v 1.2.2.2 1997/06/09 23:46:29 quark Exp $
*/
#ifndef PGPRSAGLUE_H
#define PGPRSAGLUE_H

#include "bn.h"
#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpRandomContext;

/* A structure to hold a public key */
struct RSApub {
	struct BigNum n;	/* The public modulus */
	struct BigNum e;	/* The public exponent */
};

/* A structure to hold a secret key */
struct RSAsec {
			struct BigNum n;		/* Copy of public parameters */
			struct BigNum e;
			struct BigNum d;		/* Decryption exponent */
			struct BigNum p;		/* The smaller factor of n */
			struct BigNum q;		/* The larger factor of n */
			struct BigNum u;		/* 1/p (mod q) */
	};

/* A PgpSecKey's priv points to this, an RSAsec plus the encrypted form... */
struct RSAsecPlus {
			struct RSAsec s;
			byte *cryptkey;
			size_t ckalloc, cklen;
			int locked;
	};
/* Fill in secret key structure */
void rsaFillSecKey(struct PgpSecKey *seckey, struct RSAsecPlus *sec);

/* Declarations */
int rsaKeyTooBig(struct RSApub const *pub, struct RSAsec const *sec);

/* Encrypt */
int rsaPublicEncrypt(struct BigNum *bn, byte const *in, unsigned len,
struct RSApub const *pub,
		struct PgpRandomContext const *rc);
/* Sign */
int rsaPrivateEncrypt(struct BigNum *bn, byte const *in, unsigned len,
struct RSAsec const *sec);
/* Verify */
int rsaPublicDecrypt(byte *buf, unsigned len, struct BigNum *bn,
struct RSApub const *pub);
/* Decrypt */
int rsaPrivateDecrypt(byte *buf, unsigned len, struct BigNum *bn,
struct RSAsec const *sec);

/*Modular Exponentiation*/
int
bnExpModCRA(struct BigNum *x, struct BigNum const *d,
	struct BigNum const *p, struct BigNum const *q,
	struct BigNum const *u);

#ifdef __cplusplus
}
#endif

#endif /* !PGPRSAGLUE_H */
