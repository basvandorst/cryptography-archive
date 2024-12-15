/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	$Id: CUserIDTable.h,v 1.26.4.3 1997/11/20 20:41:36 heller Exp $
____________________________________________________________________________*/

#pragma once

#include <string.h>

#include <LBroadcaster.h>
#include <LCommander.h>
#include <LDragAndDrop.h>
#include <LTableView.h>

#include "pgpMem.h"

#include "pgpUserInterface.h"
#include "pgpKeys.h"
#include "pgpGroups.h"

const short 	kMaxKeypressInterval	= 60;
const short		kMaxSearchStringSize	= 64;

typedef enum TableItemKind
{
	kTableItemKindKey	= 1,
	kTableItemKindGroup,
	kTableItemKindMissingKey
	
} TableItemKind;
#define TableItemKindIsKey( k ) \
	( (k) == kTableItemKindKey || (k) == kTableItemKindMissingKey )

#if PGP_BUSINESS_SECURITY	/* [ */
typedef enum TableItemStatus
{
	kTableItemStatusUnknown	= 1,
	kTableItemStatusSignedByCorpKey,
	kTableItemStatusNotSignedByCorpKey
	
} TableItemStatus;
#endif	/* ] */

class PGPRecipientTableItem
{
private:

	/* Names are stored as offsets into this global handle */
	PGPUInt32		nameOffset;
	
	static Handle	sNameData;
	
public:
	TableItemKind	itemKind;
	PGPUInt32		serialNumber;	/* Used to guarantee sorting order */
	PGPValidity		validity;
	PGPInt8			sortGrouping;
	
	PGPBoolean		marked			: 1;
	PGPBoolean		movedManually	: 1;
	PGPBoolean		moveToOtherList	: 1;

	/* Don't allow item to be moved from Recipients to users */
	PGPBoolean		locked			: 1;
			
	union
	{
		struct
		{
			PGPKeyRef		keyRef;
			PGPUInt16		keyBits;
			PGPUInt16		subKeyBits;
			PGPUInt32		numADKeys;
			PGPUInt16		adkRefCount;
			PGPPublicKeyAlgorithm	algorithm;
			
			PGPBoolean		isSecretKey			: 1;
			PGPBoolean		isDefaultKey		: 1;
			PGPBoolean		isAxiomaticKey		: 1;
			PGPBoolean		isPrimaryUserID		: 1;
			
			// Warn if the key is moved manually from Recipients to users.
			PGPBoolean		isEnforcedADKClass	: 1;
			
			// Set if the kPGPPrefWarnOnADK is true and this is a
			// qualifying ADK
			PGPBoolean		warnThisADK			: 1;
		} user;
		
		struct
		{
			PGPGroupSetRef	set;
			PGPGroupID		id;
			PGPUInt32		numKeys;
			PGPUInt32		numMissingKeys;
			
		} group;
	};
	
	PGPRecipientTableItem( void )
	{
		pgpClearMemory( this, sizeof( *this ) );
	}
	
		static void
	DisposeNameData(void)
	{
		if( IsntNull( sNameData ) )
		{
			DisposeHandle( sNameData );
			sNameData = NULL;
		}
	}
	
		char const *
	GetNamePtr( void ) const
	{
		pgpAssert( IsntNull( sNameData ) );
		
		return( &(*sNameData)[nameOffset] );
	}
					
		void
	SetName( const char *inName )
	{
		PGPUInt32	offset;
		PGPUInt32	nameLength;
		OSErr		err;
		
		if( IsNull( sNameData ) )
		{
			/* Zero is an invalid offset. Start at 1 */
			sNameData = NewHandle( 1 );
			pgpAssert( IsntNull( sNameData ) );
			
			**sNameData = 0;
		}
		
		offset = GetHandleSize( sNameData );
		
		/* Limit the names to 255 characters in case name is corrupted */
		nameLength = strlen( inName );
		if( nameLength > 255 )
			nameLength = 255;
			
		HUnlock( sNameData );
			err = PtrAndHand( inName, sNameData, nameLength + 1 );
			pgpAssert( IsntErr( err) );
		HLock( sNameData );
		
		if( IsntErr( err ) )
		{
			pgpCopyMemory( inName, &(*sNameData)[offset], nameLength );
			(*sNameData)[offset + nameLength] = 0;
		
			nameOffset = offset;
		}
		else
		{
			nameOffset = 0;
		}
	}
} ;


