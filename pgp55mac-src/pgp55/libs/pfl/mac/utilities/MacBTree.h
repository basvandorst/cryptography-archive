/*____________________________________________________________________________
	Copyright (C) 1996,1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: MacBTree.h,v 1.4 1997/09/18 01:34:39 lloyd Exp $
____________________________________________________________________________*/

#pragma once


enum
{
	kBTreeBadHeaderNodeError	= 10000,
	kBTreeRecordNotFoundError
};

const SInt8	kBTreeLeafNodeType		= -1;
const SInt8	kBTreeIndexNodeType		= 0;
const SInt8	kBTreeHeaderNodeType	= 1;
const SInt8	kBTreeMapNodeType		= 2;

const SInt8	kBTreeDataForkRecordType		= 0;
const SInt8	kBTreeResourceForkRecordType	= -1;

#pragma options align=mac68k

typedef struct NodeDescriptor
{
	SInt32	ndFLink;	// forward link
	SInt32	ndBLink;	// backward link
	SInt8	ndType;		// node type
	SInt8	ndNHeight;	// node level
	SInt16	ndNRecs;	// number of records in node
	SInt16	ndResv2;	// Reserved

} NodeDescriptor;

typedef struct BTreeHeaderRecord
{
	SInt16	bthDepth;		// current depth of tree
	SInt32	bthRoot;		// number of root node
	SInt32	bthNRecs;		// number of leaf records in tree
	SInt32	bthFNode;		// number of first leaf node
	SInt32	bthLNode;		// number of last leaf node
	SInt16	bthNodeSize;	// size of a node
	SInt16	bthKeyLen;		// maximum length of a key
	SInt32	bthNNodes;		// total number of nodes in tree
	SInt32	bthFree;		// number of free nodes
	SInt8	bthResv[76];		// reserved

} BTreeHeaderRecord;

typedef struct BTreeHeaderNode
{
	NodeDescriptor		descriptor;
	BTreeHeaderRecord	record;

} BTreeHeaderNode;

typedef struct BTreeKey
{
	UInt8	keyLength;
	char	keyData;

} BTreeKey;

typedef struct ExtentDescriptor
{
	UInt16	xdrStABN;		// first allocation block
	UInt16	xdrNumABlks;	// number of allocation blocks

} ExtDescriptor;

typedef struct ExtentDataRecord
{
	ExtentDescriptor	extents[3];

} ExtentDataRecord;


#pragma options align=reset

class CMacBTree
{
public:
						CMacBTree();
	virtual				~CMacBTree();

	virtual	OSStatus	VerifyHeaderNode(const BTreeHeaderNode *headerNode);

protected:

	short				mFileRefNum;	// file refnum for access
	NodeDescriptor		*mNodeBuffer;
	UInt32				mNodeSize;
	
	virtual	SInt32		CompareKeys(const BTreeKey *searchKey,
							const BTreeKey *trialKey) = 0;
	OSStatus			GetNode(UInt32 nodeNumber, NodeDescriptor *nodeData);
	void				GetRecordKeyAndData(NodeDescriptor *nodeBuffer,
							UInt16 recordIndex,
							BTreeKey **keyPtr, LogicalAddress *dataPtr);
	UInt32				GetRecordOffset(NodeDescriptor *nodeData,
							UInt16 recordIndex);
	Boolean				SearchNode(const BTreeKey *searchKey,
							NodeDescriptor *nodeData,
							UInt16 *foundRecordIndex);
	OSStatus			SearchTree(const BTreeKey *searchKey,
							NodeDescriptor *nodeBuffer,
							UInt32 *nodeNumber, UInt16 *recordIndex);
	OSStatus			Setup(void);
};

class CMacExtentsBTree : public CMacBTree
{
public:

						CMacExtentsBTree();
						~CMacExtentsBTree();

	virtual	OSStatus	GetExtent(UInt32 fileNumber, SInt8 forkType,
							UInt32 startingFABN,
							ExtentDataRecord *extentRecord);
	virtual	OSStatus	OpenVolume(short vRefNum);

protected:

	virtual	SInt32		CompareKeys(const BTreeKey *searchKey,
							const BTreeKey *trialKey);

private:

	short				mVRefNum;
};



