/* No representations are made concerning either the merchantability of
   this software or the suitability of this software for any particular
   purpose. It is provided "as is" without express or implied warranty
   of any kind.  
                                                                    
   License to copy and use this software is granted provided that these
   notices are retained in any copies of any part of this documentation
   and/or software.  
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
#include "bemparse.h"
#include "hexbinpr.h"
#include "bfstream.h"
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
#define MAX_TAG_AND_LEN_BYTES 6

/* BER class/tag codes */
#define BER_CONSTRUCTED 0x20
#define BER_CONTEXT_SPECIFIC 0x80
#define BER_INTEGER 2
#define BER_BIT_STRING 3
#define BER_OCTET_STRING 4
#define BER_NULL 5
#define BER_OBJECT_ID 6
#define BER_SEQUENCE (16 | BER_CONSTRUCTED)
#define BER_SET (17 | BER_CONSTRUCTED)

/* This is used in RIPEMDecipherPKCSUpdate */
#define ADVANCE_INPUT(len) {input += (len); inputLen -= (len);}

typedef enum {
  DS_START,
  DS_CONTENT_TYPE,
  DS_DATA_VERSION,
  DS_CHECK_DATA_VERSION,
  DS_RECIPIENT_INFOS_START,
  DS_CHECK_RECIPIENT_INFOS_END,
  DS_RECIPIENT_INFO,
  DS_DIGEST_ALGORITHMS_START,
  DS_GET_DIGEST_ALGORITHM,
  DS_CONTENT_INFO_DATA_START,
  DS_ENCRYPTION_ALGORITHM,
  DS_EXPLICIT_OCTET_STRING,
  DS_IMPLICIT_OCTET_STRING,
  DS_CHECK_OCTET_STRING_END,
  DS_OCTET_STRING_PART_START,
  DS_OCTET_STRING_PART,
  DS_END_EXPLICIT_DATA,
  DS_CONTENT_INFO_DATA_END,
  DS_CHECK_CERTIFICATES_SET,
  DS_CHECK_CERTIFICATES_SET_END,
  DS_CERTIFICATE,
  DS_CHECK_CRLS_SET,
  DS_CHECK_CRLS_SET_END,
  DS_CRL,
  DS_SIGNER_INFOS_START,
  DS_CHECK_SIGNER_INFOS_END,
  DS_SIGNER_INFO,
  DS_END_ENCODING,
  DS_FINISHED
} DECODE_STATE; 

typedef struct {
  RIPEMEncipherFrame ripemEncipherFrame;                      /* "base class */
  BufferStream outStream;     /* This holds the value returned to the caller */
  BOOL startNewOctetString;
  int pkcsMode;
  TypList issuerNames;
  R_ENVELOPE_CTX sealContext;
  R_SIGNATURE_CTX signatureContext;
} RIPEMEncipherPKCSFrame;

typedef struct {
  RIPEMDecipherFrame ripemDecipherFrame;                      /* "base class */
  BufferStream outStream;     /* This holds the value returned to the caller */
  BufferStream input;                    /* Used to accumulate during Update */
  int pkcsMode;
  int detached;
  int digestAlgorithm;
  int expectedDigestAlgorithm;                    /* Used only with detached */
  TypList certs;
  R_ENVELOPE_CTX envelopeContext;
  R_SIGNATURE_CTX signatureContext;
  R_RSA_PUBLIC_KEY senderKey;                /* used to execute final verify */
  TypList certChain;                              /* Local copy of certChain */
  ChainStatusInfo chainStatus;
  RIPEMAttributes authenticatedAttributes;
  TypList attributesBuffers;                    /* buffers attributes values */

  /* These are used to track the state during Update */
  DECODE_STATE decodeState;       /* Used to keep track of where in decoding */
  unsigned int bytesNeeded;
  BOOL foundRecipient, foundSigner;
  unsigned int octetStringContentLen;

  /* Put these in the frame since they will be allocated and freed correctly */
  DistinguishedNameStruct issuerName;
  unsigned char serialNumber[MAX_SERIAL_NUMBER_LEN];
  unsigned char signature[MAX_SIGNATURE_LEN];
  unsigned char encryptedKey[MAX_ENCRYPTED_KEY_LEN];
  unsigned int encryptedKeyLen;
} RIPEMDecipherPKCSFrame;

static char *ERR_PKCS_ENCODING = "Invalid encoding of input PKCS message";

static unsigned char END_INDEFINITE_LEN[] = {0, 0};

/* Following are chunks for a PKCS #7 ContentInfo including a SignedData,
     SignedAndEnvelopedData, etc.
  */
static unsigned char CONTENT_INFO_START[] = {
  BER_SEQUENCE, 0x80 /* Indefinite length */
};

static unsigned char CONTENT_TYPE_SIGNED_DATA[] = {
  BER_OBJECT_ID, 9, 1*40+2, 0x86, 0x48, 0x86, 0xf7, 0x0d, 1, 7, 2
};
static unsigned char CONTENT_TYPE_ENVELOPED_DATA[] = {
  BER_OBJECT_ID, 9, 1*40+2, 0x86, 0x48, 0x86, 0xf7, 0x0d, 1, 7, 3
};
static unsigned char CONTENT_TYPE_SIGNED_ENVELOPED_DATA[] = {
  BER_OBJECT_ID, 9, 1*40+2, 0x86, 0x48, 0x86, 0xf7, 0x0d, 1, 7, 4
};

/* This outputs up to the version but does not include the actual
   0 or 1 for the version.  It includes the [0] EXPLICIT at the end of the
     ContentInfo. */
static unsigned char DATA_VERSION[] = {
  BER_CONSTRUCTED | BER_CONTEXT_SPECIFIC | 0, 0x80, /* Indefinite length */
  BER_SEQUENCE, 0x80, /* Indefinite length */
  BER_INTEGER, 1 /* Must put a 0 or 1 here for the version! */
};

/* Must put out END_INDEFINITE_LEN after the recipient infos. */
static unsigned char RECIPIENT_INFOS_START[] = {
  BER_SET, 0x80 /* Indefinite length */
};

/* DigestAlgorithmIdentifiers set with no members (for certs only message) */
static unsigned char EMPTY_DIGEST_ALGORITHMS[] = {
  BER_SET, 0
};

/* Start of DigestAlgorithmIdentifiers set the right size for
     containing one MD2 or MD5 */
static unsigned char DIGEST_ALGORITHMS_START[] = {
  BER_SET, 14
};

/* Start of ContentInfo or EncryptedContentInfo for data. */
static unsigned char CONTENT_INFO_DATA_START[] = {
  BER_SEQUENCE, 0x80, /* Indefinite length */
    BER_OBJECT_ID, 9, 1*40+2, 0x86, 0x48, 0x86, 0xf7, 0x0d, 1, 7, 1
};

/* An definite-length encoding of ContentInfo with omitted content. */
static unsigned char EMPTY_CONTENT_INFO_DATA[] = {
  BER_SEQUENCE, 11,
    BER_OBJECT_ID, 9, 1*40+2, 0x86, 0x48, 0x86, 0xf7, 0x0d, 1, 7, 1
};

/* NOTE: This must be followed with the 8 byte IV. */
static unsigned char ALG_ID_DES_CBC[] = {
  BER_SEQUENCE, 17,
    BER_OBJECT_ID, 5, 0x2b, 0x0e, 0x03, 0x02, 0x07,
    BER_OCTET_STRING, 8 /* bytes of IV to follow */
};

/* NOTE: This must be followed with the 8 byte IV. */
static unsigned char ALG_ID_DES_EDE3_CBC[] = {
  BER_SEQUENCE, 20,
    BER_OBJECT_ID, 8, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x03, 0x07,
    BER_OCTET_STRING, 8 /* bytes of IV to follow */
};

/* [0] EXPLICIT start for a constructed octet string.  Used for
     ContentInfo inside SignedInfo. */
static unsigned char EXPLICIT_OCTET_STRING_START[] = {
  BER_CONSTRUCTED | BER_CONTEXT_SPECIFIC | 0, 0x80, /* Indefinite length */
    BER_OCTET_STRING | BER_CONSTRUCTED, 0x80 /* Indefinite length */
};

/* [0] IMPLICIT start for a constructed octet string.  Used for
     EncryptedContentInfo. */
static unsigned char IMPLICIT_OCTET_STRING_START[] = {
  BER_CONSTRUCTED | BER_CONTEXT_SPECIFIC | 0, 0x80 /* Indefinite length */
};

/* Must put out END_INDEFINITE_LEN after the certs. */
static unsigned char CERTIFICATES_SET[] = {
  /* [0] IMPLICIT */
  BER_CONSTRUCTED | BER_CONTEXT_SPECIFIC | 0, 0x80 /* Indefinite length */
};

/* Must put out END_INDEFINITE_LEN after the CRLs. */
static unsigned char CRLS_SET[] = {
  /* [1] IMPLICIT */
  BER_CONSTRUCTED | BER_CONTEXT_SPECIFIC | 1, 0x80 /* Indefinite length */
};

/* Must put out END_INDEFINITE_LEN after the signer infos. */
static unsigned char SIGNER_INFOS_START[] = {
  BER_SET, 0x80 /* Indefinite length */
};

/* algorithm identifier for MD2 */
static unsigned char ALG_ID_MD2[] = {
  BER_SEQUENCE, 12,
    BER_OBJECT_ID, 8, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x02, 0x02,
    BER_NULL, 0     
};

/* algorithm identifier for MD5 */
static unsigned char ALG_ID_MD5[] = {
  BER_SEQUENCE, 12,
    BER_OBJECT_ID, 8, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x02, 0x05,
    BER_NULL, 0     
};

/* algorithm identifier for rsaEncryption */
static unsigned char ALG_ID_RSA_ENCRYPTION[] = {
  BER_SEQUENCE, 13,
    BER_OBJECT_ID, 9, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 1, 1, 1,
    BER_NULL, 0     
};

/* algorithm identifier for md5WithRSAEncryption */
static unsigned char ALG_ID_MD5_WITH_RSA_ENCRYPTION[] = {
  BER_SEQUENCE, 13,
    BER_OBJECT_ID, 9, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 1, 1, 4,
    BER_NULL, 0     
};

/* Close out the SignedData/EnvelopedData/SignedAndEnvelopedData type
     and [0] EXPLICIT from the ContentInfo containing it, and also the
     outermost ContentInfo. */
static unsigned char END_DATA_EXPLICIT_AND_CONTENT_INFO[] = {
  0, 0, 0, 0, 0, 0
};

static unsigned char SIGNING_TIME_ID[] = {
  BER_OBJECT_ID, 9, 1*40+2, 0x86, 0x48, 0x86, 0xf7, 0x0d, 1, 9, 5
};
static unsigned char SIGNING_DESCRIPTION_ID[] = {
  BER_OBJECT_ID, 9, 1*40+2, 0x86, 0x48, 0x86, 0xf7, 0x0d, 1, 9, 13
};
static unsigned char CHALLENGE_PASSWORD_ID[] = {
  BER_OBJECT_ID, 9, 1*40+2, 0x86, 0x48, 0x86, 0xf7, 0x0d, 1, 9, 7
};
static unsigned char CONTENT_TYPE_ID[] = {
  BER_OBJECT_ID, 9, 1*40+2, 0x86, 0x48, 0x86, 0xf7, 0x0d, 1, 9, 3
};
static unsigned char MESSAGE_DIGEST_ID[] = {
  BER_OBJECT_ID, 9, 1*40+2, 0x86, 0x48, 0x86, 0xf7, 0x0d, 1, 9, 4
};

void RIPEMEncipherPKCSFrameConstructor P((RIPEMEncipherPKCSFrame *));
void RIPEMEncipherPKCSFrameDestructor P((RIPEMEncipherPKCSFrame *));
void RIPEMDecipherPKCSFrameConstructor P((RIPEMDecipherPKCSFrame *));
void RIPEMDecipherPKCSFrameDestructor P((RIPEMDecipherPKCSFrame *));
static char *WriteSignerInfo
  P((BufferStream *, unsigned char *, unsigned int, unsigned char *,
     unsigned int, unsigned char *, unsigned int));
static char *WriteCertifyRequestInfo
  P((BufferStream *, DistinguishedNameStruct *, R_RSA_PUBLIC_KEY *,
     RIPEMAttributes *));
#ifndef RIPEMSIG
static char *WriteRecipientInfo
  P((BufferStream *, unsigned char *, unsigned int, unsigned char *,
     unsigned int, unsigned char *, unsigned int));
static char *DecodeRecipientInfo
  P((unsigned char *, DistinguishedNameStruct *, unsigned char *,
     unsigned char **, unsigned int *));
#endif
static char *DecodeSignerInfo
  P((unsigned char *, DistinguishedNameStruct *, unsigned char *,
     int, R_SIGNATURE_CTX *, int, unsigned char **, unsigned int *,
     RIPEMAttributes *, TypList *));
static char *ProcessAuthenticatedAttributes
  P((unsigned char *, unsigned int, int, R_SIGNATURE_CTX *, int, 
     RIPEMAttributes *, TypList *));
static char *DecodeAuthenticatedAttributes
  P((unsigned char *, unsigned char *, int *, RIPEMAttributes *,
     TypList *));
static int DecodeContentType P((unsigned char *));
static char *GetSignerPublicKey
  P ((RIPEMDecipherPKCSFrame *, RIPEMInfo *, R_RSA_PUBLIC_KEY *,
      RIPEMDatabase *));
static char *FindMatchingSelfSignedCert
  P ((RIPEMInfo *, DistinguishedNameStruct *, R_RSA_PUBLIC_KEY *));
static void GetEncipherUpdateOutput
  P ((unsigned char **, unsigned int *, BufferStream *));
static char *WriteCertsAndSigner P ((RIPEMInfo *));

/* Initialize for preparing a PKCS message according to enhanceMode.
   The calling routine must already have called RIPEMLoginUser.
   pkcsMode should be PKCS_SIGNED, PKCS_ENVELOPED or
     PKCS_SIGNED | PKCS_ENVELOPED.  If pkcsMode has MODE_ENVELOPED set,
     then encryptionAlgorithm must be EA_DES_CBC or EA_DES_EDE3_CBC. Also,
     recipientKeys is an array of recipientKeyCount RecipientKeyInfo structs.
     These give the public keys and usernames of the recipients.  The username
     is used for looking up the user's issuer name and serial number
     The randomStruct in ripemInfo must already be initialized.
   This returns a pointer to the output in partOut and its length in
     partOutLen.  The memory for the output is allocated inside ripemInfo
     and should be treated as "read only".  Upon return, the output must be
     copied or written to a file since future calls to RIPEM may modify it. On
     error return, the pointer to the output is undefined.
   The output is "as is": no translation of '\n' to <CR><LF> is done because
     this message format can support binary data.  The calling routine must
     do end-of-line character translation if necessary.
   ripemDatabase is used for selecting certificates to find issuer names
     and serial numbers of recipients.
   After this, call RIPEMEncipherPKCSUpdate to enhance the text by parts,
     and call RIPEMEncipherPKCSFinal to finish.
   Return NULL for success or error string.
 */
char *RIPEMEncipherPKCSInit
  (ripemInfo, partOut, partOutLen, pkcsMode, encryptionAlgorithm,
   recipientKeys, recipientKeyCount, ripemDatabase)
