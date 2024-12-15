/*_____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.

	$Id: pgpBuildFlags.h,v 1.83.2.49.2.2 1998/11/13 04:05:37 heller Exp $
_____________________________________________________________________________*/
#pragma once


/*_____________________________________________________________________________
	This is the common header file for the PGP client.   It contains flags
	that apply globally to all modules.  Before building, make sure the flags
	here are set appropriately.
	
	Note: include this file in your project prefix
_____________________________________________________________________________*/


#ifndef BETA
#define BETA						0		// zero for release
#endif

#ifndef PGP_DEMO
#define PGP_DEMO					0		// zero for regular version
#endif

#ifndef PGP_FREEWARE
#define PGP_FREEWARE				1		// zero for non-freeware
#endif

#ifndef PGP_BUSINESS_SECURITY
#define PGP_BUSINESS_SECURITY		0		// zero for personal version
#endif

#ifndef PGP_PERSONAL_PRIVACY
#define PGP_PERSONAL_PRIVACY		0		// zero for business version
#endif

#ifndef PGP_ADMIN_BUILD
#define PGP_ADMIN_BUILD				0		 // zero for business version
#endif

#ifndef NO_RSA_KEYGEN
#define NO_RSA_KEYGEN				1
#endif

#ifndef NO_RSA_OPERATIONS
#define NO_RSA_OPERATIONS			0
#endif

#ifndef CREDIT_RSA_BSAFE
#define CREDIT_RSA_BSAFE			0
#endif

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

