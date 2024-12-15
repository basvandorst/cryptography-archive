/*
 * filter.c - Functions to turn a recipient name into a ringset.
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * Written by:	Brett A. Thomas <quark@baz.com>
 *
 * This draws heavily on Colin Plumb's original work in pgpRingUI.c,
 * but expands it greatly to allow much finer control over what User IDs
 * are checked for validity.
 *
 * $Id: filter.c,v 1.1.2.6 1997/10/07 23:57:49 quark Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "pgpDebug.h"
#include "pgpConf.h"
#include "pgpEnv.h"
#include "pgpOutput.h"
#include "pgpRngPub.h"
#include "pgpRingUI.h"
#include "pgpPubKey.h"
#include "pgpTimeDate.h"
#include "pgpUserIO.h"
#include "pgpTrstPkt.h"
#include "pgpMem.h"

#include "filter.h"
#include "pgpExit.h"

static Boolean keyRevokedOK(struct RingSet *ringset,
			    union RingObject *key);
static Boolean keyExpiredOK(struct RingSet *ringset,
			    union RingObject *key,
			    Boolean InBatchMode);
static Boolean keyValidityOK(struct RingSet *ringset,
			     struct RingIterator *iter,
			     union RingObject *key,
			     char *recipient,
			     Boolean InBatchMode,
			     Boolean FailOnInvalidKeys);
int AddMRKs(union RingObject *key,
	    struct RingSet const *src,
	    struct RingSet *dest);

/*This is from pgpRngPub.c.  I didn't want to modify the header because
 *it would make this visible to other platforms.
 */
int
FilterSingleObj(struct RingIterator *iter,
		union RingObject *obj, char *string,
		int use, const char **MatchedName,
		Boolean *MatchedKeyID);



int addRecipientToRingSet(struct PgpEnv *env,
			  struct RingPool *ringpool,
			  struct RingSet const *src,
			  struct RingSet *dest,
			  char *recipient)
{
    struct RingSet *tempset = NULL;
    union RingObject *key;
    int NumMatches = 0, err;
    Boolean InBatchMode, FailOnInvalidKeys, IsFirstKey = TRUE;
    struct RingIterator *ringiter = NULL;

    pgpAssert(env);
    pgpAssert(src);
    pgpAssert(dest);
    pgpAssert(recipient);

    tempset = ringSetCreate(ringpool);

    InBatchMode = pgpenvGetInt(env, PGPENV_BATCHMODE, NULL, NULL);
    FailOnInvalidKeys = pgpenvGetInt(env, 
				     PGPENV_NOBATCHINVALIDKEYS, 
				     NULL, 
				     NULL);

    /*We filter twice, here.  This is REALLY inneficient, but
     *it will be fixed in 5.5 when the ringset stuff goes south,
     *anyway.
     */
    if((NumMatches = ringSetFilterSpec(src,
				       tempset,
				       recipient,
				       PGP_PKUSE_ENCRYPT)) == 0)
    {
	ErrorOutput(TRUE,
		    LEVEL_SEVERE, 
		    "NO_ENCRYPTION_KEYS_FOUND_FOR",
		    recipient);
    }
    else
    {
	if(NumMatches < 0)
	{
	    exitUsage(NumMatches);
	}
    }

    /*Now, we want to loop through all the recipients we matched and
     *figure out if they're keys we can encrypt to.  This may involve
     *some user interaction, depending exactly what the circumstances
     *are.
     *
     *Here's the matrix of possible cases and actions:
     *
     *               Key Is
     *
     *|Disabled|Revoked|Expired|Invalid|
     *| Fail   | Fail  |  Ask  | Ask(1)| Interactive Mode
     *| Fail   | Fail  |  Fail |Fail(2)| Batch Mode
     *
     *(1):  We only actually ask if this is the first time the
     *      userid has been used
     *
     *(2):  We don't fail in batch mode if FailOnInvalidKeys
     *      is set to FALSE (which is NOT the default, for security reasons).
     *      Additionally, if we've already used this user ID (as in note 1)
     *      we go ahead.
     *      
     */

    ringSetFreeze(tempset);
    ringiter = ringIterCreate(tempset);
    while((err = ringIterNextObject(ringiter, 1)) > 0) {
	key = ringIterCurrentObject(ringiter, 1);
	if (ringObjectType (key) == RINGTYPE_KEY) {
#if 0
	    char keyid[64];
#endif

	    /*Assume we don't like this key:*/
	    --NumMatches;

	    if(!IsFirstKey) {
		InteractionOutputString(TRUE, "\n");
	    }
	    else {
		IsFirstKey = FALSE;
	    }

	    ringKeyPrint(OUTPUT_INFORMATION, tempset, key, 1);

	    if(!ringKeyDisabled(tempset, key)) {
		if(keyRevokedOK(tempset, key)) {
		    if(keyExpiredOK(tempset, key, InBatchMode)) {
			if(keyValidityOK(tempset, 
					 ringiter,
					 key, 
					 recipient,
					 InBatchMode,
					 FailOnInvalidKeys)) {
			    ringSetAddObjectChildren(dest, src, key);
			    ++NumMatches;
#if 0
			    ringTtyKeyIDSprintf(keyid,
						tempset,
						key);
			    NumMatches += ringSetFilterSpec(src,
							    dest,
							    keyid,
							    PGP_PKUSE_ENCRYPT);
#endif
			    NumMatches += AddMRKs(key, src, dest);
			}
		    }
		}
	    }
	    else {
		WarningOutput(TRUE,
			      LEVEL_SEVERE,
			      "ABOVE_KEY_DISABLED");
	    }
	}
    }

    if(NumMatches > 1)
    {
	WarningOutput(TRUE,
		      LEVEL_INFORMATION,
		      "MULTIPLE_RECIPIENTS_MATCHED",
		      NumMatches,
		      recipient);
	InteractionOutputString(TRUE, "\n");
    }
    else {
	InteractionOutputString(TRUE, "\n");
    }

    if(ringiter)
    {
	ringIterDestroy(ringiter);
    }

    if(tempset)
    {
	ringSetDestroy(tempset);
    }

    return(NumMatches);
}

