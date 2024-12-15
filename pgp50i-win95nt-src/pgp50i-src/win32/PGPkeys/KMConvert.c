/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//: KMConvert.c - miscellaneous conversion routines
//
//	$Id: KMConvert.c,v 1.4 1997/05/27 18:39:25 pbj Exp $
//

#include <windows.h>
#include <string.h>
#include "pgpkeys.h"
#include "resource.h"

// external globals
extern HINSTANCE g_hInst;


//----------------------------------------------------|
// Convert tm to SystemTime

void TimeToSystemTime (struct tm *ptm, SYSTEMTIME* pst) {

	pst->wYear = ptm->tm_year + 1900;
	pst->wMonth = ptm->tm_mon + 1;
	pst->wDay = ptm->tm_mday;
	pst->wDayOfWeek = ptm->tm_wday;
	pst->wHour = ptm->tm_hour;
	pst->wMinute = ptm->tm_min;
	pst->wSecond = ptm->tm_sec;
	pst->wMilliseconds = 0;
}


//----------------------------------------------------|
// Convert time to string format based on system settings

void KMConvertTimeToString (PGPTime* pTime, LPSTR sz, INT iLen) {

	SYSTEMTIME systemtime;
	struct tm *ptm;

	ptm = pgpLocalTime (pTime);
	if (ptm) {
		TimeToSystemTime (ptm, &systemtime);
		GetDateFormat (LOCALE_USER_DEFAULT, DATE_SHORTDATE, &systemtime,
			NULL, sz, iLen);
	}
	else
		LoadString (g_hInst, IDS_INVALIDDATE, sz, iLen);
}


//----------------------------------------------------|
// Convert appropriate half of KeyID from big endian to
// character string format.
// NB: sz must be at least 11 bytes long

void KMConvertStringKeyID (LPSTR sz) {
	INT i;
	ULONG ul;

	// convert appropriate half of keyid to little endian
	ul = 0;
	for (i=0; i<4; i++) {
		ul <<= 8;
		ul |= ((ULONG)sz[i+4] & 0xFF);
	}
	// convert to string
	wsprintf (sz, "0x%08lX", ul);
}

/*
void KMConvertStringKeyID (UINT uAlgorithm, LPSTR sz) {
	INT i, iOffset;
	ULONG ul;

	switch (uAlgorithm) {

	case PGP_PKALG_RSA :
		iOffset = 4;
		break;

	default :
		iOffset = 0;
		break;
	}

	// convert appropriate half of keyid to little endian
	ul = 0;
	for (i=0; i<4; i++) {
		ul <<= 8;
		ul |= ((ULONG)sz[i+iOffset] & 0xFF);
	}
	// convert to string
	wsprintf (sz, "0x%08lX", ul);
}
*/

//----------------------------------------------------|
// Convert Fingerprint from string format to presentation format
// NB: sz must be at least 42 bytes long for RSA and 52 bytes
//     for DSA keys

void KMConvertStringFingerprint (UINT uAlgorithm, LPSTR sz) {
	INT i;
	UINT u;
	char szBuf[20];
	char* p;

	switch (uAlgorithm) {
	case PGP_PKALG_RSA :
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

	case PGP_PKALG_DSA :
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


//----------------------------------------------------|
// Convert trust from weird PGP values to 0-3 scale

UINT KMConvertFromPGPTrust (UINT uPGPTrust) {
	switch (uPGPTrust & PGP_KEYTRUST_MASK) {
	case PGP_KEYTRUST_UNDEFINED	:
	case PGP_KEYTRUST_UNKNOWN :
	case PGP_KEYTRUST_NEVER :
		return 0;
	case PGP_KEYTRUST_MARGINAL :
		return 1;
	case PGP_KEYTRUST_COMPLETE :
	case PGP_KEYTRUST_ULTIMATE :
		return 2;
	}
}


//----------------------------------------------------|
// Convert validity from weird PGP values to 0-2 scale

UINT KMConvertFromPGPValidity (UINT uPGPValidity) {
	switch (uPGPValidity) {
	case PGP_VALIDITY_UNKNOWN :
	case PGP_VALIDITY_INVALID :
//	case PGP_VALIDITY_UNTRUSTED :
		return 0;
	case PGP_VALIDITY_MARGINAL :
		return 1;
	case PGP_VALIDITY_COMPLETE :
		return 2;
	}
}


//----------------------------------------------------|
// Convert trust from 0-3 scale to PGP values

UINT KMConvertToPGPTrust (UINT uTrust) {
	switch (uTrust & 0x03) {
	case 0 :
		return PGP_KEYTRUST_NEVER;
	case 1 :
		return PGP_KEYTRUST_MARGINAL;
	case 2 :
		return PGP_KEYTRUST_COMPLETE;
	case 3 :
		return PGP_KEYTRUST_ULTIMATE;
	}
}