RIPEMInfo *ripemInfo;
unsigned char **partOut;
unsigned int *partOutLen;
int pkcsMode;
int encryptionAlgorithm;
RecipientKeyInfo *recipientKeys;
unsigned int recipientKeyCount;
RIPEMDatabase *ripemDatabase;
{
  RIPEMEncipherPKCSFrame *frame;
  CertificateStruct *certStruct = (CertificateStruct *)NULL;
  TypListEntry *entry;
  TypList certList;
  BufferStream *stream, nameDER;
  int status;
  char *errorMessage = (char *)NULL;
  unsigned char iv[IV_SIZE];
  unsigned char *encryptedKeysBuffer = (unsigned char *)NULL, *derPointer;
  unsigned int i, *encryptedKeyLens = (unsigned int *)NULL;
  
#ifdef RIPEMSIG
UNUSED_ARG (encryptionAlgorithm)
UNUSED_ARG (recipientKeys)
UNUSED_ARG (recipientKeyCount)
UNUSED_ARG (ripemDatabase)
#endif
  
  InitList (&certList);
  BufferStreamConstructor (&nameDER);
  
  /* For error, break to end of do while (0) block. */
  do {
    if (pkcsMode != PKCS_SIGNED && pkcsMode != PKCS_ENVELOPED &&
        pkcsMode != (PKCS_SIGNED | PKCS_ENVELOPED)) {
      errorMessage = "Invalid encipher mode.";
      break;
    }
    if (pkcsMode & PKCS_ENVELOPED) {
#ifdef RIPEMSIG
      errorMessage = "RIPEM/SIG cannot prepare encrypted messages. You may prepare signed messages.";
      break;
#else
      if (encryptionAlgorithm != EA_DES_CBC &&
          encryptionAlgorithm != EA_DES_EDE3_CBC) {
        errorMessage = "Unsupported encryption algorithm";
        break;
      }
#endif
    }

    /* Make sure any old frame is deleted and make a new one.
     */
    if (ripemInfo->z.encipherFrame != (RIPEMEncipherFrame *)NULL) {
      /* Be sure to call the "virtual" destructor */
      (*ripemInfo->z.encipherFrame->Destructor) (ripemInfo->z.encipherFrame);
      free (ripemInfo->z.encipherFrame);
    }
    /* Be sure to malloc for the size of an entire RIPEMEncipherPKCSFrame */
    if ((ripemInfo->z.encipherFrame = (RIPEMEncipherFrame *)malloc
         (sizeof (*frame))) == (RIPEMEncipherFrame *)NULL) {
      errorMessage = ERR_MALLOC;
      break;
    }
    RIPEMEncipherPKCSFrameConstructor
      ((RIPEMEncipherPKCSFrame *)ripemInfo->z.encipherFrame);

    /* Get stream for quick access. */
    frame = (RIPEMEncipherPKCSFrame *)ripemInfo->z.encipherFrame;
    stream = &frame->outStream;

    /* Get ready to write to the output. */
    BufferStreamRewind (stream);

    frame->pkcsMode = pkcsMode;

    if (pkcsMode & PKCS_SIGNED) {
      /* Initialize signature.  Always use MD5 */
      if ((status = R_SignInit (&frame->signatureContext, DA_MD5)) != 0) {
        errorMessage = FormatRSAError (status);
        break;
      }
    }
    
#ifndef RIPEMSIG
    if (pkcsMode & PKCS_ENVELOPED) {
      if (recipientKeyCount == 0) {
        errorMessage = "You must specify at least one recipient";
        break;
      }

      /* Allocate arrays for the encrypted key pointers.
       */
      if ((encryptedKeysBuffer = (unsigned char *)malloc
           (recipientKeyCount * MAX_ENCRYPTED_KEY_LEN)) ==
          (unsigned char *)NULL) {
        errorMessage = ERR_MALLOC;
        break;
      }
      if ((encryptedKeyLens = (unsigned int *)malloc
           (recipientKeyCount * sizeof (*encryptedKeyLens))) ==
          (unsigned int *)NULL) {
        errorMessage = ERR_MALLOC;
        break;
      }

      /* Create all of the recipient key info blocks and generate the iv.
         We can't output now because the signature comes before the
           encrypted keys.
       */
      if ((errorMessage = RIPEMSealInit
           (ripemInfo, &frame->sealContext, iv, encryptedKeysBuffer,
            encryptedKeyLens, recipientKeys, recipientKeyCount,
            encryptionAlgorithm)) != (char *)NULL)
        break;
    }
#endif

    /* Write the PKCS #7 ContentInfo to the output stream.
     */
    if ((errorMessage = BufferStreamWrite
         (CONTENT_INFO_START, sizeof (CONTENT_INFO_START), stream))
        != (char *)NULL)
      break;

    if (pkcsMode == PKCS_SIGNED) {
      if ((errorMessage = BufferStreamWrite
           (CONTENT_TYPE_SIGNED_DATA, sizeof (CONTENT_TYPE_SIGNED_DATA),
            stream)) != (char *)NULL)
        break;
    }
    else if (pkcsMode == PKCS_ENVELOPED) {
      if ((errorMessage = BufferStreamWrite
           (CONTENT_TYPE_ENVELOPED_DATA, sizeof (CONTENT_TYPE_ENVELOPED_DATA),
            stream)) != (char *)NULL)
        break;
    }
    else if (pkcsMode == (PKCS_SIGNED | PKCS_ENVELOPED)) {
      if ((errorMessage = BufferStreamWrite
           (CONTENT_TYPE_SIGNED_ENVELOPED_DATA,
            sizeof (CONTENT_TYPE_SIGNED_ENVELOPED_DATA), stream))
          != (char *)NULL)
        break;
    }

    if ((errorMessage = BufferStreamWrite
         (DATA_VERSION, sizeof (DATA_VERSION), stream)) != (char *)NULL)
      break;

    /* Must explicitly put the version byte. */
    if (pkcsMode & PKCS_SIGNED) {
      /* SignedData and SigneAndEnvelopedData have version 1 */
      if ((errorMessage = BufferStreamPutc (1, stream)) != (char *)NULL)
        break;
    }
    else {
      /* EnvelopedData have version 0 */
      if ((errorMessage = BufferStreamPutc (0, stream)) != (char *)NULL)
        break;
    }

#ifndef RIPEMSIG
    if (pkcsMode & PKCS_ENVELOPED) {
      /* Output the RecipientInfos.
       */
      if ((errorMessage = BufferStreamWrite
           (RECIPIENT_INFOS_START, sizeof (RECIPIENT_INFOS_START), stream))
          != (char *)NULL)
        break;

      /* Allocate the certStruct on the heap because it's big. */
      if ((certStruct = (CertificateStruct *)malloc
           (sizeof (*certStruct))) == (CertificateStruct *)NULL) {
        errorMessage = ERR_MALLOC;
        break;
      }
      
      /* For each recipient, write out the RecipientInfo
       */
      for (i = 0; i < recipientKeyCount; ++i) {
        /* For every certificate we can find with a matching
             username and public key, write its issuer name and serial
             number.
         */

        /* Free any previous contents of certList */
        FreeList (&certList);

        if ((errorMessage = GetCertsBySmartname
             (ripemDatabase, &certList, recipientKeys[i].username,
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
"Warning: Cannot decode certificate from database for writing RecipientInfo.\n");
            continue;
          }

          if (R_memcmp
              ((POINTER)&recipientKeys[i].publicKey,
               (POINTER)&certStruct->publicKey,
               sizeof (certStruct->publicKey)) != 0)
            /* Not the same public key.  Try the next */
            continue;

          /* Use nameDER repeatedly for storing the encoded name.
             First rewind and allocate enough space.
           */
          BufferStreamRewind (&nameDER);
          if ((errorMessage = BufferStreamWrite
               ((unsigned char *)NULL,
                len_distinguishedname (&certStruct->issuer) + 4,
                &nameDER)) != (char *)NULL)
            break;
          derPointer = nameDER.buffer;
          DistinguishedNameToDER (&certStruct->issuer, &derPointer);

          if ((errorMessage = WriteRecipientInfo
               (stream, encryptedKeysBuffer + i * MAX_ENCRYPTED_KEY_LEN,
                encryptedKeyLens[i], nameDER.buffer,
                derPointer - nameDER.buffer, certStruct->serialNumber,
                sizeof (certStruct->serialNumber))) != (char *)NULL)
            break;
        }
      }
      if (errorMessage != (char *)NULL)
        /* Broke loop because of error. */
        break;

      /* End the RecipientInfos */
      if ((errorMessage = BufferStreamWrite
           (END_INDEFINITE_LEN, sizeof (END_INDEFINITE_LEN), stream))
          != (char *)NULL)
        break;
    }
#endif

    if (pkcsMode & PKCS_SIGNED) {
      /* Always output MD5 */
      if ((errorMessage = BufferStreamWrite
           (DIGEST_ALGORITHMS_START, sizeof (DIGEST_ALGORITHMS_START), stream))
          != (char *)NULL)
        break;
      if ((errorMessage = BufferStreamWrite
           (ALG_ID_MD5, sizeof (ALG_ID_MD5), stream)) != (char *)NULL)
        break;
    }

    if ((errorMessage = BufferStreamWrite
         (CONTENT_INFO_DATA_START, sizeof (CONTENT_INFO_DATA_START), stream))
        != (char *)NULL)
      break;

#ifndef RIPEMSIG
    if (pkcsMode & PKCS_ENVELOPED) {
      if (encryptionAlgorithm == EA_DES_CBC) {
        if ((errorMessage = BufferStreamWrite
             (ALG_ID_DES_CBC, sizeof (ALG_ID_DES_CBC), stream))
            != (char *)NULL)
          break;
        if ((errorMessage = BufferStreamWrite (iv, 8, stream))
            != (char *)NULL)
          break;
      }
      else if (encryptionAlgorithm == EA_DES_EDE3_CBC) {
        if ((errorMessage = BufferStreamWrite
             (ALG_ID_DES_EDE3_CBC, sizeof (ALG_ID_DES_EDE3_CBC), stream))
            != (char *)NULL)
          break;
        if ((errorMessage = BufferStreamWrite (iv, 8, stream))
            != (char *)NULL)
          break;
      }

      /* We are inside an EncryptedContentInfo which has an IMPLICIT
           OCTET STRING. */
      if ((errorMessage = BufferStreamWrite
           (IMPLICIT_OCTET_STRING_START, sizeof (IMPLICIT_OCTET_STRING_START),
            stream)) != (char *)NULL)
        break;
    }
    else
#endif
    {
      /* Not inside EncryptedContentInfo */
      if ((errorMessage = BufferStreamWrite
           (EXPLICIT_OCTET_STRING_START, sizeof (EXPLICIT_OCTET_STRING_START),
            stream)) != (char *)NULL)
        break;
    }

    /* Get ready to output the content */
    frame->startNewOctetString = TRUE;
    
    /* Set the output. */
    *partOut = frame->outStream.buffer;
    *partOutLen = frame->outStream.point;
  } while (0);
  
  FreeList (&certList);
  free (certStruct);
  free (encryptedKeysBuffer);
  free (encryptedKeyLens);
  BufferStreamDestructor (&nameDER);
  return (errorMessage);
}

/* Before this is called for the first time, the caller should have called
     RIPEMEncipherPKCSInit.  This is repeatedly called so supply the
     data to enhance.
   The data to enhance is in partIn with length partInLen.
   This returns a pointer to the output in partOut and its length in
     partOutLen.  The memory for the output is allocated inside ripemInfo
     and should be treated as "read only".  Upon return, the output must be
     copied or written to a file since future calls to RIPEM may modify it. On
     error return, the pointer to the output is undefined.
   The input and output are "as is": no translation of '\n' to <CR><LF> is
     done because this message format can support binary data.  The
     calling routine must do end-of-line character translation if necessary.
   After this, call RIPEMEncipherPKCSFinal to finalize.
   Return NULL for success or error string.
 */
char *RIPEMEncipherPKCSUpdate
  (ripemInfo, partOut, partOutLen, partIn, partInLen)
RIPEMInfo *ripemInfo;
unsigned char **partOut;
unsigned int *partOutLen;
unsigned char *partIn;
unsigned int partInLen;
{
  RIPEMEncipherPKCSFrame *frame =
    (RIPEMEncipherPKCSFrame *)ripemInfo->z.encipherFrame;
  BufferStream *stream;
  char *errorMessage;
  int status;
  unsigned int localPartOutLen;

  if (frame == (RIPEMEncipherPKCSFrame *)NULL)
    return ("Encipher not initialized");
  if (frame->ripemEncipherFrame.Destructor != 
      (RIPEM_ENCIPHER_FRAME_DESTRUCTOR)RIPEMEncipherPKCSFrameDestructor)
    return ("Encipher frame was not initialized by RIPEMEncipherPKCSInit");

  /* Get stream for quick access. */
  stream = &frame->outStream;

  if (frame->pkcsMode & PKCS_SIGNED) {
    if ((status = R_SignUpdate
         (&frame->signatureContext, partIn, partInLen)) != 0)
      return (FormatRSAError (status));
  }

  if (frame->startNewOctetString) {
    /* Get ready to write to the output. */
    BufferStreamRewind (stream);

    /* Reserve enough bytes at the beginning of the buffer for us to
         put the der tag and length bytes there later. */
    if ((errorMessage = BufferStreamWrite
         ((unsigned char *)NULL, MAX_TAG_AND_LEN_BYTES, stream))
        != (char *)NULL)
      return (errorMessage);

    frame->startNewOctetString = FALSE;
  }

  if (frame->pkcsMode == PKCS_SIGNED) {
    /* All we have to do is copy the input to the output. */
    if ((errorMessage = BufferStreamWrite (partIn, partInLen, stream))
        != (char *)NULL)
      return (errorMessage);
  }
#ifndef RIPEMSIG
  else {
    /* We must encrypt.  We need at least partInLen + 7 bytes of space
         in the output.  This space begins after the MAX_TAG_AND_LEN_BYTES
         which we already reserved. */
    if ((errorMessage = BufferStreamWrite
         ((unsigned char *)NULL, partInLen + 7, stream)) != (char *)NULL)
      return (errorMessage);

    /* Now there is enough space to encrypt. We may have just reallocated
         the output buffer, so count back from the end of the buffer
         to find the place to encrypt to. */
    if ((status = R_SealUpdate
         (&frame->sealContext,
          (stream->buffer + stream->point) - (partInLen + 7),
          &localPartOutLen, partIn, partInLen)) != 0)
      return (FormatRSAError (status));

    /* Unput bytes in case we pre-allocated too many bytes. */
    BufferStreamUnput (stream, (partInLen + 7) - localPartOutLen);
  }
#endif

  /* Make sure we have accumulated enough bytes to make a reasonably-sized
       octet string. */
  if (stream->point < 512) {
    /* Wait to accumulate more */
    *partOut = stream->buffer;
    *partOutLen = 0;
  }
  else {
    /* Set the output and get ready to make new one. */
    GetEncipherUpdateOutput (partOut, partOutLen, stream);
    frame->startNewOctetString = TRUE;
  }
  
  return ((char *)NULL);
}

/* Call this after all text has been fed to RIPEMEncipherPKCSUpdate.  This
     flushes the output and writes the final bytes.  See
     RIPEMEncipherPKCSUpdate for a description of partOut and partOutLen.
   authenticatedAttributes is for future compatibility.  You should pass
     (RIPEMAttributes *)NULL.
   Return NULL for success or error string.
 */
char *RIPEMEncipherPKCSFinal
  (ripemInfo, partOut, partOutLen, authenticatedAttributes)
RIPEMInfo *ripemInfo;
unsigned char **partOut;
unsigned int *partOutLen;
RIPEMAttributes *authenticatedAttributes;
{
  RIPEMEncipherPKCSFrame *frame =
    (RIPEMEncipherPKCSFrame *)ripemInfo->z.encipherFrame;
  BufferStream *stream;
  char *errorMessage;
  int status;
  unsigned char finalBlock[8];
  unsigned int localPartOutLen, partOutOffset;

UNUSED_ARG (authenticatedAttributes)

  if (frame == (RIPEMEncipherPKCSFrame *)NULL)
    return ("Encipher not initialized");
  if (frame->ripemEncipherFrame.Destructor != 
      (RIPEM_ENCIPHER_FRAME_DESTRUCTOR)RIPEMEncipherPKCSFrameDestructor)
    return ("Encipher frame was not initialized by RIPEMEncipherPKCSInit");

  /* Get stream for quick access. */
  stream = &frame->outStream;

#ifndef RIPEMSIG
  if (frame->pkcsMode & PKCS_ENVELOPED) {
    /* We need to put the final encryption block in the output. */

    if (frame->startNewOctetString) {
      /* Get ready to write a new octet string which just contains the
           final encryption block. */
      BufferStreamRewind (stream);

      /* Reserve enough bytes at the beginning of the buffer for us to
           put the der tag and length bytes there later. */
      if ((errorMessage = BufferStreamWrite
           ((unsigned char *)NULL, MAX_TAG_AND_LEN_BYTES, stream))
          != (char *)NULL)
        return (errorMessage);

      frame->startNewOctetString = FALSE;
    }

    /* Flush the encryption.  The final block is 8 bytes long. */
    if ((status = R_SealFinal
         (&frame->sealContext, finalBlock, &localPartOutLen)) != 0)
      return (FormatRSAError (status));

    if ((errorMessage = BufferStreamWrite
         (finalBlock, localPartOutLen, stream)) != (char *)NULL)
      return (errorMessage);
  }
#endif

  if (frame->startNewOctetString) {
    /* We are supposed to start a new octet string.  If this message
         were encrypted, then we would have set this to FALSE above,
         so we are in a signed message.  There is nothing to put in
         the output, so skip this octet string. */
    BufferStreamRewind (stream);
    partOutOffset = 0;
  }
  else {
    /* Note that the last sub-encoding of the indefinite-length octet
         string goes at the beginning of this buffer before all the rest. */
    GetEncipherUpdateOutput (partOut, &localPartOutLen, stream);
    /* partOut will be set again below, but find out how far into
         the output it is. */
    partOutOffset = *partOut - stream->buffer;
  }

  /* Close out the indefinite length constructed OCTET */
  if ((errorMessage = BufferStreamWrite
       (END_INDEFINITE_LEN, sizeof (END_INDEFINITE_LEN), stream))
      != (char *)NULL)
    return (errorMessage);

  if (! (frame->pkcsMode & PKCS_ENVELOPED)) {
    /* Close out the [0] EXPLICIT from the ContentInfo.
       (The EncryptedContentInfo has IMPLICIT and doesn't need this). */
    if ((errorMessage = BufferStreamWrite
         (END_INDEFINITE_LEN, sizeof (END_INDEFINITE_LEN), stream))
        != (char *)NULL)
      return (errorMessage);
  }

  /* Close out the ContentInfo. */
  if ((errorMessage = BufferStreamWrite
       (END_INDEFINITE_LEN, sizeof (END_INDEFINITE_LEN), stream))
      != (char *)NULL)
    return (errorMessage);

  if (frame->pkcsMode & PKCS_SIGNED) {
    if ((errorMessage = WriteCertsAndSigner (ripemInfo)) != (char *)NULL)
      return (errorMessage);
  }

  if ((errorMessage = BufferStreamWrite
       (END_DATA_EXPLICIT_AND_CONTENT_INFO,
        sizeof (END_DATA_EXPLICIT_AND_CONTENT_INFO), stream))
      != (char *)NULL)
    return (errorMessage);

  /* Set the output.  Include the offset that may have been set
       while encoding the last octet string of the data content. */
  *partOut = stream->buffer + partOutOffset;
  *partOutLen = stream->point - partOutOffset;
  
  return ((char *)NULL);
}

/* Initialize ripemInfo for deciphering a PKCS message.
   After this, call RIPEMDecipherPKCSUpdate to supply the enhanced message
     by parts and call RIPEMDecipherPKCSFinal to finish and obtain the
     sender information.
   Return NULL for success or error string.
 */
