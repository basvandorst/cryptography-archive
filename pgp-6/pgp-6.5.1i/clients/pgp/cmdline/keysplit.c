/*	keysplit.c - Implements support for splitting/rejoining keys
	base off of code found in PGPKeys. */					 

#include <stdio.h>
#include <io.h>
#include <string.h>

#include "pgpBase.h"
#include "pgpKeys.h"
#include "pgpErrors.h"
#include "pgpContext.h"
#include "pgpEnv.h"
#include "pgpPubTypes.h"
#include "pgpFileSpec.h"

#include "pgpShare.h"
#include "pgpShareFile.h"

#include "globals.h"
#include "prototypes.h"
#include "language.h"
#include "fileio.h"

/* should probably move this to a header file later but...*/
typedef struct _SHAREHOLDERSTRUCT {
	PGPBoolean					bPublicKey;
	PGPKeyID					keyid;
	PGPPublicKeyAlgorithm		keyalg;
	char						szUserID[kPGPMaxUserIDSize];
	char						*pszPassphrase;
	PGPUInt16					uShares;
} SHAREHOLDERSTRUCT, *PSHAREHOLDERSTRUCT;

/* taken from KMShare.c */
#define MAX_SHARES						99
#define MAX_SHARES_LEN					2

PGPError sCreateFilePathFromUserName (char		*pszFolder,
									  char		*pszUserID,
									  PGPUInt16	uNumShares,
									  char		*pszModifier,
									  char		*pszPath, 
									  PGPInt16	iLen);


PGPError sSaveSharesToFile (PSHAREHOLDERSTRUCT	pshs, 
							PGPContextRef		context,
							PGPShareRef			sharesTotal,
							PGPKeySetRef		keyset,
							char				*pszFolder);

PGPError sChangeKeyPhrase (PGPContextRef	context,
						   PGPKeySetRef		keyset,
						   PGPKeyRef		key, 
						   char				*szOld, 
						   PGPByte*			pPasskeyOld,
						   PGPSize			sizePasskeyOld,
						   PGPByte*			pPasskey,
						   PGPSize			sizePasskey);

PGPError pgpParseShares(struct pgpmainBones *mainbPtr,
						SHAREHOLDERSTRUCT	**pshs, 
						char				**ppszUsers,
						PGPUInt16			myArgc,
						PGPUInt32			*dwThreshold,
						PGPUInt32			*dwTotalShares,
						PGPUInt32			*dwTotal);


PGPError SplitKey (struct pgpmainBones *mainbPtr, char *pszKeyToSplit, char **ppszUsers, PGPUInt16 myArgc) 
{
    PGPContextRef			context = mainbPtr->pgpContext;
	struct pgpargsBones		*argsbPtr = mainbPtr->argsbPtr;
    struct pgpfileBones		*filebPtr = mainbPtr->filebPtr;
    struct pgpenvBones		*envbPtr = mainbPtr->envbPtr;
	PGPKeySetRef			workingset = kPGPInvalidRef;
	PGPKeySetRef			keyset = kPGPInvalidRef;
	PGPKeyID				keyidToSplit;
	PGPPublicKeyAlgorithm	keyalgToSplit;
    PGPKeyListRef			keylist = kPGPInvalidRef;
    PGPKeyIterRef			keyiter = kPGPInvalidRef;
    PGPKeyRef				keyToSplit = kPGPInvalidRef;
	PGPError				err = kPGPError_NoErr;
	char					*pszPassphrase = NULL;
	PGPShareRef				shares = kPGPInvalidRef;
	PGPUInt32				dwThreshold = 0, dwTotalShares = 0;
	PGPBoolean				bNeedsFree = FALSE;

	PGPByte*				pPasskey = NULL; 
	PGPSize					sizePasskey	= 0;
	char					*pszPhraseKeyToSplit	= NULL;
	PGPByte*				pPasskeyToSplit	= NULL;	 
	PGPSize					sizePasskeyToSplit	= 0;
	PGPUInt32				numShares = 0;
	PGPUInt16				i = 0;
											   
	/* should probably modify this structure */
	SHAREHOLDERSTRUCT		*pshs = NULL;

	err = PGPOpenDefaultKeyRings(context, kPGPKeyRingOpenFlags_Mutable, &workingset);
	if(IsPGPError(err))
		return err;

	mainbPtr->workingRingSet = workingset;

	/* setup share structure */
	err = pgpParseShares(mainbPtr, &pshs, ppszUsers, myArgc, &dwThreshold, &dwTotalShares, &numShares);
	if(IsPGPError(err))
	{
		fprintf(filebPtr->pgpout,
			LANG("Error: parameter error!\n"));
		goto done;
	}

	err = pgpGetMatchingKeySet( mainbPtr, pszKeyToSplit, 0, &keyset);
	if(IsPGPError(err))
		goto done;

    err = PGPOrderKeySet( keyset, kPGPUserIDOrdering, &keylist );
    pgpAssertNoErr(err);
    err = PGPNewKeyIter( keylist, &keyiter );
    pgpAssertNoErr(err);
    err = PGPKeyIterRewind( keyiter );
    pgpAssertNoErr(err);

    err = PGPKeyIterNext( keyiter, &keyToSplit);
    /*if error, no keys found.*/
	if(IsntPGPError(err) && keyToSplit != kPGPInvalidRef)
	{
		PGPBoolean		bIsSecret = FALSE;

		/* check to see if key is secret key */
		err = PGPGetKeyBoolean(keyToSplit, kPGPKeyPropIsSecret, &bIsSecret);
		pgpAssertNoErr(err);

		if(!bIsSecret)
		{
			fprintf(filebPtr->pgpout,
				LANG("Error, cannot split public key!\n"));
			err = kPGPError_BadParams;
			goto done; 
		}

		/* check to see if key has already been split */
		err = PGPGetKeyBoolean(keyToSplit, kPGPKeyPropIsSecretShared, &bIsSecret);
		pgpAssertNoErr(err);

		if(bIsSecret)
		{
			fprintf(filebPtr->pgpout,
				LANG("Error, key has already been split!\n"));
			err = kPGPError_BadParams;
			goto done;
		}

		err = pgpGetValidPassphrase( mainbPtr, keyToSplit, &pszPassphrase, &bNeedsFree );
		if(IsPGPError(err))
		{
			err = kPGPError_UserAbort;
			goto done;
		}

		/* get keyid and alg of key to split */
		PGPGetKeyIDFromKey(keyToSplit, &keyidToSplit);
		PGPGetKeyNumber(keyToSplit, kPGPKeyPropAlgID, &keyalgToSplit);

		/* need to allow user to specify the number of shares required 
			to decrypt/sign with, default to total number of shares */
	
		// create the shares
		err = PGPCreateShares (context, keyToSplit, 
								dwThreshold, dwTotalShares, &shares);

		if(IsPGPError(err))
		{
			fprintf(filebPtr->pgpout,
				LANG("Error splitting key!\n"));
			goto done;
		}
		
		/* get the passkey from the shares */
		err = PGPGetPasskeyFromShares (shares, &pPasskey, &sizePasskey);
		if(IsPGPError(err))
		{
			fprintf(filebPtr->pgpout,
				LANG("Error splitting key!\n"));
			goto done;
		}


		for(i = 0; i < numShares; i++)
		{
			if(!pshs[i].bPublicKey)
			{
				err = pgpPassphraseDialogCmdline(mainbPtr, TRUE, 
							"Enter passphrase: ", &pshs[i].pszPassphrase);
			}
			err = sSaveSharesToFile (&pshs[i], context, shares,	workingset,
				argsbPtr->outputFileName ? argsbPtr->outputFileName : ".");

			if(IsPGPError(err))
			{
				fprintf(filebPtr->pgpout,
					LANG("Error splitting key!\n"));
				goto done;
			}
		}

		err = sChangeKeyPhrase (context, workingset, keyToSplit,
			pszPassphrase[0] == '\0' ? NULL : pszPassphrase,
			pPasskeyToSplit, sizePasskeyToSplit,
			pPasskey, sizePasskey);
	
		if(IsntPGPError(err) && PGPKeySetNeedsCommit(workingset))
		{
			PGPCommitKeyRingChanges(workingset);	
		}
	}
	else
	{
		fprintf(filebPtr->pgpout,
			LANG("Unable to find specified key to split!\n"));
		err = kPGPError_BadParams;
	}


done:
	if(pshs != NULL)
		free(pshs);
	if(shares != kPGPInvalidRef)
		PGPFreeShares(shares);
    if(keyiter != kPGPInvalidRef)
        PGPFreeKeyIter(keyiter);
    if(keylist != kPGPInvalidRef)
        PGPFreeKeyList(keylist);
	if(keyset != kPGPInvalidRef)
		PGPFreeKeySet(keyset);
	if(workingset != kPGPInvalidRef)
	{
		PGPFreeKeySet(workingset);
		mainbPtr->workingRingSet = kPGPInvalidRef;
	}

	return err;
}


