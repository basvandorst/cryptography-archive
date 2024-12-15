/* Defines main for rcerts command line certificate manager.
 */

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#ifdef SVRV32
#include <sys/types.h>
#endif /* SVRV32 */
#include "global.h"
#include "rsaref.h"
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

#define LEN_OF_MONTH(year, month) \
  ((((year) % 4) || (month) != 2) ? MONTH_LENS[((month)-1)] : 29)

#define SECONDS_IN_DAY ((UINT4)3600 * (UINT4)24)

static unsigned int MONTH_LENS[] =
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/* Note: this is normalized so that 12 * SECONDS_IN_MONTH is 365 days. */
#define SECONDS_IN_MONTH \
  ((UINT4)((UINT4)365 * (UINT4)24 * (UINT4)3600) / (UINT4)12)

typedef struct {
  char *keyToPrivateKey;
  char *homeDir;
  char *debugFilename;
  TypList userList;
} RCERTSArgs;

typedef struct {
  TypList certChain;                        /* currently selected cert chain */
  ChainStatusInfo chainStatus;
  CertificateStruct *certStruct;     /* loaded with the selected user's cert */
  CertFieldPointers fieldPointers;
} RCERTSState;

#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

static void MainExit P((int stat));
static char *CrackCommandLine
  P((int, char **, RIPEMInfo *, RCERTSArgs *, RIPEMDatabase *));
static char *CrackKeyServerInfo P((char *, RIPEMDatabase *));
static unsigned int GetPasswordToPrivateKey
  P((unsigned char *, unsigned int, char *));
static char *DoMenu P((RCERTSState *, RIPEMInfo *, RIPEMDatabase *));
static void GetInputLine P((char *, unsigned int, char *));
static char *SelectUser
  P((RCERTSState *, RIPEMInfo *, RIPEMDatabase *));
static char *ViewUserDetail P((RCERTSState *, RIPEMInfo *));
static char *RequestCRLs P((RCERTSState *, RIPEMInfo *));
static char *ModifyChainLenAllowed
  P((RCERTSState *, RIPEMInfo *, RIPEMDatabase *));
static char *RevokeSelectedUser
  P((RCERTSState *, RIPEMInfo *, RIPEMDatabase *));
static char *EnableStandardIssuers P((RIPEMInfo *, RIPEMDatabase *));
static char *RenewCRL P((RCERTSState *, RIPEMInfo *, RIPEMDatabase *));
static char *PublishCRL P((RIPEMInfo *, RIPEMDatabase *));
static BOOL FindSubjectPublicKey
  P((TypList *, DistinguishedNameStruct *, R_RSA_PUBLIC_KEY *,
     CertificateStruct *certStruct));
static char *CertStatusString P((int));
static char *EnableIssuer
  P((RIPEMInfo *, DistinguishedNameStruct *, unsigned char *,
     unsigned int, unsigned char *, unsigned int, unsigned int, 
     unsigned int, RIPEMDatabase *));
static void GetDateFromTime P((char *, UINT4));

/* This is needed only by getsys.c so set to not initialized and
     RandomStructPointer will be ignored. */
int RandomStructInitialized = 0;
R_RANDOM_STRUCT *RandomStructPointer;        /* ignored when not initialized */

#ifdef __BORLANDC__
extern unsigned _stklen = 12192;  /* Increase stack size for Borland C */
#endif

extern char USAGE_MESSAGE_LINE1[];
extern char *RCERTS_USAGE_MESSAGE[];

/* The following initialized C arrays represent the Low Assurance CA
   public key modulus and exponent encoded as byte vectors, most
   significant byte first.
 */

static unsigned char LOW_ASSURANCE_MODULUS[96] = {
  0xb0, 0xb4, 0x0e, 0x9a, 0x3a, 0x46, 0x4e, 0x87, 0x03, 0xff, 0xb8, 0xdb,
  0xca, 0xd8, 0xaf, 0x41, 0xf3, 0xc3, 0xf4, 0x13, 0x1c, 0xf6, 0x57, 0x1e,
  0x39, 0xa5, 0x35, 0x49, 0xb4, 0x20, 0x94, 0xdc, 0x92, 0xf8, 0xee, 0x1e,
  0xa1, 0x03, 0x5f, 0x94, 0x21, 0x2b, 0x75, 0x1a, 0x3b, 0x07, 0x44, 0x5d,
  0xbd, 0xd6, 0xef, 0x4d, 0x7e, 0x23, 0x00, 0xf4, 0x2c, 0xf5, 0x73, 0x47,
  0x90, 0xbc, 0xe8, 0xba, 0x51, 0x7f, 0xa8, 0x19, 0xee, 0xf7, 0x5f, 0x17,
  0x46, 0xdc, 0xe5, 0xff, 0xd1, 0x23, 0xfe, 0x64, 0x2e, 0x68, 0x94, 0xb3,
  0x81, 0xde, 0x5a, 0x40, 0x28, 0x8d, 0xd6, 0xd7, 0x14, 0xaf, 0x84, 0xef
};

static unsigned char LOW_ASSURANCE_EXPONENT[3] = { 0x01, 0x00, 0x01 };

/* The following initialized C arrays represent the Commercial CA
   public key modulus and exponent encoded as byte vectors, most
   significant byte first.
 */

unsigned char COMMERCIAL_CA_MODULUS[125] = {
  0xa4, 0xfb, 0x81, 0x62, 0x7b, 0xce, 0x10, 0x27, 0xdd, 0xe8, 0xf7, 0xbe,
  0x6c, 0x6e, 0xc6, 0x70, 0x99, 0xdb, 0xb8, 0xd5, 0x05, 0x03, 0x69, 0x28,
  0x82, 0x9c, 0x72, 0x7f, 0x96, 0x3f, 0x8e, 0xec, 0xac, 0x29, 0x92, 0x3f,
  0x8a, 0x14, 0xf8, 0x42, 0x76, 0xbe, 0xbd, 0x5d, 0x03, 0xb9, 0x90, 0xd4,
  0xd0, 0xbc, 0x06, 0xb2, 0x51, 0x33, 0x5f, 0xc4, 0xc2, 0xbf, 0xb6, 0x8b,
  0x8f, 0x99, 0xb6, 0x62, 0x22, 0x60, 0xdd, 0xdb, 0xdf, 0x20, 0x82, 0xb4,
  0xca, 0xa2, 0x2f, 0x2d, 0x50, 0xed, 0x94, 0x32, 0xde, 0xe0, 0x55, 0x8d,
  0xd4, 0x68, 0xe2, 0xe0, 0x4c, 0xd2, 0xcd, 0x05, 0x16, 0x2e, 0x95, 0x66,
  0x5c, 0x61, 0x52, 0x38, 0x1e, 0x51, 0xa8, 0x82, 0xa1, 0xc4, 0xef, 0x25,
  0xe9, 0x0a, 0xe6, 0x8b, 0x2b, 0x8e, 0x31, 0x66, 0xd9, 0xf8, 0xd9, 0xfd,
  0xbd, 0x3b, 0x69, 0xd9, 0xeb
};

unsigned char COMMERCIAL_CA_EXPONENT[3] = { 0x01, 0x00, 0x01 };

