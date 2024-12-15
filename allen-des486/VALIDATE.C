/* 
 * validate des486 code using NBS data (in "validate.txt")
 */


#include <stdio.h>
#include <ctype.h>
#include "des.h"
typedef unsigned char ubyte;

ubyte linebuf[80];
ubyte password[8];
ubyte password_copy[8];
ubyte plaintext[8];
ubyte answer[8];        /* holds validation cryptotext */

ubyte *getbyte(ubyte *output,char *input)
 {
   char c;
   int ch;
   while((c=*input++)==' ')
        ;
   ch=toupper((int)c);
   ch-='0';
   if(ch>9)
    ch-=7;
   *output=ch*16;
   ch=toupper((int)*input++);
   ch-='0';
   if(ch>9)
    ch-=7;
   *output += ch;
   return(input);
  }
ubyte* parse(char *text,ubyte *output)
 {
   int i;
   for(i=0;i<8;++i)
    text=getbyte(output+i,text);
   return(text);
  }

int main()
 {
   FILE *fp;
   ubyte *cp;
   int i,linecount=0,errors=0;

   if(cpu()<486)
    {
      printf("This program needs a 486 cpu to run\n");
      return(1);
     }
   fp=fopen("validate.txt","rt");
   if(fp==NULL)
    {
      printf("Can't find VALIDATE.TXT in current directory\n");
      return(1);
     }
   while(fgets(linebuf,79,fp))
    {
      if(linebuf[0]==';' || linebuf[0]=='#')
       continue;
      ++linecount;
      cp=linebuf;
      cp=parse(cp,password);
      cp=parse(cp,block);
      cp=parse(cp,answer);
      for(i=0;i<8;++i)
       { 
         plaintext[i]=block[i];
         password_copy[i]=password[i];
        }
      schedule(password,keytbl);
      encrypt_block();
      for(i=0;i<8;++i)
       if(block[i]!=answer[i])
        {
          printf("bad encryption data in line %d\n",linecount);
          ++errors;
          break;
         }
    for(i=0;i<8;++i)
     block[i]=answer[i];
    decrypt_block();
    for(i=0;i<8;++i)
     if(block[i]!=plaintext[i])
      {
        printf("bad decryption data in line %d\n",linecount);
        ++errors;
        break;
       }
     }
   fclose(fp);
   printf("\n%d errors\n",errors);
   return 0;
  }