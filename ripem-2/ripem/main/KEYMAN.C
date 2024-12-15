/*--- file keyman.c -- Manage public keys
 *
 *  Mark Riordan  20 May 1992
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "global.h"
#include "rsaref.h"
#include "md5.h"
#include "ripem.h"
#include "keyfield.h"
#ifdef USE_DDES
#include "ddes.h"
#else
#include "des.h"
#endif
#include "keymanpr.h"
#include "strutilp.h"
#include "derkeypr.h"
#include "hexbinpr.h"
#include "ripemsop.h"
#include "pubinfop.h"
#include "keyderpr.h"
#include "bfstream.h"
#include "rdwrmsgp.h"
#include "certder.h"
#include "certutil.h"

#include "bemparse.h"
#include "p.h"

#define DER_SEQ 0x30
#define RECSIZE 3000
#define MAXKEYBYTES 1024

#define MAX_PRENCODE_BYTES 48

static char *GetKeyBytesFromFile
  P((char *, char *, FILE *, char *, BOOL *, unsigned char **, unsigned int *,
     RIPEMInfo *));
static char *GetNextCRLFromFile
  P((BOOL *, unsigned char **, UINT4 *, FILE *, char *, UINT4));
static char *ReadEncodedField P((unsigned char **, unsigned int *, FILE *));
static char *GetUserRecordFromFile
  P((char *, TypFile *, unsigned int, char *, BOOL *, RIPEMInfo *));
static char *OpenKeySource P((TypKeySource *, char *, RIPEMInfo *));

/* Initialize the key sources and origins.
 */
void RIPEMDatabaseConstructor (ripemDatabase)
RIPEMDatabase *ripemDatabase;
{
  InitList (&ripemDatabase->pubKeySource.filelist);
  InitList (&ripemDatabase->privKeySource.filelist);
  InitList (&ripemDatabase->crlSource.filelist);
  InitList (&ripemDatabase->privKeySource.serverlist);
  InitList (&ripemDatabase->pubKeySource.serverlist);
  InitList (&ripemDatabase->crlSource.serverlist);
  ripemDatabase->pubKeySource.origin[0] = KEY_FROM_SERVER;
  ripemDatabase->privKeySource.origin[0] = KEY_FROM_SERVER;
  ripemDatabase->crlSource.origin[0] = KEY_FROM_SERVER;
  ripemDatabase->pubKeySource.origin[1] = KEY_FROM_FILE;
  ripemDatabase->privKeySource.origin[1] = KEY_FROM_FILE;
  ripemDatabase->crlSource.origin[1] = KEY_FROM_FILE;
  ripemDatabase->pubKeySource.origin[2] = KEY_FROM_FINGER;
  ripemDatabase->privKeySource.origin[2] = KEY_FROM_FINGER;
  ripemDatabase->crlSource.origin[2] = KEY_FROM_FINGER;

  ripemDatabase->preferencesFilename = (char *)NULL;
}

/* Insert the filename at the beginning of keySource's filelist if it is not
     already in the list.  This makes a copy of the filename.
   This does not open the file.  (Use InitRIPEMDatabase after all filenames
     for all key sources have been added.)
   Return NULL for success, otherwise error string.
 */
char *AddKeySourceFilename (keySource, filename)
TypKeySource *keySource;
char *filename;
{
  TypListEntry *entry;
  TypFile *typFile;
  char *errorMessage;

  /* First check if the filename is already in the key source.
   */
  for (entry = keySource->filelist.firstptr;
       entry != (TypListEntry *)NULL; entry = entry->nextptr) {
    if (strcmp (filename, ((TypFile *)entry->dataptr)->filename) == 0)
      /* Already have the filename, so do nothing. */
      return ((char *)NULL);
  }
       
  if ((typFile = (TypFile *)malloc (sizeof (*typFile))) == (TypFile *)NULL)
    return (ERR_MALLOC);
  
  typFile->stream = NULL;
  if (strcpyalloc (&typFile->filename, filename) == (char *)NULL) {
    free (typFile);
    return (ERR_MALLOC);
  }
  if ((errorMessage = PrependToList
       (typFile, sizeof (*typFile), &keySource->filelist)) != (char *)NULL) {
    free (typFile->filename);
    free (typFile);
    return (errorMessage);
  }

  return ((char *)NULL);
}

/* homeDir is the name of the RIPEM home directory.  It must end in the
     necessary directory separator, such as '/' in Unix, so that filenames
     in the directory can just be appended.  The calling routine is also
     responsible for making sure the directory exists.  This can be done,
     for example, by trying the open the crls file for append.
   This adds pubkeys, privkey, and crls in the homeDir to pubKeySource,
     privKeySource and crlSource in ripemDatabase if not already there.
     This then makes sure these filenames can be opened for output and are
     in the front of the keySource list as required for database output files.
     Then it opens all the fileList->stream entries in the keySources for read.
     (There may already be files which were added by AddKeySourceFilename.)
   This also allocates and sets ripemDatabase->preferencesFilename to
     preferen in the homeDir and makes sure it can be opened for append.
   ripemInfo is used only for error and debug output.
 */
char *InitRIPEMDatabase (ripemDatabase, homeDir, ripemInfo)
RIPEMDatabase *ripemDatabase;
char *homeDir;
RIPEMInfo *ripemInfo;
{
  FILE *stream;
  char *path = (char *)NULL, *errorMessage = (char *)NULL;
  unsigned int homeDirLen;

  /* For error, break to end of do while (0) block. */
  do {
    /* Allocate space for the homeDir plus a filename in the path and
         copy the homeDir to the front of the path. */
    homeDirLen = strlen (homeDir);
    if ((path = malloc (homeDirLen + 10)) == (char *)NULL) {
      errorMessage = ERR_MALLOC;
      break;
    }
    strcpy (path, homeDir);

    /* For each of the databases, concatenate the file name to the homeDir,
         add the name to the key sources, and then open the key source.
     */
    strcpy (path + homeDirLen, "pubkeys");
    if ((errorMessage = AddKeySourceFilename
         (&ripemDatabase->pubKeySource, path)) != (char *)NULL)
      break;
    if ((errorMessage = OpenKeySource
         (&ripemDatabase->pubKeySource, path, ripemInfo)) != (char *)NULL)
      break;

    strcpy (path + homeDirLen, "privkey");
    if ((errorMessage = AddKeySourceFilename
         (&ripemDatabase->privKeySource, path)) != (char *)NULL)
      break;
    if ((errorMessage = OpenKeySource
         (&ripemDatabase->privKeySource, path, ripemInfo)) != (char *)NULL)
      break;

    strcpy (path + homeDirLen, "crls");
    if ((errorMessage = AddKeySourceFilename
         (&ripemDatabase->crlSource, path)) != (char *)NULL)
      break;
    if ((errorMessage = OpenKeySource
         (&ripemDatabase->crlSource, path, ripemInfo)) != (char *)NULL)
      break;

    /* Make sure the preferences file can be opened for append.  This will
         create the file if it doesn't exist.
     */
    strcpy (path + homeDirLen, "preferen");
    if ((stream = fopen (path, "a")) == (FILE *)NULL) {
      sprintf
        (ripemInfo->errMsgTxt,
         "Can't write to file \"%s\". (Does its directory exist?)", path);
      errorMessage = ripemInfo->errMsgTxt;
      break;
    }

    /* Successfully opened, so just close it. */
    fclose (stream);

    /* Set the preferences filename by trasferring the path string to
         the ripemDatabase since we are done with the path.
     */
    ripemDatabase->preferencesFilename = path;
    /* Set to NULL so it won't be freed below. */
    path = (char *)NULL;
  } while (0);

  free (path);
  return (errorMessage);
}

/* Close the files and free the key source lists.
 */
void RIPEMDatabaseDestructor (ripemDatabase)
RIPEMDatabase *ripemDatabase;
{
  TypListEntry *entry;
  TypFile *typFile;

  for (entry = ripemDatabase->privKeySource.filelist.firstptr;
       entry != (TypListEntry *)NULL; entry = entry->nextptr) {
    typFile = (TypFile *)entry->dataptr;
    free (typFile->filename);
    if (typFile->stream != (FILE *)NULL)
      fclose (typFile->stream);
  }
  for (entry = ripemDatabase->pubKeySource.filelist.firstptr;
       entry != (TypListEntry *)NULL; entry = entry->nextptr) {
    typFile = (TypFile *)entry->dataptr;
    free (typFile->filename);
    if (typFile->stream != (FILE *)NULL)
      fclose (typFile->stream);
  }
  for (entry = ripemDatabase->crlSource.filelist.firstptr;
       entry != (TypListEntry *)NULL; entry = entry->nextptr) {
    typFile = (TypFile *)entry->dataptr;
    free (typFile->filename);
    if (typFile->stream != (FILE *)NULL)
      fclose (typFile->stream);
  }

  FreeList (&ripemDatabase->pubKeySource.filelist);
  FreeList (&ripemDatabase->privKeySource.filelist);
  FreeList (&ripemDatabase->crlSource.filelist);
  FreeList (&ripemDatabase->privKeySource.serverlist);
  FreeList (&ripemDatabase->pubKeySource.serverlist);
  FreeList (&ripemDatabase->crlSource.serverlist);

  free (ripemDatabase->preferencesFilename);
}

/*  Get the public key of a user from a PublicKeyInfo field (as opposed
 *    to a certificate.  This is provided only for compatibility with
 *    RIPEM 1.1.
 *
 *  Entry:  user      is the user's email address.
 *          source      tells us where to look.
 *          ripemInfo is used to access debug and debugStream.
 * 
 *   Exit:  key     is the key (if found).
 *          found     is TRUE if we found the key.
 *          Returns an error message if something goes wrong more 
 *            serious than not being able to find the key.
 */
char *GetUnvalidatedPublicKey (user, source, key, found, ripemInfo)
char *user;
TypKeySource *source;
R_RSA_PUBLIC_KEY *key;
BOOL *found;
RIPEMInfo *ripemInfo;
{
  unsigned char *bytes = NULL;
  char *errorMessage=NULL;
  int sour;
  unsigned int derLen = 0;
  static BOOL server_ok=TRUE;
  char rec[RECSIZE];
  char coded_bytes[MAXKEYBYTES];
  unsigned char key_bytes[MAXKEYBYTES];
#define MAXDIGESTSIZE 36
  char computed_hex_digest[MAXDIGESTSIZE], records_hex_digest[MAXDIGESTSIZE];
  TypFile *fptr;

  *found = FALSE;
  
  for(sour=0; !*found && sour<MAX_KEY_SOURCES; sour++)
    switch(source->origin[sour]) {
    /* Search the key file */
  case KEY_FROM_FILE:
    FORLIST(&(source->filelist));
      fptr = (TypFile *)dptr;
      if(!fptr->stream) {
        sprintf(ripemInfo->errMsgTxt,"Can't open public key file \"%s\".",
                fptr->filename);
        errorMessage = ripemInfo->errMsgTxt;
        continue;
      }

      /* Get the user record for this user from the file. */
      errorMessage = GetUserRecordFromFile
        (user,fptr,RECSIZE,rec,found,ripemInfo);
      if(*found) goto decodekey;
    ENDFORLIST;
    break;

    /* Get key from server. */
  case KEY_FROM_SERVER:
    if(server_ok) {
      errorMessage = GetUserRecordFromServer
        (user,source, rec,RECSIZE,&server_ok,found,ripemInfo);
      goto decodekey;
    }
            
    /* Get key from finger. */
  case KEY_FROM_FINGER:
    errorMessage = GetUserRecordFromFinger (user,rec,RECSIZE,found,ripemInfo);
  decodekey:
    if(ripemInfo->debug>1) {
      if(errorMessage) {
        fprintf(ripemInfo->debugStream,
                "Error retrieving key for %s from server: %s\n",
                user,errorMessage);
      } else if(*found) {
        fprintf(ripemInfo->debugStream,
                "Found %s's public key record:\n%s\n",user,rec);
      } else {
        fprintf(ripemInfo->debugStream,
                "Couldn't find %s's public key record.\n", user);
      }
    }
    if(*found) {
      if (!CrackKeyField(rec,PUBLIC_KEY_FIELD,coded_bytes,MAXKEYBYTES)) {
        /* There is no public key field. */
        *found = FALSE;
        break;
      }
      if(ripemInfo->debug>1) {
        fprintf(ripemInfo->debugStream,"Coded pub key=\"%s\"\n",coded_bytes);
      }
      /* CrackKeyField skips all whitespace and null-terminates the
         coded_bytes.  key_bytes is the same size as coded_bytes so
         the smaller decoded bytes will fit. */
      R_DecodePEMBlock
        (key_bytes, &derLen, (unsigned char *)coded_bytes,
         strlen (coded_bytes));
      bytes = key_bytes;
    }
            
    break;

  default:
    break;   /* XXX */
  }
  if(*found) {
    /* Convert key bytes to public key structure format. */
    if(DERToPubKey(bytes,key)) {
      /* Conversion didn't work. */
      sprintf(ripemInfo->errMsgTxt,"Error parsing public key for %s.",user);
      errorMessage = ripemInfo->errMsgTxt;
    } else {
      /* Conversion from BER format worked OK.
       * Now check this public key against the enclosed digest.
       */
      MakeHexDigest((unsigned char *)bytes,derLen,computed_hex_digest);
      if(CrackKeyField(rec,PUBLIC_KEY_DIGEST_FIELD,
                       records_hex_digest,MAXDIGESTSIZE)) {
        if(ripemInfo->debug > 2) {
          fprintf(ripemInfo->debugStream,
                  "der len=%u\nComputed  MD5 of %s's pubkey=%s\n",derLen,
                  user, computed_hex_digest);
          fprintf(ripemInfo->debugStream,
                  "Retrieved MD5 of %s's pubkey=%s\n",user,
                  records_hex_digest);
        }
        if(strcmp(computed_hex_digest,records_hex_digest)) {
          sprintf(ripemInfo->errMsgTxt,
                  "Public key of '%s' is garbled--digest does not match.",
                  user);
          errorMessage = ripemInfo->errMsgTxt;
        }
      } else {
        fprintf(stderr,"Warning--could not find %s's key digest.\n",
                user);
      }
    }
  } 
  return errorMessage;
}

/*--- function GetPrivateKey -------------------------------------
 *
 *  Get the private key of a user.
 *  RIPEMInfo is used for debug and debugStream.
 */
char *GetPrivateKey (user, source, key, password, passwordLen, ripemInfo)
char *user;
TypKeySource *source;
R_RSA_PRIVATE_KEY *key;
unsigned char *password;
unsigned int passwordLen;
RIPEMInfo *ripemInfo;
{
  BOOL found = FALSE;
  TypFile *fptr;
  char *errorMessage = (char *)NULL;
  int digest_alg, sour;
  unsigned char *bytes = (unsigned char *)NULL,
    *enc_key = (unsigned char *)NULL, salt[SALT_SIZE];
  unsigned int enc_key_len, iter_count, num_der_bytes, nbytes,
    enc_keyBufferSize = 0;

  /* For error, break to end of do while (0) block. */
  do {
    for (sour = 0; sour < MAX_KEY_SOURCES; sour++) {
      if (source->origin[sour] == KEY_FROM_FILE) {
        FORLIST (&(source->filelist));
          fptr = (TypFile *)dptr;
          if (!fptr->stream) {
            sprintf
              (ripemInfo->errMsgTxt, "Private key file \"%s\" is not open.",
               fptr->filename);
            errorMessage = ripemInfo->errMsgTxt;
            break;
          }

          /* Rewind in order to search from the beginning. */
          fseek (fptr->stream, 0L, 0);

          if ((errorMessage = GetKeyBytesFromFile
               (USER_FIELD, user, fptr->stream, PRIVATE_KEY_FIELD, &found,
                &bytes, &nbytes, ripemInfo)) != (char *)NULL)
            break;
          if (found)
            break;

        ENDFORLIST;
        if (errorMessage != (char *)NULL)
          /* broke loop because of error */
          break;
      }
    }
    if (errorMessage != (char *)NULL)
      /* broke loop because of error */
      break;

    if (!found) {
      sprintf (ripemInfo->errMsgTxt, "Can't find private key for %s.", user);
      errorMessage = ripemInfo->errMsgTxt;
      break;
    }

    /* We now have the DER-encoded encrypted private key structure.
       First, decode it to obtain the encryption algorithm parameters
       and the actual bytes of the encrypted key.
     */
    if (ripemInfo->debug > 1) {
      fprintf (ripemInfo->debugStream,
               "Obtained %u byte encrypted private key for %s:\n",
               nbytes, user);
      BEMParse (bytes, ripemInfo->debugStream);
    }

    /* Allocate a buffer the the encrypted bytes. */
    enc_keyBufferSize = nbytes;
    if ((enc_key = (unsigned char *)malloc (enc_keyBufferSize))
        == (unsigned char *)NULL) {
      errorMessage = ERR_MALLOC;
      break;
    }
    if (DERToEncryptedPrivKey
        (bytes, nbytes, &digest_alg, salt, &iter_count, enc_key,
         &enc_key_len)) {
      errorMessage = "Error decoding encrypted private key.";
      break;
    }

    /* Now decrypt the encrypted key.
     */
    if (pbeWithMDAndDESWithCBC
        (FALSE, digest_alg, enc_key, enc_key_len, password, passwordLen,
         salt, iter_count, &num_der_bytes) != 0) {
      errorMessage = "Can't decrypt private key.";
      break;
    }

    /* We have the plaintext private key in DER format.
     * Check to make sure it looks as if it was decrypted OK.
     * Then Decode to RSAREF format.
     */
    if (enc_key[0] != DER_SEQ) {
      errorMessage =
        "Private key could not be decrypted with this password.";
      break;
    }

    if (DERToPrivKey (enc_key, key)) {
      sprintf
        (ripemInfo->errMsgTxt, "Error parsing private key for %s.",user);
      errorMessage = ripemInfo->errMsgTxt;
      break;
    }

    if (ripemInfo->debug > 1) {
      fprintf (ripemInfo->debugStream, "Dump of decrypted private key:\n");
      DumpPrivKey (key, ripemInfo->debugStream);
    }
  } while (0);

  if (enc_key != (unsigned char *)NULL) {
    R_memset ((POINTER)enc_key, 0, enc_keyBufferSize);
    free (enc_key);
  }
  free (bytes);
  return (errorMessage);
}

/*--- function GetKeyBytesFromFile ---------------------------------
 *  
 *  Obtain the value of a field from a flat file.
 *  This searches for the next record with the given keyName with a
 *    matching keyValue and gets the fieldName.
 *  The calling routine must rewind the file to begin a new search.
 *  The file is ASCII newline-delimited and consists of
 *  keys of form:   keyName: keyValue
 *  and fields of form:
 *     fieldName:
 *        value...  (RFC1113 encoded)
 *
 *  Entry:  keyName is the key name like "User:"
 *          keyValue        is the key value for keyName.  
 *          stream     is the stream pointing at the file.
 *          fieldName  is the name of the field, e.g. PublicKeyInfo.
 *          ripemInfo is used for debug and debugStream.
 *          
 *  Exit:   bytes    points to the bytes as a newly allocated buffer.
 *          numBytes    is the number of bytes retrieved.
 *          found       is TRUE if we found the key.
 *          Returns an error message if an error was found (more serious
 *          than the key value not being found).
 */
static char *GetKeyBytesFromFile
  (keyName, keyValue, stream, fieldName, found, bytes, numBytes, ripemInfo)
