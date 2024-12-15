#ifndef __EMS_WIN__
#define __EMS_WIN__
/* ======================================================================

    This SDK supports EMSAPI version 2
    Copyright 1995, 1996 QUALCOMM Inc.
    Send comments and questions to <emsapi-info@qualcomm.com>

    Generated: Tue Aug 20 10:36:38 PDT 1996
    Filname: ems-win.h
    
    USE THIS FILE TO BUILD YOUR WINDOWS TRANSLATOR DLL

    Note: this file is generated automatically by scripts and must be
    kept in synch with other translation API definitions, so it should
    probably not ever be changed.


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
#define EMSR_NOT_NOW         (9L)     /* Translation can be done not in current
                                         context */
#define EMSR_NOW            (10L)     /* Indicates translation can be performed
                                         right away */
#define EMSR_ABORTED        (11L)     /* Translation was aborted by user */
#define EMSR_OK_DID_NOTHING (12L)     /* Trans OK, data was not changed */
 
 
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
#define EMSF_ALL_HEADERS     (0x1000L) /* All headers in & out of trans when
                                          MIME format is used */
/* all other flag bits in the long are RESERVED and may not be used */


/* ----- The version of the API defined by this include file ------------- */
#define EMS_VERSION          (2)       /* Used in plugin init */
#define EMS_COMPONENT        'EuTL'    /* Macintosh component type */


/* ----- Translator and translator type specific return codes ------------ */
#define EMSC_SIGOK           (1L)      /* A signature verification succeeded */
#define EMSC_SIGBAD          (2L)      /* A signature verification failed */
#define EMSC_SIGUNKNOWN      (3L)      /* Result of verification unknown */

/* ========== FUNCTIONS PASSED ACROSS THE API ============================ */
typedef short (FAR* emsProgress)(short);     /* The progress function  */


/* ========== DATA STRUCTURES ============================================ */
/* All strings on Windows are NULL terminated C strings */
/* ----- MIME type data passed across the API ---------------------------- */
typedef struct emsMIMEparamS FAR*emsMIMEParamP;
typedef struct emsMIMEparamS {
    char             FAR*    name;    /* Mime parameter name (e.g., charset) */
    char             FAR*    value;   /* param value (e.g. us-ascii) */
    struct emsMIMEparamS FAR* next;   /* Linked list of parameters */
} emsMIMEparam;
typedef struct emsMIMEtypeS FAR*emsMIMEtypeP; 
typedef struct emsMIMEtypeS {
    char         FAR* mime_version;   /* The MIME-Version header */
    char         FAR* mime_type;      /* Top-level MIME type */      
    char         FAR* sub_type;       /* MIME sub-type */
    emsMIMEparam  FAR* params;        /* MIME parameter list */
} emsMIMEtype;

/* ========== FUNCTION PROTOTYPES ======================================== */

#ifdef __cplusplus
extern "C" {
#endif

/* ----- Get the API Version number this plugin implements ----------------- */
long WINAPI ems_plugin_version(
    short FAR* api_version      /* Place to return api version */
);

/* ----- Initialize plugin and get its basic info -------------------------- */
long WINAPI ems_plugin_init(
    void FAR*FAR* globals,      /* Return for allocated instance structure */
    char FAR* config_dir,       /* Optional directory for config file */
    char FAR* user_name,        /* Users full name from Eudora config */
    char FAR* user_addr,        /* Users address (RFC-822 addr spec) */
    short FAR* num_trans,       /* Place to return num of translators */
    char FAR*FAR* plugin_desc,  /* Return for string description of plugin */
    short FAR* plugin_id,       /* Place to return unique plugin id */
    HICON FAR*FAR* plugin_icon  /* Return for plugin icon data */
);

/* ----- Get details about a translator in a plugin ------------------------ */
long WINAPI ems_translator_info(
    void FAR* globals,          /* Pointer to translator instance structure */
    short trans_id,             /* ID of translator to get info for */
    long FAR* trans_type,      /* Return for translator type, e.g., EMST_xxx */
    long FAR* trans_sub_type,   /* Return for translator subtype */
    unsigned long FAR* trans_flags, /* Return for translator flags */
    char FAR*FAR* trans_desc,   /* Return for translator string description */
    HICON FAR*FAR* trans_icon   /* Return for translator icon data */
);

/* ----- Check and see if a translation can be performed ------------------- */
long WINAPI ems_can_translate(
    void FAR* globals,          /* Pointer to translator instance structure */
    long context,               /* Context for check; e.g. EMSF_ON_xxx */
    short trans_id,             /* ID of translator to call */
    emsMIMEtypeP in_mime,       /* MIME type of data to check */
    char FAR*FAR* addresses,    /* List of addresses (sender and recipients) */
    char FAR* properties,       /* Properties for queued translations */
    char FAR*FAR* out_error,    /* Place to return string with error message */
    long FAR* out_code         /* Return for translator-specific result code */
);

/* ----- Actually perform a translation on a file -------------------------- */
long WINAPI ems_translate_file(
    void FAR* globals,          /* Pointer to translator instance structure */
    long context,               /* Context for translation; e.g. EMSF_ON_xxx */
    short trans_id,             /* ID of translator to call */
    emsMIMEtypeP in_mime,       /* MIME type of input data */
    char FAR* in_file,          /* The input file name */
    char FAR*FAR* addresses,    /* List of addresses (sender and recipients) */
    char FAR* properties,       /* Properties for queued translations */
    emsProgress progress,       /* Func to report progress / check for abort */
    emsMIMEtypeP FAR* out_mime, /* Place to return MIME type of result */
    char FAR* out_file,         /* The output file (specified by Eudora) */
    HICON FAR*FAR* out_icon,    /* Place to return icon representing result */
    char FAR*FAR* out_desc,   /* Returned string for display with the result */
    char FAR*FAR* out_error,    /* Place to return string with error message */
    long FAR* out_code          /* Return for translator-specific result code */
);

/* ----- Actually perform a translation on a buffer ------------------------ */
long WINAPI ems_translate_buf(
    void FAR* globals,          /* Pointer to translator instance structure */
    long context,               /* Context for translation; e.g. EMSF_ON_xxx */
    short trans_id,             /* ID of translator to call */
    emsMIMEtypeP in_mime,       /* MIME type of input data */
    unsigned char FAR* in_buffer, /* Pointer to buffer of data to translate */
    long FAR* in_buffer_len,    /* Amount of input & returns input consumed */
    char FAR*FAR* addresses,    /* List of addresses (sender and recipients) */
    char FAR* properties,       /* Properties for queued translations */
    emsProgress progress,       /* Func to report progress/check for abort */
    emsMIMEtypeP FAR* out_mime, /* Place to return MIME type of result */
    unsigned char FAR* FAR* out_buffer, /* Eudora supplied output buffer */
    long FAR* out_buffer_len,  /* Out buffer size & returns amount filled in */
    HICON FAR*FAR* out_icon,    /* Place to return icon representing result */
    char FAR*FAR* out_desc,   /* Returned string for display with the result */
    char FAR*FAR* out_error,    /* Place to return string with error message */
    long FAR* out_code         /* Return for translator-specific result code */
);

/* ----- End use of a plugin and clean up ---------------------------------- */
long WINAPI ems_plugin_finish(
    void FAR* globals           /* Pointer to translator instance structure */
);

/* ----- Free memory allocated by EMS plug-in ------------------------------ */
long WINAPI ems_free(
    void FAR* mem               /* Memory to free */
);

/* ----- Call the plug-ins configuration Interface ------------------------- */
long WINAPI ems_plugin_config(
    void FAR* globals,          /* Pointer to translator instance structure */
    char FAR* config_dir,       /* Optional directory for config file */
    char FAR* user_name,        /* Users full name from Eudora config */
    char FAR* user_addr         /* Users address (RFC-822 addr spec) */
);

/* ----- Manage properties for queued translations ------------------------- */
long WINAPI ems_queued_properties(
    void FAR* globals,          /* Pointer to translator instance structure */
    long context,               /* EMSF_Q4_COMPLETION or _TRANSMISSION */
    short trans_id,             /* ID of translator to call */
    short FAR* selected,        /* Returned flag - is trans selected? */
    char FAR*FAR* properties    /* ASCII string encoded properties */
);

#ifdef __cplusplus
}
#endif


#endif /* __EMS_WIN__ */

