/*
* pgpPubKey.h -- Structures for PGP Public/Private Keys
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:	Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpPubKey.h,v 1.4.2.3 1997/06/07 09:51:29 mhw Exp $
*/

#ifndef PGPPUBKEY_H
#define PGPPUBKEY_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpEnv;
#ifndef TYPE_PGPENV
#define TYPE_PGPENV 1
typedef struct PgpEnv PgpEnv;
#endif

struct PgpHash;
#ifndef TYPE_PGPHASH
#define TYPE_PGPHASH 1
typedef struct PgpHash PgpHash;
#endif

struct PgpRandomContext;
#ifndef TYPE_PGPRANDOMCONTEXT
#define TYPE_PGPRANDOMCONTEXT 1
typedef struct PgpRandomContext PgpRandomContext;
#endif

struct PgpStringToKey;
#ifndef TYPE_PGPSTRINGTOKEY
#define TYPE_PGPSTRINGTOKEY 1
typedef struct PgpStringToKey PgpStringToKey;
#endif

struct PgpPkAlg;
#ifndef TYPE_PGPPKALG
#define TYPE_PGPPKALG 1
typedef struct PgpPkAlg PgpPkAlg;
#endif

struct PgpPubKey {
			struct PgpPubKey *next;
			byte pkAlg;
			byte keyID[8];
			void *priv;
			void (*destroy) (struct PgpPubKey *pubkey);
			/* The size of buffer required for encrypt */
			size_t (*maxesk) (struct PgpPubKey const *pubkey, PgpVersion version);
			int (*encrypt) (struct PgpPubKey const *pubkey, byte const *key,
			size_t keylen, byte *esk, size_t *esklen,
			struct PgpRandomContext const *rc, PgpVersion version);
			int (*verify) (struct PgpPubKey const *pubkey, int sigtype,
			byte const *sig, size_t siglen,
			struct PgpHash const *h, byte const *hash);
			size_t (*bufferLength)(struct PgpPubKey const *pubkey);
			void (*toBuffer)(struct PgpPubKey const *pubkey, byte *buf);
};
#ifndef TYPE_PGPPUBKEY
#define TYPE_PGPPUBKEY 1
typedef struct PgpPubKey PgpPubKey;
#endif

struct PgpSecKey {
			byte pkAlg;
			byte keyID[8];
			void *priv;
			void (*destroy) (struct PgpSecKey *seckey);
			struct PgpPubKey * (*pubkey) (struct PgpSecKey const *seckey);
			int (*islocked) (struct PgpSecKey const *seckey);
			int (*unlock) (struct PgpSecKey *seckey, struct PgpEnv const *env,
				 char const *phrase, size_t plen);
			void (*lock) (struct PgpSecKey *seckey);
			size_t (*maxdecrypted) (struct PgpSecKey const *seckey);
			int (*decrypt) (struct PgpSecKey *seckey, struct PgpEnv const *env,
				 	int esktype, byte const *esk, size_t esklen,
			byte *key, size_t *keylen, char const *phrase,
				 	size_t plen);
			size_t (*maxsig) (struct PgpSecKey const *seckey, PgpVersion version);
			int (*sign) (struct PgpSecKey *seckey, struct PgpHash const *h,
			byte const *hash, byte *sig, size_t *siglen,
			struct PgpRandomContext const *rc, PgpVersion version);
			int (*changeLock)(struct PgpSecKey *seckey, struct PgpEnv const *env,
			struct PgpRandomContext const *rc,
				 	char const *phrase, size_t plen);
			size_t (*bufferLength)(struct PgpSecKey const *seckey);
			void (*toBuffer)(struct PgpSecKey const *seckey, byte *buf);
	};
#ifndef TYPE_PGPSECKEY
#define TYPE_PGPSECKEY 1
typedef struct PgpSecKey PgpSecKey;
#endif

