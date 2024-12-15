/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CPGPtoolsProcessingList.h,v 1.2 1997/08/12 04:36:00 lloyd Exp $
____________________________________________________________________________*/
#ifndef Included_CPGPtoolsProcessingList_h	/* [ */
#define Included_CPGPtoolsProcessingList_h


#include <LArray.h>


typedef struct ProcessingInfo
{
	FSSpec	srcSpec;
	short	destVRefNum;
	long	destParID;
	
	UInt32	resSize;
	UInt32	dataSize;
	FInfo	fInfo;
} ProcessingInfo;


class CPGPtoolsProcessingList
{
private:
	struct CProcessingItem
	{
		CProcessingItem *	parent;
		CProcessingItem *	children;
		CProcessingItem *	next;
		PGPUInt32			fileCount;
		Boolean				isFolder;
		
		FSSpec			srcSpec;
		long			destDirID;	/* dir ID of this item itself (if folder) */
		
		UInt32			resSize;
		UInt32			dataSize;
		FInfo			fInfo;
	};
	/* a tree structure representing a directory */
	typedef struct CProcessingItem	 CProcessingItem;


	CProcessingItem *	mTreeRoot;
	Boolean				mIsFolder;
	
	FSSpec				mRootSpec;
	FSSpec				mDestSpec;
	
	/* pointer to array of CProcessingItem */
	CProcessingItem **	mIterList;
	
	void		DeleteSubTree( CProcessingItem *item );
	void		DeleteTree( void );
	OSStatus	AllocateSpace( PGPSize size, void **mem );
	OSStatus	FreeSpace( void *mem );
	
	OSStatus	NewItem( CProcessingItem *parent,
					FSSpec const *srcSpec, CProcessingItem **item );
	OSStatus	NewFolderItem( CProcessingItem *parent,
					FSSpec const *srcSpec, CProcessingItem **item );
	OSStatus	NewFileItem( CProcessingItem *parent,
					FSSpec const *srcSpec, CInfoPBRec const *cpb,
					CProcessingItem **item );
	
	OSStatus	BuildDownFromItem( CProcessingItem *item );
	OSStatus	BuildTree( void );
	
	OSStatus	CreateFolderHierarchy( CProcessingItem *item );
	
	/* qualifier needed due to compiler bug */
	CProcessingItem **
				FlattenFiles( CProcessingItem *item, CProcessingItem ** list );
	
public:
	
	OSStatus	PrepareForIteration( void );
	UInt32		GetNumFiles( void );
	void		GetIndFile( UInt32 index, ProcessingInfo *info );
	
	OSStatus	BuildSourceTree( const FSSpec *srcRoot );
	OSStatus	CreateDestTree( const FSSpec *destSpec );
	
	CPGPtoolsProcessingList( void );
	~CPGPtoolsProcessingList( void );
	
	
};


#endif /* ] Included_CPGPtoolsProcessingList_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/