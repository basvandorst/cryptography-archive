/*--- ripemmai.c --  Main program for RIPEM
 *
 *   RIPEM -- Riordan's Internet Privacy Enhanced Mail
 *
 *            (aka RSAREF-based Internet Privacy Enhanced Mail)
 *
 *   RIPEM is a public key encryption package.
 *
 *   This program implements a subset of RFC 1421-1424 Privacy
 *   Enhanced Mail.  It uses RSA Data Security's RSAREF cryptographic
 *   toolkit for the encryption/decryption/verification of messages.
 *
 *   "ripem" is meant to be called to pre-process a mail message
 *   prior to being sent.  The recipient runs the encrypted
 *   message through "ripem" to get the plaintext back.
 *
 *   For the calling sequence, see the usagemsg.c file.
 *   For more information, see the accompanying files
 *   in this distribution.
 *
 *   Mark Riordan   May - September 1992
 *   (After RPEM, March - May 1991.)
 *
 *   This code is hereby placed in the public domain.
 *   RSAREF, however, is not in the public domain.
 *   Therefore, use of this program must be governed by RSA DSI's
 *   RSAREF Program License.  This license basically allows free
 *   non-commercial use within the United States and Canada.
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

#include "headers.h"
#include "keyfield.h"
#include "strutilp.h"
#include "keyderpr.h"
#include "derkeypr.h"
#include "keymanpr.h"
#include "r_random.h"
#include "bemparse.h"
#include "hexbinpr.h"
#include "bfstream.h"
#include "crackhpr.h"
#include "rdwrmsgp.h"
#include "certder.h"
#include "certutil.h"
#include "p.h"

#ifdef UNIX
#ifdef __MACH__
#include <libc.h>
#endif
#include <pwd.h>
#endif

#ifdef MACTC
#include <stdlib.h>
#include <console.h>
#include <time.h>
#endif

#define IV_SIZE 8

typedef struct RIPEMEncipherFrame {
  BufferStream outStream;     /* This holds the value returned to the caller */
  enum enhance_mode enhanceMode;
  int messageFormat;
  unsigned char iv[IV_SIZE];
  /* We must save the encrypted keys throughout RIPEMEncipherDigestUpdate */
  unsigned int recipientKeyCount;
  RecipientKeyInfo *recipientKeys;   /* data must remain valid through calls */
  int encryptionAlgorithm;
  unsigned char *encryptedKeysBuffer;
  unsigned int *encryptedKeyLens;
  TypList issuerNames;
  R_ENVELOPE_CTX sealContext;
  R_SIGNATURE_CTX signatureContext;
  BOOL wroteHeader;
  BOOL lastWasNewline;                     /* Used in MIC-CLEAR dashstuffing */
  BufferStream buffer;                 /* Used to accumulate lines to encode */
} RIPEMEncipherFrame;

typedef struct RIPEMDecipherFrame {
  BufferStream outStream;     /* This holds the value returned to the caller */
  TypMsgInfo messageInfo;
  BufferStream lineIn;            /* Used to accumulate a line during Update */
  BOOL doingHeader;
  unsigned char decryptedLine[1000];
  unsigned char signature[MAX_SIGNATURE_LEN];
  unsigned int signatureLen;
  R_ENVELOPE_CTX envelopeContext;
  R_SIGNATURE_CTX signatureContext;
  R_RSA_PUBLIC_KEY senderKey;                /* used to execute final verify */
  BOOL foundEndBoundary;
  TypList headerList;                         /* for saving the email header */
  TypList certChain;                              /* Local copy of certChain */
  ChainStatusInfo chainStatus;
  BOOL prependHeaders;                    /* Saved copy of parameter to Init */
} RIPEMDecipherFrame;

static char *AddIssuerSerialAlias P((RIPEMInfo *, CertificateStruct *));
static char *WriteHeader
  P((RIPEMInfo *, unsigned char *, unsigned int, RIPEMDatabase *));
static char *WriteKeyInfo P((unsigned char *, unsigned int, BufferStream *));
static char *WriteRecipientIDAsymmetric
  P((DistinguishedNameStruct *, unsigned char *, unsigned int,
     BufferStream *));
static char *EncryptAndWritePrivateKey
  P ((RIPEMInfo *, unsigned char *, unsigned int, RIPEMDatabase *));
static char *FirstUsername P ((RIPEMInfo *));
static char *ReadEnhancedTextLine
  P((RIPEMDecipherFrame *, BufferStream *, char *));
static void RIPEMEncipherFrameConstructor P((RIPEMEncipherFrame *));
static void RIPEMEncipherFrameDestructor P((RIPEMEncipherFrame *));
static void RIPEMDecipherFrameConstructor P((RIPEMDecipherFrame *));
static void RIPEMDecipherFrameDestructor P((RIPEMDecipherFrame *));
static char *ProcessHeader P((RIPEMInfo *, BufferStream *, RIPEMDatabase *));
static char *SignFinalOutputHeader P((RIPEMInfo *, RIPEMDatabase *));
static char *AddNameToList P((TypList *, DistinguishedNameStruct *));
#ifndef RIPEMSIG
static char *RIPEMSealInit
  P ((RIPEMInfo *, R_ENVELOPE_CTX *, unsigned char *, unsigned char *,
      unsigned int *, RecipientKeyInfo *, unsigned int, int));
static int RIPEMOpenInit
  P ((RIPEMInfo *, R_ENVELOPE_CTX *, unsigned char *, unsigned int *));
static int MyDecryptPEMUpdateFinal PROTO_LIST
  ((R_ENVELOPE_CTX *, unsigned char *, unsigned int *, unsigned char *,
    unsigned int));
#endif

/* VERSION is defined in version.h.
   Note that we define a varibale instead of just putting #define VERSION
     in ripem.h so that just re-linking to a new library will ensure that
     the application gets the new version. */
#include "version.h"
char *RIPEM_VERSION = VERSION;

char *FieldNames[] = { DEF_FIELDS(MAKE_TEXT) };
char *IDNames[] = { DEF_IDS(MAKE_IDS) };
char *ERR_MALLOC = "Cannot allocate memory";
char *ERR_SELF_SIGNED_CERT_NOT_FOUND =
  "Couldn't find user's self-signed certificate";
char *ERR_PREFERENCES_NOT_FOUND = "Preferences not found";
char *ERR_PREFERENCES_CORRUPT = "Preference information is corrupt";
char *ERR_CERT_ALREADY_VALIDATED =
 "There is already a certificate with a current validity period for this user";

/* Set ripemInfo to its initial state.
 */
void RIPEMInfoConstructor (ripemInfo)
RIPEMInfo *ripemInfo;
{
  R_RandomInit (&ripemInfo->randomStruct);
  InitList (&ripemInfo->issuerCerts);
  R_memset ((POINTER)&ripemInfo->z, 0, sizeof (ripemInfo->z));
}

/* Finalize the ripemInfo, zeroizing sensitive information and freeing
    memory.
 */
void RIPEMInfoDestructor (ripemInfo)
RIPEMInfo *ripemInfo;
{
  R_RandomFinal (&ripemInfo->randomStruct);
  FreeList (&ripemInfo->issuerCerts);
  R_memset
    ((POINTER)&ripemInfo->privateKey, 0, sizeof (ripemInfo->privateKey));
  R_memset
    ((POINTER)ripemInfo->passwordDigest, 0,
     sizeof (ripemInfo->passwordDigest));
  free (ripemInfo->z.userCertDER);
  RIPEMResetPreferences (ripemInfo);
  if (ripemInfo->z.encipherFrame != (RIPEMEncipherFrame *)NULL) {
    RIPEMEncipherFrameDestructor (ripemInfo->z.encipherFrame);
    free (ripemInfo->z.encipherFrame);
  }
  if (ripemInfo->z.decipherFrame != (RIPEMDecipherFrame *)NULL) {
    RIPEMDecipherFrameDestructor (ripemInfo->z.decipherFrame);
    free (ripemInfo->z.decipherFrame);
  }
  if (ripemInfo->z.crlsFrame != (RIPEM_CRLsFrame *)NULL) {
    RIPEM_CRLsFrameDestructor (ripemInfo->z.crlsFrame);
    free (ripemInfo->z.crlsFrame);
  }
}

/* Generate a public/private keypair with the given bits and create a
     self-signed cert with the given validityMonths.
   The userDN in ripemInfo must already be set.  This writes the self-signed
     cert to the first filename in ripemDatabase->pubKeySource and
     encrypts the private key with the password, writing it to
     the first filename in ripemDatabase->privKeySource.
   The userList in ripemInfo must also be set: each of the names in userList
     is written as a User: field in the private key file.
   The randomStruct in ripemInfo must already be initialized.
   This sets ripemInfo's publicKey, privateKey and userCertDER
     (which is the user's self-signed cert).
 */
char *DoGenerateKeys
  (ripemInfo, bits, validityMonths, password, passwordLen, ripemDatabase)
RIPEMInfo *ripemInfo;
unsigned int bits;
unsigned int validityMonths;
unsigned char *password;
unsigned int passwordLen;
RIPEMDatabase *ripemDatabase;
{
  char *errorMessage = NULL;
  int retcode;
  unsigned int digestLen;
  R_RSA_PROTO_KEY proto_key;
  UINT4 now;

  /* Set up the desired properties of the key to generate. */
  proto_key.bits = bits;
  /* Always use Fermat # F4 as public exponent. */
  proto_key.useFermat4 = 1;

  R_memset((POINTER)&ripemInfo->publicKey, 0, sizeof (ripemInfo->publicKey));
  R_memset
    ((POINTER)&ripemInfo->privateKey, 0, sizeof (ripemInfo->privateKey));
  retcode = R_GeneratePEMKeys
    (&ripemInfo->publicKey, &ripemInfo->privateKey, &proto_key,
     &ripemInfo->randomStruct);

  if(retcode != 0)
    return (FormatRSAError(retcode));

  /* The key generation worked.  Now for each key component
       (public and private), translate the key to DER format,
       encode it in RFC1113 format, and write it out in an
       appropriately-formated file.
   */
  if ((errorMessage = WriteSelfSignedCert
       (ripemInfo, validityMonths, ripemDatabase)) != (char *)NULL)
    return (errorMessage);

  /* Now encode, encrypt, and write out the private key. */
  if ((errorMessage = EncryptAndWritePrivateKey
       (ripemInfo, password, passwordLen, ripemDatabase)) != (char *)NULL)
    return (errorMessage);

  /* Set the password digest for use in authenticating preferences.
       The preferences will be saved by RIPEMUpdateCRL. */
  R_DigestBlock
    (ripemInfo->passwordDigest, &digestLen, password, passwordLen, DA_MD5);

  /* Now write out a new CRL.  RIPEMUpdateCRL should create a new one
       because it's not already in the database.  Set serialNumber
       to NULL so there are no entries and default to 30 day
       validity period.
   */
  R_time (&now);
  return (RIPEMUpdateCRL
          (ripemInfo, (UINT4)(now + (UINT4)30 * (UINT4)24 * (UINT4)3600),
           (unsigned char *)NULL, 0, ripemDatabase));
}

/* Re-encrypt the user's private key with newPassword and rewrites it
     to the first private key file listed in ripemDatabase.
   The calling routine must already have called LoginUser.
   This also calls RIPEMSavePreferences so that it is re-authenticated
     under the new password.  Therefore, this will return an error if
     ripemDatabase->preferencesFilename cannot be opened.
 */
char *DoChangePW (ripemInfo, newPassword, newPasswordLen, ripemDatabase)
RIPEMInfo *ripemInfo;
unsigned char *newPassword;
unsigned int newPasswordLen;
RIPEMDatabase *ripemDatabase;
{
  char *errorMessage;
  unsigned int digestLen;
  
  if ((errorMessage = EncryptAndWritePrivateKey
       (ripemInfo, newPassword, newPasswordLen, ripemDatabase))
      != (char *)NULL)
    return (errorMessage);

  /* Recompute the passwordDigest */
  R_DigestBlock
    (ripemInfo->passwordDigest, &digestLen, newPassword, newPasswordLen,
     DA_MD5);

  return (RIPEMSavePreferences (ripemInfo, ripemDatabase));
}

