/*
 *  SecuDE Release 4.3 (GMD)
 */
/********************************************************************
 * Copyright (C) 1994, GMD. All rights reserved.                    *
 *                                                                  *
 *                                                                  *
 *                         NOTICE                                   *
 *                                                                  *
 *    Acquisition, use, and distribution of this module             *
 *    and related materials are subject to restrictions             *
 *    mentioned in each volume of the documentation.                *
 *                                                                  *
 ********************************************************************/

/*-----------------------------------------------------------------------*/
/* INCLUDE FILE  secure.h                                                */
/* Definition of structures and types for the basic security functions   */
/*-----------------------------------------------------------------------*/

#ifndef _SECURE_
#define _SECURE_

#include <string.h>
#if defined (ICR1)
#include "ul/psap.h"
#else
#include "psap.h"
#endif

/* Declaration of build- and parse-routines */
#include "SEC-types.h"


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define CNULL (char *)0

#ifndef NULL
#define NULL        (void *)0
#endif

#ifdef __HP__

#include <sys/types.h>
#define timelocal        mktime                        /* correct time zone */

#endif

#include <sys/time.h>
struct timeval sec_tp1, sec_tp2;
struct timezone sec_tzp1, sec_tzp2;
long hash_sec, hash_usec, rsa_sec, rsa_usec, dsa_sec, dsa_usec, des_sec, des_usec, idea_sec, idea_usec;

/*
 *    SecuDe Version
 */

extern char *secude_version;

#if defined(MAC) || defined(__STDC__)
#define SET_OF(t) struct set_of_##t {               \
        t *element;                                 \
        struct set_of_##t *next;                    \
}
#define SEQUENCE_OF(t) struct sequence_of_##t {     \
        t *element;                                 \
        struct sequence_of_##t *next;               \
}
#else
#define SET_OF(t) struct set_of_/**/t {             \
        t *element;                                 \
        struct set_of_/**/t *next;                  \
}
#define SEQUENCE_OF(t) struct sequence_of_/**/t {   \
        t *element;                                 \
        struct sequence_of_/**/t *next;             \
}
#endif /* MAC */

/*-----------------------------------------------------------------------*/
/*    T y p e d e f ' s   f o r   s e c                                  */
/*-----------------------------------------------------------------------*/

/* MPW-C defines 'Boolean', too */
#ifdef applec
#include <types.h>
#else
/* X11/Intrinsic.h defines 'Boolean', too */
#ifndef _XtIntrinsic_h
typedef char                    Boolean;
#endif
#endif

typedef int                     RC;
typedef char                    UTCTime;
typedef enum { SEC_END, SEC_MORE }  More;

typedef struct OctetString      OctetString;
typedef SET_OF(OctetString)     SET_OF_OctetString;

typedef struct BitString        BitString;
typedef struct BitString        ENCRYPTED;
#ifdef _PSAP_
typedef OIDentifier             ObjId;
#else
typedef struct OIDentifier      ObjId, OIDentifier;
#endif
typedef SET_OF(ObjId)           SET_OF_ObjId;
typedef struct AlgId            AlgId;
typedef SET_OF(AlgId)           SET_OF_AlgId;
typedef struct AlgList          AlgList;
typedef struct KeyInfo          KeyInfo;
typedef struct DigestInfo       DigestInfo;
typedef struct Key              Key;
typedef struct KeyBits          KeyBits;
typedef struct EncryptedKey     EncryptedKey;  /* e.g. for rsa-encrypted 
                                                  DES-keys               */ 
typedef struct Signature        Signature; 
typedef int                     KeyRef;
typedef struct PSE_Sel          PSESel;
typedef struct PSE_Toc          PSEToc;
typedef struct PSE_Object       PSEObject;

typedef struct ErrStack         ErrStack;

#ifndef _ARITHMETIC_
#ifndef MAC
typedef unsigned  L_NUMBER;
#else
typedef unsigned long L_NUMBER;
#endif /* MAC */
#endif


#define NULLOBJID ((ObjId *) 0)
#define NULLALGID ((AlgId *) 0)
#define NULLOCTETSTRING ((OctetString *) 0)
#define NULLBITSTRING   ((BitString *) 0)

