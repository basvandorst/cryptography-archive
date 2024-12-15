//////////////////////////////////////////////////////////////////////////////
// PGPdiskHighLevelUtils.h
//
// Declarations for PGPdiskHighLevelUtils.cpp.
//////////////////////////////////////////////////////////////////////////////

// $Id: PGPdiskHighLevelUtils.h,v 1.10 1999/02/26 04:09:59 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef	Included_PGPdiskHighLevelUtils_h	// [
#define	Included_PGPdiskHighLevelUtils_h

#include "FatUtils.h"
#include "PGPdiskFileFormat.h"

#include "Cast5.h"
#include "CipherUtils.h"


////////
// Types
////////

class File;


/////////////////////
// Exported functions
/////////////////////

DualErr		ReadPGPdiskFileMainHeader(File *diskFile, 
				PGPdiskFileHeader **fileHeader);
DualErr		ReadPGPdiskFileMainHeader(LPCSTR path, 
				PGPdiskFileHeader **fileHeader);

DualErr		WritePGPdiskFileMainHeader(File *diskFile, 
				const PGPdiskFileHeader *fileHeader);
DualErr		WritePGPdiskFileMainHeader(LPCSTR path, 
				const PGPdiskFileHeader *fileHeader);

DualErr		ValidatePGPdisk(LPCSTR path);

DualErr		VerifyNormalPassphrase(LPCSTR path, SecureString *passphrase);

DualErr		VerifyPublicKeyPassphrase(LPCSTR path, SecureString *passphrase);

DualErr		VerifyPassphrase(LPCSTR path, SecureString *passphrase);
DualErr		VerifyMasterPassphrase(LPCSTR path, SecureString *passphrase);

DualErr		HowManyAlternatePassphrases(LPCSTR path, PGPUInt32 *numAlts);

DualErr		AddPassphrase(LPCSTR path, SecureString *masterPassphrase, 
				SecureString *newPassphrase, PGPBoolean readOnly);
DualErr		ChangePassphrase(LPCSTR path, SecureString *oldPassphrase, 
				SecureString *newPassphrase);

DualErr		RemovePassphrase(LPCSTR path, SecureString *passphrase);
DualErr		RemoveAlternatePassphrases(LPCSTR path);

PGPBoolean	IsPGPdiskNotEncrypted(LPCSTR path);

PGPUInt8	GetDriveLetterPref(LPCSTR path);
DualErr		SetDriveLetterPref(LPCSTR path, PGPUInt8 drive);

PGPBoolean	GetPGPdiskMountedFlag(LPCSTR path);
DualErr		SetPGPdiskMountedFlag(File *diskFile, PGPBoolean mountedFlag);
DualErr		SetPGPdiskMountedFlag(LPCSTR path, PGPBoolean mountedFlag);

PGPUInt64	GetPGPdiskUniqueSessionId(LPCSTR path);
DualErr		SetPGPdiskUniqueSessionId(LPCSTR path, PGPUInt64 uniqueSessionId);

PGPBoolean	DoesPGPdiskHaveBadCAST(LPCSTR path);
PGPBoolean	WasPGPdiskConversionInterrupted(LPCSTR path);

DualErr		NukeAllNonADKHeaders(File *diskFile, 
				const CASTKey *newSessionKey);

#endif	// ] Included_PGPdiskHighLevelUtils_h
