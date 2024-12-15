/*
 # $Id: fortify.c,v 1.16 1998/04/10 10:27:56 fbm Exp fbm $
 # Copyright (C) 1997-2000 Farrell McKay
 # All rights reserved.
 #
 # This file is part of the Fortify distribution, a toolkit for
 # upgrading the cryptographic strength of the Netscape web browsers.
 #
 # This toolkit is provided to the recipient under the
 # following terms and conditions:-
 #   1.  This copyright notice must not be removed or modified.
 #   2.  This toolkit may not be reproduced or included in any commercial
 #       media distribution, or commercial publication (for example CD-ROM,
 #       disk, book, magazine, journal) without first obtaining the author's
 #       express permission.
 #   3.  This toolkit, or any component of this toolkit, may not be
 #       used, resold, redeveloped, rewritten, enhanced or otherwise
 #       employed by a commercial organisation, or as part of a commercial
 #       venture, without first obtaining the author's express permission.
 #   4.  Subject to the above conditions being observed (1-3), this toolkit
 #       may be freely reproduced or redistributed.
 #   5.  To the extent permitted by applicable law, this software is
 #       provided "as-is", without warranty of any kind, including
 #       without limitation, the warrantees of merchantability,
 #       freedom from defect, non-infringement, and fitness for
 #       purpose.  In no event shall the author be liable for any
 #       direct, indirect or consequential damages however arising
 #       and however caused.
 #   6.  Subject to the above conditions being observed (1-5),
 #       this toolkit may be used at no cost to the recipient.
 #
 # Farrell McKay
 # Wayfarer Systems Pty Ltd		contact@fortify.net
 */

#include "includes.h"
#include <locale.h>

#include "misc.h"
#include "cp.h"
#include "index.h"
#include "morpher.h"
#include "os2lx.h"
#include "options.h"
#include "callback.h"
#include "msgs.h"
#include "log.h"

#ifdef macintosh
#include <console.h>
#include <unix.h>
#endif

#if defined(GNU_WIN32)
#define NS_PROG		"netscape.exe"
#define EXAMPLE_PATH	"C:\\Program Files\\Netscape\\Program\\netscape.exe"
#elif defined(OS2)
#define NS_PROG		"netscape.exe"
#define EXAMPLE_PATH	"C:\\Netscape\\netscape.exe"
#else
#define NS_PROG		"netscape"
#define EXAMPLE_PATH	"/usr/local/bin/netscape"
#endif

/* Unfortunately not all unistd.h headers contain this constant. */
#ifndef	W_OK
#define	W_OK		2
#endif

#ifndef	MAXPATHLEN
#define MAXPATHLEN	1024
#endif

static char		*ffy_vern  = "";
static char		*ffy_ident = "@(#) morpher 2.3";

typedef struct {
	int	n;
	char	**paths;
} dirlist_t;

static void
search_dir(char *dir, dirlist_t *dirlist)
{
	int		n = dirlist->n;
	char		**p = dirlist->paths;
        char    	path[MAXPATHLEN+1];
	struct stat	st;

        sprintf(path, "%s%c%s", dir, PATH_SEP, NS_PROG);
        if (*dir && stat(path, &st) == 0 && S_ISREG(st.st_mode)) {
		p = (char **) f_realloc((void *) p, sizeof(char **) * ++n);
		p[n - 2] = f_strdup(path);
		p[n - 1] = NULL;
		dirlist->n = n;
		dirlist->paths = p;
	}
}

static void
search_path(dirlist_t *dirlist)
{
	char	*dir, *p;

	dirlist->n = 1;
	dirlist->paths = (char **) f_calloc(1, sizeof(char **));

	if ((dir = getenv("MOZILLA_HOME")) != NULL)
		search_dir(dir, dirlist);

	if ((dir = getenv("PATH")) != NULL) {
		while ((p = strchr(dir, PATH_DELIM)) != NULL) {
			*p = '\0';
			search_dir(dir, dirlist);
			*p++ = PATH_DELIM;
			dir = p;
		}
		search_dir(dir, dirlist);
	}
}

static void
copyright()
{
	report(R_INFO, "\n");
	report(R_INFO, Msgs_get("MCOPYRIGHT"));
}

