/*____________________________________________________________________________
____________________________________________________________________________*/

 /* ======================================================================

    Eudora Extended Message Services API SDK Macintosh 1.0b3 (July 11 1996)
    This SDK supports EMSAPI version 1
    Copyright 1995, 1996 QUALCOMM Inc.
    Send comments and questions to <emsapi-info@qualcomm.com>

    Generated: Mon Jul 15 15:41:24 PDT 1996
    Filename: emsapi-mac.h
	Last Edited: July 24, 1996

    USE THIS FILE TO BUILD YOUR MACINTOSH TRANSLATOR COMPONENT. 

    Note: this file is generated automatically by scripts and must be
    kept in synch with other translation API definitions, so it should
    probably not ever be changed.

*/
#ifndef EMSAPI_MAC_H_INCLUDED
#define EMSAPI_MAC_H_INCLUDED

/* ========== CONSTANTS AND RETURN VALUES ================================ */

/* ----- Translator return codes --- store as a long --------------------- */ 
#define EMSR_OK              (0L)     /* The translation operation succeeded */
#define EMSR_UNKNOWN_FAIL    (1L)     /* Failed for unspecified reason */
#define EMSR_CANT_TRANS      (2L)     /* Don't know how to translate this */
#define EMSR_INVALID_TRANS   (3L)     /* The translator ID given was invalid */
#define EMSR_NO_ENTRY        (4L)     /* The value requested doesn't exist */
#define EMSR_NO_INPUT_FILE   (5L)     /* Couldn't find input file */
#define EMSR_CANT_CREATE     (6L)     /* Couldn't create the output file */
#define EMSR_TRANS_FAILED    (7L)     /* The translation failed. */
#define EMSR_INVALID         (8L)     /* Invalid argument(s) given */
#define EMSR_NOT_NOW         (9L)     /* Translation can be done not
										in current  context */
#define EMSR_NOW            (10L)     /* Indicates translation
										 can be performed right away */
#define EMSR_ABORTED        (11L)     /* Translation was aborted by user */
 
 
/* ----- Translator types --- store as a long ---------------------------- */ 
#define EMST_NO_TYPE          (-1L)
#define EMST_LANGUAGE         (0x10L)
#define EMST_TEXT_FORMAT      (0x20L)
#define EMST_GRAPHIC_FORMAT   (0x30L)
#define EMST_COMPRESSION      (0x40L)
#define EMST_COALESCED        (0x50L)
#define EMST_SIGNATURE        (0x60L)
#define EMST_PREPROCESS       (0x70L)
#define EMST_CERT_MANAGEMENT  (0x80L)


/* ----- Translator info flags and contexts --- store as a long ---------- */
/* Used both as bit flags and as constants */
#define EMSF_ON_ARRIVAL      (0x0001L) /* Call on message arrivial */
#define EMSF_ON_DISPLAY      (0x0002L) /* Call when user views message */
#define EMSF_ON_REQUEST      (0x0004L) /* Call when selected from menu */
#define EMSF_Q4_COMPLETION   (0x0008L) /* Queue and call on complete 
                                         composition of a message */
#define EMSF_Q4_TRANSMISSION (0x0010L) /* Queue and call on transmission
                                         of a message */
#define EMSF_WHOLE_MESSAGE   (0x0200L) /* Works on the whole message even if
                                         it has sub-parts. (e.g. signature) */
#define EMSF_REQUIRES_MIME   (0x0400L) /* Items presented for translation
                                         should be MIME entities with
                                         canonical end of line representation,
                                         proper transfer encoding
                                         and headers */
#define EMSF_GENERATES_MIME  (0x0800L) /* Data produced will be MIME format */
/* all other flag bits in the long are RESERVED and may not be used */


/* ----- The version of the API defined by this include file ------------- */
#define EMS_VERSION          (1)       /* Used in plugin init */
#define EMS_COMPONENT        'EuTL'    /* Macintosh component type */


/* ----- Translator and translator type specific return codes ------------ */
#define EMSC_SIGOK           (1L)      /* A signature verification succeeded */
#define EMSC_SIGBAD          (2L)      /* A signature verification failed */
#define EMSC_SIGUNKNOWN      (3L)      /* Result of verification unknown */




/* ========== POWER PC/68K GLUE FOR CALL BACKS ====================== */

/* These can be treated as magic type declarations and just passed on
   to the functins below. They are the thing that Eudora passes to
   the component to indicate where the call back function is.
 */
#if GENERATINGCFM
typedef UniversalProcPtr emsProgress;       /* Power PC */
#else
typedef pascal short (*emsProgress)(short); /* 68K */
#endif
typedef emsProgress emsProgressUPP;


/* These typedefs can also be ignored in favor of the two calls below
   unless your not using emsapi-component.c and are doing your own component
   manager interface.  These are what the functions would look like
   if we didn't have to worry about differences in architecture between
   the Component and Eudora. 
 */
typedef pascal short  (*emsProgress_t)(short);     /* The progress function */

/* Use these to call the prorgress and allocator functions!  They are
   supplied in emsapi-component.c. The first argument is the cookie
   idenitifying the function passed by Eudora, and the second is the
   arguments function.
 */
short callProgress(emsProgress glop, short percent);





/* ========== DATA STRUCTURES ============================================ */
/* True Mac-style declarations aren't used yet but are included in comments. 
   All strings on the Mac are PASCAL strings and all are allocated as Handles. 
 */
