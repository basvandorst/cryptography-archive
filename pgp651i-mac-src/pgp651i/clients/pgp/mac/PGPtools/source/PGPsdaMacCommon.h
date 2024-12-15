/*____________________________________________________________________________	Copyright (C) 1999 Network Associates, Inc. and it's affiliated companies	All rights reserved.	$Id: PGPsdaMacCommon.h,v 1.2 1999/02/25 11:10:09 heller Exp $____________________________________________________________________________*/#pragma once#include "pgpBase.h"#include "CipherContext.h"#include "CipherProcGlue.h"#include "PGPsdaMacFileFormat.h"typedef struct SDABuffer{	PGPUInt32	size;	PGPUInt32	used;	PGPUInt32	pos;	PGPByte		*data;} SDABuffer;PGP_BEGIN_C_DECLARATIONSOSStatus	AllocateSDABuffer(PGPSize bufferSize, SDABuffer *buffer);OSStatus	ComputeCheckBytes(CipherProcRef cipherProc,					CipherContext *cipherContext, const CASTKey *key,					PGPsdaArchiveCheckBytes	*checkBytes);void		FreeSDABuffer(SDABuffer *buffer);void		HashSaltPassphrase(const char *passphrase,					const PassphraseSalt *salt, PGPUInt32 *hashReps,					CASTKey *key);void		HoldBuffer(void *buffer, PGPSize bufferSize);void		UnholdBuffer(void *buffer, PGPSize bufferSize,					PGPBoolean clearBuffer);PGPBoolean	VerifyCheckBytes(CipherProcRef cipherProc,	CipherContext			*cipherContext,	const CASTKey			*key,	PGPsdaArchiveCheckBytes	*checkBytes);					PGP_END_C_DECLARATIONS