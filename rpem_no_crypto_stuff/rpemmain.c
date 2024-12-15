/*--- rpemmain.c --  Main program for Rabin Privacy Enhanced Mail
 *   (Riordan Privacy Enhanced Mail?)
 *   public key encryption package.
 *
 *   This program implements almost a subset of RFC 1113 Privacy
 *   Enhanced Mail.  It uses the Rabin public key encryption system
 *   to exchange encrypted message keys.
 *   "rpem" is meant to be called to pre-process a mail message
 *   prior to being sent.  The recipient runs the encrypted
 *   message through "rpem" to get the plaintext back.
 *
 *   This code was meant to be a simple demo program for people
 *   wanting to use the Rabin public key system, but it kinda got
 *   out of hand.
 *
 *   For the calling sequence, see the usage_msg below.
 *   For more information, see the accompanying files "0readme.txt"
 *   and "0install.txt".
 *
 *   Mark Riordan   14 March 1991 - 14 April 1991
 *   This code is hereby placed in the public domain.
 */

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#ifdef MSDOS
#include <io.h>
#include <time.h>
#endif
#include <errno.h>
#include <stdlib.h>
#ifdef __TURBOC__
#define fileno(f)     ((f)->fd)
#else
#ifndef __convexc__
#include <memory.h>
#endif
#endif
#include <string.h>

#define BOOL int
#define LINEBUFSIZE  120

#include "zbigint.h"
#include "zproto.h"
#include "prcodepr.h"
#include "usagepro.h"
#include "getoptpr.h"
#include "typtime.h"
#include "blockcip.h"
#include "blockcpr.h"
#include "cippkgpr.h"
#include "rpemprot.h"
#include "getsyspr.h"
#include "newdes.h"
#ifdef UNIX
#ifdef __MACH__
#include <libc.h>
#endif
#include <pwd.h>
#endif

#include "p.h"

char author[] = "Mark Riordan  1100 Parker  Lansing MI  48912";
char author2[] = "riordanmr@clvax1.cl.msu.edu   March-April 1991";


void procoutbuf P((void));

int getopt P((int argc,char **argv, char *opts));

#define TRUE 1
#define FALSE 0

#define MOD_FILE_ENV         "RABIN_MODULUS_FILE"
#define PQ_KEY_ENV           "RABIN_PQ_KEY"
#define PQ_FILE_ENV          "RABIN_PQ_FILE"

#ifdef MSDOS
#define PQ_FILE_DEFAULT      "\\RABINKEY"
#define MODULUS_FILE_DEFAULT "\\RPUBKEYS"
#else
#define PQ_FILE_DEFAULT      ".rabinkey"
#define MODULUS_FILE_DEFAULT "/usr/local/etc/rpubkeys"
#endif

#define USER_FIELD           "User:"
#define PUB_KEY_FIELD        "Rabin-Modulus:"

#define HEADER_STRING  "-----PRIVACY-ENHANCED MESSAGE BOUNDARY-----"

#define PUBKEYSIZE   256
#define KEYSIZE      32
#define MAX_PRENCODE_BYTES 48
#define MAXRECIP     32


char *usage_msg[] = {
"rpem 1.0 -- Rabin Privacy Enhanced Mail",
"Enciphers or deciphers a message using a traditional private key (symmetric)",
"cipher with a message key generated pseudo-randomly, enciphered using the",
"Rabin public key (asymmetric) system, and included with the ciphertext of the",
"message.  Output from encryption and input to decryption are in RFC 1113",
"printable coded format.",
" ",
"(In the Rabin system,  ciphertext = plaintext^2 mod p*q  (p, q are primes);",
"the private component consists of p & q, and the public consists of p*q.)",
" ",
"Usage: ", 
"rpem {-e -r recipient [-p pubkeyfile] [-a {des|newdes|none}]",
"      [-b] [-n] [-m {ascii|binary}] [-v init_vec_hex] [-k ASCII-key]",
"  | -d [-g]}    [-u my_username] [-s mykeyfile] [-i in] [-D]  <in >out",
" ",
" where:",
"-e    means encipher,",
"-d    means decipher",
" ",
"-r    specifies the user to whom the message is being sent.  (Applies",
"      only when enciphering.)  The username is used to lookup the public",
"      key component in the keyfile.  Multiple recipients can be specified",
"      by repeating the -r option.",
" ",
"-u    specifies the username of the user running the program.",
"      Used in both encryption and decryption.  Under Unix, it",
"      defaults to <loginname>@<hostname>.  Under MS-DOS, the value of the",
"      environment variable USER_NAME is used if found, else \"me\" is used.",
" ",
"-a    specifies the encryption algorithm used for the body of the message.",
"      The default is DES (not available in exportable versions).",
"      \"newdes\" specifies Robert Scott's algorithm.  (See",
"      Cryptologia, January 1985.)",
"      Additional algorithms may be made available in the future.",
" ",
"-b    specifies that Electronic Code Book mode is used; that is, each ",
"      block of the message is encrypted independently.  The default is",
"      Cipher Block Chaining, with a pseudo-randomly generated ",
"      initialization vector (initial block).",
" ",
"-n    specifies that the program should not prompt for pseudo-random input",
"      from the keyboard.  Instead, only various information obtainable from",
"      the running system, such as the date and time, are used to generate",
"      the message key and, if applicable, the initialization vector.",
"      The default is to both prompt the user and use runtime information.",
" ",
"-m    specifies the mode of the input file--ASCII or binary.  Further",
"      options may be made available in the future.  The default is", 
"      \"ascii\".  Note:  this option has no effect on Unix systems.",
" ",
"-v    specifies the initialization vector (initial block) in hex",
"      when -b is not specified.  The value must be exactly one block",
"      long; e.g., 8 bytes (16 hex digits) for DES.  The default is",
"      for the program to generate an initialization vector ",
"      pseudo-randomly.  This option is most often used to verify",
"      correct operation of the program.",
" ",
"-k    specifies the message key, in ASCII.  The default is for rpem",
"      to generate the message key pseudo-randomly, from values obtained",
"      from the running system and possibly from a random string entered",
"      by the user.  If DES is being used, the string must be no more",
"      than 8 characters long, and the following manipulations are ",
"      performed:  The high bit of each character is set to make the",
"      character odd parity.  The bottom bit of each character is then",
"      ignored.  (This is a standard treatment for DES ASCII keys.)",
"      The key is padded to 8 bytes with odd-parity zeros.",
"      This option is used primarily to verify the correct implementation",
"      of the DES algorithm used in this program.",
" ",
"-g    means that the keys in keyfile are encrypted (garbled).  If",
"      specified, the program looks for the environment variable ", 
"      RABIN_PQ_KEY.  If the variable is found, its value is used as",
"      the key to decipher the two primes.",
"      If -g is specified and RABIN_PQ_KEY is not found,",
"      the program will prompt for the decryption key.",
"      -g is used only when deciphering.",
" ", 
"-s    specifies the file containing the secret (or private) component of",
"      the Rabin key, in the format generated by \"makepq\".",
"      This file is required for decipherment, and is also used for",
"      encipherment (to remind the recipient of your public key).",
"      If -s is not specified, the program looks for an environment",
"      variable named RABIN_PQ_FILE; if found, its value is used as",
"      the file name; otherwise, the name defaults to \"~/.rabinkey\"",
"      for Unix or \"\\RABINKEY\" for MS-DOS.",
" ",
"-p    specifies the file containing the public components of Rabin keys,",
"      indexed by user name.",
"      If -p is not specified, the program looks for an environment",
"      variable named RABIN_MODULUS_FILE; if found, its value is used",
"      as the file name; otherwise, the name defaults to ",
"      \"/usr/local/etc/rpubkeys\" for Unix, or \\RPUBKEYS for MS-DOS.",
" ",
"-i    specifies input file.  Default is standard input.",
" ",
"-D    enables debug output.",
  NULL
};

extern char *optarg;

