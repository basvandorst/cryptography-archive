/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	$Id: PGPPassphraseDialog.cpp,v 1.7 1999/04/19 17:59:55 melkins Exp $
____________________________________________________________________________*/

#include <stdio.h>
/* aCC barfs on <sys/time.h> if <sys/sigevent.h> is not included first */
#if PGP_COMPILER_HPUX
#include <sys/sigevent.h>
#endif /* PGP_COMPILER_HPUX */
#include <stdlib.h>
#include <string.h>

#include "pgpDialogs.h"
#include "pgpPassphraseUtils.h"
#include "pgpErrors.h"
#include "pgpTtyUtils.h"
#include "pgpKeys.h"

#define MAXDECRYPTIONNAMECHAR		36

// global variable structure for re-entrancy
typedef struct _GPP
{
	char *				pszPassPhrase;
	char *				pszPassPhraseConf;
	PGPInt32 iNextTabControl;
	PGPBoolean				bHideText;
	PGPContextRef		context;
	const CPGPPassphraseDialogOptions *options;
} GPP;

PGPError
PGPsdkUIErrorBox (
		PGPError error) 
{
	PGPError	err				= kPGPError_NoErr;
	char		szMessage[512];

	if (IsPGPError (error) && (error!=kPGPError_UserAbort)) 
	{
		PGPGetErrorString (error, sizeof(szMessage), szMessage);

		printf("%s: PGP Error", szMessage);
	}

	return err;
}

	PGPUInt32
PGPEstimatePassphraseQuality(const char *passphrase)
{
	return( pgpEstimatePassphraseQuality( passphrase ) );
}

//___________________________
//
// Secure memory allocation routines
//

void * 
secAlloc (PGPContextRef context, PGPUInt32 uBytes) 
{
	PGPMemoryMgrRef	memmgr;

	memmgr = PGPGetContextMemoryMgr (context);
	return (PGPNewSecureData (memmgr, uBytes, 0));
}


void 
secFree (void* p) 
{
	if (p) {
		memset ((char *)p, '\0', strlen((char *)p));
		PGPFreeData ((char *)p);
	}
}

//	__________________
//
//	Wipe edit box clean

void 
WipeEditBox (
		GPP *gpp,
		PGPUInt32 uID) 
{
	(void) gpp;
	(void) uID;
}

void FreePassphrases(GPP *gpp)
{
	if (gpp->pszPassPhrase) {
		secFree(gpp->pszPassPhrase);
		gpp->pszPassPhrase=NULL;
	}

	if (gpp->pszPassPhraseConf) {
		secFree(gpp->pszPassPhraseConf);
		gpp->pszPassPhraseConf=NULL;
	}

}

void ClearPassphrases(GPP *gpp)
{
	if(gpp->pszPassPhraseConf) {
		secFree(gpp->pszPassPhraseConf);
		gpp->pszPassPhraseConf=NULL;
	}

}

//	___________________________________________________
//
//	Message box routine using string table resource IDs

PGPInt32 
PGPclMessageBox (PGPInt32	iCaption, 
		 PGPInt32	iMessage,
		 PGPUInt32 ulFlags) 
{
	(void) iCaption;
	(void) iMessage;
	(void) ulFlags;
	return 0;
}

//	____________________________
//
//  setup key display list box

