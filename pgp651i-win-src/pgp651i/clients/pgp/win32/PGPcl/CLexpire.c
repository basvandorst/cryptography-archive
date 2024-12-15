/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	CLexpire.c - product timeout routines (from Brett Thomas)
	

	$Id: CLexpire.c,v 1.11.4.1 1999/06/09 17:51:17 build Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"

// project header files
#include "pgpclx.h"

// system header files
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#define PGP_DEMO_DAYS_EVAL		30
#define PGP_DEMO_DAYS_PREDELETE	30

extern HINSTANCE g_hInst;

#if PGP_DEMO

/* Windows registry key for expiration date */

#define EVAL_TIMEOUT_KEY	"Software\\Network Associates\\PGP"
#define EVAL_TIMEOUT_VALUE	"60Trial"

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
#define BETA_TIMEOUT		0  // This number represents the number of days
#endif                          //		till the beta expires from compile date
								// Setting this to 0 will turn off the time out feature,
								//		the default time out lenghth is 90 days

PGPError PGPclExport 
PGPclIsExpired(HWND hwnd)
{
	PGPError err = kPGPError_NoErr;
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
		err = kPGPError_FeatureNotAvailable;
		DisplayTimeoutMessage(hwnd);
	}
#endif	// BETA_TIMEOUT > 0

#if PGP_DEMO

	// The timeout starts at 30 days, which only disables encrypt and sign
	// After 30 days, display nag screen
	// After 60 days (TimeOut + 30 days), time out everything and 
	//										delete library DLL's
	
	err = PGPclEvalExpired(hwnd, PGPCOMDLG_ALLEXPIRED);
	if (IsPGPError (err))
	{
		pgpFixBeforeShip ("eval version file deletion");
///???		DeleteTimeoutFile("pgpsdk.dll");
///???		DeleteTimeoutFile("pgp.dll");
		DisplayTimeoutMessage(hwnd);
	}

	if (IsPGPError(PGPclEvalExpired(hwnd, PGPCOMDLG_ENCRYPTSIGNEXPIRED)) && 
			!IsntPGPError (err))
		PGPclNag(hwnd, NULL, NULL);

#endif	// PGP_DEMO

	return err;
}


#if PGP_DEMO

PGPError PGPclExport 
PGPclEvalExpired (HWND hwnd, int nIndex)
{
	HKEY hkey;
	PGPTime TimeOut = 0;
	PGPTime Today = 0;
	BOOL NeedInitialization = TRUE;
	DWORD disposition;
	PGPError err = kPGPError_NoErr;

    TimeOut = pgpGetTime();
	Today = TimeOut;

	TimeOut += PGP_DEMO_DAYS_EVAL * (60 * 60 * 24);

	if(RegCreateKeyEx(HKEY_LOCAL_MACHINE,
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
		err = kPGPError_FeatureNotAvailable;

	return err;
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
	char buf1[512];
	char buf2[512];
#elif PGP_DEMO
	char buf1[512];
	char buf2[512];
#endif

#if BETA_TIMEOUT > 0

	LoadString (g_hInst, IDS_BETATIMEOUT, buf1, sizeof(buf1));
	wsprintf (buf2, buf1, BETA_TIMEOUT);
	MessageBox (hwnd, buf2, "Beta Expiration", MB_OK);

#endif	// BETA_TIMEOUT > 0

#if PGP_DEMO

	LoadString (g_hInst, IDS_EVALTIMEOUT, buf1, sizeof(buf1));
	wsprintf (buf2, buf1, PGP_DEMO_DAYS_EVAL + PGP_DEMO_DAYS_PREDELETE);
	MessageBox(hwnd, buf2, "Evaluation Expiration", MB_OK);

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
