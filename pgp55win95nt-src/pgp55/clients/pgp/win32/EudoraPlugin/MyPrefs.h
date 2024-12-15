/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: MyPrefs.h,v 1.4 1997/09/20 11:30:12 elrod Exp $
____________________________________________________________________________*/
#ifndef Included_MyPrefs_h	/* [ */
#define Included_MyPrefs_h

#include "pgpSymmetricCipher.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL AutoDecrypt(void);
BOOL UsePGPMimeForEncryption(void);
BOOL UsePGPMimeForSigning(void);
BOOL ByDefaultEncrypt(void);
BOOL ByDefaultSign(void);
BOOL ByDefaultWordWrap(long* pThreshold);
BOOL GetCommentString(char *szComment, int nLength);
BOOL GetPreferredAlgorithm(PGPCipherAlgorithm *prefAlg);


#ifdef __cplusplus
}
#endif

#endif /* ] Included_MyPrefs_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