PGPBoolean 
AddKeySetToRecipientsTable (PGPKeySetRef KeySet,
							CPGPDecryptionPassphraseDialogOptions *options) 
{
	PGPBoolean			bAtLeastOneValidSecretKey	= FALSE;
	PGPKeyListRef	KeyList;
	PGPKeyIterRef	KeyIter;
	PGPKeyRef		Key;
	PGPSubKeyRef	SubKey;
	PGPUInt32			u, uKeyBits, uAlgorithm;
	PGPInt32				iKeyDefault, iKeySelected;
	PGPBoolean		bSecret, bCanEncrypt;
	char			szName[kPGPMaxUserIDSize];
	char			sz[128];

	(void) options;
	PGPOrderKeySet (KeySet, kPGPValidityOrdering, &KeyList);
	PGPNewKeyIter (KeyList, &KeyIter);

	iKeySelected = -1;
	iKeyDefault = -1;

	PGPKeyIterNext (KeyIter, &Key);
	while (Key) 
	{
		PGPGetKeyBoolean (Key, kPGPKeyPropIsSecret, &bSecret);
		PGPGetKeyBoolean (Key, kPGPKeyPropCanEncrypt, &bCanEncrypt);
		PGPGetKeyNumber (Key, kPGPKeyPropAlgID, (int *)&uAlgorithm);
		if (bSecret && bCanEncrypt) bAtLeastOneValidSecretKey = TRUE;

		// get name on key
		PGPGetPrimaryUserIDNameBuffer (Key, sizeof(szName), 
										szName, &u);
		if (u > MAXDECRYPTIONNAMECHAR) 
		{
			u = MAXDECRYPTIONNAMECHAR;
			strcat (szName, "...");
		}
		else 
			szName[u] = '\0';

		// append key type / size info to name
		strcat (szName, " (");

		switch (uAlgorithm) 
		{
			case kPGPPublicKeyAlgorithm_RSA :
				strcat (szName, sz);
				strcat (szName, "/");
				PGPGetKeyNumber (Key, kPGPKeyPropBits, (int *)&uKeyBits);
				sprintf (sz, "%i", uKeyBits);
				strcat (szName, sz);
				break;

			case kPGPPublicKeyAlgorithm_DSA :
				strcat (szName, sz);
				strcat (szName, "/");
				PGPKeyIterNextSubKey (KeyIter, &SubKey);
				if (SubKey) {
					PGPGetSubKeyNumber (SubKey, kPGPKeyPropBits, 
										(int *)&uKeyBits);
					sprintf (sz, "%i", uKeyBits);
					strcat (szName, sz);
				}
				else strcat (szName, "???");
				break;

			default :
				strcat (szName, sz);
				strcat (szName, "/");
				strcat (szName, sz);
				break;
		}
		strcat (szName, ")");

		PGPKeyIterNext (KeyIter, &Key);
	}
	PGPFreeKeyIter (KeyIter);
	PGPFreeKeyList (KeyList);

	return bAtLeastOneValidSecretKey;
}

PGPBoolean 
InitEncryptedToKeyListBox (CPGPDecryptionPassphraseDialogOptions *options) 
{
	PGPBoolean bAtLeastOneValidSecretKey = FALSE;

	if(PGPKeySetRefIsValid( options->mKeySet ))
		bAtLeastOneValidSecretKey = AddKeySetToRecipientsTable(
										options->mKeySet,options );

	if(PGPKeySetRefIsValid( *(options->mNewKeys) ))
		AddKeySetToRecipientsTable(*(options->mNewKeys),options );

	if( options->mMissingKeyIDList != NULL )
	{
		char MsgTxt[255];
	
		sprintf(MsgTxt,"%d unknown key(s)",options->mMissingKeyIDCount);

	}

	return bAtLeastOneValidSecretKey;
}

static
void GetKeyString(PGPKeyRef Key,char *szNameFinal)
{
	char			sz1[32],sz2[32];
	char			szName[kPGPMaxUserIDSize];
	PGPUInt32		uAlgorithm,uKeyBits;
	PGPUInt32			u;

	PGPGetKeyNumber (Key, kPGPKeyPropAlgID, (int *)&uAlgorithm);

	// get key type / size info to append to name
	strcpy (sz2, "   (");
	switch (uAlgorithm) 
	{
	case kPGPPublicKeyAlgorithm_RSA :
		strcat (sz2, "RSA/");
		PGPGetKeyNumber (Key, kPGPKeyPropBits, (int *)&uKeyBits);
		sprintf (sz1, "%i", uKeyBits);
		strcat (sz2, sz1);
		break;
		
	case kPGPPublicKeyAlgorithm_DSA :
		strcat (sz2, "DSS/");
		PGPGetKeyNumber (Key, kPGPKeyPropBits, (int *)&uKeyBits);
		sprintf (sz1, "%i", uKeyBits);
		strcat (sz2, sz1);
		break;
		
	default :
		strcat (sz2, "Unknown/Unknown");
		break;
	}
	
	strcat (sz2, ")");

	// get name on key
	PGPGetPrimaryUserIDNameBuffer (Key, sizeof(szName),szName, &u);

	strcpy(szNameFinal, szName);
	strcat(szNameFinal, sz2);
	
	//TruncateKeyText (hdc, szName, sz2, iComboWidth, szNameFinal);
}


