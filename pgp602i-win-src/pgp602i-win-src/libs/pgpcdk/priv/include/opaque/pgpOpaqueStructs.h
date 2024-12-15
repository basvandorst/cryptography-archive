/*____________________________________________________________________________
	pgpOpaqueStructs.h
	
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: pgpOpaqueStructs.h,v 1.18.22.1 1998/11/12 03:21:47 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pgpOpaqueStructs_h	/* [ */
#define Included_pgpOpaqueStructs_h


#include "pgpRingOpaqueStructs.h"
#include "pgpBigNumOpaqueStructs.h"
#include "pgpKeyDBOpaqueStructs.h"
#include "pgpPipelineOpaqueStructs.h"


typedef struct PGPPubKey		PGPPubKey;
typedef struct PGPSecKey		PGPSecKey;
typedef struct PGPKeySpec		PGPKeySpec;
typedef struct PGPSigSpec		PGPSigSpec;
typedef struct PGPRandomContext	PGPRandomContext;
typedef struct PGPEnv			PGPEnv;
typedef struct PGPUICb			PGPUICb;
typedef struct PGPESK			PGPESK;
typedef struct PGPConvKey		PGPConvKey;
typedef struct PGPFile			PGPFile;
typedef struct PGPFileError		PGPFileError;
typedef struct PGPFileRead		PGPFileRead;
typedef struct PGPStringToKey	PGPStringToKey;
typedef struct PGPPkAlg			PGPPkAlg;
typedef struct PGPFifoContext	PGPFifoContext;
typedef struct RingError		RingError;
typedef struct PGPPassCache		PGPPassCache;
typedef struct PGPFileSpec		PGPFileSpec;
typedef struct MemPool			MemPool;
typedef struct PGPKeyIDPriv		PGPKeyIDPriv;

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
