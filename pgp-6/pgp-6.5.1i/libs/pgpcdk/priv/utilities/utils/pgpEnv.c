/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	
 * pgpEnv.c -- Handle PGP Environments.  This contains much of the
 * "global" PGP environment code.  This contains the information that
 * is obtains from the configuration file or command-line or some
 * other means.  It defines the environment in which it operates.  It
 * is possible to have multiple threads operating in the same
 * environment, but in general each thread would have its own
 * environment.  For example, a PGP Server would have one of these
 * for each client.

	$Id: pgpEnv.c,v 1.49 1999/03/10 02:50:11 heller Exp $
____________________________________________________________________________*/


#include "pgpConfig.h"

#include <stdio.h>
#include <string.h>	

#include "pgpCharMap.h"		/* for pgpCharmaps() */
#include "pgpSymmetricCipher.h"		/* for kPGPCipherAlgorithm_IDEA */
#include "pgpCompress.h"	/* for PGP_COMPRESSALG_ZIP */
#include "pgpHash.h"		/* for kPGPHashAlgorithm_MD5 */
#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpEnv.h"
#include "pgpContext.h"
#include "pgpFileSpec.h"

typedef struct Conf
{
	union
	{
		int		num;
		char *	string;
	} val;
	int			pri;
} Conf;

struct PGPEnv
{
	Conf 	confInt[ PGPENV_MAX_INT- PGPENV_BASE_INT ];
	Conf 	confString[PGPENV_MAX_STRING-
						PGPENV_BASE_STRING];
	void *			pointers[PGPENV_MAX_POINTER -
						PGPENV_BASE_POINTER];
	size_t			pointerSize[PGPENV_MAX_POINTER -
						PGPENV_BASE_POINTER];
	
	PGPContextRef	context;
} ;

	PGPError 
pgpenvCreate(
	PGPContextRef	context,
	PGPEnv **		outEnv)
{
	PGPEnv *	env	= NULL;
	PGPError	err	= kPGPError_NoErr;

	pgpAssertAddrValid( outEnv, PGPEnv * );
	
	*outEnv	= NULL;
	
	env = (PGPEnv *)pgpContextMemAlloc( context, sizeof(*env),
		kPGPMemoryMgrFlags_Clear);
	if ( IsntNull( env ) )
	{
		env->context	= context;

		/* Now initialize the environment pointers as appropriate */
		pgpenvSetInt (env, PGPENV_CIPHER, kPGPCipherAlgorithm_IDEA,
			      PGPENV_PRI_PUBDEFAULT);
		pgpenvSetInt (env, PGPENV_HASH, kPGPHashAlgorithm_MD5,
						PGPENV_PRI_PUBDEFAULT);
		pgpenvSetInt (env, PGPENV_COMPRESS, 1, PGPENV_PRI_PUBDEFAULT);
		pgpenvSetInt (env, PGPENV_COMPRESSALG, PGP_COMPRESSALG_ZIP,
			      PGPENV_PRI_PUBDEFAULT);
		pgpenvSetInt (env, PGPENV_COMPRESSQUAL, 5, PGPENV_PRI_PUBDEFAULT);
		pgpenvSetInt (env, PGPENV_ARMORLINES, 0, PGPENV_PRI_PUBDEFAULT);
		pgpenvSetInt (env, PGPENV_CERTDEPTH, 4, PGPENV_PRI_PUBDEFAULT);
		pgpenvSetInt (env, PGPENV_COMPLETES, 1, PGPENV_PRI_PUBDEFAULT);
		pgpenvSetInt (env, PGPENV_MARGINALS, 2, PGPENV_PRI_PUBDEFAULT);
		pgpenvSetInt (env, PGPENV_TRUSTED, 120, PGPENV_PRI_PUBDEFAULT);
		pgpenvSetInt (env, PGPENV_CLEARSIG, 1, PGPENV_PRI_PUBDEFAULT);
		pgpenvSetInt (env, PGPENV_VERSION, PGPVERSION_3,
			      PGPENV_PRI_PUBDEFAULT);

		pgpenvSetString (env, PGPENV_PGPPATH, "", PGPENV_PRI_PUBDEFAULT);
		pgpenvSetString (env, PGPENV_UPATH, 
				 "/home/ch/keyserver", PGPENV_PRI_PUBDEFAULT);

	#ifdef MSDOS
		pgpenvSetString (env, PGPENV_CHARSET, "cp850", PGPENV_PRI_PUBDEFAULT);
	#else
		pgpenvSetString (env, PGPENV_CHARSET, "noconv", PGPENV_PRI_PUBDEFAULT);
	#endif
	}
	else
	{
		err	= kPGPError_OutOfMemory;
	}
	
	*outEnv	= env;
	return err;
}

	PGPContextRef
