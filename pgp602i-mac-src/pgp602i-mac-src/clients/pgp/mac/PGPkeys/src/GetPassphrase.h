/*____________________________________________________________________________	Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.			$Id: GetPassphrase.h,v 1.5.8.1 1998/11/12 03:08:21 heller Exp $____________________________________________________________________________*/#pragma once#include "pgpKeys.h"PGPError DoPassphraseDialog(PGPKeyRef theKey, char *passphrase);PGPError GetPassForKey(	PGPKeyRef		key,						PGPBoolean		*isSplit,						char			*passphrase,						PGPByte			**passKey,						PGPSize			*passKeySize );