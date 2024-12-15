/*
 # $Id: os2page.c,v 1.2 1998/04/10 10:26:06 fbm Exp fbm $
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
#include "os2lx.h"
#include "os2page.h"
#include "log.h"


static lxPage_t *
lxPages_new
(
	unsigned long		_rawOffset,
	unsigned short		_rawSz,
	unsigned short		_rawFlags,
	unsigned long		_fileOffset
)
{
	lxPage_t		*p;

	p = (lxPage_t *) f_calloc(1, sizeof(lxPage_t));
	p->rawOffset = _rawOffset;
	p->rawSz = p->size = _rawSz;
	p->rawFlags = p->flags = _rawFlags;
	p->fileOffset = _fileOffset;

	return p;
}

int
lxPages_uncompressed(lxfile_t *lx, int pageNum)
{
	if (pageNum < 0 || pageNum >= lx->npages)
		return -1;

	return (lx->pages[pageNum]->rawFlags == PAGE_PHYS)? 1: 0;
}

#if 0
/*
 * This function decompresses a block of Lempel-Ziv compressed data.
 */
static int
lz_expand(unsigned char *in, int size, unsigned char *out, int outsize)
{
				// returns 0 - output buffer overflow!
  int outleft = outsize;
  while ( size > 0 && outleft > 0 ) {
    unsigned char f = *in++;
    unsigned int plain;
    unsigned int backnum;
    unsigned int backoff;
    size--;
    switch ( f & 3 ) {
      case 0:
        plain = f >> 2;
	if ( plain != 0 ) {
//00XXXXXX
	  if ( plain > outleft ) plain = outleft;
	  memcpy(out,in,plain);
	  out += plain;
	  outleft -= plain;
	  in  += plain;
	  size -= plain;
	} else {
//00000000:XXXXXXXX:YYYYYYYY
	  unsigned char byte;
	  plain = *in++;			// number of repeatings.
	  size--;
//00000000:00000000
	  if ( plain == 0 ) break;
	  byte = *in++;
	  size--;
	  if ( plain > outleft ) plain = outleft;
	  memset(out,byte,plain);
	  out += plain;
	  outleft -= plain;
	}
        continue;
      case 1:
//10XXYYYZ:ZZZZZZZZ
	plain   = (f >> 2) & 3;
	backnum = ((f >> 4) & 7) + 3;
	backoff = (*in++ << 1) + (f >> 7);
	size--;
fullCopy:
	if ( plain > outleft ) plain = outleft;
	memcpy(out,in,plain);
	size -= plain;
	in  += plain;
	out += plain;
	outleft -= plain;
backCopy:
	if ( backnum > outleft ) backnum = outleft;
	memcpy(out,out-backoff,backnum);
	out += backnum;
	outleft -= backnum;
	continue;
      case 2:
// 01XXYYYY:YYYYYYYY
	backnum = ((f >> 2) & 3) + 3;
	backoff = (*in++ << 4) + (f >> 4);
	size--;
	goto backCopy;
      case 3: {
//11XXXXYY:YYYYZZZZ:ZZZZZZZZ
	unsigned char yz = *in++;
	unsigned char zz = *in++;
	size--;
	size--;
	plain = (f >> 2) & 0xF;
	backnum = ((f >> 6) & 3) + ((yz & 0xF) << 2);
	backoff = (yz >> 4) + (zz << 4);
	goto fullCopy;
      }
    }
    break;
  }
  if ( outleft > 0 ) memset(out,'\0',outleft);
  return 1;
}
#endif


unsigned char *
lxPages_get_data(lxfile_t *lx, int pageNum, int fd)
{
	int		i;
	lxPage_t	*p;

	if (pageNum < 0 || pageNum >= lx->npages)
		return NULL;

	p = lx->pages[pageNum];
	if (p->data == NULL) {
		i = lseek(fd, p->fileOffset, SEEK_SET);
		if ((unsigned) i != p->fileOffset) {
                        fprintf(stderr, "LX-Page %d: error locating offset %#08lx: ",
                                pageNum, p->fileOffset);
                        perror("");
			return NULL;
		}

		Log(8, "t8>> Getting page %d: %d bytes\n", pageNum, p->rawSz);
		p->data = (unsigned char *) f_calloc(lx->lx.PgSize, sizeof(unsigned char));
		i = read(fd, (char *) p->data, p->rawSz);
		if (i != p->rawSz) {
                        fprintf(stderr, "LX-Page %d: error reading %d bytes at %#08lx (%d): ",
                                pageNum, p->rawSz, p->fileOffset, i);
                        perror("");
			free(p->data);
			p->data = NULL;
			return NULL;
		}

#if 0
                if (p->rawFlags == PAGE_COMPR) {
                        unsigned char   *uncompr;
                        int             rtn;

                        Log(8, "t8>> Uncompressing page %d: ", pageNum);
                        uncompr = (unsigned char *) f_calloc(lx->lx.PgSize, sizeof(unsigned char));
                        rtn = lz_expand(p->data, p->rawSz, uncompr, lx->lx.PgSize);
                        if (rtn <= 0) {
                                fprintf(stderr, "LX-Page %d: de-compression overflow error\n", pageNum);
                                free(uncompr);
                        }
                        else {
                                free(p->data);
                                p->data = uncompr;
                                p->size = lx->lx.PgSize;
                                while (p->size > 0 && p->data[p->size - 1] == '\0')
                                        p->size--;
                                p->flags = PAGE_PHYS;
                                Log(8, "rawSz=%d size=%d\n", p->rawSz, p->size);
                        }
                }
#endif
	}
	return p->data;
}

