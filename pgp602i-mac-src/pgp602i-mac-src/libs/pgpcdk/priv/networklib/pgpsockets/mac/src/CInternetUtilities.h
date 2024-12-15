/*____________________________________________________________________________	Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.				$Id: CInternetUtilities.h,v 1.4.10.1 1998/11/12 03:22:24 heller Exp $____________________________________________________________________________*/#pragma once#include "CSocket.h"#include "MacTCP.h"class CInternetUtilities {public:							CInternetUtilities() { }	virtual					~CInternetUtilities() { }	virtual PGPHostEntry *	GetHostByName(const char * inName) = 0;	virtual	PGPHostEntry *	GetHostByAddress(PGPInternetAddress inAddress) = 0;	virtual void			GetHostName(char * outName,									SInt32 inNameLength) = 0;};