/* Initialize for preparing a message according to enhanceMode.
   The calling routine must already call LoginUser.
   If enhanceMode is MODE_ENCRYPTED, then encryptionAlgorithm is
     a value recognized by RSAREF, like EA_DES_CBC.  Also, recipientKeys
     is an array of recipientKeyCount RecipientKeyInfo structs.  These
     give the public keys and usernames of the recipients.  The username
     is used for backward compatibility with RIPEM 1.1 and for looking
     up the user's issuer name and serial number for MESSAGE_FORMAT_PEM
     compatibility. The randomStruct in ripemInfo must already be initialized.
     The data pointed to by recipientKeys must remain valid until the first
     call to RIPEMEncipherUpdate.
   messageFormat must be MESSAGE_FORMAT_RIPEM1 or MESSAGE_FORMAT_PEM to
     choose which format the message is compatible with.
     MESSAGE_FORMAT_RIPEM1 is compatible with RIPEM 1.1 and 1.2.  It has
     a Proc-Type version of 2001 and includes the Originator-Name field and
     uses Recipient-Key-Asymmetric (containing the public key) for the
     recipients of encrypted messages. MESSAGE_FORMAT_PEM is for comatibility
     with the RFC 1421 standards suite.  It has a Proc-Type version of 4 and
     omits RIPEM-specific fields such as Originator-Name and
     uses only Recipient-ID-Asymmetric (containing issuer name and serial
     number) for the recipients of encrypted messages.
   After this, call RIPEMEncipherDigestUpdate to digest the text
     by parts, RIPEMEncipherUpdate to enhance the text by parts,
     and call RIPEMEncipherFinal to finish.
   Return NULL for success or error string.
 */
char *RIPEMEncipherInit
  (ripemInfo, enhanceMode, messageFormat, encryptionAlgorithm, recipientKeys,
   recipientKeyCount)
RIPEMInfo *ripemInfo;
enum enhance_mode enhanceMode;
int messageFormat;
int encryptionAlgorithm;
RecipientKeyInfo *recipientKeys;
unsigned int recipientKeyCount;
{
  RIPEMEncipherFrame *frame;
  int status;
#ifndef RIPEMSIG
  char *errorMessage;
#endif
  
#ifdef RIPEMSIG
UNUSED_ARG (encryptionAlgorithm)
UNUSED_ARG (recipientKeys)
UNUSED_ARG (recipientKeyCount)
#endif
  
  if (enhanceMode != MODE_ENCRYPTED && enhanceMode != MODE_MIC_ONLY &&
      enhanceMode != MODE_MIC_CLEAR)
    return ("Invalid encryption mode.");
  if (messageFormat != MESSAGE_FORMAT_RIPEM1 &&
      messageFormat != MESSAGE_FORMAT_PEM)
    return ("Invalid message format.");

  /* Make sure any old frame is deleted and make a new one.
   */
  if (ripemInfo->z.encipherFrame != (RIPEMEncipherFrame *)NULL) {
    RIPEMEncipherFrameDestructor (ripemInfo->z.encipherFrame);
    free (ripemInfo->z.encipherFrame);
  }
  if ((ripemInfo->z.encipherFrame = (RIPEMEncipherFrame *)malloc
       (sizeof (*ripemInfo->z.encipherFrame))) == (RIPEMEncipherFrame *)NULL)
    return (ERR_MALLOC);
  RIPEMEncipherFrameConstructor (ripemInfo->z.encipherFrame);

  frame = ripemInfo->z.encipherFrame;
  frame->enhanceMode = enhanceMode;
  frame->messageFormat = messageFormat;

  if (enhanceMode == MODE_ENCRYPTED) {
#ifdef RIPEMSIG
    return ("RIPEM/SIG cannot prepare encrypted messages. You may prepare signed messages.");
#else
    if (recipientKeyCount == 0)
      return ("You must specify at least one recipient");

    frame->recipientKeyCount = recipientKeyCount;
    frame->recipientKeys = recipientKeys;
    frame->encryptionAlgorithm = encryptionAlgorithm;
    
    /* Allocate arrays for the encrypted key pointers.
     */
    if ((frame->encryptedKeysBuffer = (unsigned char *)malloc
         (recipientKeyCount * MAX_ENCRYPTED_KEY_LEN)) ==
        (unsigned char *)NULL)
      return (ERR_MALLOC);
    if ((frame->encryptedKeyLens = (unsigned int *)malloc
         (recipientKeyCount * sizeof (*frame->encryptedKeyLens))) ==
        (unsigned int *)NULL)
      return (ERR_MALLOC);

    /* Create all of the recipient key info blocks and generate the iv.
       We can't output now because the signature comes before the
         encrypted keys in the header.
     */
    if ((errorMessage = RIPEMSealInit
         (ripemInfo, &frame->sealContext, frame->iv,
          frame->encryptedKeysBuffer, frame->encryptedKeyLens, recipientKeys,
          recipientKeyCount, encryptionAlgorithm)) != (char *)NULL)
      return (errorMessage);
#endif
  }
  
  /* Initialize signature */
  if ((status = R_SignInit (&frame->signatureContext, DA_MD5)) != 0)
    return (FormatRSAError (status));
  
  return ((char *)NULL);
}

/* This must be called because the signature comes before the enhanced text
     so we must digest the text first before we can output the signature
     and enhanced the text.  Call this zero or more times to supply
     the input text.  Then call RIPEMEncipherUpdate to actually enhance
     and output the text.  Between the last call to RIPEMEncipherDigestUpdate
     and the first call to RIPEMEncipherUpdate, you must rewind the input.
   The text to enhance is in partIn with length partInLen.  Textual lines must
     be delimited by the '\n' character (not <CR><LF>).  This can be
     done for example by reading from a file with fgets or fread in text mode.
     This routine will convert to <CR><LF> internally as needed for
     digesting.  Make sure there is a \n at the end of the final line.
   Return NULL for success or error string.
 */
char *RIPEMEncipherDigestUpdate (ripemInfo, partIn, partInLen)
RIPEMInfo *ripemInfo;
unsigned char *partIn;
unsigned int partInLen;
{
  RIPEMEncipherFrame *frame = ripemInfo->z.encipherFrame;
  unsigned int i, status;

  if (frame == (RIPEMEncipherFrame *)NULL)
    return ("Encipher not initialized");

  /* We must digest the text, substituting <CR><LF> for all '\n'.
   */
  while (1) {
    /* Position i at the first \n or at the end of partIn and then digest.
     */
    for (i = 0; i < partInLen && partIn[i] != '\n'; ++i);
    if ((status = R_SignUpdate (&frame->signatureContext, partIn, i)) != 0)
      return (FormatRSAError (status));

    if (i == partInLen)
      /* We have digested up the end of partIn, so return to await another
           call to DigestUpdate. */
      return ((char *)NULL);

    /* i is positioned on a '\n', so digest a <CR><LF>.
     */
    if ((status = R_SignUpdate
         (&frame->signatureContext, (unsigned char *)"\r\n", 2)) != 0)
      return (FormatRSAError (status));

    /* Move partIn past the \n.
     */
    ++i;
    partInLen -= i;
    partIn += i;
  }
}

/* (Before this is called for the first time, the caller should have made
     calls to RIPEMEncipherDigestUpdate and then rewound the input.)
   This returns a pointer to the output in partOut and its length in
     partOutLen.  The memory for the output is allocated inside ripemInfo
     and should be treated as "read only".  Upon return, the output must be
     copied or written to a file since future calls to RIPEM may modify it. On
     error return, the pointer to the output is undefined.
   The output is textual lines delimited by the character '\n'.
     The caller must convert these to the local line delimiter, such as
     writing to a file with fwrite in text mode.
   The text to enhance is in partIn with length partInLen.  Textual lines must
     be delimited by the '\n' character (not <CR><LF>).  This can be
     done for example by reading from a file with fgets or fread in text mode.
     This routine will convert to <CR><LF> internally as needed.
     Make sure there is a \n at the end of the final line.
   ripemDatabase is used for selecting certificates to find issuer names
     and serial numbers of recipients for MESSAGE_FORMAT_PEM.
   Return NULL for success or error string.
 */
char *RIPEMEncipherUpdate
  (ripemInfo, partOut, partOutLen, partIn, partInLen, ripemDatabase)
RIPEMInfo *ripemInfo;
unsigned char **partOut;
unsigned int *partOutLen;
unsigned char *partIn;
unsigned int partInLen;
RIPEMDatabase *ripemDatabase;
{
  RIPEMEncipherFrame *frame = ripemInfo->z.encipherFrame;
  char *errorMessage;
  unsigned int i, remainingBytes;
#ifndef RIPEMSIG
  int status;
  unsigned int localPartOutLen;
#endif

  if (frame == (RIPEMEncipherFrame *)NULL)
    return ("Encipher not initialized");

  /* Get ready to write to the output. */
  BufferStreamRewind (&frame->outStream);

  if (!frame->wroteHeader) {
    /* This is the first call to Update, so write out the header. */

    if ((errorMessage = SignFinalOutputHeader (ripemInfo, ripemDatabase))
        != (char *)NULL)
      return (errorMessage);
    frame->wroteHeader = TRUE;
  }

  if (ripemInfo->debug > 1) {
#define DEBUGCHARS 500
    unsigned int maxbytes =
      partInLen < DEBUGCHARS ? partInLen : DEBUGCHARS, idx=0;
    unsigned char dch;

    fprintf (ripemInfo->debugStream,"%d bytes in; first %d bytes are:\n",
             partInLen, maxbytes);
    while (idx < maxbytes) {
      dch = partIn[idx++];
      if (isprint((char)dch))
        putc ((char)dch,ripemInfo->debugStream);
      else
        fprintf (ripemInfo->debugStream, "\\x%2.2x", dch);
    }
    putc ('\n', ripemInfo->debugStream);
  }

  if (frame->enhanceMode == MODE_ENCRYPTED ||
      frame->enhanceMode == MODE_MIC_ONLY) {
    /* First convert \n in the partIn to <CR><LF> by writing to
         the frame->buffer.
     */
    while (1) {
      /* Position i at the first \n or at the end of partIn and then
           copy bytes up to here into the buffer. */
      for (i = 0; i < partInLen && partIn[i] != '\n'; ++i);
      if ((errorMessage = BufferStreamWrite (partIn, i, &frame->buffer))
          != (char *)NULL)
        return (errorMessage);

      if (i == partInLen)
        /* We have copied up the end of partIn, so break. */
        break;

      /* i is positioned on a '\n', so copy a <CR><LF>.
       */
      if ((errorMessage = BufferStreamWrite
           ((unsigned char *)"\r\n", 2, &frame->buffer)) != (char *)NULL)
        return (errorMessage);

      /* Move partIn past the \n. */
      ++i;
      partInLen -= i;
      partIn += i;
    }

    /* Set i to as many multiples of ENCODE_CHUNKSIZE in buffer. */
    i = (frame->buffer.point / ENCODE_CHUNKSIZE) * ENCODE_CHUNKSIZE;

    if (i > 0) {
#ifndef RIPEMSIG
      if (frame->enhanceMode == MODE_ENCRYPTED) {
        /* Becuase i is a multiple of ENCODED_CHUNKSIZE which is
             a multiple of the DES block size (8), we can encrypt
             in place without any leftover.  localPartOutLen should equal
             the part in len.
         */
        if ((status = R_SealUpdate
             (&frame->sealContext, frame->buffer.buffer, &localPartOutLen,
              frame->buffer.buffer, i)) != 0)
          return (FormatRSAError (status));

        if (ripemInfo->debug > 1) {
          char hex_digest[36];

          fprintf
            (ripemInfo->debugStream, "EncContentLen=%u (not recoded)\n",
             localPartOutLen);
          MakeHexDigest (frame->buffer.buffer, localPartOutLen, hex_digest);
          fprintf
            (ripemInfo->debugStream, " MD5 of encrypted content = %s\n",
             hex_digest);
        }
      }
#endif
      /* Because i is a multiple of ENCODE_CHUNKSIZE, all lines will
           be written of the same length. */
      if ((errorMessage = BufferCodeAndWriteBytes
           (frame->buffer.buffer, i, "", &frame->outStream)) != (char *)NULL)
        return (errorMessage);

      /* Rewind and write the remaining bytes to the beginning of
           the buffer.  This will not overlap because the remaining bytes
           start >= ENCODE_CHUNKSIZE, but there are less than
           ENCODE_CHUNKSIZE remaining bytes.  Set remainingBytes before
           we rewind the point to 0. */
      remainingBytes = frame->buffer.point - i;
      BufferStreamRewind (&frame->buffer);
      if ((errorMessage = BufferStreamWrite
           (frame->buffer.buffer + i, remainingBytes, &frame->buffer))
          != (char *)NULL)
        return (errorMessage);
    }
  }
  else {
    /* MIC-CLEAR.  Copy the input to the output, changing "\n-"
         to "\n- -".  Note that we don't need to use the frame->buffer.
     */

    if (frame->lastWasNewline) {
      /* Special case: the character at the end of the previous partIn
           was a newline, so see if this partIn starts with a dash. */
      if (partInLen > 0 && partIn[0] == '-') {
        /* partIn starts with a dash and we have already output the
             newline, so output a "- -".
         */
        if ((errorMessage = BufferStreamWrite
             ((unsigned char *)"- -", 3, &frame->outStream)) != (char *)NULL)
          return (errorMessage);

        /* Move partIn past the dash. */
        --partInLen;
        ++partIn;
      }

      frame->lastWasNewline = FALSE;
    }

    while (1) {
      /* Position i at the first "\n-" or at the end of partIn and then
           copy bytes up to here to the output. */
      for (i = 0; i < partInLen; ++i) {
        if (partIn[i] == '\n' && (i + 1 < partInLen) &&
            partIn[i + 1] == '-')
          break;
      }
      if ((errorMessage = BufferStreamWrite (partIn, i, &frame->outStream))
          != (char *)NULL)
        return (errorMessage);

      if (i == partInLen) {
        /* We have copied up the end of partIn, so break.  Set the
             lastWasNewline flag so we can check new input for
             '-' at the beginning of line. */
        if (partInLen > 0 && partIn[partInLen - 1] == '\n')
          frame->lastWasNewline = TRUE;
        break;
      }

      /* i is positioned on a "\n-", so copy a "\n- -".
       */
      if ((errorMessage = BufferStreamWrite
           ((unsigned char *)"\n- -", 4, &frame->outStream)) != (char *)NULL)
        return (errorMessage);

      /* Move partIn past the "\n-". */
      i += 2;
      partInLen -= i;
      partIn += i;
    }
  }

  /* Set the output. */
  *partOut = frame->outStream.buffer;
  *partOutLen = frame->outStream.point;

  return ((char *)NULL);
}

