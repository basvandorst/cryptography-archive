/* sra demo utilities */
/* functions:
	send_credentials(int sock, char *user, char *pass)
		sock is a tcp socket connected to server
		user is a null terminated input user name
		pass is a null terminated input password
	get_credentials(int sock, char *user, char *pass)
		sock is a tcp socket connected to client
		user is allocated char[] for terminated output user name
		pass is allocated char[] for terminated output password
   these use the following low level pk routines from libpk.a:
	genkeys(char *public, char *secret)
	common_key(char *secret, char *public, desData *deskey)
      where
	char public[HEXKEYBYTES + 1];
	char secret[HEXKEYBYTES + 1];
 */

#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <pk.h>
 
int send_credentials(int s1, char *u, char *p)
{
    char pka[HEXKEYBYTES+1],ska[HEXKEYBYTES+1],pkb[HEXKEYBYTES+1];
    DesData ck;
    char *user,*xuser,*pass,*xpass;
    int rc;

    user = (char *) malloc(256);
    xuser = (char *) malloc(512);
    pass = (char *) malloc(256);
    xpass = (char *) malloc(512);

    /* both sides calculate pk, sk */ 
    genkeys(pka,ska);

    /* send him my public key */
    if (send(s1, pka, HEXKEYBYTES+1, 0) != HEXKEYBYTES+1) {
	return(-1);
    }

    /* get his public key */
    if ((rc = recv(s1, pkb, HEXKEYBYTES+1, 0)) <= 0) {
	return(-1);
    }

    /* both sides calculate ck */
    common_key(ska,pkb,&ck);

    /* send ck(user), ck(pass) */
    strcpy(user,u);
    strcpy(pass,p);
    pk_encode(user,xuser,&ck);
    pk_encode(pass,xpass,&ck);
    if (send(s1, xuser, strlen(xuser)+1, 0) != strlen(xuser)+1) {
	return(-1);
    }
    if (send(s1, xpass, strlen(xpass)+1, 0) != strlen(xpass)+1) {
	return(-1);
    }
}

int get_credentials(int s2, char *u, char *p)
{
    char pka[HEXKEYBYTES+1],ska[HEXKEYBYTES+1],pkb[HEXKEYBYTES+1];
    DesData ck;
    char *user,*xuser,*pass,*xpass, *h;
    int rc;

    user = (char *) malloc(256);
    xuser = (char *) malloc(512);
    pass = (char *) malloc(256);
    xpass = (char *) malloc(512);
    bzero(xuser,sizeof(xuser));
    bzero(xpass,sizeof(xpass));

    /* both sides calculate pk, sk */
    genkeys(pka,ska);

    /* get his public key */
    if ((rc = recv(s2, pkb, HEXKEYBYTES+1, 0)) <= 0) {
	return(-1);
    }
    /* send him my public key */
    if (send(s2, pka, HEXKEYBYTES+1, 0) != HEXKEYBYTES+1) {
	return(-1);
    }
    /* generate common and session keys */
    common_key(ska,pkb,&ck);

    /* get ck(user), ck(pass) */
    for (h=xuser;h<(xuser+512);h++) {
	read(s2,h,1);
	if (*h == '\0')
		break;
    }
    for (h=xpass;h<(xpass+512);h++) {
	read(s2,h,1);
	if (*h == '\0')
		break;
    }

    pk_decode(xuser,user,&ck);
    pk_decode(xpass,pass,&ck);
    strcpy(u,user);
    strcpy(p,pass);
}
