/*
 # $Id: os2page.h,v 1.2 1998/04/10 10:26:06 fbm Exp fbm $
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

#ifndef OS2PAGE_H
#define OS2PAGE_H

#define PAGE_PHYS	0
#define PAGE_ITER	1
#define PAGE_INVALID	2
#define PAGE_ZERO	3
#define PAGE_RANGE	4
#define PAGE_COMPR	5

typedef struct lxPage {
	unsigned long		rawOffset;
	unsigned short		rawSz;
	unsigned short		rawFlags;
	unsigned short		size;
	unsigned short		flags;
	unsigned long		fileOffset;
	unsigned long		addr;
	unsigned long		fixupOffset;
	unsigned char		*data;
} lxPage_t;

#ifdef __cplusplus
extern "C" {
#endif

int		lxPages_uncompressed(lxfile_t *lx, int pageNum);
unsigned char	*lxPages_get_data(lxfile_t *lx, int i, int fd);
unsigned long	lxPages_get_file_offset(lxfile_t *lx, unsigned long addr);
void		lxPages_set_addr(lxfile_t *lx, int i, unsigned long addr);
void		lxPages_set_fixup(lxfile_t *lx, int i, unsigned long offset);
void		lxPages_print(lxfile_t *lx);
int		lxPages_build(int fd, lxfile_t *_lx);

#ifdef __cplusplus
}
#endif

#endif