static char *
get_path()
{
	static char	target[1024];
	int		len;

	target[0] = '\0';
	for (;;) {
		report(R_INFO, Msgs_get("MENTERPATH"), NS_PROG);

		fgets(target, sizeof(target), stdin);

		len = strlen(target);
		while (len > 0 && (isspace(target[len-1])))
			target[--len] = '\0';

		if (!Log_toStdout())
			Log(0, "%s\n", target);

		if (strcmp(target, "?") == 0) {
			report(R_INFO, "\n");
			report(R_INFO, Msgs_get("MPATHHELP"), EXAMPLE_PATH);
			report(R_INFO, "\n");
		}
		else if (strcmp(target, "c") == 0) {
			copyright();
			report(R_INFO, "\n");
		}
		else
			break;
	}
	return target;
}

static char *
get_target(dirlist_t *dirlist)
{
	static char	ans[1024];
	int		i, len;

	report(R_INFO, "\n");

	if (dirlist->n <= 1)
		return get_path();

	for (i = 1; i < dirlist->n; i++)
		report(R_INFO, "      %d = %s \"%s\"\n",
			i, Msgs_get("MVERBFORTIFY"), dirlist->paths[i-1]);

	report(R_INFO, Msgs_get("MOTHERPATH"), i);
	report(R_INFO, "<Enter> = %s\n", Msgs_get("MQUIT"));

	ans[0] = '\0';
	for (;;) {
		report(R_INFO, Msgs_get("MDOSELECT"), dirlist->n);

		fgets(ans, sizeof(ans), stdin);
		if (!Log_toStdout())
			Log(0, ans);

		len = strlen(ans);
		while (len > 0 && isspace(ans[len-1]))
			ans[--len] = '\0';

		if (len == 0)
			break;
		if (isnumeric(ans) && atoi(ans) > 0) {
			if (atoi(ans) < dirlist->n)
				return dirlist->paths[atoi(ans) - 1];
			if (atoi(ans) == dirlist->n)
				return get_path();
		}
	}
	return ans;
}

static int
perform_fortify(char *tgt)
{
	int	len;
	char	buf[1024];

	report(R_INFO, "\n");

	for (;;) {
		report(R_INFO, Msgs_get("MDOGRADE1"));

		if (options_noprompts()) {
			strcpy(buf, Msgs_get("MVERBFORTIFY"));
			report(R_INFO, "%s\n", buf);
		}
		else {
			fgets(buf, sizeof(buf), stdin);
			len = strlen(buf);
			while (len > 0 && (isspace(buf[len-1])))
				buf[--len] = '\0';
			if (!Log_toStdout())
				Log(0, "%s\n", buf);
		}

                if (f_stricmp(buf, Msgs_get("MQUIT")) == 0)
			exit(0);
                if (f_stricmp(buf, Msgs_get("MVERBFORTIFY")) == 0)
			return 1;
                if (f_stricmp(buf, Msgs_get("MVERBDEFORTIFY")) == 0)
			return 0;
                if (f_stricmp(buf, Msgs_get("MHELP")) == 0) {
			report(R_INFO, "\n");
			report(R_INFO, Msgs_get("MSSLHELP"));
			report(R_INFO, "\n");
		}
	}
}

static void
backup_meter(void *vp, int range, int level)
{
	putchar('.');
	fflush(stdout);
}

static int
dobackup(char *tgt)
{
	char	*bak;
	int	len, rtn;

	if (!confirm(Msgs_get("MDOBACKUPCMDLINE")))
		return 0;

	len = strlen(tgt);
	bak = f_malloc(len + 8);
	sprintf(bak, "%s.sav", tgt);

	if (len > 4 && strncmp(tgt + len - 4, ".exe", 4) == 0)
		strcpy(bak + strlen(bak) - 8, ".sav");

	report(R_INFO, Msgs_get("MBACKUPTO"), bak);
	rtn = cp(tgt, bak, 0, backup_meter, NULL, 20);
#ifdef macintosh
	/* Copy the resource fork to the backup file as well. */
	if (rtn == 0)
		rtn = cp(tgt, bak, O_RSRC, NULL, NULL, 0);
#endif
	if (rtn == 0)
		report(R_INFO, Msgs_get("MBACKUPDONE"));

	free(bak);
	return rtn;
}


