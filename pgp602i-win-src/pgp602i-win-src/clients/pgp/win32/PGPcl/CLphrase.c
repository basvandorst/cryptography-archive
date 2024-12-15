/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	CLphrase.c - get passphrase from user

	Glue code to interface with PGPsdkUI

	$Id: CLphrase.c,v 1.40 1998/08/11 14:43:10 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpclx.h"
#include "pgpUserInterface.h"
extern HINSTANCE g_hInst;

//___________________________
//
// Secure memory allocation routines
//

VOID* 
secAlloc (PGPContextRef context, UINT uBytes) 
{
	PGPMemoryMgrRef	memmgr;

	memmgr = PGPGetContextMemoryMgr (context);
	return (PGPNewSecureData (memmgr, uBytes, 0));
}


VOID 
secFree (VOID* p) 
{
	if (p) {
		FillMemory (p, lstrlen (p), 0x00);
		PGPFreeData (p);
	}
}

//	________________________
//
//  wipe and free passphrase 

VOID PGPclExport 
PGPclFreePhrase (LPSTR pszPhrase) 
{
	if (pszPhrase) {
		secFree (pszPhrase);
	}
}

//	_______________________________________________________
//
//  Entry point called by app to post dialog and get phrase
// 
//  This used to be the main entry point for passphrase 
//  dialogs. Now it is used to convert the calling parameters
//  to the new PGP UI Library, and then massage the output
//  back into something the old client code can understand.
//  
//  wjb

