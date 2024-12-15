#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include "des.h"
/* #include "usuals.h"*/
#include "md5.h"
#define BUFFERSIZE 80

char password[8];
char buffer[BUFFERSIZE];
MD5_CTX digest;         /* holds md5 calc data */

void print_em(unsigned char *cp);
char parity(char);

int main(int argc, char *argv[])
 {
   int i,result,count=0;
   char *cp,x;
   FILE *fp;
   long l;

   for(i=0;i<8;++i)
    password[i]=0;

   if(argc<2 || argv[1][0]=='?')
    {
      fprintf(stderr,"password <password> displays the 8-character password\n");
      fprintf(stderr,"as interpreted by DES.EXE\n");
      printf("    Password usage:\n");
      printf("1: password_text         uses password_text for password\n");
      printf("2: #password_text        uses MD5 hash of password_text for password\n");
      printf("3: @filename[,offset]    uses data in file [at optional offset] for password\n");
      printf("4: !filename[,offset]    decrypts data in file [at optional offset] for password\n");
      return(1);
     }
   if(argv[1][0]=='@'|| argv[1][0]=='!') /* format: @filename<,offset> */
    {
      char pathname[40];
      long offset=0;
      int is_encoded=argv[1][0]=='!'?1:0;  /* if '!', must decode password */
      for(cp=&argv[1][1];*cp!=0&&*cp!=',';++cp)
       pathname[count++]=*cp;
      pathname[count]=0;
      if(*cp)
       offset=atol(++cp);
      if((fp=fopen(pathname,"rb"))==NULL)
       {
         fprintf(stderr,"Can't open %s for password\n",pathname);
         return(1);
        }
      l=filelength(fileno(fp));
      if(l < offset+8) /* we'd be reading past end of file */
       {
         fprintf(stderr,"Offset too large for file %s\n",pathname);
         fclose(fp);
         return(1);
        }
      fseek(fp,offset,SEEK_SET);
      if(is_encoded) /* must get 2nd password to decode 1st password */
       {
         if(cpu()<486) /* this involves DES486 code now */
          {
            fprintf(stderr,"Can't do this without a 486 processor. Sorry.\n");
            return(1);
           }
         fread(block,1,8,fp); /* get 1st password */
         fprintf(stderr,"Enter password: ");
         result=getstring(buffer,BUFFERSIZE-1,1);
         if(result==0) return(1);  /* quit program if no 2nd password */
         for(cp=buffer,count=0;count<8&&*cp!=0;++cp,++count)
          password[count]=*cp;
         while((x=*cp++)!=0)
          password[count++%8]^=x;       /* mash extra chars into 1st 8 */
         ascii_adjust_key(password);    /* figure parity */
         schedule(password,keytbl);     /* set up for DES decrypt */
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
      print_em(password);
      return(0);
     }
  /* else password is on cmd line */
   if(argv[1][0]=='#')
    {
      for(i=0;argv[1][i];++i)
       ;
      MD5Init(&digest);
      MD5Update(&digest,&argv[1][1],i-1);
      MD5Final(&digest);
      for(i=0;i<8;++i)
       {
         password[i]=digest.digest[i];
         password[i]^=digest.digest[i+8];
        }
      print_em(password);
      return(0);
     }
   for(cp=argv[1];count<8&&*cp!=0;++cp,++count)
      password[count]=*cp;
   while((x=*cp++)!=0)
    password[count++%8]^=(x^0x55);
   for(i=0;i<8;++i)
    password[i]=parity(password[i]);
#ifndef COMPAT
   for(i=0;i<8;++i)
    {
      x=password[i]&128;                /* rotate password byte left 1 */
      password[i]<<=1;
      password[i]|=x?1:0;
     }
#endif
   print_em(password);
   return(0);
  }


void print_em(unsigned char cp[])
 {
   int i;
   for(i=0;i<8;++i)
    printf("%02X ",cp[i]);
   printf("       ");
   for(i=0;i<8;++i)
    printf("%c",cp[i]>31&&cp[i]<128?cp[i]:'.');
   printf("\n");
  }

char parity(char input) /* computes odd parity */
 {
   char pbit=1;
   int i;
   for(i=0;i<7;++i)
   if(input&(1<<i))
    pbit ^=1;
   return(input|(pbit?128:0));
  }

#define BS      8               /* ASCII backspace */
#define CR      13              /* ASCII carriage return */
#define LF      10              /* ASCII linefeed */
#define DEL 0177                /* ASCII delete */
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
