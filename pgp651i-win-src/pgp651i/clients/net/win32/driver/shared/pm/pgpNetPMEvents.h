/*
 * Copyright (c) 1998, Network Associates, Inc.
 * All rights reserved.
 *
 * $Id: pgpNetPMEvents.h,v 1.1 1999/05/21 21:29:09 elowe Exp $
 *
 */
#ifndef _Included_pgpNetPMEvents_h
#define _Included_pgpNetPMEvents_h


void PMfireSADiedEvent(PVPN_ADAPTER adaptor, PGPipsecSPI spi);

void PMfireSARekeyEvent(PVPN_ADAPTER adapter, PGPipsecSPI spi);

void PMfireErrorEvent(PVPN_ADAPTER adapter,
					  PGPUInt32 ipAddress, 
					  PGPUInt32 errorCode);

PGPnetPMStatus PMfireSARequestEvent(PVPN_ADAPTER adaptor,
									PGPUInt32 ipAddress,
									PGPUInt32 ipAddrStart,
									PGPUInt32 ipMaskEnd);
#endif