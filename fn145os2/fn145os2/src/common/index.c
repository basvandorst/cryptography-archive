/*
 # $Id: index.c,v 1.11 1998/04/10 10:27:18 fbm Exp fbm $
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
#include "index.h"
#include "morpher.h"
#include "md5_calc.h"
#include "pef.h"
#include "os2lx.h"
#include "os2obj.h"
#include "os2page.h"
#include "log.h"
#include "callback.h"
#include "msgs.h"

#define VERN_STR	"version"

static int		idx_sz;
static index_entry_t	**idx_ents;

int
have_morphs(index_entry_t *ent)
{
	return (ent->flds[IDX_MORPHS] && ent->flds[IDX_MORPHS][0] != '\0');
}

int
build_index(char *indexfile, char **vern)
{
	char		buf[1024];
	FILE		*ifp;
	int		i, len, nflds;
	int		vern_seen	= 0;
	char		*p;
	index_entry_t	*ent;

	idx_sz = 0;

	ifp = fopen(indexfile, "r");
	if (ifp == NULL) {
		report(R_ERR, Msgs_get("MFOPENERR"), indexfile, syserr());
		return -1;
	}

	while (fgets(buf, sizeof(buf), ifp)) {
		if (*buf == '#' || *buf == '\n')
			continue;
		len = strlen(buf);
		if (len <= 1)
			continue;
		
		if (!vern_seen && strncmp(buf, VERN_STR, strlen(VERN_STR)) == 0) {
			p = buf + strlen(VERN_STR);
			while (isspace(*p))
				p++;
			*vern = f_strndup(p, strlen(p)-1);
			vern_seen++;
			continue;
		}

		ent = (index_entry_t *) f_calloc(1, sizeof(index_entry_t));

		ent->raw = p = f_strndup(buf, len);

		for (i = nflds = 0; i < IDX_NFLDS; i++) {
			while (isspace(*p))
				p++;
			ent->flds[i] = p;
			while (*p && !isspace(*p))
				p++;
			*p = '\0';
			if (strlen(ent->flds[i]) > 0)
				nflds++;
			if (strcmp(ent->flds[i], "-") == 0)
				ent->flds[i] = p;

			if (p < ent->raw + len)
				p++;
		}

		if (nflds != IDX_NFLDS) {
			free(ent->raw);
			free(ent);
			continue;
		}

		ent->size = atoi(ent->flds[IDX_SZ]);
		ent->grade = atoi(ent->flds[IDX_GRADE]);
		ent->max_grade = atoi(ent->flds[IDX_MAX_GRADE]);

		if (ent->grade == 0) {
			ent->flds[IDX_GRADE] = Msgs_get("MGRADE0");
		}
		else if (ent->grade == ent->max_grade) {
			ent->flds[IDX_GRADE] = Msgs_get("MGRADEMAX");
		}
		else if (ent->grade == 1) {
			ent->flds[IDX_GRADE] = Msgs_get("MGRADE1");
		}
		else {
			ent->flds[IDX_GRADE] = Msgs_get("MGRADENULL");
		}

		idx_sz++;
		idx_ents = (index_entry_t **) f_realloc((void *) idx_ents,
				idx_sz * sizeof(index_entry_t *));
		idx_ents[idx_sz - 1] = ent;
	}

	fclose(ifp);
	return idx_sz;
}

int
is_lx_entry(index_entry_t *ip)
{
	return (strcmp(ip->flds[IDX_SZ], "lx") == 0);
}

int
is_pef_entry(index_entry_t *ip)
{
	return (strcmp(ip->flds[IDX_SZ], "pef") == 0);
}

static int
is_script(int ifd)
{
	int		n;
	static char	script_hdr[] = { '#', '!' };
	char		tmp[sizeof(script_hdr)];

	lseek(ifd, 0L, SEEK_SET);
	n = read(ifd, tmp, sizeof(tmp));
	if (n != sizeof(tmp))
		return -1;
	return (memcmp(tmp, script_hdr, sizeof(tmp)) == 0);
}

static void
free_file_data_stub(void *vp)
{
}

int
index_lookup(const char *tgt, tgt_info_t *tinf)
{
	int		i, ifd;
	lxfile_t	*lx;
	peffile_t	*ppef;
	struct stat	st;
	index_entry_t	*ip;
	char		*md5 = NULL;
	char		*md5span = NULL;
	int		err;

	Log(2, "t2>> Index lookup:\n");

	err = 0;
	tinf->ip = (index_entry_t *)0;
	tinf->file_data = (void *)0;
	tinf->free_file_data = free_file_data_stub;

	if ((ifd = open(tgt, OPENFL(O_RDONLY), 0666)) == -1
		|| fstat(ifd, (struct stat *) &st) == -1) {
		Log(2, "t2>> Error ERR_OPEN\n");
		close(ifd);
		return ERR_OPEN;
	}

	lx = lxFile_is_lx(ifd);
	ppef = pef_is_peffile(ifd);
	Log(2, "t2>> File size = %ld\n", (long) st.st_size);
	Log(2, "t2>> File is%s LX format\n", lx? "": " not");
	Log(2, "t2>> File is%s PEF format\n", ppef? "": " not");
	if (lx) {
		lxFile_print_hdr(lx);
		lxObj_print(lx);
		lxPages_print(lx);
		tinf->file_data = (void *) lx;
		tinf->free_file_data = lxFile_free;
	}
	if (ppef) {
		pef_print_hdrs(ppef);
		tinf->file_data = (void *) ppef;
		tinf->free_file_data = pef_free;
	}

	for (i = 0; i < idx_sz; i++) {
		ip = idx_ents[i];
		if (lx && is_lx_entry(ip)) {
			if (md5 == NULL || strcmp(ip->flds[IDX_MD5SPAN], md5span) != 0) {
				md5 = lxFile_md5_calc(lx, ifd, ip->flds[IDX_MD5SPAN], &err);
				md5span = ip->flds[IDX_MD5SPAN];
				if (*md5 == '\0' && err == ERR_LXCOMPR) {
					close(ifd);
					lxFile_free(lx);
					tinf->file_data = (void *)0;
					return err;
				}
				Log(3, "t3>> md5_calc = %s\n", md5);
				Log(3, "t3>> md5_span = %s\n", md5span);
			}
			Log(3, "t3>> match against %s\n", ip->flds[IDX_MD5]);
			if (strcmp(ip->flds[IDX_MD5], md5) == 0) {
				close(ifd);
				tinf->ip = ip;
				return 0;
			}
		}
		else if (ppef && is_pef_entry(ip)) {
			if (md5 == NULL || strcmp(ip->flds[IDX_MD5SPAN], md5span) != 0) {
				md5 = pef_md5_calc(ppef, ifd, ip->flds[IDX_MD5SPAN]);
				md5span = ip->flds[IDX_MD5SPAN];
				Log(3, "t3>> md5_calc = %s\n", md5);
				Log(3, "t3>> md5_span = %s\n", md5span);
			}
			Log(3, "t3>> match against %s\n", ip->flds[IDX_MD5]);
			if (strcmp(ip->flds[IDX_MD5], md5) == 0) {
				close(ifd);
				tinf->ip = ip;
				return 0;
			}
		}
		else if (ip->size == st.st_size) {
			if (md5 == NULL || strcmp(ip->flds[IDX_MD5SPAN], md5span) != 0) {
				md5 = md5_calc(ifd, ip->flds[IDX_MD5SPAN]);
				md5span = ip->flds[IDX_MD5SPAN];
				Log(3, "t3>> md5_calc = %s\n", md5);
				Log(3, "t3>> md5_span = %s\n", md5span);
			}
			Log(3, "t3>> match against %s\n", ip->flds[IDX_MD5]);
			if (strcmp(ip->flds[IDX_MD5], md5) == 0) {
				close(ifd);
				tinf->ip = ip;
				return 0;
			}
		}
	}

	if (st.st_size < 1000000 && is_script(ifd) == 1) {
		Log(2, "t2>> Error ERR_ISSCRIPT\n");
		close(ifd);
		(*tinf->free_file_data)(tinf->file_data);
		tinf->file_data = (void *)0;
		return ERR_ISSCRIPT;
	}

	Log(2, "t2>> No matches\n");

	close(ifd);
	(*tinf->free_file_data)(tinf->file_data);
	tinf->file_data = (void *)0;
	return ERR_NOMATCH;
}
