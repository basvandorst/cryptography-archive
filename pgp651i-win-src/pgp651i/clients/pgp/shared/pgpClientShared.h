/*____________________________________________________________________________
	Included_pgpClientShared_h.h

	Copyright (C) 1998 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpClientShared.h,v 1.2 1999/03/10 02:47:39 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpClientShared_h	/* [ */
#define Included_pgpClientShared_h

#include "pgpBase.h"

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

enum
{
	kPGPPhotoIDHeaderVersion	= 1,
	
	kPGPPhotoIDFormat_Invalid	= 0,
	kPGPPhotoIDFormat_JPEG		= 1
};
	
typedef struct PGPPhotoUserIDHeader
{
	PGPUInt16	headerLength;
	PGPUInt8	headerVersion;	/* kPGPPhotoIDHeaderVersion */
	PGPUInt8	photoIDFormat;
	PGPUInt32	reserved2[3];	/* Must be zero */

} PGPPhotoUserIDHeader;

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

PGP_BEGIN_C_DECLARATIONS

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif

PGP_END_C_DECLARATIONS


#endif /* ] Included_pgpClientShared_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/