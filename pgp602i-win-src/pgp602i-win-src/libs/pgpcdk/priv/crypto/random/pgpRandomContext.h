/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	

	$Id: pgpRandomContext.h,v 1.5.10.1 1998/11/12 03:21:30 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpRandomContext_h	/* [ */
#define Included_pgpRandomContext_h


#include "pgpOpaqueStructs.h"
#include "pgpPubTypes.h"

PGP_BEGIN_C_DECLARATIONS

struct PGPRandomVTBL
{
	char const *	name;
	void	(*addBytes) (void *priv, PGPByte const *buf, PGPUInt32 len);
	void	(*getBytesEntropy) (void *priv, PGPByte *buf, PGPUInt32 len, 
	                         PGPUInt32 bits);
	void	(*stir) (void *priv);
} ;


struct PGPRandomContext
{
	PGPContextRef			context;
	PGPRandomVTBL const *	vtbl;
	void *					priv;
	void (*destroy)			(PGPRandomContext *rc);
};


#define pgpRandomAddBytes(rc,b,l) (rc)->vtbl->addBytes((rc)->priv, b, l)
#define pgpRandomGetBytesEntropy(rc,b,l,t) \
	(rc)->vtbl->getBytesEntropy((rc)->priv, b, l, t)
#define pgpRandomStir(rc) (rc)->vtbl->stir((rc)->priv)
#define pgpRandomDestroy(rc)	\
	if ( IsntNull( (rc)->destroy ) ) \
		(rc)->destroy(rc);\
	else\
		pgpContextMemFree( rc->context, rc )



	
PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpRandomContext_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
