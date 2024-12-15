/*
 *                        A C shell for DES486.asm.
 *            This one implements Triple DES in Stream Cipher Mode
 *              with IV/Cipher Feedback (for Error Propogation).
 *            This program is designed to be suitable for long-term
 *                        storage of sensitive data.
 */

#define TRIPLE_DES
#define BUFFERSIZE 512
#define DOTSIZE (10000/BUFFERSIZE) /* put dot on screen every 10K */

#include "3des.h"        /* holds prototypes for asm functions */
#include <stdio.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <dir.h>
#include <dos.h>
#include <io.h>
#include <mem.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;
typedef unsigned char *byteptr;

extern int _fmode;
extern byteptr getpassword(byteptr password, int hidden);

int crypt_to_source(void);      /* same func for encrypt & decrypt */
int crypt_to_dest(void);        /* same func for encrypt & decrypt */
int (*action)(void);            /* assignable pointer */

void who_i_am(void);
void parsefile(char *filename, char *iv);
void xorbuf(byteptr output, byteptr input, int count);
int getstring(char *strbuf, int maxlen, int echo);

unsigned char password[8];
byte buffer[BUFFERSIZE];
struct fatinfo dtable;
struct ftime filetime;
struct ffblk searchbuf;
char tempname[13+20];   /* 20 chars for path */
int dest_specified;
char dest_path[20];
FILE *fp1,*fp2;
long oldsize,newsize,l;
int on_decrypt=0;
char *diskfullmsg="Disk full:   ";
byte iv[8];             /* the initialization vector */

int main(int argc, char *argv[])
 {
   int argcount;
   char **argvec;
   char drivename=0, drivebuf[3];
   int old_drive;
   char pathname[MAXDIR],old_path[MAXDIR],pathbuf[MAXDIR];
   char filename[13];
   char namebuf[MAXFILE],extbuf[MAXEXT];
   char *c,*cp,ch;
   int i,j,result,done,count,hidden;

   if((result=cpu())<486)
    {
      who_i_am();
      printf("This program needs a 486 cpu to run.\n");
      printf("Can't run on your %d\n",result);
      return(1);
     }

   dest_specified=0;   /* default: over-write the input file */
   action=crypt_to_source;
   result=0;    /* flag: no errors in cmd line */
   count=0;     /* flag: set when crypt or decrypt specified */
   hidden=0;    /* flag: if prompting for password, don't hide input */

   argcount=1;
   argvec=argv+1;
   while(argvec[0][0]=='-')
    {
      switch (argvec[0][1])
       {
         case 'd':      /* decrypt */
         case 'D':
          count++;
          on_decrypt=1;
          break;
         case 'c':      /* crypt */
         case 'C':
          count++;
          break;
         case 'h':      /* hide the password */
         case 'H':
          hidden=1;
          break;
         case 'o':      /* output path specified */
         case 'O':
          dest_specified=1;
          action=crypt_to_dest;
          strncpy(dest_path,&argvec[0][2],19);
          strupr(dest_path);    /* make upper-case */
          i=0;
          while(dest_path[i])
                ++i;
          --i;
          if(dest_path[i]!='\\' && dest_path[i]!=':')
           strcat(dest_path,"\\");      /* ensure path properly terminated */
          break;
         default:
          result=1;     /* invalid switch is error */
        }
      ++argcount;
      ++argvec;
     }
   if(count==0)         /* encrypt/decrypt not specified */
    result=1;
   if(argc-argcount<1)  /* too few arguments */
    result=1;

   if(result)   /* print usage screen and die */
    {
      who_i_am();
      return(1);
     }


                /* parse the password */

   printf("Enter password 1: ");
   cp=getpassword(password,!hidden);
   if(cp==0) return(1);             /* exit if no password given */
   schedule(cp,en_keytbl);          /* make 16 rounds of keys */
   printf("Enter password 2: ");
   cp=getpassword(password,!hidden);
   if(cp==0) return(1);             /* exit if no password given */
   schedule(cp,de_keytbl);          /* make 16 rounds of keys */


        /* Now deal with drives and directories */

   old_drive=getdisk();         /* get current drive */
   getcwd(old_path,MAXDIR);     /* get current directory */
   drivename=old_drive+'A';
   memset(pathname,0,MAXDIR);
   memset(filename,0,13);

   result=fnsplit(argv[argc-1],drivebuf,pathbuf,namebuf,extbuf); /* filespec */
   if(result&DRIVE)
    drivename=drivebuf[0];
   if(result&DIRECTORY)
    {
      strcpy(pathname,pathbuf);
      if(pathname[strlen(pathname)-1]=='\\')
       pathname[strlen(pathname)-1]=0;    /* chdir barfs on trailing '\' */
     }

   if(drivename)
    setdisk((int)toupper(drivename)-'A');
   if(pathname[0])
    if(chdir(pathname))
     {
       setdisk(old_drive);
       printf("Can't find path %s\\ on drive %c:\n",pathname,drivename);
       for(i=0;i<BUFFERSIZE;++i)
        buffer[i]=0;    /* zero out password spec in buffer */
       killkey();        /* zero out key schedule */
       return(1);
      }
   strcpy(filename,namebuf);
   strcat(filename,extbuf);


                /* main processing loop */

   result=0;
   count=0;
   done=findfirst(filename,&searchbuf,0);
   while(!done&&result>=0)
    {
      ++count;
      result=do_process();
      done=findnext(&searchbuf);
     }
   printf("\n%d %s found.\n",count,count==1?"file":"files");
   setdisk(old_drive);
   chdir(old_path);

   for(i=0;i<BUFFERSIZE;++i)
    buffer[i]=0;        /* in case no files, kill password spec in buffer */
   killkey();           /* zero out key schedule */
   return(0);
  }                     /* normal return from main() */


