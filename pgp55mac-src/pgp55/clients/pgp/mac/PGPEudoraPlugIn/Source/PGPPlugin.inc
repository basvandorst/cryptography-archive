/*____________________________________________________________________________
	PGPPlugin.inc
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: PGPPlugin.inc,v 1.5 1997/09/11 19:38:12 lloyd Exp $
____________________________________________________________________________*/


#if __POWERPC__
	#include <MacHeadersPPC>
#elif __CFM68K__
	#include <MacHeadersCFM68K>
#else
	#include <MacHeaders68K>
#endif

#define PGP_MACINTOSH		1

#ifndef PGP_DEBUG
PGP_DEBUG must be defined 
PGP_DEBUG must be defined 
#endif



#include "pgpBuildFlags.h"

#define USE_PGP_OPERATOR_NEW			1
#define HAVE_CONFIG_H					1
#define USE_MAC_DEBUG_PATCHES			0 	
#define UNFINISHED_CODE_ALLOWED			1
#define USE_PGP_LEAKS					PGP_DEBUG


#include	"pgpMem.h"
#include	"MacDebug.h"

typedef const uchar *	ConstStringPtr;

#if RELEASE

	#if BETA
	BETA must be off if RELEASE is on
	#endif

	#if PGP_DEBUG
	PGP_DEBUG must be off if RELEASE is on
	#endif
	
#endif

#if BETA
	#include "Beta.h"
#endif



