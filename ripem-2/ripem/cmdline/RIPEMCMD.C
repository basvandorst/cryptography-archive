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

#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

#define INPUT_BUFFER_SIZE 1024

#define SECONDS_IN_MONTH \
  ((UINT4)((UINT4)365 * (UINT4)24 * (UINT4)3600) / (UINT4)12)

static void MainEnd P((int stat));
static char *CrackCmd P((RIPEMFlags *, int, char **));
static char *CrackKeyServer P((char *));
static void ShowParams P((RIPEMFlags *));
static void DoRandom P ((RIPEMInfo *));
static unsigned int GetPasswordToPrivKey
  P ((BOOL, BOOL, unsigned char *, unsigned int));
static char *OpenFiles P((RIPEMInfo *));
static char *DoEncipherDriver
  P ((RIPEMInfo *, RIPEMFlags *, FILE *, FILE *, enum enhance_mode, int, int,
      TypList *, FILE *, RIPEMDatabase *));
static char *DoDecipherDriver
  P ((RIPEMInfo *, FILE *, FILE *, int, unsigned int, BOOL, FILE *,
      RIPEMDatabase *));
static char *DoChangePWDriver
  P ((RIPEMInfo *, BOOL, char *, unsigned int, FILE *, RIPEMDatabase *));
static char *DoGenerateKeysDriver
  P ((RIPEMInfo *, char *, unsigned int, unsigned int, unsigned char *,
      unsigned int, FILE *, RIPEMDatabase *));
static char *InitUser P((char *, TypUser **));
static BOOL CaseIgnoreEqual P((char *, char *));
static char *GetCertStatusString P((int));
static char *AddUniqueUserToList P((TypUser *, TypList *));
static char *PrintCertNameAndDigest P((unsigned char *, FILE *));
static void SetNewUserDN P((RIPEMInfo *, char *));
static void WritePrependedHeaders P((TypList *, FILE *));
static char *ReadPlaintextAlloc P((unsigned char **, unsigned int *));
static char *ReadEmailHeader P((FILE *, TypList *, TypList *));

#ifndef RIPEMSIG
static char *GetRecipientKeys
  P ((RIPEMInfo *, RecipientKeyInfo *, unsigned int *, RIPEMFlags *, FILE *,
      RIPEMDatabase *));
static BOOL CheckKeyList P((TypList *, int));
#endif

#ifdef __BORLANDC__
extern unsigned _stklen = 12192;  /* Increase stack size for Borland C */
#endif

extern char usage_msg_line1[];
extern char *usage_msg[];

char author[] = "Mark Riordan  1100 Parker  Lansing MI  48912";
char author2[] = 
  "mrr@scss3.cl.msu.edu or riordanmr@clvax1.cl.msu.edu   Sept 1992";

int Argc;
char **Argv;
FILE *InStream = NULL, *OutStream = NULL;
TypList RecipientList, UserList;
char *InFileName = NULL, *OutFileName = NULL, *RandomFileName = NULL,
  *DebugFileName = NULL;
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
int MessageFormat = MESSAGE_FORMAT_RIPEM1;
BOOL GotValidityMonths=FALSE;
int ValidityMonths = 12;
char *KeyToPrivKey = NULL, *NewKeyToPrivKey = NULL;
FILE *RandomStream = (FILE *)0;
char ErrMsgTxt[LINEBUFSIZE];
FILE *DebugStream = NULL;
FILE *CertInfoStream = NULL;
int Debug = 0;

#ifdef MACTC  /* rwo */
clock_t Time0, Time1;
#endif

#ifdef TESTDOS386
extern int ProcessorType(void);
int Got386 = 0;
#endif

int
main (argc,argv)
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

  Argc = argc;
  Argv = argv;

  RIPEMDatabaseConstructor (&RipemDatabase);
  InitList (&UserList);
  
#ifdef TESTDOS386
  Got386 = (ProcessorType() >= 3);
