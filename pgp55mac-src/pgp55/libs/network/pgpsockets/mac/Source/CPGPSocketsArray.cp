// ===========================================================================
//	CPGPSocketsArray.cp				   (C)1995-1996 Metrowerks Inc.
//	All rights reserved.
//  Used in PGP by permission.
// ===========================================================================
//
//	A ordered sequence of fixed-sized items. Positions in the Array are
//	one-based--the first item is at index 1.
//
//	Item data is stored in a resizable Handle block, so items can be
//	dynamically added and removed. Also supports sorting and iteration
//	(using the friend class CPGPSocketsArrayIterator).

#define PGP_MACINTOSH	1

#include "CPGPSocketsArray.h"
#include "pgpMem.h"

#pragma mark === Construction & Destruction ===

class	StPGPPointerBlock
{
public:
	StPGPPointerBlock(Size inSize, Boolean inThrowFail = true);
	~StPGPPointerBlock();
	
	operator Ptr()	{ return mPtr; }
	
	Ptr		mPtr;
};

StPGPPointerBlock::StPGPPointerBlock(
	Size	inSize,					// Bytes to allocate
	Boolean	inThrowFail)			// Whether to Throw if allocate fails
{
	mPtr = ::NewPtr(inSize);
}


StPGPPointerBlock::~StPGPPointerBlock()
{
	if (mPtr != nil) {
		::DisposePtr(mPtr);
	}
}

// ___________________________________________________________________________
//		* CPGPSocketsArray
// ___________________________________________________________________________
//	Constructor for an empty Array
//

CPGPSocketsArray::CPGPSocketsArray(
	Uint32		inItemSize)
{
	InitArray(inItemSize);
}


// ___________________________________________________________________________
//		* InitArray
// ___________________________________________________________________________
//	Private Initializer

void
CPGPSocketsArray::InitArray(
	Uint32		inItemSize)
{
	mItemSize = inItemSize;
	mItemCount = 0;
	mDataStored = 0;
	mDataAllocated = 0;
	mItemsH = nil;
	mLockCount = 0;
}	


// ___________________________________________________________________________
//		* ~CPGPSocketsArray
// ___________________________________________________________________________
//	Destructor

CPGPSocketsArray::~CPGPSocketsArray()
{
	if (mItemsH != nil) {
		::DisposeHandle(mItemsH);
	}
}

#pragma mark === Inspectors ===

// ---------------------------------------------------------------------------
//		* ValidIndex
// ---------------------------------------------------------------------------
//	Return whether an index is valid (between 1 and the number of items)
//	for the Array.
//
//	If inIndex is the special flag index_Last, the index's value
//	is changed to the actual index of the last item.

Boolean
CPGPSocketsArray::ValidIndex(
	ArrayIndexT		&inIndex) const
{
	if (inIndex == index_Last) {
		inIndex = mItemCount;
	}
	return (inIndex > 0) && (inIndex <= mItemCount);
}


// ---------------------------------------------------------------------------
//		* GetItemsHandle
// ---------------------------------------------------------------------------
//	Return Handle used to store data for Array items
//
//	Usage Note: Treat the Handle as read-only. Changing the data in the
//	Handle could invalidate the internal state of the Array, particularly if
//	it is sorted
//
//	In general, you should only access the items Handle in order to
//	copy it (either in memory or by writing it to a file).

Handle
CPGPSocketsArray::GetItemsHandle() const
{
											// Remove excess allocation
	((CPGPSocketsArray*)this)->AdjustAllocation(0);	// Cast away const
	return mItemsH;
}


// ---------------------------------------------------------------------------
//		* FetchItemAt
// ---------------------------------------------------------------------------
//	Pass back the Item at the specified index
//
//	Returns true if an item exists at inIndex (and sets outItem)
//	Returns false if inIndex is out of range (and leaves outItem unchanged)
//
//	Caller must make sure that outItem points a buffer large enough to
//	hold the item data

