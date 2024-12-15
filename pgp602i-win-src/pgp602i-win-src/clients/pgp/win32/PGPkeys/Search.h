/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	$Id: Search.h,v 1.19.2.1 1998/10/28 23:11:43 pbj Exp $
____________________________________________________________________________*/

#ifndef _SEARCH_H
#define _SEARCH_H

#include "pgpBase.h"
#include "pgpKeyServerPrefs.h" 

#define SEARCH_DISPLAY_KEYSET		WM_APP + 100
#define SEARCH_SIZING				WM_APP + 101
#define SEARCH_REQUEST_SIZING		WM_APP + 102
#define SEARCH_PROGRESS				WM_APP + 103
#define SEARCH_ABORT				WM_APP + 104
#define SEARCH_SECURE_STATUS		WM_APP + 105

#define SEARCH_SET_LOCAL_KEYSET		WM_APP + 110
#define SEARCH_SET_CURRENT_SEARCH	WM_APP + 111
#define SEARCH_SET_FOCUS			WM_APP + 112

#define REFRESH_KEYSERVER_LIST		WM_APP + 120

#define FLAG_SEARCH_LOCAL_KEYSET	1L
#define FLAG_SEARCH_MEMORY			2L
#define FLAG_AREA_PENDING			4L

#define SEARCH_PROGRESS_INFINITE	-1L

typedef struct _SEARCHRESULT
{
	NMHDR				nmhdr;
	void*				pData;
	PGPKeyServerEntry	keyserver;
	PGPError			error;
	long				flags;

}SEARCHRESULT, *PSEARCHRESULT;

typedef struct _SEARCHPROGRESS
{
	NMHDR	nmhdr;
	char	message[256];
	long	step;
	long	total;
}SEARCHPROGRESS, *PSEARCHPROGRESS;

typedef struct _SEARCHABORT
{
	NMHDR		nmhdr;
	PGPError	error;
}SEARCHABORT, *PSEARCHABORT;

typedef struct _SEARCHSECURE
{
	NMHDR			nmhdr;
	BOOL			secure;
	char			szServerName[256];
	PGPKeyRef		keyAuth;
}SEARCHSECURE, *PSEARCHSECURE;


typedef struct _SIZEREQUEST
{
	NMHDR	nmhdr;
	int		delta;
}SIZEREQUEST, *PSIZEREQUEST;


// call this function before calling any 
// Search Functions
BOOL 
InitSearch(	void );

void 
CloseSearch( void );

// This function creates a Search Control 
// It places itself in the upper left corner
// of the parent control. 
//
HWND 
CreateSearch(HINSTANCE hInstance, 
			 HWND hwndParent );

#endif#d