/*
 * pgpPrefs.c -- Platform independent preference database code
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpPrefs.c,v 1.32.2.18.2.1 1997/07/08 02:24:14 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpKDBint.h"
#include "pgpEnv.h"
#include "pgpPrefsInt.h"
#include "pgpLeaks.h"

NumPrefEntry pgpNumPrefTable[kPGPPrefNumLast - kPGPPrefNumFirst] = {
		{ "Version",								PGPENV_VERSION,									3 },
		{ "CipherNum",								PGPENV_CIPHER,										1 },
		{ "HashNum",								PGPENV_HASH,										1 },
		{ "CompressAlg",						PGPENV_COMPRESSALG,							1 },
		{ "ArmorLines",							PGPENV_ARMORLINES,								0 },
		{ "CertDepth",								PGPENV_CERTDEPTH,								4 },
		{ "CompressQuality",				PGPENV_COMPRESSQUAL,						1 },
		{ "MailWordWrapWidth",				PGPENV_MAILWORDWRAPWIDTH,				70 },
		{ "MailPassCacheDuration",		PGPENV_MAILPASSCACHEDURATION,		120 },
		{ "SignCacheDuration",				PGPENV_SIGNCACHEDURATION,				120 },
		{ "DateOfLastSplashScreen",	PGPENV_DATEOFLASTSPLASHSCREEN,		0 },
		{ "HTTPKeyServerPort",				PGPENV_HTTPKEYSERVERPORT,		11371 }};

StringPrefEntry pgpStrPrefTable[kPGPPrefStrLast - kPGPPrefStrFirst] = {
		{ "DefaultKeyID",						PGPENV_MYNAME,						"" },
		{ "PubRing",								PGPENV_PUBRING,					"pubring.pkr" },
		{ "SecRing",								PGPENV_SECRING,					"secring.skr" },
#if UNIX
		{ "RandomDevice",						PGPENV_RANDOMDEVICE,		"/dev/random" },
#else
		{ "RandomDevice",						PGPENV_RANDOMDEVICE,		"" },
#endif
		{ "RandSeedFile",						PGPENV_RANDSEED,				NULL },
		{ "Language",								PGPENV_LANGUAGE,				"English" },
		{ "OwnerName",								PGPENV_OWNERNAME,				"" },
		{ "CompanyName",						PGPENV_COMPANYNAME,			"" },
		{ "LicenseNumber",						PGPENV_LICENSENUMBER,		"" },
	{ "HTTPKeyServerHost",	PGPENV_HTTPKEYSERVERHOST,"horowitz.surfnet.nl" },
#if UNIX
    { "LanguageFile",           PGPENV_LANGUAGEFILE,    "language50.txt" }};
#else
    { "LanguageFile",           PGPENV_LANGUAGEFILE,    "" }};
#endif

BoolPrefEntry pgpBoolPrefTable[kPGPPrefBoolLast - kPGPPrefBoolFirst] = {
		{ "Armor",										PGPENV_ARMOR,								TRUE },
		{ "EncryptToSelf",						PGPENV_ENCRYPTTOSELF,				FALSE },
		{ "Compress",								PGPENV_COMPRESS,						TRUE },
		{ "MailWordWrapEnable",			PGPENV_MAILWORDWRAPENABLE,		TRUE },
		{ "MailPassCacheEnable",		PGPENV_MAILPASSCACHEENABLE,	TRUE },
		{ "MailEncryptPGPMIME",			PGPENV_MAILENCRYPTPGPMIME,		FALSE },
		{ "MailSignPGPMIME",				PGPENV_MAILSIGNPGPMIME,			FALSE },
		{ "FastKeyGen",							PGPENV_FASTKEYGEN,						TRUE },
		{ "SignCacheEnable",				PGPENV_SIGNCACHEENABLE,			FALSE },
		{ "WarnOnMixRSADiffieHellman",		PGPENV_WARNONMIXRSAELGAMAL,	TRUE },
		{ "MailEncryptDefault",			PGPENV_MAILENCRYPTDEFAULT,		FALSE },
		{ "MailSignDefault",				PGPENV_MAILSIGNDEFAULT,			FALSE },
		{ "MarginallyValidWarning",	PGPENV_MARGINALLYVALIDWARNING,		FALSE },
		{ "FirstKeyGenerated",				PGPENV_FIRSTKEYGENERATED,		FALSE },
		{ "WarnOnRSARecipAndNonRSASigner",
																PGPENV_WARNONRSARECIPANDNONRSASIGNER, TRUE }};

DataPrefEntry pgpDataPrefTable[kPGPPrefDataLast - kPGPPrefDataFirst] = {
		{ "PubRingRef",							PGPENV_PUBRINGREF },
		{ "PrivRingRef",						PGPENV_PRIVRINGREF },
		{ "RandSeedData",						PGPENV_RANDSEEDDATA },
		{ "PGPkeysMacMainWinPos",		PGPENV_PGPKEYSMACMAINWINPOS },
		{ "PGPappMacPrivateData",		PGPENV_PGPAPPMACPRIVATEDATA },
		{ "PGPmenuMacAppSignatures",PGPENV_PGPMENUMACAPPSIGNATURES }};

		PGPError
pgpGetPrefNumber(
		PGPPrefNameNumber		prefName,
		long *								value)
{
		enum PgpEnvInts			pgpEnvSelector;
		PGPError						result = PGPERR_OK;
	
		pgpa((
				pgpaPrefNameNumberValid(prefName),
				pgpaAddrValid(value, long)));
	
		pgpEnvSelector = (enum PgpEnvInts)numPrefEntry(prefName).pgpEnvSelector;
		*value = (long)pgpenvGetInt(pgpEnv, pgpEnvSelector, NULL, &result);
	
		return result;
}

		PGPError
pgpGetPrefString(
		PGPPrefNameString		prefName,
		char *								buffer,
		size_t *						length)
{
		enum PgpEnvStrings		pgpEnvSelector;
		char const *				envStr;
		size_t								envStrLen;
		PGPError						result = PGPERR_OK;

		pgpa((
				pgpaPrefNameStringValid(prefName),
				pgpaAddrValid(length, size_t),
				pgpaAssert(*length >= 0)));
	
		pgpEnvSelector = (enum PgpEnvStrings)strPrefEntry(prefName).pgpEnvSelector;
		envStr = pgpenvGetString(pgpEnv, pgpEnvSelector, NULL, &result);
		if (envStr != NULL)
		{
				pgpa(pgpaStrValid(envStr));
			
				envStrLen = strlen(envStr);
				if (buffer == NULL || *length < envStrLen)
				{
						*length = envStrLen;
						return PGPERR_KEYDB_BUFFERTOOSHORT;
				}
				pgpa(pgpaAddrValid(buffer, char));
				*length = envStrLen;
				pgpCopyMemory(envStr, buffer, envStrLen);
		}
		return result;
}

		char *
pgpGetPrefCStringCopy(
		PGPPrefNameString		prefName)
{
		enum PgpEnvStrings		pgpEnvSelector;
		char const *				envStr;
		size_t								envStrLen;
		char *								buffer = NULL;

		pgpa(pgpaPrefNameStringValid(prefName));
	
		pgpEnvSelector = (enum PgpEnvStrings)strPrefEntry(prefName).pgpEnvSelector;
		envStr = pgpenvGetString(pgpEnv, pgpEnvSelector, NULL, NULL);
		if (envStr != NULL)
		{
				pgpa(pgpaStrValid(envStr));
			
				envStrLen = strlen(envStr);
				buffer = (char *)pgpAlloc(envStrLen + 1);
				pgpCopyMemory(envStr, buffer, envStrLen + 1);
		}
		return buffer;
}

		PGPError
pgpGetPrefBoolean(
		PGPPrefNameBoolean		prefName,
		Boolean *						value)
{
		enum PgpEnvInts			pgpEnvSelector;
		PGPError						result = PGPERR_OK;
	
		pgpa((
				pgpaPrefNameBooleanValid(prefName),
				pgpaAddrValid(value, Boolean)));
	
		pgpEnvSelector = (enum PgpEnvInts)boolPrefEntry(prefName).pgpEnvSelector;
		*value = (Boolean)pgpenvGetInt(pgpEnv, pgpEnvSelector, NULL, &result);
	
		return result;
}

		PGPError
pgpGetPrefData(
		PGPPrefNameData			prefName,
		void *								buffer,
		size_t *						length)
{
		enum PgpEnvPointers	pgpEnvSelector;
		char const *				envData;
		size_t								envDataLen;
		PGPError						result = PGPERR_OK;

		pgpa((
				pgpaPrefNameDataValid(prefName),
				pgpaAddrValid(length, size_t),
				pgpaAssert(*length >= 0)));
	
		pgpEnvSelector = (enum PgpEnvPointers)
														dataPrefEntry(prefName).pgpEnvSelector;
		envData = pgpenvGetPointer(pgpEnv, pgpEnvSelector, &envDataLen,
																&result);
		if (envData != NULL)
		{
				pgpa(pgpaAddrValid(envData, VoidAlign));
			
				if (buffer == NULL || *length < envDataLen)
				{
						*length = envDataLen;
						return PGPERR_KEYDB_BUFFERTOOSHORT;
				}
				pgpa(pgpaAddrValid(buffer, char));
				*length = envDataLen;
				pgpCopyMemory(envData, buffer, envDataLen);
		}
		return result;
}

		void *
pgpGetPrefDataCopy(
		PGPPrefNameData			prefName,
		size_t *						length)
{
		enum PgpEnvPointers	pgpEnvSelector;
		char const *				envData;
		size_t								envDataLen;
		void *								buffer = NULL;

		pgpa(pgpaPrefNameDataValid(prefName));
	
		pgpEnvSelector = (enum PgpEnvPointers)
														dataPrefEntry(prefName).pgpEnvSelector;
		envData = pgpenvGetPointer(pgpEnv, pgpEnvSelector, &envDataLen,
																NULL);
		if (envData != NULL)
		{
				pgpa(pgpaAddrValid(envData, VoidAlign));
			
				if (length != NULL)
				{
						pgpa(pgpaAddrValid(length, size_t));
						*length = envDataLen;
				}
				buffer = pgpAlloc(envDataLen);
				if (buffer != NULL)
						pgpCopyMemory(envData, buffer, envDataLen);
		}
		return buffer;
}

		PGPFileRef *
pgpGetPrefFileRef(
		PGPPrefNameData			prefName)
{
		uchar *					buffer;
		size_t						size;
		PGPFileRef *		fileRef = NULL;
	
		buffer = (uchar *)pgpGetPrefDataCopy(prefName, &size);

		if (buffer != NULL)
		{
				fileRef = pgpImportFileRef(buffer, size);
				pgpFree(buffer);
		}
		return fileRef;
}

		PGPError
pgpSetPrefNumber(
		PGPPrefNameNumber		prefName,
		long								value)
{
		enum PgpEnvInts			pgpEnvSelector;
	
		pgpa(pgpaPrefNameNumberValid(prefName));
	
		pgpPrefsChanged = TRUE;
		pgpEnvSelector = (enum PgpEnvInts)numPrefEntry(prefName).pgpEnvSelector;
		return pgpenvSetInt(pgpEnv, pgpEnvSelector, (int)value, PGPENV_PRI_FORCE);
}

		PGPError
pgpSetPrefCString(
		PGPPrefNameString		prefName,
		char const *				string)
{
		enum PgpEnvStrings		pgpEnvSelector;
		PGPError						result = PGPERR_OK;
	
		pgpa((
				pgpaPrefNameStringValid(prefName),
				pgpaStrValid(string)));
	
		pgpPrefsChanged = TRUE;
		pgpEnvSelector = (enum PgpEnvStrings)strPrefEntry(prefName).pgpEnvSelector;
		result = pgpenvSetString(pgpEnv, pgpEnvSelector, string, PGPENV_PRI_FORCE);
	
		/*	Convert to fileref form if appropriate */
		switch (prefName)
		{
				case kPGPPrefPrivRing:
				{
						PGPFileRef *		fileRef = pgpNewFileRefFromFullPath(string);
					
				if (fileRef == NULL)
								result = PGPERR_NOMEM;
						else
						{
								result = pgpSetPrefFileRef(kPGPPrefPrivRingRef, fileRef);
								pgpFreeFileRef(fileRef);
						}
						break;
				}
				case kPGPPrefPubRing:
				{
						PGPFileRef *		fileRef = pgpNewFileRefFromFullPath(string);
					
						if (fileRef == NULL)
								result = PGPERR_NOMEM;
						else
						{
								result = pgpSetPrefFileRef(kPGPPrefPubRingRef, fileRef);
								pgpFreeFileRef(fileRef);
						}
						break;
				}
				default:
						/*	Nothing to do */
						break;
		}

		return result;
}

		PGPError
