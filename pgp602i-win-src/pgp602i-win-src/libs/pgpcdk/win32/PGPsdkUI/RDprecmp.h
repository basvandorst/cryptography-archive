/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: RDprecmp.h,v 1.12.10.1 1998/11/12 03:24:32 heller Exp $
____________________________________________________________________________*/
#ifndef Included_RDPRECMP_h	/* [ */
#define Included_RDPRECMP_h

#include <stdio.h>
#include <stdlib.h>
#include <Ole2.h>

// Project headers
#include "pgpui32.h"
#include "pgpRecipientDialogCommon.h"
#include "RDstruct.h"
#include "RDdraw.h"
#include "RDlstmng.h"
#include "RDlstsrt.h"
#include "RDserver.h"
#include "RDmove.h"
#include "RDlstsub.h"
#include "RDproc.h"
#include "RDkeyDB.h"
#include "RDdrag.h"
#include "PGPImage.h"
#include "resource.h" 

#ifdef __cplusplus
extern "C" {
#endif

LRESULT 
PGPsdkUIMessageBox (
		 HWND	hWnd, 
		 INT	iCaption, 
		 INT	iMessage,
		 ULONG	ulFlags) ;

UINT newPGPRecipientDialog(PRECGBL prg);

#ifdef __cplusplus
}
#endif

#endif /* ] Included_RDPRECMP_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

