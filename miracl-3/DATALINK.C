/*
 *   Copyright (c) 1988-1993 Shamus Software Ltd. 
 *
 * The BIDDY datalink module.
 * These routines implement a bi-directional sliding-window DATALINK, which 
 * converts an error-prone physical link into a perfect error free channel
 * for communicating data. This is done by breaking the data down into
 * FRAMES, to which is appended a two byte CRC code. The receiver checks 
 * the data against the CRC. If an error is detected, an NAK control 
 * frame is sent to elicit immediate retransmission of the frame. 
 * Otherwise an ACK frame is sent to acknowledge reception, and permit 
 * transmission of another frame. If neither arrive within a certain
 * time, the sender times-out, and sends the frame again.
 * 
 * The protocol supports piggy-backed ACKs and NAKs, and uses byte
 * stuffing to allow transmission of binary data.
 *
 * This implemention is very fast and efficient. The protocol overhead
 * is about 15% (assuming a frame size of 64 bytes).
 *
 * For details see Computer Networks by Tanenbaum (Prentice-Hall)
 *
 * The parameter 'fcount' refers to the number of consecutive time-outs
 * to be interpreted as a lost link. A value of 0 is taken as meaning
 * there can be an unlimited number of time-outs.
 *
 * The user of the Datalink function must also supply 7 of her own
 * functions as parameters. Many of these may be Null for a particular
 * application.
 *
 * void initfun()            -  A function called once at the start.
 *                              Contains user initialisation code.
 *
 * void ackfun()             -  Function to be executed on receipt of an ACK
 *
 * void datafun(char *,int)  -  Function called to process received data
 *
 * int idlefun()             -  Function called inside the datalink's major
 *                              loop. Returns a code which if greater than 0
 *                              causes the data-link to terminate.
 *
 * void statsfun(int)        -  Function to process data-link statistics
 *
 * int readyfun(char *,int,int)  -  A routine which supplies data to be 
 *                              transmitted (if any). The number of bytes
 *                              is returned. If the second parameter is
 *                              non-zero then an ACK or NAK is awaiting
 *                              a piggy-back ride - which is an incentive
 *                              to immediately transmit any data that may 
 *                              be ready. If the third parameter is zero
 *                              the link is idle, so low priority data can
 *                              be sent.
 *
 * void endfun()             -  A function called on termination of the 
 *                              datalink.
 *
 * See the module 'strings.c' for simple examples of these functions.
 * More complex functions are used in 'talk.c'
 *
 * Data Frame byte structure:-    DLE STX HDR <- DATA -> CRC1 CRC2 DLE ETX
 * Control Frame byte structure:- DLE STX HDR CRC1 CRC2 DLE ETX
 *
 * HeaDeR Byte bit structure      7     6     5     4     3     2     1     0
 *                                <- sequence ->   <-  Piggy back  ACK/NAK  ->
 *                                   number        Pig  ACK/NAK <- seq. num ->
 */

#include <stdio.h>
#include <ibmkeys.h>
#include <gf.h>
#include <asiports.h>

#define LOOP 2
#define ACKING 0x10
#define NAKING 0x18
#define CONTROL 3
#define BUFLEN 512
#define TIMEOUT (asitime()+10)%20
#define MAXPAK   256
#define MAXINPUT 256
#define SMAX 7
#define WMAX 3
#define PIGMASK 0x1F
#define PIGBITS 5

#define IOMODE (ASINOUT | BINARY | NORMALRX)

long baud_rate;

static unsigned char loopbuff[BUFLEN];
static unsigned char frame[2*MAXPAK];
static unsigned char out_buff[WMAX+1][MAXPAK];
static unsigned char in_buff[WMAX+1][MAXPAK];
static unsigned char inputbuff[MAXINPUT];
static int comms_port,iseq,lowest,ipt,oplen[WMAX+1],pig,last,first;
static int connection,waitingfor,start,rptr,wptr,iplen[WMAX+1];
static bool begun,nakked[SMAX+1],arrived[SMAX+1];

void start_comms(baud)
long baud;
{ /* initialise comms ports */
    int port;
    for (port=0;port<2;port++)
    {
        asifirst(port,IOMODE,BUFLEN,BUFLEN);
        asiinit(port,baud,P_NONE,1,8);
        asrts(port,ON);
        asdtr(port,ON);
        asistart(port,ASINOUT);
        asiignore(port,IGCTS,ON);
        asiignore(port,IGCD,ON);
    }
}

