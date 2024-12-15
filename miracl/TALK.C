/*   Copyright (c) 1988-1993 Shamus Software Ltd. */

#include <stdio.h>
#include <time.h>
#include <process.h>
#include <io.h>
#include <dos.h>
#include <gf.h>
#include <ibmkeys.h>
#include <dw.h>
#include <dwmenu.h>

#define forever for(;;)
#define HANGON  while(1){if(kbhit()&&(getkey()==ESC))break;else;}
#define STX 2
#define ETX 3
#define NUL 0
#define NAK 21
#define SPACE 32
#define MAXKEYS 32
#define FILEPAK 32
#define KEY_STROKES 0
#define FILE_DATA 1
#define FILE_NAK 2
#define NO_ACK_ACTION 3
#define MAXWIN 8
#define BLOCK_SIZE 8
#define MAXLINE 37

extern int datalink(int,void(*)(),void(*)(),void(*)(),int(*)(),void(*)(),
                    int(*)(),void(*)());

extern HWND errwin,ofwin,mywin,yourwin,cted;
extern HWND win,twin,rwin,statwin,logwin,callwin,baudwin;
extern MENUHDR *mnu;
extern MENUITEM *mfile,*shell,*quit,*temp,*mlog;

extern long baud_rate;

static LISTITEM *first,*next;
static union {long p; unsigned char b[4];} lbs;
static FILE *ofp,*ifp,*logfile,*msg;
static long ofcnt,ifcnt,oflent,iflent;
static int problem,linelen,klen,lrow,lcol,ofper,ifper,unacked,kpak;
static int last_len[MAXWIN],packet[MAXWIN];
static bool flush,file_open,file_error,incoming,outgoing,abort_it,log,message;
static unsigned char key_buff[MAXKEYS];
static unsigned char logpath[128],path[128],direct[128],fname[13],filename[13];
static long stable[8];
static char string[100];
static struct tm *newtime;
static time_t aclock;
static char *announce;
static struct find_t files;

static void mymktemp(fname)
char *fname;
{ /* create temporary filename */
    FILE *fp;
    int i,cnt,nxt;
    for (cnt=0;;cnt++) if (fname[cnt]=='\0') break;
    while (--cnt>=0)
    {
        if (fname[cnt]=='.') continue;
        nxt=fname[cnt]='A';
        while (access(fname,0)==0) 
        { /* file exists */
            nxt++;
            if (nxt>'Z') break;
            fname[cnt]=nxt;
        }
        if (nxt>'Z') continue;
        break;
    }    
}

static void talk_init()
{ /* initialise for 'chatting' communications */
    int i;
    for (i=0;i<8;i++) stable[i]=0L;
    _searchenv("scramble.log","PATH",logpath);
    if (path[0]==0) strcpy(logpath,"scramble.log");
    logfile=fopen(logpath,"a");
    if (log)
    { /* time stamp */  
        visible(logwin,YES,NO);
        time(&aclock);    
        newtime=localtime(&aclock);
        fprintf(logfile,"\n*** Connection on %s\n",asctime(newtime));
    }

    vclrattr(mywin,EMPHNORML);
    leeow(mywin);
    vclrattr(mywin,REVEMPHNORML);
    vscrolup(mywin,1);    
    curdrag(mywin,ON);
    lcurdn(mywin,7);
    lcurdn(yourwin,14);

    for (i=0;i<(36-strlen(announce))/2;i++) vdispch(callwin,SPACE);
    vclrattr(callwin,EMPHNORML);
    leeow(callwin);
    vclrattr(callwin,REVEMPHNORML);
    vscrolup(callwin,1);

    vdispstr(callwin,announce);
    vatputf(baudwin,0,0,"%5ld",baud_rate);
    visible(baudwin,YES,NO);
    visible(cted,YES,NO);

    vtop(mywin);
    lcurget(mywin,&lrow,&lcol);

    flush=FALSE;
    klen=0;
    linelen=0;
    problem=0;
    file_open=FALSE;
    file_error=FALSE;
    incoming=FALSE;
    outgoing=FALSE;
    abort_it=FALSE;
    kpak=unacked=0;
    if (log) fprintf(logfile,"\nIncoming communication at %ld baud from:-\n%s\n\n"
                    ,baud_rate,announce);
}

static void talk_ack()
{ /* respond to an ACK */
    int i,row,col,last,paktype;
    unacked--;
    paktype=packet[0];
    for (i=0;i<unacked;i++) packet[i]=packet[i+1];
    if (paktype!=KEY_STROKES) return;
    kpak--;
    last=last_len[0];
    lcurpush(mywin);
    lcurget(mywin,&row,&col);
    lcurset(mywin,lrow,lcol);
    setattr(mywin,EMPHNORML);
    vclrattr(mywin,EMPHNORML);
    for (i=0;i<last;i++)
    {
        if (key_buff[i]==CR) leeol(mywin);
        if (key_buff[i]==BKSP)
        {
            setattr(mywin,REVEMPHNORML);
            vclrattr(mywin,REVEMPHNORML);
            vdispch(mywin,BKSP);
            setattr(mywin,EMPHNORML);
            vclrattr(mywin,EMPHNORML);
        }
        else vdispch(mywin,key_buff[i]);
    }
    for (i=last;i<klen;i++) key_buff[i-last]=key_buff[i];
    for (i=0;i<kpak;i++)    last_len[i]=last_len[i+1]-last;
    klen-=last;
    setattr(mywin,REVEMPHNORML);
    vclrattr(mywin,REVEMPHNORML);
    lcurget(mywin,&lrow,&lcol);
    lcurpop(mywin);
}

static void talk_data(data,len)
unsigned char *data;
int len;
{ /* process received data */
    int cnt,i,ch,per,len1,drive;
    struct diskfree_t diskspace;
    long left;
    decode(data,len,data);
    ch=data[0];
    len1=len-1;
    if (ch>=128 || ch==0) return; /* random shit man */
    if (ch==NAK)
    { /* my file couldn't be sent */
        abort_it=TRUE;
        problem=data[1];
        return;
    }
    if (ch==STX && !incoming)
    { /* open up file for writing ... */
        strcpy(fname,&data[1]);
        if ((ifp=fopen(fname,"rb"))!=NULL)
        { /* file exists - open temp file */
            fclose(ifp);
            if (log) fprintf(logfile,
            "\n*** File %s already exists - creating temporary file\n",fname);
            mymktemp(fname);
        }
        if (log) fprintf(logfile,"\n*** Receiving file %s\n",fname);
        cnt=1;    /* get file name length */
        while (data[cnt]!=NUL) cnt++;
        for (i=0;i<4;i++) lbs.b[i]=data[++cnt];
        iflent=lbs.p;
        _dos_getdrive(&drive);
        _dos_getdiskfree(drive,&diskspace);
        left=(long)diskspace.avail_clusters*diskspace.sectors_per_cluster*
             diskspace.bytes_per_sector;
        if (left<iflent)
        {
            file_error=TRUE;
            if (log) fprintf(logfile,
                             "\n*** File reception error\nno room for file\n");
            return;
        }
        ifp=fopen(fname,"wb");
        incoming=TRUE;
        ifcnt=0L;
        ifper=0;
        vatputs(rwin,0,15,"            ");
        vatputs(rwin,0,15,fname);
        vatputf(rwin,0,28,"%3d",ifper);
        visible(rwin,YES,NO);
        vtop(mywin);
        return;
    }
    if ((ch==ESC || ch==ETX) && incoming) 
    { /* write file data */
        if ((ifcnt+(long)len1)>iflent) len1=(int)(iflent-ifcnt);
        cnt=fwrite(&data[1],1,len1,ifp);
        if (cnt!=len1)
        { /* file write error */
            file_error=TRUE;
            incoming=FALSE;
            visible(rwin,NO,NO);
            fclose(ifp);
            remove(fname);
            if (log) fprintf(logfile,"\n*** File reception error\n");
            return;
        }
        ifcnt+=(long)(len1);
        per=100L*ifcnt/iflent;
        if (per!=ifper)
        { /* update percentage */
            ifper=per;
            vatputf(rwin,0,28,"%3d",ifper);
        }
    }
    if (ch==ETX)
    { /* end of file */
        incoming=FALSE;
        visible(rwin,NO,NO);
        fclose(ifp);
        if (ifcnt==iflent)
        { 
             if (log) fprintf(logfile,"\n*** File reception complete\n");
        }
        else 
        {
             if (log) fprintf(logfile,"\n*** File reception aborted\n");
             remove(fname);
        }
    }
    if (ch==ESC || ch==ETX) return;
    for (i=0;i<len;i++)
    { /* extract data */
        ch=data[i];
        if (ch==NUL) break;
        vdispch(yourwin,(char)ch);
        if (log) 
        {
            if (ch==CR) fputc('\n',logfile);
            else        fputc(ch,logfile);
        }
    }
}   

static int talk_idle()
{ /* collect key-strokes */
    int i,cnt,ch,num,room;
    bool wild;
    char *s,*filenames;
    if (problem)
    {
        vdispstr(errwin,"No room for file");
        visible(errwin,YES,NO);
        HANGON;
        visible(errwin,NO,NO);
        vclear(errwin);
        problem=0;
        return 0;
    }
    while (kbhit())
    { 
        ch=getkey();
        if (ch==ETX) continue;
        if (ch==ESC) forever 
        {
            temp=MNUDoSelect(mnu,NULLF);
            spin_rand();
            if (temp==NULL) return 0;
            if (temp==mfile && !file_open)
            { /* start a file transfer */
                forever
                { /* open valid file */
                    vdispstr(ofwin,"Filename: ");
                    visible(ofwin,YES,YES);
                    curdrag(ofwin,ON);
                    cnt=0;                  
                    while ((ch=getkey())!=CR)
                    { /* get file name */
                        if (ch==ESC)
                        { /* abort */
                            cnt=0;
                            break;
                        }
                        if (ch==BKSP)
                        { /* rub-out */
                            if (cnt==0) continue;
                            cnt--;
                            vdispch(ofwin,(char)ch);
                            continue;
                        }
                        if (cnt>=30 || ch>127 || ch<32) continue;
                        path[cnt++]=ch;
                        vdispch(ofwin,(char)ch);
                    }   
                    path[cnt]=0;
                    if (cnt==0) break;
                    strcpy(direct,path);
                    if (_dos_findfirst(path,_A_NORMAL,&files)!=0)
                    {  /* no such file */
                        vdispstr(errwin,"No such File");
                        visible(errwin,YES,NO);
                        HANGON;
                        visible(errwin,NO,NO);
                        vclear(errwin);
                        vclear(ofwin);
                        continue;
                    }
                    wild=FALSE;
                    while (cnt>0)
                    { /* extract directory info */
                        cnt--;
                        if (direct[cnt]=='*'  || direct[cnt]=='?') wild=TRUE;
                        if (direct[cnt]=='\\' || direct[cnt]==':')
                        {
                            cnt++;
                            break;
                        }
                        direct[cnt]='\0';
                    }
                    if (wild)
                    { /* reserve room for filenames */
                        room=strlen(files.name)+1;
                        while (_dos_findnext(&files)==0) 
                            room+=(strlen(files.name)+1);
                        filenames=calloc(room+1,1);
                        if (filenames==NULL)
                        {
                            vdispstr(errwin,"Out of Memory");
                            visible(errwin,YES,NO);
                            HANGON;
                            visible(errwin,NO,NO);
                            vclear(errwin);
                            vclear(ofwin);
                            continue;
                        }
                        _dos_findfirst(path,_A_NORMAL,&files);
                        cnt=0;
                        do {
                             i=0;
                             do {
                                 ch=files.name[i++];
                                 filenames[cnt++]=ch;
                             } while (ch!=0);
                        } while (_dos_findnext(&files)==0);

                        cnt=0;
                        first=initlist();
                        do {
                            adtolist(first,&filenames[cnt]);
                            while (filenames[cnt]!=0) cnt++;
                        } while (filenames[++cnt]!=0);
                        s=listsel(9,14,10,HIGHERR,REVHIGHERR,"Files",
                                 HIGHERR,FRSINGLE,HIGHERR,first,NULLF);
                        strcpy(filename,s);
                        freelist(first,1);
                        free(filenames);
                        if (s==NULL)
                        {
                            vclear(ofwin);
                            continue;
                        }     
                        strcpy(path,direct);
                        strcat(path,filename);
                    }
                    else strcpy(filename,&path[cnt]);
                    ofp=fopen(path,"rb");
                    oflent=filelength(fileno(ofp));
                    if (oflent==0L)
                    {
                        fclose(ofp);
                        break;
                    }
                    MNUDeleteItem(mfile);
                    mfile=MNUInsertItem("Abort File Transmission",NULL,'A',0,quit,mnu,NULLF);
                    file_open=TRUE;
                    ofcnt=0L;
                    ofper=0;
                    vatputs(twin,0,18,"            ");
                    vatputs(twin,0,18,filename);
                    vatputf(twin,0,31,"%3d",ofper);
                    visible(twin,YES,NO);
                    break;
                }
                visible(ofwin,NO,NO);
                vclear(ofwin);
                vtop(mywin);
                return 0;
            }
            if (temp==mfile && file_open)
            { /* cancel file transmission */
                abort_it=TRUE;
                return 0;
            }
            if (temp==shell)
            { /* shell to DOS */
                if (log) fclose(logfile);
                vrestore();
                spawnlp(P_WAIT,"command.com",NULL);
                vrefresh();
                while (gfkbhit()) getkey();
                if (log) logfile=fopen(logpath,"a");
                return 0;
            }
            if (temp==mlog)
            {
                log=!log;
                MNUDeleteItem(mlog);
                if (log) 
                {
                    mlog=MNUInsertItem("Log Off",NULL,'L',0,shell,mnu,NULLF);
                    visible(logwin,YES,NO);
                    fprintf(logfile,"\n*** Logging switched on\n");
                }
                else
                {
                    mlog=MNUInsertItem("Log On",NULL,'L',0,shell,mnu,NULLF);
                    visible(logwin,NO,NO);
                    fprintf(logfile,"\n*** Logging switched off\n");
                }
                vtop(mywin);
                return 0;
            }
            if (temp==quit)
            {
                if (file_open) fclose(ofp);
                if (incoming)  fclose(ifp);
                return 2;
            }
        }
        if (ch>126 || (ch!=BKSP && ch!=CR && ch<32)) continue;
        if (klen<MAXKEYS)
        {
            if (ch==BKSP)
            {
               if (linelen>0) linelen--;
               else continue;
            }
            if (linelen>=MAXLINE && ch!=CR)
            { /* line too long */
                vbeep();
                continue;
            }
            vdispch(mywin,(char)ch);
            key_buff[klen++]=ch;
            if (ch==BKSP) continue;
            if (ch==CR)
            {
                linelen=0;
                flush=TRUE;
                lrow--;
            }
            else
            { /* character is printable */
                if (linelen==(MAXLINE-10)) vbeep();
                linelen++;
            }
        }
    }
    return 0;
}

static void talk_stats(stat)
int stat;
{ /* update stats */
    stable[stat]++;
    vatputf(statwin,stat,23,"%6ld",stable[stat]);
}

static int talk_ready(data,pig,busy)
unsigned char *data;
int pig,busy;
{ /* provide data */
    int i,flen,per,new,len;
    bool end_it;
    if (message && fgets(string,BLOCK_SIZE,msg)!=NULL)
    { /* send prepared message */
        len=encode(string,BLOCK_SIZE,data);
        packet[unacked]=NO_ACK_ACTION;
        unacked++;
        return len;
    }
    if (message)
    {
        message=FALSE;
        fclose(msg);
    }  
    if (file_error)
    {  /* file reception error has occurred */
        data[0]=NAK;
        data[1]=1;
        len=encode(data,2,data);
        packet[unacked]=FILE_NAK;
        unacked++;
        file_error=FALSE;
        return len;    
    }
    new=klen;
    if (kpak>0) new-=last_len[kpak-1];
    if ((flush || new>=BLOCK_SIZE || pig!=0) && new>0)
    { /* ready data frame for transmission */
        for (i=0;i<new;i++) data[i]=key_buff[klen-new+i];
        len=new;
        if (new%BLOCK_SIZE!=0)
        { /* append a NUL to short blocks */
           data[len++]=NUL;
        }
        len=encode(data,len,data);
        flush=FALSE;
        last_len[kpak]=klen;
        packet[unacked]=KEY_STROKES;
        unacked++;
        kpak++;
        return len;
    }
    if (file_open)
    { /* send file details */
        end_it=FALSE;
        if (!outgoing)
        {  /* send file-name and length in first packet */
            outgoing=TRUE;
            data[0]=STX;
            flen=0;
            do data[flen+1]=filename[flen];
                while (filename[flen++]!=NUL);
            lbs.p=oflent;
            for (i=0;i<4;i++) data[++flen]=lbs.b[i];
            get_rand(&data[flen+1],FILEPAK-flen-1); /* pad out with random */
        }
        else
        { /* send file data */
            if (abort_it) flen=0;
            else flen=fread(&data[1],1,FILEPAK-1,ofp);
            if (flen==(FILEPAK-1)) data[0]=ESC;
            else  
            { /* send final packet - pad with random */
                end_it=TRUE;
                data[0]=ETX;
                get_rand(&data[flen+1],FILEPAK-flen-1);
            } 
            ofcnt+=(long)flen;
            per=100L*ofcnt/oflent;
            if (per!=ofper)
            { /* update percentages */
                ofper=per;
                vatputf(twin,0,31,"%3d",ofper);
            }
        }
        len=encode(data,FILEPAK,data);
        packet[unacked]=FILE_DATA;
        unacked++;
        if (end_it)
        { /* end of file transfer */
            fclose(ofp);
            outgoing=FALSE;
            visible(twin,NO,NO);
            file_open=FALSE;
            MNUDeleteItem(mfile);
            mfile=MNUInsertItem("Start File Transmission",NULL,'S',0,quit,mnu,NULLF);
            abort_it=FALSE;
        }
        return len;
    }
    if (!busy)
    {
        get_rand(data,BLOCK_SIZE);   /* if nothing better to do - transmit random shit */
        data[0]|=128;        
        len=encode(data,BLOCK_SIZE,data);
        packet[unacked]=NO_ACK_ACTION;
        unacked++;
        return len;
    }
    return 0;
}

static void talk_end()
{ /* delete all windows */
    int stat;
    fclose(logfile);
    visible(logwin,NO,NO);
    visible(twin,NO,NO);
    visible(rwin,NO,NO);
    visible(baudwin,NO,NO);
    visible(cted,NO,NO);
    vclear(callwin);
    curdrag(mywin,OFF);
    lhome(mywin);
    vclrattr(mywin,EMPHNORML);
    leeow(mywin);
    vclrattr(callwin,EMPHNORML);
    leeow(callwin);
    vclear(yourwin);
    for (stat=0;stat<8;stat++)
    vatputs(statwin,stat,23,"      ");
}

int exchange_chat(fcount,caller,mode)
int fcount,mode;
char *caller;
{
  log=OFF;
  message=FALSE;
  announce=caller;
  MNUDeleteItem(mlog);
  if (mode==3)
  {
      log=ON;
      mlog=MNUInsertItem("Log Off",NULL,'L',0,shell,mnu,NULLF);
      _searchenv("scramble.msg","PATH",path);
      if ((msg=fopen(path,"r"))!=NULL) message=TRUE;
  }    
  else mlog=MNUInsertItem("Log On",NULL,'L',0,shell,mnu,NULLF);
  MNUDeleteItem(mfile);
  mfile=MNUInsertItem("Start File Transmission",NULL,'S',0,quit,mnu,NULLF);
  return datalink(fcount,talk_init,talk_ack,talk_data,talk_idle,
                  talk_stats,talk_ready,talk_end);
}

