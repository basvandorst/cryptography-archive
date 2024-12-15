//  TOMB.EXE cracker.

//  program code to crack "TOMB" encryption rountine
//  based on observation of test files "encrypted" by the tomb.exe program
//  looking for a common char that decrypts to an ascii char.
//  a string of 10 psuedo-random bytes are effectively XOR'ed with message text
//  essentially, a lesson in statistical analysis, with a couple of
//  proteted test files giving the vital clues.


#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <ctype.h>
#include <mem.h>

#include "key.cpp"
#include "keylist.cpp"
#include "tmbcrk.h"

char *set_digits = "/d";
char *set_upper = "/u";
char *set_lower = "/l";

//-------------
void main(int, char **);      // Main function


void main( int argc, char *argv[])
{
if ( argc < 4)
{
DO_MAIN_HELP();
exit(-3);
}

for(i=0; i <= argc; i++)
{
    if( stricmp(argv[i], set_lower) == 0)
        strcat(Alphabet, alpha_lower);

    if( stricmp(argv[i], set_digits) == 0)
        strcat(Alphabet, alpha_digits);

    if( stricmp(argv[i], set_upper) == 0)
        strcat(Alphabet, alpha_upper);
}
strcpy(filename, argv[2]);               //keep a copy of the out filenamr
enc_string_offset = 20;      //is above 16, no blinking is shown, on first try

in = NULL;                   //clear some values
out = NULL;
//  Open the files, give an error if problems arise.
if ( !(in = fopen( argv[1], "rb")) )
    Error( "Can\'t open %s.\n", argv[1]);
if ( (out = fopen( argv[2], "rb")) ) //test if output file exists
    Error( "File already exists. Us a different output filename.\n", argv[2]);

clrscr();
normvideo();
printf("Text recovery for TOMB.EXE V1.00 protected files.\n\n");
memset(workblock, 0, sizeof(workblock));   //clear memory block
do_tomb();
printf("The recovered text is in the file %s\n", filename);
exit(0);
}           // End of main
//-------------

void find_key()
{
memset(maxcount_string, 0, sizeof(maxcount_string));    //ensure this is clear
for( i=0; i < 256; i++)                       //each possible byte up to 256
{
     memset(count_string, 0, sizeof(count_string));
     for( l=0; l <= blocks; l++)              //up to max
         {
         for( j=0; j < LINE_MAX; j++)         //do a pass over 1 key length
              {
              test = workblock[l][j] ^ i;     //do xor with i
              if( test !=0)                   //ensure 0 doesn't bias results
                  {
                  if( strchr(Alphabet, test)) //in my alphabet ??
                      count_string[j]++;      //increment the value at j
                  }   //end if test
              };      //end for j
         };           //end for l

for( k=0; k < LINE_MAX; k++)              //test for most common result
     {
     if( count_string[k] > maxcount_string[k])
         {
         enc_string[k] = i;                 //set key to current max key value
         maxcount_string[k] =count_string[k]; //top up current maximum count
         }                       //if
     }                           //for k loop
}                                //for i loop
};                               //end find_key
//--------------

void do_tomb(void)              //main action happen here
{
blocks = BLOCK_MAX;             //20 byte header, password related, step over
fseek(in, 20, SEEK_SET);        //data starts at pos 20 in file
offset = fread((char *)workblock, LINE_MAX, blocks, in); //Read input file
blocks = offset;
find_key();                     //derive key block
display_workbuf();              //show first attempt results
window(1,1,80,25);
gotoxy(1,25);                   //bottom left
clreol();
printf("A sample of the recovered text. If correct, (Y) or press (N) to alter");
if(is_ok() == 2)                //is result ok ??
 {
 enc_string_offset = 0;
 key_adjust();                   //no, then adjust it, else
 }
window(1,1,80,25);
gotoxy(1,25);                   //bottom left
clreol();
printf("Do you want to save the recovered text to a file ? Y,n   ");
if(is_ok() == 1)                //ask if they want to save recovered text
 dump_to_disk();                //if yes, do so - may lose a few bytes
};                              //from end of original file

//-------------
void key_adjust(void)       //now, adjust the key, if necessary
{
do_after_key();
window(1,1,80,25);
gotoxy(1,25);               //bottom left
clreol();
printf("Adjust key values, then press Y, or ESC to continue.");
dispatch();
};
//-------------
void dispatch(void)		    //does hard work during edit section
{
	ch=getKey();
	while (ch != KEY_F10 )  //f10 or esc are exit keys
		{		            //test input keys
		switch(ch)          //then act acordingly
		{
            case KEY_ESC:  return;      //another get out point

			case KEY_LEFT: DO_KEY_LEFT();
			do_after_key();	 //do this after each command key
			ch=0;		     //SET CHAR TO NULL
			break;

			case KEY_RIGHT: DO_KEY_RIGHT();
			do_after_key();	 //do this after each command key
			ch=0;			//SET CHAR TO NULL
			break;

			case KEY_HOME: 	DO_KEY_HOME();
			do_after_key();	 //do this after each command key
			ch=0;			//SET CHAR TO NULL
			break;

			case KEY_END: 	DO_KEY_END();
			do_after_key();	 //do this after each command key
			ch=0;			//SET CHAR TO NULL
			break;

			case KEY_UP: 	DO_KEY_UP();
			do_after_key();	 //do this after each command key
			ch=0;
			break;

			case KEY_DOWN: 	DO_KEY_DOWN();
			do_after_key();	 //do this after each command key
			ch=0;
			break;

			case KEY_F1:	DO_MAIN_HELP();
			ch=0;
			break;

			case UPPER_Y:  return;
            case LOWER_Y :  return;
            case RETURN :  return;
			default: 	ch=getKey();
			}			//end switch
		}	            //end while
}
//end dispatch
//------------------
void DO_MAIN_HELP(void)
{
int i;
window(left, top, right, bottom);			    //whole screen window
gettext(left,top,right, bottom,&scrn_buffer); 	//save scrn
for(i=0; i<25; i++)		//clear it
delline();
printf(help_buffer);	                //print it
while(!kbhit());		                //wait for key press
puttext(left, top, right, bottom, &scrn_buffer);
};
//----------