/*-----------------------------------------------------------------------*/
/*    E r r o r s                                                        */
/*-----------------------------------------------------------------------*/

#define EALGID          1
#define EAPP            2
#define EAPPNAME        3
#define ECREATEAPP      4
#define EOBJ            5
#define EOBJNAME        6
#define ECREATEOBJ      7
#define EPIN            8
#define EVERIFY         9
#define ESYSTEM         10
#define EINVALID        11
#define EDAMAGE         12
#define EMALLOC         13
#define EDECRYPT        14
#define EENCRYPT        15
#define EHASH           16
#define EENCODE         17
#define EDECODE         18
#define ESIGN           19
#define EVERIFICATION   20
#define EACCPSE         21
#define EREADPSE        22
#define EWRITEPSE       23
#define EPATH           24
#define ECHKREVLIST     25
#define ETOC            26
#define ESECOPEN       100
#define EVALIDITY      101
#define EPK            102
#define ENAME          103
#define ENOPK          104
#define ENONAME        105
#define EROOTKEY       106
#define ENODIR         107
#define ENAMEDIR       108
#define EACCDIR        109
#define EATTRDIR       110
#define EUPDATE        111
#define EPARSE         112
#define EPKCROSS       113
#define EREVOKE        114
#define EAVAILABLE     115
#define EPOINTER       202
#define EINTEGER       203
#define ERETURN        204
#define EMSGBUF        205
#define EMIC           206
#define ECTFOWNER      207
#define EMYNAME        208
#define EENCRMIC       209
#define EENCRBODY      210
#define EDECRMIC       211
#define EDECRBODY      212
#define EPEMBOUND      213
#define ESC            214
#define ESIGNATURE     215
#define EDEVLOCK       216
#define ENOCRL         217
#define EWRITEFILE     218
#define EREADFILE      219
#define ETTY           220
#define ELOGFILE       221
#define ENORECIP       222
#define EDECRYPTION    223

#ifdef SCA
#define ESCA	       250	/* Indentifies the error no/text from 	*/
				/* the SCA-IF 				*/
#define ESCAUTH        251	/* authentication failed		*/
#define ECONFIG        252	/* Error in SC-Obj-list or  		*/
				/* error in configuration file 		*/
#define ESCNOTSUPP     253	/* Function not supported by the SC-Env	*/
#define ESCPINLOCK     254	/* PIN on SC locked			*/
#define ESCPININV      255	/* New PIN value invalid 		*/
				/* (sca_unblock_pin)			*/
#define ESCPUKLOCK     256	/* PUK on SC locked			*/
#define ESCPROCDATA    257	/* Error in the SCT configuration data  */
				/* generated by a previous process. The */
				/* file is deleted and the SCT 		*/
				/* configuration is initialized.	*/
#define ESCDEVKEY      258	/* Device Key unknown or invalid	*/
#define EPSEPIN	       259	/* Cannot get PIN for SW-PIN from SC	*/
#define ESCTINST       260	/* Environment variable STAMOD not def.,*/
				/* SCT installation file missing or     */
				/* invalid or sct_id unknown		*/	
#endif


#define LASTERROR      (err_stack ? err_stack->e_number : 0)
#define LASTTEXT       (err_stack ? err_stack->e_text : CNULL)
#define LASTADDR       err_stack ? err_stack->e_addr : CNULL, err_stack ? err_stack->e_addrtype : 0

#define AUX_ADD_ERROR aux_add_error(LASTERROR, LASTTEXT, LASTADDR, proc)
#define AUX_ADD_WARNING aux_add_warning(LASTERROR, LASTTEXT, LASTADDR, proc)
#define AUX_ERROR_RETURN(error, do) { AUX_ADD_ERROR; do;return error;}
#define AUX_MALLOC_ERROR(error, do) { aux_add_error(EMALLOC, "Can't allocate memory", CNULL, 0, proc);do; return error;}

