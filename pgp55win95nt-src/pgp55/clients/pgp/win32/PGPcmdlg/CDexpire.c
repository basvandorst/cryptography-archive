/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	PGPexpire.c - product timeout routines (from Brett Thomas)
	

	$Id: CDexpire.c,v 1.5 1997/09/12 17:41:23 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

#include "pgpcdlgx.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#define PGP_DEMO_DAYS_EVAL		30
#define PGP_DEMO_DAYS_PREDELETE	30

#if PGP_DEMO

/* Windows registry key for expiration date */

#define EVAL_TIMEOUT_KEY	"Software\\PGP\\PGP"
#define EVAL_TIMEOUT_VALUE	"55Trial"

/* RunOnce settings to delete library files */

#define RUN_ONCE	"Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce"

static void DeleteTimeoutFile(char *szDelFile);

#endif	// PGP_DEMO


char Months[14][4] = {"\0",
					 "JAN",
					 "FEB",
					 "MAR",
					 "APR",
					 "MAY",
					 "JUN",
					 "JUL",
					 "AUG",
					 "SEP",
					 "OCT",
					 "NOV",
					 "DEC",
					 "\0"};

static void TranslateDate(unsigned short *Month,
				   unsigned short *Day,
				   unsigned short *Year,
				   char *DateString);

static unsigned short FindMonth(char *MonthName);
static void DisplayTimeoutMessage(HWND hwnd);

// This just makes sure that BETA_TIMEOUT isn't > 0 at the same
// time PGP_DEMO is defined

#if PGP_DEMO
#define BETA_TIMEOUT        0
#else
// This is the value to change for beta timeout
#define BETA_TIMEOUT		0
#endif

BOOL PGPcdExport PGPcomdlgIsExpired(HWND hwnd)
{
	BOOL IsTimedOut = FALSE;
#if BETA_TIMEOUT > 0
	unsigned short month, day, year;
	struct tm tm, *ptm;
	time_t TimeNow, TimeoutTime;

	//Get the compile date, in usable form:
	TranslateDate(&month, &day, &year, __DATE__);

	//Seed the tm structure with the current time:
	time(&TimeNow);
	ptm = localtime(&TimeNow);
	memcpy(&tm, ptm, sizeof(tm));

	//Set the dates to the compile time:
	tm.tm_mon = month - 1;
	tm.tm_mday = day;
	tm.tm_year = year - 1900;

	//Turn the compile time into number of secs since 1/1/1970:
	TimeoutTime = mktime(&tm);

	//Increment it plus the number of days in the beta timeout times the 
	TimeoutTime += BETA_TIMEOUT * (60 * 60 * 24); //number of seconds in a day

	if(TimeoutTime < TimeNow)
	{
		IsTimedOut = TRUE;
		DisplayTimeoutMessage(hwnd);
	}
#endif	// BETA_TIMEOUT > 0

#if PGP_DEMO

	// The timeout starts at 30 days, which only disables encrypt and sign
	// After 30 days, display nag screen
	// After 60 days (TimeOut + 30 days), time out everything and 
	//										delete library DLL's
	
	IsTimedOut = PGPcomdlgEvalExpired(hwnd, PGPCOMDLG_ALLEXPIRED);
	if (IsTimedOut)
	{
		DeleteTimeoutFile("bn.dll");
		DeleteTimeoutFile("simple.dll");
		DeleteTimeoutFile("pgp.dll");
		DeleteTimeoutFile("keydb.dll");
		DisplayTimeoutMessage(hwnd);
	}

	if (PGPcomdlgEvalExpired(hwnd, PGPCOMDLG_ENCRYPTSIGNEXPIRED) && 
			!IsTimedOut)
		PGPcomdlgNag(hwnd, NULL, NULL);

#endif	// PGP_DEMO

	return(IsTimedOut);
}


#if PGP_DEMO

