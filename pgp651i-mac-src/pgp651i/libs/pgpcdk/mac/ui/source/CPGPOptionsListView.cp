/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.		$Id: CPGPOptionsListView.cp,v 1.3 1999/04/04 11:13:47 wprice Exp $____________________________________________________________________________*/// Public includes:#include <LPopupButton.h>#include <LTextEditView.h>#include <UDrawingState.h>#include <UGAColorRamp.h>#include <URegistrar.h>#include <UTextTraits.h>// Private includes:#include "CPGPCheckboxOptionView.h"#include "CPGPOptionsListView.h"#include "CPGPPopupMenuOptionView.h"// ---------------------------------------------------------------------------//	class CPGPOptionTextEdit// ---------------------------------------------------------------------------class CPGPOptionTextEdit : public LTextEditView{public:	enum { class_ID = 'OTxt' };						CPGPOptionTextEdit(LStream *inStream);	virtual				~CPGPOptionTextEdit();		virtual Boolean		FocusDraw(LPane *inSubPane = nil);	};CPGPOptionTextEdit::CPGPOptionTextEdit(LStream *inStream) :					LTextEditView(inStream){}CPGPOptionTextEdit::~CPGPOptionTextEdit(){}	BooleanCPGPOptionTextEdit::FocusDraw(LPane *inSubPane){	Boolean	focused;		// Stupid PP whacks the background color to white for TextEdit classes.		focused = LTextEditView::FocusDraw( inSubPane );	if( focused )	{		RGBColor	textColor;			GetForeColor( &textColor );		ApplyForeAndBackColors();		RGBForeColor( &textColor );	}		return( focused );}// ---------------------------------------------------------------------------//	class CPGPOptionsListView// ---------------------------------------------------------------------------CPGPOptionsListView::CPGPOptionsListView(LStream *inStream)	: LView(inStream){	mNewViewImageOffset = 0;}CPGPOptionsListView::~CPGPOptionsListView(){}	voidCPGPOptionsListView::FinishCreateSelf(void){	LView::FinishCreateSelf();		RegisterClass_( CPGPCheckboxOptionView );	RegisterClass_( CPGPOptionTextEdit );	RegisterClass_( CPGPPopupMenuOptionView );}	voidCPGPOptionsListView::AppendOptionView(	CPGPOptionView 	*theView,	SInt32			viewOffset){	SDimension32	imageSize;		pgpAssertAddrValid( theView, VoidAlign );		if( viewOffset != 0 )	{		theView->ResizeFrameBy( -viewOffset, 0, FALSE );		theView->ChangedDescription();	}		theView->PlaceInSuperImageAt( viewOffset, mNewViewImageOffset, FALSE );	theView->GetImageSize( imageSize );	mNewViewImageOffset += imageSize.height;	ResizeImageBy( 0, imageSize.height, FALSE );}