/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//: PGPcomdlgx.h - internal header for PGPcomdlg project
//
//	$Id: PGPcomdlgx.h,v 1.3 1997/05/09 16:25:45 pbj Exp $
//

#define PGPLIB

#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "..\include\config.h"
#include "..\include\pgpkeydb.h"

#include "PGPcomdlgHelp.h"	// file containing constants
//                             passed to WINHELP

#include "..\include\PGPcomdlg.h"

#define G_BUFLEN	512

#define LINCENSEEX	150
#define LINCENSEEY	8

// prototypes

// PGPcomdlg.c
VOID PaintUserInfo (HDC hDC, INT iXpos, INT iYpos);
LRESULT PGPcomdlgMessageBox (HWND hWnd, INT iCaption, INT iMessage,
							 ULONG ulFlags);
HBITMAP LoadResourceBitmap (HINSTANCE hInstance, LPSTR lpString,
                            HPALETTE FAR* lphPalette);
