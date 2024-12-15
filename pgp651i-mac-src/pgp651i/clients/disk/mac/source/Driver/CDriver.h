/*____________________________________________________________________________	Copyright (C) 1996-1998 Network Associates Inc. and affiliated companies.	All rights reserved.		$Id: CDriver.h,v 1.6 1999/03/10 02:33:00 heller Exp $____________________________________________________________________________*/#include <Power.h>#include "DriverPrefs.h"struct PGPDiskDriverPrefs;struct SetCipherProcStruct;struct AddDriveStruct;struct PGPDriverInfoStruct;struct PGPDiskGlobals;#include "CipherProcGlue.h"class CDriver	{protected:	static CDriver *	sSelf;		typedef struct		{		SleepQRec		sleepQRec;		CDrive *		thisObject;		Boolean			installed;		} MySleepQRec;			MySleepQRec				mSleep;	ulong					mAlertDisplayTime;	Boolean					mHaveShownSleepAlert;		static pascal Boolean	sAlertHook(DialogPtr theDialog,									EventRecord *theEvent, short *itemHit);	Boolean					TellUserSleepingWithMountedDrives(									ConstStr255Param volumeName,									Boolean canCancelSleep );	Boolean					SleepDismount( short selector );	static pascal void		sSleepDismount( void );			Boolean					mDriverIsOpen;	PGPDiskDriverPrefs		mPrefs;	DCtlEntry *				mDCE;	const PGPDiskGlobals *	mPGPDiskGlobals;	FSSpec					mPrefsSpec;		void *		mScratchBuffer;	ulong		mScratchBufferSize;		void *		mPBArray;	ushort		mNumPBs;		CipherProcRef	mCipherProcRef;	Boolean			mCipherProcDamaged;		Str255			mEjectStringTemplate;		void	InstallSleepProc( void );	void	RemoveSleepProc( void );	void	InstallOrRemoveSleepProcAsAppropriate( void );		OSErr	HandleDriverGestalt( DriverGestaltParam *pb );	OSErr	HandleDriverConfigure( ParamBlockRec *pb );		OSErr	HandleEjectDiskCC( CDrive *theDrive );	OSErr	HandleRemoveDriveCC( short	driveNumber );	OSErr	HandleDriveStatusSC( short driveNumber, DrvSts * driveStatus );	OSErr	HandleGetDriveInfoSC( PGPDriveInfoStruct * info);	OSErr	HandleGetDriverInfoSC( PGPDriverInfoStruct * info);	OSErr	HandleChangeIOOptionsCC( const IOOptionsStruct	*options);	OSErr	HandleAddDriveCC( AddDriveStruct *	addInfo );	OSErr	HandleOtherCC( ParamBlockRec *	pb);	void	HandleToggleKeyBitsCC( void );	OSErr	HandleDriveUnmountedCC(const PGPDriveUnmountedStruct *info);		void	NoDrivesLeft( void );	void	DrivePeriodic( void );	void	UnmountAnyIdleDrives( void );	void	AccRun( void );		OSErr	InitCipherProc( void );	void	HandleSetPrefsCC( const PGPDiskDriverPrefs *	newPrefs);	Boolean	DriverIsOpen( void )	{ return( mDriverIsOpen ); }	void	DisposeCipherProc( void );		ulong	CalcScratchBufferSize( void );	void	DisposeScratchBuffer( void );	OSErr	ChangeScratchBufferSize( ulong	preferredSize );		void *	AllocatePBs( const ushort minRequired, ushort * numAllocated);	OSErr	SetNumPBs( const ushort numExtra );	void	DisposePBs( void );		OSErr	ExtractFromCSParam( const ParamBlockRec *pb, void *bytes,					ulong numBytes);	void	MarkCSParamUnderstood( ParamBlockRec *	pb);		void	DriveHousekeeping(  Boolean deleteUnmountedDrives = false );						CDriver( void );public:	virtual			~CDriver( void );		static OSErr		CreateDriver( CDriver **driver );		OSErr	Open( DCtlEntry * dce );	OSErr	Prime( ParamBlockRec * pb, DCtlEntry * dce );	OSErr	Control( ParamBlockRec * pb, DCtlEntry * dce );	OSErr	Status( ParamBlockRec * pb, DCtlEntry * dce );	OSErr	Close( ParamBlockRec * pb, DCtlEntry * dce );	};