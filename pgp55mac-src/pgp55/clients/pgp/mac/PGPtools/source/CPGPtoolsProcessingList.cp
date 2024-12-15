/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CPGPtoolsProcessingList.cp,v 1.3 1997/09/17 16:25:46 heller Exp $
____________________________________________________________________________*/
#include "pgpMem.h"

#include "MacFiles.h"
#include "MacStrings.h"

#include "CPGPtoolsProcessingList.h"




/*____________________________________________________________________________
	Later, we'll write a more efficient allocator
____________________________________________________________________________*/
	OSStatus
CPGPtoolsProcessingList::AllocateSpace( PGPSize size, void **mem )
{
	OSStatus	err	= noErr;
	
	*mem	= pgpAlloc( size );
	if ( IsNull( *mem ) )
		err	= memFullErr;
	return( err );
}

	OSStatus
CPGPtoolsProcessingList::FreeSpace( void *mem )
{
	if ( IsntNull( mem ) )
		pgpFree( mem );
	return( noErr );
}


	void
CPGPtoolsProcessingList::DeleteSubTree( CProcessingItem *item )
{
	CProcessingItem *	cur;
	
	/* delete any children before deleting self */
	cur	= item->children;
	while ( IsntNull( cur ) )
	{
		CProcessingItem *	next;
		
		next	= cur->next;
		DeleteSubTree( cur );
		cur	= next;
	}
	
	FreeSpace( item );
}

	void
CPGPtoolsProcessingList::DeleteTree(  )
{
	if ( IsntNull( mTreeRoot ) )
	{
		DeleteSubTree( mTreeRoot );
		mTreeRoot	= NULL;
	}
}

	
	OSStatus
CPGPtoolsProcessingList::NewItem(
	CProcessingItem *	parent,
	const FSSpec *		srcSpec,
	CProcessingItem **	itemPtr )
{
	OSStatus			err;
	void *				mem;
	CProcessingItem *	item	= NULL;
	
	err		= AllocateSpace( sizeof( *item ), &mem );
	item	= (CProcessingItem *)mem;
	if ( IsntErr( err ) )
	{
		pgpClearMemory( item, sizeof( *item ) );
		item->parent	= parent;
		item->children	= NULL;
		item->next		= NULL;
		item->srcSpec	= *srcSpec;
	}
	
	*itemPtr	= item;
	return( err );
}


	OSStatus
CPGPtoolsProcessingList::NewFolderItem(
	CProcessingItem *	parent,
	const FSSpec *		srcSpec,
	CProcessingItem **	itemPtr )
{
	OSStatus			err;
	CProcessingItem *	item;
	
	err	= NewItem( parent, srcSpec, &item );
	if ( IsntErr( err ) )
	{
		item->fileCount	= 0;
		item->isFolder	= TRUE;
	}
	
	*itemPtr	= item;
	return( err );
}


	OSStatus
CPGPtoolsProcessingList::NewFileItem(
	CProcessingItem *	parent,
	const FSSpec *		srcSpec,
	const CInfoPBRec *	cpb,
	CProcessingItem **	itemPtr )
{
	OSStatus			err;
	CProcessingItem *	item;
	
	err	= NewItem( parent, srcSpec, &item );
	if ( IsntErr( err ) )
	{
		item->fileCount	= 1;
		item->isFolder	= FALSE;
		item->resSize	= cpbResForkSize( cpb );
		item->dataSize	= cpbDataForkSize( cpb );
		item->fInfo		= cpbFInfo( cpb );
	}
	
	*itemPtr	= item;
	return( err );
}


	OSStatus
