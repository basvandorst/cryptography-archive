/*____________________________________________________________________________	Copyright (C) 1994-1998 Network Associates Inc. and affiliated companies.	All rights reserved.		$Id: PGPDiskPatches.h,v 1.3 1999/03/10 02:49:44 heller Exp $____________________________________________________________________________*/#pragma onceconst OSType	kPatchesProcResType		= 'PACH';const short		kPatchesProcResID		= 1000;typedef OSStatus (*PGPDiskPatchesProcPtr)(Boolean *patchesInstalledResultPtr);