/*	Code taken from PGPKeys
 *	_______________________________________________
 *
 *  split the key 
 */

static PGPError 
sSaveSharesToFile (
		PSHAREHOLDERSTRUCT	pshs, 
		PGPContextRef		context,
		PGPShareRef			sharesTotal,
		PGPKeySetRef		keyset,
		char				*pszFolder)
{
	PFLFileSpecRef		filespec		= NULL;
	PGPShareFileRef		sharefile		= NULL;
	PGPShareRef			sharesHolder	= NULL;
	PGPOptionListRef	encodeOptions	= NULL;
	PGPError			err				= kPGPError_NoErr;
	int					iModifier		= 0;

	char				szPath[MAX_PATH];
	char				szModifier[MAX_SHARES_LEN+1];
	char				sz1[32];
	char				sz2[kPGPMaxUserIDSize + 32];
	PGPKeyRef			key;

	// create file name and filespec
	err = sCreateFilePathFromUserName (pszFolder, pshs->szUserID, 
						pshs->uShares, "", szPath, sizeof(szPath));
	if (IsPGPError (err)) goto SaveFileCleanup;

	// check for pre-existence of file
	while (!_access (szPath, 0)) {
		iModifier++;
		if (iModifier > MAX_SHARES) {
			err = kPGPError_CantOpenFile;
			goto SaveFileCleanup;
		}
		sprintf (szModifier, " %i", iModifier);
		err = sCreateFilePathFromUserName (pszFolder, pshs->szUserID,
						pshs->uShares, szModifier, szPath, sizeof(szPath));
		if (IsPGPError (err)) goto SaveFileCleanup;
	}	

	err = PFLNewFileSpecFromFullPath (PGPGetContextMemoryMgr (context), 
		szPath, &filespec);
	if (IsPGPError (err)) goto SaveFileCleanup;
	
	err = PFLFileSpecCreate (filespec);
	if (IsPGPError (err)) goto SaveFileCleanup;

	err = PGPNewShareFile (filespec, &sharefile);
	if (IsPGPError (err)) goto SaveFileCleanup;

	err = PGPSetShareFileUserID (sharefile, pshs->szUserID);
	if (IsPGPError (err)) goto SaveFileCleanup;

	err = PGPSplitShares (sharesTotal, pshs->uShares, &sharesHolder);
	if (IsPGPError (err)) goto SaveFileCleanup;

	// if this shareholder has public key, use it
	if (pshs->bPublicKey) {
		err = PGPSetShareFileOwnerKeyID (sharefile, pshs->keyid);
		if (IsPGPError (err)) goto SaveFileCleanup;

		err = PGPGetKeyByKeyID (keyset, &(pshs->keyid), pshs->keyalg, &key);
		if(IsPGPError(err)) goto SaveFileCleanup;

		err = PGPBuildOptionList (context, &encodeOptions,
			PGPOEncryptToKey (context, key),
			PGPOLastOption (context));
		if (IsPGPError (err)) goto SaveFileCleanup;
	}

	// there is no public key for this shareholder
	else {
		err = PGPBuildOptionList (context, &encodeOptions,
			PGPOConventionalEncrypt (context,
				PGPOPassphrase (context, pshs->pszPassphrase),
				PGPOLastOption (context)),
			PGPOLastOption (context));
		if (IsPGPError (err)) goto SaveFileCleanup;
	}

	err = PGPCopySharesToFile (context, sharefile, 
									encodeOptions, sharesHolder);
	if (IsPGPError (err)) goto SaveFileCleanup;

	err = PGPSaveShareFile (sharefile);

SaveFileCleanup:

	if (encodeOptions != NULL)
		PGPFreeOptionList (encodeOptions);

	if (sharesHolder != NULL)
		PGPFreeShares (sharesHolder);

	if (sharefile != NULL)
		PGPFreeShareFile (sharefile);

	if (filespec != NULL)
		PFLFreeFileSpec (filespec);

	return err;
}


/* Code taken from PGPKeys
 *	___________________________________________________
 *
 * Change passphrase of key and all subkeys
 */

static PGPError 
sChangeKeyPhrase (
		PGPContextRef	context,
		PGPKeySetRef	keyset,
		PGPKeyRef		key, 
		char			*szOld, 
		PGPByte*		pPasskeyOld,
		PGPSize			sizePasskeyOld,
		PGPByte*		pPasskey,
		PGPSize			sizePasskey) 
{
	PGPUInt32		u;
	PGPKeyListRef	keylist;
	PGPKeyIterRef	keyiter;
	PGPSubKeyRef	subkey;
	PGPError		err;

	if (szOld) {
		err = PGPChangePassphrase (key, 
				PGPOPassphrase (context, szOld), 
				PGPOPasskeyBuffer (context, pPasskey, sizePasskey),
				PGPOLastOption (context));
	}
	else if (sizePasskeyOld > 0) {
		err = PGPChangePassphrase (key, 
				PGPOPasskeyBuffer (context, pPasskeyOld, sizePasskeyOld), 
				PGPOPasskeyBuffer (context, pPasskey, sizePasskey),
				PGPOLastOption (context));
	}
	else {
		err = PGPChangePassphrase (key, 
				PGPOPassphrase (context, ""), 
				PGPOPasskeyBuffer (context, pPasskey, sizePasskey),
				PGPOLastOption (context));
	}
	if (IsPGPError (err)) return err;

	PGPGetKeyNumber (key, kPGPKeyPropAlgID, &u);
	switch (u) {
	case kPGPPublicKeyAlgorithm_RSA :
		break;

	case kPGPPublicKeyAlgorithm_DSA :
		PGPOrderKeySet (keyset, kPGPAnyOrdering, &keylist);
		PGPNewKeyIter (keylist, &keyiter);
		PGPKeyIterSeek (keyiter, key);
		PGPKeyIterNextSubKey (keyiter, &subkey);
		while (subkey) {
			if (szOld) {
				err = PGPChangeSubKeyPassphrase (subkey, 
						PGPOPassphrase (context, szOld),
						PGPOPasskeyBuffer (context, pPasskey, sizePasskey),
						PGPOLastOption (context));
			}
			else if (sizePasskeyOld > 0) {
				err = PGPChangeSubKeyPassphrase (subkey, 
						PGPOPasskeyBuffer (context, 
											pPasskeyOld, sizePasskeyOld),
						PGPOPasskeyBuffer (context, pPasskey, sizePasskey),
						PGPOLastOption (context));
			}
			else {
				err = PGPChangeSubKeyPassphrase (subkey, 
						PGPOPassphrase (context, ""),
						PGPOPasskeyBuffer (context, pPasskey, sizePasskey),
						PGPOLastOption (context));
			}
			PGPKeyIterNextSubKey (keyiter, &subkey);
		}
		PGPFreeKeyIter (keyiter);
		PGPFreeKeyList (keylist);
		break;

	default :
		break;
	}

	return err;
}


