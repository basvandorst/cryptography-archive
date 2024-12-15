/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: Translators.c,v 1.25.2.3.2.1 1998/11/12 03:11:39 heller Exp $
____________________________________________________________________________*/

// System Headers
#include <windows.h>
#include <windowsx.h>
#include <assert.h>


// PGPsdk Headers
#include "pgpConfig.h"
#include "pgpKeys.h"
#include "pgpErrors.h"
#include "pgpUtilities.h"
#include "pgpTLS.h"
#include "pgpSDKPrefs.h"

// Eudora Headers
#include "emssdk/Mimetype.h"

// Shared Headers
#include "PGPcl.h"

// Project Headers
#include "PGPPlug.h"
#include "TranslatorIDs.h"
#include "EncryptSign.h"
#include "DecryptVerify.h"
#include "AddKey.h"
#include "TranslatorUtils.h"
#include "DisplayMessage.h"
#include "resource.h"
#include "MyMIMEUtils.h"
#include "Prefs.h"

// Global Variables
extern HINSTANCE		g_hinst;
extern PGPContextRef	g_pgpContext;
extern PGPtlsContextRef g_tlsContext;
extern HWND				g_hwndEudoraMainWindow;


PluginError CanTranslateMIMEType(long transContext,
								 emsMIMEtypeP mimeType,
								 const char* type, 
								 const char* subType);



PluginError CanPerformTranslation(
	short trans_id, 
	emsMIMEtypeP in_mime, 
	long context
)
{
	PluginError err = EMSR_CANT_TRANS;

	if ( in_mime )
	{			
		switch(trans_id)
		{
			// manual (aka On-Demand) translators
			case kManualEncryptSignTranslatorID:
			case kManualDecryptVerifyTranslatorID:
			{
				if( in_mime && match_mime_type(in_mime, "text", "plain") )
				{
					if ( ( context & EMSF_ON_REQUEST ) != 0 )
					{
						err = EMSR_NOW;
					}
				}
				break;
			}

			// Mime Translators
			case kEncryptTranslatorID:
			case kSignTranslatorID:
			case kEncryptAndSignTranslatorID:
			{
				err = EMSR_NOW;
				break;
			}

			case kDecryptTranslatorID:
			{
				err = CanTranslateMIMEType(	context, 
											in_mime, 
											"multipart", 
											"encrypted");

				break;
			}

			case kVerifyTranslatorID:
			{
				err = CanTranslateMIMEType(	context, 
											in_mime, 
											"multipart", 
											"signed");

				break;
			}
		}		
	}

	return err;
}

