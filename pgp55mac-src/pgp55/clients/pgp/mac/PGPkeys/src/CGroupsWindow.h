/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: CGroupsWindow.h,v 1.11 1997/09/24 06:01:13 wprice Exp $
____________________________________________________________________________*/
#pragma once

#include "pgpGroupsUtil.h"
#include <LGACaption.h>
#include <LDragAndDrop.h>

class CGroupTableLabels;

const ResIDT	kGroupsWindowID				=	154;
const MessageT	kGroupsWindowClosed			=	'GWcl';
const Int16		kMaxGroupTableColumnTypes	=	3;

typedef struct GroupTableColumnInfo
{
	Int16	numActiveColumns;
	Int16	columns[kMaxGroupTableColumnTypes];
	Int16	columnWidths[kMaxGroupTableColumnTypes];
} GroupTableColumnInfo;

typedef struct GroupTableEntry
{
	Boolean			isTopGroup;
	union
	{
		PGPGroupID		groupID;
		struct
		{
			PGPGroupItem	groupItem;
			PGPGroupID		ownerGroupID;
			PGPKeyRef		cachedKey;
		} item;
	} u;
} GroupTableEntry;

class	CGroupsTable	:	public LHierarchyTable,
							public LCommander,
							public LDragAndDrop
{
public:
	enum { class_ID = 'tGrp' };
						CGroupsTable(LStream *inStream);
	virtual				~CGroupsTable();
	void				FinishCreateSelf();
	
	virtual void		DrawSelf();
	virtual Boolean		ObeyCommand(CommandT inCommand, void* ioParam);	
	virtual void		FindCommandStatus(CommandT inCommand,
									Boolean &outEnabled, Boolean &outUsesMark,
									Char16 &outMark, Str255 outName);

	virtual void		DrawCell(
								const STableCell		&inCell,
								const Rect				&inLocalRect);
	virtual void		ClickCell(
								const STableCell&		inCell,
								const SMouseDownEvent&	inMouseDown);
	virtual void		ClickSelf(
								const SMouseDownEvent	&inMouseDown);
	virtual Boolean		HandleKeyPress(const EventRecord&	inKeyEvent);
	virtual void		HiliteCellActively(
								const STableCell	&inCell,
								Boolean				inHilite);
	virtual void		HiliteCellInactively(
								const STableCell	&inCell,
								Boolean				inHilite);
	void				HiliteCellDrag(const STableCell	&inCell);
	void				SetGroupSet(
								PGPGroupSetRef		groupRef,
								PFLFileSpecRef		groupsFile);
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
	void				EnterDropArea(	DragReference	inDragRef,
										Boolean			inDragHasLeftSender);
	void				LeaveDropArea(	DragReference	inDragRef);
	void				InsideDropArea(	DragReference	inDragRef);
	
	void				GetColumnInfo(GroupTableColumnInfo *columnInfo);
	void				SetLabels(CGroupTableLabels *labels)
								{	mLabels = labels;	}
	void				ScrollImageBy(
									Int32		inLeftDelta,
									Int32		inTopDelta,
									Boolean		inRefresh);
	void				InvalidateCaches();
																		
private:
	void				ResyncTable(Boolean collapse, Boolean onlyUnmarked);
	void				RedrawTable();
	void				CollapseGroups(Boolean all);
	void				ExpandGroups();
	void				DrawAddressColumnCell(
								GroupTableEntry		*gtr,
								Rect				cellRect,
								Int16				depth);
	void				DrawValidityColumnCell(
								GroupTableEntry		*gtr,
								Rect				cellRect,
								Int16				depth);
	void				DrawDescriptionColumnCell(
								GroupTableEntry		*gtr,
								Rect				cellRect,
								Int16				depth);
	void				DrawValidity(	Int32		trustValue,
										Rect		cellRect,
										Int16		depth);
	void				UpdateValidityColumn();
	void				RefreshBorder(Rect cellRect);
	Boolean				EditGroup(PGPGroupInfo *groupInfo);
	void				DeleteSelection();
	void				StartDrag(
								const STableCell&		inCell,
								const SMouseDownEvent&	inMouseDown);
	PGPError			AddKeyIDToGroup(
								PGPByte				*keyID,
								PGPSize				keyIDSize,
								PGPUInt32			algorithm,
								const STableCell	&groupCell);
	PGPError			AddGroupToGroup(
								PGPGroupID			groupID,
								const STableCell	&groupCell);
	PGPError			AddGroupItemToGroup(
								PGPGroupItem		*item,
								const STableCell	&groupCell);
	void				ResortGroups();
	void				CacheKey(GroupTableEntry *tableEntry);
	void				UpdateFromServer();
	void				RebuildTable();
	void				ShowInfo();
	
	enum
	{
		kMaxColumnTypes		=	3,
		kAddressColumnID	=	0,
		kValidityColumnID,
		kDescriptionColumnID
	};
	
	STableCell				mLastDragCell;
	Boolean					mSendingDrag;
	PGPGroupSetRef			mGroupsRef;
	PFLFileSpecRef			mGroupsFile;
	CGroupTableLabels		*mLabels;
	GroupTableColumnInfo	mColumnInfo;
	static GroupTableColumnInfo		kDefaultColumnInfo;
};

class	CGroupTableLabels	:	public LPane
{
public:
	enum { class_ID = 'groL' };

						CGroupTableLabels(LStream *inStream);
	virtual				~CGroupTableLabels();
	void				DrawSelf();
	void				FinishCreateSelf();

	void				SetTable(CGroupsTable *table)
							{	mTable = table;	}
							
	void				NotifyScrolled(Int32 delta);
	
	void				ActivateSelf();
	void				DeactivateSelf();
private:
	void				GetColumnRect(Int16 col, Rect &colRect);
	
	CGroupsTable		*mTable;
	Int32				mScrollDelta;
};

class	CGroupsWindow	:	public LWindow,
							public LListener,
							public LBroadcaster
{
public:
	enum { class_ID = 'Gwin' };
								CGroupsWindow();
								CGroupsWindow(LStream *inStream);
								~CGroupsWindow();
	void						FinishCreateSelf();
	virtual void				AttemptClose();
	void						DrawSelf();
	void						ListenToMessage(MessageT	inMessage,
												void *		ioParam);
	
	virtual void				ResizeFrameBy(
									Int16		inWidthDelta,
									Int16		inHeightDelta,
									Boolean		inRefresh);
	void						InvalidateCaches();
private:
	CGroupsTable				*mGroupsTable;
	PGPGroupSetRef				mGroupsRef;
	PFLFileSpecRef				mGroupsFile;
	
	LView						*mBorderView;
	CGroupTableLabels			*mGroupLabels;
	
	enum
	{
		kBorderViewMarker		=	'vBor',
		kGroupLabels			=	'groL'
	};
};