/*
 * Convert a run-time page base address into a file offset in the
 * LX executable file.  A linear search of the page space is sufficient
 * for the needs of the program.
 */
unsigned long
lxPages_get_file_offset(lxfile_t *lx, unsigned long addr)
{
	int	pagenum;

	for (pagenum = 0; pagenum < lx->npages; pagenum++) {
		if (lx->pages[pagenum]->addr == addr)
			return lx->pages[pagenum]->fileOffset;
	}
	return 0;
}

void
lxPages_set_addr(lxfile_t *lx, int i, unsigned long addr)
{
	if (i < lx->npages)
		lx->pages[i]->addr = addr;
	else {
		fprintf(stderr, "Error bad page number %d\n", i);
		exit(1);
	}
}

void
lxPages_set_fixup(lxfile_t *lx, int i, unsigned long offset)
{
	if (i < lx->npages)
		lx->pages[i]->fixupOffset = offset;
	else {
		fprintf(stderr, "Error bad page number %d\n", i);
		exit(1);
	}
}

static void
lxPages_print_entry(int i, lxPage_t *p)
{
	Log(6, "t6>> Page %6d: %#10lx %#10lx %10d %#10x %#10lx %#8lx %#8x ",
		i, p->rawOffset, p->fileOffset,
		p->rawSz, p->rawSz, p->addr, p->fixupOffset, p->rawFlags);
	if (p->rawFlags == PAGE_PHYS)
		Log(6, " PHYS\n");
	else if (p->rawFlags == PAGE_ITER)
		Log(6, " ITER\n");
	else if (p->rawFlags == PAGE_INVALID)
		Log(6, " INVALID\n");
	else if (p->rawFlags == PAGE_ZERO)
		Log(6, " ZERO\n");
	else if (p->rawFlags == PAGE_RANGE)
		Log(6, " RANGE\n");
	else if (p->rawFlags == PAGE_COMPR)
		Log(6, " COMPR\n");
	else
		Log(6, " ????\n");
}

void
lxPages_print(lxfile_t *lx)
{
        int	i;

        Log(6, "t6>> \n");
        Log(6, "t6>> Page             offset fileOffset     pgSize     pgSize  relocAddr fixupOff    flags\n");
        for (i = 0; i < lx->npages; i++)
                lxPages_print_entry(i+1, lx->pages[i]);
}

int
lxPages_build(int fd, lxfile_t *lx)
{
        int             i, off;
		unsigned int	pageno;
        LX_PG		pg;

        off = lx->mz.lfanew + lx->lx.ObjPgTblOfs;
        i = lseek(fd, off, SEEK_SET);
        if (i != off) {
		fprintf(stderr, "LX-PageTable: error locating offset %#08x: ", off);
		perror("");
		return -1;
	}

	lx->pages = (lxPage_t **) f_calloc(lx->lx.ModNumPgs, sizeof(lxPage_t *));
	lx->npages = lx->lx.ModNumPgs;

        for (pageno = 0; pageno < lx->lx.ModNumPgs; pageno++) {
                i = read(fd, (char *) &pg, sizeof(pg));
		if (i != sizeof(pg)) {
                        fprintf(stderr, "LX-PageTable %d: error reading %d bytes: ",
                                i, sizeof(pg));
                        perror("");
			return -1;
		}

		if (pg.size > lx->lx.PgSize) {
			fprintf(stderr, "NewPage: page size too large (%d > %ld)\n",
				pg.size, lx->lx.PgSize);
			continue;
		}

		lx->pages[pageno] = lxPages_new(pg.offset, pg.size, pg.flags,
				(pg.offset << lx->lx.PgOfsShift) + lx->lx.DataPgOfs);
        }
	return 0;
}
