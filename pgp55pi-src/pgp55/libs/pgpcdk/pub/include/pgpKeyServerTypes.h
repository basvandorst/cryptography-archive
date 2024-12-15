/*____________________________________________________________________________
	pgpKeyServerTypes.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	Error codes for all PGP KeyServer errors can be found in this file.

	$Id: pgpKeyServerTypes.h,v 1.4 1997/09/14 20:46:59 lloyd Exp $
____________________________________________________________________________*/
#ifndef Included_pgpKeyServerTypes_h	/* [ */
#define Included_pgpKeyServerTypes_h

#include "pgpPubTypes.h"

#define kPGPKeyServerStateConnect		0x01
#define kPGPKeyServerStateWait			0x02
#define kPGPKeyServerStateReceive		0x03
#define kPGPKeyServerStateSend			0x04
#define kPGPKeyServerStateDisconnect	0x05

enum PGPKeyServerKeySpace_
{
	kPGPKSKeySpaceDefault = 1,
	kPGPKSKeySpaceNormal,
	kPGPKSKeySpacePending,

	PGP_ENUM_FORCE( PGPKeyServerKeySpace_ )
};

PGPENUM_TYPEDEF( PGPKeyServerKeySpace_, PGPKeyServerKeySpace );

enum PGPKeyServerAccessType_
{
	kPGPKSAccess_Default = 1,
	kPGPKSAccess_Normal,
	kPGPKSAccess_Administrator,

	PGP_ENUM_FORCE( PGPKeyServerAccessType_ )
};

PGPENUM_TYPEDEF( PGPKeyServerAccessType_, PGPKeyServerAccessType );

typedef struct PGPKeyServerMonitor 
{
#define kPGPKeyServerMonitorMagic	0x4D4F4E49
	PGPUInt32					magic;
	char						*monitorTag;
	char						**monitorValues;
	struct PGPKeyServerMonitor	*next;
} PGPKeyServerMonitor;

typedef PGPKeyServerMonitor * PGPKeyServerMonitorRef;

#endif /* ] Included_pgpKeyServerTypes_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
