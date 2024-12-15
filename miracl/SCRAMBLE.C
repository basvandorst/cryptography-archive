/*   Copyright (c) 1988-1993 Shamus Software Ltd. */

#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <dw.h>
#include <dwmenu.h>
#include <miracl.h>

#define COMPLETE 1
#define LOOP 2
#define SPACE 32
#define SECRET_BITS 160     /* minimum of 160 bits */
#define LOG_BASE 16
#define HANGON while(1){if(kbhit()&&getkey()==CR)break;else;}
#define MAXKEY 40

HWND baudwin,errwin,ofwin,mywin,yourwin,win,twin,rwin,statwin,logwin,callwin;
HWND cted,idwin;
MENUHDR *mnu,*mainmnu;
MENUITEM *mfile,*shell,*quit,*temp,*mlog;

static FILE *fp;
static char licensee[100];
static char roottext[200];
static char compotext[200];
static char herlicense[100];
static char work[200];
static MENUITEM *b[5],*d[2],*p[3],*m[4];
static HWND dialwin,waitwin,modemwin;
static big root,compo,mypublic,myprivate,herpublic,key,licensenum,secret_num;
static long baud_rates[5]={300,1200,2400,4800,9600};
static unsigned int mode,dialmode,port,baud;
static char number[30];
static char *fixed={"AUTHORISED PC-SCRAMBLER USER -> "};
static char validchars[]= " 0123456789/,@wWrRpPtT()-;\b";

static char *advert[]={
"\n",
" Welcome to PC-SCRAMBLER, which is bad news for Phone Tappers,  as it allows\n",
" you to converse and swap files with distant associates in complete privacy,\n", 
" using State-of-the-Art ideas from Modern Cryptography.  No need to remember\n",
" a Password  -  the keys are generated automatically.  The  identity of your\n", 
" partner is even verified by PC-SCRAMBLER.\n",
"\n",
" Setting up a secure link involves some  rather  difficult  and  potentially\n",
" time-consuming calculations.  The faster your PC, the shorter these delays.\n",
"\n",
" This product is SHAREWARE and dedicated to that fine  body of (mostly) men,\n", 
" the Irish Special Branch and their equivalents the world over who regularly\n", 
" tap the telephones of journalists and political activists.\n",
"\n",
" This copy of PC-SCRAMBLER is licensed to:-\n",
"\n",
"\n",
"\n",
"                        Press RETURN to continue\n",
"\n",
"\n",
"       (C) Copyright Shamus Software Ltd 1988-1993, Dublin, Ireland"};

static bool proceed()
{
   int ch;
   forever
   {
       if (kbhit())
       {
           ch=getkey();
           if (ch==CR) return TRUE;
           if (ch==ESC) return FALSE;
       }
   }
}

static int secret(buff,maxlen)
char *buff;
int maxlen;
{ /* get secret string */
    int cnt,ch;
    cnt=0;
    while ((ch=getch())!=CR && cnt<maxlen)
    { /* until RETURN pressed */
        if (ch==ESC)
        { /* ESC */
            cnt=0;
            break;
        }
        if (ch==127 || ch==8)
        { /* delete */
            if (cnt==0) continue;
            cnt--;
            continue;
        }
        buff[cnt++]=ch;
    }
    buff[cnt]=0;
    return cnt;
}

static void onewayhash(string,n,x)
char *string;      
big n,x;
{ /* nonlinear (portable) permutation *
   * x=(fixed|string xor R(n))^2 mod n      */
    int i,len;
    char t;
    static char buff[256];
    strcpy(work,fixed);
    strcat(work,string);
    INPLEN=otstr(n,buff);
    for (i=0;i<INPLEN/2;i++)
    { /* swap bytes of buff */
        t=buff[i];
        buff[i]=buff[INPLEN-i-1];
        buff[INPLEN-i-1]=t;
    }
    len=strlen(work);
    for (i=0;i<len;i++) buff[i]^=work[i];
    instr(x,buff);
    mad(x,x,x,n,n,x);
}

static void newkey()
{
    int nd,ch;
    INPLEN=SECRET_BITS/8;
    get_rand(MR_IOBUFF,INPLEN);
    instr(myprivate,MR_IOBUFF);
    visible(waitwin,YES,NO);
    vdispstr(waitwin,"Calculating public encryption key......");

/*  mypublic = root*LOG_BASE^myprivate mod compo */

    powltr(LOG_BASE,myprivate,compo,mypublic);
    mad(mypublic,root,root,compo,compo,mypublic);
    visible(waitwin,NO,NO);
    vclear(waitwin);
}

static int connect(p)
MENUITEM *p;
{
    int i,ch,cnt,k,t;
    bool valid;
    static char mybuff[256],herbuff[256];
    newkey();
    if (mode==1)
    { /* get dial string */
        vdispstr(dialwin,"Phone Number: ");
        vdispstr(dialwin,number);
        curdrag(dialwin,ON);
        visible(dialwin,YES,YES);

        cnt=strlen(number);                  
        while ((ch=getkey())!=CR && ch!=ESC)
        { /* get phone number */
            if (ch==0)
            { /* ignore extended codes */
                getkey();
                continue;
            }
            for (i=0;;i++)
            { /* check for valid characters */
                valid=TRUE;
                t=validchars[i];
                if (ch==t) break;
                valid=FALSE;
                if (t==0) break;
            }
            if (!valid) continue;
            if (ch=='\b')
            { /* backspace */
                if (cnt==0) continue;
                cnt--;
            }
            else 
            {
                if (cnt>25) continue;
                number[cnt++]=ch;
            }
            vdispch(dialwin,(char)ch);
        }
        spin_rand();   
        number[cnt]=0;
        visible(dialwin,NO,NO);
        vclear(dialwin);
        if (ch==ESC || cnt==0) return 0;
    }
    forever
    {  /* Make Modem Connection */
        start_link(port,baud_rates[baud],mode);
        if (mode==1) dial(number,dialmode);
        vdispstr(waitwin,"Waiting for a modem connection......");
        if (mode==0 || mode==2) do
        { /* make good modem connection */
            visible(modemwin,YES,NO);
            if (!proceed()) 
            { /* abort it */
                vclear(waitwin);
                visible(waitwin,NO,NO);
                visible(modemwin,NO,NO);
                end_link();
                return 0;
            }
            spin_rand();
            visible(modemwin,NO,NO);
            visible(waitwin,YES,NO);
        } while (!modem_link());
        if (mode==1 || mode==3)
        { 
            visible(waitwin,YES,NO);
            if (!modem_link())
            {
                vclear(waitwin);
                visible(waitwin,NO,NO);
                end_link();
                return 0;
            }
        }
        vclear(waitwin);
        vbeep();

/* Okamoto's exponential key exchange */

        vdispstr(waitwin,"Communicating public encryption key......");
        cotstr(mypublic,mybuff);
        if (exchange_strings(5,mybuff,herbuff,256)!=COMPLETE)  
        {  /* there is a problem... */
            vclear(waitwin);
            vdispstr(waitwin,"Sorry - there is a communications problem");
            end_link();
            visible(waitwin,NO,NO);
            vclear(waitwin);
            if (mode!=3) return 0;
            continue;
        }
        cinstr(herpublic,herbuff);
        spin_rand();
        exchange_strings(0,licensee,herlicense,100);
        if (port!=LOOP &&  strcmp(licensee,herlicense)==0)
        { /* don't allow talking to yourself, unless in loopback */
            vclear(waitwin);
            vdispstr(waitwin,"Sorry - Can't talk to yourself!");
            end_link();
            visible(waitwin,NO,NO);
            vclear(waitwin);
            if (mode!=3) return 0;
            newkey();
            continue;
        }
        vclear(waitwin);
        vdispstr(waitwin,"Calculating private encryption key......");

/*  key = (herpublic^3*herID)^myprivate mod compo */

        power(herpublic,3,compo,herpublic);
        onewayhash(herlicense,compo,licensenum);
        mad(herpublic,licensenum,licensenum,compo,compo,herpublic);
        powmod(herpublic,myprivate,compo,key);

 /* use key to initialise one-time pad */

        cotstr(key,MR_IOBUFF);
        if (port==LOOP) set_pad(MR_IOBUFF,MR_IOBUFF,0);
        else            set_pad(MR_IOBUFF,&MR_IOBUFF[32],mode);  
        zero(key);
        zero(licensenum);

        exchange_strings(0,fixed,work,200);
        if (strcmp(fixed,work)!=0)
        { /* key exchange has failed - keys are different */
            vclear(waitwin);
            vdispstr(waitwin,"Sorry - Key negotiation has failed"); 
            end_pad();
            end_link();
            visible(waitwin,NO,NO);
            vclear(waitwin);
            if (mode!=3) return 0;
            newkey();
            continue;
        }

        visible(waitwin,NO,NO);
        vclear(waitwin);
        spin_rand();
        exchange_chat(0,herlicense,mode);

        end_pad();
        end_link();
        if (mode!=3) break;
        newkey();
    }
    return 0;
}

static int smode(p)
MENUITEM *p;
{ /* mode select function */
    int i;
    MNUUnCheckLevel(p);
    MNUToggleCheck(p);
    for (i=0;i<4;i++) if (p==m[i]) mode=i;
    return 0;
}

static int sbaud(p)
MENUITEM *p;
{ /* baud select function */
    int i;
    MNUUnCheckLevel(p);
    MNUToggleCheck(p);
    for (i=0;i<5;i++) if (p==b[i]) baud=i;
    return 0;
}

static int sport(m)
MENUITEM *m;
{ /* port select function */
    int i;
    MNUUnCheckLevel(m);
    MNUToggleCheck(m);
    for (i=0;i<3;i++) if (m==p[i]) port=i;
    return 0;
}

static int sdial(p)
MENUITEM *p;
{ /* dial mode select function */
    int i;
    MNUUnCheckLevel(p);
    MNUToggleCheck(p);
    for (i=0;i<2;i++) if (p==d[i]) dialmode=i;
    return 0;
}

void main(argc,argv)
int argc;
char **argv;
{
    MENUITEM *orig, *ans, *mbaud, *tfile, *temp=0;
    MENUITEM *mconnect,*mainquit,*mport,*mdial;
    static char kstname[20],path[128],pass[MAXKEY+1];
    int i,ch,len,liclen;
    long lr;
    unsigned short *bucket;
    int cursorpos;

    mirsys(200,256); 
    root=mirvar(0);
    compo=mirvar(0);
    mypublic=mirvar(0);
    herpublic=mirvar(0);
    myprivate=mirvar(0);
    licensenum=mirvar(0);
    secret_num=mirvar(5);
    key=mirvar(0);
    IOBASE=60;

    argv++;
    argc--;
    if (argc==1) strcpy(kstname,argv[0]);
    else         strcpy(kstname,"scramble");
    strcat(kstname,".kst");
    _searchenv(kstname,"PATH",path);
    if ((fp=fopen(path,"r"))!=NULL)
    { /* read in key data */
        fgets(licensee,100,fp);
        liclen=strlen(licensee)-1;
        licensee[liclen]='\0';  /* remove CR */
        ch=fscanf(fp,"%s%s",roottext,compotext);
        fclose(fp);
    }
    else
    {
        printf("File %s does not exist\n",kstname);
        exit(0);
    }
    if (ch!=2 || strlen(compotext)<80 || strlen(roottext)<80)
    { /*  invalid file */
        printf("Invalid .kst file format\n"); 
        exit(0);
    }
    cinstr(root,roottext);
    cinstr(compo,compotext);
    power(root,3,compo,key);
    onewayhash(licensee,compo,licensenum);

    if (compare(key,licensenum)!=0)
    { /* bad ID - is it clamped?*/
        printf("Key file appears to be clamped\nPassword: ");
        if ((len=secret(pass,MAXKEY))<8)
        { /* get secret password */
            printf("Password is incorrect\n");
            exit(0);
        }
        INPLEN=len;
        instr(secret_num,pass);
        for (i=0;i<=MAXKEY;i++) pass[i]=0;
        xgcd(secret_num,compo,secret_num,secret_num,secret_num);
        power(secret_num,64,compo,secret_num);
        mad(root,secret_num,root,compo,compo,root);
        power(root,3,compo,key);
        if (compare(key,licensenum)!=0)
        {
            printf("Password is incorrect\n");
            exit(0);
        } 
    }
    xgcd(root,compo,root,root,root);  /* root = 1/root */
    start_rand(secret_num,compo);
    zero(secret_num);
    bucket=(unsigned short *)calloc((prows()*pcols()*2),1);
    SaveScreen(&bucket,&cursorpos);

    tocolor();
    if (iscolor()) setdwlib(1,3,30,NO);
    else           setdwlib(0,7,30,NO);

    pclrattr(REVEMPHNORML);
    if (mousinst()) ctlmscur(OFF);
    win=vcreat(23,80,NORML,NO);
    vautofrm(win,NORML,FRDOUBLE);
    vlocate(win,1,1);
    vtitle(win,NORML,"PC-Scrambler V6.0");

    for (i=0;i<22;i++) vdispstr(win,advert[i]);
    visible(win,YES,NO);

    idwin=vcreat(1,36,REVEMPHNORML,NO);
    vlocate(idwin,17,19);
    for (i=0;i<(36-liclen)/2;i++) vdispch(idwin,SPACE);
    vdispstr(idwin,licensee);
    visible(idwin,YES,NO);

    waitwin=vcreat(1,64,ERR,NO);
    vautofrm(waitwin,ERR,FRSINGLE);
    vlocate(waitwin,8,3);
    vtitle(waitwin,ERR,"Please wait");
    dialwin=vcreat(1,45,HIGHERR,NO);
    vautofrm(dialwin,HIGHERR,FRSINGLE);
    vtitle(dialwin,HIGHERR,"Edit Dial String and Press RETURN");
    vlocate(dialwin,8,9);
    modemwin=vcreat(1,64,MARK,NO);
    vautofrm(modemwin,MARK,FRSINGLE);
    vlocate(modemwin,8,3);
    vtitle(modemwin,MARK,"Make Modem Connection");
    vdispstr(modemwin,"Connect modems - Press RETURN when ready - then replace handset");

    mainmnu=MNUCreateHdr(POPUP);
    MNUSetAttributes(mainmnu,REVHIGHNORML,REVNORML,REVNORML,ERR, 
                             HIGHNORML,HIGHHELP,REVHIGHNORML,EMPHNORML);
    MNUSetCheckMark(mainmnu,3,CURRENT);
    MNUSetDisplayPosition(mainmnu,1,40);
    MNUSetPopUpDisplacement(mainmnu,CURRENT,CURRENT);
    MNUSetSpaces(mainmnu,2);

    mconnect=MNUAddItem("Connect",NULL,'C',0,NULL,mainmnu,connect);
    MNUSetRepaint(mconnect,NO);
    MNUSetReturn(mconnect,OFF,NO);
    m[0]= MNUAddItem("Originate - Manual",NULL,'O',0,NULL,mainmnu,smode);
    m[1]= MNUAddItem("Originate - Autodial",NULL,'d',0,NULL,mainmnu,smode);
    m[2]= MNUAddItem("Answer    - Manual",NULL,'A',0,NULL,mainmnu,smode);
    m[3]= MNUAddItem("Answer    - Autoanswer",NULL,'a',0,NULL,mainmnu,smode);
    for (i=0;i<4;i++) MNUSetReturn(m[i],OFF,NO);
    MNUSetSeparatorBefore(m[0]);

    mbaud= MNUAddItem("Baud-rate",NULL,'B',0,NULL,mainmnu,NULLF);
    MNUSetSeparatorBefore(mbaud);
    b[0]= MNUAddItem("300",NULL,'3',0,mbaud,mainmnu,sbaud);
    b[1]= MNUAddItem("1200",NULL,'1',0,mbaud,mainmnu,sbaud);
    b[2]= MNUAddItem("2400",NULL,'2',0,mbaud,mainmnu,sbaud);
    b[3]= MNUAddItem("4800",NULL,'4',0,mbaud,mainmnu,sbaud);
    b[4]= MNUAddItem("9600",NULL,'9',0,mbaud,mainmnu,sbaud);
    for (i=0;i<5;i++) MNUSetReturn(b[i],OFF,NO);

    mdial=MNUAddItem("Dial Mode",NULL,'M',0,NULL,mainmnu,NULLF);
    d[0]= MNUAddItem("Pulse",NULL,'P',0,mdial,mainmnu,sdial);
    d[1]= MNUAddItem("Tone",NULL,'T',0,mdial,mainmnu,sdial);
    for (i=0;i<2;i++) MNUSetReturn(d[i],OFF,NO);

    mport= MNUAddItem("Comms Port",NULL,'P',0,NULL,mainmnu,NULLF);
    p[0]= MNUAddItem("COM 1",NULL,'1',0,mport,mainmnu,sport);
    p[1]= MNUAddItem("COM 2",NULL,'2',0,mport,mainmnu,sport);
    p[2]= MNUAddItem("LOOP ",NULL,'L',0,mport,mainmnu,sport);
    for (i=0;i<3;i++) MNUSetReturn(p[i],OFF,NO);

    mainquit= MNUAddItem("Quit Scrambler",NULL,'Q',0,NULL,mainmnu,NULLF);
    MNUSetSeparatorBefore(mainquit);

    mnu=MNUCreateHdr(POPUP);
    MNUSetAttributes(mnu,REVHIGHNORML,REVNORML,REVNORML,ERR, 
                         HIGHNORML,HIGHHELP,REVHIGHNORML,EMPHNORML);
    MNUSetDisplayPosition(mnu,5,4);
    quit= MNUAddItem("Quit Communication",NULL,'Q',0,NULL,mnu,NULLF);
    mfile=MNUAddItem("Start File Transmision",NULL,'S',0,NULL,mnu,NULLF);
    shell=MNUAddItem("DOS Shell",NULL,'D',0,NULL,mnu,NULLF);
    mlog=MNUAddItem("Log On",NULL,'L',0,NULL,mnu,NULLF);

    yourwin=vcreat(12,38,EMPHNORML,YES);
    vautofrm(yourwin,REVNORML,FRDOUBLE);
    vtitle(yourwin,REVNORML,"You");
    vlocate(yourwin,2,1);

    mywin=vcreat(8,38,REVEMPHNORML,YES);
    vautofrm(mywin,REVNORML,FRDOUBLE);
    vtitle(mywin,REVNORML,"Me");
    vlocate(mywin,16,1);
    vclrattr(mywin,EMPHNORML);
    leeow(mywin);


    ofwin=vcreat(1,41,HIGHERR,NO);
    vautofrm(ofwin,HIGHERR,FRSINGLE);
    vtitle(ofwin,HIGHERR,"File Transmission");
    vlocate(ofwin,6,2);

    baudwin=vcreat(1,5,REVNORML,NO);
    vlocate(baudwin,9,65);

    errwin=vcreat(1,30,ERR,NO);
    vautofrm(errwin,ERR,FRSINGLE);
    vtitle(errwin,ERR,"Press ESC to continue");
    vlocate(errwin,6,5);

    twin=vcreat(1,35,MARK,NO);
    vlocate(twin,0,0);
    vdispstr(twin,"Transmitting file                0%");

    rwin=vcreat(1,33,HIGHERR,NO);
    vlocate(rwin,0,35);
    vdispstr(rwin,"Receiving file                0%");

    statwin=vcreat(8,29,REVNORML,NO);
    vautofrm(statwin,REVNORML,FRSINGLE);
    vtitle(statwin,REVNORML,"Link Statistics");
    vlocate(statwin,16,41);
    vdispstr(statwin,"Data Frames sent\n");
    vdispstr(statwin,"Data Frames Received\n");
    vdispstr(statwin,"Naks Sent\n");
    vdispstr(statwin,"Naks Received\n");
    vdispstr(statwin,"Acks Sent\n");
    vdispstr(statwin,"Acks Received\n");
    vdispstr(statwin,"Control Frames Sent\n");
    vdispstr(statwin,"Time-Outs");

    logwin=vcreat(1,3,REVNORML,NO);
    vlocate(logwin,0,68);
    vdispstr(logwin,"Log");

    callwin=vcreat(1,36,REVEMPHNORML,NO);
    vautofrm(callwin,NORML,FRSINGLE);
    vtitle(callwin,NORML,"Your Authenticated Identity:");
    vlocate(callwin,3,2);
    vclrattr(callwin,EMPHNORML);
    leeow(callwin);

    cted=vcreat(1,22,REVNORML,NO);
    vlocate(cted,2,44);
    vdispstr(cted,"\nConnected");

    HANGON;
    spin_rand();
    _searchenv("scramble.ini","PATH",path);
    if ((fp=fopen(path,"r"))!=NULL)
    { /* get previous status */
        baud=fgetc(fp)-48;
        port=fgetc(fp)-48;
        mode=fgetc(fp)-48;
        dialmode=fgetc(fp)-48;
        fgetc(fp); /* get CR */
        i=0;
        while ((ch=fgetc(fp))!=EOF && ch!=0) number[i++]=ch;
        number[i]=0;
        fclose(fp);
    }
    else 
    {
        baud=2;
        port=0;
        mode=0;
        dialmode=0;
        number[0]=0;
    }

    if (port<3) MNUToggleCheck(p[port]);
    if (mode<4) MNUToggleCheck(m[mode]);
    if (dialmode<2) MNUToggleCheck(d[dialmode]);
    if (baud>4) baud=2;
    if (mode>3) mode=3;
    MNUToggleCheck(b[baud]);

    start_comms(baud_rates[baud]);

    vdelete(idwin,NONE);
    vdelete(win,NONE);  

    visible(yourwin,YES,NO);
    visible(mywin,YES,NO);
    visible(statwin,YES,NO);
    visible(callwin,YES,NO);

    do {
        spin_rand();
        temp=MNUDoSelect(mainmnu,NULLF);
    } while (temp!=mainquit);

    vclean();
    vdelete(logwin,NONE);
    vdelete(statwin,NONE);
    vdelete(ofwin,NONE);
    vdelete(twin,NONE);
    vdelete(rwin,NONE);
    vdelete(callwin,NONE);
    vdelete(mywin,NONE);
    vdelete(yourwin,NONE);
    vdelete(errwin,NONE);
    vdelete(baudwin,NONE);
    vdelete(cted,NONE);
    zero(root);
    zero(myprivate);
    end_comms();

  /* update status file */
    save_rand();
    mirexit();
    if (path[0]==0) fp=fopen("scramble.ini","w");
    else            fp=fopen(path,"w");
    fputc(48+baud,fp);
    fputc(48+port,fp);
    fputc(48+mode,fp);
    fputc(48+dialmode,fp);
    fputc(LF,fp);
    i=0;
    do 
    { /* put out phone number */
       ch=number[i++];
       fputc(ch,fp);
    } while (ch!=0);
    fclose(fp);
    RestoreScreen(&bucket,&cursorpos);
    pcuron();
}

