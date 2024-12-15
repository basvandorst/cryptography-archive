/*
 *  SecuDE Release 4.3 (GMD)
 */
/********************************************************************
 * Copyright (C) 1994, GMD. All rights reserved.                    *
 *                                                                  *
 *                                                                  *
 *                         NOTICE                                   *
 *                                                                  *
 *    Acquisition, use, and distribution of this module             *
 *    and related materials are subject to restrictions             *
 *    mentioned in each volume of the documentation.                *
 *                                                                  *
 ********************************************************************/

/*---------------------------sctd.c---------------------------------*/
/*------------------------------------------------------------------*/
/* GMD Darmstadt Institute for System Technic (F2.G3)               */
/* Rheinstr. 75 / Dolivostr. 15                                     */
/* 6100 Darmstadt                                                   */
/* Project ``Secure DFN'' 1990 / "SecuDe" 1991                      */
/* Grimm/Nausester/Schneider/Viebeg/Vollmer et alii                 */
/*------------------------------------------------------------------*/
/*                                                                  */
/* PACKAGE   util            VERSION   1.0                          */
/*                              DATE   01.10.1993                   */
/*                                BY   rdn                          */
/*                                                                  */
/*                            REVIEW                                */
/*                              DATE                                */
/*                                BY                                */
/* DESCRIPTION                                                      */
/*   This is a program to run the SCT driver for network access     */
/*   of the SmartCardTerminal                                       */
/*                                                                  */
/* USAGE:                                                           */
/*     sctd [-l logfile] [-p port] /dev/<term>                      */
/*              <baud>,<stop>,<data>[,parity]                       */
/*                                                                  */
/* WHERE:                                                           */
/*     logfile	- the filename where to write the log               */
/*                (reassigns stderr, requires TRACE option)         */
/*        port  - the TCP port to use                               */
/*        term  - the terminal device (e.g. ttya)                   */
/*        baud  - serial lines baud rate                            */
/*        stop  - number of stopbits (1|2)                          */
/*        data  - number of databits                                */
/*      parity  - serial lines parity (even|odd|none)               */
/*                                                                  */
/*------------------------------------------------------------------*/


#if defined(SUN) || defined(__HP__)
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <termios.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

#include "sca.h"
#include "sctport.h"

/* these defines allow linking of t1.o without too much overhead overhead ! */
#ifdef __MFCHECK
#undef malloc
#undef free
char	*proc = "sctd-main";

#ifdef __STDC__
char*	aux_malloc( char *proc, int len )
#else
char*	aux_malloc( /*char **/proc, /*int */len )
char *proc; int len;
#endif
{	return malloc(len);	}

#ifdef __STDC__
void	aux_free( char *proc, void *p )
#else
void	aux_free( /*char **/proc, /*void **/p )
char *proc; void *p;
#endif
{	free(p);	}

#endif

#ifdef __STDC__
#define _ARGS(parm) parm
#else
#define _ARGS(parm) ()
#endif


static	void connserve _ARGS(( struct s_portparam *port, int socket, struct sockaddr* addr));
static	void getargs _ARGS((u_short* port, struct s_portparam* serial, int, char**));
static	void usage _ARGS((char*));
static	void sigterm()
{
	exit(SIGTERM);
}

#ifdef __STDC__
main(int argc, char **argv)
#else
main(argc, argv)
int argc; char **argv;
#endif
{
static	struct s_portparam	serial;
int	s;			/* socket */
struct	sockaddr_in in_addr;	/* bind addr */
struct	linger	so_linger;	/* socketopt */
int	so_linger_sz;
fd_set	io_ports;		/* io channels */
int	select_val;		/* select value */
int	conn;			/* connected socket */
struct	sockaddr conn_addr;	/* connected addr */
int	len_addr = sizeof(conn_addr);
void	(*func)() = sigterm;

	getargs(&in_addr.sin_port, &serial, argc, argv);
	if (COMinit(&serial) == -1) {
		fprintf(stderr,"COMinit error\n");
		exit(1);
	}
	(void)signal(SIGTERM,func);
	(void)signal(SIGINT,func);

/* start service loop */
    s = 0;
    while(1) {

    if (s) /* use socket */ ;
    else {
	if ((s = socket( AF_INET, SOCK_STREAM, 0)) < 0) {
		perror( "socket()" );
		break;
	}

	/* set socket options, if any */
	so_linger_sz = sizeof(so_linger);
	if (getsockopt(s, SOL_SOCKET, SO_LINGER,
		&so_linger, &so_linger_sz)<0)
		perror("getsockopt");

	bzero( &in_addr, sizeof(in_addr));
	in_addr.sin_family = AF_INET;
	in_addr.sin_port = 3210;		/*** TEST ***/
	if ( bind( s, &in_addr, sizeof(in_addr) ) < 0) {
		perror( "bind()" );
		close(s);
		break;
	}
	listen(s,1);		/* allow one only */

	if ( fcntl( s, F_SETFL, FNDELAY ) < 0 ) {
		perror( "fcntl(FNDELAY)" );
		close(s);
		break;
	}
    }

	FD_ZERO(&io_ports);
	FD_SET(s,&io_ports);
	select_val = select( s+1, &io_ports, (fd_set*)0, (fd_set*)0,
				(struct timeval*)0 );
	if (select_val < 0 && errno != EINTR)	break;
	if (select_val > 0) {
	/* connect indication event */
		conn = accept(s, &conn_addr, &len_addr);
		close(s);	/* refuse other connections */
		s = 0;

		if (conn < 0) {
			if (errno != EWOULDBLOCK)	break;
		}
		else {
			connserve(&serial, conn, &conn_addr);
			COMreset(&serial);
		}
	}

    }	/* while(1) */

/* break occured */
	COMclose(serial.port_id);
}


