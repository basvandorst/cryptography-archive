/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/
#pragma once


/*_____________________________________________________________________________
	This is the common header file for the PGP client.   It contains flags
	that apply globally to all modules.  Before building, make sure the flags
	here are set appropriately.
	
	Note: include this file in your project prefix
_____________________________________________________________________________*/


#define BETA						0		// zero for release
#define PGP_DEMO					0		// zero for regular version
#define PGP_FREEWARE				1		// zero for non-freeware
#define PGP_BUSINESS_SECURITY		0		// zero for personal version
#define PGP_PERSONAL_PRIVACY		0		// zero for business version
#define PGP_ADMIN_BUILD				0		// zero for business version
#define PGP_NO_LICENSE_NUMBER		1
#define NO_RSA_KEYGEN				1
#define NO_RSA_OPERATIONS			1

#if PGP_BUSINESS_SECURITY
	#if PGP_PERSONAL_PRIVACY || PGP_FREEWARE
		#error This is not a legal build combination
	#endif
#elif PGP_PERSONAL_PRIVACY
	#if PGP_BUSINESS_SECURITY || PGP_FREEWARE
		#error This is not a legal build combination
	#endif
#elif PGP_FREEWARE
	#if PGP_BUSINESS_SECURITY || PGP_PERSONAL_PRIVACY
		#error This is not a legal build combination
	#endif
#else
	#error Must define a primary build type!
#endif

#if PGP_ADMIN_BUILD
	#if PGP_PERSONAL_PRIVACY || PGP_FREEWARE
		#error This is not a legal build combination
	#endif
#endif