char *RIPEMDecipherPKCSInit (ripemInfo)
RIPEMInfo *ripemInfo;
{
  RIPEMDecipherPKCSFrame *frame;

  /* Make sure any old frame is deleted and make a new one.
   */
  if (ripemInfo->z.decipherFrame != (RIPEMDecipherFrame *)NULL) {
    /* Be sure to call the "virtual" destructor */
    (*ripemInfo->z.decipherFrame->Destructor) (ripemInfo->z.decipherFrame);
    free (ripemInfo->z.decipherFrame);
  }
  /* Be sure to malloc for the size of an entire RIPEMDecipherPKCSFrame */
  if ((ripemInfo->z.decipherFrame = (RIPEMDecipherFrame *)malloc
       (sizeof (RIPEMDecipherPKCSFrame))) == (RIPEMDecipherFrame *)NULL)
    return (ERR_MALLOC);
  RIPEMDecipherPKCSFrameConstructor
    ((RIPEMDecipherPKCSFrame *)ripemInfo->z.decipherFrame);

  frame = (RIPEMDecipherPKCSFrame *)ripemInfo->z.decipherFrame;

  /* Pre-set to not finding any attributes. */
  InitRIPEMAttributes (&frame->authenticatedAttributes);
  frame->foundRecipient = FALSE;
  frame->foundSigner = FALSE;

  /* Set up for the first decoding state. */
  frame->decodeState = DS_START;
  frame->bytesNeeded = sizeof (CONTENT_INFO_START);

  return ((char *)NULL);
}

/* Call this after the decipher operation has been initialized with
     RIPEMDecipherPKCSInit.
   This deciphers the message in partIn of length partInLen, which contains the
     enhanced message.  partIn may contain any number of bytes.  This
     may be called multiple times.
  This returns a pointer to the output in partOut and its length in
     partOutLen.  The memory for the output is allocated inside ripemInfo
     and should be treated as "read only".  Upon return, the output must be
     copied or written to a file since future calls to RIPEM may modify it. On
     error return, the pointer to the output is undefined.
   The input and output are "as is": no translation of '\n' to <CR><LF> is
     done because this message format can support binary data.  The
     calling routine must do end-of-line character translation if necessary.
   After calling this to supply the enhanced message, call
     RIPEMDecipherPKCSFinal to finalize.
   Return NULL for success or error string.
 */
char *RIPEMDecipherPKCSUpdate
  (ripemInfo, partOut, partOutLen, partIn, partInLen, ripemDatabase)
