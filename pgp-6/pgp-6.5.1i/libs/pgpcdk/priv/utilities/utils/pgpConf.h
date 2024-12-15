/*
 * pgpConf.h - Parse configuration files and lines.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpConf.h,v 1.9 1997/06/25 19:41:43 lloyd Exp $
 */

#ifndef Included_pgpConf_h
#define Included_pgpConf_h

#include "pgpBase.h"

PGP_BEGIN_C_DECLARATIONS

#include "pgpOpaqueStructs.h"

/* Process a single line */
int  pgpConfigLineProcess (PGPUICb const *ui, void *arg,
			 PGPEnv *env, char *option, int pri);
/* Process a whole file */
int  pgpConfigFileProcess (PGPUICb const *ui, void *arg,
			 PGPEnv *env, char const *filename, int pri);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpConf_h */
