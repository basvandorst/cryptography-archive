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
/*  INCLUDE FILE  pem.h  (Privacy Enhanced Mail Interface)               */
/*  Definition of structures and types for PEM                           */
/*-----------------------------------------------------------------------*/

#ifndef _PEM_
#define _PEM_

/*
 *  P E M : Privacy Enhanced Mail (RFC 1421 - 1424) interface definition
 *
 */

#ifndef _AF_
#include "af.h"
#endif
#include <fcntl.h>
#include <stdio.h>

#define DEFAULT_MSG_ENC_ALG  "DES-CBC"
#define DEFAULT_MIC_ALG      "RSA-MD5"
#define DEFAULT_MIC_ENC_ALG  "RSA"
#define DEFAULT_DEK_ENC_ALG  "RSA"


/*
 * External initialization of PEM RFC 1421 - 1424 Definitions (pem_init.c):
 *
 *
 *
 * struct SKW  proc_type_v[] = {
 *         { "4",          PEM_4          },
 *         { 0,            0              }
 * };
 * 
 * struct SKW  proc_type_t[] = {
 *         { "ENCRYPTED",  PEM_ENC        },
 *         { "CRL",        PEM_CRL        },
 *         { "CRL-RETRIEVAL-REQUEST",  PEM_CRL_RETRIEVAL_REQUEST        },
 *         { "MIC-ONLY",   PEM_MCO        },
 *         { "MIC-CLEAR",  PEM_MCC        },
 *         { 0,            NO_PEM         }
 * };
 * 
 * struct SKW content_domain[] = {
 *         { "RFC822",     PEM_RFC822     },
 *         { "MIME",       PEM_MIME       },
 *         { 0,            0              }
 * };
 * 
 * 
 * struct SKW  rXH_kwl[] =  { 
 *         { "Proc-Type",                PEM_PROC_TYPE           },
 *         { "CRL",                      PEM_CRL_                },
 *         { "Content-Domain",           PEM_CONTENT_DOMAIN      },
 *         { "DEK-Info",                 PEM_DEK_INFO            },
 *         { "Originator-ID-Asymmetric", PEM_SENDER_ID           },
 *         { "Originator-ID-Symmetric",  PEM_SENDER_IDS          },
 *         { "Originator-Certificate",   PEM_CERTIFICATE         },
 *         { "Issuer-Certificate",       PEM_ISSUER_CERTIFICATE  },
 *         { "MIC-Info",                 PEM_MIC_INFO            },
 *         { "Recipient-ID-Asymmetric",  PEM_RECIPIENT_ID        },
 *         { "Recipient-ID-Symmetric",   PEM_ID_SYMMETRIC        },
 *         { "Key-Info",                 PEM_KEY_INFO            },
 *         { "Issuer",                   PEM_ISSUER              },
 *         { 0,                          0                       }
 * };                           
 * 
 * char    PEM_Boundary_Begin[] = "-----BEGIN PRIVACY-ENHANCED MESSAGE-----";
 * char    PEM_Boundary_End[]   = "-----END PRIVACY-ENHANCED MESSAGE-----";
 * char    PEM_Boundary_Com[]   = "PRIVACY-ENHANCED MESSAGE-----";
 * char    PEM_Boundary_BB[]    = "-----BEGIN ";
 * char    PEM_Boundary_EB[]    = "-----END ";
 * 
 *
 *       Default Algorithms:
 *
 * 
 * char  *DEK_ENC_ALG = DEFAULT_DEK_ENC_ALG;    "RSA"    
 * char  *MIC_ALG     = DEFAULT_MIC_ALG;        "RSA-MD5"
 * char  *MIC_ENC_ALG = DEFAULT_MIC_ENC_ALG;    "RSA"   
 * char  *MSG_ENC_ALG = DEFAULT_MSG_ENC_ALG;    "DES-CBC"
 * 
 */


/*
 *       PemInfo Structure
 */

typedef struct reclist {
        Certificate    *recpcert;       /* recipient's user certificate     */
        OctetString    *key;            /* RSA-encrypted DES-key            */
        struct reclist *next;
}               RecpList;

typedef struct {
        Boolean         confidential;   /* TRUE if PEM shall be encrypted   */
        Boolean         clear;          /* TRUE if PEM shall be unencoded   */
        Key            *encryptKEY;     /* plain DES-key                    */
        Certificates   *origcert;       /* originator certificates          */
        PKRoot         *rootKEY;        /* root key                         */
        AlgId          *signAI;         /* signature algorithm id           */
        RecpList       *recplist;       /* list of recipients' informations */
}               PemInfo;

