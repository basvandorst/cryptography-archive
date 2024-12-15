/*
 * Copyright (C) 1996, 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
/*++

  File: precomp.h

  Author: Mark B. Elrod (elrod@pgp.com)

  Date: November 15, 1996

  Revisions:

--*/




#ifndef _PRECOMP_
#define _PRECOMP_



// system include files
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <tchar.h>


#include "..\include\config.h"
#include "..\include\pgpTypes.h"
#include "..\include\pgpKeyDB.h"
#include "..\include\pgpMem.h"
#include "..\include\spgp.h"

// project include files
#include "define.h"
#include "pgpCodes.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#include "ShellAddKey.h"
#include "ShellEncryptSign.h"
#include "ShellDecryptVerify.h"
#include "..\include\PGPRecipientDialog.h"
#include "PassPhrase.h"

#ifdef __cplusplus
}
#endif




#endif // _PRECOMP_
