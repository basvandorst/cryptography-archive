/*
 * Copyright (c) 1998, Network Associates, Inc.
 * All rights reserved.
 *
 * $Id: pgpNetPMConfig.h,v 1.3 1999/03/02 22:49:47 elowe Exp $
 *
 */
#ifndef _Included_pgpNetPMConfig_h
#define _Included_pgpNetPMConfig_h

PGPError PMAddConfig(NDIS_HANDLE handle, PGPnetKernelConfig *pConfig);
void	 PMClearConfig(NDIS_HANDLE handle);

#endif /* _Included_pgpNetPMHost_h */