static PGPError
sCreateFilePathFromUserName (
		char		*pszFolder,
		char		*pszUserID,
		PGPUInt16	uNumShares,
		char		*pszModifier,
		char		*pszPath, 
		PGPInt16	iLen) 
{
	char		sz[kPGPMaxUserIDSize];
	char		szDefName[16];
	char		szDefExt[8];
	char		szShares[16];
	PGPInt16	iMinRequiredLen;
	PGPInt16	i;

	/* prepare number of shares substring */
	if (uNumShares == 1) {
		sprintf(szShares, "1 Share");
	}
	else {
		sprintf(szShares, "%i Shares", uNumShares);
	}

	/* get default file name and extension */
	sprintf(szDefName, "User");
	strcat (szDefName, pszModifier);
	sprintf(szDefExt, "shf");

	/* check length of destination buffer */
	iMinRequiredLen = 
		strlen (pszFolder) + strlen (szDefExt) + strlen (szDefName) +1;
	if (iMinRequiredLen >= iLen) 
		return kPGPError_CantOpenFile;

	/* put folder into path */
	/* need to make sure to support path specifiers for all platforms */
	strcpy (pszPath, pszFolder);
	iLen -= strlen (pszPath);
	if (pszPath[strlen(pszPath)-1] != '\\') {
		strcat (pszPath, "\\");
		iLen -= 1;
	}

	// look for invalid characters and truncate
	strcpy (sz, pszUserID);
	i = strcspn (sz, "\\/:*?""<>|");
	sz[i] = '\0';

	// remove trailing spaces
	while ((i > 0) && (sz[i-1] == ' ')) {
		i--;
		sz[i] = '\0';
	}

	// check if we've truncated too much
	if (strlen (sz) < 2) 
		strcpy (sz, szDefName);

	// check if name is too long
	iLen -= (strlen (szDefExt) +1);
	if ((strlen(sz) + strlen(szShares) + strlen(pszModifier)) >= iLen) {
		if ((strlen (sz) + strlen (pszModifier)) >= iLen) {
			if (strlen (sz) >= iLen) {
				sz[iLen-1] = '\0';
			}
			strcat (pszPath, sz);
			strcat (pszPath, ".");
			strcat (pszPath, szDefExt);

		}
		else {
			strcat (pszPath, sz);
			strcat (pszPath, pszModifier);
			strcat (pszPath, ".");
			strcat (pszPath, szDefExt);
		}
	}
	else {
		// construct full path
		strcat (pszPath, sz);
		strcat (pszPath, szShares);
		strcat (pszPath, pszModifier);
		strcat (pszPath, ".");
		strcat (pszPath, szDefExt);
	}

	return kPGPError_NoErr;
}

/*	End of section taken from PGPKeys
 *	____________________________________________________
 *
 */


/* this struct is used only in pgpParseShares */
typedef struct _PARSESTRUCT {
	char			szUserID[kPGPMaxUserIDSize];
	PGPUInt16		uShares;
} PARSESTRUCT;

						 
PGPError pgpParseShares(struct pgpmainBones *mainbPtr,
						SHAREHOLDERSTRUCT	**pshs, 
						char				**ppszUsers,
						PGPUInt16			myArgc,
						PGPUInt32			*pdwThreshold,
						PGPUInt32			*pdwTotalShares,
						PGPUInt32			*dwTotal)
{
	struct pgpfileBones		*filebPtr = mainbPtr->filebPtr;
	PGPUInt16				i = 0;
	PGPError				err = kPGPError_NoErr;
    PGPKeyListRef			keylist = kPGPInvalidRef;
    PGPKeyIterRef			keyiter = kPGPInvalidRef;
    PGPKeyRef				key = kPGPInvalidRef;
	PGPKeySetRef			keyset = kPGPInvalidRef;
	SHAREHOLDERSTRUCT		*pTemp = NULL;
	PGPUInt32				num = 0;
	PARSESTRUCT				*pParse = malloc(sizeof(PARSESTRUCT) * myArgc);

	if(!pParse)
		return kPGPError_OutOfMemory;

	for(i = 3; i < myArgc; i++)
	{
		if(ppszUsers[i][0] == '-')
		{
			if(ppszUsers[i][1] == 'n')
			{
				if((i + 1) > (myArgc - 1))
				{
					err = kPGPError_BadParams;
					goto done;
				}
				else
				{
					char		*numShares = &ppszUsers[i][2];
					pParse[num].uShares = atoi(numShares);
					/* check to see if valid value, if not, default to 1 share */
					if(!pParse[num].uShares)
						pParse[num].uShares = 1;
					i++;
					*pdwTotalShares += pParse[num].uShares;
					strcpy(pParse[num].szUserID, ppszUsers[i]);
				}
				num++;
			}
			else if(ppszUsers[i][1] == 'r')
			{
				if((i + 1) > (myArgc - 1))
				{
					err = kPGPError_BadParams;
					goto done;
				}
				else
				{
					char		*pszThreshold = &ppszUsers[++i][0];
					*pdwThreshold = atoi(pszThreshold);
				}
			}
			else
			{ 
				/* should get here if everything is working correctly */
				err = kPGPError_BadParams;
				goto done;
			}
		}
		else
		{
			strcpy(pParse[num].szUserID, ppszUsers[i]);
			pParse[num].uShares = 1;
			(*pdwTotalShares)++;
			num++;
		}
	}

	*dwTotal = num;

	/* make sure threshold value is valid */
	if(*pdwThreshold > *pdwTotalShares)
	{
		fprintf(filebPtr->pgpout,
			LANG("Error, threshold greater than total number of shares.\n"));
		err = kPGPError_BadParams;
		goto done;
	}
	else if(!(*pdwThreshold))
	{
		*pdwThreshold = *pdwTotalShares;
	}

	*pshs = calloc(num, sizeof(SHAREHOLDERSTRUCT));
	if(*pshs == NULL)
		return kPGPError_OutOfMemory;

	pTemp = *pshs;

	for(i = 0; i < num; i++)
	{
		//err = pgpGetMatchingKeySet( mainbPtr, ppszUsers[3 + i], 0, &keyset);
		err = pgpGetMatchingKeySet( mainbPtr, pParse[i].szUserID, 0, &keyset);
		if(IsPGPError(err))
			goto done;

		err = PGPOrderKeySet( keyset, kPGPUserIDOrdering, &keylist );
		pgpAssertNoErr(err);
		err = PGPNewKeyIter( keylist, &keyiter );
		pgpAssertNoErr(err);
		err = PGPKeyIterRewind( keyiter );
		pgpAssertNoErr(err);

		err = PGPKeyIterNext( keyiter, &key);

		if(IsntPGPError(err) && key != kPGPInvalidRef)
		{
			PGPSize		length = 0;
			pTemp[i].bPublicKey = TRUE;
			
			err = PGPGetKeyIDFromKey(key, &pTemp[i].keyid);
			if(IsPGPError(err))
				goto done;

			err = PGPGetKeyNumber(key, kPGPKeyPropAlgID, &pTemp[i].keyalg);
			if(IsPGPError(err))
				goto done;

			//strcpy(pTemp[i].szUserID, ppszUsers[3 + i]);
			strcpy(pTemp[i].szUserID, pParse[i].szUserID);

#if 0
			err = PGPGetPrimaryUserIDNameBuffer(key, sizeof(pTemp[i].szUserID),
												pTemp[i].szUserID, &length);
			if(IsPGPError(err))
				goto done;
#endif 0

			
		}
		else if(err == kPGPError_EndOfIteration)
		{
			err = kPGPError_NoErr;
			/* conventionally encrypt this bad boy */
			pTemp[i].bPublicKey = FALSE;
			//strcpy(pTemp[i].szUserID, ppszUsers[3 + i]);
			strcpy(pTemp[i].szUserID, pParse[i].szUserID);
		}
		else
		{
			/* shouldn't get here */
			goto done;
		}

		pTemp[i].uShares = pParse[i].uShares;

		if(keyiter != kPGPInvalidRef)
		{
			PGPFreeKeyIter(keyiter);
			keyiter = kPGPInvalidRef;
		}
		if(keylist != kPGPInvalidRef)
		{
			PGPFreeKeyList(keylist);
			keylist = kPGPInvalidRef;
		}
		if(keyset != kPGPInvalidRef)
		{
			PGPFreeKeySet(keyset);
			keyset = kPGPInvalidRef;
		}
	}

done:
	if(pParse)
		free(pParse);
    if(keyiter != kPGPInvalidRef)
        PGPFreeKeyIter(keyiter);
    if(keylist != kPGPInvalidRef)
        PGPFreeKeyList(keylist);
	if(keyset != kPGPInvalidRef)
		PGPFreeKeySet(keyset);

	return err;
}


