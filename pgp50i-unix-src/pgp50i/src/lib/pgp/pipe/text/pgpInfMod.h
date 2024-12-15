/*
* pgpInfMod.h -- inflation module
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by: Derek Atkins <warlord@MIT.EDU>
*
* This is a PRIVATE header file, for use only within the PGP Library.
* You should not be using these functions in an application.
*
* $Id: pgpInfMod.h,v 1.3.2.1 1997/06/07 09:51:08 mhw Exp $
*/

#ifdef __cplusplus
extern "C" {
#endif

struct PgpPipeline;
struct PgpPipeline **infModCreate (struct PgpPipeline **head);

#ifdef __cplusplus
}
#endif
