/* Copyright (c) James L. Pinson 1993,1994 */
/* I plan someday to this to C++, and have put many of the variables in
   external structures to ease the future transistion.  (you may rename
   these files to *.cpp as they will currently compile under Borland C++
   version 3.0).

   Parts of the code are from various public domain sources, see  credit.doc
   for more details.  Some string functions are from my book "Designing Screen
   Interfaces in C", and have different style comments than other functions.
   I will smooth out the code in a future release.
*/

/* link with:
   opt.c   (this file)
   encode.c
   decode.c
   getopt.c
   misc.c
   lzh.c
   crctab.c
*/



#include <dos.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "getopt.h"  /* for the enhanced getopt function */
#include "crc.h"     /* for the CRC32 calculation */

/* for file i/o */
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <conio.h>

#include "otp.h"



/* external data structures */
struct general data;      /* general variables needed by different modules */

struct cipherstruc cipherheader;
struct padstruc padheader;
struct encryptedstruc encryptedheader;


 /* help messages */

 char *help_messages[]={
   "\nOTP-PC (Ver. 1.0)\n",
   "Copyright (c) 1994, James L. Pinson\n\n",
   "Usage: otp [-qncedwx] filename [filename] [-o outputfile]\n\n",
   "e=encode file,  d=decode file,         c=create padfile,\n",
   "x=xor,          w=wipe file,           h=help,\n",
   "q=quiet,        n=no compression\n\n",

   "Create the padfile \"2jill.pad\" from the random file \"number.rnd\"\n",
   "  otp -c number.rnd -o 2jill.pad\n\n",

   "Encrypt \"my.doc\" using the padfile \"2jill.pad\"\n",
   "  otp -e my.doc 2jill.pad    (-e option not mandatory)\n\n",

   "Decode \"my.otp\"\n",
   "  otp -d my.otp             (-d option not mandatory)\n\n",

   "Wipe the file \"my.doc\"\n",
   "  opt -w my.doc\n\n"
 };

  /* these error messages match the error messages numbers
     defined in otp.h */

  char *error_messages[]={
   "",  /* I want error messages to begin with 1, not 0 */
   "Unable to open source file:",
   "Unable to find Pad file:",
   "Pad file has invalid header:",
   "No files Specified:",
   "You did not specify an action:",
   "Incorrect options",
   "Invalid Cipher file:",
   "The file did not decode properly (bad CRC32):",
   "Unsufficient space in Pad to encrypt file",
   "Unable to wipe file",
   "Unable to create Pad file",
   "Illegal option given ",
   "The Pad file has been previously used for decryption!\nCannot use for encryption.\n",
   "The Pad file has been previously used for encryption!\nCannot use for decryption.\n",
   "You need to Specify two filenames for the action requested:",
   "Unable to open output file:",
   "Unable to open compressed file:"
  };



int main(int argc,char *argv[])
{
extern struct general data;
int i;
int nonopt;  /* the number of non-option arguments on command line */
int ReturnCode;

/* if no arguments given */
 if(argc==1){
  PrintArray(0,help_messages);  /* print out help messages, begining with 0 */
  exit(1);
 }



/* now we initalize some "default" settings in the external data structure */

data.action=NONE;             /* no assumptions made about action */
data.verbose=TRUE;            /* assume verbose mode */
data.compression=TRUE;        /* assume compression */
encryptedheader.compression=data.compression;     /* use in header  */
strcpy (data.outfile,"");            /* start with empty output file name */

nonopt=ProcessOptions(argc,argv);  /* process the command line
				      options. "nonopt" is the number
				      of nonoption arguments
				      (those without a '-') */


/* if help requested */
 if(data.action==HELP){
  PrintArray(0,help_messages),  /* print out help messages, begining with 0 */
  exit(0);
 }


 /* If the action to take (data.action) was not set from the command line,
   (ie: still set to NONE) then we will try to guess the action based on the
   number of nonopt arguments given.
   For exampe the command "otp myfile.otp", has one nonopt argument
   (the file name).  In that case we will assume a decode was requested.
   If there are two nonopt arguments "otp myfile.txt myfile.pad", then
   an encode is assumed.  */

 if (nonopt==1 && data.action==NONE) data.action=DECODE;
 if (nonopt>1 && data.action==NONE) data.action=ENCODE;


 /* now let's adjust our default values according to the arguments */
 puts("\n");
 if (data.verbose==TRUE)puts("OTP-PC 1.0: Verbose mode");


 /* the action is decode */
 if (data.action==DECODE)decode();

 /* the action is create pad */
 if(data.action==CREATE_PAD){

    ReturnCode= CreatePad();
    if (ReturnCode>0 ) error_exit( NO_CREATE_PAD);
    exit(ReturnCode);
 }

 if (data.action==XOR){
  if (nonopt < 2) error_exit(NEED_MORE_NAMES); /* need at least two filenames */
  exit(SimpleXOR());  /* call xor and leave */
  /* otherwise, exit with ReturnCode */
  exit(ReturnCode);
 }

 /* action is to wipe a file */
 if (data.action==WIPE){
  if (nonopt < 1) error_exit(NEED_MORE_NAMES); /* need at least one filename */

  printf ("Overwrite the file %s y/n :",data.fileone);
  if(yes_no()=='N') exit(0);     /* wait until a y or n is typed */

  exit(wipefile(data.fileone));
 }




 if (data.action==ENCODE) encode ();  /* encode requested */

 return (0);
}




