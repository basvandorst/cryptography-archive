/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#include <Finder.h>

#include <TArray.h>
#include <UDesktop.h>

#include "MacDialogs.h"
#include "MacErrors.h"
#include "MacFiles.h"
#include "MacStrings.h"
#include "pgpEncode.h"
#include "pgpMem.h"
#include "pgpUtilities.h"

#include "PGPSharedEncryptDecrypt.h"
#include "PGPtoolsEncryptDecrypt.h"
#include "PGPtoolsResources.h"
#include "PGPtoolsUtils.h"

const ResID		kEncryptGetFileOrClipboardDialogResID 	= 1000;
const ResID		kDecryptGetFileOrClipboardDialogResID 	= 1001;
const ResID		kPromptStandardGetFileDialogResID 		= 1002;

typedef struct SFInfo
{
	StandardFileReply	sfReply;
	Boolean				useClipboard;
	Boolean				haveClipboardButton;
	Boolean				allowClipboard;
	Boolean				showAllFiles;
	ConstStr255Param	openButtonTitle;
	
	// Used by the callback procs:
	Boolean				lastSelectedWasFolder;
	Str255				folderButtonTitle;
						
} SFInfo;

enum
{
	kClipborardButtonItem	= 10,
	kShowAllFilesCheckboxItem
};

enum
{
	kPromptStaticTextItem = 10
};

	static pascal short
GetFileOrClipboardHook(short item, DialogPtr dialog, SFInfo *sfInfo)
{
	pgpAssertAddrValid( dialog, DialogRecord );
	pgpAssertAddrValid( sfInfo, SFInfo );

	if( GetWRefCon( dialog ) == sfMainDialogRefCon )
	{
		switch( item )
		{
			case sfHookFirstCall:
			{
				if( sfInfo->haveClipboardButton && ! sfInfo->allowClipboard )
				{
					short 	itemType;
					Handle	itemHandle;
					Rect	box;
		
					::GetDialogItem(dialog,
									kClipborardButtonItem,
									&itemType,
									&itemHandle,
									&box	);
					::HiliteControl( (ControlHandle) itemHandle, 255 );
					
				}
				
				GetIndDialogItemText( dialog, sfItemOpenButton,
							sfInfo->folderButtonTitle );
				
				sfInfo->lastSelectedWasFolder = TRUE;
				
				break;
			}
			
			case kClipborardButtonItem:
			{
				sfInfo->useClipboard = true;
				
				// Switch the item to simulate the Cancel button
				item = sfItemCancelButton;
				break;
			}
			
			case kShowAllFilesCheckboxItem:
			{			
				
				sfInfo->showAllFiles = ToggleDialogCheckbox( dialog,
							kShowAllFilesCheckboxItem );
				item = sfHookRebuildList;
				break;
			}
			
			case sfHookNullEvent:
			{
				if( IsntNull( sfInfo->openButtonTitle ) )
				{
					if( sfInfo->sfReply.sfIsFolder ||
						sfInfo->sfReply.sfIsVolume )
					{
						if( ! sfInfo->lastSelectedWasFolder )
						{
							sfInfo->lastSelectedWasFolder = TRUE;
							SetIndDialogItemText( dialog, sfItemOpenButton,
									sfInfo->folderButtonTitle );
						}
					}
					else
					{
						if( sfInfo->lastSelectedWasFolder )
						{
							sfInfo->lastSelectedWasFolder = FALSE;
							SetIndDialogItemText( dialog, sfItemOpenButton,
										sfInfo->openButtonTitle );
						}
					}
				}
				
				break;
			}	
		}
	}
	
	return item;
}

	static pascal Boolean
VisibleItemFileFilterProc(CInfoPBPtr cpb)
{
	Boolean	shouldFilter = FALSE;
	
	pgpAssertAddrValid( cpb, CInfoPBRec);

	if( cpbIsFile( cpb ) )
	{
		if( ( cpb->hFileInfo.ioFlFndrInfo.fdFlags & kIsInvisible ) != 0 )
		{
			shouldFilter = TRUE;
		}
	}
	else
	{
		if( ( cpb->dirInfo.ioDrUsrWds.frFlags & kIsInvisible ) != 0 )
		{
			shouldFilter = TRUE;
		}
	}
	
	return( shouldFilter );
}

	static pascal Boolean
