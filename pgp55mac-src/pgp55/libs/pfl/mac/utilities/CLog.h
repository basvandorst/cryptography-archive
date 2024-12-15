/*____________________________________________________________________________
	Copyright (C) 1996 Pretty Good Privacy, Inc.
	All rights reserved.
____________________________________________________________________________*/
#pragma once

class UInt64;

#include "CMacQueue.h"

class CLog
	{
	class CAppendQueue;
	
	CAppendQueue *	mAppendQueue;
	
protected:	
		UInt32		mBufferSize;
		UInt32		mBytesInBuffer;
		char *		mBuffer;
		
		CMacQueue	mWinnerQueue;
		
	virtual void	AllocateBuffer( UInt32	bufSize );
	virtual void	DisposeBuffer( void );
	virtual void	Init( UInt32 logSize );
	
	virtual void	AppendToQueue( const char *data, UInt32 length );
	virtual void	AppendAnyQueueItems( void );
	virtual void	Append_ItsSafeNow( const char *text, UInt32 length );
	virtual CLog &	AppendBottleneck( const char *text, UInt32 length );
	virtual void	WriteToStore( const char *text, UInt32 length )	= 0;
	
	virtual char *	GetBuffer( void )	{ return( mBuffer ); }
public:
	enum		{ kDefaultLogSize	= 32UL * 1024UL };
	
					CLog( UInt32 logSize = kDefaultLogSize);
	virtual			~CLog( void );
	
	virtual CLog &	Append( const char * theString );
	virtual CLog &	Append( ConstStr255Param theString );
	virtual CLog &	Append( const char *text, UInt32 length )
						{ return( AppendBottleneck( text, length) ); }
	virtual CLog &	Append( const uchar *text, UInt32 length )
						{ Append( (char *)text, length ); return( *this ); }
	
	virtual CLog &	AppendBoolean( Boolean	value );
	virtual CLog &	AppendNewline( void );
	
	virtual UInt32	GetSize( void )
	{
		return( mBufferSize );
	}
	
	virtual UInt32	GetFree( void )
	{
		return( mBufferSize - mBytesInBuffer );
	}
	
	virtual UInt32	GetUsed( void )	
	{
		return( GetSize() - GetFree() );
	}
	
	virtual void	Flush( void );
	virtual void	Clear( void );
	};
	


class CLogToMemory : public CLog
	{
protected:
	virtual void	WriteToStore( const char */*text*/, UInt32 /*length*/ )	{};
	
public:
				CLogToMemory( UInt32 logSize = kDefaultLogSize );
	virtual		~CLogToMemory( void );
	
	// caller may want to do something with it
	const char *	GetBufferPtr( void )	{ return( mBuffer ); }
	};

	

class CLogToFile : public CLog
	{
protected:
	
	short			mFileRefNum;
	Boolean			mCloseFileWhenDeleted;
	ParamBlockRec	mWritePB;
	char *			mWriteBuffer;
	
	void			InitCLogToFile( UInt32 logSize,
						short fileRefNum, Boolean	closeFileWhenDeleted);
	void			InitCLogToFile( UInt32 logSize, const FSSpec *spec);
	
	void			WaitIODone( void );
	
	virtual void	WriteToStore( const char *text, UInt32 length );
	
	virtual OSErr	OpenLogFile( const FSSpec *spec, short *refNum);
	
public:
	CLogToFile( const FSSpec *spec, UInt32 bufferSize = kDefaultLogSize);
	CLogToFile( ConstStr255Param, UInt32 bufferSize = kDefaultLogSize);
	CLogToFile( short fileRefNum, Boolean closeFileWhenDeleted,
			UInt32 bufferSize = kDefaultLogSize);
	virtual			~CLogToFile( void );


	static void		sTest( void );
	};
















