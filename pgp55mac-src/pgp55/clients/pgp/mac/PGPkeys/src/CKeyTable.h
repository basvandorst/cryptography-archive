/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CKeyTable.h,v 1.30 1997/10/22 07:29:48 wprice Exp $
____________________________________________________________________________*/
#pragma once

#include "pgpKeys.h"
#include "CDragSource.h"
#include "pgpKeyServerPrefs.h"
#include <LDragAndDrop.h>

class CKeyDragTask;
class CKeyTableLabels;
class CSearchWindow;
class CGroupsWindow;

const ResIDT	kColumnTitleStringsID	=	1000;
const Int16		kMaxKeyTableColumnTypes	=	9;
const Int16		kMaxKeypressInterval	=	60;
const Int16		kMaxSearchStringSize	=	64;
const Int16		kDropFlagSlop			=	20;
const Int16		kLevelIndent			=	20;
const MessageT	kRebuildServersMessage	=	'reSM';
const MessageT	kKeyTableRedrawMessage	=	'redr';
const MessageT	kKeyTableResyncMessage	=	'resy';
const Int16		kLeftIndent				=	2;
const Int16		kIconWidth				=	16;
const Int16		kIconHeight				=	12;
const Int16		kLeftBorder				=	4;
const Int16		kBottomBorder			=	5;

typedef struct KeyTableColumnInfo
{
	Int16	numActiveColumns;
	Int16	columns[kMaxKeyTableColumnTypes];
	Int16	columnWidths[kMaxKeyTableColumnTypes];
} KeyTableColumnInfo;

enum KeyTableEntryType	{	kKey, kUserID, kSignature	};

typedef struct KeyTableRef
{
	enum KeyTableEntryType type;
	PGPKeyRef		ownerKey;
	Boolean			terminated;
	union
	{
		PGPKeyRef		key;
		PGPUserIDRef	user;
		PGPSigRef		sig;
	} u;
} KeyTableRef;

typedef struct KeyImportStruct
{
	Boolean				savedImportIsFile;
	Boolean				bypassImportDialog;
	FSSpec				savedImportSpec;
	PGPByte				*savedImportBuffer;
	PGPSize				savedImportBufferSize;
	KeyImportStruct		*next;
} KeyImportStruct;


