#ifndef _SEC_FILENAMES_
#define _SEC_FILENAMES_

/*
 *      F i l e n a m e s
 */

#ifdef MS_DOS
#define PATH_SEPARATION_STRING "\\"
#define PATH_SEPARATION_CHAR   '\\'

#define DEF_PSE             "pse"        /* Default PSE Name                */
#define DEF_CADIR           "ca"         /* default CA directory            */
#define DEF_CAPSE           "capse"      /* Default PSE Name                */
/*
 *      Names of PSE Objects (File-Names in the PSE)
 */
#define SignCert_name       "SignCert"   /* Cert for Public Signature Key                          */
#define EncCert_name        "EncCert"    /* Cert for Public Encryption Key                         */
#define Cert_name           "Cert"       /* Cert for Public Signature/Encryption Key               */
#define SignCSet_name       "SignCSet"   /* Set of CrossCertificates for own Public Signature Key  */
#define EncCSet_name        "EncCSet"    /* Set of CrossCertificates for own Public Encryption Key */
#define CSet_name           "CSet"       /* Set of CrossCertificates for own Public Sign./Encr.Key */
#define SignSK_name         "SignSK"     /* Secret Signature Key                                   */
#define DecSKnew_name       "DecSKnew"   /* Secret Decrypt. Key (new)                              */
#define DecSKold_name       "DecSKold"   /* Secret Decrypt. Key (old)                              */
#define SKnew_name          "SKnew"      /* Secret Key (new)                                       */
#define SKold_name          "SKold"      /* Secret Key (old)                                       */
#define FCPath_name         "FCPath"     /* Forward Certification Path                             */
#define PKRoot_name         "PKRoot"     /* PK of Top-Level Ca (old/new)                           */
#define PKList_name         "PKList"     /* Trusted Public Verific. Keys  (SET_OF ToBeSigned)      */
#define EKList_name         "EKList"     /* Trusted Public Encrypt. Keys  (SET_OF ToBeSigned)      */
#define PCAList_name        "PCAList"    /* Trusted PCA Keys   (SET_OF ToBeSigned)                 */
#define CrossCSet_name      "CrossCSet"  /* Set of CrossCertificatePairs                           */
#define CrlSet_name         "CrlSet"     /* Revocation lists of known CAs                          */
#define SerialNumber_name   "SerialNo"   /* Serial number (for CA's only)                          */
#define EDBKey_name         "EDBKey"     /* Symmetric Key for EDB encryption (for DSA's only)      */
#define AliasList_name      "AliasLst"   /* User's AliasList                                       */
#define QuipuPWD_name       "QuipuPWD"   /* User's X.500 password                                  */
#define DHparam_name        "DHparam"    /* Prime p and base g for Diffie-Hellman key agreement    */
#define Random_name         "Random"     /* Random seed                                            */

#define PSE_tmpSignatureSK  "SgnSKtmp"   /* temporary PSE object */
#define PSE_tmpDecryptionSK "DecSKtmp"   /* temporary PSE object */
#define PSE_tmpSK           "SKtmp"      /* temporary PSE object */

#define GENPSE_DIR          "genpse"

#define X500_name           "X500"

#define ALIASFILE           "af-alias"


#define ALGS_RSA_PK_name    "R-PK"       /* not more than 4 chars */
#define ALGS_RSA_SK_name    "R-SK"       /* not more than 4 chars */
#define ALGS_DSA_PK_name    "D-PK"       /* not more than 4 chars */
#define ALGS_DSA_SK_name    "D-SK"       /* not more than 4 chars */


#define EXT_ENCRYPTION_FILE ".enc"
#define EXT_ENCRYPTION_KEY  ".key"
#define EXT_HASH            ".hsh"
#define EXT_SIGNATURE       ".sig"
#define EXT_CERTIFICATE     ".ctf"
#define EXT_BACKUP          ".BAK"

#else	/* non-MS_DOS (e.g. UNIX, MAC) */