//	____________________________
//
//  setup keyselection list o' keys
//

PGPBoolean 
InitSigningKeyComboBox (CPGPKeySetPassphraseDialogOptions *options) 
{
	PGPKeyListRef	KeyList;
	PGPKeyIterRef	KeyIter;
	PGPKeyRef		Key;
	PGPKeyRef		KeyDefault;
	PGPUInt32		uIndex;
	PGPInt32		iKeyDefault, iKeySelected;
	PGPBoolean		bSecret, bRevoked, bExpired, bCanSign;
	PGPBoolean		bAtLeastOneSecretKey;
	char			szNameFinal[kPGPMaxUserIDSize];

	PGPOrderKeySet (options->mKeySet, kPGPValidityOrdering, &KeyList);
	PGPNewKeyIter (KeyList, &KeyIter);
	PGPGetDefaultPrivateKey (options->mKeySet, &KeyDefault);

	iKeySelected = -1;
	iKeyDefault = 0;

	bAtLeastOneSecretKey = FALSE;

	PGPKeyIterNext (KeyIter, &Key);
	while (Key) {
		PGPGetKeyBoolean (Key, kPGPKeyPropIsSecret, &bSecret);
		if (bSecret) {
			PGPGetKeyBoolean (Key, kPGPKeyPropIsRevoked,
							  (unsigned char *)&bRevoked);
			PGPGetKeyBoolean (Key, kPGPKeyPropIsExpired,
							  (unsigned char *)&bExpired);
			PGPGetKeyBoolean (Key, kPGPKeyPropCanSign,
							  (unsigned char *)&bCanSign);
			if (!bRevoked && !bExpired && bCanSign) {
				bAtLeastOneSecretKey = TRUE;

				GetKeyString(Key,szNameFinal);

				fprintf(stdout, "\n%s\n", szNameFinal);
				if (options->mDefaultKey) {
					if (Key == options->mDefaultKey) 
						iKeySelected = uIndex;
				}
				if (Key == KeyDefault) iKeyDefault = uIndex;
			}
		}
		PGPKeyIterNext (KeyIter, &Key);
	}
	PGPFreeKeyIter (KeyIter);
	PGPFreeKeyList (KeyList);

	return (bAtLeastOneSecretKey);

}

PGPBoolean PassphraseLengthAndQualityOK(CPGPPassphraseDialogOptions *options,
									char *Passphrase)
{
	if(options->mMinPassphraseLength!=0)
	{
		if(strlen(Passphrase)<options->mMinPassphraseLength)
		{
			printf("Passphrase is not of sufficient length. "
						"Please choose another.");
			return FALSE;
		}
	}

	if(options->mMinPassphraseQuality!=0)
	{
		if(PGPEstimatePassphraseQuality(Passphrase) <
				options->mMinPassphraseQuality)
		{
			printf("Passphrase is not of sufficient quality. "
						"Please choose another.");
			return FALSE;
		}
	}

	return TRUE;
}

//	____________________________
//
//  search keys for matching phrase

PGPError 
ValidateSigningPhrase (GPP *gpp, char * pszPhrase, PGPKeyRef key) 
{
	char	szName[kPGPMaxUserIDSize];
	char	sz[128];
	char	sz2[kPGPMaxUserIDSize + 128];
	PGPSize	size;
	CPGPSigningPassphraseDialogOptions *options;

	options = (CPGPSigningPassphraseDialogOptions *)gpp->options;

	// does phrase match selected key ?
	if (PGPPassphraseIsValid (key, 
			PGPOPassphrase (gpp->context, pszPhrase),
			PGPOLastOption (gpp->context))) {
		*(options->mPassphraseKeyPtr) = key;
		return kPGPError_NoErr;
	}

	if (options->mFindMatchingKey) {
		// does phrase match any private key ?
		key=GetKeyForPassphrase(options->mKeySet,pszPhrase,TRUE);

		if (key != NULL) {
			// ask user to use other key
			PGPGetPrimaryUserIDNameBuffer (key, sizeof(szName), szName, &size);
			sprintf (sz2, sz, szName);
			return kPGPError_BadPassphrase;
		}
	}

	// phrase doesn't match any key
	printf("Bad Passphrase: Please re-enter\n");
	
	return kPGPError_BadPassphrase;

}