/* Call this after all text has been fed to RIPEMEncipherUpdate.  This
     flushes the output and writes the end message boundary.  See
     RIPEMEncipherUpdate for a description of partOut and partOutLen.
   Note that if the message text is blank and RIPEMEncipherUpdate was
     never called, this will still output a header and empty message.
   ripemDatabase is used for selecting certificates to find issuer names
     and serial numbers of recipients for MESSAGE_FORMAT_PEM.
   Return NULL for success or error string.
 */
char *RIPEMEncipherFinal (ripemInfo, partOut, partOutLen, ripemDatabase)
RIPEMInfo *ripemInfo;
unsigned char **partOut;
unsigned int *partOutLen;
RIPEMDatabase *ripemDatabase;
{
  RIPEMEncipherFrame *frame = ripemInfo->z.encipherFrame;
  char *errorMessage;
#ifndef RIPEMSIG
  unsigned int updateLen, finalLen;
  unsigned char finalBuffer[8];
  int status;
#endif
  
  if (frame == (RIPEMEncipherFrame *)NULL)
    return ("Encipher not initialized");

  /* Get ready to write to the output. */
  BufferStreamRewind (&frame->outStream);

  if (!frame->wroteHeader) {
    /* This implies the text is zero length. */
    if ((errorMessage = SignFinalOutputHeader (ripemInfo, ripemDatabase))
        != (char *)NULL)
      return (errorMessage);
  }

#ifndef RIPEMSIG
  if (frame->enhanceMode == MODE_ENCRYPTED) {
    /* Encrypt update the buffer's remaining bytes in place.  For encrypt
         final, use a local buffer since it might write past the frame's
         buffer end.
     */
    if ((status = R_SealUpdate
         (&frame->sealContext, frame->buffer.buffer, &updateLen,
          frame->buffer.buffer, frame->buffer.point)) != 0)
      return (FormatRSAError (status));
    if ((status = R_SealFinal
         (&frame->sealContext, finalBuffer, &finalLen)) != 0)
      return (FormatRSAError (status));
    /* Now set the buffer's point to where update has written bytes to
         and write the result of seal final.
     */
    frame->buffer.point = updateLen;
    if ((errorMessage = BufferStreamWrite
         (finalBuffer, finalLen, &frame->buffer)) != (char *)NULL)
      return (errorMessage);

    /* Encode and write out.
     */
    if ((errorMessage = BufferCodeAndWriteBytes
         (frame->buffer.buffer, frame->buffer.point, "", &frame->outStream))
        != (char *)NULL)
      return (errorMessage);
  }
#endif
  if (frame->enhanceMode == MODE_MIC_ONLY) {
    /* Encode and write out any remaining bytes in the buffer.
     */
    if ((errorMessage = BufferCodeAndWriteBytes
         (frame->buffer.buffer, frame->buffer.point, "", &frame->outStream))
        != (char *)NULL)
      return (errorMessage);
  }
  /* For MIC-CLEAR, assume the last line ended with a \n and has been
       processed. */

  if ((errorMessage = BufferStreamPuts (HEADER_STRING_END, &frame->outStream))
      != (char *)NULL)
    return (errorMessage);
  if ((errorMessage = WriteEOL (&frame->outStream)) != (char *)NULL)
    return (errorMessage);

  /* Set the output. */
  *partOut = frame->outStream.buffer;
  *partOutLen = frame->outStream.point;

  return ((char *)NULL);
}

/* Initialize ripemInfo for deciphering a new message.
   If prependHeaders is TRUE, the email headers from the input will be written
     to the output before the deciphered text.
   After this, call RIPEMDecipherUpdate to supply the enhanced message
     by parts and call RIPEMDecipherFinal to finish and obtain the
     sender information.
   Return NULL for success or error string.
 */
char *RIPEMDecipherInit (ripemInfo, prependHeaders)
RIPEMInfo *ripemInfo;
BOOL prependHeaders;
{
  /* Make sure any old frame is deleted and make a new one.
   */
  if (ripemInfo->z.decipherFrame != (RIPEMDecipherFrame *)NULL) {
    RIPEMDecipherFrameDestructor (ripemInfo->z.decipherFrame);
    free (ripemInfo->z.decipherFrame);
  }
  if ((ripemInfo->z.decipherFrame = (RIPEMDecipherFrame *)malloc
       (sizeof (*ripemInfo->z.decipherFrame))) == (RIPEMDecipherFrame *)NULL)
    return (ERR_MALLOC);
  RIPEMDecipherFrameConstructor (ripemInfo->z.decipherFrame);

  ripemInfo->z.decipherFrame->prependHeaders = prependHeaders;

  return ((char *)NULL);
}

/* Decipher the message in partIn, which contains the enhanced
     message with lines delimited by '\n' (not <CR><LF>).  partIn
     may contain parts of lines or multiple lines.
  This returns a pointer to the output in partOut and its length in
     partOutLen.  The memory for the output is allocated inside ripemInfo
     and should be treated as "read only".  Upon return, the output must be
     copied or written to a file since future calls to RIPEM may modify it. On
     error return, the pointer to the output is undefined.
   This converts any <CR><LF> line delimiters in the enhanced message
     back to '\n' in the output.
   Return NULL for success or error string.
 */
char *RIPEMDecipherUpdate
  (ripemInfo, partOut, partOutLen, partIn, partInLen, ripemDatabase)
RIPEMInfo *ripemInfo;
unsigned char **partOut;
unsigned int *partOutLen;
unsigned char *partIn;
unsigned int partInLen;
RIPEMDatabase *ripemDatabase;
{
  char *errorMessage = (char *)NULL;
  RIPEMDecipherFrame *frame = ripemInfo->z.decipherFrame;
  int status;
  unsigned int i;

  if (frame == (RIPEMDecipherFrame *)NULL)
    return ("Decipher not initialized");

  /* Get ready to write to the output. */
  BufferStreamRewind (&frame->outStream);

  while (1) {
    if (!frame->doingHeader && frame->foundEndBoundary)
      /* partIn only contains stuff after the end boundary, so
           ignore. partOutLen will be set to zero below */
      break;

    /* Position i at the first \n or at the end of partIn and then
         copy bytes up to here into lineIn. */
    for (i = 0; i < partInLen && partIn[i] != '\n'; ++i);
    if ((errorMessage = BufferStreamWrite (partIn, i, &frame->lineIn))
        != (char *)NULL)
      return (errorMessage);

    if (i == partInLen)
      /* We have copied up the end of partIn, so break to set the
           partOutLen and return to await another call to Update
           to give the rest of the line */
      break;

    /* i is positioned on a '\n', so null terminate lineIn and process.
     */
    if ((errorMessage = BufferStreamPutc (0, &frame->lineIn))
        != (char *)NULL)
      return (errorMessage);
    if (frame->doingHeader) {
      if ((errorMessage = ProcessHeaderLine
           (ripemInfo, &frame->messageInfo, &frame->doingHeader,
            (char *)frame->lineIn.buffer, frame->prependHeaders,
            &frame->headerList)) != (char *)NULL)
        return (errorMessage);

      if (!frame->doingHeader) {
        /* We have finished reading the header, so process it.
           The doingHeader flag could have been set false by an end
             PEM boundary.  Assume this only happens for CRL messages since
             process type MIC-ONLY, etc. will assume the header ended with
             a blank line and will continue to try to read the message text.
         */
        if ((errorMessage = ProcessHeader
             (ripemInfo, &frame->outStream, ripemDatabase)) != (char *)NULL)
          return (errorMessage);
          
        if (frame->foundEndBoundary)
          /* ProcessHeader set this flag which means there is nothing
               more to process. */
          break;

        /* Initialize the signature context. */
        if ((status = R_VerifyInit
             (&frame->signatureContext, frame->messageInfo.da)) != 0)
            return (FormatRSAError (status));
          
        if (ripemInfo->debug > 1)
          fprintf (ripemInfo->debugStream,
                   "Before call to ReadEnhancedTextLine, Username=%s\n",
                   FirstUsername (ripemInfo));
      }
    }
    else {
      /* We have already processed the header.
         Now process the text and write out the result.
         If already found the end PEM boundary, this will do nothing.
       */
      if ((errorMessage = ReadEnhancedTextLine
           (frame, &frame->outStream, (char *)frame->lineIn.buffer))
          != (char *)NULL)
        return (errorMessage);
    }

    /* Move partIn past the \n and rewind the lineIn to start a new line.
     */
    ++i;
    partInLen -= i;
    partIn += i;
    BufferStreamRewind (&frame->lineIn);
  }

  /* Set the output. */
  *partOut = frame->outStream.buffer;
  *partOutLen = frame->outStream.point;

  return ((char *)NULL);
}

/* Call this after the entire enhanced message has been supplied to
     RIPEMDecipherUpdate.
   Return the sender's certChain and chainStatus. The calling routine must
     InitList certChain.  chainStatus is pointer to a ChainStatusInfo struct.
   If chainStatus->overall is 0, this could not find a valid public key for
     the sender and enhanceMode is undefined. In this case, if the message
     contained a self-signed certificate, the certChain contains one entry
     which is the self-signed cert.  The calling routine may decode it and
     present the self-signed cert digest, and use ValidateAndWriteCert to
     validate the user.
   If chainStatus->overall is CERT_UNVALIDATED, then this could not find a
     certificate for the sender, but could find an unvalidated public key.
     In this case, certChain contains one entry which is the sender's
     username.  This is provided only for compatibility with RIPEM 1.1.
   For other values of chainStatus->overall, certChain and chainStatus
     contains values as described in SelectCertChain.  Note: the sender
     name is the subject of the cert at the "bottom" of the chain.
   Return the enhanced mode, such as MODE_ENCRYPTED, in enhanceMode.  If the
     message is a CRL message, enhanceMode is set to MODE_CRL, certChain is
     unmodified since there are no senders, and chainStatus->overall
     is set to zero.
   Return NULL for success or error string.
 */
char *RIPEMDecipherFinal (ripemInfo, certChain, chainStatus, enhanceMode)
RIPEMInfo *ripemInfo;
TypList *certChain;
ChainStatusInfo *chainStatus;
enum enhance_mode *enhanceMode;
{
  RIPEMDecipherFrame *frame = ripemInfo->z.decipherFrame;
  int status;

  if (frame == (RIPEMDecipherFrame *)NULL)
    return ("Decipher not initialized");

  /* The caller has reached an end of stream. Check premature end
       of stream conditions.
   */
  if (frame->doingHeader) {
    /* This really shouldn't happen since we should
         normally read a blank line after the header.
     */
    if (frame->messageInfo.foundBeginBoundary)
      return ("Unexpected end of stream while reading header");
    else
      return ("Could not find Privacy Enhanced Message header");
  }
  else {
    /* It is an error if we haven't read the end boundary. */
    if (!frame->foundEndBoundary)
      return ("Can't find end PEM boundary");
  }