#ifdef MACTC  /* rwo */
clock_t Time0, Time1;
#endif

int main (argc, argv)
int argc;
char *argv[];
{
  RCERTSArgs rcertsArgs;
  RCERTSState state;
  RIPEMDatabase ripemDatabase;
  RIPEMInfo ripemInfo;
  int j;
  char *errorMessage, password[MAX_PASSWORD_SIZE], buffer[8];
  unsigned int passwordLen;
  BOOL alreadyPrintedError = FALSE;

  /* Set ripemInfo to initial state.  Also initialize debugStream
       since the constructor doesn't. */
  RIPEMInfoConstructor (&ripemInfo);
  ripemInfo.debug = 0;
  ripemInfo.debugStream = (FILE *)NULL;

  /* Pre-zeroize all pointers if rcertsArgs. */
  R_memset ((POINTER)&rcertsArgs, 0, sizeof (rcertsArgs));

  /* Initialize state */
  R_memset ((POINTER)&state, 0, sizeof (state));
  
  RIPEMDatabaseConstructor (&ripemDatabase);

#ifdef MACTC
  setvbuf(stderr, NULL, _IONBF, 0);
  fprintf(stderr, "Off we go...\n");
  argc = ccommand(&argv);
  Time0 = clock();
#endif

  /* For error, break to end of do while (0) block. */
  do {
    /* Append the RIPEM_VERSION to the end of the first usage message line
         so it will appear if we have to print the usage. */
    strcat (USAGE_MESSAGE_LINE1, RIPEM_VERSION);

    /* Parse the command line. */
    if ((errorMessage = CrackCommandLine
         (argc, argv, &ripemInfo, &rcertsArgs, &ripemDatabase))
        != (char *)NULL) {
      usage (errorMessage, RCERTS_USAGE_MESSAGE);
      alreadyPrintedError = TRUE;
      break;
    }

    /* Open the debug file.
     */
    if (rcertsArgs.debugFilename != (char *)NULL) {
      if ((ripemInfo.debugStream = fopen
           (rcertsArgs.debugFilename, "w")) == (FILE *)NULL) {
        sprintf (ripemInfo.errMsgTxt,
                 "Can't open debug file %s.", rcertsArgs.debugFilename);
        errorMessage = ripemInfo.errMsgTxt;
        break;
      }
    }
    else
      ripemInfo.debugStream = stderr;

    /* We now have a debug stream and the user's choice of home dir.
       Make sure the home dir exists and that it has an ending directory
         separator.  This will try to get a default name is none is specified
         yet.
     */
    if ((errorMessage = EstablishRIPEMHomeDir
         (&rcertsArgs.homeDir, &ripemInfo)) != (char *)NULL)
      break;

    /* Now we have a home dir, so open the database. */
    if ((errorMessage = InitRIPEMDatabase
         (&ripemDatabase, rcertsArgs.homeDir, &ripemInfo)) != (char *)NULL)
      break;

    /* Clear the parameters so that users typing "ps" or "w" can't
     * see what parameters we are using.
     */
    for (j = 1; j < argc; j++)
      R_memset ((POINTER)argv[j], 0, strlen (argv[j]));

    ripemInfo.userList = &rcertsArgs.userList;

    /* Get the password.  Prompt twice if generating. */
    passwordLen = GetPasswordToPrivateKey
      ((unsigned char *)password, sizeof (password),
       rcertsArgs.keyToPrivateKey);

    /* Log in the user and load the preferences, checking for special errors.
     */
    if ((errorMessage = LoginUser
         (&ripemInfo, &ripemDatabase, (unsigned char *)password, passwordLen))
        != (char *)NULL) {
      /* Consider ERR_SELF_SIGNED_CERT_NOT_FOUND an error */
      if (strcmp (errorMessage, ERR_PREFERENCES_NOT_FOUND) == 0) {
        /* This is OK, just issue a warning. */
        fputs ("Warning: User preferences were not found.  RIPEM will use defaults.\n", stderr);
        errorMessage = (char *)NULL;
      }
      else if (strcmp (errorMessage, ERR_PREFERENCES_CORRUPT) == 0) {
        /* Issuer alert and continue. */
        fputs ("ALERT: Preference information has been corrupted. RIPEM will use defaults.\n", stderr);
        errorMessage = (char *)NULL;
      }
      else
        /* Other errors. */
        break;
    }

    /* Allocate the certStruct which will hold the selected user. */
    if ((state.certStruct = (CertificateStruct *)malloc
         (sizeof (*state.certStruct))) == (CertificateStruct *)NULL) {
      errorMessage = ERROR_MALLOC;
      break;
    }
    
    /* Initialize the state to the logged in user.  Don't expect the
         chain select to fail.
     */
    if ((errorMessage = SelectCertChain
         (&ripemInfo, &state.certChain, &state.chainStatus,
          &ripemInfo.userDN, &ripemInfo.publicKey, TRUE, &ripemDatabase))
        != (char *)NULL)
      break;
    /* Decode the user's cert into state's certStruct */
    DERToCertificate
      ((unsigned char *)state.certChain.firstptr->dataptr,
       state.certStruct, &state.fieldPointers);

    if (state.chainStatus.overall == CERT_CRL_OUT_OF_SEQUENCE) {
      puts ("Note that the CRL issued by you found in the database is out of sequence.");
      puts ("  This means that CRLs recently issued by you may have been deleted by");
      puts ("  someone to back up to an earlier CRL without their revocation.");
      puts ("  You may need to replace some revocations.");

      /* Make the user view this message before continuing */
      GetInputLine (buffer, sizeof (buffer), "  Hit <ENTER> to continue:");
    }

    if ((errorMessage = DoMenu
         (&state, &ripemInfo, &ripemDatabase)) != (char *)NULL)
      break;
  } while (0);
      
  R_memset ((POINTER)password, 0, sizeof (password));
  if (rcertsArgs.keyToPrivateKey)
    R_memset
    ((POINTER)rcertsArgs.keyToPrivateKey, 0,
     strlen (rcertsArgs.keyToPrivateKey));
  RIPEMInfoDestructor (&ripemInfo);
  RIPEMDatabaseDestructor (&ripemDatabase);
  if (ripemInfo.debugStream != (FILE *)NULL && ripemInfo.debugStream != stderr)
    fclose (ripemInfo.debugStream);

  /* Free all the pointers in rcertsArgs.
   */
  free (rcertsArgs.keyToPrivateKey);
  free (rcertsArgs.homeDir);
  free (rcertsArgs.debugFilename);
  FreeList (&rcertsArgs.userList);

  FreeList (&state.certChain);
  free (state.certStruct);

  if (errorMessage != (char *)NULL && !alreadyPrintedError) {
    fputs (errorMessage, stderr);
    fputc ('\n', stderr);
  }

  MainExit (errorMessage == (char *)NULL ? 0 : 1);
  
  /*NOTREACHED*/
  return (0);  /* to shut up compiler warnings */
}

/* End the program and return a returncode to the system.
 */
static void MainExit (status) 
int status;
{
#ifdef MACTC
  double x;
  (void)fflush((FILE *)NULL);
  Time1 = clock();
  x = (Time1 - Time0)/60.0;
  fprintf(stderr, "Exit %d; Elapsed : %5.2f seconds.", status, x);
#endif
  exit (status);
}
  
static char *CrackCommandLine
  (argc, argv, ripemInfo, rcertsArgs, ripemDatabase)
int argc;
char *argv[];
RIPEMInfo *ripemInfo;
RCERTSArgs *rcertsArgs;
RIPEMDatabase *ripemDatabase;
{
  extern char *optarg;
  extern int optind, optsp;

  int got_username = FALSE;
  int got_key_to_priv_key = FALSE;
  BOOL cracking=TRUE;
  int j, ch, myargc[2], iarg;
  char *errorMessage = NULL, *cptr, **myargv[2], *env_args;
  char *key_sources = "sf";
  char *usernameStr, *key_server_str=NULL;
  TypList mylist;
  TypListEntry *entry;

  InitList (&mylist);

  /* We crack a command line twice:  
   * First, we crack the pseudo-command line in the environment variable
   *   RIPEM_ARGS (if any).  This environment variable exists to make it
   *   easy for users who don't want to type the otherwise lengthy
   *   command line, and is an alternative to the other individual 
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
           (myargc[iarg], myargv[iarg], "p:s:P:S:u:k:D:Z:y:Y:H:")) != -1) {
      switch (ch) {
        case '?':
          return ("Unrecognized command line option.");
                      
        case 'u':       /* My username */
          StrCopyAlloc(&usernameStr,optarg);
          got_username = TRUE;
          break;

        case 'p':       /* Public key filename */
          if ((errorMessage = AddKeySourceFilename
               (&ripemDatabase->pubKeySource, optarg)) != (char *)NULL)
            return (errorMessage);
          break;
          
        case 'P':       /* Public key output filename */
          return ("-P is obsolete. Public keys are written to \"pubkeys\" in the RIPEM home dir.");

        case 's':       /* Secret (private) key filename */
          if ((errorMessage = AddKeySourceFilename
               (&ripemDatabase->privKeySource, optarg)) != (char *)NULL)
            return (errorMessage);
          break;
          
        case 'S':       /* Private key output filename */
          return ("-S is obsolete. Private keys are written to \"privkey\" in the RIPEM home dir.");

        case 'y':       /* Name of public key server */
          StrCopyAlloc (&key_server_str, optarg);
          break;

        case 'Y':       /* Order of sources for keys (server vs. file) */
          StrCopyAlloc (&key_sources, optarg);
          break;

        case 'k':       /* Key to private key */
          StrCopyAlloc (&rcertsArgs->keyToPrivateKey, optarg);
          got_key_to_priv_key = TRUE;
          break;

        case 'H':       /* RIPEM home directory */
          StrCopyAlloc (&rcertsArgs->homeDir, optarg);
          break;

        case 'D':       /* Debug level */
          ripemInfo->debug = atoi (optarg);
          break;

        case 'Z':       /* Debug output file */
          StrCopyAlloc (&rcertsArgs->debugFilename, optarg);
          break;
      }
    }
  }

  /* Parse the -Y argument string (sources of key info) */

  for(j=0; j<MAX_KEY_SOURCES; j++) {
    switch(key_sources[j]) {
      case 's':
      case 'S':
        ripemDatabase->pubKeySource.origin[j] = KEY_FROM_SERVER;
        break;

      case 'f':
      case 'F':
        ripemDatabase->pubKeySource.origin[j] = KEY_FROM_FILE;
        break;
        
      case 'g':
      case 'G':
        ripemDatabase->pubKeySource.origin[j] = KEY_FROM_FINGER;
        break;
    
      default:
        ripemDatabase->pubKeySource.origin[j] = KEY_FROM_NONE;
        break;
    }
  }
  
  /* If we don't have the RIPEM home dir yet, look for the environment
       variable. */
  if (!rcertsArgs->homeDir) {
    /* Set cptr to the name from the environment, or to "" */
    GetEnvFileName (HOME_DIR_ENV, "", &cptr);

    if (*cptr != '\0')
      /* Found it */
      StrCopyAlloc (&rcertsArgs->homeDir, cptr);
  }

  /* Obtain the username if it wasn't specified. */

  if (!got_username)
    GetUserAddress (&usernameStr);
  
  /* Crack the username string (which can contain multiple aliases
   * separated by commas) into a list.
   */
  
  CrackLine (usernameStr, &rcertsArgs->userList);

  /* Obtain the name of the public key server. */
  if(!key_server_str)
    GetEnvAlloc(SERVER_NAME_ENV,&key_server_str);

  errorMessage = CrackKeyServerInfo (key_server_str, ripemDatabase);
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
           (&ripemDatabase->pubKeySource, (char *)entry->dataptr))
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
           (&ripemDatabase->privKeySource, (char *)entry->dataptr))
          != (char *)NULL)
        return (errorMessage);
    }
  }

  /* Special processing for the key to the private key:
   * A key of - means to read the key to the private key
   * from standard input.
   */
  if (got_key_to_priv_key) {
    if (strcmp (rcertsArgs->keyToPrivateKey, "-") == 0) {
#define PWLEN 256
      char line[PWLEN];

      fgets(line,PWLEN,stdin);
      StrCopyAlloc (&rcertsArgs->keyToPrivateKey, line);
      for (cptr = rcertsArgs->keyToPrivateKey; *cptr; cptr++) {
        if(*cptr=='\n' || *cptr=='\r')
          *cptr='\0';
      }
    }
  }

  return ((char *)NULL);
}

/* This is a copy of CrackKeyServer from ripemcmd.c with the ripemDatabase
     argument passed in instead of being global.
 */
static char *CrackKeyServerInfo (keyServerStr, ripemDatabase)
char *keyServerStr;
RIPEMDatabase *ripemDatabase;
{
  TypList name_list;
  TypListEntry *entry;
  TypServer *server_ent;
  char *cptr, *errmsg;
  
  InitList (&ripemDatabase->pubKeySource.serverlist);
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
      errmsg = AddToList
        (NULL, server_ent, sizeof (TypServer),
         &ripemDatabase->pubKeySource.serverlist);
      if(errmsg)
        return errmsg;
    }
  }
  return NULL;    
}

/* This is a copy of GetPasswordToPrivateKey from ripemcmd.c except
     verify and new are always false and also takes rcertsArgs keyToPrivateKey
     as an argument unstead of using a global variable.
 */
static unsigned int GetPasswordToPrivateKey
  (password, maxchars, keyToPrivateKey)
unsigned char *password;
unsigned int maxchars;
char *keyToPrivateKey;
{
  unsigned int pw_len = 0;
  BOOL got_pw = FALSE;
  char *cptr;

  if (keyToPrivateKey != (char *)NULL) {
    strncpy ((char *)password, keyToPrivateKey, maxchars);
    pw_len = (unsigned int)strlen ((char *)password);
    got_pw = TRUE;
  }

  if (!got_pw) {
    GetEnvAlloc (KEY_TO_PRIVATE_KEY_ENV, &cptr);
    if(cptr && *cptr) {
       strncpy ((char *)password, cptr, maxchars);
       pw_len = (unsigned int)strlen ((char *)password);
       got_pw = TRUE;
    }
  }

  if(!got_pw) {
    pw_len = GetPasswordFromUser
      ("Enter password to private key: ", FALSE, password, maxchars);
  }

  return (pw_len);
}

