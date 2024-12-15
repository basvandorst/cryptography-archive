#define TRIPLE_DES
#include <stdio.h>
#include <io.h>
#include <conio.h>
#include <stdlib.h>
#include "3des.h"
#include "usuals.h"
#include "md5.h"

#define BUFFERSIZE 80

#define BS      8               /* ASCII backspace */
#define CR      13              /* ASCII carriage return */
#define LF      10              /* ASCII linefeed */
#define DEL 0177                /* ASCII delete */

char buf[BUFFERSIZE];
MD5_CTX digest;         /* holds md5 calc data */

int getstring(char *strbuf, int maxlen, int echo)
/*      Gets string from user, with no control characters allowed.
        maxlen is max length allowed for string.
        echo is TRUE iff we should echo keyboard to screen.
        Returns null-terminated string in strbuf.
*/
{       short i;
        char c;
        fflush(stdout);
        i=0;
        while (1)
        {
          fflush(stderr);
          c = getch();
          if (c==BS || c==DEL)
           {
            if (i)
             {
               fputc(BS,stderr);
               fputc(' ',stderr);
               fputc(BS,stderr);
               i--;
              }
             continue;
            }
          if (c < ' ' && c != LF && c != CR)
           { putc('\007', stderr);
             continue;
            }
          fputc(echo?c:'*',stderr);
          if (c==CR)
           {
             if (!echo)
              fputc(CR,stderr);
             fputc(LF,stderr);
             break;
            }
          if (c==LF)
           break;
          if (c=='\n')
           break;
          strbuf[i++] = c;
          if (i>=maxlen)
           {
             fprintf(stderr,"\007*\n"); /* -Enough! */
             while (kbhit())
              getch();  /* clean up any typeahead */
             break;
            }
         }
       strbuf[i] = '\0';        /* null termination of string */
       return(i);               /* returns string length */
 }      /* getstring */

/*
 * Get a password. input (from keyboard) may specify a file or an
 * encrypted file with an optional offset.
*/
unsigned char *getpassword(unsigned char *password,int hidden)
 {
   int i,j,result,count;
   char ch,*cp;
   long l;
   result=getstring(buf,BUFFERSIZE-1,hidden);
   if(result==0) return(0);

   for(i=0;i<8;++i)
    password[i]=0;
   count=0;

   if(buf[0]=='@' || buf[0]=='!') /* format: @filename<,offset> */
    {
      FILE *fp;
      char pathname[40];
      long offset=0;
      int is_encoded=buf[0]=='!'?1:0;  /* if '!', must decode password */
      for(cp=buf+1;*cp!=0&&*cp!=',';++cp)
       pathname[count++]=*cp;
      pathname[count]=0;
      if(*cp)
       offset=atol(++cp);
      if((fp=fopen(pathname,"rb"))==NULL)
       {
         printf("Can't open %s\n",pathname);
         return(0);
        }
      l=filelength(fileno(fp));
      if(l < offset+8) /* we'd be reading past end of file */
       {
         printf("Offset too large for file %s\n",pathname);
         fclose(fp);
         return(0);
        }
      fseek(fp,offset,SEEK_SET);
      if(is_encoded) /* must get 2nd password to decode 1st password */
       {
         fread(block,1,8,fp); /* get 1st password */
         printf("Enter aux. password: "); /* ask for second password */
         result=getstring(buf,BUFFERSIZE-1,hidden);
         if(result==0)
          {
            fclose(fp);
            return(0);
           }
         for(cp=buf,count=0;count<8&&*cp!=0;++cp,++count)
          password[count]=*cp;
         while((ch=*cp++)!=0)
          password[count++%8]^=ch;      /* mash extra chars into 1st 8 */
         ascii_adjust_key(password);    /* figure parity */
         schedule(password,de_keytbl);     /* set up for DES decrypt */
         decrypt_block();               /* now true password is in block[] */
         for(i=0;i<8;++i)
          {
            password[i]=block[i];       /* copy and destroy */
            block[i]=0;
           }
        }
      else
       fread(password,1,8,fp);
      fclose(fp);
     }                    /* now password has been read from file */
   else                   /* else password is in buffer */
   if(buf[0]=='#')     /* compute MD5 hash for password */
    {
      MD5Init(&digest);
      MD5Update(&digest,buf+1,result-1);
      MD5Final(&digest);
      for(i=0;i<8;++i)
       password[i]=digest.digest[i]^digest.digest[i+8];
     }
   else
    {
      for(cp=buf,count=0;count<8&&*cp!=0;++cp,++count)
       password[count]=*cp;
      while((ch=*cp++)!=0)
       password[count++%8]^=(ch^0x55);
      ascii_adjust_key(password);       /* figure parity */
     }
   return(password);
  }