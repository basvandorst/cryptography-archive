/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	$Id: pgpKeyServerDialogCommon.h,v 1.4 1999/03/10 02:52:20 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpKeyServerDialogCommon_h	/* [ */
#define Included_pgpKeyServerDialogCommon_h

#include "pgpUserInterface.h"

PGP_BEGIN_C_DECLARATIONS

PGPError	PGPSearchKeyServerDialogCommon(
					PGPContextRef 			context,
					const PGPKeyServerSpec 	serverList[],
					PGPUInt32 				serverCount,
					PGPtlsContextRef		tlsContext,
					PGPFilterRef 			filterRef,
					PGPEventHandlerProcPtr 	eventHandler,
					PGPUserValue 			eventUserValue,
					PGPBoolean				searchAllServers,
					PGPKeySetRef 			*foundKeys);

PGPError	PGPSendToKeyServerDialogCommon(
					PGPContextRef 			context,
					const PGPKeyServerSpec 	*server,
					PGPtlsContextRef		tlsContext,
					PGPKeySetRef			keysToSend,
					PGPEventHandlerProcPtr 	eventHandler,
					PGPUserValue 			eventUserValue,
					PGPKeySetRef 			*failedKeys);
					
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpKeyServerDialogCommon_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
