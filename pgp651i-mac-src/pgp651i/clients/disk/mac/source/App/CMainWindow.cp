/*____________________________________________________________________________	Copyright (C) 1994-1998 Network Associates Inc. and affiliated companies.	All rights reserved.		$Id: CMainWindow.cp,v 1.6 1999/03/10 02:24:41 heller Exp $____________________________________________________________________________*/#include "MacQuickdraw.h"#include "MacStrings.h"#include <UWindows.h>#include <UGraphicUtils.h>#include "PGPDiskUtils.h"#include "CMainWindow.h"	CMainWindow::CMainWindow(){}CMainWindow::CMainWindow(LStream *inStream)	: CPGPDiskWindow(inStream){}CMainWindow::~CMainWindow(){}//	Validates that the window will be visible and moveable before calling//	the real DoSetPosition().	voidCMainWindow::DoSetPosition( Point inPosition)	// Top left in global coords{	RgnHandle	myGrayRgn = NewRgn();	Rect		windowRect;	Point		upperRight;		AssertHandleIsValid( myGrayRgn, "DoSetPosition" );		CopyRgn( GetGrayRgn(), myGrayRgn );	InsetRgn( myGrayRgn, 10, 10 );		windowRect 	= UWindows::GetWindowContentRect( mMacWindowP );	upperRight		= inPosition;	upperRight.h	= inPosition.h + UGraphicUtils::RectWidth( windowRect );		if( PtInRgn( inPosition, myGrayRgn ) ||		PtInRgn( upperRight, myGrayRgn ) )	{		CPGPDiskWindow::DoSetPosition( inPosition );	}		DisposeRgn( myGrayRgn );}