/*---  testtcp.c
 *  This program is a very simple TELNET-like TCP application.
 *  It connects to a remote TCP server and sends lines of
 *  text from the keyboard to the server, and displays 
 *  received lines on stdout.
 *  It is an example of how to use "select", which is needed
 *  when you don't know from which source input is next going to come.
 *
 *  Mark Riordan    28 May 1989
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/termios.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int sock;
int ttyfd;
struct sockaddr_in name;
struct termios myios;
FILE *LocalOutStream;

#define KBDBUFSIZE 1600
#define SOCKBUFSIZE 9216
#if 0
#define PC
#endif
#define DEBUG 0

#if 1
#define myerr(str)
#else
#define myerr(str) fputs(str,stderr)
#endif

main(argc,argv)
int argc;
char *argv[];
{
	int sock,protocol=0;
	char *cp;
	int j, result, nbytes, bytesread;
	int mylen;
#ifdef PC
	struct in_addr daddr;
#else
	unsigned long daddr;
#endif
	char kbbuf[KBDBUFSIZE];
	char sockbuf[SOCKBUFSIZE];
	int myport = 119; /* 1521 for MRR's NNTP */  /* normally 119 */ 
	int nfds = 32;
	long readfds, writefds=0, exceptfds=0;
	struct timeval timeout;
	int done=0;

	/* Set up the local terminal. */
	LocalOutStream = stdout;
	ttyfd = open("/dev/tty",O_RDWR);
	ioctl(ttyfd,TCGETS,&myios);
	myios.c_lflag &= 0xffffffff ^ ECHO;
	ioctl(ttyfd,TCSETS,&myios);

	/* Set up for connection to the remote TCP server (usually the Netnews
    * NNTP server).
    */
	sock = socket(AF_INET,SOCK_STREAM,protocol);
#if DEBUG
	printf("ttyfd=%x, sock=%x hex\n",ttyfd,sock);
#endif
	if(sock == -1) {
		printf("-> Error creating socket: %d\n",errno);
	}

	/* Create "daddr" as the host address of the server we want to
	 * connect to.  "daddr" is in network byte order.
	 * Note:  ebfs2 is 35.8.2.65
	 *  cl-next1 is 35.8.4.21   msuinfo is 35.8.2.20
	 */

	if(argc > 1) {
		cp = argv[1];
	} else {
		cp = "35.8.2.20";
	}

	if(argc > 2) {
		myport = atoi(argv[2]);
	}

	daddr = inet_addr(cp);

	/* Set up name structure */

	bcopy(&daddr,&name.sin_addr.s_addr,sizeof(daddr));
	name.sin_port = htons(myport);  /* TELNET port */
	name.sin_family = AF_INET;
	for(j=0; j<8; j++) name.sin_zero[j] = 0;
#if 0
        result = bind(sock, &name, sizeof(name));
        if(result) {
                printf("-> Error binding socket: %d\n",errno);
        }
#endif
	printf("Ready to connect to %s.\n",cp);
	result = connect(sock,&name,sizeof(name));
	if(result) {
		printf("-> Error connecting: %d\n",errno);
		perror("connecting");
	}

	/* Loop, reading lines from either the terminal or the
	 * socket & echoing them to the other of those two.
	 */
	while(!done) {

		readfds = 1<<sock | 1<<ttyfd;
		timeout.tv_sec = 1;
		timeout.tv_usec = 1450;
		myerr("Before select... ");
		if(select(nfds,&readfds,NULL,NULL,NULL)) {
#if DEBUG
			printf("\nreadfds = %x  \n",readfds);
#endif
			if(readfds & (1<<ttyfd)) {
				mylen = read(ttyfd,kbbuf,KBDBUFSIZE);
#if DEBUG
				printf("Read \"");
				for(j=0; j<mylen; j++) putchar(kbbuf[j]);
				putchar('"');
#endif
				write(sock,kbbuf,mylen);
			} else if(readfds & (1<<sock)) {
				bytesread = read(sock,sockbuf,SOCKBUFSIZE);
#if DEBUG
				printf("%d bytes read: \"",bytesread);
				for(j=0; j<bytesread; j++) putchar(sockbuf[j]);
				 putchar('"'); putchar('\n');
#endif
				if(bytesread > 0) {
#ifdef USEHANDLE
					write(ttyfd,sockbuf,bytesread);
#else
					fwrite(sockbuf,bytesread,1,LocalOutStream);
#endif
				} else {
					done = 1;
				}
			} else {
				printf("Can't make sense of readfds = %x h",readfds);
			}
		} else {
			myerr("Select time expired.");
		}
	}
	myerr("Closing socket... ");
	result = close(sock);
	if(result) {
		printf("-> Error closing socket: %d\n",result);
	}
	
	myios.c_lflag |= ECHO;
	ioctl(ttyfd,TCSETS,&myios);
}
