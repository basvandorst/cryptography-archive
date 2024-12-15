/*
 * pgpPrefMac.c -- Access preference database for MacOS
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: pgpPrefMac.c,v 1.14.2.9 1997/06/07 09:50:29 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "MacFiles.h"
#include "MacStrings.h"

#include "pgpKDBint.h"
#include "pgpEnv.h"
#include "MacFiles.h"

#define kPrefMagic						0x53C3971F
#define kMajorPrefVersion		1
#define kMinorPrefVersion		1

typedef struct MacPrefHeader_
{
		long		prefMagic;
		short		majorPrefVersion;
		short		minorPrefVersion;
		uchar		data[];
} MacPrefHeader;

/*	Initializer for the kPGPPrefPGPmenuMacAppSignatures data preference */
static struct PGPmenuApps
{
		OSType		creator;
		Str31		appName;
} initPGPmenuApps[] = {
		{ 'ttxt', "\pSimpleText" },
		{ 'MACS', "\pFinder" },
/*		{ 'CSOm', "\pEudora" }, */
		{ 'MOSS', "\pNetscape" },
		{ 'AOp3', "\pAmerica Online" },
		{ 'R*ch', "\pBBEdit" },
		{ 'MSNM', "\pMS Internet Mail" }
};


		static OSErr
OpenPrefsFile(Boolean forWriting, short *refNum)
{
		uchar const *		fileName = "\pPGP Preferences";
		short						vRefNum;
		long						dirID;
		OSErr						result;

		result = FindPGPPreferencesFolder(kOnSystemDisk, &vRefNum, &dirID);
	
		if (forWriting && result == noErr)
				result = HCreate(vRefNum, dirID, fileName, 'pgpK', 'pref');
	
		if (result == noErr || (forWriting && result == dupFNErr))
				result = HOpenDF(vRefNum, dirID, fileName,
												forWriting ? fsRdWrPerm : fsRdPerm, refNum);
	
		return result;
}

		PGPError
pgpSetDefaultKeyPathInternal(void)
{
		DirInfo					pb;
		Str255						name;
		FSSpec						fsSpec;
		PGPFileRef *		fileRef;
		PGPError				result = PGPERR_OK;
	
		CopyPString("\pPGP Keyrings", name);
		if (HGetVol(NULL, &pb.ioVRefNum, &pb.ioDrDirID) != noErr)
				return PGPERR_NO_FILE;
		pb.ioNamePtr = name;
		pb.ioFDirIndex = 0;
		if (PBGetCatInfoSync((CInfoPBPtr)&pb) == noErr
								&& (pb.ioFlAttrib & ioDirMask))
		{
				fsSpec.vRefNum = pb.ioVRefNum;
				fsSpec.parID = pb.ioDrDirID;
		}
		else if (FindPGPPreferencesFolder(kOnSystemDisk, &fsSpec.vRefNum,
																&fsSpec.parID) != noErr)
				return PGPERR_NO_FILE;
	
		CopyPString("\pPGP Private Keys", fsSpec.name);
		if ((fileRef = pgpNewFileRefFromFSSpec(&fsSpec)) == NULL)
				return PGPERR_NOMEM;
		result = pgpSetPrefFileRef(kPGPPrefPrivRingRef, fileRef);
		pgpFreeFileRef(fileRef);
		if (result != PGPERR_OK)
				return result;
	
		CopyPString("\pPGP Public Keys", fsSpec.name);
		if ((fileRef = pgpNewFileRefFromFSSpec(&fsSpec)) == NULL)
				return PGPERR_NOMEM;
		result = pgpSetPrefFileRef(kPGPPrefPubRingRef, fileRef);
		pgpFreeFileRef(fileRef);
		if (result != PGPERR_OK)
				return result;
	
		return PGPERR_OK;
}

		PGPError
pgpSetDefaultPrefsInternal(void)
{
		PGPError				result = PGPERR_OK;
	
		result = pgpSetDefaultKeyPathInternal();
		if (result != PGPERR_OK)
				return result;

		result = pgpSetPrefData(kPGPPrefPGPmenuMacAppSignatures,
														(void *)initPGPmenuApps,
														sizeof(initPGPmenuApps));
		if (result != PGPERR_OK)
				return result;

		return PGPERR_OK;
}

		PGPError
