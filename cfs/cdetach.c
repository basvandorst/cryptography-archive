/*
 * The author of this software is Matt Blaze.
 *              Copyright (c) 1992, 1993, 1994 by AT&T.
 * Permission to use, copy, and modify this software without fee
 * is hereby granted, provided that this entire notice is included in
 * all copies of any software which is or includes a copy or
 * modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * This software is subject to United States export controls.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */

/*
 * client side detach
 */
#include <stdio.h>
#include <rpc/rpc.h>
#include "nfsproto.h"
#include "admproto.h"
#include "cfs.h"

main(argc,argv)
     int argc;
     char **argv;
{
	cfs_detachargs ap;
	char *pw;
	int status;
	cfsstat ret;
	
	if (argc!=2) {
		fprintf(stderr,"Usage: cdetach name\n");
		exit(1);
	}
	ap.name=argv[1];
	ap.uid=getuid();
	if ((status = callrpc("localhost",ADM_PROGRAM,ADM_VERSION,
			    ADMPROC_DETACH,xdr_cfs_detachargs,&ap,
			    xdr_cfsstat,&ret)) !=0) {
		clnt_perrno(status);
		exit(1);
	}
	if (ret!=CFS_OK)
		fprintf(stderr,"cdetach: %s\n",admmsg(ret));
	exit(ret);
}
