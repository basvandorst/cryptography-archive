#ifndef lint
static char	*sccsid = "@(#)clientlib.c	1.9	(Berkeley) 2/25/88";
#endif

/*
 * NNTP client routines.
 */

/*
 * Include configuration parameters only if we're made in the nntp tree.
 */

#include "../common/conf.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/exerrno.h>
#include <netinet/in.h>
#ifndef EXCELAN
# include <netdb.h>
#endif /* not EXCELAN */

#ifdef USG
# define	index	strchr
#endif /* USG */

#ifdef EXCELAN
# define	IPPORT_NNTP	119
#endif

#ifdef DECNET
#include <netdnet/dn.h>
#include <netdnet/dnetdb.h>
#endif /* DECNET */

#include "../common/nntp.h"

int	sockt_rd = -1;
extern	int	errno;
static char	buf[256];
static char	*bbuf = &buf[256];
static char	*ebuf = &buf[256];

/*
 * server_init  Get a connection to the remote news server.
 *
 *	Parameters:	"machine" is the machine to connect to.
 *
 *	Returns:	-1 on error
 *			server's initial response code on success.
 *
 *	Side effects:	Connects to server.
 */

server_init(machine)
char	*machine;
{
	char	line[256];
	char	*index();

	sockt_rd = get_tcp_socket(machine);

	if (sockt_rd < 0)
		return (-1);

	/* Now get the server's signon message */

	(void) get_server(line, sizeof(line));
	return (atoi(line));
}


/*
 * get_tcp_socket -- get us a socket connected to the news server.
 *
 *	Parameters:	"machine" is the machine the server is running on.
 *
 *	Returns:	Socket connected to the news server if
 *			all is ok, else -1 on error.
 *
 *	Side effects:	Connects to server.
 *
 *	Errors:		Printed via perror.
 */

