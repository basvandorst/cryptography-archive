/*____________________________________________________________________________
	Copyright (C) 1996 Pretty Good Privacy, Inc.
	All rights reserved.
____________________________________________________________________________*/
#include	"CLog.h"
#include	"UInt64.h"
#include	"CMacQueue.h"
#include	"MacMemory.h"
#include	"MacStrings.h"


#define MAX( a, b ) 	( (a) >= (b) ? (a) : (b) )
#define MIN( a, b ) 	( (a) <= (b) ? (a) : (b) )


/*____________________________________________________________________________
	Used to make interrupt level logging possible
____________________________________________________________________________*/
class CLog::CAppendQueue
	{
public:
	enum { kMaxQueuedSize = 64 };
protected:
	
	typedef struct
		{
		QElem	qElem;
		UInt16	numBytes;
		uchar	buffer[ kMaxQueuedSize ];
		} AppendElem;
	
	AppendElem *	mElemsArray;
	CMacQueue		mAvailQueue;
	CMacQueue		mReadyQueue;

		AppendElem *
	QElemToAppendElem( QElem *elem)
	{
		return( (AppendElem *)elem );
	}
	
		QElem *
	AppendElemToQElem( AppendElem *elem)
	{
		return( (QElem *)elem );
	}
	
public:
				CAppendQueue( UInt16 numBuffers = 64);
	virtual		~CAppendQueue( void );
	
	OSErr		AddData( const void *data, UInt32 numBytes );
	Boolean		RemoveData( void *data, UInt32 * numBytes );
	};


CLog::CAppendQueue::CAppendQueue( UInt16 numBuffers )
	{
	AppendElem *	elems;
	
	pgpAssert( mReadyQueue.IsEmpty() );
	pgpAssert( mAvailQueue.IsEmpty() );
	
	elems	= (AppendElem *)NewPtr( numBuffers * sizeof( AppendElem ) );
	pgpAssert( IsntNull( elems ) );
	mElemsArray	= elems;
	
	if ( IsntNull( elems ) )
		{
		// place all available buffers in our avi
		for( UInt16 index = 0; index < numBuffers; ++index )
			{
			mAvailQueue.Enqueue( AppendElemToQElem( &elems[ index ] ) );
			}
			
		pgpAssert( ! mAvailQueue.IsEmpty() );
		}
	}
	
	

CLog::CAppendQueue::~CAppendQueue( )
	{
	pgpAssert( mReadyQueue.IsEmpty() );
	
	if ( IsntNull( mElemsArray ) )
		{
		pgpAssert( ! mAvailQueue.IsEmpty() );

		DisposePtr( (Ptr)mElemsArray );
		mElemsArray	= nil;
		}
	}


	OSErr
CLog::CAppendQueue::AddData(
	const void *	data,
	UInt32			numBytes )
	{
	QElem *			qElem;
	OSErr			err	= -1;
	
	pgpAssert( numBytes <= kMaxQueuedSize );
	pgpAssert( offsetof( AppendElem, qElem ) == 0 );
	
	while ( ! mAvailQueue.IsEmpty() )
		{
		qElem	= mAvailQueue.PeekHead();
		err		= mAvailQueue.Dequeue( qElem );
		if ( IsntErr( err ) )
			{
			AppendElem *	elem;
		
			elem	= QElemToAppendElem( qElem );
			
			elem->numBytes	= MIN( numBytes, sizeof( elem->buffer ) );
			BlockMoveData( data, elem->buffer, elem->numBytes );
			
			mReadyQueue.Enqueue( qElem );
			
			// ok, we did it, let's get out of here
			break;
			}
		else
			{
			// somebody else may have taken it...try again
			}
		}
	
	AssertNoErr( err, "CLog::CAppendQueue::AddData: couldn't get buffer" );
	return( err );
	}


	Boolean