static char *DoMenu (state, ripemInfo, ripemDatabase)
RCERTSState *state;
RIPEMInfo *ripemInfo;
RIPEMDatabase *ripemDatabase;
{
  BOOL done;
  char command[8], *errorMessage = (char *)NULL;

  done = FALSE;
  while (!done) {
    puts ("");
    if (state->chainStatus.overall != 0) {
      printf ("The selected user is:\n  ");
      WritePrintableName (stdout, &state->certStruct->subject);
      printf ("\n");
    }
    puts ("S - Select user...");
    puts ("V - View detail for the selected user");
    puts ("C - request CRLs for the selected user...");
    puts ("M - Modify chain length allowed for the selected user...");
    puts ("R - Revoke the selected user...");
    puts ("E - Enable standard issuers...");
    puts ("N - reNew the CRL issued by you...");
    puts ("P - Publish the CRL issued by you in a CRL message...");
    puts ("Q - Quit");

    GetInputLine (command, sizeof (command), "  Enter choice:");

    switch (*command) {
    case 's':
    case 'S':
      errorMessage = SelectUser (state, ripemInfo, ripemDatabase);
      break;

    case 'v':
    case 'V':
      errorMessage = ViewUserDetail (state, ripemInfo);
      break;
      
    case 'c':
    case 'C':
      errorMessage = RequestCRLs (state, ripemInfo);
      break;
      
    case 'm':
    case 'M':
      errorMessage = ModifyChainLenAllowed (state, ripemInfo, ripemDatabase);
      break;

    case 'r':
    case 'R':
      errorMessage = RevokeSelectedUser (state, ripemInfo, ripemDatabase);
      break;

    case 'e':
    case 'E':
      errorMessage = EnableStandardIssuers (ripemInfo, ripemDatabase);
      break;
      
    case 'n':
    case 'N':
      errorMessage = RenewCRL (state, ripemInfo, ripemDatabase);
      break;
      
    case 'p':
    case 'P':
      errorMessage = PublishCRL (ripemInfo, ripemDatabase);
      break;
      
    case 'Q':
    case 'q':
      done = TRUE;
      break;
      
    case '\0':
      /* Blank line.  Just repeat the menu */
      break;

    default:
      puts ("ERROR: Unrecognized command.  Try again.");
      break;
    }
    if (errorMessage != (char *)NULL)
      /* Broke because of error. */
      break;
  }

  return (errorMessage);
}

/* Print the prompt, then read up to maxLineSize - 1 bytes and put into line,
     null terminated.  This is better than just calling gets, since gets
     does not check for overflow of the line buffer.
   This assumes maxLineSize is at least 1.
 */
static void GetInputLine (line, maxLineSize, prompt)
char *line;
unsigned int maxLineSize;
char *prompt;
{
  unsigned int i;
  
  puts (prompt);  
  fflush (stdout);

  fgets (line, maxLineSize, stdin);
  
  /* Replace the line terminator with a '\0'.
   */
  for (i = 0; line[i] != '\0'; i++) {
    if (line[i] == '\012' || line[i] == '\015' || i == (maxLineSize - 1)) {
      line[i] = '\0';
      return;
    }
  }
}

/* Prompt for a smart name and set the certChain and chainStatus in
     state for that user.  If there are multiple certificates that match
     the smart name, this will prompt the user for the correct one.
   If no chain can be found, this sets state->chainStatus.overall to 0
     and state->certChain is empty.  This also prints a message if no
     chain can be found.
 */
static char *SelectUser (state, ripemInfo, ripemDatabase)
RCERTSState *state;
RIPEMInfo *ripemInfo;
RIPEMDatabase *ripemDatabase;
{
  TypList selectedCerts, filteredCerts;
  TypListEntry *entry;
  CertificateStruct *filteredCertStruct = (CertificateStruct *)NULL;
  char *errorMessage = (char *)NULL, command[80], buffer[8];
  unsigned int filteredCertCount;
  BOOL foundAChain = FALSE, directCertOnly;

  InitList (&selectedCerts);
  InitList (&filteredCerts);

  /* For error, break to end of do while (0) block. */
  do {
    /* Allocate on the heap since it is so big. */
    if ((filteredCertStruct = (CertificateStruct *)malloc
         (sizeof (*filteredCertStruct))) == (CertificateStruct *)NULL) {
      errorMessage = ERROR_MALLOC;
      break;
    }

    GetInputLine
      (command, sizeof (command),
       "  Enter name of user to select (blank to cancel):");
    if (! *command)
      /* User cancelled */
      break;

    GetInputLine
      (buffer, sizeof (buffer),
       "  Hit <ENTER> to allow any certificate chain (the default) or enter D to\n    make sure the user is certified directly by you:");
    directCertOnly = (*buffer == 'd' || *buffer == 'D');

    /* Clear previous info in the state. */
    FreeList (&state->certChain);
    state->chainStatus.overall = 0;
    
    /* Select all certs which match this user name. */
    if ((errorMessage = GetCertsBySmartname
         (ripemDatabase, &selectedCerts, command, ripemInfo)) != (char *)NULL)
      break;

    /* Move certs from selectedCerts to filteredCerts, omitting ones with
         duplicate subject/public key (such as expired vs. current certs).
       Also get the count of filtered certs.
     */
    filteredCertCount = 0;
    for (entry = selectedCerts.firstptr; entry; entry = entry->nextptr) {
      /* Decode the certificate into the state's certStruct */
      if (DERToCertificate
          ((unsigned char *)entry->dataptr, state->certStruct,
           (CertFieldPointers *)NULL) < 0) {
        /* Error decoding.  Just issue a warning to debug stream and try
             the next cert. */
        if (ripemInfo->debug > 1)
          fprintf (ripemInfo->debugStream,
                   "Warning: Cannot decode certificate from database.\n");
        continue;
      }

      if (!FindSubjectPublicKey
          (&filteredCerts, &state->certStruct->subject,
           &state->certStruct->publicKey, filteredCertStruct)) {
        /* This subject/public key has not already been added to
             the filteredCerts, so add.  We will "transfer" from
             selectedCerts by setting the entry to NULL in selectedCerts
             so it won't be freed. */
        if ((errorMessage = AddToList
             ((TypListEntry *)NULL, entry->dataptr, entry->datalen,
              &filteredCerts)) != (char *)NULL)
          break;
        entry->dataptr = NULL;
        entry->datalen = 0;
        ++filteredCertCount;
      }
    }
    if (errorMessage != (char *)NULL)
      /* Broke because of error */
      break;

    if (filteredCertCount == 0) {
      /* None found */
      printf ("Cannot find any certificate for %s.\n", command);
      break;
    }

    if (filteredCertCount > 1)
      printf
    ("There are up to %u entries which match that user name. Please choose:\n",
       filteredCertCount);

    /* Let the user pick the entry.  If there is one entry, this loop will
         automatically use it.
     */
    for (entry = filteredCerts.firstptr; entry; entry = entry->nextptr) {
      DERToCertificate
        ((unsigned char *)entry->dataptr, filteredCertStruct,
         (CertFieldPointers *)NULL);

      /* Clear previous cert chain and get the chain for this cert. */
      FreeList (&state->certChain);
      if ((errorMessage = SelectCertChain
           (ripemInfo, &state->certChain, &state->chainStatus,
            &filteredCertStruct->subject, &filteredCertStruct->publicKey,
            directCertOnly, ripemDatabase)) != (char *)NULL)
        break;
      /* Note that we are done with filteredCertStruct now. */

      if (state->chainStatus.overall == 0)
        /* A cert chain for this cert could not be completed.  In this
             case filteredCertCount is too high. */
        break;

      /* Set foundAChain so we know whether there was something for
           the user to reject. */
      foundAChain = TRUE;
      
      /* Decode the selected user's cert into state's certStruct */
      DERToCertificate
        ((unsigned char *)state->certChain.firstptr->dataptr,
         state->certStruct, &state->fieldPointers);

      if (filteredCertCount == 1)
        /* There is only one possibility, so don't prompt the user. */
        break;

      /* Display this entry to the user and let them choose.
       */
      printf ("\nCertificate for:\n  ");
      WritePrintableName (stdout, &state->certStruct->subject);
      printf ("\n");
      if (state->certChain.firstptr == state->certChain.lastptr)
        /* There is only one certificate in the chain. */
        printf ("  certified directly by you.\n");
      else {
        /* This chain is certified via the subject at the top of the
             chain (end of the list).  Use filteredCertStruct show the
             subject at the top of the chain.
         */
        DERToCertificate
          ((unsigned char *)state->certChain.lastptr->dataptr,
           filteredCertStruct, (CertFieldPointers *)NULL);
        printf ("  certified indirectly through:\n  ");
        WritePrintableName (stdout, &filteredCertStruct->subject);
        printf ("\n");
      }

      GetInputLine
        (command, sizeof (command),
         "  Choose: Select, Quit or <ENTER> to see next entry:");
      if (*command == 's' || *command == 'S')
        break;
      else if (*command == 'q' || *command == 'Q') {
        state->chainStatus.overall = 0;
        /* certChain will be freed below */
        break;
      }
      /* Default is to continue */
    }
    if (errorMessage != (char *)NULL)
      /* Broke because of error. */
      break;

    if (state->chainStatus.overall == 0) {
      /* No chain, so free the certChain list */
      FreeList (&state->certChain);

      if (!foundAChain) {
        /* chainStatus.overall is zero because there are no valid chains. */
        if (directCertOnly)
          puts ("Could not find certificate for this user issued directly by you.");
        else
          puts ("Could not find any valid certificate chain.");
      }
    }
  } while (0);

  FreeList (&selectedCerts);
  FreeList (&filteredCerts);
  free (filteredCertStruct);
  return (errorMessage);
}