GetFileOrClipboardFilter(CInfoPBPtr cpb, const SFInfo *sfInfo)
{
	Boolean	shouldFilter = FALSE;
	
	shouldFilter = VisibleItemFileFilterProc( cpb );
	if( ! shouldFilter )
	{
		if( cpbIsFile( cpb ) && ! sfInfo->showAllFiles )
		{
			// Show only the files possibly created by PGP or
			// compatible products
			if( cpbFileType( cpb ) != 'TEXT' &&
				cpbFileType( cpb ) != kPGPMacFileTypeEncryptedData &&
				cpbFileType( cpb ) != kPGPMacFileTypeSignedData &&
				cpbFileType( cpb ) != kPGPMacFileTypeDetachedSig &&
				cpbFileType( cpb ) != kPGPMacFileTypeMacPGPEncryptedData )
			{
				shouldFilter = TRUE;
			}
		}
	}
		
	return( shouldFilter );
}

// Display SFGetFile dialog to select a file or choose a "Use Clipboard"
// button.

	OSStatus
GetFileOrClipboard(
	PGPtoolsOperation	operation,
	ConstStr255Param	openButtonTitle,
	FSSpec 				*fsSpec,
	Boolean 			*useClipboard)
{
	SFInfo				sfInfo;
	SFTypeList			typeList;
	DlgHookYDUPP		cbHook;
	FileFilterYDUPP		filterUPP;
	static Point		where={0,0};
	long				scrapOffset;
	OSStatus			status = noErr;
	short				dialogResID;
	Boolean				isEncryptSign;
	
	pgpAssertAddrValid( fsSpec, FSSpec );
	pgpAssertAddrValid( useClipboard, Boolean );
	
	pgpClearMemory( &sfInfo, sizeof( sfInfo ) );
	
	
	isEncryptSign	= ( operation == kPGPtoolsEncryptOperation ||
			 			operation == kPGPtoolsSignOperation ||
						 operation == kPGPtoolsEncryptSignOperation );
			 
	sfInfo.allowClipboard		= GetScrap( NULL, 'TEXT', &scrapOffset ) > 0;
	sfInfo.showAllFiles			= TRUE;
	sfInfo.haveClipboardButton	= TRUE;
	sfInfo.useClipboard 		= FALSE;
	sfInfo.openButtonTitle		= openButtonTitle;
	
	
	if ( operation == kPGPtoolsDecryptVerifyOperation )
	{
		sfInfo.showAllFiles	= FALSE;
	}
	else if ( operation == kPGPtoolsWipeOperation )
	{
		sfInfo.haveClipboardButton = FALSE;
	}
		
	cbHook 		= NewDlgHookYDProc( GetFileOrClipboardHook );
	filterUPP	= NewFileFilterYDProc( GetFileOrClipboardFilter );
	
	if( operation == kPGPtoolsWipeOperation )
	{
		dialogResID = sfGetDialogID;
	}
	else if( isEncryptSign )
	{
		dialogResID = kEncryptGetFileOrClipboardDialogResID;
	}
	else
	{
		dialogResID = kDecryptGetFileOrClipboardDialogResID;
	}
			
	UDesktop::Deactivate();
	
	::CustomGetFile(	filterUPP,
						-1,
						typeList,
						&sfInfo.sfReply,
						dialogResID,
						where,
						cbHook,
						NULL,
						NULL,
						NULL,
						&sfInfo);
						
	UDesktop::Activate();
	
	DisposeRoutineDescriptor( cbHook );
	DisposeRoutineDescriptor( filterUPP );
	
	if( sfInfo.sfReply.sfGood || sfInfo.useClipboard )
	{
		*fsSpec 		= sfInfo.sfReply.sfFile;
		*useClipboard 	= sfInfo.useClipboard;
	}
	else
	{
		status = userCanceledErr;
	}

	return( status );
}

//	Gets the error string for the error err assuming a phrasing such as
// 	Couldn't do X because Y, where Y is the error string.

	void
GetErrorString(CToolsError err, StringPtr errorStr)
{
	Str255	pStr;
	char	cStr[ sizeof( pStr ) ];
		
	pgpAssertAddrValid( errorStr, uchar );
	pgpAssert( err.IsError() && ! err.IsCancelError() );
	
	errorStr[0] = 0;
	
	pgpAssert( err.IsError() && (! err.IsCancelError() ) );

	GetIndString( errorStr, kErrorStringListResID,
				kAnErrorOccurredStrIndex );
	
	if ( err.HavePGPError() )
	{
		PGPGetErrorString( err.pgpErr, sizeof( cStr ), cStr );
		CToPString( cStr, pStr );
	}
	else if ( err.err != noErr )
	{
		GetOSErrorString( err.err, pStr );
	}
	else
	{
		pgpDebugMsg( "impossible case" );
	}
		
	PrintPString( errorStr, errorStr, pStr );
}

	Boolean