PluginError PerformTranslation(
	short trans_id, 
	char* in_file, 
	char* out_file, 
	char** addresses, 
	emsMIMEtypeP  in_mime,
	emsMIMEtypeP* out_mime
)
{
	PluginError pluginReturn			= EMSR_UNKNOWN_FAIL;
	BOOL		bSign					= FALSE;
	PGPError	error					= 0;
	PGPSize		mimeBodyOffset			= 0;
	char		szExe[256];
	char		szDll[256];

	assert(in_file);
	assert(out_file);

	LoadString(g_hinst, IDS_EXE, szExe, sizeof(szExe));
	LoadString(g_hinst, IDS_DLL, szDll, sizeof(szDll));

	switch( trans_id )
	{
		case kEncryptTranslatorID:
		case kSignTranslatorID:
		case kEncryptAndSignTranslatorID:
		{
			char** RecipientList = NULL;
			unsigned long numRecipients = 0;
			PGPOptionListRef pgpOptions = NULL;
			PGPOptionListRef signOptions = NULL;
			char mimeSeparator[kPGPMimeSeparatorSize];
			PRECIPIENTDIALOGSTRUCT prds = NULL;	
		
			// allocate a recipient dialog structure
			prds = (PRECIPIENTDIALOGSTRUCT) 
					calloc(sizeof(RECIPIENTDIALOGSTRUCT), 1);

			if(prds)
			{
				char szTitle[256] = {0x00};		// title for recipient dialog
				UINT recipientReturn = FALSE;	// recipient dialog result

				error = PGPsdkLoadDefaultPrefs(g_pgpContext);
				if (IsPGPError(error))
				{
					PGPclEncDecErrorBox(g_hwndEudoraMainWindow, error);
					return EMSR_UNKNOWN_FAIL;
				}

				error = PGPOpenDefaultKeyRings(g_pgpContext, 
							(PGPKeyRingOpenFlags)0, 
							&(prds->OriginalKeySetRef));

				if (IsPGPError(error))
				{
					PGPclEncDecErrorBox(g_hwndEudoraMainWindow, error);
					return EMSR_UNKNOWN_FAIL;
				}

				if ((trans_id == kEncryptTranslatorID) ||
					(trans_id == kEncryptAndSignTranslatorID))
				{
					if(addresses) // do we have addresses to pass along
					{
						numRecipients = CreateRecipientList(addresses, 
															&RecipientList);
					}

					LoadString(GetModuleHandle("PGPplugin.dll"), 
						IDS_TITLE_RECIPIENTDIALOG, szTitle, sizeof(szTitle));

					prds->Context			= g_pgpContext;
					prds->tlsContext		= g_tlsContext;
					prds->Version			= CurrentPGPrecipVersion;
					prds->hwndParent		= g_hwndEudoraMainWindow;			
					prds->szTitle			= szTitle;
					prds->dwOptions			= PGPCL_ASCIIARMOR;	
			
					prds->dwDisableFlags	= PGPCL_DISABLE_WIPEORIG |
											  PGPCL_DISABLE_ASCIIARMOR;

					prds->dwNumRecipients	= numRecipients;	
					prds->szRecipientArray	= RecipientList;

					// If shift is pressed, force the dialog to pop.
					if (GetAsyncKeyState( VK_CONTROL) & 0x8000)
						prds->dwDisableFlags|=PGPCL_DISABLE_AUTOMODE;

					// See who we wish to encrypt this to
					recipientReturn = PGPclRecipientDialog( prds );
				
					if (prds->AddedKeys != NULL)
					{
						PGPUInt32 numKeys;
					
						PGPCountKeys(prds->AddedKeys, &numKeys);
						if (numKeys > 0)
							PGPclQueryAddKeys(g_pgpContext, g_tlsContext, 
								g_hwndEudoraMainWindow, prds->AddedKeys, 
								NULL);
					
						PGPFreeKeySet(prds->AddedKeys);
						prds->AddedKeys = NULL;
					}

					if (!recipientReturn)
					{
						if (RecipientList)
							FreeRecipientList(RecipientList, numRecipients);
						if (prds->SelectedKeySetRef != NULL)
							PGPFreeKeySet(prds->SelectedKeySetRef);
						PGPFreeKeySet(prds->OriginalKeySetRef);
						free(prds);
						return EMSR_UNKNOWN_FAIL;
					}
				}

				if( IsntPGPError(error) )
				{
					error = PGPBuildOptionList(g_pgpContext, &pgpOptions,
								PGPOOutputLineEndType(g_pgpContext, 
									kPGPLineEnd_CRLF),
								PGPOPGPMIMEEncoding(g_pgpContext, 
									TRUE, 
									&mimeBodyOffset, 
									mimeSeparator),     
								PGPOLastOption(g_pgpContext) );
				}

				if(IsntPGPError( error))
				{
					error = EncryptSignFile(g_hinst, g_hwndEudoraMainWindow, 
								g_pgpContext, g_tlsContext, szExe, 
								szDll, in_file, prds, pgpOptions, 
								&signOptions, out_file, 
								((trans_id == 
									kSignTranslatorID) ? FALSE : TRUE ),
								((trans_id == 
									kEncryptTranslatorID) ? FALSE : TRUE ),
								FALSE);

					PGPFreeOptionList(pgpOptions);
					PGPFreeOptionList(signOptions);
				}
				else
				{
					PGPclEncDecErrorBox (NULL, error);
				}

				if (RecipientList)
					FreeRecipientList(RecipientList, numRecipients);
				if (prds->SelectedKeySetRef != NULL)
					PGPFreeKeySet(prds->SelectedKeySetRef);
				PGPFreeKeySet(prds->OriginalKeySetRef);
				free(prds);

				if( IsntPGPError(error) )
				{
					if( out_mime )
					{
						pluginReturn = BuildEncryptedPGPMIMEType( 
													out_mime, 
													mimeSeparator );

						if(EMSR_OK == pluginReturn)
						{
							pluginReturn = AddMIMEParam(*out_mime, 
														"PGPFormat", 
														"PGPMIME-encrypted" );
						}
					}
				}
			}

			break;
		}

		case kDecryptTranslatorID:
		case kVerifyTranslatorID:
		{
			char *szTempFile = NULL;
			BOOL bDummy;

			error = DecryptVerifyFile(g_hinst, g_hwndEudoraMainWindow, 
						g_pgpContext, g_tlsContext, szExe, szDll, in_file, 
						TRUE, FALSE, &szTempFile, NULL, NULL, &bDummy);

			if( IsntPGPError(error) ) 
			{
				CopyFile(szTempFile, out_file, FALSE);
				DeleteFile(szTempFile);

//				AddKeyFile(g_hwndEudoraMainWindow, g_pgpContext, 
//					g_tlsContext, out_file);

				if( out_mime )
				{
					ParseFileForMIMEType( out_file, out_mime );
				}

				pluginReturn = EMSR_OK;
			}

			if (szTempFile != NULL)
				PGPFreeData(szTempFile);
			break;
		}

	}

	return pluginReturn;
}


PluginError CanTranslateMIMEType(
	long					transContext,
	emsMIMEtypeP			mimeType,
	const char*				type,
	const char*				subType)
{
	PluginError	err	= EMSR_CANT_TRANS;
	
	if ( mimeType )
	{
		BOOL		haveValidContext;
		const long	kValidContextMask	= EMSF_ON_DISPLAY | EMSF_ON_ARRIVAL;
		
		haveValidContext	= ( transContext & kValidContextMask ) != 0;
		
		if ( haveValidContext )
		{
			BOOL	haveMatch;
			
			haveMatch	= match_mime_type( mimeType, type, subType );

			if( haveMatch )
			{
				const char* PGPMimeParam = NULL;

				PGPMimeParam = get_mime_parameter(mimeType, "protocol");

				if(PGPMimeParam && 
					(!strcmp(PGPMimeParam, "application/pgp-encrypted") || 
					!strcmp(PGPMimeParam, "application/pgp-signature") ) )
				{
					haveMatch = TRUE;
				}
				else
				{
					haveMatch = FALSE;
				}
			}
			
			if ( haveMatch )
			{
	 		    if( transContext == EMSF_ON_ARRIVAL )
	 		    {
		   	        err = EMSR_NOT_NOW;
		   	    }
		   	    else
		   	    {
		   		    err = EMSR_NOW;
		   		}
			}
		}
	}
	
	return( err );
}
