/*____________________________________________________________________________
	pgpOpaqueStructs.h
	
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpOpaqueStructs.h,v 1.2 1999/03/10 02:53:57 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpOpaqueStructs_h	/* [ */
#define Included_pgpOpaqueStructs_h

typedef struct PGPHashList					PGPHashList;
typedef struct PGPHashList *				PGPHashListRef;

typedef struct PGPHashVTBL					PGPHashVTBL;
typedef struct PGPPublicKeyContext			PGPPublicKeyContext;
typedef struct PGPPrivateKeyContext			PGPPrivateKeyContext;
typedef struct PGPCipherVTBL				PGPCipherVTBL;
typedef struct PGPHashContext				PGPHashContext;
typedef struct PGPCFBContext				PGPCFBContext;
typedef struct PGPCBCContext				PGPCBCContext;
typedef struct PGPSymmetricCipherContext	PGPSymmetricCipherContext;

typedef struct PGPRandomVTBL				PGPRandomVTBL;



#endif /* ] Included_pgpOpaqueStructs_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
