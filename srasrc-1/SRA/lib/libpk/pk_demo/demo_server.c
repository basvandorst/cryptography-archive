/*
 *  demo_server   - demonstrate pk login exchange
 *
 *  synopsis:  demo_server
 *
 */
#include          <stdio.h>
#include          <errno.h>
#include          <sys/types.h>
#include          <sys/socket.h>
#include          <sys/time.h>
#include          <netinet/in.h>
#include          <netdb.h>

#include	  <pk.h>
#define            DEMO_PORT 3000

int 
main()		/* SERVER_IN ()   */
{
    struct sockaddr_in server, client;

    int         s1,s2,len;
    char	user[256],pass[256];

    if ((s1 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	perror("DEMO_SERVER: SOCKET-1");
	exit(-1);
    }
    bzero((char *) &server, sizeof(server));
    server.sin_port = DEMO_PORT;
    if (bind(s1, &server, sizeof(server)) < 0) {
	perror("DEMO_SERVER: BIND-1");
	printf("DEMO_SERVER: BIND-1 ADDR = %d \n", DEMO_PORT);
	exit(-1);
    }
    if (listen(s1, 1) < 0) {
	perror("DEMO_SERVER: LISTEN");
	exit(-1);
    }
    len = sizeof(client);
    if ((s2 = accept(s1, &client, &len)) < 0) {
	perror("DEMO_SEVER: ACCEPT");
	exit(-1);
    }

    get_credentials(s2, user, pass);

    /* print stuff out */
    printf("user %s pass %s\n",user,pass);

}