#endif

  /* For error, break to end of do while (0) block. */
  do {
    /* Append the RIPEM_VERSION to the end of the first usage message line
         so it will appear if we have to print the usage. */
    strcat (usage_msg_line1, RIPEM_VERSION);

    /* Parse the command line. */
    if ((errorMessage = CrackCmd (&ripemFlags, argc, argv)) != (char *)NULL) {
      usage(errorMessage,usage_msg);
      status = 4;
      break;
    }

    /* Open files.  Set ripemInfo.debugStream after this opens it. */
    if ((errorMessage = OpenFiles (&ripemInfo)) != (char *)NULL) {
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
    ripemInfo.z.usernameAliases = &UserList;
  
    /* Get the password.  Prompt twice if generating. */
    passwordLen = GetPasswordToPrivKey
      (Action == ACT_GEN_KEY, FALSE, (unsigned char *)password,
       sizeof (password));

    if (Action != ACT_GEN_KEY) {
      /* Need to log in for all actions other than key generation.  Log in
           the user and load the preferences, checking for special errors.
         Use the first name in UserList as the one to log in by.
       */
      if ((errorMessage = RIPEMLoginUser
           (&ripemInfo, (char *)UserList.firstptr->dataptr, &RipemDatabase,
            (unsigned char *)password, passwordLen)) != (char *)NULL) {
        if (Action == ACT_CHANGE_PW &&
            !strcmp (errorMessage, ERR_SELF_SIGNED_CERT_NOT_FOUND))
          /* No self-signed cert, but we are trying to use change password
               to upgrade from RIPEM 1.1.  We will create a self-signed cert
               below. */
          needSelfSignedCert = TRUE;
        else if (strcmp (errorMessage, ERR_PREFERENCES_NOT_FOUND) == 0) {
          /* This is OK, just issue a warning.
             Give the warning even if Debug is 0 since it may mean the
               preferences has been unexpectedly deleted. */
          fputs ("Warning: User preferences were not found.  RIPEM will use defaults.\n", CertInfoStream);
          errorMessage = (char *)NULL;
        }
        else if (strcmp (errorMessage, ERR_PREFERENCES_CORRUPT) == 0) {
          /* Issuer alert and continue.
             Give the alert even if Debug is 0 since it may mean the
               preferences has been unexpectedly deleted. */
          fputs ("ALERT: Preference information has been corrupted.  RIPEM will use defaults.\n", CertInfoStream);
          errorMessage = (char *)NULL;
        }
        else {
          /* Other errors. */
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
         MessageFormat, EncryptionAlgorithm, &RecipientList,
         CertInfoStream, &RipemDatabase);
    } else if(Action == ACT_DECRYPT){
      errorMessage = DoDecipherDriver
        (&ripemInfo, InStream, OutStream, GotValidityMonths, ValidityMonths,
         ripemFlags.prependHeaders, CertInfoStream, &RipemDatabase);
    } else if(Action == ACT_CHANGE_PW) {
      errorMessage = DoChangePWDriver
        (&ripemInfo, needSelfSignedCert, (char *)UserList.firstptr->dataptr,
         ValidityMonths, CertInfoStream, &RipemDatabase);
    } else {
      errorMessage = DoGenerateKeysDriver
        (&ripemInfo, (char *)UserList.firstptr->dataptr, Bits, ValidityMonths,
         (unsigned char *)password, passwordLen, CertInfoStream,
         &RipemDatabase);
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
  RIPEMDatabaseDestructor (&RipemDatabase);

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
  char *recip_opts = "m";
  TypUser *recipient;
  char *usernameStr, *key_server_str=NULL;
  TypList mylist;
  TypListEntry *entry;

  InitList (&mylist);

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
           "3edgcr:h:b:A:R:p:s:P:S:m:M:u:k:K:i:o:D:F:Z:C:y:Y:T:v:H:")) != -1) {
      switch (ch) {
        case '?':
          return ("Unrecognized command line option.");
                      
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
          if (EncryptionAlgorithm < 0)
    return ("Symmetric cipher must be either \"des-cbc\" or \"des-ede-cbc\".");
          break;

        case 'm':       /* Encryption mode */
          if (CaseIgnoreEqual (optarg, "encrypted"))
            EnhanceMode = MODE_ENCRYPTED;
          else if (CaseIgnoreEqual (optarg, "mic-only"))
            EnhanceMode = MODE_MIC_ONLY;
          else if (CaseIgnoreEqual (optarg, "mic-clear"))
            EnhanceMode = MODE_MIC_CLEAR;
          else
            return
("Processing mode must be one of \"encrypted\"  \"mic-only\" or \"mic-clear\".");

          break;

        case 'M':       /* Message format */
          if (CaseIgnoreEqual (optarg, "ripem1"))
            MessageFormat = MESSAGE_FORMAT_RIPEM1;
          else if (CaseIgnoreEqual (optarg, "pem"))
            MessageFormat = MESSAGE_FORMAT_PEM;
          else
            return ("Message format must be one of \"ripem1\" or \"pem\".");

          break;

        case 'b':       /* Number of bits in generated key */
          Bits = atoi(optarg);
          if(Bits < MIN_RSA_MODULUS_BITS || Bits > MAX_RSA_MODULUS_BITS) {
            sprintf(ErrMsgTxt,"Number of bits must be %d <= bits <= %d",
                    MIN_RSA_MODULUS_BITS,MAX_RSA_MODULUS_BITS);
            return (ErrMsgTxt);
          }
          break;

        case 'v':                 /* Number of months to validate sender for */
          ValidityMonths = atoi (optarg);
          if (ValidityMonths <= 0)
            return ("Validity months must be > 0");
          else
            GotValidityMonths = TRUE;
          break;

        case 'p':       /* Public key filename */
          if ((errorMessage = AddKeySourceFilename
               (&RipemDatabase.pubKeySource, optarg)) != (char *)NULL)
            return (errorMessage);
          break;
          
        case 'P':       /* Public key output filename */
          return ("-P is obsolete. Public keys are written to \"pubkeys\" in the RIPEM home dir.");

        case 's':       /* Secret (private) key filename */
          if ((errorMessage = AddKeySourceFilename
               (&RipemDatabase.privKeySource, optarg)) != (char *)NULL)
            return (errorMessage);
          break;
          
        case 'S':       /* Private key output filename */
          return ("-S is obsolete. Private keys are written to \"privkey\" in the RIPEM home dir.");

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
        return ("-R option should be one or more of \"cefks\"");
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
        return ("-h option should be one or more of \"ipr\"");
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
        return ("-T option should be one or more of \"amn\"");
    }
  }

  /* If we don't have the RIPEM home dir yet, look for the environment
       variable. */
  if (!HomeDir) {
    GetEnvFileName (HOME_DIR_ENV, "", &HomeDir);
    if (*HomeDir == '\0')
      /* GetEnvFileName returned the "" */
      HomeDir = NULL;
  }

  /* Check for syntax error. */

  if (got_action != 1) {
    return ("Must specify one of -e, -d, -g, -c");
  }
  else if(Action==ACT_ENCRYPT && EnhanceMode==MODE_ENCRYPTED) {
    if(!RecipientList.firstptr && !ripemFlags->addRecip)
      return ("Must specify recipient(s) when enciphering.");
  }
  else if(Action != ACT_ENCRYPT && RecipientList.firstptr != NULL)
    return ("-r should be specified only when enciphering.");
  else if(Action == ACT_ENCRYPT &&
          RipemDatabase.pubKeySource.origin[0] == KEY_FROM_NONE
          && RipemDatabase.pubKeySource.origin[1] == KEY_FROM_NONE)
    return ("Must specify at least one source of public keys.");
  
  /* Obtain the username if it wasn't specified. */

  if (!got_username)
    GetUserAddress (&usernameStr);
  
  /* Crack the username string (which can contain multiple aliases
   * separated by commas) into a list.
   */
  
  CrackLine(usernameStr,&UserList);

  /* Obtain the name of the public key server. */
  if(!key_server_str)
    GetEnvAlloc(SERVER_NAME_ENV,&key_server_str);

  errorMessage = CrackKeyServer (key_server_str);
  free (key_server_str);
  if (errorMessage != (char *)NULL)
    return (errorMessage);

  /* Add any public and private keys specified by the environment variables.
   */
  GetEnvFileName (PUBLIC_KEY_FILE_ENV, "", &cptr);
  if (strcmp (cptr, "") != 0) {
    CrackLine (cptr, &mylist);
    free (cptr);
    for (entry = mylist.firstptr; entry; entry = entry->nextptr) {
      /* Set length to zero so that FreeList won't try to zeroize it. */
      entry->datalen = 0;
      ExpandFilename ((char **)&entry->dataptr);
      if ((errorMessage = AddKeySourceFilename
           (&RipemDatabase.pubKeySource, (char *)entry->dataptr))
          != (char *)NULL)
        return (errorMessage);
    }
  }

  GetEnvFileName (PRIVATE_KEY_FILE_ENV, "", &cptr);
  if (strcmp (cptr, "") != 0) {
    CrackLine (cptr, &mylist);
    free (cptr);
    for (entry = mylist.firstptr; entry; entry = entry->nextptr) {
      /* Set length to zero so that FreeList won't try to zeroize it. */
      entry->datalen = 0;
      ExpandFilename ((char **)&entry->dataptr);
      if ((errorMessage = AddKeySourceFilename
           (&RipemDatabase.privKeySource, (char *)entry->dataptr))
          != (char *)NULL)
        return (errorMessage);
    }
  }

  /* Obtain the name of the file containing random data. */
  if(UseRndFile && !RandomFileName) {
    GetEnvFileName(RANDOM_FILE_ENV,"",&RandomFileName);
    if (strlen (RandomFileName) == 0)
      RandomFileName = NULL;
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
        if(*cptr=='\n' || *cptr=='\r')
          *cptr='\0';
      }
    }
  }

  return ((char *)NULL);
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
  
  InitList (&(RipemDatabase.pubKeySource.serverlist));
  InitList (&name_list);
  
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
    nbytes = GetRandomBytes(ranbuf,RANBUFSIZE,ripemInfo);
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
 *  Exit:   InStream, OutStream, RandomStream contain file pointers to the
 *          corresponding files (or streams), if there's no error.
 *          DebugStream is opened and ripemInfo->debugStream is set to it.
 *          The RipemDatabase is initialized.
 *
 *          Returns NULL if no error, else address of error string.
 */