static char *ViewUserDetail (state, ripemInfo)
RCERTSState *state;
RIPEMInfo *ripemInfo;
{
  CertificateStruct *localCertStruct = (CertificateStruct *)NULL;
  TypListEntry *entry;
  char *errorMessage = (char *)NULL, buffer[8], startDate[9], endDate[9];
  unsigned char digest[16];
  unsigned int chainLenAllowed, i, digestLen;

  if (state->chainStatus.overall == 0) {
    puts ("First use \"Select user\" to select the user to view.");
    return ((char *)NULL);
  }

  /* For error, break to end of do while (0) block. */
  do {
    printf ("The selected user is:\n  ");
    WritePrintableName (stdout, &state->certStruct->subject);
    printf ("\n");

    if (state->certChain.firstptr == state->certChain.lastptr) {
      /* There is only one certificate in the chain. */

      /* Get validity dates. */
      GetDateFromTime (startDate, state->certStruct->notBefore);
      GetDateFromTime (endDate, state->certStruct->notAfter);
      
      if (R_memcmp
          ((POINTER)&state->certStruct->publicKey,
           (POINTER)&ripemInfo->publicKey, sizeof (ripemInfo->publicKey))
          == 0) {
        printf ("This is your own self-signed certificate.\n");
        /* For one cert, the overall is the same as the individual status. */
        printf ("It has a certificate status of %s\n",
                CertStatusString (state->chainStatus.overall));
        printf
          ("  with a validity period from %s to %s GMT.\n", startDate,
           endDate);

        /* Print the self-signed cert digest.
         */
        R_DigestBlock
          (digest, &digestLen, state->fieldPointers.innerDER,
           state->fieldPointers.innerDERLen,
           state->certStruct->digestAlgorithm);
        printf ("self-signed certificate digest:");
        for (i = 0; i < digestLen; ++i)
          printf (" %02X", (int)digest[i]);
        printf ("\n");
      }
      else {
        printf ("This user is certified directly by you.\n");
        /* For one cert, the overall is the same as the individual status. */
        printf ("The certificate for this user has a status of %s\n",
                CertStatusString (state->chainStatus.overall));
        printf
          ("  with a validity period from %s to %s GMT.\n", startDate,
           endDate);

        /* Get chain length allowed.
         */
        if ((errorMessage = GetPublicKeyDigest
             (digest, &state->certStruct->publicKey)) != (char *)NULL)
          break;
        chainLenAllowed = GetChainLenAllowed (ripemInfo, digest);
        if (chainLenAllowed == 0)
          puts
            ("You do not allow this user to make certificates for others.");
        else
          printf
          ("You allow this user to make certificate chains up to length %u.\n",
           chainLenAllowed);
      }
    }
    else {
      /* There is more than one certificate in the chain. */
      printf ("This user has overall certificate chain status of %s.\n",
              CertStatusString (state->chainStatus.overall));

      /* Allocate a certStruct and show the issuers up the chain. */
      if ((localCertStruct = (CertificateStruct *)malloc
           (sizeof (*localCertStruct))) == (CertificateStruct *)NULL) {
        errorMessage = ERR_MALLOC;
        break;
      }

      /* Make i track as we go through the cert list.
       */
      for (entry = state->certChain.firstptr, i = 0;
           entry != (TypListEntry *)NULL;
           entry = entry->nextptr, ++i) {
        DERToCertificate
          ((unsigned char *)entry->dataptr, localCertStruct,
           (CertFieldPointers *)NULL);

        if (entry != state->certChain.lastptr) {
          if (i == 0)
            /* The first cert in the chain */
            printf
              ("This user has a certificate with status %s issued by:\n  ",
               CertStatusString (state->chainStatus.individual[i]));
          else
            /* For cert indexes above the first show that it is an issuer cert.
             */
            printf
            ("And this issuer has a certificate with status %s issued by:\n  ",
             CertStatusString (state->chainStatus.individual[i]));
          WritePrintableName (stdout, &localCertStruct->issuer);
          printf ("\n");
        }
        else {
          printf
          ("And this issuer has a certificate with status %s issued by you\n",
           CertStatusString (state->chainStatus.individual[i]));
        }

        /* Now show the validity period for the cert.
         */
        GetDateFromTime (startDate, localCertStruct->notBefore);
        GetDateFromTime (endDate, localCertStruct->notAfter);
        printf
          ("  with a validity period from %s to %s GMT.\n", startDate,
           endDate);
      }
    }

    /* Let the user view the info before continuing */
    GetInputLine (buffer, sizeof (buffer), "  Hit <ENTER> to continue:");
  } while (0);

  free (localCertStruct);
  return (errorMessage);
}