  if (frame->messageInfo.proc_type == PROC_TYPE_CRL_ID_ENUM) {
    /* CRL message */
    *enhanceMode = MODE_CRL;
    chainStatus->overall = 0;
    return ((char *)NULL);
  }

  /* If the chain status is zero, we couldn't find a public key.
       Perhaps there is a self-signed cert to validate, so copying the
       certChain later is important, but don't try to validate the message
       signature. */
  if (frame->chainStatus.overall != 0) {
    /* We have digested the text, so check the signature.
     */
    if ((status = R_VerifyFinal
         (&frame->signatureContext, frame->signature, frame->signatureLen,
          &frame->senderKey)) != 0)
      return (FormatRSAError (status));

    /* Convert the enum_ids to an enhance_mode value.
     */
    if (frame->messageInfo.proc_type == PROC_TYPE_ENCRYPTED_ID_ENUM)
      *enhanceMode = MODE_ENCRYPTED;
    else if (frame->messageInfo.proc_type == PROC_TYPE_MIC_ONLY_ID_ENUM)
      *enhanceMode = MODE_MIC_ONLY;
    else if (frame->messageInfo.proc_type == PROC_TYPE_MIC_CLEAR_ID_ENUM)
      *enhanceMode = MODE_MIC_CLEAR;
  }
  
  /* Now return the sender's info to the caller.  Copy the chain's TypList
       which will transfer all the pointers to the caller.  Then re-initialize
       the frame's TypList so that the destructor won't try to free the memory.
   */
  *certChain = frame->certChain;
  InitList (&frame->certChain);
  /* This copies the entire ChainStatusInfo. */
  *chainStatus = frame->chainStatus;

  return ((char *)NULL);
}

/*--- function FormatRSAError -----------------------------------------
 * Return a string corresponding to the RSAREF errorCode.
 */
char *FormatRSAError(errorCode)
int errorCode;
{
  char *errorMessage;

  switch(errorCode) {
  case RE_CONTENT_ENCODING:
    errorMessage = "(Encrypted) content has RFC 1113 encoding error";
    break;
  case RE_DIGEST_ALGORITHM:
    errorMessage = "Message-digest algorithm is invalid";
    break;
  case RE_KEY:
    errorMessage =
      "Recovered DES key cannot decrypt encrypted content or encrypt signature";
    break;
  case RE_KEY_ENCODING:
    errorMessage = "Encrypted key has RFC 1113 encoding error";
    break;
  case RE_MODULUS_LEN:
    errorMessage = "Modulus length is invalid";
    break;
  case RE_NEED_RANDOM:
    errorMessage = "Random structure is not seeded";
    break;
  case RE_PRIVATE_KEY:
    errorMessage =
      "Private key cannot encrypt message digest, or cannot decrypt encrypted key";
    break;
  case RE_PUBLIC_KEY:
    errorMessage = "Public key cannot encrypt DES key, or cannot decrypt signature";
    break;
  case RE_SIGNATURE:
    errorMessage = "Signature on content or block is incorrect";
    break;
  case RE_SIGNATURE_ENCODING:
    errorMessage = "(Encrypted) signature has RFC 1113 encoding error";
    break;
  default:
    errorMessage = "Unknown error returned from RSAREF routines";
    break;
  }
  return errorMessage;
}

/* Use the first entry in the ripemInfo's userList to select the user's
     private key and self-signed cert.  The password is used to decrypt
     the private key.
   This sets ripemInfo's publicKey, privateKey, userDN and userCertDER
     (which is the user's self-signed cert).
   This also updates ripemInfo's issuerSerialAlises table with the
     issuer/serial aliases for all certs which match the user's public key.
   If this returns the error string ERR_SELF_SIGNED_CERT_NOT_FOUND,
     the public and private keys have already been set, but couldn't
     find the user's self-signed certificate.  This should only be used
     during a change password if the application wants to do an "upgrade" from
     RIPEM 1.1 by calling CreateSelfSignedCert.
   This also loads the RIPEM preferences from
     ripemDatabase->preferencesFilename.
   If this returns error ERR_PREFERENCES_NOT_FOUND, then the user has been
     successfully logged in but ripemDatabase->preferencesFilename
     is NULL or the entry for the RIPEM user doesn't exist.  In this case,
     the user should be alerted that default preferences will be used.
     The preferences will be saved at the next call to RIPEMSavePreferences
     or other functions like SetChainLenAllowed than save the preferences.
   If this returns ERR_PREFERENCES_CORRPUT, then the user has been successfully
     logged in but the preferences decodes badly or the signature doesn't
     check.  In this case, the user should be alerted that default values
     are now in use and that any previous preferences must be set again.
     The preferences will be saved at the next call to RIPEMSavePreferences
     or other functions like SetChainLenAllowed that save the preferences.
 */
char *LoginUser (ripemInfo, ripemDatabase, password, passwordLen)
RIPEMInfo *ripemInfo;
RIPEMDatabase *ripemDatabase;
unsigned char *password;
unsigned int passwordLen;
{
  BOOL gotSelfSignedCert;
  CertificateStruct *certStruct = (CertificateStruct *)NULL;
  CertFieldPointers fieldPointers;
  TypList certs, issuerNames;
  TypListEntry *entry;
  char *errorMessage = (char *)NULL;
  int certDERLen, status;
  unsigned int digestLen;

  InitList (&certs);
  InitList (&issuerNames);

  /* For error, break to end of do while (0) block. */
  do {
    /* Set the password digest for use in authenticating preferences. */
    R_DigestBlock
      (ripemInfo->passwordDigest, &digestLen, password, passwordLen, DA_MD5);

    if ((errorMessage = GetPrivateKey
         (FirstUsername (ripemInfo), &ripemDatabase->privKeySource,
          &ripemInfo->privateKey, password, passwordLen, ripemInfo))
        != (char *)NULL)
      break;

    if (ripemInfo->debug > 2)
      DumpPrivKey (&ripemInfo->privateKey, ripemInfo->debugStream);

    /* Construct the public key from the private key.
     */
    ripemInfo->publicKey.bits = ripemInfo->privateKey.bits;
    R_memcpy
      ((POINTER)ripemInfo->publicKey.modulus,
       (POINTER)ripemInfo->privateKey.modulus,
       sizeof (ripemInfo->publicKey.modulus));
    R_memcpy
      ((POINTER)ripemInfo->publicKey.exponent,
       (POINTER)ripemInfo->privateKey.publicExponent,
       sizeof (ripemInfo->publicKey.modulus));

    /* Select all certs for this username. */
    if ((errorMessage = GetCertsBySmartname
         (ripemDatabase, &certs, FirstUsername (ripemInfo), ripemInfo))
        != (char *)NULL)
      break;

    /* Allocate the certStruct on the heap because it's big. */
    if ((certStruct = (CertificateStruct *)malloc
         (sizeof (*certStruct))) == (CertificateStruct *)NULL) {
      errorMessage = ERR_MALLOC;
      break;
    }
    
    /* Find the first cert with the user's public key and the same
         subject name and issuer name.  Also, use the public key to
         check the signature on the self-signed cert.
       We are also going to record the digest of issuer name and serial
         number for all certificates with the user's public key which
         will be used later for finding the issuer/serial based recipient
         info when receiving an encrypted mesage.
     */
    gotSelfSignedCert = FALSE;

    for (entry = certs.firstptr; entry; entry = entry->nextptr) { 
      if ((certDERLen = DERToCertificate
           ((unsigned char *)entry->dataptr, certStruct, &fieldPointers))
          < 0) {
        /* Error decoding.  Just issue a warning to debug stream and try
             the next cert. */
        if (ripemInfo->debug > 1)
          fprintf (ripemInfo->debugStream,
                   "Warning: Cannot decode certificate from database.\n");
        continue;
      }

      if (R_memcmp ((POINTER)&certStruct->publicKey,
                    (POINTER)&ripemInfo->publicKey,
                    sizeof (ripemInfo->publicKey)) != 0)
        /* Doesn't have the user's public key */
        continue;

      if ((errorMessage = AddIssuerSerialAlias (ripemInfo, certStruct))
          != (char *)NULL)
        break;

      if (R_memcmp ((POINTER)&certStruct->issuer,
                    (POINTER)&certStruct->subject,
                    sizeof (certStruct->issuer)) != 0) {
        /* This is not a self-signed cert.  Add the issuer name in this cert
             to the issuerName list if not already there so we can find
             Issuer-Cert chains later. */
        if ((errorMessage = AddNameToList (&issuerNames, &certStruct->issuer))
            != (char *)NULL)
          break;
      }

      if (gotSelfSignedCert)
        /* We have already copied the issuer/serial and we already found
             the user's self-signed cert, so skip to the next cert. */
        continue;

      if (R_memcmp ((POINTER)&certStruct->issuer,
                    (POINTER)&certStruct->subject,
                    sizeof (certStruct->issuer)) != 0)
        /* Doesn't have the same issuer and subject name */
        continue;

      if ((status = R_VerifyBlockSignature
           (fieldPointers.innerDER, fieldPointers.innerDERLen,
            certStruct->signature, certStruct->signatureLen,
            certStruct->digestAlgorithm, &ripemInfo->publicKey)) != 0) {
        /* Can't verify the signature.  Just print an error to the
             debug stream and try more certs. */
        if (ripemInfo->debug > 1)
          fprintf (ripemInfo->debugStream,
          "Warning: %s while checking signature on self-signed certificate.\n",
                   FormatRSAError (status));
        continue;
      }

      /* We have the correct self-signed certificate, so copy info.
       */
      ripemInfo->userDN = certStruct->subject;

      /* malloc and copy the cert DER.  Allocate an extra space as required
           by CodeAndWriteBytes.
       */
      if ((ripemInfo->z.userCertDER = (unsigned char *)malloc
           (certDERLen + 1)) == (unsigned char *)NULL) {
        errorMessage = ERR_MALLOC;
        break;
      }

      R_memcpy
        ((POINTER)ripemInfo->z.userCertDER, (POINTER)entry->dataptr,
         certDERLen);
      ripemInfo->z.userCertDERLen = certDERLen;

      /* Found the self-signed cert */
      gotSelfSignedCert = TRUE;
    }
    if (errorMessage != (char *)NULL)
      break;

    if (!gotSelfSignedCert) {
      errorMessage = ERR_SELF_SIGNED_CERT_NOT_FOUND;
      break;
    }

    /* Load the preferences as the last thing we do so that its
         meaningful errors like ERR_PREFERENCES_NOT_FOUND will be returned
         only after the user is successfully logged in. */
    if ((errorMessage = RIPEMLoadPreferences (ripemInfo, ripemDatabase))
        != (char *)NULL)
      break;

    /* Add certs to use as Issuer-Certicificates.  Note that if
         RIPEMLoadPreferences failed to load the preferences, there is
         no point in trying to add issuer certs since there are no
         chain length allowed entries to allow extended certificate
         chains. */
    if ((errorMessage = AddUserIssuerCerts
         (ripemInfo, &issuerNames, ripemDatabase)) != (char *)NULL)
      break;
  } while (0);

  FreeList (&certs);
  FreeList (&issuerNames);
  free (certStruct);
  return (errorMessage);
}

/* This is just like realloc, except frees the original pointer if realloc
     fails.
   This also returns a non-NULL value if size is zero, as opposed to some
     implementations which may return NULL for a block of size zero.
   Returns the reallocated pointer on success, or NULL on failure.
 */
void *R_realloc (pointer, size)
void *pointer;
unsigned int size;
{
  void *result;

  if (size == 0)
    /* Prevent allocating a block of size zero which may return NULL. */
    size = 1;

  if (pointer == NULL)
    /* Explicitly call malloc, because some realloc implementations
         don't like to realloc a NULL pointer. */
    return ((void *)malloc (size));

  if ((result = (void *)realloc (pointer, size)) == NULL)
    /* Explicitly free the old pointer, which is unchanged by realloc. */
    free (pointer);

  return (result);
}

/* Digest the issuer name and serial number in the certStruct, and add
     it to ripemInfo's issuerSerialAliases if not already there.
   Returns NULL for OK, otherwise error message.
 */