TypAlgorithm algorithm = des;
TypFileMode file_mode = FMascii;
TypBlockMode block_mode = cbc;
char *recipient[MAXRECIP], username[LINEBUFSIZE];
char pubfilename[LINEBUFSIZE], privfilename[LINEBUFSIZE];
char infilename[LINEBUFSIZE];
unsigned char messagekey[KEYSIZE];
char err_msg_text[LINEBUFSIZE];
unsigned char user_init_vec[LINEBUFSIZE];
int encipher, garble = FALSE, prompt = TRUE;
int got_infilename = FALSE, got_init_vec = FALSE;
int got_message_key = FALSE;
int nrecip = 0;
int debug=FALSE;
int init_vec_size;

int inhandle, outhandle;
FILE *instream, *outstream, *pubstream, *privstream;

int
main(argc,argv)
int argc;
char *argv[];
{

   char *err_msg;

   /* Initialize the BIGINT package. */
   zstart();
   zrstart((MYINT)44444447);

   /* Parse the command line. */
   err_msg = CrackCmd(argc,argv);
   
   if(err_msg) {
      usage(err_msg,usage_msg);
      return(1);
   }

   /* Open files. */
   if(got_infilename) {
      instream = fopen(infilename,"r");
      if(!instream) {
         fprintf(stderr,"Can't open input file %s\n",infilename);
         return(1);
      }
   } else {
      instream = stdin;
   }

   inhandle = fileno(instream);
   outstream = stdout;
   outhandle = fileno(outstream);

   privstream = fopen(privfilename,"r");
   if(!privstream) {
      fprintf(stderr,"Can't open key file \"%s\".\n",privfilename);
      return(1);
   }

   if(encipher) {
      pubstream = fopen(pubfilename,"r");
      if(!pubstream) {
         fprintf(stderr,"Can't open public key file \"%s\".\n",pubfilename);
         return(1);
      }
      if(debug) {
         fprintf(stderr,"User is \"%s\"; recipient is \"%s\"; \n public key file is \"%s\".\n",
          username,recipient,pubfilename);
      }
   }

   /* Do the enciphering/deciphering. */
   if(encipher) {
      err_msg = DoEncipher();
   } else {
      err_msg = DoDecipher();
   }

   if(err_msg) {
      fputs(err_msg,stderr);
      fputc('\n',stderr);
      return(1);
   }
   return(0);
}

/*--- function CrackCmd ---------------------------------------------------
 *
 *  Parse the command line.
 *
 *  Entry   argc     is the usual argument count.
 *          argv     is the usual vector of pointers to arguments.
 *
 *  Exit    Returns the address of a string if an error occurred,
 *            else NULL is returned and some subset of the following
 *            global variables has been set:
 *
 *    encipher       = TRUE if enciphering selected; FALSE for deciphering.
 *    recipient      is the name of the recipient (if enciphering).
 *    file_mode      indicates the mode of the input file (ASCII vs binary)
 *                   if enciphering.
 *    user_init_vec  is the desired initialization vector if specified.
 *    init_vec_size  is the number of bytes in user_init_vec, if specified.
 *    username       is the name of the user running the program.
 *    algorithm      is the desired encryption technique if enciphering.
 *    block_mode     is the block mode (CBC vs. ECB) if enciphering.
 *    prompt         = TRUE if we are to prompt the user for a "random"
 *                   string to help generate the message key.
 *    garble         is TRUE if the private key is encrypted, if deciphering.
 *    pubfilename    is the name of the public key file if encrypting.
 *    privfilename   is the name of the private key file.
 *    debug          is TRUE if debugging has been selected.
 *    infilename     is the name of the input file if specified.
 *                   Normally, standard input is used; this option was
 *                   implemented due to shortcomings in Microsoft's
 *                   Codeview, which was used during development.
 *    got_infilename is TRUE if the input file was specified explicitly.
 */

char *
CrackCmd(argc, argv)
int argc;
char *argv[];
{
   int gotcip = FALSE;
   int gotalgorithm = FALSE, gotusername = FALSE;
   int gotblock = FALSE, gotgarble = FALSE, gotfilemode = FALSE;
   int gotprivfilename = FALSE, gotpubfilename = FALSE;
   int j, ch;
   char *err_msg = NULL, *cptr;
#ifdef UNIX
   struct passwd *pwptr;
#endif

   while((ch = getopt(argc,argv,"cdeu:r:p:s:m:v:k:a:i:bngD")) != -1) {
      switch (ch) {
         case 'd':
            encipher = FALSE;
            gotcip = TRUE;
            break;
            
         case 'e':
            encipher = TRUE;
            gotcip = TRUE;
            break;
            
         case 'r':
            /* Store the name of another recipient.  */
            cptr = malloc(strlen(optarg)+1);
            if(!cptr) return("Can't allocate memory!");
            strcpy(cptr,optarg);
            recipient[nrecip++] = cptr;
            break;

         case 'm':
            if(strcmp(optarg,"ascii")==0) {
               file_mode = FMascii;
            } else if(strcmp(optarg,"binary")==0) {
               file_mode = FMbinary;
            } else {
               err_msg = "Incorrect file mode.";
            }
            gotfilemode = TRUE;
            break;

         case 'v':
            init_vec_size = HexToBin(optarg,LINEBUFSIZE,user_init_vec);
            got_init_vec = TRUE;
            break;

         case 'k':
            strncpy((char *)messagekey,optarg,KEYSIZE);
            got_message_key = TRUE;
            break;
                     
         case 'u':
            strncpy(username,optarg,LINEBUFSIZE);
            gotusername = TRUE;
            break;
            
         case 'a':
            if(match(optarg,"des")) {
               algorithm = des;
            } else if(match(optarg,"newdes")) {
               algorithm = newdes;
            } else if(match(optarg,"none")) {
               algorithm = none;
               block_mode = ecb;
            } else {
               err_msg = "Algorithm must be \"des\", \"newdes\", or \"none\".";
            } 
            gotalgorithm = TRUE;
            break;
                    
         case 'b':
            block_mode = ecb;
            gotblock = TRUE;
            break;
            
         case 'n':
            prompt = FALSE;
            break;
            
         case 'g':
            garble = TRUE;
            gotgarble = TRUE;
            break;
            
         case 'p':
            strncpy(pubfilename,optarg,LINEBUFSIZE);
            gotpubfilename = TRUE;
            break;

         case 's':
            strncpy(privfilename,optarg,LINEBUFSIZE);
            gotprivfilename = TRUE;
            break;

         case 'D':
            debug = TRUE;
            break;

         case 'i':
            strncpy(infilename,optarg,LINEBUFSIZE);
            got_infilename = TRUE;
            break;

      }
   }

   /* Clear the parameters so that users typing "ps" or "w" can't
    * see what parameters we are using.
    */

   for(j=1; j<argc; j++) {
      cptr = argv[j];
      while(*cptr) *(cptr++) = '\0';
   }
   
   /* Check for syntax error. */

   if(!gotcip) {
      err_msg = "Must specify -e or -d";
   } else if(encipher) {
      if(!nrecip) {
         err_msg = "Must specify recipient when enciphering.";
      } else if(gotgarble) {
         err_msg = "-g is only for deciphering.";
      }
   } else if(gotalgorithm) {
      err_msg = "-a is only for enciphering (automatically detected for -d).";
   } else if(nrecip) {
      err_msg = "-r should be specified only when enciphering.";
   } else if(got_init_vec) {
      err_msg = "-v is only for enciphering.";
   }

   /* Obtain the username if it wasn't specified. */

   if(!gotusername) {
      GetUserAddress(username,LINEBUFSIZE);
   }

   /* Obtain the name of the file containing the private key.
    */

   if(!gotprivfilename) {
      cptr = getenv(PQ_FILE_ENV);
      if(!cptr) {
#ifdef UNIX
         GetUserHome(privfilename,LINEBUFSIZE);
         if(privfilename[0]) {
            if(privfilename[strlen(privfilename)-1] != '/') {
               strncat(privfilename,"/",LINEBUFSIZE-strlen(privfilename));
            }
            strncat(privfilename,PQ_FILE_DEFAULT,
             LINEBUFSIZE-strlen(privfilename));
         } else {
            strcpy(privfilename,PQ_FILE_DEFAULT);
         }
#else
         strcpy(privfilename,PQ_FILE_DEFAULT);
#endif
      } else {
         strncpy(privfilename,cptr,LINEBUFSIZE);
      }
   }

   /* If we are enciphering, obtain the name of the file containing
    * the public keys.
    */
   if(encipher) {
      if(!gotpubfilename) {
         cptr = getenv(MOD_FILE_ENV);
         if(!cptr) {
            cptr = MODULUS_FILE_DEFAULT;
         }
         strncpy(pubfilename,cptr,LINEBUFSIZE);
      }
   }
   return(err_msg);
}


