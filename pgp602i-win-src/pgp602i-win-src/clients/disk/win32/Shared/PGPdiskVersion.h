///////////////////////////////////////////////////////////////////////////////
// PGPdiskVersion.h
//
// Includes version constants and strings for the entire project.
///////////////////////////////////////////////////////////////////////////////

// $Id: PGPdiskVersion.h,v 1.2.2.27.2.18.2.1 1998/11/12 03:06:28 heller Exp $

// Copyright (C) 1998 by Network Associates, Inc.
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
#define	kVersionTextString	"6.0.2i"

// Version information that goes in the 'Version' resource of modules.
#define	kPGPdiskFileVerData		6,0,2,0
#define	kPGPdiskProductVerData	kPGPdiskFileVerData

#define kPGPdiskFileVerString		"6.0.2.0\0"
#define kPGPdiskProductVerString	kPGPdiskFileVerString

#define	kPGPdiskCompanyName		"Network Associates, Inc.\0"
#define	kPGPdiskCopyright		"Copyright (C) 1998 Network Associates, Inc.\0"
#define	kPGPdiskProductVerName	"PGPdisk for Windows v6.0.2i\0"

#endif	// ] Included_PGPdiskVersion_h
