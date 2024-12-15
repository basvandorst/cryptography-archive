/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#pragma once

no longer used; use pfl/common/util/pgpWordWrap.c

#ifdef __cplusplus
extern "C" {
#endif

OSErr	FSpWordWrapFile( const FSSpec *	inSpec, const FSSpec * outSpec,
			ushort wrapLength );


#ifdef __cplusplus
}
#endif