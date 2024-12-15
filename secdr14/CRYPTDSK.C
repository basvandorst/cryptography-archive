/* Secure Drive CRYPTDSK V1.4 */
/* Encrypts/decrypts disks */

#include "secdrv.h"

extern char pass1[MAXPASS];
extern char pass2[MAXPASS];
extern char filekey[16];
extern int  fkset,fkapply;
extern char Old13Check[4];

extern int  tsr_not_installed;
extern int  tsr_wrong_version;
extern int  df10,ef10;
extern int  incfo, outcfo;  /*1.3c*/
extern char compat_mode;

void readseca(unsigned drive,unsigned head,unsigned cyl,unsigned sector,
     unsigned nsects,unsigned secsize,unsigned char *buffer);
void writeseca(unsigned drive,unsigned head,unsigned cyl,unsigned sector,
     unsigned nsects,unsigned secsize,unsigned char *buffer);
void freebuf(void);
void clrbufs(void);

unsigned char *buf;
char *pgpv;
int c_break_hit=0;                        /*1.4*/

int c_break_int()                         /*1.4*/
 {                                        /*1.4*/
  c_break_hit=1;                          /*1.4*/
  return(1);  /*continue program*/        /*1.4*/
 }                                        /*1.4*/

int main()
{
unsigned drive,firstcyl,firsthead,encrypt,decrypt;            /*1.1*/
unsigned cyl,head,sector,maxcyl,maxhead,maxsector,secsize,t;
unsigned serial[2];
unsigned char dkey[16],dcheck[4],iv[8],tiv[8];                /*1.1*/
unsigned char ekey[16],echeck[4];                             /*1.1*/
unsigned char *bufptr,*dummy;
unsigned i,iz,hdx;
int sethdsafe=0;
int dkeyexp=0;
int ekeyexp=0;

char drvltr,r;                                                /*1.1*/
word16 dexpkey[52],eexpkey[52];                               /*1.1*/

clrscr();


if(!(buf=malloc(512))) {
    printf("Error: unable to allocate memory for sector buffer.\n");
    exit(1); }

atexit(freebuf);

printf("\
Secure Drive CryptDisk Version 1.4\n\
\n\
This program will encrypt or decrypt a floppy disk or hard drive\
 partition.\n");

pgpv=getenv("PGPPASS");
set_compat_mode();

cryptdata=gettsradr();

if(tsr_wrong_version)
 {
  printf ("\nERROR: Wrong Version (not 1.4) of SECTSR is Installed\n");
  exit(1);
 }

if (tsr_not_installed)
  printf("Warning: SECTSR not installed.\n");

read_filekey();

askdrive:
printf("\nEnter the letter of the drive to process, or X to enter the\n\
drive, cylinder, and head manually, or Z to cancel: ");
while(!isalpha(drvltr=toupper(getch())));
printf("%c\n",drvltr);


if(drvltr=='A') { firstcyl=0; firsthead=0; drive=0; }
else if(drvltr=='B') { firstcyl=0; firsthead=0; drive=1; }

else if(drvltr=='X') {
    printf("\nEnter physical drive (0,1,2,...), cylinder, and head for the\
 beginning\n\(boot sector) of this partition: ");
    scanf("%u,%u,%u",&drive,&firstcyl,&firsthead);
    drive+=0x80;
    }

else if(drvltr=='Z') { printf("\n"); exit(0); }

else {
    drive=255;
    /*find disk params if previously set by LOGIN   1.4*/
    if (!tsr_not_installed)
     {
      for (i=0;i<MAXDRV;i++)
       {
         if (  cryptdata->hd[i].dddrv    != 0  &&
               cryptdata->hd[i].drvltr   == drvltr )
          {
           drive=cryptdata->hd[i].dddrv - 0x80;
           firstcyl=cryptdata->hd[i].firstcyl;
           firsthead=cryptdata->hd[i].firsthd;
          }
       }
     }

    if(drive==255)
     readptbl(drvltr,&drive,&firsthead,&firstcyl);  /*1.1*/
    if(drive==255) {
        printf("\nDrive not found.\n\n");
        goto askdrive; }
    printf("\nDrive %c is physical hard drive %u, cylinder %u, head %u\n\n",
            drvltr,drive,firstcyl,firsthead);     /*1.3b*/
    drive+=0x80;
    }

if(drive<0x80) {
   printf("\nInsert disk in drive %c and press any key to\
 continue ",drvltr);
   getch();
   printf("\n\n"); }

readsec(drive,firsthead,firstcyl,1,1,buf);
if((buf[510]!=0x55)||(buf[511]!=0xaa)) {
    printf("This is not a boot sector.\n\n");
    exit(1); }


decrypt=(!memcmp(buf+incfo,"CRYP",4) || !memcmp(buf+outcfo,"CRYP",4));
encrypt=!decrypt;                       /*1.1*/

calcdiskparams(buf,&maxcyl,&maxhead,&maxsector,
               &secsize,serial);

printf("This disk has \
%u cylinders, %u sectors, %u heads, sector size %u bytes\n\n",
       maxcyl+1,maxsector,maxhead,secsize);

if((buf=realloc(buf,maxsector*secsize)) != NULL)
    printf("Allocated %u bytes for track buffer\n\n",
            maxsector*secsize);
else {
    printf("Error: unable to allocate %u bytes for track buffer\n",
            maxsector*secsize);
    exit(1); }

if(encrypt)
    {
    int needkey=1;


    printf("This disk is not encrypted. Do you want to encrypt it? ");
    if(!getyn())
        {
        printf("\n");
        exit(0);
        }

    if (pgpv != NULL)
     {
      printf("\nUse PGPPASS as passphrase? ");
      if (getyn())
       {
        strcpy(pass1,pgpv);
        setkeye(ekey,echeck);
        printf("\nPGPPASS entered as %s passphrase\n",
               ef10 ? "(V 1.0)":"(V 1.1)");       /*1.3b*/
        clrbufs();
        needkey=0;
       }
     }

    if (needkey && cryptdata != NULL && drive<0x80)
     {
      if (memcmp(cryptdata->fkeychk,"\xff\xff\xff\xff",4) &&
         (compat_mode != 'N') == (cryptdata->fkeyv10 != 0))
         /*key taken from SECTSR for encryption must match SD10CMP*/
       {
        printf("Use Floppy Key already in SECTSR? ");
        if (getyn())
         {
          memcpy(echeck,cryptdata->fkeychk,4);
          memcpy(eexpkey,cryptdata->fkey,104);
          ef10=cryptdata->fkeyv10;                   /*1.3b*/
          ekeyexp=1;
          needkey=0;
         }
       }
     }

    if (needkey && cryptdata != NULL && drive<0x80)
     {
      if (memcmp(cryptdata->hkeychk,"\xff\xff\xff\xff",4) &&
         (compat_mode != 'N') == (cryptdata->hkeyv10 != 0))
         /*key taken from SECTSR for encryption must match SD10CMP*/
       {
        printf("Use Hard Disk key already in SECTSR? ");
        if (getyn())
         {
          memcpy(echeck,cryptdata->hkeychk,4);
          memcpy(eexpkey,cryptdata->hkey,104);
          ef10=cryptdata->hkeyv10;                   /*1.3b*/
          ekeyexp=1;
          needkey=0;
          memset(filekey,0,16);
         }
       }
     }

    if (needkey)
     {
      getkey(ekey,echeck,TRUE);
      clrbufs();
      memset(filekey,0,16);
     }
    }
else
    {
     int needdkey=1;
     printf("This disk is encrypted.\n\
Select (C)hange passphrase, (D)ecrypt, (E)xit: ");           /*1.1*/
     do r=toupper(getch()); while(r!='C'&&r!='D'&&r!='E');   /*1.1*/
     printf("%c\n",r);                                       /*1.1*/
     if(r=='E')                                              /*1.1*/
      {
       printf("\n");
       exit(0);
      }

     if (cryptdata != NULL && drive<0x80 &&
         (!memcmp(cryptdata->fkeychk,buf+incfo+4,4)  ||
          !memcmp(cryptdata->fkeychk,buf+outcfo+4,4)  )  )
      {
       printf("Floppy Key already in SECTSR will decrypt this diskette.\n");
       memcpy(dcheck,cryptdata->fkeychk,sizeof(dcheck));
       memcpy(dexpkey,cryptdata->fkey,104);
       df10=cryptdata->fkeyv10;
       needdkey=0;
       dkeyexp=1;
      }

     if (needdkey && fkset)  /* If file key, try null passphrase */
      {
       pass1[0] = 0;
       setkeydf(dkey,dcheck,buf);

       if(memcmp(dcheck,buf+incfo+4,4) && memcmp(dcheck,buf+outcfo+4,4))
        {
         printf("\nFile Key alone will not decrypt.\n");
        }
       else
        {
         printf("\nFile Key alone will decrypt this disk.\n");
         needdkey=0;
        }
      }

     if (needdkey && pgpv != NULL)
      {
       strcpy(pass1,pgpv);
       setkeydf(dkey,dcheck,buf);

       if(!df10 && !memcmp(Old13Check,buf+outcfo+4,4))
        {
         printf("Check bytes in Disk %c: Boot Sector need updating from 1.3 to 1.1/1.3A. Proceed? ",
                drvltr);
         if(getyn())
          {
           memcpy(buf+outcfo+4,dcheck,4);
           writesec(drive,firsthead,firstcyl,1,1,buf);
          }
        }

       if(memcmp(dcheck,buf+incfo+4,4) && memcmp(dcheck,buf+outcfo+4,4))
        {
         printf("\nPGPPASS is wrong passphrase.\n");
        }
       else
        {
         printf("\nPGPPASS entered as ");
         if(r=='C') printf("old ");                 /*1.1*/
         if(df10)
          printf("(V 1.0) ");
         else
          printf("(V 1.1) ");
         printf("passphrase");
         if (fkset)
          {
           printf(" with");
           if (!fkapply)
            printf("out");
           printf(" File Key");
          }
         printf(" will decrypt this disk.\n");
         needdkey=0;
        }
      }


     if (needdkey)
      {
       for(t=0;t<3;t++)
        {
         printf("\nEnter ");                        /*1.1*/
         if(r=='C') printf("old ");                 /*1.1*/
         printf("passphrase: ");                    /*1.1*/
         getkeydf(dkey,dcheck,buf);                 /*1.3b*/
         if(!df10 && !memcmp(Old13Check,buf+outcfo+4,4))
          {
           printf("Check bytes in Disk %c: Boot Sector need updating from 1.3 to 1.1/1.3A. Proceed? ",
                  drvltr);
           if(getyn())
            {
             memcpy(buf+outcfo+4,dcheck,4);
             writesec(drive,firsthead,firstcyl,1,1,buf);
            }
          }
         clrbufs();
         if(!memcmp(dcheck,buf+incfo+4,4) || !memcmp(dcheck,buf+outcfo+4,4))
          {
           printf("\n");
           if(r=='C') printf("Old ");                 /*1.1*/
           if(df10)
            printf("(V 1.0) ");
           else
            printf("(V 1.1) ");
           printf("passphrase");
           if (fkset)
            {
             printf(" with");
             if (!fkapply)
              printf("out");
             printf(" File Key");
            }
           printf(" will decrypt this disk.\n");
           break;
          }
         printf("Wrong passphrase.\n");
         if(t==2) exit(1);                          /*1.1*/
        }
      }
    }
    if(r=='C')                                     /*1.1*/
     {                                             /*1.1*/
      int needekey=1;
      encrypt=TRUE;                                /*1.1*/
      if (pgpv != NULL)
       {
        printf("\nUse PGPPASS as new passphrase? ");
        if (getyn())
         {
          strcpy(pass1,pgpv);                                   /*1.3b*/
          if (fkset)
           {
            printf("\nUse File Key with PGPPASS? ");
            if (getyn())
             fkapply=1;
            else
             fkapply=0;
           }
          setkeye(ekey,echeck);                                 /*1.3b*/
          printf("\nPGPPASS entered as new %s passphrase\n",
                  ef10 ? "(V 1.0)":"(V 1.1)");                  /*1.3b*/
          needekey=0;
         }
       }

        if (needekey && cryptdata != NULL && drive<0x80 )
         {
          if (!dkeyexp) en_key_idea((word16 *)dkey,dexpkey);
          if (memcmp(dexpkey,cryptdata->fkey,sizeof(dexpkey)) != 0)
            /*floppy key can be new key if not also old key!*/
           {
            if (memcmp(cryptdata->fkeychk,"\xff\xff\xff\xff",4))
             {
              printf("Use Floppy Key already in SECTSR as new key? ");
              if (getyn())
               {
                memcpy(echeck,cryptdata->fkeychk,sizeof(echeck));
                memcpy(eexpkey,cryptdata->fkey,104);
                needekey=0;
                ekeyexp=1;
                ef10=cryptdata->fkeyv10;
               }
             }
           }
         }

        if (needekey && cryptdata != NULL && drive<0x80 )
         {
          if (!dkeyexp) en_key_idea((word16 *)dkey,dexpkey);
          if (memcmp(dexpkey,cryptdata->hkey,sizeof(dexpkey)) != 0)
            /*HD key can be new key if not also old key!*/
           {
            if (memcmp(cryptdata->hkeychk,"\xff\xff\xff\xff",4))
             {
              printf("Use Hard Drive Key already in SECTSR as new key? ");
              if (getyn())
               {
                memcpy(echeck,cryptdata->hkeychk,sizeof(echeck));
                memcpy(eexpkey,cryptdata->hkey,104);
                needekey=0;
                ekeyexp=1;
                ef10=cryptdata->hkeyv10;
               }
             }
           }
         }

        if (needekey)
         {
          printf("\n");                              /*1.1*/
          getkey(ekey,echeck,TRUE);                  /*1.1*/
          clrbufs();
         }

      if (memcmp(dkey,ekey,sizeof(dkey)) == 0)
       {
        printf("ERROR: Attempt to (C)hange Passphrase with same Key\n");
        exit(1);
       }
     }

   if(cryptdata != NULL  && drive >= 0x80)
    {
     /*find empty or duplicate HD slot*/
     for (i=0,iz=MAXDRV;i<MAXDRV;i++)
      {
        if  ( cryptdata->hd[i].dddrv    == 0 )
         iz=i;
        if   (cryptdata->hd[i].dddrv    == drive &&
              cryptdata->hd[i].firstcyl == firstcyl)
         break;
      }
     if (i == MAXDRV)
      i=iz;
     if (i < MAXDRV)
      {
       printf("SECTSR will protect drive during %s.\n",
        decrypt ?
          (encrypt ? "passphrase change" : "decryption")
         : "encryption");
       sethdsafe=1;
      }
    }

printf("\nLast chance to abort. Continue? ");
if(!getyn()) exit(1);
ctrlbrk(c_break_int);                            /*1.4*/
bdos(0x0D, 0, 0);          /* Reset Disk Subsystem - Flush all buffers */
clrscr();
gotoxy(1,8);
if (sethdsafe)
 {
  hdx=i;
  if (cryptdata->hd[i].dddrv == 0)
   {
    cryptdata->hd[i].dddrv=drive;
    cryptdata->hd[i].drvltr=drvltr;
    cryptdata->hd[i].firstcyl=firstcyl;
    cryptdata->hd[i].firsthd=firsthead;
    cryptdata->hd[i].firstsec=1;
    cryptdata->hd[i].lastcyl=firstcyl+maxcyl;
    cryptdata->hd[i].maxsec=maxsector;
    cryptdata->hd[i].maxhd=maxhead;
    cryptdata->hd[i].secsize=secsize;
    cryptdata->hd[i].active=0;
   }
  else if (cryptdata->hd[i].active)
   {
    cryptdata->hd[i].active=0;
    memset(cryptdata->hkey,0xbb,104);
   }
 }

if (encrypt && decrypt)
 printf("Changing Old %s Passphrase to New %s Passphrase\n",
         df10 ? "(V 1.0) " : "(V 1.1) ",
         ef10 ? "(V 1.0) " : "(V 1.1) ");
else if (encrypt)
 printf("Encrypting %s",
         ef10 ? "(V 1.0) ":"(V 1.1) ");
else
 printf("Decrypting %s",
         df10 ? "(V 1.0) " : "(V 1.1) ");
printf("disk %c:  %u cyls, %u sectors, %u heads, sector size %u bytes",
       drvltr,maxcyl+1,maxsector,maxhead,secsize);

if(encrypt && !ekeyexp) en_key_idea((word16 *)ekey,eexpkey);     /*1.1,1.3*/
if(decrypt && !dkeyexp) en_key_idea((word16 *)dkey,dexpkey);     /*1.1,1.3*/

printf("\n");
for(cyl=0;cyl<=maxcyl;cyl++)
 {
  for(head=0;head<maxhead;head++)
   {
    if(cyl==0&&head<firsthead) head=firsthead;
    if (!c_break_hit)
     {
      gotoxy(1,11);
      printf("Cylinder %u, Head %u ",cyl,head);     /*1.1*/
     }
    readseca(drive,head,cyl+firstcyl,1,maxsector,secsize,buf);
    bufptr=buf;
    for(sector=1;sector<=maxsector;sector++)
     {
      if(cyl==0&&head==firsthead&&sector==1)
       if(encrypt)
        {
         memcpy(&buf[outcfo],"CRYP",4);      /*1.3c*/
         memcpy(&buf[outcfo+4],echeck,4);    /*1.3c*/
         if (!memcmp(buf+3,"CRYP",4))
          memcpy(buf+3,"MSDOS   ",8);
        }
       else
        {
         if (!memcmp(buf+3,"CRYP",4))
          memcpy(buf+3,"MSDOS   ",8);
         memcpy(buf+outcfo,"MSDOS   ",8);
        }
       else
        {
         t=cyl+firstcyl;
         iv[0]=t%256;
         iv[1]=t/256;
         iv[2]=head;
         iv[3]=sector;
         iv[4]=serial[0]%256;
         iv[5]=serial[0]/256;
         iv[6]=serial[1]%256;
         iv[7]=serial[1]/256;
         if(decrypt)                                 /*1.1*/
          {
           memcpy(tiv,iv,sizeof(iv));
           IdeaCFB(tiv,dexpkey,dummy,dummy,1);
           IdeaCFBx(tiv,dexpkey,bufptr,bufptr,secsize/8+1);
          }
         if(encrypt)                                 /*1.1*/
          {
           memcpy(tiv,iv,sizeof(iv));
           IdeaCFB(tiv,eexpkey,dummy,dummy,1);
           IdeaCFB(tiv,eexpkey,bufptr,bufptr,secsize/8+1);
          }
        }
       bufptr+=secsize;
     }
    writeseca(drive,head,cyl+firstcyl,1,maxsector,secsize,buf);
    if (c_break_hit) break;                          /*1.4*/
   }
  if (c_break_hit)                                   /*1.4*/
   {                                                 /*1.4*/
    gotoxy(1,11);                                    /*1.4*/
    printf("Cylinder %u, Head %u ",cyl,head);        /*1.4*/
    gotoxy(1,12);                                    /*1.4*/
    printf("Processing Stopped by User.");           /*1.4*/
    break;                                           /*1.4*/
   }                                                 /*1.4*/
 }


for(t=0;t<16;t++) { ekey[t]='\0'; dkey[t]='\0'; }   /*1.1*/
for(t=0;t<52;t++) { eexpkey[t]=0; dexpkey[t]=0; }   /*1.1*/

gotoxy(1,13);
printf("\n\nDone.\n");
bdos(0x0D, 0, 0);          /* Reset Disk Subsystem - Flush all buffers */
if (sethdsafe)
 {
  if (encrypt)
   {
    printf("Encrypted hard disk now in safe mode.  Use LOGIN to access.\n");
    if (!decrypt)
     printf("Before re-boot, add appropriate LOGIN /S to AUTOEXEC.BAT.\n");
   }
  else
   {
    cryptdata->hd[hdx].dddrv = 0;
    cryptdata->hd[hdx].drvltr= 0;
     /*remove decrypted hard disk from disk table.
       note: re-boot may be required to recover drive table slot(s)*/
    printf("Decrypted disk may now be accessed. Before re-boot,\n\
remove corresponding LOGIN  /S from AUTOEXEC.BAT.\n");
   }
 }
return(0);
}

void freebuf(void)
{
free(buf);
}

void readseca(unsigned drive,unsigned head,unsigned cyl,unsigned sector,
     unsigned nsects,unsigned secsize,unsigned char *buffer)
{
unsigned i,j;
char c;
unsigned r;
for(i=0;i<3;i++)
 {
  r=rldbios(2,drive,head,cyl,sector,nsects,buffer);
  if (!r) return;
 }
gotoxy(1,13);
printf("\nRead error (%02X): drive %02x, head %u, cyl %u\n",
       r,drive,head,cyl);
printf("Reading one sector at a time.\n");
for(j=0;j<nsects;j++) {
  for(i=0;i<3;i++)
   {
    r=rldbios(2,drive,head,cyl,sector+j,1,buffer);
    if(!r) goto goodsec;
   }
  gotoxy(1,15);
  printf("Bad sector (%02X): drive %02x, head %u, cyl %u, sector %u\n",
         r,drive,head,cyl,sector+j);
  goodsec:
  buffer+=secsize;
  }
}

void writeseca(unsigned drive,unsigned head,unsigned cyl,unsigned sector,
     unsigned nsects,unsigned secsize,unsigned char *buffer)
{
unsigned i,j;
char c;
unsigned r;
for(i=0;i<3;i++)
 {
  r=rldbios(3,drive,head,cyl,sector,nsects,buffer);
  if(!r) return;
 }
gotoxy(1,13);
if (r == 03 && drive<0x80)
 {
  printf("\nERROR: Disk %c is Write-Protected!\n",drive+'A');
  exit(1);
 }
printf("\nWrite error (%02X): drive %02x, head %u, cyl %u\n",
       r,drive,head,cyl);
printf("Writing one sector at a time.\n");
for(j=0;j<nsects;j++) {
  for(i=0;i<3;i++)
   {
    r=rldbios(3,drive,head,cyl,sector+j,1,buffer);
    if(!r) goto goodsec;
   }
  gotoxy(1,15);
  printf("Bad sector (%02X): drive %02x, head %u, cyl %u, sector %u\n",
         r,drive,head,cyl,sector+j);
  goodsec:
  buffer+=secsize;
  }
}
