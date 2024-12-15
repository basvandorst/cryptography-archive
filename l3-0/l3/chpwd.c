/* chpwd.c - change password to private key 		*/
/* old private key in stdin, new one in stdout 		*/
#include <gmp.h>
#include "lucas.h"
#include "d3des.h"
#include "prandom.h"
#include <stdio.h>
#include "misc.h"

main(int argc,char *argv[])
{
    MP_INT *e,*p,*q;
	unsigned char *ss,*p0,*p1;
	unsigned char k[16];
	unsigned char iv[8];
	int i;

	init(e); init(p); init(q);
    fprintf(stderr, "\nEnter old password to private key: ");
    ss = getstring(1);
    mdstring(ss,strlen(ss),k);
    for (i=0;i<8;i++)
        iv[i] = 0;

    if (!read_numberx(stdin,p,k,iv) || !read_numberx(stdin,q,k,iv)) {
        fprintf(stderr, "wrong key to key entered or key corrupt\n");
        exit(1);
    }
    read_number(stdin,e);

	p0 = p1 = NULL;
    do {
        if (p0)
            free(p0);
        if (p1)
            free(p1);
        fprintf(stderr, "\nenter new password for key -- end with <CR> \n");
        p0=getstring(1);
        fprintf(stderr, "\nenter again for verification\n");
        p1=getstring(1);
    } while(strcmp(p0,p1));

    mdstring(p0,strlen(p0),k);
    
    write_numberx(stdout,p,k,iv);
    write_numberx(stdout,q,k,iv);
    write_number(stdout,e);

	zap(p0,strlen(p0));
	zap(p1,strlen(p1));
	zap(ss,strlen(ss));
	zap(k,strlen(k));
	clear(p); clear(q); clear(e);
	exit(0);
}
