/*
 * pgpAppFile.h -- build filenames for PGP applications
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * Written By:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpAppFile.h,v 1.3.2.1 1997/06/07 09:49:04 mhw Exp $
 */

#ifdef __cplusplus
extern "C" {
#endif

char *fileNameBuild (char const *seg, ...);
char *fileNameExtend (char const *base, char const *ext);
char *fileNameContract (char const *base);
char *fileNameNextDirectory (char const *path, char const **rest);
char *fileNameExtendPath (char const *path, char const *dir, int front);

#ifdef __cplusplus
}
#endif
