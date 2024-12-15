#include <ctype.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include "otp.h"

/* this routine extracts the short filename from a full path.
   The filename is stored in char *filename */

int extract_name(char *fullpath,char *filename)
{

int location ;

 location=find_right_char(fullpath,'\\');  /* find rightmost '\' */
 if (location==-1) location=find_right_char (fullpath,':'); /* try for ':' */
  if(location==-1)  /* if we didn't find a '\' or a ':' then there is no path */
   strcpy(filename,fullpath);
    else
   copy(fullpath,filename,location+1, strlen(fullpath)-location);  /* chip off path */
 return (0);
}




/**********************   L_GETKEY.C  ***************************/


/*****************************************************************

 Usage: void get_key(char *ch, char *ext);


  char *ch=  character to read from keyboard.

  char *ext= extended character (if any).

  Reads character and extended character from keyboard.

*****************************************************************/


void get_key(char *ch, char *ext)
{
  *ch=getch();      /* get the character */

    if(!*ch){       /* if the character is zero (a special key) */
      *ext=getch(); /* get the extension */
    }

}


/* wait for user to press a 'y' or 'n'.  Case not significant */

char yes_no(void)
{
char ch=' ',ext=' ';
 while(ch!='Y' && ch!='N'){
  get_key(&ch,&ext);
  ch=toupper(ch);
 }
 putchar(ch);putchar('\n');
return (ch);
}




/*   I didn't use this yes_no function, but have left it in for anyone
     doing a port.  It should work in more environments than the one listed
     above
char yes_no()
{
char ch=' ';
 while(ch!='Y' && ch!='N'){
  ch=getch();
  ch=toupper(ch);
 }

return (ch);
}

*/

/* remove the last occurance of a specified character */

void rem_trail( char *string, char ch)
{
int len;

len=strlen(string);

if (string[len-1]==ch)
   string[len-1]='\0';
}







/**********************   L_CHIP.C   ***************************/


/*****************************************************************

 Usage: void chip_left(char *chip,char *block,int number);

  char *chip  = string to receive characters.
  char *block = string to lose characters.
  int  number = number of characters to move.

  Takes a "chip of the old block", removing characters from the
  left side of "block" and placing them in "chip".

  Example: block= "this is a test"
           chip= ""
           chip_left(&chip,&block,4);

           results: chip=  "this"
	   block= " is a test"
*****************************************************************/



void chip_left(char *chip,char *block,int number)

{
char temp[MAX_STRING];
strcpy(chip,"\0");
 if (number <1 ) return;  /* nothing to chip */
 if (number > strlen(block)) number=strlen(block);  /* attempt to chip too much */
 strcpy (temp,block);
 copy (block,chip,0,number);
 copy(block,temp,number,strlen(block)-number);
 strcpy (block,temp);
 return;
}


/*****************************************************************

 Usage: void chip_right(char *chip,char *block,int number);

  char *chip  = string to receive characters.
  char *block = string to lose characters.
  int  number = number of characters to move.

  Works just like chip_left() but moves characters from the
  right side.

*****************************************************************/


void chip_right(char *chip,char *block, int number)
{
char temp[MAX_STRING];
strcpy(chip,"\n");
if (number <1 || number >strlen(block))return;
copy(block,chip,strlen(block)-number,number);
copy (block,block,0,strlen(block)-number);
return;
}





int find_right_char( char *string, char ch)
{
int pos;
int length;

for(length=strlen(string)-1;length>=0;length--)
 if (string[length]==ch) return(length);
 return -1;
}


/* this function prints out an array of char, it stops when
   it gets to an empty string ""  */

void PrintArray(int start, char *array[])
{


int done=FALSE;

 for(;;){
  fprintf(stdout,array[start++]);
  if ((array[start][0])=='\0')break;
 }
}