#define NEWSTRUCT(var, type, error, do) {if(!(var = (type *)calloc(1, sizeof(type)))) AUX_MALLOC_ERROR(error, do) }
#define NEWSTRING(var, size, error, do) {if(!(var = (char *)malloc(size+1))) AUX_MALLOC_ERROR(error, do) }

struct ErrList {
        int  id;
        char *msg;
};
typedef enum{
        char_n,
        DName_n,
        OctetString_n,
        BitString_n,                       
        Certificate_n,
        Certificates_n,
        CertificatePair_n,
        PKList_n,
        SET_OF_Certificate_n,
        SET_OF_CertificatePair_n,
        OCList_n,
        AlgId_n,
        CRLTBS_n,
        CRL_n,
        CRLEntry_n,
        CrlSet_n,
        Crl_n,
        PemInfo_n,
        KeyInfo_n,
        FCPath_n,
        PKRoot_n,
        IssuedCertificate_n,
        SET_OF_IssuedCertificate_n,
        SET_OF_Name_n,
        ToBeSigned_n,
        ObjId_n,
        KeyBits_n,
        PSEToc_n,
        PSESel_n,
        CRLWithCertificates_n,
        SET_OF_CRLWithCertificates_n,
        int_n
} Struct_No;

struct ErrStack{
        Boolean          e_is_error;
        int              e_number;
        char            *e_text;
        char            *e_addr;
        Struct_No        e_addrtype;
        char            *e_proc;
        struct ErrStack *next;
} ;
extern struct ErrList  err_list[];
extern struct ErrStack *err_stack;
extern struct ErrStack err_malloc;

/*-----------------------------------------------------------------------*/
/*    B i t s t r i n g   and   O c t e t s t r i n g                    */
/*-----------------------------------------------------------------------*/

struct OctetString {
        unsigned int    noctets;
        char           *octets;
};

struct BitString {
        unsigned int    nbits;
        char           *bits;
};

/*-----------------------------------------------------------------------*/
/*    O I D ' s  and  A L G I d 's                                       */
/*-----------------------------------------------------------------------*/

/*
 *    Parameter types (parmtype member of struct AlgList, 
 *    returned by aux_ObjId2ParmType())
 */

typedef enum {
        NoParmType = -1,
        PARM_ABSENT,
        PARM_INTEGER,
        PARM_OctetString,
        PARM_KeyBits,
        PARM_NULL
} ParmType;

/*
 *    Algorithm types (algtype member of struct AlgList, 
 *    returned by aux_ObjId2AlgType())
 */

typedef enum { 
        NoAlgType = -1,
        SYM_ENC, 
        ASYM_ENC, 
        HASH, 
        SIG, 
        KEY_AGREEMENT
} AlgType;

extern char *algtype_name[];

/*
 *    Encryption method of algorithm (algenc member of struct AlgList, 
 *    returned by aux_ObjId2AlgEnc())
 */

typedef enum { 
        NoAlgEnc = -1,
        RSA, 
        DES, 
        DES3,
        DSA,
        IDEA
} AlgEnc;

extern char *algenc_name[];

/*
 *    Encryption mode of algorithm (algmode member of struct AlgList, 
 *    returned by aux_ObjId2AlgMode())
 */

typedef enum { 
        NoAlgMode = -1,
        ECB, 
        CBC
} AlgMode;

/*
 *    Hash method of algorithm (alghash member of struct AlgList, 
 *    returned by aux_ObjId2AlgHash())
 */

typedef enum { 
        NoAlgHash = -1,
        SQMODN, 
        MD2, 
        MD4,
        MD5,
        SHA
} AlgHash;

extern char *alghash_name[];

typedef enum { 
        NoAlgSpecial = -1,
        PKCS_BT_01 = 1,
        PKCS_BT_02,
        WITH_PADDING,
        WITH_PEM_PADDING,
        PKCS_BT_TD,
        WITH_B0_PADDING,
        WITH_B1_PADDING
} AlgSpecial;

#ifndef _PSAP_
struct OIDentifier {
        int             oid_nelem;
        unsigned int   *oid_elements;
};
#endif


/*
 *    Algorithm parameter types
 */

#define null_parm  0

