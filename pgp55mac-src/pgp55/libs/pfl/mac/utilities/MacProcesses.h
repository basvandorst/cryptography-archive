/*
** Copyright (C) 1997 Pretty Good Privacy, Inc.
** All rights reserved.
*/

#pragma once

#include <Processes.h>

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif



Boolean		FindProcess(OSType type, OSType creator, ProcessSerialNumber *psn,
						ProcessInfoRec *processInfo, FSSpec *processSpec,
						StringPtr processName);



#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS

