/*
 * pgpConf.h - Parse configuration files and lines.
 *
 * Copyright (C) 1993-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpConf.h,v 1.4.2.1 1997/06/07 09:51:39 mhw Exp $
 */

#ifndef PGPCONF_H
#define PGPCONF_H

#ifdef __cplusplus
extern "C" {
#endif

struct PgpEnv;
#ifndef TYPE_PGPENV
#define TYPE_PGPENV 1
typedef struct PgpEnv PgpEnv;
#endif

struct PgpUICb;
#ifndef TYPE_PGPUICB
#define TYPE_PGPUICB 1
typedef struct PgpUICb PgpUICb;
#endif

/* Process a single line */
int PGPExport pgpConfigLineProcess (struct PgpUICb const *ui, void *arg,
			struct PgpEnv *env, char *option, int pri);
/* Process a whole file */
int PGPExport pgpConfigFileProcess (struct PgpUICb const *ui, void *arg,
			 		struct PgpEnv *env, char const *filename, int pri);

#ifdef __cplusplus
}
#endif

#endif /* PGPCONF_H */
