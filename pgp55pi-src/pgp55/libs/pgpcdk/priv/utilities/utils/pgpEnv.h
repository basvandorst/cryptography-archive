/*
 * pgpEnv.h -- The PGPlib Environment.
 *
 * Keep track of what used to be "global" state; these values are valid
 * only within a working environment.  Multiple environments can co-exist
 * within a single application, however each one would have its own unique
 * environment structure and should pass it on to its children.
 *
 * The main purpose of this structure is to gather together a lot of
 * little fiddly things that a user might want to adjust into a single
 * variable so code that doesn't want to fiddle with them doesn't have to
 * think about the issue.  It makes passing a different value of an
 * environment variable to a function more awkward (you have to
 * copy a parent environment and set the value(s) of interest), but
 * it's quite possible if you want more detailed control.
 *
 * The big win is that to add an extra control parameter to a function
 * that already gets some parameters from an environment, you can just
 * add it to the environment and change the function to get its parmaeter
 * from the environment and all of the code in between can just be
 * relinked.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * This is a Public API Function Header.
 *
 * $Id: pgpEnv.h,v 1.25 1997/08/25 20:24:23 lloyd Exp $
 */

#ifndef Included_pgpEnv_h
#define Included_pgpEnv_h

#include "pgpBase.h"
#include "pgpPubTypes.h"

PGP_BEGIN_C_DECLARATIONS

#include "pgpOpaqueStructs.h"

#define PGPENV_BASE_INT	1000
enum PgpEnvInts_
{
	PGPENV_ARMOR	= PGPENV_BASE_INT,
	PGPENV_ARMORLINES,
	PGPENV_CERTDEPTH,
	PGPENV_CIPHER,
	PGPENV_CLEARSIG,
	PGPENV_COMPLETES,
	PGPENV_COMPRESS,
	PGPENV_COMPRESSALG,
	PGPENV_COMPRESSQUAL,
	PGPENV_ENCRYPTTOSELF,
	PGPENV_HASH,
	PGPENV_MARGINALS,
	PGPENV_SHOWPASS,
	PGPENV_TEXTMODE,
	PGPENV_TRUSTED,
	PGPENV_TZFIX,
	PGPENV_VERBOSE,
	PGPENV_VERSION,
	PGPENV_BATCHMODE,
	PGPENV_FORCE,
	PGPENV_MAGIC,
	PGPENV_NOOUT,
	PGPENV_PGPMIME,
	PGPENV_MAILWORDWRAPENABLE,
	PGPENV_MAILWORDWRAPWIDTH,
	PGPENV_MAILPASSCACHEENABLE,
	PGPENV_MAILPASSCACHEDURATION,
	PGPENV_MAILENCRYPTPGPMIME,
	PGPENV_MAILSIGNPGPMIME,
	PGPENV_FASTKEYGEN,
	PGPENV_SIGNCACHEENABLE,
	PGPENV_SIGNCACHEDURATION,
	PGPENV_WARNONMIXRSAELGAMAL,
	PGPENV_MAILENCRYPTDEFAULT,
	PGPENV_MAILSIGNDEFAULT,
	PGPENV_DATEOFLASTSPLASHSCREEN,
	PGPENV_MARGINALLYVALIDWARNING,
	PGPENV_PGPMIMEPARSEBODY,
	PGPENV_FIRSTKEYGENERATED,
	PGPENV_HTTPKEYSERVERPORT,
	PGPENV_WARNONRSARECIPANDNONRSASIGNER,
	PGPENV_PGPMIMEVERSIONLINE,
	PGPENV_MAX_INT,
	
	PGP_ENUM_FORCE( PgpEnvInts_ )
};
PGPENUM_TYPEDEF( PgpEnvInts_, PgpEnvInts );