/* code to rejoin split keys */
typedef struct {
	PGPContextRef		context;
	struct pgpmainBones	*mainbPtr;
	/*PGPKeySetRef		keyset;*/
	PGPKeyRef			keyToJoin;
	PGPKeyID			keyidToJoin;
	PGPKeyRef			keyAuthenticating;
	PGPKeySetRef		keysetDecryption;
	PGPUInt32			iKeyIDCount;
	PGPKeyID*			keyidsDecryption;
	PGPKeySetRef		keysetToAdd;
	char				szAuthUserID[kPGPMaxUserIDSize+1];
	char				*pszPhraseAuth;
	PGPByte*			pPasskeyAuth;
	PGPSize				sizePasskeyAuth;
	PGPUInt16			uNeededShares;
	PGPUInt16			uCollectedShares;
	PGPShareRef			sharesCombined;
	/*PGPskepRef			skep;			/* used for key exchange via network I think */
	PGPUInt16			iIconIndex;
	PGPBoolean			bServerMode;
	PGPBoolean			bStop;
	PGPBoolean			bUserCancel;
	PGPBoolean			bUserOK;
	PGPBoolean			bBadPassphrase;
} RECONKEYSTRUCT, *PRECONKEYSTRUCT;

#define UNKNOWN_SHARES_NEEDED			999999

PGPError sHandlerDecode(PGPContextRef	context,
						PGPEvent*		event,
						PGPUserValue	userValue);




PGPError JoinSplitKey(struct pgpmainBones	*mainbPtr,
					  char					*pszKeyToSplit,
					  char					**ppszUsers,
					  PGPUInt16				myArgc)
{
    PGPContextRef			context = mainbPtr->pgpContext;
	struct pgpargsBones		*argsbPtr = mainbPtr->argsbPtr;
    struct pgpfileBones		*filebPtr = mainbPtr->filebPtr;
    struct pgpenvBones		*envbPtr = mainbPtr->envbPtr;
	PGPKeySetRef			workingset = kPGPInvalidRef;
	PGPKeySetRef			keyset = kPGPInvalidRef;
    PGPKeyListRef			keylist = kPGPInvalidRef;
    PGPKeyIterRef			keyiter = kPGPInvalidRef;
    /*PGPKeyRef				keyToJoin = kPGPInvalidRef;*/


	PGPShareRef				shares = kPGPInvalidRef;
	PGPUInt32				dwThreshold = 0, dwTotalShares = 0;
	PGPError				err = kPGPError_NoErr;

	PGPByte*				pPasskey = NULL;
	PGPUInt32				iPasskeyLength;
	char					*pszNew = NULL;

	RECONKEYSTRUCT			recon;
	memset(&recon, 0, sizeof(RECONKEYSTRUCT));

	recon.mainbPtr = mainbPtr;

	/* open default key rings */
	err = PGPOpenDefaultKeyRings(context, kPGPKeyRingOpenFlags_Mutable, &workingset);
	if(IsPGPError(err))
		return err;
	mainbPtr->workingRingSet = workingset;

	err = pgpGetMatchingKeySet( mainbPtr, pszKeyToSplit, 0, &keyset);
	if(IsPGPError(err))
		goto done;

    err = PGPOrderKeySet( keyset, kPGPUserIDOrdering, &keylist );
    pgpAssertNoErr(err);
    err = PGPNewKeyIter( keylist, &keyiter );
    pgpAssertNoErr(err);
    err = PGPKeyIterRewind( keyiter );
    pgpAssertNoErr(err);

    err = PGPKeyIterNext( keyiter, &recon.keyToJoin);
	if(recon.keyToJoin != kPGPInvalidRef)
	{
		PGPBoolean			bIsSplit = FALSE;
		PGPBoolean			bEnough = FALSE;
		PGPUInt16			i = 3;

		/* make sure that key is split */
		err = PGPGetKeyBoolean (recon.keyToJoin, kPGPKeyPropIsSecretShared, &bIsSplit);
		pgpAssertNoErr(err);
		if(!bIsSplit)
		{
			fprintf(filebPtr->pgpout,
				LANG("Error: Specified key is not split!\n"));
			err = kPGPError_BadParams;
			goto done;
		}

		/* gather shares to rejoin key */
		while(!bEnough)
		{
			err = AddShareFile(mainbPtr, &recon, ppszUsers[i++], &bEnough);
			if(IsPGPError(err))
			{
			}
			if(!bEnough && i > myArgc)
			{
				/* not enough shares to rejoin key */
				fprintf(filebPtr->pgpout,
					LANG("Error: Not enough shares to rejoin key!\n"));
				err = kPGPError_BadParams;
				goto done;
			}
		}

		err = PGPGetPasskeyFromShares (recon.sharesCombined, 
						&pPasskey, &iPasskeyLength);
		if(IsPGPError(err))
		{
			fprintf(filebPtr->pgpout,
				LANG("Error extracting passkey from shares!\n"));
			goto done;
		}

		if (!PGPPassphraseIsValid(recon.keyToJoin, 
					PGPOPasskeyBuffer (context, pPasskey, iPasskeyLength),
					PGPOLastOption (context)))
		{
			fprintf(filebPtr->pgpout,
				LANG("Error: Bad passphrase!\n"));
			err = kPGPError_BadPassphrase;
			goto done;
		}

		/* need to change passphrase of key to something here */ 
		err = pgpPassphraseDialogCmdline(mainbPtr, TRUE, 
					"Enter new passphrase: ", &pszNew);
		if(IsPGPError(err))
		{
			fprintf(filebPtr->pgpout,
				LANG("Error getting new passphrase for key!\n"));
			err = kPGPError_BadPassphrase;
			goto done;
		}


		/* get new passphrase for rejoined key here and put it in szNew */
		err = PGPChangePassphrase (recon.keyToJoin, 
				PGPOPasskeyBuffer (context, pPasskey, iPasskeyLength),
				PGPOPassphrase (context, pszNew), 
				PGPOLastOption (context));



	}
	else
	{
		fprintf(filebPtr->pgpout,
			LANG("Error: Key not found!\n"));
		err = kPGPError_BadParams;
	}

done:
    if(keyiter != kPGPInvalidRef)
        PGPFreeKeyIter(keyiter);
    if(keylist != kPGPInvalidRef)
        PGPFreeKeyList(keylist);
	if(keyset != kPGPInvalidRef)
		PGPFreeKeySet(keyset);
	if(workingset != kPGPInvalidRef)
	{
		PGPFreeKeySet(workingset);
		mainbPtr->workingRingSet = kPGPInvalidRef;
	}
	return err;
}