/* connserve - start relay COM interface */
static void connserve(
#ifdef __STDC__
struct s_portparam *portpar,
int s,
struct	sockaddr *addr
) 
#else
portpar, s, addr )
struct s_portparam *portpar;
int s;
struct	sockaddr *addr;
#endif
{
char	apdu[256], resp[256], *out, ret;
int	n, w;
struct	iovec	io[2];
extern	int	tp1_err;

/* init io vector */
	io[0].iov_base = &ret;
	io[0].iov_len = 1;
	io[1].iov_base = resp;
	
	/* set socket options, if any */

	/* disable non blocking IO */
	fcntl( s, F_SETFL, 0 );

	for(w = 1; w > 0; ) {
		n = read(s, apdu, sizeof(apdu));
		if (n==0)	break;	/* done with it */
		if (n<0 && errno==EINTR) continue;

		/* relay request */
		/* init length cnt */
		io[1].iov_len = 0;
		if (COMtrans(portpar, apdu, n, resp, &io[1].iov_len) == 0) {
		/* well! */
			ret = 0;
			w = writev(s, io, 2);
		}
		else {
			ret = tp1_err;
			w = write(s, &ret, 1);
		}
	}
	close(s);
}

static	void getargs(
#ifdef __STDC__
u_short* portid,
struct s_portparam* portpar,
int	n,
char**	argvec
)
#else
portid, portpar, n, argvec )
u_short* portid;
struct s_portparam* portpar;
int	n;
char**	argvec;
#endif
{
extern	char*	optarg;
extern	int	optind, opterr;
char	opt;
int	errflag;
char	*logfile;
char	*port;
char	*device;
char	*conf, *tok;
int	baud, stopb, datab, parity;
int	idx_baud, chk_baud;

	logfile = port = device = conf = NULL;
	baud = 19200, stopb = 0, datab = CS8, parity = 0;
	*portid = 3210;	/*** TEST ***/

	errflag = opterr = 0;
	while( (opt = getopt(n,argvec,"l:p:")) != -1 ) {
		switch(opt) {
		case 'p': port = optarg;
			break;
		case 'l': logfile = optarg;
			break;
		default: errflag = 1;
		}
	}

	if (errflag || (n - optind) != 2) {
		usage(argvec[0]);
	}

	if (logfile) {
	FILE*	fp = freopen(logfile,"w", stderr);
		if (!fp) {
			perror("logfile");
			exit(1);
		}
	}

	if(port) *portid = atoi(port);
	device = argvec[optind++];
	conf = argvec[optind++];

    do {
	tok = strtok(conf,",");
	if (tok)	baud = atoi(tok);
	else break;
	tok = strtok(NULL,",");
	if (tok) if (strcmp(tok,"1"))	/* ~1 */
		if (strcmp(tok,"2"))	/* ~2 */
			usage(argvec[0]);
		else	stopb = CSTOPB;
		else	stopb = 0;
	else break;
	tok = strtok(NULL,",");
	if (tok) if(strcmp(tok,"8"))	/* ~8 */
		if (strcmp(tok,"7"))
			usage(argvec[0]);
		else	datab = CS7;
		else	datab = CS8;
	else break;
	tok = strtok(NULL,",");
	if (tok) if(strcmp(tok,"none"))	/* ~none */
		if(strcmp(tok,"even"))	/* ~even */
		if(strcmp(tok,"odd"))	/* ~odd */
			usage(argvec[0]);
		else	parity = PARENB|PARODD;
		else	parity = PARENB;
		else	parity = 0;
	else	break;
    }	while(0);

	if (datab == CS8)	parity = 0;

	for ( chk_baud = 38400, idx_baud = B38400;
		idx_baud >= B2400;
		idx_baud--, chk_baud >>= 1  ) {
		if (baud == chk_baud)	break;
	}

	if (idx_baud < B2400)	usage(argvec[0]);

	if (*device != '/')	usage(argvec[0]);
	strcpy(portpar->port_name,device);
	portpar->bwt = 5;
	portpar->cwt = 2;
	portpar->baud = idx_baud;
	portpar->stopbits = stopb;
	portpar->databits = datab;
	portpar->parity = parity;
	portpar->dataformat = 0x3b;	/* ??? */
	portpar->tpdusize = 258;
	portpar->apdusize = 254;
	portpar->edc = 0;
	portpar->chaining = C_ON;
	portpar->schistory = NULL;
	portpar->port_id = 0;
	portpar->ssc = 0;
	portpar->sad = 2 /*SAD*/;
	portpar->dad = 1 /*DAD*/;
	portpar->sc_request = 0;
	portpar->setmode = 0;
	portpar->session_key.subjectAI = NULL;
	portpar->session_key.subjectkey.bits = NULL;
	portpar->session_key.subjectkey.nbits = 0;
	portpar->secure_messaging.response = SEC_NORMAL;
	portpar->secure_messaging.command = SEC_NORMAL;
}

static void usage(
#ifdef __STDC__
char* cmd)
#else
cmd)
char *cmd;
#endif
{
		fprintf(stderr,"usage: %s [-l logfile] [-p port] device baud,stop,data[,parity]\n", cmd);
		exit(1);
}
#endif
