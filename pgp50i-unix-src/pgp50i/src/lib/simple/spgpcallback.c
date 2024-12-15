/*
* spgpcallback.c -- Simple PGP API helper routines to deal with callbacks
* from pipeline on decryption
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: spgpcallback.c,v 1.23.2.2 1997/06/07 09:51:53 mhw Exp $
*/

#include "spgp.h"
#include "spgpint.h"
#include "pgpFileRef.h"
#include "pgpTypes.h"
#include "pgpDebug.h"


/*
* This is used for many of the analyze functions. Once we get our answer,
* we return the INTERRUPTED error so that it doesn't grind through the
* whole message.
*/
static int
spgpAnnotate (void *arg, struct PgpPipeline *origin, int type,
		byte const *string, size_t size)
	{
		SPgpSimpUI *ui_arg;	/* Parameters for callbacks */

	ui_arg = (SPgpSimpUI *) arg;
	(void) origin;
	(void) string;
	(void) size;

	if (ui_arg->analyze < 0) {
		switch (type) {
		case PGPANN_PGPKEY_BEGIN:
		case PGPANN_CIPHER_BEGIN:
		case PGPANN_COMPRESSED_BEGIN:
		case PGPANN_CLEARSIG_BEGIN:
		case PGPANN_SIGNED_SEP:
			/* Abort once we have our answer */
			ui_arg->analyze = type;
			return PGPERR_INTERRUPTED;
		/* Signed may be a detached or a regular sig, need another anno */
		/*		case PGPANN_SIGNED_BEGIN: */
		case PGPANN_SIGNED_SIG:
		case PGPANN_SIGNED_SIG2:
			ui_arg->analyze = PGPANN_SIGNED_BEGIN;
			return PGPERR_INTERRUPTED;
		default:
			return PGPERR_OK;
		}
	} else if (ui_arg->analyze) {
		return PGPERR_INTERRUPTED;
	}
	/* Detect bad parsing situations */
	if (type == PGPANN_ESK_TOO_BIG
		|| type == PGPANN_SIGNATURE_TOO_BIG
		|| type == PGPANN_PACKET_SHORT
		|| type == PGPANN_PACKET_TRUNCATED
		|| type == PGPANN_SCOPE_TRUNCATED) {
		return PGPERR_FILEIO_BADPKT;
	} else if (type == PGPANN_ARMOR_TOOLONG
		|| type == PGPANN_ARMOR_BADLINE
		|| type == PGPANN_ARMOR_NOCRC
		|| type == PGPANN_ARMOR_CRCCANT
		|| type == PGPANN_ARMOR_CRCBAD) {
		return PGPERR_PARSEASC_BADINPUT;
	}
	return PGPERR_OK;
}


/*
* Display a message as appropriate.
* Nothing is appropriate for us, as we have no UI. Assert an error
* if it happens. (Perhaps better simply to ignore it.)
*/
static int
spgpSimpMessage (void *arg, int type, int msg, unsigned numargs, ...)
	{
		SPgpSimpUI *ui_arg;	/* Parameters for callbacks */

		ui_arg = (SPgpSimpUI *) arg;

		(void)type;
		(void)msg;
		(void)numargs;
		return 0;
}

/*
 * Say what to do when we unwrap a layer.
 * We will always fully process messages.
 */
static int
spgpSimpDoCommit (void *arg, int scope)
{
	SPgpSimpUI *ui_arg; /* Parameters for callbacks */

	ui_arg = (SPgpSimpUI *) arg;
	(void) scope;

	/* Now do all analyze actions in annotate callback so we can abort */
#if 0
	if (ui_arg->analyze) {
		if (ui_arg->analyze < 0)
			ui_arg->analyze = scope;
		else if (scope == PGPANN_CLEARSIG_BEGIN)
			ui_arg->analyze = scope;
		return PGPANN_PARSER_EATIT;
	}
#endif
	return PGPANN_PARSER_RECURSE;
}


/* Set up the output pipeline as appropriate */
static int
spgpSimpNewOutput (void *arg, struct PgpPipeline **output, int type,
	char const *suggested_name)
	{
		SPgpSimpUI	*ui_arg;	/* Parameters for callbacks */
		PgpFile	*pgpf;			/* PgpFile output stream */
		PGPFileRef		 		*ref;			/* Ref to output filename */
		int	nullflag;	/* True if discarding output */
		PGPError	 			error;

	ui_arg = (SPgpSimpUI *) arg;
	nullflag = 0;

	/* Try to keep output open if we have multiple key packets in a row */
	if (*output && ui_arg->addkey && ui_arg->pipebuf &&
			ui_arg->outref==NULL && ui_arg->outbuf==NULL) {
		byte enable;
		if (type==PGPANN_NONPGP_BEGIN) {
			 		/*
			* Non-key material found between the two packets. We send a
			* disable annotation to tell the pipebuf module to discard this
			* intermediate data.
			 		*/
			enable = 0;
			(ui_arg->pipebuf)->annotate(ui_arg->pipebuf, NULL,
				PGPANN_MEM_ENABLE, &enable, 1);
			return 0;
		} else if (type==PGPANN_PGPKEY_BEGIN) {
			/*
			* A second or later key packet after first. We may have disabled
			* the mem buffer if there was nonkey material, so re-enable it
			* now.
			*/
			enable = 1;
			(ui_arg->pipebuf)->annotate(ui_arg->pipebuf, NULL,
				PGPANN_MEM_ENABLE, &enable, 1);
			return 0;
		}
	}

	/* Ignore non-PGP portion and later packets */
	if (type == PGPANN_NONPGP_BEGIN ||
		(ui_arg->outref==NULL && ui_arg->outbuf==NULL)) {
		type = PGP_LITERAL_TEXT;
		nullflag = 1;
	}

	/* Discard key packets, or non-key packets if we are adding a key */
	if ((type == PGPANN_PGPKEY_BEGIN) != ui_arg->addkey) {
		/* No longer set keyfail, just skip unexpected key packets. That
		* works better for the case where we are decrypting an incoming
		* message which starts with a key packet.
		*	if (!ui_arg->addkey)
		*		 ++ui_arg->keyfail;
			*/
		nullflag = 1;
	}

	/* Detect magic filename which means "for your eyes only" */
	ui_arg->fyeo = suggested_name && 0==strcmp (suggested_name, "_CONSOLE");

	/* No support for other types than text or binary now */
	if (type != PGP_LITERAL_TEXT)
		type = PGP_LITERAL_BINARY;

	/* Close old output pipeline */
	if (*output) {
		if (ui_arg->pipebuf) {
			(ui_arg->pipebuf)->annotate(ui_arg->pipebuf, NULL,
				PGPANN_MEM_BYTECOUNT, (byte *)&ui_arg->outbufsize,
				sizeof(ui_arg->outbufsize));
			ui_arg->pipebuf = 0;
		}
		(*output)->sizeAdvise (*output, 0);
		(*output)->teardown (*output);
		*output = NULL;
	}

	/* Handle discarded data */
	if (nullflag) {
		pgpDevNullCreate (output);
		return 0;
	}

	if (type==PGP_LITERAL_TEXT) {
		/* Convert to local line endings if appropriate */
		output = pgpTextFiltCreate (output,
		pgpenvGetPointer (ui_arg->env, PGPENV_CHARMAPTOLATIN1, NULL,
					 		 NULL),
		0, SimplePGPGetLineEndType());
	}

	if (ui_arg->outbuf) {
		ui_arg->pipebuf = pgpMemModCreate (output, (char *)ui_arg->outbuf,
			ui_arg->outbufsize);
		ui_arg->outbuf = NULL;
		if (!ui_arg->pipebuf)
			return PGPERR_NOMEM;
		return 0;
	}

	/* Only use output name ref once, discard other packets */

	ref = ui_arg->outref;
	ui_arg->outref = NULL;

	pgpAssert((ui_arg->localEncode & ~(kPGPFileOpenMaybeLocalEncode
									| kPGPFileOpenNoMacBinCRCOkay)) == 0);

	/* Create output file pipeline */
	pgpf = pgpFileRefOpen(ref, (kPGPFileOpenStdWriteFlags |
								ui_arg->localEncode),
						(type == PGP_LITERAL_TEXT ?
							kPGPFileTypeDecryptedText :
							kPGPFileTypeDecryptedBin), &error);
	if (!pgpf)
		return error;
	if (!pgpFileWriteCreate (output, pgpf, 1)) {
		return PGPERR_NOMEM;
	}
	return 0;
}


/*
* Look for input file when we find a detached signature.
* We can't do that.
*/
static int
spgpSimpNeedInput (void *arg, struct PgpPipeline *head)
{
	SPgpSimpUI *ui_arg; /* Parameters for callbacks */

	ui_arg = (SPgpSimpUI *) arg;
	(void)head;

	return SIMPLEPGP_DETACHEDSIGNATUREFOUND;
}


/*
* Given the signature structure, sig, verify it against the hash
* to see if this signature is valid. This requires looking up the
* public key in the keyring and validating the key.
*
* Returns 0 on success or an error code.
*/
static int
spgpSimpSigVerify (void *arg, struct PgpSig const *sig, byte const *hash)
{
	SPgpSimpUI	*ui_arg; /* Parameters for callbacks */
	RingObject	*ringobj;	/* Public key ring object */
	PgpPubKey	*pubkey;		/* Public key for verify */
	byte const	*keyid;		/* KeyID from signature */
	byte	pkalg;		/* Public key alg from signature */
	int	err;			/* Error from pgplib */
	
	ui_arg = (SPgpSimpUI *) arg;
	keyid = pgpSigId8 (sig);
	pkalg = pgpSigPKAlg (sig);
	pubkey = NULL;
	ringobj = NULL;

	if (ui_arg->analyze < 0) {
			/*
		* Clearsign messages get here without going through commit.
		* (Not clear if this is necessary any more, now that we are doing
		* analyze in annotate callback.)
			*/
		ui_arg->analyze = PGPANN_CLEARSIG_BEGIN;
		return 0;
	}

	if (ui_arg->ringset) {
		ringobj = ringKeyById8 (ui_arg->ringset, pkalg, keyid);
		if (ringobj) {
			pubkey = ringKeyPubKey (ui_arg->ringset, ringobj, 0);
		}
	}
	if (pubkey && !pubkey->verify) {
		/* Make sure we can use this key */
		pgpPubKeyDestroy (pubkey);
		pubkey = NULL;
	}
	
	if (!pubkey) {
		ui_arg->sigstatus = SIGSTS_NOTVERIFIED;
		ringObjectRelease (ringobj);
		return 0;
	}
	
	err = pgpSigCheck (sig, pubkey, hash);
	pgpPubKeyDestroy (pubkey);
	
	if (err == 1) {
		if (spgpKeyOKToSign (ui_arg->ringset, ringobj))
			ui_arg->sigstatus = SIGSTS_VERIFIED;
		else
			ui_arg->sigstatus = SIGSTS_VERIFIED_UNTRUSTED;
	} else {
		ui_arg->sigstatus = SIGSTS_BADSIG;
	}
	ringObjectRelease (ringobj);
		
	ui_arg->sigtimedate = pgpSigTimestamp (sig);
	memcpy (ui_arg->sigkeyid, pgpSigId8 (sig), sizeof(ui_arg->sigkeyid));
	ui_arg->sigpkalg = pgpSigPKAlg (sig);

	return 0;
}


/*
* given a list of Encrypted Session Keys (esklist), try to decrypt
* them to get the session key. Fills in keylen with the length of
* the session key buffer.
*
* Returns 0 on success or PGPANN_PARSER_EATIT on failure.
*/
static int
spgpSimpEskDecrypt (void *arg, struct PgpESK const *esklist, byte *key,
		size_t *keylen,
		int (*tryKey) (void *arg, byte const *key, size_t keylen),
		void *tryarg)
{
		SPgpSimpUI	*ui_arg; /* Parameters for callbacks */
		PgpESK const	*esk; /* ESK being tested */
		RingObject	*ringobj;	 /* Secret key ring object */
		PgpSecKey	*seckey;	/* Secret key */
		byte const	*keyid;	/* Key ID from ESK */
		size_t	passlen;	/* Length of pass phase from UI */
	byte pkalg;	/* Pubkey algorithm from ESK */
	int					success=0; /* True if had a successful decrypt */
	int err;		/* Error from pgplib */

	ui_arg = (SPgpSimpUI *) arg;
	passlen = strlen (ui_arg->passphrase);
	seckey = NULL;
	ringobj = NULL;

	/* Loop over all ESK's trying to find one we can decrypt */
	for (esk = esklist; esk; esk = pgpEskNext (esk)) {
		switch (pgpEskType (esk)) {
		case PGP_ESKTYPE_PASSPHRASE:
			/* Try our pass phrase */
			if (success) {			/* If already succeeded, skip */
				ui_arg->npass += 1;
				break;
			}
			err = pgpEskConvDecrypt (esk, ui_arg->env,
							ui_arg->passphrase, passlen, key);
			*keylen = err;
			if (err >= 0) {
				err = tryKey (tryarg, key, *keylen);
				if (err) {
					/* Record bad passphrase */
					++ui_arg->failpass;
				}
			}
			if (!err) {
				/* Success */
				success = 1;
			}
			break;

		case PGP_ESKTYPE_PUBKEY:
			/* Look up key from ESK */
			keyid = pgpEskId8 (esk);
			pkalg = pgpEskPKAlg (esk);
			
			/* If already succeeded, just record the count and quit */
			if (success) {
				if (ui_arg->nesk < SPGPUI_ESKMAX) {
					memcpy (ui_arg->otheresk[ui_arg->nesk], keyid, 8);
					ui_arg->otheralg[ui_arg->nesk] = pkalg;
				}
				ui_arg->nesk += 1;
				break;
			}
			
			/* seckey != NULL is used as a flag here for success so far */
			seckey = NULL;
			if (ui_arg->ringset) {
				ringobj = ringKeyById8 (ui_arg->ringset, pkalg, keyid);
				if (ringobj) {
					seckey = ringSecSecKey (ui_arg->ringset, ringobj,
											PGP_PKUSE_ENCRYPT);
					ringObjectRelease (ringobj);
				}
			}

			/* See if have a good decryption key */
			if (seckey && !seckey->decrypt) {
				/* A matching secret key of a type which can't decrypt? */
				pgpSecKeyDestroy (seckey);
				seckey = NULL;
			}

			/* Unlock decryption key */
			if (seckey) {
				err = pgpSecKeyUnlock (seckey, ui_arg->env,
								ui_arg->passphrase, passlen);
				if (err <= 0) {
					/* Pass phrase failed to unlock */
					++ui_arg->failesk;
					pgpSecKeyDestroy (seckey);
					seckey = NULL;
				}
			}

			/* Try decrypting the ESK */
			if (seckey) {
				err = pgpEskPKdecrypt (esk, ui_arg->env, seckey, key);
				*keylen = err;
				pgpSecKeyDestroy (seckey);
				if (err <= 0) {
					/* Failed to decrypt */
					seckey = NULL;
				}
			}

			/* Now try the decrypted ESK against the rest of the message */
			if (seckey) {
				err = tryKey (tryarg, key, *keylen);
				if (err) {
					/* Failure */
					seckey = NULL;
				}
			}

			/* Did everything work? */
			if (seckey) {
				success = 1;
			} else {
				/* If failed, add to list of "other" esk's */
				if (ui_arg->nesk < SPGPUI_ESKMAX) {
					memcpy (ui_arg->otheresk[ui_arg->nesk], keyid, 8);
					ui_arg->otheralg[ui_arg->nesk] = pkalg;
				}
				ui_arg->nesk += 1;
				break;
			}
			break;

		default:
			/* Unknown ESK type */
			pgpAssert (0);
			break;
		}
	}

	if (success)
		return 0;

	/*
	* At this point, none of the PgpESKs have been decrypted, so let's
	* inform the user that we failed.
	*/
	ui_arg->encfail = 1;
	return PGPERR_INTERRUPTED;	/* Will be PGPERR_CB_INVALID for app */
}


/* Set up the UI pointers and clear the ui_arg */
void
spgpUISetup (PgpUICb *ui, SPgpSimpUI *ui_arg)
{
	ui->message = spgpSimpMessage;
	ui->doCommit = spgpSimpDoCommit;
	ui->newOutput = spgpSimpNewOutput;
	ui->needInput = spgpSimpNeedInput;
	ui->sigVerify = spgpSimpSigVerify;
	ui->eskDecrypt = spgpSimpEskDecrypt;
	ui->annotate = spgpAnnotate;
	
	memset (ui_arg, 0, sizeof (*ui_arg));
}

/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
