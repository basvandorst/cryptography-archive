/*
 * pgpkKeyGen.h
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: pgpkKeyGen.h,v 1.1.2.2 1997/06/07 09:49:14 mhw Exp $
 */

/* Optional Arguments: name type keybits validity*/
int
doKeyGenerate (PGPKeySet *defaultset, int argc, char *argv[], void *ui_arg);