RIPEMInfo *ripemInfo;
unsigned char **partOut;
unsigned int *partOutLen;
unsigned char *partIn;
unsigned int partInLen;
RIPEMDatabase *ripemDatabase;
{
  char *errorMessage;
  int status, encryptionAlgorithm;
  RIPEMDecipherPKCSFrame *frame =
    (RIPEMDecipherPKCSFrame *)ripemInfo->z.decipherFrame;
  unsigned char *input, *saveInput, *iv, *toSign, *signature, *alloced,
    *encryptedKey, issuerSerialAlias[16];
  unsigned int inputLen, contentLen, octetPartInLen, toSignLen,
    localPartOutLen, signatureLen, encryptedKeyLen;
  UINT2 tag;
  R_RSA_PUBLIC_KEY publicKey;

  if (frame == (RIPEMDecipherPKCSFrame *)NULL)
    return ("Decipher not initialized");
  if (frame->ripemDecipherFrame.Destructor != 
      (RIPEM_DECIPHER_FRAME_DESTRUCTOR)RIPEMDecipherPKCSFrameDestructor)
    return ("Decipher frame was not initialized by RIPEMDecipherPKCSInit");

  /* Get ready to write to the output. */
  BufferStreamRewind (&frame->outStream);

  if (frame->decodeState == DS_FINISHED)
    /* Ignore input past the end of the message */
    return ((char *)NULL);

  /* Accumulate all of partIn into the input buffer. */
  if ((errorMessage = BufferStreamWrite (partIn, partInLen, &frame->input))
      != (char *)NULL)
    return (errorMessage);

  /* Use local input and inputLen to point to the bytes we are decoding. */
  input = frame->input.buffer;
  inputLen = frame->input.point;
  
  while (inputLen > 0) {
    /* If bytesNeeded is non-zero, then we must make sure we have
         accumulated at least this many before going to the case statement. */
    if (frame->bytesNeeded > 0 && inputLen < frame->bytesNeeded)
      break;

    switch (frame->decodeState) {
    case DS_START:
      if (R_memcmp
          ((POINTER)input, (POINTER)CONTENT_INFO_START,
           sizeof (CONTENT_INFO_START)) != 0)
        return ("Invalid encoding at start of message");
      ADVANCE_INPUT (sizeof (CONTENT_INFO_START));

      frame->decodeState = DS_CONTENT_TYPE;
      /* All content type identifiers are the same length, so use the
           length of one of them. */
      frame->bytesNeeded = sizeof (CONTENT_TYPE_SIGNED_DATA);
      break;

    case DS_CONTENT_TYPE:
      if ((frame->pkcsMode = DecodeContentType (input)) == 0)
        return ("Unrecognized content type");

#ifdef RIPEMSIG
      if (frame->pkcsMode & PKCS_ENVELOPED)
        return ("RIPEM/SIG cannot process ENCRYPTED messages. You may process signed messages.");
#endif

      if (frame->detached && frame->pkcsMode != PKCS_SIGNED)
        return ("The detached PKCS information is not of type SIGNED");

      /* All identifiers are the same length */
      ADVANCE_INPUT (sizeof (CONTENT_TYPE_SIGNED_DATA));

      frame->decodeState = DS_DATA_VERSION;
      frame->bytesNeeded = sizeof (DATA_VERSION);
      break;

    case DS_DATA_VERSION:
      if (R_memcmp
          ((POINTER)input, (POINTER)DATA_VERSION, sizeof (DATA_VERSION)) != 0)
        return (ERR_PKCS_ENCODING);
      ADVANCE_INPUT (sizeof (DATA_VERSION));

      frame->decodeState = DS_CHECK_DATA_VERSION;
      frame->bytesNeeded = 1;
      break;

    case DS_CHECK_DATA_VERSION:
      if (frame->pkcsMode & PKCS_SIGNED) {
        if (input[0] != 1)
          return ("Invalid version for content type");
      }
      else {
        if (input[0] != 0)
          return ("Invalid version for content type");
      }
      ADVANCE_INPUT (1);

      if (frame->pkcsMode & PKCS_ENVELOPED) {
        /* Get ready for recipientsInfos. */
        frame->decodeState = DS_RECIPIENT_INFOS_START;
        frame->bytesNeeded = sizeof (RECIPIENT_INFOS_START);
      }
      else {
        /* This is signed only */
        frame->decodeState = DS_DIGEST_ALGORITHMS_START;
        frame->bytesNeeded = sizeof (DIGEST_ALGORITHMS_START);
      }

      break;

    case DS_RECIPIENT_INFOS_START:
#ifndef RIPEMSIG
      if (R_memcmp
          ((POINTER)input, (POINTER)RECIPIENT_INFOS_START,
           sizeof (RECIPIENT_INFOS_START)) != 0)
        return (ERR_PKCS_ENCODING);
      ADVANCE_INPUT (sizeof (RECIPIENT_INFOS_START));

      frame->decodeState = DS_CHECK_RECIPIENT_INFOS_END;
      /* Get enough bytes for either END_INDEFINITE_LEN or the beginning
           of a new RecipientInfo. */
      frame->bytesNeeded = MAX_TAG_AND_LEN_BYTES;
#endif
      break;

    case DS_CHECK_RECIPIENT_INFOS_END:
#ifndef RIPEMSIG
      /* This case may get executed multiple times.  We need to see
           if we are at the end of the recipient infos yet. */
      if (R_memcmp
          ((POINTER)input, (POINTER)END_INDEFINITE_LEN,
           sizeof (END_INDEFINITE_LEN)) == 0) {
        /* We are finished with recipient infos. */
        ADVANCE_INPUT (sizeof (END_INDEFINITE_LEN));

        if (!frame->foundRecipient)
          return ("You are not listed as a recipient in this message.");

        if (frame->pkcsMode & PKCS_SIGNED) {
          frame->decodeState = DS_DIGEST_ALGORITHMS_START;
          frame->bytesNeeded = sizeof (DIGEST_ALGORITHMS_START);
        }
        else {
          /* This is enveloped only, so skip over digest algorithms. */
          frame->decodeState = DS_CONTENT_INFO_DATA_START;
          frame->bytesNeeded = sizeof (CONTENT_INFO_DATA_START);
        }
      }
      else {
        /* Find out how long this RecipientInfo is */
        saveInput = input;
        if (gettaglen (&tag, &contentLen, &input) < 0)
          return (ERR_PKCS_ENCODING);
        /* We will check the tag later */

        /* We will need the entire encoding including tag and length
             octets, so set bytesNeeded before restoring input. */
        frame->bytesNeeded = (input - saveInput) + contentLen;
        /* We don't want to update the input because we need the tag and
             length octets, so restore it. */
        input = saveInput;

        frame->decodeState = DS_RECIPIENT_INFO;
      }
#endif
      break;

    case DS_RECIPIENT_INFO:
#ifndef RIPEMSIG
      /* Only decode the RecipientInfo if we haven't found one yet. */
      if (!frame->foundRecipient) {
        /* Note that we can use frame->issuerName here, and that we will
             be done with it in time to use it for DecodeSignerInfo
             below. */
        if ((errorMessage = DecodeRecipientInfo
             (input, &frame->issuerName, frame->serialNumber,
              &encryptedKey, &encryptedKeyLen)) != (char *)NULL)
          return (errorMessage);

        /* Compute the alias and check if it is in the ripemInfo.
         */
        ComputeIssuerSerialAlias
          (issuerSerialAlias, &frame->issuerName, frame->serialNumber,
           sizeof (frame->serialNumber));
        if (IsIssuerSerialAlias (ripemInfo, issuerSerialAlias)) {
          /* This RecipientInfo is for the recipient, so copy the
               encryptedKey which will be used below in R_OpenInit.
           */
          frame->foundRecipient = TRUE;

          R_memcpy
            ((POINTER)frame->encryptedKey, (POINTER)encryptedKey,
             encryptedKeyLen);
          frame->encryptedKeyLen = encryptedKeyLen;
        }
      }

      /* bytesNeeded is still set to the total length of the RecipientInfo */
      ADVANCE_INPUT (frame->bytesNeeded);

      /* Go back to check for more recipients */
      frame->decodeState = DS_CHECK_RECIPIENT_INFOS_END;
      /* Get enough bytes for either END_INDEFINITE_LEN or the beginning
           of a new RecipientInfo. */
      frame->bytesNeeded = MAX_TAG_AND_LEN_BYTES;
#endif
      break;
      
    case DS_DIGEST_ALGORITHMS_START:
      /* Note that we are requiring that there only be zero or one digest
           algorithm. */
      if (R_memcmp
          ((POINTER)input, (POINTER)EMPTY_DIGEST_ALGORITHMS,
           sizeof (EMPTY_DIGEST_ALGORITHMS)) == 0) {
        /* There are no digest algorithms.  This is supposedly a
             certs-and-crls-only message. */
        if (frame->pkcsMode != PKCS_SIGNED)
          return ("Empty digest algorithm set only allowed in PKCS message type SignedData.");
        if (frame->detached)
          return ("Detached signature has an empty digest algorithm set.");

        ADVANCE_INPUT (sizeof (EMPTY_DIGEST_ALGORITHMS));
        /* Don't initialize the signatureContext */
        frame->digestAlgorithm = 0;

        /* Skip over DS_GET_DIGEST_ALGORITHM */
        frame->decodeState = DS_CONTENT_INFO_DATA_START;
        frame->bytesNeeded = sizeof (CONTENT_INFO_DATA_START);
      }
      else if (R_memcmp
          ((POINTER)input, (POINTER)DIGEST_ALGORITHMS_START,
           sizeof (DIGEST_ALGORITHMS_START)) == 0) {
        ADVANCE_INPUT (sizeof (DIGEST_ALGORITHMS_START));

        frame->decodeState = DS_GET_DIGEST_ALGORITHM;
        /* All digest algorithms are the same length */
        frame->bytesNeeded = sizeof (ALG_ID_MD5);
      }
      else
        return (ERR_PKCS_ENCODING);

      break;

    case DS_GET_DIGEST_ALGORITHM:
      if (R_memcmp
          ((POINTER)input, (POINTER)ALG_ID_MD2, sizeof (ALG_ID_MD2)) == 0)
        frame->digestAlgorithm = DA_MD2;
      else if (R_memcmp
          ((POINTER)input, (POINTER)ALG_ID_MD5, sizeof (ALG_ID_MD5)) == 0)
        frame->digestAlgorithm = DA_MD5;
      else
        return ("Unrecognized digest algorithm");

      ADVANCE_INPUT (sizeof (ALG_ID_MD5));

      if (frame->detached) {
        /* For detached signatures, we have already used R_VerifyInit and
             R_VerifyUpdate. Just make sure that the digest algorithm we
             used is what was expected */
        if (frame->digestAlgorithm != frame->expectedDigestAlgorithm)
          return ("Unexpected digest algorithm specified in detached PKCS signature.");
      }
      else {
        if ((status = R_VerifyInit
             (&frame->signatureContext, frame->digestAlgorithm)) != 0)
          return (FormatRSAError (status));
      }

      frame->decodeState = DS_CONTENT_INFO_DATA_START;
      frame->bytesNeeded = sizeof (CONTENT_INFO_DATA_START);
      break;

    case DS_CONTENT_INFO_DATA_START:
      if (R_memcmp
          ((POINTER)input, (POINTER)CONTENT_INFO_DATA_START,
           sizeof (CONTENT_INFO_DATA_START)) != 0) {
        if (R_memcmp
            ((POINTER)input, (POINTER)EMPTY_CONTENT_INFO_DATA,
             sizeof (EMPTY_CONTENT_INFO_DATA)) == 0) {
          /* We have a ContentInfo with definite length encoding and
               an omitted content.  Treat this the same way we treat an
               omitted content below in DS_EXPLICIT_OCTET_STRING. */
          if (frame->pkcsMode == PKCS_SIGNED &&
              (frame->digestAlgorithm == 0 || frame->detached)) {
            ADVANCE_INPUT (sizeof (EMPTY_CONTENT_INFO_DATA));

            frame->decodeState = DS_CHECK_CERTIFICATES_SET;
            frame->bytesNeeded = sizeof (CERTIFICATES_SET);
            break;
          }
          else
            return (ERR_PKCS_ENCODING);
        }
        else
          return (ERR_PKCS_ENCODING);
      }
      ADVANCE_INPUT (sizeof (CONTENT_INFO_DATA_START));

      if (frame->pkcsMode & PKCS_ENVELOPED) {
        frame->decodeState = DS_ENCRYPTION_ALGORITHM;
        /* Set bytesNeeded to the largest algorithm identifier.
           Be sure to include the 8 bytes for the IV */
        frame->bytesNeeded = sizeof (ALG_ID_DES_EDE3_CBC) + 8;
      }
      else {
        frame->decodeState = DS_EXPLICIT_OCTET_STRING;
        frame->bytesNeeded = sizeof (EXPLICIT_OCTET_STRING_START);
      }
      break;

    case DS_ENCRYPTION_ALGORITHM:
#ifndef RIPEMSIG
      if (R_memcmp
          ((POINTER)input, (POINTER)ALG_ID_DES_CBC, sizeof (ALG_ID_DES_CBC))
          == 0) {
        encryptionAlgorithm = EA_DES_CBC;
        iv = input + sizeof (ALG_ID_DES_CBC);

        /* The iv pointer will still be valid below when we use it. */
        ADVANCE_INPUT (sizeof (ALG_ID_DES_CBC) + 8);
      }
      else if (R_memcmp
          ((POINTER)input, (POINTER)ALG_ID_DES_EDE3_CBC,
           sizeof (ALG_ID_DES_EDE3_CBC)) == 0) {
        encryptionAlgorithm = EA_DES_EDE3_CBC;
        iv = input + sizeof (ALG_ID_DES_EDE3_CBC);

        /* The iv pointer will still be valid below when we use it. */
        ADVANCE_INPUT (sizeof (ALG_ID_DES_EDE3_CBC) + 8);
      }
      else
        return ("Unrecognized encryption algorithm");

      /* Decrypt the encryptedKey.  We have already made sure that
           foundRecipient is TRUE. */
      if ((status = R_OpenInit
           (&frame->envelopeContext, encryptionAlgorithm, frame->encryptedKey,
            frame->encryptedKeyLen, iv, &ripemInfo->privateKey)) != 0)
        return (FormatRSAError (status));

      frame->decodeState = DS_IMPLICIT_OCTET_STRING;
      frame->bytesNeeded = sizeof (IMPLICIT_OCTET_STRING_START);
#endif
      break;

    case DS_EXPLICIT_OCTET_STRING:
      /* We know this is only called for a PKCS_SIGNED message */

      if (R_memcmp
          ((POINTER)input, (POINTER)EXPLICIT_OCTET_STRING_START,
           sizeof (EXPLICIT_OCTET_STRING_START)) != 0) {
        if (frame->digestAlgorithm == 0 || frame->detached) {
          /* This could be a good thing.  For certs-and-CRLs only or
               detached signatures, we expect the optional content to be
               omitted. */
          frame->decodeState = DS_CONTENT_INFO_DATA_END;
          frame->bytesNeeded = sizeof (END_INDEFINITE_LEN);
        }
        else
          return (ERR_PKCS_ENCODING);
      }
      else {
        /* There is a data content */
        if (frame->digestAlgorithm == 0)
          /* This effectively ensures that there is no content if there
               are no digest algorithms (certs-and-CRLs only message).*/
          return("No digest algorithm is specified for digesting the content");
        if (frame->detached)
          return ("The detached PKCS information must not contain message content.");

        ADVANCE_INPUT (sizeof (EXPLICIT_OCTET_STRING_START));

        frame->decodeState = DS_CHECK_OCTET_STRING_END;
        frame->bytesNeeded = sizeof (END_INDEFINITE_LEN);
      }
      
      break;

    case DS_IMPLICIT_OCTET_STRING:
      /* Note that we only handle constructed octet strings for the content.
           To handle a non-constructed octet string, we
           would have to check for a different encoding here. */
      
      if (R_memcmp
          ((POINTER)input, (POINTER)IMPLICIT_OCTET_STRING_START,
           sizeof (IMPLICIT_OCTET_STRING_START)) != 0)
        return (ERR_PKCS_ENCODING);
      ADVANCE_INPUT (sizeof (IMPLICIT_OCTET_STRING_START));

      frame->decodeState = DS_CHECK_OCTET_STRING_END;
      frame->bytesNeeded = sizeof (END_INDEFINITE_LEN);
      break;

    case DS_CHECK_OCTET_STRING_END:
      /* This case may get executed multiple times.  We need to see
           if we are at the end of the octet string yet.
         Note that we only handle constructed octet strings for the content.
           To handle a non-constructed octet string, we would not check
           for END_INDEFINITE_LEN. */
      
      if (R_memcmp
          ((POINTER)input, (POINTER)END_INDEFINITE_LEN,
           sizeof (END_INDEFINITE_LEN)) == 0) {
        /* We are finished with octet string. */
        ADVANCE_INPUT (sizeof (END_INDEFINITE_LEN));

        if (!(frame->pkcsMode & PKCS_ENVELOPED)) {
          frame->decodeState = DS_END_EXPLICIT_DATA;
          frame->bytesNeeded = sizeof (END_INDEFINITE_LEN);
        }
        else {
          /* EncryptedContentInfo does not need to close out the
               content, so go straight to closing the EncryptedContentInfo,
               which is the same for closing the ContentInfo. */
          frame->decodeState = DS_CONTENT_INFO_DATA_END;
          frame->bytesNeeded = sizeof (END_INDEFINITE_LEN);
        }
      }
      else {
        /* There are more parts to the octet string, so set up to read them.
           Note that we do not advance the input. */
        frame->decodeState = DS_OCTET_STRING_PART_START;
        frame->bytesNeeded = MAX_TAG_AND_LEN_BYTES;
      }
      break;

    case DS_OCTET_STRING_PART_START:
      /* save the input since gettaglen advances it. */
      saveInput = input;

      /* Note that we only handle constructed octet strings for the content.
         If this were non-constructed and within an EncryptedContentInfo,
           then the tag would be the IMPLICIT [0]. */
      if (gettaglen (&tag, &frame->octetStringContentLen, &input) < 0 ||
          tag != BER_OCTET_STRING)
        return (ERR_PKCS_ENCODING);
      /* We have advanced input, so update inputLen */
      inputLen -= (input - saveInput);

      frame->decodeState = DS_OCTET_STRING_PART;
      /* Set to zero since we will work with whatever we get instead of
           requiring all of the octet string sub-encoding to be accumulated
           into frame->input.  This allows it to be longer than we would
           want to accumulate. */
      frame->bytesNeeded = 0;
      break;

    case DS_OCTET_STRING_PART:
      /* We want as much of the input we can get, up to the end of the
           octet string part. */
      octetPartInLen = inputLen < frame->octetStringContentLen ?
        inputLen : frame->octetStringContentLen;

#ifndef RIPEMSIG
      if (frame->pkcsMode & PKCS_ENVELOPED) {
        /* We want to decrypt to the output stream's buffer, so reserve
             enough length as specified by R_OpenUpdate. */
        if ((errorMessage = BufferStreamWrite
             ((unsigned char *)NULL, octetPartInLen + 7,
              &frame->outStream)) != (char *)NULL)
          return (errorMessage);
        /* We have to set toSign after BufferStreamWrite since if we
             set it before, BufferStreamWrite may reallocate the buffer. */
        toSign = (frame->outStream.buffer + frame->outStream.point) -
          (octetPartInLen + 7);

        if ((status = R_OpenUpdate
             (&frame->envelopeContext, toSign, &toSignLen, input,
              octetPartInLen)) != 0)
          return (FormatRSAError (status));

        /* Now, readjust the buffer length to how much R_OpenUpdate put
             out in case we allocated too much space. */
        BufferStreamUnput
          (&frame->outStream, (octetPartInLen + 7) - toSignLen);
      }
      else
#endif
      {
        /* Signed only, so just copy to the output. */
        if ((errorMessage = BufferStreamWrite
             (input, octetPartInLen, &frame->outStream)) != (char *)NULL)
          return (errorMessage);
        /* We have to set toSign after BufferStreamWrite since if we
             set it before, BufferStreamWrite may reallocate the buffer. */
        toSign = (frame->outStream.buffer + frame->outStream.point) -
          octetPartInLen;
        toSignLen = octetPartInLen;
      }

      if (frame->pkcsMode & PKCS_SIGNED) {
        /* toSign and toSignLen have been set above. */
        if ((status = R_VerifyUpdate
             (&frame->signatureContext, toSign, toSignLen)) != 0)
          return (FormatRSAError (status));
      }

      ADVANCE_INPUT (octetPartInLen);
      frame->octetStringContentLen -= octetPartInLen;
      if (frame->octetStringContentLen == 0) {
        /* We have finished processing this octet string sub-encoding,
             so see if we are done. */
        frame->decodeState = DS_CHECK_OCTET_STRING_END;
        frame->bytesNeeded = sizeof (END_INDEFINITE_LEN);
      }
      else
        /* Keep the decodeState here so we can get some more bytes */
        frame->bytesNeeded = 0;

      break;
      
    case DS_END_EXPLICIT_DATA:
      /* This is called for non-EncryptedContentInfo */
      if (R_memcmp
          ((POINTER)input, (POINTER)END_INDEFINITE_LEN,
           sizeof (END_INDEFINITE_LEN)) != 0)
        return (ERR_PKCS_ENCODING);
      ADVANCE_INPUT (sizeof (END_INDEFINITE_LEN));

      frame->decodeState = DS_CONTENT_INFO_DATA_END;
      frame->bytesNeeded = sizeof (END_INDEFINITE_LEN);
      break;

    case DS_CONTENT_INFO_DATA_END:
      if (R_memcmp
          ((POINTER)input, (POINTER)END_INDEFINITE_LEN,
           sizeof (END_INDEFINITE_LEN)) != 0)
        return (ERR_PKCS_ENCODING);
      ADVANCE_INPUT (sizeof (END_INDEFINITE_LEN));

#ifndef RIPEMSIG
      if (frame->pkcsMode & PKCS_ENVELOPED) {
        /* We need to call R_OpenFinal (and maybe R_VerifyUpdate again), so
             reserve as many bytes as it  may write out. */
        if ((errorMessage = BufferStreamWrite
             ((unsigned char *)NULL, 7, &frame->outStream)) != (char *)NULL)
          return (errorMessage);
        /* We have to set toSign after BufferStreamWrite since if we
             set it before, BufferStreamWrite may reallocate the buffer. */
        toSign = (frame->outStream.buffer + frame->outStream.point) - 7;

        if ((status = R_OpenFinal
             (&frame->envelopeContext, toSign, &toSignLen)) != 0)
          return (FormatRSAError (status));

        /* Now, readjust the buffer length to how much R_OpenFinal put
             out in case we allocated too much space. */
        BufferStreamUnput (&frame->outStream, 7 - toSignLen);

        if (frame->pkcsMode & PKCS_SIGNED) {
          if ((status = R_VerifyUpdate
               (&frame->signatureContext, toSign, toSignLen)) != 0)
            return (FormatRSAError (status));
        }
      }
#endif

      if (frame->pkcsMode & PKCS_SIGNED) {
        frame->decodeState = DS_CHECK_CERTIFICATES_SET;
        frame->bytesNeeded = sizeof (CERTIFICATES_SET);
      }
      else {
        frame->decodeState = DS_END_ENCODING;
        frame->bytesNeeded = sizeof (END_DATA_EXPLICIT_AND_CONTENT_INFO);
      }
      break;

    case DS_CHECK_CERTIFICATES_SET:
      if (R_memcmp
          ((POINTER)input, (POINTER)CERTIFICATES_SET,
           sizeof (CERTIFICATES_SET)) == 0) {
        ADVANCE_INPUT (sizeof (CERTIFICATES_SET));

        frame->decodeState = DS_CHECK_CERTIFICATES_SET_END;
        /* Get enough bytes for either END_INDEFINITE_LEN or the beginning
             of a new certificate. */
        frame->bytesNeeded = MAX_TAG_AND_LEN_BYTES;
      }
      else {
        /* This certificates set seems to be omitted, so look for CRLs */
        frame->decodeState = DS_CHECK_CRLS_SET;
        frame->bytesNeeded = sizeof (CRLS_SET);
      }
      break;

    case DS_CHECK_CERTIFICATES_SET_END:
      /* This case may get executed multiple times.  We need to see
           if we are at the end of the certificates set yet. */
      if (R_memcmp
          ((POINTER)input, (POINTER)END_INDEFINITE_LEN,
           sizeof (END_INDEFINITE_LEN)) == 0) {
        /* We are finished with certificates. */
        ADVANCE_INPUT (sizeof (END_INDEFINITE_LEN));

        if (frame->digestAlgorithm != 0) {
          /* For a non-certs-and-CRLs-only-message, we have cached the certs,
               so insert the certificates now in case there are CRLs below. */
          if ((errorMessage = InsertCerts
               (&frame->certs, ripemInfo, ripemDatabase)) != (char *)NULL)
            return (errorMessage);
        }

        frame->decodeState = DS_CHECK_CRLS_SET;
        frame->bytesNeeded = sizeof (CRLS_SET);
      }
      else {
        /* Find out how long this certificate is */
        saveInput = input;
        if (gettaglen (&tag, &contentLen, &input) < 0)
          return (ERR_PKCS_ENCODING);
        /* We will check the tag later */

        /* We will need the entire encoding including tag and length
             octets, so set bytesNeeded before restoring input. */
        frame->bytesNeeded = (input - saveInput) + contentLen;
        /* We don't want to update the input because we need the tag and
             length octets, so restore it. */
        input = saveInput;

        frame->decodeState = DS_CERTIFICATE;
      }
      break;
      
    case DS_CERTIFICATE:
      /* bytesNeeded is still set to the total length of the certificate */
      if (frame->digestAlgorithm == 0) {
        /* This is a certs-and-CRLs-only message.  There are no signers so
             there is no need to cache the certificates in frame->certs, so
             just try to insert this cert into the database.  This way, we can
             handle an arbitrarily large certs-and-CRLs-only message. */
        if ((errorMessage = InsertUniqueCert
             (input, ripemInfo, ripemDatabase)) != (char *)NULL)
          return (errorMessage);
      }
      else {
        /* We must cache the certs so that we can look for an issuer/serial
             match with the SignerInfo.  If the database could look up
             certificates by issuer/serial, then this would not be necessary.
         */
        if ((alloced = (unsigned char *)malloc (frame->bytesNeeded))
            == (unsigned char *)NULL)
          return (ERR_MALLOC);
        R_memcpy ((POINTER)alloced, (POINTER)input, frame->bytesNeeded);
        if ((errorMessage = AddToList
             ((TypListEntry *)NULL, alloced, frame->bytesNeeded,
              &frame->certs)) != (char *)NULL) {
          /* AddToList failed to incorporate the alloced data, so free it. */
          free (alloced);
          return (errorMessage);
        }
      }
      
      ADVANCE_INPUT (frame->bytesNeeded);

      /* Go back to check for more certificates */
      frame->decodeState = DS_CHECK_CERTIFICATES_SET_END;
      /* Get enough bytes for either END_INDEFINITE_LEN or the beginning
           of a new certificate. */
      frame->bytesNeeded = MAX_TAG_AND_LEN_BYTES;
      break;
      
    case DS_CHECK_CRLS_SET:
      if (R_memcmp
          ((POINTER)input, (POINTER)CRLS_SET, sizeof (CRLS_SET)) == 0) {
        ADVANCE_INPUT (sizeof (CRLS_SET));

        frame->decodeState = DS_CHECK_CRLS_SET_END;
        /* Get enough bytes for either END_INDEFINITE_LEN or the beginning
             of a new CRL. */
        frame->bytesNeeded = MAX_TAG_AND_LEN_BYTES;
      }
      else {
        /* This CRLs set seems to be omitted, so go to SignerInfos */
        frame->decodeState = DS_SIGNER_INFOS_START;
        frame->bytesNeeded = sizeof (SIGNER_INFOS_START);
      }
      break;

    case DS_CHECK_CRLS_SET_END:
      /* This case may get executed multiple times.  We need to see
           if we are at the end of the CRLs set yet. */
      if (R_memcmp
          ((POINTER)input, (POINTER)END_INDEFINITE_LEN,
           sizeof (END_INDEFINITE_LEN)) == 0) {
        /* We are finished with CRLs. */
        ADVANCE_INPUT (sizeof (END_INDEFINITE_LEN));

        frame->decodeState = DS_SIGNER_INFOS_START;
        frame->bytesNeeded = sizeof (SIGNER_INFOS_START);
      }
      else {
        /* Find out how long this CRL is */
        saveInput = input;
        if (gettaglen (&tag, &contentLen, &input) < 0)
          return (ERR_PKCS_ENCODING);
        /* We will check the tag later */

        /* We will need the entire encoding including tag and length
             octets, so set bytesNeeded before restoring input. */
        frame->bytesNeeded = (input - saveInput) + contentLen;
        /* We don't want to update the input because we need the tag and
             length octets, so restore it. */
        input = saveInput;

        frame->decodeState = DS_CRL;
      }
      break;
      
    case DS_CRL:
      /* Try to insert the CRL now instead of saving it to insert
           later.  This is helpful if the CRL is large.
       */
      if ((errorMessage = VerifyAndInsertCRL (input, ripemInfo, ripemDatabase))
          != (char *)NULL)
        return (errorMessage);

      /* bytesNeeded is still set to the total length of the CRL. */
      ADVANCE_INPUT (frame->bytesNeeded);

      /* Go back to check for more CRLs */
      frame->decodeState = DS_CHECK_CRLS_SET_END;
      /* Get enough bytes for either END_INDEFINITE_LEN or the beginning
           of a new CRL. */
      frame->bytesNeeded = MAX_TAG_AND_LEN_BYTES;
      break;
      
    case DS_SIGNER_INFOS_START:
      if (R_memcmp
          ((POINTER)input, (POINTER)SIGNER_INFOS_START,
           sizeof (SIGNER_INFOS_START)) != 0)
        return (ERR_PKCS_ENCODING);
      ADVANCE_INPUT (sizeof (SIGNER_INFOS_START));

      frame->decodeState = DS_CHECK_SIGNER_INFOS_END;
      /* Get enough bytes for either END_INDEFINITE_LEN or the beginning
           of a new SignerInfo. */
      frame->bytesNeeded = MAX_TAG_AND_LEN_BYTES;
      break;

    case DS_CHECK_SIGNER_INFOS_END:
      /* This case may get executed multiple times.  We need to see
           if we are at the end of the signer infos yet. */
      if (R_memcmp
          ((POINTER)input, (POINTER)END_INDEFINITE_LEN,
           sizeof (END_INDEFINITE_LEN)) == 0) {
        /* We are finished with signer infos. */
        ADVANCE_INPUT (sizeof (END_INDEFINITE_LEN));

        frame->decodeState = DS_END_ENCODING;
        frame->bytesNeeded = sizeof (END_DATA_EXPLICIT_AND_CONTENT_INFO);
      }
      else {
        if (frame->digestAlgorithm == 0)
          /* This should be a certs-and-CRLs-only message. */
          return ("A SignerInfo is supplied but there is no message content to be verified");

        if (frame->foundSigner)
          /* We already have one signer and we do not support multiples. */
          return ("Too many SignerInfos in message");

        /* Find out how long this SignerInfo is */
        saveInput = input;
        if (gettaglen (&tag, &contentLen, &input) < 0)
          return (ERR_PKCS_ENCODING);
        /* We will check the tag later */

        /* We will need the entire encoding including tag and length
             octets, so set bytesNeeded before restoring input. */
        frame->bytesNeeded = (input - saveInput) + contentLen;
        /* We don't want to update the input because we need the tag and
             length octets, so restore it. */
        input = saveInput;

        frame->decodeState = DS_SIGNER_INFO;
      }
      break;

    case DS_SIGNER_INFO:
      frame->foundSigner = TRUE;
      
      if ((errorMessage = DecodeSignerInfo
           (input, &frame->issuerName, frame->serialNumber,
            frame->digestAlgorithm, &frame->signatureContext, frame->pkcsMode,
            &signature, &signatureLen, &frame->authenticatedAttributes,
            &frame->attributesBuffers))
          != (char *)NULL)
        return (errorMessage);

      /* This also sets the certChain */
      if ((errorMessage = GetSignerPublicKey
           (frame, ripemInfo, &publicKey, ripemDatabase)) != (char *)NULL)
        return (errorMessage);

      /* If the chain status is zero, we couldn't find a public key.
           Perhaps there is a self-signed cert to validate, so copying the
           certChain later is important, but don't try to validate the message
           signature. */
      if (frame->chainStatus.overall != 0) {
#ifndef RIPEMSIG
        if (frame->pkcsMode & PKCS_ENVELOPED) {
          /* Must decrypt the signature.  Use the buffer in the frame for
               the result.
             The envelopeContext has already been finalized, so we can
               use it to decrypt again. */
          if ((status = R_OpenUpdate
               (&frame->envelopeContext, frame->signature, &signatureLen,
                signature, signatureLen)) != 0)
            return (FormatRSAError (status));
          if ((status = R_OpenFinal
               (&frame->envelopeContext, frame->signature + signatureLen,
                &localPartOutLen)) != 0)
            return (FormatRSAError (status));
          signatureLen += localPartOutLen;

          /* The result is now in the buffer, so point there. */
          signature = frame->signature;
        }
#endif

        if ((status = R_VerifyFinal
             (&frame->signatureContext, signature, signatureLen,
              &publicKey)) != 0)
          /* This will return a bad signature error if there is one */
          return (FormatRSAError (status));
      }

      /* bytesNeeded is still set to the total length of the SignerInfo */
      ADVANCE_INPUT (frame->bytesNeeded);

      /* Go back to check for more signers */
      frame->decodeState = DS_CHECK_SIGNER_INFOS_END;
      /* Get enough bytes for either END_INDEFINITE_LEN or the beginning
           of a new SignerInfo. */
      frame->bytesNeeded = MAX_TAG_AND_LEN_BYTES;
      break;
      
    case DS_END_ENCODING:
      if (R_memcmp
          ((POINTER)input, (POINTER)END_DATA_EXPLICIT_AND_CONTENT_INFO,
           sizeof (END_DATA_EXPLICIT_AND_CONTENT_INFO)) != 0)
        return ("Invalid encoding at closing bytes of message");
      ADVANCE_INPUT (sizeof (END_DATA_EXPLICIT_AND_CONTENT_INFO));

      frame->decodeState = DS_FINISHED;
      frame->bytesNeeded = 0;
      break;

    default:
      /* This should never happen since we define the state. If it does, either
           we forgot to process a state, or the decodeState is corrupt. */
      return ("Internal error: invalid decode state");
    }
  }

  /* We have processed as much of the input as possible, so shift any
       remaining bytes to the front to get ready for the next Update. */
  BufferStreamFlushBytes (&frame->input, input - frame->input.buffer);

  /* Set the output */
  *partOut = frame->outStream.buffer;
  *partOutLen = frame->outStream.point;

  return ((char *)NULL);
}

