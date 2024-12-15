/*
 # $Id: index.h,v 1.9 1998/04/10 10:27:18 fbm Exp fbm $
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

#ifndef INDEX_H
#define INDEX_H

#ifdef __cplusplus
extern "C" {
#endif

#define	IDX_SZ		0
#define	IDX_MD5SPAN	1
#define	IDX_MD5		2
#define	IDX_PROD	3
#define	IDX_VERN	4
#define	IDX_GRADE	5
#define	IDX_MAX_GRADE	6
#define	IDX_MORPHS	7
#define	IDX_ARCH	8
#define	IDX_COMMENTS	9
#define	IDX_NFLDS	10

typedef struct {
	char		*raw;
	char		*flds[IDX_NFLDS];
	int		size;
	int		grade;
	int		max_grade;
} index_entry_t;

typedef struct {
	index_entry_t	*ip;
	void		*file_data;
	void		(*free_file_data)(void *);
} tgt_info_t;

int		have_morphs(index_entry_t *ent);
int		build_index(char *indexfile, char **vern);
int		index_lookup(const char *tgt, tgt_info_t *tinf);
int		is_pef_entry(index_entry_t *ip);
int		is_lx_entry(index_entry_t *ip);

#ifdef __cplusplus
}
#endif

#endif