pgpLoadPrefsInternal(void)
{
		short						refNum = 0;
		long						fileSize;
		uchar *					buffer = NULL;
		uchar *					bufferEnd;
		uchar *					p;
		MacPrefHeader *	header;
		Boolean					required;
		ushort						selector;
		long						longVal;
		OSErr						macErr;
		PGPError				result = PGPERR_OK;
	
		pgpAssert(sizeof(uchar) == 1 && sizeof(ushort) == 2 && sizeof(long) == 4
						&& kPGPPrefNumFirst > 0
						&& kPGPPrefNumLast <= kPGPPrefStrFirst
						&& kPGPPrefStrLast <= kPGPPrefBoolFirst
						&& kPGPPrefBoolLast <= kPGPPrefDataFirst
						&& kPGPPrefDataLast == (kPGPPrefDataLast & 0x7FFF));

		if ((macErr = OpenPrefsFile(FALSE, &refNum)) != noErr
						|| GetEOF(refNum, &fileSize) != noErr)
		{
				if (macErr != fnfErr)
						result = PGPERR_NO_FILE;
				goto error;
		}
		if (fileSize < sizeof(MacPrefHeader))
		{
		corrupt:
				result = PGPERR_KEYDB_CORRUPT;
				goto error;
		}
	
		if ((buffer = pgpAlloc((size_t)fileSize)) == NULL)
		{
				result = PGPERR_NOMEM;
				goto error;
		}
		if (FSRead(refNum, &fileSize, (Ptr)buffer) != noErr)
		{
				result = PGPERR_KEYDB_IOERROR;
				goto error;
		}
		FSClose(refNum);
		refNum = 0;
	
		header = (MacPrefHeader *)buffer;
		if (header->prefMagic != kPrefMagic
						|| header->majorPrefVersion < 1
						|| header->minorPrefVersion < 1)
				goto corrupt;
	
		if (header->majorPrefVersion > kMajorPrefVersion)
		{
		tooNew:
				result = PGPERR_KEYDB_VERSIONTOONEW;
				goto error;
		}
	
#if 0
		if (header->majorPrefVersion < kMajorPrefVersion
				|| header->minorPrefVersion < kMinorPrefVersion)
		{
				/*	Updating code goes here after we bump the version */
		}
#endif

		bufferEnd = buffer + fileSize;
		p = header->data;
		while (p + 2 <= bufferEnd)
		{
				selector = (p[0] << 8) | p[1];
				p += 2;
				required = (selector & 0x8000);
				selector &= 0x7FFF;
				if (selector >= kPGPPrefDataFirst)
				{
						/*	Data preference */
						long length;
					
						if (p + 4 > bufferEnd)
								goto corrupt;
						length = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
						p += 4;
						if (length < 0 || p + length > bufferEnd)
								goto corrupt;
						switch (selector)
						{
								case kPGPPrefPubRingRef:
								case kPGPPrefPrivRingRef:
								case kPGPPrefRandSeedData:
								case kPGPPrefPGPkeysMacMainWinPos:
								case kPGPPrefPGPappMacPrivateData:
								case kPGPPrefPGPmenuMacAppSignatures:
										result = pgpSetPrefData((PGPPrefNameData)selector,
																						(char *)p, length);
										if (result != PGPERR_OK)
												goto error;
										break;
								default:
										if (required)
												goto tooNew;
										break;
						}
						p += length;
				}
				else if (selector >= kPGPPrefBoolFirst)
				{
						/*	Boolean preference */
						if (p + 1 > bufferEnd)
								goto corrupt;
						switch (selector)
						{
								case kPGPPrefArmor:
								case kPGPPrefEncryptToSelf:
								case kPGPPrefCompress:
								case kPGPPrefMailWordWrapEnable:
								case kPGPPrefMailPassCacheEnable:
								case kPGPPrefMailEncryptPGPMIME:
								case kPGPPrefMailSignPGPMIME:
								case kPGPPrefFastKeyGen:
								case kPGPPrefSignCacheEnable:
								case kPGPPrefWarnOnMixRSAElGamal:
								case kPGPPrefMailEncryptDefault:
								case kPGPPrefMailSignDefault:
								case kPGPPrefMarginallyValidWarning:
								case kPGPPrefFirstKeyGenerated:
								case kPGPPrefWarnOnRSARecipAndNonRSASigner:
										result = pgpSetPrefBoolean((PGPPrefNameBoolean)selector,
																								(p[0] != 0));
										if (result != PGPERR_OK)
												goto error;
										break;
								default:
										if (required)
												goto tooNew;
										break;
						}
						p++;
				}
				else if (selector >= kPGPPrefStrFirst)
				{
						/*	String preference */
						uchar *			s;
					
						s = p;
						while (s < bufferEnd && *s != '\0')
								s++;
						if (s >= bufferEnd)
								goto corrupt;
						switch (selector)
						{
								case kPGPPrefUserID:
								case kPGPPrefPubRing:
								case kPGPPrefPrivRing:
								case kPGPPrefRandSeed:
								case kPGPPrefLanguage:
								case kPGPPrefOwnerName:
								case kPGPPrefCompanyName:
								case kPGPPrefLicenseNumber:
								case kPGPPrefHTTPKeyServerHost:
										result = pgpSetPrefString((PGPPrefNameString)selector,
																								(char *)p, s - p);
										if (result != PGPERR_OK)
												goto error;
										break;
								default:
										if (required)
												goto tooNew;
										break;
						}
						p = s + 1;
				}
				else if (selector >= kPGPPrefNumFirst)
				{
						/*	Number preference */
						if (p + 4 > bufferEnd)
								goto corrupt;
						longVal = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
						switch (selector)
						{
								case kPGPPrefVersion:
								case kPGPPrefCipherNum:
								case kPGPPrefHashNum:
								case kPGPPrefCompressNum:
								case kPGPPrefArmorLines:
								case kPGPPrefCertDepth:
								case kPGPPrefCompressQual:
								case kPGPPrefMailWordWrapWidth:
								case kPGPPrefMailPassCacheDuration:
								case kPGPPrefSignCacheDuration:
								case kPGPPrefDateOfLastSplashScreen:
								case kPGPPrefHTTPKeyServerPort:
										result = pgpSetPrefNumber((PGPPrefNameNumber)selector,
																								longVal);
										if (result != PGPERR_OK)
												goto error;
										break;
								default:
										if (required)
												goto tooNew;
										break;
						}
						p += 4;
				}
				else
						goto corrupt;
		}
	
error:
		if (refNum != 0)
				FSClose(refNum);
		if (buffer != NULL)
				pgpFree(buffer);
		return result;
}

		PGPError
