/*____________________________________________________________________________	Copyright (C) 1994-1998 Network Associates Inc. and affiliated companies.	All rights reserved.		$Id: PGPDiskCastCFB.h,v 1.3 1999/03/10 02:49:46 heller Exp $____________________________________________________________________________*/#pragma once#include "cast5.h"typedef struct ExpandedCASTKey	{	word32	keyWords[32];	} ExpandedCASTKey;		typedef struct CASTCFBContext{	ExpandedCASTKey		expandedKey;} CASTCFBContext;void CASTCFB_InitIV(const UInt32 *iv, UInt32 *initMe);void CASTCFB_Init(CASTCFBContext *context, const byte *keyToExpand);void CASTCFB_Destroy(CASTCFBContext *context);void CASTCFB_Encrypt( const CASTCFBContext *context, const UInt32 *iv,			const UInt32 *src, UInt32 *dest, UInt32 byteCount);void CASTCFB_Decrypt( const CASTCFBContext *context, const UInt32 *iv,			const UInt32 *src, UInt32 *dest, UInt32 byteCount);