pgpenvGetContext( PGPEnv const *env )
{
	return( env->context );
}



/*
 * Copy an environment.  WARNING: Although the strings will be copied,
 * the "pointers" will not.  Take care when destroying an environment;
 * you can only destroy the pointers from the first one.  Since the
 * environment doesn't do this on its own, this is not a problem for
 * this code.
 */
	PGPError 
pgpenvCopy (
	PGPEnv const *	env,
	PGPEnv **		outEnv )
{
	PGPEnv *			tmp	= NULL;
	PGPContextRef		context;
	char const *		s;
	char *				s2;
	int			 		i;
	PGPError			err	= kPGPError_NoErr;
	PGPBoolean			allocationFailure	= FALSE;

	pgpAssertAddrValid( env, PGPEnv );

	pgpAssertAddrValid( env, PGPEnv );
	pgpAssertAddrValid( outEnv, PGPEnv * );
	
	*outEnv	= NULL;
	context	= env->context;

	/* Create the new env */
	tmp = (PGPEnv *)pgpContextMemAlloc ( context,
			sizeof (*tmp), 0 );
	if ( IsntNull( tmp ) )
	{
		/* Clone the old env */
		pgpCopyMemory( env, tmp, sizeof(*env) );

		/* Alloc and copy the strings */
		for (i=0; i < PGPENV_MAX_STRING-PGPENV_BASE_STRING; i++)
		{
			s = tmp->confString[i].val.string;
			if ( IsntNull( s ) )
			{
				s2 = (char *)pgpContextMemAlloc( context, strlen (s) + 1,
												 0);
				if ( IsntNull( s2 ) )
				{
					pgpCopyMemory( s, s2, strlen (s) + 1);
				}
				else
				{
					/*
					 * if we fail, continue loop to fully clear or form
					 * duplicate so we can dispose of the whole thing later
					*/
					allocationFailure	= TRUE;
					err	= kPGPError_OutOfMemory;
				}

				tmp->confString[i].val.string = s2;
			}
		}
	}
	else
	{
		err	= kPGPError_OutOfMemory;
	}
	
	/* return an error if we couldn't fully form the environment */
	if ( allocationFailure )
	{
		pgpenvDestroy( tmp );
		tmp	= NULL;
		err	= kPGPError_OutOfMemory;
	}
	
	*outEnv	= tmp;
	return err;
}


	void
pgpenvDestroy (PGPEnv *env)
{
	int		i;
	PGPContextRef		context;

	pgpAssertAddrValid( env, PGPEnv );
	context	= env->context;
	
	pgpAssertAddrValid( env, PGPEnv );

	/* De-allocate all the string variables */
	for (i=0; i < PGPENV_MAX_STRING-PGPENV_BASE_STRING; i++)
	{
		char *	s;
		s = env->confString[i].val.string;
		if (s)
		{
			pgpClearMemory( s,  strlen (s));
			pgpContextMemFree( context, s);
		}
	}
	

	pgpClearMemory( env,  sizeof (*env));
	pgpContextMemFree ( context, env);
}

/*
 * Set an Integer Value for the environment variable var in the
 * environment env to num with priority pri.
 */
	PGPError
pgpenvSetInt (
	PGPEnv *	env,
	PgpEnvInts	var,
	PGPInt32	num,
	PGPInt32	pri)
{
	int idx;
	
	pgpAssertAddrValid( env, PGPEnv );

	if (var < PGPENV_BASE_INT || var >= PGPENV_MAX_INT)
		return kPGPError_BadParams;

	idx = (int)var - PGPENV_BASE_INT;

	if (pri < env->confInt[idx].pri)
		return kPGPError_EnvPriorityTooLow;

	env->confInt[idx].val.num = num;
	env->confInt[idx].pri = pri;
	return kPGPError_NoErr;
}

	static PGPError