static char *AddIssuerSerialAlias (ripemInfo, certStruct)
RIPEMInfo *ripemInfo;
CertificateStruct *certStruct;
{
  unsigned char alias[16];
  unsigned int count;

  ComputeIssuerSerialAlias
    (alias, &certStruct->issuer, certStruct->serialNumber,
     sizeof (certStruct->serialNumber));

  if (!IsIssuerSerialAlias (ripemInfo, alias)) {
    /* Alias is not already in the ripemInfo, so realloc room in the
         table and add it. */
    count = ripemInfo->z.issuerSerialAliasCount;
    if ((ripemInfo->z.issuerSerialAliases = (unsigned char *)R_realloc
         (ripemInfo->z.issuerSerialAliases, 16 * (count + 1)))
        == (unsigned char *)NULL)
      return (ERR_MALLOC);

    R_memcpy
      ((POINTER)&ripemInfo->z.issuerSerialAliases[16 * count],
       (POINTER)alias, 16);
    ripemInfo->z.issuerSerialAliasCount = count + 1;
  }

  return ((char *)NULL);
}

/*  Write the Privacy Enhanced Mail header.
    Entry:   ripemInfo contains the enhance frame with the information
              to output
            frame->stream    is the I/O stream to write to.
            frame->iv        is the init vector that was used to encrypt.
            frame->recipientKeys contains the recipient public keys
            frame->encryptedKeysBuffer contains the concatenated encrypted
             DEKs, where each takes MAX_ENCRYPTED_KEY_LEN bytes in the buffer.
           frame->encryptedKeyLens is the actual length of each encrypted DEK.
            frame->recipientKeyCount is the number of recipients.
            encryptedSignature      is the signature, possibly encrypted.
            encryptedSignatureLen is the number of bytes in above.
   ripemDatabase is used for selecting certificates to find issuer names
     and serial numbers of recipients for MESSAGE_FORMAT_PEM.
 
   Exit:    We have written the header out to the stream, followed
              by a blank line.
            This will prencode the signature and recipient key infos.
            Returns NULL if no error, else error message.
 */
static char *WriteHeader
  (ripemInfo, encryptedSignature, encryptedSignatureLen, ripemDatabase)
RIPEMInfo *ripemInfo;
unsigned char *encryptedSignature;
unsigned int encryptedSignatureLen;
RIPEMDatabase *ripemDatabase;
{
  RIPEMEncipherFrame *frame = ripemInfo->z.encipherFrame;
  BufferStream *stream;
  TypListEntry *entry;
  CertificateStruct *certStruct = (CertificateStruct *)NULL;
  char *errorMessage;
  char iv_hex[2 * IV_SIZE + 1];
  unsigned char *der = (unsigned char *)NULL;
  unsigned int derLen, i;
  enum enum_ids enhanceMode;
  TypList certList;

  InitList (&certList);
  
  /* For error, break to end of do while (0) block. */
  do {
    /* Convert enum enhance_mode to enum_ids.
     */
    if (frame->enhanceMode == MODE_ENCRYPTED)
      enhanceMode = PROC_TYPE_ENCRYPTED_ID_ENUM;
    else if (frame->enhanceMode == MODE_MIC_ONLY)
      enhanceMode = PROC_TYPE_MIC_ONLY_ID_ENUM;
    else
      enhanceMode = PROC_TYPE_MIC_CLEAR_ID_ENUM;

    /* Get stream pointer by itself for convenience. */
    stream = &frame->outStream;

    /* Put out header indicating encapsulated message follows. */
    if ((errorMessage = BufferStreamPuts (HEADER_STRING_BEGIN, stream))
        != (char *)NULL)
      break;
    if ((errorMessage = WriteEOL (stream)) != (char *)NULL)
      break;

    /* Put out field indicating processing type. */
    if ((errorMessage = BufferStreamPuts (PROC_TYPE_FIELD, stream))
        != (char *)NULL)
      break;
    if ((errorMessage = BufferStreamPuts (" ", stream)) != (char *)NULL)
      break;
    if ((errorMessage = BufferStreamPuts
         (frame->messageFormat == MESSAGE_FORMAT_RIPEM1 ?
          PROC_TYPE_RIPEM_ID : PROC_TYPE_PEM_ID, stream)) != (char *)NULL)
      break;
    if ((errorMessage = BufferStreamPuts (SPEC_SEP, stream)) != (char *)NULL)
      break;

    if ((errorMessage = BufferStreamPuts (IDNames[enhanceMode], stream))
        != (char *)NULL)
      break;
    if ((errorMessage = WriteEOL (stream)) != (char *)NULL)
      break;

    /* Put out content domain. */
    if ((errorMessage = BufferStreamPuts (CONTENT_DOMAIN_FIELD, stream))
        != (char *)NULL)
      break;
    if ((errorMessage = BufferStreamPuts (" RFC822", stream)) != (char *)NULL)
      break;
    if ((errorMessage = WriteEOL (stream)) != (char *)NULL)
      break;

    /* If encrypting, put out DEK-Info field. */
    if (enhanceMode == PROC_TYPE_ENCRYPTED_ID_ENUM) {
      if ((errorMessage = BufferStreamPuts (DEK_FIELD, stream))
          != (char *)NULL)
        break;
      if ((errorMessage = BufferStreamPuts (" ", stream)) != (char *)NULL)
        break;
      if (frame->encryptionAlgorithm == EA_DES_EDE2_CBC) {
        if ((errorMessage = BufferStreamPuts (DEK_ALG_TDES_CBC_ID, stream))
            != (char *)NULL)
          break;
      }
      else {
        if ((errorMessage = BufferStreamPuts (DEK_ALG_DES_CBC_ID, stream))
            != (char *)NULL)
          break;
      }
      if ((errorMessage = BufferStreamPuts (SPEC_SEP, stream)) != (char *)NULL)
        break;
      BinToHex (frame->iv, IV_SIZE, iv_hex);
      if ((errorMessage = BufferStreamPuts (iv_hex, stream)) != (char *)NULL)
        break;
      if ((errorMessage = WriteEOL (stream)) != (char *)NULL)
        break;
    }

    if (frame->messageFormat == MESSAGE_FORMAT_RIPEM1) {
      /* Write Originator's name. */

      if ((errorMessage = BufferStreamPuts (SENDER_FIELD, stream))
          != (char *)NULL)
        break;
      if ((errorMessage = BufferStreamPuts (" ", stream)) != (char *)NULL)
        break;
      if ((errorMessage = BufferStreamPuts (FirstUsername (ripemInfo), stream))
          != (char *)NULL)
        break;
      if ((errorMessage = WriteEOL (stream)) != (char *)NULL)
        break;
    }

    if ((errorMessage = WriteOriginatorAndIssuerCerts
         (ripemInfo, stream, frame->messageFormat)) != (char *)NULL)
      break;

    /* Write out the digital signature.
     */
    if ((errorMessage = BufferStreamPuts (MIC_INFO_FIELD, stream))
        != (char *)NULL)
      break;
    if ((errorMessage = BufferStreamPuts (" ", stream)) != (char *)NULL)
      break;
    if ((errorMessage = BufferStreamPuts (MIC_MD5_ID, stream)) != (char *)NULL)
      break;
    if ((errorMessage = BufferStreamPuts (SPEC_SEP, stream)) != (char *)NULL)
      break;
    if ((errorMessage = BufferStreamPuts (ENCRYPTION_ALG_RSA_ID, stream))
        != (char *)NULL)
      break;
    if ((errorMessage = BufferStreamPuts (SPEC_SEP, stream)) != (char *)NULL)
      break;
    if ((errorMessage = WriteEOL (stream)) != (char *)NULL)
      break;

    if ((errorMessage = BufferCodeAndWriteBytes
         (encryptedSignature, encryptedSignatureLen, " ", stream))
        != (char *)NULL)
      break;

    if (enhanceMode == PROC_TYPE_ENCRYPTED_ID_ENUM) {
      /* Allocate the certStruct on the heap because it's big. */
      if ((certStruct = (CertificateStruct *)malloc
           (sizeof (*certStruct))) == (CertificateStruct *)NULL) {
        errorMessage = ERR_MALLOC;
        break;
      }
      
      /* For each recipient, write out the recipient identifier and
           encrypted message key.
       */
      for (i = 0; i < frame->recipientKeyCount; ++i) {
        if (frame->messageFormat == MESSAGE_FORMAT_RIPEM1) {
          /* Write user name (email address) */
          if ((errorMessage = BufferStreamPuts (RECIPIENT_FIELD, stream))
              != (char *)NULL)
            break;
          if ((errorMessage = BufferStreamPuts (" ", stream)) != (char *)NULL)
            break;
          if ((errorMessage = BufferStreamPuts
               (frame->recipientKeys[i].username, stream)) != (char *)NULL)
            break;
          if ((errorMessage = WriteEOL (stream)) != (char *)NULL)
            break;

          /* Write the recipient's public key */
          if ((errorMessage = BufferStreamPuts (RECIPIENT_KEY_FIELD, stream))
              != (char *)NULL)
            break;
          if ((errorMessage = WriteEOL (stream)) != (char *)NULL)
            break;

          derLen = PubKeyToDERLen (&frame->recipientKeys[i].publicKey);
          /* Use realloc since we reuse this buffer in this loop. */
          if ((der = (unsigned char *)R_realloc (der, derLen + 1))
              == (unsigned char *)NULL) {
            errorMessage = ERR_MALLOC;
            break;
          }
          PubKeyToDER (&frame->recipientKeys[i].publicKey, der, &derLen);
          if ((errorMessage = BufferCodeAndWriteBytes
               (der, derLen, " ", stream)) != (char *)NULL)
            break;

          /* Write encrypted message key. */
          if ((errorMessage = WriteKeyInfo
               (frame->encryptedKeysBuffer + i * MAX_ENCRYPTED_KEY_LEN,
                frame->encryptedKeyLens[i], stream)) != (char *)NULL)
            break;
        }

        if (frame->messageFormat == MESSAGE_FORMAT_PEM) {
          /* For every certificate we can find with a matching
               username and public key, write its issuer name and serial
               number as a Recipient-ID-Asymmetric and its accompanying
               Key-Info.
           */
          /* Free any previous contents of certList */
          FreeList (&certList);

          if ((errorMessage = GetCertsBySmartname
               (ripemDatabase, &certList, frame->recipientKeys[i].username,
                ripemInfo)) != (char *)NULL)
            break;
          for (entry = certList.firstptr; entry; entry = entry->nextptr) {
            /* Decode the certificate.  On failure, just warn and continue.
             */
            if (DERToCertificate
                ((unsigned char *)entry->dataptr, certStruct,
                 (CertFieldPointers *)NULL) < 0) {
              /* Error decoding.  Just issue a warning to debug stream and try
                 the next cert. */
              if (ripemInfo->debug > 1)
                fprintf (ripemInfo->debugStream,
 "Warning: Cannot decode certificate from database for writing Recipient-ID.\n");
              continue;
            }
            
            if (R_memcmp
                ((POINTER)&frame->recipientKeys[i].publicKey,
                 (POINTER)&certStruct->publicKey,
                 sizeof (certStruct->publicKey)) != 0)
              /* Not the same public key.  Try the next */
              continue;

            if ((errorMessage = WriteRecipientIDAsymmetric
                 (&certStruct->issuer, certStruct->serialNumber,
                  sizeof (certStruct->serialNumber), stream)) != 0)
              break;
            /* Write encrypted message key. */
            if ((errorMessage = WriteKeyInfo
                 (frame->encryptedKeysBuffer + i * MAX_ENCRYPTED_KEY_LEN,
                  frame->encryptedKeyLens[i], stream)) != (char *)NULL)
              break;
          }
          if (errorMessage != (char *)NULL)
            /* Broke loop because of error. */
            break;
        }
      }
      if (errorMessage != (char *)NULL)
        /* Broke loop because of error. */
        break;
    }

    /* Write blank line that separates headers from text. */
    if ((errorMessage = WriteEOL (stream)) != (char *)NULL)
      break;
  } while (0);

  free (der);
  FreeList (&certList);
  free (certStruct);
  return (errorMessage);
}

static char *WriteKeyInfo (encryptedKey, encryptedKeyLen, stream)
unsigned char *encryptedKey;
unsigned int encryptedKeyLen;
BufferStream *stream;
{
  char *errorMessage;
  
  if ((errorMessage = BufferStreamPuts (MESSAGE_KEY_FIELD, stream))
      != (char *)NULL)
    return (errorMessage);
  if ((errorMessage = BufferStreamPuts (" ", stream)) != (char *)NULL)
    return (errorMessage);
  if ((errorMessage = BufferStreamPuts (ENCRYPTION_ALG_RSA_ID, stream))
      != (char *)NULL)
    return (errorMessage);
  if ((errorMessage = BufferStreamPuts (SPEC_SEP, stream)) != (char *)NULL)
    return (errorMessage);
  if ((errorMessage = WriteEOL (stream)) != (char *)NULL)
    return (errorMessage);

  if ((errorMessage = BufferCodeAndWriteBytes
       (encryptedKey, encryptedKeyLen, " ", stream)) != (char *)NULL)
    return (errorMessage);
  return ((char *)NULL);
}