pgpSetPrefString(
		PGPPrefNameString		prefName,
		char const *				buffer,
		size_t								length)
{
		char *								envStr;
		PGPError						result = PGPERR_OK;
	
		pgpa((
				pgpaPrefNameStringValid(prefName),
				pgpaAddrValid(buffer, char),
				pgpaAssert(length >= 0)));
	
		envStr = pgpAlloc(length + 1);
		if (envStr == NULL)
				return PGPERR_NOMEM;
		pgpCopyMemory(buffer, envStr, length);
		envStr[length] = '\0';
	
		result = pgpSetPrefCString(prefName, envStr);
		pgpFree(envStr);
	
		return result;
		}

		PGPError
pgpSetPrefBoolean(
		PGPPrefNameBoolean		prefName,
		Boolean							value)
{
		enum PgpEnvInts			pgpEnvSelector;
	
		pgpa(pgpaPrefNameBooleanValid(prefName));
	
		pgpPrefsChanged = TRUE;
		pgpEnvSelector = (enum PgpEnvInts)boolPrefEntry(prefName).pgpEnvSelector;
		return pgpenvSetInt(pgpEnv, pgpEnvSelector, (int)value, PGPENV_PRI_FORCE);
}

		PGPError
pgpSetPrefData(
		PGPPrefNameData			prefName,
		void const *				buffer,
		size_t								length)
{
		enum PgpEnvPointers	pgpEnvSelector;
		void *								oldBuffer;
		void *								theCopy;
		PGPError						result = PGPERR_OK;
	
		pgpa((
				pgpaPrefNameDataValid(prefName),
				pgpaAddrValid(buffer, VoidAlign),
				pgpaAssert(length >= 0)));
	
		theCopy = pgpAlloc(length);
		if (theCopy == NULL)
				return PGPERR_NOMEM;
		pgpLeaksIgnoreMemory(theCopy);		/*	XXX Maybe fix this later */
		pgpCopyMemory(buffer, theCopy, length);
	
		pgpPrefsChanged = TRUE;
		pgpEnvSelector = (enum PgpEnvPointers)
																dataPrefEntry(prefName).pgpEnvSelector;
		oldBuffer = pgpenvGetPointer(pgpEnv, pgpEnvSelector, NULL, NULL);
		result = pgpenvSetPointer(pgpEnv, pgpEnvSelector, theCopy, length);
		if (oldBuffer != NULL)
				pgpFree(oldBuffer);
	
		return result;
}

		PGPError
