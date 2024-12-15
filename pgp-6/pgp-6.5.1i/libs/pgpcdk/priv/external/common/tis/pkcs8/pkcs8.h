/*******************************************************************
 *
 * This file was generated by TIS/ASN1COMP Ver. 4.1, an ASN.1 compiler.
 * TIS/ASN1COMP is Copyright (c) 1998, TIS Labs at Network Associates, Inc.
 *
 * This file was AUTOMATICALLY GENERATED on Fri Apr  9 20:38:17 1999
 *
 ******************************************************************/

#ifndef pkcs8_H_
#define pkcs8_H_

#include "pkcs8_struct.h"

/*******************************************************************
 *
 * Function declarations for internal functions
 *
 ******************************************************************/

#define PKISizeofPKCS8EncryptedDataInternal PKISizeofOCTET_STRINGInternal
#define PKIDropInPlacePKCS8EncryptedData PKIDropInPlaceOCTET_STRING
#define PKIPackPKCS8EncryptedDataInternal PKIPackOCTET_STRINGInternal
#define PKIUnpkInPlacePKCS8EncryptedData PKIUnpkInPlaceOCTET_STRING
#define PKIUnpackPKCS8EncryptedDataInternal PKIUnpackOCTET_STRINGInternal

#define PKISizeofPrivateKeyInternal PKISizeofOCTET_STRINGInternal
#define PKIDropInPlacePrivateKey PKIDropInPlaceOCTET_STRING
#define PKIPackPrivateKeyInternal PKIPackOCTET_STRINGInternal
#define PKIUnpkInPlacePrivateKey PKIUnpkInPlaceOCTET_STRING
#define PKIUnpackPrivateKeyInternal PKIUnpackOCTET_STRINGInternal

#define PKISizeofEncryptionAlgorithmIdentifierInternal PKISizeofAlgorithmIdentifierInternal
#define PKIDropInPlaceEncryptionAlgorithmIdentifier PKIDropInPlaceAlgorithmIdentifier
#define PKIPackEncryptionAlgorithmIdentifierInternal PKIPackAlgorithmIdentifierInternal
#define PKIUnpkInPlaceEncryptionAlgorithmIdentifier PKIUnpkInPlaceAlgorithmIdentifier
#define PKIUnpackEncryptionAlgorithmIdentifierInternal PKIUnpackAlgorithmIdentifierInternal

#define PKISizeofPrivateKeyAlgorithmIdentifierInternal PKISizeofAlgorithmIdentifierInternal
#define PKIDropInPlacePrivateKeyAlgorithmIdentifier PKIDropInPlaceAlgorithmIdentifier
#define PKIPackPrivateKeyAlgorithmIdentifierInternal PKIPackAlgorithmIdentifierInternal
#define PKIUnpkInPlacePrivateKeyAlgorithmIdentifier PKIUnpkInPlaceAlgorithmIdentifier
#define PKIUnpackPrivateKeyAlgorithmIdentifierInternal PKIUnpackAlgorithmIdentifierInternal

size_t PKISizeofEncryptedPrivateKeyInfoInternal(
    PKIEncryptedPrivateKeyInfo *asnstruct,
    int outerSizeFlag,
    int expTaggedFlag);
void PKIDropInPlaceEncryptedPrivateKeyInfo(
    PKICONTEXT *ctx,
    PKIEncryptedPrivateKeyInfo *f);
size_t PKIPackEncryptedPrivateKeyInfoInternal(
    PKICONTEXT *ctx,
    unsigned char *buf,
    size_t buflen,
    PKIEncryptedPrivateKeyInfo *asnstruct,
    unsigned char tag,
    int *erret);
size_t PKIUnpkInPlaceEncryptedPrivateKeyInfo (
    PKICONTEXT *ctx,
    PKIEncryptedPrivateKeyInfo *asnstruct,
    const unsigned char *buf,
    size_t buflen,
    unsigned char tag,
    int *erret);
size_t PKIUnpackEncryptedPrivateKeyInfoInternal(
    PKICONTEXT *ctx,
    PKIEncryptedPrivateKeyInfo **asnstruct,
    const unsigned char *buf,
    size_t buflen,
    unsigned char tag,
    int *erret);

size_t PKISizeofPrivateKeyInfoInternal(
    PKIPrivateKeyInfo *asnstruct,
    int outerSizeFlag,
    int expTaggedFlag);
void PKIDropInPlacePrivateKeyInfo(
    PKICONTEXT *ctx,
    PKIPrivateKeyInfo *f);
size_t PKIPackPrivateKeyInfoInternal(
    PKICONTEXT *ctx,
    unsigned char *buf,
    size_t buflen,
    PKIPrivateKeyInfo *asnstruct,
    unsigned char tag,
    int *erret);
size_t PKIUnpkInPlacePrivateKeyInfo (
    PKICONTEXT *ctx,
    PKIPrivateKeyInfo *asnstruct,
    const unsigned char *buf,
    size_t buflen,
    unsigned char tag,
    int *erret);
size_t PKIUnpackPrivateKeyInfoInternal(
    PKICONTEXT *ctx,
    PKIPrivateKeyInfo **asnstruct,
    const unsigned char *buf,
    size_t buflen,
    unsigned char tag,
    int *erret);


#endif /* pkcs8_H_ */