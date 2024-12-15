/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: ListMng.h,v 1.3 1999/03/10 02:44:04 heller Exp $
____________________________________________________________________________*/
#ifndef Included_LISTMNG_h	/* [ */
#define Included_LISTMNG_h

#include "OwnDraw.h"

#define NUMCOLUMNS 4
#define LISTBOX 0

typedef struct _liststruct
{
	HWND hwnd;
	HWND hwndlist;
	HWND hwndtext[NUMCOLUMNS];
	int colwidth[NUMCOLUMNS];
	float *colratio;
} LISTSTRUCT;

BOOL InitList(HWND hwnd,LISTSTRUCT *ls,char **ColText,float *ColRatio);
int SetListCursor(HWND hwndList,int index);
int AddAnItem(HWND hwndList,DRAWDATA *dd);
void MoveList(LISTSTRUCT *ls,int Width,int Height);

#endif /* ] Included_LISTMNG_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
