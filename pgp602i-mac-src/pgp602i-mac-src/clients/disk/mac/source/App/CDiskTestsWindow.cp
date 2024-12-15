/*____________________________________________________________________________	Copyright (C) 1994-1998 Network Associates, Inc. and its affiliates.	All rights reserved.		$Id: CDiskTestsWindow.cp,v 1.4.8.1 1998/11/12 03:05:04 heller Exp $____________________________________________________________________________*/#if PGP_DEBUG	// [#include <LPushButton.h>#include "CDiskTestsTable.h"#include "CDiskTestsWindow.h"const PaneIDT	kDiskTestsTableScrollerPaneID	= 21001;const PaneIDT	kDiskTestsTablePaneID			= 21002;const PaneIDT	kAddTestButtonPaneID			= 21003;const PaneIDT	kDeleteTestButtonPaneID			= 21004;const PaneIDT	kDeleteAllTestsButtonPaneID		= 21005;const MessageT	msg_AddTest				= kAddTestButtonPaneID;const MessageT	msg_DeleteTest			= kDeleteTestButtonPaneID;const MessageT	msg_DeleteAllTests		= kDeleteAllTestsButtonPaneID;const MessageT	msg_SelectionChanged	= kDiskTestsTablePaneID;CDiskTestsWindow::CDiskTestsWindow(){}CDiskTestsWindow::CDiskTestsWindow(LStream *inStream)	: LWindow(inStream){	mDiskTestsTable = nil;}CDiskTestsWindow::~CDiskTestsWindow(){	if( mDiskTestsTable != nil )		mDiskTestsTable->DeleteAllTests();}	voidCDiskTestsWindow::FinishCreateSelf(void){	LPushButton	*controlObj;		LWindow::FinishCreateSelf();		controlObj = (LPushButton *) FindPaneByID( kAddTestButtonPaneID );	pgpAssertAddrValid( controlObj, VoidAlign );	controlObj->AddListener( this );	mDeleteButtonObj = (LPushButton *) FindPaneByID( kDeleteTestButtonPaneID );	pgpAssertAddrValid( mDeleteButtonObj, VoidAlign );		mDeleteButtonObj->AddListener( this );	mDeleteButtonObj->Disable();			mDeleteAllButtonObj =			(LPushButton *) FindPaneByID( kDeleteAllTestsButtonPaneID );	pgpAssertAddrValid( mDeleteAllButtonObj, VoidAlign );	mDeleteAllButtonObj->AddListener( this );	mDeleteAllButtonObj->Disable();		mDiskTestsTable =			(CDiskTestsTable *) FindPaneByID( kDiskTestsTablePaneID );	pgpAssertAddrValid( mDiskTestsTable, CDiskTestsTable );		mDiskTestsTable->AddListener( this );}	voidCDiskTestsWindow::AttemptClose(void){	Hide();}	voidCDiskTestsWindow::ListenToMessage(MessageT inMessage, void *ioParam){#pragma unused( ioParam )	switch( inMessage )	{		case msg_AddTest:		{			if( mDiskTestsTable->AddTest() )			{				mDeleteAllButtonObj->Enable();			}						break;		}		case msg_DeleteTest:		{			STableCell	theCell(0,0);						mDiskTestsTable->DeleteSelectedTests();			mDeleteButtonObj->Disable();			if( mDiskTestsTable->GetNextCell( theCell ) )			{							mDeleteAllButtonObj->Enable();					}			else			{				mDeleteAllButtonObj->Disable();					}			break;		}				case msg_DeleteAllTests:		{			mDiskTestsTable->DeleteAllTests();			mDeleteButtonObj->Disable();					mDeleteAllButtonObj->Disable();					break;		}							case msg_SelectionChanged:		{			STableCell	theCell;						theCell = mDiskTestsTable->GetFirstSelectedCell();						if( mDiskTestsTable->IsValidCell( theCell ) )			{				mDeleteButtonObj->Enable();					}			else			{				mDeleteButtonObj->Disable();					}						break;		}	}}#endif	// ] Debug