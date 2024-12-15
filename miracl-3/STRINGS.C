/*
 *   Copyright (c) 1988-1993 Shamus Software Ltd. 
 *
 * These routines use the datalink to swap a pair of strings 
 */

#include <stdio.h>
#define bool int
#define TRUE 1
#define FALSE 0
#define ESC 27

extern int datalink(int,void(*)(),void(*)(),void(*)(),int(*)(),
                    void(*)(),int(*)(),void(*)());

static int maximum;
static unsigned char *my_string,*her_string;
static bool done,sent_my_string,got_her_string;

static void str_init()
{ /* initialise */
    done=FALSE;
    if (my_string!=NULL) sent_my_string=FALSE;
    else                 sent_my_string=TRUE;
    if (her_string!=NULL)got_her_string=FALSE;
    else                 got_her_string=TRUE;
}

static void str_ack()
{ /* respond to an ACK being received */
    sent_my_string=TRUE;
}

static void str_data(data,len)
char *data;
int len;
{ /* process received data */
    int i;
    if (len>=maximum) return;
    decode(data,len,her_string);
    got_her_string=TRUE;
}   

static int str_idle()
{ /* soft exit when finished */
    int ch;
    while (kbhit())
    { /* check keyboard */
        ch=getkey();
        if (ch==ESC) return 2;
    }
    if (sent_my_string && got_her_string) return 1;
    return 0;
}

static void str_stats(stat)
int stat;
{
}

static int str_ready(data,pig,busy)
unsigned char *data;
int pig,busy;
{
    int len=0;
    if (done) return 0;
    len=encode(my_string,strlen(my_string)+1,data);
    done=TRUE;
    return len;
}

static void str_end()
{
}

int exchange_strings(fcount,mine,hers,max)
int fcount,max;
unsigned char *mine,*hers;
{
    int r;
    my_string=mine;
    her_string=hers;
    maximum=max;
    r=datalink(fcount,str_init,str_ack,str_data,str_idle,
                    str_stats,str_ready,str_end);
    return r;
}

