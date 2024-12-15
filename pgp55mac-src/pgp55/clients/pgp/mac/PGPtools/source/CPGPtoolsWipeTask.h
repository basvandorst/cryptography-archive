/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CPGPtoolsWipeTask.h,v 1.10 1997/10/22 01:07:13 heller Exp $
____________________________________________________________________________*/
#pragma once

#include "CPGPtoolsTask.h"
	

class CPGPtoolsWipeTask : public CPGPtoolsTask
{
	friend class CMyWiper;
	
private:
	CPGPtoolsTaskProgressDialog	*mProgress;


public:
						CPGPtoolsWipeTask( const FSSpec *spec );
	virtual				~CPGPtoolsWipeTask(void);
							
	virtual	OSStatus	CalcProgressBytes(PGPContextRef context,
							ByteCount *bytes);
	virtual void		GetTaskItemName(StringPtr name);
	
	virtual CToolsError	Wipe( CPGPtoolsTaskProgressDialog * progressDialogObj );

protected:
	FSSpec				mWipeSpec;
	
	UInt32				mAllocBlockSize;
	/* careful, it should be able to hold more then 2^32 bytes */
	PGPUInt64			mTotalBytesToWipe;
	UInt32				mTotalFilesToWipe;
	UInt32				mNumLockedFiles;
	UInt32				mNumOpenFiles;
	
	PGPUInt64			mCurFileBytesWiped;
	
	PGPUInt64			CalcFileSize( const CInfoPBRec *cpb );
	void				PrescanFile( const CInfoPBRec *	cpb);
	OSStatus			PrescanFolder( short vRefNum, long dirID );
	OSStatus			Prescan( void );
	
	
	UInt32				GetScaledProgress( PGPUInt64 num );
	OSStatus			WipeHook( ConstStringPtr fileName,
							PGPUInt64 additionalDone );
	OSStatus			WipeFile( const FSSpec *spec );
	OSStatus			WipeFolder( short vRefNum, long dirID );
	CToolsError			WipeSpec( const FSSpec *spec );
};