/**********************   L_COPY.C   ***************************/


/*****************************************************************

 Usage: void copy (char *from,char *to,int first,int length);

  char *from  = string to copy from.
  char *to    = string to copy to.
  int first   = position within string to start copying.
  int length  = number of character to copy.

  Copies a section of text, begining at position "first" in
  string "from" and copies "length" number to string "to".
  (zero based counting, begins at zero not one)

  Example: copy (&to "test",1,2);
	   results: to = "es"

*****************************************************************/


void copy (char *from,char *to,int first,int length)
{
int i;
 if ( (first <0) ) return;  /*  invalid number */

 /* if attempt made to copy beyond end of string then adjust*/
 if((first+length+1 ) > strlen(from))length=strlen(from)-first ;

  for(i=0;i<length;i++)
   to[i]= from[(first)+i];
  to[i]='\0';
}



/*****************************************************************

 Usage: void GetTempFile (char *path, char *ext)

  char *path  = Directory portion of filename.
  char *ext   = Extension to add to name.

  Creates a unique filename using the path and extension provided.

  The filename is made of random letters and numbers.  A check is made
  to insure the name does not already exist.  The function keeps trying
  until it makes a name.

  The path should already exist.



  Example:  (assume myfile = "c:\\temp")
   GetTempFile (myfile, "tmp" );
	   results: result= "c:\\temp\\SE3KK83W.tmp"

*****************************************************************/


void GetTempFile (char *path, char *ext)
{
char string[9];
int done=FALSE;
int i;
char fullpath[80];
randomize();      /* seed ramdom number generator PRNG's ok for this purpose :-) */
while (!done){

 for(i=0;i<8;i++){
  string[i]=   (char) random (36);

    if((unsigned int) string[i]<26) string[i]+= 65;  /* shift to character */
      else
	  string[i]+=22 ;   /* shift to number */

 }
 string[i]='\x0'; /* terminate string */
 strcpy(fullpath,path);   /* start with orig path */
 strcat(fullpath,string); /* add name */
 strcat(fullpath,".");    /* add . */
 strcat(fullpath,ext);    /* add extension */

  /* string construction finished, now check to see if file exists */

 if(!is_file(fullpath)){
  strcpy(path,fullpath);
  done=TRUE;
 }
 } /*  end while */

}


/* Print error message using strings stored in error_messages
   and defined values in error_code (otp.h) */



void error_exit( int error_code)  /* exit here if error  */
{
fputs("\n\nERROR: OTP-PC Beta test: ",stderr);
fputs(error_messages[error_code],stderr);  /* write our error message */
fputs("\n",stderr);
fcloseall();                               /* close any open files */
exit(error_code);                          /* tell parent about problem */


}

/* this function tries to open a file to see if it exists
   Returns a TRUE (1) if the file exists, a FALSE  (0) if the
   file does not exist */

int is_file(char *filename)
{
int handle;
int ret_code;

handle= open(filename,O_RDWR|O_BINARY);

ret_code=close(handle);
if (handle==-1)return (FALSE);
else return(TRUE);

}


/* This function takes a random binary file as input and writes out a
   pad file. If no output file is specified, the pad file has the same
   name as the source but with a .pad extension.   The newly created
   pad file has a header, so it is slightly larger that the random file.
   This function does NOT check to see if the file is truly random. */