// ****************************************************************************
// ****************************************************************************

// Just a simple decryption
	PGPError
pgpPassphraseDialogPlatform(
	PGPContextRef					context,
	CPGPPassphraseDialogOptions 	*options)
{
	PGPError err = 0;
	GPP	gpp;

	memset(&gpp,0x00,sizeof(GPP));
	gpp.context=context;
	gpp.options=options;

//	InitRandomKeyHook(&hhookMouse,&hhookKeyboard);

//	err = DialogBoxParam (g_hInst, 
	//	MAKEPGPInt32 (IDD_PASSPHRASE), 
	//	options->mHwndParent,
	//	(DLGPROC)pgpPassphraseDlgProc, (LPARAM)&gpp);

//	*(options->mPassphrasePtr)=gpp.pszPassPhrase;

//	UninitRandomKeyHook(hhookMouse,hhookKeyboard);

	return(err);
}

// Show the recipients
	PGPError
pgpDecryptionPassphraseDialogPlatform(
	PGPContextRef							context,
	CPGPDecryptionPassphraseDialogOptions	*options)
{
	PGPError err = 0;
	GPP	gpp;

	memset(&gpp,0x00,sizeof(GPP));
	gpp.context=context;
	gpp.options=options;

//	InitRandomKeyHook(&hhookMouse,&hhookKeyboard);

	//err = DialogBoxParam (g_hInst, 
	//	MAKEINTRESOURCE (IDD_PASSPHRASEDECRYPTKEYS), 
	//	options->mHwndParent,
	//	(DLGPROC)pgpDecryptionPassphraseDlgProc, (LPARAM)&gpp);

	//*(options->mPassphrasePtr)=gpp.pszPassPhrase;

	//UninitRandomKeyHook(hhookMouse,hhookKeyboard);

	return(err);
}

// Signer combo box
PGPError
pgpSigningPassphraseDialogPlatform(
	PGPContextRef						context,
	CPGPSigningPassphraseDialogOptions 	*options)
{
	PGPError err = 0;
	GPP	gpp;

	memset(&gpp,0x00,sizeof(GPP));
	gpp.context=context;
	gpp.options=options;

	/*
	 * XXX
	 * Currently expects to display only *ONE* key.
	 * The caller must set the keyset in the options to consist
	 * of the signing key that the caller wants this routine
	 * to verify
	 */
	
	// Initialize stuff
	if (!InitSigningKeyComboBox (options)) {
		return kPGPError_UserAbort; // kPGPError_Win32_NoSecret_Key
	}
	
	if (options->mPrompt)
		puts(options->mPrompt);
	else
		printf("Need a pass phrase to use this key\n");
	
	// Need to ask and get Passphrase
	

	// Ask for passphrase
	// put into CBreak mode for no echo
	// Need to get the size of the passphrase
	// Get first key out of keyset. Keyset is supposed to
	// only have one key XXX
	
	PGPKeyListRef	KeyList;
	PGPKeyIterRef	KeyIter;
	PGPKeyRef		key;
	PGPOrderKeySet (options->mKeySet,kPGPValidityOrdering,&KeyList);
	PGPNewKeyIter (KeyList, &KeyIter);
	PGPKeyIterNext (KeyIter, &key);

	while (1) {
		FreePassphrases(&gpp);
		gpp.pszPassPhrase = (char *)secAlloc (gpp.context, 500); // XXX 500
		if (gpp.pszPassPhrase) {
			PGPBoolean  bShared;
			
			PGPInt32 len = pgpTtyGetPass(stdout, gpp.pszPassPhrase, 500);
			if (len < 0) {
				err = kPGPError_UserAbort;
				goto end;
			}
			
			*(options->mPassphrasePtr) = gpp.pszPassPhrase;
			
			// Check Shared status
			err = PGPGetKeyBoolean(key, kPGPKeyPropIsSecretShared, &bShared);
			
			if (IsntPGPError(err) && bShared) {
				// So, they want to do a shared key
				*(options->mPassphraseKeyPtr) = key;
				err = kPGPError_KeyUnusableForSignature;
				goto end;
			}
			
			if (PassphraseLengthAndQualityOK(options,gpp.pszPassPhrase)) {
				if (!options->mVerifyPassphrase) {
					err = kPGPError_NoErr;
					goto end;
				}
				
				err = ValidateSigningPhrase(&gpp,gpp.pszPassPhrase,key);
				
				if (IsntPGPError(err)) {
					err = kPGPError_NoErr;
					goto end;
				}
			}
			
			// Bad passphrase/quality
			err = kPGPError_BadPassphrase;
		}
	}
	
	// Couldn't allocate passphrases
	err = kPGPError_OutOfMemory;
	
	
 end:
	PGPFreeKeyIter(KeyIter);
	PGPFreeKeyList(KeyList);
	ClearPassphrases(&gpp);
	if (err != kPGPError_NoErr)
		FreePassphrases(&gpp);
	return(err);
}

