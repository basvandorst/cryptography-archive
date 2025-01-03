/*

Released to public domain!


It has been recently discovered by paul elliott
that the PGP26 (eastern University) legal kludge can be disabled by invoking
pgp with the following parameters:

pgp +CERT_DEPTH=0 +LEGAL_KLUDGE=OFF +CERT_DEPTH=real_desired_value others

This program invokes pgp with the above parameters. "real_desired_value"
is taken from the CONFIG.TXT file. This will cause the legal kludge
that makes PGP from an Eastern University incompatible with earlier
versions of PGP be turned off. This program does not address
the incompatible signature format problem.


The program to be invoked may be controled by the environment
variable "PGPEU". EU stands for Eastern university.
this variable may specify the file name or a complete path.
If this varriable is undefined, the program indicated
by the hard coded string "PGPEXE" will be invoked
this string may be defined with a -DPGPEXE= flag
at compile time.

The program searches your config.txt file for the default
value of CERT_DEPTH. It will run slightly faster if this
variable is at the top of the file.

deveolped for borland c++ compilers for msdos and os2
to compile

bcc pgpknolg.cpp

*/


// necessary include files.
#include <stdlib.h>
#include <string.h>
#include <dir.h>
#include <iostream.h>
#include <fstream.h>
#include <strstrea.h>
#include <iomanip.h>
#include <process.h>

// define executable path to invoke pgp.
#ifndef PGPEXE
#define PGPEXE "pgp"
#endif
#if defined(__OS2__) || defined(__MSDOS__)
#define SEP '\\'
#define SEPSTR "\\"
#else
#define SEP '/'
#define SEPSTR "/"
#endif
int main(int argc,char  *argv[])
{

 // path to invoke pgp.
 char pgpexe[MAXPATH];

 // if environment variable PGPEU is defined use it
 // as the program to invoke as pgp!
 char * pgpeust=getenv("PGPEU");
 if (pgpeust) strcpy(pgpexe,pgpeust); else *pgpexe=0;

 // If no such environment variable use hard coded PGPEXE macro!
 if (*pgpexe == 0 ) strcpy(pgpexe,PGPEXE);
 char path[MAXPATH];


 // get the path where the config.txt file is supposed to be
 strcpy(path,getenv("PGPPATH") );

 // and get its length
 int len=strlen(path);

#if defined(__OS2__) || defined(__MSDOS__)
 // convert all '/' to backslashes for dos os/2
 char *w;
 for(w=path;*w;w++) if(*w=='/') *w=SEP;
#endif
 
 // if there some chars and last one is not \ then add one.
 if (len)
 {
   if ( path[len-1] != SEP ) strcat(path,SEPSTR);
 };

 // add the filespec.
 strcat(path,"CONFIG.TXT");

 // default value if can not get from config.txt file
 // this is the value in the pgp26 executable
 int cert_depth=4;

 // parse the config file for the value of CERT_DEPTH
 {
   // open the file
   ifstream config(path);

   if ( config.good() )
   {
   	
   // read till eof
    while( !config.eof() )
    {
     char buf[512];

     // read a line
     config.getline(buf,sizeof(buf));

     // if not commented.
     if ( *buf != '#' )
     {
       istrstream line(buf);
       char field[80],equ[80];
       int depth;

       // parse line to first field equal char and value
       line >> field >> equ >>depth;
       if ( line)
       {
       	 // upcase the field
	 strupr(field);

	 // if we have a "CERT_DEPTH=val" LINE
	 if ( (strcmp(field,"CERT_DEPTH") == 0) &&
	      (strcmp(equ  ,"=") == 0 ) )
         {
           // save stored depth
           cert_depth =depth;

           // abort search of rest of file
           // this program will go faster if CERT_DEPTH is
           // at top of file!
           break;
         }
       }
     }
    }
   }
 }

 // create a parameter string fo the form "CERT_DEPTH=val"
 // where val was found in the config.txt file!
 char reset_par[20];
 {
    ostrstream reset_file(reset_par,sizeof(reset_par));
    reset_file << "+CERT_DEPTH=" << cert_depth << ends;
 }
 typedef char * string;

 // list of arguements!
 string * list= new string[argc +3 +5];

 // leading arguemnts
 int out_idx = 0;
 // name of program
 list[out_idx++] = "PGP.EXE";

 // we do this to set the variable "value" in pgp 2.6 to =0
 // the value of CERT_DEPTH is not really needed to be 0
 list[out_idx++] = "+CERT_DEPTH=0";

 // this will set pgp varriable "legal_kludge" =value=0
 // works because value is zero because of above.
 list[out_idx++] = "+LEGAL_KLUDGE=OFF";

 // set value of CERT_DEPTH back to its proper value.
 list[out_idx++] = reset_par;

 // add all the rest of the parameters from the command line!
 for( argc--, argv++; argc; argc--,argv++,out_idx++)
   list[out_idx] = *argv;
 // termintate the line!
 list[out_idx]=NULL;

 // replace the current program with an execvp call
 // never returns from execvp
 return execvp(pgpexe,list);
}