CLog::CAppendQueue::RemoveData(
	void *	data,
	UInt32 *	numBytes )
	{
	Boolean		haveData	= FALSE;
	
	QElem *	qElem	= mReadyQueue.PeekHead();
	if ( IsntNull( qElem ) )
		{
		if ( mReadyQueue.Dequeue( qElem ) == noErr )
			{
			AppendElem *elem	= QElemToAppendElem( qElem );
			
			*numBytes	= elem->numBytes;
			BlockMoveData( elem->buffer, data, elem->numBytes );
			haveData	= TRUE;
			
			mAvailQueue.Enqueue( qElem );
			}
		else
			{
			pgpDebugPStr( "\pCLog::CAppendQueue::RemoveData: impossible" );
			}
		}
		
	return( haveData );
	}
	
	
#pragma mark -
	
	void
CLog::Init( UInt32 logSize )
	{
	AllocateBuffer( logSize );
	
	mBytesInBuffer	= 0;
	}
	
	
CLog::CLog( UInt32 logSize )
	{
	Init( logSize );
	
	mAppendQueue	= new CAppendQueue();
	pgpAssert( IsntNull( mAppendQueue ) );
	}
	
	
CLog::~CLog(  )
	{
	DisposeBuffer();
	
	delete mAppendQueue;
	mAppendQueue	= nil;
	}


	void
CLog::AllocateBuffer( UInt32 logSize )
	{
	mBufferSize	= logSize;
	mBuffer		= NewPtr( logSize);
	pgpAssert( IsntNull( mBuffer ) );
	}
	

	void
CLog::DisposeBuffer( void )
	{
	pgpAssertAddrValid( mBuffer, VoidAlign );
	if ( IsntNull( mBuffer ) )
		{
		DisposePtr( (Ptr)mBuffer );
		mBuffer	= nil;
		}
	}

		

	CLog &
CLog::Append( const char *theString )
	{
	const char *cur	= theString;
	UInt32		len;
	
	while ( *cur++ != '\0' )
		{
		}
	len	= ( cur - theString ) - 1;
	
	Append( theString, len );
	return( *this );
	}
	
	
	CLog &
CLog::Append( ConstStr255Param	theString )
	{
	Append( &theString[ 1 ], theString[ 0 ] );
	return( *this );
	}
	
	
	
	CLog &
CLog::AppendBoolean( Boolean	theBool )
	{
	return( Append( theBool ? "true" : "false" ) );
	}
	
	
	CLog &
CLog::AppendNewline( )
	{
	const char	newline	= (char)'\r';
	
	return( Append( &newline, sizeof(newline) ) );
	}


	
/*____________________________________________________________________________
	We have exclusive access to the buffer (guaranteed by caller)
	Append the data to it.
____________________________________________________________________________*/
	void
CLog::Append_ItsSafeNow(
	const char *	text,
	const UInt32		length )
	{
	const char *	cur	= text;
	
	pgpAssert( IsntNull( mBuffer ) && mBufferSize != 0 );
		
	UInt32	remaining	= length;
	while ( remaining != 0 )
		{
		UInt32	count;
		UInt32	free;
		
		count	= remaining;
		free	= GetFree();
		
		// make sure we have at least some space available
		if ( free == 0 )
			{
			Flush();
			pgpAssert( GetFree() != 0 );
			free	= GetFree();
			}
		
		if ( count > free )
			{
			count	= free;
			}
		
		BlockMoveData( cur, &mBuffer[ GetUsed() ], count );
		
		pgpAssert( count <= remaining );
		remaining		-= count;
		cur				+= count;
		mBytesInBuffer	+= count;
		}
	}
	
	
/*____________________________________________________________________________
	Append any queueud items to the buffer if we can obtain control of it.
____________________________________________________________________________*/
	void
