/* Secure Drive LOGIN V1.4a */
/* Logs into encrypted disks */

#include "secdrv.h"

extern char pass1[MAXPASS];
extern char pass2[MAXPASS];
extern char filekey[16];
extern int  fkset,fkapply;
extern char Old13Check[4];
extern unsigned far pspseg;

extern int  tsr_not_installed;
extern int  tsr_wrong_version;
extern int  df10,ef10;
extern int  incfo,outcfo;     /*1.3c*/
extern char compat_mode;

int   setenv(char*,char *);
void  clrbufs(void);
void upd13cw (void);
void rcfproc(void);
void ucfoproc(void *);

char *pgpv;
char drvltr;
unsigned safemode=FALSE;
unsigned pgpsw=FALSE;
unsigned ucfosw=FALSE;
unsigned rcfsw=FALSE;
unsigned addsw=FALSE;                    /*1.4*/
unsigned char buf[512],key[16],check[4];
unsigned drive,firstcyl,firsthead,maxcyl;
unsigned maxhead,maxsector,secsize,i,iz;

int main(int argc,char *argv[])
{
word16 expkey[52];
unsigned serial[2];
unsigned char *p;
int ekeyexp=0;

if(argc==1)
 {
    printf("\n\
Secure Drive Login Version 1.4b\n\
This program sets parameters and loads passphrases for encrypted\
 drives.\n\n\
LOGIN /F  [/PGP] [/UCFO] [/RCF]  to enter floppy disk passphrase\n\
                                 [use/set PGPPASS] [Update CryptFlag\n\
                                 Offset] [Recover CryptFlag]\n\
\n\
LOGIN /C  [/PGP]                 to erase keys and disable encryption\n\
                                 [clear PGPPASS]\n\
\n\
LOGIN drive letter  [/PGP]       to activate an encrypted hard drive\n\
  [/UCFO] [/RCF] [/ADD]          ADD a HD partition with same pswd\n\
LOGIN D: /S                      to prevent accidental access to an\n\
                                 encrypted drive without logging in\n\
\n\
LOGIN /PGP                       to set PGPPASS environment variable\n\
\n\
LOGIN drive cylinder head [x]    to manually enter parameters for\n\
 [/PGP] [/UCFO] [/RCF]           a hard drive partition\n\
LOGIN 0 100 1 D /S               to prevent accidental access\n\
      drives are numbered from zero\n\n");
    exit(1);
  }


for (i=1;i<argc;i++)
 {
   if (!stricmp(argv[i],"/PGP"))
    {
     pgpsw=TRUE;
     pgpv=getenv("PGPPASS");
    }
   if (!stricmp(argv[i],"/UCFO"))
    ucfosw=TRUE;
   if (!stricmp(argv[i],"/RCF"))
    rcfsw=TRUE;
   if (!stricmp(argv[i],"/ADD"))          /*1.4*/
    addsw=TRUE;
 }

pspseg = _psp;   /*for setenv 1.4b*/

if (argc == 2 && pgpsw)
 {
  printf("\nEnter PGPPASS passphrase for PGP Secret Key: ");
  invgets(pass1);
  if(pass1[0] != 0)
   {
    if (!setenv("PGPPASS",pass1))
     {
      printf("PGPPASS set. \n");
      clrbufs();
      exit(0);
     }
    else
     {
      printf("PGPPASS not set. \n");
      clrbufs();
      exit(1);
     }
   }
  else
   {
    if (pgpv != NULL)
     {
      for (i=0;i<strlen(pgpv);i++)
       pass1[i]='x';
      pass1[i]=0;
      setenv("PGPPASS",pass1);
      setenv("PGPPASS","");
      printf("PGPPASS removed from environment\n");
      clrbufs();
      exit(0);
     }
    else
     {
      printf("PGPPASS not removed from environment because not present\n");
      clrbufs();
      exit(1);
     }
   }
 }

set_compat_mode();

if(!(cryptdata=gettsradr()))
 {
  printf("\nError: Secure Drive TSR %s.\n",
         tsr_wrong_version ? "wrong Version (not 1.4/a/b)" : "not loaded");
  exit(1);
 }


if((*argv[1]=='/')&&(toupper(*(argv[1]+1))=='C'))
{
  bdos(0x0D, 0, 0);          /* Reset Disk Subsystem - Flush all buffers */

  for (i=0;i<MAXDRV;i++)
   cryptdata->hd[i].active=0;

  cryptdata->fda.active=0;                     /*1.4*/
  cryptdata->fdb.active=0;                     /*1.4*/

  memset(cryptdata->fkey,0xaa,104);
  memset(cryptdata->hkey,0xbb,104);
  memset(cryptdata->fkeychk,0x0ff,4);
  memset(cryptdata->hkeychk,0x0ff,4);
  printf("\nClearing free memory...");
  while ((p=malloc(1024)) != NULL)
   {
    memset(p,0,1024);
   }
  while ((p=malloc(8)) != NULL)
   {
    memset(p,0,8);
   }

   printf("\nAll keys erased. System secured.\n");
   if (pgpsw && pgpv != NULL)
    {
     for (i=0;i<strlen(pgpv);i++)
      pass1[i]='x';
     pass1[i]=0;
     setenv("PGPPASS",pass1);
     setenv("PGPPASS","");
     printf("PGPPASS removed from environment\n");
    }
  clrbufs();
  exit(0);
 }


if((*argv[1]=='/')&&(toupper(*(argv[1]+1))=='F'))
 {
  char c;
  int fdpr=1;
  firsthead=0;       /*1.3b*/
  firstcyl=0;
  read_filekey();
  while (1)
   {
    printf("\nInsert encrypted diskette in drive A or B\n\
then press A or B to check passphrase.\n\
Or Press Esc to %s.: ",
       fkset || ucfosw || rcfsw ? "exit" : "bypass passphrase check");

    while ((c=toupper(getch())) != 'A' && c != 'B' && c != ESC) {}
    bdos(0x0D, 0, 0);          /* Reset Disk Subsystem - Flush all buffers */
    if (c == ESC)
     {
      if(!ucfosw && !rcfsw && !fkset)
       {
        printf("\n\n");                 /*1.3c*/
        break;                          /*1/3c*/
       }
      else
       exit(1);
     }
    printf("%c\n\n",c);
    drvltr=c;
    drive=(unsigned) c-'A';
    readsec(drive,0,0,1,1,buf);
    if((buf[510]!=0x55)||(buf[511]!=0xaa))
     {
      printf("\nThis diskette is not formatted.\n\n");
      continue;
     }
    if(memcmp(buf+incfo,"CRYP",4) && memcmp(buf+outcfo,"CRYP",4) && !rcfsw) /*1.3c*/
     {
      printf ("\nThis diskette is not encrypted.\n\n");
      continue;
     }
    break;
   } /* while (1)*/

  {
   unsigned char temp[16];
   unsigned char nokey [104];
   MD5_CTX md5buf;

   memset(nokey,0xbb,104);
   if (memcmp(cryptdata->hkey,nokey,104) != 0)
    {
     if (c != ESC)
      {
       if(!memcmp(cryptdata->hkeychk,buf+incfo+4,4) ||
          !memcmp(cryptdata->hkeychk,buf+outcfo+4,4))
        {
         memcpy(check,cryptdata->hkeychk,4);
         memcpy(expkey,cryptdata->hkey,104);
         ef10 = df10 = cryptdata->fkeyv10 = cryptdata->hkeyv10;
         ekeyexp=1;
         fdpr=0;
         printf("\nFloppy disk encryption enabled with\n\
verified active hard disk partition %s passphrase.\n",
          df10 ? "(V 1.0)" : "(V 1.1)");
          ucfoproc(cryptdata->hkeychk);  /*Update CryptFlag Offset if /UCFO  1.3c*/


        }
      }
     else /*verify bypassed*/
      {
       printf("\nUse active Hard Disk passphrase as unverified floppy passphrase? :");
       if (getyn())
        {
         memcpy(check,cryptdata->hkeychk,4);
         memcpy(expkey,cryptdata->hkey,104);
         ef10 = cryptdata->hkeyv10;
         ekeyexp=1;
         fdpr=0;
         printf("\nFloppy disk encryption enabled with\n\
unverified active hard disk partition %s passphrase.\n",
          ef10 ? "(V 1.0)" : "(V 1.1)");
        }
      }
    }
  }

 if (fdpr && fkset)  /* If file key, try null passphrase */
  {
   pass1[0] = 0;
   setkeydf(key,check,buf);

   if(memcmp(check,buf+incfo+4,4) && memcmp(check,buf+outcfo+4,4))
    {
     printf("\nFile Key alone will not decrypt.\n");
    }
   else
    {
     printf("\nFile Key alone will decrypt this diskette.\n");
     fdpr=0;
    }
  }

   if (fdpr && pgpsw && pgpv != NULL)
    {
     strcpy(pass1,pgpv);
     if (c != ESC)
      {
       setkeydf(key,check,buf);
       upd13cw();         /*update 1.3 checkword to 1.3a */  /*1/3c*/

       ef10=df10;
       if(!memcmp(check,buf+incfo+4,4) || !memcmp(check,buf+outcfo+4,4))
        {
         printf("\nPGPPASS entered as %s floppy disk passphrase",
               df10 ? "(V 1.0)" : "(V 1.1)");
         if (fkset)
          {
           printf(" with");
           if (!fkapply)
            printf("out");
           printf(" File Key");
          }
         printf(".\n");
         ucfoproc(check);     /*Update CryptFlag Offset if /UCFO  1.3c*/
         fdpr=0;
        }
       else
        {
         printf("\nPGPPASS is wrong passphrase.\n");
         fdpr=1;
        }
      }
     else
      {
       setkeye (key,check);
       printf("\nPGPPASS entered as %s unverified floppy passphrase\n",
               ef10 ? "(V 1.0)":"(V 1.1)");
       fdpr=0;
      }
    }
   if (fdpr)
    {
     printf("\nEnter floppy disk passphrase: ");
     if (c != ESC)
      {
       getkeydf(key,check,buf);
       upd13cw();         /*update 1.3 checkword to 1.3a */  /*1/3c*/
       ef10=df10;
       if(!memcmp(check,buf+incfo+4,4) || !memcmp(check,buf+outcfo+4,4))
        {
         printf("\nFloppy disk encryption enabled with\n");
         printf("verified %s passphrase",
                 df10 ? "(V 1.0)" : "(V 1.1)");
         if (fkset)
          {
           printf(" with");
           if (!fkapply)
            printf("out");
           printf(" File Key");
          }
         printf(".\n");
         ucfoproc(check);  /*Update CryptFlag Offset if /UCFO  1.3c*/
        }
       else
        rcfproc();  /*Incorrect Passphrase or /RCF*/  /*1.3c*/
      }
     else
      {
       getkey(key,check,FALSE);
       printf("\nFloppy disk encryption enabled with\n");
       printf("Unverified %s passphrase.\n",
               ef10 ? "(V 1.0)":"(V 1.1)");
      }
    }
   if (!ekeyexp)
    en_key_idea((word16 *) key,expkey);
   memcpy(cryptdata->fkey,expkey,104);
   memcpy(cryptdata->fkeychk,check,4);
   cryptdata->fkeyv10=(char) ef10;
   cryptdata->fda.firstcyl=0;
   cryptdata->fdb.firstcyl=0;
   cryptdata->fda.active=0;                     /*1.4*/
   cryptdata->fdb.active=0;                     /*1.4*/
   if (pgpsw && pgpv == NULL && !setenv("PGPPASS",pass1))
    printf("PGPPASS set. \n");                /*1.3b*/
   bdos(0x0D, 0, 0);          /* Reset Disk Subsystem - Flush all buffers */
   if (c != ESC)
    readsec(drive,0,0,1,1,buf);
   clrbufs();
   memset(filekey,0,16);
   exit(0);
 }

else if(isalpha(*argv[1]))
 {
  drvltr=toupper(*argv[1]);
  drive=255;
  /*find disk params if previously set by LOGIN   1.4*/
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

  if (drive == 255)
   readptbl(drvltr,&drive,&firsthead,&firstcyl);   /*1.1*/
  if(drive==255) {
      printf("\nDrive not found.\n");
      exit(1); }
  if((*argv[2]=='/')&&(toupper(*(argv[2]+1))=='S'))
   safemode=TRUE;
  printf("\nDrive %c is physical hard drive %u, cylinder %u, head %u\n",
          drvltr,drive,firstcyl,firsthead);     /*1.3b*/
 }

else
 {
  drvltr='X';
  i=sscanf(argv[1],"%u",&drive);
  i=i&&sscanf(argv[2],"%u",&firstcyl);
  i=i&&sscanf(argv[3],"%u",&firsthead);
  if (argc>3)
   {
    if ((*argv[4]=='/')&&(toupper(*(argv[4]+1))=='S'))
       safemode=TRUE;
    else
     if (isalpha (*argv[4]))
      drvltr=toupper(*argv[4]);
    if((argc>4 && *argv[5]=='/')&&(toupper(*(argv[5]+1))=='S'))
       safemode=TRUE;
   }
  if(!i)
   {
    printf("Incorrect drive, cylinder, or head input.\n\
Run without a command line for help.\n");
    exit(1);
   }
 }

drive+=0x80;

readsec(drive,firsthead,firstcyl,1,1,buf);

if((buf[510]!=0x55)||(buf[511]!=0xaa)) {
    printf("\nThis is not a boot sector.\n");
    exit(1); }

calcdiskparams(buf,&maxcyl,&maxhead,&maxsector,
               &secsize,serial);
if (!safemode && !addsw)
  read_filekey();

printf("\nDisk %c: has: %i cylinders, %i sectors, %i heads, sector \
size %i bytes\n",drvltr,maxcyl+1,maxsector,maxhead,secsize);

if(memcmp(buf+incfo,"CRYP",4) && memcmp(buf+outcfo,"CRYP",4) && !rcfsw) /*1.3c*/
 {
  printf("\nThis disk is not encrypted.\n\n");
  exit(1);
 }

if (addsw)
 {
  unsigned char nokey [104];
  memset(nokey,0xbb,104);
  if (!memcmp(cryptdata->hkey,nokey,104))
   {
    printf("\nADD parameter requires currently active HD key\n");
    exit(1);
   }
  if(memcmp(cryptdata->hkeychk,buf+incfo+4,4) &&
     memcmp(cryptdata->hkeychk,buf+outcfo+4,4))
   {
    printf("\nADDed Partition is not same key.\n");
    exit(1);
   }
 }


if(!safemode && !addsw)          /*1.4*/
{
 int hdpr=1;

 if (hdpr && fkset)  /* If file key, try null passphrase */
  {
   pass1[0] = 0;
   setkeydf(key,check,buf);

   if(memcmp(check,buf+incfo+4,4) && memcmp(check,buf+outcfo+4,4))
    {
     printf("\nFile Key alone will not decrypt.\n");
    }
   else
    {
     printf("\nFile Key alone will decrypt this disk.\n");
     hdpr=0;
    }
  }


 if (hdpr && pgpsw && pgpv != NULL)
  {
   strcpy(pass1,pgpv);
   setkeydf(key,check,buf);
   upd13cw();         /*update 1.3 checkword to 1.3a */  /*1/3c*/
   if(memcmp(check,buf+incfo+4,4) && memcmp(check,buf+outcfo+4,4))
    {
     printf("\nPGPPASS is wrong passphrase.\n");
     hdpr=1;
    }
   else
    {
     printf("\nPGPPASS entered as %s hard disk passphrase",
            df10 ? "(V 1.0)" : "(V 1.1)");
     if (fkset)
      {
       printf(" with");
       if (!fkapply)
        printf("out");
       printf(" File Key");
      }
     printf(".\n");
     ucfoproc(check);  /*Update CryptFlag Offset if /UCFO  1.3c*/
     hdpr=0;
    }
  }
 if (hdpr)
  {
   printf("\nEnter hard disk passphrase: ");
   getkeydf(key,check,buf);
   upd13cw();         /*update 1.3 checkword to 1.3a */  /*1/3c*/
  }

 if(memcmp(check,buf+incfo+4,4) && memcmp(check,buf+outcfo+4,4))
  rcfproc();  /*Incorrect Passphrase or /RCF*/  /*1.3c*/
 else
  ucfoproc(check);  /*Update CryptFlag Offset if /UCFO  1.3c*/


 en_key_idea((word16 *)key,expkey);

 memcpy(cryptdata->hkey,expkey,104);
 memcpy(cryptdata->hkeychk,check,4);
 cryptdata->hkeyv10=(char) df10;
} /*if(!safemode && !addsw)*/

bdos(0x0D, 0, 0);          /* Reset Disk Subsystem - Flush all buffers */
/*set all slots inactive*/
if (!addsw)         /*1.4*/
 {
  for (i=0;i<MAXDRV;i++)
   {
     if (cryptdata->hd[i].active)
      printf("\nDrive %c: set to Safe mode.\n",cryptdata->hd[i].drvltr);
     cryptdata->hd[i].active=0;
   }
 }

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
  cryptdata->hd[i].dddrv=drive;
  cryptdata->hd[i].drvltr=drvltr;
  cryptdata->hd[i].firstcyl=firstcyl;
  cryptdata->hd[i].firsthd=firsthead;
  cryptdata->hd[i].firstsec=1;
  cryptdata->hd[i].lastcyl=firstcyl+maxcyl;
  cryptdata->hd[i].maxsec=maxsector;
  cryptdata->hd[i].maxhd=maxhead;
  cryptdata->hd[i].secsize=secsize;
  cryptdata->hd[i].drvltr=drvltr;    /*1.4*/
  if(!safemode)
   {
    cryptdata->hd[i].serial[0]=serial[0];
    cryptdata->hd[i].serial[1]=serial[1];
    cryptdata->hd[i].active=1;

    if (addsw)
     {
      printf("\nPartition ADDed.\n");
      ucfoproc(cryptdata->hkeychk);  /*Update CryptFlag Offset if /UCFO  1.3c*/
     }
    else  /*!addsw*/
     {
      printf("\nHard disk %s encryption",
             df10 ? "(V 1.0)" : "(V 1.1)");
      if (fkset)
       {
        printf(" with");
        if (!fkapply)
         printf("out");
        printf(" File Key");
       }
      printf(" enabled logical drive %c:.\n",drvltr);
      if (pgpsw && pgpv == NULL && !setenv("PGPPASS",pass1))
       printf("PGPPASS set. \n");                /*1.3c*/
     }
   }
  else
   {
    printf("\nHard disk set to safe mode.\n");
   }
 }

if (i == MAXDRV)
 printf("\nERROR: More than %d Encrypted Partitions not supported\n",MAXDRV);

for(i=0;i<16;i++) key[i]='\0';
for(i=0;i<52;i++) expkey[i]=0;

bdos(0x0D, 0, 0);          /* Reset Disk Subsystem - Flush all buffers */
clrbufs();
memset(filekey,0,16);
return(0);
}

void upd13cw (void)
 {
      if(!df10 && !memcmp(Old13Check,buf+outcfo+4,4))
       {
        printf("\nCheck bytes in Disk %c: Boot Sector need updating from 1.3 to 1.1/1.3A. Proceed? ",
               drvltr);
        if(getyn())
         {
          if (memcmp(buf+incfo,"CRYP",4))
           memcpy(buf+incfo,"MSDOS   ",8);
          memcpy(buf+outcfo,"CRYP",4);
          memcpy(buf+outcfo+4,check,4);
          writesec(drive,firsthead,firstcyl,1,1,buf);
         }
       }

      else if(!df10 && !memcmp(Old13Check,buf+incfo+4,4) && ucfosw)  /*1.3c*/
       {
        printf("\
Check bytes in Disk %c: Boot Sector need updating from 1.3 to 1.1/1.3A.\n\
CryptFlag Offset will also be changed to %d. Proceed? ",
               drvltr,outcfo);
        if(getyn())
         {
          memcpy(buf+outcfo,"CRYP",4);
          memcpy(buf+outcfo+4,check,4);
          memcpy(buf+incfo,"MSDOS   ",8);
          writesec(drive,firsthead,firstcyl,1,1,buf);
         }
       }
 }

void rcfproc(void)
 {
  if (!rcfsw)
   {
    printf("\nWrong passphrase.\n");
    exit(1);
   }
  else
   {
    printf("\nPassphrase Entered does not match CryptFlag.\n");
    if (compat_mode == 'X')
     {
      printf("You specified /RCF, but this is not allowed with SD10CMP=X\n");
      exit(1);
     }
    else
     {
      printf("\n\
/RCF Specified. Re-Enter Same Passphrase to Ignore present CryptFlag,\n\
compute new %s CryptFlag and write it at offset %d.\n\n",
      compat_mode != 'N' ? "(V 1.0)":"(V 1.1)", outcfo);
      strcpy(pass2,pass1);
      if (fkset)
       {
        printf("\nUse File Key with this Passphrase? ");
        if (getyn())
         fkapply=1;
        else
         fkapply=0;
        }
      printf("Re-Enter passphrase: ");
      getkey(key,check,FALSE);
      if (!strcmp(pass1,pass2))
       {
        if (!memcmp(buf+incfo,"CRYP",4))
         memcpy(buf+incfo,"MSDOS   ",8);
        memcpy(buf+outcfo,"CRYP",4);
        memcpy(buf+outcfo+4,check,4);
        writesec(drive,firsthead,firstcyl,1,1,buf);
        printf("\nCryptFlag Reconstructed.\n");
       }
      else
       {
        printf("Passphrases do not match.\n");
        exit(1);
       }
     }
   }
 } /*rcfproc*/

void ucfoproc(void *keychk)
 {
  if (!ucfosw) return;
  if (memcmp(buf+incfo,"CRYP",4)) return;
  memcpy(buf+incfo,"MSDOS   ",8);
  memcpy(buf+outcfo,"CRYP",4);
  memcpy(buf+outcfo+4,keychk,4);
  writesec(drive,firsthead,firstcyl,1,1,buf);
  printf("\nCryptFlag written at offset %d\n",outcfo);
 } /*ucfoproc*/
