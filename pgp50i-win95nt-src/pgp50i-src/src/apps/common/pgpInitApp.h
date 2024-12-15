/*
 * pgpInitApp.h -- Initalize a PGP Application from config files, etc.
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * Written By:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpInitApp.h,v 1.3.2.1 1997/06/07 09:49:05 mhw Exp $
 */

#ifndef PGPINITAPP_H
#define PGPINITAPP_H

#ifdef __cplusplus
extern "C" {
#endif

struct PgpEnv;
struct PgpUICb;

/*
 * envp must be a valid pointer,
 * stacktop should be the top of the stack (&argc)
 * ui and ui_arg should be filled in.
 *
 * This function must be called before any allocations take place.
 * so, it should probably be the first (or close to the first) thing
 * that takes place.
 */
int pgpInitApp (struct PgpEnv **envp, void *stacktop,
		struct PgpUICb const *ui, void *ui_arg, int exit_prog,
		int newver);

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