void DO_KEY_UP(void)	                //page up - take off 1 line
{
enc_string[enc_string_offset]++;
};
//--------
void DO_KEY_DOWN(void)		            //page down - add 1 line
{
enc_string[enc_string_offset]--;
};
//--------------
void DO_KEY_LEFT(void)		            //left arrow sub 1 pos
{
if( enc_string_offset != 0)
enc_string_offset--;
else
enc_string_offset = LOOP_MAX;
};
//---------------
void DO_KEY_RIGHT(void)	                //left arrow add 1 pos
{
if( enc_string_offset == LOOP_MAX)
enc_string_offset = 0;
else
enc_string_offset++;
};
//------
void DO_KEY_END(void)	                //end - goto end pos
{
enc_string_offset = LOOP_MAX;
};
//----------------
void DO_KEY_HOME(void)	            //home - goto start pos
{
enc_string_offset = 0;
};
//---------------
void do_after_key(void)		        //do this after each command key
{
int i;
textattr(WHITE);	                // normal again
normvideo();
window(1,1,80,25);
gotoxy(1,3);                        //show the enc string
printf("Derived Keystream  :-  ");  //show what's happening
for( i=0; i < LINE_MAX; i++)
     {
     if( i != enc_string_offset)
         {
            cprintf("%02X ", enc_string[i]);
         }
         else
         {
            textattr(WHITE + BLINK + (BLACK << 4));	    // blink highlighted char
            cprintf("%02X ", enc_string[i]);
            textattr(WHITE);	            // normal again
            normvideo();
         }                  //end if
     }                      //end for
display_workbuf();		    //show new position and file layout
};

//----------
void display_workbuf(void)  //SHOW RESULT
{
int t,i;
window(1,1,80,25);	        //set a window on screen
gotoxy(1,4);
printf("Sample of recovered text -");
window(1,5,80,24);	        //set a window on screen
clrscr();
textattr(WHITE + (BLACK << 4));
normvideo();

for( t=0; t < blocks; t++)  //up to max
     {
     for( i=0; i < LINE_MAX; i++)         //do a pass over 1 key length
          {
          test = workblock[t][i] ^ enc_string[i];
          if( i != enc_string_offset)
              {
              if(test == 0x0d || test == 0x0a)
              cprintf("\r\n");
              else
              cprintf("%c", test);
              if(wherey() > 18)
              return;
              }
          else
              {
              textattr(WHITE + BLINK + (BLACK << 4));	    // blink highlighted char
              if(test == 0x0d || test == 0x0a)   //line feed/carriage return ??
              cprintf("\r\n");
              else
              cprintf("%c", test);
              textattr(WHITE);	            // normal again
              normvideo();
              if(wherey() > 18)
              return;
              }
	      }
      }
};
//  -----------

void dump_to_disk()                                 //dump data to disk
{
fseek(in, 20, SEEK_SET);        //data starts a pos 20 in file
out = fopen( filename, "wb");                       //open output file
while( blocks)
    {
    memset(workblock, 0, sizeof(workblock));   //clear memory block
    blocks = fread((char *)workblock, LINE_MAX, BLOCK_MAX, in);
    write_buf( out, blocks);                        //do middle block of data
    }                            //now, do the last lot of blocks
fclose(out);                // close the file

};


void write_buf(FILE *stream, long blocks )
{
int t,i;
for( t=0; t < blocks; t++)         //up to max size of workblock
     {
     for( i=0; i < LINE_MAX; i++)
        fprintf(stream, "%c", workblock[t][i] ^ enc_string[i]);	//write data
     }
};                                  //end writebuf
//---------------
int is_ok()                         //get an answer, yes or no
{
char yes[] ="Yy";
char no[] ="Nn";
char answer;
int i;
answer = getch();
if(strchr(yes, answer) )       //  answered yes
    return(1);
if(strchr(no, answer ))        //  answered no
    return(2);
else
return(NULL);
};
//---------------
void Error( char *msg, char *str)
{
  if ( in)
    fclose(in);      // Close files if still open
  if ( out)
    fclose( out);
  if ( str)
    fprintf( stderr, msg, str);
  else
    fprintf( stderr, msg);
  exit(1);
}
//---------------