CLog::AppendAnyQueueItems()
	{
	QElem			qElem;
		
	mWinnerQueue.Enqueue( &qElem );
	if ( mWinnerQueue.PeekHead() == &qElem )
		{
		// we have exclusive acccess to buffer
		
		UInt32	bufferSize;
		char	buffer[ CAppendQueue::kMaxQueuedSize ];
		
		while ( mAppendQueue->RemoveData( buffer, &bufferSize) )
			{
			Append_ItsSafeNow( buffer, bufferSize );
			}
		}
	else
		{
		// someone else has exclusive acccess to buffer
		}
		
	mWinnerQueue.Dequeue( &qElem );
	}
	
	
	void
CLog::AppendToQueue(
	const char *	text,
	const UInt32		length )
	{
	UInt32	remaining	= length;
	
	const char *cur	= text;
	while ( remaining != 0 )
		{
		UInt32	count;
		
		count	= MIN( remaining, CAppendQueue::kMaxQueuedSize );
		(void)mAppendQueue->AddData( cur, count );
		
		remaining	-= count;
		cur			+= count;
		}
	}


/*____________________________________________________________________________
	All appends *MUST* be routed to this routine; it ensures proper logging
	under interrupt conditions.
____________________________________________________________________________*/
	CLog &
CLog::AppendBottleneck(
	const char *	text,
	const UInt32		length )
	{
	const char *	cur	= text;
	QElem			winnerElem;
	
	AppendAnyQueueItems();
		
	mWinnerQueue.Enqueue( &winnerElem );
	if ( mWinnerQueue.PeekHead() == &winnerElem )
		{
		// we got there first. Just use buffer directly
		Append_ItsSafeNow( text, length );
		}
	else
		{
		AppendToQueue( text, length );
		}
		
	// all done--someone else can use it now
	mWinnerQueue.Dequeue( &winnerElem );
	
	// someone else could have appended while we had exclusive access
	AppendAnyQueueItems();
	
	return( *this );
	}
	
	
	void
CLog::Flush( void )
	{
	if ( GetUsed() != 0 )
		{
		const char *	buffer	= GetBuffer();
		
		if ( IsntNull( buffer ) )
			{
			WriteToStore( buffer, GetUsed() );
			}
			
		Clear();
		}
	}
	
	
	void
CLog::Clear( void )
	{
	mBytesInBuffer	= 0;
	}
	
	
#pragma mark -


	void
CLogToFile::InitCLogToFile(
	UInt32	logSize,
	short	fileRefNum,
	Boolean	closeFileWhenDeleted)
	{
	mFileRefNum				= fileRefNum;
	mCloseFileWhenDeleted	= closeFileWhenDeleted;
	
	pgpClearMemory( &mWritePB, sizeof(mWritePB) );
	
	mWriteBuffer	= NewPtr( logSize );
	pgpAssert( IsntNull( mWriteBuffer ) );
	}
	
	
	void
CLogToFile::InitCLogToFile(
	UInt32			logSize,
	const FSSpec *	spec)
	{
	OSErr			err;
	
	AssertSpecIsValid( spec, "CLogToFile::CLogToFile" );
	
	err	= OpenLogFile( spec, &mFileRefNum );
	AssertFileRefNumIsValid( mFileRefNum, "CLogToFile::CLogToFile" );
	AssertNoErr( err, "CLogToFile::CLogToFile" );
	
	InitCLogToFile( logSize, mFileRefNum, TRUE );
	}
	

	OSErr
CLogToFile::OpenLogFile(
	const FSSpec *	spec,
	short *			refNum)
	{
	HParamBlockRec	pb;
	OSErr			err	= noErr;
	
	(void)FSpCreate( spec, 'CWIE', 'TEXT', smSystemScript );
	
	// open the file asynchronously so it's not tracked by the Process Mgr
	pb.fileParam.ioCompletion	= nil;
	pb.fileParam.ioNamePtr		= (StringPtr)spec->name;
	pb.fileParam.ioVRefNum		= spec->vRefNum;
	pb.fileParam.ioDirID		= spec->parID;
	err	= PBHOpenDFAsync( &pb );
	AssertNoErr( err, "CLogToFile::CLogToFile" );
	while ( pb.ioParam.ioResult == 1 )
		{
		}
	err	= pb.ioParam.ioResult;
	*refNum	= pb.ioParam.ioRefNum;
	
	if ( IsntErr( err ) )
		{
		long	eof;
		
		GetEOF( *refNum, &eof );
		SetFPos( *refNum, fsFromStart, eof );
		}
	
	return( err );
	}


