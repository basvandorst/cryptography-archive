/*******************************************************************
 *
 * This file was generated by TIS/ASN1COMP Ver. 4.1, an ASN.1 compiler.
 * TIS/ASN1COMP is Copyright (c) 1998, TIS Labs at Network Associates, Inc.
 *
 * This file was AUTOMATICALLY GENERATED on Fri Apr  9 20:38:17 1999
 *
 ******************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pkcs8_asn.h"

/*******************************************************************
 *
 * Code for internal routines
 *
 *  DropInPlaceXXX:  drops the contents of a block, in place
 *
 *  _PackXXX: packs up a structure into an ASN.1 block
 *
 *  UnpkInPlaceXXX:  unpacks an ASN.1 block into a structure in place
 *
 *  _unPackXXX: carves a structure block and unpacks into it
 *
 ******************************************************************/


/******************************************************************
 * Routines for EncryptedPrivateKeyInfo
 ******************************************************************/

size_t PKISizeofEncryptedPrivateKeyInfoInternal(
    PKIEncryptedPrivateKeyInfo *asnstruct,
    int outerSizeFlag,
    int expTaggedFlag)
{
    size_t body_size = 0;

    if (asnstruct == NULL)
        return 0;

    body_size =
            PKISizeofEncryptionAlgorithmIdentifierInternal(&asnstruct->encryptionAlgorithm, PKITRUE, PKIFALSE)
          + PKISizeofPKCS8EncryptedDataInternal(&asnstruct->encryptedData, PKITRUE, PKIFALSE) ;

    if (outerSizeFlag == PKITRUE)
        body_size = PKITagged(body_size, 1);

    if (expTaggedFlag == PKITRUE)
        body_size = PKITagged(body_size, 1); /* this is seq like */

    return body_size;

} /* PKISizeofEncryptedPrivateKeyInfoInternal */

void PKIDropInPlaceEncryptedPrivateKeyInfo(
    PKICONTEXT *ctx,
    PKIEncryptedPrivateKeyInfo *f)
{
    if (ctx == NULL)
        return;

    if (f == NULL) return ;
    PKIDropInPlaceEncryptionAlgorithmIdentifier(ctx, &(f->encryptionAlgorithm));
    PKIDropInPlacePKCS8EncryptedData(ctx, &(f->encryptedData));
} /* PKIDropInPlaceEncryptedPrivateKeyInfo */

size_t PKIPackEncryptedPrivateKeyInfoInternal(
    PKICONTEXT *ctx,
    unsigned char *buf,
    size_t buflen,
    PKIEncryptedPrivateKeyInfo *asnstruct,
    unsigned char tag,
    int *erret)
{
    size_t bytesused;
    size_t tagsize;
    size_t datasize;

    if (erret == NULL) return 0; /* can't report errors */

    if (ctx == NULL) {
        PKIERR(PKIErrBadContext);
        return 0;
    }
    if (asnstruct == NULL) return 0;

    /* lth of the block body */
    datasize = PKISizeofEncryptedPrivateKeyInfo(ctx, asnstruct, PKIFALSE);
    tagsize = 1 + PKILengthSize(datasize);
    if (datasize+tagsize > buflen) {
        PKIERR(PKIErrPackBufferTooShort);
        return 0;
    }

    /* this is a SEQUENCE */
    bytesused = PKIPutTag(buf, (unsigned char)(tag|0x20), datasize);
    if (bytesused != tagsize) {
        PKIERR(PKIErrPackOverrun);
        return bytesused;
    }
    datasize += tagsize;

  do {

    /* field encryptionAlgorithm of EncryptedPrivateKeyInfo */
    bytesused += PKIPackEncryptionAlgorithmIdentifierInternal(ctx, buf+bytesused, buflen-bytesused,
                       &(asnstruct->encryptionAlgorithm), PKIID_EncryptionAlgorithmIdentifier, erret);
    if (bytesused > datasize || *erret != 0)
        break;

    /* field encryptedData of EncryptedPrivateKeyInfo */
    bytesused += PKIPackPKCS8EncryptedDataInternal(ctx, buf+bytesused, buflen-bytesused,
                       &(asnstruct->encryptedData), PKIID_PKCS8EncryptedData, erret);
    if (bytesused > datasize || *erret != 0)
        break;

  } while(0);

    if (bytesused < datasize && *erret == 0)
        PKIERR(PKIErrPackUnderrun)
    else if (bytesused > datasize && *erret == 0)
        PKIERR(PKIErrPackOverrun)

    return bytesused;
} /* PKIPackEncryptedPrivateKeyInfoInternal */