static char *WriteRecipientIDAsymmetric
  (issuerName, serialNumber, serialNumberLen, stream)
DistinguishedNameStruct *issuerName;
unsigned char *serialNumber;
unsigned int serialNumberLen;
BufferStream *stream;
{
  char *errorMessage, *hex = (char *)NULL;
  unsigned char *der = (unsigned char *)NULL, *derPointer,
    *zero = (unsigned char *)"";

  /* For error, break to end of do while (0) block. */
  do {
    if ((errorMessage = BufferStreamPuts
         (RECIPIENT_ID_ASYMMETRIC_FIELD, stream)) != (char *)NULL)
      break;
    if ((errorMessage = WriteEOL (stream)) != (char *)NULL)
      break;

    /* Allocate buffer and convert issuer name to DER and write it.
     */
    if ((der = (unsigned char *)malloc
         (len_distinguishedname (issuerName) + 4)) == (unsigned char *)NULL) {
      errorMessage = ERR_MALLOC;
      break;
    }
    derPointer = der;
    DistinguishedNameToDER (issuerName, &derPointer);
    if ((errorMessage = BufferCodeAndWriteBytes
         (der, (unsigned int)(derPointer - der), " ", stream)) != (char *)NULL)
      break;
    
    /* Strip leading zeroes off serialNumber. */
    while ((serialNumberLen > 0) && (*serialNumber == 0)) {
      ++serialNumber;
      --serialNumberLen;
    }
    if (serialNumberLen == 0) {
      /* Make sure there is a zero byte */
      serialNumber = zero;
      serialNumberLen = 1;
    }

    /* Allocate array and write hex serial number.  Don't forget the " ,".
     */
    if ((hex = malloc (2 * serialNumberLen + 1)) == (char *)NULL) {
      errorMessage = ERR_MALLOC;
      break;
    }
    BinToHex (serialNumber, serialNumberLen, hex);
    if ((errorMessage = BufferStreamPuts (" ", stream)) != (char *)NULL)
      break;
    if ((errorMessage = BufferStreamPuts (SPEC_SEP, stream)) != (char *)NULL)
      break;
    if ((errorMessage = BufferStreamPuts (hex, stream)) != (char *)NULL)
      break;
    if ((errorMessage = WriteEOL (stream)) != (char *)NULL)
      break;
  } while (0);

  free (der);
  free (hex);
  return (errorMessage);
}

/* Process the enhanced text line, decoding and decrpyting if needed,
     call R_VerifyUpdate with signatureContext, and write out the result to
     outStream.
   The signatureContext must already be initialized.  On return, the
     caller can call R_VerifyFinal.
   If the frame's messageInfo.proc_type is not MIC-CLEAR, decode the input,
     otherwise by strip "- " from lines that begin with "- -" and adding CR/LF
     to the end of line for calling R_VerifyUpdate.
   If the frame's messageInfo.proc_type is ENCRYPTED, envelopeContext must
     already be initialized. Use it to decrypt the content.
   When we encounter the PEM end boundary line, set frame->foundEndBoundary.
     After this is set, all further calls to this routine do nothing.
 */
static char *ReadEnhancedTextLine (frame, outStream, line)
RIPEMDecipherFrame *frame;
BufferStream *outStream;
char *line;
{
  unsigned char *linecp;
  char *errorMessage;
  int header_len = strlen(HEADER_STRING_END), status;
  unsigned int decodedLineLen, linecpLen;

  if (frame->foundEndBoundary)
    /* We are reading past the end boundary, so ignore. */
    return ((char *)NULL);

  if (strncmp ((char *)line, HEADER_STRING_END, header_len) == 0) {
    /* Reached the end PEM boundary.  If we are decrypting we
         must process the final block.
     */
    if (frame->messageInfo.proc_type == PROC_TYPE_ENCRYPTED_ID_ENUM) {
      /* Finalize to decryptedLine, setting linecpLen to the length. */
      if ((status = R_OpenFinal
           (&frame->envelopeContext, frame->decryptedLine, &linecpLen))
          != 0)
        return (FormatRSAError (status));
      if ((status = R_VerifyUpdate
           (&frame->signatureContext, frame->decryptedLine, linecpLen))
          != 0)
        return (FormatRSAError (status));
      if ((errorMessage = WriteMessage
           (frame->decryptedLine, linecpLen, FALSE, outStream))
          != (char *)NULL)
        return (errorMessage);
    }

    /* Done, so set end flag and return. */
    frame->foundEndBoundary = TRUE;
    return ((char *)NULL);
  }

  /* Strip end-of-line CR and/or NL */
  for (linecp = (unsigned char *)line;
       *linecp && *linecp!='\r' && *linecp!='\n';
       linecp++);
  *linecp = '\0';

  if (frame->messageInfo.proc_type != PROC_TYPE_MIC_CLEAR_ID_ENUM) {
    /* Decode the line in place.  This will work since the output
         of decoding is smaller than the input.
       Note: this assumes the input is a multiple of 4 bytes.
     */
    if ((status = R_DecodePEMBlock
         ((unsigned char *)line, &decodedLineLen, (unsigned char *)line,
          strlen (line))) != 0)
      return (FormatRSAError (status));

    if (frame->messageInfo.proc_type == PROC_TYPE_ENCRYPTED_ID_ENUM) {
      /* Decrypt to decryptedLine, setting linecpLen to the length. */
      if ((status = R_OpenUpdate
           (&frame->envelopeContext, frame->decryptedLine, &linecpLen,
            (unsigned char *)line, decodedLineLen)) != 0)
        return (FormatRSAError (status));

      linecp = frame->decryptedLine;
    }
    else {
      /* Just point to the decoded line. */
      linecp = (unsigned char *)line;
      linecpLen = decodedLineLen;
    }

    /* linecp and linecpLen now have the result, so digest and write out.
       Note that WriteMessage will convert the CR/LF correctly to
         local format.
     */
    if ((status = R_VerifyUpdate
         (&frame->signatureContext, linecp, linecpLen)) != 0)
      return (FormatRSAError (status));
    if ((errorMessage = WriteMessage (linecp, linecpLen, FALSE, outStream))
        != (char *)NULL)
      return (errorMessage);
  }
  else {
    /* Strip quoted hyphens. */
    linecp = (unsigned char *)line;
    if (strncmp ((char *)linecp, (char *)"- -", 3) == 0)
      linecp += 2;

    /* Digest the line plus CR/LF */
    if ((status = R_VerifyUpdate
         (&frame->signatureContext, linecp, strlen ((char *)linecp))) != 0)
      return (FormatRSAError (status));
    if ((status = R_VerifyUpdate
         (&frame->signatureContext, (unsigned char *)"\r\n", 2)) != 0)
      return (FormatRSAError (status));

    if ((errorMessage = BufferStreamPuts
         ((char *)linecp, outStream)) != (char *)NULL)
      return (errorMessage);
    if ((errorMessage = WriteEOL (outStream)) != (char *)NULL)
      return (errorMessage);
  }

  return ((char *)NULL);
}

static void RIPEMEncipherFrameConstructor (frame)
RIPEMEncipherFrame *frame;
{
  BufferStreamConstructor (&frame->outStream);
  frame->encryptedKeysBuffer = (unsigned char *)NULL;
  frame->encryptedKeyLens = (unsigned int *)NULL;
  InitList (&frame->issuerNames);
  frame->wroteHeader = FALSE;
  frame->lastWasNewline = FALSE;
  BufferStreamConstructor (&frame->buffer);
}

static void RIPEMEncipherFrameDestructor (frame)
RIPEMEncipherFrame *frame;
{
  BufferStreamDestructor (&frame->outStream);
  BufferStreamDestructor (&frame->buffer);
  free (frame->encryptedKeysBuffer);
  free (frame->encryptedKeyLens);
  R_memset ((POINTER)&frame->sealContext, 0, sizeof (frame->sealContext));
  R_memset
    ((POINTER)&frame->signatureContext, 0, sizeof (frame->signatureContext));
}

static void RIPEMDecipherFrameConstructor (frame)
RIPEMDecipherFrame *frame;
{
  BufferStreamConstructor (&frame->outStream);
  TypMsgInfoConstructor (&frame->messageInfo);
  BufferStreamConstructor (&frame->lineIn);
  frame->doingHeader = TRUE;
  frame->foundEndBoundary = FALSE;
  InitList (&frame->headerList);
  InitList (&frame->certChain);
}

static void RIPEMDecipherFrameDestructor (frame)
RIPEMDecipherFrame *frame;
{
  BufferStreamDestructor (&frame->outStream);
  TypMsgInfoDestructor (&frame->messageInfo);
  BufferStreamDestructor (&frame->lineIn);
  R_memset ((POINTER)frame->decryptedLine, 0, sizeof (frame->decryptedLine));
  R_memset ((POINTER)frame->signature, 0, sizeof (frame->signature));
  R_memset
    ((POINTER)&frame->envelopeContext, 0, sizeof (frame->envelopeContext));
  R_memset
    ((POINTER)&frame->signatureContext, 0, sizeof (frame->signatureContext));
  frame->foundEndBoundary = FALSE;
  FreeList (&frame->headerList);
  FreeList (&frame->certChain);
}

/* Process the header and set the ripemInfo's frame's signature,
     certChain, etc.  If this finds an unvalidated self-signed certificate,
     it sets frame->foundEndBoundary to prevent processing of the text.
   outStream is for writing the prepended headers.
 */
static char *ProcessHeader (ripemInfo, outStream, ripemDatabase)
RIPEMInfo *ripemInfo;
BufferStream *outStream;
RIPEMDatabase *ripemDatabase;
{
  char *errorMessage = (char *)NULL;
  CertificateStruct *certStruct = (CertificateStruct *)NULL;
  CertFieldPointers fieldPointers;
  int isSelfSigned, status, certLen;
  BOOL found;
  void *certCopy;
  TypListEntry *entry;
  RIPEMDecipherFrame *frame;

  /* For error, break to end of do while (0) block. */
  do {
    frame = ripemInfo->z.decipherFrame;

    if (frame->messageInfo.proc_type == PROC_TYPE_CRL_ID_ENUM) {
      /* CRL message, so first insert certificates into database,
           then insert CRLs.  Note that the CRLs require the certs
           to already be in the database so they can find the public key
           for the issuer of the CRL.
       */
      if ((errorMessage = InsertCerts
           (&frame->messageInfo.certs, ripemInfo, ripemDatabase))
          != (char *)NULL)
        break;
      if ((errorMessage = InsertCRLs
           (&frame->messageInfo.crls, ripemInfo, ripemDatabase))
          != (char *)NULL)
        break;

      /* We have inserted the certs and CRLs, so break.
         Setting foundEndBoundary will prevent the processing of
           any more of the message. */
      frame->foundEndBoundary = TRUE;
      break;
    }

    if (!frame->messageInfo.msg_key &&
        frame->messageInfo.proc_type == PROC_TYPE_ENCRYPTED_ID_ENUM) {
      errorMessage = "You are not listed as a recipient in this message.";
      break;
    }
    if (ripemInfo->debug > 1) {
      fprintf
        (ripemInfo->debugStream,"From input encapsulated message header:\n");
      fprintf
        (ripemInfo->debugStream,"  Proc-Type = %s",
         IDNames[frame->messageInfo.proc_type]);
      if(frame->messageInfo.proc_type == PROC_TYPE_ENCRYPTED_ID_ENUM) {
        char ivhex[20];
        char hex_digest[36];

        BinToHex(frame->messageInfo.iv,8,ivhex);
        fprintf(ripemInfo->debugStream,"  DES iv = %s",ivhex);

        MakeHexDigest
          (frame->messageInfo.msg_key, frame->messageInfo.msg_key_len,
           hex_digest);
        fprintf
          (ripemInfo->debugStream," Digest of Encrypted Key = %s\n",
           hex_digest);
      }
      fprintf(ripemInfo->debugStream,"\n");
      if(frame->messageInfo.orig_name) {
        fprintf(ripemInfo->debugStream,"  Originator-Name = %s\n",
                frame->messageInfo.orig_name);
      }
      fprintf
        (ripemInfo->debugStream,"  %s Originator's public key in header.\n",
         frame->messageInfo.got_orig_pub_key ? "Got" : "Didn't get");
      fprintf
        (ripemInfo->debugStream,"  %d bytes in encoded & encrypted MIC:\n",
         frame->messageInfo.mic_len);
      fprintf(ripemInfo->debugStream,"   %s\n", frame->messageInfo.mic);
      if(frame->messageInfo.msg_key) {
        fprintf
          (ripemInfo->debugStream,
           "  %d bytes in encoded & encrypted message key:\n",
           frame->messageInfo.msg_key_len);
        fprintf
          (ripemInfo->debugStream,"   %s\n", frame->messageInfo.msg_key);
      }
    }

    if (frame->messageInfo.certs.firstptr != (TypListEntry *)NULL) {
      /* This is a certificate-based message.  The Originator-Cert is
           the first in the list and the rest are Issuer-Certs.
         First insert all non-self-signed certs into the database, then
           process the Originator-Cert.
       */
      if ((errorMessage = InsertCerts
           (&frame->messageInfo.certs, ripemInfo, ripemDatabase))
          != (char *)NULL)
        break;

      if ((certStruct = (CertificateStruct *)malloc (sizeof (*certStruct)))
          == (CertificateStruct *)NULL) {
        errorMessage = ERR_MALLOC;
        break;
      }

      /* Decode originator cert. */
      if ((certLen = DERToCertificate
           ((unsigned char *)frame->messageInfo.certs.firstptr->dataptr,
            certStruct, &fieldPointers)) < 0) {
        errorMessage = "Cannot decode originator certificate";
        break;
      }

      /* Try to get the sender's cert chain.  We set directCertOnly
           FALSE to allow any chain. */
      if ((errorMessage = SelectCertChain
           (ripemInfo, &frame->certChain, &frame->chainStatus,
            &certStruct->subject, &certStruct->publicKey, FALSE,
            ripemDatabase)) != (char *)NULL)
        break;

      if (frame->chainStatus.overall == 0) {
        /* Couldn't find a chain, so try to process a self-signed cert.
         */
        CheckSelfSignedCert
          (&isSelfSigned, certStruct, fieldPointers.innerDER,
           fieldPointers.innerDERLen);

        if (isSelfSigned) {
          /* Copy the self-signed cert to the cert chain.
             The certChain should be empty now.
             Note that chainStatus.overall is still 0.
           */
          if ((certCopy = malloc (certLen)) == NULL) {
            errorMessage = ERR_MALLOC;
            break;
          }
          R_memcpy
            ((POINTER)certCopy,
             (POINTER)frame->messageInfo.certs.firstptr->dataptr, certLen);
          if ((errorMessage = PrependToList
               (certCopy, certLen, &frame->certChain)) != (char *)NULL) {
            /* Free the cert copy we just allocated */
            free (certCopy);
            break;
          }

          /* We have copied the self-signed cert, so break.
             Setting foundEndBoundary will prevent the processing of
               the message text. */
          frame->foundEndBoundary = TRUE;
          break;
        }
        else {
          /* Can't find a chain and this is not a self-signed cert,
               so just return error. */
          errorMessage = "Cannot find certificate chain for sender.";
          break;
        }
      }

      /* Decode the cert at the "bottom" of the chain and point the
           public key to it.  Note DERToCertificate won't return an error
           since it was already successfully decoded.
       */
      DERToCertificate
        ((unsigned char *)frame->certChain.firstptr->dataptr, certStruct,
         (CertFieldPointers *)NULL);
      frame->senderKey = certStruct->publicKey;
    }
    else {
      /* Process non-certificate based message */
      frame->chainStatus.overall = CERT_UNVALIDATED;

      if(!frame->messageInfo.orig_name) {
        errorMessage = "Can't find Originator's name in message.";
        break;
      }

      /* Copy the originator's name as the entry in the "certChain" */
      if ((errorMessage = AppendLineToList
           (frame->messageInfo.orig_name, &frame->certChain)) != (char *)NULL)
        break;

      /* Obtain the sender's public key. */

      errorMessage = GetUnvalidatedPublicKey
        (frame->messageInfo.orig_name, &ripemDatabase->pubKeySource,
         &frame->senderKey, &found, ripemInfo);

      if (errorMessage || !found) {
        if (frame->messageInfo.got_orig_pub_key) {
          if (!errorMessage) {
            fprintf(ripemInfo->debugStream,
                    "Warning: public key of \"%s\" not on file.\n",
                    frame->messageInfo.orig_name);
          }
          else {
            fprintf
              (ripemInfo->debugStream,
               "Warning: problem encountered with public key of \"%s\":\n",
               frame->messageInfo.orig_name);
            fprintf(ripemInfo->debugStream,"  %s\n",errorMessage);
          }
          
          fprintf (ripemInfo->debugStream,"Using key supplied in message.\n");
          frame->senderKey = frame->messageInfo.orig_pub_key;
          ripemInfo->z.used_pub_key_in_message = 1;
        }
        else {
          /* Don't have a public key */
          if (errorMessage == (char *)NULL)
            /* Setting foundEndBoundary will prevent the processing of
               the message text. */
            frame->foundEndBoundary = TRUE;
          break;
        }
      } else {
        /* frame->senderKey already has the public key.
           Check to make sure that the sender's public key in the
             message header matches the sender's recorded public key.
         */
        if(frame->messageInfo.got_orig_pub_key) {
          if(R_memcmp((POINTER)&frame->messageInfo.orig_pub_key,
                      (POINTER)&frame->senderKey,
                      sizeof (frame->senderKey))) {
            fprintf(ripemInfo->debugStream,
                    "Warning: %s's public key in message does not match retrieved value.\n",
                    frame->messageInfo.orig_name);
          }
        }
      }
    }

    /* Write out the original message header if requested. */
    if (frame->prependHeaders) {
      /* Copy the headers to the output. */
      for (entry = frame->headerList.firstptr; entry;
           entry = entry->nextptr) {
        if ((errorMessage = BufferStreamPuts
             ((char *)entry->dataptr, outStream)) != (char *)NULL)
          break;
        if ((errorMessage = WriteEOL (outStream)) != (char *)NULL)
          break;
      }
      if (frame->headerList.firstptr != (TypListEntry *)NULL) {
        /* There were in fact lines, so put a blank line */
        if ((errorMessage = WriteEOL (outStream)) != (char *)NULL)
          break;
      }
    }

    if (frame->messageInfo.proc_type == PROC_TYPE_ENCRYPTED_ID_ENUM) {
#ifdef RIPEMSIG
      errorMessage = "RIPEM/SIG cannot process ENCRYPTED messages. You may process signed messages.";
      break;
#else
      if(ripemInfo->debug>1) {
        char hex_digest[36], line[120];

        fprintf(ripemInfo->debugStream,
                "Decrypting.    keyLen=%u, sigLen=%u, user=%s\n",
                frame->messageInfo.msg_key_len, frame->messageInfo.mic_len,
                FirstUsername (ripemInfo));
        MakeHexDigest(frame->messageInfo.msg_key,
                      frame->messageInfo.msg_key_len,hex_digest);
        fprintf
          (ripemInfo->debugStream," MD5 of Encrypted Key     = %s\n",
           hex_digest);
        MakeHexDigest
          (frame->messageInfo.mic, frame->messageInfo.mic_len, hex_digest);
        fprintf
          (ripemInfo->debugStream," MD5 of encrypted signat. = %s\n",
           hex_digest);
        fprintf(ripemInfo->debugStream,"  Encrypted, encoded MIC =\n");
        WriteCoded
          (frame->messageInfo.mic, frame->messageInfo.mic_len, "   ",
           ripemInfo->debugStream);
        BinToHex(frame->messageInfo.iv,8,line);
        fprintf
          (ripemInfo->debugStream," Initializing vector      = %s DigAlg=%d\n",
           line,frame->messageInfo.da);
      }

      /* Initialize envelopeContext and decrypt signature.
       */
      if ((status = RIPEMOpenInit
           (ripemInfo, &frame->envelopeContext, frame->signature,
            &frame->signatureLen)) != 0) {
        errorMessage = FormatRSAError (status);
        break;
      }
#endif /* end RIPEMSIG */
    }
    else if (frame->messageInfo.proc_type == PROC_TYPE_MIC_ONLY_ID_ENUM ||
             frame->messageInfo.proc_type == PROC_TYPE_MIC_CLEAR_ID_ENUM) {
      /* Just decode the signature.
       */
      if (frame->messageInfo.mic_len > MAX_PEM_SIGNATURE_LEN) {
        errorMessage = "Signature in header is too long.";
        break;
      }

      if ((status = R_DecodePEMBlock
           (frame->signature, &frame->signatureLen, frame->messageInfo.mic,
            frame->messageInfo.mic_len)) != 0) {
        errorMessage = FormatRSAError (status);
        break;
      }
    }
    else {
      errorMessage = "Invalid message proc type";
      break;
    }
  } while (0);
  
  free (certStruct);
  return (errorMessage);
}

/* The encipher frame's signature context has been updated, so finalize
     signature and write out the PEM header.
   This does not rewind the outStream.
 */
static char *SignFinalOutputHeader (ripemInfo, ripemDatabase)
RIPEMInfo *ripemInfo;
RIPEMDatabase *ripemDatabase;
{
  RIPEMEncipherFrame *frame = ripemInfo->z.encipherFrame;
  char *errorMessage = (char *)NULL;
  int status;
  unsigned char signature[MAX_PEM_ENCRYPTED_SIGNATURE_LEN];
  unsigned int signatureLen;
#ifndef RIPEMSIG
  unsigned int localPartOutLen;
#endif

  /* For error, break to end of do while (0) block. */
  do {
    if ((status = R_SignFinal
         (&frame->signatureContext, signature, &signatureLen,
          &ripemInfo->privateKey)) != 0) {
      errorMessage = FormatRSAError(status);
      break;
    }

    if (frame->enhanceMode == MODE_ENCRYPTED) {
#ifndef RIPEMSIG
      /* Encrypt the signature in place.
       */
      if ((status = R_SealUpdate
           (&frame->sealContext, signature, &signatureLen, signature,
            signatureLen)) != 0) {
        errorMessage = FormatRSAError (status);
        break;
      }
      if ((status = R_SealFinal
           (&frame->sealContext, signature + signatureLen, &localPartOutLen))
          != 0) {
        errorMessage = FormatRSAError (status);
        break;
      }
      signatureLen += localPartOutLen;
      
      if (ripemInfo->debug > 1) {
        char hex_digest[36];
        
        fprintf (ripemInfo->debugStream, "sigLen=%u (not recoded)\n",
                 signatureLen);
        MakeHexDigest(signature, signatureLen,hex_digest);
        fprintf
          (ripemInfo->debugStream," MD5 of encrypted signat. = %s\n",
           hex_digest);
      }

      /* Note: make sure we pass enum_ids for the enhanceMode. */
      if ((errorMessage = WriteHeader
           (ripemInfo, signature, signatureLen, ripemDatabase))
          != (char *)NULL)
        break;
#endif
    }
    else {
      /* Write out a header for a non-encrypted message.
         This will encode the signature and ignore the iv and recipient
           stuff.
         Note: make sure we pass enum_ids for the enhanceMode. */
      if ((errorMessage = WriteHeader
           (ripemInfo, signature, signatureLen, ripemDatabase))
          != (char *)NULL)
        break;
    }
  } while (0);

  R_memset ((POINTER)signature, 0, sizeof (signature));
  return (errorMessage);
}

#ifndef RIPEMSIG

/* Generate a DEK and encrypt it with each recipient's public key.
   Also generate the IV and initialize the sealContext.
   Note that the encrypted DEKs are not ASCII recoded.  They must be
     ASCII recoded when they are written to the header.
 */
static char *RIPEMSealInit
  (ripemInfo, sealContext, iv, encryptedKeysBuffer, encryptedKeyLens,
   recipientKeys, recipientKeyCount, encryptionAlgorithm)