typedef unsigned int            rsa_parm_type;
/* STARCOS */
typedef unsigned int            rsa_B1_parm_type;
/* STARCOS - ENDE */
typedef unsigned int            dsaSK_parm_type;
typedef unsigned int            sqmodn_parm_type;
typedef unsigned int            sqmodnWithRsa_parm_type;
typedef struct OctetString      desCBC_parm_type;
typedef struct OctetString      desCBC_pad_parm_type;
typedef struct OctetString      desCBC3_parm_type;
typedef struct OctetString      desCBC3_pad_parm_type;
/* STARCOS */
typedef struct OctetString      desCBC_ISO0_parm_type;
typedef struct OctetString      desCBC_ISO1_parm_type;
typedef struct OctetString      desCBC3_ISO0_parm_type;
typedef struct OctetString      desCBC3_ISO1_parm_type;
/* STARCOS - ENDE */

typedef struct KeyBits          dhKeyAgreement_parm_type;


struct AlgId {
        ObjId          * objid;
        char           * param;
        union {
                int                keyorblocksize;
                desCBC_parm_type * des_parm;
                KeyBits          * diffie_hellmann_parm;
        } un;
        int              paramchoice;
};

struct AlgList {
        char           *name;
        AlgId          *algid;
        ParmType       parmtype;
        AlgType        algtype;
        AlgEnc         algenc;
        AlgHash        alghash;
        AlgSpecial     algspecial;
        AlgMode        algmode;
};


#define DEF_ASYM_KEYSIZE 512
#define MIN_ASYM_KEYSIZE 256
#define MAX_ASYM_KEYSIZE 2048

extern int public_modulus_length;
extern UTCTime *sec_SignatureTimeDate;

#define RSA_PARM(x) (( x ? *(int *)(x) : (public_modulus_length ? public_modulus_length : DEF_ASYM_KEYSIZE)))

#define DES_PARM(x) ((OctetString *) (x))
                 

/* AlgId addresses */

extern AlgId    *rsa;
extern AlgId    *sqmodn;
extern AlgId    *sqmodnWithRsa;
extern AlgId    *dsaSK;

extern AlgId    *idea;

extern AlgId    *md2;
extern AlgId    *md4;
extern AlgId    *md5;
extern AlgId    *md2WithRsa;
extern AlgId    *md4WithRsa;
extern AlgId    *md5WithRsa;
extern AlgId    *sha;
extern AlgId    *dsa;
extern AlgId    *dsaWithSHA;

extern AlgId    *desECB;
extern AlgId    *desCBC;
extern AlgId    *desEDE;

extern AlgId    *desCBC_pad;
extern AlgId    *desCBC3;
extern AlgId    *desCBC3_pad;

extern AlgId    *rsaEncryption;
extern AlgId    *md2WithRsaEncryption;
extern AlgId    *md4WithRsaEncryption;
extern AlgId    *md5WithRsaEncryption;

extern AlgId    *md2WithRsaTimeDate;
extern AlgId    *md4WithRsaTimeDate;
extern AlgId    *md5WithRsaTimeDate;


extern AlgId    *dhKeyAgreement;
extern AlgId    *dhWithCommonModulus;

/* STARCOS - AlgId addresses */
extern AlgId    *desCBC_ISO0;   
extern AlgId    *desCBC_ISO1;   
extern AlgId    *desCBC3_ISO0;  
extern AlgId    *desCBC3_ISO1;
extern AlgId    *rsa_B1;        /* noch klaeren und zu benennen */
/* STARCOS - ENDE */



