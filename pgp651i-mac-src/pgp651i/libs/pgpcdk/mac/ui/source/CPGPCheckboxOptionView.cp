/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.		$Id: CPGPCheckboxOptionView.cp,v 1.5 1999/04/04 11:13:42 wprice Exp $____________________________________________________________________________*/// Public includes:#include <LCheckBox.h>#include <PP_Messages.h>#include "MacStrings.h"#include "pgpMem.h"// Private includes:#include "CPGPCheckboxOptionView.h"const ResIDT	kCheckboxOptionViewResourceID 	= 4800;const PaneIDT	kCheckboxPaneID 				= 4800;CPGPCheckboxOptionView::CPGPCheckboxOptionView(LStream *inStream) :					CPGPOptionView(inStream){	mCheckbox 	= NULL;	mResultPtr	= NULL;}CPGPCheckboxOptionView::~CPGPCheckboxOptionView(){}	voidCPGPCheckboxOptionView::FinishCreateSelf(void){	CPGPOptionView::FinishCreateSelf();		mCheckbox = (LCheckBox *) FindPaneByID( kCheckboxPaneID );	pgpAssertAddrValid( mCheckbox, VoidAlign );		mCheckbox->AddListener( this );}	voidCPGPCheckboxOptionView::ListenToMessage(	MessageT 	inMessage,	void 		*ioParam){	switch( inMessage )	{		case kCheckboxPaneID:		{			BroadcastMessage( msg_ChangedViewControlValue, ioParam );			break;		}				case msg_ChangedViewControlValue:		{			if( *((SInt32 *) ioParam) != 0 )			{				mCheckbox->SetValue( 1 );			}						break;		}				case msg_OK:		{			pgpAssertAddrValid( mCheckbox, VoidAlign );			pgpAssertAddrValid( mResultPtr, PGPUInt32 );						*mResultPtr = mCheckbox->GetValue();			break;		}				default:			CPGPOptionView::ListenToMessage( inMessage, ioParam );			break;	}}	voidCPGPCheckboxOptionView::SetInfo(const PGPOUICheckboxDesc *info){	Str255	pTitle;		pgpAssertAddrValid( info, PGPOUICheckboxDesc );	pgpAssertAddrValid( info->title, char );	pgpAssertAddrValid( info->description, char );	pgpAssertAddrValid( info->valuePtr, PGPBoolean );		mItemID 	= info->itemID;	mResultPtr 	= info->valuePtr;		CToPString( info->title, pTitle );	mCheckbox->SetDescriptor( pTitle );		mCheckbox->SetValue( *mResultPtr > 0 ? 1 : 0 );	SetDescription( info->description );	}	CPGPCheckboxOptionView *CPGPCheckboxOptionView::CreateCheckboxOptionView(	LView 						*superView,	const PGPOUICheckboxDesc	*info){	CPGPCheckboxOptionView	*checkboxView;		pgpAssertAddrValid( superView, VoidAlign );	pgpAssertAddrValid( info, PGPOUICheckboxDesc );		checkboxView = (CPGPCheckboxOptionView *)						CPGPOptionView::CreateOptionView(							superView, kCheckboxOptionViewResourceID );	if( IsntNull( checkboxView ) )	{		checkboxView->SetInfo( info );	}		return ( checkboxView );}