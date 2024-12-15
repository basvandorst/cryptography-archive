/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: RDprecmp.h,v 1.7 1997/09/19 23:45:47 wjb Exp $
____________________________________________________________________________*/
#ifndef Included_RDPRECMP_h	/* [ */
#define Included_RDPRECMP_h


#define WIN32_LEAN_AND_MEAN

// Project defines
#define STATUS_FAILURE            0x0000000
#define STATUS_SUCCESS            0x0000001

#define UNFINISHED_CODE_ALLOWED 1
#define _PGPRDDLL

// System headers
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h> 
#include <string.h>
#include <Ole2.h>

#include "resource.h"                

// PGPlib headers
#include "..\..\shared\pgpBuildFlags.h"
#include "pgpKeys.h"
#include "pgpConfig.h"
#include "pgpErrors.h"
#include "pgpEncode.h"
#include "pgpUtilities.h"
#include "..\..\shared\pgpGroups.h"
#include "..\..\shared\pgpGroupsUtil.h"

// DLL headers
#include "..\include\PGPrecip.h"
#include "..\include\PGPcmdlg.h"
#include "..\include\PGPkm.h"
#include "..\include\PGPImage.h"
#include "..\..\shared\pgpClientPrefs.h"
#include "..\..\shared\pgpAdminPrefs.h"

// Project headers
#include "RDstruct.h"
#include "RDdraw.h"
#include "RDlstmng.h"
#include "RDlstsrt.h"
#include "RDserver.h"
#include "RDmove.h"
#include "RDlstsub.h"
#include "RDproc.h"
#include "..\include\help\RDhelp.h"
#include "RDkeyDB.h"
#include "resource.h" 
#include "RDdrag.h"
#include "RDwarn.h"

#define OPENKEYRINGFLAGS 0 //kPGPKeyRingOpenFlags_Mutable

#endif /* ] Included_RDPRECMP_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