#define PGP_PKALG_RSA	1
/*
* These are ViaCrypt's "restricted" versions of RSA. There are reasons
* to want PGP to limit you in this way. Some forces which might try
* to force disclosure of your key (such as courts) can be dissuaded on
* the grounds that nothing is being hidden by the keys.
*
* The *annoying* thing, however, is that ViaCrypt chose to leave the
* encrypted session kay and signature packets with a pkalg byte of 1.
* Which means that various bits of code contain kludges to deal with
* this fact.
*/
#define PGP_PKALG_RSA_ENC	2
#define PGP_PKALG_RSA_SIG	3

#define PGP_PKALG_ELGAMAL	0x10	 /* A.K.A. Diffie-Hellman */
#define PGP_PKALG_DSA		 0x11

#define PGP_PKUSE_SIGN 0x01
#define PGP_PKUSE_ENCRYPT 0x02
#define PGP_PKUSE_SIGN_ENCRYPT (PGP_PKUSE_SIGN | PGP_PKUSE_ENCRYPT)

#define pgpPubKeyNext(p)	(p)->next
#define pgpPubKeyDestroy(p)	 (p)->destroy(p)
#define pgpPubKeyMaxesk(p,v)	(p)->maxesk(p,v)
#define pgpPubKeyEncrypt(p,k,kl,e,el,r,v)	(p)->encrypt(p,k,kl,e,el,r,v)
#define pgpPubKeyVerify(p,t,s,sl,h,ha)	(p)->verify(p,t,s,sl,h,ha)
#define pgpPubKeyBufferLength(p)	(p)->bufferLength(p)
#define pgpPubKeyToBuffer(p,b)		(p)->toBuffer(p,b)

#define pgpSecKeyDestroy(s)	 (s)->destroy(s)
#define pgpSecKeyPubkey(s)	(s)->pubkey(s)
#define pgpSecKeyIslocked(s)	(s)->islocked(s)
#define pgpSecKeyUnlock(s,e,p,pl)	(s)->unlock(s,e,p,pl)
#define pgpSecKeyLock(s)	(s)->lock(s)
#define pgpSecKeyMaxdecrypted(s)	(s)->maxdecrypted(s)
#define pgpSecKeyDecrypt(s,env,t,e,el,k,kl,p,pl)	(s)->decrypt(s,env,\
		t,e,el,k,kl,p,pl)
#define pgpSecKeyMaxsig(s,v)	(s)->maxsig(s,v)
#define pgpSecKeySign(s,h,ha,si,sil,r,v)	(s)->sign(s,h,ha,si,sil,r,v)
#define pgpSecKeyChangeLock(s,e,r,p,pl)	(s)->changeLock(s,e,r,p,pl)
#define pgpSecKeyBufferLength(s)	(s)->bufferLength(s)
#define pgpSecKeyToBuffer(s,b)		(s)->toBuffer(s,b)

struct PgpPkAlg const PGPExport *pgpPkalgByNumber(byte pkalg);
int PGPExport pgpKeyUse(struct PgpPkAlg const *pkAlg);
size_t PGPExport pgpPubKeyPrefixSize(byte pkAlg, byte const *p, size_t len);

struct PgpPubKey PGPExport *
pgpPubKeyFromBuf(byte pkAlg, byte const *p, size_t len, int *error);
struct PgpSecKey PGPExport *
pgpSecKeyFromBuf(byte pkAlg, byte const *p, size_t len, int *error);

unsigned PGPExport pgpSecKeyEntropy(struct PgpPkAlg const *pkAlg,
	unsigned bits, Boolean fastgen);
struct PgpSecKey PGPExport *pgpSecKeyGenerate(struct PgpPkAlg const *pkAlg,
	unsigned bits, Boolean fastgen,
	struct PgpRandomContext const *rc, int (*progress)(void *arg, int c),
	void *arg, int *error);

#ifdef __cplusplus
}
#endif

#endif /* PGPPUBKEY_H */