RIPEMInfo *ripemInfo;
R_ENVELOPE_CTX *sealContext;
unsigned char *iv;
unsigned char *encryptedKeysBuffer;
unsigned int *encryptedKeyLens;
RecipientKeyInfo *recipientKeys;
unsigned int recipientKeyCount;
int encryptionAlgorithm;
{
  R_RSA_PUBLIC_KEY **publicKeys = (R_RSA_PUBLIC_KEY **)NULL;
  char *errorMessage = (char *)NULL;
  int status;
  unsigned char **encryptedKeys = (unsigned char **)NULL;
  unsigned int i;

  /* For error, break to end of do while (0) block. */
  do {
    /* Allocate arrays for the public key and encrypted key pointers.
     */
    if ((publicKeys = (R_RSA_PUBLIC_KEY **)malloc
         (recipientKeyCount * sizeof (*publicKeys))) ==
        (R_RSA_PUBLIC_KEY **)NULL ||
        (encryptedKeys = (unsigned char **)malloc
         (recipientKeyCount * sizeof (*encryptedKeys))) ==
        (unsigned char **)NULL) {
      errorMessage = ERR_MALLOC;
      break;
    }

    /* Set up the arrays for each public key and allocate the buffers
         which will hold the actual encrypted keys.
     */
    for (i = 0; i < recipientKeyCount; ++i) {
      publicKeys[i] = &recipientKeys[i].publicKey;
      encryptedKeys[i] = encryptedKeysBuffer + i * MAX_ENCRYPTED_KEY_LEN;
    }

    /* Now we can call R_SealInit which will create all the encrypted
         DEKs and generate the iv.
     */
    if ((status = R_SealInit
         (sealContext, encryptedKeys, encryptedKeyLens, iv, recipientKeyCount,
          publicKeys, encryptionAlgorithm, &ripemInfo->randomStruct)) != 0)
      return (FormatRSAError (status));

    if (ripemInfo->debug > 1) {
      char hex_digest[36], line[80];

      /* Now print the debug info.
       */
      for (i = 0; i < recipientKeyCount; ++i) {
        fprintf
          (ripemInfo->debugStream, "keyLen=%u, user=%s\n",
           encryptedKeyLens[i], recipientKeys[i].username);
        MakeHexDigest (encryptedKeys[i], encryptedKeyLens[i], hex_digest);
        fprintf
          (ripemInfo->debugStream,
           " MD5 of Encrypted Key (not recoded) = %s\n", hex_digest);
        fprintf(ripemInfo->debugStream,"  Encrypted, encoded MIC =\n");
        BinToHex (iv, 8, line);
        fprintf
          (ripemInfo->debugStream, " Initializing vector      = %s\n", line);
      }

      BinToHex (iv, 8, line);
      fprintf
        (ripemInfo->debugStream, " Initializing vector      = %s\n", line);
    }
  } while (0);

  free (publicKeys);
  free (encryptedKeys);
  return (errorMessage);
}

/* This decodes messageInfo->msg_key and initializes envelopeContext
     with messageInfo->ea and messageInfo->iv and ripemInfo's private key.
   This also decodes and decrypts the messageInfo->mic, writing it to
     signature and setting signatureLen. Assume signature is at
     least MAX_SIGNATURE_LEN.
   This returns 0 or an RSAREF error code (not a char * message).
 */
static int RIPEMOpenInit
  (ripemInfo, envelopeContext, signature, signatureLen)
RIPEMInfo *ripemInfo;
R_ENVELOPE_CTX *envelopeContext;
unsigned char *signature;
unsigned int *signatureLen;
{
  int status;
  unsigned char encryptedKeyBlock[MAX_ENCRYPTED_KEY_LEN];
  unsigned int encryptedKeyBlockLen;
  RIPEMDecipherFrame *frame = ripemInfo->z.decipherFrame;

  /* Make sure input values are not too big (just like in R_OpenPEMBlock).
   */
  if (frame->messageInfo.msg_key_len > MAX_PEM_ENCRYPTED_KEY_LEN)
    return (RE_KEY_ENCODING);
  
  if (frame->messageInfo.mic_len > MAX_PEM_ENCRYPTED_SIGNATURE_LEN)
    return (RE_SIGNATURE_ENCODING);

  if (R_DecodePEMBlock 
      (encryptedKeyBlock, &encryptedKeyBlockLen, frame->messageInfo.msg_key,
       frame->messageInfo.msg_key_len) != 0)
    return (RE_KEY_ENCODING);

  if ((status = R_OpenInit
       (envelopeContext, frame->messageInfo.ea, encryptedKeyBlock,
        encryptedKeyBlockLen, frame->messageInfo.iv,
        &ripemInfo->privateKey)) != 0)
    return (status);

  if ((status = MyDecryptPEMUpdateFinal
       (envelopeContext, signature, signatureLen, frame->messageInfo.mic,
        frame->messageInfo.mic_len)) != 0) {
    if (status == RE_LEN || status == RE_ENCODING)
      status = RE_SIGNATURE_ENCODING;
    else
      status = RE_KEY;
    return (status);
  }

  /* encryptedKeyBlock is not sensitive, so no need to zeroize. */
  return (0);
}

/* This is an exact copy of DecryptPEMUpdateFinal in RSAREF.
 */
static int MyDecryptPEMUpdateFinal
  (context, output, outputLen, input, inputLen)
R_ENVELOPE_CTX *context;
unsigned char *output;                          /* decoded, decrypted block */
unsigned int *outputLen;                                /* length of output */
unsigned char *input;                           /* encrypted, encoded block */
unsigned int inputLen;                                            /* length */
{
  int status;
  unsigned char encryptedPart[24];
  unsigned int i, len;
  
  do {
    /* Choose a buffer size of 24 bytes to hold the temporary decoded output
         which will be decrypted.
       Decode and decrypt as many 32-byte input blocks as possible.
     */
    *outputLen = 0;
    for (i = 0; i < inputLen/32; i++) {
      /* len is always 24 */
      if ((status = R_DecodePEMBlock
           (encryptedPart, &len, &input[32*i], 32)) != 0)
        break;

      /* Excpect no error return */
      R_OpenUpdate (context, output, &len, encryptedPart, 24);
      output += len;
      *outputLen += len;
    }
    if (status)
      break;

    /* Decode the last part */  
    if ((status = R_DecodePEMBlock
         (encryptedPart, &len, &input[32*i], inputLen - 32*i)) != 0)
      break;

    /* Decrypt the last part.
     */
    R_OpenUpdate (context, output, &len, encryptedPart, len);
    output += len;
    *outputLen += len;
    if ((status = R_OpenFinal (context, output, &len)) != 0)
      break;
    *outputLen += len;
  } while (0);

  /* Zeroize sensitive information.
   */
  R_memset ((POINTER)&context, 0, sizeof (context));
  R_memset ((POINTER)encryptedPart, 0, sizeof (encryptedPart));

  return (status);
}

#endif /* RIPEMSIG */

/* This closes the first private key file in ripemDatabase and overwrites
     it with the encrypted private key.  Then it re-opens the file for read.
 */
static char *EncryptAndWritePrivateKey
  (ripemInfo, password, passwordLen, ripemDatabase)
RIPEMInfo *ripemInfo;
unsigned char *password;
unsigned int passwordLen;
RIPEMDatabase *ripemDatabase;
{
  char *errorMessage = NULL;
  unsigned char *der = (unsigned char *)NULL,
    *der_enc_priv = (unsigned char *)NULL, salt[SALT_SIZE];
  unsigned int derlen, iter_count=100, enc_priv_len, der_enc_priv_len;
  size_t nbytes;
  TypListEntry *entry;
  TypFile *typFile;
  FILE *privOutStream = (FILE *)NULL;

  /* For error, break to end of do while (0) block. */
  do {
    if (!ripemDatabase->privKeySource.filelist.firstptr)
      /* No private key source entries. */
      break;

    /* Close the private key input stream since we must overwrite */
    typFile =
      (TypFile *)ripemDatabase->privKeySource.filelist.firstptr->dataptr;
    if (typFile->stream != (FILE *)NULL) {
      fclose (typFile->stream);
      typFile->stream = (FILE *)NULL;
    }

    /* Open the private key output file and write:
     * -- The header.
     * -- The list of usernames for this user.
     * -- The public key.
     */
    if ((privOutStream = fopen (typFile->filename, "w")) == (FILE *)NULL) {
      /* We don't really expect this error since it has already been
           checked in OpenKeySource. */
      sprintf(ripemInfo->errMsgTxt,"Can't open private key output file %s.",
              typFile->filename);
      errorMessage = ripemInfo->errMsgTxt;
      break;
    }
  
    fprintf(privOutStream,"%s\n",PRIV_KEY_STRING_BEGIN);
    for(entry=ripemInfo->userList->firstptr; entry; entry=entry->nextptr) {
      fprintf(privOutStream,"User: %s\n",(char *)entry->dataptr);
    }

    /* Now process the private key.
     */
    nbytes = PrivKeyToDERLen(&ripemInfo->privateKey)+DES_BLOCK_SIZE;
    der = (unsigned char *) malloc(nbytes);
    if (der == (unsigned char *)NULL) {
      errorMessage = ERR_MALLOC;
      break;
    }

    /* DER encode the private key */
    PrivKeyToDER (&ripemInfo->privateKey, der, &derlen);
    if(ripemInfo->debug>1) {
      fprintf(ripemInfo->debugStream,"DER encoding of private key:\n        ");
      BEMParse(der,ripemInfo->debugStream);
    }

    /* Encrypt the private key.  We must get the
     * password from the user, and we must calculate a 
     * pseudo-random salt.
     */
    R_RandomUpdate(&ripemInfo->randomStruct,password,passwordLen);
    R_GenerateBytes(salt,sizeof(salt),&ripemInfo->randomStruct);

    if (pbeWithMDAndDESWithCBC
        (TRUE,DA_MD5,der,derlen,password, passwordLen,salt,iter_count,
         &enc_priv_len) != 0) {
      errorMessage = "Can't encrypt private key.";
      break;
    }

    /* DER-encode the encrypted private key and write it out.
     */
    der_enc_priv = (unsigned char *) 
      malloc(EncryptedPrivKeyToDERLen(iter_count,enc_priv_len));
    if(!der_enc_priv) {
      errorMessage = ERR_MALLOC;
      break;
    }

    if(EncryptedPrivKeyToDER
       (salt, iter_count, der, enc_priv_len, der_enc_priv,
        &der_enc_priv_len) != 0) {
      errorMessage = "Can't DER encode encrypted private key.";
      break;
    }

    if(ripemInfo->debug>1) {
      fprintf
        (ripemInfo->debugStream,"DER encoding of encrypted private key:\n");
      BEMParse(der_enc_priv,ripemInfo->debugStream);
    }

    fprintf(privOutStream,"EncryptedPrivateKeyInfo:\n");
      CodeAndWriteBytes(der_enc_priv,der_enc_priv_len," ",privOutStream);

    if(ripemInfo->debug>1) {
      DumpPubKey(&ripemInfo->publicKey, ripemInfo->debugStream);
      DumpPrivKey(&ripemInfo->privateKey, ripemInfo->debugStream);
    }

    fprintf(privOutStream,"%s\n",PRIV_KEY_STRING_END);

    /* Now close for writing and re-open for read.
     */
    fclose (privOutStream);
    privOutStream = (FILE *)NULL;
    if ((typFile->stream = fopen (typFile->filename, "r")) == (FILE *)NULL) {
      sprintf(ripemInfo->errMsgTxt,
              "Can't open key file %s for read", typFile->filename);
      errorMessage = ripemInfo->errMsgTxt;
      break;
    }
  } while (0);

  free(der_enc_priv);
  free(der);
  if (privOutStream != (FILE *)NULL)
    fclose (privOutStream);

  return (errorMessage);
}

/* Returns the first entry in ripemInfo's userList, or NULL if empty.
 */
static char *FirstUsername (ripemInfo)
RIPEMInfo *ripemInfo;
{
  if (ripemInfo->userList->firstptr)
    return ((char *)ripemInfo->userList->firstptr->dataptr);
  else
    return ((char *)NULL);
}

/* Add name to nameList if it is not already there.
 */
static char *AddNameToList (nameList, name)
TypList *nameList;
DistinguishedNameStruct *name;
{
  TypListEntry *entry;
  DistinguishedNameStruct *nameCopy;

  for (entry = nameList->firstptr; entry; entry = entry->nextptr) {
    if (R_memcmp
        ((POINTER)entry->dataptr, (POINTER)name, sizeof (*name)) == 0)
      /* name is already in the list. */
      return ((char *)NULL);
  }

  /* Allocate a copy of the name and add it to the list.
   */
  if ((nameCopy = (DistinguishedNameStruct *)malloc (sizeof (*nameCopy)))
      == (DistinguishedNameStruct *)NULL)
    return (ERR_MALLOC);
  *nameCopy = *name;
  return (AddToList
          ((TypListEntry *)NULL, nameCopy, sizeof (*nameCopy), nameList));
}

