/*
 * pgpKB.h -- interface for keyboard I/O
 * The implementation is in pgpKBUnix.c, pgpKBMSDOS.c,
 * kbvms.c, etc.
 *
 * Copyright (C) 1993-1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: pgpKB.h,v 1.2.2.2.2.1 1997/08/18 22:12:27 quark Exp $
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
