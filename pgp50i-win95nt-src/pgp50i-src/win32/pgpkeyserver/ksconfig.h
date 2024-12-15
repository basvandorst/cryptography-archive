/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
KSERR PGPExport GetKeyserverValues(char *Keyserver, unsigned short *Port,
									BOOL *AutoGet);
KSERR SetKeyserverValues(char *Keyserver, unsigned short Port, BOOL AutoGet);

#define DEFAULT_KEYSERVER	"horowitz.surfnet.nl"
