/*
* pgpRot13Mod.h -- definitions for a stupid encryption pipeline: rot13
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpRot13Mod.h,v 1.2.2.1 1997/06/07 09:51:18 mhw Exp $
*/

#ifdef __cplusplus
extern "C" {
#endif

struct PgpPipeline;

struct PgpPipeline **rot13Create (struct PgpPipeline **head);

#ifdef __cplusplus
}
#endif
