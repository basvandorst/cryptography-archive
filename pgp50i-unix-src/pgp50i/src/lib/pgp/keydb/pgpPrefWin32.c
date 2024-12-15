/*
 * pgpPrefWin32.c -- Access preference database for Win32 and Win3.1
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpPrefWin32.c,v 1.11.2.3 1997/06/07 09:50:30 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpKDBint.h"
#include "pgpEnv.h"
#include "pgpPrefsInt.h"

#include <windows.h>

/*	Size of buffer to hold pref strings; will be doubled if won't fit */
#define INITLEN			128

/*	Name for holding register/init file keys */
#define APPNAME			"PGP"

/*	Registry key */
#define PGPLIBKEY		"Software\\PGP\\PGPlib"


#if USE_REGISTRY

/*
 * Return a newly allocated profile string, the value for the specified key.
 * If no such key is in the preference database, return a newly allocated
 * string holding the dflt value.
 * Return NULL on a fatal error.
 */
static char *
getprofstring (const char *key, PGPError *error)
{
		char		*buf = NULL;					/*	Allocated return buffer */
		long				buflen;							/*	Size of buf */
		long				result;							/*	Return code from Windows */
		HKEY				hKey;								/*	Handle to our library key */

		*error = 0;
		result = RegOpenKey (HKEY_CURRENT_USER, PGPLIBKEY, &hKey);
		if (result == ERROR_SUCCESS) {
				result = RegQueryValueEx (hKey, key, 0, NULL, NULL, &buflen);
				if (result == ERROR_SUCCESS) {
						buf = (char *) pgpMemAlloc (buflen);
						if (!buf) {
								*error = PGPERR_NOMEM;
								RegCloseKey (hKey);
								return NULL;
						}
						RegQueryValueEx (hKey, key, 0, NULL, buf, &buflen);
				}
				RegCloseKey (hKey);
		}
		if (!buf)
				*error = PGPERR_GENERIC;		/*	no such key */
		return buf;
}

/*	Set a profile string using the specifed key to the specified value */
static PGPError
setprofstring (const char *key, const char *val)
{
		long				result;							/*	Return code from Windows */
		HKEY				hKey;								/*	Handle to our library key */
#if 0
		DWORD				dwdum;								/*	Unused return value */

		result = RegCreateKeyEx (HKEY_CURRENT_USER, PGPLIBKEY, 0, NULL,
								REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwdum);
#else
		result = RegCreateKey (HKEY_CURRENT_USER, PGPLIBKEY, &hKey);
#endif
		if (result != ERROR_SUCCESS)
				return PGPERR_GENERIC;
		result = RegSetValueEx (hKey, key, 0, REG_SZ, val, strlen(val)+1);
		RegCloseKey (hKey);
		if (result != ERROR_SUCCESS)
				return PGPERR_GENERIC;
		return PGPERR_OK;
}


#else /*	USE_REGISTRY */

/*
 * Return a newly allocated profile string, the value for the specified key.
 * If no such key is in the preference database, return a newly allocated
 * string holding the dflt value.
 * Return NULL on a fatal error.
 */
static char *
getprofstring (const char *key, PGPError *error)
{
		char *buf;
		long buflen = INITLEN;
		long nstr;
		static char dummydflt = "";

		*error = 0;
		for ( ; ; ) {
				buf = (char *) pgpMemAlloc (buflen);
				if (!buf) {
						*error = PGPERR_NOMEM;
						return NULL;
				}
				nstr = GetProfileString (APPNAME, key, dummydflt, buf, buflen);
				if (nstr == 0) {
						pgpMemFree (buf);
						*error = PGPERR_GENERIC;
						return NULL;
				}
				if (nstr < buflen-1)
						return buf;
				buflen *= 2;
				pgpMemFree (buf);
		}
}

/*	Set a profile string using the specifed key to the specified value */
static PGPError
setprofstring (const char *key, const char *val)
{
		if (!WriteProfileString (APPNAME, key, val))
				return PGPERR_GENERIC;				/*	XXX */
		return PGPERR_OK;
}