BOOL PGPcdExport PGPcomdlgEvalExpired (HWND hwnd, int nIndex)
{
	HKEY hkey;
	PGPTime TimeOut = 0;
	PGPTime Today = 0;
	BOOL NeedInitialization = TRUE;
	DWORD disposition;
	BOOL EvalTimeOut = FALSE;

    TimeOut = pgpGetTime();
	Today = TimeOut;

	TimeOut += PGP_DEMO_DAYS_EVAL * (60 * 60 * 24);

	if(RegCreateKeyEx(HKEY_CURRENT_USER,
					  EVAL_TIMEOUT_KEY,
					  0,
					  "",
					  REG_OPTION_NON_VOLATILE,
					  KEY_ALL_ACCESS,
					  NULL,
					  &hkey,
					  &disposition) == ERROR_SUCCESS)
	{
		DWORD Size,Type;

		Size = sizeof(TimeOut);
		if(RegQueryValueEx(hkey, 
						   EVAL_TIMEOUT_VALUE, 
						   NULL, 
						   &Type, 
						   (BYTE*) &TimeOut, 
						   &Size) != ERROR_SUCCESS)
		{
			/*Assume it's a new key...*/
			RegSetValueEx(hkey,
						  EVAL_TIMEOUT_VALUE,
						  0,
						  REG_DWORD,
						  (BYTE *) &TimeOut,
						  sizeof(TimeOut));
		}
		RegCloseKey(hkey);
	}

	// The timeout starts at 30 days, which only disables encrypt and sign
	// After 30 days, display nag screen
	// After 60 days (TimeOut + 30 days), time out everything and 
	//										delete library DLL's
	if (nIndex == PGPCOMDLG_ALLEXPIRED)
		TimeOut += PGP_DEMO_DAYS_PREDELETE*24*60*60;

	if (Today > TimeOut)
		EvalTimeOut = TRUE;

	return(EvalTimeOut);
}

#endif	// PGP_DEMO


static void TranslateDate(unsigned short *Month,
				   unsigned short *Day,
				   unsigned short *Year,
				   char *DateString)
{
	char *BlockStart, *BlockEnd, *TempBuffer;

	assert(DateString);
	assert(Year);
	assert(Day);
	assert(Month);

	*Month = *Day = *Year = 0;

	if(TempBuffer = malloc((strlen(DateString) + 1) * sizeof(char)))
	{
		strcpy(TempBuffer, DateString);
		BlockStart = BlockEnd = TempBuffer;
		while(BlockEnd && *BlockEnd && !isspace(*BlockEnd))
			++BlockEnd;

		if(BlockEnd && *BlockEnd)
		{
			*BlockEnd = '\0';
			*Month = FindMonth(BlockStart);

			BlockStart = ++BlockEnd;
			while(BlockStart && *BlockStart && isspace(*BlockStart))
				++BlockStart;

			BlockEnd = BlockStart;

			while(BlockEnd && *BlockEnd && !isspace(*BlockEnd))
				++BlockEnd;

			if(BlockEnd && *BlockEnd)
			{
				*BlockEnd = '\0';
				*Day = atoi(BlockStart);

				BlockStart = ++BlockEnd;

				*Year = atoi(BlockStart);
			}

		}
		free(TempBuffer);		
	}
}

static unsigned short FindMonth(char *MonthName)
{
	unsigned short MonthNum = 0;
	int x;

	assert(MonthName);

	for(x = 1; Months[x][0] && !MonthNum; ++x)
	{
		if(stricmp(Months[x], MonthName) == 0)
			MonthNum = x;
	}

	return(MonthNum);
}

static void DisplayTimeoutMessage(HWND hwnd)
{
#if BETA_TIMEOUT > 0
	char buf[1024];
#elif PGP_DEMO
	char buf[1024];
#endif

#if BETA_TIMEOUT > 0

	strcpy(buf, "Thank you for testing Pretty Good Privacy.  The 30\n"
				"day beta period has expired.  Please order your\n"
				"permanent copy today, or uninstall PGP through the\n"
				"Windows Control Panel-Add/Remove Programs applet.\n"
				"Call 1-800-536-2664 or visit http://www.pgp.com\n"
				"to order a registered copy.\n\n");

	MessageBox(hwnd, buf, "Beta Expiration", MB_OK);

#endif	// BETA_TIMEOUT > 0

#if PGP_DEMO

	strcpy(buf, "Thank you for evaluating Pretty Good Privacy.  The 60\n"
				"day evaluation period has expired.  Please order your\n"
				"permanent copy today.\n\n"
				"Call 1-800-536-2664 or visit http://www.pgp.com\n"
				"to order a registered copy.\n\n");

	MessageBox(hwnd, buf, "Evaluation Expiration", MB_OK);

#endif	// PGP_DEMO
}


#if PGP_DEMO

static void DeleteTimeoutFile(char *szDelFile)
{
	HKEY hkey;
	char szSystemDir[MAX_PATH];
	char szCommand[MAX_PATH*2];
	DWORD Size;
	int nError;

	GetSystemDirectory(szSystemDir, MAX_PATH);
	wsprintf(szCommand, "command /c del %s\\%s", szSystemDir, szDelFile);

	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
					  RUN_ONCE,
					  0,
					  KEY_SET_VALUE,
					  &hkey) == ERROR_SUCCESS)
	{
		Size = strlen(szCommand)+1;
		nError = RegSetValueEx(	hkey,
								szDelFile,
								0,
								REG_SZ,
								szCommand,
								Size);
		RegCloseKey(hkey);
	}
	
	return;
}

#endif	// PGP_DEMO