int CreatePad(void)
{

FILE *PadFilePTR;
FILE *InFilePTR;
int retcode;
char buffer[2000];
int amountread;
extern struct padstruc padheader;

  if(strlen(data.outfile)==0){   /* if no output name has been specified */
   strcpy (data.outfile,data.fileone);  /* start with the input file name */
   force_extension(data.outfile,"pad");
  }
  else
   default_extension(data.outfile,"pad"); /* if a out filename is given, but
					     no extension was specified, then
					     set extension to "pad" */

   default_extension(data.fileone,"rnd");  /* default extension is "rnd" */

 strcpy(padheader.signature,PAD_SIGNATURE);

 padheader.bookmark=(long) sizeof (padheader);
 padheader.type=0;  /* undefined type of file, later becomes encode or
		      decode type */

 if(data.verbose==TRUE)printf("Creating Pad File \"%s\"\nUsing random file \"%s\":",data.outfile,data.fileone);

 /* open the input file */
 if (( InFilePTR= fopen(data.fileone, "rb")) == NULL)
   error_exit(NO_OPEN_IN);


 /* open the output file */
 if (( PadFilePTR= fopen(data.outfile, "wb")) == NULL)
  error_exit(NO_OPEN_OUT);

  fwrite(&padheader, sizeof(padheader), 1, PadFilePTR); /* write struct s to file */


  for(;;){
   amountread= fread(buffer,sizeof(char),sizeof(buffer),InFilePTR );
   if(amountread<=0)break;



   fwrite(buffer,sizeof(char),amountread,PadFilePTR);
  }

 /* close open files */
   if(data.verbose==TRUE)printf("\n\n");
   puts("Creation of padfile complete.\n\n");

 fcloseall();
 return (0);
}


/* searches the specified string looking for the character specified
   returns the position if found, else returns a -1  */

int LocateChar(char *string, char ch)
{
int i=0;
  while(string[i]!= '\0')               /* while not end of string */
   if(string[i++]==ch)
    return (i-1);                         /* return location */

    return (-1);                        /* else return -1 */
}


/* This function creates an output name using info provided on
   the command line */

void SetOutName(char *ext)
{
extern struct general data;
extern struct padstruc padheader;

  if(strlen(data.outfile)==0){   /* if no output name has been specified */

   /* extract the name portion of first file on command line (no path) */
   extract_name(data.fileone,data.outfile);
   force_extension(data.outfile,ext);
  }
  else
   default_extension(data.outfile,ext); /* if a out filename is given, but
					     no extension was specified, then
					     set extension to "pad" */

   default_extension(data.fileone,ext);  /* default extension is "rnd" */


}


int ProcessOptions(int argc,char *argv[])
{
int option;
int nonopt=0;
 /* now we scan the list of arguments, and figure out what we are
    suppose to do */

 /*
    Note: We are using an enhanced version of the getopt(3) UNIX style
    argument scanner.  See the module "getopt" for more details.

    Options are command line arguments that begin with the switch
    character, '-', other arguments that don't begin with the switch
    character are refered to as nonoption arguments.  An example of a
    nonoption argument would be the input file name.
 */

 while (((option=getopt (argc, argv,"hqncedwxo:"))!= NONOPT) || (optarg !=NULL)){

  switch (option){
     case 'q': data.verbose=FALSE;break;
     case 'c': data.action= CREATE_PAD;break;  /* create pad file */
     case 'e': data.action=ENCODE;break;     /* encode */
     case 'd': data.action=DECODE;break;     /* decode */
     case 'o': strcpy(data.outfile,optarg); break; /* an output file specified */
     case 'w': data.action=WIPE;break;
     case 'x': data.action=XOR;break;
     case 'n': data.compression=NONE;break;
     case 'h': data.action=HELP;break;
     case '?': error_exit (BAD_OPTION);

   case NONOPT:         /* we have an non option argument (a filename?) */
      nonopt+=1;        /* increment the count of non-option arguments */
      if(nonopt==1)        /* is this the first non-option argument */
	strcpy (data.fileone,optarg);   /* it must be the infile name */
      if (nonopt==2)                   /* is this the second non-option argument */
	strcpy(data.filetwo,optarg);   /* it must be the outfile name */
      break;
   default:break;
  }

 } /* end of while loop */
return (nonopt);
}


