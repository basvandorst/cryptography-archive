/*

 * pgpKB.h -- interface for keyboard I/O

 *            The implementation is in pgpKBUnix.c, pgpKBMSDOS.c,

 *            kbvms.c, etc.

 *

 * Copyright (C) 1993-1997 Network Associates Inc. and affiliated companies.  All rights reserved.

 *

 * $Id: pgpKB.h,v 1.2 1999/03/10 02:51:57 heller Exp $

 */



#ifdef __cplusplus

extern "C" {

#endif



int kbOpenKbd(int flags, int InBatchmode);

int kbCloseKbd(int fd);

void kbCbreak(int InBatchmode), kbNorm(void);

int kbGet(void);

void kbFlush(int thorough);



#ifdef __cplusplus

}

#endif

