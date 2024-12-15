/*
 * pgpFileNames.h
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Utility functions to work with filenames in different OS's
 *
 * Based on pgpAppFile,
 * written by: Colin Plumb and Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpFileNames.h,v 1.3.2.1 1997/06/07 09:50:05 mhw Exp $
 */

#ifdef __cplusplus
extern "C" {
#endif

char PGPExport *
pgpFileNameBuild (char const *seg, ...);

char PGPExport *
pgpFileNameExtend (char const *base, char const *ext);

char PGPExport *
pgpFileNameContract (char const *base);

char PGPExport *
pgpFileNameDirectory (char const *base);

char PGPExport *
pgpFileNameNextDirectory (char const *path, char const **rest);

char PGPExport *
pgpFileNameExtendPath (char const *path, char const *dir, int front);

/*
 * This returns the filename (not including directory path)
 * It returns a pointer within <path>, rather than dynamically allocating it
 */
char PGPExport *
pgpFileNameTail (char const *path);

#ifdef __cplusplus
}
#endif
