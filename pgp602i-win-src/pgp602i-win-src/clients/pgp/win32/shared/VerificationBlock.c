/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: VerificationBlock.c,v 1.3.2.1.2.1 1998/11/12 03:14:15 heller Exp $
____________________________________________________________________________*/

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>

#include "pgpKeys.h"
#include "pgpConfig.h"
#include "pgpErrors.h"
#include "pgpEncode.h"
#include "pgpUtilities.h"
#include "pgpPubTypes.h"
#include "pgpMem.h"

#include "Prefs.h"
#include "SharedStrings.h"


static void StdTimeToSystemTime(struct tm *ptm, SYSTEMTIME *pst) 
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


static void ConvertPGPTimeToString(PGPTime time,
								   char *dateString, 
								   PGPUInt32 dateStrLength,
								   char *timeString,
								   PGPUInt32 timeStrLength) 
{
	SYSTEMTIME	systemtime;
	time_t		ttTime;
	struct tm*	ptm;

	ttTime = PGPGetStdTimeFromPGPTime(time);
	ptm = localtime(&ttTime);

	StdTimeToSystemTime(ptm, &systemtime);

	GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &systemtime, 
		NULL, dateString, dateStrLength);

	GetTimeFormat(LOCALE_USER_DEFAULT, LOCALE_NOUSEROVERRIDE, &systemtime,
		NULL, timeString, timeStrLength);

	return;
}


static PGPBoolean GetKeyIDString(PGPKeyID keyID, 
								 char *idBuffer, 
								 PGPUInt32 bufferSize) 
{

	char tempBuffer[kPGPMaxKeyIDStringSize];

	if (bufferSize < 11) return FALSE;

	PGPGetKeyIDString(&keyID, kPGPKeyIDString_Abbreviated, tempBuffer);
	lstrcpy(idBuffer, "0x");
	lstrcat(idBuffer, &tempBuffer[2]);

	return TRUE;
}


PGPError CreateVerificationBlock(HINSTANCE hInst, 
								 PGPContextRef context,
								 PGPEventSignatureData *sigData, 
								 PGPBoolean wasEncrypted,
								 char **blockBegin,
								 char **blockEnd)
{
	PGPUInt32 validity;
	PGPBoolean isAxiomatic;
	PGPBoolean keyCanSign;
	PGPMemoryMgrRef memoryMgr;
	char dateString[256];
	char timeString[256];
	char tempString[256];
	PGPError err = kPGPError_NoErr;

	PGPValidatePtr(sigData);
	PGPValidatePtr(blockBegin);
	PGPValidatePtr(blockEnd);

	isAxiomatic = FALSE;
	keyCanSign = TRUE;

	memoryMgr = PGPGetContextMemoryMgr(context);

	*blockBegin = (char *) PGPNewData(memoryMgr,
								1024,
								kPGPMemoryMgrFlags_Clear);
								
	if (IsNull(*blockBegin))
		return kPGPError_OutOfMemory;

	*blockEnd = (char *) PGPNewData(memoryMgr,
							1024,
							kPGPMemoryMgrFlags_Clear);
								
	if (IsNull(*blockEnd))
		return kPGPError_OutOfMemory;

	LoadString(hInst, IDS_SIGSTATUS, tempString, sizeof(tempString));
	strcat(*blockBegin, tempString);

	if (sigData->signingKey != 0)
	    PGPGetKeyBoolean(sigData->signingKey, kPGPKeyPropIsSigningKey, 
			&keyCanSign);

	if (!keyCanSign)
		LoadString(hInst, IDS_ALGNOTSUPPORTED, tempString, 
			sizeof(tempString));
	else if (sigData->verified)
		LoadString(hInst, IDS_GOODSIG, tempString, sizeof(tempString));
	else if (sigData->signingKey == 0)
		LoadString(hInst, IDS_UNKNOWNSIG, tempString, sizeof(tempString));
	else
		LoadString(hInst, IDS_BADSIG, tempString, sizeof(tempString));

	strcat(*blockBegin, tempString);

	LoadString(hInst, IDS_SIGNER, tempString, sizeof(tempString));
	strcat(*blockBegin, tempString);

	// Get name and raw Validity number
	if (sigData->signingKey == 0)
	{
		char keyIDStr[11];

		GetKeyIDString(sigData->signingKeyID, keyIDStr, sizeof(keyIDStr));
		
		LoadString(hInst, IDS_UNKNOWNSIGNER, tempString, 
			sizeof(tempString));

		strcat(*blockBegin, tempString);
		strcat(*blockBegin, keyIDStr);
		strcat(*blockBegin,"\r\n");
	}
	else
	{
		int nameLength = kPGPMaxUserIDSize-1;
		char name[kPGPMaxUserIDSize];

		PGPGetPrimaryUserIDNameBuffer(sigData->signingKey,
			sizeof(name), name, &nameLength);

		strcat(*blockBegin, name);

		PGPGetKeyNumber(sigData->signingKey, kPGPKeyPropValidity,
			&validity);

	    PGPGetKeyBoolean(sigData->signingKey, kPGPKeyPropIsAxiomatic, 
			&isAxiomatic);

		if (sigData->keyRevoked)
			LoadString(hInst, IDS_REVOKEDKEY, tempString, 
				sizeof(tempString));
		else if (sigData->keyExpired)
			LoadString(hInst, IDS_EXPIREDKEY, tempString, 
				sizeof(tempString));
		else if (sigData->keyDisabled)
			LoadString(hInst, IDS_DISABLEDKEY, tempString, 
				sizeof(tempString));
		else if (!isAxiomatic)
		{
			if ((validity == kPGPValidity_Invalid) ||
					((validity == kPGPValidity_Marginal) && 
					MarginalIsInvalid(memoryMgr)))
			{
				LoadString(hInst, IDS_INVALIDKEY, tempString, 
					sizeof(tempString));
			}
			else
				strcpy(tempString, "\r\n");
		}
		else
			strcpy(tempString, "\r\n");

		strcat(*blockBegin, tempString);
	}

	LoadString(hInst, IDS_SIGDATE, tempString, sizeof(tempString));
	strcat(*blockBegin, tempString);

	ConvertPGPTimeToString(sigData->creationTime, dateString, 
		sizeof(dateString), timeString, sizeof(timeString));

	strcat(*blockBegin, dateString);
	strcat(*blockBegin, " ");
	strcat(*blockBegin, timeString);
	strcat(*blockBegin, "\r\n");

	LoadString(hInst, IDS_VERIFIED, tempString, sizeof(tempString));
	strcat(*blockBegin, tempString);

	ConvertPGPTimeToString(PGPGetTime(), dateString, sizeof(dateString),
		timeString, sizeof(timeString));

	strcat(*blockBegin, dateString);
	strcat(*blockBegin, " ");
	strcat(*blockBegin, timeString);
	strcat(*blockBegin, "\r\n");

	if (wasEncrypted)
	{
		LoadString(hInst, IDS_BEGINDECRYPTED, tempString, 
			sizeof(tempString));
		strcat(*blockBegin, tempString);
		LoadString(hInst, IDS_ENDDECRYPTED, tempString, 
			sizeof(tempString));
		strcpy(*blockEnd, tempString);
	}
	else
	{
		LoadString(hInst, IDS_BEGINVERIFIED, tempString, 
			sizeof(tempString));
		strcat(*blockBegin, tempString);
		LoadString(hInst, IDS_ENDVERIFIED, tempString, 
			sizeof(tempString));
		strcpy(*blockEnd, tempString);
	}

	return err;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