/* Call this after the entire enhanced message has been supplied to
     RIPEMDecipherPKCSUpdate.
   Return the sender's certChain and chainStatus. The calling routine must
     InitList certChain.  chainStatus is pointer to a ChainStatusInfo struct.
   If chainStatus->overall is 0, this could not find a valid public key for
     the sender and pkcsMode is undefined. In this case, if the message
     contained a self-signed certificate, the certChain contains one entry
     which is the self-signed cert.  The calling routine may decode it and
     present the self-signed cert digest, and use ValidateAndWriteCert to
     validate the user.
   For other values of chainStatus->overall, certChain and chainStatus
     contains values as described in SelectCertChain.  Note: the sender
     name is the subject of the cert at the "bottom" of the chain.
     Unlike RIPEMDecipherFinal (for PEM), this does not support a chain
     status of CERT_UNVALIDATED since PKCS identifies senders and recipients
     via certificates.
   pkcsMode is set to PKCS_SIGNED, PKCS_ENVELOPED,
     PKCS_SIGNED|PKCS_ENVELOPED, or PKCS_CERTS_AND_CRLS_ONLY.  If pkcsMode
     is set to PKCS_CERTS_AND_CRLS_ONLY or PKCS_ENVELOPED, then certChain is
     unmodified since there are no senders, and chainStatus->overall
     is set to zero.
   If authenticatedAttributes is not NULL, then it points to a
     RIPEMAttributes which receives the authenticated attributes in the
     message if any.  The "have" flag is set if the attributes is present.
     For example, if there was a signing time, haveSigningTime is set TRUE
     and signingTime contains the result.  The memory for the buffers such
     as signingDescription is allocated inside ripemInfo
     and should be treated as "read only".  Upon return, the value must be
     copied before a future call to RIPEM since RIPEM may modify it.
     authenticatedAttributes may be NULL, in which case it is ignored.
   Return NULL for success or error string.
 */
char *RIPEMDecipherPKCSFinal
  (ripemInfo, certChain, chainStatus, pkcsMode, authenticatedAttributes)
RIPEMInfo *ripemInfo;
TypList *certChain;
ChainStatusInfo *chainStatus;
int *pkcsMode;
RIPEMAttributes *authenticatedAttributes;
{
  RIPEMDecipherPKCSFrame *frame =
    (RIPEMDecipherPKCSFrame *)ripemInfo->z.decipherFrame;

  if (frame == (RIPEMDecipherPKCSFrame *)NULL)
    return ("Decipher not initialized");
  if (frame->ripemDecipherFrame.Destructor != 
      (RIPEM_DECIPHER_FRAME_DESTRUCTOR)RIPEMDecipherPKCSFrameDestructor) 
    return ("Decipher frame was not initialized by RIPEMDecipherPKCSInit");

  if (frame->decodeState != DS_FINISHED)
    return ("End of input was signalled before reading all of the message");

  if (frame->pkcsMode == PKCS_SIGNED && frame->digestAlgorithm == 0) {
    /* Certs-and-CRLs-only message */
    *pkcsMode = PKCS_CERTS_AND_CRLS_ONLY;
    chainStatus->overall = 0;
    return ((char *)NULL);
  }

  *pkcsMode = frame->pkcsMode;

  if (frame->pkcsMode == PKCS_ENVELOPED) {
    /* There is no sender. */
    chainStatus->overall = 0;
    return ((char *)NULL);
  }
    
  /* Now return the sender's info to the caller.  Copy the chain's TypList
       which will transfer all the pointers to the caller.  Then re-initialize
       the frame's TypList so that the destructor won't try to free the memory.
   */
  *certChain = frame->certChain;
  InitList (&frame->certChain);
  /* This copies the entire ChainStatusInfo. */
  *chainStatus = frame->chainStatus;

  if (authenticatedAttributes != (RIPEMAttributes *)NULL)
    *authenticatedAttributes = frame->authenticatedAttributes;

  return ((char *)NULL);
}

/* Initialize for preparing a PKCS detached signature.
   The calling routine must already have called RIPEMLoginUser.
   After this, call RIPEMSignDetachedPKCSDigestUpdate to digest the text by
     parts, and call RIPEMSignDetachedPKCSFinal to finish.
   Return NULL for success or error string.
 */
char *RIPEMSignDetachedPKCSInit (ripemInfo)
RIPEMInfo *ripemInfo;
{
  RIPEMEncipherPKCSFrame *frame;
  int status;
  char *errorMessage = (char *)NULL;
  
  /* For error, break to end of do while (0) block. */
  do {
    /* Make sure any old frame is deleted and make a new one.
     */
    if (ripemInfo->z.encipherFrame != (RIPEMEncipherFrame *)NULL) {
      /* Be sure to call the "virtual" destructor */
      (*ripemInfo->z.encipherFrame->Destructor) (ripemInfo->z.encipherFrame);
      free (ripemInfo->z.encipherFrame);
    }
    /* Be sure to malloc for the size of an entire RIPEMEncipherPKCSFrame */
    if ((ripemInfo->z.encipherFrame = (RIPEMEncipherFrame *)malloc
         (sizeof (*frame))) == (RIPEMEncipherFrame *)NULL) {
      errorMessage = ERR_MALLOC;
      break;
    }
    RIPEMEncipherPKCSFrameConstructor
      ((RIPEMEncipherPKCSFrame *)ripemInfo->z.encipherFrame);

    frame = (RIPEMEncipherPKCSFrame *)ripemInfo->z.encipherFrame;
    frame->pkcsMode = PKCS_SIGNED;

    /* Initialize signature.  Always use MD5 */
    if ((status = R_SignInit (&frame->signatureContext, DA_MD5)) != 0) {
      errorMessage = FormatRSAError (status);
      break;
    }
  } while (0);
  
  return (errorMessage);
}

/* Before this is called for the first time, the caller should have called
     RIPEMSignDetachedPKCSInit.  This is repeatedly called to digest the
     data to sign.
   The data to digest is in partIn with length partInLen.
   The input is "as is": no translation of '\n' to <CR><LF> is
     done because this message format can support binary data.  The
     calling routine must do end-of-line character translation if necessary.
   After this, call RIPEMSignDetachedPKCSFinal to finalize.  (There is
     no RIPEMSignDetachedPKCSUpdate function since RIPEMSignDetachedPKCSFinal
     produces the entire output.)
   Return NULL for success or error string.
 */
char *RIPEMSignDetachedPKCSDigestUpdate (ripemInfo, partIn, partInLen)
RIPEMInfo *ripemInfo;
unsigned char *partIn;
unsigned int partInLen;
{
  RIPEMEncipherPKCSFrame *frame =
    (RIPEMEncipherPKCSFrame *)ripemInfo->z.encipherFrame;
  int status;

  if (frame == (RIPEMEncipherPKCSFrame *)NULL)
    return ("Sign detached not initialized");
  if (frame->ripemEncipherFrame.Destructor != 
      (RIPEM_ENCIPHER_FRAME_DESTRUCTOR)RIPEMEncipherPKCSFrameDestructor)
    return ("Signing frame was not initialized by RIPEMSignDetachedPKCSInit");

  if ((status = R_SignUpdate
       (&frame->signatureContext, partIn, partInLen)) != 0)
    return (FormatRSAError (status));

  return ((char *)NULL);
}

/* Call this after all text has been digested
     RIPEMSignDetachedPKCSDigestUpdate. This writes the entire PKCS detached
     signature data.
   This returns a pointer to the output in partOut and its length in
     partOutLen.  The memory for the output is allocated inside ripemInfo
     and should be treated as "read only".  Upon return, the output must be
     copied or written to a file since future calls to RIPEM may modify it. On
     error return, the pointer to the output is undefined.
   The output is "as is": no translation of '\n' to <CR><LF> is
     done because this message format can support binary data.
   authenticatedAttributes is for future compatibility.  You should pass
     (RIPEMAttributes *)NULL.
   Return NULL for success or error string.
 */
char *RIPEMSignDetachedPKCSFinal
  (ripemInfo, partOut, partOutLen, authenticatedAttributes)
RIPEMInfo *ripemInfo;
unsigned char **partOut;
unsigned int *partOutLen;
RIPEMAttributes *authenticatedAttributes;
{
  RIPEMEncipherPKCSFrame *frame =
    (RIPEMEncipherPKCSFrame *)ripemInfo->z.encipherFrame;
  BufferStream *stream;
  char *errorMessage;

UNUSED_ARG (authenticatedAttributes)

  if (frame == (RIPEMEncipherPKCSFrame *)NULL)
    return ("Encipher not initialized");
  if (frame->ripemEncipherFrame.Destructor != 
      (RIPEM_ENCIPHER_FRAME_DESTRUCTOR)RIPEMEncipherPKCSFrameDestructor)
    return ("Encipher frame was not initialized by RIPEMSignDetachedPKCSInit");

  /* Get stream for quick access. */
  stream = &frame->outStream;

  /* Get ready to write to the output. */
  BufferStreamRewind (stream);

  /* Write the PKCS #7 ContentInfo to the output stream.
   */
  if ((errorMessage = BufferStreamWrite
       (CONTENT_INFO_START, sizeof (CONTENT_INFO_START), stream))
      != (char *)NULL)
    return (errorMessage);

  if ((errorMessage = BufferStreamWrite
       (CONTENT_TYPE_SIGNED_DATA, sizeof (CONTENT_TYPE_SIGNED_DATA),
        stream)) != (char *)NULL)
    return (errorMessage);

  if ((errorMessage = BufferStreamWrite
       (DATA_VERSION, sizeof (DATA_VERSION), stream)) != (char *)NULL)
    return (errorMessage);

  /* Must explicitly put the version byte. */
  /* SignedData and SigneAndEnvelopedData have version 1 */
  if ((errorMessage = BufferStreamPutc (1, stream)) != (char *)NULL)
    return (errorMessage);

  /* Always output MD5 */
  if ((errorMessage = BufferStreamWrite
       (DIGEST_ALGORITHMS_START, sizeof (DIGEST_ALGORITHMS_START), stream))
      != (char *)NULL)
    return (errorMessage);
  if ((errorMessage = BufferStreamWrite
       (ALG_ID_MD5, sizeof (ALG_ID_MD5), stream)) != (char *)NULL)
    return (errorMessage);

  if ((errorMessage = BufferStreamWrite
       (CONTENT_INFO_DATA_START, sizeof (CONTENT_INFO_DATA_START), stream))
      != (char *)NULL)
    return (errorMessage);

  /* Omit the content for detached signature. */

  /* Close out the ContentInfo. */
  if ((errorMessage = BufferStreamWrite
       (END_INDEFINITE_LEN, sizeof (END_INDEFINITE_LEN), stream))
      != (char *)NULL)
    return (errorMessage);

  if ((errorMessage = WriteCertsAndSigner (ripemInfo)) != (char *)NULL)
    return (errorMessage);

  if ((errorMessage = BufferStreamWrite
       (END_DATA_EXPLICIT_AND_CONTENT_INFO,
        sizeof (END_DATA_EXPLICIT_AND_CONTENT_INFO), stream))
      != (char *)NULL)
    return (errorMessage);

  /* Set the output.  Include the offset that may have been set
       while encoding the last octet string of the data content. */
  *partOut = stream->buffer;
  *partOutLen = stream->point;
  
  return ((char *)NULL);
}

/* Initialize ripemInfo for verifying a PKCS detached signature using
     the given digest algorithm.
   The digestAlgorithm is used by RIPEMVerifyDetachedDigestUpdate to digest
     the message and is also compared with the digest algorithm specified in
     the detached signature information.  digestAlgorithm should be one of
     the values defined by RSAREF such as DA_MD2 or DA_MD5.
   After this, call RIPEMVerifyDetachedPKCSDigestUpdate to digest the message
     text by parts, call RIPEMVerifyDetachedPKCSUpdate to supply the PKCS
     detached signature by parts and call RIPEMVerifyDetachedPKCSFinal to
     finish and obtain the sender information.
   Return NULL for success or error string.
 */
char *RIPEMVerifyDetachedPKCSInit (ripemInfo, digestAlgorithm)
RIPEMInfo *ripemInfo;
int digestAlgorithm;
{
  RIPEMDecipherPKCSFrame *frame;
  char *errorMessage;
  int status;

  if ((errorMessage = RIPEMDecipherPKCSInit (ripemInfo)) != (char *)NULL)
    return (errorMessage);

  /* RIPEMDecipherPKCSInit has created the frame. */
  frame = (RIPEMDecipherPKCSFrame *)ripemInfo->z.decipherFrame;
  frame->detached = 1;
  frame->expectedDigestAlgorithm = digestAlgorithm;

  if ((status = R_VerifyInit
       (&frame->signatureContext, digestAlgorithm)) != 0)
    return (FormatRSAError (status));

  return ((char *)NULL);
}

/* Call this after the verify operation has been initialized with
     RIPEMVerifyDetachedPKCSInit.
   This digests the message data in partIn of length partInLen.
     partIn may contain any number of bytes.  This may
     be called multiple times.
   The input is "as is": no translation of '\n' to <CR><LF> is
     done because this message format can support binary data.  The
     calling routine must do end-of-line character translation if necessary.
   After calling this to digest the message, call RIPEMVerifyDetachedPKCSUpdate
     to supply the detached signature and RIPEMVerifyDetachedPKCSFinal to
     finalize.
   Return NULL for success or error string.
 */
char *RIPEMVerifyDetachedPKCSDigestUpdate (ripemInfo, partIn, partInLen)
RIPEMInfo *ripemInfo;
unsigned char *partIn;
unsigned int partInLen;
{
  RIPEMDecipherPKCSFrame *frame =
    (RIPEMDecipherPKCSFrame *)ripemInfo->z.decipherFrame;
  int status;
  
  if (frame == (RIPEMDecipherPKCSFrame *)NULL)
    return ("Decipher not initialized");
  if (frame->ripemDecipherFrame.Destructor != 
      (RIPEM_DECIPHER_FRAME_DESTRUCTOR)RIPEMDecipherPKCSFrameDestructor)
   return("Decipher frame was not initialized by RIPEMVerifyDetachedPKCSInit");

  if ((status = R_VerifyUpdate
       (&frame->signatureContext, partIn, partInLen)) != 0)
    return (FormatRSAError (status));

  return ((char *)NULL);
}

/* Call this after the verify operation has been initialized with
     RIPEMVerifyDetachedPKCSInit and the message has been digested with
     RIPEMVerifyDetachedPKCSDigestUpdate.
   This decodes the PKCS detached signature information in partIn of length
     partInLen.  partIn may contain any number of bytes.  This may
     be called multiple times.
   The input is "as is": no translation of '\n' to <CR><LF> is
     done because this message format can support binary data.
   After calling this to supply the detached signature, call
     RIPEMVerifyDetachedPKCSFinal to finalize.
   Return NULL for success or error string.
 */
char *RIPEMVerifyDetachedPKCSUpdate
  (ripemInfo, partIn, partInLen, ripemDatabase)
RIPEMInfo *ripemInfo;
unsigned char *partIn;
unsigned int partInLen;
RIPEMDatabase *ripemDatabase;
{
  RIPEMDecipherPKCSFrame *frame =
    (RIPEMDecipherPKCSFrame *)ripemInfo->z.decipherFrame;
  unsigned char *dummyPartOut;
  unsigned int dummyPartOutLen;

  if (frame == (RIPEMDecipherPKCSFrame *)NULL)
    return ("Decipher not initialized");
  if (frame->ripemDecipherFrame.Destructor != 
      (RIPEM_DECIPHER_FRAME_DESTRUCTOR)RIPEMDecipherPKCSFrameDestructor)
   return("Decipher frame was not initialized by RIPEMVerifyDetachedPKCSInit");

  /* The is no output for a detached signature.
     Assume RIPEMVerifyDetachedInit has already set frame-detached. */
  return (RIPEMDecipherPKCSUpdate
          (ripemInfo, &dummyPartOut, &dummyPartOutLen, partIn, partInLen,
           ripemDatabase));
}

/* Call this after the entire detached signature has been supplied to
     RIPEMVerifyDetachedPKCSUpdate.
   See RIPEMDecipherPKCSFinal for the meaning of certChain, chainStatus
     and authenticatedAttributes.  The PKCS mode is not returned because
     it can only be PKCS_SIGNED.
   Return NULL for success or error string.
 */
char *RIPEMVerifyDetachedPKCSFinal
  (ripemInfo, certChain, chainStatus, authenticatedAttributes)
