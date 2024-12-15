/*____________________________________________________________________________
    usuals.h

    Copyright(C) 1998,1999 Network Associates, Inc.
    All rights reserved.

	PGP 6.5 Command Line 

    $Id: usuals.h,v 1.9 1999/05/12 21:01:05 sluu Exp $
____________________________________________________________________________*/


#ifndef USUALS_H /* Assures no redefinitions of usual types...*/
#define USUALS_H

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL (void*)0
#endif
#endif

#ifndef TRUE
#define FALSE 0
#define TRUE (!FALSE)
#endif /* if TRUE not already defined */

/* void for use in pointers */
#ifndef NO_VOID_STAR
#define VOID void
#else
#define VOID char
#endif

 /* Zero-fill the byte buffer. */
#define fill0(buffer,count) memset( buffer, 0, count )

 /* This macro is for burning sensitive data.  Many of the
    file I/O routines use it for zapping buffers */
#define burn(x) fill0((VOID *)&(x),sizeof(x))

#endif /* USUALS_H */