/* taken from CLrecon.c */
PGPError AddShareFile(struct pgpmainBones	*mainbPtr,
					  RECONKEYSTRUCT		*pRecon,
					  char					*pszShareFile,
					  PGPBoolean			*pbIsEnough)
{
    PGPContextRef			context = mainbPtr->pgpContext;
    struct pgpfileBones		*filebPtr = mainbPtr->filebPtr;

	PGPError				err	= kPGPError_NoErr;
	PFLFileSpecRef			fileref	= NULL;
	PGPShareFileRef			sharefileref = NULL;
	PGPOptionListRef		optionsDecode = NULL;
	PGPShareRef				shares = NULL;
	PGPShareRef				sharesTemp = NULL;


	char					*p;
	char					szName[kPGPMaxUserIDSize+1];
	char					sz[256];
	PGPUInt32				size;
	PGPUInt32				uNumShares;
	PGPUInt32				uThreshold;
	PGPKeyID				keyid;

	/* initialize */
	pRecon->keysetToAdd			= kInvalidPGPKeySetRef;
	pRecon->keysetDecryption	= kInvalidPGPKeySetRef;
	pRecon->iKeyIDCount			= 0;
	pRecon->keyidsDecryption	= NULL;

	err = PFLNewFileSpecFromFullPath (context, pszShareFile, &fileref);
	if (IsPGPError (err)) goto AddCleanup;

	err = PGPOpenShareFile (fileref, &sharefileref);
	if (IsPGPError (err)) goto AddCleanup;

	err = PGPGetShareFileSharedKeyID (sharefileref, &keyid);
	if (IsPGPError (err)) goto AddCleanup;

	if (PGPCompareKeyIDs (&keyid, &(pRecon->keyidToJoin)))
	{
		goto AddCleanup;
	}

	// check that threshold corresponds to other share files
	uThreshold = PGPGetShareThresholdInFile (sharefileref);
	if (pRecon->uNeededShares != UNKNOWN_SHARES_NEEDED) 
	{
		if (uThreshold != pRecon->uNeededShares)
		{
			fprintf(filebPtr->pgpout,
				LANG("Error: Threshold found in shares differs!\n"));
			goto AddCleanup;
		}
	}

	err = PGPGetShareFileUserID (sharefileref,
								sizeof(szName), szName, &size);
	if (IsPGPError (err)) goto AddCleanup;

	uNumShares = PGPGetNumSharesInFile (sharefileref);

	// decrypt specified share file
	pRecon->bBadPassphrase = FALSE;
	PGPBuildOptionList (context, &optionsDecode,
				PGPOKeySetRef (context, mainbPtr->workingRingSet),
				PGPOEventHandler (context, sHandlerDecode, pRecon),
				PGPOLastOption (context));
	err = PGPCopySharesFromFile (context, sharefileref, 
					optionsDecode, &shares);
	if (IsPGPError (err)) goto AddCleanup;

	// add shares to collection
	if (pRecon->sharesCombined) {
		err = PGPCombineShares (shares, 
								pRecon->sharesCombined, &sharesTemp);
		if (IsPGPError (err)) 
			goto AddCleanup;
		PGPFreeShares (pRecon->sharesCombined);
		pRecon->sharesCombined = sharesTemp;
	}
	else {
		pRecon->sharesCombined = shares;
		shares = NULL;
	}

	// share is OK, add it to list
	pRecon->uNeededShares = uThreshold;
	pRecon->uCollectedShares += uNumShares;

	if(pRecon->uCollectedShares >= pRecon->uNeededShares)
		pbIsEnough = TRUE;

AddCleanup :

	if (shares) 
		PGPFreeShares (shares);

	if (sharefileref)
		PGPFreeShareFile (sharefileref);

	if (fileref)
		PFLFreeFileSpec (fileref);

	if (optionsDecode)
		PGPFreeOptionList(optionsDecode);

	if (PGPKeySetRefIsValid (pRecon->keysetDecryption))
		PGPFreeKeySet (pRecon->keysetDecryption);

	if (pRecon->keyidsDecryption)
		free(pRecon->keyidsDecryption);

	switch (err) 
	{
		/* should probably print some descriptive info here for each case */
		case kPGPClientError_IdenticalShares :
#if 0
			PGPclMessageBox (hwnd, IDS_CAPTION, IDS_DUPLICATESHARES,
					MB_OK|MB_ICONEXCLAMATION);
#endif 0
			break;

		case kPGPClientError_DifferentSharePool :
#if 0
			PGPclMessageBox (hwnd, IDS_CAPTION, IDS_SHARENUMMISMATCH, 
					MB_OK|MB_ICONEXCLAMATION);
#endif 0
			break;

		case kPGPClientError_DifferentSplitKeys :
#if 0
			PGPclMessageBox (hwnd, IDS_CAPTION, IDS_SHAREKEYMISMATCH, 
					MB_OK|MB_ICONEXCLAMATION);
#endif 0
			break;

		default:
#if 0
			PGPclErrorBox (hwnd, err);
#endif 0
			break;
	}

#if 0	/* this allowed the user to add keys to their keyring, don't think this is needed for command line */
	
	if (PGPKeySetRefIsValid (prks->keysetToAdd)) {
		if (IsntPGPError (err)) {
			PGPclQueryAddKeys (prks->context, prks->tlsContext, hwnd,
					prks->keysetToAdd, prks->keyset);
		}
		PGPFreeKeySet (prks->keysetToAdd);
	}
#else 0
	if(PGPKeySetRefIsValid(pRecon->keysetToAdd))
		PGPFreeKeySet(pRecon->keysetToAdd);
#endif
	return err;
}


//	______________________________________________
//
//  decode event handler

static PGPError
sHandlerDecode (
		PGPContextRef	context,
		PGPEvent*		event,
		PGPUserValue	userValue)
{
	PGPError		err		= kPGPError_NoErr;
	RECONKEYSTRUCT	*pRecon = NULL;

	switch (event->type) {
	case kPGPEvent_PassphraseEvent:
		{
			char		*psz		= NULL;
			PGPByte*	pbyte		= NULL;
			PGPSize		size;
			PGPUInt16	uLen;
			char		szPrompt[64];

			pRecon = (RECONKEYSTRUCT *)userValue;

			if (!pRecon->bBadPassphrase)
			{
#if 0
				LoadString (g_hInst, IDS_DECRYPTSHARESPROMPT, 
											szPrompt, sizeof(szPrompt));
#endif 0	 
			}
			else
			{
#if 0
				LoadString (g_hInst, IDS_BADSHAREFILEPASSPHRASE, 
											szPrompt, sizeof(szPrompt));
#endif 0
			}
			if (event->data.passphraseData.fConventional)
			{
				err = pgpPassphraseDialogCmdline(pRecon->mainbPtr, TRUE, 
							"Enter share descryption passphrase: ", &psz);

			}
			else
			{
				/* need to get key from the keyset pRecon->keysetDecryption and then get passphrase for it */
				/* if key is split, need to rejoin key to get keybuffer and pass that to PGPAddJobOptions below */
				PGPKeyRef			key = kPGPInvalidRef;
				PGPBoolean			bSplitKey = FALSE;
				PGPBoolean			bNeedsFree = FALSE;
				PGPKeyListRef		keylist = kPGPInvalidRef;
				PGPKeyIterRef		keyiter = kPGPInvalidRef;

				err = PGPOrderKeySet(pRecon->keysetDecryption, kPGPUserIDOrdering, &keylist );
				pgpAssertNoErr(err);
				err = PGPNewKeyIter(keylist, &keyiter );
				pgpAssertNoErr(err);
				err = PGPKeyIterRewind( keyiter );
				pgpAssertNoErr(err);
				err = PGPKeyIterNext(keyiter, &key);

				if(key != kPGPInvalidRef)
				{
					/* check to see if key is a split key */
					err = PGPGetKeyBoolean(key, kPGPKeyPropIsSecretShared, &bSplitKey);
					pgpAssertNoErr(err);
					if(bSplitKey)
					{
						/* reconstitute that key dude */
					}
					else
					{
						/* get passphrase for key */
						err = pgpGetValidPassphrase(pRecon->mainbPtr, key, &psz, &bNeedsFree );
					}
				}
				else
				{
					err = kPGPError_UserAbort;
				}



#if 0
				err = KMGetDecryptionPhrase (context, prks->tlsContext, 
								prks->hwndDlg, szPrompt, prks->keyset, 
								NULL, prks->keysetDecryption,
								prks->iKeyIDCount, prks->keyidsDecryption,
								&prks->keysetToAdd, &psz, &pbyte, &size);
				prks->iIconIndex = IDX_DSAUSERID;
#endif 0
			}

			if (IsntPGPError (err)) 
			{
				if (psz) 
				{
					uLen = strlen (psz);
					PGPAddJobOptions (event->job, 
						PGPOPassphraseBuffer (context, psz, uLen),
						PGPOLastOption (context));
				}
				else
				{
					PGPAddJobOptions (event->job, 
						PGPOPasskeyBuffer (context, pbyte, size),
						PGPOLastOption (context));
				}
			}

			if(psz)
				PGPFreeData(psz);
			if (pbyte)
			{
				memset(pbyte, 0, size);
				PGPFreeData(pbyte);
			}

			// If passphrase event comes up again, the passphrase
			// must have been bad

			pRecon->bBadPassphrase = TRUE;
		}
		break;

	case kPGPEvent_RecipientsEvent:
		{
			PGPUInt32	i;

			PGPEventRecipientsData *pData = &event->data.recipientsData;
			pRecon = (RECONKEYSTRUCT *)userValue;
		
			/* Save recipient key set for passphrase dialog */
			pRecon->keysetDecryption = pData->recipientSet;
			PGPIncKeySetRefCount (pRecon->keysetDecryption);

			/* Save unknown keyids */
			/* should probably just error if there are keys required that aren't available in key set */
			if (pData->keyCount > 0)
			{
				pRecon->iKeyIDCount = pData->keyCount;
				pRecon->keyidsDecryption = 
					(PGPKeyID *)calloc(pData->keyCount, sizeof(PGPKeyID));

				for (i=0; i<pData->keyCount; i++)
				{
					pRecon->keyidsDecryption[i] = pData->keyIDArray[i];
				}
			}
		}
		break;
	}

	return err;
}