get_tcp_socket(machine)
char	*machine;
{
	int	s;

	long rhost();

	struct	sockaddr_in sin, sout;
	memset((char *) &sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(IPPORT_NNTP);

	/* set up addr for the connect */
	sin.sin_addr.s_addr = rhost(&machine);
	if (sin.sin_addr.s_addr < 0){
		fprintf(stderr, "%s: Unknown host.\n", machine);
		return (-1);
	}

	memset((char *) &sout, 0, sizeof(sout));
	sout.sin_family = AF_INET;
	sout.sin_port = htons(0);
	sout.sin_addr.s_addr = 0;
	if ((s = socket(SOCK_STREAM, (struct sockproto *) NULL,
		&sout, SO_KEEPALIVE)) < 0)
		{
		experror("socket");
		return (-1);
	}
	/* And then connect */

	while (connect(s, &sin) < 0) {
		if (errno != EWOULDBLOCK && errno != EINPROGRESS)
			{
			experror("connect");
			(void) soclose(s);
			return (-1);
			}
	}
	free(machine);

	return (s);
}

#ifdef DECNET
/*
 * get_dnet_socket -- get us a socket connected to the news server.
 *
 *	Parameters:	"machine" is the machine the server is running on.
 *
 *	Returns:	Socket connected to the news server if
 *			all is ok, else -1 on error.
 *
 *	Side effects:	Connects to server.
 *
 *	Errors:		Printed via nerror.
 */

get_dnet_socket(machine)
char	*machine;
{
	int	s, area, node;
	struct	sockaddr_dn sdn;
	struct	nodeent *getnodebyname(), *np;

	memset((char *) &sdn, 0, sizeof(sdn));

	switch (s = sscanf( machine, "%d%*[.]%d", &area, &node )) {
		case 1: 
			node = area;
			area = 0;
		case 2: 
			node += area*1024;
			sdn.sdn_add.a_len = 2;
			sdn.sdn_family = AF_DECnet;
			sdn.sdn_add.a_addr[0] = node % 256;
			sdn.sdn_add.a_addr[1] = node / 256;
			break;
		default:
			if ((np = getnodebyname(machine)) == NULL) {
				fprintf(stderr, 
					"%s: Unknown host.\n", machine);
				return (-1);
			} else {
				bcopy(np->n_addr, 
					(char *) sdn.sdn_add.a_addr, 
					np->n_length);
				sdn.sdn_add.a_len = np->n_length;
				sdn.sdn_family = np->n_addrtype;
			}
			break;
	}
	sdn.sdn_objnum = 0;
	sdn.sdn_flags = 0;
	sdn.sdn_objnamel = strlen("NNTP");
	bcopy("NNTP", &sdn.sdn_objname[0], sdn.sdn_objnamel);

	if ((s = socket(AF_DECnet, SOCK_STREAM, 0)) < 0) {
		nerror("socket");
		return (-1);
	}

	/* And then connect */

	if (connect(s, (struct sockaddr *) &sdn, sizeof(sdn)) < 0) {
		nerror("connect");
		close(s);
		return (-1);
	}

	return (s);
}
#endif



/*
 * handle_server_response
 *
 *	Print some informative messages based on the server's initial
 *	response code.  This is here so inews, rn, etc. can share
 *	the code.
 *
 *	Parameters:	"response" is the response code which the
 *			server sent us, presumably from "server_init",
 *			above.
 *			"server" is the news server we got the
 *			response code from.
 *
 *	Returns:	-1 if the error is fatal (and we should exit).
 *			0 otherwise.
 *
 *	Side effects:	None.
 */

handle_server_response(response, server)
int	response;
char	*server;
{
    switch (response) {
	case OK_NOPOST:		/* fall through */
    		printf(
	"NOTE: This machine does not have permission to post articles.\n");
		printf(
	"      Please don't waste your time trying.\n\n");

	case OK_CANPOST:
		return (0);
		break;

	case ERR_ACCESS:
		printf(
   "This machine does not have permission to use the %s news server.\n",
		server);
		return (-1);
		break;

	default:
		printf("Unexpected response code from %s news server: %d\n",
			server, response);
		return (-1);
		break;
    }
	/*NOTREACHED*/
}


/*
 * put_server -- send a line of text to the server, terminating it
 * with CR and LF, as per ARPA standard.
 *
 *	Parameters:	"string" is the string to be sent to the
 *			server.
 *
 *	Returns:	Nothing.
 *
 *	Side effects:	Talks to the server.
 *
 *	Note:		This routine flushes the buffer each time
 *			it is called.  For large transmissions
 *			(i.e., posting news) don't use it.  Instead,
 *			do the fprintf's yourself, and then a final
 *			fflush.
 */

void
put_server(string)
char *string;
{
#ifdef DEBUG
	fprintf(stderr, ">>> %s\n", string);
#endif
	sowrite(sockt_rd, string, strlen(string));
	sowrite(sockt_rd, "\r\n", 2);
}


/*
 * get_server -- get a line of text from the server.  Strips
 * CR's and LF's.
 *
 *	Parameters:	"string" has the buffer space for the
 *			line received.
 *			"size" is the size of the buffer.
 *
 *	Returns:	-1 on error, 0 otherwise.
 *
 *	Side effects:	Talks to server, changes contents of "string".
 */

get_server(string, size)
char	*string;
int	size;
{
	register char *cp;
	register int i;
	int	rdlen;
	char	*index();

	for (cp = string, i = 0; i < size; i++, cp++)
		{
		if (bbuf >= ebuf)
			{
			if ((rdlen = soread(sockt_rd, buf, 256)) < 0)
				return(-1);
			bbuf = buf;
			ebuf = buf + rdlen;
			}
		*cp = *bbuf++;
		if (*cp == '\r')
			{
			*cp = '\0';
			}
		if (*cp == '\n')
			{
			*cp = '\0';
			return(0);
			}
		}
	return (0);
}


/*
 * close_server -- close the connection to the server, after sending
 *		the "quit" command.
 *
 *	Parameters:	None.
 *
 *	Returns:	Nothing.
 *
 *	Side effects:	Closes the connection with the server.
 *			You can't use "put_server" or "get_server"
 *			after this routine is called.
 */

void
close_server()
{
	char	ser_line[256];

	if (sockt_rd < 0)
		return;

	put_server("QUIT");
	(void) get_server(ser_line, sizeof(ser_line));

	(void) soclose(sockt_rd);
	sockt_rd = -1;
}
