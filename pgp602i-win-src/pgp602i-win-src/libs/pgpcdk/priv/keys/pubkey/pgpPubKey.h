/*
 * pgpPubKey.h -- Structures for PGP Public/Private Keys
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpPubKey.h,v 1.25 1998/06/11 18:28:21 hal Exp $
 */

#ifndef Included_pgpPubKey_h
#define Included_pgpPubKey_h

#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS

#include "pgpOpaqueStructs.h"
#include "pgpPublicKey.h"
#include "pgpStr2Key.h"


struct PGPPubKey {
	PGPContextRef	context;
	
	PGPPubKey *next;
	PGPByte pkAlg;
	PGPByte keyID[8];
	void *priv;
	void (*destroy) (PGPPubKey *pubkey);
	/* The sizes of buffers required for encrypt, etc. */
	size_t (*maxesk) (PGPPubKey const *pubkey,
					PGPPublicKeyMessageFormat format);
	size_t (*maxdecrypted) (PGPPubKey const *pubkey,
					PGPPublicKeyMessageFormat format);
	size_t (*maxsig) (PGPPubKey const *pubkey,
					PGPPublicKeyMessageFormat format);
	int (*encrypt) (PGPPubKey const *pubkey, PGPByte const *key,
					size_t keylen, PGPByte *esk, size_t *esklen,
					PGPRandomContext const *rc,
					PGPPublicKeyMessageFormat format);
	int (*verify) (PGPPubKey const *pubkey,
					PGPByte const *sig, size_t siglen,
					PGPHashVTBL const *h, PGPByte const *hash,
					PGPPublicKeyMessageFormat format);
	size_t (*bufferLength)(PGPPubKey const *pubkey);
	void (*toBuffer)(PGPPubKey const *pubkey, PGPByte *buf);
};


struct PGPSecKey {
	PGPContextRef	context;
	
	PGPByte pkAlg;
	PGPByte keyID[8];
	void *priv;
	void (*destroy) (PGPSecKey *seckey);
	PGPPubKey * (*pubkey) (PGPSecKey const *seckey);
	int (*islocked) (PGPSecKey const *seckey);
	PGPError (*lockingalgorithm) (PGPSecKey const *seckey,
					PGPCipherAlgorithm *alg, PGPSize *algKeySize);
	PGPError (*s2ktype) (PGPSecKey const *seckey,
					PGPStringToKeyType *s2kType);
	PGPError (*convertpassphrase) (PGPSecKey *seckey, PGPEnv const *env,
					char const *phrase, PGPSize plen, PGPByte *outbuf);
	int (*unlock) (PGPSecKey *seckey, PGPEnv const *env,
					char const *phrase, size_t plen, PGPBoolean hashedPhrase);
	void (*lock) (PGPSecKey *seckey);
	/* The sizes of buffers required for decrypt, etc. */
	size_t (*maxesk) (PGPSecKey const *seckey,
					PGPPublicKeyMessageFormat format);
	size_t (*maxdecrypted) (PGPSecKey const *seckey,
					PGPPublicKeyMessageFormat format);
	size_t (*maxsig) (PGPSecKey const *seckey,
					PGPPublicKeyMessageFormat format);
	int (*decrypt) (PGPSecKey *seckey, PGPEnv const *env,
					PGPByte const *esk, size_t esklen,
					PGPByte *key, size_t *keylen,
					char const *phrase, size_t plen,
					PGPPublicKeyMessageFormat format);
	int (*sign) (PGPSecKey *seckey,
					PGPHashVTBL const *h, PGPByte const *hash,
					PGPByte *sig, size_t *siglen,
					PGPRandomContext const *rc,
					PGPPublicKeyMessageFormat format);
	int (*changeLock)(PGPSecKey *seckey, PGPEnv const *env,
					PGPRandomContext const *rc,
					char const *phrase, size_t plen,
					PGPStringToKeyType s2ktype);
	size_t (*bufferLength)(PGPSecKey const *seckey);
	void (*toBuffer)(PGPSecKey const *seckey, PGPByte *buf);
};


#include "pgpKeys.h"	/* to get 'PGPPublicKeyAlgorithm' type */
/*
 * Note on kPGPPublicKeyAlgorithm_RSAEncryptOnly/SignOnly:
 *
 * These are ViaCrypt's "restricted" versions of RSA.  There are reasons
 * to want PGP to limit you in this way.  Some forces which might try
 * to force disclosure of your key (such as courts) can be dissuaded on
 * the grounds that nothing is being hidden by the keys.
 *
 * The *annoying* thing, however, is that ViaCrypt chose to leave the
 * encrypted session kay and signature packets with a pkalg byte of 1.
 * Which means that various bits of code contain kludges to deal with
 * this fact.
 */

