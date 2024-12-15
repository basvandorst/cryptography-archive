/* ONEPAD.CPP one time pad -- exclusive or each byte of the first two files
   and output them to the third file name on the command line.
   
   Compile with Borland C++:
	bcc -mc -w -d -Ff=512 -f- -v- -G -O onepad.cpp

   Written by Mike Johnson.
   Copyright (C) 1990-1991 Mike Johnson.  All rights reserved.
   Registered users are granted a license to use this source code in 
   derivative works.  Register by sending $20.00 and the name of this
   program (ONEPAD) to:
   			Mike Johnson
			PO BOX 1151
			LONGMONT CO 80502-1151
			USA

*/

#include <stdio.h>	// file i/o
#include <io.h>	    	// open() read()
#include <stdlib.h> 	// strtoul()
#include <string.h> 	// strlen()
#include <dos.h>      	// open() constants O_RDONLY & O_WRONLY
#include <iostream.h>	// cout
#include <limits.h>	// LONG_MAX

#define RWBUFFERSIZE 32768U

long offset = 0;	// Offset into output file to start at.
FILE *f1 = 0;		// File pointer for input file.
FILE *f2 = 0;		// File pointer for key file.
FILE *f3 = 0;		// File pointer for output file.

void die(void)
  {
    if (f1 != NULL) fclose(f1);
    if (f2 != NULL) fclose(f2);
    if (f3 != NULL) fclose(f3);
    cout << "\n";
    exit(1);
  }



int main(int argc, char * argv[])
{
  int ok = 1;

  // If there aren't enough command line arguments, give help.

  if (argc < 3)
    {
      cout << "One time pad Copyright (C) 1990-1991 Mike Johnson.  All rights reserved.\n"
      	      "Version 1.10  15 September 1991  Support BBS:  303-678-9939\n"
	      "This program is believed to be reliable, but is distributed with no warranty.\n"
	      "This is an implementation of the classic one time pad encryption algorithm.\n"
	      "Permission is granted to distribute this software for free.  To register your\n"
	      "copy and get a disk with this program on it, send $20.00 to:\n\n"
	      "        Mike Johnson\n"
	      "        PO BOX 1151\n"
	      "        LONGMONT CO 80502-1151\n"
	      "        USA\n\n"
	      "Syntax:\n";
      if (strlen(argv[0]) < 1)
        cout << "ONEPAD";
      else
        cout << argv[0];
      cout << " infile keyfile outfile [offset]\n\n"
	      "Each byte of infile is exclusive-or'ed with each byte of keyfile (starting\n"
	      "at byte offset in keyfile), and the result written to outfile.  The default\n"
	      "offset is 0.  Offset may be decimal (10), octal (012), or hex (0xA).\n";
    }
  else
    {

      // If there is an offset, convert it to long.

      if (argc > 4)
        {
	  offset = strtol(argv[4], NULL, 0);
	  if (offset < 0)
	    {
	      cout << "Bad offset value:  " << argv[4] << 
	              "\nOffset must be a positive integer.\n";
	      exit(1);
	    }
	  cout << "Skipping " << offset << " bytes of key file.\n";
	}

      // Open files.

      f1 = fopen(argv[1], "rb");
      if (f1 == NULL)
        {
	  cout << "Unable to open input file " << argv[1] << "\n";
	  exit(1);
	}
      f2 = fopen(argv[2], "rb");
      if (f2 == NULL)
        {
	  cout << "Unable to open key file " << argv[2] << "\n";
	  die();
	}
      f3 = fopen(argv[3], "rb");
      if (f3 != NULL)
        {
	  cout << argv[3] 
	       << " exists!\nErase it or try another output file name.";
	  die();
	}
      f3 = fopen(argv[3], "wb");
      if (f1 == NULL)
        {
	  cout << "Unable to open output file " << argv[3] << "\n";
	  die();
	}

      // Go to the right place in the key file.

      if (offset != 0)
        {
	  if (fseek(f2, offset, SEEK_SET))
	    {
	      cout << "Unable to seek to offset " << offset << " in key file "
	           << argv[2] << "\n";
	      die();
	    }
	}

      // Allocate the disk buffers.

      char * b1;	// Buffer for input & output files.
      char * b2;	// Buffer for key file.

      b1 = new char[RWBUFFERSIZE];
      b2 = new char[RWBUFFERSIZE];
      if ((b1 == NULL) || (b2 == NULL))
        {
	  cout << "Not enough memory to continue.\n";
	  exit(1);
	}

      // This is where the work gets done.

      size_t toread = RWBUFFERSIZE;
      size_t towrite = RWBUFFERSIZE;
      size_t keyread;
      unsigned int u;
      long bytesdone = 0;

      while ((towrite > 0) && ok)
        {
	  cout << "\r" << bytesdone;
	  towrite = fread(b1, 1, toread, f1);
	  keyread = fread(b2, 1, towrite, f2);
	  if (keyread < towrite)
	    {
	      cout << "\nERROR:  Key length after offset of " << offset <<
	              "\nis too short to encrypt/decrypt entire file!\n";
	      towrite = keyread;
	      ok = 0;
	    }
	  for (u = 0; u < RWBUFFERSIZE; u++)
	    b1[u] ^= b2[u];
	  keyread = fwrite(b1, 1, towrite, f3);
	  bytesdone += keyread;
	  if (keyread != towrite)
	    {
	      cout << "\nERROR:  Unable to write entire output file " 
	           << argv[3] << "\n";
	      ok = 0;
	    }
	}

      // Close files & return to DOS.

      fclose(f1);
      fclose(f2);
      fclose(f3);
      cout << "\r" << bytesdone << " bytes written to " << argv[3] << "\n";
    }
  return(0);
}