static char *OpenFiles (ripemInfo)
RIPEMInfo *ripemInfo;
{
  char *errorMessage;
  FILE *stream;
        
  if(InFileName) {
    InStream = fopen(InFileName,"r");
    if(!InStream) {
      sprintf(ErrMsgTxt,"Can't open input file %s.",InFileName);
      return(ErrMsgTxt);
    }
  } else
    InStream = stdin;
  
  if(OutFileName) {
    OutStream = fopen(OutFileName,"w");
    if(!OutStream) {
      sprintf(ErrMsgTxt,"Can't open output file %s.",OutFileName);
      return(ErrMsgTxt);
    }
  } else
    OutStream = stdout;
  
  if(DebugFileName) {
    DebugStream = fopen(DebugFileName,"w");
    if(!DebugStream) {
      sprintf(ErrMsgTxt,"Can't open debug file %s.",DebugFileName);
      return(ErrMsgTxt);
    }
  } else
    DebugStream = stderr;

  CertInfoStream = DebugStream;

  /* Set up debug info in ripemInfo */
  ripemInfo->debugStream = DebugStream;
  ripemInfo->debug = Debug;

  /* We now have a debug stream and the user's choice of home dir.
     Make sure the home dir exists and that it has an ending directory
       separator.  This will try to get a default name is none is specified
       yet.
   */
  if ((errorMessage = EstablishRIPEMHomeDir (&HomeDir, ripemInfo))
      != (char *)NULL)
    return (errorMessage);

  /* Now we have a home dir, so open the database. */
  if ((errorMessage = InitRIPEMDatabase
       (&RipemDatabase, HomeDir, ripemInfo)) != (char *)NULL)
    return (errorMessage);

  if (RandomFileName == (char *)NULL) {
    /* Random file has not been specified yet, so try to open randomin in
         home dir for read. If successful, use it.
     */
    if ((RandomFileName = (char *)malloc (strlen (HomeDir) + 9))
        == (char *)NULL)
      return (ERROR_MALLOC);
    strcpy (RandomFileName, HomeDir);
    strcat (RandomFileName, "randomin");
    
    if ((stream = fopen (RandomFileName, "r")) != (FILE *)NULL)
      /* The randomin file was opened, so close it to be opened later. */
      fclose (stream);
    else {
      /* There is no randomin, so get rid of RandomFileName */
      free (RandomFileName);
      RandomFileName = (char *)NULL;
    }
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
  
  return ((char *)NULL);
}

/* Do processing on recipient list and email headers.  Then call DoEncipher.
   recipientList is a list of TypUser.  If addRecip in ripemFlags is set, this
     modifies recipientList by adding recipients from the email header.
 */
static char *DoEncipherDriver
  (ripemInfo, ripemFlags, inStream, outStream, enhanceMode, messageFormat,
   encryptionAlgorithm, recipientList, certInfoStream, ripemDatabase)
RIPEMInfo *ripemInfo;
RIPEMFlags *ripemFlags;
FILE *inStream;
FILE *outStream;
enum enhance_mode enhanceMode;
int messageFormat;
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
  unsigned int recipientKeyCount = 0;
  TypList headerList, recipientNames;
  TypListEntry *entry;
#ifndef RIPEMSIG
  unsigned int usernameCount;
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
      
      /* Allocate recipientKeys buffer. We are done using usernameCount,
           so increment it if there is an entry for myselfAsRecip.
       */
      if (ripemFlags->myselfAsRecip)
        ++usernameCount;
      if ((recipientKeys = (RecipientKeyInfo *)malloc
           (usernameCount * sizeof (*recipientKeys)))
          == (RecipientKeyInfo *)NULL) {
        errorMessage = ERROR_MALLOC;
        break;
      }
      if ((errorMessage = GetRecipientKeys
           (ripemInfo, recipientKeys, &recipientKeyCount, ripemFlags,
            certInfoStream, ripemDatabase)) != (char *)NULL)
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
        R_RandomUpdate
          (&ripemInfo->randomStruct, (unsigned char *)line, sizeof (line));

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
      ReportCPUTime ("Before RIPEMEncipherInit",ripemInfo);
#endif
    if ((errorMessage = RIPEMEncipherInit
         (ripemInfo, enhanceMode, messageFormat, encryptionAlgorithm,
          recipientKeys, recipientKeyCount)) != (char *)NULL)
      break;
#ifdef DOGETRUSAGE
    if (ripemInfo->debug > 1)
      ReportCPUTime ("After RIPEMEncipherInit",ripemInfo);
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
              strlen ((char *)entry->dataptr), ripemDatabase)) != (char *)NULL)
          break;
        fwrite (partOut, 1, partOutLen, outStream);

        if ((errorMessage = RIPEMEncipherUpdate
             (ripemInfo, &partOut, &partOutLen, (unsigned char *)"\n", 1,
              ripemDatabase)) != (char *)NULL)
          break;
        fwrite (partOut, 1, partOutLen, outStream);
      }
      if (errorMessage != (char *)NULL)
        /* broke because of error */
        break;

      /* Encipher a blank line after the email header. */
      if ((errorMessage = RIPEMEncipherUpdate
           (ripemInfo, &partOut, &partOutLen, (unsigned char *)"\n", 1,
            ripemDatabase)) != (char *)NULL)
        break;
      fwrite (partOut, 1, partOutLen, outStream);
    }

    /* Encipher the input text.
     */
    if (inStream == stdin) {
      if ((errorMessage = RIPEMEncipherUpdate
           (ripemInfo, &partOut, &partOutLen, plaintext, plaintextLen,
            ripemDatabase)) != (char *)NULL)
        break;
      fwrite (partOut, 1, partOutLen, outStream);
    }
    else {
      while (fgets (line, sizeof (line), inStream)) {
        /* Line already includes the '\n'.
         */
        if ((errorMessage = RIPEMEncipherUpdate
             (ripemInfo, &partOut, &partOutLen, (unsigned char *)line,
              strlen (line), ripemDatabase)) != (char *)NULL)
          break;
        fwrite (partOut, 1, partOutLen, outStream);
      }
      if (errorMessage != (char *)NULL)
        /* broke because of error */
        break;
    }

    /* Finalize and flush the output.
     */
    if ((errorMessage = RIPEMEncipherFinal
         (ripemInfo, &partOut, &partOutLen, ripemDatabase)) != (char *)NULL)
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
  (ripemInfo, inStream, outStream, validate, validityMonths, prependHeaders,
   certInfoStream, ripemDatabase)
RIPEMInfo *ripemInfo;
FILE *inStream;
FILE *outStream;
int validate;
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
  ChainStatusInfo chainStatus;
  unsigned char *partOut;
  unsigned int partOutLen;
  char lineIn[500];
  UINT4 endValidity;

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

    if (enhanceMode == MODE_CRL) {
      /* CRL message.  There are no senders, so just print a message and
           finish. */
      fprintf (certInfoStream, "Received CRL message.\n");
      break;
    }

    if (chainStatus.overall == 0) {
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

        if (validate) {
          /* The user specified the validity months, so validate
               the sender's certificate.
             certStruct already has the correct subject name and public
               key.  Set the validity, not allowing it to go past the
               end validity of the sender's self-signed cert.
           */
          R_time (&certStruct->notBefore);
          if (certStruct->notAfter < certStruct->notBefore) {
            /* The expiration date in the sender's cert is earlier than now. */
            errorMessage =
              "Cannot validate the sender because their self-signed certificate has expired.";
            break;
          }
          endValidity = certStruct->notBefore +
            ((UINT4)validityMonths * SECONDS_IN_MONTH);
          if (endValidity < certStruct->notAfter)
            /* The end validity is within the limit set forth by the
                 sender's self-signed certificate. */
            certStruct->notAfter = endValidity;

          if ((errorMessage = ValidateAndWriteCert
               (ripemInfo, certStruct, ripemDatabase)) != (char *)NULL)
            break;

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
      if (chainStatus.overall == CERT_UNVALIDATED) {
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
           GetCertStatusString (chainStatus.overall));
      }
      fputs ("-------------------------\n", certInfoStream);
    }
  } while (0);

  free (certStruct);
  FreeList (&certChain);
  return (errorMessage);
}

