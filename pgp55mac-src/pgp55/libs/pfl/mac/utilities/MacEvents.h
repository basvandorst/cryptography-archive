/*
 * MacEvents.h -- macintosh event handling utilities
 *
 * Copyright (C) 1996 Pretty Good Privacy, Inc.
 * All rights reserved.
 *
 */

#pragma once


PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif



UInt16		GetAllModifiers(void);
UInt16		GetModifiers(void);
Boolean		CapsLockIsDown(void);



#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS
