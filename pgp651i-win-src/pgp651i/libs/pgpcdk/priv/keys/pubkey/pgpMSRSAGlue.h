/* 
 * pgpMSRSAGlue.h - RSA support using Microsoft CAPI
 *
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpMSRSAGlue.h,v 1.2 1999/04/07 23:06:11 cpeterson Exp $
 */
#ifndef Included_pgpMSRSAGlue_h
#define Included_pgpMSRSAGlue_h

#include "pgpSDKBuildFlags.h"

#if PGP_USECAPIFORMD2 && !PGP_USECAPIFORRSA
#error "PGP_USECAPIFORMD2 requires PGP_USECAPIFORRSA"
#endif

#if PGP_USECAPIFORRSA

/* Include CAPI headers */
#define _WIN32_WINNT 0x0400
#include <windows.h>

#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS

/* 
 * Data structure for dynamic access to CAPI.  Not all users have it
 * so we don't want to statically link with it.
 */
typedef struct _CAPIEntries {
	FARPROC CryptAcquireContext;
	FARPROC CryptReleaseContext;
	FARPROC CryptImportKey;
	FARPROC CryptExportKey;
	FARPROC CryptDestroyKey;
	FARPROC CryptCreateHash;
	FARPROC CryptHashData;
	FARPROC CryptSetHashParam;
	FARPROC CryptDestroyHash;
	FARPROC CryptVerifySignature;
	FARPROC CryptSignHash;
} CAPIEntries;

/* Test for CAPI availability */
PGPUInt32
pgpCAPIuse (void);


PGP_END_C_DECLARATIONS

#endif /* PGP_USECAPIFORRSA */

#endif /* !Included_pgpMSRSAGlue_h */
