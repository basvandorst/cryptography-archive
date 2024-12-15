/*_____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: prototype.h,v 1.3 1999/05/18 23:38:58 philipn Exp $
_____________________________________________________________________________*/

#if defined(__cplusplus)
extern "C" {
#endif

	INT		PGPclStartMemLockDriver			(VOID);
	BOOL	WINAPI MyGetOpenFileName32		(HWND, LPSTR, LPSTR, LPSTR, LPSTR);
	BOOL	WINAPI MyBrowseForFolder32		(HWND, LPSTR);
	UINT	CALLBACK CenterOpenFileName		(HWND, UINT, WPARAM, LPARAM);
	INT		CALLBACK CenterBrowseForFolder	(HWND, UINT, LPARAM, LPARAM);
	DWORD	WINAPI ResUtilStopResourceService(LPCTSTR);
	INT		StopService						(LPSTR);
	INT		StartServ				    	(LPSTR);
	VOID	RefreshStartMenu				();
	BOOL	CopyRegistryKeyValues			(LPSTR,	LPSTR);
	static
	HKEY	sParseRegKeyString				(LPSTR, LPSTR*);
	int		UninstInitialize				(HWND, HANDLE, LONG);
	void	UninstUnInitialize				(HWND, HANDLE, LONG);
	BOOL	CreateAndWait					(char* CommandLine, LPSTR CmdExe, LPSTR ERRORSTRING);
	void	DelSrv							(int arg);
	void	AddSrv							(int arg);
	void	errorHandler					(char* s, DWORD err);
	BOOL	IsWin95OSR2Compatible			();
	BOOL	IsWin2000Compatible				();
	enum	WinVersion	GetWindowsVersion	();
	INT		CreateIISVDir					(int arg);

#if defined(__cplusplus)
}
#endif

__inline UINT 
GetLowWord(DWORD dw)
{
	return (UINT) (dw & 0x0000FFFF);
}

enum WinVersion 
{
	Win95OSR1, 
	Win95OSR2, 
	Win98OrFuture, 
	WinNT3, 
	WinNT4NoSp, 
	WinNT4Sp1, 
	WinNT4Sp2, 
	WinNT4Sp3, 
	WinNT4PostSp3, 
	WinNTPost4, 
	Win2000, 
	WinUnknown, 
	Uninitialized
};