ShouldReportError(CToolsError err)
{
	if ( err.IsError() )
	{
		if ( ! err.IsCancelError() )
		{
			return( TRUE );
		}
	}
	return( FALSE );
}




typedef struct PromptInfo
{
	ConstStr255Param	prompt;
	FileFilterUPP		fileFilter;

} PromptInfo;

	static pascal short
StandardGetFileWithPromptHook(
	short 				item,
	DialogPtr 			dialog,
	const PromptInfo 	*promptInfo)
{
	pgpAssertAddrValid( dialog, DialogRecord );
	pgpAssertAddrValid( promptInfo, PromptInfo );
	
	if( GetWRefCon( dialog ) == sfMainDialogRefCon )
	{
		switch( item )
		{
			case sfHookFirstCall:
				SetIndDialogItemText( dialog, kPromptStaticTextItem,
							promptInfo->prompt );
				break;
		}
	}
	
	return item;
}

	static pascal Boolean
StandardGetFileWithPromptFilter(CInfoPBPtr cpb, const PromptInfo *promptInfo)
{
	Boolean	shouldFilter = FALSE;
	
	pgpAssertAddrValid( cpb, CInfoPBRec );
	pgpAssertAddrValid( promptInfo, PromptInfo );
	
	if( IsntNull( promptInfo->fileFilter ) )
	{
		shouldFilter = CallFileFilterProc( promptInfo->fileFilter, cpb );
	}
	
	return( shouldFilter );
}


	static void
StandardGetFileWithPrompt(
	FileFilterUPP 		fileFilter,
	short 				numTypes,
	ConstSFTypeListPtr 	typeList,
	ConstStr255Param	prompt,
	StandardFileReply 	*reply)
{
	DlgHookYDUPP		hookUPP;
	FileFilterYDUPP		filterUPP;
	static Point		where = {0,0};
	PromptInfo			promptInfo;
	
	pgpAssertAddrValid( typeList, OSType );
	pgpAssertAddrValid( prompt, uchar );
	pgpAssertAddrValid( reply, StandardFileReply );

	hookUPP 	= NewDlgHookYDProc( StandardGetFileWithPromptHook );
	filterUPP	= NewFileFilterYDProc( StandardGetFileWithPromptFilter );

	promptInfo.prompt		= prompt;
	promptInfo.fileFilter	= fileFilter;
	
	UDesktop::Deactivate();
	
	CustomGetFile( filterUPP, numTypes, typeList, reply,
				kPromptStandardGetFileDialogResID,
				where, hookUPP, NULL,NULL, NULL, &promptInfo );
						
	UDesktop::Activate();
	
	DisposeRoutineDescriptor( hookUPP );
	DisposeRoutineDescriptor( filterUPP );
}

	OSStatus
PromptForSignatureSourceFile(
	const FSSpec 	*sigFileSpec,
	FSSpec 			*sourceFileSpec)
{
	FileFilterUPP		filterUPP;
	OSStatus			status = noErr;
	StandardFileReply	sfReply;
	SFTypeList			typeList;
	Str255				prompt;
	
	AssertSpecIsValid( sigFileSpec, "PromptForSignatureSourceFile" );
	pgpAssertAddrValid( sourceFileSpec, FSSpec );
	
	filterUPP = NewFileFilterProc( VisibleItemFileFilterProc );
	
	GetIndString( prompt, kDialogStringsListResID,
				kLocateDetachedSigSourceFilePromptStrIndex );
	PrintPString( prompt, prompt, sigFileSpec->name );

	// Set Standard File to point to the signature file location.
	LMSetSFSaveDisk( -sigFileSpec->vRefNum );
	LMSetCurDirStore( sigFileSpec->parID );

	StandardGetFileWithPrompt( filterUPP, -1, typeList, prompt, &sfReply );
	
	if( sfReply.sfGood )
	{
		*sourceFileSpec = sfReply.sfFile;
	}
	else
	{
		status = userCanceledErr;
	}
		
	DisposeRoutineDescriptor( filterUPP );
	
	return( status );
}

	Boolean
