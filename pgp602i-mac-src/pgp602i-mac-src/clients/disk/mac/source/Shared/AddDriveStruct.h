/*____________________________________________________________________________	Copyright (C) 1994-1998 Network Associates, Inc. and its affiliates.	All rights reserved.		$Id: AddDriveStruct.h,v 1.4.8.1 1998/11/12 03:06:02 heller Exp $____________________________________________________________________________*/#pragma once#include <CodeFragments.h>#include "MacFiles.h"#if PRAGMA_STRUCT_ALIGN#pragma options align=mac68k#endif#include "CipherContext.h"#include "CipherProc.h"typedef struct BWIcon	{	long	bytes[ ( kLargeIconSize / 2) / sizeof( long ) ];	long	mask[ ( kLargeIconSize / 2) / sizeof( long ) ];	} BWIcon;struct FlagsDrvQEl;typedef struct AddDriveStruct	{	short				resultDriveNumber;	short				fileRefNum;	const DiskExtent *	extents;	PGPUInt32			numExtents;	ulong				numBlocks;	CASTKey				decryptionKey;	PassphraseSalt		salt;		Boolean				mountReadOnly;	Boolean				minimizeWriteBufferSize;	Boolean				shouldBreakUpRequests;	Boolean				unused1;		BWIcon				mediaIcon;	BWIcon				physicalIcon;		FSSpec				prefsSpec;	} AddDriveStruct;#if PRAGMA_STRUCT_ALIGN#pragma options align=reset#endif