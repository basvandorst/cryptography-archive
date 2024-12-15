/*
 *   Secure random number module
 *
 *   Copyright (c) 1988-1993 Shamus Software Ltd.
 */

/* assumes IOBASE=60, mirsys(..,256), 512 bit compo */

#include <stdio.h>
#include <stdlib.h>
#include <miracl.h>
#include <sys\timeb.h>
#include <time.h>

#define NUMBYTES 32

static FILE *fp;
static char path[128];
static struct timeb tbuff;
static char ranbuff[NUMBYTES];
static int buffptr=0;
static big compo,ranno;

void spin_rand()
{ /* give random number generator a spin */
    int i,len,inc;
    ftime(&tbuff);
    inc=(tbuff.millitm/10+((int)(tbuff.time%300)*100))+2;
    incr(ranno,inc,ranno);    
    do {
        mad(ranno,ranno,ranno,compo,compo,ranno);
        len=otstr(ranno,MR_IOBUFF);
    } while (len<NUMBYTES);
    for (i=0;i<NUMBYTES;i++) ranbuff[i]=MR_IOBUFF[--len];
    buffptr=0;
}

void start_rand(secret,num)
big secret,num;
{ /* start secure random number generator */
    big r;
    ranno=mirvar(0);
    compo=mirvar(0);
    r=mirvar(0);
    copy(num,compo);
    ftime(&tbuff);
    irand(tbuff.millitm/10+((int)(tbuff.time%300)*100));
    bigrand(compo,r);
    _searchenv("random.num","PATH",path);
    fp=fopen(path,"r");
    if (fp!=NULL)
    { /* read in ranno */
        cinnum(ranno,fp);
        fclose(fp);
    }
    add(ranno,r,ranno);
    mad(ranno,secret,ranno,compo,compo,ranno);
    spin_rand();
    free(r);
}

void get_rand(buff,len)
char *buff;
int len;
{ /* get len random bytes */
    int i;
    for (i=0;i<len;i++)
    {
        buff[i]=ranbuff[buffptr++];
        if (buffptr==NUMBYTES) spin_rand();
    }     
}

void save_rand()
{ /* save random number for next time */
    spin_rand();
    if (path[0]==0) fp=fopen("random.num","w");
    else            fp=fopen(path,"w");
    cotnum(ranno,fp);
    fclose(fp);
    free(compo);
    free(ranno);
}

