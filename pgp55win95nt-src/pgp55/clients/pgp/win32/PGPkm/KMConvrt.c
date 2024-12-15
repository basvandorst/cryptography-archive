/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	KMConvrt.c - miscellaneous conversion routines
	

	$Id: KMConvrt.c,v 1.6 1997/10/20 15:23:57 pbj Exp $
____________________________________________________________________________*/
#include "pgpPFLConfig.h"	

#include "pgpkmx.h"
#include <string.h>
#include <time.h>
#include "resource.h"

// external globals
extern HINSTANCE g_hInst;

//	________________________
//
//	Convert tm to SystemTime

VOID 
TimeToSystemTime (
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

//	______________________________________________________
//
//	Convert time to string format based on system settings

VOID
KMConvertTimeToString (
		PGPTime		Time, 
		LPSTR		sz, 
		INT			iLen) 
{
	SYSTEMTIME	systemtime;
	time_t		ttTime;
	struct tm*	ptm;

	ttTime = PGPGetStdTimeFromPGPTime (Time);
	ptm = localtime (&ttTime);

	if (ptm) {
		TimeToSystemTime (ptm, &systemtime);
		GetDateFormat (LOCALE_USER_DEFAULT, DATE_SHORTDATE, &systemtime, 
			NULL, sz, iLen);
	}
	else 
		LoadString (g_hInst, IDS_INVALIDDATE, sz, iLen);
}

//	____________________________________________________
//
//	Convert appropriate half of KeyID from big endian to 
//	character string format. 
//	NB: sz must be at least 11 bytes long

VOID 
KMConvertStringKeyID (LPSTR sz) 
{
	INT		i;
	ULONG	ul;

	// convert appropriate half of keyid to little endian
	ul = 0;
	for (i=0; i<4; i++) {
		ul <<= 8;
		ul |= ((ULONG)sz[i+4] & 0xFF);
	}
	// convert to string
	wsprintf (sz, "0x%08lX", ul);
}

//	___________________________________________________________
//
//	Convert Fingerprint from string format to presentation format
//	NB: sz must be at least 42 bytes long for RSA and 52 bytes
//     for DSA keys

VOID 
KMConvertStringFingerprint (
		UINT	uAlgorithm, 
		LPSTR	sz) 
{
	INT		i;
	UINT	u;
	CHAR	szBuf[20];
	CHAR*	p;

	switch (uAlgorithm) {
	case kPGPPublicKeyAlgorithm_RSA :
		memcpy (szBuf, sz, 16);
		p = sz;
		for (i=0; i<16; i+=2) {
			switch (i) {
			case 0:
				break;
			case 8:
				*p++ = ' ';
			default :
				*p++ = ' ';
				break;
			}
			u = ((unsigned long)szBuf[i] & 0xFF);
			u <<= 8;
			u |= ((unsigned long)szBuf[i+1] & 0xFF);
			wsprintf (p, "%04lX", u);
			p += 4;
		}
		break;

	case kPGPPublicKeyAlgorithm_DSA :
		memcpy (szBuf, sz, 20);
		p = sz;
		for (i=0; i<20; i+=2) {
			switch (i) {
			case 0:
				break;
			case 10:
				*p++ = ' ';
			default :
				*p++ = ' ';
				break;
			}
			u = ((unsigned long)szBuf[i] & 0xFF);
			u <<= 8;
			u |= ((unsigned long)szBuf[i+1] & 0xFF);
			wsprintf (p, "%04lX", u);
			p += 4;
		}
		break;

	default :
		lstrcpy (sz, "");
		break;
	}
}

//	________________________________________________
//
//	Convert trust from weird PGP values to 0-3 scale

UINT 
KMConvertFromPGPTrust (UINT uPGPTrust) 
{
	switch (uPGPTrust & kPGPKeyTrust_Mask) {
	case kPGPKeyTrust_Undefined	:
	case kPGPKeyTrust_Unknown :
	case kPGPKeyTrust_Never :
		return 0;
	case kPGPKeyTrust_Marginal :
		return 1;
	case kPGPKeyTrust_Complete :
	case kPGPKeyTrust_Ultimate :
		return 2;
	default :
		return 0;
	}
}

//	___________________________________________________
//
//	Convert validity from weird PGP values to 0-2 scale

UINT 
KMConvertFromPGPValidity (UINT uPGPValidity) 
{
	switch (uPGPValidity) {
	case kPGPValidity_Unknown :
	case kPGPValidity_Invalid :
		return KM_VALIDITY_INVALID;
	case kPGPValidity_Marginal :
		return KM_VALIDITY_MARGINAL;
	case kPGPValidity_Complete :
		return KM_VALIDITY_COMPLETE;
	default :
		return 0;
	}
}

//	__________________________________________
//
//	Convert trust from 0-3 scale to PGP values

UINT 
KMConvertToPGPTrust (UINT uTrust) 
{
	switch (uTrust & 0x03) {
	case 0 :
		return kPGPKeyTrust_Never;
	case 1 :
		return kPGPKeyTrust_Marginal;
	case 2 :
		return kPGPKeyTrust_Complete;
	case 3 :
		return kPGPKeyTrust_Ultimate;
	default :
		return 0;
	}
}


