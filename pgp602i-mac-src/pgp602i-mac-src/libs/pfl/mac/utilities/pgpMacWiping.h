/*____________________________________________________________________________	Copyright (C) 1998 Network Associates, Inc. and its affiliates.	All rights reserved.		$Id: pgpMacWiping.h,v 1.1.12.1 1998/11/12 03:19:58 heller Exp $____________________________________________________________________________*/#ifndef Included_pgpMacWiping_h	/* [ */#define Included_pgpMacWiping_h#include "pgpBase.h"#define kNumPGPWipingPatternBytes	512Ltypedef struct PGPWipingPattern{	PGPByte	bytes[kNumPGPWipingPatternBytes];} PGPWipingPattern;#endif /* ] Included_pgpMacWiping_h *//*__Editor_settings____	Local Variables:	tab-width: 4	End:	vi: ts=4 sw=4	vim: si_____________________*/