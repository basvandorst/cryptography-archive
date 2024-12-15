/*
 * ANSI C stadard, section 3.5.3: "An object that has volatile-qualified
 * type may be modified in ways unknown to the implementation or have
 * other unknown side effects."  Yes, we can't expect a compiler to
 * understand law...
 *
 * $Id: bnlegal.h,v 1.3 1997/05/06 00:23:45 mhw Exp $
 */

#include "pgpBase.h"

PGP_BEGIN_C_DECLARATIONS

extern volatile const char bnCopyright[];

PGP_END_C_DECLARATIONS