static int
morph(char *tgt, char *morphs)
{
	int		err;
	int		new_grade;
	char		dflt_morphs[1024];
	char		*prod;
	char		*action;
	tgt_info_t	tgt_info;
	index_entry_t	*ent;

#ifndef macintosh
	int		access_mode;

	access_mode = options_nowrite()? R_OK: W_OK;
	if (access(tgt, access_mode) == -1) {
		if (errno == ENOENT)
			report(R_ERR, Msgs_get("MNOENT"), tgt);
		else if (options_nowrite())
			report(R_ERR, Msgs_get("MREADPERM"), tgt, syserr());
		else
			report(R_ERR, Msgs_get("MWRITEPERM"), tgt, syserr());
		return 1;
	}
#endif

	err = 1;

	report(R_INFO, "\n");
	report(R_INFO, Msgs_get("MFILEIS"), tgt);
	report(R_INFO, "\n");

	err = index_lookup(tgt, &tgt_info);
	ent = tgt_info.ip;
	if (ent == NULL) {
		if (err == ERR_OPEN) {
			report(R_INFO, Msgs_get("MNOTRECOG"));
			report(R_INFO, Msgs_get("MFOPENERR"), tgt, syserr());
		}
		else if (err == ERR_LXCOMPR) {
			report(R_INFO, Msgs_get("MLXCOMPR"), tgt);
		}
		else if (err == ERR_ISSCRIPT) {
			report(R_INFO, Msgs_get("MNOTRECOG"));
			report(R_INFO, Msgs_get("MISSCRIPT"));
		}
		else {
			report(R_INFO, Msgs_get("MNOTRECOG"));
			report(R_INFO, Msgs_get("MNOTNETSC"));
		}
		goto done;
	}

	if (strcmp(ent->flds[IDX_PROD], "nav") == 0)
		prod = Msgs_get("MPRODNAV");
	else if (strcmp(ent->flds[IDX_PROD], "gold") == 0)
		prod = Msgs_get("MPRODGOLD");
	else if (strcmp(ent->flds[IDX_PROD], "comm") == 0)
		prod = Msgs_get("MPRODCOMM");
	else if (strcmp(ent->flds[IDX_PROD], "pro") == 0)
		prod = Msgs_get("MPRODPRO");
	else {
		report(R_INFO, Msgs_get("MPRODUNKN"), ent->flds[IDX_PROD]);
		goto done;
	}

	report(R_INFO, Msgs_get("MPRODDESC"),
		prod, ent->flds[IDX_VERN], ent->flds[IDX_ARCH],
		ent->flds[IDX_GRADE], ent->flds[IDX_COMMENTS]);
	report(R_INFO, "\n");

	if (morphs == NULL && !have_morphs(ent)) {
		report(R_INFO, Msgs_get("MVERUNSUPP"), ffy_vern);
		goto done;
	}

	if (ent->grade == ent->max_grade) {
		if (!options_nowrite()) {
			if (confirm(Msgs_get("MDODEFORTIFYCMDLINE")) == 0) {
				report(R_INFO, Msgs_get("MNOTMODIF"), tgt);
				err = 0;
				goto done;
			}
		}
		new_grade = 0;
		action = Msgs_get("MACTIONDEFORTIFIED");
	}
	else if (ent->grade == 0) {
		if (!options_nowrite()) {
			if (confirm(Msgs_get("MDOFORTIFYCMDLINE")) == 0) {
				report(R_INFO, Msgs_get("MNOTMODIF"), tgt);
				err = 0;
				goto done;
			}
		}
		new_grade = ent->max_grade;
		action = Msgs_get("MACTIONFORTIFIED");
	}
	else {
		if (!perform_fortify(tgt)) {
			new_grade = 0;
			action = Msgs_get("MACTIONDEFORTIFIED");
		} else {
			new_grade = ent->max_grade;
			action = Msgs_get("MACTIONUPGRADED");
		}
	}

	if (!options_nowrite()) {
		err = dobackup(tgt);
		if (err == -1) {		/* cancelled/failed? */
			err = 0;
			goto done;
		}
	}

	if (morphs != NULL) {
		report(R_INFO, Msgs_get("MOVERRIDE"), morphs);
	} else {
		int	len;
		char	cwd[2048];

		getcwd(cwd, sizeof(cwd));
		len = strlen(cwd);
		if (len > 0 && cwd[len-1] == PATH_SEP)
			cwd[len-1] = '\0';
		sprintf(dflt_morphs, "%s%c%s%c%s-%s%c%s",
			cwd, PATH_SEP,
			ent->flds[IDX_ARCH], PATH_SEP,
			ent->flds[IDX_PROD], ent->flds[IDX_VERN], PATH_SEP,
			ent->flds[IDX_MORPHS]);
		morphs = dflt_morphs;
	}

	err = morpher(tgt, morphs, &tgt_info, ent->grade, new_grade, action);

#if !defined(OS2) && !defined(macintosh)
	if (!err && !options_nowrite() && (new_grade > ent->grade)) {
		report(R_INFO, "\n");
		if (confirm(Msgs_get("MDOTESTCMDLINE"))) {
			int	rtn;
			char	cmd[4096];

			sprintf(cmd, "%s https://www.fortify.net/sslcheck.html?fn-%s-cmdline &", tgt, ffy_vern);
			rtn = system(cmd);
			if (rtn == -1)
				report(R_ERR, Msgs_get("MNOTRUN"), tgt, syserr());
			else 
				report(R_INFO, Msgs_get("MCONNECTING"), tgt);
		}
	}
#endif

done:
	(*tgt_info.free_file_data)(tgt_info.file_data);
	return err;
}

