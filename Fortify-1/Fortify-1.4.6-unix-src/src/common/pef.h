/*
 # $Id: pef.h,v 1.2 1998/04/10 10:29:58 fbm Exp fbm $
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

#ifndef PEF_H
#define PEF_H

#define PEF_SECTION_CODE	0
#define PEF_SECTION_UNPACKED	1
#define PEF_SECTION_PIDATA	2
#define PEF_SECTION_CONST	3
#define PEF_SECTION_LOADER	4
#define PEF_SECTION_EXEC	6

typedef unsigned char	uint8;
typedef unsigned short	uint16;
typedef unsigned int	uint32;
typedef int		sint32;

typedef struct {
	uint32			tag1;
	uint32			tag2;
	uint32			arch;
	uint32			formatVersion;
	uint32			dateTimeStamp;
	uint32			oldDefVersion;
	uint32			oldImpVersion;
	uint32			currentVersion;
	uint16			sectionCount;
	uint16			instSectionCount;
	uint32			resA;
} PEFContainerHdr;

typedef struct {
	sint32			nameOffset;
	uint32			defaultAddress;
	uint32			totalSize;
	uint32			unpackedSize;
	uint32			packedSize;
	uint32			containerOffset;
	uint8			sectionKind;
	uint8			shareKind;
	uint8			alignment;
	uint8			resA;
} PEFSectionHdr;

typedef struct {
	uint32			resA;
	uint32			resB;
	uint32			version;
	uint32			resC;
	uint32			resD;
	uint32			resE;
	uint32			resF;
	uint32			nFrags;
} cfrgResHdr_t;

typedef struct {
	uint32			codeType;
	uint32			updateLevel;
	uint32			currentVersion;
	uint32			oldVersion;
	uint32			stackSize;
	uint16			libDir;
	uint8			fragType;
	uint8			fragLocn;
	uint32			fragOffset;
	uint32			fragSize;
	uint32			resA;
	uint32			resB;
	uint16			descSize;
} cfrgDesc_t;

typedef struct {
	PEFContainerHdr		cHdr;
	PEFSectionHdr		*oldsHdr;
	PEFSectionHdr		*newsHdr;
	int			sHdrBytes;

	char			*packedBuf;
	int			packedSize;
	char			*unpackedBuf;
	int			unpackedSize;
} peffile_t;


#ifdef __cplusplus
extern "C" {
#endif

peffile_t	*pef_is_peffile(int ifd);
void		pef_print_hdrs(peffile_t *ppef);
void		pef_cfrg_dumpDesc(int i, cfrgDesc_t *dp);
int		pef_is_packed(peffile_t *ppef);
int		pef_prepare(const char *target, char *wkfile, peffile_t *ppef);
char *		pef_md5_calc(peffile_t *ppef, int ifd, char *span);
int		pef_unload(peffile_t *ppef, int ifd, int ofd, int *size0, int *growth);
int		pef_pidump(peffile_t *ppef, int fd);
void		pef_free(void *vp);

#ifdef __cplusplus
}
#endif

#endif

