/*
* pgpUI.h -- Header for PGP UI Callback defintions.
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:	Derek Atkins <warlord@MIT.EDU>
*
* This is a Public API Function Header.
*
* $Id: pgpUI.h,v 1.3.2.1 1997/06/07 09:50:20 mhw Exp $
*/

#ifndef PGPUI_H
#define PGPUI_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpPipeline;
#ifndef TYPE_PGPPIPELINE
#define TYPE_PGPPIPELINE 1
typedef struct PgpPipeline PgpPipeline;
#endif

struct PgpESK;
#ifndef TYPE_PGPESK
#define TYPE_PGPESK 1
typedef struct PgpESK PgpESK;
#endif

struct PgpSig;
#ifndef TYPE_PGPSIG
#define TYPE_PGPSIG 1
typedef struct PgpSig PgpSig;
#endif

/*
* This is the structure that contains the callback function pointers
* for the PGP UI callbacks. An application fills in this structure
* with the appropriate functions to perform the callbacks. Some
* callbacks support additional arguments to the msg. Callback
* functions should use stdarg, where each argument is of type "struct
* PgpUICbArg const *"
*/
struct PgpUICb {
			/* required callbacks */
			int (*message) (void *arg, int type, int msg, unsigned numargs, ...);
			int (*doCommit) (void *arg, int scope);
			int (*newOutput) (void *arg, struct PgpPipeline **output,
				 	int type, char const *suggested_name);
			int (*needInput) (void *arg, struct PgpPipeline *head);
			int (*sigVerify) (void *arg, struct PgpSig const *sig,
			  	byte const *hash);
			int (*eskDecrypt) (void *arg, struct PgpESK const *esklist,
				 	byte *key, size_t *keylen,
				 	int (*tryKey) (void *arg, byte const *key,
				 		 	size_t keylen),
				 	void *tryarg);
			/* optional callbacks below this line */
			int (*annotate) (void *arg, struct PgpPipeline *origin, int type,
				 	byte const *string, size_t size);
	};
#ifndef TYPE_PGPUICB
#define TYPE_PGPUICB 1
typedef struct PgpUICb PgpUICb;
#endif

/* Define whether a callback argument is an int or a string */
	#define PGP_UI_ARG_INT	 		1
	#define PGP_UI_ARG_UNSIGNED		2
	#define PGP_UI_ARG_WORD32		3
	#define PGP_UI_ARG_STRING		4
	#define PGP_UI_ARG_BUFFER		5

/*
* This structure is used for callback arguments. Whenever a callback
* of any sort requires arguments, this structure is used to hold the
* arguments. The type is any PGP_UI_ARG_* and the appropriate entry
* in val is used.
*/
struct PgpUICbArg {
			int type;
			union {
					int		i;
					unsigned u;
					word32	 w32;
					char const *s;
					struct {
							byte const *buf;
							unsigned len;
					} buf;
			} val;
	};
#ifndef TYPE_PGPUICBARG
#define TYPE_PGPUICBARG 1
typedef struct PgpUICbArg PgpUICbArg;
#endif

#ifdef __cplusplus
}
#endif

#endif /* PGPUI_H */
