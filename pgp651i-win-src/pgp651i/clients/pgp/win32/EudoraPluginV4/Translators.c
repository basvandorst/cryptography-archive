/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: Translators.c,v 1.25.4.1 1999/06/07 16:48:59 dgal Exp $
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
#include "PGPsc.h"

// Project Headers
#include "PluginMain.h"
#include "EncryptSign.h"
#include "DecryptVerify.h"
#include "AddKey.h"
#include "TranslatorUtils.h"
#include "MyMIMEUtils.h"
#include "DisplayMessage.h"
#include "Prefs.h"
#include "resource.h"


// Global Variables
extern HINSTANCE		g_hinst;
extern PGPContextRef	g_pgpContext;
extern PGPtlsContextRef	g_tlsContext;


long CanTranslateMIMEType(	long transContext,
							emsMIMEtypeP mimeType,
							const char* type, 
							const char* subType);


long CanPerformTranslation(
	const long trans_id, 
	const emsMIMEtypeP in_mime, 
	const long context
)
{
	long err = EMSR_CANT_TRANS;

	if ( in_mime )
	{			
		switch(trans_id)
		{
			// manual (aka On-Demand) translators
			case kManualEncryptTranslatorID:
			case kManualSignTranslatorID:
			case kManualEncryptSignTranslatorID:
			case kManualDecryptVerifyTranslatorID:
			{
				if( in_mime && match_mime_type(	in_mime, 
												"text", 
												"plain") )
				{
					if( ( context & EMSF_ON_REQUEST ) != 0 )
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
				if( in_mime && match_mime_type(	in_mime, 
												"multipart", 
												"encrypted") )
				{
					err = CanTranslateMIMEType(	context, 
												in_mime, 
												"multipart", 
												"encrypted");
				}

				break;
			}

			case kVerifyTranslatorID:
			{
				if( in_mime && match_mime_type(	in_mime, 
												"multipart", 
												"signed") )
				{
					err = CanTranslateMIMEType(	context, 
												in_mime, 
												"multipart", 
												"signed");
				}

				break;
			}
		}		
	}

	return err;
}

long PerformTranslation(
	const long trans_id, 
	const char* in_file, 
	const char* out_file, 
	emsHeaderDataP header, 
	emsMIMEtypeP in_mime,
	emsMIMEtypeP* out_mime
)
{
	long		pluginReturn	= EMSR_UNKNOWN_FAIL;
	BOOL		bSign			= FALSE;
	PGPError	error			= kPGPError_NoErr;
	PGPSize		mimeBodyOffset	= 0;
	BOOL		bMIME			= FALSE;
	BOOL		bManual			= TRUE;
	char		szExe[256];
	char		szDll[256];
	char		temp_file[256];

	assert(in_file);
	assert(out_file);
	temp_file[0] = 0;

	LoadString(g_hinst, IDS_EXE, szExe, sizeof(szExe));
	LoadString(g_hinst, IDS_DLL, szDll, sizeof(szDll));

	switch( trans_id )
	{
		case kEncryptTranslatorID:
		case kSignTranslatorID:
		case kEncryptAndSignTranslatorID:
		{
			bManual = FALSE;

			if(UsePGPMimeForEncryption(PGPGetContextMemoryMgr(g_pgpContext))
				|| ( in_mime && !match_mime_type(in_mime, "text", "plain") ) 
			)
			{
				bMIME = TRUE;
			}
		}

		case kManualEncryptTranslatorID:
		case kManualSignTranslatorID:
		case kManualEncryptSignTranslatorID:
		{
			char** addresses = NULL;
			unsigned long numRecipients = 0;
			PGPOptionListRef pgpOptions = NULL;
			PGPOptionListRef signOptions = NULL;
			char mimeSeparator[kPGPMimeSeparatorSize];
			BOOL bEncrypt = FALSE;
			BOOL bSign = FALSE;
			PRECIPIENTDIALOGSTRUCT prds = NULL;	

			if(bManual)
			{
				long lWrapWidth = 0;

				bEncrypt = ((trans_id == kManualSignTranslatorID) 
								? FALSE : TRUE );

				bSign = ((trans_id == kManualEncryptTranslatorID) 
								? FALSE : TRUE );

				if (ByDefaultWordWrap(PGPGetContextMemoryMgr(g_pgpContext), 
						&lWrapWidth))
				{
					HANDLE hFile = NULL;
					DWORD dwBytes;
					DWORD dwLength;
					char *szFileData = NULL;
					char *szWrapped = NULL;

					hFile = CreateFile(in_file, GENERIC_READ, 0, NULL, 
								OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

					dwLength = GetFileSize(hFile, NULL);

					szFileData = (char *) calloc(sizeof(char), dwLength+1);

					ReadFile(hFile, szFileData, dwLength, &dwBytes, NULL);
					CloseHandle(hFile);
					szFileData[dwLength] = 0;

					if (WrapBuffer(&szWrapped, szFileData, (short)lWrapWidth))
					{
						strcpy(temp_file, in_file);
						dwBytes = strlen(temp_file);
						temp_file[dwBytes-5]++;
						temp_file[dwBytes-6]++;
						temp_file[dwBytes-7]++;

						hFile = CreateFile(temp_file, GENERIC_WRITE, 0, NULL, 
									CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 
									NULL);

						WriteFile(hFile, szWrapped, strlen(szWrapped),
							&dwBytes, NULL);
						CloseHandle(hFile);

						in_file = temp_file;
					}

					free(szFileData);
					free(szWrapped);
				}
			}
			else
			{
				bEncrypt = ((trans_id == kSignTranslatorID) ? FALSE : TRUE );
				bSign = ((trans_id == kEncryptTranslatorID) ? FALSE : TRUE );

				if(!bMIME)
				{
					// want it to look clean upon decryption
					// we need only the text of the message
					StripMIMEHeader(in_file);
				}
			}			

			// allocate a recipient dialog structure
			prds = (PRECIPIENTDIALOGSTRUCT) 
					calloc(sizeof(RECIPIENTDIALOGSTRUCT), 1);

			if(prds)
			{
				char szTitle[256] = {0x00};		// title for recipient dialog
				UINT recipientReturn = FALSE;	// recipient dialog result
				PGPKeySetRef	keyset	= kPGPInvalidRef;

				error = PGPsdkLoadDefaultPrefs(g_pgpContext);
				if (IsPGPError(error))
				{
					PGPclEncDecErrorBox(NULL, error);
					return EMSR_UNKNOWN_FAIL;
				}

				error = PGPOpenDefaultKeyRings(g_pgpContext, 
							(PGPKeyRingOpenFlags)0, 
							&(prds->OriginalKeySetRef));

				if (IsPGPError(error))
				{
					PGPclEncDecErrorBox(NULL, error);
					return EMSR_UNKNOWN_FAIL;
				}

				if (bEncrypt)
				{
					// do we have the header structure?
					// should we autofind recipients?
					if(header) 
					{
						numRecipients = ExtractAddressesFromMailHeader(	
													header,
													&addresses);
					}

					LoadString(GetModuleHandle("PGPplugin.dll"), 
						IDS_TITLE_RECIPIENTDIALOG, szTitle, sizeof(szTitle));

					prds->Context			= g_pgpContext;
					prds->tlsContext		= g_tlsContext;
					prds->Version			= CurrentPGPrecipVersion;
					prds->hwndParent		= NULL;			
					prds->szTitle			= szTitle;
					prds->dwOptions			= PGPCL_ASCIIARMOR;	
			
					prds->dwDisableFlags	= PGPCL_DISABLE_WIPEORIG |
											  PGPCL_DISABLE_ASCIIARMOR |
											  PGPCL_DISABLE_SDA;

					prds->dwNumRecipients	= numRecipients;	
					prds->szRecipientArray	= addresses;

					// If shift is pressed, force the dialog to pop.
					if (GetAsyncKeyState( VK_CONTROL ) & 0x8000)
						prds->dwDisableFlags|=PGPCL_DISABLE_AUTOMODE;

					// See who we wish to encrypt this to
					recipientReturn = PGPclRecipientDialog( prds );
				
					if (prds->AddedKeys != NULL)
					{
						PGPUInt32 numKeys;
						
						PGPCountKeys(prds->AddedKeys, &numKeys);
						if (numKeys > 0)
							PGPclQueryAddKeys(g_pgpContext, g_tlsContext, 
								NULL, prds->AddedKeys, 
								NULL);
					
						PGPFreeKeySet(prds->AddedKeys);
						prds->AddedKeys = NULL;
					}

					if (!recipientReturn)
					{
						if (addresses)
							FreeRecipientList(addresses, numRecipients);
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
								(bMIME  ? 
									PGPOPGPMIMEEncoding(g_pgpContext, 
										TRUE, 
										&mimeBodyOffset, 
										mimeSeparator) : 
									PGPONullOption(g_pgpContext) ),     
								PGPOLastOption(g_pgpContext) );
				}

				if(IsntPGPError( error))
				{
					error = EncryptSignFile(g_hinst, NULL, 
								g_pgpContext, g_tlsContext, szExe, 
								szDll, (char *) in_file, prds, 
								pgpOptions, &signOptions, (char *) out_file, 
								bEncrypt, bSign, FALSE); 

					PGPFreeOptionList(pgpOptions);
					PGPFreeOptionList(signOptions);
				}
				else
				{
					PGPclEncDecErrorBox (NULL, error);
				}

				if (addresses)
					FreeRecipientList(addresses, numRecipients);
				if (prds->SelectedKeySetRef != NULL)
					PGPFreeKeySet(prds->SelectedKeySetRef);
				PGPFreeKeySet(prds->OriginalKeySetRef);
				free(prds);

				if (in_file == temp_file)
					DeleteFile(temp_file);

				if( IsntPGPError(error) )
				{
					if( out_mime )
					{
						if(bMIME)
						{
							*out_mime = make_mime_type(	"multipart",
														"encrypted", 
														"1.0");

							if( *out_mime )
							{
								pluginReturn = EMSR_OK;

								add_mime_parameter(	*out_mime, 
													"boundary", 
													mimeSeparator);

								add_mime_parameter(	*out_mime, 
													"PGPFormat", 
													"PGPMIME-encrypted");
							}
						}
						else
						{
							if( *out_mime )
							{
								pluginReturn = EMSR_OK;

								*out_mime = make_mime_type(	"text",
															"plain", 
															"1.0");
							}
						}
					}
				}
			}

			break;
		}

		case kDecryptTranslatorID:
		case kVerifyTranslatorID:
		{
			bManual = FALSE;
		}
		case kManualDecryptVerifyTranslatorID:

		{
			char *szTempFile = NULL;
			BOOL bFYEO = FALSE;
			void *pOutput = NULL;
			PGPSize outSize = 0;

			error = DecryptVerifyFile(g_hinst, NULL, g_pgpContext, 
						g_tlsContext, szExe, szDll, (char *) in_file, 
						!bManual, FALSE, &szTempFile, &pOutput, &outSize,
						&bFYEO);

			if( IsntPGPError(error) ) 
			{
				if ((bFYEO)||(GetSecureViewerPref((void *)g_pgpContext)))
					TempestViewer((void *)g_pgpContext,NULL,pOutput,outSize,
						bFYEO);

				CopyFile(szTempFile, out_file, FALSE);
				DeleteFile(szTempFile);

				if( out_mime )
				{
					ParseFileForMIMEType( out_file, out_mime );
				}

				if ((bFYEO)||(GetSecureViewerPref((void *)g_pgpContext)))
					pluginReturn = EMSR_UNKNOWN_FAIL;
				else
					pluginReturn = EMSR_OK;
			}

			if (szTempFile != NULL)
				PGPFreeData(szTempFile);
			if (pOutput != NULL)
				PGPFreeData(pOutput);

			break;
		}

	}

	return pluginReturn;
}


long CanTranslateMIMEType(
	long					transContext,
	emsMIMEtypeP			mimeType,
	const char*				type,
	const char*				subType)
{
	long	err	= EMSR_CANT_TRANS;
	
	if ( mimeType )
	{
		BOOL		haveValidContext;
		const long	kValidContextMask = EMSF_ON_DISPLAY | EMSF_ON_ARRIVAL;
		
		haveValidContext	= ( transContext & kValidContextMask ) != 0;
		
		if ( haveValidContext )
		{
			BOOL haveMatch;
			
			haveMatch = match_mime_type( mimeType, type, subType );

			if( haveMatch )
			{
				const char* PGPMimeParam = NULL;

				PGPMimeParam = get_mime_parameter(	mimeType, 
													"protocol");

				if(	PGPMimeParam && 
					(!strcmp(PGPMimeParam, "application/pgp-encrypted") || 
					!strcmp(PGPMimeParam, "application/pgp-signature") ) 
				)
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