/*
 *     External initialization of the known AlgId's:
 *


AlgId *                 ObjectIdentifier               Parameter

 rsa                    { 2, 5, 8, 1, 1 }              INTEGER (default 512)
 sqmodn                 { 2, 5, 8, 2, 1 }              INTEGER (default 512)
 sqmodnWithRsa          { 2, 5, 8, 3, 1 }              INTEGER (default 512)
 md2                    { 1, 2, 840, 113549, 2, 2 }    NULL
 md4                    { 1, 2, 840, 113549, 2, 4 }    NULL
 md5                    { 1, 2, 840, 113549, 2, 5 }    NULL
 md2WithRsa             { 1, 3, 14, 7, 2, 3, 1 }       NULL
 md4WithRsa             { 1, 3, 14, 3, 2, 2 }          NULL
 md5WithRsa             { 1, 3, 14, 3, 2, 3 }          NULL
 dsa                    { 1, 3, 14, 3, 2, 12 }         NULL
 sha                    { 1, 3, 14, 3, 2, 18 }         NULL 
 dsaSK                  { 1, 3, 36, 3, 1, 20 }         INTEGER (default 512)
 dsaWithSHA             { 1, 3, 14, 3, 2, 13 }         NULL
 idea                   { 1, 3, 36, 3, 1, 30 }         NULL
 desECB                 { 1, 3, 14, 3, 2, 6 }          NULL
 desCBC                 { 1, 3, 14, 3, 2, 7 }          {0, ""}
 desEDE                 { 1, 3, 14, 3, 2, 17 }         NULL
 desCBC_pad             { 1, 3, 36, 3, 1, 5 }          {0, ""}
 desCBC3                { 1, 3, 36, 3, 1, 11 }         {0, ""}
 desCBC3_pad            { 1, 3, 36, 3, 1, 13 }         {0, ""}
 rsaEncryption          { 1, 2, 840, 113549, 1, 1, 1 } NULL
 md2WithRsaEncryption   { 1, 2, 840, 113549, 1, 1, 2 } NULL
 md4WithRsaEncryption   { 1, 3, 14, 3, 2, 4 }          NULL
 md5WithRsaEncryption   { 1, 2, 840, 113549, 1, 1, 4 } NULL
 md2WithRsaTimeDate     { 1, 3, 36, 3, 1, 22 }         NULL
 md4WithRsaTimeDate     { 1, 3, 36, 3, 1, 24 }         NULL
 md5WithRsaTimeDate     { 1, 3, 36, 3, 1, 25 }         NULL
 DhKeyAgreement         { 1, 2, 840, 113549, 1, 3, 1 } {{ 0, "" }, { 0, "" }, { 0, "" }, { 0, "" } }
 dhWithCommonModulus    { 1, 3, 14, 3, 2, 16 }         NULL

STARCOS
 desCBC_ISO0            { 1, 3, 36, 3, 1, 1, 4, 1, 2 } {0, ""}
 desCBC_ISO1            { 1, 3, 36, 3, 1, 1, 4, 1, 1 } {0, ""}
 desCBC3_ISO0           { 1, 3, 36, 3, 1, 3, 4, 1, 2 } {0, ""}
 desCBC3_ISO1           { 1, 3, 36, 3, 1, 3, 4, 1, 1 } {0, ""}

STARCOS - ENDE
 */

/*
 *     List of all known algorithms
 */

extern AlgList  alglist[];