/*  do_process() Return from this func can be:
  ( 0) normal successful file processing
  ( 1) non-fatal error (continue trying other files)
  (-1) fatal: give up trying other files
*/
int do_process()
 {
   int result;
   int i,j;

   /* may we over-write? */
   if((searchbuf.ff_attrib & FA_RDONLY) && !dest_specified)
    {
      printf("%s:   Access Denied\n",searchbuf.ff_name);
      return(1);
     }

   fp1=fopen(searchbuf.ff_name,dest_specified?"rb":"r+b");
   if(fp1==NULL)
    {
      printf("Can't open %s for input\n",searchbuf.ff_name);
      return(1);
     }

   if(dest_specified)
    {
      strcpy(tempname,dest_path);
      strncat(tempname,searchbuf.ff_name,32);
      fp2=fopen(tempname,"wb");
      if(fp2==NULL)
       {
         printf("Can't open %s for output\n",tempname);
         fclose(fp1);
         return(1);
        }
     }
   getftime(fileno(fp1),&filetime);         /* save file time & date */

   parsefile(searchbuf.ff_name,iv); /* set IV seed from filename */

   if((result=action())!=0)    /* encrypt/decrypt done here */
    {
      printf("Aborted\n");
      fclose(fp1);
      if(dest_specified)
       { fclose(fp2);
         unlink(tempname);         /* erase temp file */
        }
      return(result);
     }

   setftime(fileno(dest_specified?fp2:fp1),&filetime); /* reset to old time */
   if(dest_specified)
    fclose(fp2);
   fclose(fp1);

   printf("done\n");
   return(0);                           /* normal successful return */
  }




int crypt_to_source()
 {
   int readsize, count,chunksize, blocknum=0;
   long readpos=0, writepos=0;
   byte *buf;

   printf("%s %s:   ",on_decrypt?"Decrypting: ":"Encrypting: ",\
    searchbuf.ff_name);
   while((count=fread(buffer,1,BUFFERSIZE,fp1))>0)
    {
      readsize=count;
      readpos += readsize;
      buf=buffer;
      while((chunksize=min(count,8))>0)
       {
         mov64(block,iv);
         encrypt_block();       /* encrypt IV. Result is in block[] */
         decrypt_block();
         encrypt_block();
         if(on_decrypt)         /* we're decrypting */
          xor64(iv,buf);
         xor64(buf,block);
         if(!on_decrypt)        /* we're encrypting */
          xor64(iv,buf);
         count -= chunksize;
         buf += chunksize;
        }
      fseek(fp1,writepos,SEEK_SET);
      if(fwrite(buffer,1,readsize,fp1)!=readsize)
       {
         fprintf(stderr,diskfullmsg);
         return(-1);    /* fatal error */
        }
      writepos += readsize;
      fseek(fp1,readpos,SEEK_SET);
      if(++blocknum==DOTSIZE)
       {
         blocknum=0;
         fputc('.',stdout);  /* show user we're still alive... */
        }
     }
   fflush(fp1);
   return(0);
  }

int crypt_to_dest()
 {
   int readsize, count,chunksize;
   int blocknum=0;
   byte *buf;

   printf("%s %s to %s: ",on_decrypt?"Decrypting":"Encrypting",\
    searchbuf.ff_name,tempname);
   while((count=fread(buffer,1,BUFFERSIZE,fp1))>0)
    {
      readsize=count;   /* save for file writes */
      buf=buffer;
      while((chunksize=min(count,8))>0) /* do while (buffer) */
       {
         mov64(block,iv);
         encrypt_block();
         decrypt_block();
         encrypt_block();
         if(on_decrypt)   /* we're decrypting */
          xor64(iv,buf);
         xor64(buf,block);
         if(!on_decrypt)   /* we're encrypting */
          xor64(iv,buf);
         count -= chunksize;
         buf += chunksize;
        }
      if(fwrite(buffer,1,readsize,fp2)!=readsize)
       {
         fprintf(stderr,diskfullmsg);
         return(-1);    /* fatal error */
        }
      if(++blocknum==DOTSIZE)
       {
         blocknum=0;
         fputc('.',stdout);  /* show user we're still alive... */
        }
     }
   fflush(fp1);
   fflush(fp2);
   return(0);
  }

void who_i_am()
 {
   clrscr();
   printf("\n3DES: a fast, secure stream-mode Triple-DES encryption program for 486 ISA PCs.\n");
   printf("   By Steve Allen     73277.620@compuserve.com (CIS 73277,620)\n");
   printf("\nUsage: 3DES -c|d -h [-oOUTPUTPATH] filespec\n");
   printf("-c -d crypt or decrypt (must specify)\n");
   printf("-h hide password input\n");
   printf("[-oOUTPUTPATH] place output file here\n");
   printf(" (If OUTPUTPATH not specified, file is over-written)\n");
   printf("Wildcards OK in filespec\n\n");
   printf("    Password usage:\n");
   printf("1: password_text         uses password_text for password\n");
   printf("2: #password_text        uses MD5 hash of password_text for password\n");
   printf("3: @filename[,offset]    uses data in file [at optional offset] for password\n");
   printf("4: !filename[,offset]    decrypts data in file [at optional offset] for password\n");
  }

/* seed the IV with the filename */
void parsefile(char *filename, char *iv)
 {

   char *cp;
   int i,j;

   cp=filename;
   for(i=0,j=0;i<8&&cp[i];++i)
    iv[j++]=cp[i];
   while(i<13&&cp[i])
    {
      iv[j%8]^=cp[i];   /* mash extra chars into 1st 8 bytes */
      ++i;
      ++j;
     }
   while(j<8)
    iv[j++]=0;
  }
