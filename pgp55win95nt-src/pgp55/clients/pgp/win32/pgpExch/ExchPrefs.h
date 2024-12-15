/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: ExchPrefs.h,v 1.5 1997/10/10 23:33:28 dgal Exp $
____________________________________________________________________________*/
#ifndef Included_ExchPrefs_h	/* [ */
#define Included_ExchPrefs_h

#include <windows.h>
#include "pgpSymmetricCipher.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL UsePGPMime(void);
BOOL UsePGPMimeForSigning(void);
BOOL ByDefaultEncrypt(void);
BOOL ByDefaultSign(void);
BOOL ByDefaultWordWrap(long* pThreshold);
BOOL GetCommentString(char *szComment, int nLength);
BOOL GetPreferredAlgorithm(PGPCipherAlgorithm *prefAlg);
BOOL GetAllowedAlgorithms(PGPCipherAlgorithm *pAllowedAlgs, int *pNumAlgs);
BOOL AutoDecrypt(void);

#ifdef __cplusplus
}
#endif

#endif /* ] Included_ExchPrefs_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/


