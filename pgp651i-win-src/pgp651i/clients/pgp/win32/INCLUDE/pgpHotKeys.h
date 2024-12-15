/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	pgpHotKeys.h	- 	common Windows HotKey definitions

	$Id: pgpHotKeys.h,v 1.3 1999/03/04 00:07:24 pbj Exp $
	
____________________________________________________________________________*/
#ifndef Included_pgpHotKeys_h	/* [ */
#define Included_pgpHotKeys_h

#include <windows.h>

#include "pgpPubTypes.h"

#define	kPGPHotKeyEnabled			0x80000000
#define kPGPHotKeyAltModifier		0x08000000
#define kPGPHotKeyCtrlModifier		0x04000000
#define kPGPHotKeyExtModifier		0x02000000
#define kPGPHotKeyShiftModifier		0x01000000

typedef struct PGPHotKeys {
	PGPUInt32	hotkeyPurgePassphraseCache;
	PGPUInt32	hotkeyEncrypt;
	PGPUInt32	hotkeySign;
	PGPUInt32	hotkeyEncryptSign;
	PGPUInt32	hotkeyDecrypt;
	PGPUInt32	hotkeyDiskUnmount;
} PGPHotKeys;

static const PGPHotKeys sDefaultHotKeys =
{
	kPGPHotKeyCtrlModifier|VK_F12,
	kPGPHotKeyCtrlModifier|kPGPHotKeyShiftModifier|'E',
	kPGPHotKeyCtrlModifier|kPGPHotKeyShiftModifier|'S',
	kPGPHotKeyCtrlModifier|kPGPHotKeyShiftModifier|'C',
	kPGPHotKeyCtrlModifier|kPGPHotKeyShiftModifier|'D',
	0x00
};

#endif /* ] Included_pgpHotKeys_h */