static Boolean keyRevokedOK(struct RingSet *ringset,
			    union RingObject *key)
{
    Boolean KeyOK = TRUE;

    if (ringKeyRevoked(ringset, key)) {
	WarningOutput(TRUE,
		      LEVEL_SEVERE, 
		      "ABOVE_KEY_REVOKED");
	KeyOK = FALSE;
    }
    
    return(KeyOK);
}

static Boolean keyExpiredOK(struct RingSet *ringset,
			    union RingObject *key,
			    Boolean InBatchMode)
{
    Boolean KeyOK = TRUE;
    time_t expiration;
    char datestring[11];

    if ((expiration = ringKeyExpiration (ringset, key)) > 0 &&
	expiration <= time ((time_t *) 0)) {
	if(!InBatchMode) {
	    pgpDateString (expiration, datestring);
	    InteractionOutput(TRUE,
			      "STILL_USE_EXPIRED_KEY",
			      datestring);
	    KeyOK = pgpTtyGetBool (0, TRUE, NULL);
	}
	else {
	    WarningOutput(TRUE,
			  LEVEL_SEVERE,
			  "ABOVE_KEY_EXPIRED",
			  datestring);
	    KeyOK = FALSE;
	}
    }

    return(KeyOK);
}

static Boolean keyValidityOK(struct RingSet *ringset,
			     struct RingIterator *iter,
			     union RingObject *key,
			     char *recipient,
			     Boolean InBatchMode,
			     Boolean FailOnInvalidKeys)
{
					    
    const char *MatchedName;
    const char *NameString;
    Boolean MatchedKeyID;
    struct RingIterator *NameIter = NULL;
    union RingObject *name;
    Boolean OKToUse = FALSE, FoundName = FALSE;
    int len = 0;
    int trust;
    char *TempNameString = NULL;

    /*What did we match on, exactly?*/
    FilterSingleObj(iter, 
		    key, 
		    recipient, 
		    PGP_PKUSE_ENCRYPT,
		    &MatchedName, 
		    &MatchedKeyID);
    
    NameIter = ringIterCreate(ringset);
    /*Find the key:*/
    ringIterSeekTo(NameIter, key);
    
    /*Loop through the names until we find the one we matched on:*/
    while(ringIterNextObject(NameIter, 2) > 0 && !FoundName)
    {
	name = ringIterCurrentObject(NameIter, 2);
	if(ringObjectType(name) == RINGTYPE_NAME) {
	    NameString = ringNameName(ringset, name, &len);
	    if(MatchedKeyID || strcmp(NameString, MatchedName) == 0) {
		/*It's our boy.  How do we trust this fella?*/
		OKToUse = TRUE;
		FoundName = TRUE;

		trust = ringNameTrust(ringset, name);
		switch(trust) {
		    case PGP_NAMETRUST_UNKNOWN:
			WarningOutput(TRUE,
				      LEVEL_INFORMATION,
				      "PGP_NAMETRUST_UNKNOWN");
			OKToUse = FALSE;
			break;
		    case PGP_NAMETRUST_UNTRUSTED:
			WarningOutput(TRUE,
				      LEVEL_INFORMATION,
				      "PGP_NAMETRUST_UNTRUSTED");
			OKToUse = FALSE;
			break;
		    case PGP_NAMETRUST_MARGINAL:
			WarningOutput(TRUE,
				      LEVEL_INFORMATION,
				      "PGP_NAMETRUST_MARGINAL");
			OKToUse = FALSE;
			break;
		}
		
		if(!OKToUse) {
		    if((TempNameString = pgpMemAlloc(sizeof(char) * 
						     (len + 1)))) {
			memcpy(TempNameString, NameString, len);
			*(TempNameString + len) = '\0';
			WarningOutputString(TRUE,
					    LEVEL_INFORMATION,
					    TempNameString);
			WarningOutputString(TRUE, LEVEL_INFORMATION, "\n");
			pgpFree(TempNameString);
		    }
		    if(ringNameWarnonly(ringset, name)) {
			WarningOutput(TRUE,
				      LEVEL_INFORMATION,
				      "PREVIOUSLY_APPROVED_KEY");
			OKToUse = TRUE;
		    }
		    else {
			if(!InBatchMode) {
			    InteractionOutput(TRUE,
					  "DO_YOU_WISH_TO_USE_UNTRUSTED_KEY");
			    if (pgpTtyGetBool (0, TRUE, NULL)) {
				ringNameSetWarnonly (ringset,
						     name);
				OKToUse = TRUE;
			    }
			}
			else { /*We're in batch mode*/
			    OKToUse = !FailOnInvalidKeys;
			}
		    }
		}
	    }
	}
    }

    if(NameIter)
    {
	ringIterDestroy(NameIter);
    }

    return(OKToUse);
}

int AddMRKs(union RingObject *key,
	    struct RingSet const *src,
	    struct RingSet *dest)
{
    int NumMRKs, keys, error, NumMatches = 0;
    byte class;
    union RingObject *MRKObj;
#if 0
    char keyid[64];
#endif

    NumMRKs = 0;

    do {
	MRKObj = ringKeyRecoveryKey(key, 
				    (struct RingSet *) src,
				    NumMRKs, 
				    &class, 
				    &keys, 
				    &error);
	if(error == PGPERR_NO_RECOVERYKEY) {
	    WarningOutput(TRUE,
			  LEVEL_INFORMATION,
			  "WARNING_NO_MRK");
	}
	else {
	    if(MRKObj) {
		ringSetAddObjectChildren(dest, src, key);
		++NumMatches;
#if 0
		ringTtyKeyIDSprintf(keyid,
				    src,
				    MRKObj);
		InformationOutput(TRUE,
				  "MRK_FOUND",
				  keyid);

		NumMatches += ringSetFilterSpec(src,
						dest,
						keyid,
						PGP_PKUSE_ENCRYPT);
#endif
	    }
	    else {
		keys = 0;
	    }
	}
	++NumMRKs;
    }while(NumMRKs < keys);

    return(NumMatches);
}