RIPEMInfo *ripemInfo;
TypList *certChain;
ChainStatusInfo *chainStatus;
RIPEMAttributes *authenticatedAttributes;
{
  RIPEMDecipherPKCSFrame *frame =
    (RIPEMDecipherPKCSFrame *)ripemInfo->z.decipherFrame;
  int dummyPKCSMode;

  if (frame == (RIPEMDecipherPKCSFrame *)NULL)
    return ("Decipher not initialized");
  if (frame->ripemDecipherFrame.Destructor != 
      (RIPEM_DECIPHER_FRAME_DESTRUCTOR)RIPEMDecipherPKCSFrameDestructor)
   return("Decipher frame was not initialized by RIPEMVerifyDetachedPKCSInit");

  /* PKCS mode can only be PKCS_SIGNED */
  return (RIPEMDecipherPKCSFinal
          (ripemInfo, certChain, chainStatus, &dummyPKCSMode,
           authenticatedAttributes));
}

/* Preset all "have attributes" to FALSE.
 */
void InitRIPEMAttributes (attributes)
RIPEMAttributes *attributes;
{
  R_memset ((POINTER)attributes, 0, sizeof (*attributes));
}

/* Produce a PKCS #10 certification request for the user in ripemInfo.
   If attributes->haveChallengePassword is TRUE, then
     attributes->challengePassowrd is a null-terminated string to use as
     the challenge password.  The caller is responsible for allocating the
     memory that attributes->challengePassowrd points to.  Other attributes
     in the RIPEMAttributes are ignored.
   attributes may be (RIPEMAttributes *)NULL, in which case no attributes
     are used.
   This returns a pointer to the output in output and its length in
     outputLen.  The memory for the output is allocated inside ripemInfo
     and should be treated as "read only".  Upon return, the output must be
     copied or written to a file since future calls to RIPEM may modify it. On
     error return, the pointer to the output is undefined.
 */
char *RIPEMCertifyRequestPKCS (ripemInfo, output, outputLen, attributes)
RIPEMInfo *ripemInfo;
unsigned char **output;
unsigned int *outputLen;
RIPEMAttributes *attributes;
{
  RIPEMEncipherPKCSFrame *frame;
  BufferStream *stream;
  char *errorMessage;
  unsigned char signature[MAX_SIGNATURE_LEN], buffer[MAX_TAG_AND_LEN_BYTES],
    *p;
  unsigned int signatureLen;
  
  /* Make sure any old frame is deleted and make a new one.  Use
       the encipher frame.
   */
  if (ripemInfo->z.encipherFrame != (RIPEMEncipherFrame *)NULL) {
    /* Be sure to call the "virtual" destructor */
    (*ripemInfo->z.encipherFrame->Destructor) (ripemInfo->z.encipherFrame);
    free (ripemInfo->z.encipherFrame);
  }
  /* Be sure to malloc for the size of an entire RIPEMEncipherPKCSFrame */
  if ((ripemInfo->z.encipherFrame = (RIPEMEncipherFrame *)malloc
       (sizeof (*frame))) == (RIPEMEncipherFrame *)NULL)
    return (ERR_MALLOC);
  RIPEMEncipherPKCSFrameConstructor
    ((RIPEMEncipherPKCSFrame *)ripemInfo->z.encipherFrame);

  /* Get stream for quick access. */
  frame = (RIPEMEncipherPKCSFrame *)ripemInfo->z.encipherFrame;
  stream = &frame->outStream;

  /* Get ready to write to the output. */
  BufferStreamRewind (stream);

  /* Reserve space for the tag and length of the outer encoding. */
  if ((errorMessage = BufferStreamWrite
       ((unsigned char *)NULL, MAX_TAG_AND_LEN_BYTES, stream))
      != (char *)NULL)
    return (errorMessage);

  if ((errorMessage = WriteCertifyRequestInfo
       (stream, &ripemInfo->userDN, &ripemInfo->publicKey, attributes))
      != (char *)NULL)
    return (errorMessage);

  /* Now sign it.  The encoding is offset by MAX_TAG_AND_LEN_BYTES */
  R_SignBlock
    (signature, &signatureLen, stream->buffer + MAX_TAG_AND_LEN_BYTES,
     stream->point - MAX_TAG_AND_LEN_BYTES, DA_MD5, &ripemInfo->privateKey);

  /* We can now encode the algorithm ID and the signature as bit string,
       which includes the unused bits octet.
   */
  if ((errorMessage = BufferStreamWrite
       (ALG_ID_MD5_WITH_RSA_ENCRYPTION,
        sizeof (ALG_ID_MD5_WITH_RSA_ENCRYPTION), stream)) != (char *)NULL)
    return (errorMessage);
  p = buffer;
  *p++ = BER_BIT_STRING;
  put_der_len (&p, signatureLen + 1);
  if ((errorMessage = BufferStreamWrite (buffer, p - buffer, stream))
      != (char *)NULL)
    return (errorMessage);
  if ((errorMessage = BufferStreamPutc (0, stream))
      != (char *)NULL)
    return (errorMessage);
  if ((errorMessage = BufferStreamWrite (signature, signatureLen, stream))
      != (char *)NULL)
    return (errorMessage);

  /* Now we can insert the tag and length octets for the entire encoding.
   */
  p = buffer;
  *p++ = BER_SEQUENCE;
  put_der_len (&p, stream->point - MAX_TAG_AND_LEN_BYTES);

  /* This is where the final result will be. */
  *output = stream->buffer + MAX_TAG_AND_LEN_BYTES - (p - buffer);
  R_memcpy ((POINTER)*output, buffer, p - buffer);
  *outputLen = stream->point - ((*output) - stream->buffer);

  return ((char *)NULL);
}

/* Initializes a certs-and-crls-only message for outputting the CRL and
     certificates for the user in ripemInfo.
   This returns a pointer to the output in partOut and its length in
     PartOutLen.  The memory for the output is allocated inside ripemInfo
     and should be treated as "read only".  Upon return, the output must be
     copied or written to a file since future calls to RIPEM may modify it. On
     error return, the pointer to the output is undefined.
   After this, call RIPEMCertsAndCRL_PKCSUpdate to include extra certs in the
     output, and call RIPEMCertsAndCRL_PKCSFinal to output the CRL and finish.
   Returns NULL for success, otherwise error string.
 */
char *RIPEMCertsAndCRL_PKCSInit (ripemInfo, partOut, partOutLen)
RIPEMInfo *ripemInfo;
unsigned char **partOut;
unsigned int *partOutLen;
{
  RIPEMEncipherPKCSFrame *frame;
  BufferStream *stream;
  char *errorMessage;

  /* Make sure any old frame is deleted and make a new one.  Use
       the encipher frame.
   */
  if (ripemInfo->z.encipherFrame != (RIPEMEncipherFrame *)NULL) {
    /* Be sure to call the "virtual" destructor */
    (*ripemInfo->z.encipherFrame->Destructor) (ripemInfo->z.encipherFrame);
    free (ripemInfo->z.encipherFrame);
  }
  /* Be sure to malloc for the size of an entire RIPEMEncipherPKCSFrame */
  if ((ripemInfo->z.encipherFrame = (RIPEMEncipherFrame *)malloc
       (sizeof (*frame))) == (RIPEMEncipherFrame *)NULL)
    return (ERR_MALLOC);
  RIPEMEncipherPKCSFrameConstructor
    ((RIPEMEncipherPKCSFrame *)ripemInfo->z.encipherFrame);

  /* Get stream for quick access. */
  frame = (RIPEMEncipherPKCSFrame *)ripemInfo->z.encipherFrame;
  stream = &frame->outStream;

  /* Get ready to write to the output. */
  BufferStreamRewind (stream);

  /* Write the PKCS #7 ContentInfo to the output stream.
   */
  if ((errorMessage = BufferStreamWrite
       (CONTENT_INFO_START, sizeof (CONTENT_INFO_START), stream))
      != (char *)NULL)
    return (errorMessage);

  if ((errorMessage = BufferStreamWrite
       (CONTENT_TYPE_SIGNED_DATA, sizeof (CONTENT_TYPE_SIGNED_DATA),
        stream)) != (char *)NULL)
    return (errorMessage);

  if ((errorMessage = BufferStreamWrite
       (DATA_VERSION, sizeof (DATA_VERSION), stream)) != (char *)NULL)
    return (errorMessage);
  /* Must explicitly put the version byte. */
  if ((errorMessage = BufferStreamPutc (1, stream)) != (char *)NULL)
    return (errorMessage);

  if ((errorMessage = BufferStreamWrite
       (EMPTY_DIGEST_ALGORITHMS, sizeof (EMPTY_DIGEST_ALGORITHMS), stream))
      != (char *)NULL)
    return (errorMessage);

  if ((errorMessage = BufferStreamWrite
       (CONTENT_INFO_DATA_START, sizeof (CONTENT_INFO_DATA_START), stream))
      != (char *)NULL)
    return (errorMessage);
  /* Omit the content for certs-and-crls-only message. */
  if ((errorMessage = BufferStreamWrite
       (END_INDEFINITE_LEN, sizeof (END_INDEFINITE_LEN), stream))
      != (char *)NULL)
    return (errorMessage);

  /* Get ready to output certificates.
   */
  if ((errorMessage = BufferStreamWrite
       (CERTIFICATES_SET, sizeof (CERTIFICATES_SET), stream))
      != (char *)NULL)
    return (errorMessage);

  /* Set the output. */
  *partOut = frame->outStream.buffer;
  *partOutLen = frame->outStream.point;

  return ((char *)NULL);
}

/* Call this after RIPEMCertsAndCRL_PKCSInit.  This may be called zero or more
     times to add extra certificates to the certs-and-crls-only message.  This
     is a way to export certificates for other user's to import into their
     database.  Note that this can be called an arbitrarily large number of
     times (especially using the RIPEMDatabaseCursor) without overrunning
     memory.
   See RIPEMCertsAndCRL_PKCSInit for a description of partOut and partOutLen.
   Return NULL for success or error string.
 */
char *RIPEMCertsAndCRL_PKCSUpdate (ripemInfo, partOut, partOutLen, certs)
RIPEMInfo *ripemInfo;
unsigned char **partOut;
unsigned int *partOutLen;
TypList *certs;
{
  TypListEntry *entry;
  RIPEMEncipherPKCSFrame *frame =
    (RIPEMEncipherPKCSFrame *)ripemInfo->z.encipherFrame;
  BufferStream *stream;
  char *errorMessage;
  
  if (frame == (RIPEMEncipherPKCSFrame *)NULL)
    return ("Not initialized by RIPEMCertsAndCRL_PKCSInit");
  if (frame->ripemEncipherFrame.Destructor != 
      (RIPEM_ENCIPHER_FRAME_DESTRUCTOR)RIPEMEncipherPKCSFrameDestructor)
    return ("Encoding frame was not initialized by RIPEMCertsAndCRL_PKCSInit");

  /* Get stream for quick access. */
  stream = &frame->outStream;

  /* Get ready to write to the output. */
  BufferStreamRewind (stream);

  for (entry = certs->firstptr; entry; entry = entry->nextptr) {
    if ((errorMessage = BufferStreamWrite
         ((unsigned char *)entry->dataptr, entry->datalen, stream))
        != (char *)NULL)
      return (errorMessage);  
  }

  /* Set the output. */
  *partOut = stream->buffer;
  *partOutLen = stream->point;

  return ((char *)NULL);
}

/* Call this after RIPEMCertsAndCRL_PKCSInit and zero or more calls to
     RIPEMCertsAndCRL_PKCSUpdate.
   If includeSenderCerts is true, this includes the self-
     signed certificate and issuer certificates for the logged-in user.
     If includeCRL is TRUE, this also adds the CRL for
     the user in ripemInfo by getting it from ripemDatabase.
     (If neither includeSenderCerts or includeCRL is true, the only use
      of this message is if certs were added with RIPEMCertsAndCRL_PKCSUpdate.)
   If adding the CRL, this returns an error if the CRL cannot be found or the
     signature is corrupt.  Otherwise, if the CRL is expired, it is still used.
   See RIPEMCertsAndCRL_PKCSInit for a description of partOut and partOutLen.
   Return NULL for success or error string.
 */
char *RIPEMCertsAndCRL_PKCSFinal
  (ripemInfo, partOut, partOutLen, includeSenderCerts, includeCRL,
   ripemDatabase)
RIPEMInfo *ripemInfo;
unsigned char **partOut;
unsigned int *partOutLen;
BOOL includeSenderCerts;
BOOL includeCRL;
RIPEMDatabase *ripemDatabase;
{
  RIPEMEncipherPKCSFrame *frame =
    (RIPEMEncipherPKCSFrame *)ripemInfo->z.encipherFrame;
  BufferStream *stream;
  char *errorMessage = (char *)NULL;
  unsigned char *crlDER = (unsigned char *)NULL;
  int crlLen;
  TypListEntry *entry;

  if (frame == (RIPEMEncipherPKCSFrame *)NULL)
    return ("Not initialized by RIPEMCertsAndCRL_PKCSInit");
  if (frame->ripemEncipherFrame.Destructor != 
      (RIPEM_ENCIPHER_FRAME_DESTRUCTOR)RIPEMEncipherPKCSFrameDestructor)
    return ("Encoding frame was not initialized by RIPEMCertsAndCRL_PKCSInit");

  /* Get stream for quick access. */
  stream = &frame->outStream;

  /* Get ready to write to the output. */
  BufferStreamRewind (stream);

  /* For error, break to end of do while (0) block. */
  do {
    if (includeSenderCerts) {
      /* Output the self-signed certificate and issuer certs. */
      if ((errorMessage = BufferStreamWrite
           (ripemInfo->z.userCertDER, ripemInfo->z.userCertDERLen, stream))
          != (char *)NULL)
        break;
      for (entry = ripemInfo->issuerCerts.firstptr; entry;
           entry = entry->nextptr) {
        if ((errorMessage = BufferStreamWrite
             ((unsigned char *)entry->dataptr, entry->datalen, stream))
            != (char *)NULL)
          break;
      }
    }

    /* End the certificates set. */
    if ((errorMessage = BufferStreamWrite
         (END_INDEFINITE_LEN, sizeof (END_INDEFINITE_LEN), stream))
        != (char *)NULL)
      break;

    if (includeCRL) {
      /* Output a set containing the user's CRL.
       */      
      if ((errorMessage = GetLoggedInLatestCRL
           (&crlDER, &crlLen, ripemInfo, ripemDatabase)) != (char *)NULL)
        break;

      if ((errorMessage = BufferStreamWrite
           (CRLS_SET, sizeof (CRLS_SET), stream)) != (char *)NULL)
        break;
      if ((errorMessage = BufferStreamWrite (crlDER, crlLen, stream))
          != (char *)NULL)
        break;
      if ((errorMessage = BufferStreamWrite
           (END_INDEFINITE_LEN, sizeof (END_INDEFINITE_LEN), stream))
          != (char *)NULL)
        break;
    }

    /* Output an empty set of set of signerInfos.
     */
    if ((errorMessage = BufferStreamWrite
         (SIGNER_INFOS_START, sizeof (SIGNER_INFOS_START), stream))
        != (char *)NULL)
      break;
    if ((errorMessage = BufferStreamWrite
         (END_INDEFINITE_LEN, sizeof (END_INDEFINITE_LEN), stream))
        != (char *)NULL)
      break;

    if ((errorMessage = BufferStreamWrite
         (END_DATA_EXPLICIT_AND_CONTENT_INFO,
          sizeof (END_DATA_EXPLICIT_AND_CONTENT_INFO), stream))
        != (char *)NULL)
      break;

    /* Set the output. */
    *partOut = stream->buffer;
    *partOutLen = stream->point;
  } while (0);
  
  free (crlDER);
  return (errorMessage);
}

void RIPEMEncipherPKCSFrameConstructor (frame)
RIPEMEncipherPKCSFrame *frame;
{
  /* Must set the pointer to the virtual destructor */
  frame->ripemEncipherFrame.Destructor =
    (RIPEM_ENCIPHER_FRAME_DESTRUCTOR)RIPEMEncipherPKCSFrameDestructor;

  BufferStreamConstructor (&frame->outStream);
  InitList (&frame->issuerNames);
}

void RIPEMEncipherPKCSFrameDestructor (frame)
RIPEMEncipherPKCSFrame *frame;
{
  BufferStreamDestructor (&frame->outStream);
  R_memset ((POINTER)&frame->sealContext, 0, sizeof (frame->sealContext));
  R_memset
    ((POINTER)&frame->signatureContext, 0, sizeof (frame->signatureContext));
}

void RIPEMDecipherPKCSFrameConstructor (frame)
RIPEMDecipherPKCSFrame *frame;
{
  /* Must set the pointer to the virtual destructor */
  frame->ripemDecipherFrame.Destructor =
    (RIPEM_DECIPHER_FRAME_DESTRUCTOR)RIPEMDecipherPKCSFrameDestructor;
  BufferStreamConstructor (&frame->outStream);
  BufferStreamConstructor (&frame->input);
  InitList (&frame->certs);
  InitList (&frame->certChain);
  InitDistinguishedNameStruct (&frame->issuerName);
  InitList (&frame->attributesBuffers);

  /* Default to not detached */
  frame->detached = 0;
}

void RIPEMDecipherPKCSFrameDestructor (frame)
RIPEMDecipherPKCSFrame *frame;
{
  BufferStreamDestructor (&frame->outStream);
  BufferStreamDestructor (&frame->input);
  R_memset
    ((POINTER)&frame->envelopeContext, 0, sizeof (frame->envelopeContext));
  R_memset
    ((POINTER)&frame->signatureContext, 0, sizeof (frame->signatureContext));
  FreeList (&frame->certs);
  FreeList (&frame->certChain);
  R_memset ((POINTER)&frame->signature, 0, sizeof (frame->signature));
  FreeList (&frame->attributesBuffers);
}

/* Write a SignerInfo to stream with the given info.  serialNumber does
     not need to have the leading zeros removed.
   Returns NULL, otherwise error string.
 */
static char *WriteSignerInfo
  (stream, signature, signatureLen, issuerName, issuerNameLen,
   serialNumber, serialNumberLen)