Boolean
CPGPSocketsArray::FetchItemAt(
	ArrayIndexT		inAtIndex,
	void			*outItem) const
{
	Boolean	itemExists = ValidIndex(inAtIndex);
	
	if (itemExists) {
		PeekItem(inAtIndex, outItem);
	}
	
	return itemExists;
}


// ---------------------------------------------------------------------------
//		* FetchItemAt
// ---------------------------------------------------------------------------
//	Pass back the Item at the specified index
//
//	Returns true if an item exists at inIndex (and sets outItem)
//	Returns false if inIndex is out of range (and leaves outItem unchanged)
//
//	Copies data to the buffer pointed to by outItem. Number of bytes copied
//	is the minimum of the size of the item and ioItemSize.
//
//	On input, ioItemSize is the maximum number of bytes to copy.
//	On output, ioItemSize is the actual number of bytes copied
//
//	Use this form of the function (as opposed to the one above) if you
//	only want to fetch a portion of the data for an item.

Boolean
CPGPSocketsArray::FetchItemAt(
	ArrayIndexT		inAtIndex,
	void			*outItem,
	Uint32			&ioItemSize) const
{
	Boolean	itemExists = ValidIndex(inAtIndex);
	
	if (itemExists) {
		Uint32	dataSize = GrabItemSize(inAtIndex);
		
		if (dataSize == ioItemSize) {
			PeekItem(inAtIndex, outItem);
		
		} else {
			if (dataSize > ioItemSize) {
				dataSize = ioItemSize;
			}
			
			::BlockMoveData(GetItemPtr(inAtIndex), outItem, dataSize);
			ioItemSize = dataSize;
		}
	}
	
	return itemExists;
}


// ---------------------------------------------------------------------------
//		* FetchIndexOf
// ---------------------------------------------------------------------------
//	Returns the index of the specified item within the Array
//
//	Returns index_Bad if the item is not in the Array

ArrayIndexT
CPGPSocketsArray::FetchIndexOf(
	const void		*inItem,
	Uint32			inItemSize) const
{
	ArrayIndexT	index = index_Bad;
	
	if (inItemSize == 0) {
		inItemSize = mItemSize;
	}
	
	if (mItemCount > 0) {
		index = LinearSearch(inItem, inItemSize);
	}
	
	return index;
}

#pragma mark === Manipulators ===

// ---------------------------------------------------------------------------
//		* InsertItemsAt
// ---------------------------------------------------------------------------
//	Insert items at the specified position in an Array
//
//	inCount items are inserted into the Array starting at inAtIndex.
//
//	All items are set to the same value, as specified by inItem.
//	inItem may be nil, in which case the data for the inserted items
//	is unspecified (but space is allocated)
//
//	inAtIndex is adjusted if necessary:
//		> to sorted position if Array is kept sorted
//		> to after last item if inAtIndex is too big
//		> to 1 if inAtIndex is too small
//
//	Returns index at which items were inserted. This can differ from
//	the input value of inAtIndex as described in the paragraph above.
//
//	Errors:
//		Insertion can fail if there is not enough memory to store
//		the new items 

ArrayIndexT
CPGPSocketsArray::InsertItemsAt(
	Uint32			inCount,
	ArrayIndexT		inAtIndex,
	const void		*inItem,
	Uint32			inItemSize)
{
	if (inCount < 1) {				// Exit if nothing to insert
		return index_Bad;
	}
	
	if (mLockCount > 0) {
		SignalPStr_("\pCan't insert into a locked Array");
		return index_Bad;
	}
	
	if (inItemSize == 0) {
		inItemSize = mItemSize;
	}
	
									// Adjust Insertion Index if necessary
									
	if (inAtIndex > mItemCount) {
		inAtIndex = mItemCount +1;	// Index too big, insert at end
		
	} else if (inAtIndex < 1) {
		inAtIndex = 1;				// Index too small, insert at start
	}

	Uint32		beforeCount = mItemCount;
	
	AdjustStorage(inCount, inCount * inItemSize);
	
	if ((inAtIndex <= beforeCount) && (beforeCount > 0)) {
									// Move existing items to make
									//   room for new ones
		ShiftItems(inAtIndex, beforeCount, inCount, inCount * inItemSize);
	}
	
									// Inserted items are all set
									//   to the same value, if specified
	StoreNewItems(inCount, inAtIndex, inItem, inItemSize);
	
	return inAtIndex;
}


