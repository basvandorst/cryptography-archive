/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: Translators.c,v 1.17 1997/10/22 23:06:00 elrod Exp $
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

// Eudora Headers
#include "emssdk/Mimetype.h"

// Shared Headers
#include "PGPcmdlg.h"
#include "PGPkm.h"

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
#include "MyPrefs.h"

// Global Variables
extern HINSTANCE		g_hinst;
extern PGPContextRef	g_pgpContext;
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
			case kManualAddKeyTranslatorID:
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

	assert(in_file);
	assert(out_file);

	switch( trans_id )
	{
		case kEncryptTranslatorID:
		case kSignTranslatorID:
		case kEncryptAndSignTranslatorID:
		{
			char** RecipientList = NULL;
			unsigned long numRecipients = 0;
			PGPOptionListRef pgpOptions;
			PGPFileSpecRef inFileRef;
			PGPFileSpecRef outFileRef;
			char mimeSeparator[kPGPMimeSeparatorSize];

			error = PGPNewFileSpecFromFullPath(	g_pgpContext,
												in_file, 
												&inFileRef);
			if(IsPGPError( error))
			{
				return pluginReturn;
			}

			error = PGPNewFileSpecFromFullPath(	g_pgpContext,
												out_file, 								
												&outFileRef);

			if(IsPGPError( error))
			{
				PGPFreeFileSpec(inFileRef);
				return pluginReturn;
			}

			if(IsntPGPError( error))
			{
				PGPKeySetRef	keyset	= kPGPInvalidRef;

				if(addresses) // do we have addresses to pass along
				{
					numRecipients = CreateRecipientList(addresses, 
														&RecipientList);
				}

				error = PGPOpenDefaultKeyRings( g_pgpContext, 0, &keyset );

				if( IsntPGPError(error) )
				{
					error = PGPBuildOptionList(g_pgpContext, &pgpOptions,
								PGPOInputFile(g_pgpContext, inFileRef),
								PGPOOutputFile(g_pgpContext, outFileRef),
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
					error = EncryptSign(
						g_hwndEudoraMainWindow,	// Parent Window
						&keyset,				// main keyset
						pgpOptions,				// Encoding Options
						kPGPInvalidRef,			// Optional User Options Out
						RecipientList,			// Array of email addresses
						numRecipients,			// Length of Array
						((trans_id == 
							kSignTranslatorID) ? FALSE : TRUE ),// Encrypt??
						((trans_id == 
							kEncryptTranslatorID) ? FALSE : TRUE ) );// Sign??

					if( PGPRefIsValid(keyset) )
					{
						PGPFreeKeySet(keyset);
					}

					PGPFreeOptionList(pgpOptions);
				}

				FreeRecipientList(RecipientList, numRecipients);

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
				else
				{
					PGPcomdlgErrorMessage (error);
				}

				PGPFreeFileSpec(inFileRef);
				PGPFreeFileSpec(outFileRef);
			}

			break;
		}

		case kDecryptTranslatorID:
		case kVerifyTranslatorID:
		{
			PGPOptionListRef pgpOptions;
			PGPFileSpecRef inFileRef, outFileRef;

			error = PGPNewFileSpecFromFullPath(	g_pgpContext,
												in_file, 
												&inFileRef);
			if(IsPGPError(error))
			{
				return pluginReturn;
			}

			error = PGPNewFileSpecFromFullPath(	g_pgpContext,
												out_file, 								
												&outFileRef);

			if(IsPGPError(error))
			{
				PGPFreeFileSpec(inFileRef);
				return pluginReturn;
			}

			if(IsntPGPError(error))
			{
				error = PGPBuildOptionList(g_pgpContext, &pgpOptions,
								PGPOInputFile(g_pgpContext, inFileRef),
								PGPOOutputFile(g_pgpContext, outFileRef),
								PGPOOutputLineEndType(g_pgpContext, 
									kPGPLineEnd_CRLF),
								PGPOLastOption(g_pgpContext) );

				if(IsntPGPError(error))
				{
					error = DecryptVerify(	NULL,		// Parent Window
											pgpOptions);// Encoding Options
								
					PGPFreeOptionList(pgpOptions);
				}

				if( IsntPGPError(error) ) 
				{
					AddKey(out_file, MAX_PATH, KEY_IN_FILE);

					if( out_mime )
					{
						ParseFileForMIMEType( out_file, out_mime );
					}

					pluginReturn = EMSR_OK;
				}
				else
				{
					PGPcomdlgErrorMessage (error);
				}

				PGPFreeFileSpec(inFileRef);
				PGPFreeFileSpec(outFileRef);
			}
			break;
		}

		case kManualAddKeyTranslatorID:
		{
			error = AddKey(in_file, MAX_PATH, KEY_IN_FILE);

			if( IsntPGPError(error)  && 
				CopyFile(in_file, out_file, FALSE) ) 
			{
				DisplayMessage(	NULL, 
								IDS_SUCCESS_ADDKEY, 
								IDS_TITLE_ADDKEY, 
								MB_OK);
				pluginReturn = EMSR_OK;
			}
			else
			{
				PGPcomdlgErrorMessage (error);
			}
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
