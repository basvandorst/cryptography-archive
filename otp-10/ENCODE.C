#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "otp.h"

/* does encode of OTP file */
int encode(void)
{
extern struct general data;                   /* general info */
extern struct padstruc padheader;             /* header for codebook */
extern struct cipherstruc cipherheader;       /* header for OTP cipher file */
extern struct encryptedstruc encryptedheader; /* encrypted header */

FILE *padptr,      /* file pointer to padfile*/
     *sourceptr,    /* file pointer to plain text source */
     *outptr,       /* file pointer to output file */
     *compressedptr, /* file pointer to compressed intermediate file */
     *actualptr;     /* ptr to file actually used for encryption.
			 this is either the original sourceptr, or
			 it could be the compressedptr.*/

unsigned char c1,c2,c3;

extern struct padstruc padheader;             /* header for codebook */
extern struct cipherstruc cipherheader;       /* header for OTP encrypted file */
extern struct general data;                   /* general  (misc) variables */

long offset;            /* for offsets in a file */
int tempi;              /* temp integer */
unsigned char *chptr;   /* general purpose pointer to char */


 SetOutName("otp");

 /* the pad file default extension to ".pad" */
 default_extension(data.filetwo, "pad");

 strcpy(cipherheader.signature,CIPHER_SIGNATURE);
 encryptedheader.compression=data.compression;  /* update encrypted header */
 encryptedheader.crc32= 0xFFFFFFFFL;   /* Initalize CRC */

 /* grab the 32bit CRC for the input file and stick in in the header */
 getcrc32(&encryptedheader.crc32, data.fileone);

 if (data.verbose==TRUE)    /* chat up the user */
  printf("\nThe original file has a CRC32 of %lX \n",encryptedheader.crc32);

 /* exit if no input file */
 if ((sourceptr = fopen(data.fileone, "rb"))  == NULL)
   error_exit(NO_OPEN_IN);


   /* get the size of the original file */
   fseek(sourceptr,0,SEEK_END); /* move to end of file to find size */
   data.SourceFileSize=ftell(sourceptr);
   fseek(sourceptr,0,SEEK_SET);  /* back to the begining */

    if(data.verbose==TRUE)
     printf("The source file size is %lu bytes\n\n",data.SourceFileSize);

 if ((padptr = fopen(data.filetwo, "r+b")) == NULL)
   error_exit(NO_PAD_FILE);


 /* now that we opened the pad file, let's get the header */
 fread((char *)&padheader,sizeof(padheader),1,padptr);

 /* Does it have a valid signature ? */
 if (strcmp(padheader.signature, PAD_SIGNATURE)!=0)
  error_exit(INVALID_PAD);

  if (padheader.type==DECODE) error_exit (NOT_ENCODE_PAD);

  if(padheader.type==UNDEFINED){
    padheader.type=ENCODE;    /* mark as encode-only pad */
    if(data.verbose==TRUE) puts("** First use of pad: marking pad file as encode-only: **\n");
  }
 if ((outptr = fopen(data.outfile, "w+b")) == NULL)
  {
   fprintf(stderr, "Unable to open outfile");
   return 1;
  }

 actualptr=sourceptr;  /* until we know otherwise, assume encode of sourceptr */
		      /* actualprt is the file pointer we really use as "source"*/

 /* are we compressing the file first ?, if so, run compression routine */
    if(encryptedheader.compression==TRUE){
      if (data.verbose==TRUE)
      printf("Compressing File:...");
     GetTempFile (data.compfile, "lzh"); /* make up random name for temp file */
			    /*   void lzhDecode( FILE *in, FILE *out ); */

       /* try to open the compressed file with random name */
       if ((compressedptr = fopen(data.compfile, "w+b"))== NULL)
	error_exit(NO_OPEN_COMPRESSED);

       lzhEncode( sourceptr, compressedptr );

     actualptr=compressedptr;   /* we will be encrypting the compressed file */

    }  /* end of compression==TRUE */

 fseek(padptr,0,SEEK_END); /* move to end of file to find size */
 data.CodeBookSize=ftell(padptr); /* how big? */

 data.CodeBookRemain=data.CodeBookSize-padheader.bookmark; /* bytes left in codebook */

 fseek(actualptr,0,SEEK_END); /* move to end of file to find size */

 data.CompFileSize=ftell(actualptr);

 if (data.verbose==TRUE){
     if(encryptedheader.compression==TRUE)
      printf("\nThe compressed file size is %lu bytes\n",data.CompFileSize);
 }

 fseek( actualptr,0,SEEK_SET);  /* move back to begining */

 if (data.verbose==TRUE){
  printf("\nThe Pad size is %lu bytes\n",data.CodeBookSize);
  printf("Unused Pad data begins at offset %lu  (bookmark)\n",padheader.bookmark);
  printf("\n%lu unused bytes in Pad file before encryption \n",data.CodeBookRemain);
 }

  /* do we have enough unused space in the pad to encode the file? */

  if (encryptedheader.compression==TRUE){ /* we will encode compressed file */
    if(data.CompFileSize+sizeof(encryptedheader) > data.CodeBookRemain){

      /* Compressed intermediate file too big. Print messages and clean up */
      fclose(outptr); /* close output file */
      unlink(data.outfile); /* delete file (currently with 0 bytes) */
      if (data.verbose==TRUE) printf("Wiping intermediate compressed file:");
      if (wipefile(data.compfile) >0) error_exit (CANT_WIPE_FILE);

      error_exit(FILE_TOO_BIG);   /* not enough pad file left */
    }
  } else                         /* not using compressed intermediate file */
    if(data.SourceFileSize +sizeof(encryptedheader) > data.CodeBookRemain){
      fclose(outptr); /* close output file */
      unlink(data.outfile); /* delete file (currently with 0 bytes) */
      error_exit (FILE_TOO_BIG);
    }

 fseek(padptr,padheader.bookmark,SEEK_SET); /* move to bookmark in padfile */



 /* Note: the input file, and padfile specified on the command line
    may contain a full path. We need to extract the file name.
    We are following MSDOS naming conventions */

 extract_name(data.filetwo,cipherheader.padfilename);  /* padfile name */

 extract_name(data.fileone,encryptedheader.outfilename);  /* output file name */


 cipherheader.bookmark= padheader.bookmark;  /*record starting point in padfile */


 /* write out non-encrypted file header */
 fwrite(&cipherheader,sizeof(cipherheader),1,outptr);


 cipherheader.start=ftell(outptr);  /* remember where encrypted stuff goes */

 /* now we can write out the encrypted header */

 chptr= (unsigned char *) &encryptedheader;  /* set a character pointer to
				      point to start of header structure */

 tempi=sizeof (encryptedheader);     /* how big in bytes is encrypted header ? */

  while ((tempi--)>0){  /* go until we have processed every byte in encrypted header */
   fread(&c1, sizeof(unsigned char),1,padptr );   /* read a byte from pad */

   c3 = c1 ^ *(chptr++) ; /* xor with byte in encrypted header  */

   fwrite(&c3,sizeof(unsigned char),1,outptr);        /* write out xor'd byte */

  }

 /* Now that we have written out the encrypted header, we take bytes
    from the source/compressed file, XOR them with pad file, and write to output */

 for(;;){         /* this loop xor's the source file with the pad file */

  if(fread(&c2, sizeof(unsigned char),1, actualptr ) <1) break;
  if(fread(&c1, sizeof(unsigned char),1,padptr )<1) break;  /* break out if no more to read */

  c3 = c1 ^ c2 ;   /* XOR here, remember we use pointers */

  fwrite(&c3,sizeof(unsigned char),1,outptr);
 }

 /* we now store SessionBegin, and SessionEnd to mark the begining
    and end of the pad used for this session.  We can then overwrite that
    section if necessary */

 data.SessionBegin=padheader.bookmark;   /* where we began in pad */
 data.SessionEnd=ftell(padptr);

 padheader.bookmark=data.SessionEnd;  /* store new bookmark in padheader */

 fseek(padptr,0,SEEK_SET);  /* move to begining Pad */

 fwrite(&padheader,sizeof(padheader),1,padptr);  /* write out pad header */

 fseek(outptr,0,SEEK_SET);   /* move to begining of cipher file */
 fwrite(&cipherheader,sizeof(cipherheader),1,outptr); /* write updated header */

 if (data.verbose==TRUE){
  printf("(Note: Unencrypted header = %i bytes; encrypted header =%i bytes)\n",sizeof(cipherheader),sizeof(encryptedheader));
  printf("%lu Bytes of Pad consumed by encryption (includes encrypted header) \n",data.SessionEnd-data.SessionBegin);
  printf("%lu bytes remain unused in Pad after encryption\n\n",data.CodeBookSize-padheader.bookmark);
 }

 /* now wipe out part of pad used in encryption  */

   if(data.verbose==TRUE) puts("Overwriting Pad:... ");

     WipeArea(padptr, data.SessionBegin, data.SessionEnd); /* zap it */

  fcloseall(); /* close open files */

 /* delete compressed file */
 if (data.compression==TRUE){
  if (data.verbose==TRUE) printf("Wiping intermediate compressed file:");
  if (wipefile(data.compfile) >0) error_exit (CANT_WIPE_FILE);

  return(0);
 }
 if(data.verbose==TRUE)  puts ("\n\nencryption complete!\n");
}
