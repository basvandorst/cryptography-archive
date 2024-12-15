/* Defines main for the RIPEM command line demo.
   Calls RIPEM "library" functions like DoEncipher.
 */

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#ifdef SVRV32
#include <sys/types.h>
#endif /* SVRV32 */
#include "global.h"
#include "rsaref.h"
#include "r_random.h"                              /* to get R_GenerateBytes */
#include "ripem.h"

#ifdef MSDOS
#include <io.h>
#include <time.h>
#ifndef __TURBOC__
#include <malloc.h>
#else
#include <alloc.h>
#endif
#endif

#ifndef IBMRT
#include <stdlib.h>
#endif
#include <errno.h>

#if !defined (__convexc__) && !defined(apollo) && !defined(__TURBOC__)
#include <memory.h>
#endif

#include <string.h>

#include "usagepro.h"
#include "getsyspr.h"
#include "parsitpr.h"
#include "getoptpr.h"

typedef struct struct_user {
  char *emailaddr;                                          /* Email address */
  BOOL gotpubkey;                     /* =TRUE if we have his/her public key */
} TypUser;

typedef struct {
  BOOL addRecip, includeHeaders, prependHeaders, abortIfRecipUnknown,
    myselfAsRecip, useRndMessage;
} RIPEMFlags;

#define MAX_AVA_TYPE 17
static char *AVA_TYPES[MAX_AVA_TYPE + 1] = {
  "unknownType", "type1", "type2", "CN", "type4", "type5", "C", "L", "ST",
  "SA", "O", "OU", "T", "type13", "type14", "type15", "type16", "PC"
};

#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

#define INPUT_BUFFER_SIZE 1024

static void MainEnd P((int stat));
static char *InitMain P((void));
static char *CrackCmd P((RIPEMFlags *, int, char **));
static char *CrackKeyServer P((char *));
static void ShowParams P((RIPEMFlags *));
static char *InitHomeDir P((void));
static void DoRandom P ((RIPEMInfo *));
static unsigned int GetPasswordToPrivKey
  P ((BOOL, BOOL, unsigned char *, unsigned int));
static char *OpenFiles P((void));
static char *DoEncipherDriver
  P ((RIPEMInfo *, RIPEMFlags *, FILE *, FILE *, enum enhance_mode, int,
      TypList *, FILE *, RIPEMDatabase *));
static char *DoDecipherDriver
  P ((RIPEMInfo *, FILE *, FILE *, int, char *, unsigned int, BOOL, FILE *,
      RIPEMDatabase *));
static char *DoChangePWDriver
  P ((RIPEMInfo *, char *, BOOL, char *, unsigned int, FILE *,
      RIPEMDatabase *));
static char *DoGenerateKeysDriver
  P ((RIPEMInfo *, char *, char *, unsigned int, unsigned int, unsigned char *,
      unsigned int, FILE *));
static char *InitUser P((char *, TypUser **));
static BOOL CaseIgnoreEqual P((char *, char *));
static char *GetCertStatusString P((int));
static char *AddUniqueUserToList P((TypUser *, TypList *));
static void WritePrintableName P((FILE *, DistinguishedNameStruct *));
static char *PrintCertNameAndDigest P((unsigned char *, FILE *));
static void SetNewUserDN P((RIPEMInfo *));
static void WritePrependedHeaders P((TypList *, FILE *));
static char *ReadPlaintextAlloc P((unsigned char **, unsigned int *));

#ifndef RIPEMSIG
static char *GetRecipientKeys
  P ((RIPEMInfo *, RecipientKeyInfo *, unsigned int *, BOOL, FILE *,
      RIPEMDatabase *));
static BOOL CheckKeyList P((TypList *, int));
#endif

#ifdef __BORLANDC__
extern unsigned _stklen = 12192;  /* Increase stack size for Borland C */
#endif

extern char *usage_msg[];

char author[] = "Mark Riordan  1100 Parker  Lansing MI  48912";
char author2[] = 
  "mrr@scss3.cl.msu.edu or riordanmr@clvax1.cl.msu.edu   Sept 1992";

int Argc;
char **Argv;
FILE *InStream = NULL, *OutStream = NULL;
FILE *PubOutStream = NULL, *PrivOutStream = NULL;
TypList RecipientList, UserList;
char *PubKeyOutFileName = NULL, *PrivKeyOutFileName = NULL, *InFileName = NULL,
  *OutFileName = NULL, *RandomFileName = NULL, *DebugFileName = NULL;
RIPEMDatabase RipemDatabase; 
int  RandomCmdIndex=0;
BOOL UseRndCmdLine=FALSE, UseRndFile=FALSE, UseRndKeyboard=FALSE;
BOOL UseRndSystem=FALSE;
int RandomStructInitialized = 0;
R_RANDOM_STRUCT *RandomStructPointer;                /* used by GetUserInput */
char *HomeDir = NULL;
int Bits=0;
int EncryptionAlgorithm=EA_DES_CBC;
enum enum_action {ACT_NOT_SPEC, ACT_ENCRYPT, ACT_DECRYPT, ACT_GEN_KEY,
 ACT_CHANGE_PW} Action=ACT_NOT_SPEC;
enum enhance_mode EnhanceMode = MODE_ENCRYPTED;
BOOL GotValidityMonths=FALSE;
int ValidityMonths = 24;
char *KeyToPrivKey = NULL, *NewKeyToPrivKey = NULL;
FILE *RandomStream = (FILE *)0;
char ErrMsgTxt[LINEBUFSIZE];
FILE *DebugStream = NULL;
FILE *CertInfoStream = NULL;
int Debug = 0;
static char *ERROR_MALLOC = "Cannot allocate memory";

#ifdef MACTC  /* rwo */
clock_t Time0, Time1;
#endif

#ifdef TESTDOS386
extern int ProcessorType(void);
int Got386 = 0;
#endif

int
main(argc,argv)
int argc;
char *argv[];
{
  RIPEMInfo ripemInfo;
  RIPEMFlags ripemFlags;
  BOOL needSelfSignedCert = FALSE;
  int j, status = 0;
  char *errorMessage, *cptr, password[MAX_PASSWORD_SIZE];
  unsigned int passwordLen;

  /* Set ripemInfo to initial state */
  RIPEMInfoConstructor (&ripemInfo);

#ifdef MACTC
  setvbuf(stderr, NULL, _IONBF, 0);
  fprintf(stderr, "Off we go...\n");
  argc = ccommand(&argv);
  Time0 = clock();
#endif

  Argc = argc; Argv = argv;
  InitMain();

  /* For error, break to end of do while (0) block. */
  do {
    /* Parse the command line. */
    errorMessage = CrackCmd (&ripemFlags, argc, argv);

    if(errorMessage) {
      usage(errorMessage,usage_msg);
      status = 4;
      break;
    }

    /* Open files. */
    errorMessage = OpenFiles();
    if(errorMessage) {
      fprintf(stderr,"%s\n",errorMessage);
      status = 3;
      break;
    }
    
    if (Debug > 1)
      ShowParams (&ripemFlags);

    /* Obtain "random" data from various sources and initialize
     * random structure with it.
     */
    if(Action != ACT_DECRYPT)
      DoRandom (&ripemInfo);

    /* Clear the parameters so that users typing "ps" or "w" can't
     * see what parameters we are using.
     */

    for(j=1; j<argc; j++) {
      cptr = argv[j];
      while(*cptr) *(cptr++) = '\0';
    }

    /* Get down to business and do the action requested of us. */
    ripemInfo.debugStream = DebugStream;
    ripemInfo.debug = Debug;
    ripemInfo.userList = &UserList;
  
    /* Get the password.  Promt twice if generating. */
    passwordLen = GetPasswordToPrivKey
      (Action == ACT_GEN_KEY, FALSE, password, sizeof (password));

    if (Action != ACT_GEN_KEY) {
      /* Need to log in for all actions other than key generation. */
      if ((errorMessage = LoginUser
           (&ripemInfo, &RipemDatabase, password, passwordLen))
          != (char *)NULL) {
        if (Action == ACT_CHANGE_PW &&
            !strcmp (errorMessage, ERR_SELF_SIGNED_CERT_NOT_FOUND))
          /* No self-signed cert, but we are trying to use change password
               to upgrade from RIPEM 1.1.  We will create a self-signed cert
               below. */
          needSelfSignedCert = TRUE;
        else {
          fputs(errorMessage,stderr);
          fputc('\n',stderr);
          status = 1;
          break;
        }
      }
    }

    if(Action == ACT_ENCRYPT) {
      errorMessage = DoEncipherDriver
        (&ripemInfo, &ripemFlags, InStream, OutStream, EnhanceMode,
         EncryptionAlgorithm, &RecipientList, CertInfoStream, &RipemDatabase);
    } else if(Action == ACT_DECRYPT){
      errorMessage = DoDecipherDriver
        (&ripemInfo, InStream, OutStream, GotValidityMonths, PubKeyOutFileName,
         ValidityMonths, ripemFlags.prependHeaders, CertInfoStream,
         &RipemDatabase);
    } else if(Action == ACT_CHANGE_PW) {
      errorMessage = DoChangePWDriver
        (&ripemInfo, PrivKeyOutFileName, needSelfSignedCert, PubKeyOutFileName,
         ValidityMonths, CertInfoStream, &RipemDatabase);
    } else {
      errorMessage = DoGenerateKeysDriver
        (&ripemInfo, PubKeyOutFileName, PrivKeyOutFileName, Bits,
         ValidityMonths, password, passwordLen, CertInfoStream);
    }

    if(errorMessage) {
      fputs(errorMessage,stderr);
      fputc('\n',stderr);
      status = 1;
      break;
    }
    status = ripemInfo.z.used_pub_key_in_message;
  } while (0);
      
  R_memset ((POINTER)password, 0, sizeof (password));
  if (KeyToPrivKey)
    R_memset ((POINTER)KeyToPrivKey, 0, strlen (KeyToPrivKey));
  if (NewKeyToPrivKey)
    R_memset ((POINTER)NewKeyToPrivKey, 0, strlen (NewKeyToPrivKey));
  RIPEMInfoDestructor (&ripemInfo);

  MainEnd (status);
  
  /*NOTREACHED*/
  return (0);  /* to shut up compiler warnings */
}

/*--- function MainEnd ------------------------------------------
 *
 *  End the program and return a returncode to the system.
 */
static void 
MainEnd(stat) 
int stat;
{
#ifdef MACTC
  double x;
  (void)fflush((FILE *)NULL);
  Time1 = clock();
  x = (Time1 - Time0)/60.0;
  fprintf(stderr, "Exit %d; Elapsed : %5.2f seconds.", stat, x);
#endif
  exit(stat);
}
  
/*--- function InitMain -----------------------------------------------
 *
 *  Do any necessary initialization before we really get going.
 *
 *       Entry: The program has just started.
 *
 *       Exit:  Some global variables have been initialized.
 */
static char *
InitMain()
{
  InitList(&(RipemDatabase.pubKeySource.filelist));
  InitList(&(RipemDatabase.privKeySource.filelist));
  InitList(&(RipemDatabase.privKeySource.serverlist));
  InitList(&(RipemDatabase.pubKeySource.serverlist));
  RipemDatabase.pubKeySource.origin[0] = KEY_FROM_SERVER;
  RipemDatabase.privKeySource.origin[0] = KEY_FROM_SERVER;
  RipemDatabase.pubKeySource.origin[1] = KEY_FROM_FILE;
  RipemDatabase.privKeySource.origin[1] = KEY_FROM_FILE;
  RipemDatabase.pubKeySource.origin[2] = KEY_FROM_FINGER;
  RipemDatabase.privKeySource.origin[2] = KEY_FROM_FINGER;
  
  /* PublicKey.bits = 0; */

#ifdef TESTDOS386
  Got386 = (ProcessorType() >= 3);
#endif

  return NULL;
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
 *    RipemDatabase.pubKeySource.filename
 *                   is the name of the public key file if encrypting.
 *    PrivFileSource.filename   is the name of the private key file.
 *    debug          is TRUE if debugging has been selected.
 *    infilename     is the name of the input file if specified.
 *                   Normally, standard input is used; this option was
 *                   implemented due to shortcomings in Microsoft's
 *                   Codeview, which was used during development.
 *    got_infilename is TRUE if the input file was specified explicitly.
 */
static char *CrackCmd (ripemFlags, argc, argv)
RIPEMFlags *ripemFlags;
int argc;
char *argv[];
{
  extern char *optarg;
  extern int optind, optsp;

  int got_action = FALSE;
  int got_username = FALSE;
  int got_key_to_priv_key = FALSE;
  BOOL cracking=TRUE;
  int j, ch, myargc[2], iarg;
  char *errorMessage = NULL, *cptr, **myargv[2], *env_args;
  char *random_sources = "efms";
  char *key_sources = "sf";
  char *header_opts = "i";
  char *recip_opts = "n";
  TypUser *recipient;
  char *usernameStr, *key_server_str=NULL;
  TypList mylist;
  TypFile *fptr;

  /* Preset flags to FALSE */
  R_memset ((POINTER)ripemFlags, 0, sizeof (*ripemFlags));

  /* We crack a command line twice:  
   * First, we crack the pseudo-command line in the environment variable
   *   RIPEM_ARGS (if any).  This environment variable exists to make it
   *   easy for users who don't want to type the otherwise lengthy
   *   RIPEM command line, and is an alternative to the other individual 
   *   environment variables.
   * Then we crack the real command line.  
   * Processing in this way causes the real command line variables
   * to supercede the environment variables.
   *
   * Start by allocating space for a copy of RIPEM_ARGS.  
   * We need to fill in the first token, the name of the program.
   */ 
  env_args = malloc(8);
  strcpy(env_args,"ripem ");
  GetEnvAlloc(RIPEM_ARGS_ENV, &cptr);
  if(cptr) {
    /* If the environment variable exists, call parsit to tokenize
     * the line as the shell normally does.
     */
    StrConcatRealloc(&env_args,cptr);
    myargv[0] = (char **)NULL;
    myargc[0] = parsit(env_args,&(myargv[0]));
    free(env_args);
  } else {
    /* No environment variable exists; just dummy this up. */
    myargv[0] = (char **)NULL;
    myargc[0] = 0;
  }
  myargv[1] = argv;
  myargc[1] = argc;
  /* Now execute the argument processing loop twice. */
  for(iarg=0; iarg<2; optind=1,optsp=1,cracking=TRUE,iarg++)  {
    while(cracking &&
          (ch = mygetopt
           (myargc[iarg],myargv[iarg],
            "3edgcr:h:b:A:R:p:s:P:S:m:u:k:K:i:o:D:F:Z:C:y:Y:T:v:H:")) != -1) {
      switch (ch) {
        /* Program modes */
        case 'd':    /* Decipher */
          Action = ACT_DECRYPT;
          got_action++;
          break;

        case 'e':       /* Encipher */
          Action = ACT_ENCRYPT;
          got_action++;
          break;

        case 'g':       /* Generate keypair */
          Action = ACT_GEN_KEY;
          got_action++;
          break;
          
        case 'c':   /* Change key to private key */
          Action = ACT_CHANGE_PW;
          got_action++;
          break;

        /* Names (email addresses) of users */
        case 'r':
          /* Store the name of another recipient.  */
          InitUser(optarg,&recipient);
          cptr = AddUniqueUserToList(recipient,&RecipientList);
          if(cptr) return cptr;
          break;
          
        case 'T':   /* Flags governing recipient processing */
          StrCopyAlloc(&recip_opts,optarg);
          break;

        case 'h':       /* Flags governing message headers */
          /* See processing of this string below.
           */
          StrCopyAlloc(&header_opts,optarg);
          break;

        case 'u':       /* My username */
          StrCopyAlloc(&usernameStr,optarg);
          got_username = TRUE;
          break;

        case '3':       /* short for -A des-ede-cbc */
          EncryptionAlgorithm = EA_DES_EDE2_CBC;
          break;

        case 'A':       /* symmetric cipher */
          EncryptionAlgorithm = -1;
          if (!strcmp(optarg, "des-cbc"))
            EncryptionAlgorithm = EA_DES_CBC;
          if (!strcmp(optarg, "des-ede-cbc"))
            EncryptionAlgorithm = EA_DES_EDE2_CBC;
          if (EncryptionAlgorithm < 0) {
            errorMessage = "Symmetric cipher must be either \"des-cbc\"\
              or \"des-ede-cbc\".";
          }
          break;

        case 'm':       /* Encryption mode */
          if (CaseIgnoreEqual (optarg, "encrypted"))
            EnhanceMode = MODE_ENCRYPTED;
          else if (CaseIgnoreEqual (optarg, "mic-only"))
            EnhanceMode = MODE_MIC_ONLY;
          else if (CaseIgnoreEqual (optarg, "mic-clear"))
            EnhanceMode = MODE_MIC_CLEAR;
          else
            errorMessage =
 "Processing mode must be one of \"encrypted\"  \"mic-only\" or \"mic-clear\".";

          break;

        case 'b':       /* Number of bits in generated key */
          Bits = atoi(optarg);
          if(Bits < MIN_RSA_MODULUS_BITS || Bits > MAX_RSA_MODULUS_BITS) {
            sprintf(ErrMsgTxt,"Number of bits must be %d <= bits <= %d",
                    MIN_RSA_MODULUS_BITS,MAX_RSA_MODULUS_BITS);
            errorMessage = ErrMsgTxt;
          }
          break;

        case 'v':                 /* Number of months to validate sender for */
          ValidityMonths = atoi (optarg);
          if (ValidityMonths <= 0)
            errorMessage = "Validity months must be > 0";
          else
            GotValidityMonths = TRUE;
          break;

        case 'p':       /* Public key filename */
          fptr = (TypFile *)malloc(sizeof(TypFile));
          fptr->stream = NULL;
          StrCopyAlloc(&(fptr->filename),optarg);
          AddToList
            (NULL,fptr,sizeof(TypFile),&RipemDatabase.pubKeySource.filelist);
          break;
          
        case 'P':       /* Public key output filename */
          StrCopyAlloc(&PubKeyOutFileName,optarg);
          break;

        case 's':       /* Secret (private) key filename */
          fptr = (TypFile *)malloc(sizeof(TypFile));
          fptr->stream = NULL;
          StrCopyAlloc(&(fptr->filename),optarg);
          AddToList
            (NULL,fptr,sizeof(TypFile),&RipemDatabase.privKeySource.filelist);
          break;
          
        case 'S':       /* Private key output filename */
          StrCopyAlloc(&PrivKeyOutFileName,optarg);
          break;

        case 'y':       /* Name of public key server */
          StrCopyAlloc(&key_server_str,optarg);
          break;

        case 'Y':       /* Order of sources for keys (server vs. file) */
          StrCopyAlloc(&key_sources,optarg);
          break;

        case 'k':       /* Key to private key */
          StrCopyAlloc((char **)&KeyToPrivKey,optarg);
          got_key_to_priv_key = TRUE;
          break;

        case 'K':       /* New key to private key for changing password */
          StrCopyAlloc((char **)&NewKeyToPrivKey,optarg);
          break;

        case 'H':       /* RIPEM home directory */
          StrCopyAlloc ((char **)&HomeDir, optarg);
          break;

        case 'i':       /* Input file */
          StrCopyAlloc(&InFileName,optarg);
          break;

        case 'o':       /* Output file */
          StrCopyAlloc(&OutFileName,optarg);
          break;

        case 'D':       /* Debug level */
          Debug = atoi(optarg);
          break;

        case 'Z':       /* Debug output file */
          StrCopyAlloc(&DebugFileName,optarg);
          break;

        case 'F':       /* Random input file */
          StrCopyAlloc(&RandomFileName,optarg);
          break;

        case 'R':       /* Sources of random data */
          StrCopyAlloc(&random_sources,optarg);
          break;

        case 'C':       /* Random command args */
          RandomCmdIndex = optind-1;
          cracking = FALSE;
          break;
      }
    }
  }

  /* Parse the -R argument string (sources of random info) */

  for(cptr=random_sources; *cptr; cptr++) {
    switch(*cptr) {
      case 'c':
        UseRndCmdLine = TRUE;
        break;
      case 'e':
        UseRndCmdLine = TRUE;
        RandomCmdIndex = 0;
        break;
      case 'f':
        UseRndFile = TRUE;
        break;
      case 'k':
        UseRndKeyboard = TRUE;
        break;
      case 'm':
        ripemFlags->useRndMessage = TRUE;
        break;
      case 's':
        UseRndSystem = TRUE;
        break;
      default:
        errorMessage = "-R option should be one or more of \"cefks\"";
        break;
    }
  }

  /* Parse the -Y argument string (sources of key info) */

  for(j=0; j<MAX_KEY_SOURCES; j++) {
    switch(key_sources[j]) {
      case 's':
      case 'S':
        RipemDatabase.pubKeySource.origin[j] = KEY_FROM_SERVER;
        break;

      case 'f':
      case 'F':
        RipemDatabase.pubKeySource.origin[j] = KEY_FROM_FILE;
        break;
        
      case 'g':
      case 'G':
        RipemDatabase.pubKeySource.origin[j] = KEY_FROM_FINGER;
        break;
    
      default:
        RipemDatabase.pubKeySource.origin[j] = KEY_FROM_NONE;
        break;
    }
  }
  
  /* Parse the -h option (how to process plaintext message headers) */
  
  for(cptr=header_opts; *cptr; cptr++) {
    switch(*cptr) {
      case 'r':
        ripemFlags->addRecip = TRUE;
        break;
      
      case 'i':
        ripemFlags->includeHeaders = TRUE;
        break;
        
      case 'p':
        ripemFlags->prependHeaders = TRUE;
        break;
        
      default:
        errorMessage = "-h option should be one or more of \"ipr\"";
        break;
    }
  }
  
  /* Parse the -T option (options for recipients) */
  
  for(cptr=recip_opts; *cptr; cptr++) {
    switch(*cptr) {
      case 'm':     /* Send a copy to myself */
        ripemFlags->myselfAsRecip = TRUE;
        break;
      
      case 'a':     /* Always abort if I can't find key for user */
        ripemFlags->abortIfRecipUnknown = TRUE;
        break;
        
      case 'n':     /* None of the above */
        ripemFlags->myselfAsRecip = FALSE;
        ripemFlags->abortIfRecipUnknown = FALSE;
        break;
        
      default:
        errorMessage = "-T option should be one or more of \"amn\"";
        break;
    }
  }

  if (!HomeDir) {
    GetEnvFileName (HOME_DIR_ENV, "", &HomeDir);
    if (*HomeDir == '\0')
      /* GetEnvFileName returned the "" */
      HomeDir = NULL;
  }

  /* Add the public key file, etc. from the RIPEM home dir */
  if ((errorMessage = InitHomeDir ()) != (char *)NULL)
    return (errorMessage);

  /* Check for syntax error. */

  if(got_action != 1) {
    errorMessage = "Must specify one of -e, -d, -g, -c";
  } else if(Action==ACT_ENCRYPT && EnhanceMode==MODE_ENCRYPTED) {
    if(!RecipientList.firstptr && !ripemFlags->addRecip) {
      errorMessage = "Must specify recipient(s) when enciphering.";
    }
  } else if(Action != ACT_ENCRYPT && RecipientList.firstptr != NULL) {
    errorMessage = "-r should be specified only when enciphering.";
  } else if(Action == ACT_ENCRYPT &&
            RipemDatabase.pubKeySource.origin[0] == KEY_FROM_NONE
            && RipemDatabase.pubKeySource.origin[1] == KEY_FROM_NONE) {
    errorMessage = "Must specify at least one source of public keys.";
  } else if(Action == ACT_GEN_KEY && (!PubKeyOutFileName || 
                                      !PrivKeyOutFileName)) {
    errorMessage = "Must specify public and private (-P, -S) key output files.";
  } else if(Action == ACT_CHANGE_PW && !PrivKeyOutFileName) {
    errorMessage = "Must specify private key output file (-S).";
  }
  
  if(errorMessage) return errorMessage;

  /* Obtain the username if it wasn't specified. */

  if(!got_username) {
    GetUserAddress(&usernameStr);
  }
  
  /* Crack the username string (which can contain multiple aliases
   * separated by commas) into a list.
   */
  
  CrackLine(usernameStr,&UserList);


  /* Include the sender as cryptorecipient if specified and if
   * encrypting.
   */
  if (ripemFlags->myselfAsRecip && Action==ACT_ENCRYPT && 
      EnhanceMode==MODE_ENCRYPTED) {
    InitUser ((char *)UserList.firstptr->dataptr,&recipient);
    cptr = AddUniqueUserToList(recipient,&RecipientList);
    if(cptr)return cptr;
  }
  
  /* Obtain the name of the public key server. */
  if(!key_server_str) {
    GetEnvAlloc(SERVER_NAME_ENV,&key_server_str);
  }
  errorMessage = CrackKeyServer(key_server_str);
  if(errorMessage) return errorMessage;

  /* Obtain the names of the files containing the private keys.
   */

  if(!RipemDatabase.privKeySource.filelist.firstptr) {
    GetEnvFileName(PRIVATE_KEY_FILE_ENV,PRIVATE_KEY_FILE_DEFAULT,
                   &cptr);
    CrackLine(cptr,&mylist);
    free(cptr);
    FORLIST(&mylist);
    fptr = (TypFile *)malloc(sizeof(TypFile));
    fptr->stream = NULL;
    StrCopyAlloc(&(fptr->filename),dptr);
    ExpandFilename(&(fptr->filename));
    AddToList
      (NULL,fptr,sizeof(TypFile),&RipemDatabase.privKeySource.filelist);
    ENDFORLIST;
  }

  /* Obtain the names of the files containing
   * the public keys.
   */
  if(!RipemDatabase.pubKeySource.filelist.firstptr) {
    GetEnvFileName(PUBLIC_KEY_FILE_ENV,PUBLIC_KEY_FILE_DEFAULT,
                   &cptr);
    CrackLine(cptr,&mylist);
    free(cptr);
    FORLIST(&mylist);
      fptr = (TypFile *)malloc(sizeof(TypFile));
      fptr->stream = NULL;
      StrCopyAlloc(&(fptr->filename),dptr);
      ExpandFilename(&(fptr->filename));
      AddToList
        (NULL,fptr,sizeof(TypFile),&RipemDatabase.pubKeySource.filelist);
    ENDFORLIST;
  }

  /* Obtain the name of the file containing random data. */
  if(UseRndFile && !RandomFileName) {
    GetEnvFileName(RANDOM_FILE_ENV,"",&RandomFileName);
    if(strlen(RandomFileName)==0) RandomFileName=NULL;
  }
  
  /* Special processing for the key to the private key:
   * A key of - means to read the key to the private key
   * from standard input.
   */
  if(got_key_to_priv_key) {
    if(strcmp(KeyToPrivKey,"-")==0) {
#define PWLEN 256
      char line[PWLEN];

      fgets(line,PWLEN,stdin);
      StrCopyAlloc((char **)&KeyToPrivKey,line);
      for(cptr=KeyToPrivKey; *cptr; cptr++) {
        if(*cptr=='\n' || *cptr=='\r') *cptr='\0';
      }
    }
  }
  return(errorMessage);
}

/*--- function CrackKeyServer ----------------------------------------
 * 
 *  Function to help CrackCmd parse the list of key server names.
 *  The list is specified as a string (either in the -y option or
 *  in the RIPEM_KEY_SERVER env variable) that looks like:
 *
 *     domain_name[:port_num][,domain_name2[:port_num2]...
 *
 *  Entry:  keyServerStr  is a zero-terminated string that contains
 *                  one or more key server names as above,
 *                  or NULL.
 *
 *  Exit: RipemDatabase.pubKeySource  contains the cracked information.
 */
static char *
CrackKeyServer(keyServerStr)
char *keyServerStr;
{
  TypList name_list;
  TypListEntry *entry;
  TypServer *server_ent;
  char *cptr, *errmsg;
  
  InitList(&(RipemDatabase.pubKeySource.serverlist));
  
  if(keyServerStr) {
    CrackLine(keyServerStr,&name_list);
    for(entry=name_list.firstptr; entry; entry=entry->nextptr) {
      server_ent = (TypServer *) malloc(sizeof(TypServer));
      
      server_ent->servername = entry->dataptr;
      server_ent->serverport = 0;
      cptr = strchr(server_ent->servername,':');
      if(cptr) {
        server_ent->serverport = atoi(cptr+1);
        if(!server_ent->serverport) {
          return "Invalid server port number";
        }
        *cptr = '\0';
      } else {
        server_ent->serverport = SERVER_PORT;
      }
      errmsg = AddToList(NULL,server_ent,sizeof(TypServer),
       &(RipemDatabase.pubKeySource.serverlist));
      if(errmsg) return errmsg;
    }
  }
  return NULL;    
}

/*--- function ShowParams -------------------------------------
 *
 *  Display the values of various user-supplied options,
 *  defaults, filenames, etc., for debugging purposes.
 */
static void ShowParams (ripemFlags)
RIPEMFlags *ripemFlags;
{
  char *cptr;
  char *not_present = "<none>";
  int j;
  TypListEntry *entry;
#define IFTHERE(str) (str ? str : not_present)

  fprintf(DebugStream,"%s\n",usage_msg[0]);
  fprintf(DebugStream,"Action=");
  switch(Action) {
    case ACT_NOT_SPEC:
      cptr = "<none>";
      break;
    case ACT_ENCRYPT:
      cptr = "Encrypt";
      break;
    case ACT_DECRYPT:
      cptr = "Decrypt";
      break;
    case ACT_GEN_KEY:
      cptr = "Generate";
      break;
    case ACT_CHANGE_PW:
      cptr = "Change PW";
      break;
  }
  fprintf(DebugStream,"%s  ",cptr);
  fprintf(DebugStream,"Recipients=");
  fprintf(DebugStream,"\n");
  /* Username is the first of the aliases */
  fprintf(DebugStream,"Your Username=%s  KeyToPrivKey=%s\n",
          UserList.firstptr ? (char *)UserList.firstptr->dataptr : not_present,
          IFTHERE(KeyToPrivKey));
  fprintf(DebugStream,"List of aliases to your username: \n");
  for(entry=UserList.firstptr; entry; entry=entry->nextptr) {
    fprintf(DebugStream,"   %s\n",(char *)entry->dataptr);
  }
  
  if(Action==ACT_GEN_KEY) {
    fprintf(DebugStream,"Bits in gen key=%d  ",Bits);
  }
  if(Action==ACT_ENCRYPT) {
      fprintf(DebugStream,"Proc mode=\"%s\"", EnhanceMode == MODE_ENCRYPTED ?
              "encrypted" : (EnhanceMode == MODE_MIC_ONLY ?
                             "mic-only" : "mic-clear"));
  }
  fprintf(DebugStream,"\n");

  fprintf(DebugStream,"Input=%s Output=%s\n",
          InFileName ? InFileName : "<stdin>",
          OutFileName ? OutFileName : "<stdout>");
  fprintf(DebugStream,"PubKeyFiles=");
  FORLIST(&RipemDatabase.pubKeySource.filelist);
    fprintf(DebugStream,"%s ",((TypFile *)dptr)->filename);
  ENDFORLIST;
  fprintf(DebugStream,"\n");
  fprintf(DebugStream,"PrivKeyFiles=");
  FORLIST(&RipemDatabase.privKeySource.filelist);
    fprintf(DebugStream,"%s ",((TypFile *)dptr)->filename);
  ENDFORLIST;
  fprintf(DebugStream,"\n");
  fprintf(DebugStream,"Home directory=%s\n",HomeDir ? HomeDir : "(None)");
  fprintf(DebugStream,"Sources of \"random\" data: ");
  if(UseRndCmdLine) {
    fprintf(DebugStream,"Command line, args %d-%d;\n ",
            RandomCmdIndex,Argc);
  }
  if(UseRndFile) fprintf(DebugStream,"File \"%s\"; ",IFTHERE(RandomFileName));
  if(UseRndKeyboard) fprintf(DebugStream,"Keyboard; ");
  if(ripemFlags->useRndMessage) fprintf(DebugStream,"Message; ");
  if(UseRndSystem) fprintf(DebugStream,"running System.");
  fprintf(DebugStream,"\n");

  if(UseRndCmdLine) {
    fprintf(DebugStream,"Random command-line arguments: ");
    for(j=RandomCmdIndex; j<Argc; j++) {
      fprintf(DebugStream,"%s ",Argv[j]);
    }
    fprintf(DebugStream,"\n");
  }

  fprintf(DebugStream, "Public key servers:\n");
  { TypServer *server_ent;
    TypListEntry *entry;
    
    for(entry=RipemDatabase.pubKeySource.serverlist.firstptr; entry; 
        entry=entry->nextptr) {
      server_ent = (TypServer *) entry->dataptr;
      if(server_ent->servername)
        fprintf(DebugStream,"   %s port %d\n",server_ent->servername,
                server_ent->serverport);
    }
  }
  fprintf(DebugStream,"Public key key sources (in order) = ");
  for(j=0; j<MAX_KEY_SOURCES; j++) {
    switch(RipemDatabase.pubKeySource.origin[j]) {
      case KEY_FROM_FILE:
        fprintf(DebugStream,"file ");
        break;
      case KEY_FROM_SERVER:
        fprintf(DebugStream,"server ");
        break;
      case KEY_FROM_FINGER:
        fprintf(DebugStream,"finger ");
        break;
      default:
        fprintf(DebugStream,"UNKNOWN");
        break;
    }
  }
  putc('\n',DebugStream);

  {
    R_RSA_PRIVATE_KEY privKey;
    
    fprintf(DebugStream,"sizeof PrivateKey=%d ",sizeof(privKey));
    fprintf(DebugStream,"sizeof components = %d %d %d %d %d %d %d %d %d \n",
            sizeof(privKey.bits),sizeof(privKey.modulus),
            sizeof(privKey.publicExponent),
            sizeof(privKey.exponent),sizeof(privKey.prime[0]),
            sizeof(privKey.prime[1]),
            sizeof(privKey.primeExponent[0]),
            sizeof(privKey.primeExponent[1]),
            sizeof(privKey.coefficient));
  }
}

/* Add the public and private key files in the homedir to the front
     of the lists.
   If the private key file is not in the homedir try to create one and if
     successful set newPrivateKeyFile.  This way, the calling routine
     can copy the private key in from elsewhere.
   If HomeDir is NULL, this does nothing.
 */
static char *InitHomeDir ()
{
  FILE *fp = (FILE *)NULL;
  TypFile *typFile;
  char *path = (char *)NULL, *errorMessage = (char *)NULL;

  if (!HomeDir)
    return ((char *)NULL);

  /* Try to make sure there is the correct separator between the
       directory and the file name.  E.g. on UNIX, ensure an ending /
     If we don't recognize the machine type, then just hope the user
       already put the right separator.
     Assume only one of UNIX, MSDOS, MACTC, etc. are set.
   */
  if (*HomeDir != '\0') {
#ifdef UNIX
    if (HomeDir[strlen (HomeDir) - 1] != '/')
      StrConcatRealloc (&HomeDir, "/");
#endif
  
#ifdef MSDOS
    if (HomeDir[strlen (HomeDir) - 1] != '\\' &&
        HomeDir[strlen (HomeDir) - 1] != ':')
      StrConcatRealloc (&HomeDir, "\\");  
#endif

#ifdef MACTC
    if (HomeDir[strlen (HomeDir) - 1] != ':')
      StrConcatRealloc (&HomeDir, ":");  
#endif
    
    if (!HomeDir)
      /* Error in reallocating the string */
      return (ERROR_MALLOC);
  }

  do {
    if ((path = malloc (strlen (HomeDir) + 9)) == (char *)NULL) {
      errorMessage = (ERROR_MALLOC);
      break;
    }

    /* Check for the existence of the privkey file by trying to open it
         for read.
       If it can't be opened for read, open it for write to make sure
         we can create it.
     */
    strcpy (path, HomeDir);
    strcat (path, "privkey");
    if ((fp = fopen (path, "r")) == (FILE *)NULL) {
      /* Cannot open for read, try to open for write */
      if ((fp = fopen (path, "w")) == (FILE *)NULL) {
        sprintf (ErrMsgTxt,
                 "Can't write to directory %s. (Does it exist?)", HomeDir);
        errorMessage = ErrMsgTxt;
        break;
      }

      fclose (fp);
      fp = (FILE *)NULL;
    }
    else {
      /* Successfully opened file for read.  Close it and prepend to the list.
       */
      fclose (fp);
      fp = (FILE *)NULL;

      typFile = (TypFile *)malloc (sizeof (TypFile));
      typFile->stream = NULL;
      StrCopyAlloc (&typFile->filename, path);
      PrependToList
        (typFile, sizeof (TypFile), &RipemDatabase.privKeySource.filelist);

      if (Action == ACT_GEN_KEY && !PrivKeyOutFileName)
        /* Private key in home dir already exists and we are going to
             overwrite it in keygen, so give warning */
        fprintf
          (stderr,"WARNING: key generation will replace existing file %s\n",
           path);
    }

    if (!PrivKeyOutFileName)
      /* User did not specify -S file, so set to file in home dir */
      StrCopyAlloc (&PrivKeyOutFileName, path);

    /* Try to open the public key file for read.  If successful, add to list.
     */
    strcpy (path, HomeDir);
    strcat (path, "pubkeys");
    if ((fp = fopen (path, "r")) != (FILE *)NULL) {
      fclose (fp);
      fp = (FILE *)NULL;

      typFile = (TypFile *)malloc (sizeof (TypFile));
      typFile->stream = NULL;
      StrCopyAlloc (&typFile->filename, path);
      PrependToList
        (typFile, sizeof (TypFile), &RipemDatabase.pubKeySource.filelist);

      if (Action == ACT_GEN_KEY && !PubKeyOutFileName)
        /* Public key in home dir already exists and we are going to
             overwrite it in keygen, so give warning */
        fprintf
          (stderr, "WARNING: key generation will replace existing file %s\n",
           path);
    }

    if (!PubKeyOutFileName)
      /* User did not specify -P file, so set to file in home dir */
      StrCopyAlloc (&PubKeyOutFileName, path);

    if (!RandomFileName) {
      /* Random file has not been specified on the command line, so
          try to random input file in home dir for read. If successful, use it.
       */
      strcpy (path, HomeDir);
      strcat (path, "randomin");
      if ((fp = fopen (path, "r")) != (FILE *)NULL) {
        fclose (fp);
        fp = (FILE *)NULL;
        StrCopyAlloc (&RandomFileName, path);
      }
    }
  } while (0);

  free (path);
  if (fp != (FILE *)NULL)
    fclose (fp);

  return (errorMessage);
}

/*--- function DoRandom ---------------------------------------
 *
 *  Assemble pseudo-random data from various locations and
 *  feed it into a R_RANDOM_STRUCT structure.
 *
 *  Entry: UseRndCmdLine     \
 *         UseRndFile         \
 *         UseRndKeyboard     / These tell which sources to use
 *         UseRndSystem      /  for random data
 *         RandomCmdIndex       "argv" index at which to start, if
 *                              using command line params as random.
 *          RandomStream        Stream pointer to random file, if any.
 *
 *   Exit:  RandomStruct    contains the init'ed random struct.
 */
static void
DoRandom (ripemInfo)
RIPEMInfo *ripemInfo;
{
#define RANBUFSIZE 1024
  unsigned char ranbuf[RANBUFSIZE];
  unsigned char timebuf[RANBUFSIZE];
  int nbytes, ntimebytes, jarg, totbytes=0, getting_random=TRUE;

  R_memset(ranbuf,0,RANBUFSIZE);

  /* RandomStruct is already initialized.
     Set up global variables accessed by GetUserInput */
  RandomStructInitialized = 1;
  RandomStructPointer = &ripemInfo->randomStruct;

  /* Because we use the random struct during the
    * process of obtaining random data, we seed it first
    * to avoid RE_NEED_RANDOM errors.
    */
  while(getting_random) {
    unsigned int nbytes_needed;

    R_GetRandomBytesNeeded(&nbytes_needed,&ripemInfo->randomStruct);
    if(nbytes_needed) {
      R_RandomUpdate(&ripemInfo->randomStruct,ranbuf,256);
    } else {
      getting_random = FALSE;
    }
  }

  /* If requested, obtain random info from the running system. */
  if(UseRndSystem) {
    nbytes = GetRandomBytes(ranbuf,RANBUFSIZE);
    R_RandomUpdate(&ripemInfo->randomStruct,ranbuf,nbytes);
    totbytes += nbytes;
  }

  /* If requested, obtain random info from the user at the
    * keyboard.
    */
  if(UseRndKeyboard) {
    fprintf(stderr,"Enter random string: ");
    nbytes = ntimebytes = RANBUFSIZE;
    GetUserInput(ranbuf,&nbytes,timebuf,&ntimebytes,TRUE);
    R_RandomUpdate(&ripemInfo->randomStruct,ranbuf,nbytes);
    R_RandomUpdate(&ripemInfo->randomStruct,timebuf,ntimebytes);
    totbytes += nbytes+ntimebytes;
  }

  /* If requested, obtain random info from the command line
    * arguments.
    */
  if(UseRndCmdLine) {
    for(jarg=RandomCmdIndex; jarg<Argc; jarg++) {
      nbytes = strlen(Argv[jarg]);
      R_RandomUpdate
        (&ripemInfo->randomStruct,(unsigned char *)Argv[jarg], nbytes);
      totbytes += nbytes;
    }
  }

  /* If requested & available, read random information from
   * randomly-selected spots on the "random" file.
   */
  if(UseRndFile && RandomStream) {
    long int filesize, myoffset;
    int iterations;

    /* Find the size of the file by seeking to the end
     * and then finding out where we are.
     */
    fseek(RandomStream,0L,2);  /* seek to end of file */
    filesize = ftell(RandomStream);

    /* Figure out how many blocks to read. Do this by
     * computing a pseudo-random number from the information
     * seeded so far.
     */

    R_GenerateBytes(ranbuf,1,&ripemInfo->randomStruct);
    iterations = 1 + (ranbuf[0] & 7);
    if(Debug>1) {
      fprintf(DebugStream,"Random file: seeking to byte ");
    }

    while(iterations--) {
      R_GenerateBytes
        ((unsigned char *)&myoffset,sizeof(myoffset), &ripemInfo->randomStruct);
      if(myoffset<0) myoffset = (-myoffset);
      myoffset %= filesize;
      if(Debug>1) fprintf(DebugStream,"%ld ",myoffset);
      fseek(RandomStream,myoffset,0); /* seek to location */
      nbytes = fread(ranbuf,1,RANBUFSIZE,RandomStream);
      R_RandomUpdate(&ripemInfo->randomStruct,ranbuf,nbytes);
      totbytes += nbytes;
    }
    if(Debug>1) fprintf(DebugStream,"\n");
  }

  if(Debug>1) {
    fprintf(DebugStream,"%d bytes of pseudo-random data obtained.\n",
            totbytes);
  }
}

/*--- function GetPasswordToPrivKey ---------------------------------
 *
 *  Get the password to the encrypted private key.
 *
 *  Entry:  verify   is TRUE if we should prompt twice.
 *          new      is TRUE if it's a new password (change prompt)
 *          maxchars is the buffer size for password.
 *
 *  Exit:   password is the zero-terminated password.
 *
 *  Look for it in this order:
 *    Argument extracted from command line,
 *    value of environment variable,
 *    prompt user interactively.
 */
static unsigned int
GetPasswordToPrivKey (verify, new, password, maxchars)
BOOL verify;
BOOL new;
unsigned char *password;
unsigned int maxchars;
{
   unsigned int pw_len = 0;
   BOOL got_pw = FALSE;
   char *cptr;

  if (new) {
    if(NewKeyToPrivKey) {
      strncpy((char *)password,NewKeyToPrivKey,maxchars);
      pw_len = (unsigned int)strlen((char *)password);
      got_pw = TRUE;
    }
  }
  else {
   if(KeyToPrivKey) {
      strncpy((char *)password,KeyToPrivKey,maxchars);
      pw_len = (unsigned int)strlen((char *)password);
      got_pw = TRUE;
   }
  }

   if(!got_pw && !new) {
      GetEnvAlloc(KEY_TO_PRIVATE_KEY_ENV, &cptr);
      if(cptr && *cptr) {
         strncpy((char *)password,cptr,maxchars);
         pw_len = (unsigned int)strlen((char *)password);
         got_pw = TRUE;
      }
   }

   if(!got_pw) {
      if(new) {
         cptr = "Enter new password to private key: ";
      } else {
         cptr = "Enter password to private key: ";
      }
      pw_len = GetPasswordFromUser(cptr,verify,password,maxchars);
   }

   return pw_len;
}

/*--- function OpenFiles --------------------------------------
 *
 *  Open files for RIPEM.
 *
 *  Entry:
 *
 *  Exit:   InStream, OutStream, PubStream, PrivStream,
 *          RandomStream contain file pointers to the corresponding
 *          files (or streams), if there's no error.
 *
 *          Returns NULL if no error, else address of error string.
 */
static char *
OpenFiles()
{
  if(InFileName) {
    InStream = fopen(InFileName,"r");
    if(!InStream) {
      sprintf(ErrMsgTxt,"Can't open input file %s.",InFileName);
      return(ErrMsgTxt);
    }
  } else {
    InStream = stdin;
  }

  if(OutFileName) {
    OutStream = fopen(OutFileName,"w");
    if(!OutStream) {
      sprintf(ErrMsgTxt,"Can't open output file %s.",OutFileName);
      return(ErrMsgTxt);
    }
  } else {
    OutStream = stdout;
  }
  
  if(DebugFileName) {
    DebugStream = fopen(DebugFileName,"w");
    if(!DebugStream) {
      sprintf(ErrMsgTxt,"Can't open debug file %s.",DebugFileName);
      return(ErrMsgTxt);
    }
  } else {
    DebugStream = stderr;
  }
  CertInfoStream = DebugStream;

  if(Action != ACT_GEN_KEY ) {
    FORLIST(&RipemDatabase.privKeySource.filelist);
      ((TypFile*)dptr)->stream = 
        fopen(((TypFile*)dptr)->filename,"r");
      if(!((TypFile*)dptr)->stream) {
        sprintf(ErrMsgTxt,"Can't open private key file \"%s\".",
                ((TypFile*)dptr)->filename);
        return(ErrMsgTxt);
      }
    ENDFORLIST;
  }

  if(Action != ACT_GEN_KEY ) {
    FORLIST(&RipemDatabase.pubKeySource.filelist);  
      ((TypFile*)dptr)->stream = 
        fopen(((TypFile*)dptr)->filename,"r");
      if(Debug>1 && !((TypFile*)dptr)->stream ) {
        fprintf(DebugStream,"Warning:  can't open public key file %s\n",
                ((TypFile*)dptr)->filename);
      }
    ENDFORLIST;
  }

  if(Action != ACT_DECRYPT && UseRndFile) {
    if(RandomFileName) {
      RandomStream = fopen(RandomFileName,"r");
      if(!RandomStream) {
        sprintf(ErrMsgTxt,
                "Can't open random data file \"%s\".",RandomFileName);
        return(ErrMsgTxt);
      } else {
#ifdef MSDOS
#ifndef O_BINARY
#define O_BINARY _O_BINARY
#endif
#ifdef __GNUC__
        _setmode(fileno(RandomStream),O_BINARY);
#else
#ifdef __TURBOC__
#define _setmode setmode
#define _fileno fileno
#endif
        _setmode(_fileno(RandomStream),O_BINARY);
#endif
#endif
      }
    }
  }
  return NULL;
}

/* Do processing on recipient list and email headers.  Then call DoEncipher.
   recipientList is a list of TypUser.  If addRecip in ripemFlags is set, this
     modifies recipientList by adding recipients from the email header.
 */
static char *DoEncipherDriver
  (ripemInfo, ripemFlags, inStream, outStream, enhanceMode,
   encryptionAlgorithm, recipientList, certInfoStream, ripemDatabase)
RIPEMInfo *ripemInfo;
RIPEMFlags *ripemFlags;
FILE *inStream;
FILE *outStream;
enum enhance_mode enhanceMode;
int encryptionAlgorithm;
TypList *recipientList;
FILE *certInfoStream;
RIPEMDatabase *ripemDatabase;
{
  BOOL useHeaderList, useReadEmailHeader = FALSE;
  unsigned char *partOut, *plaintext = (unsigned char *)NULL;
  unsigned int partOutLen, plaintextLen;
  char *errorMessage, line[INPUT_BUFFER_SIZE];
  RecipientKeyInfo *recipientKeys = (RecipientKeyInfo *)NULL;
  unsigned int recipientKeyCount;
  TypList headerList, recipientNames;
#ifndef RIPEMSIG
  unsigned int usernameCount;
  TypListEntry *entry;
  TypUser *recipient;
#endif

#ifdef RIPEMSIG
UNUSED_ARG (recipientList)
UNUSED_ARG (certInfoStream)
UNUSED_ARG (ripemDatabase)
#endif
  
  /* Set to initial state so it is OK to free on error at any time. */
  InitList (&headerList);
  InitList (&recipientNames);

  /* For error, break to end of do while (0) block. */
  do {
    if (ripemFlags->includeHeaders && !ripemFlags->addRecip &&
        !ripemFlags->prependHeaders)
      /* includeHeaders is set, but we don't need to add recipients or
           prepend the headers.  This means DoEncipher should just read the
           whole text as is without interpreting any part of it as a header. */
      useHeaderList = FALSE;
    else {
      /* Process the email header.  Note that if none of includeHeaders,
           prependHeaders or addRecip is set, this has the effect of omitting
           the email header from the message. */
      if ((errorMessage = ReadEmailHeader
           (inStream,
            (ripemFlags->includeHeaders || ripemFlags->prependHeaders) ?
            &headerList : (TypList *)NULL,
            ripemFlags->addRecip ? &recipientNames : (TypList *)NULL))
          != (char *)NULL)
        break;
      useReadEmailHeader = TRUE;
      useHeaderList = ripemFlags->includeHeaders;
    }
#ifndef RIPEMSIG
    if (enhanceMode == MODE_ENCRYPTED) {
      if (ripemFlags->addRecip) {
        /* The recipients in the email header were put in recipientNames,
             so explicitly add them to recipentList now.
         */
        for (entry = recipientNames.firstptr; entry;
             entry = entry->nextptr) {
          if ((errorMessage = InitUser ((char *)entry->dataptr, &recipient))
              != (char *)NULL)
            break;
          if ((errorMessage = AddUniqueUserToList (recipient, recipientList))
              != (char *)NULL) {
            /* Free the recipient that we just allocated */
            free (recipient);
            break;
          }
        }
        if (errorMessage != (char *)NULL)
          break;
      }
      /* Print recipients to debug stream and also get count.
       */
      usernameCount = 0;
      if (ripemInfo->debug > 1)
        fprintf (ripemInfo->debugStream, "Recipients: ");
      for (entry = recipientList->firstptr; entry;
           entry = entry->nextptr) {
        recipient = (TypUser *)entry->dataptr;
        if (ripemInfo->debug > 1)
          fprintf(ripemInfo->debugStream,"%s,", recipient->emailaddr);
        ++usernameCount;
      }
      if (ripemInfo->debug > 1)
        fprintf
        (ripemInfo->debugStream, "%d Total Recipients\n", usernameCount);
      
      /* Allocate recipientKeys buffer. */
      if ((recipientKeys = (RecipientKeyInfo *)malloc
           (usernameCount * sizeof (*recipientKeys)))
          == (RecipientKeyInfo *)NULL) {
        errorMessage = ERROR_MALLOC;
        break;
      }
      if ((errorMessage = GetRecipientKeys
           (ripemInfo, recipientKeys, &recipientKeyCount,
            ripemFlags->abortIfRecipUnknown, certInfoStream, ripemDatabase))
          != (char *)NULL)
        break;
    }
#endif

    /* Write the email headers before the PEM message if requested. */
    if (ripemFlags->prependHeaders)
      WritePrependedHeaders (&headerList, outStream);

    if (inStream == stdin) {
      /* This is a special case since we can't rewind stdin.
         Read the entire remaining input into an allocated buffer and
           use this for enciphering. */
      if ((errorMessage = ReadPlaintextAlloc (&plaintext, &plaintextLen))
          != (char *)NULL)
        break;
    }

#ifndef RIPEMSIG
    /* If encrypting and useRndMessage is
         set, read some bytes from the message for random update.
       If reading from a file, remember to rewind.
     */
    if (enhanceMode == MODE_ENCRYPTED && ripemFlags->useRndMessage) {
      if (inStream == stdin)
        R_RandomUpdate (&ripemInfo->randomStruct, plaintext, plaintextLen);
      else {
        /* Don't bother about how many bytes fread returns, since we'll
             just feed the whole buffer to random update anyway. */
        fread (line, 1, sizeof (line), inStream);
        R_RandomUpdate (&ripemInfo->randomStruct, line, sizeof (line));

        /* Rewind and skip past the headers if necessary to reposition. */
        fseek (inStream, 0L, 0);
        if (useReadEmailHeader)
          ReadEmailHeader (inStream, (TypList *)NULL, (TypList *)NULL);
      }
    }
#endif
    
    /* Initialize the enhance process. */
#ifdef DOGETRUSAGE
    if (ripemInfo->debug > 1)
      ReportCPUTime ("Before RIPEMEncipherInit");
#endif
    if ((errorMessage = RIPEMEncipherInit
         (ripemInfo, enhanceMode, encryptionAlgorithm, recipientKeys,
          recipientKeyCount)) != (char *)NULL)
      break;
#ifdef DOGETRUSAGE
    if (ripemInfo->debug > 1)
      ReportCPUTime ("After RIPEMEncipherInit");
#endif

    /* Make first pass through input to digest the text.
     */
    if (useHeaderList) {
      /* Headers are included in the text, so digest them now.
       */
      for (entry = headerList.firstptr; entry; entry = entry->nextptr) {
        if ((errorMessage = RIPEMEncipherDigestUpdate
             (ripemInfo, (unsigned char *)entry->dataptr,
              strlen ((char *)entry->dataptr))) != (char *)NULL)
          break;

        if ((errorMessage = RIPEMEncipherDigestUpdate
             (ripemInfo, (unsigned char *)"\n", 1)) != (char *)NULL)
          break;
      }
      if (errorMessage != (char *)NULL)
        /* broke because of error */
        break;

      /* Add a blank line after the email header. */
      if ((errorMessage = RIPEMEncipherDigestUpdate
           (ripemInfo, (unsigned char *)"\n", 1)) != (char *)NULL)
        break;
    }

    /* Digest the input text.
     */
    if (inStream == stdin) {
      if ((errorMessage = RIPEMEncipherDigestUpdate
           (ripemInfo, plaintext, plaintextLen)) != (char *)NULL)
        break;
    }
    else {
      while (fgets (line, sizeof (line), inStream)) {
        /* Line already includes the '\n', so digest.
         */
        if ((errorMessage = RIPEMEncipherDigestUpdate
             (ripemInfo, (unsigned char *)line, strlen (line)))
            != (char *)NULL)
          break;
      }
      if (errorMessage != (char *)NULL)
        /* broke because of error */
        break;

      /* Rewind for second pass.  If we used ReadEmailHeader above, we
           must call it again to skip past the header.
       */
      fseek (inStream, 0L, 0);
      if (useReadEmailHeader)
        ReadEmailHeader (inStream, (TypList *)NULL, (TypList *)NULL);
    }
    
    if (useHeaderList) {
      /* Headers are included in the text, so we must encipher them first.
       */
      for (entry = headerList.firstptr; entry; entry = entry->nextptr) {
        if ((errorMessage = RIPEMEncipherUpdate
             (ripemInfo, &partOut, &partOutLen,
              (unsigned char *)entry->dataptr,
              strlen ((char *)entry->dataptr))) != (char *)NULL)
          break;
        fwrite (partOut, 1, partOutLen, outStream);

        if ((errorMessage = RIPEMEncipherUpdate
             (ripemInfo, &partOut, &partOutLen, (unsigned char *)"\n", 1))
            != (char *)NULL)
          break;
        fwrite (partOut, 1, partOutLen, outStream);
      }
      if (errorMessage != (char *)NULL)
        /* broke because of error */
        break;

      /* Encipher a blank line after the email header. */
      if ((errorMessage = RIPEMEncipherUpdate
           (ripemInfo, &partOut, &partOutLen, (unsigned char *)"\n", 1))
          != (char *)NULL)
        break;
      fwrite (partOut, 1, partOutLen, outStream);
    }

    /* Encipher the input text.
     */
    if (inStream == stdin) {
      if ((errorMessage = RIPEMEncipherUpdate
           (ripemInfo, &partOut, &partOutLen, plaintext, plaintextLen))
          != (char *)NULL)
        break;
      fwrite (partOut, 1, partOutLen, outStream);
    }
    else {
      while (fgets (line, sizeof (line), inStream)) {
        /* Line already includes the '\n'.
         */
        if ((errorMessage = RIPEMEncipherUpdate
             (ripemInfo, &partOut, &partOutLen, (unsigned char *)line,
              strlen (line))) != (char *)NULL)
          break;
        fwrite (partOut, 1, partOutLen, outStream);
      }
      if (errorMessage != (char *)NULL)
        /* broke because of error */
        break;
    }

    /* Finalize and flush the output.
     */
    if ((errorMessage = RIPEMEncipherFinal (ripemInfo, &partOut, &partOutLen))
        != (char *)NULL)
      break;
    fwrite (partOut, 1, partOutLen, outStream);
  } while (0);

  if (plaintext != (unsigned char *)NULL) {
    /* zeroize and free */
    R_memset ((POINTER)plaintext, 0, plaintextLen);
    free (plaintext);
  }
  R_memset ((POINTER)line, 0, sizeof (line));
  free (recipientKeys);
  FreeList (&headerList);
  FreeList (&recipientNames);
  return (errorMessage);
}

static char *DoDecipherDriver
  (ripemInfo, inStream, outStream, validate, pubKeyOutFilename,
   validityMonths, prependHeaders, certInfoStream, ripemDatabase)
RIPEMInfo *ripemInfo;
FILE *inStream;
FILE *outStream;
int validate;
char *pubKeyOutFilename;                             /* for validating certs */
unsigned int validityMonths;
BOOL prependHeaders;
FILE *certInfoStream;
RIPEMDatabase *ripemDatabase;
{
  TypList certChain;
  enum enhance_mode enhanceMode;
  char *errorMessage;
  CertificateStruct *certStruct = (CertificateStruct *)NULL;
  CertFieldPointers fieldPointers;
  FILE *pubOutStream = (FILE *)NULL;
  int chainStatus;
  unsigned char *partOut;
  unsigned int partOutLen;
  char lineIn[500];

  /* Set to initial state so it is OK to free on error at any time. */
  InitList (&certChain);

  do {
    /* Allocate the certStruct on the heap since it is so big. */
    if ((certStruct = (CertificateStruct *)malloc (sizeof (*certStruct)))
        == (CertificateStruct *)NULL) {
      errorMessage = ERROR_MALLOC;
      break;
    }

    if ((errorMessage = RIPEMDecipherInit (ripemInfo, prependHeaders))
        != (char *)NULL)
      break;
    
    while (fgets (lineIn, sizeof (lineIn), inStream)) {
      if ((errorMessage = RIPEMDecipherUpdate
           (ripemInfo, &partOut, &partOutLen, (unsigned char *)lineIn,
            strlen (lineIn), ripemDatabase)) != (char *)NULL)
        break;
      fwrite (partOut, 1, partOutLen, outStream);
    }
    if (errorMessage != (char *)NULL)
      /* Broke while loop because of error */
      break;
    if (!feof (inStream)) {
      errorMessage = "Error reading input stream";
      break;
    }
    
    if ((errorMessage = RIPEMDecipherFinal
         (ripemInfo, &certChain, &chainStatus, &enhanceMode)) != (char *)NULL)
      break;

    if (chainStatus == 0) {
      /* Check if there is an entry in the certChain which will be
           a self-signed certificate. */
      if (!certChain.firstptr) {
        errorMessage = "The sender has not been validated. Have them send a self-signed certificate.";
        break;
      }
      else {
        /* Decode the self-signed cert at the "bottom" of the chain.
           Note DERToCertificate won't return an error
             since it was already successfully decoded.
         */
        DERToCertificate
          ((unsigned char *)certChain.firstptr->dataptr, certStruct,
           &fieldPointers);

        if (validate && pubKeyOutFilename != (char *)NULL) {
          /* The user specified the validity months, so validate
             the sender's certificate.
           */
          if ((pubOutStream = fopen (pubKeyOutFilename, "a")) == NULL) {
            errorMessage = "Can't open public key output file";
            break;
          }

          if ((errorMessage = ValidateAndWriteCert
               (ripemInfo, certStruct, validityMonths, pubOutStream))
              != (char *)NULL)
            break;
          fflush (pubOutStream);

          fprintf (certInfoStream,
                   "This user has now been validated:\n");
          WritePrintableName (certInfoStream, &certStruct->subject);
          fprintf (certInfoStream,
                   "\nYou may now receive messages from this user.\n");
          break;
        }
        else {
          /* We are not supposed to validate the sender's public key,
             so just output a message giving the sender's name and
             self-signed digest.
           */
          if ((errorMessage = PrintCertNameAndDigest
               ((unsigned char *)certChain.firstptr->dataptr,
                certInfoStream)) != (char *)NULL)
            break;

          fprintf (certInfoStream,
                   "Contact sender to verify certificate digest.\n");
          errorMessage =
          "To validate sender, receive message again in validation mode (-v).";
          break;
        }
      }
    }
    else {
      /* Non-zero chain status, so write out the sender information.
       */
      fputs ("-------------------------\n", certInfoStream);
      fprintf (certInfoStream, "Received %s message.\n",
               enhanceMode == MODE_ENCRYPTED ?
               "signed and encrypted" : "signed");
      if (chainStatus == CERT_UNVALIDATED) {
        /* For unvalidated keys, the sender's name is the first entry in
             the "certChain". */
        fprintf (certInfoStream, "Sender username: %s\n",
                 (char *)certChain.firstptr->dataptr);
        fprintf (certInfoStream,
                 "Signature status: key found but not validated.\n");
      }
      else {
        /* Decode the cert at the "bottom" of the chain.
           Note DERToCertificate won't return an error
            since it was already successfully decoded.
         */
        DERToCertificate
          ((unsigned char *)certChain.firstptr->dataptr, certStruct,
           (CertFieldPointers *)NULL);
        fprintf (certInfoStream, "Sender name: ");
        WritePrintableName
          (certInfoStream, &certStruct->subject);
        fprintf (certInfoStream, "\n");

        fprintf
          (certInfoStream, "Signature status: %s.\n",
           GetCertStatusString (chainStatus));
      }
      fputs ("-------------------------\n", certInfoStream);
    }
  } while (0);

  if (pubOutStream != (FILE *)NULL)
    fclose (pubOutStream);
  free (certStruct);
  FreeList (&certChain);
  return (errorMessage);
}

/* If needSelfSignedCert is TRUE, call WriteSelfSignedCert with
     pubKeyOutFilename and validityMonths for upgrading from RIPEM 1.1.
     Otherwise ignore pubKeyOutFilename and validityMonths.
 */
static char *DoChangePWDriver
  (ripemInfo, privKeyOutFilename, needSelfSignedCert, pubKeyOutFilename,
   validityMonths, certInfoStream, ripemDatabase)
RIPEMInfo *ripemInfo;
char *privKeyOutFilename;
BOOL needSelfSignedCert;
char *pubKeyOutFilename;
unsigned int validityMonths;
FILE *certInfoStream;
RIPEMDatabase *ripemDatabase;
{
  char newPassword[MAX_PASSWORD_SIZE], *errorMessage;
  unsigned int newPasswordLen;

  /* For error, break to end of do while (0) block. */
  do {
    if (needSelfSignedCert) {
      /* ripemInfo already has public and private key.  We need a self-signed
           cert for upgrading from RIPEM 1.1 */
      SetNewUserDN (ripemInfo);
      if ((errorMessage = WriteSelfSignedCert
           (ripemInfo, validityMonths, pubKeyOutFilename)) != (char *)NULL)
        break;
    }

    newPasswordLen = GetPasswordToPrivKey
      (TRUE, TRUE, newPassword, sizeof (newPassword));

    if ((errorMessage = DoChangePW
         (ripemInfo, privKeyOutFilename, newPassword, newPasswordLen,
          ripemDatabase)) != (char *)NULL)
      break;
    if ((errorMessage = PrintCertNameAndDigest
         (ripemInfo->z.userCertDER, certInfoStream))
        != (char *)NULL)
      break;
  } while (0);

  R_memset ((POINTER)newPassword, 0, sizeof (newPassword));
  return (errorMessage);
}

static char *DoGenerateKeysDriver
  (ripemInfo, pubKeyOutFilename, privKeyOutFilename, bits, validityMonths,
   password, passwordLen, certInfoStream)
RIPEMInfo *ripemInfo;
char *pubKeyOutFilename;
char *privKeyOutFilename;
unsigned int bits;
unsigned int validityMonths;
unsigned char *password;
unsigned int passwordLen;
FILE *certInfoStream;
{
  char *errorMessage;
  unsigned char buf[4];

  if (bits == 0) {
    R_GenerateBytes (buf, 1, &ripemInfo->randomStruct);
    bits = 508 + (0x0f & buf[0]);
    if (bits < 512)
      bits = 512;
    if (ripemInfo->debug > 1) {
      fprintf (ripemInfo->debugStream,
               "Selected size of key being generated = %d bits.\n", bits);
    }
  }
  
  SetNewUserDN (ripemInfo);
  if ((errorMessage = DoGenerateKeys
       (ripemInfo, pubKeyOutFilename, privKeyOutFilename, bits,
        validityMonths, password, passwordLen)) != (char *)NULL)
    return (errorMessage);

  /* Write the new cert's DN and self-signed digest.
   */
  return (PrintCertNameAndDigest
          (ripemInfo->z.userCertDER, certInfoStream));
}

/*--- function InitUser ---------------------------------------
 *
 *  Initialize a TypUser structure.
 *
 *  Entry: email       points to the user's email address (zero-terminated).
 *
 *  Exit:  userEntry       points to a pointer to a newly-allocated TypUser 
 *                                              structure.
 */
static char *
InitUser(email,userEntry)
char *email;
TypUser **userEntry;
{
  char *errorMessage = NULL;
  char *cptr;
  
  *userEntry = (TypUser *) malloc(sizeof **userEntry);
  if(*userEntry) {
    (*userEntry)->gotpubkey = FALSE;
    if(!StrCopyAlloc(&cptr,email)) {
      free (*userEntry);
      errorMessage = ERROR_MALLOC;
    } else {
      (*userEntry)->emailaddr = cptr;
    }
  } else {
    errorMessage = ERROR_MALLOC;
  }
  
  return errorMessage;
}                       

/* Return TRUE if strings are the same, ignoring case, otherwise FALSE.
   [Note, this is similar to the function "match" in the RIPEM library.]
 */
static BOOL CaseIgnoreEqual (str,pattern)
char *str;
char *pattern;
{
  char ch1, ch2;

  do {
    ch1 = (char) (islower(*str) ? toupper(*str) : *str);
    ch2 = (char) (islower(*pattern) ? toupper(*pattern) : *pattern);
    if (ch1 != ch2)
      return FALSE;
    str++; pattern++;
  } while(ch1 == ch2 && ch1 && ch2);
  
  if(!ch1 && !ch2)
    return TRUE;
  else
    return FALSE;
}

/* Convert a CERT_ validity status into a string such as "VALID".
 */
static char *GetCertStatusString (certStatus)
int certStatus;
{
  switch (certStatus) {
  case CERT_VALID:
    return ("VALID");
  case CERT_PENDING:
    return ("PENDING");
  case CERT_EXPIRED:
    return ("EXPIRED");
  case CERT_CRL_EXPIRED:
    return ("CRL EXPIRED");
  case CERT_REVOCATION_UNKNOWN:
    return ("REVOCATION UNKNOWN");
  case CERT_UNVALIDATED:
    return ("UNVALIDATED");
  case CERT_REVOKED:
    return ("REVOKED");
    
  default:
    return ("UNRECOGNIZED TYPE");
  }
}

/*--- function AddUniqueUserToList --------------------------------------------
 *
 *  Add a TypUser structure to a list, first checking to ensure that
 *  the user isn't already on the list.
 *
 *  Entry:  user    points to a TypUser structure.
 *
 *   Exit:  list     may have been updated to include this entry.
 *        Returns NULL if successful, else a pointer to an error message.
 */
static char *
AddUniqueUserToList(user,list)
TypUser *user;
TypList *list;
{
  TypListEntry *entry_ptr = list->firstptr;
  TypUser *user_ptr;
  BOOL found=FALSE;
  
  for(; !found && entry_ptr; entry_ptr = entry_ptr->nextptr) {
    user_ptr = (TypUser *)(entry_ptr->dataptr);
    
    if(CaseIgnoreEqual(user_ptr->emailaddr,user->emailaddr)) {
      found = TRUE;
    }
  }
  if(!found) {
    return AddToList(NULL,user,sizeof *user,list);
  } else {
    return NULL;
  }
}

/* Write the dn to the stream in the format
   "CN = User, OU = Persona Certificate ...".
   This uses a + instead of , for AVAs on the same level.
   This does not write a newline at the end.
 */
static void WritePrintableName (stream, dn)
FILE *stream;
DistinguishedNameStruct *dn;
{
  int rdn, ava;

  for (rdn = MAX_RDN - 1; rdn >= 0; --rdn) {
    if (dn->RDNIndexStart[rdn] == -1)
      continue;

    for (ava = dn->RDNIndexStart[rdn]; ava <= dn->RDNIndexEnd[rdn]; ++ava) {
      /* Output the AVA.  AVA_TYPES[0] is "unknown" for bad types.
       */
      fputs (dn->AVATypes[ava] >
             MAX_AVA_TYPE ? AVA_TYPES[0] : AVA_TYPES[dn->AVATypes[ava]],
             stream);
      fputs (" = ", stream);
      fputs (dn->AVAValues[ava], stream);

      if (ava == dn->RDNIndexEnd[rdn]) {
        /* This is the last AVA in the RDN, so put a comma.
           But don't put anything if it is the last RDN. */
        if (rdn != 0)
          fputs (", ", stream);
      }
      else
        /* Put a plus because there are more AVAs in this RDN. */
        fputs (" + ", stream);
    }
  }
}

static char *PrintCertNameAndDigest (certDER, outStream)
unsigned char *certDER;
FILE *outStream;
{
  unsigned char digest[16];
  unsigned int digestLen, i;
  CertificateStruct *certStruct = (CertificateStruct *)NULL;
  CertFieldPointers fieldPointers;
  char *errorMessage = (char *)NULL;

  /* For error, break to end of do while (0) block. */
  do {
    /* Allocate the certStruct on the heap because it's big. */
    if ((certStruct = (CertificateStruct *)malloc
         (sizeof (*certStruct))) == (CertificateStruct *)NULL) {
      errorMessage = ERROR_MALLOC;
      break;
    }

    if (DERToCertificate (certDER, certStruct, &fieldPointers) < 0) {
      errorMessage = "Can't decode certificate for printing digest";
      break;
    }
    
    R_DigestBlock
      (digest, &digestLen, fieldPointers.innerDER, fieldPointers.innerDERLen,
       certStruct->digestAlgorithm);

    fputs ("User: ", outStream);
    WritePrintableName (outStream, &certStruct->subject);
    fprintf (outStream, "\n");

    fprintf (outStream, "User certificate digest: ");
    for (i = 0; i < digestLen; ++i)
      fprintf (outStream, "%02X ", (int)digest[i]);
    fprintf (outStream, "\n");
  } while (0);

  free (certStruct);
  return (errorMessage);
}

/* Take the first name in ripemInfo's userList and set the userDN
     to the Persona CA version.
 */
static void SetNewUserDN (ripemInfo)
RIPEMInfo *ripemInfo;
{
  unsigned int i;

  /* Pre-zeroize */
  R_memset ((POINTER)&ripemInfo->userDN, 0, sizeof (ripemInfo->userDN));
  
  strcpy (ripemInfo->userDN.AVAValues[0], "US");
  ripemInfo->userDN.AVATypes[0] = ATTRTYPE_COUNTRYNAME;
  ripemInfo->userDN.RDNIndexStart[0] =
    ripemInfo->userDN.RDNIndexEnd[0] = 0;

  strcpy (ripemInfo->userDN.AVAValues[1], "RSA Data Security, Inc.");
  ripemInfo->userDN.AVATypes[1] = ATTRTYPE_ORGANIZATIONNAME;
  ripemInfo->userDN.RDNIndexStart[1] =
    ripemInfo->userDN.RDNIndexEnd[1] = 1;

  strcpy (ripemInfo->userDN.AVAValues[2], "Persona Certificate");
  ripemInfo->userDN.AVATypes[2] = ATTRTYPE_ORGANIZATIONALUNITNAME;
  ripemInfo->userDN.RDNIndexStart[2] =
    ripemInfo->userDN.RDNIndexEnd[2] = 2;

  strcpy
    (ripemInfo->userDN.AVAValues[3],
     (char *)ripemInfo->userList->firstptr->dataptr);
  ripemInfo->userDN.AVATypes[3] = ATTRTYPE_COMMONNAME;
  ripemInfo->userDN.AVAIsT61[3] =
    (IsPrintableString
     ((unsigned char *)ripemInfo->userList->firstptr->dataptr,
      strlen ((char *)ripemInfo->userList->firstptr->dataptr)) ? 0 : 1);
  ripemInfo->userDN.RDNIndexStart[3] =
    ripemInfo->userDN.RDNIndexEnd[3] = 3;

  /* Set the rest of the RDN indexes to -1. */
  for(i = 4; i < MAX_RDN; ++i)
    ripemInfo->userDN.RDNIndexStart[i] =
     ripemInfo->userDN.RDNIndexEnd[i] = -1;
}

/* Write out each line in headerList to outStream, then a blank line.
 */
static void WritePrependedHeaders (headerList, outStream)
TypList *headerList;
FILE *outStream;
{
  TypListEntry *entry_ptr;
  long int nlines=0;
    
  for(entry_ptr=headerList->firstptr; entry_ptr; 
      entry_ptr = entry_ptr->nextptr) {
    fputs((char *)entry_ptr->dataptr,outStream);
    fputc ('\n', outStream);
    nlines++;
  }
  if(nlines)
    fputc ('\n', outStream);
}

/* Allocate a buffer and read stdin up to end of stream.
   While buffer is being resized, the previous memory for the buffer
     is zeroized so no sensitive data is left in memory.
   Return the alloced buffer in plaintext and its length in plaintextLen.
 */
static char *ReadPlaintextAlloc (plaintext, plaintextLen)
unsigned char **plaintext;
unsigned int *plaintextLen;
{
  unsigned char *newBuffer;
  unsigned int partInLen;
  
  *plaintext = (unsigned char *)NULL;
  *plaintextLen = 0;

  /* Keep expanding the buffer by a big chunk and read in more of the input.
   */
  while (1) {
    /* Leave an extra byte in case we need to append a \n at the very end. */
    if ((newBuffer = (unsigned char *)malloc
         (*plaintextLen + INPUT_BUFFER_SIZE + 1)) == (unsigned char *)NULL)
      return ("Cannot fit the input into memory. Try using a file for input.");

    /* Copy over contents from old buffer, zeroize and free the old buffer.
     */
    R_memcpy ((POINTER)newBuffer, (POINTER)*plaintext, *plaintextLen);
    R_memset ((POINTER)*plaintext, 0, *plaintextLen);
    free (*plaintext);

    *plaintext = newBuffer;

    /* Read the input into the newly allocated space. */
    partInLen = fread
      (*plaintext + *plaintextLen, 1, INPUT_BUFFER_SIZE, stdin);
    *plaintextLen += partInLen;

    if (partInLen < INPUT_BUFFER_SIZE) {
      /* fread returned less than the bytes requested, so end of stream */
      if (*plaintextLen > 0 && *(*plaintext + *plaintextLen - 1) != '\n')
        /* Make the last line end with a \n */
        *(*plaintext + ((*plaintextLen)++)) = '\n';

      return ((char *)NULL);
    }
  }
}

#ifndef RIPEMSIG

/* Set recipientKeys to the keys for the recipients in RecipientList.
   First, look for keys in certificates.  Then, for compatibility with
     RIPEM 1.1, look for unsigned keys.
   recipientKeys must be a buffer with at least as many elements as
     in RecipientList.
   Also, print the recipient statuses to the certInfoStream.
   This sets recipientKeyCount to this number of keys found.
   If abortIfRecipUnknown is TRUE and not all recipient keys are
     found, this returns an error.
 */
static char *GetRecipientKeys
  (ripemInfo, recipientKeys, recipientKeyCount, abortIfRecipUnknown,
   certInfoStream, ripemDatabase)
RIPEMInfo *ripemInfo;
RecipientKeyInfo *recipientKeys;
unsigned int *recipientKeyCount;
BOOL abortIfRecipUnknown;
FILE *certInfoStream;
RIPEMDatabase *ripemDatabase;
{
  CertificateStruct *certStruct = (CertificateStruct *)NULL;
  TypList certChain, certs;
  TypListEntry *entry;
  TypUser *recipient;
  unsigned int usernameCount;
  char *errorMessage = (char *)NULL;
  int chainStatus;

  /* Set to initial state so it is OK to free on error at any time. */
  InitList (&certChain);
  InitList (&certs);
  *recipientKeyCount = 0;

  /* For error, break to end of do while (0) block. */
  do {
    /* Get the public keys of all the users first.
     * We do this first because we want to know whether some
     * of the keys are unavailable before we do a lot of
     * time-consuming RSA encryption.
     */
       
    /* Allocate cert struct on the heap since it is so big */
    if ((certStruct = (CertificateStruct *)malloc
         (sizeof (*certStruct))) == (CertificateStruct *)NULL) {
      errorMessage = ERROR_MALLOC;
      break;
    }
    
    /* Prepare to put recipient statuses to the certInfoStream. */
    fputs ("Recipient status:\n", certInfoStream);

    for(entry = RecipientList.firstptr; entry;
        entry = entry->nextptr) {
      recipient = (TypUser *)entry->dataptr;
      
      /* Prepare to get recipient chain by freeing content from
         previous iteration. */
      FreeList (&certs);
      FreeList (&certChain);
      
      if ((errorMessage = GetCertsBySmartname
           (ripemDatabase, &certs, recipient->emailaddr, ripemInfo))
          != (char *)NULL)
        break;
      if (!certs.firstptr)
        /* Couldn't find a certificate for this smartname, so try
             next recipient. */
        continue;
      if (DERToCertificate
          ((unsigned char *)certs.firstptr->dataptr, certStruct,
           (CertFieldPointers *)NULL) < 0) {
        if (ripemInfo->debug > 1)
          fprintf (ripemInfo->debugStream,
                   "Warning: Cannot decode certificate from database.\n");
        continue;
      }
      if ((errorMessage = SelectCertChain
           (ripemInfo, &certChain, &chainStatus, &certStruct->subject,
            (unsigned char *)NULL, 0, ripemDatabase)) != (char *)NULL)
        break;
      if (chainStatus != 0) {
        /* Decode the certificate at the "bottom" of the chain.  Don't check
             for error decoding since it already decoded sucessfully. */
        DERToCertificate
          ((unsigned char *)certChain.firstptr->dataptr, certStruct,
           (CertFieldPointers *)NULL);

        fprintf (certInfoStream, "%s: ",
                 GetCertStatusString (chainStatus));
        WritePrintableName (certInfoStream, &certStruct->subject);
        fprintf (certInfoStream, "\n");

        recipient->gotpubkey = TRUE;
        recipientKeys[*recipientKeyCount].publicKey = certStruct->publicKey;
        recipientKeys[*recipientKeyCount].username = recipient->emailaddr;
        ++ (*recipientKeyCount);
      }
    }
    if (errorMessage != (char *)NULL)
      break;

    /* Now try getting public keys from non-certificates, finger, etc.
       This is only to support the "pre-certificate" model of RIPEM 1.1. */
    for(entry = RecipientList.firstptr; entry;
        entry = entry->nextptr) {
      recipient = (TypUser *)entry->dataptr;

      if (recipient->gotpubkey)
        continue;

      if(ripemInfo->debug > 1) {
        fprintf(ripemInfo->debugStream,"== Getting public key for %s\n",
                recipient->emailaddr);
      }   

      if ((errorMessage = GetUnvalidatedPublicKey
           (recipient->emailaddr, &ripemDatabase->pubKeySource,
            &recipientKeys[*recipientKeyCount].publicKey,
            &recipient->gotpubkey, ripemInfo))
          != (char *)NULL)
        break;

      if (recipient->gotpubkey) {
        fprintf (certInfoStream,
                 "%s (on file but not validated)\n", recipient->emailaddr);
        recipientKeys[*recipientKeyCount].username = recipient->emailaddr;
        ++ (*recipientKeyCount);
      }
    }
    if (errorMessage != (char *)NULL)
      break;
    
    /* Put a blank line after the recipients */
    fputs ("\n", certInfoStream);           

    if (!CheckKeyList (&RecipientList, abortIfRecipUnknown)) {
      errorMessage =  "Can't find some public keys; RIPEM aborting.";
      break;
    }
  } while (0);

  FreeList (&certChain);
  FreeList (&certs);
  free (certStruct);
  return (errorMessage);
}

/*--- function CheckKeyList ----------------------------------------
 *
 *  Check a list of users to make sure that we have a public key for
 *  each one.  
 *
 *  Entry:  userList is a list of TypUser types, containing
 *                   information on users (including whether
 *                   we know their public keys).
 *
 *  Exit:   Returns TRUE if it is OK to proceed, else FALSE.
 *          It's OK if we have the key of each user, or if the
 *          user has been prompted and has said it's OK to proceed
 *          even if some keys are unknown.
 */
static BOOL
CheckKeyList(userList, abortIfRecipUnknown)
TypList *userList;
{
  TypListEntry *entry_ptr;
  TypUser *recip_ptr;
  int bad_users = 0;
  BOOL ok = TRUE, asking=TRUE;
#define REPLY_BYTES 4
  unsigned char userbytes[REPLY_BYTES],timebytes[REPLY_BYTES];
  char reply;
  int n_userbytes, n_timebytes;
  
  for(entry_ptr = userList->firstptr; ok && entry_ptr;
      entry_ptr = entry_ptr->nextptr) {
    recip_ptr = (TypUser *)entry_ptr->dataptr;
    
    if(!recip_ptr->gotpubkey) bad_users++; 
  }        
  
  if(bad_users) {
    if(abortIfRecipUnknown) {
      ok = FALSE;
    } else {
      if(bad_users==1) {
        fprintf(stderr,"Could not find public keys for this user:\n");
      } else {
        fprintf(stderr,"Could not find public keys for these %d users:\n",
                bad_users);
      }
      for(entry_ptr = userList->firstptr; ok && entry_ptr;
          entry_ptr = entry_ptr->nextptr) {
        recip_ptr = (TypUser *)entry_ptr->dataptr;
        if(!recip_ptr->gotpubkey) {
          fprintf(stderr,"   %s\n",recip_ptr->emailaddr);
        }
      }
      do {
        fprintf(stderr,"Proceed anyway, deleting these users? ");
        n_userbytes = n_timebytes = REPLY_BYTES;
        GetUserInput(userbytes,&n_userbytes,timebytes,&n_timebytes,TRUE);
        reply = (char) userbytes[0];
        if(reply == 'y' || reply=='Y') {
          ok = TRUE;
          asking = FALSE;
        } else if(reply=='n' || reply=='N') {
          ok = FALSE;
          asking = FALSE;
        }
      } while(asking);
    }
  }
  return ok;
}

#endif