#ifdef MAC
#define PATH_SEPARATION_STRING ":"
#define PATH_SEPARATION_CHAR   ':'
#else   /* non-MAC (e.g. UNIX) */
#define PATH_SEPARATION_STRING "/"
#define PATH_SEPARATION_CHAR   '/'
#endif  /* MAC */

#define DEF_PSE             ".pse"        /* Default PSE Name                */
#define DEF_CADIR           ".ca"         /* default CA directory            */
#define DEF_CAPSE           ".capse"      /* Default PSE Name                */
/*
 *      Names of PSE Objects (File-Names in the PSE)
 */
#define SignCert_name       "SignCert"   /* Cert for Public Signature Key                          */
#define EncCert_name        "EncCert"    /* Cert for Public Encryption Key                         */
#define Cert_name           "Cert"       /* Cert for Public Signature/Encryption Key               */
#define SignCSet_name       "SignCSet"   /* Set of CrossCertificates for own Public Signature Key  */
#define EncCSet_name        "EncCSet"    /* Set of CrossCertificates for own Public Encryption Key */
#define CSet_name           "CSet"       /* Set of CrossCertificates for own Public Sign./Encr.Key */
#define SignSK_name         "SignSK"     /* Secret Signature Key                                   */
#define DecSKnew_name       "DecSKnew"   /* Secret Decrypt. Key (new)                              */
#define DecSKold_name       "DecSKold"   /* Secret Decrypt. Key (old)                              */
#define SKnew_name          "SKnew"      /* Secret Key (new)                                       */
#define SKold_name          "SKold"      /* Secret Key (old)                                       */
#define FCPath_name         "FCPath"     /* Forward Certification Path                             */
#define PKRoot_name         "PKRoot"     /* PK of Top-Level Ca (old/new)                           */
#define PKList_name         "PKList"     /* Trusted Public Verific. Keys (SET_OF ToBeSigned)       */
#define EKList_name         "EKList"     /* Trusted Public Encrypt. Keys (SET_OF ToBeSigned)       */
#define PCAList_name        "PCAList"    /* Trusted PCA Keys   (SET_OF ToBeSigned)                 */
#define CrossCSet_name      "CrossCSet"  /* Set of CrossCertificatePairs                           */
#define CrlSet_name         "CrlSet"     /* Revocation lists of known CAs                          */
#define SerialNumber_name   "SerialNumber"   /* Serial number (for CA's only)                      */
#define EDBKey_name         "EDBKey"     /* Symmetric Key for EDB encryption (for DSA's only)      */
#define AliasList_name      "AliasList"  /* User's AliasList                                       */
#define QuipuPWD_name       "QuipuPWD"   /* User's X.500 password                                  */
#define DHparam_name        "DHparam"    /* Prime p and base g for Diffie-Hellman key agreement    */
#define Random_name         "Random"     /* Random seed                                            */

#define PSE_tmpSignatureSK  "SignSKtmp"       /* temporary PSE object */
#define PSE_tmpDecryptionSK "DecSKtmp"        /* temporary PSE object */
#define PSE_tmpSK           "SKtmp"           /* temporary PSE object */

#define GENPSE_DIR          "genpse"

#define X500_name           "X500"

#define ALIASFILE           ".af-alias"

#define ALGS_RSA_PK_name    "RSA-PK-"       /* not more than 12 chars */
#define ALGS_RSA_SK_name    "RSA-SK-"       /* not more than 12 chars */
#define ALGS_DSA_PK_name    "DSA-PK-"       /* not more than 12 chars */
#define ALGS_DSA_SK_name    "DSA-SK-"       /* not more than 12 chars */

#define EXT_ENCRYPTION_FILE ".enc"
#define EXT_ENCRYPTION_KEY  ".key"
#define EXT_HASH            ".hsh"
#define EXT_SIGNATURE       ".sig"
#define EXT_CERTIFICATE     ".ctf"
#define EXT_BACKUP          ".BAK"



#endif /* MS_DOS */
#endif /* _SEC_FILENAMES_ */
