/* Secure Drive V1.3A */
/* Code common to CRYPTDSK and LOGIN */

#include "secdrv.h"

struct tsrdata far *cryptdata;

char pass1[MAXPASS];
char pass2[MAXPASS];
char Old13Check[4];

int  tsr_not_installed=0;
int  tsr_wrong_version=0;
int  df10=1,ef10=1;


char compat_mode='N';


void setrldb(void far *real_dbios_vect);

void set_compat_mode(void)
{
 char *p;
 if ((p=getenv("SD10CMP")) != NULL)
 compat_mode = toupper(p[0]);
 if ( compat_mode == 'Y' || compat_mode == 'X')
  {
   printf("\nVersion 1.0 %s Compatibility is Active\n",
    compat_mode == 'X' ? "eXclusive" : "Normal");
  }
 else compat_mode='N';
}

void invgets(char *input)
{
unsigned i=0;
char c;
while(i<(MAXPASS-1)) {
    c=getch();
    if(c=='\x0d')
        {
        printf("\n");
        *input='\0';
        return;
        }
    else if(c=='\b')
        {
        if(i>0)
            {
            i--;
            input--;
            printf("\b \b");
            }
        }
    else
        {
        printf("*");
        *input++=c;
        i++;
        }
    }
}

void readsec(unsigned drive,unsigned head,unsigned track,
             unsigned sector,unsigned nsects,void *buffer)
{
unsigned i;
char c;
for(;;) {
    for(i=0;i<3;i++)
        if(!rldbios(2,drive,head,track,sector,nsects,buffer)) return;
    printf("\nRead error: drive %02x, head %u, track %u\n",
            drive,head,track);
    printf("Abort, Retry, Ignore? ");
    c=toupper(getch()); printf("%c\n",c);
    if(c=='I') return;
    if(c=='A') exit(1);
    }
}

void writesec(unsigned drive,unsigned head,unsigned track,
              unsigned sector,unsigned nsects,void *buffer)
{
unsigned i;
char c;
unsigned r;
for(;;) {
    for(i=0;i<3;i++)
     {
        r=rldbios(3,drive,head,track,sector,nsects,buffer);
        if(!r) return;
     }
    if (r == 03 && drive<0x80)
     {
      printf("\nERROR: Disk %c is Write-Protected!\n",drive+'A');
     }
    else
     printf("\nWrite error: drive %02x, head %u, track %u\n",
            drive,head,track);
    printf("Abort, Retry, Ignore? ");
    c=toupper(getch()); printf("%c\n",c);
    if(c=='I') return;
    if(c=='A') exit(1);
    }
}

int getyn(void)
{
char c;
for(;;)
    {
    c=getch();
    if(c=='y'||c=='Y') { printf("Yes\n\n"); return(TRUE); }
    if(c=='n'||c=='N') { printf("No\n\n"); return(FALSE); }
    }
}

void getkeydf(unsigned char *key,unsigned char *check,unsigned char *bufck)
{
unsigned char temp[16];
MD5_CTX md5buf;
unsigned k;

invgets(pass1);
setkeydf(key,check,bufck);
}

void getkey(unsigned char *key,unsigned char *check,int confirm)
{
unsigned char temp[16];
MD5_CTX md5buf;
unsigned k;

if(confirm) printf("\
You need a passphrase to encrypt this disk. Your passphrase should be\n\
fairly long, and should not appear verbatim in any literature or text.\n\
Passphrases are case sensitive - this can be a security feature or it\n\
can be a hazard. If you lose or forget your passphrase, there is no way\n\
to recover the encrypted data!\n");
enterpass:
if(confirm) printf("\nEnter passphrase: ");
invgets(pass1);
if(confirm) {
   int good=TRUE;
   int upper=FALSE;
   int lower=FALSE;
   int other=FALSE;
   unsigned i,j;
   char c;
   printf("\n");
   i=strlen(pass1);
   if(i<8) {
       printf("Your passphrase is only %i characters long.\n",i);
       good=FALSE; }
   for(j=0;j<i;j++) {
       c=pass1[j];
       if(isupper(c)) upper=TRUE;
       else if (islower(c)) lower=TRUE;
       else if (!isspace(c)) other=TRUE; }
   if(upper&&!lower&&!other) {
       printf("Your passphrase is monocase (uppercase only).\n");
       good=FALSE; }
   if(lower&&!upper&&!other) {
       printf("Your passphrase is monocase (lowercase only).\n");
       good=FALSE; }
   if(i>15) good=TRUE;
   if(!good) printf("\nThis passphrase may not be secure.\
 If you want to use it, retype it to\nconfirm. If not, press return to\
 try again.\n\n");
   printf("Re-enter passphrase: ");
   invgets(pass2);
   printf("\n");
   if(strcmp(pass1,pass2)) {
       printf("Passphrases do not match. Try again.\n");
       goto enterpass; }
   }
setkeye(key,check);
}