CLogToFile::CLogToFile(
	const FSSpec *	spec,
	UInt32			logSize)
	: CLog( logSize )
	{
	OSErr			err;
	
	AssertSpecIsValid( spec, "CLogToFile::CLogToFile" );
	
	err	= OpenLogFile( spec, &mFileRefNum );
	AssertFileRefNumIsValid( mFileRefNum, "CLogToFile::CLogToFile" );
	AssertNoErr( err, "CLogToFile::CLogToFile" );
	
	InitCLogToFile( GetSize(), mFileRefNum, TRUE );
	}


CLogToFile::CLogToFile(
	ConstStr255Param	name,
	UInt32				logSize)
	{
	FSSpec	spec;
	
	spec.vRefNum	= -1;
	spec.parID		= 2;
	CopyPString( name, spec.name );
	InitCLogToFile( logSize, &spec );
	}
	
	
CLogToFile::CLogToFile(
	short		fileRefNum,
	Boolean		closeFileWhenDeleted,
	UInt32		logSize)
	: CLog( logSize )
	{
	InitCLogToFile( GetSize(), fileRefNum, closeFileWhenDeleted );
	}
	
	
	
CLogToFile::~CLogToFile()
	{
	Flush();
	WaitIODone();
	
	if ( mFileRefNum != 0 && mCloseFileWhenDeleted )
		{
		FSClose( mFileRefNum );
		}
	mFileRefNum	= 0;
	
	if ( IsntNull( mWriteBuffer ) )
		{
		DisposePtr( mWriteBuffer );
		}
	}
	


	

	void
CLogToFile::WaitIODone( )
	{
	while ( mWritePB.ioParam.ioResult == 1 )
		{
		}
	}


/*____________________________________________________________________________
	Write asynchronously so that caller may proceed.
	If IO already going, then 
____________________________________________________________________________*/
	void
CLogToFile::WriteToStore(
	const char *	data,
	const UInt32		length )
	{
	OSErr	err	= noErr;
	AssertFileRefNumIsValid( mFileRefNum, "CLogToFile::WriteToStore" );
	
	WaitIODone();	// in case other IO is in progress
	
	BlockMoveData( data, mWriteBuffer, length );
	
	mWritePB.ioParam.ioCompletion	= nil;
	mWritePB.ioParam.ioBuffer		= (Ptr)mWriteBuffer;
	mWritePB.ioParam.ioReqCount		= length;
	mWritePB.ioParam.ioRefNum		= mFileRefNum;
	mWritePB.ioParam.ioPosMode		= fsAtMark;
	mWritePB.ioParam.ioPosOffset	= 0;
	err	= PBWriteAsync( &mWritePB );
	AssertNoErr( err, "CLogToFile::WriteToStore" );
	}


	void
CLogToFile::sTest( )
	{
	FSSpec		spec;
	
	spec.vRefNum	= -1;
	spec.parID		= 2;
	CopyPString( "\pTest Log File\r\r\r", spec.name );
	(void)FSpDelete( &spec );
	
	CLogToFile	log( &spec, 4 * 1024UL);
	
	UInt32	size;
	
	size	= log.GetSize();
	size	= log.GetFree();
	size	= log.GetUsed();
	
	log.Clear();
	log.Flush();
	log.AppendBoolean( TRUE );
	log.AppendNewline();
	log.Append( "\phello world\r" );
	log.Append( "hello world\r" );
	
	log.Flush();
	
	for( UInt32 index = 0; index < 2 * log.GetSize(); ++index )
		{
		Str255	msg;
		
		NumToString( index, msg );
		
		log.Append( msg );
		}
	
	}
	
	


