/* ----- Macintosh: MIME type data passed across the API ----------------- */
/* typedef struct emsMIMEparamS **emsMIMEparamHandle; /*Mac style declaration */
typedef struct emsMIMEparamS **emsMIMEParamP;
typedef struct emsMIMEparamS {
    Str63                 name;      /* MIME parameter name */
    Handle                value;     /* handle size determines string length */
    struct emsMIMEparamS **next;     /* Handle for next param in list */
/*  emsMIMEparamHandle     next;     /* Mac style declaration for above */ 
} emsMIMEparam;
/* typedef struct emsMIMEtypeS **emsMIMEtypeHandle; /* Mac style declaration */
typedef struct emsMIMEtypeS **emsMIMEtypeP; 
typedef struct emsMIMEtypeS {
    Str63              mime_version; /* MIME-Version: header */
    Str63              mime_type;    /* Top-level MIME type: text,message...*/
    Str63              sub_type;     /* sub-type */
    emsMIMEparam      **params;      /* Handle to first parameter in list */
/*  emsMIMEparamHandle  params;      /* Mac style declaration of above */
} emsMIMEtype;

/* ========== FUNCTION PROTOTYPES ======================================== */

/* ----- Get the API Version number this plugin implements ----------------- */
pascal long ems_plugin_version(
    short *api_version           /* Place to return api version */
);

/* ----- Initialize plugin and get its basic info -------------------------- */
pascal long ems_plugin_init(
    Handle globals,              /* Handle to translator instance structure */
    short    *num_trans,         /* Place to return num of translators */
    StringPtr **plugin_desc,     /* Return for string description of plugin */
    short *plugin_id,            /* Place to return unique plugin id */
    Handle *plugin_icon          /* Return for plugin icon suite */
);

/* ----- Get details about a translator in a plugin ------------------------ */
pascal long ems_translator_info(
    Handle globals,              /* Handle to translator instance structure */
    short trans_id,              /* ID of translator to get info for */
    long *trans_type,          /* Return for translator type, e.g., EMST_xxx */
    long *trans_sub_type,        /* Return for translator subtype */
    unsigned long *trans_flags,  /* Return for translator flags */
    StringPtr **trans_desc,      /* Return for translator string description */
    Handle *trans_icon           /* Return for translator icon suite */
);

/* ----- Check and see if a translation can be performed (file version) ---- */
pascal long ems_can_translate_file(
    Handle globals,              /* Handle to translator instance structure */
    long context,                /* Context for check; e.g. EMSF_ON_xxx */
    short trans_id,              /* ID of translator to call */
    emsMIMEtypeP in_mime,        /* MIME type of data to check */
    FSSpec *in_file,             /* The input file FSSpec */
    StringPtr ***addresses,      /* List of addresses (sender and recipients) */
    long *out_len,               /* Place to return estimated length (unused) */
    StringPtr **out_error,       /* Place to return string with error message */
    long *out_code             /* Return for translator-specific result code */
);

/* ----- Actually perform a translation on a file -------------------------- */
pascal long ems_translate_file(
    Handle globals,              /* Handle to translator instance structure */
    long context,                /* Context for translation; e.g. EMSF_ON_xxx */
    short trans_id,              /* ID of translator to call */
    emsMIMEtypeP in_mime,        /* MIME type of input data */
    FSSpec *in_file,             /* The input file name */
    StringPtr ***addresses,      /* List of addresses (sender and recipients) */
    emsProgress progress,        /* Func to report progress / check for abort */
    emsMIMEtypeP *out_mime,      /* Place to return MIME type of result */
    FSSpec *out_file,            /* The output file (specified by Eudora) */
    Handle *out_icon,            /* Returned icon suite representing
    								the result */
    StringPtr **out_desc,        /* Returned string for display with 
    								the result */
    StringPtr **out_error,       /* Place to return string with error message */
    long *out_code               /* Return for translator-specific
    								result code */
);

/* ----- Actually perform a translation on a buffer ------------------------ */
#ifdef EMS_VERSION_2
pascal long ems_translate_buf(
    Handle globals,              /* Handle to translator instance structure */
    long context,                /* Context for translation; e.g. EMSF_ON_xxx */
    short trans_id,              /* ID of translator to call */
    emsMIMEtypeP in_mime,        /* MIME type of input data */
    unsigned char **in_buffer,   /* Handle to buffer of data to translate */
    long in_offset,              /* Offset to start of input data in buffer */
    long *in_buffer_len,         /* Amount of input & returns input consumed */
    StringPtr ***addresses,      /* List of addresses (sender and recipients) */
    emsProgress progress,        /* Func to report progress/check for abort */
    emsMIMEtypeP *out_mime,      /* Place to return MIME type of result */
    unsigned char **out_buffer,  /* Hd be placed in */
    Handle *out_icon,            /* Returned icon suite representing the
    								result */
    StringPtr **out_desc,        /* Returned string for display with the
    									result */
    StringPtr **out_error,       /* Place to return string with
    								 error message */
    long *out_code               /* Return for translator-specific
    								result code */
);
#endif //EMS_VERSION_2

/* ----- End use of a plugin and clean up ---------------------------------- */
pascal long ems_plugin_finish(
    Handle globals               /* Handle to translator instance structure */
);

#endif /* EMSAPI_MAC_H_INCLUDED */
