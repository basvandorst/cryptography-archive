/*
 * The author of this software is Matt Blaze.
 *              Copyright (c) 1992, 1993, 1994 by AT&T.
 * Permission to use, copy, and modify this software without fee
 * is hereby granted, provided that this entire notice is included in
 * all copies of any software which is or includes a copy or
 * modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * This software is subject to United States export controls.  You may
 * not export it, in whole or in part, or cause or allow such export,
 * through act or omission, without prior authorization from the United
 * States government and written permission from AT&T.  In particular,
 * you may not make any part of this software available for general or
 * unrestricted distribution to others, nor may you disclose this software
 * to persons other than citizens and permanent residents of the United
 * States and Canada. 
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */

/*
 * cfs admin protocol RPC spec
 *  new version, supporting multiple crypt modes.
 */

/* error conditions */
enum cfsstat {
	CFS_OK=0,		/* ok */
	CFSERR_PERM=1,		/* permission denied */
	CFSERR_IFULL=2,		/* instance table full; */
	CFSERR_NOINS=3,		/* no such instance */
	CFSERR_EXIST=4,		/* name already in use */
	CFSERR_NODIR=5,		/* no such directory */
	CFSERR_BADKEY=6,	/* invalid key */
	CFSERR_BADNAME=7	/* badly formed name */
};

enum ciphers {
	CFS_STD_DES=0,		/* 2 key hybrid single DES */
	CFS_THREE_DES=1,	/* 2 key hybrid 3DES */
	CFS_IDEA=2,		/* 2 key hybrid IDEA */
	CFS_BLOWFISH=3,		/* 2 key hybrid BLOWFISH */
	CFS_SKIPJACK=4		/* 2 key hybrid SKIPJACK, (PCMCIA) */
};

const CFS_MAXCOMP=255;
const CFS_MAXNAME=1024;



struct cfs_adm_deskey {
	long pl;	/* for the align police */
	u_char primary[8];
	u_char secondary[8];
};

struct cfs_adm_3deskey {
	long pl;	/* for the align police */
	u_char primary1[8];
	u_char primary2[8];
	u_char secondary1[8];	/* same as primaries */
	u_char secondary2[8];
};

union cfs_admkey switch (ciphers cipher) {
    case CFS_STD_DES:
	cfs_adm_deskey deskey;
    case CFS_THREE_DES:
	cfs_adm_3deskey des3key;
    default:
	void;
};
struct cfs_attachargs {
	string dirname<CFS_MAXNAME>;	/* directory to attach to */
	string name<CFS_MAXCOMP>;	/* instance name */
	cfs_admkey key;			/* key to use */
	int uid;			/* uid to apply - need not be
					   same as in rpc */
	int highsec;			/* nonzero for highsec mode */
	bool anon;			/* anonymousness */
	int expire;			/* number of minutes to live */
};

struct cfs_detachargs {
	string name<CFS_MAXCOMP>;	/* instance name */
	int uid;			/* just has to match */
};


struct cfs_lsargs {
	int cookie;			/* enrty with which to start */
};

struct lsok {
	string dirname<CFS_MAXCOMP>;	/* name (or .OPAQUE_*) */
	int owner;			/* owner */
	int iid;			/* instance id */
	string where<CFS_MAXNAME>;	/* where to */
};

union cfs_lsrepl switch (cfsstat status) {
    case CFS_OK:
	struct lsok ok;
    default:
	void;
};


program ADM_PROGRAM {
	version ADM_VERSION {
		void ADMPROC_NULL(void) = 0;
		cfsstat ADMPROC_ATTACH(cfs_attachargs) = 1;
		cfsstat ADMPROC_DETACH(cfs_detachargs) = 2;
		cfs_lsrepl ADMPROC_LS(cfs_lsargs) = 3;
	} = 1;
} = 0x21234567;
