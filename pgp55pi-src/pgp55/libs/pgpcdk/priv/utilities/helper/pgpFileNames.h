/*
 * pgpFileNames.h
 *
 * Utility functions to work with filenames in different OS's
 *
 * Based on pgpAppFile,
 *  written by:	Colin Plumb and Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpFileNames.h,v 1.5 1997/06/25 01:17:26 hal Exp $
 */

#include "pgpContext.h"

PGP_BEGIN_C_DECLARATIONS

char  *
pgpFileNameBuild (PGPContextRef cdkContext, char const *seg, ...);

char  *
pgpFileNameExtend (PGPContextRef cdkContext, char const *base,
					char const *ext);

char  *
pgpFileNameContract (PGPContextRef cdkContext, char const *base);

char  *
pgpFileNameDirectory (PGPContextRef cdkContext, char const *base);

char  *
pgpFileNameNextDirectory (PGPContextRef cdkContext, char const *path,
					char const **rest);

char  *
pgpFileNameExtendPath (PGPContextRef cdkContext, char const *path,
					char const *dir, int front);

/*
 * This returns the filename (not including directory path)
 * It returns a pointer within <path>, rather than dynamically allocating it
 */
char  *
pgpFileNameTail (PGPContextRef cdkContext, char const *path);

PGP_END_C_DECLARATIONS