PGPError PGPclExport 
PGPclGetPhrase (
		PGPContextRef	context,
		PGPKeySetRef	MainKeySet,
		HWND			hWndParent, 
		LPSTR			szPrompt,
		LPSTR*			ppszPhrase, 
		PGPKeySetRef	KeySet,
		PGPKeyID*		pKeyIDs,
		UINT			uKeyCount,
		PGPKeyRef*		pKey, 
		UINT*			puOptions, 
		UINT			uFlags,
		PGPByte**		ppPasskeyBuffer,
		PGPUInt32*		piPasskeyLength,
		PGPUInt32		MinLength,
		PGPUInt32		MinQuality,
		PGPtlsContextRef tlsContext,
		PGPKeySetRef	*AddedKeys
		) 
{										
	PGPError					err				= kPGPError_NoErr;
	PGPOptionListRef			optionList		= kInvalidPGPOptionListRef;
	PGPKeyServerEntry			*ksEntries		= NULL;
	PGPUInt32					numKSEntries	= 0;
	PGPKeyServerSpec			*serverList		= NULL;
	PGPKeySetRef				foundKeys		= kInvalidPGPKeySetRef;
	PGPPrefRef					clientPrefsRef	= kInvalidPGPPrefRef;
	PGPMemoryMgrRef				memMgr			= kInvalidPGPMemoryMgrRef;
	PCLIENTSERVERSTRUCT			pcss			= NULL;

	CHAR						StrRes1[100],StrRes2[100];
	PGPKeyRef 					decryptionKey;	// dummy, we don't care
	
	memMgr = PGPGetContextMemoryMgr (context);
	err = PGPclOpenClientPrefs (memMgr,&clientPrefsRef);

	if (IsntPGPError(err))
	{
		err=CLInitKeyServerPrefs(PGPCL_DEFAULTSERVER,NULL,
			hWndParent,context,MainKeySet,clientPrefsRef,"",
			&pcss,&ksEntries,&serverList,&numKSEntries);

		PGPclCloseClientPrefs (clientPrefsRef, FALSE);
	}

	optionList = kInvalidPGPOptionListRef;

	// Everybody needs a passphrase buffer and a hwnd
	// If no length quality needed, zeros are default anyway
	err = PGPBuildOptionList( context, &optionList,
			PGPOUIOutputPassphrase( context, ppszPhrase ),
			PGPOUIParentWindowHandle( context, hWndParent ),
			PGPOUIMinimumPassphraseLength(context,MinLength),
			PGPOUIMinimumPassphraseQuality(context,MinQuality),
			PGPOLastOption( context ) );

	if( IsntPGPError( err ) )
	{
		PGPUInt32	detachedSignature	= 0;
		PGPUInt32	textOutput			= 0;
		Boolean		haveFileOptions 	= FALSE;

		// If we have a prompt, use it
		if( IsntNull( szPrompt ) )
		{
			err = PGPAppendOptionList( optionList,
				PGPOUIDialogPrompt( context, szPrompt ),
				PGPOLastOption( context ) );
		}
		
		// If we have options, convert them to new options API
		if( IsntPGPError( err ) && ( puOptions != 0 ))
		{
			haveFileOptions = TRUE;

			if( (*puOptions & PGPCL_DETACHEDSIG) != 0 )
				detachedSignature = 1;

			if( (*puOptions & PGPCL_ASCIIARMOR) != 0 )
				textOutput = 1;
		
			LoadString (g_hInst, IDS_DETACHEDSIG, StrRes1, sizeof(StrRes1));
			LoadString (g_hInst, IDS_TEXTOUTPUT, StrRes2, sizeof(StrRes2));

			err = PGPAppendOptionList( optionList,
					PGPOUIDialogOptions( context,
						PGPOUICheckbox( context, 804,
							StrRes1,NULL,
							detachedSignature, &detachedSignature ),
						PGPOUICheckbox( context, 801,
							StrRes2,NULL,
							textOutput, &textOutput ),
					PGPOLastOption( context ) ),
					PGPOLastOption( context ) );
		}

		if( IsntPGPError( err ) )
		{
			// Conventional encryption passphrase needed
			if(uFlags&PGPCL_ENCRYPTION)
			{
				PGPAppendOptionList( optionList,
					PGPOUIShowPassphraseQuality(context,TRUE),
					PGPOLastOption( context ) );

				err=PGPConventionalEncryptionPassphraseDialog(context,
					optionList,
					PGPOLastOption( context ) );
			}
			// We're decoding......
			else if(uFlags&PGPCL_DECRYPTION)
			{
				PGPInt32 numKeys;

				numKeys=0;

				if(PGPRefIsValid(KeySet))
					PGPCountKeys( KeySet, &numKeys );

				// A conventionally encrypted file
				if((numKeys==0)&&(uKeyCount==0))
				{
					err=PGPConventionalDecryptionPassphraseDialog(context,
						optionList,
						PGPOLastOption( context ) );
				}
				// A RSA or DH encrypted file
				else
				{
					if(pKey==NULL)
						pKey=&decryptionKey;
				
					err=PGPDecryptionPassphraseDialog(
						context,							
						KeySet,
						uKeyCount,
						pKeyIDs,
						pKey, // for recon dialog
						optionList,
						PGPOUIKeyServerUpdateParams(context,
							numKSEntries, serverList,
							tlsContext,FALSE,&foundKeys,
							PGPOLastOption( context ) ),
						PGPOLastOption( context ) );
				}
			}
			else if(uFlags&PGPCL_KEYPASSPHRASE)
			{
				err=PGPKeyPassphraseDialog(
					context,
					*pKey,
					optionList,
					PGPOLastOption( context ) ); 
			}
			// We're signing something and need the combo box
			else
			{
				err = PGPSigningPassphraseDialog( context, MainKeySet,
					pKey,
					optionList,
					PGPOUIDefaultKey( context, *pKey ),
					PGPOLastOption( context ) );
			}
		}

		// Shared key has been selected. Go to reconstitution dialog
		if(((err==kPGPError_KeyUnusableForSignature)||
            (err==kPGPError_KeyUnusableForDecryption)) &&
		    (!(uFlags & PGPCL_REJECTSPLITKEYS)))
		{
			if((ppPasskeyBuffer!=NULL)&&(piPasskeyLength!=NULL)&&(pKey!=NULL))
			{
				err=PGPclReconstituteKey(
					context,
					tlsContext,
					hWndParent,
					MainKeySet,
					*pKey,
					ppPasskeyBuffer,
					piPasskeyLength);
			}
		}

		if( (IsntPGPError(err)) && 
			(ppPasskeyBuffer!=NULL) &&
			(piPasskeyLength!=NULL) &&
			(pKey!=NULL))
		{
			// Convert passphrase to passkey
			if((*ppPasskeyBuffer==NULL)&&(*pKey!=NULL)&&(*ppszPhrase!=NULL))
				
			{
				if (lstrlen(*ppszPhrase)>0) 
				{
					PGPUInt32 uKeyLockingBits;

					PGPGetKeyNumber (*pKey, kPGPKeyPropLockingBits, 
										&uKeyLockingBits);

					*piPasskeyLength=(uKeyLockingBits+7)/8; // Bits to bytes
					*ppPasskeyBuffer=
						(PGPByte *)secAlloc(context,*piPasskeyLength);

					err = PGPGetKeyPasskeyBuffer(*pKey, *ppPasskeyBuffer,
						PGPOPassphrase(context, *ppszPhrase),
						PGPOLastOption(context));
				}
				else 
				{
					*piPasskeyLength=0;
					*ppPasskeyBuffer=NULL;
				}
			}
		}

		// If we had options, read the results and send em back
		if( IsntPGPError( err ) && haveFileOptions )
		{
			*puOptions = 0;
			
			if( detachedSignature != 0 )
				*puOptions |= PGPCL_DETACHEDSIG;
				
			if( textOutput != 0 )
				*puOptions |= PGPCL_ASCIIARMOR;
		}
		
		PGPFreeOptionList( optionList );
	}

	CLUninitKeyServerPrefs(PGPCL_DEFAULTSERVER,
		pcss,ksEntries,serverList,numKSEntries);

	if(AddedKeys!=NULL)
	{
		*AddedKeys=foundKeys;
	}
	else
	{
		if(PGPRefIsValid(foundKeys))
			PGPFreeKeySet(foundKeys);
	}

	return(err);
}