#endif /*	USE_REGISTRY */


PGPError
pgpSetDefaultPrefsInternal (void)
{
		return PGPERR_OK;
}

PGPError
pgpSetDefaultKeyPathInternal (void)
{
		return PGPERR_OK;
}

/*	Read preferences and put them into the environment */
PGPError
pgpLoadPrefsInternal (void)
{
		PGPPrefNameNumber		pni;
		PGPPrefNameString		psi;
		PGPPrefNameBoolean		pbi;
		char						*pstr;
		PGPError						error;

		for (pni=kPGPPrefNumFirst; pni<kPGPPrefNumLast; ++pni) {
				NumPrefEntry *pe;
				long val;
			
				pe = &numPrefEntry(pni);
				pstr = getprofstring (pe->prefName, &error);
				if (pstr) {
						if (sscanf (pstr, "%i", &val) == 1) {
								pgpenvSetInt (pgpEnv, pe->pgpEnvSelector, val,
																PGPENV_PRI_FORCE);
						}
						pgpMemFree (pstr);
				}
		}
		for (psi=kPGPPrefStrFirst; psi<kPGPPrefStrLast; ++psi) {
				StringPrefEntry *pe;
			
				pe = &strPrefEntry(psi);
				pstr = getprofstring (pe->prefName, &error);
				if (pstr) {
						pgpenvSetString (pgpEnv, pe->pgpEnvSelector, pstr,
																PGPENV_PRI_FORCE);
						pgpMemFree (pstr);
				}
		}
		for (pbi=kPGPPrefBoolFirst; pbi<kPGPPrefBoolLast; ++pbi) {
				BoolPrefEntry *pe;
				Boolean boolVal;
			
				pe = &boolPrefEntry(pbi);
				pstr = getprofstring (pe->prefName, &error);
				if (pstr) {
						boolVal = _stricmp (pstr, "true")==0 || _stricmp (pstr, "t")==0;
						pgpenvSetInt (pgpEnv, pe->pgpEnvSelector, boolVal,
														PGPENV_PRI_FORCE);
						pgpMemFree (pstr);
				}
		}
		return PGPERR_OK;
}

/*	Save preferences which got set in environment, to disk */
PGPError
pgpSavePrefsInternal (void)
{
		PGPPrefNameNumber		pni;
		PGPPrefNameString		psi;
		PGPPrefNameBoolean		pbi;
		int									pri;
		char								buf[20];
		PGPError						error;

		for (pni=kPGPPrefNumFirst; pni<kPGPPrefNumLast; ++pni) {
				NumPrefEntry *pe;
				long val;
			
				pe = &numPrefEntry(pni);
				val = pgpenvGetInt (pgpEnv, pe->pgpEnvSelector, &pri, NULL);
				if (pri >= PGPENV_PRI_FORCE) {
						sprintf (buf, "%15d", val);
						error = setprofstring (pe->prefName, buf);
				}
		}
		for (psi=kPGPPrefStrFirst; psi<kPGPPrefStrLast; ++psi) {
				StringPrefEntry *pe;
				char const *pstr;
			
				pe = &strPrefEntry(psi);
				pstr = pgpenvGetString (pgpEnv, pe->pgpEnvSelector, &pri, NULL);
				if (pri >= PGPENV_PRI_FORCE) {
						error = setprofstring (pe->prefName, pstr);
				}
		}
		for (pbi=kPGPPrefBoolFirst; pbi<kPGPPrefBoolLast; ++pbi) {
				BoolPrefEntry *pe;
				Boolean bool;
			
				pe = &boolPrefEntry(pbi);
				bool = pgpenvGetInt (pgpEnv, pe->pgpEnvSelector, &pri, NULL);
				if (pri >= PGPENV_PRI_FORCE) {
						char const *pstr = bool ? "true" : "false";
					
						error = setprofstring (pe->prefName, pstr);
				}
		}
		return PGPERR_OK;
}


/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
