/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: Prefs.h,v 1.2.8.1 1998/11/12 03:14:12 heller Exp $
____________________________________________________________________________*/
#ifndef Included_Prefs_h	/* [ */
#define Included_Prefs_h

#include <windows.h>
#include "pgpUtilities.h"
#include "pgpSymmetricCipher.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL UsePGPMimeForEncryption(PGPMemoryMgrRef memoryMgr);
BOOL UsePGPMimeForSigning(PGPMemoryMgrRef memoryMgr);
BOOL ByDefaultEncrypt(PGPMemoryMgrRef memoryMgr);
BOOL ByDefaultSign(PGPMemoryMgrRef memoryMgr);
BOOL ByDefaultWordWrap(PGPMemoryMgrRef memoryMgr, long* pThreshold);

BOOL GetCommentString(PGPMemoryMgrRef memoryMgr, 
					  char *szComment, 
					  int nLength);

BOOL GetPreferredAlgorithm(PGPMemoryMgrRef memoryMgr, 
						   PGPCipherAlgorithm *prefAlg);

BOOL GetAllowedAlgorithms(PGPMemoryMgrRef memoryMgr, 
						  PGPCipherAlgorithm *pAllowedAlgs, 
						  int *pNumAlgs);

BOOL AutoDecrypt(PGPMemoryMgrRef memoryMgr);
BOOL SyncOnVerify(PGPMemoryMgrRef memoryMgr);
BOOL MarginalIsInvalid(PGPMemoryMgrRef memoryMgr);

#ifdef __cplusplus
}
#endif

#endif /* ] Included_Prefs_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/