void setkeye (unsigned char *key, unsigned char *check)
{
  ef10=(compat_mode != 'N');
  if (ef10)
   setkey10(key,check);
  else
   setkey13(key,check);
}


void setkeydf(unsigned char *key,unsigned char *check,unsigned char *bufck)
{
 memset(Old13Check,0xff,4);
 setkey10(key,check);
 if(memcmp(check,bufck,4) == 0)
  {
   df10=1;
   return;
  }
 df10=0;
 if (compat_mode == 'X')
  return;
 setkey13(key,check);
}

void setkey10(unsigned char *key,unsigned char *check)
 {
  unsigned char temp[16];
  MD5_CTX md5buf;

  MD5Init(&md5buf);
  MD5Update(&md5buf,pass1,strlen(pass1));
  MD5Final(key,&md5buf);

  MD5Init(&md5buf);
  MD5Update(&md5buf,key,16);
  MD5Final(temp,&md5buf);
  memcpy(check,temp,4);
 }

void setkey13(unsigned char *key,unsigned char *check)
 {
  unsigned char temp[16];
  int k;
  MD5_CTX md5buf;
  MD5_CTX md5sav;

  MD5Init(&md5buf);
  MD5Update(&md5buf,pass1,strlen(pass1));
  MD5Final(key,&md5buf);

  for(k=0;k<PASS_ITER;k++)
   {
    MD5Init(&md5buf);
    MD5Update(&md5buf,key,16);
    MD5Update(&md5buf,pass1,strlen(pass1));
    MD5Final(key,&md5buf);
   }

  MD5Init(&md5buf);
  MD5Update(&md5buf,key,16);
  memcpy((void *)&md5sav,(void *)&md5buf,sizeof(md5buf));
  MD5Update(&md5buf,pass1,strlen(pass1));
  MD5Final(temp,&md5buf);
  memcpy(check,temp,4);               /*1.1/1.3A  Check*/
  memcpy((void *)&md5buf,(void *)&md5sav,sizeof(md5buf));
  MD5Final(temp,&md5buf);             /*1.3 Check*/
  memcpy(Old13Check,temp,4);
 }

struct tsrdata far *gettsradr(void)
{
unsigned seg;
unsigned ofs;
struct tsrdata far *ptr;
struct REGPACK rgs;

rgs.r_ax=0x0800;
rgs.r_dx=0x00f0;
intr(0x13,&rgs);
if(rgs.r_ax==0x0edcb)
 {
  tsr_wrong_version=1;
  return( (struct tsrdata far *) NULL);
 }
if(rgs.r_ax!=0x0edcc)
 {
  tsr_not_installed=1;
  return( (struct tsrdata far *) NULL);
 }
/* ptr=(long) rgs.r_dx+(long) 0x10000*rgs.r_cx; */
ptr = MK_FP(rgs.r_cx,rgs.r_dx);
if (memcmp(ptr->tsrver,"130A",4) != 0)
 {
  tsr_wrong_version=1;
  return( (struct tsrdata far *) NULL);
 }
setrldb(&ptr->real_dbios_vect);
return(ptr);
}

