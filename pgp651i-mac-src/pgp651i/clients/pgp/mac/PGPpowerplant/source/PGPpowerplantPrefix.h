/*____________________________________________________________________________	Copyright (C) 1999 Network Associates Inc. and affiliated companies.	All rights reserved.	$Id: PGPpowerplantPrefix.h,v 1.2 1999/03/16 07:28:38 heller Exp $____________________________________________________________________________*/#pragma once#if __POWERPC__	#include "MacHeadersPPC++"#elif __CFM68K__	#include "MacHeadersCFM68K++"#else	#include "MacHeaders68K++"#endif#ifndef PGP_DEBUG	#define PGP_DEBUG	0#endif#define USE_PGP_OPERATOR_NEW	1