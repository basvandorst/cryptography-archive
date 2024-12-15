/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	(C) Copyright 1995-1996 by David K. Heller
	All rights reserved.

	David K. Heller
	730 E. Evelyn Avenue, #438
	Sunnyvale, CA  94086
	(408) 732-8960

	$Id: MacCursors.h,v 1.6.10.1 1997/12/07 04:27:09 mhw Exp $
____________________________________________________________________________*/

#pragma once

#include "MacBasics.h"

typedef struct OpaqueAnimatedCursorRef	*AnimatedCursorRef;

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif



OSStatus	GetAnimatedCursor(ResID resID, AnimatedCursorRef *cursorRef);
OSStatus	Get1AnimatedCursor(short resID, AnimatedCursorRef *cursorRef);
void		DisposeAnimatedCursor(AnimatedCursorRef cursorRef);

Boolean		AnimateCursor(AnimatedCursorRef cursorRef);
void		ResetAnimatedCursor(AnimatedCursorRef cursorRef);

void		SetAnimatedCursorDelay(AnimatedCursorRef cursorRef,
				UInt32 delayTicks);
void		SetAnimatedCursorInitialDelay(AnimatedCursorRef cursorRef,
				UInt32 initialTicks);



#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS
