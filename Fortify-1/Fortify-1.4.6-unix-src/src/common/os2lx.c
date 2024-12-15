/*
 # $Id: os2lx.c,v 1.2 1998/04/10 10:26:06 fbm Exp fbm $
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
#include "os2obj.h"
#include "os2page.h"
#include "log.h"
#include "morpher.h"
#include "md5.h"
#include "spans.h"

#define PAGESZ			4096

static int
lxFile_read_hdr(int fd, lxfile_t *lx)
{
	int	i;

	i = read(fd, (char *) &lx->mz, sizeof(SIMPLE_MZ_EXE));
	if (i != sizeof(SIMPLE_MZ_EXE) || lx->mz.magic != 0x5a4d) /* "MZ" as a short int */
		return -1;

	i = lseek(fd, lx->mz.lfanew, SEEK_SET);
	if ((unsigned) i != lx->mz.lfanew)
		return -1;

	i = read(fd, (char *) &lx->lx, sizeof(LX_EXE));
	if (i != sizeof(LX_EXE))
		return -1;

	if (lx->lx.Magic != 0x584c			/* "LX" as a short int */
		|| lx->lx.PgSize != PAGESZ
		|| lx->lx.ModNumPgs <= 0
		|| lx->lx.NumObjects <= 0)
		return -1;

	return 0;
}

void
lxFile_print_hdr(lxfile_t *lx)
{
	Log(6, "t6>> mz.lfanew = %ld\n", lx->mz.lfanew);
	Log(6, "t6>> LX header: %d [0x%x] bytes\n", sizeof(lx->lx), sizeof(lx->lx));

	Log(6, "t6>> \n");
	Log(6, "t6>> Magic =  %d  0x%x\n", lx->lx.Magic, lx->lx.Magic);
	Log(6, "t6>> ByteOrder =  %d  0x%x\n", lx->lx.ByteOrder, lx->lx.ByteOrder);
	Log(6, "t6>> WordOrder =  %d  0x%x\n", lx->lx.WordOrder, lx->lx.WordOrder);
	Log(6, "t6>> FormatLevel =  %ld  0x%lx\n", lx->lx.FormatLevel, lx->lx.FormatLevel);
	Log(6, "t6>> CpuType =  %d  0x%x\n", lx->lx.CpuType, lx->lx.CpuType);
	Log(6, "t6>> OSType =  %d  0x%x\n", lx->lx.OSType, lx->lx.OSType);
	Log(6, "t6>> ModVersion =  %ld  0x%lx\n", lx->lx.ModVersion, lx->lx.ModVersion);
	Log(6, "t6>> ModFlags =  %ld  0x%lx\n", lx->lx.ModFlags, lx->lx.ModFlags);
	Log(6, "t6>> ModNumPgs =  %ld  0x%lx\n", lx->lx.ModNumPgs, lx->lx.ModNumPgs);
	Log(6, "t6>> EIPObjNum =  %ld  0x%lx\n", lx->lx.EIPObjNum, lx->lx.EIPObjNum);
	Log(6, "t6>> EIP =  %ld  0x%lx\n", lx->lx.EIP, lx->lx.EIP);
	Log(6, "t6>> ESPObjNum =  %ld  0x%lx\n", lx->lx.ESPObjNum, lx->lx.ESPObjNum);
	Log(6, "t6>> Esp =  %ld  0x%lx\n", lx->lx.Esp, lx->lx.Esp);
	Log(6, "t6>> PgSize =  %ld  0x%lx\n", lx->lx.PgSize, lx->lx.PgSize);
	Log(6, "t6>> PgOfsShift =  %ld  0x%lx\n", lx->lx.PgOfsShift, lx->lx.PgOfsShift);
	Log(6, "t6>> FixupSectionSize =  %ld  0x%lx\n", lx->lx.FixupSectionSize, lx->lx.FixupSectionSize);
	Log(6, "t6>> FixupCksum =  %ld  0x%lx\n", lx->lx.FixupCksum, lx->lx.FixupCksum);
	Log(6, "t6>> LoaderSecSize =  %ld  0x%lx\n", lx->lx.LoaderSecSize, lx->lx.LoaderSecSize);
	Log(6, "t6>> LoaderSecCksum =  %ld  0x%lx\n", lx->lx.LoaderSecCksum, lx->lx.LoaderSecCksum);
	Log(6, "t6>> ObjTblOfs =  %ld  0x%lx\n", lx->lx.ObjTblOfs, lx->lx.ObjTblOfs);
	Log(6, "t6>> NumObjects =  %ld  0x%lx\n", lx->lx.NumObjects, lx->lx.NumObjects);
	Log(6, "t6>> ObjPgTblOfs =  %ld  0x%lx\n", lx->lx.ObjPgTblOfs, lx->lx.ObjPgTblOfs);
	Log(6, "t6>> ObjIterPgsOfs =  %ld  0x%lx\n", lx->lx.ObjIterPgsOfs, lx->lx.ObjIterPgsOfs);
	Log(6, "t6>> RscTblOfs =  %ld  0x%lx\n", lx->lx.RscTblOfs, lx->lx.RscTblOfs);
	Log(6, "t6>> NumRscTblEnt =  %ld  0x%lx\n", lx->lx.NumRscTblEnt, lx->lx.NumRscTblEnt);
	Log(6, "t6>> ResNameTblOfs =  %ld  0x%lx\n", lx->lx.ResNameTblOfs, lx->lx.ResNameTblOfs);
	Log(6, "t6>> EntryTblOfs =  %ld  0x%lx\n", lx->lx.EntryTblOfs, lx->lx.EntryTblOfs);
	Log(6, "t6>> ModDirOfs =  %ld  0x%lx\n", lx->lx.ModDirOfs, lx->lx.ModDirOfs);
	Log(6, "t6>> NumModDirs =  %ld  0x%lx\n", lx->lx.NumModDirs, lx->lx.NumModDirs);
	Log(6, "t6>> FixupPgTblOfs =  %ld  0x%lx\n", lx->lx.FixupPgTblOfs, lx->lx.FixupPgTblOfs);
	Log(6, "t6>> FixupRecTblOfs =  %ld  0x%lx\n", lx->lx.FixupRecTblOfs, lx->lx.FixupRecTblOfs);
	Log(6, "t6>> ImpModTblOfs =  %ld  0x%lx\n", lx->lx.ImpModTblOfs, lx->lx.ImpModTblOfs);
	Log(6, "t6>> NumImpModEnt =  %ld  0x%lx\n", lx->lx.NumImpModEnt, lx->lx.NumImpModEnt);
	Log(6, "t6>> ImpProcTblOfs =  %ld  0x%lx\n", lx->lx.ImpProcTblOfs, lx->lx.ImpProcTblOfs);
	Log(6, "t6>> PerPgCksumOfs =  %ld  0x%lx\n", lx->lx.PerPgCksumOfs, lx->lx.PerPgCksumOfs);
	Log(6, "t6>> DataPgOfs =  %ld  0x%lx\n", lx->lx.DataPgOfs, lx->lx.DataPgOfs);
	Log(6, "t6>> NumPreloadPg =  %ld  0x%lx\n", lx->lx.NumPreloadPg, lx->lx.NumPreloadPg);
	Log(6, "t6>> NResNameTblOfs =  %ld  0x%lx\n", lx->lx.NResNameTblOfs, lx->lx.NResNameTblOfs);
	Log(6, "t6>> NResNameTblLen =  %ld  0x%lx\n", lx->lx.NResNameTblLen, lx->lx.NResNameTblLen);
	Log(6, "t6>> NResNameTblCksum =  %ld  0x%lx\n", lx->lx.NResNameTblCksum, lx->lx.NResNameTblCksum);
	Log(6, "t6>> AutoDSObj =  %ld  0x%lx\n", lx->lx.AutoDSObj, lx->lx.AutoDSObj);
	Log(6, "t6>> DebugInfoOfs =  %ld  0x%lx\n", lx->lx.DebugInfoOfs, lx->lx.DebugInfoOfs);
	Log(6, "t6>> DebugInfoLen =  %ld  0x%lx\n", lx->lx.DebugInfoLen, lx->lx.DebugInfoLen);
	Log(6, "t6>> NumInstPreload =  %ld  0x%lx\n", lx->lx.NumInstPreload, lx->lx.NumInstPreload);
	Log(6, "t6>> NumInstDemand =  %ld  0x%lx\n", lx->lx.NumInstDemand, lx->lx.NumInstDemand);
	Log(6, "t6>> HeapSize =  %ld  0x%lx\n", lx->lx.HeapSize, lx->lx.HeapSize);
}

