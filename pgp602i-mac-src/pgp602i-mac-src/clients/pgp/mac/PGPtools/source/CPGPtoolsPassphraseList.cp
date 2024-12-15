/*____________________________________________________________________________	Copyright (C) 1997-1998 Network Associates, Inc. and its affiliates.	All rights reserved.	$Id: CPGPtoolsPassphraseList.cp,v 1.7.8.1 1998/11/12 03:09:11 heller Exp $____________________________________________________________________________*/#include <string.h>#include <LArrayIterator.h>#include "MacStrings.h"#include "pgpMem.h"#include "pgpUtilities.h"#include "CSecureMemory.h"#include "CPGPtoolsPassphraseList.h"CPGPtoolsPassphraseList::CPGPtoolsPassphraseList(void) :		LArray( sizeof( PassphraseListItem ) ){}CPGPtoolsPassphraseList::~CPGPtoolsPassphraseList(void){	PassphraseListItem	listItem;	LArrayIterator		iterator( *this );		while( iterator.Next( &listItem ) )	{		pgpAssert( IsntNull( listItem.passphrase ) );				PGPFreeData( listItem.passphrase );	}}	voidCPGPtoolsPassphraseList::RememberPassphrase(	PGPContextRef 	context,	const char 		*passphrase){	PassphraseListItem	listItem;		listItem.passphrase = (char *) PGPNewSecureData(							PGPGetContextMemoryMgr( context ),							strlen( passphrase ) + 1, 0 );	if( IsntNull( listItem.passphrase ) )	{		CopyCString( passphrase, listItem.passphrase );		InsertItemsAt( 1, 1, &listItem );	}}