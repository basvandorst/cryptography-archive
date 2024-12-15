/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpHashPriv.h,v 1.8 1997/10/14 01:48:21 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpHashingPriv_h	/* [ */
#define Included_pgpHashingPriv_h

#include "pgpBase.h"
#include "pgpOpaqueStructs.h"
#include "pgpHash.h"


PGP_BEGIN_C_DECLARATIONS


/*____________________________________________________________________________
	A static description of a hash; analagous to a C++ virtual function tbl.
____________________________________________________________________________*/
struct PGPHashVTBL
{
	char const *		name;
	PGPHashAlgorithm	algorithm;
	void const *		DERprefix;		/* DER-encoded prefix */
	PGPSize				DERprefixsize;	/* DER-encoded prefix length */
	PGPSize				hashsize;		/* Bytes of output */
	PGPSize				context_size;
	PGPSize				context_align;
	
	void			(*init)( void *priv );
	void			(*update)( void *priv, void const *buf, PGPSize len );
	void const *	(*final)( void *priv );
};



PGPHashVTBL const  *	pgpHashGetVTBL( const PGPHashContext * );


/* Create and Destroy a big bunch of hashes at once. */
PGPError 			pgpHashListCreate( PGPContextRef context,
						void const *buf,
						PGPHashListRef *outList, PGPUInt32 numHashes );
PGPHashContextRef	pgpHashListGetIndHash( PGPHashListRef list,
						PGPUInt32 algIndex );
void 				pgpHashListDestroy( PGPHashListRef hashes );
PGPUInt32 			pgpHashListGetSize( PGPHashListRef hashes );

/* Find the context of a given type in the list */
PGPHashContextRef	pgpHashListFind(  PGPHashListRef hashList,
							PGPHashVTBL const *vtbl);


/*____________________________________________________________________________
	The following are glue routines for compatibility with existing source.
	Use is discouraged in favor of official routines.
____________________________________________________________________________*/

void const *		pgpHashFinal( PGPHashContextRef ref );

/* Allocate, deallocate, and copy a hash context */
PGPHashContextRef	pgpHashCreate( PGPContextRef context,
						PGPHashVTBL const *vtbl);
PGPHashContextRef	pgpHashCopy( PGPConstHashContextRef	hc);
void 				pgpHashCopyData( PGPConstHashContextRef src,
						PGPHashContext *dest );

/* Return a hash struction of the appropriate number */
PGPHashVTBL const  *	pgpHashByNumber( PGPHashAlgorithm type);
PGPHashVTBL const  *	pgpHashByName( char const *name, PGPSize namelen);



PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpHashingPriv_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
