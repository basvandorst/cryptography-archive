 
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
EditSubclassWndProc(HWND hwnd, 
					UINT msg, 
					WPARAM wParam, 
					LPARAM lParam);


#endif