void end_comms()
{ /* reset comms ports */
    asiquit(-1);
}

void dial(number,dialmode)
char *number;
int dialmode;
{ /* dial a number */
    if (comms_port==LOOP) return;
    if (dialmode==0) HMSetDialingMethod(comms_port,PULSE);
    else             HMSetDialingMethod(comms_port,TOUCH_TONE);
    HMDial(comms_port,number);
}

void start_link(port,baud,mode)
int port,mode;
long baud;
{ /* initialise datalink */
    int i;
    comms_port=port;
    baud_rate=baud;
    pig=0;
    iseq=lowest=0;
    rptr=wptr=0;
    ipt=0;
    last=NUL;
    begun=FALSE;
    waitingfor=0;
    first=start=0;
    connection=mode;
    for (i=0;i<=SMAX;i++) 
    {
        arrived[i]=FALSE;
        nakked[i]=FALSE;
    }
    if (comms_port==LOOP) return;

    asiinit(comms_port,baud,P_NONE,1,8);
    asdtr(comms_port,ON);
    asrts(comms_port,ON);
    HMWaitForOK(TICKS_PER_SECOND,NULL);
    HMSetUpAbortKey(ESC);
    HMReset(comms_port);
    HMSetEchoMode(comms_port,OFF);
    HMSelectExtendedResultCodes(comms_port,ON);
    if (connection==3) HMSetRegister(comms_port,0,1);
    else               HMSetRegister(comms_port,0,0);
}

void end_link()
{ /* end comms, hang up, reset modem and clear I/O buffers */
    if (comms_port==LOOP) 
    {
        timer(30);
        return;
    }
    asdtr(comms_port,OFF);
    timer(30);
    if (connection==3) 
    { /* turn off auto-answer */
        HMSetRegister(comms_port,0,0);
    }
    asiclear(comms_port,ASINOUT);
}

bool modem_link()
{ /* make modem connection */
    int alarm,start_ticks,ticks,ticks_left;
    bool gotit;
    long baud;
    char line[81];

    if (comms_port==LOOP) 
    {
        timer(30);
        if (kbhit() && getkey()==ESC) return FALSE;
        return TRUE;
    }

    if (connection==0) HMDial(comms_port,"");
    if (connection==2) HMAnswer(comms_port);

    start_ticks=TICKS_PER_SECOND*15;
    if (connection==1 || connection==3) start_ticks*=3;

    ticks=start_ticks;
    for(;;)
    {
        if (kbhit() && getkey()==ESC) return FALSE;

        ticks_left=HMInputLine(comms_port,ticks,line,81);

        if (connection==3 && ticks_left<=0)
        {
            ticks=start_ticks;
            timer(TICKS_PER_SECOND);
            continue;
        }
        if (connection!=3 && ticks_left<=0) break;

        baud=0L;
        if (strcmp(line,"CONNECT")==0) baud=300L;
        if (strcmp(line,"CONNECT 600")==0) baud=600L;
        if (strcmp(line,"CONNECT 1200")==0) baud=1200L;
        if (strcmp(line,"CONNECT 2400")==0) baud=2400L;
        if (strcmp(line,"CONNECT 4800")==0) baud=4800L;
        if (strcmp(line,"CONNECT 9600")==0) baud=9600L;
        if (baud!=0L)
        {
            baud_rate=baud;
            asiinit(comms_port,baud_rate,P_NONE,1,8);
            break;
        }
        ticks=ticks_left;
        continue;
    }

    alarm=(asitime()+10)%60;
    while (asitime()!=alarm) if (iscd(comms_port,DIRECT)) return TRUE;
#ifdef NO_CD
    return TRUE;
#endif
    return FALSE;
}

static void putbuff(port,buff,len)
int port,len;
char *buff;
{
    int i,nptr;
    if (port!=LOOP)
    {
        asiputb(port,buff,len);
        return;
    }
    for (i=0;i<len;i++)
    { /* avoid head-eat-tail in ring buffer */
        nptr=(wptr+1)&(BUFLEN-1);
        if (nptr==rptr) return;
        loopbuff[wptr]=buff[i];
        wptr=nptr;
    }
}

