/*
 * pgpRingUI.h
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpRingUI.h,v 1.3.2.2.2.1 1997/08/18 22:38:01 quark Exp $
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

size_t PGPTTYExport ringTtyFormatFingerprint16(struct RingSet const *set,
	union RingObject *key, char *buf, size_t len);
size_t PGPTTYExport ringTtyFormatFingerprint20(struct RingSet const *set,
	union RingObject *key, char *buf, size_t len);
int PGPTTYExport ringTtyPutFingerprint16(PgpOutputType OutputType,
					 struct RingSet const *set,
					 union RingObject *key,
					 unsigned wid);
int PGPTTYExport ringTtyPutFingerprint20(PgpOutputType OutputType,
					 struct RingSet const *set,
					 union RingObject *key,
					 unsigned wid);
int PGPTTYExport ringTtyPutKeyID(Boolean DisplayHeaders,
				 PgpOutputType OutputType,
				 struct RingSet const *set,
				 union RingObject *key);
int ringTtyKeyIDSprintf(char *dest,
			struct RingSet const *set, 
			union RingObject *key);
int PGPTTYExport ringTtyPutSigID(Boolean DisplayHeaders,
				 PgpOutputType OutputType,
				 struct RingSet const *set,
				 union RingObject *sig);
unsigned PGPTTYExport ringTtyPutString(char const *str,
				       size_t len,
				       unsigned maxlen,
				       Boolean DisplayHeaders,
				       PgpOutputType OutputType,
				       char q1,
				       char q2);
void PGPTTYExport ringTtyPutKeyInfo(Boolean DisplayHeaders,
				    PgpOutputType OutputType,
				    struct RingSet const *set,
				    union RingObject *obj);
void PGPTTYExport ringKeyIDprint(Boolean DisplayHeaders,
				 PgpOutputType OutputType,
				 char const *prompt,
				 byte const keyID[8]);
void PGPTTYExport ringKeyPrint(PgpOutputType OutputType,
			       struct RingSet const *set,
			       union RingObject *obj,
			       int level);
void PGPTTYExport
ringObjPrint(PgpOutputType OutputType,
	     struct RingSet const *set,
	     union RingObject *obj,
	     int level);

int PGPTTYExport ringTopListTrouble(Boolean DisplayOutput,
				    PgpOutputType OutputType,
				    struct RingFile *file,
				    int writeflag);

int PGPTTYExport
ringTtyKeyView (struct RingSet const *viewkeys,
		struct RingSet const *allkeys,
		char const *keyring,
		int mode);

/*	Display second segment of -kc output (trust info) only. */

int PGPTTYExport
ringTtyKeyCheck (Boolean DisplayHeaders,
		 PgpOutputType OutputType,
		 struct RingSet const *viewkeys,
		 struct RingSet const *allkeys,
		 int mode);



/*	Show a single key in appropriate key view format.
	Associated names are also displayed.  */

int PGPTTYExport
ringTtyShowKey (Boolean DisplayHeaders,
		PgpOutputType OutputType,
		union RingObject *key,
		struct RingSet const *set,
		int mode);

/*	Show second segment of -kc output for designated key. */

int PGPTTYExport
ringTtyCheckKey (Boolean DisplayHeaders,
		 PgpOutputType OutputType,
		 union RingObject *key,
		 struct RingSet const *set,
		 int mode);



/*	Show signatures in appropriate key view format.  Similar concept to
	pgpTtyShowKey.  The object for the name with which the signatures are
	associated should be passed. */

int PGPTTYExport
ringTtyShowSigs (Boolean DisplayHeader,
		 PgpOutputType OutputType,
		 union RingObject *name,
		 struct RingSet const *set,
		 int mode);

/*	Service routine for ringTtyShowSigs, to show a single sig.  Check
	means to indicate signature check status. */
int PGPTTYExport
ringTtyShowSig(union RingObject *sig,
	       struct RingSet const *set,
	       Boolean DisplayHeader,
	       PgpOutputType OutputType,
	       int check);
/*	Show second segment of -kc output (trust info) for signatures
	attached to designated name. */

int PGPTTYExport
ringTtyCheckSigs (Boolean DisplayHeaders,
		  PgpOutputType OutputType,
		  union RingObject *name,
		  struct RingSet const *set,
		  int mode);

/* Return whether key can be used for encryption */

int PGPTTYExport
ringTtyKeyOKToEncrypt (struct RingSet const *set,
		       union RingObject *key);

void PGPTTYExport
ringTtyKeyOKToSign (struct RingSet const *set,
	union RingObject *key);

#ifdef __cplusplus
}
#endif

#endif /* PGPRINGUI_H */
