/*
 * pgpConvKey.h -- Conventional Encryption Keys for PGP
 *
 * Written By:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpConvKey.h,v 1.6 1997/07/29 20:31:01 lloyd Exp $
 */

#ifndef Included_pgpConvKey_h
#define Included_pgpConvKey_h

#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS


struct PGPConvKey {
	PGPConvKey const *		next;
	PGPStringToKey const *	stringToKey;
	char  *			pass;
	size_t					passlen;
};


PGP_END_C_DECLARATIONS

#endif /* Included_pgpConvKey_h */
