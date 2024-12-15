/*
 * Copyright (c) 1998, Network Associates, Inc.
 * All rights reserved.
 *
 * $Id: pgpNetPMSA.h,v 1.12 1999/03/02 22:49:47 elowe Exp $
 *
 */
#ifndef _Included_pgpNetPMSA_h
#define _Included_pgpNetPMSA_h

#include "pgpPubTypes.h"
#include "pgpNetKernel.h"

PGPnetKernelSA *	PMFindSA(NDIS_HANDLE handle,
							 PGPUInt32 ipAddress,
							 PGPUInt32 ipAddrStart,
							 PGPBoolean incoming);

PGPnetKernelSA *	PMFindSAspi(NDIS_HANDLE handle,
							 PGPUInt32 ipAddress,
							 PGPUInt32 spi);

PGPnetKernelSA * 	PMAddSA(NDIS_HANDLE handle, 
							PGPikeSA *ikeSA);

PGPnetKernelSA *	PMAddPendingSA(NDIS_HANDLE handle,
							PGPUInt32 ipAddress,
							PGPUInt32 ipAddrStart,
							PGPUInt32 ipMaskEnd);

void				PMRemovePendingSA(NDIS_HANDLE handle,
							PGPUInt32 ipAddress,
							PGPUInt32 ipAddrStart,
							PGPUInt32 ipMaskEnd);

void				PMRemoveSA(NDIS_HANDLE handle,
							   PGPipsecSPI spi);

void				PMUpdateSA(NDIS_HANDLE handle,
							   PGPipsecSPI spi);

void				PMClearSAs(NDIS_HANDLE handle);


#endif /* _Included_pgpNetPMSA_h */