char *keyName;
char *keyValue;
FILE *stream;
char *fieldName;
BOOL *found;
unsigned char **bytes;
unsigned int *numBytes;
RIPEMInfo *ripemInfo;
{
#define VALUELEN 120
  char value[VALUELEN];
  BOOL gotRecord;
  char *errorMessage = NULL;

  /* Default to not found */
  *found = FALSE;
  *bytes = (unsigned char *)NULL;

  /* For error, break to end of do while (0) block. */
  do {
    /* Position to just after the line containing the keyValue's name.
     */
    gotRecord = FALSE;
    while (GetFileLine (stream, keyName, value, sizeof (value))) {
      if (R_match (value, keyValue)) {
        gotRecord = TRUE;
        break;
      }
    }
    if (!gotRecord)
      break;

    /* We are now in the section of the file that corresponds
       to this keyValue.  Position to the desired field.
       (There aren't many fields; we do this in case there
       are multiple "keyValue:" fields & to account for future changes.)
       Assume the fieldName is on a line by itself.
     */
    if (!PosFileLine (stream, fieldName))
      /* *found is already false */
      break;

    /* Now read the RFC1113-encoded lines and translate
       them to binary. */
    if ((errorMessage = ReadEncodedField
         (bytes, numBytes, stream)) != (char *)NULL)
      break;

    *found = TRUE;
  } while (0);

  if(ripemInfo->debug > 1) {
    if (*found) {
      fprintf (ripemInfo->debugStream,"Obtained %s for %s from file.\n",
               fieldName, keyValue);
    }
    else {
      fprintf (ripemInfo->debugStream,
               "Could not find %s of keyValue %s in file.\n",
               fieldName, keyValue);
    } 
  }
  
  return (errorMessage);
}

/* Read an encoded field from a file and return it in an allocated buffer.
   This reads lines until a blank line, end of file, or a line
     with non-whitespace in the first column.
   Return NULL for success, otherwise error string.
 */
static char *ReadEncodedField (field, fieldLen, stream)
unsigned char **field;
unsigned int *fieldLen;
FILE *stream;
{
#define KALLOC_INC 1080
  char line[VALUELEN];
  unsigned char *bytes, *newField;
  int bytesleft, allocSize;
  unsigned int bytesInLine;

  /* Allocate initial buffer. */
  if ((*field = (unsigned char *)malloc (KALLOC_INC)) == (unsigned char *)NULL)
    return (ERR_MALLOC);

  bytes = *field;
  allocSize = bytesleft = KALLOC_INC;
  *fieldLen = 0;
  
  while (1) {
    if (!fgets (line, sizeof (line), stream)) {
      /* Make sure we got NULL because of end of file. */
      if (!feof (stream))
        return ("Error reading bytes from flat file");

      /* Done */
      return ((char *)NULL);
    }

    if (!WhiteSpace (line[0]) || LineIsWhiteSpace (line))
      /* Reached the end */
      return ((char *)NULL);

    /* If we need more room in field, allocate more.
       Reassign pointers as necessary.
     */
    if (bytesleft < MAX_PRENCODE_BYTES) {
      allocSize += KALLOC_INC;
      if ((newField = (unsigned char *)R_realloc (*field, allocSize))
            == (unsigned char *)NULL) {
        free (*field);
        return (ERR_MALLOC);
      }

      *field = newField;
      bytes = *field + *fieldLen;
    }

    /* Decode the line into the value buffer.
       Assume the input line has one prefix space and that the
         decode won't return more than MAX_PRENCODE_BYTES.
       Assume each line except the last has a multiple of 4 chars.
       Also assume there is one '\n' and no spaces at the end of line.
     */
    R_DecodePEMBlock
      (bytes, &bytesInLine, (unsigned char *)(line + 1),
       strlen (line + 1) - 1);
    bytes += bytesInLine;
    *fieldLen += bytesInLine;
    bytesleft -= bytesInLine;
  }
}

/*--- function GetUserRecordFromFile --------------------------------------
 *
 *  Get the user record for a user from a file.  A user record
 *  is a series of ASCII lines like:
 *    User: joe@bigu.edu
 *    PublicKeyInfo:
 *     MIGcMAoGBFUIAQECAgQAA4GNADCBiQKBgQDGQci5pOCGqQgW6XUYyGCcZFIyyLb7
 *     18nsKtQNjHZRODHkd+5tmHzMWp2BdFfV+CQzbMeNcdC9lC/RhLb7AgMBAAE=
 *    MD5OfPublicKey: E69AB9AA2A2697FCB5B1821DC3596345
 *
 *  Entry:  user     is the user name (email address) whose record we want.
 *          fileptr  contains the stream pointing at the file.
 *          maxBytes is the size of the buffer used to return data.
 *          ripemInfo is used for debug and debugStream.
 * 
 *  Exit:   userRec  contains the user record, if found.  It is 
 *                   zero-terminated.
 *          found    is TRUE if we found the key.
 *          Returns an error message if an error was found (more serious
 *          than the key value not being found), else NULL.
 */
static char *
GetUserRecordFromFile(user,fileptr,maxBytes,userRec,found,ripemInfo)
char *user;
TypFile *fileptr;
unsigned int maxBytes;
char *userRec;
BOOL *found;
RIPEMInfo *ripemInfo;
{
  BOOL got_next_rec, looking=TRUE;
   
  *found = FALSE;
  fseek(fileptr->stream,0L,0);  /* Rewind the file. */
      
  if(ripemInfo->debug>1) {
    fprintf(ripemInfo->debugStream,"Looking in '%s' for public key for %s.\n",
            fileptr->filename,user);
  }
  while(looking) {
    GetNextUserRecordFromFile(fileptr->stream,maxBytes,
                              userRec,&got_next_rec);
    if(got_next_rec) {
      if(FindUserInRecord(user,userRec)) {
        *found = TRUE;
        looking = FALSE;
      }
    } else {
      looking = FALSE;
    }
  }
  if(ripemInfo->debug>1) {
    if(*found) {
      fprintf(ripemInfo->debugStream,"Found %s's public key record in file.\n",
              user);
    } else {
      fprintf(ripemInfo->debugStream,"Didn't find %s's public key in file.\n",
              user);
    }
  }
  return (char *)NULL;
}

/*--- function GetNextUserRecordFromFile ----------------------------------
 *
 *  Get the next user record from a sequential file.
 *  A user record is just a sequence of lines limited by a 
 *  blank line or a line starting with "--".
 *
 *  Entry:  ustream     is the stream of the file.
 *          maxBytes    is the buffer size of userRec.
 *
 *  Exit:   userRec     is the user record, if found.
 *          found       is TRUE if we successfully retrieved a record.
 *          Returns an error message if there was a problem worse
 *          than EOF, else NULL.
 */