/*--- function PosFileLine ---------------------------------------------
 *
 *  Position the input file, consisting of lines formatted as below,
 *  to just after a given field name.
 *
 *  The lines look like:
 *     <fieldname> <white space> <optional values>
 *
 *  Entry:  stream   is the input file to search.
 *          field    is the name of the field to search for.
 *
 *  Exit:   Returns TRUE if the field is found, else FALSE.
 *          The input stream is positioned just after the line
 *            containing the field name.
 */
BOOL
PosFileLine(stream,field)
FILE *stream;
char *field;
{
   char line[LINEBUFSIZE];
   int fieldlen = strlen(field);

   while(fgets(line,LINEBUFSIZE,stream)) {
      if(strncmp(line,field,fieldlen)==0) {
         return(TRUE);
      }
   }
   return(FALSE);
}

/*--- function GetFileLine ---------------------------------------------
 *
 *  Search an input stream for a line starting with a given field name,
 *  and return the rest of that line.
 *  Lines are formatted as described in PosFileLine.
 *
 *  Entry   stream   is the input stream.
 *          field    is the text of the field (including trailing
 *                   colon if applicable).
 *          valuelen is the maximum number of bytes that can fit in "value".
 *
 *  Exit    value    is the rest of the line, not including the
 *                   name of the field and the intervening white space.
 *          Returns TRUE if the field was found, else FALSE.
 */
BOOL
GetFileLine(stream,field,value,valuelen)
FILE *stream;
char *field;
char *value;
int valuelen;
{
   char line[LINEBUFSIZE], *cptr;
   int fieldlen = strlen(field);

      while(fgets(line,LINEBUFSIZE,stream)) {
         if(strncmp(line,field,fieldlen)==0) {
            cptr = line+fieldlen;
            while((*cptr==' ' || *cptr=='\t') && *cptr) cptr++;
            strncpy(value,cptr,valuelen);
            return (TRUE);
         }
      }
   return(FALSE);
}

/*--- function DoEncipher ------------------------------------------------
 *
 *  Obtain or generate the necessary information (recipient public key,
 *  message key, etc.) to encipher the message, output the message
 *  header, and call the routine that will do the work of enciphering.
 *
 *  Entry:  Global variables contain the result of cracking the
 *            control statement.
 *
 *  Exit:   Returns NULL if encipherment completed OK, else the
 *            address of an error message.
 */
char *
DoEncipher()
{
   char line[LINEBUFSIZE];
   unsigned char mypubkey[PUBKEYSIZE];
   unsigned char *msgkeycip[MAXRECIP], mic[PUBKEYSIZE];
   unsigned char *init_vec;
   int msgkeycipsize[MAXRECIP];

#define KEY_RAN         0
#define KEY_TIMER_RAN   1
#define USER_RAN        2
#define USER_TIMER_RAN  3
#define SYSTEM_RAN      4
#define NUM_RAN         5
   unsigned char *random_bytes[NUM_RAN];
   unsigned char user_rb[LINEBUFSIZE], user_timer_rb[LINEBUFSIZE];
   unsigned char key_rb[LINEBUFSIZE], key_timer_rb[LINEBUFSIZE];
   unsigned char system_rb[LINEBUFSIZE];
   unsigned char ran_array[NUM_RAN*LINEBUFSIZE];
   int random_cnt[NUM_RAN];
   char *err_msg;
   int nbytes=0, j, rancnt, keysize, blksize;
   int mypubkeysize, micsize=0;
   TypCipherInfo  msgcipinfo;

   msgcipinfo.algorithm = algorithm;
   msgcipinfo.blockmode = block_mode;
   BlockInit(&msgcipinfo);

   if(algorithm != none) {
      random_bytes[KEY_RAN] = key_rb;
      random_bytes[KEY_TIMER_RAN] = key_timer_rb;
      random_bytes[USER_RAN] = user_rb;
      random_bytes[USER_TIMER_RAN] = user_timer_rb;
      random_bytes[SYSTEM_RAN] = system_rb;
      for(j=0; j<NUM_RAN; j++) random_cnt[j] = 0;

      /* Generate a message key.  */

      keysize = msgcipinfo.keysize;
      blksize = msgcipinfo.blocksize;

      /* If the user has requested it, obtain a random string from the
       * user typing at the keyboard.
       */
      if(prompt) {
         fputs("Enter a random string: ",stderr);
         random_cnt[USER_RAN] = random_cnt[USER_TIMER_RAN] = LINEBUFSIZE;
         GetUserInput(random_bytes[USER_RAN],&random_cnt[USER_RAN],
          random_bytes[USER_TIMER_RAN],&random_cnt[USER_TIMER_RAN],TRUE);
      }

      /* Get "random" bytes from the running system and mash the whole
       * thing together.
       * Use this to make the message key unless the user has explicitly
       * specified the message key.
       */
      random_cnt[SYSTEM_RAN] = GetRandomBytes(random_bytes[SYSTEM_RAN],
       LINEBUFSIZE);
      rancnt = ConcatArrays(random_bytes,random_cnt,NUM_RAN,ran_array,
       NUM_RAN*LINEBUFSIZE);
      if(got_message_key) {
         if(msgcipinfo.algorithm == des) {
            /* The 56-bit DES key is typically obtained from 8 ASCII
             * characters in a bizzare way.  The top bit of each character
             * is set to reflect odd parity.  The DES code then looks
             * only at the top 7 bits of each byte.
             */
            int parity, bit;
            unsigned char c;

            for(j=0; j<keysize; j++) {
               c = messagekey[j];
               for(parity=0x80,bit=0; bit<7; bit++) {
                  c <<= 1;
                  parity ^= (c&0x80);
               }
               messagekey[j] = (messagekey[j]&0x7f) | parity;
            }
            /* Zero-pad the key with odd parity. */
            for(; j<keysize; j++) messagekey[j] = 0x80; 
         }
      } else {
         RandomizeBytes(ran_array,rancnt,messagekey,keysize);
      }
      if(debug) {
         BinToHex(messagekey,keysize,line);
         fprintf(stderr,"Message key (plain, %d bytes) = %s\n",keysize,line);
      }

      /* If CBC is being used, we need an initialization vector (IV).
       * Unless the user has specified one, generate an IV.
       * We are using the same pseudo-random information to generate
       * the IV as was used to generate the message key.
       * Since the IV is passed in the clear, we must be careful to
       * thoroughly scramble the information in a different way so that
       * the message key cannot (reasonably) be determined from the IV.
       */

      if(block_mode == cbc) {
         if(got_init_vec) {
            if(init_vec_size == blksize) {
               init_vec = user_init_vec;
            } else {
               return("Improper size for initialization vector.");
            }
         } else {
            init_vec = (unsigned char *) malloc(msgcipinfo.blocksize);
            if(!init_vec) return("Error--cannot allocate memory.");

            RandomizeBytes(ran_array,rancnt,init_vec,2);

            srand(init_vec[0]+(init_vec[1]<<8));
            for(j=0; j<rancnt; j++) {
               ran_array[j] ^= (unsigned char)(rand());
            }
            for(j=0; j<5; j++) {
               RandomizeBytes(ran_array,rancnt,ran_array,rancnt-1);
            }
            RandomizeBytes(ran_array,rancnt,init_vec,blksize);
         }
      }

      /* For each recipient, find his/her public key and encipher
       * the message key with it.
       */
      for(j=0; j<nrecip; j++) {
         err_msg = DoEncRecipient(j,messagekey,keysize,recipient,
           msgkeycip,msgkeycipsize);
         if(err_msg) return(err_msg);
      }
   }
   /* Get our own public key so we can include it in the message. */

   PosFileLine(privstream,PUB_KEY_FIELD);
   mypubkeysize = GetEncodedBytes(privstream,mypubkey,PUBKEYSIZE);

   /* Now write out the message header.   */

   WriteHeader(outstream,file_mode,nrecip,recipient,username,&msgcipinfo,init_vec,
     msgkeycip,msgkeycipsize,mypubkey,mypubkeysize,mic,micsize);

   /* Finally, it's time to start doing some encryption!  */

#ifdef MSDOS
   /* Switch read mode to binary if necessary.    */
   if(file_mode == FMbinary) {
      setmode(inhandle,O_BINARY);
   }
#endif

   err_msg = EncipherHandle(inhandle,outstream,messagekey,init_vec,&msgcipinfo);
   fputs(HEADER_STRING,outstream);
   fputc('\n',outstream);

   return err_msg;
}

