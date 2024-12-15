/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
/*++

Module Name:

    precomp.h

Abstract:

    This file includes all the headers required to build the dll
    to ease the process of building a precompiled header.

Author:


Revision History:


--*/

#ifndef _PRECOMP_H
#define _PRECOMP_H

#define WIN32_LEAN_AND_MEAN
#define PGPLIB

// Project defines
#define STATUS_FAILURE           0x0000000
#define STATUS_SUCCESS           0x0000001

// System headers
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "resource.h"

// PGPlib headers
#include "..\include\config.h"
#include "..\include\pgpTypes.h"
#include "..\include\pgpKeyDB.h"
#include "..\include\pgpMem.h"

// DLL headers
#include "..\include\PGPkeyserversupport.h"
#include "..\include\pgpcomdlg.h"
#include "..\include\PGPRecip.h"

// Project headers
#include "structs.h"
#include "strstri.h"
#include "ListSort.h"
#include "KeyServe.h"
#include "MoveItem.h"
#include "OwnDraw.h"
#include "ListSub.h"
#include "RecProc.h"
#include "RecHelp.h"
#include "RecKeyDB.h"
#include "resource.h"
#include "strstri.h"
#include "DragItem.h"
#include "WarnProc.h"
#include "conerror.h"

#endif  // _PRECOMP_H