char *
GetNextUserRecordFromFile(ustream,maxBytes,userRec,found)
FILE *ustream;
unsigned int maxBytes;
char *userRec;
BOOL  *found;
{
#define LINELEN 256
  char line[LINELEN];
  char *errmsg=NULL;
  char *uptr=userRec;
  char *got_line;
  unsigned int mylen;
  
  *found = FALSE;
  /* Skip past leading blank lines */
  do {
    if(!fgets(line,LINELEN,ustream)) {
      if (!feof (ustream))
        errmsg = "Error in stream while reading user record";
      goto endnext;
    }
  } while(LineIsWhiteSpace(line) || strncmp(line,"---",2)==0);
  
  /* We hit a non-blank line.
   * Copy lines into the buffer until we hit EOF or blank line. 
   */
  *found = TRUE;
  do {
    mylen = (unsigned int)strlen(line);
    /* Copy this line into the buffer if there's room, 
     * else just return the truncated buffer.
     */
    if(maxBytes > mylen) {
      strcpy(uptr,line);
      uptr += mylen;
      maxBytes -= mylen;
    } else {
      goto endnext;
    }
    got_line = fgets(line,LINELEN,ustream);
  } while(got_line && !LineIsWhiteSpace(line) && strncmp(line,"---",2));
  
endnext:
  return errmsg;
}

/*--- function pbeWithMDAndDESWithCBC --------------------------------------
 *
 * Encrypt or decrypt a buffer with DES-CBC, using a key derived
 * from using a message disest (MDx) function on a password
 * and salt value.
 */
int
pbeWithMDAndDESWithCBC
  (encrypt, digestAlg, buf, numInBytes, password, passwordLen, salt,
   iterationCount, numOutBytes)
int encrypt;
int digestAlg;
unsigned char *buf;
unsigned int numInBytes;
unsigned char *password;
unsigned int passwordLen;
unsigned char *salt;
unsigned int iterationCount;
unsigned int *numOutBytes;
{
#define DIGEST_SIZE 16
  unsigned char *pass_and_salt, byte, parity, *bptr;
  MD5_CTX context;
  unsigned char digest[DIGEST_SIZE], des_key[DES_KEY_SIZE];
  unsigned char iv[DES_BLOCK_SIZE];
  unsigned int n_pad_bytes;
  unsigned int j, bit;
  
  if(digestAlg != DA_MD5) return 1;
  
  /* Create concatenation of password and salt */
  pass_and_salt = (unsigned char *) malloc(passwordLen+SALT_SIZE);
  if(!pass_and_salt) return 1;
  R_memcpy(pass_and_salt,password,passwordLen);
  R_memcpy(pass_and_salt+passwordLen,salt,SALT_SIZE);
  
  /* First iteration is a digest of password || salt */
  MD5Init(&context);
  MD5Update(&context,pass_and_salt, passwordLen+SALT_SIZE);
  MD5Final(digest,&context);
  
  free(pass_and_salt);
  
  /* Subsequent iterations are digests of the previous digest. */
  while(--iterationCount) {
    MD5Init(&context);
    MD5Update(&context,digest,DIGEST_SIZE);
    MD5Final(digest,&context);
  }
  
  /* Create the DES key by taking the first 8 bytes of the
   * digest and setting the low order bit to be an odd parity bit.
   */
  for(j=0; j<DES_KEY_SIZE; j++) {
    byte = digest[j];
    for(parity=0x01,bit=0; bit<7; bit++) {
      byte >>= 1;
      parity ^= (byte&0x01);
    }
    des_key[j] = (digest[j]&0xfe) | parity;
  }
  
  /* Create the initialization vector from the last 8 bytes of the digest */
  R_memcpy(iv,digest+DES_KEY_SIZE,DES_BLOCK_SIZE);
  
  /* Now we have the DES key and the init vector.
   * Do the encrypt or decrypt.
   */

  if(encrypt) {
    /* Pad the last block of the buffer with 1 to 8 bytes of
     * the value 01 or 0202 or 030303 or...
     */

    n_pad_bytes = DES_BLOCK_SIZE - numInBytes%DES_BLOCK_SIZE;
    for(bptr=buf+numInBytes,j=0; j<n_pad_bytes; j++,bptr++) {
      *bptr = n_pad_bytes;
    }
    *numOutBytes = numInBytes+n_pad_bytes;
    
    DESWithCBC(encrypt,buf,*numOutBytes,des_key,iv);
    
  } else {
    /* Do the decryption */
    if(numInBytes%DES_BLOCK_SIZE) return 1;
    DESWithCBC(encrypt,buf,numInBytes,des_key,iv);
    n_pad_bytes = buf[numInBytes-1];
    *numOutBytes = numInBytes - n_pad_bytes;
  }
  
  return 0;
}


/*--- function DESWithCBC ------------------------------------------
 *
 * Encrypt or decrypt a buffer with DES with Cipher Block Chaining.
 *
 *  Entry:  encrypt  is TRUE to encrypt, else decrypt.
 *          buf      is the beginning of the buffer.
 *          numBytes is the number of bytes to encrypt/decrypt.
 *                   It is rounded up to a multiple of 8.
 *          key      is the 8-byte key.
 *          iv       is the initialization vector.  (We pretend
 *                   it is the output from the previous round
 *                   of encryption.)
 *
 *  Exit:   buf      has been encrypted/decrypted.
 */
void
DESWithCBC(encrypt,buf,numBytes,key,iv)
int encrypt;
unsigned char *buf;
unsigned int numBytes;
unsigned char *key;
unsigned char *iv;
{
#ifdef USE_DDES
  int mode = !encrypt, count;
  unsigned char my_iv[DES_BLOCK_SIZE], save_iv[DES_BLOCK_SIZE];
  unsigned char *source, *targ;
  unsigned int block_cnt = (numBytes+DES_BLOCK_SIZE-1)/DES_BLOCK_SIZE;
  
  deskey(key,mode);
  R_memcpy(my_iv,iv,DES_BLOCK_SIZE);
  
  if(encrypt) {
    while(block_cnt--) {
      for(targ=buf,source=my_iv,count=DES_BLOCK_SIZE; count; count--) {
        *(targ++) ^= *(source++);
      }
      des(buf,buf);
      R_memcpy(my_iv,buf,DES_BLOCK_SIZE);
      buf += DES_BLOCK_SIZE;
    }
  } else {
    while(block_cnt--) {
      R_memcpy(save_iv,buf,DES_BLOCK_SIZE);
      des(buf,buf);
      for(targ=buf,source=my_iv,count=DES_BLOCK_SIZE; count; count--) {
        *(targ++) ^= *(source++);
      }
      R_memcpy(my_iv,save_iv,DES_BLOCK_SIZE);
      buf += DES_BLOCK_SIZE;
    }
  }
#else
  DES_CBC_CTX context;
  unsigned int len;
  
  len = (numBytes+DES_BLOCK_SIZE-1)/DES_BLOCK_SIZE;
  len *= DES_BLOCK_SIZE;
  
  DES_CBCInit(&context, key, iv, encrypt);
  DES_CBCUpdate(&context, buf, buf, len);
  R_memset ((POINTER)&context, 0, sizeof (context));
#endif
}

/*--- function DumpPubKey ------------------------------------------
 *
 */