HaveDirWriteAccess(short vRefNum, long dirID)
{
	HParamBlockRec	pb;
	Str255			tempStr;
	Boolean			haveWriteAccess = TRUE;	// Optimistic result
	
	// Check to see if the volume is locked first.
	
	pgpClearMemory( &pb, sizeof( pb ) );
	
	pb.volumeParam.ioVRefNum	= vRefNum;
	pb.volumeParam.ioNamePtr	= tempStr;

	if( IsntErr( PBHGetVInfoSync( &pb ) ) )
	{
		haveWriteAccess =
					((pb.volumeParam.ioVAtrb & kVolumeAttributeLockMask) == 0);
	}
	
	if( haveWriteAccess )
	{
		CInfoPBRec	cpb;
		
		// Check the folder access permissions
		
		pgpClearMemory( &cpb, sizeof( cpb ) );
		
		cpb.dirInfo.ioVRefNum	= vRefNum;
		cpb.dirInfo.ioDrDirID	= dirID;
		cpb.dirInfo.ioNamePtr	= tempStr;
		cpb.dirInfo.ioFDirIndex	= -1;
		
		if( IsntErr( PBGetCatInfoSync( &cpb ) ) )
		{
#define		kCannotSeeFilesMask			0x02
#define		kCannotMakeChangesMask		0x04
#define		kNoWriteAccessMask			( kCannotSeeFilesMask |  \
										kCannotMakeChangesMask )
			
			haveWriteAccess = (( cpb.dirInfo.ioACUser &
						kNoWriteAccessMask ) == 0);
		}
	}

	return( haveWriteAccess );
}
	OSStatus
GetPGPFileKindInfo(
	const FSSpec *	spec,
	FileKindInfo *	info )
{
	OSStatus	err;
	CInfoPBRec	cpb;
	
	pgpClearMemory( info, sizeof( *info ) );
	err	= FSpGetCatInfo( spec, &cpb );
	if ( IsErr( err ) )
		return( err );
	
	if ( cpbFileCreator( &cpb ) == kPGPMacFileCreator_Tools )
	{
		/* it's a PGP file */
		info->isPGPFile	= TRUE;
		
		switch( cpbFileType( &cpb ) )
		{
			default:
			case kPGPMacFileTypeArmorFile:
				break;
				
			case kPGPMacFileTypeEncryptedData:
				info->isEncrypted	= TRUE;
				break;
				
			case kPGPMacFileTypeSignedData:
				info->isSigned	= TRUE;
				break;
				
			case kPGPMacFileTypeDetachedSig:
				info->isDetachedSig	= TRUE;
				break;
		}
	}
	else if( cpbFileType( &cpb ) == 'TEXT' )
	{
		info->maybePGPFile = TRUE;
	}
	else
	{
		const uchar 	kPGPSuffix[]	= "\p.pgp";
		const uchar 	kSigSuffix[]	= "\p.sig";
		const uchar 	kAscSuffix[]	= "\p.asc";
		
		/* look for file name extensions */
		if ( PStringHasSuffix( spec->name, kPGPSuffix, FALSE) )
		{
			info->maybePGPFile = TRUE;
		}
		else if ( PStringHasSuffix( spec->name, kSigSuffix, FALSE) )
		{
			info->maybePGPFile = TRUE;
		}
		else if ( PStringHasSuffix( spec->name, kAscSuffix, FALSE) )
		{
			info->maybePGPFile = TRUE;
		}
	}
	
	return( err );
}

	OSStatus
RemoveChildAndDuplicateSpecsFromList(TArray<FSSpec>	*specList)
{
	OSStatus		macErr = noErr;
	PGPUInt32 		specIndex;
	PGPUInt32		numSpecs;
	
	pgpAssertAddrValid( specList, VoidAlign );
	
	// Walk the list backwards and see if the target spec
	// is contained within any other spec in the list. Slow, but simple.
	
	numSpecs = specList->GetCount();
	for( specIndex = numSpecs; specIndex >= 1; specIndex-- )
	{
		FSSpec		targetSpec;
		PGPUInt32	checkIndex;
		
		specList->FetchItemAt( specIndex, targetSpec );
		
		for( checkIndex = 1; checkIndex <= numSpecs; checkIndex++ )
		{
			if( checkIndex != specIndex )
			{
				FSSpec	checkSpec;
				Boolean	remove = FALSE;
				
				specList->FetchItemAt( checkIndex, checkSpec );
		
				if( FSpEqual( &checkSpec, &targetSpec ) )
				{
					remove = TRUE;
				}
				else
				{
					Boolean	isContained;
					
					macErr = FSSpecContainsFSSpec( &checkSpec, &targetSpec,
							&isContained );
					if( IsntErr( macErr ) && isContained )
					{
						remove = TRUE;
					}
				}
				
				if( remove )
				{
					specList->RemoveItemsAt( 1, specIndex );
					--numSpecs;
					
					break;
				}
			}
		}
		
		if( IsErr( macErr ) )
			break;
	}
	
	return( macErr );
}
















