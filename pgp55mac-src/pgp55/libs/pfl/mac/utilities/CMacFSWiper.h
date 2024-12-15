/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CMacFSWiper.h,v 1.1 1997/08/12 05:40:17 lloyd Exp $
____________________________________________________________________________*/
#ifndef Included_CMacFSWiper_h	/* [ */
#define Included_CMacFSWiper_h




class CMacFSWiper
{
	FSSpec					mWipeSpec;
	PGPUInt64				mTotalDone;

protected:
	virtual OSStatus	FSWipeInternal( short refNum, UInt32 wipeLength,
								const void * pattern, UInt32 patternLength);
								
	virtual OSStatus	FSWipe( short refNum, const void *pattern,
							UInt32 patternLength);
					
	virtual OSStatus	FSpWipeFork( const FSSpec *spec,
							ForkKind whichFork,
							const void *pattern, UInt32 patternLength);
				
	virtual OSStatus	FSpWipe( const FSSpec *spec,
							const void *pattern, UInt32 patternLength );
	
/*____________________________________________________________________________
	To get progress while wiping, subclass and override Progress()
	Progress  is called periodically with the number of *additional* bytes
	it has done.  It doesn't tell you the total nor does it tell you
	the cumulative number.
	
	This places a burden on the client to know what the total job size is.
____________________________________________________________________________*/
	virtual OSStatus	Progress(  ConstStringPtr fileName,
							PGPUInt64 additionalDone );
	
public:
						CMacFSWiper( void );
	virtual OSStatus	FSpWipeAndDelete( const FSSpec *spec );
};


#endif /* ] Included_CMacFSWiper_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