void readptbl(unsigned char drvltr,unsigned *ptdrive,         /*1.1*/
              unsigned *pthead,unsigned *pttrack)             /*1.1*/
{                                                             /*1.1*/
unsigned char buf[512];                                       /*1.1*/
*ptdrive=255;                                                 /*1.1*/
rdptx(0,0,0,&drvltr,ptdrive,pthead,pttrack,FALSE,buf);        /*1.1*/
if(*ptdrive!=255) return;                                     /*1.1*/
else rdptx(1,0,0,&drvltr,ptdrive,pthead,pttrack,FALSE,buf);   /*1.1*/
if(*ptdrive!=255) return;                                     /*1.1*/
else {                                                        /*1.1*/
    drvltr++;                                                 /*1.1*/
    rdptx(0,0,0,&drvltr,ptdrive,pthead,pttrack,TRUE,buf);     /*1.1*/
     }                                                        /*1.1*/
if(*ptdrive!=255) return;                                     /*1.1*/
else {                                                        /*1.1*/
    drvltr++;                                                 /*1.1*/
    rdptx(1,0,0,&drvltr,ptdrive,pthead,pttrack,TRUE,buf);     /*1.1*/
     }                                                        /*1.1*/
}                                                             /*1.1*/

void rdptx(unsigned drive,unsigned head,unsigned track,       /*1.1*/
           unsigned char *letter, unsigned *ptdrive,          /*1.1*/
           unsigned *pthead,unsigned *pttrack,int recurse,    /*1.1*/
           unsigned char *buf)                                /*1.1*/
{                                                             /*1.1*/
unsigned i,pt,ph,pc;                                          /*1.1*/
unsigned char *bufp;                                          /*1.1*/
if(rldbios(2,drive+0x80,head,track,1,1,buf)) return;          /*1.1*/
for(i=0;i<4;i++) {                                            /*1.1*/
    bufp=buf+0x1be+(i*16);                                    /*1.1*/
    pt=bufp[4];                                               /*1.1*/
    ph=bufp[1];                                               /*1.1*/
    pc=bufp[3]+(bufp[2]>>6)*256;                              /*1.1*/
    if((pt==1)||(pt==4)||(pt==6)) {                           /*1.1*/
        if(*letter=='C') {                                    /*1.1*/
            *ptdrive=drive;                                   /*1.1*/
            *pthead=ph;                                       /*1.1*/
            *pttrack=pc;                                      /*1.1*/
            return; }                                         /*1.1*/
        else {                                                /*1.1*/
           (*letter)--;                                       /*1.1*/
           if(!recurse) return; } }                           /*1.1*/
    else if(pt==5) rdptx(drive,ph,pc,letter,ptdrive,          /*1.1*/
                         pthead,pttrack,TRUE,buf); }          /*1.1*/
}                                                             /*1.1*/

/*      Compute IDEA encryption subkeys Z */
void en_key_idea(word16 *userkey, word16 *Z)
{
        unsigned i,j;
        word16 *Y=Z;
        /*
         * shifts
         */
        for (j=0; j<8; j++)
                Z[j] = *userkey++;

        for (i=0; j<KEYLEN; j++)
        {       i++;
                Z[i+7] = Z[i & 7] << 9 | Z[i+1 & 7] >> 7;
                Z += i & 8;
                i &= 7;
        }
        for(i=0;i<52;i++)
                Y[i]^=0x0dae;
}        /* en_key_idea */

void calcdiskparams(unsigned char *buf,unsigned *maxtrack,
                    unsigned *maxhead,unsigned *maxsector,
                    unsigned *secsize,unsigned serial[2])
{
unsigned long i,l0,l1,l2,l3;
*maxsector=buf[0x18]+256*buf[0x19];
*maxhead=buf[0x1a]+256*buf[0x1b];
*secsize=buf[0x0b]+256*buf[0x0c];
serial[0]=buf[0x27]+256*buf[0x28];
serial[1]=buf[0x29]+256*buf[0x2a];
l0=(unsigned char)buf[0x20];
l1=(unsigned char)buf[0x21];
l2=(unsigned char)buf[0x22];
l3=(unsigned char)buf[0x23];
i=l0+256*(l1+256*(l2+256*(l3)));
if(i==0) i=(unsigned)buf[0x13]+256*buf[0x14];
*maxtrack=i / *maxsector / *maxhead;
if((i%(*maxsector * *maxhead))==0) (*maxtrack)--;
}

void clrbufs(void)
 {
  int k;
  for(k=0;k<MAXPASS;k++)
  {
    pass1[k]='\0';
    pass2[k]='\0';
  }
 }