struct SKW {
        char           *name;
        int             value;
};

extern struct SKW update_modes[];    /* modes for installing CRLs             */
extern struct SKW content_domain[];  /* valid content domains                 */
extern struct SKW proc_type_v[];     /* valid Proc-Type values                */
extern struct SKW proc_type_t[];     /* valid Proc-Type types                 */
extern struct SKW rXH_kwl[];         /* header field specifiers               */

extern char   PEM_Boundary_Begin[];  /* PEM Boundary line Pre-EB              */
extern char   PEM_Boundary_End[];    /* PEM Boundary line Post-EB             */
extern char   PEM_Boundary_Com[];    /* equal portion of boundary line        */
extern char   PEM_Boundary_BB[];     /* begin portion of boundary begin line  */
extern char   PEM_Boundary_EB[];     /* begin portion of boundary end line    */

extern char   pem_verbose_0, pem_verbose_1, pem_verbose_2;
extern char   pem_verbose_level, pem_insert_cert, pem_option_r, pem_option_K;
extern char   *DEK_ENC_ALG, *MIC_ALG, *MIC_ENC_ALG, *MSG_ENC_ALG;
extern int    pem_Depth, pem_content_domain;
extern Boolean isCA;
extern OctetString **mic_for_certification;


typedef enum {
        RXH_empty,
        RXH_PROC_TYPE,
        RXH_CRL_,
        RXH_CONTENT_DOMAIN,
        RXH_DEK_INFO,
        RXH_SENDER_ID,
        RXH_SENDER_IDS,
        RXH_CERTIFICATE,
        RXH_ISSUER_CERTIFICATE,
        RXH_MIC_INFO,
        RXH_RECIPIENT_ID,
        RXH_ID_SYMMETRIC,
        RXH_KEY_INFO,
        RXH_ISSUER
}               RXH_Header_Fields;

typedef enum {
        PEM_PROC_TYPE,
        PEM_CRL_,
        PEM_CONTENT_DOMAIN,
        PEM_DEK_INFO,
        PEM_SENDER_ID,
        PEM_SENDER_IDS,
        PEM_CERTIFICATE,
        PEM_ISSUER_CERTIFICATE,
        PEM_MIC_INFO,
        PEM_RECIPIENT_ID,
        PEM_ID_SYMMETRIC,
        PEM_KEY_INFO,
        PEM_ISSUER
}               PEM_Header_Fields;

typedef enum {
        PEM_4
}               PEM_Proc_Type_values;

typedef enum {
        PEM_ENC,
        PEM_CRL,
        PEM_CRL_RETRIEVAL_REQUEST,
        PEM_MCO,
        PEM_MCC,
	NO_PEM
}               PEM_Proc_Types;

typedef enum {
        PEM_RFC822,
        PEM_MIME
}               PEM_Content_Domains;


typedef enum {
        PEM_NO_RFC822_HEADER,
        PEM_RFC822_HEADER,
        PEM_RFC822_HEADER_INTERACTIVE
}               PEM_RFC822_Header;


typedef enum         {
        NO_CRL_MESSAGE, 
        CRL_MESSAGE, 
        CRL_RETRIEVAL_REQUEST_MESSAGE 
}        PEM_CRL_Mode;

typedef enum         {
        UPDATE_ASK,
        UPDATE_NO,
        UPDATE_CADB,
        UPDATE_PSE,
        UPDATE_YES,
	UPDATE_ERROR
}        UPDATE_Mode;
typedef enum {
	PEM_error,
	PEM_help,
	PEM_scan,
	PEM_mic_clear,
	PEM_mic_only,
	PEM_encrypted,
	PEM_certify,
	PEM_crl,
	PEM_crl_rr
} PemOperation;


#define ONCE_MAX      4   /* number of header fields which appear only  */
                          /* once in a message                          */
                          /* these have to be declared at first in the  */
                          /* rXH_kwl structure (see pem_init.c)         */

#define ELEVEL      302
#define EPSEOPEN    303
#define EPROCT      304
#define EORIGCERT   305
#define EISSCERTF   306
#define EMICINFO    307
#define EDEKINFO    308
#define ERECID      309
#define EKEYINF     310
#define ERIFILE     311
#define ECODE       312
#define ESYMM       313

