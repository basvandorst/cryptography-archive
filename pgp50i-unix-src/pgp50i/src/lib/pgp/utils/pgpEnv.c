/*
 * pgpEnv.c -- Handle PGP Environments. This contains much of the
 * "global" PGP environment code. This contains the information that
 * is obtains from the configuration file or command-line or some
 * other means. It defines the environment in which it operates. It
 * is possible to have multiple threads operating in the same
 * environment, but in general each thread would have its own
 * environment. For example, a PGP Server would have one of these
 * for each client.
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpEnv.c,v 1.4.2.1 1997/06/07 09:51:40 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>				/* for memset() */

#include "pgpCharMap.h"				/* for pgpCharmaps() */
#include "pgpCipher.h"				/* for PGP_CIPHER_IDEA */
#include "pgpCompress.h"			/* for PGP_COMPRESSALG_ZIP */
#include "pgpHash.h"				/* for PGP_HASH_MD5 */
#include "pgpMem.h"
#include "pgpErr.h"
#include "pgpEnv.h"

struct Conf {
		union {
				int num;
				char *string;
		} val;
		int pri;
};

struct PgpEnv {
			struct Conf 	 		confInt[PGPENV_MAX_INT-
				 					 PGPENV_BASE_INT];
			struct Conf 	 		confString[PGPENV_MAX_STRING-
				 			 		 PGPENV_BASE_STRING];
			void *				pointers[PGPENV_MAX_POINTER -
				 			 		 PGPENV_BASE_POINTER];
			size_t				pointerSize[PGPENV_MAX_POINTER -
				 								PGPENV_BASE_POINTER];
	};

struct PgpEnv *
pgpenvCreate (void)
{
	struct PgpEnv *env;

	env = (struct PgpEnv *)pgpMemAlloc (sizeof (*env));
	if (!env)
		return NULL;

			memset (env, 0, sizeof (*env));

			/* Now initialize the environment pointers as appropriate */
			pgpenvSetInt (env, PGPENV_CIPHER, PGP_CIPHER_IDEA,
				 PGPENV_PRI_PUBDEFAULT);
			pgpenvSetInt (env, PGPENV_HASH, PGP_HASH_MD5, PGPENV_PRI_PUBDEFAULT);
			pgpenvSetInt (env, PGPENV_COMPRESS, 1, PGPENV_PRI_PUBDEFAULT);
			pgpenvSetInt (env, PGPENV_COMPRESSALG, PGP_COMPRESSALG_ZIP,
				 PGPENV_PRI_PUBDEFAULT);
			pgpenvSetInt (env, PGPENV_COMPRESSQUAL, 5, PGPENV_PRI_PUBDEFAULT);
			pgpenvSetInt (env, PGPENV_ARMORLINES, 720, PGPENV_PRI_PUBDEFAULT);
			pgpenvSetInt (env, PGPENV_CERTDEPTH, 4, PGPENV_PRI_PUBDEFAULT);
			pgpenvSetInt (env, PGPENV_COMPLETES, 1, PGPENV_PRI_PUBDEFAULT);
			pgpenvSetInt (env, PGPENV_MARGINALS, 2, PGPENV_PRI_PUBDEFAULT);
			pgpenvSetInt (env, PGPENV_TRUSTED, 120, PGPENV_PRI_PUBDEFAULT);
			pgpenvSetInt (env, PGPENV_CLEARSIG, 1, PGPENV_PRI_PUBDEFAULT);
			pgpenvSetInt (env, PGPENV_VERSION, PGPVERSION_2_6,
				 PGPENV_PRI_PUBDEFAULT);

			pgpenvSetString (env, PGPENV_PGPPATH, "", PGPENV_PRI_PUBDEFAULT);
			pgpenvSetString (env, PGPENV_UPATH,
				 	"/home/ch/keyserver", PGPENV_PRI_PUBDEFAULT);

#ifdef MSDOS
			pgpenvSetString (env, PGPENV_CHARSET, "cp850", PGPENV_PRI_PUBDEFAULT);
#else
			pgpenvSetString (env, PGPENV_CHARSET, "noconv", PGPENV_PRI_PUBDEFAULT);
#endif
		
			return env;
	}

/*
 * Copy an environment. WARNING: Although the strings will be copied,
 * the "pointers" will not. Take care when destroying an environment;
 * you can only destroy the pointers from the first one. Since the
 * environment doesn't do this on its own, this is not a problem for
 * this code.
 */
struct PgpEnv *
pgpenvCopy (struct PgpEnv const *env)
{
			struct PgpEnv *tmp;
			char const *s;
			char *s2;
			int i;

if (!env)
	return NULL;

/* Create the new env */
tmp = (struct PgpEnv *)pgpMemAlloc (sizeof (*tmp));
if (!tmp)
	return NULL;

/* Clone the old env */
memcpy (tmp, env, sizeof (*env));

/* Alloc and copy the strings */
for (i=0; i < PGPENV_MAX_STRING-PGPENV_BASE_STRING; i++) {
			s = tmp->confString[i].val.string;
			if (s) {
						s2 = (char *)pgpMemAlloc (strlen (s) + 1);
						if (s2)
							memcpy (s2, s, strlen (s) + 1);

							tmp->confString[i].val.string = s2;
					}
		}
			return tmp;
}

void
pgpenvDestroy (struct PgpEnv *env)
{
			int i;
			char *s;

			if (!env)
				 return;

			/* De-allocate all the string variables */
			for (i=0; i < PGPENV_MAX_STRING-PGPENV_BASE_STRING; i++) {
					s = env->confString[i].val.string;
					if (s) {
							memset (s, 0, strlen (s));
							pgpMemFree (s);
					}
		}
						
			memset (env, 0, sizeof (*env));
			pgpMemFree (env);
}