static int getcc(port)
int port;
{
    int ch;
    if (port!=LOOP) return asigetc(port);
    ch=loopbuff[rptr++];
    rptr&=(BUFLEN-1);
    return ch;
}

static bool isempty(port)
int port;
{
    if (port!=LOOP) return isrxempty(port);
    if (rptr==wptr) return TRUE;
    return FALSE;
}
 
static unsigned int compute_crc(ptr,len)
char *ptr;
int len;
{ /* error detection by CRC */
    int i;
    unsigned int crc;
    crc=0;
    while (len--)
    {
        crc^=(unsigned int)(*ptr++)<<8;
        for (i=0;i<8;++i)
        {
            if (crc&0x8000) crc=(crc<<1)^0x1021;
            else crc<<=1;
        }
    }
    return crc;
}

static int make_frame(data,len,seq,piggy)
unsigned char *data;
int len,seq,piggy;
{ /* make up a new frame for transmission */
    int i,j,ipt,ptr;
    unsigned int crc;
    frame[0]=DLE;
    frame[1]=STX;
    frame[2]=((seq<<PIGBITS)|piggy);
    ptr=3;
    for (i=0;i<len;i++) frame[ptr++]=data[i];
    crc=compute_crc(&frame[2],ptr-2);
    frame[ptr++]=((crc>>8)&0xFF);
    frame[ptr++]=(crc&0xFF);
    frame[ptr++]=DLE;
    frame[ptr++]=ETX;
    ipt=2;
    for (i=0;i<len+3;i++) if (frame[ipt++]==DLE) 
    { /* now do character stuffing - stuff in a DLE */
        for (j=ptr;j>ipt;j--) frame[j]=frame[j-1];
        frame[ipt++]=DLE;
        ptr++;
    }
/* debug only - create random errors 
    for (i=0;i<ptr;i++) if (rand()%1000==1) frame[i]=0; */
    return ptr;
}

int datalink(fcount,initfun,ackfun,datafun,idlefun,statsfun,readyfun,endfun)
int fcount;
void (*initfun)(),(*ackfun)(),(*datafun)(),(*statsfun)(),(*endfun)();
int  (*readyfun)(),(*idlefun)();
{ /* main datalink program                      *
   * returns -1 if too many time-outs           *
   * returns -2 if carry detect is lost         *
   * or user defined +ve code +n from idlefun() */

    int i,j,c,len,ilen,seqnum,piggy,stop_code,failures;
    int seq,owptr,iwptr,got_ya,alarm[WMAX+1];
    bool duplicate,got_a_frame;
    (*initfun)();
    failures=0;
    got_a_frame=FALSE;
    do
    { /* main loop - important to get round at least once per second */
        got_ya=0;
        while (!isempty(comms_port))
        { /* process received character */
            c=getcc(comms_port);
            if (begun)
            { /* get frame data */
                if (ipt>=MAXINPUT)
                {
                    begun=FALSE;
                    ipt=0;
                    last=NUL;
                    continue;
                }
                inputbuff[ipt++]=c;
            }
            if (c==STX && last==DLE)
            { /* new frame start */
                if (begun && ipt>=CONTROL+4)
                { /* Huh - what about previous frame? */
                    ipt-=2;
                    got_a_frame=TRUE;
                    break;
                }
                begun=TRUE;
                ipt=0;
                last=NUL;
            }
            if (begun && c==ETX && last==DLE)
            { /* end of frame */
                begun=FALSE;
                got_a_frame=TRUE;
                break;
            }
            if (c==DLE && last==DLE)
            { /* destuff double DLE */
                last=NUL;
                if (begun) ipt--;
            }
            else last=c;
        }
        if (got_a_frame)
        { /* got a frame! */
            got_a_frame=FALSE;
            failures=0;
            ilen=ipt-2;
            ipt=0;
            last=NUL;
            if (ilen<CONTROL) continue;
            seqnum=((inputbuff[0]>>PIGBITS)&SMAX);
            piggy=  (inputbuff[0]&PIGMASK);
            if (compute_crc(inputbuff,ilen)!=0)
            { /* an error has occurred */
                if (ilen==CONTROL) continue;
                for (i=0;i<=WMAX;i++) if (seqnum==((iseq+i)&SMAX))
                if (!arrived[seqnum]  && !nakked[seqnum])
                { /* prepare an NAK if looks like data frame */
                    pig=(NAKING|seqnum);
                    nakked[seqnum]=TRUE;
                    (*statsfun)(2);
                    break;
                }
                if (waitingfor>0 && piggy==(NAKING|lowest))
                { /* possible NAK on first frame - send it again */
                    len=make_frame(out_buff[first],oplen[first],lowest,pig);
                    putbuff(comms_port,frame,len);
                    alarm[first]=TIMEOUT;
                    pig=0;
                }
                ilen=0;
            }
            else  for (i=0;i<waitingfor;i++) 
            { /* check for valid ACK or NAK */
                seq=((lowest+i)&SMAX);
                if (piggy==(ACKING|seq))
                { /* received ack */
                    for (j=0;j<=i;j++)
                    { /* acks all outstanding frames */
                        (*ackfun)();
                        waitingfor--;
                        first++;
                        first&=WMAX;
                        lowest++;
                        lowest&=SMAX;
                    }
                    (*statsfun)(5);
                    break;
                }
                if (piggy==(NAKING|seq))
                { /* received nak - send bad frame again */
                    owptr=((first+i)&WMAX);
                    len=make_frame(out_buff[owptr],oplen[owptr],seq,0);
                    putbuff(comms_port,frame,len);
                    alarm[owptr]=TIMEOUT;
                    (*statsfun)(3);
                    break;
                }
            }
            if (ilen>CONTROL)
            { /* deal with incoming data - prepare to ack */
                duplicate=TRUE;
                for (i=0;i<=WMAX;i++) if (seqnum==((iseq+i)&SMAX))
                { /* Store expected frames */
                    duplicate=FALSE;
                    if (arrived[seqnum]) break;
                    iwptr=((start+i)&WMAX);
                    for (j=0;j<ilen-CONTROL;j++)
                         in_buff[iwptr][j]=inputbuff[j+1];
                    iplen[iwptr]=ilen-CONTROL;
                    arrived[seqnum]=TRUE;
                    nakked[seqnum]=FALSE;
                    break;
                }
                if (duplicate)
                { /* ack last frame received */
                    if (iseq==0) seq=SMAX;
                    else         seq=iseq-1;
                    pig=(ACKING|seq);
                }
                else  while(arrived[iseq])
                { /* count consecutive frames - ack highest */
                    arrived[iseq]=FALSE;
                    pig=(ACKING|iseq);
                    iseq++;
                    iseq&=SMAX;
                    got_ya++;
                }
                if (pig) (*statsfun)(4);
            }
        }
        owptr=((first+waitingfor)&WMAX);
        if (waitingfor<=WMAX)
        if ((oplen[owptr]=(*readyfun)(out_buff[owptr],pig,waitingfor))>0)
        { /* transmit a data frame */
            seq=((lowest+waitingfor)&SMAX);
            len=make_frame(out_buff[owptr],oplen[owptr],seq,pig);
            putbuff(comms_port,frame,len);
            alarm[owptr]=TIMEOUT;
            waitingfor++;
            pig=0;
            (*statsfun)(0);
        }
        for (i=0;i<waitingfor;i++) 
        { /* check for time-outs */
            owptr=((first+i)&WMAX);
            if ((asitime()%20)==alarm[owptr]) 
            { /* Time-Out - Retransmit an unacked frame */
                seq=((lowest+i)&SMAX);
                len=make_frame(out_buff[owptr],oplen[owptr],seq,pig);
                putbuff(comms_port,frame,len);
                alarm[owptr]=TIMEOUT;
                pig=0;
                failures++;
                (*statsfun)(7);
                break;
            }
        }
        if (pig)
        { /* transmit a control frame  */
            len=make_frame(NULL,0,0,pig);
            putbuff(comms_port,frame,len);
            pig=0;
            (*statsfun)(6);
        }
        for (i=0;i<got_ya;i++) 
        { /* process arrived frames in order */
            (*datafun)(in_buff[start],iplen[start]);
            start++;
            start&=WMAX;
            (*statsfun)(1);
        }
        stop_code=(*idlefun)();
        if (fcount>0 && failures>fcount) stop_code=(-1);
#ifndef NO_CD
        if (comms_port!=LOOP) if(!iscd(comms_port,DIRECT)) stop_code=(-2);
#endif
    } while (!stop_code);
    (*endfun)();
    return stop_code;
}

