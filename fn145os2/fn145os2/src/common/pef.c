/*
 # $Id: pef.c,v 1.2 1998/04/10 10:29:58 fbm Exp fbm $
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

#include <assert.h>
#include "includes.h"

#include "md5.h"
#include "pef.h"
#include "pidata.h"
#include "msgs.h"
#include "misc.h"
#include "morpher.h"
#include "md5_calc.h"
#include "options.h"
#include "cp.h"
#include "callback.h"
#include "log.h"

#ifdef macintosh
#include <console.h>
#include <unix.h>
#include "Files.h"
#include "Resources.h"
#include "TextUtils.h"
#endif

#define PEF_ALIGN_CONTAINER	0x10
#define PEF_ALIGN_UNPACKED	0x10

#ifndef O_RSRC
#define	O_RSRC			0
#endif


static int
pef_read_hdr(int ifd, peffile_t *ppef)
{
        int     i;

        i = lseek(ifd, (off_t)0, SEEK_SET);
        if ((unsigned) i != (off_t)0)
                return -1;

        i = read(ifd, (char *) &ppef->cHdr, sizeof(ppef->cHdr));
        if (i != sizeof(ppef->cHdr))
                return -1;

        if (ppef->cHdr.tag1 != 0x4A6F7921			/* 'Joy!' as an int */
		|| ppef->cHdr.tag2 != 0x70656666)		/* 'peff' as an int */
                return -1;

	ppef->sHdrBytes = ppef->cHdr.sectionCount * sizeof(PEFSectionHdr);
	ppef->oldsHdr = (PEFSectionHdr *) f_malloc(ppef->sHdrBytes);
	ppef->newsHdr = (PEFSectionHdr *) f_malloc(ppef->sHdrBytes);

	i = read(ifd, (char *) ppef->oldsHdr, ppef->sHdrBytes);
	if (i != ppef->sHdrBytes) {
		Log(1, "t1>> Short read of PEF section header array.\n");
		return -1;
	}
	if (ppef->cHdr.sectionCount != 3
		|| ppef->oldsHdr[0].sectionKind != PEF_SECTION_CODE
		|| ppef->oldsHdr[2].sectionKind != PEF_SECTION_LOADER
		|| (ppef->oldsHdr[1].containerOffset & (PEF_ALIGN_UNPACKED - 1)) != 0) {
		Log(1, "t1>> PEF section layout is not currently supported.\n");
		return -1;
	}

	memcpy((void *) ppef->newsHdr, (void *) ppef->oldsHdr, ppef->sHdrBytes);
        return 0;
}

static int
pef_load(int ifd, peffile_t *ppef)
{
	int		i;

	Log(4, "t4>> pef_load: section 1 kind is '%s' [%d]\n",
		(ppef->oldsHdr[1].sectionKind == PEF_SECTION_UNPACKED)? "unpacked":
		(ppef->oldsHdr[1].sectionKind == PEF_SECTION_PIDATA)? "pidata": "??",
		ppef->oldsHdr[1].sectionKind);

	if (ppef->oldsHdr[1].sectionKind != PEF_SECTION_UNPACKED
		&& ppef->oldsHdr[1].sectionKind != PEF_SECTION_PIDATA)
		return -1;

	ppef->packedSize = ppef->oldsHdr[1].packedSize;
	ppef->unpackedSize = ppef->oldsHdr[1].unpackedSize;
	ppef->packedBuf = ppef->unpackedBuf = NULL;

	i = lseek(ifd, (off_t) ppef->oldsHdr[1].containerOffset, SEEK_SET);
	if (i != ppef->oldsHdr[1].containerOffset) {
		Log(1, "t1>> Error while seeking to pidata offset [%s]\n", syserr());
		goto done;
	}

	ppef->packedBuf = (char *) f_malloc(ppef->packedSize);

	i = read(ifd, ppef->packedBuf, ppef->packedSize);
	if (i != ppef->packedSize) {
		Log(1, "t1>> Error while reading pidata data [%s]\n", syserr());
		goto done;
	}

	if (pef_is_packed(ppef)) {
		ppef->unpackedBuf = (char *) f_malloc(ppef->unpackedSize);
		pidata_unpack(ppef->packedBuf, ppef->packedSize,
				ppef->unpackedBuf, ppef->unpackedSize);
		free(ppef->packedBuf);
		ppef->packedBuf = NULL;
	}
	else {
		assert(ppef->packedSize == ppef->unpackedSize);
		ppef->unpackedBuf = ppef->packedBuf;
		ppef->packedBuf = NULL;
	}

	return 0;
done:
	if (ppef->packedBuf)
		free(ppef->packedBuf);
	if (ppef->unpackedBuf)
		free(ppef->unpackedBuf);
	return -1;
}