class	CKeyTable	:	public LHierarchyTable,
						public LBroadcaster,
						public LCommander,
						public CDragSource,
						public LDragAndDrop,
						public LListener,
						public LPeriodical
{
public:
	enum { class_ID = 'keyT' };
						CKeyTable(LStream *inStream);
	virtual				~CKeyTable();
	void				FinishCreateSelf();
	virtual Boolean		ObeyCommand(CommandT inCommand, void* ioParam);	
	virtual void		FindCommandStatus(CommandT inCommand,
							Boolean &outEnabled, Boolean &outUsesMark,
							Char16 &outMark, Str255 outName);
	virtual Boolean		HandleKeyPress(const EventRecord&	inKeyEvent);
	virtual void		DrawSelf();
	virtual void		ClickSelf(const SMouseDownEvent	&inMouseDown);
	virtual void		Click(SMouseDownEvent	&inMouseDown);
	virtual void		ListenToMessage(	MessageT	inMessage,
											void		*ioParam);
	virtual void		SpendTime(const EventRecord		&inMacEvent);
	
	void				ScrollImageBy(
									Int32		inLeftDelta,
									Int32		inTopDelta,
									Boolean		inRefresh);
	void				SetKeyLabels(CKeyTableLabels *keyLabels)
							{	mKeyLabels = keyLabels;		}


	// Drag and Drop Class Overrides
	void				FocusDropArea();
	virtual Boolean		ItemIsAcceptable(
										DragReference	inDragRef,
										ItemReference	inItemRef);
	virtual void		ReceiveDragItem(
										DragReference	inDragRef,
										DragAttributes	inDragAttrs,
										ItemReference	inItemRef,
										Rect			&inItemBounds);
	void				HiliteDropArea(	DragReference	inDragRef);
	void				UnhiliteDropArea(DragReference	inDragRef);
	
	// The next four functions are overrides from CDragSource
	virtual void		GetDragHiliteRgn(	RgnHandle		ioHiliteRgn);
	virtual void		AddFlavors(			CKeyDragTask	*dragTask);
	virtual Boolean		GetTextBlock(		ItemReference	item, 
											void			**outData,
											Int32			*len );
	virtual Boolean		GetHFS(				ItemReference	item,
											DragReference	dragRef,
											FSSpec			*spec);

	// Key Management routines
	virtual void		SetKeyDBInfo(	PGPKeySetRef	keySet,
										Boolean			writable,
										Boolean			defaultRings );
	void				GetKeyColumnInfo(KeyTableColumnInfo *columnInfo);
	void				ImportKeysFromHandle(Handle data);
	void				ImportKeysFromKeySet(PGPKeySetRef keySet);

	// Table Management routines
	Boolean				ResyncTable(Boolean collapse, Boolean onlyUnmarked);
	void				RedrawTable();
	void				ClearTable();
	void				Reset1();
	void				Empty();
	void				MoveColumn(Int16 oldCol, Int16 newCol);
	
	static void			DrawIcon(				ResIDT		iconID,
												Rect		cellRect,
												UInt32		indentLevel);
	
	Boolean				IsWritable()	{	return mMutableRing;	}
	PGPKeySetRef		GetKeySet()		{	return mKeySet;			}
	PGPKeyListRef		GetKeyList()	{	return mKeyList;		}
	void				BuildServerSubmenu();
protected:
	virtual void		DrawCell(
								const STableCell		&inCell,
								const Rect				&inLocalRect);
	virtual void		ClickCell(
								const STableCell&		inCell,
								const SMouseDownEvent&	inMouseDown);
	virtual void		HiliteCellActively(
								const STableCell	&inCell,
								Boolean				inHilite);
	virtual void		HiliteCellInactively(
								const STableCell	&inCell,
								Boolean				inHilite);
	void				ClickValidityCell(
								const STableCell&		inCell);
private:
	void				Reset();
	void				CommitKeySet();
	Boolean				AskUserForUserID(
										Str255			userIDStr);
	
	Boolean				SelectionExists();
	void				CollapseKeys(Boolean all);
	void				ExpandKeys();
	UInt32				SelectionAction(
							Boolean (CKeyTable::*function)(STableCell&));
	Boolean				ShowKeyInfo(STableCell& inCell);
	Boolean				Revoke(STableCell& inCell);
	Boolean				Delete(STableCell& inCell);
	Boolean				AddDrag(STableCell& inCell);
	Boolean				GetCellOnServer(STableCell& inCell);
	Boolean				SendKeyToServer(STableCell& inCell);
	Boolean				GetKeyOnServer(PGPKeyRef key);
	Boolean				SetDefault();
	void				AddUserID();
	void				Sign();
	void				ImportHandler();
	void				ImportKeysFromFile(FSSpec *keyFile, Boolean noDialog);
	void				UnmarkKey(PGPKeyRef inKey);
	void				RemoveKeyFromTable(PGPKeyRef inKey);
	void				TerminateKey(PGPKeyRef inKey);
	void				RemoveTerminatedKeys();
	void				CopyKeys();
	void				ExportSelected();
	void				SelectSearchString();
	void				SetupColumns();
	Boolean				ColumnVisible(Int16	columnID);
	void				ToggleColumn(Int16 columnID);
	void				GetColumnCheck(	Int16		columnID,
										Boolean		&outEnabled,
										Boolean		&outUsesMark,
										Char16		&outMark);
	void				DrawUserIDColumnCell(	KeyTableRef	*ktr,
												Rect		cellRect,
												Int16		depth);
	void				DrawValidityColumnCell(	KeyTableRef	*ktr,
												Rect		cellRect,
												Int16		depth);
	void				DrawTrustColumnCell(	KeyTableRef	*ktr,
												Rect		cellRect,
												Int16		depth);
	void				DrawSizeColumnCell(		KeyTableRef	*ktr,
												Rect		cellRect,
												Int16		depth);
	void				DrawKeyIDColumnCell(	KeyTableRef	*ktr,
												Rect		cellRect,
												Int16		depth);
	void				DrawCreationColumnCell(	KeyTableRef	*ktr,
												Rect		cellRect,
												Int16		depth);
	void				DrawExpirationColumnCell(KeyTableRef *ktr,
												Rect		cellRect,
												Int16		depth);
	void				DrawMRKColumnCell(		KeyTableRef	*ktr,
												Rect		cellRect,
												Int16		depth);
	void				DrawDescriptionColumnCell(KeyTableRef *ktr,
												Rect		cellRect,
												Int16		depth);
	void				DrawTrust(				Int32		trustValue,
												Rect		cellRect,
												Int16		depth);
	void				DrawValidity(			Int32		trustValue,
												Rect		cellRect,
												Int16		depth);
	void				RefreshBorder(			Rect		cellRect);
	Boolean				WeAreFront();
	void				StepForward();
	void				SaveColumnInfo();

	
	PGPKeyIterRef		mKeyIter;
	PGPKeySetRef		mKeySet;
	PGPKeyListRef		mKeyList;
	KeyTableColumnInfo	mColumnInfo;
	CKeyTableLabels		*mKeyLabels;
	Boolean				mMutableRing;
	Boolean				mDefaultRing;
	CKeyDragTask		*mDragTask;
	PixPatHandle		mBarberPixPat;
	PGPKeyRef			mDefaultKey;
	KeyImportStruct		*mImports;
	Int32				mBlockDialogDelays;
	Boolean				mNotified,
						mSendingDrag;
	PGPKeyServerEntry	*mTargetKeyServer;
	
	UInt32				mLastKeypress;
	char				mSearchString[kMaxSearchStringSize];
	
	static KeyTableColumnInfo	kDefaultColumnInfo;
	
	enum	{
				kUserIDColumnID = 0,
				kValidityColumnID,
				kTrustColumnID,
				kKeySizeColumnID,
				kKeyIDColumnID,
				kCreationColumnID,
				kExpirationColumnID,
				kMRKColumnID,
				kDescriptionColumnID
			};

	enum	{
				kStringListID		= 1004,
				kBadPassphraseID	= 1,
				kRevokeOnlyPairsID,
				kAddNameOnlyPairsID,
				kClearQuestion,
				kCantDefaultStringID,
				kBadPassID,
				kExpandSelectionID,
				kCollapseSelectionID,
				kExpandAllID,
				kCollapseAllID,
				kSigUserUnavailableID,
				kRevokeConfirmID,
				kCertUnavailableID,
				kUserIDSampleID,
				kNoKeyToExportID,
				kDupeUserID,
				kGetSignerMenuItemID,
				kGetSelectedKeyMenuItemID,
				kSetDefaultMenuItemID,
				kSetPrimaryNameMenuItemID,
				kNeverExpiresID,
				kBadNameID,
				kNoKeysFoundID,
				kSpecifyKeysToImportID,
				kStepForwardID,
				kExportKeysPromptID,
				kExportKeysDefaultNameID,
				kServerSendSuccessful,
				
				kKeyDescriptionsStringListID	=	1018,
				kRSAKeyDescriptionID			=	1,
				kDHDSSKeyDescriptionID,
				kRSAKeyPairDescriptionID,
				kDHDSSKeyPairDescriptionID,
				kUserIDDescriptionID,
				kSignatureDescriptionID
			};
};

