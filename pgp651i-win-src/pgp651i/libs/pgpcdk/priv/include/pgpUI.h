/*
 * pgpUI.h -- Header for PGP UI Callback defintions.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * This is a Public API Function Header.
 *
 * $Id: pgpUI.h,v 1.8 1997/07/16 01:44:50 heller Exp $
 */

#ifndef Included_pgpUI_h
#define Included_pgpUI_h

#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS

/*
 * This is the structure that contains the callback function pointers
 * for the PGP UI callbacks.  An application fills in this structure
 * with the appropriate functions to perform the callbacks.  Some
 * callbacks support additional arguments to the msg.  Callback
 * functions should use stdarg, where each argument is of type "struct
 * PgpUICbArg const *"
 */
struct PGPUICb
{
	/* required callbacks */
	int (*message) (void *arg, int type, int msg, unsigned numargs, ...);
	int (*doCommit) (void *arg, int scope);
	int (*newOutput) (void *arg, PGPPipeline **output,
			  int type, char const *suggested_name);
	int (*needInput) (void *arg, PGPPipeline *head);
	int (*sigVerify) (void *arg, PGPSig const *sig,
			   PGPByte const *hash);
	int (*eskDecrypt) (void *arg, PGPESK const *esklist,
			   PGPByte *key, PGPSize *keylen,
			   int (*tryKey) (void *arg, PGPByte const *key,
					  PGPSize keylen),
			   void *tryarg);
	/* optional callbacks below this line */
	int (*annotate) (void *arg, PGPPipeline *origin, int type,
			 PGPByte const *string, PGPSize size);
} ;



/* Define whether a callback argument is an int or a string */
#define PGP_UI_ARG_INT		1
#define PGP_UI_ARG_UNSIGNED	2
#define PGP_UI_ARG_WORD32	3
#define PGP_UI_ARG_STRING	4
#define PGP_UI_ARG_BUFFER	5

/*
 * This structure is used for callback arguments.  Whenever a callback
 * of any sort requires arguments, this structure is used to hold the
 * arguments.  The type is any PGP_UI_ARG_* and the appropriate entry
 * in val is used.
 */
typedef struct PgpUICbArg
{
	int type;
	union {
		int	i;
		unsigned u;
		PGPUInt32	w32;
		char const *s;
		struct {
			PGPByte const *buf;
			unsigned len;
		} buf;
	} val;
} PgpUICbArg ;

PGP_END_C_DECLARATIONS

#endif /* Included_pgpUI_h */
