/*
 * pgpPrefsInt.h -- Internal preference database header
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpPrefsInt.h,v 1.2.2.1 1997/06/07 09:50:31 mhw Exp $
 */

#ifndef PGPPREFSINT_H		/*	[ */
#define PGPPREFSINT_H

#include "pgpKDBint.h"

/*
 * Note that the structures below must be updated if the preference
 * names in pgpKeyDB.h change
 */

typedef struct NumPrefEntry_
{
		char const *		prefName;
		int							pgpEnvSelector;
		long						defValue;
} NumPrefEntry;

typedef struct StringPrefEntry_
{
		char const *		prefName;
		int							pgpEnvSelector;
		char const *		defValue;
} StringPrefEntry;

typedef struct BoolPrefEntry_
{
		char const *		prefName;
		int							pgpEnvSelector;
		Boolean					defValue;
} BoolPrefEntry;

typedef struct DataPrefEntry_
{
		char const *		prefName;
		int							pgpEnvSelector;
} DataPrefEntry;

extern NumPrefEntry pgpNumPrefTable[kPGPPrefNumLast - kPGPPrefNumFirst];
extern StringPrefEntry pgpStrPrefTable[kPGPPrefStrLast - kPGPPrefStrFirst];
extern BoolPrefEntry pgpBoolPrefTable[kPGPPrefBoolLast - kPGPPrefBoolFirst];
extern DataPrefEntry pgpDataPrefTable[kPGPPrefDataLast - kPGPPrefDataFirst];

#define numPrefEntry(prefName)																						\
				pgpNumPrefTable[(prefName) - kPGPPrefNumFirst]
#define strPrefEntry(prefName)																						\
				pgpStrPrefTable[(prefName) - kPGPPrefStrFirst]
#define boolPrefEntry(prefName)																					\
				pgpBoolPrefTable[(prefName) - kPGPPrefBoolFirst]
#define dataPrefEntry(prefName)																					\
				pgpDataPrefTable[(prefName) - kPGPPrefDataFirst]

#define pgpaPrefNameNumberValid(prefName)																\
				pgpaAssert((prefName) >= kPGPPrefNumFirst												\
								&& (prefName) < kPGPPrefNumLast)
#define pgpaPrefNameStringValid(prefName)																\
				pgpaAssert((prefName) >= kPGPPrefStrFirst												\
								&& (prefName) < kPGPPrefStrLast)
#define pgpaPrefNameBooleanValid(prefName)																\
				pgpaAssert((prefName) >= kPGPPrefBoolFirst												\
								&& (prefName) < kPGPPrefBoolLast)
#define pgpaPrefNameDataValid(prefName)																	\
				pgpaAssert((prefName) >= kPGPPrefDataFirst												\
								&& (prefName) < kPGPPrefDataLast)


#endif		/*	] PGPPREFSINT_H */

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