/*--- function DoEncRecipient -----------------------------------------
 *
 *  Do the header-related per-recipient preparations for encipherment:
 *  Find the recipient's public key, and encrypt the message key
 *  using this public key.
 *
 *  Entry:  irecip      is the number of the recipient to process.
 *          messagekey  is the message key.
 *          keysize     is the number of bytes in messagekey.
 *          recipient   is the array of recipient names.
 *
 *  Exit:   Returns     pointer to an error message, or NULL.
 *          msgkeycip   is an array whose irecip-th entry contains
 *                      the message key enciphered with this recipient's
 *                      public key.
 *          msgkeycipsize  is an array whose irecip-th entry contains
 *                      the number of bytes in msgkeycip[irecip].
 */
char *
DoEncRecipient(irecip,messagekey,keysize,recipient,msgkeycip,msgkeycipsize)
int irecip;
unsigned char *messagekey;
int keysize;
char *recipient[];
unsigned char *msgkeycip[];
int msgkeycipsize[];
{
   char *recip = recipient[irecip];
   unsigned char *mkeycip = msgkeycip[irecip];
   int gotone, found, j, pubkeysize;
   char line[LINEBUFSIZE];
   unsigned char pubkey[PUBKEYSIZE];
   BIGINT modulus;

   /* Get the modulus (public key) for this user.
    * First, find the user.
    */
   fseek(pubstream,0L,0);   /* rewind file */
   do {
      gotone = GetFileLine(pubstream,USER_FIELD,line,LINEBUFSIZE);
      for(j=0,found=TRUE; found && recip[j]; j++) {
         if(line[j] != recip[j]) found=FALSE;
      }
      /* Guard against mere initial substring matches */
      if(found) {
         if(isprint(line[j]) && line[j]!=' ') found=FALSE;
      }
   } while(gotone && !found);

   if(!found) {
      sprintf(err_msg_text,"Can't find user %s in the public key file.",
         recip);
      return(err_msg_text);
   }

   /* Now, find the public key for that user. */

   gotone = GetFileLine(pubstream,PUB_KEY_FIELD,line,LINEBUFSIZE);
   if(!gotone) {
      return("No public key found for this user.");
   }

   /* Read in the lines of printable-encoded characters, decoding
    * them to binary and placing them in "pubkey".
    */
   pubkeysize = GetEncodedBytes(pubstream,pubkey,PUBKEYSIZE);

   /* Convert this recipient's public key to BIGINT form.
    */

   BytesToBignum(pubkey,pubkeysize,&modulus);

   /* Encrypt the message key using the recipient's public key. */

   msgkeycip[irecip] = (unsigned char *)malloc(PUBKEYSIZE);
   if(!msgkeycipsize) return("Can't allocate memory!");
   msgkeycipsize[irecip] = RabinEncipherBuf(messagekey,keysize,
    &modulus,msgkeycip[irecip]);
   if(debug) {
      BinToHex(msgkeycip[irecip],msgkeycipsize[irecip],line);
      fprintf(stderr,"Message key (encrypted, %d bytes) = %s\n",msgkeycipsize,line);
   }
   return NULL;
}

/*--- function EncipherHandle ---------------------------------------
 *
 *  Do the actual work of reading the input, enciphering it,
 *  and writing it out.
 *
 *  Entry:  inhand      is the handle of the input file or stream.
 *                      A handle is used instead of a stream because
 *                      because of the desire to use the more efficient
 *                      "read" system call.
 *          outstream   is the output stream.  A stream rather than
 *                      handle is used because the output is coded
 *                      lines (using fputs).
 *          key         is the message key.
 *          init_vec    is the initialization vector (ignored if
 *                      Electronic Code Book mode being used).
 *          cipinfo     describes the cipher algorithm to be used.
 *
 *  Exit:   Returns NULL if encipherment went OK, else the address
 *            of an error message.
 */
char *
EncipherHandle(inhand,outstream,key,init_vec,cipinfo)
int inhand;
FILE *outstream;
unsigned char *key;
unsigned char *init_vec;
TypCipherInfo *cipinfo;
{
/* BUFSIZE1 is the I/O buffer size.  I can't decide if it should be
 * small to increase memory cache hits, or large to increase I/O subsystem
 * efficiency.
 */
#define BUFSIZE1 (4096)
   unsigned char *begbuf, *buf, *prev_blk, *bptr;
   unsigned char *end_prcode_ptr, *end_cip_ptr, *blockptr;
   int blksize, do_CBC, already_cip, not_cip, bytes_to_cip, j;
   int bytes_to_code, bytes_in_line;
   char linebuf[LINEBUFSIZE];

   do_CBC = (cipinfo->blockmode == cbc);
   blksize = cipinfo->blocksize;
   begbuf = (unsigned char *)malloc(BUFSIZE1+cipinfo->blocksize+MAX_PRENCODE_BYTES);
   if(!begbuf) return("Error--cannot allocate memory.");
   if(do_CBC) {
      memcpy(begbuf,init_vec,blksize);
   }
   buf = begbuf+ cipinfo->blocksize;

   BlockSetKey(key,cipinfo->keysize,1,cipinfo);

   /* The code below loops through the input, reading bytes,
    * enciphering them, encoding them to printable characters,
    * and writing out the ASCII lines.
    * The code is complicated by the fact that except for the
    * end of the input, bytes must be enciphered in groups of
    * blksize bytes, and must be printable-encoded in groups of
    * MAX_PRENCODE_BYTES bytes.  (See RFC 1113; all line except the
    * last must represent exactly MAX_PRENCODE_BYTES binary bytes.)
    *
    * This means that at any time, the buffer buffer we are using
    * could be divided into quite a number of different regions:
    *
    *
    *  begbuf ->  -----------------------------
    *                                               \
    *              Initialization vector              blksize bytes
    *              (initial init_vec)               /
    *  buf    ->  ------------------------------
    *              Bytes already enciphered but     \
    *              not yet prencoded or written      \
    *              out.  This will be a multiple       already_cip bytes
    *  prev_blk->  of blksize and less than          /
    *              MAX_PRENCODE_BYTES               /
    *             -------------------------------
    *              Bytes read in last time but      \
    *              not enciphered.  Will be less       not_cip bytes
    *              than blksize bytes.              /
    *             -------------------------------
    *              Bytes just read in.
    *              Will be <= BUFSIZE1 bytes.
    *
    *  endbptr->  -------------------------------
    */

   blksize = cipinfo->blocksize;
   already_cip = not_cip = 0;
   while((bytes_to_cip=read(inhand,buf+already_cip+not_cip,BUFSIZE1)) > 0) {
      /* The number of bytes eligible for enciphering is the number
       * of bytes just read in plus the number not enciphered during
       * the last pass (because they wouldn't fit in a block).
       */
      bytes_to_cip += not_cip;

      /* Don't encipher at all unless we have at least a block's worth */
      if(bytes_to_cip >= blksize) {
         blockptr = buf+already_cip;
         not_cip = bytes_to_cip % blksize;
         end_cip_ptr = blockptr + (bytes_to_cip - not_cip);
         prev_blk = buf + already_cip - blksize;;

         /* Here's the actual encipherment loop. */
         for(; blockptr != end_cip_ptr; blockptr += blksize) {
            if(do_CBC) {
               for(bptr=blockptr, j=blksize; j; j--) {
                  *(bptr++) ^= *(prev_blk++);
               }
            }
            BlockEncipherBlock(blockptr,cipinfo);
         }

         /* Now we must printable-encode and output the enciphered
          * bytes.  Encode and output only in multiples of
          * MAX_PRENCODE_BYTES bytes.
          */
         already_cip = (end_cip_ptr - buf) % MAX_PRENCODE_BYTES;
         blockptr = buf;
         end_prcode_ptr = blockptr + (end_cip_ptr - buf - already_cip);
         for(; blockptr != end_prcode_ptr; blockptr+=MAX_PRENCODE_BYTES) {
            prencode(blockptr,MAX_PRENCODE_BYTES,linebuf);
            fputs(linebuf,outstream);   fputc('\n',outstream);
         }

         /* Copy the enciphered bytes that didn't get encoded
          * to the beginning of the buffer for processing on
          * the next pass.
          */
         memcpy(buf,end_prcode_ptr,already_cip);

         /* If the number of bytes to encipher wasn't a multiple of
          * blksize, copy the extra bytes back to the beginning of the
          * buffer and next time, start the read just after these bytes.
          * This will be no more than blksize-1 bytes.
          */
         memcpy(buf+already_cip,end_cip_ptr,not_cip);

         /* Copy the last enciphered block to the initialization vector. */
         init_vec = buf + already_cip - blksize;
         memcpy(init_vec,end_cip_ptr-blksize,blksize);
      }
   }

   /* We have read the last part of the input.
    * Encipher, encode, and put out the last portion of the buffer.
    */

   blockptr = buf + already_cip;
   bytes_to_cip = blksize + (not_cip/blksize)*blksize;
   blockptr[bytes_to_cip-1] = (unsigned char) (not_cip % blksize);
   end_cip_ptr = blockptr + bytes_to_cip;

   for(; blockptr != end_cip_ptr; blockptr += blksize) {
      if(do_CBC) {
         for(bptr=blockptr, j=blksize; j; j--) {
            *(bptr++) ^= *(init_vec++);
         }
      }
      BlockEncipherBlock(blockptr,cipinfo);
   }

   /* Now the buffer has been completely enciphered;
    * printable encode & output it.
    */
   blockptr = buf;
   bytes_to_code = already_cip + bytes_to_cip;
   for(; bytes_to_code; blockptr+=MAX_PRENCODE_BYTES) {
      bytes_in_line = bytes_to_code >= MAX_PRENCODE_BYTES ?
          MAX_PRENCODE_BYTES : bytes_to_code;
      prencode(blockptr,bytes_in_line,linebuf);
      fputs(linebuf,outstream);  fputc('\n',outstream);
      bytes_to_code -= bytes_in_line;
   }
   return(NULL);
}

