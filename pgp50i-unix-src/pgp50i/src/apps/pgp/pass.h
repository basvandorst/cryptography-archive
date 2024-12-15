/*
 * pass.h
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: pass.h,v 1.1.2.2 1997/06/07 09:49:08 mhw Exp $
 */

#ifdef PGP_MAIN
#define EXTERN
#define GLOBAL_ASSIGN(x) = x;
#else
#define EXTERN extern
#define GLOBAL_ASSIGN(x)
#endif
#define PASSLEN 256

EXTERN char phrase[PASSLEN] GLOBAL_ASSIGN("\0");
EXTERN struct PgpPassCache *passcache GLOBAL_ASSIGN(NULL);

#undef EXTERN
#undef GLOBAL_ASSIGN
