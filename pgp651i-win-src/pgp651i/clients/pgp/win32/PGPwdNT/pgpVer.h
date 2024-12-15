/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPver.h - PGP company-specific version information
	

	$Id: pgpVer.h,v 1.1 1999/05/11 02:41:56 wjb Exp $
____________________________________________________________________________*/

#define VER_COMPANYNAME_STR		"Network Associates, Inc."

#define VER_LEGALTRADEMARKS_STR	\
"PGP is a trademark of Network Associates, Inc."

#define VER_LEGALCOPYRIGHT_YEARS    "1998"

#define VER_LEGALCOPYRIGHT_STR		\
	"Copyright (c) Network Associates, Inc. " 	\
	VER_LEGALCOPYRIGHT_YEARS


/* default is nodebug */
#if DBG
#define VER_DEBUG				VS_FF_DEBUG
#else
#define VER_DEBUG				0
#endif

/* default is release */
#if BETA
#define VER_PRERELEASE			VS_FF_PRERELEASE
#else
#define VER_PRERELEASE			0
#endif

#define VER_FILEFLAGSMASK  VS_FFI_FILEFLAGSMASK
#define VER_FILEOS		   VOS_NT_WINDOWS32
#define VER_FILEFLAGS	   (VER_PRERELEASE | VER_DEBUG)