// ---------------------------------------------------------------------------
//		* RemoveItemsAt
// ---------------------------------------------------------------------------
//	Remove items from an Array starting at a specified position
//
//	Does nothing if inAtIndex is out of range. Checks if inCount would remove
//	items past the end of the Array, and adjusts it accordingly to remove
//	the items from inAtIndex to the end of the Array. That means you can pass
//	a large number to remove the items from inAtIndex to the end of the Array.

void
CPGPSocketsArray::RemoveItemsAt(
	Uint32			inCount,
	ArrayIndexT		inAtIndex)
{
	if (inCount < 1) {				// Nothing to do
		return;
	}

	if (mLockCount > 0) {
		SignalPStr_("\pCan't remove from a locked Array");
		return;
	}

	if (ValidIndex(inAtIndex)) {
	
		if (inAtIndex + inCount > mItemCount) {
									// Limit inCount to the number of items
									//   from inAtIndex to the end
			inCount = mItemCount - inAtIndex + 1;
		}
	
		Int32	bytesRemoved =
					GrabItemRangeSize(inAtIndex, inAtIndex + inCount - 1);
	
		if (inAtIndex + inCount <= mItemCount) {
									// Removing items from the middle
									// Shift down items that are above
									//   the ones being removed
			ShiftItems(inAtIndex + inCount, mItemCount, -inCount,
					-bytesRemoved);
		}
				
		AdjustStorage(-inCount, -bytesRemoved);
	}
}


// ---------------------------------------------------------------------------
//		* AssignItemsAt
// ---------------------------------------------------------------------------
//	Assign the same value to Items starting at the specified index
//
//	inValue is a pointer to the item data. The Array makes and stores
//	a copy of the item data.
//
//	Does nothing if inIndex is out of range

void
CPGPSocketsArray::AssignItemsAt(
	Uint32			inCount,
	ArrayIndexT		inAtIndex,
	const void		*inValue,
	Uint32			inItemSize)
{
	if (ValidIndex(inAtIndex) && (inCount > 0)) {
		
		ArrayIndexT		lastIndex = inAtIndex + inCount - 1;
		if (lastIndex > mItemCount) {
			lastIndex = mItemCount;	// Don't go past end of Array
		}
		
		if (inItemSize == 0) {
			inItemSize = mItemSize;
		}
		
		for (ArrayIndexT i = inAtIndex; i <= lastIndex; i++) {
			PokeItem(i, inValue, inItemSize);
		}
	}
}


// ---------------------------------------------------------------------------
//		* SwapItems
// ---------------------------------------------------------------------------
//	Swap the values of the Items at the specified indexes
//
//	Does nothing if either index is out of range

void
CPGPSocketsArray::SwapItems(
	ArrayIndexT		inIndexA,
	ArrayIndexT		inIndexB)
{
									// Do nothing if either index is out
									//   of range
	if (ValidIndex(inIndexA) && ValidIndex(inIndexB)) {								
		 
		 							// Allocate buffer to store copy of A
		StPGPPointerBlock	itemBuffer(GrabItemSize(inIndexA));
		InternalSwapItems(inIndexA, inIndexB, itemBuffer.mPtr);
	}
}


// ---------------------------------------------------------------------------
//		* MoveItem
// ---------------------------------------------------------------------------
//	Move an item from one position to another in an Array. The net result
//	is the same as removing the item and inserting at a new position.
//
//	Does nothing if either index is out of range
//
//	Example:
//				BEFORE						AFTER MoveItem(2, 5)
//		index	1  2  3  4  5  6			1  2  3  4  5  6
//		item	a  b  c  d  e  f			a  c  d  e  b  f

