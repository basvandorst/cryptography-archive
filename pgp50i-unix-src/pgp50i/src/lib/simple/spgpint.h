/*
* spgpint.h -- Simple PGP API internal header file
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: spgpint.h,v 1.23.2.1 1997/06/07 09:51:55 mhw Exp $
*/

#ifndef SPGPINT_H
#define SPGPINT_H

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#ifndef PGPExport
#define PGPExport
#endif

/* PGP library */

#include "pgpAnnotate.h"
#include "pgpArmor.h"
#include "pgpConf.h"
#include "pgpConvKey.h"
#include "pgpDecPipe.h"
#include "pgpDevNull.h"
#include "pgpEncPipe.h"
#include "pgpEnv.h"
#include "pgpErr.h"
#include "pgpESK.h"
#include "pgpFIFO.h"
#include "pgpFile.h"
#include "pgpFileRef.h"
#include "pgpFileMod.h"
#include "pgpMem.h"
#include "pgpMemMod.h"
#include "pgpPipeline.h"
#include "pgpPubKey.h"
#include "pgpRndom.h"
#include "pgpRndPool.h"
#include "pgpRndSeed.h"
#include "pgpRngPub.h"
#include "pgpRngRead.h"
#include "pgpSig.h"
#include "pgpSigPipe.h"
#include "pgpSigSpec.h"
#include "pgpTextFilt.h"
#include "pgpTrstPkt.h"
#include "pgpTrust.h"
#include "pgpUI.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
* Structs
*/

/* Number of keyid's to record as destinations beside this one */
#define SPGPUI_ESKMAX	3

typedef struct SPgpSimpUI {
		PgpEnv	*env;	 		 /* PGP environment vars */
		RingSet const	*ringset;		/* RingSet to seek for keys */
		word32	sigtimedate;	/* Timestamp of sig we verified */
		byte	sigpkalg; /* PKAlg of key which made sig */
		byte	sigkeyid[8];	/* KeyID of key which made sig */
		char	*passphrase;		/* Pass phrase to unlock key */
		PGPFileRef			*outref;			/* Output filename ref */
		byte	*outbuf;			/* Output buffer, alt. to filename */
		size_t	outbufsize;		/* Initial/final outbuf count */
		PgpPipeline	*pipebuf;			/* Output buf pipeline pointer */
		int	sigstatus;		/* Status of sig verify attempt */
		int	encfail;	 		/* True if decryption failed */
		int	keyfail;	 		/* True if got a key packet */
		int	analyze;	/* Nonzero to request analysis */
		int	addkey;			/* Nonzero to request keyadd */
		int		 			fyeo;		 		/* Incoming message was for eyes only */
	PGPFileOpenFlags localEncode;	 /* MacBinary flags for receive */
		int		 			nesk;		 	/* Number of other esk's encrypted to */
		byte	 			otheresk[SPGPUI_ESKMAX][8];	/* encrypted to others */
		byte	 			otheralg[SPGPUI_ESKMAX];		 /* pkalg's '' */
		int		 			npass;	 		/* # other passwords encrypted to */
		int		 			failpass;			/* Count of conv passphrases failed */
		int		 			failesk;			/* Count of pubkey passphrases failed */
	} SPgpSimpUI;


	/*
	* Prototypes
	*/

int
spgpKeyOKToEncrypt (RingSet const *set, RingObject *key, int UseUntrustedKeys);

int
spgpKeyOKToSign (RingSet const *set, RingObject *key);

PgpPubKey *
spgpRecipToPubkeys (char *RecipientList, RingSet const *rspub,
	int UseUntrustedKeys, PgpEnv *env, int *err);

char const *
spgpNameByID8 (RingSet const *set, byte pkalg, byte keyID[8], size_t *namelen);

void
spgpSignStatus (SPgpSimpUI *ui_arg, RingSet const *rspub, int *psigstatus,
	byte *pkeyid, size_t keyidlen, char *psign, size_t signlen,
	char *pdate, size_t datelen);

RingObject *
spgpGetSignerKey (char *SignerKeyID, size_t SignerBufferLen, PgpEnv *env,
	RingSet const *rset);

PGPError
spgpDecryptStatus (SPgpSimpUI *ui_arg, RingSet const *rspub, char *OtherBuf,
	size_t *OtherBufLen);

int
spgpMemPump (PgpPipeline *head, byte *InputBuffer, size_t InputBufferLen,
	SPGPProgressCallBack callBack, void *callBackArg);

int
spgpMemOutput (PgpPipeline *pipebuf, size_t written, size_t *OutputBufferLen);

int
spgpInit (PgpEnv **envp, RingPool **ringpoolp);

void
spgpFinish(PgpEnv *env0, RingPool *ringpool, int code);

char *
spgpFileNameBuild(char const *seg, ...);

char *
spgpFileNameExtend (char const *base, char const *ext);

char *
spgpFileNameContract (char const *base);

char *
spgpFileNameNextDirectory (char const *path, char const **rest);

char *
spgpFileNameExtendPath (char const *path, char const *dir, int front);

int
spgpOpenRingfiles (PgpEnv *env, RingPool *ringpool,
	PGPFileRef *refPubRing, PGPFileRef *refPrivRing,
	PgpFile **pfpub, RingFile **rfpub, RingSet const **rspub,
	PgpFile **pfsec, RingFile **rfsec, RingSet const **rssec,
	Boolean writable);

int
spgpWriteoutSet (PgpEnv *env, PGPFileRef *fileref, RingSet *set,
	RingFile *oldrfile, PgpFile *pfp, Boolean isPrivate);

int
spgpWriteArmoredSetFile (FILE *fp, RingSet const *set, PgpEnv *env);

int
spgpWriteArmoredSetBuffer (byte *buf, size_t *buflen, RingSet const *set,
	PgpEnv *env);

void
spgpRingFileClose (RingFile *rfile);

void
spgpKeyIDText8 (char *out, byte *keyid);

void
spgpDateText10 (char *out, word32 date);

void
spgpExpText3 (char *out, int exp);

void
spgpUISetup (PgpUICb *ui, SPgpSimpUI *ui_arg);

int
spgpDearmorKeyFile (PgpEnv *env, PGPFileRef *InputFileRef,
	byte **OutputBuffer, size_t *OutputBufferLen);

int
spgpDearmorKeyBuffer (PgpEnv *env, byte *InputBuffer, size_t InputBufferLen,
	byte **OutputBuffer, size_t *OutputBufferLen);

PGPError
spgpRngCheck (void);


#ifdef __cplusplus
}
#endif

#endif /* SPGPINT_H */

/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
