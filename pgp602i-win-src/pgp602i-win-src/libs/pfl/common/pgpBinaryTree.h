/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	
	
	$Id: pgpBinaryTree.h,v 1.3.16.1 1998/11/12 03:17:50 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpBinaryTree_h	/* [ */
#define Included_pgpBinaryTree_h

#include "pgpMemoryMgr.h"

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


typedef struct SPGPBinaryTree * PGPBinaryTreeRef;

#define	kInvalidPGPBinaryTreeRef		((PGPBinaryTreeRef) NULL)
#define PGPBinaryTreeRefIsValid(ref)	((ref) != kInvalidPGPBinaryTreeRef)


/* Creating and disposing of binary trees */
PGPError	PGPNewBinaryTree(PGPMemoryMgrRef inMemoryMgr,
				PGPBinaryTreeRef * outBinaryTree);
					
PGPError	PGPDisposeBinaryTree(PGPBinaryTreeRef inBinaryTree);



/* Creating, disposing, finding and changing nodes */
PGPError	PGPAddNode(PGPBinaryTreeRef inBinaryTree,
				PGPInt32 inKey, PGPUserValue inUserValue);

PGPError	PGPDisposeNode(PGPBinaryTreeRef inBinaryTree,
				PGPInt32 inKey, PGPUserValue * outUserValue);

PGPError	PGPFindNode(PGPBinaryTreeRef inBinaryTree,
				PGPInt32 inKey, PGPUserValue * outUserValue);

PGPError	PGPChangeNode(PGPBinaryTreeRef inBinaryTree,
				PGPInt32 inKey, PGPUserValue inNewUserValue,
				PGPUserValue * outOldUserValue);
					
/* Traversal of tree. Any return value other than kPGPError_NoErr will
result in halting the traversal and that error will be returned. Note
that this has not been tested for node deletion safety .*/
typedef	PGPError	(*PGPBinaryTreeTraversalProc)(PGPUserValue inUserValue,
						PGPInt32 inNodeKey, PGPUserValue inNodeUserValue);

PGPError	PGPInOrderTraversal(PGPBinaryTreeRef inBinaryTree,
				PGPBinaryTreeTraversalProc inUserProc,
				PGPUserValue inUserValue);

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS



#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif


#endif /* ] Included_pgpBinaryTree_h */