static char *RequestCRLs (state, ripemInfo)
RCERTSState *state;
RIPEMInfo *ripemInfo;
{
  FILE *outStream;
  TypListEntry *entry;
  CertificateStruct *localCertStruct = (CertificateStruct *)NULL;
  char *errorMessage, filename[256];
  unsigned char *partOut;
  unsigned int partOutLen;
  
  if (state->chainStatus.overall == 0) {
    puts ("First use \"Select user\" to select the user to request CRLs for.");
    return ((char *)NULL);
  }

  if (state->certChain.firstptr == state->certChain.lastptr) {
    /* There is only one certificate in the chain. */
    puts ("The selected user is certified directly by you, so no CRL retrieval is needed.");
    return ((char *)NULL);
  }
  
  GetInputLine
    (filename, sizeof (filename),
     "  Enter output filename for CRL retrieval request:");
  if ((outStream = fopen (filename, "w")) == (FILE *)NULL)
    return ("Cannot open CRL request file.");

  /* For error, break to end of do while (0) block. */
  do {
    /* Allocate the localCertStruct on the heap because it's big. */
    if ((localCertStruct = (CertificateStruct *)malloc
         (sizeof (*localCertStruct))) == (CertificateStruct *)NULL) {
      errorMessage = ERROR_MALLOC;
      break;
    }

    if ((errorMessage = RIPEMRequestCRLsInit
         (ripemInfo, &partOut, &partOutLen)) != (char *)NULL)
      break;
    fwrite (partOut, 1, partOutLen, outStream);

    /* Write out issuer names for all certs in the chain but the
         top, which is the logged in user.  We have already made
         sure there is at least one certificate in the chain.
     */
    for (entry = state->certChain.firstptr; entry != state->certChain.lastptr;
         entry = entry->nextptr) {
      /* Get issuer name.  Note DERToCertificate won't return an error
           since it was already successfully decoded.
       */
      DERToCertificate
        ((unsigned char *)entry->dataptr, localCertStruct,
         (CertFieldPointers *)NULL);

      if ((errorMessage = RIPEMRequestCRLsUpdate
           (ripemInfo, &partOut, &partOutLen, &localCertStruct->issuer))
          != (char *)NULL)
        break;
      fwrite (partOut, 1, partOutLen, outStream);
    }
    if (errorMessage != (char *)NULL)
      /* Broke loop because of error. */
      break;

    if ((errorMessage = RIPEMRequestCRLsFinal
         (ripemInfo, &partOut, &partOutLen)) != (char *)NULL)
      break;
    fwrite (partOut, 1, partOutLen, outStream);
  } while (0);

  fclose (outStream);
  free (localCertStruct);
  return (errorMessage);
}

static char *ModifyChainLenAllowed (state, ripemInfo, ripemDatabase)
RCERTSState *state;
RIPEMInfo *ripemInfo;
RIPEMDatabase *ripemDatabase;
{
  unsigned char publicKeyDigest[16];
  unsigned int chainLenAllowed;
  char *errorMessage, command[80];
  
  if (state->chainStatus.overall == 0) {
    puts ("First use \"Select user\" to select the user you want to modify.");
    puts ("Be sure to choose \"D\" for selecting a direct certificate.");
    return ((char *)NULL);
  }

  if (state->certChain.firstptr != state->certChain.lastptr) {
    /* There is more than one certificate in the chain. */
    puts ("This user's certificate is issued by someone else.  You can only set the");
    puts ("  chain length allowed for a user certified directly by you.");
    puts ("Do \"Select user\" again and choose \"D\" for selecting a direct certificate.");
    return ((char *)NULL);
  }
  
  if (R_memcmp
      ((POINTER)&state->certStruct->publicKey,
       (POINTER)&ripemInfo->publicKey, sizeof (ripemInfo->publicKey)) == 0) {
    puts ("This is your own self-signed certificate.  There is no need to modify the");
    puts ("  chain length allowed.");
    return ((char *)NULL);
  }

  /* Get current chain length allowed.
   */
  if ((errorMessage = GetPublicKeyDigest
       (publicKeyDigest, &state->certStruct->publicKey)) != (char *)NULL)
    return (errorMessage);
  chainLenAllowed = GetChainLenAllowed (ripemInfo, publicKeyDigest);
  if (chainLenAllowed == 0)
    puts
     ("You do not currently allow this user to make certificates for others.");
  else
    printf
    ("You currently allow this user to make certificate chains up to length %u.\n",
     chainLenAllowed);

  GetInputLine
    (command, sizeof (command),
     "  Enter new chain length allowed, 0 to not allow chains, or blank to cancel:");

  if (! *command)
    /* User cancelled. */
    return ((char *)NULL);

  /* Set chainLenAllowed to new value.  If sscanf doesn't return 1, it
       means junk was entered. */
  if (sscanf (command, "%u", &chainLenAllowed) != 1) {
    puts ("You did not enter a number.  Chain length allowed was not modified.");
    return ((char *)NULL);
  }

  /* Set the new value and save the preferences. */
  return (SetChainLenAllowed
          (ripemInfo, publicKeyDigest, chainLenAllowed, ripemDatabase));
}

static char *RevokeSelectedUser (state, ripemInfo, ripemDatabase)
RCERTSState *state;
RIPEMInfo *ripemInfo;
RIPEMDatabase *ripemDatabase;
{
  char command[80], *errorMessage;
  unsigned int validityMonths;
  UINT4 now;
  
  if (state->chainStatus.overall == 0) {
    puts ("First use \"Select user\" to select the user you want to revoke.");
    puts ("Be sure to choose \"D\" for selecting a direct certificate.");
    return ((char *)NULL);
  }

  if (state->certChain.firstptr != state->certChain.lastptr) {
    /* There is more than one certificate in the chain. */
    puts ("This user's certificate is issued by someone else.  You can only revoke a user");
    puts ("  who is certified directly by you.");
    puts ("Do \"Select user\" again and choose \"D\" for selecting a direct certificate.");
    return ((char *)NULL);
  }
  
  if (R_memcmp
      ((POINTER)&state->certStruct->publicKey,
       (POINTER)&ripemInfo->publicKey, sizeof (ripemInfo->publicKey)) == 0) {
    puts ("This is your own self-signed certificate.  You cannot place yourself on the");
    puts ("  CRL issued by you.");
    return ((char *)NULL);
  }

  if (state->chainStatus.overall == CERT_REVOKED) {
    puts ("This user is already revoked.");
    return ((char *)NULL);
  }

  puts ("This will revoke the selected user by adding an entry to the CRL issued by");
  puts ("  you.  It will also renew the CRL.  Once a user is revoked it cannot be");
  puts ("  removed from the CRL until the certificate you issued for the user expires.");
  puts ("If you have never issued a CRL, this will create a new one.");
  
  GetInputLine
    (command, sizeof (command),
     "  Enter the number of months the new CRL will be valid, or blank to cancel:");

  if (! *command)
    /* User cancelled. */
    return ((char *)NULL);

  /* Set validityMonths to new value.  If sscanf doesn't return 1, it
       means junk was entered. */
  if (sscanf (command, "%u", &validityMonths) != 1) {
    puts ("You did not enter a number.  The CRL was not renewed.");
    return ((char *)NULL);
  }
  if (validityMonths == 0) {
    puts ("Validity months must be one or more.  The CRL was not renewed.");
    return ((char *)NULL);
  }

  /* Get the present time so we can compute the nextUpdate time and update
       the CRL with the serialNumber to revoke. */
  R_time (&now);
  if ((errorMessage = RIPEMUpdateCRL
       (ripemInfo, now + (UINT4)validityMonths * SECONDS_IN_MONTH,
        state->certStruct->serialNumber,
        sizeof (state->certStruct->serialNumber), ripemDatabase))
      != (char *)NULL)
    return (errorMessage);

  puts ("The user has been revoked.  Since the certification status has now changed,");
  puts ("  you must use \"Select User\" to re-select the user's certificate chain.");
  /* Make the user view this message before continuing */
  GetInputLine (command, sizeof (command), "  Hit <ENTER> to continue:");
  
  /* Clear the current user from the state. */
  FreeList (&state->certChain);
  state->chainStatus.overall = 0;

  return ((char *)NULL);
}