void
CPGPSocketsArray::MoveItem(
	ArrayIndexT		inFromIndex,
	ArrayIndexT		inToIndex)
{
									// Do nothing if either index is out
									//   of range or if "from" and "to"
									//   indices are the same
	if ( ValidIndex(inFromIndex) && ValidIndex(inToIndex) &&								
		 (inFromIndex != inToIndex) ) {
		
									// Allocate space to store copy of item
		StPGPPointerBlock	buffer(GrabItemSize(inFromIndex));
		
		InternalMoveItem(inFromIndex, inToIndex, buffer.mPtr);
	}
}


// ---------------------------------------------------------------------------
//		* Remove
// ---------------------------------------------------------------------------
//	Remove an item from an Array

void
CPGPSocketsArray::Remove(
	const void		*inItem,
	Uint32			inItemSize)
{
	ArrayIndexT	index = FetchIndexOf(inItem, inItemSize);
	if (index != index_Bad) {
		RemoveItemsAt(1, index);
	}
}

#pragma mark === Low-level Access ===

// ---------------------------------------------------------------------------
//		* Lock
// ---------------------------------------------------------------------------
//	Lock the Handle that stores the data for the items in the Array
//
//	Class maintains a lock count, so each call to Lock() should be
//	balanced by a corresponding call to Unlock()

void
CPGPSocketsArray::Lock() const
{
	((CPGPSocketsArray*) this)->mLockCount += 1;
	if ((mLockCount == 1) && (mItemsH != nil)) {
										// First lock,
		::HLock(mItemsH);				//   so really lock the Handle
	}
}


// ---------------------------------------------------------------------------
//		* Unlock
// ---------------------------------------------------------------------------
//	Unlock the Handle that stores the data for the items in the Array
//
//	Class maintains a lock count, so each call to Lock() should be
//	balanced by a corresponding call to Unlock()

void
CPGPSocketsArray::Unlock() const
{
	SignalIf_(mLockCount == 0);			// Too many Unlocks
	
	((CPGPSocketsArray*) this)->mLockCount -= 1;
	if ((mLockCount == 0) && (mItemsH != nil)) {
										// Last unlock
		::HUnlock(mItemsH);				//   so really unlock the Handle
	}
}


// ---------------------------------------------------------------------------
//		* GetItemPtr
// ---------------------------------------------------------------------------
//	Returns a pointer to the start of an Items data within the internal
//	storage Handle.
//
//	WARNING: The return pointer references information inside a
//	relocatable block. This pointer will become invalid if the
//	Handle block moves. Call Lock() and then Unlock() where necessary.
//
//	WARNING: Treat the data as read-only. Changing the data could
//	invalidate the internal state of the Array.

void*
CPGPSocketsArray::GetItemPtr(
	ArrayIndexT		inAtIndex) const
{
	return (*mItemsH + (inAtIndex - 1) * mItemSize);
}

#pragma mark === Storage Management ===

// ---------------------------------------------------------------------------
//		* AdjustAllocation
// ---------------------------------------------------------------------------
//	Adjust the size of the Handle used to store Array items.
//
//	You can call this routine to pre-allocate space for additional
//	items to be added later. This can reduce the calls to the
//	Toolbox routine SetHandleSize() when new items are inserted.
//
//	inExtraItems is an actual value, not relative.
//	So if there is currently space for 2 extra items and you call
//	AdjustAllocation(3), you will get 3 extra items, not 5.
//
//	inExtraData, which has a default parameter value of zero,
//	is ignored. For fixed-size items, the number of items determines
//	the amount of data stored.

void
CPGPSocketsArray::AdjustAllocation(
	Uint32		inExtraItems,
	Uint32		/* inExtraData */)
{
	InternalAdjustAllocation(0, (mItemCount + inExtraItems) * mItemSize);
}


// ---------------------------------------------------------------------------
//		* InternalAdjustAllocation [protected]
// ---------------------------------------------------------------------------
//	Called internally to change the size of the storage used
//
//	The amount of storage allocated may be larger than the amount
//	actually used.
//
//	inItemAllocation is the number of items to allocate space for
//	inDataAllocation is the amount of data to allocate space for
//
//	Fixed-size item Array only stores data, so inItemAllocation is ignored.

