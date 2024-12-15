/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
____________________________________________________________________________*/

#include "pgpTypes.h"

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif



UInt16		GetSystemVersion( void );
Boolean		VirtualMemoryIsOn( void );
void		GetMachineOwnerName(StringPtr owner);
void		InitMacToolbox(void);


#if powerc
	#define		RunningOnPowerPC()		( true )
#else
	Boolean		RunningOnPowerPC( void );
#endif



#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS