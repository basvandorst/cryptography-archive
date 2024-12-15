/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#include <string.h>

#include "MacStrings.h"
#include "pgpMem.h"
#include "pgpUtilities.h"

#include "CSecureMemory.h"
#include "CPGPtoolsPassphraseList.h"

CPGPtoolsPassphraseList::CPGPtoolsPassphraseList(void) :
		LArray( sizeof( PassphraseListItem ) )
{
}

CPGPtoolsPassphraseList::~CPGPtoolsPassphraseList(void)
{
	PassphraseListItem	listItem;
	LArrayIterator		iterator( *this );
	
	while( iterator.Next( &listItem ) )
	{
		pgpAssert( IsntNull( listItem.passphrase ) );
		
		PGPFreeData( listItem.passphrase );
	}
}

	void
CPGPtoolsPassphraseList::RememberPassphrase(
	PGPContextRef 	context,
	const char 		*passphrase)
{
	PassphraseListItem	listItem;
	
	listItem.passphrase = (char *) PGPNewSecureData( context,
							strlen( passphrase ) + 1, NULL );
	if( IsntNull( listItem.passphrase ) )
	{
		CopyCString( passphrase, listItem.passphrase );
		InsertItemsAt( 1, 1, &listItem );
	}
}