void
CPGPSocketsArray::InternalAdjustAllocation(
	Uint32		/* inItemAllocation */,
	Uint32		inDataAllocation)
{
	if (inDataAllocation != mDataAllocated) {
		
		if (mItemsH == nil) {			// First allocation			
			mItemsH = ::NewHandle(inDataAllocation);
			
		} else {						// Resize existing data Handle
			::SetHandleSize(mItemsH, inDataAllocation);
		}
		
		ThrowIfMemError_();
		mDataAllocated = inDataAllocation;
	}
}


// ---------------------------------------------------------------------------
//		* AdjustStorage [protected]
// ---------------------------------------------------------------------------
//	Called internally when the number of bytes used by Items in the Array
//	changes.
//
//	inDeltaItems is the change in the number of items in the Array
//	inDeltaData is ignored, since the number of items determines the
//		amount of data stored for fixed-size item Arrays
//		
//	This implementation allocates space for one extra Item when more space
//	for more items is needed.

void
CPGPSocketsArray::AdjustStorage(
	Int32		inDeltaItems,
	Int32		/* inDeltaData */)
{
	Uint32		newItemCount = mItemCount + inDeltaItems;
	Uint32		newDataStored = newItemCount * mItemSize;
	
	if (newDataStored > mDataAllocated) {	// Adding items
		InternalAdjustAllocation(0, newDataStored + mItemSize);
		
	} else if (inDeltaItems < 0) {			// Removing items
		Uint32	currentExtra = mDataAllocated - mDataStored;
		InternalAdjustAllocation(0, newDataStored + currentExtra);
	}
	
	mItemCount = newItemCount;
	mDataStored = newDataStored;
}


#pragma mark === Sorting ===

// ___________________________________________________________________________
//		* FetchInsertIndexOf
// ___________________________________________________________________________
//	Return the index at which the specified item would be inserted
//	
//	Returns index_Last if the Array is not sorted

ArrayIndexT
CPGPSocketsArray::FetchInsertIndexOf(
	const void		*inItem,
	Uint32			inItemSize) const
{
	ArrayIndexT		current = index_Last;
	
	if (inItemSize == 0) {
		inItemSize = mItemSize;
	}
	
	return current;
}


// ---------------------------------------------------------------------------
//		* PeekItem [protected]
// ---------------------------------------------------------------------------
//	Pass back the data for the Item at the specified index
//
//	Used internally to read Item data

void
CPGPSocketsArray::PeekItem(
	ArrayIndexT		inAtIndex,
	void			*outItem) const
{
	Uint32	itemSize = GrabItemSize(inAtIndex);
	
	if (itemSize == sizeof(long)) {
		*(long*) outItem = *(long*) GetItemPtr(inAtIndex);
		
	} else {
		::BlockMoveData(GetItemPtr(inAtIndex), outItem, itemSize);
	}
}


// ---------------------------------------------------------------------------
//		* PokeItem [protected]
// ---------------------------------------------------------------------------
//	Store data for the Item at the specified index
//
//	Used internally to write Item data

void
CPGPSocketsArray::PokeItem(
	ArrayIndexT		inAtIndex,
	const void		*inItem,
	Uint32			inItemSize)
{
	if (inItemSize == sizeof(long)) {
		*(long*) GetItemPtr(inAtIndex) = *(long*) inItem;
		
	} else {
		::BlockMoveData(inItem, GetItemPtr(inAtIndex), inItemSize);
	}
}


// ---------------------------------------------------------------------------
//		* ShiftItems [protected]
// ---------------------------------------------------------------------------
//	Moves items within the Handle used for internal storage
//	Moves items in the range inStartIndex to inEndIndex (inclusive)
//
//	For internal use. Performs no bounds checking.

