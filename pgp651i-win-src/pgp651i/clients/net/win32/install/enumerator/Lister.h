/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	

	$Id: Lister.h,v 1.24 1999/03/30 23:12:37 philipn Exp $
____________________________________________________________________________*/
#pragma message( "Compiling " __FILE__ ) 
#pragma message( "Last modified on " __TIMESTAMP__ ) 

#pragma once

#include <windows.h>

/* Window procedure function prototypes			*/
LRESULT CALLBACK WndProc		(HWND,
								UINT,
								WPARAM,
								LPARAM);

BOOL CALLBACK ListerDlgProc		(HWND,
								UINT,
								WPARAM,
								LPARAM);

BOOL CALLBACK HelpDlgProc		(HWND,
								UINT,
								WPARAM,
								LPARAM);

BOOL CALLBACK RebindDlgProc		(HWND,
								UINT,
								WPARAM,
								LPARAM);

/* Message handler function prototypes			*/
void Lister_OnDestroy			(HWND);

BOOL Lister_OnCreate			(HWND,
								LPCREATESTRUCT);

BOOL Lister_InitDialog			(HWND,
								HWND,
								LPARAM);

BOOL Lister_OnCommand			(HWND,
								int,
								HWND,
								UINT);

/* Function prototypes							*/
BOOL		SetSelectedItems		(HWND, BOOL);

BOOL		SetSelectedItems95		(HWND hwnd, LPSTR *RealMac);

BOOL		SetbWan					(HWND);

BOOL		SetRealMac				(LPSTR RealMac);

LPTSTR		ResolveRealName			(LPTSTR);

LPTSTR		ResolveServiceName		(LPTSTR);

BOOL		CreateImageList			();

BOOL		IsCorrectVersion		(void);

LPTSTR		GetTxtAfterLastSlash	(LPTSTR);

LPTSTR		GetTxtAfterFirstSlash	(LPTSTR);

BOOL		BindReview				(HWND);

BOOL		InstallInfFile			(HWND);

BOOL		RemoveInfFile			(void);

static
HKEY		sParseRegKeyString		(LPSTR, LPSTR*);

BOOL		CopyRegistryKeyValues	(LPSTR,	LPSTR);

BOOL		DisableBinding			(LPTSTR);

void		SetState				(void);

BOOL		AmIBoundToTcpip95		(char *);

BOOL		ConfigSystemNetTrans	(void);

BOOL		ConfigSystemNet			(void);

BOOL		ConfigEnumNet			(void);

BOOL		ConfigEnumNetwork		(void);

BOOL		Remove95				(char *, int);

BOOL		RemoveNT				(void);

BOOL		StopService				(LPSTR);

BOOL		CleanRegNT				(void);

BOOL		AddStringToMultiString	(HKEY, LPSTR, LPSTR);

BOOL		DeleteStringFromMultiString (HKEY, LPSTR, LPSTR);

int			CheckNTBindStatus		(void);

BOOL		RemoveInf				(void);

BOOL		AddInf					(void);

BOOL		RebootYesNo				(void);

char*		SetExePath				();

BOOL		PutSAinRunKey			();

BOOL		DisableExtraProtocols	();

BOOL		MultiStrStrCmp			(LPSTR, LPSTR);

BOOL		NeedReboot				();

/*NetSetupReviewBindings function prototype		*/
typedef int __stdcall NetReviewFunc		(/*Handle of parent window*/
										HWND hwndParent, 
										/*Mode flags*/
										DWORD dwBindFlags);

/*netcfg exported function prototypes			*/
typedef int __stdcall NetInstallFunc	(/*Handle of parent window*/
										HWND hwndParent,
										/*Name of product to be installed*/
										PCWSTR pszInfOption,
										/*Path of INF file*/
										PCWSTR pszInfName,
										/*Source file path*/
										PCWSTR pszInstallPath,
										/*INF symbols; form name1\0value1\0...namen\0valuen\0\0*/
										PCWSTR plszInfSymbols,
										/*Mode flags*/
										DWORD dwInstallFlags, 
										/*Return*/
										PDWORD pdwReturn);

/* NetSetupComponentRemove function prototype*/
typedef int __stdcall NetRemoveFunc		(/*Handle of parent window*/
										HWND hwndParent,		
										/*Name of product to be removed*/
										PCWSTR pszInfOption,	
										/*Mode flags*/
										DWORD dwInstallFlags,
										/*Return value*/
										PDWORD pdwReturn);

typedef struct _RegSearchState RegSearchState;

struct _RegSearchState
{
	RegSearchState *nextState;

	HKEY hKey;
	DWORD dwKeyIndex;
	DWORD dwValueIndex;
	BOOL bMatch;
};

/*

	RegistrySearch: Looks for a registry key with a certain value name
	                and data value. You can call this function until
					it quits returning TRUE.

	szName		- Value name to search for
	szValue		- Data value to search for (must be a string)
	szSubKey	- Initialize to "", if RegistrySearch returns TRUE,
				  this will hold the subkey where the match was found
	pState		- Pointer to the search state

*/

BOOL RegistrySearch(char *szName, char *szValue, 
					char *szSubKey, RegSearchState *pState);

/*

	FreeRegSearchState: Frees the search state structure. Call this
	                    function after finishing with the RegistrySearch
						function.

	pState	- Pointer to the search state
*/

void FreeRegSearchState(RegSearchState *pState);