struct struct_filemode_text {
   TypFileMode fenum;
   char       *ftxt;
} file_mode_conv[] = {{FMascii,"ASCII"},{FMbinary,"Binary"},
  {FMlast,"Unknown" }};


/*--- function WriteHeader -----------------------------------------
 *
 *  Write the privacy enhanced mail header.
 *
 *  Entry:  stream         is the stream to which the header
 *                         should be written.
 *          filemode       is the mode of the input file (ascii vs. binary).
 *          nrecip         is the number of recipients.
 *          recip          is an array of pointers to names of recipients.
 *          username       is the name of the user running the program
 *                         (the sender of the message).
 *          msgcipinfo     is a structure describing the algorithm
 *                         used to encipher the message.
 *          init_vec       is an initial block to use (ignored if
 *                         ECB mode being used.
 *          msgkeycip      is an array of the message key, enciphered with
 *                         the public keys of the various recipients.
 *          msgkeycipsize  is an array containing the number of bytes
 *                         in the above.
 *          mypubkey       is the byte form (i.e., not the BIGINT form)
 *                         of this user's public key.  This is used only
 *                         to remind the recipient of our message
 *                         what our public key is (in case he wants
 *                         to reply).
 *          mypubkeysize   is the number of bytes in the above.
 *          mic            is the message integrity check (not used yet).
 *          micsize        is the size of the above (not used).
 *
 *  Exit:   The header has been written, including the blank line
 *            which separates the header from the encrypted text.
 */
void
WriteHeader(stream,filemode,nrecip,recip,username,msgcipinfo,init_vec,msgkeycip,
     msgkeycipsize,mypubkey,mypubkeysize,mic,micsize)
FILE *stream;
TypFileMode filemode;
int nrecip;
char *recip[];
char *username;
TypCipherInfo *msgcipinfo;
unsigned char *init_vec;
unsigned char *msgkeycip[];
int msgkeycipsize[];
unsigned char *mypubkey;
int mypubkeysize;
unsigned char *mic;
int micsize;
{
   char line[LINEBUFSIZE], *cptr;
   int j;

#define PROC_TYPE_FIELD       "Proc-Type:"
#define FILE_MODE_FIELD       "File-Mode:"
#define DEK_FIELD             "DEK-Info:"
#define SENDER_FIELD          "Sender-ID:"
#define SENDER_PUB_KEY_FIELD  "Sender-Public-Key:"
#define RECIPIENT_FIELD       "Recipient-ID:"
#define MESSAGE_KEY_FIELD     "Key-Info:"

#define PROC_TYPE_RABIN       "2000"
#define PROC_TYPE_ENCRYPTED   "ENCRYPTED"
#define PROC_TYPE_PLAIN       "PLAIN"
#define DEK_ALG_DES           "DES"
#define DEK_ALG_NEWDES        "NEWDES"
#define MESSAGE_KEY_RABIN     "Rabin"

   fputc('\n',stream);
   fputs(HEADER_STRING,stream);  fputc('\n',stream);

   fputs(PROC_TYPE_FIELD,stream); fputc(' ',stream);
   fputs(PROC_TYPE_RABIN,stream); fputc(',',stream);
   if(msgcipinfo->algorithm == none) {
      cptr = PROC_TYPE_PLAIN;
   } else {
      cptr = PROC_TYPE_ENCRYPTED;
   }
   fputs(cptr,stream);
   fputc('\n',stream);

   if(msgcipinfo->algorithm != none) {
      fputs(DEK_FIELD,stream); fputc(' ',stream);
      switch(msgcipinfo->algorithm) {
         case des:
            cptr = DEK_ALG_DES;
            break;
         case newdes:
            cptr = DEK_ALG_NEWDES;
            break;
      }
      fputs(cptr,stream);
      if(msgcipinfo->blockmode == cbc) {
         fputs("-CBC,",stream);
         BinToHex(init_vec,msgcipinfo->blocksize,line);
         fputs(line,stream);
      }
      putc('\n',stream);
   }

   fputs(FILE_MODE_FIELD,stream); fputc(' ',stream);
   for(j=0; file_mode_conv[j].fenum != FMlast; j++) {
      if(filemode == file_mode_conv[j].fenum) break;
   }
   cptr = file_mode_conv[j].ftxt;
   fputs(cptr,stream);
   fputc('\n',stream);

   fputs(SENDER_FIELD,stream); fputc(' ',stream);
   fputs(username,stream);
   fputc('\n',stream);

   fputs(SENDER_PUB_KEY_FIELD,stream);  fputc('\n',stream);
   PutEncodedBytes(stream,mypubkey,mypubkeysize);

   for(j=0; j<nrecip; j++) {
      fputs(RECIPIENT_FIELD,stream); fputc(' ',stream);
      fputs(recip[j],stream);
      fputc('\n',stream);

      if(msgcipinfo->algorithm != none) {
         fputs(MESSAGE_KEY_FIELD,stream); fputc(' ',stream);
         fputs(MESSAGE_KEY_RABIN,stream);
         fputs(",\n",stream);
         PutEncodedBytes(stream,msgkeycip[j],msgkeycipsize[j]);
      }
   }

   fputc('\n',stream);
}

/*--- function DoDecipher ---------------------------------------------------
 *
 *  Decipher the message.  This includes reading and interpreting the
 *  message header, deciphering the message key, and reading and
 *  deciphering the message text.
 */

char *
DoDecipher()
{
#define NUM_PRIV_KEYS 2
   char *cptr, *err_msg;
   char line[LINEBUFSIZE];
   unsigned char msg_key_bytes[PUBKEYSIZE];
   unsigned char priv_bytes[NUM_PRIV_KEYS][PUBKEYSIZE];
   unsigned char msg_key[LINEBUFSIZE];
   unsigned char key_to_key[LINEBUFSIZE];
   unsigned char init_vec[LINEBUFSIZE];
   unsigned char key_rb[LINEBUFSIZE], key_timer_rb[1];
   BIGINT priv_num[NUM_PRIV_KEYS];
   int key_cnt, key_timer_cnt, msg_key_size, priv_size[NUM_PRIV_KEYS];
   int ikey, last_block_size;
   TypCipherInfo cipinfo, key_cipinfo;
   TypFileMode filemode;
   static char *priv_name[NUM_PRIV_KEYS] = {"p", "q"};

   /* Set mode of output file if necessary.
    */
   msg_key_size = PUBKEYSIZE;
   err_msg = CrackHeader(instream,&filemode,&cipinfo,init_vec,msg_key_bytes,&msg_key_size);
   if(err_msg) return (err_msg);

#ifdef MSDOS
   if(filemode == FMbinary) {
      setmode(outhandle,O_BINARY);
   }
#endif

   if(debug) {
      BinToHex(msg_key_bytes,msg_key_size,line);
      fprintf(stderr,"Message key (enciphered, %d bytes) = %s\n",msg_key_size,line);
   }

   if(cipinfo.algorithm != none) {
      /* Get the private component of the public key. */
      for(ikey=0; ikey<NUM_PRIV_KEYS; ikey++ ) {
         PosFileLine(privstream,priv_name[ikey]);
         priv_size[ikey] = GetEncodedBytes(privstream,priv_bytes[ikey],PUBKEYSIZE);
         if(debug) {
            fprintf(stderr,"%s is %d bytes long.\n",priv_name[ikey],priv_size[ikey]);
         }
      }

      /* If the private component of our public key is encrypted,
       * decrypt it.
       */
      if(garble) {
         cptr = getenv(PQ_KEY_ENV);
         if(cptr) {
            key_cnt = strlen((char *)cptr);
            memcpy(key_rb,cptr,key_cnt);
         } else {
            fputs("Enter the key to the key: ",stderr);
            key_cnt = LINEBUFSIZE;
            key_timer_cnt = 0;
            GetUserInput(key_rb,&key_cnt, key_timer_rb,&key_timer_cnt,FALSE);
         }


         /* We now have the user-typed key to the key in "key_rb".
          * There are "key_cnt" bytes there.
          * Make use of all of these bytes in preparing the real
          * key to the key, even if we
          * have to take several passes through userinbuf.
          * Fill all of the key bytes, even if we have to reuse
          * some of the user input.
          * This parallels the way the key was prepared in "makepq".
          */
         ExpandBytes(key_rb,key_cnt,key_to_key,NEWDES_USER_KEY_BYTES);

         key_cipinfo.algorithm = newdes;
         key_cipinfo.blockmode = ecb;
         BlockInit(&key_cipinfo);
         BlockSetKey(key_to_key,NEWDES_USER_KEY_BYTES,0,&key_cipinfo);

         for(ikey=0; ikey<NUM_PRIV_KEYS; ikey++) {
            if(priv_size[ikey]%key_cipinfo.blocksize) {
               if(debug) {
                  fprintf(stderr,"priv_size[%d] = %d ", ikey,priv_size[ikey]);
               }
               sprintf(err_msg_text,
                  "Error: enciphered key is not a multiple of %d bytes.",
                key_cipinfo.blocksize);
               return(err_msg_text);
            }
            BlockDecipherBuf(priv_bytes[ikey],priv_size[ikey],&key_cipinfo);

            /* Adjust the size of the key; when it's enciphered, the
             * correct size of the last block is stored in the last byte.
             */
            last_block_size = priv_bytes[ikey][priv_size[ikey]-1];
            if(last_block_size >= key_cipinfo.blocksize) {
               return("I can tell that you entered the wrong key to the keys.");
            }
            priv_size[ikey] -= (key_cipinfo.blocksize - last_block_size);
            if(debug) {
               fprintf(stderr,"%s adjusted = %d bytes\n",priv_name[ikey],priv_size[ikey]);
            }
         }
         BlockFinish(&key_cipinfo);
      }

      /* Convert the private component of the key to BIGINT format. */
      for(ikey=0; ikey<NUM_PRIV_KEYS; ikey++) {
         BytesToBignum(priv_bytes[ikey],priv_size[ikey],&(priv_num[ikey]));
         if(debug) {
            fprintf(stderr,"%s = ",priv_name[ikey]);
            fzwrite(stderr,&(priv_num[ikey]),70,"\\","");
            fputc('\n',stderr);
         }
      }

      msg_key_size = RabinDecipherBuf(msg_key_bytes,msg_key_size,
       &(priv_num[0]),&(priv_num[1]),msg_key);

      if(debug) {
         BinToHex(msg_key,msg_key_size,line);
         fprintf(stderr,"Message key (deciphered, %d bytes) = %s\n",
          msg_key_size,line);
         BinToHex(init_vec,cipinfo.blocksize,line);
         if(cipinfo.blockmode == cbc) {
            cptr = "CBC";
         } else if(cipinfo.blockmode == ecb) {
            cptr = "ECB";
         } else {
            cptr = "unknown";
         }
         fprintf(stderr,"Initialization vector = %s, block mode = %s\n",
           line,cptr);
      }
   }

   /* Read in the enciphered and printable encoded bytes.
    * Decode and decipher them.
    */
   err_msg = DecipherStream(instream,outhandle,cipinfo,msg_key,msg_key_size,init_vec);

   return(err_msg);
}

/*--- function DecipherStream -------------------------------------
 *
 *  Routine to decipher a printable decode and decipher an input
 *  stream.  Actual decryption is done by calling other routines;
 *  this function just does the nasty bookkeeping of buffer management
 *  and so on.
 *
 *  We use double-buffering of output because after we've decrypted
 *  a block, we don't, in general, know whether it was the last
 *  block of input.  The last block must be treated differently;
 *  it contains, as the last byte in the block, the number of
 *  good bytes in that block.  We can't write a buffer to disk
 *  until we are absolutely certain that it doesn't contain the
 *  last block of input (or until we have recognized that last
 *  block and have processed it appropriately).
 *
 *  buf1       points to the buffer into which we are currently
 *             putting plaintext.
 *  buf2       points to the buffer which contains already decrypted
 *             bytes.
 *  bufgood    is TRUE iff buf2 contains valid data.
 *  not_cip    is the number of bytes in buf1 which we cannot
 *             decipher on this pass through the loop.  This is
 *             is non-zero only if the number of bytes waiting
 *             to be deciphered is not a multiple of the blocksize.
 *  ivptr      points to the block with which the current block
 *             being deciphered should be XORed in CBC mode.
 *  next_ivptr points to the block with which the next block
 *             being deciphered should be XORed in CBC mode
 *             (namely, the encrypted version of the current block).
 */
char *
DecipherStream(instream,outhandle,cipinfo,msg_key,msg_key_size,init_vec)
FILE *instream;
int outhandle;
TypCipherInfo cipinfo;
unsigned char *msg_key;
int msg_key_size;
unsigned char *init_vec;
{
/* Not sure what would be the optimal BUFSIZE2.  Note that MS-DOS
 * small model does place limitations here, though, with all the
 * other garbage in DGROUP.
 */
#ifdef MSDOS
#define BUFSIZE2      1024
#else
#define BUFSIZE2      4096
#endif
   unsigned char *buf, *buf1, *buf2, *cipptr, *blockptr, *bufptr, *endbptr;
   unsigned char *bptr, *sourptr;
   unsigned char cip_blk[2][LINEBUFSIZE], *ivptr, *next_ivptr;
   static char line[LINEBUFSIZE];
   int not_cip, bytes_in_buf, bytes_in_line, blksize, bufgood;
   int do_CBC, j;

   BlockInit(&cipinfo);
   BlockSetKey(msg_key,msg_key_size,FALSE,&cipinfo);

   blksize = cipinfo.blocksize;
   buf1 = (unsigned char *) malloc(BUFSIZE2+2*MAX_PRENCODE_BYTES);
   buf2 = (unsigned char *) malloc(BUFSIZE2+2*MAX_PRENCODE_BYTES);
   if(!buf1 || !buf2) {
      return("Error--cannot allocate memory");
   }
   ivptr = cip_blk[0];  next_ivptr = cip_blk[1];
   memcpy(ivptr,init_vec,blksize);
   cipptr = bufptr = buf1;
   not_cip = 0;
   bufgood = FALSE;
   do_CBC = (cipinfo.blockmode == cbc);

   /* Each pass through the while loop below reads in one
    * line of printable encoded bytes, unencodes the line,
    * and decrypts as many bytes of the line as possible,
    * placing the bytes in buf1.  When buf1 fills, it is
    * exchanged with buf2 (flushing buf2 to output if necessary).
    */
   while(fgets(line,LINEBUFSIZE,instream)) {
      bytes_in_line = prdecode(line,bufptr,LINEBUFSIZE);
      /* Compute the number of bytes that we won't be able to
       * decrypt in this pass.
       */
      not_cip = (bytes_in_line+not_cip) % blksize;
      endbptr = bufptr + bytes_in_line - not_cip;
      /* Decrypt this chunk of bytes. */
      for(blockptr=cipptr; blockptr!=endbptr; blockptr+=blksize) {
         /* Save a copy of this block for XORing next time in CBC mode. */
         if(do_CBC) {
            for(bptr=next_ivptr, sourptr=blockptr, j=blksize; j; j--) {
               *(bptr++) = *(sourptr++);
            }
         }
         BlockDecipherBlock(blockptr,&cipinfo);
         if(do_CBC) {
            for(bptr=blockptr, sourptr=ivptr, j=blksize; j; j--) {
               *(bptr++) ^= *(sourptr++);
            }
            buf = ivptr; ivptr = next_ivptr; next_ivptr = buf;
         }
      }
      cipptr = endbptr;
      bufptr += bytes_in_line;

      /* buf1 filled.  Flush buf2 if necessary and exchange buf1 and buf2.
       * Also, copy any undecrypted bytes from the end of buf1
       * to the beginning of the new buf1.
       */
      if(cipptr - buf1 >= BUFSIZE2) {
         if(bufgood) {
            write(outhandle,buf2,BUFSIZE2);
         }
         buf = buf1; buf1 = buf2; buf2 = buf;
         memcpy(buf1,buf2+BUFSIZE2,bufptr-buf2-BUFSIZE2);
         bufptr = buf1 + (bufptr-buf2-BUFSIZE2);
         cipptr = bufptr - not_cip;
         bufgood = TRUE;
      }
   }

   /* We hit the end of input.  Flush the buffers. */

   if(bufptr == buf1) {
      bytes_in_buf = BUFSIZE2 - 8 + buf2[BUFSIZE2-1];
      if(bufgood) write(outhandle,buf2,bytes_in_buf);
   } else {
      if(bufgood) write(outhandle,buf2,BUFSIZE2);
      bytes_in_buf = bufptr-buf1 - 8 + *(bufptr-1);
      write(outhandle,buf1,bytes_in_buf);
   }

   if(not_cip) {
      return("Error--input file is not a multiple of the block size.");
   }

   return(NULL);
}

/*--- function CrackHeader -----------------------------------------
 *
 *  Crack the Privacy Enhanced Message header.
 *
 *  Entry:  stream      is a stream positioned somewhere before the
 *                      the privacy enhanced message boundary.
 *
 *  Exit:   Returns NULL if the header cracked OK, else a pointer
 *            to an error message.
 *          The variables below describe information obtained from
 *            the header.
 *          filemode    is the desired mode of the output file.
 *                      Doesn't matter for Unix, but OS/2 and MS-DOS
 *                      do care.  Might be extended for other OS's
 *                      such as VMS.
 *          cipinfo     describes the message cipher specified:
 *                      algorithm and block mode.
 *          init_vec    is the initialization vector (if CBC mode
 *                      was specified).
 *          msg_key_bytes  is the encrypted message key in byte
 *                         format.  (It's really a BIGINT.)
 *          num_key_bytes  is the number of bytes in msg_key_bytes.
 */