void
CPGPSocketsArray::ShiftItems(
	ArrayIndexT		inStartIndex,
	ArrayIndexT		inEndIndex,
	Int32			inIndexShift,
	Int32			/* inDataShift */)
{
	::BlockMoveData( GetItemPtr(inStartIndex),
					 GetItemPtr(inStartIndex + inIndexShift),
					 GrabItemRangeSize(inStartIndex, inEndIndex) );
}


// ---------------------------------------------------------------------------
//		* StoreNewItems [protected]
// ---------------------------------------------------------------------------
//	Store values within the internal storage Handle.
//	Items all have the same value, and space must already have
//	been allocated for them.
//
//	For internal use. Performs no bounds checking.

void
CPGPSocketsArray::StoreNewItems(
	Uint32			inCount,
	ArrayIndexT		inAtIndex,
	const void		*inItem,
	Uint32			inItemSize)
{
	if (inItem != nil) {
		ArrayIndexT	lastIndex = inAtIndex + inCount - 1;
		for (ArrayIndexT i = inAtIndex; i <= lastIndex; i++) {
			PokeItem(i, inItem, inItemSize);
		}
	}
}


// ---------------------------------------------------------------------------
//		* InternalSwapItems [protected]
// ---------------------------------------------------------------------------
//	Swap the values of the Items at the specified indexes
//
//	Internal routine. Performs no range checking

void
CPGPSocketsArray::InternalSwapItems(
	ArrayIndexT		inIndexA,
	ArrayIndexT		inIndexB,
	void			*inBuffer)		// Large enough to hold data for A
{
	PeekItem(inIndexA, inBuffer);				// Store Copy of A
	Uint32	sizeOfA = GrabItemSize(inIndexA);
										
	InternalCopyItem(inIndexB, inIndexA);		// A = B
	
	PokeItem(inIndexB, inBuffer, sizeOfA);		// B = Copy of A
}


// ---------------------------------------------------------------------------
//		* InternalMoveItem [protected]
// ---------------------------------------------------------------------------
//	Move an item from one position to another in an Array. The net result
//	is the same as removing the item and inserting at a new position.
//
//	Internal routine. Performs no range checking

void
CPGPSocketsArray::InternalMoveItem(
	ArrayIndexT		inFromIndex,
	ArrayIndexT		inToIndex,
	void			*inBuffer)	// To hold copy of Item at inFromIndex
{
								// Store copy of Item to move
	Uint32	itemSize = GrabItemSize(inFromIndex);
	PeekItem(inFromIndex, inBuffer);
	
	if (inFromIndex < inToIndex) {
								// Move item to a higher index
								// Shift items between "from" and "to"
								//   down one spot
		ShiftItems(inFromIndex + 1, inToIndex, -1, -itemSize);
	} else {
								// Move item to a lower index
								// Shift items between "to" and "from"
								//   up one spot
		ShiftItems(inToIndex, inFromIndex - 1, 1, itemSize);
	}
	
								// Store item at new position
	PokeItem(inToIndex, inBuffer, itemSize);
}


void
CPGPSocketsArray::InternalCopyItem(
	ArrayIndexT		inSourceIndex,
	ArrayIndexT		inDestIndex)
{
	PokeItem(inDestIndex, GetItemPtr(inSourceIndex), mItemSize);
}

#pragma mark === Searching ===

// ---------------------------------------------------------------------------
//		* LinearSearch
// ---------------------------------------------------------------------------
//	Return the index of the specified item, searching linearly from the
//	start of the Array

ArrayIndexT
CPGPSocketsArray::LinearSearch(
	const void	*inItem,
	Uint32		inItemSize) const
{
	ArrayIndexT	findIndex = 0;		// Search from beginning of Array
	
	Lock();
	char	*itemPtr = *mItemsH;
	while (++findIndex <= mItemCount) {
		Uint32	itemSize = GrabItemSize(findIndex);
		if (pgpMemoryEqual(inItem, itemPtr, itemSize)) {
			break;
		}
		itemPtr += itemSize;
	}
	Unlock();
	
	if (findIndex > mItemCount) {	// Search stopped because we reached the 
		findIndex = index_Bad;		//   end without finding the item
	}

	return findIndex;
}
