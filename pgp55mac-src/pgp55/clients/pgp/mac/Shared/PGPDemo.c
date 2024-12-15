/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#include "MacFiles.h"
#include "MacInternet.h"
#include "MacResources.h"

#include "PGPDemo.h"
#include "PGPLibUtils.h"

#if PGP_DEMO	// [

#define	kDemoEncryptSignEvaluationDays	30
#define	kDemoTotalEvaluationDays		60

#define	kDemoAbsoluteExpirationYear		1998
#define	kDemoAbsoluteExpirationMonth	6
#define	kDemoAbsoluteExpirationDay		1

// Change the reosurce ID to get a new demo expiration tracking sequence
#define	kDemoInfoResType				'D\252mo'
#define	kDemoInfoResID					21894

typedef struct DemoInfo
{
	OSType	magic;
	UInt32	firstUseDate;
} DemoInfo;

	static UInt32
GetFirstDemoUseDateFromFile(const FSSpec *fileSpec)
{
	UInt32		firstUseDate = 0;
	Boolean		saveResLoad;
	short		saveResFile;
	short		fileRef;
	
	saveResLoad = LMGetResLoad();
	saveResFile	= CurResFile();
	
	SetResLoad( FALSE );
		fileRef = FSpOpenResFile( fileSpec, fsRdPerm );
	SetResLoad( TRUE );
	
	if( fileRef > 0 )
	{
		DemoInfo	**demoInfoRes;

		UseResFile( fileRef );
		
		demoInfoRes = (DemoInfo **) Get1Resource( kDemoInfoResType,
					kDemoInfoResID );
		if( IsntNil( demoInfoRes ) &&
			(**demoInfoRes).magic == kDemoInfoResType )
		{
			firstUseDate = (**demoInfoRes).firstUseDate;
			
			ReleaseResource( (Handle) demoInfoRes );
		}
		
		CloseResFile( fileRef );
		UseResFile( saveResFile );
	}
	
	SetResLoad( saveResLoad );

	return( firstUseDate );
}

	static UInt32
GetFirstDemoUseDate(void)
{
	UInt32		firstUseDate = 0xFFFFFFFF;
	FSSpec		fileSpec;
	short		vRefNum;
	long		dirID;
	
	// Check several files and take the latest date.
	// Start with the shared library
	
	if( IsntErr( PGPGetPGPLibraryFSSpec( &fileSpec ) ) )
	{
		UInt32	tempDate;
		
		tempDate = GetFirstDemoUseDateFromFile( &fileSpec );
		if( tempDate != 0 && tempDate < firstUseDate )
		{
			firstUseDate = tempDate;
		}
	}
	
	// Check the Note Pad and Scrapbook files.
	if( IsntErr( FindFolder( kOnSystemDisk, kSystemFolderType,
			kDontCreateFolder, &vRefNum, &dirID ) ) )
	{
		if( IsntErr( FSMakeFSSpec( vRefNum, dirID, "\pScrapbook File",
					&fileSpec ) ) )
		{
			UInt32	tempDate;
			
			tempDate = GetFirstDemoUseDateFromFile( &fileSpec );
			if( tempDate != 0 && tempDate < firstUseDate )
			{
				firstUseDate = tempDate;
			}
		}

		if( IsntErr( FSMakeFSSpec( vRefNum, dirID, "\pNote Pad File",
					&fileSpec ) ) )
		{
			UInt32	tempDate;
			
			tempDate = GetFirstDemoUseDateFromFile( &fileSpec );
			if( tempDate != 0 && tempDate < firstUseDate )
			{
				firstUseDate = tempDate;
			}
		}
	}
	
	if( firstUseDate == 0xFFFFFFFF )
		firstUseDate = 0;
		
	return( firstUseDate );
}

	static void
SaveFirstDemoUseDateToFile(
	const FSSpec 	*fileSpec,
	UInt32			firstUseDate)
{
	Boolean		saveResLoad;
	short		saveResFile;
	short		fileRef;
	
	saveResLoad = LMGetResLoad();
	saveResFile	= CurResFile();
	
	SetResLoad( FALSE );
		fileRef = FSpOpenResFile( fileSpec, fsRdWrPerm );
	SetResLoad( TRUE );
	
	if( fileRef > 0 )
	{
		DemoInfo	demoInfo;

		UseResFile( fileRef );

		(void) Remove1Resource( kDemoInfoResType, kDemoInfoResID, TRUE );
		
		demoInfo.magic			= kDemoInfoResType;
		demoInfo.firstUseDate 	= firstUseDate;
		
		(void) AddWriteResourcePtr( &demoInfo, sizeof( demoInfo ),
				kDemoInfoResType, kDemoInfoResID, NULL );
		
		CloseResFile( fileRef );
		UseResFile( saveResFile );
	}

	SetResLoad( saveResLoad );
}

	static void
SaveFirstDemoUseDate(UInt32 firstUseDate)
{
	FSSpec	fileSpec;
	short	vRefNum;
	long	dirID;
	
	// Add to several files. Start with the shared library
	
	if( IsntErr( PGPGetPGPLibraryFSSpec( &fileSpec ) ) )
	{
		SaveFirstDemoUseDateToFile( &fileSpec, firstUseDate );
	}
	
	// Check the Note Pad and Scrapbook files.
	if( IsntErr( FindFolder( kOnSystemDisk, kSystemFolderType,
			kDontCreateFolder, &vRefNum, &dirID ) ) )
	{
		if( IsntErr( FSMakeFSSpec( vRefNum, dirID, "\pScrapbook File",
					&fileSpec ) ) )
		{
			SaveFirstDemoUseDateToFile( &fileSpec, firstUseDate );
		}

		if( IsntErr( FSMakeFSSpec( vRefNum, dirID, "\pNote Pad File",
					&fileSpec ) ) )
		{
			SaveFirstDemoUseDateToFile( &fileSpec, firstUseDate );
		}
	}
}

	PGPDemoState
