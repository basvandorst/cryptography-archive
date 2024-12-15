//  word for windows decryptor

//  program code for the Word for Windows Password Recovery tool, by
//  Lyal Collins, 1995
//  Recovers text from Word 2.x and Word 6.0 files that
//  have been password protected.
//  Word 2.0 files use a simple technique easily reversed, while
//  Word 6.0 files need a litle bit of guessing.


#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <ctype.h>
#include <mem.h>

#include "key.cpp"
#include "keylist.cpp"
#include "wp.h"

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
        strcat(&Alphabet, alpha_lower);

    if( stricmp(argv[i], set_digits) == 0)
        strcat(&Alphabet, alpha_digits);

    if( stricmp(argv[i], set_upper) == 0)
        strcat(&Alphabet, alpha_upper);
}

enc_string_offset = 20;      //is above 16, no blinking is shown, on first try
in = NULL;                   //clear some values
out = NULL;

//  Open the files, give an error if problems arise.
if ( !(in = fopen( argv[1], "rb")) )
    Error( "Can\'t open %s.\n", argv[1]);

strcpy(filename, argv[2]);               //keep a copy of the out filenamr

if ( (out = fopen( argv[2], "rb")) ) //test if output file exists
    Error( "File already exists. Us a different output filename.\n", argv[2]);

clrscr();
normvideo();
printf("Text recovery for Word for Windows password protected files.\n\n");

memset(workblock, 0, sizeof(workblock));   //clear memory block
offset = fread(&headerblock, sizeof(headerblock), 1, in); //Read the input file

i = test_hdr();                           //test file header
if( i == 2)                              //its word2, do it
    do_word2();
else
if( i == 6)                              //else its word6
    do_word6();
else
{
  printf("Input file is not a valid WORD for Windows file !!!\n");
  exit(-2);
}
fcloseall();
clrscr();
printf("The recovered text is in the file %s\n", filename);
exit(0);
}           // End of main
//-------------

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
void find_key()
{
memset(maxcount_string, 0, sizeof(maxcount_string));

for( i=0; i < 256; i++)                        //for each count, up to 256
{
     memset(count_string, 0, sizeof(count_string)); //this shows result on current i
     for( l=0; l <= blocks; l++)               //up to max
         {
         for( j=0; j < LINE_MAX; j++)         //do a pass over 1 key length
              {
              test = workblock[l][j] ^ i;     //do xor with i
              if( test !=0)                   //ensure 0 don't bias result
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
         }                  // if
     }                      //for k loop
}                           // for i loop
};                          //end find_key
//-------------
int test_hdr()              //test the header for word2, word 6 format
{
if( headerblock.sig == WORD6)
return(6);                          //word 6 header
if( headerblock.sig == WORD2)       //word 2 file
    {
    if( headerblock.password != 0)  //password ???
        return(2);                  //word 2 header and password
    else
        {
            printf("Not password protected or not a Word for Windows 2 file !!\n");
            exit(-1);
        };
    };
return(0);
};
//-------------------
void do_word2(void)     //do as a word for win 2 document
{
starttext = headerblock.start_text;         //get pointer
endtext = headerblock.end_text;             //now have start,end of text
blocks = (endtext-starttext)/LINE_MAX;      //number of whole 16 byte chunks
if(blocks > BLOCK_MAX)
blocks = BLOCK_MAX;

fseek(in, 0x40, SEEK_SET);                      //seek to appropriate pos
offset = fread((char *)enc_string, 16, 1, in);  //Read the XOR'ed null data
                                                // which forms key.
fseek(in, starttext, SEEK_SET);             //seek to appropriate pos
offset = fread((char *)workblock, LINE_MAX, blocks, in); //Read the input file
do_after_key();                 //do the tidy up, show result etc
window(1,1,80,25);
gotoxy(1,25);                   //bottom left
clreol();
printf("A sample of the recovered text. If correct, (Y) or (N) to alter");

if( is_ok() == 2)                //is result ok ??
    {
    enc_string_offset = 0;
    key_adjust();                //no, then adjust it, else
    }
window(1,1,80,25);
gotoxy(1,25);                   //bottom left
clreol();
printf("Do you want to save the recovered text to a file ?? Y,n");
if(is_ok() == 1)                //ask if the want to save recovered text
dump_to_disk();
};
//--------------
void do_word6(void)             //do as a word for win 6 document
{
offset = 16;                    //find word2 embeded header the hard way
fseek(in, offset, SEEK_SET);
do {
offset = fread(&test_sig, 2, 1, in);    //Read the input file, word by word
} while (test_sig != WORD6_EMBED);      //till singature found

offset = ftell(in);                     //get pointer
offset--;                               //dec 2 for last int read
offset--;
fseek(in, offset, SEEK_SET);            //seek to appropriate pos
fread(&headerblock, sizeof(headerblock), 1, in); //Read the input file

if( headerblock.password == 0)           //password ???
    {
    printf("Not password protected or not a Word for Windows 6 file !!\n");
    exit(-1);
    };

starttext = headerblock.start_text;
endtext = headerblock.end_text;            //i now have start, end of text

blocks = (endtext-starttext)/16;           //number of whole 16 byte chunks
if( blocks > BLOCK_MAX)
    blocks = BLOCK_MAX;                    //set max size for now

starttext = starttext + offset;
endtext = endtext + offset;
fseek(in, starttext, SEEK_SET);            //seek to appropriate pos
offset = fread((char *)workblock, LINE_MAX, blocks, in); //Read the input file

find_key();                                 //derive key block
display_workbuf();

window(1,1,80,25);
gotoxy(1,25);                   //bottom left
clreol();
printf("A sample of the recovered text. If correct, (Y) or (N) to alter");

if(is_ok() == 2)                //is result ok ??
{
enc_string_offset = 0;
key_adjust();                   //no, then adjust it, else
}
window(1,1,80,25);
gotoxy(1,25);                   //bottom left
clreol();
printf("Do you want to save the recovered text to a file ?? Y,n");
if(is_ok() == 1)                //ask if they want to save recovered text
dump_to_disk();
};

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

void dispatch(void)		    //does hard work during edit section
{
	ch=getKey();
	while (ch != KEY_F10 )
				            //f10 or esc are exit keys
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
long blocks_to_get;                                 // how many left to handle
blocks_to_get = (endtext - starttext)/LINE_MAX;     //total number of blocks
out = fopen( filename, "wb");                       //open output file
write_buf( out, blocks);                            //do first block of data
blocks_to_get -= offset;                            //adjust amount left to do
while( blocks_to_get > blocks)
    {
    offset = fread((char *)workblock, LINE_MAX, blocks, in); //Read the input file
    blocks_to_get -= offset;                        //adjust amount left to do
    write_buf( out, blocks);                        //do middle block of data
    }
                            //now, do the last lot of blocks
offset = fread((char *)workblock, LINE_MAX, blocks_to_get, in); //Read the input file
write_buf( out, blocks_to_get);                     //do last block of data
fclose(out);                // close the file
};


void write_buf(FILE *stream, long blocks )
{
int t,i;
for( t=0; t <= blocks; t++)         //up to max size of workblock
     {
     for( i=0; i < LINE_MAX; i++)
        fprintf(stream, "%c", workblock[t][i] ^ enc_string[i]);	//write data
     }
};                                  //end writebuf

int is_ok()                         //get an answer, yes or no
{
char yes[] ="Yy";
char no[] ="Nn";
char answer;
int i;
answer = getch();
if(strchr(&yes, answer) )       //  answered yes
    return(1);
if(strchr(&no, answer ))        //  answered no
    return(2);
else
return(NULL);
};