#ifdef macintosh
static void
pef_cfrg_update(char *cp, int size0, int delta)
{
	int		i, nFrags;
	cfrgResHdr_t	*hdr;
	cfrgDesc_t	*p;

	hdr = (cfrgResHdr_t *) cp;
	cp += sizeof(cfrgResHdr_t);
	nFrags = hdr->nFrags;
	Log(6, "t6>> PEF 'cfrg' resource: nFrags = %d\n", nFrags);
	Log(4, "t4>> pef_cfrg_update: size0 = %d [0x%x]  delta = %d [0x%x]\n", size0, size0, delta, delta);

	/* Update the length field for container 0 */
	p = (cfrgDesc_t *) cp;
	cp += p->descSize;
	p->fragSize = size0;
	pef_cfrg_dumpDesc(0, p);

	/* Apply the offset delta to all the PEF containers following container 0 */
	for (i = 1; i < nFrags; i++) {
		p = (cfrgDesc_t *) cp;
		cp += p->descSize;
		p->fragOffset += delta;
		pef_cfrg_dumpDesc(i, p);
	}
}
#endif

void
pef_cfrg_dumpDesc(int i, cfrgDesc_t *dp)
{
	Log(6, "t6>> 'cfrg' Frag %d:\n", i);
	Log(6, "t6>>     fragType = %d\n", dp->fragType);
	Log(6, "t6>>     fragLocn = %d [0x%x]\n", dp->fragLocn, dp->fragLocn);
	Log(6, "t6>>     fragOffset = %d [0x%x]\n", dp->fragOffset, dp->fragOffset);
	Log(6, "t6>>     fragSize = %d [0x%x]\n", dp->fragSize, dp->fragSize);
	Log(6, "t6>>     descSize = %d [0x%x]\n", dp->descSize, dp->descSize);
}

static void
PEFContainerHdr_dump(char *heading, PEFContainerHdr *p)
{
	Log(6, "t6>> %s:\n", heading);
	Log(6, "t6>>   tag1: %.*s\n", sizeof(p->tag1), (char *) &p->tag1);
	Log(6, "t6>>   tag2: %.*s\n", sizeof(p->tag2), (char *) &p->tag2);
	Log(6, "t6>>   arch: %.*s\n", sizeof(p->arch), (char *) &p->arch);
	Log(6, "t6>>   formatVersion: %d\n", p->formatVersion);
	Log(6, "t6>>   dateTimeStamp: %d\n", p->dateTimeStamp);
	Log(6, "t6>>   sectionCount: %d\n", p->sectionCount);
	Log(6, "t6>>   instSectionCount: %d\n", p->instSectionCount);
}

static void
PEFSectionHdr_dump(char *heading, int sectionNo, PEFSectionHdr *p)
{
	Log(6, "t6>> %s:\n", heading);
	Log(6, "t6>>   section Number: %d\n", sectionNo);
	Log(6, "t6>>   defaultAddress: 0x%x\n", p->defaultAddress);
	Log(6, "t6>>   totalSize: 0x%x [%d]\n", p->totalSize, p->totalSize);
	Log(6, "t6>>   unpackedSize: 0x%x [%d]\n", p->unpackedSize, p->unpackedSize);
	Log(6, "t6>>   packedSize: 0x%x [%d]\n", p->packedSize, p->packedSize);
	Log(6, "t6>>   containerOffset: 0x%x [%d]\n", p->containerOffset, p->containerOffset);
	Log(6, "t6>>   sectionKind: 0x%x [%d]\n", p->sectionKind, p->sectionKind);
	Log(6, "t6>>   shareKind: 0x%x [%d]\n", p->shareKind, p->shareKind);
	Log(6, "t6>>   alignment: 0x%x [%d]\n", p->alignment, p->alignment);
}

