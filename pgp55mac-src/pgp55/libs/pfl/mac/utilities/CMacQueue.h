#pragma once

#include "pgpMem.h"

/*____________________________________________________________________________
	A wrapper class for the Macintosh Enqueue/Dequeue routines which implement
	an atomic queueing mechanism.
____________________________________________________________________________*/

class CMacQueue
	{
protected:
	QHdr		mQHeader;
	
public:
				CMacQueue( void )
					{
					MakeEmpty();
					}
					
	virtual void	MakeEmpty( void )
					{
					mQHeader.qHead	= nil;
					mQHeader.qTail	= nil;
					mQHeader.qFlags	= 0;
					}
	
	
	
	virtual void		Enqueue( QElem *elem )
							{
							::Enqueue( elem, &mQHeader );
							}
							
	virtual OSErr		Dequeue( QElem *elem )
							{
							return( ::Dequeue( elem, &mQHeader ) );
							}
							
	virtual Boolean		IsEmpty( void )
							{
							return( IsNull( mQHeader.qHead )  );
							}
							
							
	virtual QElem *		PeekHead( void )
							{
							return( mQHeader.qHead );
							}
							
							
	virtual QElem *		PeekTail( void )
							{
							return( mQHeader.qTail );
							}
	};