// Double edit window
	PGPError
pgpConfirmationPassphraseDialogPlatform(
	PGPContextRef								context,
	CPGPConfirmationPassphraseDialogOptions 	*options)
{
	PGPError err = 0;
	GPP	gpp;

	memset(&gpp,0x00,sizeof(GPP));
	gpp.context=context;
	gpp.options=options;

//	InitRandomKeyHook(&hhookMouse,&hhookKeyboard);
	
	//err = DialogBoxParam (g_hInst, 
	//	MAKEINTRESOURCE (IDD_PASSPHRASEENCRYPT),
	//	options->mHwndParent,
	//	(DLGPROC)pgpConfirmationDlgProc, (LPARAM)&gpp);
//
//	*(options->mPassphrasePtr)=gpp.pszPassPhrase;
//
//	UninitRandomKeyHook(hhookMouse,hhookKeyboard);

	return(err);
}

	PGPError
pgpKeyPassphraseDialogPlatform(
	PGPContextRef					context,
	CPGPKeyPassphraseDialogOptions 	*options)
{
	PGPError err;
	PGPBoolean bShared,bNeedsPhrase;
	GPP	gpp;

	memset(&gpp,0x00,sizeof(GPP));
	gpp.context=context;
	gpp.options=options;

	err=PGPGetKeyBoolean( options->mDefaultKey, kPGPKeyPropIsSecretShared,
				&bShared);
	
	if(IsntPGPError(err) && bShared)
	{
		// So, they want to do a shared key....
		return(kPGPError_KeyUnusableForSignature);
	}

	err=PGPGetKeyBoolean (options->mDefaultKey, kPGPKeyPropNeedsPassphrase,
				&bNeedsPhrase);

	if(IsntPGPError(err) && !bNeedsPhrase)
	{
		*(options->mPassphrasePtr)=(char *)secAlloc (context, 1);
		if(*(options->mPassphrasePtr)==0)
			return(kPGPError_OutOfMemory);

		// Doesn't need a passphrase
		strcpy(*(options->mPassphrasePtr),"");
		return(kPGPError_NoErr);
	}

//	InitRandomKeyHook(&hhookMouse,&hhookKeyboard);

	//err = DialogBoxParam (g_hInst, 
	//	MAKEINTRESOURCE (IDD_KEYPASSPHRASE), 
	//	options->mHwndParent,
	//	(DLGPROC)pgpKeyPassphraseDlgProc, (LPARAM)&gpp);
//
//	*(options->mPassphrasePtr)=gpp.pszPassPhrase;
//
//	UninitRandomKeyHook(hhookMouse,hhookKeyboard);

	return(err);
}