PGPError ReconstituteKey(struct pgpmainBones	*mainbPtr,
						 PGPKeyRef				key,
						 PGPByte				**ppPasskeyBuffer,
						 PGPSize				*piPasskeyLength) 
{
	PGPError			err = kPGPError_NoErr;

	return err;
}




/*
__________________________________________________________________________________________
*/

#if 0
/* taken from KMChange.c */

  //	___________________________________________________
//
//  Change passphrase on key 

BOOL 
KMChangePhrase (
		HWND				hwndParent, 
		PKEYMAN				pKM,
		PGPContextRef		context,
		PGPtlsContextRef	tlsContext,
		PGPKeySetRef		keyset,
		PGPKeyRef			key) 
{
	LPSTR		pszOldPhrase		= NULL;
	LPSTR		pszNewPhrase		= NULL;
	PGPByte*	pPasskey			= NULL;
	PGPPrefRef	prefs				= kInvalidPGPPrefRef;
	INT			iMinPhraseLength	= 0;
	INT			iMinPhraseQuality	= 0;

	CHAR		szPrompt[64];
	PGPSize		sizePasskey;
	PGPError	err;
	BOOL		bSplit;
#if PGP_BUSINESS_SECURITY
	PGPBoolean	b;
	PGPUInt32	u;
#endif

	if (!sbChangingPhrase) {
		sbChangingPhrase = TRUE;

	// minimum passphrase length
#if PGP_BUSINESS_SECURITY
		err = PGPclOpenAdminPrefs (
				PGPGetContextMemoryMgr (context), 
				&prefs, PGPclIsAdminInstall ()); CKERR;

		b = FALSE;
		err = PGPGetPrefBoolean (prefs, kPGPPrefEnforceMinChars, &b); CKERR;
		if (b) {
			err = PGPGetPrefNumber (prefs, kPGPPrefMinChars, &u); CKERR;
			iMinPhraseLength = (INT)u;
		}
#endif 

	// minimum passphrase quality
#if PGP_BUSINESS_SECURITY
		b = FALSE;
		PGPGetPrefBoolean (prefs, kPGPPrefEnforceMinQuality, &b); CKERR;
		if (b) {
			err = PGPGetPrefNumber (prefs, kPGPPrefMinQuality, &u); CKERR;
			iMinPhraseQuality = (INT)u;
		}
#endif 

		err = KMGetKeyPhrase (
			context,
			tlsContext,
			hwndParent, 
			NULL,
			keyset,
			key,
			&pszOldPhrase,
			&pPasskey,
			&sizePasskey);

		if (IsntPGPError (err)) {

			LoadString (g_hInst, IDS_NEWPHRASEPROMPT, 
							szPrompt, sizeof(szPrompt));
			err = KMGetConfirmationPhrase (
				context, 
				hwndParent,
				szPrompt,
				keyset,
				iMinPhraseLength,
				iMinPhraseQuality,
				&pszNewPhrase);

			if (IsntPGPError (err)) {
				if (pszOldPhrase) bSplit = FALSE;
				else bSplit = TRUE;

				if (bSplit) {
					err = sChangePhraseSplit (
						context,
						keyset,
						key,
						pPasskey,
						sizePasskey, 
						pszNewPhrase);
				}
				else {
					err = sChangePhraseNormal (
						context,
						keyset,
						key,
						pszOldPhrase, 
						pszNewPhrase);
				}
			}
		}

		if (pszOldPhrase)
			KMFreePhrase (pszOldPhrase);
		if (pszNewPhrase)
			KMFreePhrase (pszNewPhrase);
		if (pPasskey)
			KMFreePasskey (pPasskey, sizePasskey);

		if (IsPGPError (err))
			PGPclErrorBox (hwndParent, err);
		else {
			PGPclNotifyPurgePassphraseCache (
					PGPCL_DECRYPTIONCACHE|PGPCL_SIGNINGCACHE, 0);
			if (bSplit)
				KMMessageBox (hwndParent, IDS_PGP, IDS_KEYRECONSTITUTED, 
								MB_OK|MB_ICONINFORMATION);
			else
				KMMessageBox (hwndParent, IDS_PGP, IDS_PHRASECHANGED, 
								MB_OK|MB_ICONINFORMATION);
		}

	}

#if PGP_BUSINESS_SECURITY
done :
	if (PGPPrefRefIsValid (prefs))
		PGPclCloseAdminPrefs (prefs, FALSE);
#endif

	sbChangingPhrase = FALSE;

	PGPclErrorBox (hwndParent, err);

	return (IsntPGPError (err));
}

//	___________________________________________________
//
//  Change Passphrase of split key and all subkeys

static PGPError 
sChangePhraseSplit (
		PGPContextRef	context,
		PGPKeySetRef	keyset,
		PGPKeyRef		key, 
		PGPByte*		passkey,
		PGPSize			sizePasskey,
		LPSTR			szNew) 
{
	UINT			u;
	PGPKeyListRef	keylist;
	PGPKeyIterRef	keyiter;
	PGPSubKeyRef	subkey;
	PGPError		err;

	err = PGPChangePassphrase (key, 
			PGPOPasskeyBuffer (context, passkey, sizePasskey),
			PGPOPassphrase (context, szNew), 
			PGPOLastOption (context));
	if (IsPGPError (err)) return err;

	PGPGetKeyNumber (key, kPGPKeyPropAlgID, &u);
	switch (u) {
	case kPGPPublicKeyAlgorithm_RSA :
		break;

	case kPGPPublicKeyAlgorithm_DSA :
		PGPOrderKeySet (keyset, kPGPAnyOrdering, &keylist);
		PGPNewKeyIter (keylist, &keyiter);
		PGPKeyIterSeek (keyiter, key);
		PGPKeyIterNextSubKey (keyiter, &subkey);
		while (subkey) {
			err = PGPChangeSubKeyPassphrase (subkey, 
						PGPOPasskeyBuffer (context, passkey, sizePasskey),
						PGPOPassphrase (context, szNew),
						PGPOLastOption (context));
			PGPKeyIterNextSubKey (keyiter, &subkey);
		}
		PGPFreeKeyIter (keyiter);
		PGPFreeKeyList (keylist);
		break;

	default :
		break;
	}

	return err;
}