/* If needSelfSignedCert is TRUE, make a DN from username and call
     WriteSelfSignedCert with validityMonths for upgrading from RIPEM 1.1.
     Otherwise ignore username and validityMonths.
 */
static char *DoChangePWDriver
  (ripemInfo, needSelfSignedCert, username, validityMonths, certInfoStream,
   ripemDatabase)
RIPEMInfo *ripemInfo;
BOOL needSelfSignedCert;
char *username;
unsigned int validityMonths;
FILE *certInfoStream;
RIPEMDatabase *ripemDatabase;
{
  char *errorMessage;
  unsigned char newPassword[MAX_PASSWORD_SIZE];
  unsigned int newPasswordLen;

  /* For error, break to end of do while (0) block. */
  do {
    if (needSelfSignedCert) {
      /* ripemInfo already has public and private key.  We need a self-signed
           cert for upgrading from RIPEM 1.1 */
      SetNewUserDN (ripemInfo, username);
      if ((errorMessage = WriteSelfSignedCert
           (ripemInfo, validityMonths, ripemDatabase)) != (char *)NULL)
        break;
    }

    newPasswordLen = GetPasswordToPrivKey
      (TRUE, TRUE, newPassword, sizeof (newPassword));

    if ((errorMessage = RIPEMChangePassword
         (ripemInfo, newPassword, newPasswordLen, ripemDatabase))
        != (char *)NULL)
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
  (ripemInfo, username, bits, validityMonths, password, passwordLen,
   certInfoStream, ripemDatabase)
RIPEMInfo *ripemInfo;
char *username;
unsigned int bits;
unsigned int validityMonths;
unsigned char *password;
unsigned int passwordLen;
FILE *certInfoStream;
RIPEMDatabase *ripemDatabase;
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
  
  SetNewUserDN (ripemInfo, username);
  if ((errorMessage = RIPEMGenerateKeys
       (ripemInfo, bits, validityMonths, password, passwordLen, ripemDatabase))
      != (char *)NULL)
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
  case CERT_REVOCATION_UNKNOWN:
    return ("REVOCATION UNKNOWN");
  case CERT_PENDING:
    return ("PENDING");
  case CERT_EXPIRED:
    return ("EXPIRED");
  case CERT_CRL_EXPIRED:
    return ("CRL EXPIRED");
  case CERT_UNVALIDATED:
    return ("UNVALIDATED");
  case CERT_CRL_OUT_OF_SEQUENCE:
    return ("CRL OUT OF SEQUENCE");
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

/* Take the username and set the userDN to the Persona CA version.
 */
static void SetNewUserDN (ripemInfo, username)
RIPEMInfo *ripemInfo;
char *username;
{
  /* Make sure RDN indexes are -1 */
  InitDistinguishedNameStruct (&ripemInfo->userDN);
  
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

  ripemInfo->userDN.AVATypes[3] = ATTRTYPE_COMMONNAME;
  /* Use PRINTABLE_STRING tag if possible, otherwise T61_STRING */
  ripemInfo->userDN.AVATag[3] =
    (IsPrintableString ((unsigned char *)username, strlen (username)) ?
     ATTRTAG_PRINTABLE_STRING : ATTRTAG_T61_STRING);
  strcpy (ripemInfo->userDN.AVAValues[3], username);
  ripemInfo->userDN.RDNIndexStart[3] =
    ripemInfo->userDN.RDNIndexEnd[3] = 3;
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

/* Read a header up to the first blank line.
   If headerList is not NULL, add header lines to the list.  The calling
     routine must initialize the list.
   If recipientNames is not NULL, add email addresses in all To: and
     Cc: fields.  The calling routine must initialize the list.
 */
static char *ReadEmailHeader (stream, headerList, recipientNames)
FILE *stream;
TypList *headerList;
TypList *recipientNames;
{
  unsigned char line[1024], *linecp;
  BOOL to_field = FALSE;
  char *errorMessage = (char *)NULL, *readStatus;

  /* For error, break to end of do while (0) block. */
  do {
    while ((readStatus = fgets ((char *)line, sizeof (line), stream))
           != NULL) {
      /* Strip end-of-line CR and/or NL */
      for (linecp=line; *linecp && *linecp!='\r' && *linecp!='\n'; linecp++);
      *linecp = '\0';
      
      if(line[0] == '\0')
        break;

      if (recipientNames != (TypList *)NULL) {
        if (matchn ((char *)line,"To:", 3) ||
            matchn ((char *)line,"cc:",3)) {
          to_field = TRUE;
          CrackRecipients ((char *)line + 3, recipientNames);
        } else if (to_field) {
          if (line[0] == ' ' || line[0] == '\t')
            CrackRecipients ((char *)line, recipientNames);
          else
            to_field = FALSE;
        }
      }
      if (headerList != (TypList *)NULL) {
        if ((errorMessage = AppendLineToList ((char *)line, headerList))
            != (char *)NULL)
          break;
      }
    }
    if (errorMessage != (char *)NULL)
      /* broke because of error */
      break;

    if (readStatus == (char *)NULL) {
      /* Make sure we got a NULL read status becuase of end of file. */
      if (!feof (stream)) {
        errorMessage = "Error reading header from stream";
        break;
      }

      /* Note: we have reached the end of stream before reading a blank line */
    }
  } while (0);

  /* Zeroize the line */
  R_memset ((POINTER)line, 0, sizeof (line));  
  return (errorMessage);
}

#ifndef RIPEMSIG

/* Set recipientKeys to the keys for the recipients in RecipientList.
   If ripemFlags->myselfAsRecip is TRUE, add entry to the logged-in user
     using the smart name of ripemInfo->userDN.
   Next, look for keys in certificates.  Then, for compatibility with
     RIPEM 1.1, look for unsigned keys.
   recipientKeys must be a buffer with at least as many elements as
     in RecipientList plus an extra if ripemFlags->myselfAsRecip is TRUE.
   Also, print the recipient statuses to the certInfoStream.
   This sets recipientKeyCount to this number of keys found.
   If ripemFlags->abortIfRecipUnknown is TRUE and not all recipient keys are
     found, this returns an error.
 */
static char *GetRecipientKeys
  (ripemInfo, recipientKeys, recipientKeyCount, ripemFlags, certInfoStream,
   ripemDatabase)
RIPEMInfo *ripemInfo;
RecipientKeyInfo *recipientKeys;
unsigned int *recipientKeyCount;
RIPEMFlags *ripemFlags;
FILE *certInfoStream;
RIPEMDatabase *ripemDatabase;
{
  CertificateStruct *certStruct = (CertificateStruct *)NULL;
  TypList certChain, certs;
  TypListEntry *entry;
  TypUser *recipient;
  char *errorMessage = (char *)NULL;
  ChainStatusInfo chainStatus;

  /* Set to initial state so it is OK to free on error at any time. */
  InitList (&certChain);
  InitList (&certs);
  *recipientKeyCount = 0;

  /* For error, break to end of do while (0) block. */
  do {
    if (ripemFlags->myselfAsRecip) {
      recipientKeys[*recipientKeyCount].publicKey = ripemInfo->publicKey;
      recipientKeys[*recipientKeyCount].username =
        GetDNSmartNameValue (&ripemInfo->userDN);
      ++ (*recipientKeyCount);
    }

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

      /* We have a cert with the full subject distinguished name, so
           select a chain.  Pass NULL for public key since we don't know
           now what the best public key is.  Also, set directCertOnly
           FALSE to allow any certificate chain. */
      if ((errorMessage = SelectCertChain
           (ripemInfo, &certChain, &chainStatus, &certStruct->subject,
            (R_RSA_PUBLIC_KEY *)NULL, FALSE, ripemDatabase)) != (char *)NULL)
        break;
      if (chainStatus.overall != 0) {
        /* Decode the certificate at the "bottom" of the chain.  Don't check
             for error decoding since it already decoded sucessfully. */
        DERToCertificate
          ((unsigned char *)certChain.firstptr->dataptr, certStruct,
           (CertFieldPointers *)NULL);

        fprintf (certInfoStream, "%s: ",
                 GetCertStatusString (chainStatus.overall));
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

    if (!CheckKeyList (&RecipientList, ripemFlags->abortIfRecipUnknown)) {
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

