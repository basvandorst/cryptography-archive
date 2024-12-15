/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif


EXTERN_C BOOL KMGetPhrase (HINSTANCE hInst, HWND hparent, int istring,
							LPSTR buffer, int buflen);
