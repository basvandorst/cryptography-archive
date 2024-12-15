/*
* bnlegal.h
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* ANSI C stadard, section 3.5.3: "An object that has volatile-qualified
* type may be modified in ways unknown to the implementation or have
* other unknown side effects." Yes, we can't expect a compiler to
* understand law...
*
* $Id: bnlegal.h,v 1.2.2.1 1997/06/07 09:49:39 mhw Exp $
*/

#ifdef __cplusplus
extern "C" {
#endif

extern volatile const char bnCopyright[];

#ifdef __cplusplus
}
#endif
