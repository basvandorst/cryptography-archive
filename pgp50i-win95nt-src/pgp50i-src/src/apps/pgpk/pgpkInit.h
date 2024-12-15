/*
 * pgpkInit.h -- Initalize the PGPK Application
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: pgpkInit.h,v 1.1.2.1 1997/06/07 09:49:13 mhw Exp $
 */

#ifndef PGPINITAPP_H
#define PGPINITAPP_H

#ifdef __cplusplus
extern "C" {
#endif

int pgpInitApp (void *stacktop, int exit_prog);

/*
 * This routine must be defined for each application.  It works just like
 * main.  pgpInitApp.c contains the real main(), which calls appMain().
 * This is because on the Macintosh, main gets called without parameters,
 * so we need to read the arguments from the command line from within main().
 */
int appMain (int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif /* PGPINITAPP_H */
