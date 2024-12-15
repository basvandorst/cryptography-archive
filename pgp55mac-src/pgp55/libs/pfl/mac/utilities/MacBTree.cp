/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	macintosh B*-Tree utilities (read-only)

	$Id: MacBTree.cp,v 1.6 1997/09/18 01:34:39 lloyd Exp $
____________________________________________________________________________*/

#include "pgpMem.h"
#include "MacBTree.h"
#include "MacFiles.h"



typedef struct ExtentKeyRecord
{
	SInt8	xkrKeyLen;	// key length
	SInt8	xkrFkType;	// fork type
	UInt32	xkrFNum;	// file number
	UInt16	xkrFABN;	// starting file allocation block
} ExtentKeyRecord;


CMacBTree::CMacBTree()
{
	mFileRefNum = 0;
}

CMacBTree::~CMacBTree()
{
	mFileRefNum = 0;
}

	OSStatus
CMacBTree::VerifyHeaderNode(const BTreeHeaderNode *headerNode)
{
	if( headerNode->descriptor.ndType != kBTreeHeaderNodeType )
	{
		pgpDebugMsg( "VerifyHeaderNode: Wrong node type" );
		return( kBTreeBadHeaderNodeError );
	}
	
	return( noErr );
}

	OSStatus
CMacBTree::Setup(void)
{
	OSStatus		status;
	BTreeHeaderNode	headerNode;
	
	pgpAssert( mFileRefNum != 0 );
	
	// Read and verify the header node

	status = FSReadAtOffset( mFileRefNum, 0,
				sizeof( headerNode ), &headerNode, nil, TRUE );
	if( IsntErr( status ) )
	{
		status = VerifyHeaderNode( &headerNode );
		if( IsntErr( status ) )
		{
			// Allocate a buffer to store a single node.
			
			mNodeSize = headerNode.record.bthNodeSize;
			
			mNodeBuffer = (NodeDescriptor *) NewPtr( mNodeSize );
			if( IsntNull( mNodeBuffer ) )
			{
			
			
			}
			else
			{
				status = memFullErr;
			}
		}
	}
	
	return( status );
}

	OSStatus
CMacBTree::GetNode(UInt32 nodeNumber, NodeDescriptor *nodeData)
{
	OSStatus	status;
	
	pgpAssert( IsntNull( mNodeBuffer ) );
	
	status = FSReadAtOffset( mFileRefNum, nodeNumber * mNodeSize, mNodeSize,
			nodeData, nil, TRUE);
			
	return( status );
}


/*____________________________________________________________________________
	Returns TRUE if the search key was found. If key was not found,
	foundRecordIndex indicates the record offset for record insertion. 
____________________________________________________________________________*/

	Boolean
CMacBTree::SearchNode(
	const BTreeKey 	*searchKey,
	NodeDescriptor 	*nodeData,
	UInt16			*foundRecordIndex)
{
	UInt16	recordIndex;
	UInt16	*curRecordOffsetPtr;
	
	pgpAssertAddrValid( searchKey, BTreeKey );
	pgpAssertAddrValid( nodeData, NodeDescriptor );
	pgpAssertAddrValid( foundRecordIndex, UInt16 );
	
	curRecordOffsetPtr = (UInt16 *)
		((uchar *) nodeData + mNodeSize - sizeof( UInt16 ));
	
	for( recordIndex = 0; recordIndex < nodeData->ndNRecs; recordIndex++)
	{
		BTreeKey	*trialKey;
		SInt32		compareResult;
		
		trialKey = (BTreeKey *) ((uchar *) nodeData + *curRecordOffsetPtr);
		
		compareResult = CompareKeys( searchKey, trialKey );
		if( compareResult < 0 )
		{
			*foundRecordIndex = recordIndex;
			return( FALSE );
		}
		else if( compareResult == 0 )
		{
			*foundRecordIndex = recordIndex;
			return( TRUE );
		}
		
		--curRecordOffsetPtr;
	}
	
	*foundRecordIndex = recordIndex;	// Not a real index
	return( FALSE );
}

	UInt32
CMacBTree::GetRecordOffset(NodeDescriptor *nodeData, UInt16 recordIndex)
{
	LogicalAddress	offsetPtr;
		
	offsetPtr = (uchar *)
		nodeData + mNodeSize - ( recordIndex * 2 ) - sizeof( UInt16 );
	
	return( *((UInt16 *) offsetPtr) );
}

	void
CMacBTree::GetRecordKeyAndData(
	NodeDescriptor 	*nodeBuffer,
	UInt16			recordIndex,
	BTreeKey		**keyPtr,
	LogicalAddress	*dataPtr)
{
	BTreeKey	*recordKey;
	uchar 		*recordData;
	UInt32		recordOffset;
			
	pgpAssertAddrValid( nodeBuffer, NodeDescriptor );
			
	recordOffset	= GetRecordOffset( nodeBuffer, recordIndex );			
	recordKey 		= (BTreeKey *) ((uchar *) nodeBuffer + recordOffset);
	recordData 		= (uchar *) recordKey + recordKey->keyLength;

	if( ( recordKey->keyLength & 1 ) != 0 )
	{
		recordData += 1;
	}
	else
	{
		recordData += 2;
	}
	
	*keyPtr = recordKey;
	*dataPtr	= recordData;
}

	OSStatus