#if 0
BOOL CALLBACK 
pgpSigningPassphraseDlgProc (
        HWND    hDlg, 
        UINT    uMsg, 
        WPARAM  wParam,
        LPARAM  lParam) 
{
    CPGPSigningPassphraseDialogOptions *options;
    GPP             *gpp;
    INT             i;
    DWORD           Common;
 
    Common=DoCommonCalls (hDlg,uMsg,wParam,lParam); 
 
    if(Common)
        return Common;
 
    switch (uMsg) 
    {
        case WM_INITDIALOG:
        {
            gpp=(GPP *)GetWindowLong (hDlg, GWL_USERDATA);
            options = (CPGPSigningPassphraseDialogOptions *)
                gpp->options;
 
            gpp->iNextTabControl = IDOK;
 
            gpp->wpOrigPhrase1Proc = (WNDPROC) SetWindowLong(
                GetDlgItem(hDlg, IDC_PHRASE1), 
                GWL_WNDPROC, 
                (LONG) PhraseSubclassProc); 
 
            if (InitSigningKeyComboBox (hDlg, options)) 
            {
                EnablePassPhraseControl (hDlg);
            }
            else 
            {
                EndDialog (hDlg, kPGPError_UserAbort);
            }
 
            DisplayOptions(hDlg,options->mDialogOptions,120);
 
            return TRUE;
        }
 
        case WM_COMMAND:
        {
            gpp=(GPP *)GetWindowLong (hDlg, GWL_USERDATA);
            options = (CPGPSigningPassphraseDialogOptions *)
                gpp->options;
 
            switch (LOWORD (wParam)) 
            {
                case IDOK:
                {
                    int ComboSelection;
 
                    FreePassphrases(gpp);
					
                    ComboSelection = SendDlgItemMessage (hDlg,
                    					IDC_SIGNKEYCOMBO, CB_GETCURSEL, 0, 0);
					
                    if (ComboSelection == CB_ERR) 
                    {
                        PGPclMessageBox (hDlg, 
                            IDS_ERRORMESSAGECAPTION, IDS_NOSIGNKEY, 
                            MB_OK | MB_ICONEXCLAMATION);
                        break;
                    }
 
                    i = SendDlgItemMessage (hDlg, IDC_PHRASE1, 
                        WM_GETTEXTLENGTH, 0, 0) +1;
 
                    gpp->szDummy = (char *)secAlloc (gpp->context, i);
 
                    if(gpp->szDummy)
                    {
                        gpp->pszPassPhrase = (char *)secAlloc (gpp->context,
                        			i); 
                        if (gpp->pszPassPhrase) 
                        {
                            PGPKeyRef   key;
                            PGPError    err;
                            PGPBoolean  bShared;
 
                            GetDlgItemText (hDlg, IDC_PHRASE1, gpp->szDummy, i);
 
                            key = (PGPKeyRef)SendDlgItemMessage (hDlg, 
                                IDC_SIGNKEYCOMBO, CB_GETITEMDATA,
                                ComboSelection , 0);
 
                            err=PGPGetKeyBoolean( key,
                            		kPGPKeyPropIsSecretShared , &bShared);
    
                            if(IsntPGPError(err) && bShared)
                            {
                                // So, they want to do a shared key....
                                ClearPassphrases(hDlg,gpp);
                                FreePassphrases(gpp);
                                *(options->mPassphraseKeyPtr) = key;
                                EndDialog (hDlg,
                                		kPGPError_KeyUnusableForSignatu
re);
                                break;
                            }
 
                            if(PassphraseLengthAndQualityOK(options,
                            		gpp->pszPass
Phrase))
                            {
                                if (!options->mVerifyPassphrase)
                                {
                                    ClearPassphrases(hDlg,gpp);
                                    EndDialog (hDlg, kPGPError_NoErr);
                                    break;
                                }
 
                                err=ValidateSigningPhrase(hDlg,gpp,
                                			gpp->pszPassPhrase,key);
 
                                if(IsntPGPError(err))
                                {
                                    ClearPassphrases(hDlg,gpp);
                                    EndDialog (hDlg, kPGPError_NoErr);
                                    break;
                                }
                            }
 
                            // Bad passphrase/quality
                            ClearPassphrases(hDlg,gpp);
                            FreePassphrases(gpp);
                            break;
                        }
                    }
 
                    // Couldn't allocate passphrases
                    ClearPassphrases(hDlg,gpp);
                    FreePassphrases(gpp);
                    EndDialog (hDlg, kPGPError_OutOfMemory);
                    break;
                }
 
                case IDC_SIGNKEYCOMBO :
                {
                    if(HIWORD(wParam)==CBN_SELCHANGE)
                        EnablePassPhraseControl (hDlg);
                    break;
                }
            }
        }
        break;
    }
    return FALSE;
}
#endif