size_t PKIUnpkInPlaceEncryptedPrivateKeyInfo(
    PKICONTEXT *ctx,
    PKIEncryptedPrivateKeyInfo *asnstruct,
    const unsigned char *buf,
    size_t buflen,
    unsigned char tag,
    int *erret)
{
    size_t bytesused = 0;
    size_t datasize;
    size_t localsize;
    int indef = 0;

    PKITRACE_PRINT_FN((tag|0x20), 0x30, "SEQUENCE", "EncryptedPrivateKeyInfo" );

    if (erret == NULL) return 0; /* can't report errors */
    *erret = 0;

    if (ctx == NULL) {
        PKIERR(PKIErrBadContext);
        return 0;
    }

    if (asnstruct == NULL) {
        PKIERR(PKIErrUnpackNoStructure);
        return 0;
    }

    if (buf == NULL) {
        PKIERR(PKIErrUnpackNoBlockPtr);
        return 0;
    }

    if (buflen <= 0) return 0; /* no error -- no block */

    if ( (*buf & 0xDF) != (tag & 0xDF) )
        return 0; /* no error code, just no block */
    if ( (*buf & 0x20) != 0x20) {
        PKIERR(PKIErrUnpackInvalidEncoding);
        return 0;
    }

    /* accept the tag byte */
    bytesused++;

    /* get the block length */
    bytesused += PKIGetLength(buf+bytesused, &datasize);
    if ((int)datasize == -1) {
        localsize = buflen;
        indef = 1;
    }
    else {
        localsize = bytesused + datasize;
        if (localsize > buflen) {
            PKIERR(PKIErrUnpackOverrun);
            return 0;
        }
    }

    PKITRACE_INCR_LEVEL;
  do {

    /* field encryptionAlgorithm of EncryptedPrivateKeyInfo */
    bytesused += PKIUnpkInPlaceEncryptionAlgorithmIdentifier(ctx, &(asnstruct->encryptionAlgorithm), buf+bytesused,
                        localsize-bytesused, PKIID_EncryptionAlgorithmIdentifier, erret);
    if (bytesused > localsize || *erret != 0)
        break;

    /* field encryptedData of EncryptedPrivateKeyInfo */
    bytesused += PKIUnpkInPlacePKCS8EncryptedData(ctx, &(asnstruct->encryptedData), buf+bytesused,
                        localsize-bytesused, PKIID_PKCS8EncryptedData, erret);
    if (bytesused > localsize || *erret != 0)
        break;

    if (indef) {
        if ( *(buf+bytesused) != 0x00 &&
             *(buf+bytesused+1) != 0x00 ) {
            PKIERR(PKIErrUnpackInvalidEncoding);
            break;
        }
        bytesused += 2;
    }
  } while(0);

    PKITRACE_DECR_LEVEL;
    if (bytesused > localsize && *erret == 0)
        PKIERR(PKIErrUnpackOverrun);
    if (!indef && bytesused < localsize && *erret == 0)
        PKIERR(PKIErrUnpackUnderrun);

    return bytesused;
} /* PKIUnpkInPlaceEncryptedPrivateKeyInfo */

size_t PKIUnpackEncryptedPrivateKeyInfoInternal(
    PKICONTEXT *ctx,
    PKIEncryptedPrivateKeyInfo **asnstruct,
    const unsigned char *buf,
    size_t buflen,
    unsigned char tag,
    int *erret)
{
    size_t bytesused;
    PKIEncryptedPrivateKeyInfo *local = NULL;

    if (erret == NULL) return 0;
    *erret = 0;

    if (ctx == NULL) {
        PKIERR(PKIErrBadContext);
        return 0;
    }
    if (asnstruct == NULL) {
        PKIERR(PKIErrUnpackNoStructure);
        return 0;
    }
    *asnstruct = NULL;

    if (buflen <= 0) return 0; /* no bytes left */

    if ( (*buf & 0xDF) != (tag & 0xDF) ) 
        return 0; /* not correct tag */

    local = PKINewEncryptedPrivateKeyInfo(ctx); /* carve a block for it */
    bytesused = PKIUnpkInPlaceEncryptedPrivateKeyInfo(ctx, local, buf, buflen, tag, erret);
    if (*erret != 0) {
        if (local != NULL) PKIFreeEncryptedPrivateKeyInfo(ctx, local);
        return 0;
    }
    *asnstruct = local;
    return bytesused;
} /* PKIUnpackEncryptedPrivateKeyInfoInternal */


/******************************************************************
 * Routines for PrivateKeyInfo
 ******************************************************************/

size_t PKISizeofPrivateKeyInfoInternal(
    PKIPrivateKeyInfo *asnstruct,
    int outerSizeFlag,
    int expTaggedFlag)
{
    size_t body_size = 0;

    if (asnstruct == NULL)
        return 0;

    body_size =
            PKISizeofVersionInternal(&asnstruct->version, PKITRUE, PKIFALSE)
          + PKISizeofPrivateKeyAlgorithmIdentifierInternal(&asnstruct->privateKeyAlgorithm, PKITRUE, PKIFALSE)
          + PKISizeofPrivateKeyInternal(&asnstruct->privateKey, PKITRUE, PKIFALSE)
          + PKISizeofAttributesInternal(asnstruct->attributes, PKITRUE, PKIFALSE) ;

    if (outerSizeFlag == PKITRUE)
        body_size = PKITagged(body_size, 1);

    if (expTaggedFlag == PKITRUE)
        body_size = PKITagged(body_size, 1); /* this is seq like */

    return body_size;

} /* PKISizeofPrivateKeyInfoInternal */

void PKIDropInPlacePrivateKeyInfo(
    PKICONTEXT *ctx,
    PKIPrivateKeyInfo *f)
{
    if (ctx == NULL)
        return;

    if (f == NULL) return ;
    PKIDropInPlaceVersion(ctx, &(f->version));
    PKIDropInPlacePrivateKeyAlgorithmIdentifier(ctx, &(f->privateKeyAlgorithm));
    PKIDropInPlacePrivateKey(ctx, &(f->privateKey));
    PKIFreeAttributes(ctx, f->attributes);
    f->attributes = NULL;
} /* PKIDropInPlacePrivateKeyInfo */

size_t PKIPackPrivateKeyInfoInternal(
    PKICONTEXT *ctx,
    unsigned char *buf,
    size_t buflen,
    PKIPrivateKeyInfo *asnstruct,
    unsigned char tag,
    int *erret)
{
    size_t bytesused;
    size_t tagsize;
    size_t datasize;

    if (erret == NULL) return 0; /* can't report errors */

    if (ctx == NULL) {
        PKIERR(PKIErrBadContext);
        return 0;
    }
    if (asnstruct == NULL) return 0;

    /* lth of the block body */
    datasize = PKISizeofPrivateKeyInfo(ctx, asnstruct, PKIFALSE);
    tagsize = 1 + PKILengthSize(datasize);
    if (datasize+tagsize > buflen) {
        PKIERR(PKIErrPackBufferTooShort);
        return 0;
    }

    /* this is a SEQUENCE */
    bytesused = PKIPutTag(buf, (unsigned char)(tag|0x20), datasize);
    if (bytesused != tagsize) {
        PKIERR(PKIErrPackOverrun);
        return bytesused;
    }
    datasize += tagsize;

  do {

    /* field version of PrivateKeyInfo */
    bytesused += PKIPackVersionInternal(ctx, buf+bytesused, buflen-bytesused,
                       &(asnstruct->version), PKIID_Version, erret);
    if (bytesused > datasize || *erret != 0)
        break;

    /* field privateKeyAlgorithm of PrivateKeyInfo */
    bytesused += PKIPackPrivateKeyAlgorithmIdentifierInternal(ctx, buf+bytesused, buflen-bytesused,
                       &(asnstruct->privateKeyAlgorithm), PKIID_PrivateKeyAlgorithmIdentifier, erret);
    if (bytesused > datasize || *erret != 0)
        break;

    /* field privateKey of PrivateKeyInfo */
    bytesused += PKIPackPrivateKeyInternal(ctx, buf+bytesused, buflen-bytesused,
                       &(asnstruct->privateKey), PKIID_PrivateKey, erret);
    if (bytesused > datasize || *erret != 0)
        break;

    /* field attributes of PrivateKeyInfo */
    if (asnstruct->attributes != NULL) { /* optional */
        bytesused += PKIPackAttributesInternal(ctx, buf+bytesused, buflen-bytesused,
                           asnstruct->attributes, 0x80 | 0x00, erret ) ;
        if (bytesused > datasize || *erret != 0)
            break;
    }

  } while(0);

    if (bytesused < datasize && *erret == 0)
        PKIERR(PKIErrPackUnderrun)
    else if (bytesused > datasize && *erret == 0)
        PKIERR(PKIErrPackOverrun)

    return bytesused;
} /* PKIPackPrivateKeyInfoInternal */