/* taken from clrecon.c */
typedef struct {
	PGPContextRef		context;
	PGPtlsContextRef	tlsContext;
	HWND				hwndDlg;
	HWND				hwndList;
	HIMAGELIST			hIml;
	PGPKeySetRef		keyset;
	PGPKeyRef			keyToReconstitute;
	PGPKeyID			keyidToReconstitute;
	PGPKeyRef			keyAuthenticating;
	PGPKeySetRef		keysetDecryption;
	PGPUInt32			iKeyIDCount;
	PGPKeyID*			keyidsDecryption;
	PGPKeySetRef		keysetToAdd;
	CHAR				szAuthUserID[kPGPMaxUserIDSize+1];
	LPSTR				pszPhraseAuth;
	PGPByte*			pPasskeyAuth;
	PGPSize				sizePasskeyAuth;
	UINT				uNeededShares;
	UINT				uCollectedShares;
	PGPShareRef			sharesCombined;
	PGPskepRef			skep;
	INT					iIconIndex;
	BOOL				bServerMode;
	BOOL				bStop;
	BOOL				bUserCancel;
	BOOL				bUserOK;
	BOOL				bBadPassphrase;
	CRITICAL_SECTION	critsecAddShare;
} RECONKEYSTRUCT, *PRECONKEYSTRUCT;


//	______________________________________________
//
//  Reconstitute specified key

PGPError PGPclExport
PGPclReconstituteKey (
		PGPContextRef		context,
		PGPtlsContextRef	tlsContext,
		HWND				hwnd,
		PGPKeySetRef		keyset,
		PGPKeyRef			key,
		PGPByte**			ppPasskey,
		PGPUInt32*			piPasskeyLength) 
{
	RECONKEYSTRUCT		rks;
	PGPError			err;

	// initialize struct
	rks.context				= context;
	rks.tlsContext			= tlsContext;
	rks.keyset				= keyset;
	rks.keyToReconstitute	= key;
	PGPGetKeyIDFromKey (key, &(rks.keyidToReconstitute));
	rks.uCollectedShares	= 0;
	rks.uNeededShares		= UNKNOWN_SHARES_NEEDED;
	rks.sharesCombined		= NULL;
	rks.bServerMode			= FALSE;
	rks.bUserCancel			= FALSE;
	rks.bUserOK				= FALSE;
	rks.bBadPassphrase		= FALSE;
	rks.pszPhraseAuth		= NULL;
	rks.pPasskeyAuth		= NULL;
	rks.skep				= NULL;
	rks.iIconIndex			= -1;

	InitializeCriticalSection (&rks.critsecAddShare);

	do {
		if (DialogBoxParam (g_hInst, MAKEINTRESOURCE (IDD_RECONSTITUTEKEY), 
			hwnd, sReconKeyDlgProc, (LPARAM)&rks)) {
			err = PGPGetPasskeyFromShares (rks.sharesCombined, 
							ppPasskey, piPasskeyLength);
			PGPclErrorBox (hwnd, err);

			if (!PGPPassphraseIsValid (key, 
					PGPOPasskeyBuffer (context, 
								*ppPasskey, *piPasskeyLength),
					PGPOLastOption (context))) {
				PGPclMessageBox (hwnd, IDS_CAPTION, IDS_BADPASSKEY, 
									MB_OK|MB_ICONSTOP);
				err = kPGPError_BadPassphrase;
			}
		}
		else {
			*ppPasskey = NULL;
			*piPasskeyLength = 0;
			err = kPGPError_UserAbort;
		}
	} while (err == kPGPError_BadPassphrase);

	DeleteCriticalSection (&rks.critsecAddShare);

	if (rks.sharesCombined)
		PGPFreeShares (rks.sharesCombined);

	if (rks.pszPhraseAuth)
		KMFreePhrase (rks.pszPhraseAuth);

	if (rks.pPasskeyAuth)
		KMFreePasskey (rks.pPasskeyAuth, rks.sizePasskeyAuth);

	return err;
}


//	______________________________________________
//
//  add share file to list


static VOID 
sAddShareFile (
		HWND			hwnd, 
		PRECONKEYSTRUCT	prks)
{
	PFLFileSpecRef		fileref				= NULL;
	PGPShareFileRef		sharefileref		= NULL;
	PGPOptionListRef	optionsDecode		= NULL;
	PGPShareRef			shares				= NULL;
	PGPShareRef			sharesTemp			= NULL;
	PGPError			err					= kPGPError_NoErr;

	OPENFILENAME	ofn;
	LPSTR			p;
	CHAR			szFile[MAX_PATH];
	CHAR			szName[kPGPMaxUserIDSize+1];
	CHAR			sz[256];
	CHAR			szTitle[64];
	PGPUInt32		size;
	PGPUInt32		uNumShares;
	UINT			uThreshold;
	PGPKeyID		keyid;

	// initialize
	prks->keysetToAdd		= kInvalidPGPKeySetRef;
	prks->keysetDecryption	= kInvalidPGPKeySetRef;
	prks->iKeyIDCount		= 0;
	prks->keyidsDecryption	= NULL;

	// prompt user for name of share file to send
	szFile[0] = '\0';
	LoadString (g_hInst, IDS_SHAREFILEFILTER, sz, sizeof(sz));
	while (p = strrchr (sz, '@')) *p = '\0';
	LoadString (g_hInst, IDS_SHAREFILECAPTION, szTitle, sizeof(szTitle));

	ofn.lStructSize       = sizeof (OPENFILENAME);
	ofn.hwndOwner         = hwnd;
	ofn.hInstance         = (HANDLE)g_hInst;
	ofn.lpstrFilter       = sz;
	ofn.lpstrCustomFilter = (LPTSTR)NULL;
	ofn.nMaxCustFilter    = 0L;
	ofn.nFilterIndex      = 1L;
	ofn.lpstrFile         = szFile;
	ofn.nMaxFile          = sizeof (szFile);
	ofn.lpstrFileTitle    = NULL;
	ofn.nMaxFileTitle     = 0;
	ofn.lpstrInitialDir   = NULL;
	ofn.lpstrTitle        = szTitle;
	ofn.Flags			  = OFN_HIDEREADONLY;
	ofn.nFileOffset       = 0;
	ofn.nFileExtension    = 0;
	ofn.lpstrDefExt       = "";
	ofn.lCustData         = 0;

	EnterCriticalSection (&prks->critsecAddShare);

	if (GetOpenFileName (&ofn)) {
		err = PFLNewFileSpecFromFullPath (
				PGPGetContextMemoryMgr (prks->context), szFile, &fileref);
		if (IsPGPError (err)) goto AddCleanup;
 
		err = PGPOpenShareFile (fileref, &sharefileref);
		if (IsPGPError (err)) goto AddCleanup;

		err = PGPGetShareFileSharedKeyID (sharefileref, &keyid);
		if (IsPGPError (err)) goto AddCleanup;

		if (PGPCompareKeyIDs (&keyid, &(prks->keyidToReconstitute))) {
			PGPclMessageBox (prks->hwndDlg, IDS_CAPTION, 
						IDS_SHAREKEYMISMATCH, MB_OK|MB_ICONEXCLAMATION);
			goto AddCleanup;
		}

		// check that threshold corresponds to other share files
		uThreshold = PGPGetShareThresholdInFile (sharefileref);
		if (prks->uNeededShares != UNKNOWN_SHARES_NEEDED) {
			if (uThreshold != prks->uNeededShares) {
				PGPclMessageBox (prks->hwndDlg, IDS_CAPTION, 
						IDS_SHARENUMMISMATCH, MB_OK|MB_ICONEXCLAMATION);
				goto AddCleanup;
			}
		}

		err = PGPGetShareFileUserID (sharefileref,
									sizeof(szName), szName, &size);
		if (IsPGPError (err)) goto AddCleanup;

		uNumShares = PGPGetNumSharesInFile (sharefileref);

		// decrypt specified share file
		prks->bBadPassphrase = FALSE;
		PGPBuildOptionList (prks->context, &optionsDecode,
					PGPOKeySetRef (prks->context, prks->keyset),
					PGPOEventHandler (prks->context, sHandlerDecode, prks),
					PGPOLastOption (prks->context));
		err = PGPCopySharesFromFile (prks->context, sharefileref, 
						optionsDecode, &shares);
		if (IsPGPError (err)) goto AddCleanup;

		// add shares to collection
		if (prks->sharesCombined) {
			err = PGPCombineShares (shares, 
									prks->sharesCombined, &sharesTemp);
			if (IsPGPError (err)) 
				goto AddCleanup;
			PGPFreeShares (prks->sharesCombined);
			prks->sharesCombined = sharesTemp;
		}
		else {
			prks->sharesCombined = shares;
			shares = NULL;
		}

		// share is OK, add it to list
		prks->uNeededShares = uThreshold;
		SetDlgItemInt (prks->hwndDlg, IDC_SHARESNEEDED, 
					prks->uNeededShares, FALSE);

		prks->uCollectedShares += uNumShares;
		SetDlgItemInt (hwnd, IDC_SHARESCOLLECTED, 
								prks->uCollectedShares, FALSE);
		sAddShareHolderToList (prks, szName, uNumShares);
	}

AddCleanup :
	LeaveCriticalSection (&prks->critsecAddShare);

	if (shares) 
		PGPFreeShares (shares);

	if (sharefileref)
		PGPFreeShareFile (sharefileref);

	if (fileref)
		PFLFreeFileSpec (fileref);

	if (optionsDecode)
		PGPFreeOptionList(optionsDecode);

	if (PGPKeySetRefIsValid (prks->keysetDecryption))
		PGPFreeKeySet (prks->keysetDecryption);

	if (prks->keyidsDecryption)
		clFree (prks->keyidsDecryption);

	switch (err) {
		case kPGPClientError_IdenticalShares :
			PGPclMessageBox (hwnd, IDS_CAPTION, IDS_DUPLICATESHARES,
					MB_OK|MB_ICONEXCLAMATION);
			break;

		case kPGPClientError_DifferentSharePool :
			PGPclMessageBox (hwnd, IDS_CAPTION, IDS_SHARENUMMISMATCH, 
					MB_OK|MB_ICONEXCLAMATION);
			break;

		case kPGPClientError_DifferentSplitKeys :
			PGPclMessageBox (hwnd, IDS_CAPTION, IDS_SHAREKEYMISMATCH, 
					MB_OK|MB_ICONEXCLAMATION);
			break;

		default:
			PGPclErrorBox (hwnd, err);
	}

	if (PGPKeySetRefIsValid (prks->keysetToAdd)) {
		if (IsntPGPError (err)) {
			PGPclQueryAddKeys (prks->context, prks->tlsContext, hwnd,
					prks->keysetToAdd, prks->keyset);
		}
		PGPFreeKeySet (prks->keysetToAdd);
	}
}