void
DumpPubKey(pubKey, stream)
R_RSA_PUBLIC_KEY *pubKey;
FILE *stream;
{
  fprintf(stream,"Dump of %d bit key:\n",pubKey->bits);
  fprintf(stream,"    Mod=");
  DumpBigNum(pubKey->modulus,MAX_RSA_MODULUS_LEN, stream);
  fputs("    exp=",stream);
  DumpBigNum(pubKey->exponent,MAX_RSA_MODULUS_LEN, stream);
}

/*--- function DumpPrivKey ------------------------------------------
 *
 */
void
DumpPrivKey(privKey, stream)
R_RSA_PRIVATE_KEY *privKey;
FILE *stream;
{
  fprintf(stream,"Dump of %d bit private key:\n",privKey->bits);
  fputs(" mod   =",stream);
  DumpBigNum(privKey->modulus,MAX_RSA_MODULUS_LEN, stream);
  fputs(" pubExp=",stream);
  DumpBigNum(privKey->publicExponent,MAX_RSA_MODULUS_LEN, stream);
  fputs(" exp   =",stream);
  DumpBigNum(privKey->exponent,MAX_RSA_MODULUS_LEN, stream);
  fputs(" prime1=",stream);
  DumpBigNum(privKey->prime[0],MAX_RSA_PRIME_LEN, stream);
  fputs(" prime2=",stream);
  DumpBigNum(privKey->prime[1],MAX_RSA_PRIME_LEN, stream);
  fputs(" prExp1=",stream);
  DumpBigNum(privKey->primeExponent[0],MAX_RSA_PRIME_LEN, stream);
  fputs(" prExp2=",stream);
  DumpBigNum(privKey->primeExponent[1],MAX_RSA_PRIME_LEN, stream);
  fputs(" coeffi=",stream);
  DumpBigNum(privKey->coefficient,MAX_RSA_PRIME_LEN, stream);
}

/*--- function DumpBigNum -------------------------------------------
 *
 */
void
DumpBigNum(bigNum, numLen, stream)
unsigned char *bigNum;
int numLen;
FILE *stream;
{
  char buf[4];
  int j, bytesonline=0;
  
  for(j=0; j<numLen && !bigNum[j]; j++);
  for(; j<numLen; j++) {
    BinToHex(bigNum+j,1,buf);
    if(++bytesonline >= 32) {
      fputs("\n        ",stream);
      bytesonline=1;
    }
    fputs(buf,stream);
  }
  fputs("\n",stream);
}

/* Look in the public key source files for all records with the smartName and
     append the contents of each CertificateInfo field to the certs list.
   This will ASCII decode the CertificateInfo fields.
   ripemInfo is only used for debug and debugStream.
 */
char *GetCertsBySmartname (ripemDatabase, certs, smartName, ripemInfo)
RIPEMDatabase *ripemDatabase;
TypList *certs;
char *smartName;
RIPEMInfo *ripemInfo;
{
  BOOL gotCert;
  TypFile *typFile;
  char *errorMessage;
  unsigned char *newCert;
  unsigned int i, newCertLen;       
  

  if(ripemInfo->debug>1) {
    fprintf(ripemInfo->debugStream,
      "GetCertsBySmartname looking for '%s'\n",smartName);
  }
   

  for (i = 0; i < MAX_KEY_SOURCES; ++i) {
    /* Only look in files for now. */
    if (ripemDatabase->pubKeySource.origin[i] == KEY_FROM_FILE) {
      FORLIST(&ripemDatabase->pubKeySource.filelist);
        typFile = (TypFile *)dptr;
        if (!typFile->stream)
          return ("Public key file is not open.");

        /* Rewind the file */
        fseek (typFile->stream, 0L, 0);

        /* Look for certificates until end of file.
         */
        do {
          if ((errorMessage = GetKeyBytesFromFile
               (USER_FIELD, smartName, typFile->stream, CERT_INFO_FIELD,
                &gotCert, &newCert, &newCertLen, ripemInfo)) != (char *)NULL)
            return (errorMessage);

          if (!gotCert)
            /* Finished with this record.  Try the next. */
            continue;

          /* Append the newly allocated cert to the list. */
          if ((errorMessage = AddToList
               ((TypListEntry *)NULL, newCert, newCertLen, certs))
              != (char *)NULL) {
            /* error, so free the newCert */
            free (newCert);
            return (errorMessage);
          }
        } while (!feof (typFile->stream));
      ENDFORLIST;
    }
  }

  return ((char *)NULL);
}

/* Search the CRL file for the most recent CRL for the issuer with a last
     update equal to or before the given time.  The issuer is given by
     the public key digest.  (This is done instead of the issuer name since
     the issuer may have multiple public keys.)
   time is seconds since 1/1/70. Typically, it is set to now, but may
     also be set to a certificate's expiration time to see if it was
     revoked at that time.
   If found, crlDER is set to an allocated buffer containing the CRL DER.
     The caller must free this buffer.  If not found, crlDER is set to NULL.
   Returns NULL for success, otherwise error string.
 */
char *GetLatestCRL (ripemDatabase, crlDER, publicKeyDigest, time)
RIPEMDatabase *ripemDatabase;
unsigned char **crlDER;
unsigned char *publicKeyDigest;
UINT4 time;
{
  BOOL gotCRL;
  TypFile *typFile;
  UINT4 lastUpdate, bestLastUpdate;
  char *errorMessage, publicKeyHexDigest[33];
  unsigned char *newCRL;
  unsigned int i;

  /* Initialize allocated CRL pointer to NULL. */
  *crlDER = (unsigned char *)NULL;

  /* We need to match on the hex digest of the public key */
  BinToHex (publicKeyDigest, 16, publicKeyHexDigest);

  bestLastUpdate = (UINT4)0;
  
  for (i = 0; i < MAX_KEY_SOURCES; ++i) {
    /* Only look in files for now. */
    if (ripemDatabase->crlSource.origin[i] == KEY_FROM_FILE) {
      FORLIST(&ripemDatabase->crlSource.filelist);
        typFile = (TypFile *)dptr;
        if (!typFile->stream)
          return ("CRL file is not open.");

        /* Rewind the file */
        fseek (typFile->stream, 0L, 0);

        /* Look for CRL until end of file.
         */
        do {
          if ((errorMessage = GetNextCRLFromFile
               (&gotCRL, &newCRL, &lastUpdate, typFile->stream,
                publicKeyHexDigest, time)) != (char *)NULL)
            return (errorMessage);

          if (!gotCRL)
            /* Finished with this record.  Try the next. */
            continue;

          if (lastUpdate > bestLastUpdate) {
            /* Found a more recent CRL, so assign it to the return arg. */
            *crlDER = newCRL;
            bestLastUpdate = lastUpdate;
          }
          else
            /* Free newCRL for next pass */
            free (newCRL);
        } while (!feof (typFile->stream));
      ENDFORLIST;
    }
  }

  return ((char *)NULL);
}