size_t PKIUnpkInPlacePrivateKeyInfo(
    PKICONTEXT *ctx,
    PKIPrivateKeyInfo *asnstruct,
    const unsigned char *buf,
    size_t buflen,
    unsigned char tag,
    int *erret)
{
    size_t bytesused = 0;
    size_t datasize;
    size_t localsize;
    int indef = 0;

    PKITRACE_PRINT_FN((tag|0x20), 0x30, "SEQUENCE", "PrivateKeyInfo" );

    if (erret == NULL) return 0; /* can't report errors */
    *erret = 0;

    if (ctx == NULL) {
        PKIERR(PKIErrBadContext);
        return 0;
    }

    if (asnstruct == NULL) {
        PKIERR(PKIErrUnpackNoStructure);
        return 0;
    }

    if (buf == NULL) {
        PKIERR(PKIErrUnpackNoBlockPtr);
        return 0;
    }

    if (buflen <= 0) return 0; /* no error -- no block */

    if ( (*buf & 0xDF) != (tag & 0xDF) )
        return 0; /* no error code, just no block */
    if ( (*buf & 0x20) != 0x20) {
        PKIERR(PKIErrUnpackInvalidEncoding);
        return 0;
    }

    /* accept the tag byte */
    bytesused++;

    /* get the block length */
    bytesused += PKIGetLength(buf+bytesused, &datasize);
    if ((int)datasize == -1) {
        localsize = buflen;
        indef = 1;
    }
    else {
        localsize = bytesused + datasize;
        if (localsize > buflen) {
            PKIERR(PKIErrUnpackOverrun);
            return 0;
        }
    }

    PKITRACE_INCR_LEVEL;
  do {

    /* field version of PrivateKeyInfo */
    bytesused += PKIUnpkInPlaceVersion(ctx, &(asnstruct->version), buf+bytesused,
                        localsize-bytesused, PKIID_Version, erret);
    if (bytesused > localsize || *erret != 0)
        break;

    /* field privateKeyAlgorithm of PrivateKeyInfo */
    bytesused += PKIUnpkInPlacePrivateKeyAlgorithmIdentifier(ctx, &(asnstruct->privateKeyAlgorithm), buf+bytesused,
                        localsize-bytesused, PKIID_PrivateKeyAlgorithmIdentifier, erret);
    if (bytesused > localsize || *erret != 0)
        break;

    /* field privateKey of PrivateKeyInfo */
    bytesused += PKIUnpkInPlacePrivateKey(ctx, &(asnstruct->privateKey), buf+bytesused,
                        localsize-bytesused, PKIID_PrivateKey, erret);
    if (bytesused > localsize || *erret != 0)
        break;

    /* field attributes of PrivateKeyInfo */
    if (!indef && bytesused >= localsize) {
        PKITRACE_DECR_LEVEL;
        return bytesused;
    }
    if (indef && *(buf+bytesused) == 0x00 &&
                 *(buf+bytesused+1) == 0x00) {
        PKITRACE_DECR_LEVEL;
        bytesused += 2;
        return bytesused;
    }
    if (asnstruct->attributes != NULL)
        PKIFreeAttributes(ctx, asnstruct->attributes);
    bytesused += PKIUnpackAttributesInternal(ctx, &(asnstruct->attributes),
                 buf+bytesused, localsize-bytesused,
                 0x80 | 0x00, erret);
    if (bytesused > localsize || *erret != 0)
        break;

    if (indef) {
        if ( *(buf+bytesused) != 0x00 &&
             *(buf+bytesused+1) != 0x00 ) {
            PKIERR(PKIErrUnpackInvalidEncoding);
            break;
        }
        bytesused += 2;
    }
  } while(0);

    PKITRACE_DECR_LEVEL;
    if (bytesused > localsize && *erret == 0)
        PKIERR(PKIErrUnpackOverrun);
    if (!indef && bytesused < localsize && *erret == 0)
        PKIERR(PKIErrUnpackUnderrun);

    return bytesused;
} /* PKIUnpkInPlacePrivateKeyInfo */

size_t PKIUnpackPrivateKeyInfoInternal(
    PKICONTEXT *ctx,
    PKIPrivateKeyInfo **asnstruct,
    const unsigned char *buf,
    size_t buflen,
    unsigned char tag,
    int *erret)
{
    size_t bytesused;
    PKIPrivateKeyInfo *local = NULL;

    if (erret == NULL) return 0;
    *erret = 0;

    if (ctx == NULL) {
        PKIERR(PKIErrBadContext);
        return 0;
    }
    if (asnstruct == NULL) {
        PKIERR(PKIErrUnpackNoStructure);
        return 0;
    }
    *asnstruct = NULL;

    if (buflen <= 0) return 0; /* no bytes left */

    if ( (*buf & 0xDF) != (tag & 0xDF) ) 
        return 0; /* not correct tag */

    local = PKINewPrivateKeyInfo(ctx); /* carve a block for it */
    bytesused = PKIUnpkInPlacePrivateKeyInfo(ctx, local, buf, buflen, tag, erret);
    if (*erret != 0) {
        if (local != NULL) PKIFreePrivateKeyInfo(ctx, local);
        return 0;
    }
    *asnstruct = local;
    return bytesused;
} /* PKIUnpackPrivateKeyInfoInternal */