static char *EnableStandardIssuers (ripemInfo, ripemDatabase)
RIPEMInfo *ripemInfo;
RIPEMDatabase *ripemDatabase;
{
  DistinguishedNameStruct name;
  char command[8];

  puts ("");
  puts ("L - Low Assurance Certification Authority...");
  puts ("C - Commercial Certification Authority...");

  GetInputLine
    (command, sizeof (command),
     "  Select the issuer to enable (blank to cancel):");

  if (! *command)
    /* User cancel */
    return ((char *)NULL);
  
  switch (*command) {
  case 'l':
  case 'L':
    InitDistinguishedNameStruct (&name);
    
    strcpy (name.AVAValues[0], "US");
    name.AVATypes[0] = ATTRTYPE_COUNTRYNAME;
    name.RDNIndexStart[0] = name.RDNIndexEnd[0] = 0;

    strcpy (name.AVAValues[1], "RSA Data Security, Inc.");
    name.AVATypes[1] = ATTRTYPE_ORGANIZATIONNAME;
    name.RDNIndexStart[1] = name.RDNIndexEnd[1] = 1;

    strcpy (name.AVAValues[2], "Low Assurance Certification Authority");
    name.AVATypes[2] = ATTRTYPE_ORGANIZATIONALUNITNAME;
    name.RDNIndexStart[2] = name.RDNIndexEnd[2] = 2;

    /* Note: multiplying the byte size by 8 to get the modulusBits only
         works because all 8 bits of the MSB are used. */
    return (EnableIssuer
            (ripemInfo, &name, LOW_ASSURANCE_MODULUS,
             sizeof (LOW_ASSURANCE_MODULUS),
             LOW_ASSURANCE_EXPONENT, sizeof (LOW_ASSURANCE_EXPONENT),
             8 * sizeof (LOW_ASSURANCE_MODULUS), 2, ripemDatabase));

  case 'c':
  case 'C':
    InitDistinguishedNameStruct (&name);
    
    strcpy (name.AVAValues[0], "US");
    name.AVATypes[0] = ATTRTYPE_COUNTRYNAME;
    name.RDNIndexStart[0] = name.RDNIndexEnd[0] = 0;

    strcpy (name.AVAValues[1], "RSA Data Security, Inc.");
    name.AVATypes[1] = ATTRTYPE_ORGANIZATIONNAME;
    name.RDNIndexStart[1] = name.RDNIndexEnd[1] = 1;

    strcpy (name.AVAValues[2], "Commercial Certification Authority");
    name.AVATypes[2] = ATTRTYPE_ORGANIZATIONALUNITNAME;
    name.RDNIndexStart[2] = name.RDNIndexEnd[2] = 2;

    /* Note: multiplying the byte size by 8 to get the modulusBits only
         works because all 8 bits of the MSB are used. */
    return (EnableIssuer
            (ripemInfo, &name, COMMERCIAL_CA_MODULUS,
             sizeof (COMMERCIAL_CA_MODULUS),
             COMMERCIAL_CA_EXPONENT, sizeof (COMMERCIAL_CA_EXPONENT),
             8 * sizeof (COMMERCIAL_CA_MODULUS), 2, ripemDatabase));

  default:
    puts ("ERROR: Unrecognized selection.");
    return ((char *)NULL);
  }
}

static char *RenewCRL (state, ripemInfo, ripemDatabase)
RCERTSState *state;
RIPEMInfo *ripemInfo;
RIPEMDatabase *ripemDatabase;
{
  char command[80], *errorMessage;
  unsigned int validityMonths;
  UINT4 now;
  
  puts ("This will renew the CRL issued by you.  This is useful if the CRL validity");
  puts ("  has expired.  Any revocation entries already in the CRL will be kept.");
  puts ("If you have never issued a CRL, this will create a new one.");
  
  GetInputLine
    (command, sizeof (command),
     "  Enter the number of months the new CRL will be valid, or blank to cancel:");

  if (! *command)
    /* User cancelled. */
    return ((char *)NULL);

  /* Set validityMonths to new value.  If sscanf doesn't return 1, it
       means junk was entered. */
  if (sscanf (command, "%u", &validityMonths) != 1) {
    puts ("You did not enter a number.  The CRL was not renewed.");
    return ((char *)NULL);
  }
  if (validityMonths == 0) {
    puts ("Validity months must be one or more.  The CRL was not renewed.");
    return ((char *)NULL);
  }

  /* Get the present time so we can compute the nextUpdate time and update
       the CRL. */
  R_time (&now);
  if ((errorMessage = RIPEMUpdateCRL
       (ripemInfo, now + (UINT4)validityMonths * SECONDS_IN_MONTH,
        (unsigned char *)NULL, 0, ripemDatabase)) != (char *)NULL)
    return (errorMessage);

  if (state->chainStatus.overall != 0) {
    puts ("The CRL has been renewed.  Since the certification status of the current user");
    puts ("  may have changed, you must use \"Select User\" to re-select the user's");
    puts ("  certificate chain.");
    /* Make the user view this message before continuing */
    GetInputLine (command, sizeof (command), "  Hit <ENTER> to continue:");
  
    /* Clear the current user from the state. */
    FreeList (&state->certChain);
    state->chainStatus.overall = 0;
  }

  return ((char *)NULL);
}