BufferStream *stream;
unsigned char *signature;
unsigned int signatureLen;
unsigned char *issuerName;
unsigned int issuerNameLen;
unsigned char *serialNumber;
unsigned int serialNumberLen;
{
  char *errorMessage;
  unsigned int issuerSerialContentLen, totalContentLen;
  unsigned char *p;

  /* We have to pre-compute the content length of the entire SignerInfo.
       start by getting the content length of the IssuerAndSerialNumber,
       which we will use again. */
  issuerSerialContentLen =
    issuerNameLen +
    der_len (len_large_unsigned (serialNumber, serialNumberLen));
  totalContentLen =
    der_len (1) +   /* version */
    der_len (issuerSerialContentLen) +  /* issuerAndSerialNumber */
    sizeof (ALG_ID_MD5) +  /* digestAlgorithm */
    sizeof (ALG_ID_RSA_ENCRYPTION) +  /* digestEncryptionAlgorithm */
    der_len (signatureLen);    

  /* Save where the buffer stream will write next and reserve enough
       bytes to write the SignerInfo */
  p = stream->buffer + stream->point;
  if ((errorMessage = BufferStreamWrite
       ((unsigned char *)NULL, der_len (totalContentLen), stream))
      != (char *)NULL)
    return (errorMessage);

  /* Start the SEQUENCE */
  *p++ = BER_SEQUENCE;
  put_der_len (&p, totalContentLen);

  /* Put the version */
  *p++ = BER_INTEGER;
  put_der_len (&p, 1);
  *p++ = 1;

  /* Put the issuerAndSerialNumber */
  *p++ = BER_SEQUENCE;
  put_der_len (&p, issuerSerialContentLen);
  put_der_data (&p, issuerName, issuerNameLen);
  put_der_large_unsigned
    (&p, serialNumber, serialNumberLen,
     len_large_unsigned (serialNumber, serialNumberLen));

  put_der_data (&p, ALG_ID_MD5, sizeof (ALG_ID_MD5));
  put_der_data (&p, ALG_ID_RSA_ENCRYPTION, sizeof (ALG_ID_RSA_ENCRYPTION));

  /* Put the signature */
  *p++ = BER_OCTET_STRING;
  put_der_len (&p, signatureLen);
  put_der_data (&p, signature, signatureLen);

  return ((char *)NULL);
}

/* Write a CertificationRequestInfo to stream with the given info.  Use
     only challengePassword from attributes.  attributes my be NULL.
   Returns NULL, otherwise error string.
 */
static char *WriteCertifyRequestInfo
  (stream, subjectName, publicKey, attributes)
BufferStream *stream;
DistinguishedNameStruct *subjectName;
R_RSA_PUBLIC_KEY *publicKey;
RIPEMAttributes *attributes;
{
  char *errorMessage;
  unsigned int attributesContentLen, totalContentLen, publicKeyLen,
    challengePasswordLen;
  unsigned char *p;

  /* We have to pre-compute the content length of the entire encoding.
       Start by getting the content length of the attributes,
       which we will use again.  If zero, then there are no attributes */
  if (attributes == (RIPEMAttributes *)NULL ||
      attributes->haveChallengePassword == FALSE)
    attributesContentLen = 0;
  else {
    challengePasswordLen = strlen (attributes->challengePassword);
    attributesContentLen =
      der_len                   /* sequence for the challenge password */
        (sizeof (CHALLENGE_PASSWORD_ID) +
         der_len                /* set containing one value */
         (der_len (challengePasswordLen)));
  }

  totalContentLen =
    der_len (1) +                                    /* version */
    der_len (len_distinguishedname (subjectName)) +  /* subjectName */
    PubKeyToDERLen (publicKey) +                     /* publicKey */
    (attributesContentLen == 0 ? 0 : der_len (attributesContentLen));

  /* Save where the buffer stream will write next and reserve enough
       bytes to write the CertificationRequestInfo */
  p = stream->buffer + stream->point;
  if ((errorMessage = BufferStreamWrite
       ((unsigned char *)NULL, der_len (totalContentLen), stream))
      != (char *)NULL)
    return (errorMessage);

  /* Start the SEQUENCE */
  *p++ = BER_SEQUENCE;
  put_der_len (&p, totalContentLen);

  /* Put the version 0 */
  *p++ = BER_INTEGER;
  put_der_len (&p, 1);
  *p++ = 0;

  /* Put the subjectName */
  DistinguishedNameToDER (subjectName, &p);

  /* Put the public key */
  PubKeyToDER (publicKey, p, &publicKeyLen);
  p += publicKeyLen;

  if (attributesContentLen > 0) {
    /* Luckily, there is only one attribute so we don't have to worry
         about ordering attributes for correct DER encoding. */
    /* Use [0] IMPLICIT. */
    *p++ = BER_CONSTRUCTED | BER_CONTEXT_SPECIFIC | 0;
    put_der_len (&p, attributesContentLen);

    *p++ = BER_SEQUENCE;
    put_der_len
      (&p, sizeof (CHALLENGE_PASSWORD_ID) +
           der_len                /* set containing one value */
           (der_len (challengePasswordLen)));
    put_der_data (&p, CHALLENGE_PASSWORD_ID, sizeof (CHALLENGE_PASSWORD_ID));
    *p++ = BER_SET;
    put_der_len (&p, der_len (challengePasswordLen));
    /* Finally put the challenge password, choosing between
         printable string and T.61 string */
    *p++ = IsPrintableString
      ((unsigned char *)attributes->challengePassword, challengePasswordLen) ?
      ATTRTAG_PRINTABLE_STRING : ATTRTAG_T61_STRING;
    put_der_len (&p, challengePasswordLen);
    put_der_data
      (&p, (unsigned char *)attributes->challengePassword,
       challengePasswordLen);
  }

  return ((char *)NULL);
}

#ifndef RIPEMSIG
/* Write a RecipientInfo to stream with the given info.  serialNumber does
     not need to have the leading zeros removed.
   Returns NULL, otherwise error string.
 */
static char *WriteRecipientInfo
  (stream, encryptedKey, encryptedKeyLen, issuerName, issuerNameLen,
   serialNumber, serialNumberLen)
BufferStream *stream;
unsigned char *encryptedKey;
unsigned int encryptedKeyLen;
unsigned char *issuerName;
unsigned int issuerNameLen;
unsigned char *serialNumber;
unsigned int serialNumberLen;
{
  char *errorMessage;
  unsigned int issuerSerialContentLen, totalContentLen;
  unsigned char *p;

  /* We have to pre-compute the content length of the entire RecipientInfo.
       start by getting the content length of the IssuerAndSerialNumber,
       which we will use again. */
  issuerSerialContentLen =
    issuerNameLen +
    der_len (len_large_unsigned (serialNumber, serialNumberLen));
  totalContentLen =
    der_len (1) +   /* version */
    der_len (issuerSerialContentLen) +  /* issuerAndSerialNumber */
    sizeof (ALG_ID_RSA_ENCRYPTION) +  /* keyEncryptionAlgorithm */
    der_len (encryptedKeyLen);    

  /* Save where the buffer stream will write next and reserve enough
       bytes to write the SignerInfo */
  p = stream->buffer + stream->point;
  if ((errorMessage = BufferStreamWrite
       ((unsigned char *)NULL, der_len (totalContentLen), stream))
      != (char *)NULL)
    return (errorMessage);

  /* Start the SEQUENCE */
  *p++ = BER_SEQUENCE;
  put_der_len (&p, totalContentLen);

  /* Put the version */
  *p++ = BER_INTEGER;
  put_der_len (&p, 1);
  *p++ = 0;

  /* Put the issuerAndSerialNumber */
  *p++ = BER_SEQUENCE;
  put_der_len (&p, issuerSerialContentLen);
  put_der_data (&p, issuerName, issuerNameLen);
  put_der_large_unsigned
    (&p, serialNumber, serialNumberLen,
     len_large_unsigned (serialNumber, serialNumberLen));

  put_der_data (&p, ALG_ID_RSA_ENCRYPTION, sizeof (ALG_ID_RSA_ENCRYPTION));

  /* Put the encryptedKey */
  *p++ = BER_OCTET_STRING;
  put_der_len (&p, encryptedKeyLen);
  put_der_data (&p, encryptedKey, encryptedKeyLen);

  return ((char *)NULL);
}

/* Decode the RecipientInfo given by der.
   This requires the version to be 0.
   Decode the issuer name into the supplied DistinguishedNameStruct.
   Returns the serialNumber in the buffer which is MAX_SERIAL_NUMBER_LEN
     in length, padded as described by getlargeunsigned.
   Returns a pointer to the encryptedKey.
   This requires key encryption algorithm to be RSA.
   Returns null for success, otherwise error string.
 */
static char *DecodeRecipientInfo
  (der, issuerName, serialNumber, encryptedKey, encryptedKeyLen)
unsigned char *der;
DistinguishedNameStruct *issuerName;
unsigned char *serialNumber;
unsigned char **encryptedKey;
unsigned int *encryptedKeyLen;
{
  unsigned char *recipientInfoEnd, *issuerSerialEnd;
  UINT2 tag;
  unsigned int contentLen;

  if (gettaglen (&tag, &contentLen, &der) < 0 || tag != BER_SEQUENCE)
    return (ERR_PKCS_ENCODING);

  /* Remember where the encoding should end */
  recipientInfoEnd = der + contentLen;

  /* Check that the version is 0. */
  if (gettaglen (&tag, &contentLen, &der) < 0 || tag != BER_INTEGER)
    return (ERR_PKCS_ENCODING);
  if (*der != 0)
    return ("Invalid RecipientInfo version");
  der += 1;

  /* Decode the issuerAndSerialNumber.
   */
  if (gettaglen (&tag, &contentLen, &der) < 0 || tag != BER_SEQUENCE)
    return (ERR_PKCS_ENCODING);
  issuerSerialEnd = der + contentLen;
  if (DERToDistinguishedName (&der, issuerName) != 0)
    return ("Invalid issuer name encoding in RecipientInfo");
  if (getlargeunsigned (serialNumber, MAX_SERIAL_NUMBER_LEN, &der) != 0)
    return (ERR_PKCS_ENCODING);
  if (der != issuerSerialEnd)
    return (ERR_PKCS_ENCODING);

  if (R_memcmp
      ((POINTER)der, (POINTER)ALG_ID_RSA_ENCRYPTION,
       sizeof (ALG_ID_RSA_ENCRYPTION)) != 0)
    return ("Unrecognized key encryption algorithm");
  der += sizeof (ALG_ID_RSA_ENCRYPTION);

  if (gettaglen (&tag, &contentLen, &der) < 0 || tag != BER_OCTET_STRING)
    return (ERR_PKCS_ENCODING);
  *encryptedKey = der;
  *encryptedKeyLen = contentLen;
  der += contentLen;

  if (der != recipientInfoEnd)
    return (ERR_PKCS_ENCODING);

  return ((char *)NULL);
}
#endif

/* Decode the SignerInfo given by der.
   This requires the version to be 1.
   Decode the issuer name into the supplied DistinguishedNameStruct.
   Returns the serialNumber in the buffer which is MAX_SERIAL_NUMBER_LEN
     in length, padded as described by getlargeunsigned.
   Returns a pointer to the signature.
   This checks der's digest algorithm against the supplied value.
   This requires digest encryption algorithm to be RSA.
   attributes and attributesBuffers are for
     DecodeAuthenticatedAttributes.  If there are authenticated attributes,
     this uses signatureContext to finalize the digesting of the content
     and reinitializes it to verify the attributes.  In this case,
     when the caller uses R_VerifyFinal, it will check the signature of
     the attributes.  Also, this makes sure the contentType in the attributes
     matches pkcsMode.
   This skips over unauthenticated attributes if found.
   Returns null for success, otherwise error string.
 */
static char *DecodeSignerInfo
  (der, issuerName, serialNumber, digestAlgorithm, signatureContext, pkcsMode,
   signature, signatureLen, attributes, attributesBuffers)
unsigned char *der;
DistinguishedNameStruct *issuerName;
unsigned char *serialNumber;
int digestAlgorithm;
R_SIGNATURE_CTX *signatureContext;
int pkcsMode;
unsigned char **signature;
unsigned int *signatureLen;
RIPEMAttributes *attributes;
TypList *attributesBuffers;
{
  unsigned char *signerInfoEnd, *issuerSerialEnd, *attributesDER;
  UINT2 tag;
  unsigned int contentLen, attributesDERLen;
  char *errorMessage;

  if (gettaglen (&tag, &contentLen, &der) < 0 || tag != BER_SEQUENCE)
    return (ERR_PKCS_ENCODING);

  /* Remember where the encoding should end */
  signerInfoEnd = der + contentLen;

  /* Check that the version is 1. */
  if (gettaglen (&tag, &contentLen, &der) < 0 || tag != BER_INTEGER)
    return (ERR_PKCS_ENCODING);
  if (*der != 1)
    return ("Invalid SignerInfo version");
  der += 1;

  /* Decode the issuerAndSerialNumber.
   */
  if (gettaglen (&tag, &contentLen, &der) < 0 || tag != BER_SEQUENCE)
    return (ERR_PKCS_ENCODING);
  issuerSerialEnd = der + contentLen;
  if (DERToDistinguishedName (&der, issuerName) != 0)
    return ("Invalid issuer name encoding in SignerInfo");
  if (getlargeunsigned (serialNumber, MAX_SERIAL_NUMBER_LEN, &der) != 0)
    return (ERR_PKCS_ENCODING);
  if (der != issuerSerialEnd)
    return (ERR_PKCS_ENCODING);

  if (digestAlgorithm == DA_MD2) {
    if (DERCheckData (&der, ALG_ID_MD2, sizeof (ALG_ID_MD2)) < 0)
      return ("Digest algorithm in SignerInfo doesn't match SignedData value");
  }
  else if (digestAlgorithm == DA_MD5) {
    if (DERCheckData (&der, ALG_ID_MD5, sizeof (ALG_ID_MD5)) < 0)
      return ("Digest algorithm in SignerInfo doesn't match SignedData value");
  }
  else
    /* This shouldn't happen since we set the digestAlgorithm ourselves */
    return ("Internal: bad digestAlgorithm in DecodeSignerInfo");

  if (*der == (BER_CONSTRUCTED | BER_CONTEXT_SPECIFIC | 0)) {
    /* There are authenticated attributes. */
    attributesDER = der;
    if (gettaglen (&tag, &contentLen, &der) < 0)
      return (ERR_PKCS_ENCODING);
    attributesDERLen = (der - attributesDER) + contentLen;
    
    if ((errorMessage = ProcessAuthenticatedAttributes
         (attributesDER, attributesDERLen, digestAlgorithm, signatureContext,
          pkcsMode, attributes, attributesBuffers)) != 0)
      return (errorMessage);

    der += attributesDERLen;
  }

  if (DERCheckData
      (&der, ALG_ID_RSA_ENCRYPTION, sizeof (ALG_ID_RSA_ENCRYPTION)) < 0)
    return ("Unrecognized digest encryption algorithm");

  if (gettaglen (&tag, &contentLen, &der) < 0 || tag != BER_OCTET_STRING)
    return (ERR_PKCS_ENCODING);
  *signature = der;
  *signatureLen = contentLen;
  der += contentLen;

  if (der < signerInfoEnd) {
    /* Assume this is an unauthenticated attributes, which we want to skip.
     */
    if (gettaglen (&tag, &contentLen, &der) < 0 ||
        tag != (BER_CONSTRUCTED | BER_CONTEXT_SPECIFIC | 1))
      return (ERR_PKCS_ENCODING);
    der += contentLen;
  }

  if (der != signerInfoEnd)
    return (ERR_PKCS_ENCODING);

  return ((char *)NULL);
}

/* Process the attributesDER of length attributesDERLen as described in
     DecodeSignerInfo.
   Note that this may return a bad signature error if the digests don't match.
 */
static char *ProcessAuthenticatedAttributes
  (attributesDER, attributesDERLen, digestAlgorithm, signatureContext,
   pkcsMode, attributes, attributesBuffers)
unsigned char *attributesDER;
unsigned int attributesDERLen;
int digestAlgorithm;
R_SIGNATURE_CTX *signatureContext;
int pkcsMode;
RIPEMAttributes *attributes;
TypList *attributesBuffers;
{
  char *errorMessage = (char *)NULL;
  unsigned char digestInAttributes[MAX_DIGEST_LEN],
    digestOfContent[MAX_DIGEST_LEN];
  unsigned int digestLen;
  int contentType;
    
  /* For error, break to end of do while (0) block. */
  do {
    if ((errorMessage = DecodeAuthenticatedAttributes
         (attributesDER, digestInAttributes, &contentType, attributes,
          attributesBuffers)) != (char *)NULL)
      break;
    if (contentType != pkcsMode) {
      errorMessage =
        "Content type in authenticated attributes does not match message type";
      break;
    }

    /* Now get the digest and check it.  We must go inside the
         R_SIGNATURE_CTX to get the digest context.
     */
    R_DigestFinal
      (&signatureContext->digestContext, digestOfContent, &digestLen);
    if (R_memcmp ((POINTER)digestOfContent, (POINTER)digestInAttributes,
                  digestLen) != 0) {
      errorMessage = "Signature on message content is incorrect";
      break;
    }

    /* Now restart the signature check on the authenticated attributes.
     */
    R_VerifyInit (signatureContext, digestAlgorithm);
    R_VerifyUpdate (signatureContext, attributesDER, attributesDERLen);
  } while (0);

  R_memset ((POINTER)digestInAttributes, 0, sizeof (digestInAttributes));
  R_memset ((POINTER)digestOfContent, 0, sizeof (digestOfContent));
  return (errorMessage);
}

