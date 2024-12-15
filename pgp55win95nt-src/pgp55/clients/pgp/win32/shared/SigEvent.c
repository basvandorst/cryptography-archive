/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: SigEvent.c,v 1.10.4.1 1997/11/07 17:53:06 wjb Exp $
____________________________________________________________________________*/

#define PGPExport __declspec( dllexport )

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>

#include "pgpKeys.h"
#include "pgpConfig.h"
#include "pgpErrors.h"
#include "pgpEncode.h"
#include "pgpUtilities.h"
#include "..\include\PGPsc.h"
#include "pgpPubTypes.h"

void SigTimeToSystemTime (
	struct tm*	ptm, 
	SYSTEMTIME* pst) 
{
	pst->wYear = ptm->tm_year + 1900;
	pst->wMonth = ptm->tm_mon + 1;
	pst->wDay = ptm->tm_mday;
	pst->wDayOfWeek = ptm->tm_wday;
	pst->wHour = ptm->tm_hour;
	pst->wMinute = ptm->tm_min;
	pst->wSecond = ptm->tm_sec;
	pst->wMilliseconds = 0;
}

void ConvertSigTimeToString (
		PGPTime		Time, 
		LPSTR		sz, 
		INT			iLen) 
{
	SYSTEMTIME	systemtime;
	time_t		ttTime;
	struct tm*	ptm;

	ttTime = PGPGetStdTimeFromPGPTime (Time);
	ptm = localtime (&ttTime);

	SigTimeToSystemTime (ptm, &systemtime);
	GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &systemtime, 
		NULL, sz, iLen);
	strcat(sz," ");
	GetTimeFormat(LOCALE_USER_DEFAULT, LOCALE_NOUSEROVERRIDE, &systemtime,
		NULL, &(sz[strlen(sz)]),iLen-strlen(sz));
}

// SigEvent
//
// Call after kPGPEvent_SignatureEvent. Must link to PGPsc.
//
// Params:
//    d          Event data (cast)
//    fileName   fileName (or application name)

void SigEvent(PGPEventSignatureData *d,char *fileName)
{
	DWORD Validity;
	PGPBoolean bAxiomatic,bKeyCanSign;
	char szMsgStr[500];
	BOOL bMarginalInvalid;

	bAxiomatic=FALSE;
	bKeyCanSign=TRUE;

	// Icon information for PGPlog

	if(d->verified)
		strcpy(szMsgStr,"Good Signature\n");
	else
		strcpy(szMsgStr,"Bad Signature\n");

	// Filename (or Clipboard... or Eudora.. etc)
	sprintf(&(szMsgStr[strlen(szMsgStr)]),"%s\n",
		JustFile(fileName));

	// Get name and raw Validity number
	if((d->signingKey)==0)
	{
		strcat(szMsgStr,"(unknown signer)\n");
		Validity=kPGPValidity_Unknown ;
	}
	else
	{
		int namelen=kPGPMaxUserIDSize-1;
		char name[kPGPMaxUserIDSize];

		PGPGetPrimaryUserIDNameBuffer( d->signingKey,
			sizeof( name ), name, &namelen );
		sprintf(&(szMsgStr[strlen(szMsgStr)]),"%s\n",name);

		PGPGetKeyNumber(d->signingKey,kPGPKeyPropValidity,
			&Validity);

	    PGPGetKeyBoolean (d->signingKey, kPGPKeyPropIsAxiomatic, 
			&bAxiomatic);

	    PGPGetKeyBoolean (d->signingKey, kPGPKeyPropCanSign, 
			&bKeyCanSign);
	}

	// Decode validity into string

	if(bAxiomatic)
		strcat(szMsgStr,"Axiomatic Key\n");
	else switch(Validity)
	{
		case kPGPValidity_Marginal :
			strcat(szMsgStr,"Marginal Key\n");
			break;
		case kPGPValidity_Complete :
			strcat(szMsgStr,"Valid Key\n");
			break;
		default :
			strcat(szMsgStr,"Invalid Key\n");
			break;
	}

	// Output date and/or extra info
	if((d->signingKey)==0)
		strcat(szMsgStr,"Signing key not found");
	else if(!bKeyCanSign)
		strcat(szMsgStr,"Signing algorithm not supported");
	else if(d->verified==0)
		strcat(szMsgStr,"Bad Signature\n");
	else
	{
/*		sprintf(&(szMsgStr[strlen(szMsgStr)]), 
			"%s", ctime((time_t *)&d->creationTime));

		// Get rid of ctime \n
		szMsgStr[strlen(szMsgStr)-1]=0;*/

		GetMarginalInvalidPref(&bMarginalInvalid);

		ConvertSigTimeToString (
			d->creationTime,
			&(szMsgStr[strlen(szMsgStr)]),
			500-strlen(szMsgStr));

		if (d->keyRevoked)
			strcat(szMsgStr," (Revoked Key)");
		else if (d->keyExpired)
			strcat(szMsgStr," (Expired Key)");
		else if (d->keyDisabled)
			strcat(szMsgStr," (Disabled Key)");
		else if((Validity==kPGPValidity_Unknown)||
		   (Validity==kPGPValidity_Invalid))
			strcat(szMsgStr," (Invalid Key)");
		else if((bMarginalInvalid)&&
			(Validity==kPGPValidity_Marginal))
			strcat(szMsgStr," (Invalid Key)");
	}
				
	// Send DDEML msg to PGPlog
	SendPGPlogMsg(szMsgStr);
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
