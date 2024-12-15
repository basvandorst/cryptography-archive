/******************************************************************* * * This file was generated by TIS/ASN1COMP Ver. 4.1, an ASN.1 compiler. * TIS/ASN1COMP is Copyright (c) 1998, TIS Labs at Network Associates, Inc. * * This file was AUTOMATICALLY GENERATED on Fri Apr  9 20:38:17 1999 * ******************************************************************/#ifndef pkcs8_STRUCT_H_#define pkcs8_STRUCT_H_#include "pkcs8_sizes.h"#include "pkcs7_asn.h"/******************************************************************* * * Structure and type definitions * ******************************************************************/typedef PKIOCTET_STRING   PKIPKCS8EncryptedData;#define PKIID_PKCS8EncryptedData PKIID_OCTET_STRINGtypedef PKIOCTET_STRING   PKIPrivateKey;#define PKIID_PrivateKey PKIID_OCTET_STRINGtypedef PKIAlgorithmIdentifier PKIEncryptionAlgorithmIdentifier;#define PKIID_EncryptionAlgorithmIdentifier PKIID_AlgorithmIdentifiertypedef PKIAlgorithmIdentifier PKIPrivateKeyAlgorithmIdentifier;#define PKIID_PrivateKeyAlgorithmIdentifier PKIID_AlgorithmIdentifiertypedef struct {    PKIEncryptionAlgorithmIdentifier  encryptionAlgorithm;    PKIPKCS8EncryptedData  encryptedData;} PKIEncryptedPrivateKeyInfo;#define PKIID_EncryptedPrivateKeyInfo PKIID_SEQUENCEtypedef struct {    PKIVersion  version;    PKIPrivateKeyAlgorithmIdentifier  privateKeyAlgorithm;    PKIPrivateKey  privateKey;    PKIAttributes *attributes;} PKIPrivateKeyInfo;#define PKIID_PrivateKeyInfo PKIID_SEQUENCE#endif /* pkcs8_STRUCT_H_ */