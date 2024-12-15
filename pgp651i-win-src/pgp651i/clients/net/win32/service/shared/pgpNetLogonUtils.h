/*____________________________________________________________________________
	Copyright (c) 1998 Network Associates, Inc. and its Affiliated Companies
	All rights reserved.
	
	$Id: pgpNetLogonUtils.h,v 1.6 1999/04/13 19:26:55 elowe Exp $
____________________________________________________________________________*/

#ifndef Included_pgpNetLogonUtils_h
#define Included_pgpNetLogonUtils_h

void pgpNetSendLogonStatus(HWND hSendingWnd);
void pgpNetPromptForPassphrase(HWND hInitiatorWnd, HWND hSendingWnd, PGPBoolean bForce);
void pgpNetClearPassphrases();
void pgpNetLogonInit();
void pgpNetLogonCleanup();

#endif // Included_pgpNetLogonUtils_h