CPGPtoolsProcessingList::BuildDownFromItem( CProcessingItem *parent )
{
	OSStatus			err	= noErr;
	CInfoPBRec			cpb;
	FSSpec				spec;
	CProcessingItem *	curChild	= NULL;
	long				parentDirID;
	
	pgpAssert( IsNull( parent->children ) );
	pgpAssert( IsntNull( parent->parent ) || parent == mTreeRoot );
	
	/* get directory ID of 'parent' */
	FSpGetCatInfo( &parent->srcSpec, &cpb );
	pgpAssert( cpbIsFolder( &cpb ) );
	parentDirID	= cpb.hFileInfo.ioDirID;

	spec.vRefNum	= parent->srcSpec.vRefNum;
	spec.parID		= parentDirID;
	
	cpb.hFileInfo.ioVRefNum		= spec.vRefNum;
	cpb.hFileInfo.ioDirID		= spec.parID;
	cpb.hFileInfo.ioNamePtr		= spec.name;
	cpb.hFileInfo.ioFDirIndex	= 1;
	while ( TRUE )
	{
		CProcessingItem *	newItem	= NULL;
		Boolean				isFolder;
		
		err	= PBGetCatInfoSync( &cpb );
		if ( IsErr( err ) )
		{
			err	= noErr;
			break;
		}
		
		isFolder	= cpbIsFolder( &cpb );
		if ( isFolder )
		{
			err	= NewFolderItem( parent, &spec, &newItem );
		}
		else
		{
			err	= NewFileItem( parent, &spec, &cpb, &newItem );
			pgpAssert( IsntNull( newItem->parent ) || newItem == mTreeRoot );
			parent->fileCount	+= 1;
		}
		if ( IsErr( err ) )
			break;
		
		if ( IsNull( curChild ) )
		{
			parent->children	= newItem;
			curChild			= newItem;
		}
		else
		{
			curChild->next	= newItem;
			curChild		= newItem;
		}
		
		if ( isFolder )
		{
			err	= BuildDownFromItem( newItem );
			parent->fileCount	+= newItem->fileCount;
		}
		if ( IsErr( err ) )
			break;
		
		cpb.hFileInfo.ioDirID		= spec.parID;
		cpb.hFileInfo.ioFDirIndex	+= 1;
	}

	return( err );
}


	OSStatus
CPGPtoolsProcessingList::BuildTree( void )
{
	CInfoPBRec		cpb;
	OSStatus		err	= noErr;
	
	pgpAssert( IsNull( mTreeRoot ) );
	
	err	= FSpGetCatInfo( &mRootSpec, &cpb );
	mIsFolder	= cpbIsFolder( &cpb );
	if ( IsntErr( err ) )
	{
		CProcessingItem *	item;
		
		if ( cpbIsFile( &cpb ) )
		{
			err	= NewFileItem( NULL, &mRootSpec, &cpb, &item );
		}
		else
		{
			err	= NewFolderItem( NULL, &mRootSpec, &item );
		}
		mTreeRoot	= item;
	}
	
	if ( IsntErr( err ) && mIsFolder )
	{
		err	= BuildDownFromItem( mTreeRoot );
	}
	
	return( err );
}


	OSStatus
CPGPtoolsProcessingList::BuildSourceTree( const FSSpec *srcRoot )
{
	OSStatus		err	= noErr;
	
	mRootSpec	= *srcRoot;

	err	= BuildTree();
	
	pgpAssertNoErr( err );
	return( err );
}



	OSStatus
CPGPtoolsProcessingList::CreateFolderHierarchy( CProcessingItem *parent )
{
	OSStatus			err	= noErr;
	CProcessingItem *	cur;
	
	cur	= parent->children;
	while ( IsntNull( cur ) )
	{
		if ( cur->isFolder )
		{
			FSSpec	newSpec;
			
			newSpec.vRefNum	= mDestSpec.vRefNum;
			newSpec.parID	= parent->destDirID;
			CopyPString( cur->srcSpec.name, newSpec.name );
			err	= FSpDirCreate( &newSpec, smSystemScript, &cur->destDirID );
			if ( IsntPGPError( err ) )
			{
				err	= CreateFolderHierarchy( cur );
			}
			if ( IsErr( err ) )
				break;
		}
		
		cur	= cur->next;
	}
	
	return( err );
}


	OSStatus