/* Read the next record from stream with a public key digest (for the
     CRL issuer) equal to publicKeyHexDigest and a lastUpdate equal to
     or before time.  Return the lastUpdate and the allocated crlDER.
   publicKeyHexDigest is a null-terminated string of the hex value of
     the digest.
   Set found to whether a field is found.
   Return NULL for success, otherwise error string.
 */
static char *GetNextCRLFromFile
  (found, crlDER, lastUpdate, stream, publicKeyHexDigest, time)
BOOL *found;
unsigned char **crlDER;
UINT4 *lastUpdate;
FILE *stream;
char *publicKeyHexDigest;
UINT4 time;
{
  char value[VALUELEN];
  BOOL gotRecord;
  char *errorMessage;
  unsigned int numBytes;

  /* Default to not found */
  *found = FALSE;
  *crlDER = (unsigned char *)NULL;

  /* Position to just after the line containing the hex digest.
   */
  gotRecord = FALSE;
  while (GetFileLine
         (stream, PUBLIC_KEY_DIGEST_FIELD, value, sizeof (value))) {
    if (!R_match (value, publicKeyHexDigest))
      /* Try next one. */
      continue;
      
    /* We are now in the section of the file that corresponds
       to this issuer public key.  Get the lastUpdate time.
       Assume this is on the next line in the file. */
    GetFileLine (stream, LAST_UPDATE_FIELD, value, sizeof (value));
    sscanf (value, "%lX", lastUpdate);

    if (*lastUpdate > time)
      /* This lastUpdate time is later than the specified value, so
           try the next one. */
      continue;

    gotRecord = TRUE;
    break;
  }
  if (!gotRecord)
    return ((char *)NULL);

  /* Position on CRL field. Assume the "CRLInfo:" is on a line by itself.
   */
  if (!PosFileLine (stream, CRL_INFO_FIELD))
    /* Didn't find a CRLInfo in this record. *found is already false */
    return ((char *)NULL);

  /* Now read the RFC1113-encoded lines and translate
     them to binary. */
  if ((errorMessage = ReadEncodedField (crlDER, &numBytes, stream))
      != (char *)NULL)
    return (errorMessage);

  *found = TRUE;
  return ((char *)NULL);
}

/* Write the cert's subject smartname, the cert and the public key digest
     to the public key file.  certDERLen is computed internally when certDER
     is decoded to get the smart name, etc.
   This opens the filename in the first entry in
     ripemDatabase->pubKeySource.filelist opened in "a" mode.  If there are no
     entries, this does nothing.  If the file cannot be opened, this
     returns an error.
   Return NULL for success, otherwise error.
 */
char *WriteCert (certDER, ripemDatabase)
unsigned char *certDER;
RIPEMDatabase *ripemDatabase;
{
  CertificateStruct *certStruct = (CertificateStruct *)NULL;
  char *errorMessage = (char *)NULL, hexDigest[33];
  int certDERLen;
  FILE *pubOutStream = (FILE *)NULL;
  unsigned char digest[16];

  /* For error, break to end of do while (0) block. */
  do {
    if (!ripemDatabase->pubKeySource.filelist.firstptr)
      /* No public key source entries. */
      break;

    if ((pubOutStream = fopen
         (((TypFile *)ripemDatabase->pubKeySource.filelist.firstptr->dataptr)->
          filename, "a")) == (FILE *)NULL) {
      errorMessage = "Cannot open public key file for append";
      break;
    }
    
    /* Allocate the certStruct on the heap because it's big. */
    if ((certStruct = (CertificateStruct *)malloc
         (sizeof (*certStruct))) == (CertificateStruct *)NULL) {
      errorMessage = ERR_MALLOC;
      break;
    }

    if ((certDERLen = DERToCertificate
         (certDER, certStruct, (CertFieldPointers *)NULL)) < 0) {
      errorMessage = "Can't decode certificate while saving it";
      break;
    }

    /* Print seperating blank. */
    fprintf (pubOutStream, "\n");
    
    /* We are going to use the smart name in the "username" field.
       Write a blank line and the Username.
     */
    fprintf
      (pubOutStream, "%s %s\n", USER_FIELD,
       GetDNSmartNameValue (&certStruct->subject));

    /* Write the digest of the public key.
       Note that we write this before the certificate since we may
         need to use it as an index field in the future.
     */
    if ((errorMessage = GetPublicKeyDigest (digest, &certStruct->publicKey))
        != (char *)NULL)
      break;
    BinToHex (digest, 16, hexDigest);
    fprintf (pubOutStream, "%s %s\n", PUBLIC_KEY_DIGEST_FIELD, hexDigest);

    /* Encode and output the cert DER.
     */
    fprintf (pubOutStream, "%s\n", CERT_INFO_FIELD);
    CodeAndWriteBytes (certDER, (unsigned int)certDERLen, " ", pubOutStream);
  } while (0);

  free (certStruct);
  if (pubOutStream != (FILE *)NULL)
    fclose (pubOutStream);
  return (errorMessage);
}

/* Write the given digest of the issuer's public key and the
     crlDER to the CRL file.  crlDERLen is computed internally when crlDER
     is decoded to get the last update.
   This opens the filename in the first entry in
     ripemDatabase->crlSource.filelist opened in "a" mode.  If there are no
     entries, this does nothing.  If the file cannot be opened, this
     returns an error.
   Return NULL for success, otherwise error.
 */
char *WriteCRL (crlDER, publicKeyDigest, ripemDatabase)
unsigned char *crlDER;
unsigned char *publicKeyDigest;
RIPEMDatabase *ripemDatabase;
{
  char hexDigest[33], *errorMessage = (char *)NULL;
  CRLStruct *crlStruct = (CRLStruct *)NULL;
  int crlDERLen;
  FILE *crlOutStream = (FILE *)NULL;

  /* For error, break to end of do while (0) block. */
  do {
    if (!ripemDatabase->crlSource.filelist.firstptr)
      /* No CRL entries. */
      break;

    if ((crlOutStream = fopen
         (((TypFile *)ripemDatabase->crlSource.filelist.firstptr->dataptr)->
          filename, "a")) == (FILE *)NULL) {
      errorMessage = "Cannot open CRL file for append";
      break;
    }
    
    /* Allocate the crlStruct on the heap because it's big. */
    if ((crlStruct = (CRLStruct *)malloc
         (sizeof (*crlStruct))) == (CRLStruct *)NULL) {
      errorMessage = ERR_MALLOC;
      break;
    }

    if ((crlDERLen = DERToCRL
         (crlDER, crlStruct, (CRLFieldPointers *)NULL)) < 0) {
      errorMessage = "Can't decode CRL while saving it";
      break;
    }

    /* Print seperating blank. */
    fprintf (crlOutStream, "\n");
    
    /* Write the digest of the issuer's public key.
     */
    BinToHex (publicKeyDigest, 16, hexDigest);
    fprintf (crlOutStream, "%s %s\n", PUBLIC_KEY_DIGEST_FIELD, hexDigest);

    /* Write the last update time as a hex number. */
    fprintf
      (crlOutStream, "%s %lX\n", LAST_UPDATE_FIELD, crlStruct->lastUpdate);

    /* Encode and output the DER.
     */
    fprintf (crlOutStream, "%s\n", CRL_INFO_FIELD);
    CodeAndWriteBytes (crlDER, crlDERLen, " ", crlOutStream);
  } while (0);

  free (crlStruct);
  if (crlOutStream != (FILE *)NULL)
    fclose (crlOutStream);
  return (errorMessage);
}

