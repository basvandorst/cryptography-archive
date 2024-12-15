/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: GUIDS.CPP,v 1.7 1999/03/10 02:42:57 heller Exp $
____________________________________________________________________________*/

#define INITGUID
#define USES_IID_IExchExt
#define USES_IID_IExchExtPropertySheets
#define USES_IID_IExchExtMessageEvents
#define USES_IID_IExchExtAttachedFileEvents
#define USES_IID_IMessage
#define USES_IID_IDistList
#define USES_IID_IABContainer

#include <objbase.h>
#include <initguid.h>
#include <mapiguid.h>

#define DEFINE_EXCHEXTGUID(name, b) \
    DEFINE_GUID(name, 0x00020D00 | (b), 0, 0, 0xC0,0,0,0,0,0,0,0x46)

#ifndef NOEXCHEXTGUIDS
DEFINE_EXCHEXTGUID(IID_IExchExtCallback,            0x10);
DEFINE_EXCHEXTGUID(IID_IExchExt,                    0x11);
DEFINE_EXCHEXTGUID(IID_IExchExtCommands,            0x12);
DEFINE_EXCHEXTGUID(IID_IExchExtUserEvents,          0x13);
DEFINE_EXCHEXTGUID(IID_IExchExtSessionEvents,       0x14);
DEFINE_EXCHEXTGUID(IID_IExchExtMessageEvents,       0x15);
DEFINE_EXCHEXTGUID(IID_IExchExtAttachedFileEvents,  0x16);
DEFINE_EXCHEXTGUID(IID_IExchExtPropertySheets,      0x17);
DEFINE_EXCHEXTGUID(IID_IExchExtAdvancedCriteria,    0x18);
DEFINE_EXCHEXTGUID(IID_IExchExtModeless,            0x19);
DEFINE_EXCHEXTGUID(IID_IExchExtModelessCallback,    0x1a);
#endif // NOEXCHEXTGUIDS

// Application-specific GUIDs

#include "guids.h"



/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
