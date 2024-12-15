/*
 # $Id: os2obj.c,v 1.2 1998/04/10 10:27:18 fbm Exp fbm $
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

#include "misc.h"
#include "os2lx.h"
#include "os2page.h"
#include "os2obj.h"
#include "log.h"


static lxObject_t *
lxObj_new
(
	unsigned long		_size,
	unsigned long		_loadAddr,
	unsigned long		_flags,
	unsigned long		_pageTblIndex,
	unsigned long		_nPageTblEntries
)
{
	lxObject_t		*p;

	p = (lxObject_t *) f_calloc(1, sizeof(lxObject_t));
	p->size = _size;
	p->loadAddr = _loadAddr;
	p->flags = _flags;
	p->pageTblIndex = _pageTblIndex;
	p->nPageTblEntries = _nPageTblEntries;

	return p;
}

static void
lxObj_print_entry(int i, lxObject_t *lxObj)
{
	Log(6, "t6>> Object %3d: %#10lx %#10lx %8ld %8ld %#8lx ",
			i, lxObj->loadAddr, lxObj->size,
			lxObj->pageTblIndex, lxObj->nPageTblEntries,
			lxObj->flags);

	if (lxObj->flags) {
		if (lxObj->flags & 0x1)
			Log(6, " RD");
		if (lxObj->flags & 0x2)
			Log(6, " WR");
		if (lxObj->flags & 0x4)
			Log(6, " EX");
		if (lxObj->flags & 0x8)
			Log(6, " RESRC");
		if (lxObj->flags & 0x10)
			Log(6, " DISCRD");
		if (lxObj->flags & 0x20)
			Log(6, " SHARED");
		if (lxObj->flags & 0x40)
			Log(6, " PRELOAD-PGS");
		if (lxObj->flags & 0x80)
			Log(6, " INVALID-PGS");
		if (lxObj->flags & 0x300)
			Log(6, " RES-CONTIG");
		else {
			if (lxObj->flags & 0x100)
				Log(6, " ZERO-PGS");
			if (lxObj->flags & 0x200)
				Log(6, " RES");
		}
		if (lxObj->flags & 0x400)
			Log(6, " RES-LONG-LOCK");
		if (lxObj->flags & 0x800)
			Log(6, " RESRVD");
		if (lxObj->flags & 0x1000)
			Log(6, " 16:16");
		if (lxObj->flags & 0x2000)
			Log(6, " BIG-BIT");
		if (lxObj->flags & 0x4000)
			Log(6, " CONFORM");
		if (lxObj->flags & 0x8000)
			Log(6, " IO-PRIV");
	}
	Log(6, "\n");
}

void
lxObj_print(lxfile_t *lx)
{
	int	i;

	Log(6, "t6>> \n");
	Log(6, "t6>> Object        baseAddr     size   pgTblIdx pgTblEnts   flags\n");
	for (i = 0; i < lx->nlxObjects; i++)
		lxObj_print_entry(i+1, lx->lxObjects[i]);
}

static void
lxObj_inject(lxfile_t *lx, lxObject_t *lxObj, int pgSize)
{
	unsigned int	j;

	for (j = 0; j < lxObj->nPageTblEntries; j++) {
		lxPages_set_addr(lx, lxObj->pageTblIndex + j - 1, 
					lxObj->loadAddr + (j * pgSize));
	}
}

int
lxObj_build(lxfile_t *lx, int fd)
{
	int		i, off;
	unsigned int	objno;
	LX_OBJ		obj;

	off = lx->mz.lfanew + lx->lx.ObjTblOfs;
	i = lseek(fd, off, SEEK_SET);
        if (i != off) {
                fprintf(stderr, "LX-ObjTable: error locating offset %#08x: ", off);
                perror("");
		return -1;
	}

        lx->lxObjects = (lxObject_t **) f_calloc(lx->lx.NumObjects, sizeof(lxObject_t *));
        lx->nlxObjects = lx->lx.NumObjects;

        for (objno = 0; objno < lx->lx.NumObjects; objno++) {
		i = read(fd, (char *) &obj, sizeof(obj));
		if (i != sizeof(obj)) {
                        fprintf(stderr, "LX-ObjTable %d: error reading %d bytes: ",
                                i, sizeof(obj));
                        perror("");
			return -1;
		}

                lx->lxObjects[objno] = lxObj_new(obj.size, obj.reloc_base_addr,
                                obj.obj_flags, obj.pg_tbl_index, obj.num_pg_tbl_entries);

		lxObj_inject(lx, lx->lxObjects[objno], lx->lx.PgSize);
        }
	return 0;
}


int
lxObj_write_mem_image(lxfile_t *lx, int ifd, int ofd)
{
        int     	i;
	unsigned int	pageno, objno;
	unsigned long	addr;
	lxObject_t	*obj;
	char		*nulls;
	unsigned char	*p;

        addr = lseek(ifd, 0L, SEEK_SET);
        if (addr != 0)
                return -1;
	
	nulls = (char *) f_calloc(lx->lx.PgSize, sizeof(char));

	Log(8, "t8>> write_mem_image\n");

        for (objno = 0; objno < lx->lx.NumObjects; objno++) {
		obj = lx->lxObjects[objno];
		if (obj->loadAddr == 0)
			continue;
		
		Log(8, "t8>> next obj addr is %#lx\n", obj->loadAddr);

		for (; addr < obj->loadAddr; addr += lx->lx.PgSize) {
			i = write(ofd, nulls, lx->lx.PgSize);
			if ((unsigned) i != lx->lx.PgSize) {
				free(nulls);
				return -1;
			}
		}

		for (pageno = 0; pageno < obj->nPageTblEntries; pageno++) {
			p = lxPages_get_data(lx, obj->pageTblIndex + pageno - 1, ifd);
			i = write(ofd, (char *) p, lx->lx.PgSize);
			if ((unsigned) i != lx->lx.PgSize) {
				free(nulls);
				return -1;
			}
			addr += lx->lx.PgSize;
		}
	}

	free(nulls);
	return 0;
}
