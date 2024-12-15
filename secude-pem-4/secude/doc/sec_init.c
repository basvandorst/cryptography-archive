{\scriptsize
\begin{verbatim}
/*------------------------------------------------------------------*/
/* FILE  sec_init.c                                                 */
/* Initialization of global variables of Secure-IF                  */
/*------------------------------------------------------------------*/

#include "secure.h"

char *secude_version = SECUDEVERSION;

static char       error_text[128];
struct ErrStack * err_stack = (struct ErrStack * ) 0;
struct ErrStack   err_malloc;

typedef struct {
        int             number;
        int             data;
        char           *addr;
} SecError;

SecError        sec_error = {0, 0, error_text};        /* number, data, addr */
int             sec_error_print = 0;


/*
 *    ObjectIdentifier macro (parameter alg) builds
 *               ObjId <alg>_oid;
 *
 *    AlgIdWithParm macro (parameter alg) builds
 *               AlgId <alg>_aid;
 *               AlgId *<alg> = &<alg>_aid;
 *    with <alg>_oid and <alg>_dfl_parm
 *
 *    AlgIdWithoutParm macro (parameter alg) builds
 *               AlgId <alg>_aid;
 *               AlgId *<alg> = &<alg>_aid;
 *    with <alg>_oid and no parameter
 *
 *    Example rsa:
 *    ------------
 *    AlgIdWithParm(rsa) builds
 *
 *    unsigned int  rsa_oid_elements[] = { 2, 5, 8, 1, 1 };
 *    ObjId   rsa_oid  = { sizeof(rsa_oid_elements)/sizeof(int),
 *                               rsa_oid_elements };
 *    rsa_parm_type rsa_parm = 512;
 *    AlgId         rsa_aid  = { &rsa_oid, (char *)(&rsa_parm) };
 *    AlgId        *rsa      = &rsa_aid;
 *
 *
 *    Example alg:
 *    ------------
 *    AlgIdWithoutParm(alg) builds
 *
 *    unsigned int  alg_oid_elements[] = { 2, 5, 8, 2, 3 };
 *    ObjId   alg_oid  = { sizeof(alg_oid_elements)/sizeof(int),
 *                               alg_oid_elements };
 *    AlgId         alg_aid  = { &alg_oid, (char *)0 };
 *    AlgId        *alg      = &alg_aid;
 */


#if defined(MAC) || defined(__HP__) || defined(__STDC__)
#define ObjectIdentifier(alg)                                             \
        unsigned int alg##_oid_elements[] = alg##_OID;                    \
        ObjId alg##_oid = {                                               \
        sizeof(alg##_oid_elements)/sizeof(int), alg##_oid_elements };

#define AlgIdWithParm(alg)                                                \
        ObjectIdentifier(alg)                                             \
        alg##_parm_type alg##_parm = alg##_dfl_parm;                      \
        AlgId alg##_aid = { &alg##_oid, (char *)(&alg##_parm) } ;         \
        AlgId *alg = &alg##_aid

#define AlgIdWithoutParm(alg)                                             \
        ObjectIdentifier(alg)                                             \
        AlgId alg##_aid = { &alg##_oid, (char *)0 } ;                     \
        AlgId *alg = &alg##_aid
#else
#define ObjectIdentifier(alg)                                             \
        unsigned int alg/**/_oid_elements[] = alg/**/_OID;                \
        ObjId alg/**/_oid = {                                             \
        sizeof(alg/**/_oid_elements)/sizeof(int), alg/**/_oid_elements };

#define AlgIdWithParm(alg)                                                \
        ObjectIdentifier(alg)                                             \
        alg/**/_parm_type alg/**/_parm = alg/**/_dfl_parm;                \
        AlgId alg/**/_aid = { &alg/**/_oid, (char *)(&alg/**/_parm) } ;   \
        AlgId *alg = &alg/**/_aid;

#define AlgIdWithoutParm(alg)                                             \
        ObjectIdentifier(alg)                                             \
        AlgId alg/**/_aid = { &alg/**/_oid, (char *)0 } ;                 \
        AlgId *alg = &alg/**/_aid;
#endif /* MAC */

/*
 *  ObjectIdentifier in <algorithm>_OID
 */


/* X.509 defined */

#define rsa_OID                         { 2, 5, 8, 1, 1 }
#define sqmodn_OID                      { 2, 5, 8, 2, 1 }
#define sqmodnWithRsa_OID               { 2, 5, 8, 3, 1 }

/* OIW defined */

#define md2WithRsa_OID                  { 1, 3, 14, 7, 2, 3, 1 }
#define md4WithRsa_OID                  { 1, 3, 14, 3, 2, 2 }
#define md5WithRsa_OID                  { 1, 3, 14, 3, 2, 3 }
#define md4WithRsaEncryption_OID        { 1, 3, 14, 3, 2, 4 }
#define desECB_OID                      { 1, 3, 14, 3, 2, 6 }
#define desCBC_OID                      { 1, 3, 14, 3, 2, 7 }
#define desEDE_OID                      { 1, 3, 14, 3, 2, 17 }
#define dsa_OID                         { 1, 3, 14, 3, 2, 12 }
#define sha_OID                         { 1, 3, 14, 3, 2, 18 }
#define dsaWithSHA_OID                  { 1, 3, 14, 3, 2, 13 }
#define dhWithCommonModulus_OID         { 1, 3, 14, 3, 2, 16 }


/* PKCS or PEM defined */

#define md2_OID                         { 1, 2, 840, 113549, 2, 2 }
#define md4_OID                         { 1, 2, 840, 113549, 2, 4 }
#define md5_OID                         { 1, 2, 840, 113549, 2, 5 }
#define rsaEncryption_OID               { 1, 2, 840, 113549, 1, 1, 1 }
#define md2WithRsaEncryption_OID        { 1, 2, 840, 113549, 1, 1, 2 }
#define md5WithRsaEncryption_OID        { 1, 2, 840, 113549, 1, 1, 4 }
#define dhKeyAgreement_OID              { 1, 2, 840, 113549, 1, 3, 1 }

/* DES algorithms TTT defined */

#define desCBC_pad_OID                  { 1, 3, 36, 3, 1, 5 }
#define desCBC3_OID                     { 1, 3, 36, 3, 1, 11 }
#define desCBC3_pad_OID                 { 1, 3, 36, 3, 1, 13 }
#define desCBC_ISO0_OID                 { 1, 3, 36, 3, 1, 1, 4, 1, 2 }
#define desCBC_ISO1_OID                 { 1, 3, 36, 3, 1, 1, 4, 1, 1 }
#define desCBC3_ISO0_OID                { 1, 3, 36, 3, 1, 3, 4, 1, 2 }
#define desCBC3_ISO1_OID                { 1, 3, 36, 3, 1, 3, 4, 1, 1 }
#define md2WithRsaTimeDate_OID          { 1, 3, 36, 3, 1, 22 }
#define md4WithRsaTimeDate_OID          { 1, 3, 36, 3, 1, 24 }
#define md5WithRsaTimeDate_OID          { 1, 3, 36, 3, 1, 25 }

/* other algorithms TTT defined */

#define idea_OID                        { 1, 3, 36, 3, 1, 30 }  /* this ist a temporary OID */
#define dsaSK_OID                       { 1, 3, 36, 3, 1, 20 }  /* this is only used locally to store
                                                                   a secret DSA key with keysize */


/*
 *  Default-Parameter in <algorithm>_dfl_parm
 */

#define rsa_dfl_parm            512        /* KeySize               */
#define dsaSK_dfl_parm          512        /* KeySize               */
#define sqmodn_dfl_parm         512        /* BlockSize             */
#define sqmodnWithRsa_dfl_parm  512        /* KeyAndBlockSize       */

#define desCBC_dfl_parm         { 0, "" }        /* Initialization vector */
#define desCBC_pad_dfl_parm     { 0, "" }        /* Initialization vector */
#define desCBC3_dfl_parm        { 0, "" }        /* Initialization vector */
#define desCBC3_pad_dfl_parm    { 0, "" }        /* Initialization vector */
/* STARCOS */
#define desCBC_ISO0_dfl_parm    { 0, "" }        
#define desCBC_ISO1_dfl_parm    { 0, "" }        
#define desCBC3_ISO0_dfl_parm   { 0, "" }        
#define desCBC3_ISO1_dfl_parm   { 0, "" }        
/* STARCOS - ENDE */


#define dhKeyAgreement_dfl_parm    {{ 0, "" }, { 0, "" }, { 0, "" }, { 0, "" } }       /* p, a */

/*
 *  Generate for each algorithm <alg> the global variables
 *               ObjId <alg>_oid
 *               AlgId <alg>_dfl
 *               AlgId *<alg>    (address of <alg>_dfl)
 */


AlgIdWithParm(rsa);
AlgIdWithParm(sqmodn);
AlgIdWithParm(sqmodnWithRsa);
AlgIdWithParm(dsaSK);

AlgIdWithoutParm(md2);
AlgIdWithoutParm(md4);
AlgIdWithoutParm(md5);
AlgIdWithoutParm(md2WithRsa);
AlgIdWithoutParm(md4WithRsa);
AlgIdWithoutParm(md5WithRsa);
AlgIdWithoutParm(sha);
AlgIdWithoutParm(dsa);
AlgIdWithoutParm(dsaWithSHA);

AlgIdWithoutParm(rsaEncryption);
AlgIdWithoutParm(md2WithRsaEncryption);
AlgIdWithoutParm(md4WithRsaEncryption);
AlgIdWithoutParm(md5WithRsaEncryption);

AlgIdWithoutParm(md2WithRsaTimeDate);
AlgIdWithoutParm(md4WithRsaTimeDate);
AlgIdWithoutParm(md5WithRsaTimeDate);


AlgIdWithoutParm(idea);
AlgIdWithoutParm(dhWithCommonModulus);
AlgIdWithParm(dhKeyAgreement);

AlgIdWithoutParm(desECB);
AlgIdWithParm(desCBC);
AlgIdWithoutParm(desEDE);

AlgIdWithParm(desCBC_pad);
AlgIdWithParm(desCBC3);
AlgIdWithParm(desCBC3_pad);
/* STARCOS */
AlgIdWithParm(desCBC_ISO0);
AlgIdWithParm(desCBC_ISO1);
AlgIdWithParm(desCBC3_ISO0);
AlgIdWithParm(desCBC3_ISO1);
/* STARCOS - ENDE */


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
        { "DES-CBC-ISO0", &desCBC_ISO0_aid, PARM_OctetString,SYM_ENC, DES, NoAlgHash, WITH_B0_PADDING, CBC },
        { "DES-CBC-ISO1", &desCBC_ISO1_aid, PARM_OctetString,SYM_ENC, DES, NoAlgHash, WITH_B1_PADDING, CBC },
        { "DES3-CBC-ISO0",&desCBC3_ISO0_aid,PARM_OctetString,SYM_ENC, DES3,NoAlgHash, WITH_B0_PADDING, CBC },
        { "DES3-CBC-ISO1",&desCBC3_ISO1_aid,PARM_OctetString,SYM_ENC, DES3,NoAlgHash, WITH_B1_PADDING, CBC },
        { "dhKeyAgreement", &dhKeyAgreement_aid, PARM_KeyBits, KEY_AGREEMENT, NoAlgEnc, NoAlgHash, NoAlgSpecial, NoAlgMode } ,
        { "dhWithCommonModulus", &dhWithCommonModulus_aid, PARM_NULL, KEY_AGREEMENT, NoAlgEnc, NoAlgHash, NoAlgSpecial, NoAlgMode } ,
        { CNULL }
};

char *algenc_name[] = {
        "\0",
        "RSA",
        "DES",
        "DES3",
        "DSA",
        "IDEA",
        CNULL
};
char *alghash_name[] = {
        "\0",
        "SQMODN",
        "MD2",
        "MD4",
        "MD5",
        "SHA",
        CNULL
};
char *algtype_name[] = {
        "\0",
        "SYM_ENC",
        "ASYM_ENC",
        "HASH",
        "SIG",
        "KEY_AGREEMENT",
        CNULL
};

int public_modulus_length;
UTCTime *sec_SignatureTimeDate;

static unsigned char bdk[][8] = {
        { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 } ,
        { 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE } ,
        { 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F } ,
        { 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0 } ,
        { 0xE0, 0xFE, 0xE0, 0xFE, 0xF1, 0xFE, 0xF1, 0xFE } ,
        { 0x1F, 0xFE, 0x1F, 0xFE, 0x0E, 0xFE, 0x0E, 0xFE } ,
        { 0x01, 0xFE, 0x01, 0xFE, 0x01, 0xFE, 0x01, 0xFE } ,
        { 0x1F, 0x0E, 0x1F, 0xE0, 0x0E, 0xF1, 0x0E, 0xF1 } ,
        { 0x01, 0xE0, 0x01, 0xE0, 0x01, 0xF1, 0x01, 0xF1 } ,
        { 0x01, 0xE0, 0x01, 0xE0, 0x01, 0xF1, 0x01, 0xF1 } ,
        { 0xFE, 0xE0, 0xFE, 0xE0, 0xFE, 0xF1, 0xFE, 0xF1 } ,
        { 0xFE, 0x1F, 0xFE, 0x1F, 0xFE, 0x0E, 0xFE, 0x0E } ,
        { 0xFE, 0x01, 0xFE, 0x01, 0xFE, 0x01, 0xFE, 0x01 } ,
        { 0xFE, 0x1F, 0xFE, 0x1F, 0xFE, 0x0E, 0xFE, 0x0E } ,
        { 0xE0, 0x01, 0xE0, 0x01, 0xF1, 0x01, 0xF1, 0x01 } ,
        { 0x1F, 0x01, 0x1F, 0x01, 0x0E, 0x01, 0x0E, 0x01 } 
};
unsigned char  *bad_des_keys[] = {
        &bdk[0][0],
        &bdk[1][0],
        &bdk[2][0],
        &bdk[3][0],
        &bdk[4][0],
        &bdk[5][0],
        &bdk[6][0],
        &bdk[7][0],
        &bdk[8][0],
        &bdk[9][0],
        &bdk[10][0],
        &bdk[11][0],
        &bdk[12][0],
        &bdk[13][0],
        &bdk[14][0],
        &bdk[15][0]
};
int             no_of_bad_des_keys = sizeof(bad_des_keys) / sizeof(char *);

struct ErrList  err_list[] = {
        { EALGID, "Invalid Algorithm Identifier" } ,
        { EAPP, "Cannot select PSE" } ,
        { EAPPNAME, "PSE does not exist" } ,
        { ECREATEAPP, "Cannot create PSE (e.g. PSE already exists)" } ,
        { EOBJ, "Cannot select PSE object" } ,
        { EOBJNAME, "PSE Object does not exist" } ,
        { ECREATEOBJ, "Cannot create PSE object (e.g. object already exists)" } ,
        { EPIN, "Invalid PIN" } ,
        { EVERIFY, "" } ,
        { ESYSTEM, "System call failed inside routine" } ,
        { EINVALID, "Invalid argument" } ,
        { EDAMAGE, "Toc of PSE not readable or PSE inconsistent" } ,
        { EMALLOC, "Unable to allocate new memory" } ,
        { EDECRYPT, "Wrong state of sec_decrypt" } ,
        { EENCRYPT, "Wrong state of sec_encrypt" } ,
        { EHASH, "Wrong state of sec_hash" } ,
        { EENCODE, "ASN.1-encoding error" } ,
        { EDECODE, "ASN.1-decoding error" } ,
        { ESIGN, "Wrong state of sec_sign" } ,
        { EVERIFICATION, "Verification unsuccessful" } ,
        { EACCPSE, "PSE or PSE object cannot be accessed (create/open/close)" } ,
        { EREADPSE, "Object value cannot be read from PSE" } ,
        { EWRITEPSE, "Object value cannot be written on PSE" } ,
        { EPATH, "Certification path incomplete" } ,
        { ECHKREVLIST, "Check of revocation list failed" } ,
        { ETOC, "Cannot read TOC of PSE" } ,
        { ESECOPEN, "Cannot open " } ,
        { EVALIDITY, "Invalid validity date of certificate" } ,
        { EPK, "PK already exists in PKList" } ,
        { ENAME, "Name already exists in PKList" } ,
        { ENOPK, "PK not found in PKList" } ,
        { ENONAME, "Name not found in PKList" } ,
        { EROOTKEY, "Highest verification key not available" } ,
        { ENODIR, "Directory service does not respond" } ,
        { ENAMEDIR, "No object assigned to this name found in directory" } ,
        { EACCDIR, "Directory access rights not sufficient for requested operation" } ,
        { EATTRDIR, "No directory entry of requested attribute type found in directory" } ,
        { EUPDATE, "Cannot update directory entry" } ,
        { EPARSE, "Cannot parse directory attribute" } ,
        { EPKCROSS, "Cross certificate for same PK already exists in PKList" } ,
        { EPOINTER, "" } ,
        { EINTEGER, "" } ,
        { ERETURN, "" } ,
        { EMSGBUF, "" } ,
        { EMIC, "" } ,
        { ECTFOWNER, "" } ,
        { EMYNAME, "" } ,
        { EENCRMIC, "Can't encrypt MIC" } ,
        { EENCRBODY, "Can't encrypt body" } ,
        { EDECRMIC, "Can't decrypt MIC" } ,
        { EDECRBODY, "Can't decrypt body" } ,
        { EPEMBOUND, "Missing PEM Boundary line" } ,
        { ESC, "SCT-/SC-ERROR:  " } ,
        { EDEVLOCK, "Cannot open device for SCT (No such device or device busy)" } ,
        { ENOCRL, "No CRL of this issuer found" } ,
        { EWRITEFILE, "Can't write file" } ,
        { EREADFILE, "Can't read file" } ,
        { ETTY, "Can't open device for user input" } ,
        { ELOGFILE, "Can't open log file" } ,
        { ENORECIP, "You are not on the recipients list" } ,
        { EDECRYPTION, "Wrong key used for decyption" } ,
        { 0, "Error code unknown" }
};


unsigned char        print_keyinfo_flag = ALGID;
unsigned int         print_cert_flag = TBS | ALG | SIGNAT;
unsigned int        print_indent = 0;

Boolean    sec_verbose = FALSE;
Boolean    sec_gen_verbose = FALSE;

/* ASN.1 length encoding of e_*() functions */

int     sec_asn1_length_encoding = DEFINITE;
int     sec_debug = 0;
int     sec_dsa_keysize = DEF_ASYM_KEYSIZE;
Boolean sec_dsa_predefined = FALSE;
Boolean sec_onekeypair = TRUE;
Boolean sec_time = 0;

char *proc = "??";
Boolean MF_check = FALSE;


#ifdef SCA
Boolean sec_sca = TRUE;
#else
Boolean sec_sca = FALSE;
#endif


char *input_device = "/dev/tty"; /* device to read inputs from */
\end{verbatim}
}
