/****************************************************************************
This program reverses the screensaver password encrypt function.
End result is that password is recovered from control.ini

Written by :
Lyal Collins,
7 Trafalgar Street,
Annandale, NSW 2038,
Australia.

Function is based on description in PC MAG, MAY 16, 1995, page 290
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <ctype.h>
#include <string.h>

void rec_password(char * pass_string);
void reverse_password(char * pass_string);
void xor( char x, char y);

char x1;
unsigned char F[80];
char bad_alpha[] = {        //chars not allowed here
0x20, 0x3d, 0x5b, 0x5d, 0x7f, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
0x88,0x89,0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 0x90, 0x93, 0x94, 0x95, 0x96,
0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f
};

char pass_string[80];   // = "algebra123total";
//char pass_string1[80] = "algebra123total";
char *infile;

unsigned char *pass;
int pass_len;

FILE *in;

//    ==========
void main(void)
{
//unsigned long count =0;
clrscr();
printf("\n WINDOWS 3.1 Screen Saver password recovery");
printf("\n Freeware by Lyal Collins, Compuserve 100353,2506\n");

in = fopen("control.ini", "rt");
while (!strstr(F, "Password="))
{
memset(F, 0, sizeof(F));       //make it nul terminated - includes a \n char**
fgets(F, 80, in);              //get the right bit of data = password line
}
pass_len = strlen(F);
pass = strstr(F, "Password=");
pass = pass + 9;               //step over Password=  bit
pass_len -= 10;                //remove "password=" bit


memcpy(&pass_string, pass , pass_len);
pass_len--;
//test//
/*
pass_len = strlen(pass_string)-1;                //remove "password=" bit
strupr(pass_string);
memcpy(&F, &pass_string, pass_len+1);

while(!memicmp(&pass_string, &F, pass_len))
{
 memcpy(&pass_string, &pass_string1 , pass_len);
 memcpy(&F, &pass_string1 , pass_len);
 rec_password(pass_string);
// memcpy(&pass_string1, &pass_string , pass_len);

 reverse_password(pass_string1);
 count++;
 if (count%100 ==0)
 printf ( "\rcount = %ld ", count);
}
*/
//end test
 reverse_password(pass_string);
printf ( "\nBye for now.");
};       //end program
//       ==========

void rec_password(char * pass_string)   //this is the ss encrypt function
{
int j;
strupr(pass_string);
for (j=0; j <= pass_len; j++)
 {
 x1 = pass_string[j];
 xor((char)pass_len+1, x1);  //ok
 if(j == 0)
  xor(0x2a, x1);
 else
 {
 xor((char)j, x1);
 xor(pass_string[j-1],x1);
 }
 pass_string[j] = x1;
}

for (j = pass_len; j >= 0; j--)
 {
  x1 = pass_string[j];
  xor((char)pass_len+1, x1);  //ok
 if(j == pass_len)
  xor(0x2a, x1);
 else
  {
  xor((char)j, x1);
  xor(pass_string[j+1],x1);
  }
 pass_string[j] = x1;
 }
};      //end function
//  ================

void reverse_password(char * pass_string)   //this reverses above encrypt
{
int j;
pass_string[pass_len+1] = 0;            //terminate string
for (j=0; j <= pass_len; j++)          //do opposite of make password
 {                                    //do the backward part fowards
 x1 = pass_string[j];

 if(j == pass_len)
  xor(0x2a, x1);
 else
  {
   xor(pass_string[j+1],x1);
   xor((char)j, x1);
  }
  xor((char)pass_len+1, x1);  //ok
  pass_string[j] = x1;
 }

for (j = pass_len; j >= 0; j--)      //do the foward part backwards
 {
  x1 = pass_string[j];

 if(j == 0)
  xor(0x2a, x1);
 else
 {
  xor(pass_string[j-1],x1);
  xor((char)j, x1);
 }
 xor((char)pass_len+1, x1);  //ok
 pass_string[j] = x1;
}
printf("The password seems to be \t%s \nalthough this is only an educated guess.\n", pass_string);
};      //end function

//      ===============

void xor(char y2, char x2)
{
char a1;

a1 = x2^y2;
//bad chars are 0 - 0x20, 7f-90, 3d, 5b, 5d
//if (!iscntrl(a1) && !strchr(bad_alpha, a1) && a1!=0)
if (isprint(a1) && a1!=0x20 && a1!=0x3d && a1!=0x5b && a1!=0x5d)
x1 = x2^y2;

};
