#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "otp.h"


/* does decode of OTP file */

int decode()
{

extern struct padstruc padheader;   /* header for codebook */
extern struct cipherstruc cipherheader;       /* header for OTP encrypted file */
extern struct general data;                   /* general  (misc) variables */
struct encryptedstruc encryptedheader;

int retcode;
FILE *padfile,
     *cipherfile,
     *outfile,
     *finalfile;

int tempi;     /* temp integer */
char *chptr;   /* general purpose pointer to char */
unsigned char c1,c2,c3;
char *actualfile; /* actual file used */
long offset;

if (data.fileone[0]=='\0') error_exit(NO_FILES_GIVEN);
if(data.verbose==TRUE) printf("\nDecoding %s\n",data.fileone);

 default_extension(data.fileone, "otp");  /* data.fileone contains the name
					     of the file we will decode.  Let's
					     give it the .otp extension if no
					     extension was given.  */


 if (data.verbose==TRUE) puts ("Begining decode...\n");

 if ((cipherfile = fopen(data.fileone, "rb")) == NULL) {
   fprintf(stderr, "Unable to open cipherfile");
   return 1;
 }



   /* read in the header */
   fread((char *)&cipherheader,sizeof(cipherheader),1,cipherfile);

   /* does it have a valid signature ?*/
   if (strcmp(cipherheader.signature, CIPHER_SIGNATURE)!=0)
    error_exit(INVALID_CIPHER);

 fseek(cipherfile,cipherheader.start,SEEK_SET);  /* move to starting position */


  /* open the pad file */

  if ((padfile = fopen(cipherheader.padfilename, "r+b")) == NULL){

   fprintf(stderr, "Unable to open the padfile");
   return 1;
  }

  /* get the pad header */
  fread((char *)&padheader,sizeof(padheader),1,padfile);
    if (strcmp(padheader.signature, PAD_SIGNATURE)!=0)  /* a valid file ?*/
     error_exit(INVALID_PAD);

/* make sure the pad is not a encode-only type */
  if (strcmp(cipherheader.signature, CIPHER_SIGNATURE)!=0)
    error_exit(INVALID_PAD);

/* now we must move the file pointer in the padfile to the position
 specified in the cipherfile header. */

fseek(padfile,cipherheader.bookmark,SEEK_SET);


data.SessionBegin=cipherheader.bookmark;   /* store begin of session info,
					      so we can use it for overwrite */

chptr= (char *) &encryptedheader;  /* set a character pointer to
				      point to start of header structure */


 /* now we must read in the encrypted header, XORing as we go */
 tempi=sizeof (encryptedheader);     /* how big in bytes is encrypted header ? */

  while ((tempi--) > 0){
   fread(&c1, sizeof(unsigned char),1,cipherfile );   /* read a byte from cipher */
   fread(&c2,sizeof(unsigned char),1,padfile);        /* read a byte from pad */

   *(chptr++) = c1 ^ c2 ; /* xor with byte in encrypted header */
  }

 if (padheader.type==ENCODE) error_exit (NOT_DECODE_PAD);

  if(padheader.type==UNDEFINED){
   offset=ftell(padfile);   /* save our current location */
   padheader.type=DECODE;    /* mark as encode-only pad */
   if(data.verbose==TRUE) puts("** First use of pad: marking pad file as decode-only: **\n");
   fseek(padfile,0,SEEK_SET);  /* back to the begining */
   fwrite(&padheader,sizeof(padheader),1,padfile);
   fseek(padfile,offset,SEEK_SET);  /* put pointer back */
  }


/* if the cipher file is compressed then we decode to an intermediate
   file, else we decode to final output file */

 if (encryptedheader.compression==TRUE){
     if (data.verbose==TRUE) puts("Decompressing file...");

     GetTempFile (data.compfile, "lzh"); /* make up random name for temp file */

	    /*   void lzhDecode( FILE *in, FILE *out ); */
      actualfile=data.compfile;      /* write output to compressed file name */

 }  else
    actualfile=encryptedheader.outfilename;   /* write output to final form */

 if ((outfile = fopen(actualfile, "w+b")) == NULL){
   fprintf(stderr, "Unable to open outfile specified in encrypted header.\n");
   fprintf(stderr, "The Pad file may have been previously used\n");
   return 1;
 }

 for(;;){

  if(fread(&c2, sizeof(unsigned char),1,cipherfile ) <1) break;
  if(fread(&c1, sizeof(unsigned char),1,padfile )<1) break;

  c3 = c1 ^ c2 ;
  fwrite(&c3,sizeof(unsigned char),1,outfile);

 }


   /* if compression was set, we must first decompress the intermediate file */
    if (encryptedheader.compression==TRUE){

     if ((finalfile = fopen(encryptedheader.outfilename, "wb"))== NULL){
       fprintf(stderr, "Unable to open outfile");
       return 1;
      }
     fseek(outfile,0L,SEEK_SET);
     lzhDecode( outfile, finalfile);
    }

fclose(finalfile);

  /* last step, verify CRC */
  data.crc32= 0xFFFFFFFFL;   /* Initalize CRC */
  getcrc32(&data.crc32,encryptedheader.outfilename);


  if (data.verbose==TRUE){
   printf("\n\nThe source file had a CRC of:      %08lX \n",encryptedheader.crc32);
   printf("Reconstructed file has a CRC32 of: %08lX \n",data.crc32);
  }

 if (data.crc32!=encryptedheader.crc32) error_exit(BAD_CRC);

 if (data.verbose==TRUE) printf("\nThe file \"%s\" was reconstructed successfully :-)\n",encryptedheader.outfilename);

 /* we now store SessionBegin, and SessionEnd to mark the begining
    and end of the pad used for this session.  We can then overwrite that
    section */


 data.SessionEnd=ftell(padfile);        /* the end of this session, this info used
					   to overwrite pad */

     /* now "nul" out part of pad used in decryption */
     WipeArea(padfile, data.SessionBegin, data.SessionEnd); /* zap it */


 fcloseall();  /* close all open files */
  if(encryptedheader.compression==TRUE){  /* wipe compressed intermediate file */
   if (data.verbose==TRUE) printf("Wiping intermediate compressed file:");
   if (wipefile(data.compfile) >0) error_exit (CANT_WIPE_FILE);
  }
}