CMacBTree::SearchTree(
	const BTreeKey 	*searchKey,
	NodeDescriptor 	*nodeBuffer,
	UInt32			*nodeNumber,
	UInt16			*recordIndex)
{
	OSStatus	status;
	
	pgpAssertAddrValid( searchKey, BTreeKey );
	pgpAssertAddrValid( nodeBuffer, NodeDescriptor );
	pgpAssertAddrValid( nodeNumber, UInt32 );
	pgpAssertAddrValid( recordIndex, UInt16 );
	
	// Always start at the header node.
	// This isn't the most efficient implementation,
	// but it is always correct.
	
	status = GetNode( 0, nodeBuffer );
	if( IsntErr( status ) )
	{
		UInt32	curNodeNumber;
		Boolean	foundKey;

		// Check for empty BTree
		if( ((BTreeHeaderNode *) nodeBuffer)->record.bthDepth == 0 )
			return( kBTreeRecordNotFoundError );
	
		curNodeNumber = ((BTreeHeaderNode *) nodeBuffer)->record.bthRoot;
	
		while( TRUE )
		{
			LogicalAddress	recordData;
			
			status = GetNode( curNodeNumber, nodeBuffer );
			if( IsErr( status ) )
			{
				goto Exit;
			}

			foundKey = SearchNode( searchKey, nodeBuffer, recordIndex );

			if( ((NodeDescriptor *) nodeBuffer)->ndType == kBTreeLeafNodeType)
			{
				// If the current node type is a leaf node, then we're done
				break;
			}
			
			// If we didn't find the record, back up one index and
			// get that record
			if( ( ! foundKey ) && ( *recordIndex != 0 ) )
				*recordIndex -= 1;
			
			GetRecordKeyAndData( nodeBuffer, *recordIndex, nil, &recordData );
			curNodeNumber = *(UInt32 *) recordData;
		}
		
		*nodeNumber = curNodeNumber;
		
		if( ! foundKey )
			status = kBTreeRecordNotFoundError;
	}

Exit:

	return( status );
}

#pragma mark -

CMacExtentsBTree::CMacExtentsBTree()
{
	mVRefNum	= 0;
	mNodeBuffer	= nil;
}

CMacExtentsBTree::~CMacExtentsBTree()
{
	if( IsntNull( mNodeBuffer ) )
	{
		DisposePtr( (Ptr) mNodeBuffer );
		mNodeBuffer = nil;
	}
}

	OSStatus
CMacExtentsBTree::OpenVolume(short vRefNum)
{
	const VCB	*vcb;
	OSStatus	status = noErr;
	
	pgpAssert( mVRefNum == 0 );
	
	vcb = GetVCBForVolume( vRefNum );
	if( IsNull( vcb ) )
		return( nsvErr );
		
	mVRefNum 	= vRefNum;
	mFileRefNum	= vcb->vcbXTRef;

	status = Setup();
	
	return( status );
}

	OSStatus
CMacExtentsBTree::CompareKeys(
	const BTreeKey *searchKey, const BTreeKey *trialKey)
{
	SInt32			result;
	ExtentKeyRecord	*searchExtKey	= (ExtentKeyRecord *) searchKey;
	ExtentKeyRecord	*trialExtKey	= (ExtentKeyRecord *) trialKey;
	
	pgpAssertAddrValid( searchKey, BTreeKey );
	pgpAssertAddrValid( trialKey, BTreeKey );
	
	// Compare the file numbers first.
	if( searchExtKey->xkrFNum > trialExtKey->xkrFNum )
	{
		result = 1;
	}
	else if( searchExtKey->xkrFNum < trialExtKey->xkrFNum )
	{
		result = -1;
	}
	else
	{
		// If the file numbers are equal, then we check the fork tpy.
		// Data forks come before resource forks in the extents BTree.
		
		if( searchExtKey->xkrFkType == trialExtKey->xkrFkType )
		{
			// Same fork type. Compare starting FABN's
			
			if (searchExtKey->xkrFABN > trialExtKey->xkrFABN)
			{
				result = 1;
			}
			else if (searchExtKey->xkrFABN < trialExtKey->xkrFABN)
			{
				result = -1;
			}
			else
			{
				result = 0;
			}
		}
		else if( searchExtKey->xkrFkType == kBTreeDataForkRecordType &&
				 trialExtKey->xkrFkType == kBTreeResourceForkRecordType )
		{
			result = -1;
		}
		else if( searchExtKey->xkrFkType == kBTreeResourceForkRecordType &&
				 trialExtKey->xkrFkType == kBTreeDataForkRecordType )
		{
			result = 1;
		}
		else
		{
			pgpDebugMsg( "CMacExtentsBTree::CompareKeys: Unknown fork type!");
			result = 1;	// Arbitrary result;
		}	
	}

	return (result);
}

	OSStatus
CMacExtentsBTree::GetExtent(
	UInt32 				fileNumber,
	SInt8 				forkType,
	UInt32 				startingFABN,
	ExtentDataRecord	*extentRecord)
{
	OSStatus		status;
	UInt32			nodeNumber;
	UInt16			recordIndex;
	ExtentKeyRecord	searchKey;
	
	pgpAssertAddrValid( extentRecord, ExtentDataRecord );
	
	searchKey.xkrKeyLen = sizeof( searchKey ) - 1;
	searchKey.xkrFkType = forkType;
	searchKey.xkrFNum 	= fileNumber;
	searchKey.xkrFABN 	= startingFABN;
	
	status = SearchTree( (BTreeKey *) &searchKey,
		mNodeBuffer, &nodeNumber, &recordIndex );
	if( IsntErr( status ) )
	{
		LogicalAddress	recordData;
		
		GetRecordKeyAndData( mNodeBuffer, recordIndex, nil, &recordData );
		BlockMoveData( recordData, extentRecord, sizeof( *extentRecord ) );
	}
	
	return( status );
}
