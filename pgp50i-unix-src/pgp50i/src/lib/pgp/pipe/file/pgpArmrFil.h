/*
* pgpArmrFil.h -- Armor Write File module
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Creates output files based upon a "name". If the data is within a
* MULTIARMOR scope, multiple files will be opened for the output.
* Otherwise, just a single file will be output when data is first
* written into this module.
*
* Written By:	Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpArmrFil.h,v 1.4.2.1 1997/06/07 09:50:53 mhw Exp $
*/

#ifndef PGPARMRFIL_H
#define PGPARMRFIL_H

#ifdef __cplusplus
extern "C" {
#endif

struct PgpPipeline;
#ifndef TYPE_PGPPIPELINE
#define TYPE_PGPPIPELINE 1
typedef struct PgpPipeline PgpPipeline;
#endif

struct PgpFile;
#ifndef TYPE_PGPFILE
#define TYPE_PGPFILE 1
typedef struct PgpFile PgpFile;
#endif

struct PgpPipeline PGPExport *
pgpArmorFileWriteCreate (struct PgpPipeline **head,
							struct PgpFile * (*fileOpen) (void *arg,
								 			void const *baseName,
								 			unsigned number),
							void *arg,
							void const *baseName);

#ifdef __cplusplus
}
#endif

#endif /* PGPARMRFIL_H */