/* Decode the IMPLICIT [0] authenticated attributes given by der.
   This requires there to be a digest and contentType attribute.  The
     supplied digest must be MAX_DIGEST_LEN bytes long, and contentType will be
     set to one of PKCS_SIGNED, etc.
   If an attribute is recognized, put it in the supplied attributes
     struct, otherwise ignore it.
   For signing description, use the supplied attributesBuffers to hold the
     value and point attributes->signingDescription to it.
   Assume that attributes->haveSigningTime, etc. have already all been
     set to FALSE.
   Returns null for success, otherwise error string.
 */
static char *DecodeAuthenticatedAttributes
  (der, digest, contentType, attributes, attributesBuffers)
unsigned char *der;
unsigned char *digest;
int *contentType;
RIPEMAttributes *attributes;
TypList *attributesBuffers;
{
  unsigned char *attributesEnd, *attributeEnd, *valueSetEnd,
    *attributeType, *buffer;
  UINT2 tag;
  unsigned int contentLen;
  BOOL haveDigest, haveContentType;
  char *errorMessage;

  haveDigest = FALSE;
  haveContentType = FALSE;
  
  if (gettaglen (&tag, &contentLen, &der) < 0 ||
      tag != (BER_CONSTRUCTED | BER_CONTEXT_SPECIFIC | 0))
    return (ERR_PKCS_ENCODING);

  /* Remember where the encoding should end */
  attributesEnd = der + contentLen;

  while (der < attributesEnd) {
    if (gettaglen (&tag, &contentLen, &der) < 0 || tag != BER_SEQUENCE)
      return (ERR_PKCS_ENCODING);

    /* Remember where the encoding should end */
    attributeEnd = der + contentLen;

    attributeType = (unsigned char *)NULL;
    if (DERCheckData (&der, SIGNING_TIME_ID, sizeof (SIGNING_TIME_ID)) >= 0) 
      attributeType = SIGNING_TIME_ID;
    else if (DERCheckData
             (&der, SIGNING_DESCRIPTION_ID, sizeof (SIGNING_DESCRIPTION_ID))
             >= 0) 
      attributeType = SIGNING_DESCRIPTION_ID;
    else if (DERCheckData (&der, CONTENT_TYPE_ID, sizeof (CONTENT_TYPE_ID))
             >= 0) 
      attributeType = CONTENT_TYPE_ID;
    else if (DERCheckData (&der, MESSAGE_DIGEST_ID, sizeof (MESSAGE_DIGEST_ID))
             >= 0) 
      attributeType = MESSAGE_DIGEST_ID;

    if (attributeType == (unsigned char *)NULL) {
      /* Unrecognized identifier, so just skip the object ID and value set.
       */
      if (gettaglen (&tag, &contentLen, &der) < 0 || tag != BER_OBJECT_ID)
        return (ERR_PKCS_ENCODING);
      der += contentLen;
      
      if (gettaglen (&tag, &contentLen, &der) < 0 || tag != BER_SET)
        return (ERR_PKCS_ENCODING);
      der += contentLen;
    }
    else {
      /* Decode the value set.
       */
      if (gettaglen (&tag, &contentLen, &der) < 0 || tag != BER_SET)
        return (ERR_PKCS_ENCODING);

      valueSetEnd = der + contentLen;
      while (der < valueSetEnd) {
        if (attributeType == SIGNING_TIME_ID) {
          if (DERToUTC (&der, &attributes->signingTime) < 0)
            return ("Invalid encoding for signing time");

          attributes->haveSigningTime = TRUE;
        }
        else if (attributeType == SIGNING_DESCRIPTION_ID) {
          if (gettaglen (&tag, &contentLen, &der) < 0)
            return (ERR_PKCS_ENCODING);

          /* Use the attributesBuffers to hold the value, including null
               terminator.
           */
          if ((buffer = (unsigned char *)malloc (contentLen + 1)) ==
              (unsigned char *)NULL)
            return (ERR_MALLOC);
          R_memcpy ((POINTER)buffer, (POINTER)der, contentLen);
          buffer[contentLen] = '\0';
          if ((errorMessage = AddToList
               ((TypListEntry *)NULL, buffer, contentLen + 1,
                attributesBuffers)) != (char *)NULL) {
            /* error, so free the buffer */
            free (buffer);
            return (errorMessage);
          }

          der += contentLen;

          attributes->signingDescription = (char *)buffer;
          attributes->haveSigningDescription = TRUE;
        }
        else if (attributeType == CONTENT_TYPE_ID) {
          if ((*contentType = DecodeContentType (der)) == 0)
            return ("Unrecognized content type in authenticated attributes");

          haveContentType = TRUE;
        }
        else if (attributeType == MESSAGE_DIGEST_ID) {
          if (gettaglen (&tag, &contentLen, &der) < 0 ||
              tag != BER_OCTET_STRING || contentLen > MAX_DIGEST_LEN)
            return ("Invalid encoding of message digest attribute");

          R_memcpy ((POINTER)digest, (POINTER)der, contentLen);
          der += contentLen;
          haveDigest = TRUE;
        }
        else
          /* This shouldn't happen since we set attributeType ourselves */
          return
          ("Internal: bad attributeType in DecodeAuthenticatedAttributes");
        
        /* The "else if's" should have handled all cases */
      }

      if (der != valueSetEnd)
        return (ERR_PKCS_ENCODING);
    }

    if (der != attributeEnd)
      return (ERR_PKCS_ENCODING);
  }

  if (der != attributesEnd)
    return (ERR_PKCS_ENCODING);

  if (!haveDigest)
    return ("Message digest is missing from authenticated attributes");
  if (!haveContentType)
    return ("Content type is missing from authenticated attributes");

  return ((char *)NULL);
}

/* Return PKCS_SIGNED if input is the SignedData identifier, etc.  Else
      return 0 if there is no match.
   All identifiers are of length sizeof (CONTENT_TYPE_SIGNED_DATA).
 */
static int DecodeContentType (input)
unsigned char *input;
{
  if (R_memcmp
      ((POINTER)input, (POINTER)CONTENT_TYPE_SIGNED_DATA,
       sizeof (CONTENT_TYPE_SIGNED_DATA)) == 0)
    return (PKCS_SIGNED);
  else if (R_memcmp
           ((POINTER)input, (POINTER)CONTENT_TYPE_ENVELOPED_DATA,
            sizeof (CONTENT_TYPE_ENVELOPED_DATA)) == 0)
    return (PKCS_ENVELOPED);
  else if (R_memcmp
           ((POINTER)input, (POINTER)CONTENT_TYPE_SIGNED_ENVELOPED_DATA,
            sizeof (CONTENT_TYPE_SIGNED_ENVELOPED_DATA)) == 0)
    return (PKCS_SIGNED|PKCS_ENVELOPED);
  else
    return (0);
}

/* Use the information now in frame to match the signer's issuer/serial to a
     certificate, select a certificate chain, and copy the signer's public
     key to publicKey.
   This sets the frame's certChain and chainStatus.
   If chainStatus.overall is 0, this found a self-signed cert and
     put it in the first entry of certChain.  In this case, publicKey
     is undefined.  (The caller should check chainStatus.overall)
   This returns an error if a valid public key can't be found or if
     there is no self-signed cert.
   Return NULL for success, otherwise error string.
 */
static char *GetSignerPublicKey (frame, ripemInfo, publicKey, ripemDatabase)
RIPEMDecipherPKCSFrame *frame;
RIPEMInfo *ripemInfo;
R_RSA_PUBLIC_KEY *publicKey;
RIPEMDatabase *ripemDatabase;
{
  char *errorMessage = (char *)NULL;
  CertificateStruct *certStruct = (CertificateStruct *)NULL;
  CertFieldPointers fieldPointers;
  TypListEntry *entry;
  BOOL isSelfSigned;
  void *certCopy;
  int certLen;

  /* For error, break to end of do while (0) block. */
  do {
    if ((certStruct = (CertificateStruct *)malloc (sizeof (*certStruct)))
        == (CertificateStruct *)NULL) {
      errorMessage = ERR_MALLOC;
      break;
    }

    /* Search through all the certs in the message looking for a match
         with the issuer name/serial number that was in the SignerInfo. */
    for (entry = frame->certs.firstptr;
         entry != (TypListEntry *)NULL;
         entry = entry->nextptr) {
      if ((certLen = DERToCertificate
           ((unsigned char *)entry->dataptr, certStruct, &fieldPointers)) < 0)
        /* Can't decode, so just skip and try the next one. */
        continue;

      if (R_memcmp
          ((POINTER)&frame->issuerName, (POINTER)&certStruct->issuer,
           sizeof (certStruct->issuer)) != 0 ||
          R_memcmp
          ((POINTER)&frame->serialNumber, (POINTER)&certStruct->serialNumber,
           sizeof (certStruct->serialNumber)) != 0)
        /* issuer/serial doesn't match */
        continue;

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
          R_memcpy ((POINTER)certCopy, (POINTER)entry->dataptr, certLen);
          if ((errorMessage = PrependToList
               (certCopy, certLen, &frame->certChain)) != (char *)NULL) {
            /* Free the cert copy we just allocated */
            free (certCopy);
            break;
          }

          /* We have copied the self-signed cert, so break. */
          break;
        }
        else {
          /* Try to find a self-signed cert based on the given subject name
               and public key.  This will return an error if not found.
               In either case, we're done. */
          errorMessage = FindMatchingSelfSignedCert
            (ripemInfo, &certStruct->subject, &certStruct->publicKey);
          break;
        }
      }

      /* Found the public key so break and return */
      *publicKey = certStruct->publicKey;
      break;
    }
    if (errorMessage != (char *)NULL)
      /* Broke loop because of error */
      break;

    if (entry == (TypListEntry *)NULL) {
      /* Finished loop without a match */
      errorMessage =
    "Cannot find a certificate in the message with the sender's issuer/serial";
      break;
    }

    /* Assume we broke the loop because we got the public key */
  } while (0);

  free (certStruct);
  return (errorMessage);
}

/* This is a helper function to find a self-signed certificate in the
     message matching the subject name and public key of the certificate
     which matches the SignerInfo's issuerNameAndSerialNumber.
   If found, the frame's certChain has the self-signed cert.
   If not found, returns "Cannot find certificate chain for sender."
 */
static char *FindMatchingSelfSignedCert (ripemInfo, subjectName, publicKey)
RIPEMInfo *ripemInfo;
DistinguishedNameStruct *subjectName;
R_RSA_PUBLIC_KEY *publicKey;
{
  RIPEMDecipherPKCSFrame *frame =
    (RIPEMDecipherPKCSFrame *)ripemInfo->z.decipherFrame;
  char *errorMessage = (char *)NULL;
  CertificateStruct *certStruct = (CertificateStruct *)NULL;
  CertFieldPointers fieldPointers;
  TypListEntry *entry;
  BOOL isSelfSigned;
  void *certCopy;
  int certLen;

  /* For error, break to end of do while (0) block. */
  do {
    if ((certStruct = (CertificateStruct *)malloc (sizeof (*certStruct)))
        == (CertificateStruct *)NULL) {
      errorMessage = ERR_MALLOC;
      break;
    }

    /* Search through all the certs in the message looking for a match
         with the subject name and public key. */
    for (entry = frame->certs.firstptr;
         entry != (TypListEntry *)NULL;
         entry = entry->nextptr) {
      if ((certLen = DERToCertificate
           ((unsigned char *)entry->dataptr, certStruct, &fieldPointers)) < 0)
        /* Can't decode, so just skip and try the next one. */
        continue;

      if (R_memcmp
          ((POINTER)subjectName, (POINTER)&certStruct->subject,
           sizeof (certStruct->subject)) != 0 ||
          R_memcmp
          ((POINTER)publicKey, (POINTER)&certStruct->publicKey,
           sizeof (certStruct->publicKey)) != 0)
        /* subject name/public key doesn't match */
        continue;

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
        R_memcpy ((POINTER)certCopy, (POINTER)entry->dataptr, certLen);
        if ((errorMessage = PrependToList
             (certCopy, certLen, &frame->certChain)) != (char *)NULL) {
          /* Free the cert copy we just allocated */
          free (certCopy);
          break;
        }

        /* We have copied the self-signed cert, so break. */
        break;
      }
    }
    if (errorMessage != (char *)NULL)
      /* Broke loop because of error */
      break;

    if (entry == (TypListEntry *)NULL) {
      /* Finished loop without a match */
      errorMessage = "Cannot find certificate chain for sender.";
      break;
    }
  } while (0);

  free (certStruct);
  return (errorMessage);
}

/* Encode tag and length octets for the content that has been accumulating
     in the stream.
 */
static void GetEncipherUpdateOutput (partOut, partOutLen, stream)
unsigned char **partOut;
unsigned int *partOutLen;
BufferStream *stream;
{
  unsigned char *p;

  /* We must write the tag and length bytes.  der_len returns the length
       of these plus the actual data, so set *partOutLen to this. Note
       that the content follows the space we allocated for tag and length
       octets. */
  *partOutLen = der_len (stream->point - MAX_TAG_AND_LEN_BYTES);
  /* Point p to where we should put the tag. */
  p = (stream->buffer + stream->point) - *partOutLen;
  /* This is also the result. */
  *partOut = p;

  /* Write the tag and length octets. */
  *p++ = BER_OCTET_STRING;
  put_der_len (&p, stream->point - MAX_TAG_AND_LEN_BYTES);
}

/* This helper routine is used by RIPEMEncipherPKCSFinal (if PKCS_SIGNED) and
     RIPEMSignDetachedPKCSFinal.
   Assume the ripemInfo has a valid RIPEMEncipherPKCSFrame.
   Finalize the signatureContext.  Encrypt the signature if PKCS_ENVELOPED.
   Write the certificate set and the set of one signerInfo to the frame's
     outStream.
   This does not write an END_DATA_EXPLICIT_AND_CONTENT_INFO (the calling
     routine must do this.)
   (If later versions handle authenticated attributes, this can be added
     to this routine.)
   Return NULL for success or error string.
 */
static char *WriteCertsAndSigner (ripemInfo)
RIPEMInfo *ripemInfo;
{
  RIPEMEncipherPKCSFrame *frame =
    (RIPEMEncipherPKCSFrame *)ripemInfo->z.encipherFrame;
  BufferStream *stream;
  CertificateStruct *certStruct = (CertificateStruct *)NULL;
  TypListEntry *entry;
  char *errorMessage = (char *)NULL;
  int status;
  unsigned char signature[MAX_PEM_ENCRYPTED_SIGNATURE_LEN],
    *nameDER = (unsigned char *)NULL, *derPointer;
  unsigned int signatureLen, localPartOutLen;

  /* For error, break to end of do while (0) block. */
  do {
    /* Get stream for quick access. */
    stream = &frame->outStream;

    /* Output originator and issuer certificates.
     */
    if ((errorMessage = BufferStreamWrite
         (CERTIFICATES_SET, sizeof (CERTIFICATES_SET), stream))
        != (char *)NULL)
      break;
    if ((errorMessage = BufferStreamWrite
         (ripemInfo->z.userCertDER, ripemInfo->z.userCertDERLen, stream))
        != (char *)NULL)
      break;
    for (entry = ripemInfo->issuerCerts.firstptr; entry;
         entry = entry->nextptr) {
      if ((errorMessage = BufferStreamWrite
           ((unsigned char *)entry->dataptr, entry->datalen, stream))
          != (char *)NULL)
        break;
    }
    if ((errorMessage = BufferStreamWrite
         (END_INDEFINITE_LEN, sizeof (END_INDEFINITE_LEN), stream))
        != (char *)NULL)
      break;

    /* Finalize the signature and encrypt it if enveloping.
     */
    if ((status = R_SignFinal
         (&frame->signatureContext, signature, &signatureLen,
          &ripemInfo->privateKey)) != 0) {
      errorMessage = FormatRSAError (status);
      break;
    }

#ifndef RIPEMSIG
    if (frame->pkcsMode & PKCS_ENVELOPED) {
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
    }
#endif

    /* Allocate the certStruct on the heap since it is so big. */
    if ((certStruct = (CertificateStruct *)malloc (sizeof (*certStruct)))
        == (CertificateStruct *)NULL) {
      errorMessage = ERR_MALLOC;
      break;
    }
    /* Find the issuer/serial for the sender.  If there is a certificate
         in the issuer cert chains, use that since it is more likely
         to make hierarchy-based implementations happy.  (When RIPEM
         receives this, it will only use it to find the sender's name and
         public key anyway just as it would with a self-signed cert.)
         Otherwise, if there is no issuer cert, use the self-signed cert.
       DERToCertificate will not return an error.
     */
    if (ripemInfo->z.issuerChainCount == 0)
      DERToCertificate
        (ripemInfo->z.userCertDER, certStruct, (CertFieldPointers *)NULL);
    else
      DERToCertificate
        ((unsigned char *)ripemInfo->issuerCerts.firstptr->dataptr,
         certStruct, (CertFieldPointers *)NULL);

    /* Allocate buffer and convert issuer name to DER.
     */
    if ((nameDER = (unsigned char *)malloc
         (len_distinguishedname (&certStruct->issuer) + 4))
        == (unsigned char *)NULL) {
      errorMessage = ERR_MALLOC;
      break;
    }
    derPointer = nameDER;
    DistinguishedNameToDER (&certStruct->issuer, &derPointer);

    /* Output the SignerInfos with one SignerInfo in it.
     */
    if ((errorMessage = BufferStreamWrite
         (SIGNER_INFOS_START, sizeof (SIGNER_INFOS_START), stream))
        != (char *)NULL)
      break;
    if ((errorMessage = WriteSignerInfo
         (stream, signature, signatureLen, nameDER, derPointer - nameDER,
          certStruct->serialNumber, sizeof (certStruct->serialNumber)))
        != (char *)NULL)
      break;
    if ((errorMessage = BufferStreamWrite
         (END_INDEFINITE_LEN, sizeof (END_INDEFINITE_LEN), stream))
        != (char *)NULL)
      break;
  } while (0);
  
  R_memset ((POINTER)signature, 0, sizeof (signature));
  free (certStruct);
  free (nameDER);
  return (errorMessage);
}

