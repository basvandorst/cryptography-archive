/*
* pgpDefMod.h -- Header for ZIP Compression (deflation)
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by: Derek Atkins <warlord@MIT.EDU>
*
* This is a PRIVATE header file, for use only within the PGP Library.
* You should not be using these functions in an application.
*
* $Id: pgpDefMod.h,v 1.3.2.1 1997/06/07 09:51:07 mhw Exp $
*/

#ifdef __cplusplus
extern "C" {
#endif

struct PgpPipeline;
struct PgpPipeline **defModCreate (struct PgpPipeline **head, int quality);

#ifdef __cplusplus
}
#endif
