/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: Translators.c,v 1.1.2.4 1997/11/22 01:08:09 elrod Exp $
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
#include "PluginMain.h"
#include "EncryptSign.h"
#include "DecryptVerify.h"
#include "AddKey.h"
#include "TranslatorUtils.h"
#include "MyMIMEUtils.h"
#include "DisplayMessage.h"
#include "MyPrefs.h"
#include "resource.h"


// Global Variables
extern HINSTANCE		g_hinst;
extern PGPContextRef	g_pgpContext;


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
			case kManualAddKeyTranslatorID:
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

	assert(in_file);
	assert(out_file);

	switch( trans_id )
	{
		case kEncryptTranslatorID:
		case kSignTranslatorID:
		case kEncryptAndSignTranslatorID:
		{
			bManual = FALSE;

			if(UsePGPMimeForEncryption() ||
				( in_mime && 
				!match_mime_type(in_mime, 
								"text", 
								"plain") ) 
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
			PGPOptionListRef pgpOptions;
			PGPFileSpecRef inFileRef;
			PGPFileSpecRef outFileRef;
			char mimeSeparator[kPGPMimeSeparatorSize];
			BOOL bEncrypt = FALSE;
			BOOL bSign = FALSE;

			if(bManual)
			{
				long wrapLength;

				bEncrypt = ((trans_id == kManualSignTranslatorID) 
								? FALSE : TRUE );

				bSign = ((trans_id == kManualEncryptTranslatorID) 
								? FALSE : TRUE );


				if(ByDefaultWordWrap(&wrapLength))
				{
					WrapFile(in_file, wrapLength);
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

				// do we have the header structure?
				// should we autofind recipients?
				if(header) 
				{
					numRecipients = ExtractAddressesFromMailHeader(	
												header,
												&addresses);
				}

				error = PGPOpenDefaultKeyRings( g_pgpContext, 0, &keyset );

				if( IsntPGPError(error) )
				{
					error = PGPBuildOptionList(g_pgpContext, &pgpOptions,
								PGPOInputFile(g_pgpContext, inFileRef),
								PGPOOutputFile(g_pgpContext, outFileRef),
								PGPOOutputLineEndType(g_pgpContext, 
									kPGPLineEnd_CRLF),
								(bMIME  ? 
									PGPOPGPMIMEEncoding(g_pgpContext, 
										TRUE, 
										&mimeBodyOffset, 
										mimeSeparator) : 
									PGPODataIsASCII(g_pgpContext,
													TRUE) ),        
								PGPOLastOption(g_pgpContext) );
				}

				if(IsntPGPError( error))
				{
					error = EncryptSign(
								NULL,			// Parent Window
								&keyset,		// main keyset
								pgpOptions,		// Encoding Options
								kPGPInvalidRef, // Optional User Options Out
								addresses,		// Array of addresses
								numRecipients,	// Length of Array
								bEncrypt,		// Encrypt??
								bSign );		// Sign??


					if( PGPRefIsValid(keyset) )
					{
						PGPFreeKeySet(keyset);
					}

					PGPFreeOptionList(pgpOptions);
				}

				if(addresses)
				{
					FreeRecipientList(addresses, numRecipients);
				}

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
			bManual = FALSE;
		}
		case kManualDecryptVerifyTranslatorID:

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
								(bManual  ? 
									PGPOPassThroughIfUnrecognized(
										g_pgpContext,
										TRUE):
									PGPONullOption(g_pgpContext) ),     
								PGPOOutputLineEndType(g_pgpContext, 
									kPGPLineEnd_CRLF),
								PGPOLastOption(g_pgpContext) );

				if(IsntPGPError(error))
				{
					error = DecryptVerify(	NULL,	// Parent Window
											pgpOptions);// Encoding Options
								
					PGPFreeOptionList(pgpOptions);
				}

				if( IsntPGPError(error) ) 
				{
					//AddKey((char*)out_file, MAX_PATH, KEY_IN_FILE);

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
			error = AddKey((char*)in_file, MAX_PATH, KEY_IN_FILE);

			if( IsntPGPError(error)  && 
				CopyFile(in_file, out_file, FALSE) ) 
			{
				pluginReturn = EMSR_DATA_UNCHANGED;
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
