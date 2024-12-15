/*
 * pgpScreen.h
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpScreen.h,v 1.2.2.1 1997/06/07 09:52:04 mhw Exp $
 */

#ifdef __cplusplus
extern "C" {
#endif

void screenSizeGet(unsigned *rowsp, unsigned *colsp);
void screenClear(FILE *f);

#ifdef __cplusplus
}
#endif