pgpSetPrefFileRef(
		PGPPrefNameData			prefName,
		PGPFileRef const *		fileRef)
{
		uchar *			buffer;
		size_t				size;
		PGPError		result = PGPERR_OK;
	
		buffer = pgpExportFileRef(fileRef, &size);
		if (buffer == NULL)
				return PGPERR_NOMEM;
		result = pgpSetPrefData(prefName, buffer, size);
		pgpFree(buffer);
		return result;
}

		PGPError
pgpSetDefaultKeyPath(void)
{
		PGPError		result;

		if ((result = pgpSetPrefCString(kPGPPrefPubRing,
										strPrefEntry(kPGPPrefPubRing).defValue)) != PGPERR_OK)
				return result;

		if ((result = pgpSetPrefCString(kPGPPrefPrivRing,
										strPrefEntry(kPGPPrefPrivRing).defValue)) != PGPERR_OK)
				return result;

		return pgpSetDefaultKeyPathInternal();
}

		static PGPError
pgpSetDefaultPrefs(void)
{
		PGPPrefNameNumber		numPref;
		PGPPrefNameString		strPref;
		PGPPrefNameBoolean		boolPref;
	
		for (numPref = kPGPPrefNumFirst; numPref < kPGPPrefNumLast; numPref++)
				pgpenvSetInt(pgpEnv,
								(enum PgpEnvInts)numPrefEntry(numPref).pgpEnvSelector,
								(int)numPrefEntry(numPref).defValue, PGPENV_PRI_PRIVDEFAULT);
	
		for (strPref = kPGPPrefStrFirst; strPref < kPGPPrefStrLast; strPref++)
				if (strPrefEntry(strPref).defValue)
						pgpenvSetString(pgpEnv,
								(enum PgpEnvStrings)strPrefEntry(strPref).pgpEnvSelector,
								strPrefEntry(strPref).defValue, PGPENV_PRI_PRIVDEFAULT);
	
		for (boolPref = kPGPPrefBoolFirst; boolPref < kPGPPrefBoolLast; boolPref++)
				pgpenvSetInt(pgpEnv,
								(enum PgpEnvInts)boolPrefEntry(boolPref).pgpEnvSelector,
								(int)boolPrefEntry(boolPref).defValue, PGPENV_PRI_PRIVDEFAULT);
	
		return pgpSetDefaultPrefsInternal();
}

		PGPError