/*
 * Set an Integer Value for the environment variable var in the
 * environment env to num with priority pri.
 */
int
pgpenvSetInt (struct PgpEnv *env, enum PgpEnvInts var, int num, int pri)
	{
			int idx;
		
			if (!env)
				 return PGPERR_BADPARAM;

			if (var < PGPENV_BASE_INT || var >= PGPENV_MAX_INT)
				 return PGPERR_ENV_BADVAR;

			idx = (int)var - PGPENV_BASE_INT;

			if (pri < env->confInt[idx].pri)
				 return PGPERR_ENV_LOWPRI;

env->confInt[idx].val.num = num;
env->confInt[idx].pri = pri;
return 0;
}

static int
DoSetString (struct PgpEnv *env, unsigned var, char const *string, int pri)
{
			char *s;

var -= PGPENV_BASE_STRING;
if (string) {
				s = (char *)pgpMemRealloc (env->confString[var].val.string,
							strlen (string) + 1);
				if (s == NULL)
					return PGPERR_NOMEM;

			strcpy (s, string);
		} else {
			if (env->confString[var].val.string)
				 pgpMemFree (env->confString[var].val.string);
			s = NULL;
		}

env->confString[var].val.string = s;
env->confString[var].pri = pri;
return 0;
}

static int
SetPointer (struct PgpEnv *env, unsigned var, void *ptr, size_t size)
{
			var -= PGPENV_BASE_POINTER;
			env->pointers[var] = ptr;
			env->pointerSize[var] = size;
			return 0;
}

/*
 * Set a String Value for the environment variable var in the
 * environment env to string with priority pri. Space is allocated
 * for the string.
 *
 * If the string is null, free the old string (if one existed) and
 * then set the placeholder to NULL.
 */
int
pgpenvSetString (struct PgpEnv *env, enum PgpEnvStrings var,
				 char const *string, int pri)
	{
		int err = 0;

		if (!env)
			return PGPERR_BADPARAM;

		if (var < PGPENV_BASE_STRING || var >= PGPENV_MAX_STRING)
			return PGPERR_ENV_BADVAR;

			if (pri < env->confString[var-PGPENV_BASE_STRING].pri)
				 return PGPERR_ENV_LOWPRI;	/* not assigned */

			switch (var) {
			case PGPENV_CHARSET:
					{
							byte const *toLocal, *toLatin1;

							err = pgpCharmaps (string, strlen (string), &toLocal,
								 	&toLatin1);
							if (!err) {
									DoSetString (env, PGPENV_CHARSET, string, pri);
									SetPointer (env, PGPENV_CHARMAPTOLOCAL,
										(void *)toLocal, 256);
									SetPointer (env, PGPENV_CHARMAPTOLATIN1,
											(void *)toLatin1, 256);
							}
					}
					break;
			default:
				 err = DoSetString (env, var, string, pri);
			}

			return err;
}

/*
 * Set a Pointer Value for the environment variable var in the
 * environment env to the Pointer passed in
 */
int
pgpenvSetPointer (struct PgpEnv *env, enum PgpEnvPointers var, void *ptr,
				 	size_t size)
{
			if (!env)
				 return PGPERR_BADPARAM;

			if (var < PGPENV_BASE_POINTER || var >= PGPENV_MAX_POINTER)
				 return PGPERR_ENV_BADVAR;

 			if (var == PGPENV_CHARMAPTOLOCAL || var == PGPENV_CHARMAPTOLATIN1)
				 return PGPERR_ENV_BADVAR;

			return SetPointer (env, var, ptr, size);
}

int
pgpenvGetInt (struct PgpEnv const *env, enum PgpEnvInts var, int *pri,
			int *error)
	{
			int err = 0;
			int val = 0;
			int idx;

			if (!env) {
				 err = PGPERR_BADPARAM;
				 goto getint_end;
			}

			if (var < PGPENV_BASE_INT || var >= PGPENV_MAX_INT) {
				 err = PGPERR_ENV_BADVAR;
				 goto getint_end;
			}

			idx = (int)var - PGPENV_BASE_INT;

			if (pri)
				 *pri = env->confInt[idx].pri;

			val = env->confInt[idx].val.num;

getint_end:
	if (error)
		*error = err;

	return val;
}

char const *
pgpenvGetString (struct PgpEnv const *env, enum PgpEnvStrings var, int *pri,
				 int *error)
{
			int err = 0;
			char const *s = NULL;
			int idx;

			if (!env) {
				 err = PGPERR_BADPARAM;
				 goto getstr_end;
			}

			if (var < PGPENV_BASE_STRING || var >= PGPENV_MAX_STRING) {
				 err = PGPERR_ENV_BADVAR;
				 goto getstr_end;
			}

		idx = (int)var - PGPENV_BASE_STRING;

		if (pri)
			*pri = env->confString[idx].pri;

		s = env->confString[idx].val.string;

		getstr_end:
			if (error)
				 *error = err;

	return s;
}

void *
pgpenvGetPointer (struct PgpEnv const *env, enum PgpEnvPointers var,
		size_t *size, int *error)
{
	int err = 0;
	void *ptr = NULL;
int idx;

if (!env) {
	err = PGPERR_BADPARAM;
	goto getptr_end;
}

if (var < PGPENV_BASE_POINTER || var >= PGPENV_MAX_POINTER) {
	err = PGPERR_ENV_BADVAR;
	goto getptr_end;
}

idx = (int)var - PGPENV_BASE_POINTER;

if (size)
	*size = env->pointerSize[idx];

ptr = env->pointers[idx];

	getptr_end:
	if (error)
		*error = err;

	return ptr;
}
