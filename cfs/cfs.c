/*
 * The author of this software is Matt Blaze.
 *              Copyright (c) 1992, 1993, 1994, 1997 by AT&T.
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
 * Crypto file system
 *  main 1.4
 * mab - 11/30/92
 * mab - 01/11/94
 * mab - 11/10/94
 * mab - 11/97
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/socket.h>
/* #include <netinet/in.h> */
/* #include <arpa/inet.h> */
#include <netdb.h>
#include <rpc/rpc.h>
#include <sys/time.h>
#ifndef NORLIMITS
#include <sys/resource.h>
#endif
#include <signal.h>
#include "nfsproto.h"
#include "admproto.h"
#include "cfs.h"

struct in_addr validhost;

#if defined(SOLARIS2X) || defined(__NetBSD__)
void nfs_program_2();
void adm_program_2();
#include <string.h>
#else
int nfs_program_2();
int adm_program_2();
#endif
#ifdef __NetBSD__
int _rpcsvcdirty;
#endif

void grimreap();

char zerovect[]={0,0,0,0,0,0,0,0,0};
int cursecs;

main(argc,argv)
     int argc;
     char **argv;
{
	int port=CFS_PORT;

	struct timeval tv;
	struct hostent *hp;
	struct sockaddr_in sin;
	int svrsock;
	SVCXPRT *tp;
	int pid;
#ifdef SOLARIS2X
	struct netconfig *nc;
	struct t_bind tbind, *tres;
	struct t_info tinfo;
#endif

	/* create the right kind of socket */
	if (argc > 2) {
		fprintf(stderr,"Usage: cfsd [port]\n");
		exit(1);
	}
	if (argc==2) {
		if ((port=atoi(argv[1]))<=0) {
			fprintf(stderr,"Usage: cfsd [port]\n");
			exit(1);
		}
	}
	if ((hp=gethostbyname("localhost"))==NULL) {
		fprintf(stderr,"Can't deal with localhost\n");
		exit(1);
	}
	bzero((char *)&sin,sizeof(sin));
	sin.sin_family=AF_INET;
	bcopy((char *)hp->h_addr,(char *)&sin.sin_addr,hp->h_length);
	/* sin.sin_addr = inet_makeaddr(INADDR_ANY,np);*/
	validhost.s_addr=sin.sin_addr.s_addr;
	sin.sin_port = htons(port);

#ifdef SOLARIS2X
	if ((nc = getnetconfigent("udp")) == NULL) {
		nc_perror("udp");
		exit(1);
	}

	if ((svrsock = t_open(nc->nc_device, O_RDWR, &tinfo)) < 0) {
		t_error("Can't t_open UDP device");
		exit(1);
	}

	if ((tres = (struct t_bind *)t_alloc(svrsock, T_BIND, T_ADDR)) == NULL){
		t_error("Can't t_alloc buffer");
		exit(1);
	}

	tbind.qlen = 0;
	tbind.addr.buf = (char *)&sin;
	tbind.addr.len = tbind.addr.maxlen = tinfo.addr;

	if (t_bind(svrsock, &tbind, tres) != 0) {
		t_error("t_bind");
		exit(1);
	}

	if (tbind.addr.len != tres->addr.len ||
	    memcmp(tbind.addr.buf, tres->addr.buf, tres->addr.len) != 0) {
		/* bound address does not match requested one */
		fprintf(stderr,
"t_bind did not bind to requested address (is another cfsd running?)\n");
		exit(1);
	}

	if ((tp = svc_dg_create(svrsock, 0, 0)) == NULL) {
		fprintf(stderr,"Can't create UDP RPC Service\n");
		exit(1);
	}

	/* Assign the local bind address and type of service */
	tp->xp_ltaddr = tres->addr;
	tp->xp_type = tinfo.servtype;
	tp->xp_rtaddr.len = 0;
	tp->xp_rtaddr.maxlen = tres->addr.maxlen;
	tp->xp_netid = strdup(nc->nc_netid);
	tp->xp_tp = strdup(nc->nc_device);

	if ((tp->xp_rtaddr.buf = malloc(tp->xp_rtaddr.maxlen)) == NULL) {
		fprintf(stderr, "Can't malloc buffer space\n");
		exit(1);
	}

	/* now register w/ the local dispatcher */
	/*  don't register nfs w/ portmaper, tho */
	if (!svc_reg(tp, NFS_PROGRAM, NFS_VERSION, nfs_program_2,
			(port==2049? nc : NULL))) {
		fprintf(stderr,"Can't register CFS NFS\n");
		exit(1);
	}
	if (!rpcb_unset(ADM_PROGRAM, ADM_VERSION, nc)) {
		fprintf(stderr, "Can't init CFS ADM rpcbind mapping\n");
		exit(1);
	}
	if (!svc_reg(tp, ADM_PROGRAM, ADM_VERSION, adm_program_2, nc)) {
		fprintf(stderr,"Can't register CFS ADM\n");
		exit(1);
	}
#else
	if ((svrsock=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
		perror("socket");
		exit(1);
	}

	if (bind(svrsock,(struct sockaddr *)&sin,sizeof(sin)) != 0) {
		perror("bind");
		exit(1);
	}

	if ((tp = svcudp_create(svrsock)) == NULL) {
		fprintf(stderr,"Can't create UDP RPC Service\n");
		exit(1);
	}
	
	/* now register w/ the local dispatcher */
	/* don't register nfs w/ portmaper, tho, in case */
	/* a real nfsd is already running */
	if (!svc_register(tp,NFS_PROGRAM,NFS_VERSION,nfs_program_2,
			(port==2049?IPPROTO_UDP:0))) {
		fprintf(stderr,"Can't register CFS NFS\n");
		exit(1);
	}
	pmap_unset(ADM_PROGRAM,ADM_VERSION);
	if (!svc_register(tp,ADM_PROGRAM,ADM_VERSION,adm_program_2,
			  IPPROTO_UDP)) {
		fprintf(stderr,"Can't register CFS ADM\n");
		exit(1);
	}
#endif

#ifndef	DEBUG
	if ((pid=fork())!=0) {
		if (pid<0) {
			perror("cfsd: fork\n");
			exit(1);
		}
		fprintf(stderr,"cfs ready\n",pid);
		printf("%d\n",pid);
		exit(0);
	}
#else
	printf("cfs running DEBUG (%d)\n", getpid());
#endif
	initstuff();
	/* and lauch the timeout handler (which we have to do in child) */
	gettimeofday(&tv,NULL);
	cursecs=tv.tv_sec;
	signal(SIGALRM,grimreap);
	alarm(60); /* every 60 secs */

	svc_run(); /* do it */
	fprintf(stderr,"Huh??  Where the hell am I?\n");
	exit(1);
}

initstuff()
{
	int i;
	static instance ina,inb;
#ifndef NORLIMITS
	struct rlimit rl;
#endif
	
	/* first set uid to 0, if we can */
	/* now we can go back and forth easily */
 	setuid(0);
	umask(0);

#if defined(__NetBSD__) || defined(__bsdi__)
#ifndef DEBUG
	/* detach from terminal */
	daemon(0,0);
#endif /* DEBUG */
#endif /* __NetBSD__ */

#ifndef NORLIMITS
	/* try to make sure we don't spill a corefile */
	rl.rlim_cur=0;
	rl.rlim_max=0;
	setrlimit(RLIMIT_CORE,&rl);
#else	
	/* set signal handlers */
	/* for things that can dump core */
	signal(SIGQUIT,SIG_IGN);
	signal(SIGILL,SIG_IGN);
	signal(SIGTRAP,SIG_IGN);
	signal(SIGABRT,SIG_IGN);
	signal(SIGEMT,SIG_IGN);
	signal(SIGFPE,SIG_IGN);
	signal(SIGBUS,SIG_IGN);
	signal(SIGSEGV,SIG_IGN);
	signal(SIGSYS,SIG_IGN);
#ifdef SIGLOST
	signal(SIGLOST,SIG_IGN);
#endif
#endif
	/* clear out the instances table */
	for (i=0; i<NINSTANCES; i++)
		instances[i]=NULL;
}

/* look for instances to kill */
void grimreap()
{
	struct timeval tv;
	int i;

	gettimeofday(&tv,NULL);
	cursecs=tv.tv_sec;
#ifdef DEBUG
	fprintf(stderr,"grimly reaping\n");
#endif
	for (i=0; i<NINSTANCES; i++)
		if (instances[i] != NULL) {
			if (instances[i]->dead > 4)
				freeinstance(i);
			else if ((instances[i]->timeout)
			    && (instances[i]->timeout < cursecs))
					instances[i]->dead++;
			else if ((instances[i]->idle)
			    && ((instances[i]->access + instances[i]->idle)
				< cursecs))
					instances[i]->dead++;
		}
	signal(SIGALRM,grimreap);
	alarm(60);
}
