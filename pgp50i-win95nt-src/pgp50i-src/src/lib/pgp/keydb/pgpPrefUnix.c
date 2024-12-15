/*
 * pgpPrefUnix.c -- Access preference database for Unix systems
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpPrefUnix.c,v 1.6.2.8.2.1 1997/07/01 18:14:10 quark Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h> /*for stat() to check for directory*/
#endif

#include "pgpKDBint.h"
#include "pgpDebug.h"
#include "pgpEnv.h"

#include "pgpConf.h"
#include "pgpFileNames.h"
#include "pgpUI.h"
#include "pgpErr.h"
#include "pgpPrefUnix.h"

/* Callback function for problems reading config file from pgpLoadPrefs */
static int
plpInitMessage (void *arg, int type, int msg, unsigned numargs, ...)
{
		int			  *ui_arg;				/*	Parameters for callbacks */

		ui_arg = (int *) arg;

		(void)type;
		(void)msg;
		(void)numargs;

		++(*ui_arg);								/*	Just count messages for now */

		return 0;
}

PGPError
pgpSetDefaultPrefsInternal (void)
{
		return PGPERR_OK;
}

PGPError
pgpSetDefaultKeyPathInternal (void)
{
		return PGPERR_OK;
}

/*	Read preferences and put them into the environment */
PGPError
pgpLoadPrefsInternal (void)
{
		char						*fn, *confdir;
		char const				*pc, *pgppath;
		PgpUICb ui;
		int ui_arg;

		/*	Try to find the PGPPATH */
		if ((pc = getenv ("PGPPATH")) != NULL) {
				pgpenvSetString (pgpEnv, PGPENV_PGPPATH, pc,
								PGPENV_PRI_PUBDEFAULT);
		} else if ((pc = getenv ("HOME")) != NULL) {
				fn = pgpFileNameBuild (pc, ".pgp", NULL);
				CheckAndCreatePGPDir(fn);
				pgpenvSetString (pgpEnv, PGPENV_PGPPATH, fn,
								PGPENV_PRI_PUBDEFAULT);
				pgpMemFree (fn);
		}
		pgppath = pgpenvGetString (pgpEnv, PGPENV_PGPPATH, NULL, NULL);

		/*	Ok, PGPPATH is a PATH of directories. Find the first one.. */
		confdir = pgpFileNameNextDirectory (pgppath, NULL);
		pgpAssert (confdir);

		fn = pgpFileNameBuild (confdir, "pubring.pkr", NULL);
		pgpenvSetString (pgpEnv, PGPENV_PUBRING, fn, PGPENV_PRI_PRIVDEFAULT);
		pgpMemFree (fn);

		fn = pgpFileNameBuild (confdir, "secring.skr", NULL);
		pgpenvSetString (pgpEnv, PGPENV_SECRING, fn, PGPENV_PRI_PRIVDEFAULT);
		pgpMemFree (fn);

		fn = pgpFileNameBuild (confdir, "randseed.bin", NULL);
		pgpenvSetString (pgpEnv, PGPENV_RANDSEED, fn, PGPENV_PRI_PRIVDEFAULT);
		pgpMemFree (fn);

	fn = pgpFileNameBuild (confdir, "language50.txt", NULL);
	pgpenvSetString (pgpEnv, PGPENV_LANGUAGEFILE, fn, PGPENV_PRI_PRIVDEFAULT);
	pgpMemFree (fn);

		/*	Prepare to read config file(s) */
		memset (&ui, 0, sizeof(ui));
		ui.message = plpInitMessage;
		ui_arg = 0;

		/*	Read the user's config file */
	fn = pgpFileNameBuild (confdir, "pgp.cfg", NULL);
		pgpConfigFileProcess (&ui, &ui_arg, pgpEnv, fn, PGPENV_PRI_CONFIG);
		pgpMemFree (fn);

		pgpMemFree (confdir);

		return PGPERR_OK;
}

