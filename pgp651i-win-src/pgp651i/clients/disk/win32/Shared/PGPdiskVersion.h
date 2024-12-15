///////////////////////////////////////////////////////////////////////////////
// PGPdiskVersion.h
//
// Includes version constants and strings for the entire project.
///////////////////////////////////////////////////////////////////////////////

// $Id: PGPdiskVersion.h,v 1.83.2.21.2.5.2.1 1999/07/23 00:16:24 heller Exp $

// Copyright (C) 6.5.1 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_PGPdiskVersion_h	// [
#define Included_PGPdiskVersion_h


////////////
// Constants
////////////

// The version numbers of the app and driver.
#define	kPGPdiskAppVersion		0x60000001
#define	kPGPdiskDriverVersion	kPGPdiskAppVersion

// The version numbers the app/driver are compatible with.
#define	kCompatDriverVersion	kPGPdiskAppVersion
#define	kCompatAppVersion		kPGPdiskDriverVersion

// The version text string seen in the about box.
#define	kVersionTextString		"6.5.1i"

// Version information that goes in the 'Version' resource of modules.
#define	kPGPdiskFileVerData		6,5,1,0
#define	kPGPdiskProductVerData	kPGPdiskFileVerData

#define kPGPdiskFileVerString		"6.5.1.0\0"
#define kPGPdiskProductVerString	kPGPdiskFileVerString

#define	kPGPdiskCompanyName		"Network Associates Technology, Inc.\0"
#define	kPGPdiskCopyright		"Copyright (C) 1989-1999 Network Associates Technology, Inc.\0"
#define	kPGPdiskProductVerName	"PGPdisk for Windows v6.5.1i\0"

#endif	// ] Included_PGPdiskVersion_h
