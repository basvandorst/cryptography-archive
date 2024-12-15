/*
 * Copyright (C) 1996 Pretty Good Privacy, Inc.
 * All rights reserved.
 *
 */

#pragma once

enum
{
	kOSErrorStringListResID		= 31600
};

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif



Boolean		GetOSErrorString(OSStatus error, StringPtr errorStr);
PGPError	MacErrorToPGPError( OSStatus macErr );
OSStatus	PGPErrorToMacError( PGPError pgpErr );



#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