pgpSavePrefsInternal(void)
{
		short						refNum = 0;
		PGPError				result = PGPERR_OK;
		MacPrefHeader		header;
		long						size;
		ushort						selector;
	
		pgpAssert(sizeof(uchar) == 1 && sizeof(ushort) == 2 && sizeof(long) == 4
						&& kPGPPrefNumFirst > 0
						&& kPGPPrefNumLast <= kPGPPrefStrFirst
						&& kPGPPrefStrLast <= kPGPPrefBoolFirst
						&& kPGPPrefBoolLast <= kPGPPrefDataFirst
						&& kPGPPrefDataLast == (kPGPPrefDataLast & 0x7FFF));
	
		if (OpenPrefsFile(TRUE, &refNum) != noErr || SetEOF(refNum, 0) != noErr)
		{
				result = PGPERR_NO_FILE;
				goto error;
		}
	
		pgpClearMemory(&header, sizeof(header));
		header.prefMagic = kPrefMagic;
		header.majorPrefVersion = kMajorPrefVersion;
		header.minorPrefVersion = kMinorPrefVersion;
		size = header.data - (char *)&header;
		if (FSWrite(refNum, &size, (Ptr)&header) != noErr)
		{
		ioError:
				result = PGPERR_KEYDB_IOERROR;
				goto error;
		}
	
		for (selector = kPGPPrefNumFirst; selector < kPGPPrefNumLast; selector++)
		{
				long						longVal;
			
				result = pgpGetPrefNumber((PGPPrefNameNumber)selector, &longVal);
				if (result != PGPERR_OK)
						goto error;
				size = 2;
				if (FSWrite(refNum, &size, (Ptr)&selector) != noErr)
						goto ioError;
				size = 4;
				if (FSWrite(refNum, &size, (Ptr)&longVal) != noErr)
						goto ioError;
		}
for (selector = kPGPPrefStrFirst; selector < kPGPPrefStrLast; selector++)
		{
				char *						strVal;
			
				strVal = pgpGetPrefCStringCopy((PGPPrefNameString)selector);
				if (strVal == NULL)
				{
						result = PGPERR_NOMEM;
						goto error;
				}
				size = 2;
				if (FSWrite(refNum, &size, (Ptr)&selector) != noErr)
						goto ioError;
				size = strlen(strVal) + 1;
				if (FSWrite(refNum, &size, (Ptr)strVal) != noErr)
						goto ioError;
				pgpFree(strVal);
		}
		for (selector = kPGPPrefBoolFirst; selector < kPGPPrefBoolLast; selector++)
		{
				Boolean					boolVal;
				uchar						theByte;
			
				result = pgpGetPrefBoolean((PGPPrefNameBoolean)selector, &boolVal);
				if (result != PGPERR_OK)
						goto error;
				theByte = boolVal ? 0xFF : 0;
				size = 2;
				if (FSWrite(refNum, &size, (Ptr)&selector) != noErr)
						goto ioError;
				size = 1;
				if (FSWrite(refNum, &size, (Ptr)&theByte) != noErr)
						goto ioError;
		}
		for (selector = kPGPPrefDataFirst; selector < kPGPPrefDataLast; selector++)
		{
				char *						dataVal;
				size_t						len;
				long						dataLen;
			
				dataVal = pgpGetPrefDataCopy((PGPPrefNameData)selector, &len);
				if (dataVal != NULL)
				{
						size = 2;
						if (FSWrite(refNum, &size, (Ptr)&selector) != noErr)
								goto ioError;
						dataLen = len;
						size = 4;
						if (FSWrite(refNum, &size, (Ptr)&dataLen) != noErr)
								goto ioError;
						size = dataLen;
						if (FSWrite(refNum, &size, (Ptr)dataVal) != noErr)
								goto ioError;
						pgpFree(dataVal);
				}
		}
	
error:
		if (refNum != 0)
				FSClose(refNum);
		return result;
}


/*
 * Local Variables:
 * tab-width: 4
 * End:
* vi: ts=4 sw=4
* vim: si
*/
