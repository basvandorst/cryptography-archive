#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

/*
** pg_inetd.c
**
** Mini-inetd for testing PGPD
**
** Ben Cox <thoth@netcom.com>, 19 Nov 1993
**
** See the file COPYING distributed with this program for
** information regarding distribution of this program.
*/



static int sock;


/*
** We install a signal handler to try to close the socket if we
** are interrupted (to avoid EADDRINUSE).  It doesn't always
** work, but it helps.
*/

static void handle_intr(int s)
{
    close(sock);                /* clean up after myself */
    printf("Caught SIGINT; exiting.\n");
    exit(0);
}


/*
** This provides a mini-inetd-like service: it listens on the
** KTPPORT, and when a connection is made, spawns a PGPD to
** handle the connection.  Thus PGPD can be written to use
** stdin/stdout/stderr.
**
** I wrote this because I don't have root on my workstation,
** and thus can't change /etc/services or /etc/inetd.conf.
*/

void main(int argc, char *argv[])
{
    struct sockaddr_in sin;
    struct sockaddr_in peer;
    int dsock;
    int alen;
    pid_t pid;
    fd_set readfds;
    struct timeval tm;
    int sel;

    /* This program takes no arguments.  Or else.  :)  */

    if ( argc != 1 ) {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        exit(1);
    }

    if ( (sock = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror("socket");
        exit(1);
    }

    /* wait to do this until after socket is open, because handler closes it */
    (void) signal(SIGINT, handle_intr);

    sin.sin_family      = AF_INET;
    sin.sin_port        = htons(KTPPORT);
    sin.sin_addr.s_addr = INADDR_ANY;

    if ( bind(sock, (const void *) &sin, sizeof(struct sockaddr_in)) < 0 ) {
        perror("bind");
        exit(1);
    }

    if ( listen(sock, 5) < 0 ) {
        perror("listen");
        exit(1);
    }

    alen = sizeof(struct sockaddr_in);

    /* 30 second timeout on select so that we may reap
       children within a reasonable period of time. */
    tm.tv_sec = 30;
    tm.tv_usec = 0;

    while ( 1 ) {
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);

        if ( (sel = select(sock + 1, (int *) &readfds,
                           (int *) NULL, (int *) NULL, &tm)) < 1 ) {
            if ( sel == 0 ) {
                /* reap children when nothing else to do */
                waitpid(-1, NULL, WNOHANG);
                continue;
            } else
                break;
        }

        dsock = accept(sock, (void *) &peer, &alen);

        if ( (pid = fork()) < 0 ) {
            perror("fork");
            close(dsock);
            alen = sizeof(struct sockaddr_in);
            continue;
        }

        if ( pid == 0 ) {       /* child */
            close(sock);
            (void) dup2(dsock, 0);
            (void) dup2(dsock, 1);
            (void) dup2(dsock, 2);
            close(dsock);
            /* Oh, tell me you'll be back! */
            execl(PGPD, "pgpd", NULL);
            /* Only if somethin' doesn't work out, baby... */
            /* MrAttribution: Mark-Jason Dominus <mjd@saul.cis.upenn.edu> */
            perror("exec");
            exit(1);
        }
        /* parent */
        close(dsock);
        alen = sizeof(struct sockaddr_in);
    }

    perror("accept");
    fprintf(stderr, "(%s exiting.)\n", argv[0]);
    exit(1);
}