/* this function gets the size of a file */
long unsigned GetFileSize( char *filename)
{
FILE *fileptr;
long filesize;

 if ((fileptr = fopen(filename, "rb")) == NULL) return -1;

 fseek(fileptr,0,SEEK_END); /* move to end of file to find size */
 filesize=ftell(fileptr);
 fclose(fileptr);
 return (filesize);
}



/* this function wipes a file, then deletes it */

int wipefile(char *filename)
{
extern struct general data;
FILE *InFilePTR;
long filelength;
int i;
unsigned char ch;

 if ((InFilePTR = fopen(filename, "r+")) == NULL) return (CANT_WIPE_FILE);
 fseek(InFilePTR,0L,SEEK_END);    /* go to end of file */
 filelength=ftell(InFilePTR);  /* get length */
 WipeArea(InFilePTR,0l,filelength);  /* wipe the area */


 fclose(InFilePTR);      /* get rid of file */
 unlink (filename);
 return (0);  /* no errors (Ok.. there *might* be errors, but it's late
		 and I'm in a hurry to finish this thing)*/

}


/* This function wipes an area within a file. The file must already
   be open, in a mode that allows writing/update.    The begin and
   end values specify the range to wipe.

This function is used by the wipefile function, and for wiping pads.

(A recomendation on Cypherpunks discussion list suggest not wiping
a file with the same byte since some disk compression such as Stacker
will compress the repeating characters, and the resultant file may not
extend as far at the original, and not overwrite the whole file.
I therefore use random numbers for  the wipe character.  A check of
the compression ratio of files so wiped (and undeleted) indicates
zero compression */


void WipeArea(FILE *fileptr, long begin, long end)
{
char ch;
randomize();    /* randomize the number generator */
ch=random(256);

 fseek(fileptr,begin,SEEK_SET);  /* move to begin position */
  while(begin < end ){     /* while we still have stuff to overwrite */
   fwrite(&ch,1,sizeof(char),fileptr);  /* write character */
   ch=random(256);  /* get new character */
   begin++;
 }


}

/* This function does a simple XOR with the files specified
   XORs data.fileone with data.filetwo.  Produces output
   data.outfile.  Does not over write either source files.
   The out file is the size of the smallest of the two input
   files.
*/


int SimpleXOR(void)
{
extern struct general data;
FILE *fileoneptr,      /* file pointer to padfile*/
     *filetwoptr,    /* file pointer to plain text source */
     *outfileptr;       /* file pointer to output file */

char c1,c2,c3;


char *chptr;   /* general purpose pointer to char */

 SetOutName( "xor");  /* set output name with correct extension */

 /* exit if no input file */
 if ((fileoneptr = fopen(data.fileone, "rb"))  == NULL)
   error_exit(NO_OPEN_IN);

 if ((filetwoptr = fopen(data.filetwo, "rb")) == NULL)
   error_exit(NO_OPEN_IN);

 if ((outfileptr = fopen(data.outfile, "wb")) == NULL)
  error_exit(NO_OPEN_OUT);

  if (data.verbose==TRUE){
   printf("\nXOR of %s with %s\n",data.fileone,data.filetwo);
   printf("Output to %s\n",data.outfile);
  }

  for(;;){         /* this loop xor's the source file with the pad file */
   if(fread(&c1, sizeof(char),1, fileoneptr ) <1) break;
   if(fread(&c2, sizeof(char),1,filetwoptr )<1) break;

     c3= c1 ^ c2;

   fwrite(&c3,sizeof(char),1,outfileptr);
  }



 /* close open files */
 fcloseall();
 return(0);
}


/* this function will remove any existing extension from *filename,
   and replace it with the one specified by *extension */

void force_extension(char *filename, char *extension){
int i;

 i= find_right_char( filename, '.');   /* is there a  '.'  (an extension) */

 if (i>0) filename[i]='\0';  /* remove any existing extension */

 strcat(filename,".");  /* add the '.' */
 strcat (filename,extension);  /* add specified extension */
}


/* this function adds a default extension if none specified */
void default_extension (char *filename, char *extension){

 if(find_right_char(filename,'.')<0){  /* if there is no '.' */
  strcat(filename,".");
  strcat(filename,extension);   /* add extension */
 }
}
