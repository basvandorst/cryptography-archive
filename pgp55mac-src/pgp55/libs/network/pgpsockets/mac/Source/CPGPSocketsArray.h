// ===========================================================================
//	CPGPSocketsArray.h				   (C)1995-1996 Metrowerks Inc.
//  All rights reserved.
//  Used in PGP by permission.
// ===========================================================================

#ifndef _H_CPGPSocketsArray
#define _H_CPGPSocketsArray
#pragma once

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import on
#endif

#include <PP_Prefix.h>

typedef const char		*ConstPtr;
typedef const ConstPtr	*ConstHandle;

typedef	Int32			ArrayIndexT;


class	CPGPSocketsArray {
	
public:
	enum				{ index_Bad		= 0,
						  index_First	= 1,
						  index_Last	= 0x7FFFFFFF };
						  
						CPGPSocketsArray(
								Uint32			inItemSize);
								
	virtual				~CPGPSocketsArray();
								
	Boolean				ValidIndex(
								ArrayIndexT		&inIndex) const;
								
	virtual Uint32		GetItemSize(
								ArrayIndexT		/* inIndex */) const
						{
							return mItemSize;
						}

	Uint32				GetCount() const		{ return mItemCount; }
	
	Handle				GetItemsHandle() const;
	
	virtual Boolean		FetchItemAt(
								ArrayIndexT		inAtIndex,
								void			*outItem) const;

	virtual Boolean		FetchItemAt(
								ArrayIndexT		inAtIndex,
								void			*outItem,
								Uint32			&ioItemSize) const;
							
	virtual ArrayIndexT	FetchIndexOf(
								const void		*inItem,
								Uint32			inItemSize = 0) const;

	virtual ArrayIndexT	InsertItemsAt(
								Uint32			inCount,
								ArrayIndexT		inAtIndex,
								const void		*inItem,
								Uint32			inItemSize = 0);

	virtual void		RemoveItemsAt(
								Uint32			inCount,
								ArrayIndexT		inAtIndex);
								
	virtual void		AssignItemsAt(
								Uint32			inCount,
								ArrayIndexT		inAtIndex,
								const void		*inValue,
								Uint32			inItemSize = 0);
	
	virtual void		SwapItems(
								ArrayIndexT		inIndexA,
								ArrayIndexT		inIndexB);
								
	virtual void		MoveItem(
								ArrayIndexT		inFromIndex,
								ArrayIndexT		inToIndex);
						
	virtual void		Remove(
								const void		*inItem,
								Uint32			inItemSize = 0);
	
	void				Lock() const;
	void				Unlock() const;
	virtual void*		GetItemPtr(
								ArrayIndexT		inAtIndex) const;
								
	virtual void		AdjustAllocation(
								Uint32			inExtraItems,
								Uint32			inExtraData = 0);
								
	virtual ArrayIndexT	FetchInsertIndexOf(
								const void		*inItem,
								Uint32			inItemSize = 0) const;
								
protected:
	Uint32				mItemSize;
	Uint32				mItemCount;
	Uint32				mDataStored;
	Uint32				mDataAllocated;
	Handle				mItemsH;
	Uint32				mLockCount;
	
							// Internal Handle Maintenance
							
	virtual void		PeekItem(
								ArrayIndexT		inAtIndex,
								void			*outItem) const;

	virtual void		PokeItem(
								ArrayIndexT		inAtIndex,
								const void		*inItem,
								Uint32			inItemSize);
						
	virtual void		InternalAdjustAllocation(
								Uint32			inItemAllocation,
								Uint32			inDataAllocation);
								
	virtual void		AdjustStorage(
								Int32			inDeltaItems,
								Int32			inDeltaData);

	virtual void		ShiftItems(
								ArrayIndexT		inStartIndex,
								ArrayIndexT		inEndIndex,
								Int32			inIndexShift,
								Int32			inDataShift);
								
	virtual void		StoreNewItems(
								Uint32			inCount,
								ArrayIndexT		inAtIndex,
								const void		*inValue,
								Uint32			inItemSize);
								
	virtual void		InternalSwapItems(
								ArrayIndexT		inIndexA,
								ArrayIndexT		inIndexB,
								void			*inBuffer);
								
	virtual void		InternalMoveItem(
								ArrayIndexT		inFromIndex,
								ArrayIndexT		inToIndex,
								void			*inBuffer);
								
	virtual void		InternalCopyItem(
								ArrayIndexT		inSourceIndex,
								ArrayIndexT		inDestIndex);
								
							// Searching
								
	ArrayIndexT			LinearSearch(
								const void		*inItem,
								Uint32			inItemSize) const;
								
	virtual Uint32		GrabItemSize(
								ArrayIndexT		/* inIndex */) const
						{
							return mItemSize;
						}
						
	virtual Uint32		GrabItemRangeSize(
								ArrayIndexT		inStartIndex,
								ArrayIndexT		inEndIndex) const
						{
							return (inEndIndex - inStartIndex + 1)
							* mItemSize;
						}
								
private:
	void				InitArray(
								Uint32			inItemSize);
};

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import reset
#endif

#endif
