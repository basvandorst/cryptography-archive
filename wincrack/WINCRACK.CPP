/*

WINCRACK "cracks" files enciphered via PC Magazine's WINCRYPT utility.

  Usage:  WINCRACK <enciphered file> <deciphered file> [<most common character>]

    where

      <enciphered file> is the file descriptor for the enciphered file,
      <deciphered file> is the file descriptor for the output of this program,
      <most common character> is the decimal value of the most common character
        in the original (unenciphered) file.  The default is 32 (space).  Try
        0 (null) for binary files.

    Do *NOT* make <deciphered file> the same as <enciphered file>!

Example:  WINCRACK TEXT.ENC TEXT.DEC 32

*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream.h>

#define TRUE -1
#define FALSE 0

int main(int,char **);

int main(
  int  argc,
  char *argv[])
    {
      static   unsigned char buffer [512];
      register int           byte;
      register int           byte_num;
      static   int           fatal_error;
      static   unsigned long file_size;
      static   unsigned int  *freq [512];
      static   unsigned int  *freq_ptr;
      static   unsigned char key [512];
               FILE          *infile;
      static   unsigned char max_char;
      static   unsigned int  max_freq;
      static   unsigned char most_common;
      static   int           num_bytes;
               FILE          *outfile;

      fatal_error=FALSE;
      if ((argc == 3) || (argc == 4))
        {
          for (byte_num=0; byte_num < 512; byte_num++)
            {
              freq_ptr=new unsigned int [256];
              freq[byte_num]=freq_ptr;
              for (byte=0; byte < 256; byte++)
                *(freq_ptr++)=(unsigned int) 0;
            }
          if ((infile=fopen(argv[1],"rb")) == NULL)
            {
              fatal_error=TRUE;
              cout << "Fatal error:  cannot open " << argv[1] << " for input."
               << '\n';
            }
          else
            {
              file_size=(unsigned long) 0L;
              while ((num_bytes=fread(&buffer[0],1,512,infile)) > 0)
                {
                  for (byte_num=0; byte_num < num_bytes; byte_num++)
                    (freq[byte_num][buffer[byte_num]])++;
                  file_size+=((unsigned int) num_bytes);
                }
              if (file_size >= (unsigned long) 10240L)
                {
                  if (argc == 4)
                    most_common=(unsigned char) atoi(argv[3]);
                  else
                    most_common=(unsigned char) ' ';
                  for (byte_num=0; byte_num < 512; byte_num++)
                    {
                      max_char=(unsigned char) '\0';
                      max_freq=(unsigned int) 0;
                      for (byte=0; byte < 256; byte++)
                        if (freq[byte_num][byte] > max_freq)
                          {
                            max_freq=freq[byte_num][byte];
                            max_char=(unsigned char) byte;
                          }
                      key[byte_num]=max_char^most_common;
                    }
                  fseek(infile,0L,SEEK_SET);
                  if ((outfile=fopen(argv[2],"wb")) == NULL)
                    {
                      fatal_error=TRUE;
                      cout << "Fatal error:  cannot open " << argv[2]
                       << " for output." << '\n';
                    }
                  else
                    {
                      while (((num_bytes=fread(&buffer[0],1,512,infile)) > 0)
                      &&     (! fatal_error))
                        {
                          for (byte_num=0; byte_num < num_bytes; byte_num++)
                            buffer[byte_num]^=key[byte_num];
                          if (fwrite(&buffer[0],1,num_bytes,outfile)
                           != num_bytes)
                            {
                              fatal_error=TRUE;
                              cout << "Fatal error:  failure to write "
                               << argv[3] << '.' << '\n';
                            }
                        }
                      fclose(outfile);
                    }
                }
              else
                {
                  fatal_error=TRUE;
                  cout << "Fatal error: " << argv[2]
                   << " is less than 10240 bytes long." << '\n';
                }
              fclose(infile);
            }
          for (byte_num=255; byte_num >= 0; byte_num--)
            delete[] freq[byte_num];
        }
      else
        cout << '\n' << "WINCRACK " << '\"' << "cracks" << '\"'
         << " files enciphered via PC Magazine" << '\''
         << "s WINCRYPT utility." << '\n' << '\n'
         << "  Usage:  WINCRACK <enciphered file> <deciphered file> "
         << "[<most common character>]" << '\n' << '\n'
         << "    where" << '\n' << '\n'
         << "      <enciphered file> is the file descriptor for the "
         << "enciphered file," << '\n'
         << "      <deciphered file> is the file descriptor for the "
         << "output of this program," << '\n'
         << "      <most common character> is the decimal value of the "
         << "most common character" << '\n'
         << "        in the original (unenciphered) file.  "
         << "The default is 32 (space).  Try" << '\n' 
         << "0 (null) for binary files." << '\n' << '\n'
         << "    Do *NOT* make <deciphered file> the same as <enciphered file>!"
         << '\n' << '\n'
         << "Example:  WINCRACK TEXT.ENC TEXT.DEC" << '\n';
      return fatal_error;
    }