CPGPtoolsProcessingList::CreateDestTree( const FSSpec *destSpec )
{
	OSStatus	err	= noErr;
	
	pgpAssert( IsntNull( mTreeRoot ) );
	
	mDestSpec	= *destSpec;
	if ( FSpEqual( destSpec, &mTreeRoot->srcSpec ) )
	{
		pgpDebugMsg( "source and dest may not be the same" );
		return( paramErr );
	}
	
	if ( mIsFolder )
	{
		/* create the destination root, which may have a different name */
		err	= FSpDirCreate( destSpec, smSystemScript, &mTreeRoot->destDirID );
		if ( IsntPGPError( err ) )
		{
			err	= CreateFolderHierarchy( mTreeRoot );
		}
	}
	
	pgpAssertNoErr( err );
	return( err );
}


	UInt32
CPGPtoolsProcessingList::GetNumFiles( void )
{
	pgpAssert( IsntNull( mTreeRoot ) );
	
	return( mTreeRoot->fileCount );
}

/*____________________________________________________________________________
	Flatten all files in the tree into an array of pointers to items in the
	tree.
	
	Each invocation returns a pointer to the next available list item.
____________________________________________________________________________*/
	CPGPtoolsProcessingList::CProcessingItem **
CPGPtoolsProcessingList::FlattenFiles(
	CProcessingItem *	item,
	CProcessingItem **	list )
{
	if ( item->isFolder )
	{
		CProcessingItem *	cur;
	
		cur	= item->children;
		while ( IsntNull( cur ) )
		{
			list	= FlattenFiles( cur, list);
			cur	= cur->next;
		}
	}
	else
	{
		*list++	= item;
	}
	return( list );
}


	OSStatus
CPGPtoolsProcessingList::PrepareForIteration( void )
{
	OSStatus	err	= noErr;
	
	pgpAssert( IsntNull( mTreeRoot ) );
	if ( IsNull( mIterList ) )
	{
		err	= AllocateSpace( sizeof( mIterList[ 0 ] ) * GetNumFiles(),
					&mIterList);
		if ( IsntErr( err ) )
		{
			CProcessingItem **	doneList;
			
			doneList	= FlattenFiles( mTreeRoot, mIterList );
			pgpAssert( mIterList + GetNumFiles() == doneList );
		}
	}
	
	return( err );
}


	void
CPGPtoolsProcessingList::GetIndFile(
	UInt32				index,
	ProcessingInfo *	info )
{
	CProcessingItem *	item;
	
	pgpAssert( index < GetNumFiles() );
	pgpAssert( IsntNull( mIterList ) );
	MacDebug_FillWithGarbage( info, sizeof( *info ) );
	
	item	= mIterList[ index ];
	pgpAssert( ! item->isFolder );
	
	info->srcSpec		= item->srcSpec;
	info->destVRefNum	= mDestSpec.vRefNum;
	if ( item == mTreeRoot )
		info->destParID		= mDestSpec.parID;
	else
		info->destParID		= item->parent->destDirID;
	
	info->resSize	= item->resSize;
	info->dataSize	= item->dataSize;
	info->fInfo		= item->fInfo;
}


CPGPtoolsProcessingList::CPGPtoolsProcessingList(  )
{
	mTreeRoot	= NULL;
	mIsFolder	= FALSE;
	mIterList	= NULL;
	MacDebug_FillWithGarbage( &mRootSpec, sizeof( mRootSpec ) );
	MacDebug_FillWithGarbage( &mDestSpec, sizeof( mDestSpec ) );
}


CPGPtoolsProcessingList::~CPGPtoolsProcessingList( void )
{
	if ( IsntNull( mIterList ) )
		FreeSpace( mIterList );
		
	DeleteTree();
}




/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/