#define CERTIFY     1
#define SCAN        0

extern VerificationResult *pem_VerifResult;













typedef struct PemMessageCanon 	PemMessageCanon;
typedef struct PemHeaderCanon 	PemHeaderCanon;
typedef struct PemDekCanon	PemDekCanon;
typedef struct PemOriginatorCanon	PemOriginatorCanon;
typedef struct PemRecCanon	PemRecCanon;
typedef struct PemMicCanon	PemMicCanon;
typedef struct PemCrlCanon	PemCrlCanon;
typedef SET_OF(PemCrlCanon)	SET_OF_PemCrlCanon;
typedef SET_OF(PemRecCanon)	SET_OF_PemRecCanon;
typedef SET_OF(PemMessageCanon)	SET_OF_PemMessageCanon;

typedef struct PemMessageLocal	PemMessageLocal;
typedef struct PemHeaderLocal	PemHeaderLocal;
typedef struct PemDekLocal	PemDekLocal;
typedef struct PemOriginatorLocal	PemOriginatorLocal;
typedef struct PemMicLocal	PemMicLocal;
typedef struct PemCrlLocal	PemCrlLocal;
typedef struct PemRecLocal	PemRecLocal;
typedef SET_OF(PemCrlLocal)	SET_OF_PemCrlLocal;
typedef SET_OF(PemRecLocal)	SET_OF_PemRecLocal;
typedef SET_OF(PemMessageLocal)	SET_OF_PemMessageLocal;

typedef struct ValidationResult	ValidationResult;


struct PemMessageCanon {
	PemHeaderCanon 		*header;
	char	  		*body;
};

struct PemHeaderCanon {
	char			*rfc_version,
	 	 		*proctype,
				*content_domain;
	PemDekCanon	 	*dek_fields;
	PemOriginatorCanon	*orig_fields;
	SET_OF_Name		*issuer_fields;
	PemMicCanon	 	*mic_fields;
	SET_OF_PemCrlCanon	*crl_fields;
	SET_OF_Name		*crl_rr_fields;
	
};
struct PemDekCanon {
	char			*dekinfo_enc_alg,
	 	 		*dekinfo_param,
	 	 		*keyinfo_enc_alg,
	 	 		*keyinfo_dek;
	SET_OF_PemRecCanon	*recipients;
	
};
struct PemRecCanon {
	char			*enc_alg,
	 	 		*dek,
				*issuer,		/* Originator ID Asymmetric	*/
				*serialnumber;		/* Originator ID Asymmetric	*/
	
};

struct PemOriginatorCanon {
	char			*certificate,
	 	 		*issuer,		/* Originator ID Asymmetric	*/
				*serialnumber;		/* Originator ID Asymmetric	*/
	
};
struct PemMicCanon {
	char			*mic_alg,
	 	 		*micenc_alg,
				*mic;
	
};
struct PemCrlCanon {
	char			*crl,
	 	 		*certificate;	
	SET_OF_Name		*issuer_certificate;
};




struct PemMessageLocal {
	PemHeaderLocal		*header;
	OctetString		*body;
	VerificationResult	*validation_result;
	char			*error;
	char			*comment;
};
struct PemHeaderLocal {
	int			 rfc_version;
	PEM_Proc_Types 		 proctype;
	PEM_Content_Domains	 content_domain;
	PemDekLocal	 	*dek_fields;
	Certificates		*certificates;
	Certificate		*root_certificate;
	PemMicLocal	 	*mic_fields;
	SET_OF_CRLWithCertificates	*crl_fields;
	SET_OF_DName	 	*crl_rr_fields;
};

struct PemDekLocal {
	AlgId			*dekinfo_enc_alg;
	AlgId	 		*keyinfo_enc_alg;
	BitString 		*keyinfo_dek;
	SET_OF_PemRecLocal	*recipients;
	
};
struct PemRecLocal {
	AlgId	 		*enc_alg;
	BitString 		*dek;
	Certificate		*certificate;
	
};
struct PemMicLocal {
	AlgId			*signAI;
	BitString		*mic;
	Boolean			 PEM_conformant;
	
};






struct ValidationResult {
	VerificationResult	*verification_result;
};
	








char *CATSPRINTF();
char *CATNSTR();


#include "pem_prototypes.h"

#endif