/*
 * External initialization of alglist[]:
 *  
AlgList         alglist[] = {
        { "DES-ECB", &desECB_aid, PARM_NULL, SYM_ENC, DES, NoAlgHash, NoAlgSpecial,ECB } ,
        { "DES-CBC", &desCBC_aid, PARM_OctetString, SYM_ENC, DES, NoAlgHash, WITH_PEM_PADDING,CBC } ,
        { "DES-EDE",  &desEDE_aid, PARM_NULL, SYM_ENC, DES3, NoAlgHash, NoAlgSpecial,ECB } ,
        { "RSA-MD2", &md2_aid, PARM_NULL, HASH, NoAlgEnc, MD2, NoAlgSpecial,NoAlgMode } ,
        { "RSA-MD4", &md4_aid, PARM_NULL, HASH, NoAlgEnc, MD4, NoAlgSpecial,NoAlgMode } ,
        { "RSA-MD5", &md5_aid, PARM_NULL, HASH, NoAlgEnc, MD5, NoAlgSpecial,NoAlgMode } ,
        { "RSA", &rsaEncryption_aid, PARM_NULL, ASYM_ENC, RSA, NoAlgHash, PKCS_BT_02,NoAlgMode } ,
        { "NIST-SHA", &sha_aid, PARM_NULL, HASH, NoAlgEnc, SHA, NoAlgSpecial,NoAlgMode } ,
        { "NIST-DSA", &dsa_aid, PARM_NULL, ASYM_ENC, DSA, NoAlgHash, NoAlgSpecial,NoAlgMode } ,
        { "rsa", &rsa_aid, PARM_INTEGER, ASYM_ENC, RSA, NoAlgHash, NoAlgSpecial,NoAlgMode } ,
        { "sqmodn", &sqmodn_aid, PARM_INTEGER, HASH, NoAlgEnc, SQMODN, NoAlgSpecial,NoAlgMode } ,
        { "sqmodnWithRsa", &sqmodnWithRsa_aid, PARM_INTEGER, SIG, RSA, SQMODN, NoAlgSpecial,NoAlgMode } ,
        { "md2", &md2_aid, PARM_NULL, HASH, NoAlgEnc, MD2, NoAlgSpecial,NoAlgMode } ,
        { "md4", &md4_aid, PARM_NULL, HASH, NoAlgEnc, MD4, NoAlgSpecial,NoAlgMode } ,
        { "md5", &md5_aid, PARM_NULL, HASH, NoAlgEnc, MD5, NoAlgSpecial,NoAlgMode } ,
        { "md2WithRsa", &md2WithRsa_aid, PARM_NULL, SIG, RSA, MD2, NoAlgSpecial,NoAlgMode } ,
        { "md4WithRsa", &md4WithRsa_aid, PARM_NULL, SIG, RSA, MD4, NoAlgSpecial,NoAlgMode } ,
        { "md5WithRsa", &md5WithRsa_aid, PARM_NULL, SIG, RSA, MD5, NoAlgSpecial,NoAlgMode } ,
        { "sha", &sha_aid, PARM_NULL, HASH, NoAlgEnc, SHA, NoAlgSpecial,NoAlgMode } ,
        { "dsa", &dsa_aid, PARM_NULL, ASYM_ENC, DSA, NoAlgHash, NoAlgSpecial,NoAlgMode } ,
        { "dsaSK", &dsaSK_aid, PARM_INTEGER, ASYM_ENC, DSA, NoAlgHash, NoAlgSpecial,NoAlgMode } ,
        { "dsaWithSHA", &dsaWithSHA_aid, PARM_NULL, SIG, DSA, SHA, NoAlgSpecial,NoAlgMode } ,
        { "desECB", &desECB_aid, PARM_NULL, SYM_ENC, DES, NoAlgHash, NoAlgSpecial,ECB } ,
        { "desCBC", &desCBC_aid, PARM_OctetString, SYM_ENC, DES, NoAlgHash, WITH_PEM_PADDING,CBC } ,
        { "desEDE", &desEDE_aid, PARM_NULL, SYM_ENC, DES3, NoAlgHash, NoAlgSpecial,ECB } ,
        { "desCBC_pad", &desCBC_pad_aid, PARM_OctetString, SYM_ENC, DES, NoAlgHash, WITH_PADDING,CBC } ,
        { "desECB3", &desEDE_aid, PARM_NULL, SYM_ENC, DES3, NoAlgHash, NoAlgSpecial,ECB } ,
        { "desCBC3", &desCBC3_aid, PARM_OctetString, SYM_ENC, DES3, NoAlgHash, WITH_PEM_PADDING,CBC } ,
        { "desCBC3_pad", &desCBC3_pad_aid, PARM_OctetString, SYM_ENC, DES3, NoAlgHash, WITH_PADDING,CBC } ,
        { "md2WithRsaEncryption", &md2WithRsaEncryption_aid, PARM_NULL, SIG, RSA, MD2, PKCS_BT_01,NoAlgMode } ,
        { "md4WithRsaEncryption", &md4WithRsaEncryption_aid, PARM_NULL, SIG, RSA, MD4, PKCS_BT_01,NoAlgMode } ,
        { "md5WithRsaEncryption", &md5WithRsaEncryption_aid, PARM_NULL, SIG, RSA, MD5, PKCS_BT_01,NoAlgMode } ,
        { "md2WithRsaTimeDate", &md2WithRsaTimeDate_aid, PARM_NULL, SIG, RSA, MD2, PKCS_BT_TD,NoAlgMode } ,
        { "md4WithRsaTimeDate", &md4WithRsaTimeDate_aid, PARM_NULL, SIG, RSA, MD4, PKCS_BT_TD,NoAlgMode } ,
        { "md5WithRsaTimeDate", &md5WithRsaTimeDate_aid, PARM_NULL, SIG, RSA, MD5, PKCS_BT_TD,NoAlgMode } ,
        { "IDEA", &idea_aid, PARM_NULL, SYM_ENC, IDEA, NoAlgHash, NoAlgSpecial, NoAlgMode } ,
        { "dhKeyAgreement", &dhKeyAgreement_aid, PARM_KeyBits, KEY_AGREEMENT, NoAlgEnc, NoAlgHash, NoAlgSpecial, NoAlgMode } ,
        { "dhWithCommonModulus", &dhWithCommonModulus_aid, PARM_NULL, KEY_AGREEMENT, NoAlgEnc, NoAlgHash, NoAlgSpecial, NoAlgMode } ,
        { CNULL }
};


*/

extern int sec_dsa_keysize;
extern Boolean sec_dsa_predefined;

/*
 *  Bad DES keys  (initialized in sec_init.c)
 */

extern unsigned char *bad_des_keys[];
extern int no_of_bad_des_keys;


/*
 *  HashInput
 */

typedef BitString       SQMODN_input;

typedef union Hashinput {
        SQMODN_input sqmodn_input;
} HashInput;

/*-----------------------------------------------------------------------*/
/*    P S E                                                              */
/*-----------------------------------------------------------------------*/

#include "filename.h"
 
struct PSE_Sel {
        char    *app_name;       /* Appl name or PSE name           */
        char    *pin;            /* PSE-PIN or password             */
        struct {
           char *name;
           char *pin;
        }        object;         /* object name and PIN             */
        int      app_id;         /* application id, 0 if default    */
};


/*
 *      Table of Contents of PSE
 */

struct PSE_Toc {
        char                    *owner;
        UTCTime                 *create;
        UTCTime                 *update;
        unsigned int            status;
        struct PSE_Objects      *obj;
};

struct PSE_Objects {
        char                    *name;
        UTCTime                 *create;
        UTCTime                 *update;
        int                     noOctets;
        unsigned int            status;
        struct PSE_Objects      *next;
};

struct PSE_Object {
        ObjId         *objectType;
        OctetString   *objectValue;
        PE             asn;
};


PSEToc *sec_read_toc();
int     sec_write_toc();



typedef enum {
	ERR_in_pseconfig = -1,
        NOT_ON_SC, 
        APP_ON_SC, 
        FILE_ON_SC, 
        KEY_ON_SC
} PSEConfig;

PSEConfig sec_pseconfig();


typedef enum {
	ERR_in_psetest = -1,
        SWpse = 0, 
        SCpse = 1 
} PSELocation;


typedef enum {
	ERR_in_scttest = -1,
        SCTDev_not_avail = 0, 
        SCTDev_avail = 1 
} SCTDevice;




/*-----------------------------------------------------------------------*/
/*    K e y ' s   and   S i g n a t u r e                                */
/*-----------------------------------------------------------------------*/

struct Signature {                   /* algorithm of subject's signature */
        AlgId     *signAI;
        BitString signature;
};

struct KeyInfo {
        AlgId     *subjectAI;
        BitString subjectkey;
};

struct DigestInfo {        /* DigestInfo as defined in PKCS#1 and PEM    */
        AlgId       *digestAI;
        OctetString digest;
};

