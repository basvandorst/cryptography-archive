/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: UIhelp.h,v 1.7 1999/03/10 03:04:35 heller Exp $
____________________________________________________________________________*/
#ifndef Included_UIHELP_h	/* [ */
#define Included_UIHELP_h

/* defines constants for help file */

// Passphrase stuff starts at 1000

#define IDH_PGPCLPHRASE_HIDETYPING		1010
#define IDH_PGPCLPHRASE_PHRASE			1011
#define IDH_PGPCLPHRASE_SIGNINGKEY		1012
#define IDH_PGPCLPHRASE_KEYLIST			1016
#define IDH_PGPCLPHRASE_CONFIRMATION	1017
#define IDH_PGPCLPHRASE_QUALITY			1018
#define IDH_PGPCLPHRASE_KEYNAME			1019 // *

// Recipient dialog starts at 3000
#define IDH_IDD_RECIPIENTDLG			3000
#define IDH_IDC_RECIPIENT_LIST			3006
#define IDH_IDC_USER_ID_LIST			3007

// Also note that the RSADSA encryption warning
// has moved to PGPcl. 3009 is the still the help 
// ID for it.
// #define IDH_RSADSARECWARNING			3009

// ClientLib options that we've hardcoded in 
// help for.
#define IDH_TEXTOUTPUT					3100
#define IDH_CONVENCRYPTION				3101
#define IDH_WIPEORIGINAL				3102
#define IDH_DETACHEDSIG					3103
#define IDH_FYEO						3104
#define IDH_SDA							3105

#endif /* ] Included_UIHELP_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
