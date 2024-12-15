/* Secure Drive LOGIN V1.3A */
/* Logs into encrypted disks */

#include "secdrv.h"

extern char pass1[MAXPASS];
extern char pass2[MAXPASS];
extern char Old13Check[4];

extern int  tsr_not_installed;
extern int  tsr_wrong_version;
extern int  df10,ef10;
extern char compat_mode;

int   setenv(char*,char *);
void  clrbufs(void);
char *pgpv;

int main(int argc,char *argv[])
{
unsigned drive,firstcyl,firsthead,maxcyl;
unsigned maxhead,maxsector,secsize,i;
unsigned safemode=FALSE;
unsigned pgpsw=FALSE;
unsigned char buf[512],key[16],check[4];
word16 expkey[52];
unsigned serial[2];
unsigned char *p;
char drvltr;
int ekeyexp=0;

if(argc==1)
 {
    printf("\n\
Secure Drive Login Version 1.3A\n\
This program sets parameters and loads passphrases for encrypted\
 drives.\n\n\
LOGIN /F  [/PGP]                 to enter floppy disk passphrase\n\
                                 [use/set PGPPASS]\n\
\n\
LOGIN /C  [/PGP]                 to erase keys and disable encryption\n\
                                 [clear PGPPASS]\n\
\n\
LOGIN drive letter  [/PGP]       to activate an encrypted hard drive\n\
                                 [use/set PGPPASS]\n\
LOGIN D:    (example)\n\
LOGIN D: /S                      to prevent accidental access to an\n\
                                 encrypted drive without logging in\n\
\n\
LOGIN /PGP                       to set PGPPASS environment variable\n\
\n\
LOGIN drive cylinder head        to manually enter parameters for\n\
                                 a hard drive partition\n\
LOGIN drive cylinder head /S     to prevent accidental access\n\
LOGIN 0 100 1 (example)\n\
      drives are numbered from zero\n\n");
    exit(1);
  }


for (i=1;i<argc;i++)
 {
   if (stricmp(argv[i],"/PGP") == 0)
    {
     pgpsw=TRUE;
     pgpv=getenv("PGPPASS");
     break;
    }
 }

if (argc == 2 && pgpsw)
 {
  printf("\nEnter PGPPASS passphrase for PGP Secret Key: ");
  invgets(pass1);
  if(pass1[0] != 0)
   {
    if (setenv("PGPPASS",pass1) == 0)
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

if(!(cryptdata=gettsradr()))
 {
  printf("\nError: Secure Drive TSR %s.\n",
         tsr_wrong_version ? "wrong Version" : "not loaded");
  exit(1);
 }



if((*argv[1]=='/')&&(toupper(*(argv[1]+1))=='C'))
{
  bdos(0x0D, 0, 0);          /* Reset Disk Subsystem - Flush all buffers */
  for (i=0;i<MAXDRV;i++)
   cryptdata->hd[i].active=0;

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

set_compat_mode();

if((*argv[1]=='/')&&(toupper(*(argv[1]+1))=='F'))
 {
  char c;
  int drive;
  int fdpr=1;
  while (1)
   {
    printf("\nInsert encrypted diskette in drive A or B\n\
then press A or B to check passphrase.\n\
Or Press Esc to bypass passphrase check.: ");
    while ((c=toupper(getch())) != 'A' && c != 'B' && c != ESC) {}
    if (c == ESC) break;
    printf("%c\n\n",c);
    drvltr=c;
    drive=(unsigned) c-'A';
    readsec(drive,0,0,1,1,buf);
    if((buf[510]!=0x55)||(buf[511]!=0xaa))
     {
      printf("This diskette is not formatted.\n\n");
      continue;
     }
    if(memcmp(buf+3,"CRYP",4) != 0)
     {
      printf ("This diskette is not encrypted.\n\n");
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
       if(memcmp(cryptdata->hkeychk,buf+7,4) == 0)
        {
         memcpy(check,cryptdata->hkeychk,4);
         memcpy(expkey,cryptdata->hkey,104);
         ef10 = df10 = cryptdata->fkeyv10 = cryptdata->hkeyv10;
         ekeyexp=1;
         fdpr=0;
         printf("Floppy disk encryption enabled with\n\
verified active hard disk partition %s passphrase.\n",
          df10 ? "(V 1.0)" : "(V 1.1)");

        }
      }
     else /*verify bypassed*/
      {
       printf("Use active Hard Disk passphrase as unverified floppy passphrase? :");
       if (getyn())
        {
         memcpy(check,cryptdata->hkeychk,4);
         memcpy(expkey,cryptdata->hkey,104);
         ef10 = cryptdata->hkeyv10;
         ekeyexp=1;
         fdpr=0;
         printf("Floppy disk encryption enabled with\n\
unverified active hard disk partition %s passphrase.\n",
          ef10 ? "(V 1.0)" : "(V 1.1)");
        }
      }
    }
  }

   if (fdpr && pgpsw && pgpv != NULL)
    {
     strcpy(pass1,pgpv);
     if (c != ESC)
      {
       setkeydf(key,check,buf+7);
       if(!df10 && !memcmp(Old13Check,buf+7,4))
        {
         printf("Check bytes in Disk %c: Boot Sector need updating from 1.3 to 1.1/1.3A. Proceed? ",
                drvltr);
         if(getyn())
          {
           memcpy(buf+7,check,4);
           writesec(drive,0,0,1,1,buf);
          }
        }

       ef10=df10;
       if(memcmp(check,buf+7,4) == 0)
        {
         printf("\nPGPPASS entered as %s floppy disk passphrase\n",
               df10 ? "(V 1.0)" : "(V 1.1)");
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
       getkeydf(key,check,buf+7);
       if(!df10 && !memcmp(Old13Check,buf+7,4))
        {
         printf("Check bytes in Disk %c: Boot Sector need updating from 1.3 to 1.1/1.3A. Proceed? ",
                drvltr);
         if(getyn())
          {
           memcpy(buf+7,check,4);
           writesec(drive,0,0,1,1,buf);
          }
        }
       ef10=df10;
       if(memcmp(check,buf+7,4) == 0)
        {
         printf("Floppy disk encryption enabled with\n");
         printf("verified %s passphrase.\n",
                 df10 ? "(V 1.0)" : "(V 1.1)");
        }
       else
        {
         printf("Wrong passphrase.\n");
         exit(1);
        }
      }
     else
      {
       getkey(key,check,FALSE);
       printf("Floppy disk encryption enabled with\n");
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
   if (pgpsw && pgpv == NULL && setenv("PGPPASS",pass1) == 0)
    printf("PGPPASS set. \n");
   bdos(0x0D, 0, 0);          /* Reset Disk Subsystem - Flush all buffers */
   clrbufs();
   exit(0);
 }

else if(isalpha(*argv[1]))
 {
  drvltr=toupper(*argv[1]);
  drive=255;
  readptbl(drvltr,&drive,&firsthead,&firstcyl);   /*1.1*/
  if(drive==255) {
      printf("\nDrive not found.\n");
      exit(1); }
  if((*argv[2]=='/')&&(toupper(*(argv[2]+1))=='S'))
      safemode=TRUE;
  printf("\nDrive %c is physical hard drive %u, head %u,\
 cylinder %u\n",drvltr,drive,firsthead,firstcyl);
 }

else
 {
  drvltr='X';
  i=sscanf(argv[1],"%u",&drive);
  i=i&&sscanf(argv[2],"%u",&firstcyl);
  i=i&&sscanf(argv[3],"%u",&firsthead);
  if((*argv[4]=='/')&&(toupper(*(argv[4]+1))=='S'))
      safemode=TRUE;
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
    printf("This is not a boot sector.\n");
    exit(1); }

calcdiskparams(buf,&maxcyl,&maxhead,&maxsector,
               &secsize,serial);

printf("Disk %c: has: %i cylinders, %i sectors, %i heads, sector \
size %i bytes\n",drvltr,maxcyl+1,maxsector,maxhead,secsize);

if(memcmp("CRYP",buf+3,4)) {
    printf("This disk is not encrypted.\n\n");
    exit(1); }

if(!safemode)
{
 int hdpr=1;
 if (pgpsw && pgpv != NULL)
  {
   strcpy(pass1,pgpv);
   setkeydf(key,check,buf+7);
   if(!df10 && !memcmp(Old13Check,buf+7,4))
    {
     printf("Check bytes in Disk %c: Boot Sector need updating from 1.3 to 1.1/1.3A. Proceed? ",
            drvltr);
     if(getyn())
      {
       memcpy(buf+7,check,4);
       writesec(drive,firsthead,firstcyl,1,1,buf);
      }
    }
   if(memcmp(check,buf+7,4))
    {
     printf("\nPGPPASS is wrong passphrase.\n");
     hdpr=1;
    }
   else
    {
     printf("\nPGPPASS entered as %s hard disk passphrase.\n",
            df10 ? "(V 1.0)" : "(V 1.1)");
     hdpr=0;
    }
  }
 if (hdpr)
  {
   printf("\nEnter hard disk passphrase: ");
   getkeydf(key,check,buf+7);
   if(!df10 && !memcmp(Old13Check,buf+7,4))
    {
     printf("Check bytes in Disk %c: Boot Sector need updating from 1.3 to 1.1/1.3A. Proceed? ",
            drvltr);
     if(getyn())
      {
       memcpy(buf+7,check,4);
       writesec(drive,firsthead,firstcyl,1,1,buf);
      }
    }
  }

 if(memcmp(check,buf+7,4))
  {
   printf("Wrong passphrase.\n");
   exit(1);
  }

 en_key_idea((word16 *)key,expkey);

 memcpy(cryptdata->hkey,expkey,104);
 memcpy(cryptdata->hkeychk,check,4);
 cryptdata->hkeyv10=(char) df10;
}

bdos(0x0D, 0, 0);          /* Reset Disk Subsystem - Flush all buffers */
/*set all slots inactive*/
for (i=0;i<MAXDRV;i++)
 {
   if (cryptdata->hd[i].active)
    printf("Drive %c: set to Safe mode.\n",cryptdata->hd[i].drvltr);
   cryptdata->hd[i].active=0;
 }

/*find empty or duplicate HD slot*/
for (i=0;i<MAXDRV;i++)
 {
   if (  cryptdata->hd[i].dddrv    == 0  ||
        (cryptdata->hd[i].dddrv    == drive &&
         cryptdata->hd[i].firstcyl == firstcyl) )
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
     if(!safemode)
      {
       cryptdata->hd[i].serial[0]=serial[0];
       cryptdata->hd[i].serial[1]=serial[1];
       cryptdata->hd[i].active=1;
       printf("\nHard disk %s encryption enabled logical drive %c:.\n",
              df10 ? "(V 1.0)" : "(V 1.1)",
              drvltr);
       if (pgpsw && pgpv == NULL && setenv("PGPPASS",pass1) == 0)
        printf("PGPPASS set. \n");
       break;
      }
     else
      {
       printf("\nHard disk set to safe mode.\n");
       break;
      }
    }
 }

if (i == MAXDRV)
 printf("\nERROR: More than %d Encrypted Partitions not supported\n",MAXDRV);

for(i=0;i<16;i++) key[i]='\0';
for(i=0;i<52;i++) expkey[i]=0;

bdos(0x0D, 0, 0);          /* Reset Disk Subsystem - Flush all buffers */
clrbufs();
return(0);
}