GetDemoState(void)
{
	static PGPDemoState	state = kDemoStateUnknown;
	
	if( state == kDemoStateUnknown )
	{
		UInt32	firstUseDate;
		
		pgpAssert( kDemoTotalEvaluationDays >= 
				kDemoEncryptSignEvaluationDays );
		
		state = kDemoStateActive;
		
		// First time. Check our resources for expiration info.
		
		firstUseDate = GetFirstDemoUseDate();
		if( firstUseDate != 0 )
		{
			UInt32		now;
			UInt32		absExpiration;
			DateTimeRec	dtRec;
			
			GetDateTime( &now );
			
			// Check for absolute expiration first.
			
			dtRec.year		= kDemoAbsoluteExpirationYear;
			dtRec.month		= kDemoAbsoluteExpirationMonth;
			dtRec.day		= kDemoAbsoluteExpirationDay;
			dtRec.hour		= 0;
			dtRec.minute	= 0;
			dtRec.second	= 0;
			dtRec.dayOfWeek	= 0;
			
			DateToSeconds( &dtRec, &absExpiration );
			
			if( now >= absExpiration )
			{
				state = kDemoStateExpired;
			}
			else if( now > firstUseDate +
						( kDemoTotalEvaluationDays * 24L * 60L * 60L ) )
			{
				state = kDemoStateExpired;
			}
			else if( now > firstUseDate +
						( kDemoEncryptSignEvaluationDays * 24L * 60L * 60L ) )
			{
				state = kDemoStateEncryptionSigningDisabled;
			}
		}
		else
		{
			GetDateTime( &firstUseDate );
		}
		
		SaveFirstDemoUseDate( firstUseDate );
	}

	return( state );
}

#define	kDemoOrderDialogResID	28395
#define	kDemoExpiredAlertResID	28396
	

enum
{
	kNotYetButton	= 1,
	kOrderButton	= 2
};

	static short
ShowOrderDialog(Boolean canOpenURL)
{
	GrafPtr		savePort;
	DialogRef	theDialog;
	short		itemHit = 0;
	
	GetPort( &savePort );

	theDialog = GetNewDialog( kDemoOrderDialogResID, NULL, (WindowPtr) -1L );
	if( IsntNil( theDialog ) )
	{
		SetPort( theDialog );
		
		SetDialogDefaultItem( theDialog, kNotYetButton );

		if( ! canOpenURL )
		{
			HideDialogItem( theDialog, kOrderButton );
		}
		
		SysBeep( 1 );
		
		ShowWindow( theDialog );
		SelectWindow( theDialog );

		do
		{
			ModalDialog( NULL, &itemHit );
		} while( itemHit != kNotYetButton && itemHit != kOrderButton );
		
		DisposeDialog( theDialog );
	}
	
	SetPort( savePort );
	
	return( itemHit );
}

	PGPDemoState
ShowDemoDialog(Boolean canOpenURL)
{
	PGPDemoState	state = GetDemoState();

	if( state == kDemoStateEncryptionSigningDisabled ||
		state == kDemoStateExpired )
	{
		FSSpec	libFileSpec;
		
		if( IsntErr( PGPGetPGPLibraryFSSpec( &libFileSpec ) ) )
		{
			Boolean	saveResLoad;
			short	saveResFile;
			short	fileRef;
			
			saveResLoad = LMGetResLoad();
			saveResFile	= CurResFile();
			
			SetResLoad( FALSE );
				fileRef = FSpOpenResFile( &libFileSpec, fsRdPerm );
			SetResLoad( TRUE );
		
			if( fileRef > 0 )
			{
				UseResFile( fileRef );

				FlushEvents( everyEvent, 0 );
				InitCursor();
				
				if( state == kDemoStateEncryptionSigningDisabled )
				{
					short	itemHit;
					
					itemHit = ShowOrderDialog( canOpenURL );
					if( itemHit == kOrderButton )
					{
						if( IsErr( OpenURL( "\phttp://www.pgp.com" ) ) )
						{
							SysBeep( 1 );
							SysBeep( 1 );
							SysBeep( 1 );
						}
					}
				}
				else
				{
					StopAlert( kDemoExpiredAlertResID, NULL );
				}

				CloseResFile( fileRef );
				UseResFile( saveResFile );
			}
			
			SetResLoad( saveResLoad );

			if( state == kDemoStateExpired )
			{
				// DeletePGPLib if the demo has expired
				if( IsErr( FSpDelete( &libFileSpec ) ) )
				{
					FSSpec	trashSpec;
					
					// Move to trash if we cannot delete the file.
					
					if( IsntErr( FindFolder( libFileSpec.vRefNum,
						kTrashFolderType, kCreateFolder, &trashSpec.vRefNum,
						&trashSpec.parID ) ) )
					{
						if( IsntErr( FSMakeFSSpec( trashSpec.vRefNum,
							trashSpec.parID, NULL, &trashSpec ) ) )
						{
							(void) FSpCatMove( &libFileSpec, &trashSpec );
						}
					}
				}
			}	
		}
		else
		{
			SysBeep( 1 );
		}

	}
	
	return( state );
}

#endif	// ]