peffile_t *
pef_is_peffile(int ifd)
{
        peffile_t	*ppef;

        ppef = (peffile_t *) f_calloc(1, sizeof(peffile_t));

        if (pef_read_hdr(ifd, ppef) == -1
                || pef_load(ifd, ppef) == -1) {
                pef_free((void *) ppef);
                return NULL;
        }

        return ppef;
}

void
pef_print_hdrs(peffile_t *ppef)
{
	int		i;

	PEFContainerHdr_dump("PEFContainerHdr:", &ppef->cHdr);

	for (i = 0; i < ppef->cHdr.sectionCount; i++)
		PEFSectionHdr_dump("PEFSectionHdr:", i, &ppef->oldsHdr[i]);
}

char *
pef_md5_calc(peffile_t *ppef, int ifd, char *span)
{
        int                     i;
        MD5_CTX                 c;
        unsigned char           md[MD5_DIGEST_LENGTH];
        static char             *p, buf[512];

	/*
	 * PEF file MD5 signatures are calculated from the section[0]
	 * contents (the code section), plus the unpacked section[1] contents
	 * (the unpacked data segment).
	 */

        MD5_Init(&c);

	md5_calc_piece(ppef->oldsHdr[0].containerOffset, 
			ppef->oldsHdr[0].containerOffset + ppef->oldsHdr[0].unpackedSize,
			(void *) ifd, (void *) &c, (void *)0);

	Log(4, "t4>> md5_calc unpacked data span [%#x bytes]\n", ppef->unpackedSize);

	MD5_Update(&c, (unsigned char *) ppef->unpackedBuf, (unsigned long) ppef->unpackedSize);

        MD5_Final(md, &c);

        for (i = 0, p = buf; i < MD5_DIGEST_LENGTH; i++, p += 2)
                sprintf(p, "%02x", md[i]);
        return buf;
}

static int
pef_resfork_update(char *resfile, int size0, int growth)
{
#ifdef macintosh
	int		err;
	int		rtn = -1;
	int		prevResFile;
	Handle		cfrgH = NULL;
	FSSpec		spec;
	short		refNo;
	char		*fx;

	prevResFile = CurResFile();

	fx = f_strdup(resfile);
	err = FSMakeFSSpec(0, 0, c2pstr(fx), &spec);
	if (err != 0) {
		Log(1, "t1>> Error: FSMakeFSSpec for file \"%s\" [%d]\n", resfile, err);
		free(fx);
		return -1;
	}

	refNo = FSpOpenResFile(&spec, 0);
	if ((err = ResError()) != 0) {
		Log(1, "t1>> Error: cannot open resource file \"%s\" [%d]\n", resfile, err);
		free(fx);
		return -1;
	}

	UseResFile(refNo);
	if ((err = ResError()) != 0) {
		Log(1, "t1>> Error: cannot use resource file \"%s\" [%d]\n", resfile, err);
		goto done;
	}

	cfrgH = Get1Resource('cfrg', 0);
	if (cfrgH == NULL) {
		Log(1, "t1>> Error: cannot get 'cfrg' resource [%d]\n", ResError());
		goto done;
	}

	pef_cfrg_update((char *) *cfrgH, size0, growth);

	ChangedResource(cfrgH);
	if ((err = ResError()) != 0) {
		Log(1, "t1>> Error while changing resource fork [%d]\n", err);
		goto done;
	}

	UpdateResFile(refNo);
	if ((err = ResError()) != 0) {
		Log(1, "t1>> Error while updating resource fork on disk [%d]\n", err);
		goto done;
	}

	Log(1, "t1>> Resource fork updated ok\n");
	rtn = 0;
done:
	if (cfrgH)
		ReleaseResource(cfrgH);
	CloseResFile(refNo);
	UseResFile(prevResFile);
	free(fx);
#endif
	return 0;
}


int
pef_is_packed(peffile_t *ppef)
{
	return (ppef->oldsHdr[1].sectionKind == PEF_SECTION_PIDATA);
}


