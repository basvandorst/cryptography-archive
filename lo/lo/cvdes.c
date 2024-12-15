#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "linux/des.h"
#include "/usr/src/linux/kernel/des.c"

#define BUFFER_SIZE (16*1024)


static des_key_schedule key;
static unsigned long x_init[2] = { 0,0 };


static void translate(unsigned char *block)
{
    unsigned long x0,x1,p0,p1,tmp[2];
    int i;

    x0 = x_init[0];
    x1 = x_init[1];
    for (i = 0; i < 1024; i += 8) {
	des_ecb_encrypt((des_cblock *) block,(des_cblock *) block,key,
	  DES_DECRYPT);
	tmp[0] = (p0 = ((unsigned long *) block)[0])^x0;
	tmp[1] = (p1 = ((unsigned long *) block)[1])^x1;
	des_ecb_encrypt((des_cblock *) tmp,(des_cblock *) block,key,
	  DES_ENCRYPT);
	x0 = p0^((unsigned long *) block)[0];
	x1 = p1^((unsigned long *) block)[1];
	block += 8;
    }
}


static void usage(char *name)
{
    fprintf(stderr,"usage: %s name\n",name);
    exit(1);
}


int main(int argc,char **argv)
{
    int fd,size,i;
    unsigned char buffer[BUFFER_SIZE];
    char *pass,*init,padded[8];

    if (argc != 2) usage(argv[0]);
    if ((fd = open(argv[1],O_RDWR)) < 0) {
	perror(argv[1]);
	return 1;
    }
    pass = getpass("Password: ");
    memset(padded,0,8);
    strncpy(padded,pass,8);
    des_set_key((des_cblock *) padded,key);
    init = getpass("Init (up to 16 hex digits): ");
    for (i = 0; i < 16 && init[i]; i++)
        if (isxdigit(init[i]))
	    x_init[i >> 3] |= (init[i] > '9' ? (islower(init[i]) ?
	      toupper(init[i]) : init[i])-'A'+10 : init[i]-'0') << (i & 7)*4;
	else {
	    fprintf(stderr,"Non-hex digit '%c'.\n",init[i]);
	    exit(1);
	}
    while ((size = read(fd,buffer,BUFFER_SIZE)) > 0) {
	if (size & 1023) {
	    fprintf(stderr,"Device does not span an integral number of "
	      "blocks.\n");
	    return 1;
	}
	for (i = 0; i < size; i += 1024) translate(buffer+i);
	(void) lseek(fd,-BUFFER_SIZE,SEEK_CUR);
	if (write(fd,buffer,BUFFER_SIZE) != BUFFER_SIZE) {
	    perror(argv[1]);
	    return 1;
	}
    }
    if (close(fd) < 0) perror("close");
    return 0;
}
