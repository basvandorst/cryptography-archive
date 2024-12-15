/*____________________________________________________________________________
	Copyright (C) 1999 Network Associates, Inc.
	All rights reserved.

	$Id: pgpProxyServer.h,v 1.1 1999/02/16 17:48:51 dgal Exp $
____________________________________________________________________________*/
#ifndef Included_pgpProxyServer_h	/* [ */
#define Included_pgpProxyServer_h

#include "pgpPubTypes.h"
#include "pgpErrors.h"

enum PGPProxyServerType_
{
	kPGPProxyServerType_Invalid		= 0,
	kPGPProxyServerType_HTTP		= 1,

	PGP_ENUM_FORCE(PGPProxyServerType_)
};

PGPENUM_TYPEDEF(PGPProxyServerType_, PGPProxyServerType);


PGP_BEGIN_C_DECLARATIONS

/* proxyAddress must be freed with PGPFreeData */

PGPError PGPGetProxyServer(PGPContextRef context, PGPProxyServerType type,
			char **proxyAddress, PGPUInt16 *proxyPort);

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpProxyServer_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/