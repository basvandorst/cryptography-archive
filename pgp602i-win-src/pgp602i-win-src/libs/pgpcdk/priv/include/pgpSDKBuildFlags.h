/*____________________________________________________________________________
	pgpSDKBuildFlags.h
	
	Copyright (C) 1997 Network Associates, Inc. and its affiliates. 
	All rights reserved.
	
	

	$Id: pgpSDKBuildFlags.h,v 1.37.2.24.2.1 1998/11/12 03:21:44 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpSDKBuildFlags_h	/* [ */
#define Included_pgpSDKBuildFlags_h

#define PGP_RSA				1
#define PGP_RSA_KEYGEN		0

#define PGP_USECAPIFORRSA	0	/* Try to use Microsoft CAPI library for RSA */
#define PGP_USECAPIFORMD2	0	/* Try to use Microsoft CAPI library for MD2 */
#define PGP_USEBSAFEFORRSA	0	/* Use RSA's BSAFE library for RSA support */
#define PGP_USEPGPFORRSA	1	/* Use the PGP implementation for RSA support */
#define PGP_USERSAREF		0	/* Use the non-commercial RSAREF library for RSA */


/* These probably will always be on */
#define PGP_CAST5		1
#define PGP_DES3		1
/* It turns out that IDEA is used on some DH keys, so it needs to be enabled
   even if RSA is off */
#define PGP_IDEA		1

/* Allows turning off signing/verification capability in library */
#ifndef PGP_SIGN_DISABLE
	#define PGP_SIGN_DISABLE	0
#endif

#ifndef PGP_VERIFY_DISABLE
	#define PGP_VERIFY_DISABLE	0
#endif

/* Allows turning off encryption/decryption capability in library */
#ifndef PGP_ENCRYPT_DISABLE
	#define PGP_ENCRYPT_DISABLE	0
#endif

#ifndef PGP_DECRYPT_DISABLE
	#define PGP_DECRYPT_DISABLE	0
#endif

/*____________________________________________________________________________
	Check for invalid combinations of build flags
____________________________________________________________________________*/

#if !( defined(PGP_MACINTOSH) || defined(PGP_UNIX) || defined(PGP_WIN32) )
#error one of {PGP_MACINTOSH, PGP_UNIX, PGP_WIN32} must be defined
#endif

#if PGP_RSA	/* [ */

	#if ! PGP_IDEA
	#error PGP_RSA requires PGP_IDEA
	#endif

	#if (PGP_USECAPIFORRSA + PGP_USEBSAFEFORRSA + PGP_USERSAREF + \
			PGP_USEPGPFORRSA) != 1
	#error Must enable exactly one RSA implementation option
	#endif

	#if PGP_USECAPIFORRSA && (PGP_MACINTOSH || PGP_UNIX)
	#error Cannot enable CAPI RSA implementation on this platform
	#endif
	
#else	/* ] PGP_RSA [ */

	#if PGP_RSA_KEYGEN
	#error Cannot enable PGP_RSA_KEYGEN without PGP_RSA
	#endif

	#if (PGP_USECAPIFORRSA + PGP_USEBSAFEFORRSA + PGP_USERSAREF + \
			PGP_USEPGPFORRSA) != 0
	#error Cannot enable any RSA implementation options without PGP_RSA
	#endif

#endif	/* ] PGP_RSA */

#if PGP_USECAPIFORMD2 && ! PGP_USECAPIFORRSA
#error Cannot use CAPI MD2 without CAPI RSA
#endif

#endif /* ] Included_pgpSDKBuildFlags_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