/*	Save preferences to disk */
PGPError
pgpSavePrefsInternal (void)
{
		/*	XXX Not implemented */
	
		return PGPERR_OK;
}

/*Checks for the existance of the requested directory. If it exists and is a
 *directory, we don't do anything. If it doesn't exist at all, we create it.
 *If it does exist and isn't a directory, we advise the user to eliminate her
 *file ("Hey, only an IDIOT would have a file called ~/.pgp..." ;) or to
 *user PGPPATH to set it to something else.
 *
 *This is ifdefed UNIX so that it doesn't break anything else. There's no
 *theoretical reason this couldn't work under DOS, if mkdir() has a dummy
 *mode parameter; otherwise, that little bit will need tweaking.
 *
 *BAT 970511
 */

void CheckAndCreatePGPDir(const char *fn)
{
	Boolean dir_exists = FALSE, name_exists = FALSE;
#ifdef HAVE_SYS_STAT_H
	/*This is the preferred method, but it may not be universally portable,
	*so I've implemented a less-than-desirable check below using definitely
	*available functions.
	*/
	struct stat stat_buf;

	if(stat(fn, &stat_buf) == 0) {
		name_exists = TRUE;
		/*OK, the file exists. Is it a directory?*/
		if(S_ISDIR(stat_buf.st_mode))
			dir_exists = TRUE;
	}
#else
	/*This is suboptimal. If there's a better, definitely portable way to
	*do this, I'd like to know about it.
     *Basically, we try to read the pgp.cfg file; if we can't, we try to
     *create an empty pgp.cfg file; if we can't, we try to read fn (in case
	*it exists as a file). If that doesn't do it, we give up and assume
	*mkdir() can create it, later.
	*/
	FILE *inf, *ouf;
	char *config_txt = NULL;

    if((config_txt = pgpFileNameBuild(fn, "pgp.cfg"))) {
		if((inf = fopen(config_txt, "r"))) {
			/*Can we just read it?*/
			name_exists = dir_exists = TRUE;
			fclose(inf);
		}
		else {
			/*Ok, let's try to write there. Maybe the dir exists, but the
			*file doesn't. Another, less likely possibility is that we have
			*write access, but not read access. While unlikely, we don't
	     *want to overwrite an existing pgp.cfg, in this case, so we
			*open append; if the file just doesn't exist, this should create
			*it.
			*/
		if((ouf = fopen(config_txt, "a"))) {
				name_exists = dir_exists = TRUE;
				fclose(ouf);
			}
			else {
				/*Last chance - perhaps the "directory" already exists as
				*a file.
				*/
				if((inf = fopen(fn, "r"))) {
					name_exists = TRUE;
					fclose(inf):
				}
				/*If we haven't hit anything, either this thing doesn't
				 *exist, or it exists and isn't a directory and we don't
				 *have read access, or it exists and is IS a directory AND
		 *pgp.cfg doesn't exist in it AND we don't have write
				 *access. Whew!
				 */
			}
		}
		pgpFree(config_txt);
	}
#endif

	if(!dir_exists) {
		if(!name_exists) {
			fprintf(stderr, "Creating %s...", fn);
			/*Note this is not neccessarily 0700; this modifies the umask,
			*which is currently set to 077 in pgp.c and pgpk.c.*/
			/*XXX If DOS mkdir is like mkdir(char *), some finessing of this
			*XXX will be needed to make this compile.
			*/
			if(mkdir(fn, 0700) == 0) {
				fprintf(stderr, "complete.\n");
			}
			else {
				fprintf(stderr, "error!\n");
				perror("Unable to create directory");
			}
		}
		else {
			fprintf(stderr,
					"%s exists, but it is not a directory.\n",
					fn);
			fprintf(stderr,
					"Either remove this file, or use the PGPPATH ");
			fprintf(stderr,
					"environmental variable to\nset your PGP home ");
			fprintf(stderr,
					"directory.\n");
			}
		}
}


/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
*/
