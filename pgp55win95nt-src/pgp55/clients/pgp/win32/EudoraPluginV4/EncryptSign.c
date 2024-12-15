/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: EncryptSign.c,v 1.1 1997/11/05 16:23:32 elrod Exp $
____________________________________________________________________________*/

// System Headers
#include <windows.h> 
#include <windowsx.h>
#include <assert.h>

// PGPsdk Headers
#include "pgpConfig.h"
#include "pgpKeys.h"
#include "pgpErrors.h"
#include "pgpWerr.h"
#include "pgpUtilities.h"
#include "pgpVersionHeader.h"
#include "pgpSDKPrefs.h"


// Shared Headers
#include "PGPcmdlg.h"
#include "PGPkm.h"

// Project Headers
#include "PGPRecip.h"
#include "PGPphras.h"
#include "MyPrefs.h"
#include "DisplayMessage.h"  
#include "Working.h"
#include "resource.h"

// Global Variables
extern PGPContextRef	g_pgpContext;

PGPError EncodeEventHandler(PGPContextRef context,
							PGPEvent *event, 
							PGPUserValue userValue);


PGPError 
EncryptSign(HWND				hwndParent,		// Parent Window
			PGPKeySetRef*		keyset,			// main keyset
			PGPOptionListRef	pgpOptions,		// Encoding Options
			PGPOptionListRef	userOptions,	// Optional User Options Out
			char**				origRecipients,	// Array of email addresses
			DWORD				numRecipients,	// Length of Array
			BOOL				bEncrypt,		// Encrypt??
			BOOL				bSign			// Sign??
)
{
	PGPError			error			= kPGPError_NoErr;	
	PGPKeySetRef		selectedKeySet	= kPGPInvalidRef;
	PGPOptionListRef	encryptOptions	= kPGPInvalidRef;
	PGPOptionListRef	signOptions		= kPGPInvalidRef;
	char				szComment[256]	= {0x00};

	assert(pgpOptions);

	// refresh the prefs
	PGPsdkLoadDefaultPrefs(g_pgpContext);

	// Check for expiration
	if (PGPcomdlgEvalExpired(hwnd, PGPCOMDLG_ENCRYPTSIGNEXPIRED))
	{
		return kPGPError_Win32_Expired;
	}

	if (GetCommentString(szComment, sizeof(szComment)))
	{
		PGPAppendOptionList(g_pgpContext,
							pgpOptions,
							PGPOCommentString(g_pgpContext, szComment),
							PGPOLastOption(g_pgpContext));	
	}

	
	error = PGPNewOptionList( g_pgpContext, &encryptOptions);

	// are we encrypting?
	if( bEncrypt )
	{
		PRECIPIENTDIALOGSTRUCT prds = NULL;	
		
		// allocate a recipient dialog structure
		prds = (PRECIPIENTDIALOGSTRUCT)malloc(sizeof(RECIPIENTDIALOGSTRUCT));

		if(prds)
		{
			char szTitle[256]		= {0x00};	// title for recipient dialog
			UINT recipientReturn	= FALSE;	// recipient dialog result

			// zero out the structure
			memset(prds, 0x00, sizeof(RECIPIENTDIALOGSTRUCT) );

			LoadString(GetModuleHandle("PGPplugin.dll"), 
				IDS_TITLE_RECIPIENTDIALOG, szTitle, sizeof(szTitle));

			if( IsntPGPError(error) )
			{
				prds->Context			= g_pgpContext;
				prds->Version			= CurrentPGPrecipVersion;
				prds->hwndParent		= hwndParent;			
				prds->szTitle			= szTitle;
				prds->dwOptions			= PGPCOMDLG_ASCIIARMOR;	
				prds->OriginalKeySetRef	= *keyset;
				
				prds->dwDisableFlags	= PGPCOMDLG_DISABLE_WIPEORIG |
										  PGPCOMDLG_DISABLE_ASCIIARMOR;

				prds->dwNumRecipients	= numRecipients;	
				prds->szRecipientArray	= origRecipients;

				// See who we wish to encrypt this to
				recipientReturn = PGPRecipientDialog( prds );

				// may change if people use network to update keys
				*keyset = prds->OriginalKeySetRef;

				if( recipientReturn )
				{
					if( prds->dwOptions & PGPCOMDLG_ASCIIARMOR )
					{
						PGPAppendOptionList(g_pgpContext,
										encryptOptions,
										PGPOArmorOutput(g_pgpContext, TRUE),
										PGPOLastOption(g_pgpContext));
					}

					if( prds->dwOptions & PGPCOMDLG_PASSONLY )
					{
						char* passphrase = NULL;
						char szPrompt[] = "Please enter the passphrase for "
										  "encrypting.";

						error = PGPcomdlgGetPhrase (
										g_pgpContext,
										NULL,									 
										szPrompt,
										&passphrase, 
										NULL,
										NULL,
										0,
										NULL, 
										NULL,
										PGPCOMDLG_ENCRYPTION);


						if( IsntPGPError (error) )
						{
							PGPCipherAlgorithm  prefAlg;

							GetPreferredAlgorithm(&prefAlg);

							PGPAppendOptionList(g_pgpContext,
												encryptOptions,
												PGPOConventionalEncrypt(	
													g_pgpContext,
													PGPOPassphrase(	
															g_pgpContext, 
															passphrase),
												PGPOLastOption(g_pgpContext)),
											PGPOCipherAlgorithm(g_pgpContext, 
																prefAlg),
											PGPOLastOption(g_pgpContext));
						}

						if (passphrase)
						{
							PGPcomdlgFreePhrase(passphrase);
							passphrase = NULL;
						}
					}
					else
					{
						selectedKeySet = prds->SelectedKeySetRef;

						PGPAppendOptionList(g_pgpContext,
											encryptOptions,
											PGPOEncryptToKeySet(g_pgpContext, 
															selectedKeySet),
											PGPOLastOption(g_pgpContext));

					}
				}
				else
				{
					error = kPGPError_UserAbort;
				}
			}

			// clean up
			free(prds);

		}	// IF PRDS
	} // IF bEncrypt

	if( IsntPGPError(error) )
	{
		static BOOL bAlreadyAsked	= FALSE;

		do
		{ 
			char* passphrase = NULL;

			 error = PGPNewOptionList( g_pgpContext, &signOptions);

			if( bSign && IsntPGPError(error))
			{
				DWORD		passphraseReturn= 0;
				PGPKeyRef	key				= kPGPInvalidRef;
				UINT		options			= 0;
				char*		szPrompt		= "Please enter your passphrase.";

				/*if(bAlreadyAsked)
				{
					szPrompt = "Passphrase did not match. Please try again.";
				}*/

				error = PGPGetSignCachedPhrase(
									g_pgpContext,
									hwndParent, 
									szPrompt,
									bAlreadyAsked,
									&passphrase, 
									*keyset,
									&key, 
									NULL,
									NULL,
									0);

				// remember that we have asked once already
				bAlreadyAsked = TRUE;

				if( IsntPGPError(error) )
				{
					PGPAppendOptionList(g_pgpContext,
				 						signOptions,
										PGPOSignWithKey(
											g_pgpContext, 
											key,
											PGPOPassphrase( g_pgpContext,
															passphrase),
											PGPOLastOption(g_pgpContext)),
										PGPOLastOption(g_pgpContext));

					PGPFreePhrase( passphrase );
					passphrase = NULL;
				}
			} // IF bSign 

			if( IsntPGPError(error) )
			{
				HWND hwndWorking = NULL;

				// create progress dialog
				hwndWorking = WorkingDlgProcThread(	
					GetModuleHandle("PGPplugin.dll"), 
					NULL,
					(bSign
					? "PGP Encrypting and Signing":
					"PGP Encrypting"),
					"");

				error = PGPEncode(	g_pgpContext,
									encryptOptions,
									signOptions,
									(bSign ? PGPOClearSign(	g_pgpContext, 
									TRUE) :
										PGPONullOption(g_pgpContext)),
									pgpOptions,
									PGPOEventHandler( g_pgpContext,
										EncodeEventHandler,hwndWorking),
									PGPOVersionString(g_pgpContext, 
										pgpVersionHeaderString),
									PGPOSendNullEvents(g_pgpContext, 75),
									PGPOLastOption(g_pgpContext));

				// destroy progress dialog
				PostMessage(hwndWorking, WM_QUIT, 0, 0);
			}

			if(error != kPGPError_BadPassphrase)
			{
				if( PGPRefIsValid( userOptions ) )
				{	
					PGPAppendOptionList(g_pgpContext,
										userOptions,
										signOptions,
										PGPOLastOption(g_pgpContext));
				}
			}

			if( PGPRefIsValid( signOptions ) )
			{
				PGPFreeOptionList( signOptions );
			}

		}while ( error == kPGPError_BadPassphrase );

		bAlreadyAsked	= FALSE;
	}

	// clean up
	if( PGPRefIsValid(selectedKeySet) )
	{
		PGPFreeKeySet(selectedKeySet);
	}

	if( PGPRefIsValid( encryptOptions ) )
	{
		if( PGPRefIsValid( userOptions ) )
		{	
			PGPAppendOptionList(g_pgpContext,
								userOptions,
								encryptOptions,
								PGPOLastOption(g_pgpContext));
		}
 
		PGPFreeOptionList( encryptOptions );
	}

	return error;
}

PGPError EncodeEventHandler(PGPContextRef context,
							PGPEvent *event, 
							PGPUserValue userValue)
{
	PGPError error = kPGPError_NoErr;

	switch (event->type)
	{
		case kPGPEvent_NullEvent:
		{	
			PGPEventNullData *d = &event->data.nullData;
			BOOL bCancel;

			bCancel = WorkingCallback (	(HWND)userValue, 
										(unsigned long)d->bytesWritten, 
										(unsigned long)d->bytesTotal) ;

			if(bCancel)
			{
				return kPGPError_UserAbort;
			}
		}
		break;

		case kPGPEvent_FinalEvent:
		{
			break;
		}

	}


	return error;
}
