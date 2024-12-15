/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.  
   
	$Id: Choice.h,v 1.6 1998/08/11 15:19:58 pbj Exp $
____________________________________________________________________________*/
 
#ifndef _CHOICE_H
#define _CHOICE_H


typedef struct _CHOICE
{
	HWND hwnd;
	struct _CHOICE* next;

}CHOICE, *PCHOICE;


PCHOICE NewChoice(HWND hwndParent);
PCHOICE RemoveLastChoice(HWND hwndParent, HWND* hwndChoice);
int		ChoiceCount(HWND hwndParent);
PCHOICE	FirstChoice(HWND hwndParent);
PCHOICE NextChoice(HWND hwndParent, PCHOICE choice);

LRESULT 
CALLBACK 
SearchSubclassWndProc(HWND hwnd, 
					UINT msg, 
					WPARAM wParam, 
					LPARAM lParam);


#endif