/* Open ripemDatabase->preferencesFilename and search for the user's
     preferences where the user is given by the public key digest.
   If found, preferencesDER is set to an allocated buffer containing the
     preferences DER. The caller must free this buffer.  If not found,
     preferencesDER is set to NULL.  If ripemDatabase->preferencesFilename is
     NULL or the file cannot be opened for read, this also sets
     preferencesDER to NULL.
   ripemInfo is for debug info only.
   Returns NULL for success, otherwise error string.
 */
char *GetPreferencesByDigest
  (ripemDatabase, preferencesDER, publicKeyDigest, ripemInfo)
RIPEMDatabase *ripemDatabase;
unsigned char **preferencesDER;
unsigned char *publicKeyDigest;
RIPEMInfo *ripemInfo;
{
  char *errorMessage = (char *)NULL, publicKeyHexDigest[33];
  FILE *stream = (FILE *)NULL;
  unsigned int derLen;
  BOOL found; 
  
  if(ripemInfo->debug>1) {
    fprintf(ripemInfo->debugStream,
      "GetPreferencesByDigest looking for %s\n",
      PUBLIC_KEY_DIGEST_FIELD);
  }
   

  /* Initialize allocated pointer to NULL. */
  *preferencesDER = (unsigned char *)NULL;

  /* For error, break to end of do while (0) block. */
  do {
    if (ripemDatabase->preferencesFilename == (char *)NULL)
      /* preferencesDER is already NULL.  Just return. */
      break;

    if ((stream = fopen (ripemDatabase->preferencesFilename, "r"))
        == (FILE *)NULL)
      /* preferencesDER is already NULL.  Just return. */
      break;

    /* We need to match on the hex digest of the public key */
    BinToHex (publicKeyDigest, 16, publicKeyHexDigest);

    if ((errorMessage = GetKeyBytesFromFile
         (PUBLIC_KEY_DIGEST_FIELD, publicKeyHexDigest, stream,
          PREFERENCES_INFO_FIELD, &found, preferencesDER, &derLen,
          ripemInfo)) != (char *)NULL)
      break;
  } while (0);

  if (stream != (FILE *)NULL)
    fclose (stream);
  return (errorMessage);
}

/* Open ripemDatabase->preferencesFilename for "w" and write the
     encoded preferences in der of length derLen with a key field of
     the publicKeyDigest.
   This overwites any previous info in the file.  In the future this
     should be enhanced to replace any record in the file with the
     public key digest as the key field, but presently, there is
     no code for such "update" database operations.
   It is an error if the filename is NULL.
 */
char *WriteRIPEMPreferences (der, derLen, publicKeyDigest, ripemDatabase)
unsigned char *der;
unsigned int derLen;
unsigned char *publicKeyDigest;
RIPEMDatabase *ripemDatabase;
{
  FILE *stream;
  char *errorMessage = (char *)NULL, hexDigest[33];

  /* For error, break to end of do while (0) block. */
  do {
    if (ripemDatabase->preferencesFilename == (char *)NULL) {
      errorMessage = "Preferences filename is not specified.";
      break;
    }

    if ((stream = fopen (ripemDatabase->preferencesFilename, "w"))
        == (FILE *)NULL) {
      errorMessage = "Can't open preferences file for writing.";
      break;
    }

    /* Print seperating blank. */
    fprintf (stream, "\n");
    
    /* Write the digest of the public key.
     */
    BinToHex (publicKeyDigest, 16, hexDigest);
    fprintf (stream, "%s %s\n", PUBLIC_KEY_DIGEST_FIELD, hexDigest);

    /* Encode and output the DER.
     */
    fprintf (stream, "%s\n", PREFERENCES_INFO_FIELD);
    CodeAndWriteBytes (der, derLen, " ", stream);
  } while (0);

  if (stream != (FILE *)NULL)
    fclose (stream);
  return (errorMessage);
}

/* Open all the entries in keySource's filelist for read.
   This makes sure outFilename is at the front of the
     list and that it can be opened for append.  This assumes outFilename
     is already in the keySource list.
   ripemInfo is used only for error and debug output.
   Return NULL for success, otherwise error string.
 */
static char *OpenKeySource (keySource, outFilename, ripemInfo)
TypKeySource *keySource;
char *outFilename;
RIPEMInfo *ripemInfo;
{
  FILE *stream;
  TypListEntry *entry;
  TypFile *typFile;
  void *tempPointer;

  /* Make sure we can open the outFilename for append.  This will create
       it if it doesn't exist. */
  if ((stream = fopen (outFilename, "a")) == (FILE *)NULL) {
    sprintf (ripemInfo->errMsgTxt,
             "Can't write to file \"%s\". (Does its directory exist?)",
             outFilename);
    return (ripemInfo->errMsgTxt);
  }
  else
    /* Successfully opened, so just close it to be opened for read. */
    fclose (stream);

  /* We want to make sure the output filename is the first
       in the list since that is the one RIPEM will use.  First
       scan to see if this filename is already listed as one of
       the key sources. */
  for (entry = keySource->filelist.firstptr;
       entry != (TypListEntry *)NULL; entry = entry->nextptr) {
    if (strcmp (((TypFile *)entry->dataptr)->filename, outFilename) == 0) {
      /* This output filename is already in the list, make sure it
           is the first one and quit. */
      tempPointer = keySource->filelist.firstptr->dataptr;
      keySource->filelist.firstptr->dataptr = entry->dataptr;
      entry->dataptr = tempPointer;
      break;
    }
  }    

  /* Open the files.
   */
  for (entry = keySource->filelist.firstptr;
       entry != (TypListEntry *)NULL; entry = entry->nextptr) {
    typFile = (TypFile *)entry->dataptr;

    if (typFile->stream != (FILE *)NULL)
      /* Somehow, this is already opened. */
      continue;

    if ((typFile->stream = fopen (typFile->filename, "r")) == (FILE *)NULL) {
      sprintf(ripemInfo->errMsgTxt,
              "Can't open file \"%s\" for read", typFile->filename);
      return (ripemInfo->errMsgTxt);
    }
  }

  return ((char *)NULL);
}