char *
CrackHeader(stream,filemode,cipinfo,init_vec,msg_key_bytes,num_key_bytes)
FILE *stream;
TypFileMode *filemode;
TypCipherInfo *cipinfo;
unsigned char *init_vec;
unsigned char *msg_key_bytes;
int *num_key_bytes;
{
   char line[LINEBUFSIZE];
   char field_name[LINEBUFSIZE], *cptr;
#define MAXVALS 4
   char *vals[MAXVALS];
   int got_header = FALSE, doing_header = TRUE, repeat_state;
   int doing_enc = FALSE; /* TRUE if the message is encrypted,
                           * FALSE if plain.  */
   int this_user = FALSE; /* TRUE if the last Recipient-ID is us. */
   int got_msgkey = FALSE;
   int nvals, j, more, seen_field, nbytes, found;
   int max_key_bytes = *num_key_bytes;
   enum enum_state {ST_Begin, ST_Look_Field, ST_Get_Message_Key} state = ST_Begin;
   /* Ahhh, the joys of portability. 
    * I refuse to give up my enum's on non-Ultrix systems.
    */
#ifdef ultrix
#define STRICT_ENUM
#endif

#ifdef STRICT_ENUM
#define PROC_TYPE       0
#define FILE_MODE       1
#define RECIPIENT       2
#define DEK             3
#define SENDER          4
#define SENDER_PUB_KEY  5
#define MESSAGE_KEY     6
#define LAST            7
   int fld;
   typedef int typfield;
#else
   enum enum_field {PROC_TYPE, FILE_MODE, RECIPIENT, DEK, SENDER,
    SENDER_PUB_KEY, MESSAGE_KEY, LAST} fld;
   typedef enum enum_field typfield;
#endif

#ifndef sun
#ifndef ultrix
#ifndef sgi
#define USE_TOKEN_PASTING
#endif
#endif
#endif

#ifdef USE_TOKEN_PASTING
#define f_ent(txt) {txt##_FIELD, txt, FALSE}
#else
#define f_ent(txt) {txt/**/_FIELD, txt, FALSE}
#endif

   static struct struct_field {
      char *fieldnm;
      typfield fieldtype;
      int  seen;
   } fields[] = {
      f_ent(PROC_TYPE), f_ent(FILE_MODE), f_ent(RECIPIENT), f_ent(DEK),
      f_ent(SENDER), f_ent(SENDER_PUB_KEY), f_ent(MESSAGE_KEY)
   };

   *filemode = FMascii;
   *num_key_bytes = 0;
   cipinfo->algorithm = none;
   cipinfo->blockmode = ecb;
   for(j=0; j<MAXVALS; j++) {
      vals[j] = malloc(LINEBUFSIZE);
      if(!vals[j]) return("Error--cannot allocate memory");
   }
   
   while((cptr = fgets(line,LINEBUFSIZE,stream)) && doing_header) {
      /* Stop when we hit a blank line. */
      if(state != ST_Begin && WhiteSpace(*cptr)) {
         while(WhiteSpace(*cptr)) cptr++;
         if(!(*cptr)) {
            doing_header = FALSE;
            break;
         }
      }
      do {
         repeat_state = FALSE;
         switch(state) {

         /* We haven't hit the beginning message boundary yet. */
         case ST_Begin:
            if(strncmp(line,HEADER_STRING,strlen(HEADER_STRING))==0) {
               state = ST_Look_Field;
            }
            break;

         /* We are inside the header. */
         case ST_Look_Field:
            nvals = CrackHeaderLine(line,field_name,vals,MAXVALS,&more);
            if(debug) {
               fprintf(stderr,"Field = \"%s\" [%d]",field_name,strlen(field_name));
               for(j=0; j<nvals; j++) {
                  fprintf(stderr," \"%s\" [%d]",vals[j],strlen(vals[j]));
               }
               fputc('\n',stderr);
            }

            if(!field_name[0] && nvals==0) {
               /* We've reached the end of the header. */
               doing_header = FALSE;
               if(debug) fprintf(stderr,"Got end of header.\n");
               seen_field = TRUE;
            } else {
               seen_field = FALSE;
            }

            for(fld=PROC_TYPE; !seen_field && fld<LAST; fld++) {
               if(match(field_name,fields[fld].fieldnm)) {
                  if(debug) {
                     fprintf(stderr,"Field matches type %d.\n",fld);
                  }
                  switch(fld) {
                     case PROC_TYPE:
                        if(!match(vals[0],PROC_TYPE_RABIN)) {
                           return("I can't process this Proc-Type");
                        }

                        if(match(vals[1],PROC_TYPE_ENCRYPTED)) {
                           /* Do nothing */
                        } else if(match(vals[1],PROC_TYPE_PLAIN)) {
                           cipinfo->algorithm = none;
                        } else {
                           return("Bad Proc-Type in message header.");
                        }
                        break;

                     case FILE_MODE:
                        for(j=0,found=FALSE; !found &&
                         file_mode_conv[j].fenum!=FMlast; j++) {
                           if(match(vals[0],file_mode_conv[j].ftxt)) {
                              *filemode = file_mode_conv[j].fenum;
                              found = TRUE;
                           }
                        }
                        if(!found) return("Bad file mode.");
                        break;

                     case RECIPIENT:
                        if(match(vals[0],username)) {
                           this_user = TRUE;
                           if(debug) {
                              fprintf(stderr,"(This is you.)\n");
                           }
                        }
                        break;

                     case DEK:
                        /* Get the message encryption type & if it
                         * involves Cipher Block Chaining, also
                         * get the Initialization Vector.
                         */
                        cptr = tail(vals[0],4);
                        cipinfo->blockmode = ecb;
                        if(*cptr == '-') {
                           if(match(cptr,"-CBC")) {
                              cipinfo->blockmode = cbc;
                           } else if(match(cptr,"-ECB")) {
                              cipinfo->blockmode = ecb;
                           } else {
                              sprintf(err_msg_text,"Unrecognized DEK type: %s.",cptr+1);
                              return(err_msg_text);
                           }

                           *cptr = '\0';
                        }
                        if(match(vals[0],DEK_ALG_DES)) {
                           cipinfo->algorithm = des;
                        } else if(match(vals[0],DEK_ALG_NEWDES)) {
                           cipinfo->algorithm = newdes;
                        } else {
                           sprintf(err_msg_text,"Unrecognized DEK type: %s.",vals[0]);
                           return(err_msg_text);
                        }

                        BlockInit(cipinfo);
                        if(cipinfo->blockmode == cbc) {
                           if(nvals<2) {
                              return("No init vector given for CBC mode.");
                           } else {
                              nbytes = HexToBin(vals[1],LINEBUFSIZE,init_vec);
                              if(nbytes != cipinfo->blocksize) {
                                 return("Bad size for init vector.");
                              }
                           }
                        }
                        break;

                     case SENDER:
                        break;

                     case SENDER_PUB_KEY:
                        break;

                     case MESSAGE_KEY:
                        /* Get the algorithm used to encrypt the message
                         * key, and get ready to get the encrypted message key.
                         */
                        if(this_user) {
                           this_user = FALSE;
                           got_msgkey = TRUE;
                           if(strcmp(vals[0],MESSAGE_KEY_RABIN)) {
                              sprintf(err_msg_text,"Unrecognized encryption type: %s",vals[0]);
                              return(err_msg_text);
                           } else if(!more) {
                              return("Missing comma after encryption type; missing message key?");
                           } else {
                              state = ST_Get_Message_Key;
                           }
                        }
                        break;
                  }
                  seen_field = TRUE;
               }
            }
            break;

         /* We are reading lines of the message key. */
         case ST_Get_Message_Key:
            if(!WhiteSpace(line[0])) {
               state = ST_Look_Field;
               repeat_state = TRUE;
            } else {
               cptr = line+1;
               while(WhiteSpace(*cptr) && *cptr) cptr++;
               if(*cptr) {
                  nbytes = prdecode(cptr,msg_key_bytes,max_key_bytes);
                  max_key_bytes -= nbytes;
                  msg_key_bytes += nbytes;
                  *num_key_bytes += nbytes;
               }
               if(debug) fprintf(stderr,"Read a line of the message key.\n");
            }
            break;
         }
      } while(repeat_state);
   }
   for(j=0; j<MAXVALS; j++) {
      free(vals[j]);
   }

   cptr = NULL;
   if(state == ST_Begin) {
      cptr = "Could not find Privacy Enhanced Message header";
   }
   if(cipinfo->algorithm != none && !got_msgkey) {
      cptr = "Can't find you in the message header.";
   }
   return cptr;
}

/*--- function CrackHeaderLine ---------------------------------------
 *
 *  Break a header line into its constituent components.
 *  The line is considered to consist of a field name,
 *  optionally followed by comma-separated values.
 *
 *  Entry:  line        is a line from the message header.
 *          maxvals     is the number of entries in the array "vals".
 *
 *  Exit:   field_name  is the field name, starting in the first column.
 *          vals        contains pointers to cracked-off comma-separated
 *                      values.  Leading and trailing spaces have
 *                      been trimmed.
 *          more        is TRUE iff the line ended in a comma, indicating
 *                      that additional values follow.
 *          Returns the number of values cracked.
 */
int
CrackHeaderLine(line,field_name,vals,maxvals,more)
char *line;
char *field_name;
char **vals;
int maxvals;
int *more;
{
   int nvals=0, leng;
   register char *cptr=line;
   char *cvalp;
   int got_comma = FALSE;
   
   for(leng=0; *cptr && !WhiteSpace(*cptr) && leng<LINEBUFSIZE-1; leng++) {
      *(field_name++) = *(cptr++);
   }
   *field_name = '\0';
   
   while(*cptr) {    
      while(*cptr && WhiteSpace(*(cptr))) cptr++;
      if(*cptr) {
         /* We have found a value.  Copy it into the array. */
         got_comma = FALSE;
         if(nvals < maxvals) nvals++;
         cvalp = vals[nvals-1];
         for(leng=0; *cptr && *cptr!=',' && *cptr!='\n' && 
          leng<LINEBUFSIZE-1; leng++) {
            *(cvalp++) = *(cptr++);
         }
         /* Trim trailing blanks. */
         while(leng > 0 && *(cvalp-1) == ' ') {
            cvalp--;
            leng--;
         }
         *cvalp = '\0';
         /* If we're not already at a comma, skip to the comma or EOL */
         while(*cptr && *cptr != ',' && *cptr != '\n') cptr++;
         if(*cptr == ',') {
            got_comma = TRUE;
            cptr++;
         }
      }
   }
   *more = got_comma;
   return (nvals);
}


/*--- function match ----------------------------------------------
 *
 *  Determine whether two character strings match.  Case insensitive.
 *
 *  Entry:  str      is a string.
 *          pattern  is a pattern to which we are comparing.
 *
 *  Exit:   Returns TRUE iff the strings match.
 */
int
match(str,pattern)
char *str;
char *pattern;
{
   char ch1, ch2;

   do {
      ch1 = (char) (islower(*str) ? toupper(*str) : *str);
      ch2 = (char) (islower(*pattern) ? toupper(*pattern) : *pattern);
      if(ch1 != ch2) return FALSE;
      str++; pattern++;
   } while(ch1 == ch2 && ch1 && ch2);

   if(!ch1 && !ch2) {
      return TRUE;
   } else {
      return FALSE;
   }
}

/*--- function tail ------------------------------------------------
 *
 *  Returns the trailing N characters of a string.
 *
 *  Entry:  str      is a null-terminated string.
 *          num      is the desired length of the tail.
 *
 *  Exit:   Returns a pointer to the tail, or to the beginning
 *            of the string if the string is less than N characters long.
 */
char *
tail(str,num)
char *str;
int num;
{
   int mylen = strlen(str);

   if(mylen < num) {
      return(str);
   } else {
      return(str+mylen-num);
   }
}


/*--- function WhiteSpace -------------------------------------------
 *
 *  Determine whether a character is "white space".
 *
 *  Entry:  ch    is a character.
 *
 *  Exit:   Returns TRUE iff the character is "white space".
 */
int
WhiteSpace(ch)
char ch;
{
   return(ch==' ' || ch=='\t' || ch=='\n');
} 
 
