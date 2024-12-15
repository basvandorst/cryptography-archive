/*
 # $Id: os2lx.h,v 1.2 1998/04/10 10:25:03 fbm Exp fbm $
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

#ifndef OS2LX_H
#define OS2LX_H

/* OS/2 NE and LX Header structures and structures of */
/* all fixed size table entry types. */

/* Dummy DOS struct that points to the file offset of the NE/LX exe header. */
/* This ignores MZ header items other than ID word and lfanew. */
/* This struct is at file offset 0. */

typedef struct {
	unsigned short		magic;		/* MUST BE ASCII "MZ" */
	char			padding[58];
	unsigned long		lfanew;		/* File offset of new exe header. */
} SIMPLE_MZ_EXE;


/* Structure that defines the LX exe header. */
typedef struct {
	unsigned short		Magic;		/* "LX" */
	unsigned char		ByteOrder;	/* LITTLE_ENDIAN or BIG_ENDIAN */
	unsigned char		WordOrder;	/* LITTLE_ENDIAN or BIG_ENDIAN */
	unsigned long		FormatLevel;	/* Loader format level, currently 0 */
	unsigned short		CpuType;	/* 286 through Pentium+ */
	unsigned short		OSType;		/* DOS, Win, OS/2 ... */
	unsigned long		ModVersion;	/* Version of this exe */
	unsigned long		ModFlags;	/* Program/Library ... */
	unsigned long		ModNumPgs;	/* Number of non-zero-fill or invalid pages */
	unsigned long		EIPObjNum;	/* Initial code object */
	unsigned long		EIP;		/* Start address within EIPObjNum */
	unsigned long		ESPObjNum;	/* Initial stack object */
	unsigned long		Esp;		/* Top of stack within ESPObjNum */
	unsigned long		PgSize;		/* Page size, fixed at 4k  */
	unsigned long		PgOfsShift;	/* Page alignment shift */
	unsigned long		FixupSectionSize;	/* Size of fixup information in file */
	unsigned long		FixupCksum;	/* Checksum of FixupSection */
	unsigned long		LoaderSecSize;	/* Size of Loader Section */
 	unsigned long		LoaderSecCksum;	/* Loader Section checksum */
	unsigned long		ObjTblOfs;	/* File offset of Object Table */
	unsigned long		NumObjects;	/* Number of Objects */
	unsigned long		ObjPgTblOfs;	/* File offset of Object Page Table */
	unsigned long		ObjIterPgsOfs;	/* File offset of Iterated Data Pages */
	unsigned long		RscTblOfs;	/* File offset of Resource Table */
	unsigned long		NumRscTblEnt;	/* # of entries in Resource Table */
        unsigned long		ResNameTblOfs;	/* File offset of Resident Name Table */
        unsigned long		EntryTblOfs;	/* File offset of Entry Table */
	unsigned long		ModDirOfs;	/* File offset of Module Directives */
	unsigned long		NumModDirs;	/* Number of Module Directives */
	unsigned long		FixupPgTblOfs;	/* File offset of Fixup Page Table */
	unsigned long		FixupRecTblOfs;	/* File offset of Fixup Record Table */
	unsigned long		ImpModTblOfs;	/* File offset of Imp Module Table */
	unsigned long		NumImpModEnt;	/* Number of Imported Modules */
	unsigned long		ImpProcTblOfs;	/* File offset of Imported Proc Table */
	unsigned long		PerPgCksumOfs;	/* File offset of Per-Page Checksum Table */
	unsigned long		DataPgOfs;	/* File offset of Data Pages */
	unsigned long		NumPreloadPg;	/* Number of Preload Pages */
	unsigned long		NResNameTblOfs;	/* File offset of Non Resident  */
						/* Name Table from beginning of file! */
	unsigned long		NResNameTblLen;	/* Length in bytes of Non Resident  */
						/* Name Table; table is also NULL terminated. */
	unsigned long		NResNameTblCksum;	/* Non Resident Name Table checksum */
	unsigned long		AutoDSObj;	/* Object number of auto data */
	unsigned long		DebugInfoOfs;	/* File offset of debugging info */
	unsigned long		DebugInfoLen;	/* Length of Debugging Info */
	unsigned long		NumInstPreload;	/* Number of instance-preload pages */
	unsigned long		NumInstDemand;	/* Number of instance-demand pages */
	unsigned long		HeapSize;	/* Heap size */
	/* unsigned long	StackSize; */	/* Stack size */
} LX_EXE;


/* An entry in the LX object table */
typedef struct {
	unsigned long		size;		/* Load-time size of object */
	unsigned long		reloc_base_addr;/* Address the object wants  */
						/* to be loaded at. */
	unsigned long		obj_flags;	/* Read/Write/Execute, Resource, Zero-fill ... */
	unsigned long		pg_tbl_index;	/* Index in Object Page Table at  */
						/* which this objects first page  */
						/* is located. */
	unsigned long		num_pg_tbl_entries;	/* Number of consecutive Object  */
						/* Page Table entries that belong to  */
						/* this object. */
	unsigned long		reserved;
} LX_OBJ;


/* An entry in the LX Object Page Table. */
typedef struct {
	unsigned long		offset;		/* File offset of this pages data. */
						/* Relative to beginning of Iterated */
						/* or Preload Pages  */
	unsigned short		size;		/* Size of this page.  <= 4096 */
	unsigned short		flags;		/* Iterated, Zero-filled, Invalid ... */
} LX_PG;


/* An entry in the LX Resource Table */
typedef struct {
	unsigned short		type_id;	/* one of rsc_types */
	unsigned short		name_id;	/* ID application uses to load this resource */
	unsigned long		size;		/* size of the resource */
	unsigned short		object;		/* which object is this resource located in? */
	/*unsigned long		offset;		// resource offset within the object  */
	unsigned char		offlow;		/* least-sig byte of offset. */
} LX_RSC;


/* A structure to pull everything together into one piece. */
typedef struct {
	SIMPLE_MZ_EXE		mz;
	LX_EXE			lx;
	struct lxObject		**lxObjects;
	int			nlxObjects;
	struct lxPage          	**pages;
	int              	npages;
} lxfile_t;

#ifdef __cplusplus
extern "C" {
#endif

#if 0
int		lxFile_compressed(lxfile_t *lx, char *md5_span);
#endif
char		*lxFile_md5_calc(lxfile_t *lx, int fd, char *md5_span, int *err);
lxfile_t	*lxFile_is_lx(int fd);
void		lxFile_free(void *vp);
unsigned long	lxFile_get_offset(lxfile_t *lx, unsigned long addr);
void		lxFile_print_hdr(lxfile_t *lx);

#ifdef __cplusplus
}
#endif

#endif
