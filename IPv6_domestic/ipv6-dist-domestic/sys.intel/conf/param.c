/*	BSDI param.c,v 2.1 1995/02/03 07:22:14 polk Exp	*/

/*
 * Copyright (c) 1980, 1986, 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)param.c	8.2 (Berkeley) 1/21/94
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/socket.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#include <sys/file.h>
#include <sys/callout.h>
#include <sys/mbuf.h>
#include <sys/kernel.h>

#include <ufs/ufs/quota.h>

#include <machine/vmparam.h>

/*
 * System parameter formulae and other configuration-dependent information.
 *
 * This file is copied into each directory where we compile
 * the kernel; it should be modified there to suit local taste
 * if necessary.
 *
 * Compiled with -DHZ=xx -DTIMEZONE=x -DDST=x -DMAXUSERS=xx
 */

#ifndef HZ
#define	HZ 100
#endif
#ifndef TIMEZONE
#define TIMEZONE 0
#endif
#ifndef DST
#define DST 0
#endif
int	hz = HZ;
int	tick = 1000000 / HZ;
int	tickadj = 30000 / (60 * HZ);		/* can adjust 30ms in 60s */
struct	timezone tz = { TIMEZONE, DST };
#define	NPROC (20 + 16 * MAXUSERS)
int	maxproc = NPROC;
#define	NTEXT (80 + NPROC / 8)			/* actually the object cache */
#define	NVNODE (NPROC * 2 + NTEXT + 100)
int	desiredvnodes = NVNODE;
int	maxfiles = 3 * (NPROC + MAXUSERS) + 80;
int	ncallout = 16 + NPROC;
int	child_max = CHILD_MAX;	/* default cur limit, processes per user */
int	open_max = OPEN_MAX;	/* default cur limit, open files per user */
int	cmask = CMASK;
#ifdef INSECURE
int	securelevel = -1;	/* do not go to secure mode automatically */
#endif

int	dfldsiz = DFLDSIZ;
int	maxdsiz = MAXDSIZ;
int	dflssiz = DFLSSIZ;
int	maxssiz = MAXSSIZ;

#ifndef SOMAXCONN
#define	SOMAXCONN	SOMAXCONN_DFLT
#endif
int	somaxconn = SOMAXCONN;
int	nmbclusters = NMBCLUSTERS;
int	fscale = FSCALE;	/* kernel uses `FSCALE', user uses `fscale' */

#ifndef BUFMEM
#define	BUFMEM	0		/* dynamically chosen based on physmem */
#endif
int	maxbufmem = BUFMEM;	/* maxbufmem in bytes */

#ifndef KMEMSIZE
#ifndef NKMEMCLUSTERS	/* compat with old systems, options NKMEMCLUSTERS */
#if MAXUSERS > 128
#define	NKMEMCLUSTERS	(8192*1024/CLBYTES)
#else
#if MAXUSERS >= 64
#define	NKMEMCLUSTERS	(4096*1024/CLBYTES)
#else
#define	NKMEMCLUSTERS	(2048*1024/CLBYTES)
#endif
#endif
#endif
#define	KMEMSIZE	(NKMEMCLUSTERS * CLBYTES)
#endif

int	kmemsize = KMEMSIZE;	/* basic malloc arena size */

#define	MAX_KMAPENTRIES	500
#ifndef KMAPENTRIES
#if KMEMSIZE > 2048*1024
#define	KMAPENTRIES	(MAX_KMAPENTRIES * (KMEMSIZE / 1024) / 2048)
#else
#define	KMAPENTRIES	MAX_KMAPENTRIES
#endif
#endif
int	max_kmapentries = KMAPENTRIES;

/*
 * These are initialized at bootstrap time
 * to values dependent on memory size
 */
int	nswbuf;

/*
 * These have to be allocated somewhere; allocating
 * them here forces loader errors if this file is omitted
 * (if they've been externed everywhere else; hah!).
 */
struct 	callout *callout;
struct	buf *swbuf;

/*
 * Proc/pgrp hashing.
 * Here so that hash table sizes can depend on MAXUSERS/NPROC.
 * Hash size must be a power of two.
 * NOW omission of this file will cause loader errors!
 */

#if NPROC > 1024
#define	PIDHSZ		512
#else
#if NPROC > 512
#define	PIDHSZ		256
#else
#if NPROC > 256
#define	PIDHSZ		128
#else
#define	PIDHSZ		64
#endif
#endif
#endif

struct	proc *pidhash[PIDHSZ];
struct	pgrp *pgrphash[PIDHSZ];
int	pidhashmask = PIDHSZ - 1;
#ifdef FORK_WAIT
int	fork_wait = 3;		/* delay child 3 ticks after fork */
#else
int	fork_wait = 0;		/* do not delay child after fork */
#endif


#ifdef NFS
#ifdef NFS_SERV_ASYNC
int nfs_serv_async = 1;
#else
int nfs_serv_async = 0;
#endif
#endif /* NFS */

#include "bpfilter.h"
#if NBPFILTER == 0
/*
 * dummy bpf routines to satisfy driver references
 * if bpf is not configured.
 */

void
bpfattach(driverp, ifp, dlt, hdrlen)
	caddr_t *driverp;
	struct ifnet *ifp;
	u_int dlt, hdrlen;
{
}

void
bpf_tap(arg, pkt, pktlen)
	caddr_t arg;
	u_char *pkt;
	u_int pktlen;
{
}

void
bpf_mtap(arg, m)
	caddr_t arg;
	struct mbuf *m;
{
}
#endif /* NBPFILTER == 0 */