static char *PublishCRL (ripemInfo, ripemDatabase)
RIPEMInfo *ripemInfo;
RIPEMDatabase *ripemDatabase;
{
  char command[256], *errorMessage = (char *)NULL;
  FILE *stream = (FILE *)NULL;
  unsigned char *output;
  unsigned int outputLen;
  
  /* For error, break to end of do while (0) block. */
  do {
    puts ("This will publish the CRL issued by you in a file so that you can send it to");
    puts ("  other users.  This is useful only if other users trust you as a");
    puts ("  certification authority and need to know the status of users you certify.");

    GetInputLine
      (command, sizeof (command),
       "  Enter the name of the output file for the CRL message, or blank to cancel:");

    if (! *command)
      /* User cancelled. */
      break;

    if ((stream = fopen (command, "w")) == (FILE *)NULL) {
      printf ("Cannot open %s for write.  Quitting.\n", command);
      break;
    }

    /* Produce the output message.  We use MESSAGE_FORMAT_RIPEM1 instead of
         PEM because in a strict RFC 1422 environment, we wouldn't expect
         individual users to be making CRL messages. */
    if ((errorMessage = RIPEMPublishCRL
         (ripemInfo, &output, &outputLen, MESSAGE_FORMAT_RIPEM1,
          ripemDatabase)) != (char *)NULL)
      break;

    fwrite (output, 1, outputLen, stream);
  } while (0);

  if (stream != (FILE *)NULL)
    fclose (stream);
  return (errorMessage);
}

/* Scan certList for a cert with the given subject name and public key.
   Return TRUE if found or FALSE if not.
   certStruct must already be allocated for convenience to this routine
     so it doesn't have to allocate it for each call.  This also assumes
     the certs in certList have already been decoded once so there is
     no decoding error.
 */
static BOOL FindSubjectPublicKey (certList, subject, publicKey, certStruct)
TypList *certList;
DistinguishedNameStruct *subject;
R_RSA_PUBLIC_KEY *publicKey;
CertificateStruct *certStruct;
{
  TypListEntry *entry;

  for (entry = certList->firstptr; entry; entry = entry->nextptr) {
    DERToCertificate
      ((unsigned char *)entry->dataptr, certStruct, (CertFieldPointers *)NULL);

    if (R_memcmp
        ((POINTER)&certStruct->subject, (POINTER)subject,
         sizeof (*subject)) == 0 &&
        R_memcmp
        ((POINTER)&certStruct->publicKey, (POINTER)publicKey,
         sizeof (*publicKey)) == 0)
      /* Found one */
      return (TRUE);
  }

  return (FALSE);
}

/* Convert a CERT_ validity status into a string such as "VALID".
 */
static char *CertStatusString (certStatus)
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

/* Enable the issuer given by name with a public key with the given
     modulus, exponent and modulusBits.  modulusLen and exponentLen
     are lengths in bytes, not bits.
   This creates a certificate for the issuer if the issuer isn't already
     validated.  Then this sets the chain length allowed to the given value.
 */
static char *EnableIssuer
  (ripemInfo, name, modulus, modulusLen, exponent, exponentLen,
   modulusBits, chainLenAllowed, ripemDatabase)
RIPEMInfo *ripemInfo;
DistinguishedNameStruct *name;
unsigned char *modulus;
unsigned int modulusLen;
unsigned char *exponent;
unsigned int exponentLen;
unsigned int modulusBits;
unsigned int chainLenAllowed;
RIPEMDatabase *ripemDatabase;
{
  CertificateStruct *certStruct = (CertificateStruct *)NULL;
  char *errorMessage = (char *)NULL, command[80];
  unsigned int validityMonths;
  unsigned char digest[16];

  /* For error, break to end of do while (0) block. */
  do {
    puts ("");
    puts ("This will create a certificate for the standard issuer.  If there is already");
    puts ("  a certificate with a current validity period (even if it is revoked), this");
    puts ("  will not create a new one.  Once enabled, the only way to disable the");
    puts ("  issuer is to set the its chain length allowed to zero, or to revoke it.");

    GetInputLine
      (command, sizeof (command),
       "  Enter the number of months the certificate will be valid, or blank to cancel:");

    if (! *command)
      /* User cancelled. */
      break;

    /* Set validityMonths to new value.  If sscanf doesn't return 1, it
         means junk was entered. */
    if (sscanf (command, "%u", &validityMonths) != 1) {
      puts ("You did not enter a number.  The issuer was not enabled.");
      break;
    }
    if (validityMonths == 0) {
      puts ("Validity months must be one or more.  The issuer was not enabled.");
      break;
    }

    /* Allocate the certStruct on the heap because it's big. */
    if ((certStruct = (CertificateStruct *)malloc (sizeof (*certStruct)))
        == (CertificateStruct *)NULL) {
      errorMessage = ERROR_MALLOC;
      break;
    }

    /* Copy in modulus and exponent, with zero padding in high bytes.
     */
    R_memset
      ((POINTER)&certStruct->publicKey, 0, sizeof (certStruct->publicKey));
    R_memcpy
      ((POINTER)(certStruct->publicKey.modulus +
                 (sizeof (certStruct->publicKey.modulus) -  modulusLen)),
       (POINTER)modulus, modulusLen);
    R_memcpy
      ((POINTER)(certStruct->publicKey.exponent +
                 (sizeof (certStruct->publicKey.exponent) - exponentLen)),
       (POINTER)exponent, exponentLen);
    certStruct->publicKey.bits = modulusBits;

    /* Copy the subject name and set the validity period */
    certStruct->subject = *name;
    R_time (&certStruct->notBefore);
    certStruct->notAfter =
      certStruct->notBefore + (UINT4)validityMonths * SECONDS_IN_MONTH;

    if ((errorMessage = ValidateAndWriteCert
         (ripemInfo, certStruct, ripemDatabase)) != (char *)NULL) {
      if (strcmp (errorMessage, ERR_CERT_ALREADY_VALIDATED) == 0) {
        /* Intercept this error and don't treat as fatal. */
        puts ("You have already created a certificate for this issuer.  You can use");
        puts ("  \"Select User\" to select it for viewing its status.");
        errorMessage = (char *)NULL;
      }
      break;
    }

    /* Set chain length allowed to the given value and save the
         preferences.
     */
    if ((errorMessage = GetPublicKeyDigest (digest, &certStruct->publicKey))
        != (char *)NULL)
      break;
    if ((errorMessage = SetChainLenAllowed
         (ripemInfo, digest, chainLenAllowed, ripemDatabase)) != (char *)NULL)
      break;
  } while (0);

  free (certStruct);
  return (errorMessage);
}

/* Take the time which is in seconds since 1/1/70 GMT and put the
     date into the date string of the form 08/24/94 also in GMT.
   This assumes the date string is at least 9 bytes long.
 */
static void GetDateFromTime (date, time)
char *date;
UINT4 time;
{
  int year, month, day;
  unsigned long tempTime;

  /* Note: this code is taken from put_UTC inside the RIPEM library. */
  
  /* Count up seconds in the years starting from 1970 to bring the time
       down to the number of seconds in a year. */
  year = 70;
  while (time >= 
         (tempTime = year % 4 ?
          (SECONDS_IN_DAY * (UINT4)365) : (SECONDS_IN_DAY * (UINT4)366))) {
    time -= tempTime;
    year++;
  }

  /* Count up seconds in the months starting from 1 to bring the time
       down to the number of seconds in a month. */
  month = 1;
  while (time >=
         (tempTime = SECONDS_IN_DAY * (UINT4)LEN_OF_MONTH (year, month))) {
    time -= tempTime;
    month++;
  }
  
  day = (int)(time / SECONDS_IN_DAY) + 1;

  if (year >= 100)
    /* Adjust year 2000 or more to encode as 00 and greater. */
    year -= 100;

  sprintf (date, "%02d/%02d/%02d", month, day, year);
}