#define PGPENV_BASE_STRING	2000
enum PgpEnvStrings_
{
	PGPENV_BAKRING	= PGPENV_BASE_STRING,
	PGPENV_CHARSET,
	PGPENV_COMMENT,
	PGPENV_COMPANYKEY,
	PGPENV_RANDOMDEVICE,
	PGPENV_LANGUAGE,
	PGPENV_MYNAME,
	PGPENV_PGPPATH,
	PGPENV_PUBRINGnolongerused,
	PGPENV_RANDSEEDnolongerused,
	PGPENV_SECRINGnolongerused,
	PGPENV_SYSDIR,
	PGPENV_TMP,
	PGPENV_UPATH,
	PGPENV_OWNERNAME,
	PGPENV_COMPANYNAME,
	PGPENV_LICENSENUMBER,
	PGPENV_HTTPKEYSERVERHOST,
	PGPENV_VERSION_STRING,
	PGPENV_MAX_STRING,
	
	PGP_ENUM_FORCE( PgpEnvStrings_ )
};
PGPENUM_TYPEDEF( PgpEnvStrings_, PgpEnvStrings );


#define PGPENV_BASE_POINTER	3000
enum PgpEnvPointers_
{
	PGPENV_CHARMAPTOLATIN1	= PGPENV_BASE_POINTER,
	PGPENV_CHARMAPTOLOCAL,
	PGPENV_PGPKEYSMACMAINWINPOS,
	PGPENV_PGPAPPMACPRIVATEDATA,
	PGPENV_ENCRYPTIONCALLBACK,
	PGPENV_PGPMENUMACAPPSIGNATURES,
	PGPENV_RANDSEEDREF,
	PGPENV_MAX_POINTER,
	
	PGP_ENUM_FORCE( PgpEnvPointers_ )
};
PGPENUM_TYPEDEF( PgpEnvPointers_, PgpEnvPointers );

/* Configuration priorities */
#define PGPENV_PRI_FORCE	25
#define PGPENV_PRI_CMDLINE	20
#define PGPENV_PRI_CONFIG	15
#define PGPENV_PRI_PRIVDEFAULT	10
#define PGPENV_PRI_SYSCONF	5
#define PGPENV_PRI_PUBDEFAULT	0

/* Exported Functions */

/* create and initialize, copy, and destroy an environment */
PGPError  	pgpenvCreate ( PGPContextRef, PGPEnv **outEnv);
PGPError  	pgpenvCopy ( PGPEnv const *env, PGPEnv **outEnv);
void 		pgpenvDestroy (PGPEnv *env);

/*
 * Set Integer and String configuration parameters.  Strings are
 * copied by the environment; the caller need not preserve them.
 * (They must, of course, be null-terminated.)
 */
PGPError 	pgpenvSetInt(PGPEnv *env,
					PgpEnvInts var, PGPInt32 num, PGPInt32 pri);
PGPError 	pgpenvSetString (PGPEnv *env, PgpEnvStrings var,
		    		 char const *string, PGPInt32 pri);

/*
 * Retrieve Integer and String configuration parameters.
 */
PGPInt32 		pgpenvGetInt (PGPEnv const *env, PgpEnvInts var,
						PGPInt32 *pri, PGPError *error);
const char   *	pgpenvGetCString(PGPEnv const *env,
						PgpEnvStrings var, PGPInt32 *pri);


PGPContextRef	pgpenvGetContext( PGPEnv const *env );

/*
 * Set and get Pointer parameters -- these are not configuration values,
 * in general, so they are treated somewhat differently.  Note:
 * the environment code does *not* copy the objects associated with the
 * pointers, since it doesn't know how large they are or what other
 * dependent objects they may contain.  It is up to the caller to
 * ensure that the pointers remain valid as long as a copy of them
 * might exist, and to free them when a pointer no longer exists.
 */
PGPError 	pgpenvSetPointer (PGPEnv *env, PgpEnvPointers var,
					void *ptr, size_t size);
void   *	pgpenvGetPointer(PGPEnv const *env,
					PgpEnvPointers var, size_t *optionalSize );

PGP_END_C_DECLARATIONS

#endif /* Included_pgpEnv_h */







