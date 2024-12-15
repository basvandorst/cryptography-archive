/*
 # $Id: msgs.c,v 1.2 1998/04/10 10:29:58 fbm Exp fbm $
 # Copyright (C) 1997-1999 Farrell McKay
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
#include "misc.h"
#include "callback.h"
#include "log.h"
#include "msgs.h"

typedef struct {
	int		hash;
	char		*name;
	char		*txt;
} msgs_entry_t;

static int		nmsgs;
static msgs_entry_t	**msgs;

static int
Msgs_namehash(char *name)
{
	int	i = 0;

	while (*name)
		i += (int) (*name++);

	return i;
}

static msgs_entry_t *
Msgs_find(char *name)
{
	int		i, hash;
	msgs_entry_t	**pmsgs;

	hash = Msgs_namehash(name);
	pmsgs = msgs;
	for (i = 0; i < nmsgs; i++, pmsgs++) {
		if ((*pmsgs)->hash == hash)
			if (strcmp((*pmsgs)->name, name) == 0)
				return (*pmsgs);
	}
	return (msgs_entry_t *)0;
}

static void
Msgs_add(char *name, char *txt)
{
	msgs_entry_t	*ent;

	ent = Msgs_find(name);
	if (ent) {
		if (ent->txt)
			free(ent->txt);
		ent->txt = f_strdup(txt);
	}
	else {
		ent = (msgs_entry_t *) f_calloc(1, sizeof(msgs_entry_t));

		ent->hash = Msgs_namehash(name);
		ent->name = f_strdup(name);
		ent->txt = f_strdup(txt);

		nmsgs++;
		msgs = (msgs_entry_t **) f_realloc((void *) msgs,
				nmsgs * sizeof(msgs_entry_t *));
		msgs[nmsgs - 1] = ent;
	}
}

static void
Msgs_preload(void)
{
	Msgs_add("MFATALMEM", "Fatal error: Not enough memory\n");
	Msgs_add("MFOPENERR", "Error: Cannot open \"%s\": %s\n");
	Msgs_add("MLOGOPEN",  "Error: Cannot open log file \"%s\": %s\n");
        Msgs_add("MTERMS",
		"==  This software is free for all forms of non-commercial use.  ==\n"
		"==      Commercial use of this software must be licenced.       ==\n");
	Msgs_add("MUSAGE",
		"Usage: %s [-c] [-f] [-i index] [-l {logfile|-}] [-m morphs] [-n] [-V] [target ...]\n"
		"\t-c  display the copyright and licence information.\n"
		"\t-f  force execution; no user prompts are performed\n"
		"\t-i  override the default index file \"./Index\"\n"
		"\t-l  print log messages, to a file, or \"-\" for stdout\n"
		"\t-m  override the default morphs file\n"
		"\t-n  no file writes; run the program but make no updates\n"
		"\t-V  print this program's version and exit\n");
	Msgs_add("MCOPYRIGHT",
		"This is Fortify for Netscape, a toolkit for upgrading the\n"
		"cryptographic strength of the Netscape web browsers.\n"
		"\n"
		"This toolkit is provided to the recipient under the\n"
		"following terms and conditions:-\n"
		"  1.  This copyright notice must not be removed or modified.\n"
		"  2.  This toolkit may not be reproduced or included in any commercial\n"
		"      media distribution, or commercial publication (for example CD-ROM,\n"
		"      disk, book, magazine, journal) without first obtaining the author's\n"
		"      express permission.\n"
		"  3.  This toolkit, or any component of this toolkit, may not be\n"
		"      used, resold, redeveloped, rewritten, enhanced or otherwise\n"
		"      employed by a commercial organisation, or as part of a commercial\n"
		"      venture, without first obtaining the author's express permission.\n"
		"  4.  Subject to the above conditions being observed (1-3), this toolkit\n"
		"      may be freely reproduced or redistributed.\n"
		"  5.  To the extent permitted by applicable law, this software is\n"
		"      provided \"as-is\", without warranty of any kind, including\n"
		"      without limitation, the warrantees of merchantability,\n"
		"      freedom from defect, non-infringement, and fitness for\n"
		"      purpose.  In no event shall the author be liable for any\n"
		"      direct, indirect or consequential damages however arising\n"
		"      and however caused.\n"
		"  6.  Subject to the above conditions being observed (1-5),\n"
		"      this toolkit may be used at no cost to the recipient.\n");
}

void
Msgs_init(char *msgsfile)
{
	char		buf[4096], *bufptr;
	int		len, textlen;
	char		*name;
	char		*text;
	FILE		*ifp;
	enum		{ begin, msgname, msgtext } state;

	Msgs_preload();

	ifp = fopen(msgsfile, "r");
	if (ifp == NULL) {
		report(R_ERR, Msgs_get("MFOPENERR"), msgsfile, syserr());
		return;
	}

	state = begin;
	textlen = 0;
	name = text = NULL;

	while (fgets(buf, sizeof(buf), ifp)) {

		for (bufptr = buf; isspace(*bufptr); bufptr++)
			;
		len = strlen(bufptr);
		while (len > 0 && isspace(bufptr[len-1]))
			bufptr[--len] = '\0';

		if (len == 0 || *bufptr == '#')
			continue;

		switch (state) {
		case msgname:
			if (strcmp(bufptr, "}") == 0)
				state = begin;
			else {
				name = f_strdup(bufptr);
				state = msgtext;
			}
			break;
		case msgtext:
			if (strcmp(bufptr, "}") == 0) {
				if (name && text)
					Msgs_add(name, text);
				state = begin;
			}
			else {
				if (len < 2 || *bufptr != '"' || bufptr[len-1] != '"')
					continue;
				bufptr++;
				len -= 2;

				len = unescape(bufptr, len);
				bufptr[len] = '\0';

				if (text) {
					textlen += len;
					text = (char *) f_realloc((void *) text, textlen + 1);
					strcat(text, bufptr);
				}
				else {
					textlen = len;
					text = f_strdup(bufptr);
				}
			}
			break;
		default:
			if (name)
				free(name);
			if (text)
				free(text);
			textlen = 0;
			name = text = NULL;
			if (strcmp(bufptr, "{") == 0) {
				state = msgname;
			}
			break;
		}
	}

	fclose(ifp);
}

char *
Msgs_get(char *name)
{
	msgs_entry_t	*ent;
	static char	notext[1024];

	ent = Msgs_find(name);
	if (ent && ent->txt)
		return ent->txt;

	/*
	 * We must wire in this fatal error message definition to avoid
	 * a potential recursive loop in the message handling routines.
	 */
	if (strcmp(name, "MFATALMEM") == 0)
		return "Fatal error: Not enough memory\n";
	
	sprintf(notext, "Error: message text is missing for message \"%s\"\n", name);
	return notext;
}

void
Msgs_dump(void)
{
	int		i;
	msgs_entry_t	**pmsgs;

	Log(1, "t1>> Msgs_dump: nmsgs=%d\n", nmsgs);

	pmsgs = msgs;
	for (i = 0; i < nmsgs; i++, pmsgs++)
		Log(2, "t2>>   \"%s\" [%d]\t\"%s\"\n", (*pmsgs)->name, (*pmsgs)->hash, (*pmsgs)->txt);
}
