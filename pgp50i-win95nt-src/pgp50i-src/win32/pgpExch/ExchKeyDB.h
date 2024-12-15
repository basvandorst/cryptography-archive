/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
#ifndef _EXCHKEYDB_H
#define _EXCHKEYDB_H

#ifdef __cplusplus
extern "C" {
#endif

unsigned long GetDefaultSigningKey(char* szKeyId, size_t length);
void KMConvertStringKeyID (char* sz);

BOOL UsePGPMime(void);
BOOL UsePGPMimeForSigning(void);
BOOL ByDefaultEncrypt(void);
BOOL ByDefaultSign(void);
BOOL ByDefaultWordWrap(long* pThreshold);

#ifdef __cplusplus
}
#endif

#endif
