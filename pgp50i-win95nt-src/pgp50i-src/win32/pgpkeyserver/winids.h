/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
#ifndef __WINIDS_H

#define __WINIDS_H

#define HARRY_CANTRELL		(WM_USER + 100)

#define PGPM_PROGRESS_UPDATE				(HARRY_CANTRELL + 1)
#define PGPM_DISMISS_PROGRESS_DLG			(HARRY_CANTRELL + 2)

/*KS_ID_START is in pgpkeyserversupport.h*/

#define ID_GET_KEY					KS_ID_START + 0
#define ID_SEND_KEYS_TO_SERVER		KS_ID_START + 1
#define ID_GET_NEW_KEY              KS_ID_START + 2

#endif //__WINIDS_H
