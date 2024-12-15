/*
 # $Id: os2obj.h,v 1.2 1998/04/10 10:25:03 fbm Exp fbm $
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

#ifndef OS2OBJ_H
#define OS2OBJ_H

typedef struct lxObject {
	unsigned long		size;             /* Load-time size of object */
	unsigned long		loadAddr;	  /* Address the object wants to be loaded at. */
	unsigned long		flags;            /* Read/Write/Execute, Resource, Zero-fill ... */
	unsigned long		pageTblIndex;     /* Index in Object Page Table at which this */
						  /* objects first page is located. */
	unsigned long		nPageTblEntries;  /* Number of consecutive Object  */
} lxObject_t;

#ifdef __cplusplus
extern "C" {
#endif

int		lxObj_build(lxfile_t *lx, int fd);
void		lxObj_print(lxfile_t *lx);
int		lxObj_write_mem_image(lxfile_t *lx, int ifd, int ofd);

#ifdef __cplusplus
}
#endif

#endif
