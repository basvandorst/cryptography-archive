#ifndef _SEARCHCOMMON_H
#define _SEARCHCOMMON_H

#include "pgpKeyServerPrefs.h"

#define MAX_YEAR	2099
#define MIN_YEAR	1970

typedef struct _SEARCHPATH
{
	PGPPrefRef			prefRef;
	PGPKeyServerEntry*	keyserverList;
	PGPUInt32			keyserverCount;

}SEARCHPATH,*PSEARCHPATH;

void HANDLE_IDC_ATTRIBUTE(	HWND hwnd, 
							UINT msg, 
							WPARAM wParam, 
							LPARAM lParam);

void HANDLE_IDC_MONTH(	HWND hwnd, 
						UINT msg, 
						WPARAM wParam, 
						LPARAM lParam);

void InitializeControlValues(HWND hwnd, int defaultSelection);

void ChangeAttributeSelection(HWND hwnd);



#endif