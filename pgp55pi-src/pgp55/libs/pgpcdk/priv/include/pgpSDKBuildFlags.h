/*____________________________________________________________________________
	pgpSDKBuildFlags.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpSDKBuildFlags.h,v 1.9 1997/10/31 22:04:29 lloyd Exp $
____________________________________________________________________________*/
#ifndef Included_pgpSDKBuildFlags_h	/* [ */
#define Included_pgpSDKBuildFlags_h

/* Enable support for using RSA keys */
#define PGP_RSA				0

/* Enable support for RSA key generation */
#define PGP_RSA_KEYGEN		0



/* these probably will always be on */
#define PGP_CAST5		1
#define PGP_DES3		1
/* it turns out that IDEA is used on some DH keys, so it needs to be enabled
   even if RSA is off */
#define PGP_IDEA		1




/*____________________________________________________________________________
	Check for invalid combinations of build flags
____________________________________________________________________________*/
#if PGP_RSA_KEYGEN && ! PGP_RSA
#error cant have PGP_RSA_KEYGEN on with PGP_RSA off
#endif


#if PGP_RSA && ! PGP_IDEA
#error PGP_RSA requires PGP_IDEA
#endif



#endif /* ] Included_pgpSDKBuildFlags_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