struct KeyBits {           /* internal structure of BitString subjectkey
                              in case of RSA keys and DSA keys and of the 
                              signature BitString in case of DSA signatures.
                              In case of rsa keys only part1 and part 2 are used. 
                              In case of secret RSA keys, part1 and part2 
                              contain the prime numbers p and q.
                              In case of public RSA keys, part1 and part2
                              contain the modulus m and the exponent e.
                              In case of secret DSA keys, part1 to part4
                              contain x, p, q and g.
                              In case of public DSA keys, part1 to part4
                              contain y, p, q and g.
                              In case of a DSA signature part1 contains s
                              and part2 contains r.
                          */
        OctetString part1;
        OctetString part2;
        OctetString part3;
        OctetString part4;
        int choice;
};


struct EncryptedKey {
        AlgId     *encryptionAI;
        AlgId     *subjectAI;
        ENCRYPTED subjectkey;
};

struct Key {
        KeyInfo  *key;
        KeyRef    keyref;
        PSESel   *pse_sel;
        AlgId    *alg;
};

KeyRef sec_put_key();

/*-----------------------------------------------------------------------*/
/*  The key reference (integer) of the SEC-IF can be used to address     */
/*      - a key stored on the SC (DF-level) or                           */
/*      - a key stored in the SCT or                                     */
/*      - a key stored in the key pool (SW-PSE).                         */
/*  The following masks are used to indicate the address of the key.     */
/*-----------------------------------------------------------------------*/

#define        SC_KEY      0xFF010000  /* Address a key on the SC on DF-level   */
#define        SCT_KEY     0xFF020000  /* Address a key in the SCT              */
#define        KeyPool_KEY 0x00000000  /* Address a key in the keypool (SW-PSE) */




#include "sec_prototypes.h"
/*
 *  sec_asn1_length_encoding controls the ASN.1 length encoding of the 
 *  e_*() functions. DEFINITE or INDEFINITE encoding can be chosen.
 *  For the sake of distinguished ASN.1 encoding either of the two 
 *  methods must be agreed and fixed between all participants of the
 *  security infrastructure. X.509 says DEFINITE shall be used for
 *  DER encoding. sec_asn1_length_encoding is defined in sec_init.c
 *  and set to DEFINITE by default.
 *
 *  Don't change this parameter unless you are sure what you are doing, 
 *  otherwise the verification of certificates could yield some surprises.
 */ 

extern int sec_asn1_length_encoding;
#define INDEFINITE 1
#define DEFINITE   2

extern Boolean aux_localtime;   /* if TRUE, UTCTime is generated as local time
                                   else UTCTime is generated as GMT time  */

extern Boolean sec_verbose;     /* if FALSE, RSA stuff is silent          */
extern Boolean sec_gen_verbose; /* if FALSE, key generation is silent     */
extern unsigned char print_keyinfo_flag; /* controls aux_fprint_KeyInfo() : 
                                            ALGID:   AlgId
                                            BSTR:    DER-Code of BitString 
                                                     subjectkey
                                            KEYBITS: Components of subjectKey 
                                                     in case of RSA, DSA, DH */
extern unsigned int print_indent;

extern Boolean sec_time, sec_onekeypair;

#define ALGID     1
#define BSTR      2
#define KEYBITS   4
#define PK        8
#define SK       16

extern unsigned int print_cert_flag;    /* controls aux_fprint_Certificate() :           */
#define DER          1
#define TBS          2
#define KEYINFO      4
#define VAL          8
#define ISSUER      16
#define ALG         32
#define SIGNAT      64
#define HSH        128 
#define VER        256 
#define SUBJECT    512 

#define SHORT_HELP 0
#define LONG_HELP 1

#define ONEKEYPAIRONLY   1

#define MSBITINBYTE 0x80

#ifdef SCA
#include "secsc.h"
#endif
extern Boolean sec_sca;

#include "MF_check.h"

extern int sec_debug;


extern char *input_device; /* device to read inputs from */

extern Boolean random_init_from_tty;    /* if TRUE, init random number from TTY */
extern Boolean random_from_pse;         /* if TRUE, read random number from PSE */
extern Boolean random_update_always;    /* if TRUE, update random number of PSE after each request for a
                                           random number ( if random_from_pse == TRUE only ) */
extern Boolean random_destroy;          /* if TRUE, destroy random seed after each request for a
                                           random number  */
extern char *secudeversion;


#endif