pgpLoadPrefs(void)
{
		PGPError		result;
		void		*buffer;

		/*	Initialize fileref pointers to NULL in case we are called twice */
		if (NULL !=
		(buffer = pgpenvGetPointer (pgpEnv, PGPENV_PRIVRINGREF, NULL, NULL))) {
				pgpMemFree (buffer);
		}
		if (NULL !=
		(buffer = pgpenvGetPointer (pgpEnv, PGPENV_PUBRINGREF, NULL, NULL))) {
				pgpMemFree (buffer);
		}
		pgpenvSetPointer(pgpEnv, PGPENV_PRIVRINGREF, NULL, 0);
		pgpenvSetPointer(pgpEnv, PGPENV_PUBRINGREF, NULL, 0);
	
		result = pgpSetDefaultPrefs();
		if (result == PGPERR_OK)
				result = pgpLoadPrefsInternal();
	
		/*	XXX This is hopefully temporary */
		if (result == PGPERR_OK)
		{
				PGPFileRef *		fileRef;
				char const *		path;
			
				if (pgpenvGetPointer(pgpEnv, PGPENV_PRIVRINGREF, NULL, NULL) == NULL)
				{
						path = pgpenvGetString(pgpEnv, PGPENV_SECRING, NULL, NULL);
						if (path != NULL)
						{
								fileRef = pgpNewFileRefFromFullPath(path);
								if (fileRef != NULL)
								{
										result = pgpSetPrefFileRef(kPGPPrefPrivRingRef, fileRef);
										pgpFreeFileRef(fileRef);
				}
						}
				}
			
				if (result == PGPERR_OK &&
						pgpenvGetPointer(pgpEnv, PGPENV_PUBRINGREF, NULL, NULL) == NULL)
				{
						path = pgpenvGetString(pgpEnv, PGPENV_PUBRING, NULL, NULL);
						if (path != NULL)
						{
								fileRef = pgpNewFileRefFromFullPath(path);
								if (fileRef != NULL)
				{
										result = pgpSetPrefFileRef(kPGPPrefPubRingRef, fileRef);
										pgpFreeFileRef(fileRef);
								}
						}
				}
		}
		pgpPrefsChanged = FALSE;
		return result;
}

		PGPError
pgpSavePrefs(void)
{
		if (pgpPrefsChanged)
		{
				pgpPrefsChanged = FALSE;
				return pgpSavePrefsInternal();
		}
		else
				return PGPERR_OK;
}


/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
*/
