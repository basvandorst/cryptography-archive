/*
 *  demo_client	 - demonstrate pk login
 */

#include          <stdio.h>
#include          <errno.h>
#include          <sys/types.h>
#include	  <sys/time.h>
#include          <sys/socket.h>
#include          <netinet/in.h>
#include	  <netinet/tcp.h>
#include          <netdb.h>

#include	  <pk.h>
#define            DEMO_PORT 3000


int  main(argc, argv)		
int             argc;
char          **argv;
{
    struct sockaddr_in server;
    struct hostent *hp;
    int s1;

    if (argc != 4) {
	fprintf(stderr, "demo_client <host> <user> <pass>, ARGC = %d \n", argc);
	exit(-1);
    }
    if ((hp = gethostbyname(argv[1])) == NULL) {
	perror("GETHOSTBYNAME-1");
	exit(-1);
    } else if (hp->h_length != 4) {
	fprintf(stderr, "ILLEGAL NET ADDR LENGTH-1 = %d \n",
		hp->h_length);
	exit(-1);
    } else if (hp->h_addrtype != AF_INET) {
	fprintf(stderr, "ILLEGAL ADDRTYPE-1 = %d \n", 
		hp->h_addrtype);
	exit(-1);
    }
    
    if ((s1 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	perror("DEMO_CLIENT: SOCKET-1");
	exit(-1);
    }

    bzero((char *) &server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = DEMO_PORT;
    bcopy(hp->h_addr, (char *) &server.sin_addr, 4);
    if (connect(s1, &server, sizeof(server)) < 0) {
	perror("CONNECT");
	exit(-1);
    }

    if (send_credentials(s1,argv[2],argv[3]) < 0) {
	printf("demo_client error in send_credentials\n");
    }

}