int
pef_prepare(const char *srcfile, char *wkfile, peffile_t *ppef)
{
	int		flags;
	int		ifd, wkfd;
	int		size0, growth;
	int		rtn;

	/*
	 * If a temporary work file is going to be used, we only need
	 * read access to the srcfile at this stage.  However, we open
	 * it as read/write to check the write permission validity as
	 * early as possible.
	 */
	flags = options_nowrite()? OPENFL(O_RDONLY): OPENFL(O_RDWR);
	ifd = open(srcfile, flags, 0666);
	if (ifd == -1) {
		report(R_ERR, Msgs_get("MFOPENERR"), srcfile, syserr());
		return -1;
	}

#ifdef macintosh
	_ftype = 'APPL';
	_fcreator = 'MOSS';
#endif

	wkfd = open(wkfile, OPENFL(O_RDWR|O_CREAT|O_TRUNC), 0666);
	if (wkfd == -1) {
		report(R_ERR, Msgs_get("MFOPENERR"), wkfile, syserr());
		close(ifd);
		return -1;
	}

	rtn = pef_unload(ppef, ifd, wkfd, &size0, &growth);
	if (rtn == -1) {
		report(R_ERR, Msgs_get("MWKSETUP"), wkfile, syserr());
		close(ifd);
		close(wkfd);
		unlink(wkfile);
		return -1;
	}

	close(ifd);
	close(wkfd);

	rtn = cp(srcfile, wkfile, O_RSRC, NULL, (void *)0, 0);
	if (rtn == -1) {
		report(R_ERR, Msgs_get("MWKSETUP"), wkfile, syserr());
		unlink(wkfile);
		return -1;
	}

	rtn = pef_resfork_update(wkfile, size0, growth);
	if (rtn == -1) {
		report(R_ERR, Msgs_get("MWKSETUP"), wkfile, syserr());
		unlink(wkfile);
		return -1;
	}
	return 0;
}

static int
pef_copy(int ifd, int ofd, int nbytes)
{
	int		nb, nw;
	int		nr = 0;
	char		*p, buf[16*1024];

	Log(1, "t1>> pef_copy: ifd=%d ofd=%d nbytes=%d\n", ifd, ofd, nbytes);

	nb = min(nbytes, sizeof(buf));
	while (nb > 0 && (nr = read(ifd, buf, nb)) > 0) {
		for (p = buf; nr > 0;) {
			nw = write(ofd, p, nr);
			if (nw <= 0) {
				Log(1, "t1>> Error while copying to PEF file output [%s]\n", syserr());
				return -1;
			}
			p += nw;
			nr -= nw;
			nbytes -= nw;
		}
		nb = min(nbytes, sizeof(buf));
	}
	if (nb != 0 || nr < 0) {
		Log(1, "t1>> Error while copying from PEF file input (nb=%d, nr=%d) [%s]\n", nb, nr, syserr());
		return -1;
	}

	return 0;
}

static int
pef_container_size(int rawSize)
{
	if (rawSize & (PEF_ALIGN_CONTAINER - 1)) {
		rawSize += PEF_ALIGN_CONTAINER;
		rawSize &= ~(PEF_ALIGN_CONTAINER - 1);
	}
	return rawSize;
}

/*
 * Copy the data fork for an input file to output, and at the same time
 * merge the unpacked sections held in memory.
 * The function returns zero for success, -1 for an error.
 * The new size of PEF container 0 is returned via 'size0'.
 * The number of bytes by which the data fork grows during
 * the copy process is returned via 'growth'.
 */
