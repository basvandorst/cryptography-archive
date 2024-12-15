/*
 * Copyright (c) 1998, Network Associates, Inc.
 * All rights reserved.
 *
 * $Id: pgpNetPMHost.h,v 1.8 1998/12/05 00:54:20 elowe Exp $
 *
 */
#ifndef _Included_pgpNetPMHost_h
#define _Included_pgpNetPMHost_h

#include "pgpPubTypes.h"
#include "pgpNetKernel.h"

enum PGPNetHostType_
{
	kPGPnetInsecureHost			= 0,
	kPGPnetSecureHost			= 1,
	kPGPnetSecureGateway		= 2,

	PGP_ENUM_FORCE( PGPNetHostType_ )
};
PGPENUM_TYPEDEF( PGPNetHostType_, PGPNetHostType );

PGPBoolean 			PMIsSecureHost(NDIS_HANDLE handle,
								   PGPUInt32 ipAddress);

PGPBoolean 			PMIsInsecureHost(NDIS_HANDLE handle,
									 PGPUInt32 ipAddress);

PGPNetHostEntry *	PMFindHost(NDIS_HANDLE handle,
							   PGPUInt32 ipAddress);

PGPUInt32			PMParentHostIP(NDIS_HANDLE handle, 
								   struct _PGPNetHostEntry *);

PGPNetHostEntry *	PMAddHost(NDIS_HANDLE handle,
							  void *host);

void				PMClearHosts(NDIS_HANDLE handle);


#endif /* _Included_pgpNetPMHost_h */
