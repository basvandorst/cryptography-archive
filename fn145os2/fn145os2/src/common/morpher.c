/*
 # $Id: morpher.c,v 1.7 1998/04/10 10:27:56 fbm Exp fbm $
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
#include <memory.h>

#include "misc.h"
#include "index.h"
#include "morpher.h"
#include "pef.h"
#include "cp.h"
#include "os2lx.h"
#include "options.h"
#include "log.h"
#include "callback.h"
#include "msgs.h"


static void
show_vec(int level, unsigned char *src, int len, int format)
{
	if (len > 0) {
		if (format == VEC_NUM_FMT) {
			char pre = '[';
			while (len-- > 0) {
				Log(level, "%c%02x", pre, (unsigned int) *src++);
				pre = ' ';
			}
			Log(level, "]");
		} else if (format == VEC_STR_FMT) {
			Log(level, "\"");
			for (; len-- > 0; src++) {
				if (isprint(*src))
					Log(level, "%c", *src);
				else
					Log(level, "\\%03o", (unsigned int) *src);
			}
			Log(level, "\"");
		}
	}
}


#define MAXMORPHLEN	1024

static int
apply_morphs(int tgt, morph_set *ms, int old_grade, int new_grade, int pass2, int *count)
{
	int		i;
	int		cmp;
	int		candidates = 0;
	morph_t		*p;
	off_t		offset;
	unsigned char	tmp[MAXMORPHLEN];
	vec_t		*oldv, *newv;
	int		errs = 0;

	for (i = 0; i < ms->morphs_sz; i++) {
		p = ms->morphs[i];
		if (p->is_context)
			continue;

		if (new_grade < old_grade && p->grade <= old_grade && p->grade > new_grade) {
			oldv = &p->newv;
			newv = &p->oldv;
		}
		else if (new_grade > old_grade && p->grade > old_grade && p->grade <= new_grade) {
			oldv = &p->oldv;
			newv = &p->newv;
		}
		else
			continue;

		candidates++;
		if (newv->len > MAXMORPHLEN) {
			errs++;
			Log(0, "t0>> Error: Morph \"%s\": this morph is too big!\n",
				p->name);
			continue;
		}

		if (!pass2) {
			cmp = memcmp((void *) oldv->data, (void *) newv->data, oldv->len);
			if (cmp == 0) {
				Log(1, "t1>> Warning: Morph \"%s\": morph changes nothing ??!\n",
					p->name);
			}

			Log(1, "t1>> At file offset 0x%06lx, replacing ", p->filepos);
			show_vec(1, oldv->data, oldv->len, oldv->format);
			Log(1, " with ");
			show_vec(1, newv->data, newv->len, newv->format);
			Log(1, "\n");
		}

		offset = lseek(tgt, (off_t) p->filepos, SEEK_SET);
		if (offset == -1) {
			errs++;
			Log(0, "Morph \"%s\": error locating offset 0x%08lx [run-time 0x%lx]: %s\n",
				p->name, p->filepos, p->base + p->offset, syserr());
			continue;
		}

		if (read(tgt, (char *) tmp, oldv->len) != oldv->len) {
			errs++;
			Log(0, "Morph \"%s\": error reading %d bytes at 0x%08lx: %s\n",
				p->name, oldv->len, p->filepos, syserr());
			continue;
		}

		cmp = memcmp((void *) tmp, (void *) oldv->data, (size_t) oldv->len);
		if (cmp != 0) {
			errs++;
			Log(0, "t0>> Error: Morph \"%s\": data mismatch: expected ", p->name);
			show_vec(0, oldv->data, oldv->len, VEC_NUM_FMT);
			Log(0, ", found ");
			show_vec(0, tmp, oldv->len, VEC_NUM_FMT);
			Log(0, "\n");
			continue;
		}

		if (pass2 && !options_nowrite()) {
			offset = lseek(tgt, (off_t) p->filepos, SEEK_SET);
			if (offset == -1) {
				errs++;
				Log(0,
					"Morph \"%s\": error repositioning at address 0x%08lx [run-time 0x%lx]: %s\n",
					p->name, p->filepos, p->base + p->offset, syserr());
				continue;
			}
			if (write(tgt, (char *) newv->data, newv->len) != newv->len) {
				errs++;
				Log(0, "Morph \"%s\": error writing data to 0x%08lx [run-time 0x%lx]: %s\n",
					p->name, p->filepos, p->base + p->offset, syserr());
				continue;
			}
		}
	}
	*count = candidates;
	return errs;
}

int
morpher(const char *target_name, char *morph_file, tgt_info_t *tinf, int old_grade, int new_grade, char *action)
{
	int			i, len, tgt, errs, rtn, flags;
	const char		*target_orig = target_name;
	char			*wkfile = NULL;
	lxfile_t		*lx = NULL;
	int			candidates = 0;
	FILE			*mf;
	morph_set		*ms;
	static char		msg[8192];
	char			msgtmp[8192];

	*msg = '\0';

	mf = fopen(morph_file, "r");
	if (mf == NULL) {
		sprintf(msg, Msgs_get("MFOPENERR"), morph_file, syserr());
		strcat(msg, Msgs_get("MUNPACKED"));
		report(R_ERR, msg);
		return 1;
	}

	ms = parse(mf);
	fclose(mf);

	if (is_lx_entry(tinf->ip))
		lx = (lxfile_t *) tinf->file_data;

	if (is_pef_entry(tinf->ip)) {
		peffile_t *ppef = tinf->file_data;

		/*
		 * A temporary workfile is required if the target's data segment
		 * is in 'pidata' (pattern initialized) form.
		 */
		if (pef_is_packed(ppef)) {
			len = strlen(target_name);
			wkfile = f_malloc(len + 8);
			sprintf(wkfile, "%s.ffywk", target_name);

			if (len > 4 && strncmp(target_name + len - 4, ".exe", 4) == 0)
				strcpy(wkfile + strlen(wkfile) - 8, ".tmp");
			Log(1, "t1>> Intermediate PEF file: \"%s\"\n", wkfile);

			rtn = pef_prepare(target_name, wkfile, ppef);
			if (rtn == -1) {
				errs = 1;
				goto done;
			}
			
			target_name = wkfile;
		}
	}

	flags = options_nowrite()? OPENFL(O_RDONLY): OPENFL(O_RDWR);
	tgt = open(target_name, flags, 0666);
	if (tgt == -1) {
		sprintf(msg, Msgs_get("MFOPENERR"), target_name, syserr());
		strcat(msg, Msgs_get("MRUNNING"));
		report(R_ERR, msg);
		errs = 1;
		goto done;
	}

	for (i = 0; i < ms->morphs_sz; i++) {
		morph_t *p = ms->morphs[i];
		if (lx)
			p->filepos = lxFile_get_offset(lx, p->base + p->offset);
		else
			p->filepos = p->base + p->offset - ms->offsets[p->seg];
	}

	errs = apply_morphs(tgt, ms, old_grade, new_grade, 0, (int *) &candidates);
	if (errs == 0 && ms->nmorphs > 0 && !options_nowrite()) {
		errs = apply_morphs(tgt, ms, old_grade, new_grade, 1, (int *) &candidates);
	}
	else {
		char	*p;

		p = (errs == 1)? "MRESULTNOCHANGE1": "MRESULTNOCHANGE0";
		sprintf(msg, Msgs_get(p), errs, target_name);
	}

	close(tgt);

	if (wkfile != NULL && errs == 0 && !options_nowrite()) {
		/*
		 * In the case where a temporary work file has been used, this is now
		 * copied back, overwriting the original file.  A copy, rather than a
		 * simple rename, is used to preserve the semantics of file aliases
		 * that are supported on some platforms.
		 */
		Log(1, "t1>> Committing mods to \"%s\"\n", target_orig);
		rtn = cp(wkfile, target_orig, 0, NULL, NULL, 0);
#ifdef macintosh
		if (rtn == 0)
			rtn = cp(wkfile, target_orig, O_RSRC, NULL, NULL, 0);
#endif
		if (rtn == 0)
			Log(1, "t1>> Commit ok.\n");
		else {
			errs = 1;
			sprintf(msg, Msgs_get("MERRCOMMIT"), wkfile, target_orig, syserr());
		}
	}

	if (errs == 0) {
		if (options_nowrite())
			sprintf(msgtmp, Msgs_get("MRESULTCANDO"), target_orig, action);
		else if (new_grade > old_grade)
			sprintf(msgtmp, Msgs_get("MRESULTCONGRATS"), target_orig, action);
		else
			sprintf(msgtmp, Msgs_get("MRESULTDONE"), target_orig, action);

		strcat(msg, msgtmp);

#if defined(OS2)
		if (!options_nowrite()) {
			sprintf(msgtmp, Msgs_get("MOS2REPACK"), target_orig);
			strcat(msg, msgtmp);
		}
#endif
	}

	report((errs > 0)? R_ERR: R_INFO, msg);
done:
	if (wkfile != NULL) {
		unlink(wkfile);
		free(wkfile);
	}
	return ((errs > 0)? 1: 0);
}