/* taken from clrecon.c */
//	______________________________________________
//
//  decode event handler

static PGPError
sHandlerDecode (
		PGPContextRef	context,
		PGPEvent*		event,
		PGPUserValue	userValue)
{
	PGPError		err		= kPGPError_NoErr;
	PRECONKEYSTRUCT	prks;

	switch (event->type) {
	case kPGPEvent_PassphraseEvent:
		{
			LPSTR		psz			= NULL;
			PGPByte*	pbyte		= NULL;
			PGPSize		size;
			UINT		uLen;
			CHAR		szPrompt[64];

			prks = (PRECONKEYSTRUCT)userValue;

			if (!prks->bBadPassphrase)
				LoadString (g_hInst, IDS_DECRYPTSHARESPROMPT, 
											szPrompt, sizeof(szPrompt));
			else
				LoadString (g_hInst, IDS_BADSHAREFILEPASSPHRASE, 
											szPrompt, sizeof(szPrompt));

			if (event->data.passphraseData.fConventional) {
				err = KMGetKeyPhrase (context, prks->tlsContext, 
								prks->hwndDlg, szPrompt, prks->keyset, 
								NULL, &psz, &pbyte, &size);
				prks->iIconIndex = IDX_HEAD;
			}
			else {
				err = KMGetDecryptionPhrase (context, prks->tlsContext, 
								prks->hwndDlg, szPrompt, prks->keyset, 
								NULL, prks->keysetDecryption,
								prks->iKeyIDCount, prks->keyidsDecryption,
								&prks->keysetToAdd, &psz, &pbyte, &size);
				prks->iIconIndex = IDX_DSAUSERID;
			}

			if (IsntPGPError (err)) {
				if (psz) {
					uLen = strlen (psz);
					PGPAddJobOptions (event->job, 
						PGPOPassphraseBuffer (context, psz, uLen),
						PGPOLastOption (context));
				}
				else {
					PGPAddJobOptions (event->job, 
						PGPOPasskeyBuffer (context, pbyte, size),
						PGPOLastOption (context));
				}
			}

			if (psz) 
				KMFreePhrase (psz);

			if (pbyte)
				KMFreePasskey (pbyte, size);

			// If passphrase event comes up again, the passphrase
			// must have been bad

			prks->bBadPassphrase = TRUE;
		}
		break;

	case kPGPEvent_RecipientsEvent:
		{
			PGPUInt32	i;

			PGPEventRecipientsData *pData = &event->data.recipientsData;
			prks = (PRECONKEYSTRUCT)userValue;
		
			// Save recipient key set for passphrase dialog
			prks->keysetDecryption = pData->recipientSet;
			PGPIncKeySetRefCount (prks->keysetDecryption);

			// Save unknown keyids
			if (pData->keyCount > 0) {
				prks->iKeyIDCount = pData->keyCount;
				prks->keyidsDecryption = 
					(PGPKeyID *)clAlloc (pData->keyCount * sizeof(PGPKeyID));

				for (i=0; i<pData->keyCount; i++) {
					prks->keyidsDecryption[i] = pData->keyIDArray[i];
				}
			}
		}
		break;
	}

	return err;
}


/* taken from KMUser.c */
//----------------------------------------------------|
// get passphrase for key from user

PGPError  
KMGetKeyPhrase (
		PGPContextRef		context,
		PGPtlsContextRef	tlsContext,
		HWND				hwnd, 
		LPSTR				szPrompt,
		PGPKeySetRef		keyset,
		PGPKeyRef			key,
		LPSTR*				ppszPhrase,
		PGPByte**			ppPasskeyBuffer,
		PGPUInt32*			piPasskeyLength) 
{
	PGPError	err				= kPGPError_BadParams;
	PGPBoolean	bSplit			= FALSE;
	LPSTR		psz;
	UINT		uFlags;

	if (!ppPasskeyBuffer) return err;
	if (!piPasskeyLength) return err;

	psz = NULL;
	*ppPasskeyBuffer = NULL;
	*piPasskeyLength = 0;

	if (key) 
		PGPGetKeyBoolean (key, kPGPKeyPropIsSecretShared, &bSplit);

	do {
		if (bSplit) {
			err=PGPclReconstituteKey(
				context,			// in context
				tlsContext,			// in TLS context
				hwnd,				// in hwnd of parent
				keyset,				// in keyset
				key,				// in key
				ppPasskeyBuffer,	// out passkey buffer
				piPasskeyLength);	// out passkey length
		}
		else {
			if (key) uFlags = PGPCL_KEYPASSPHRASE;
			else uFlags = PGPCL_DECRYPTION;
			err = PGPclGetPhrase (
				context,			// in context
				keyset,				// in main keyset
				hwnd,				// in hwnd of parent
				szPrompt,			// in prompt
				&psz,				// out phrase
				NULL,				// in keyset
				NULL,				// in keyids
				0,					// in keyid count
				&key,				// in/out key
				NULL,				// out options
				uFlags,				// in flags
				ppPasskeyBuffer,	// out passkey buffer
				piPasskeyLength,	// out passkey length
				0,0,				// in min length/quality
				tlsContext,			// in tlsContext,
				NULL,				// out AddedKeys
				NULL);				

		}

		if (IsPGPError (err)) {
			if (psz) {
				PGPFreeData (psz);
				psz = NULL;
			}
			if (*ppPasskeyBuffer) {
				PGPFreeData (*ppPasskeyBuffer);
				*ppPasskeyBuffer = NULL;
			}
			PGPclErrorBox (hwnd, err);
		}
		else {
			if (ppszPhrase) *ppszPhrase = psz;
			else KMFreePhrase (psz);
		}

	} while (err == kPGPError_BadPassphrase);

	return err;
}

#endif 0

/*__________________________________________________________________________________________
*/