DoSetString (
	PGPEnv *		env,
	PgpEnvStrings	var,
	char const *	string,
	PGPInt32 		pri)
{
	PGPError	err	= kPGPError_NoErr;

	pgpAssertAddrValid( env, PGPEnv );
	
	var = (PgpEnvStrings) ( (PGPUInt32)var - (PGPUInt32)PGPENV_BASE_STRING );
	if (string)
	{
		if( IsNull( env->confString[var].val.string ) ) {
			env->confString[var].val.string = (char *)
			  pgpContextMemAlloc( env->context, strlen(string) + 1,
								  0);
			if( IsNull( env->confString[var].val.string ) )
				err = kPGPError_OutOfMemory;
		} else {
			err = pgpContextMemRealloc( env->context,
						 (void **)&env->confString[var].val.string,
						 strlen(string) + 1,
						 0);
			
		}
		if ( IsPGPError( err ) )
			return err;

		strcpy (env->confString[var].val.string, string);
	}
	else
	{
		if (env->confString[var].val.string)
			pgpContextMemFree( env->context, env->confString[var].val.string);
		env->confString[var].val.string = NULL;
	}

	env->confString[var].pri = pri;
	return kPGPError_NoErr;
}

	static PGPError
SetPointer (PGPEnv *env, unsigned var, void *ptr, size_t size)
{
	pgpAssertAddrValid( env, PGPEnv );
	
	var -= PGPENV_BASE_POINTER;
	env->pointers[var] = ptr;
	env->pointerSize[var] = size;
	return kPGPError_NoErr;
}

/*
 * Set a String Value for the environment variable var in the
 * environment env to string with priority pri.  Space is allocated
 * for the string.
 *
 * If the string is null, free the old string (if one existed) and
 * then set the placeholder to NULL.
 */
	PGPError
pgpenvSetString (PGPEnv *env, PgpEnvStrings var,
		 char const *string, int pri)
{
	PGPError err = kPGPError_NoErr;

	pgpAssertAddrValid( env, PGPEnv );
	if ( env == NULL )
		return kPGPError_BadParams;

	if (var < PGPENV_BASE_STRING || var >= PGPENV_MAX_STRING)
		return kPGPError_BadParams;

	if (pri < env->confString[var-PGPENV_BASE_STRING].pri)
		return kPGPError_EnvPriorityTooLow;	/* not assigned */

	switch (var) {
	case PGPENV_CHARSET:
		{
			PGPByte const *toLocal, *toLatin1;

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
	PGPError
pgpenvSetPointer (
	PGPEnv *		env,
	PgpEnvPointers	var,
	void *			ptr,
	size_t			size)
{
	pgpAssertAddrValid( env, PGPEnv );

	if (var < PGPENV_BASE_POINTER || var >= PGPENV_MAX_POINTER)
		return kPGPError_BadParams;

	if (var == PGPENV_CHARMAPTOLOCAL || var == PGPENV_CHARMAPTOLATIN1)
		return kPGPError_BadParams;

	return SetPointer (env, var, ptr, size);
}

	PGPInt32
pgpenvGetInt (PGPEnv const *env, PgpEnvInts var, PGPInt32 *pri,
	      PGPError *error)
{
	PGPError err = kPGPError_NoErr;
	int val = 0;
	int idx;

	if (!env)
	{
		err = kPGPError_BadParams;
		goto getint_end;
	}

	if (var < PGPENV_BASE_INT || var >= PGPENV_MAX_INT)
	{
		err = kPGPError_BadParams;
		goto getint_end;
	}

	idx = (int)var - PGPENV_BASE_INT;

	if (pri)
		*pri = env->confInt[idx].pri;

	val =  env->confInt[idx].val.num;

 getint_end:
	if ( error != NULL )
		*error = err;

	return val;
}

	const char  *
pgpenvGetCString(
	PGPEnv const *	env,
	PgpEnvStrings	var,
	int *			pri)
{
	char const *	s 	= NULL;
	int				idx;

	pgpAssertAddrValid( env, PGPEnv );
	pgpAssert( var >= PGPENV_BASE_STRING || var < PGPENV_MAX_STRING );
	
	idx = (int)var - PGPENV_BASE_STRING;

	if (pri)
		*pri = env->confString[idx].pri;

	s	= env->confString[idx].val.string;

	return s;
}



	void  *
pgpenvGetPointer (
	PGPEnv const *	env,
	PgpEnvPointers	var,
	size_t *		size)
{
	void *ptr = NULL;
	int		idx;

	pgpAssertAddrValid( env, PGPEnv );
	pgpAssert( var >= PGPENV_BASE_POINTER && var < PGPENV_MAX_POINTER );
	
	idx = (int)var - PGPENV_BASE_POINTER;
	
	if (size)
		*size = env->pointerSize[idx];

	ptr = env->pointers[idx];

	return ptr;
}