lxfile_t *
lxFile_is_lx(int fd)
{
	lxfile_t	*lx;

	lx = (lxfile_t *) f_calloc(1, sizeof(lxfile_t));

	if (lxFile_read_hdr(fd, lx) == -1
		|| lxPages_build(fd, lx) == -1
		|| lxObj_build(lx, fd) == -1) {
		free(lx);
		return NULL;
	}
	return lx;
}

void
lxFile_free(void *vp)
{
	int		i;
	lxfile_t	*lx = (lxfile_t *) vp;

	if (lx) {
		for (i = 0; i < lx->nlxObjects; i++)
			free(lx->lxObjects[i]);
		free(lx->lxObjects);
		lx->lxObjects = NULL;
		lx->nlxObjects = 0;

		for (i = 0; i < lx->npages; i++)
			free(lx->pages[i]);
		free(lx->pages);
		lx->pages = NULL;
		lx->npages = 0;
	}
}

static int
lxFile_md5_page(int start, int stop, void *p1, void *p2, void *p3)
{
	unsigned char	*p;
	int		pageno;
	lxfile_t	*lx = (lxfile_t *) p1;
	int		fd = (int) p2;
	MD5_CTX		*c = (MD5_CTX *) p3;

	for (pageno = start; pageno <= stop; pageno++) {

		Log(4, "t4>> LX MD5 calc for page %d\n", pageno);

		if (lxPages_uncompressed(lx, pageno) != 1) {
			Log(3, "t3>> LX page %d is compressed\n", pageno);
			return ERR_LXCOMPR;
		}

		p = lxPages_get_data(lx, pageno, fd);
		if (!p)
			return -1;

		MD5_Update(c, p, (unsigned long) lx->lx.PgSize);
	}
	return 0;
}

char *
lxFile_md5_calc(lxfile_t *lx, int fd, char *span, int *err)
{
	int			i, rtn;
        MD5_CTX			c;
        unsigned char           md[MD5_DIGEST_LENGTH];
        static char   		*p, buf[512];

	MD5_Init(&c);

	rtn = do_spans(span, lxFile_md5_page, (void *) lx, (void *) fd, (void *) &c);
	if (rtn != 0) {
		*err = rtn;
		return "";
	}

	MD5_Final(md, &c);

        for (i = 0, p = buf; i < MD5_DIGEST_LENGTH; i++, p += 2)
                sprintf(p, "%02x", md[i]);
	return buf;
}

unsigned long
lxFile_get_offset(lxfile_t *lx, unsigned long addr)
{
	unsigned long	page_base = (addr & ~(lx->lx.PgSize - 1));
	unsigned long	page_offset = (addr & (lx->lx.PgSize - 1));

	page_base = lxPages_get_file_offset(lx, page_base);
	if (page_base != 0)
		return page_base + page_offset;
	return 0;
}