class	CUserIDTable :	public LTableView,
						public LBroadcaster,
						public LCommander,
						public LDragAndDrop
{
public:

	enum { class_ID = 'UItv' };
	
						CUserIDTable(LStream *inStream);
	virtual				~CUserIDTable();
	static CUserIDTable *CreateFromStream(LStream *inStream);

	void				AddTableItem(PGPRecipientTableItem *inTableItem,
									Boolean inRefresh);
	PGPError			AddKey( PGPKeyRef theKey, Boolean isDefaultKey,
									Boolean addDefaultUserOnly );
	PGPError			AddMissingUserID(const char *userID,
							Boolean showAtTopOfList );
	void				ClassifyItems( PGPUInt32 *numUsers,
									PGPUInt32 *numGroups,
									PGPUInt32 *numIncompleteGroups,
									PGPUInt32 *numMissingUsers,
									PGPUInt32 *numMissingGroups,
									PGPValidity *minValidity,
									PGPBoolean *haveNonDefaultUsers);
	PGPError			CreateKeySet(PGPKeySetRef allKeys, Boolean markedOnly,
									PGPKeySetRef *outSet);
	void				DisallowSelections(void);
	void				DrawSelf();
	Boolean				FindUserID(const char *userID, TableIndexT startingRow,
									TableIndexT *foundRow);
	void				FinishCreateSelf();
	virtual void		GetCellData(const STableCell &inCell, void *outDataPtr,
									Uint32 &ioDataSize) const;
	virtual void		GetCellData(const STableCell &inCell,
									PGPRecipientTableItem *outTableItem ) const;
	Boolean				HandleKeyPress(const EventRecord&	inKeyEvent);
	Boolean				HaveKey(PGPKeyRef  searchKey, UInt32 *row);
	void				MarkAll(void);
	void				MarkOneRow(TableIndexT rowIndex);
	void				MarkSelected(void);
	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam);
	virtual void		ResizeFrameBy(Int16 inWidthDelta, Int16 inHeightDelta,
									Boolean inRefresh);
	void				SetAllowDeleteKeyToSend(Boolean allowSend)
									{ mDeleteSendsMessage = allowSend; };
	virtual	void		SetCellData(const STableCell &inCell,
									const PGPRecipientTableItem *inTableItem);
	virtual void		SetCellData(const STableCell &inCell,
									const void *inDataPtr, Uint32 inDataSize);
	void				SetDisplayColumns(Boolean showIconColumn,
									Boolean showValidityColumn,
									Boolean	showKeySizeColumn);
	void				SetPreferences(Boolean showMarginalValidity,
									Boolean	marginalIsInvalid);
	void				UnmarkAll(void);

	PGPError			AddGroup( PGPKeySetRef keySet,
							PGPGroupSetRef set, PGPGroupID group );
	void				UpdateGroupValidityAndKeyCounts(PGPKeySetRef allKeys);
	
protected:

	virtual void		ClickCell(
								const STableCell&		inCell,
								const SMouseDownEvent&	inMouseDown);
	virtual void		ClickSelf(
								const SMouseDownEvent	&inMouseDown);
	virtual void		DisableSelf(void);
	virtual void		DontBeTarget();
	virtual void		DrawCell(
								const STableCell		&inCell,
								const Rect				&inLocalRect);
	virtual void		EnableSelf(void);
	virtual void		HiliteCellActively(
								const STableCell		&inCell,
								Boolean					inHilite);
	virtual void		HiliteCellInactively(
								const STableCell		&inCell,
								Boolean					inHilite);
	virtual void		HiliteDropArea(DragReference 	inDragRef);
	virtual Boolean		ItemIsAcceptable( DragReference inDragRef,
								ItemReference 			inItemRef );
	virtual void		ReceiveDragItem( DragReference inDragRef,
								DragAttributes inDragAttrs,
								ItemReference inItemRef,
								Rect &inItemBounds );
	virtual void		UnhiliteDropArea(DragReference	inDragRef);

private:

	PGPUInt32			mLastClickTime,
						mLastKeypress;
	STableCell			mLastClickCell;
	char				mSearchString[kMaxSearchStringSize];
	LArray				*mStorageArray;
	Boolean				mDeleteSendsMessage;
	PixPatHandle		mBarberPixPat;
	Boolean				mShowMarginalValidity;
	Boolean				mMarginalIsInvalid;
	
	Boolean				mShowIconColumn;
	Boolean				mShowValidityColumn;
	Boolean				mShowKeySizeColumn;
	
	static PGPUInt32	mNextSerialNumber;

	void				DragSelectedCells(const EventRecord& inMacEvent);
	void				DrawCellContents(const STableCell &inCell,
								const Rect &inLocalRect,
								UInt32 inBitDepth);
	void				DrawValidity(const PGPRecipientTableItem *tableItem,
								const Rect *columnRect,
								UInt16 bitDepth);
	void				GetCellColumnRects(const Rect *cellRect,
								Rect *iconColumnRect,
								Rect *userIDColumnRect,
								Rect *validityColumnRect,
								Rect *keySizeColumnRect);
	void				PrepareToDraw(void);
	void				SelectSearchString(void);
};