static void
banner()
{
	report(R_INFO, "====   Fortify %s; Copyright (C) 1997-2000 Farrell McKay   ====\n", ffy_vern);
	report(R_INFO, Msgs_get("MTERMS"));

	if (options_nowrite())
		report(R_INFO, Msgs_get("MNOWRITE"));
}

static void
usage(char *prog_name)
{
	report(R_INFO, Msgs_get("MUSAGE"), prog_name);
	exit(1);
}

int
main(int argc, char *argv[])
{
	int		i;
	int		rtn = 0;
	char		*prog_name = argv[0];
	char		*index = "Index";
	char		*msgsfile = "Messages";
	char		*morphs = NULL;
	char		*tgt;

	setlocale(LC_ALL, "");

#ifdef macintosh
	argc = ccommand(&argv);
#endif

	Log_init(NULL);
	Log_level(0);
	Msgs_init(msgsfile);

	argc--, argv++;
	while (argc > 0 && argv[0][0] == '-') {
		if (argv[0][1] == '-')
			;
		else if (argv[0][1] == 'c') {
			banner();
			copyright();
			exit(0);
		}
		else if (argv[0][1] == 'f') {
			options_setNoPrompts(1);
		}
		else if (argv[0][1] == 'l') {
			argc--, argv++;
			if (argc < 1)
				usage(prog_name);
			options_setLogFile(*argv);
			Log_init(options_getLogFile());
			Log_level(9);
			/* Msgs_dump(); */
		}
		else if (argv[0][1] == 'm') {
			argc--, argv++;
			if (argc < 1)
				usage(prog_name);
			morphs = *argv;
		}
		else if (argv[0][1] == 'n')
			options_setNoWrite(1);
		else if (argv[0][1] == 'i') {
			argc--, argv++;
			if (argc < 1)
				usage(prog_name);
			index = *argv;
		}
#if 0
		else if (argv[0][1] == 't') {
			int	level;

			argc--, argv++;
			if (argc < 1)
				usage(prog_name);
			level = atoi(*argv);
			if (level < 2)
				usage(prog_name);
			Log_level(level);
		}
#endif
		else if (argv[0][1] == 'V') {
			report(R_INFO, "%s\n", ffy_ident);
			return 0;
		}
		else
			usage(prog_name);
		argc--, argv++;
	}

	if (build_index(index, (char **) &ffy_vern) == -1) {
		usage(prog_name);
		exit(1);
	}

	banner();

	{
		char	buf[2048];

		getcwd(buf, sizeof(buf));
		Log(1, "Cwd is \"%s\"\n", buf);
	}

	if (argc > 0) {
		for (i = 0; i < argc; i++) {
			rtn |= morph(argv[i], morphs);
		}
	}
	else {
		dirlist_t	d;

		search_path(&d);
		for (i = 0; *(tgt = get_target(&d)) != '\0'; i++) {
			rtn |= morph(tgt, morphs);
		}
	}
	return rtn;
}