#define PGP_PKUSE_SIGN          0x01
#define PGP_PKUSE_ENCRYPT       0x02
#define PGP_PKUSE_SIGN_ENCRYPT  (PGP_PKUSE_SIGN | PGP_PKUSE_ENCRYPT)

#define pgpPubKeyNext(p)	(p)->next
#define pgpPubKeyDestroy(p)	(p)->destroy(p)
#define pgpPubKeyMaxesk(p,f)	(p)->maxesk(p,f)
#define pgpPubKeyMaxdecrypted(p,f)	(p)->maxdecrypted(p,f)
#define pgpPubKeyMaxsig(p,f)	(p)->maxsig(p,f)
#define pgpPubKeyMaxsighash(p,f)	(p)->maxsighash(p,f)
#define pgpPubKeyEncrypt(p,k,kl,e,el,r,f)	(p)->encrypt(p,k,kl,e,el,r,f)
#define pgpPubKeyVerify(p,s,sl,h,ha,f)	(p)->verify(p,s,sl,h,ha,f)
#define pgpPubKeyBufferLength(p)	(p)->bufferLength(p)
#define pgpPubKeyToBuffer(p,b)		(p)->toBuffer(p,b)

#define pgpSecKeyDestroy(s)	(s)->destroy(s)
#define pgpSecKeyPubkey(s)	(s)->pubkey(s)
#define pgpSecKeyIslocked(s)	(s)->islocked(s)
#define pgpSecKeyLockingalgorithm(s,a,k)	(s)->lockingalgorithm(s,a,k)
#define pgpSecKeyS2Ktype(s,t)	(s)->s2ktype(s,t)
#define pgpSecKeyConvertPassphrase(s,e,p,l,o) \
					(s)->convertpassphrase(s,e,p,l,o)
#define pgpSecKeyIslocked(s)	(s)->islocked(s)
#define pgpSecKeyUnlock(s,e,p,pl,hp)	(s)->unlock(s,e,p,pl,hp)
#define pgpSecKeyLock(s)	(s)->lock(s)
#define pgpSecKeyMaxesk(s,f)	(s)->maxesk(s,f)
#define pgpSecKeyMaxdecrypted(s,f)	(s)->maxdecrypted(s,f)
#define pgpSecKeyMaxsig(s,f)	(s)->maxsig(s,f)
#define pgpSecKeyMaxsighash(s,f)	(s)->maxsighash(s,f)
#define pgpSecKeyDecrypt(s,env,e,el,k,kl,p,pl,f)	(s)->decrypt(s,env,\
		e,el,k,kl,p,pl,f)
#define pgpSecKeySign(s,h,ha,si,sil,r,f)	(s)->sign(s,h,ha,si,sil,r,f)
#define pgpSecKeyChangeLock(s,e,r,p,pl,t)	(s)->changeLock(s,e,r,p,pl,t)
#define pgpSecKeyBufferLength(s)	(s)->bufferLength(s)
#define pgpSecKeyToBuffer(s,b)		(s)->toBuffer(s,b)

PGPPkAlg const  *pgpPkalgByNumber(PGPByte pkalg);
int  pgpKeyUse(PGPPkAlg const *pkAlg); 
size_t  pgpPubKeyPrefixSize(PGPByte pkAlg, PGPByte const *p, size_t len);

PGPPubKey  *	pgpPubKeyFromBuf(PGPContextRef	context,
					PGPByte pkAlg, PGPByte const *p, size_t len,
					PGPError *error);
					
PGPSecKey  *	pgpSecKeyFromBuf( PGPContextRef	context,
					PGPByte pkAlg, PGPByte const *p, size_t len, PGPBoolean v3,
					PGPError *error);

unsigned			pgpSecKeyEntropy(PGPPkAlg const *pkAlg,
						unsigned bits, PGPBoolean fastgen);
						
PGPSecKey  *	pgpSecKeyGenerate(PGPContextRef	context,
					PGPPkAlg const *pkAlg,
					unsigned bits, PGPBoolean fastgen,
					PGPRandomContext const *rc,
					int (*progress)(void *arg, int c),
					void *arg, PGPError *error);

PGPBoolean		pgpIsKeyRelatedError( PGPError err );


PGP_END_C_DECLARATIONS

#endif /* Included_pgpPubKey_h */