int
pef_unload(peffile_t *ppef, int ifd, int ofd, int *size0, int *growth)
{
	int		inputPosn, outputPosn;
	int		inputsz;
	int		padSize;
	static char	padding[PEF_ALIGN_CONTAINER];
	struct stat	st;

	*size0 = *growth = 0;

	/* Adjust the new section 1 attributes. */
	ppef->newsHdr[1].packedSize = ppef->newsHdr[1].unpackedSize;
	ppef->newsHdr[1].sectionKind = PEF_SECTION_UNPACKED;
	/* assert((ppef->newsHdr[1].containerOffset & (PEF_ALIGN_UNPACKED - 1)) == 0); */

	/* Rewind the input and output files. */
	zlseek(ifd, (off_t)0, "pef_unload input rewind");
	zlseek(ofd, (off_t)0, "pef_unload output rewind");

	if (fstat(ifd, &st) == -1) {
		Log(1, "t1>> Error: Cannot get size of input file [%s]\n", syserr());
		return -1;
	}
	inputsz = st.st_size;

	Log(1, "t1>> pef_unload: inputsz = %d\n", inputsz);

	/*
	 * Write the new container header, followed by the new section Headers.
	 */
	if (write(ofd, (char *) &ppef->cHdr, sizeof(ppef->cHdr)) != sizeof(ppef->cHdr)) {
		Log(1, "t1>> Error while writing PEF container header [%s]\n", syserr());
		return -1;
	}
	if (write(ofd, (char *) ppef->newsHdr, ppef->sHdrBytes) != ppef->sHdrBytes) {
		Log(1, "t1>> Error while writing PEF section headers [%s]\n", syserr());
		return -1;
	}
	zlseek(ifd, (off_t) (sizeof(ppef->cHdr) + ppef->sHdrBytes), "pef_unload seek 1");
	inputPosn = sizeof(ppef->cHdr) + ppef->sHdrBytes;

	Log(1, "t1>> pef_unload: cHdr and sHdrs written; inputPosn = 0x%x\n", inputPosn);

	/*
	 * Copy the container data that lies between the section
	 * headers and the beginning of the section 1 data block.
	 */
	if (pef_copy(ifd, ofd, ppef->oldsHdr[1].containerOffset - inputPosn))
		return -1;
	inputPosn = ppef->oldsHdr[1].containerOffset;

	Log(1, "t1>> pef_unload: sec 0,2 data blocks written; inputPosn = 0x%x\n", inputPosn);

	/*
	 * Write the container section 1 data in unpacked form from memory.
	 * Then skip past the corresponding data in the input file.
	 */
	if (write(ofd, ppef->unpackedBuf, ppef->unpackedSize) != ppef->unpackedSize) {
		Log(1, "t1>> Error while writing PEF unpacked data [%s]\n", syserr());
		return -1;
	}

	inputPosn += ppef->packedSize;
	zlseek(ifd, (off_t) inputPosn, "pef_unload seek 2");

	Log(1, "t1>> pef_unload: sec 1 unpacked data; inputPosn = 0x%x\n", inputPosn);

	/*
	 * Pad the output container to the correct alignment.
	 */
	outputPosn = ppef->newsHdr[1].containerOffset + ppef->unpackedSize;
	*size0 = outputPosn;
	padSize = pef_container_size(outputPosn) - outputPosn;
	if (padSize > 0) {
		Log(1, "t1>> Writing %d pad bytes to 0x%x\n", padSize, outputPosn);
		if (write(ofd, padding, padSize) != padSize) {
			Log(0, "t0>> Error while writing container padding: %d bytes [%s]\n", padSize, syserr());
			return -1;
		}
		outputPosn += padSize;
	}

	Log(1, "t1>> pef_unload: padding written\n");

	/*
	 * Now copy any remaining containers across from the input file.
	 * 'growth' is a measure of the size by which container 0 has
	 * increased.  This figure is used to adjust all the container
	 * offsets that are stored in the 'cfrg' resource.
	 */
	inputPosn = pef_container_size(inputPosn);
	if (inputsz > inputPosn) {
		zlseek(ifd, (off_t) inputPosn, "pef_unload seek 3");
		*growth = outputPosn - inputPosn;
		assert(*growth >= 0);
		Log(1, "t1>> pef_unload: growth = %d; inputPosn = 0x%x\n", *growth, inputPosn);

		if (pef_copy(ifd, ofd, inputsz - inputPosn) == -1)
			return -1;
	}

	return 0;
}

int
pef_pidump(peffile_t *ppef, int fd)
{
	int		n = 0;
	int		len;
	char		*ptr;

	if (ppef && ppef->unpackedBuf) {
		ptr = ppef->unpackedBuf;
		len = ppef->unpackedSize;
		while (len > 0 && (n = write(fd, ptr, len)) > 0) {
			ptr += n;
			len -= n;
		}
	}
	return n;
}

void
pef_free(void *vp)
{
	peffile_t *ppef = (peffile_t *) vp;

	if (ppef) {
		if (ppef->oldsHdr)
			free((char *) ppef->oldsHdr);
		if (ppef->newsHdr)
			free((char *) ppef->newsHdr);
		if (ppef->packedBuf)
			free(ppef->packedBuf);
		if (ppef->unpackedBuf)
			free(ppef->unpackedBuf);

		ppef->oldsHdr = ppef->newsHdr = NULL;
		ppef->packedSize = ppef->unpackedSize = 0;
		ppef->packedBuf = ppef->unpackedBuf = NULL;
	}
}
