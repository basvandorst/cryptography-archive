/*
 * pgpkUI.h
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: pgpkUI.h,v 1.1.2.2 1997/06/24 13:26:03 quark Exp $
 */

#ifndef PGPRINGUI_H
#define PGPRINGUI_H

#include <stdio.h>

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct RingFile;
#ifndef TYPE_RINGFILE
#define TYPE_RINGFILE 1
typedef struct RingFile RingFile;
#endif

union RingObject;
#ifndef TYPE_RINGOBJECT
#define TYPE_RINGOBJECT 1
typedef union RingObject RingObject;
#endif

struct RingSet;
#ifndef TYPE_RINGSET
#define TYPE_RINGSET 1
typedef struct RingSet RingSet;
#endif

size_t
kdbTtyFormatFingerprint16(char *hash, char *buf, size_t len);

size_t
kdbTtyFormatFingerprint20(char *hash, char *buf, size_t len);

int
kdbTtyPutFingerprint16(Boolean DisplayHeaders,
		       PgpOutputType OutputType,
		       char *hash,
		       unsigned wid);

int
kdbTtyPutFingerprint20(Boolean DisplayHeaders,
		       PgpOutputType OutputType,
		       char *hash,
		       unsigned wid);

int
kdbTtyPutKeyID(Boolean DisplayHeaders,
	       PgpOutputType OutputType,
	       char *keyid);

int
kdbTtyPutSigID(Boolean DisplayHeaders,
	       PgpOutputType OutputType,
	       PGPCert *sig);

unsigned
kdbTtyPutString(char const *str,
		size_t len,
		unsigned maxlen,
		Boolean DisplayHeaders,
		PgpOutputType OutputType,
		char q1,
		char q2);

void
kdbTtyPutKeyInfo(Boolean DisplayHeaders,
		 PgpOutputType OutputType,
		 PGPKey *key);

void
kdbKeyPrint(PgpOutputType OutputType, PGPKeyIter *keyiter, int level);


int
kdbTtyKeyView (PGPKeyList *viewkeys,
	       PGPKeySet *allkeys,
	       char const *keyring, int mode);

/*	Display second segment of -kc output (trust info) only. */
				
int
kdbTtyKeyCheck (PgpOutputType OutputType, PGPKeyList *viewkeys,
		PGPKeySet *allkeys, int mode);
		
		
		
/*	Show a single key in appropriate key view format.
	Associated names are also displayed.  */
				
int
kdbTtyShowKey (PgpOutputType OutputType,
	       PGPKeyIter *iter,
	       PGPKeySet *allkeys,
	       int mode);
				
/*	Show second segment of -kc output for designated key. */

int
kdbTtyCheckKey (PgpOutputType OutputType,
		PGPKeyIter *iter,
		PGPKeySet *allkeys,
		int mode);

/*	Show signatures in appropriate key view format. Similar concept to
	pgpTtyShowKey. The object for the name with which the signatures are
	associated should be passed. */
				
int
kdbTtyShowSigs (PgpOutputType OutputType,
		PGPKeyIter *iter,
		PGPKeySet *allkeys,
		int mode);
			
/*	Service routine for kdbTtyShowSigs, to show a single sig.  Check
 	means to indicate signature check status. */

int
kdbTtyShowSig(PgpOutputType OutputType,
	      PGPKeyIter *sigiter,
	      PGPKeySet *allkeys,
	      int check);


/*	Show second segment of -kc output (trust info) for signatures
	attached to designated name. */			
				
int
kdbTtyCheckSigs (PgpOutputType OutputType,
		 PGPKeyIter *iter,
		 PGPKeySet *allkeys,
		 int mode);


#ifdef __cplusplus
}
#endif

#endif /* PGPRINGUI_H */
