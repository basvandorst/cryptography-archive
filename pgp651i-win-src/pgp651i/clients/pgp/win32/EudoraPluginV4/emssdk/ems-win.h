#ifndef __EMS_WIN__
#define __EMS_WIN__
/* ======================================================================

    C header for EMSAPI version 4
    Copyright 1996, 1997 QUALCOMM Inc.
    Send comments and questions to <emsapi-info@qualcomm.com>

    Generated: Mon Oct 6 15:22:45 PDT 1997
    Filname: ems-win.h
    
    USE THIS FILE TO BUILD YOUR WINDOWS TRANSLATOR DLL

    Note: this file is generated automatically by scripts and must be
    kept in synch with other translation API definitions, so it should
    probably not ever be changed.


/* ========== CONSTANTS AND RETURN VALUES ================================ */

/* ----- Translator return codes --- store as a long --------------------- */ 
#define EMSR_OK              (0L)     /* The translation operation 
										succeeded */
#define EMSR_UNKNOWN_FAIL    (1L)     /* Failed for unspecified reason */
#define EMSR_CANT_TRANS      (2L)     /* Don't know how to translate this */
#define EMSR_INVALID_TRANS   (3L)     /* The translator ID given was 
											invalid */
#define EMSR_NO_ENTRY        (4L)     /* The value requested doesn't exist */
#define EMSR_NO_INPUT_FILE   (5L)     /* Couldn't find input file */
#define EMSR_CANT_CREATE     (6L)     /* Couldn't create the output file */
#define EMSR_TRANS_FAILED    (7L)     /* The translation failed. */
#define EMSR_INVALID         (8L)     /* Invalid argument(s) given */
#define EMSR_NOT_NOW         (9L)     /* Translation can be done not in 
											current context */
#define EMSR_NOW            (10L)     /* Indicates translation can be 
											performed right away */
#define EMSR_ABORTED        (11L)     /* Translation was aborted by user */
#define EMSR_DATA_UNCHANGED (12L)     /* Trans OK, data was not changed */
 
 
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
#define EMSF_WHOLE_MESSAGE   (0x0200L) /* Works on the whole message even 
											if it has sub-parts. 
											(e.g. signature) */
#define EMSF_REQUIRES_MIME   (0x0400L) /* Items presented for translation
                                          should be MIME entities with
                                          canonical end of line 
										representation,
                                          proper transfer encoding
                                          and headers */
#define EMSF_GENERATES_MIME  (0x0800L) /* Data produced will be 
											MIME format */
#define EMSF_ALL_HEADERS     (0x1000L) /* All headers in & out of 
										trans when MIME format is used */
#define EMSF_BASIC_HEADERS   (0x2000L) /* Just the basic to, from, 
											subject, cc,bcc headers */
#define EMSF_DEFAULT_Q_ON    (0x4000L) /* Causes queued translation to be 
										on for a new message by default */
#define EMSF_TOOLBAR_PRESENCE (0x8000L)/* Appear on the Toolbar */
#define EMSF_ALL_TEXT		 (0x10000L)/* ON_REQUEST WANTS WHOLE MESSAGE */


/* ----- The version of the API defined by this include file ------------- */
#define EMS_VERSION          (4)       /* Used in plugin init */
#define EMS_PB_VERSION       (3)       /* Minimum version that 
											uses parameter blocks */
#define EMS_COMPONENT        'EuTL'    /* Macintosh component type */


/* ----- Translator and translator type specific return codes ------------ */
#define EMSC_SIGOK           (1L)      /* A signature verification 
											succeeded */
#define EMSC_SIGBAD          (2L)      /* A signature verification failed */
#define EMSC_SIGUNKNOWN      (3L)      /* Result of verification unknown */


/* ----- Progress Data ---------------------------- */
typedef struct emsProgressDataS FAR* emsProgressDataP;
typedef struct emsProgressDataS {
	long			size;			/* Size of this data structure */
	long			value;			/* Range of Progress, percent complete */
	LPSTR			message;		/* Progress Message */
} emsProgressData;


/* ----- User Address ---------------------------- */
typedef struct emsAddressS FAR*emsAddressP; 
typedef struct emsAddressS {
	long			size;			/* Size of this data structure */
	LPSTR			address;		/* user address */
	LPSTR			realname;		/* Users full name from Eudora config */
	emsAddressP		next;			/* Linked list of addresses */
} emsAddress;


/* == CALL BACK FUNCTIONS FROM EUDORA ACROSS THE API======================== */
typedef short     (FAR* emsProgress)(emsProgressDataP);/* The progress 
																function  */

/* ========== DATA STRUCTURES ============================================ */
/* All strings on Windows are NULL terminated C strings */
/* ----- MIME type data passed across the API ---------------------------- */

/* ----- MIME Params ---------------------------- */
typedef struct emsMIMEparamS FAR*emsMIMEParamP;
typedef struct emsMIMEparamS {
    long			size;
    LPSTR			name;			/* Mime parameter name (e.g., charset) */
    LPSTR			value;			/* param value (e.g. us-ascii) */
    emsMIMEParamP  	next;			/* Linked list of parameters */
} emsMIMEparam;


/* ----- MIME Info ---------------------------- */
typedef struct emsMIMEtypeS FAR*emsMIMEtypeP; 
typedef struct emsMIMEtypeS {
    long			size;
    LPSTR			version;		/* The MIME-Version header */
    LPSTR			type;			/* Top-level MIME type */      
    LPSTR           subType;		/* sub-type */
    emsMIMEParamP	params;			/* MIME parameter list */
    LPSTR       	contentDisp;  	/* Content-Disposition */
    emsMIMEParamP   contentParams; 	/* Handle to first parameter in list */
} emsMIMEtype;

/* ----- Header Data ---------------------------- */
typedef struct emsHeaderDataS FAR*emsHeaderDataP; 
typedef struct emsHeaderDataS {
    long			size;			/* Size of this data structure */
    emsAddressP		to;				/* To Header */
    emsAddressP		from;			/* From Header */
    LPSTR			subject;		/* Subject Header */
    emsAddressP		cc;				/* cc Header */
    emsAddressP		bcc;			/* bcc Header */
    LPSTR			rawHeaders;		/* The 822 headers */
} emsHeaderData;



/* ----- How Eudora is configured ---------------------------- */
typedef struct emsMailConfigS FAR*emsMailConfigP; 
typedef struct emsMailConfigS {
	long			size;			/* Size of this data structure */
	HWND			FAR*eudoraWnd;	/* Eudora's main window */
    LPSTR			configDir;		/* Optional directory for config file */
	emsAddress		userAddr;		/* Users full name from Eudora config */
	unsigned long	reserved;		/* V4! reserved */
} emsMailConfig;

/* ----- Plugin Info ---------------------------- */
typedef struct emsPluginInfoS FAR*emsPluginInfoP; 
typedef struct emsPluginInfoS {
    long			size;			/* Size of this data structure */
    long			numTrans;		/* Place to return num of translators */
    long			numAttachers;	/* Place to return num of attach hooks */
    long			numSpecials;/* Place to return num of special hooks */
    LPSTR			desc;		/* Return for string description of plugin */
    long			id;				/* Place to return unique plugin id */
    HICON			FAR*icon;		/* Return for plugin icon data */
    long			mem_rqmnt;/* V4! Return Memory Required to run this */
    long            reserved1;		/* V4! reserved */
    long            reserved2;		/* V4! reserved */
} emsPluginInfo;

/* ----- Translator Info ---------------------------- */
typedef struct emsTranslatorS FAR*emsTranslatorP; 
typedef struct emsTranslatorS {
    long			size;			/* Size of this data structure */
    long			id;				/* ID of translator to get info for */
    long			type;			/* translator type, e.g., EMST_xxx */
    ULONG			flags;			/* translator flags */
    LPSTR			desc;			/* translator string description */
    HICON			FAR*icon;		/* translator icon data */
    LPSTR			properties;		/* Properties for queued translations */
} emsTranslator;

/* ----- Menu Item Info ---------------------------- */
typedef struct emsMenuS FAR*emsMenuP; 
typedef struct emsMenuS {
    long			size;			/* Size of this data structure */
    long			id;				/* ID of translator to get info for */
    LPSTR			desc;			/* translator string description */
    HICON			FAR*icon;		/* NEW V4! menu icon  */
    long			flags;		/* NEW V4! flags like EMSF_TOOLBAR_PRESENCE */
} emsMenu;

/* ----- Translation Data ---------------------------- */
typedef struct emsDataFileS FAR*emsDataFileP; 
typedef struct emsDataFileS {
    long			size;			/* Size of this data structure */
    long			context;
    emsMIMEtypeP	info;			/* MIME type of data to check */
    emsHeaderDataP  header;			/* EMSF_BASIC_HEADERS & EMSF_ALL_HEADERS
										determine contents */
    LPSTR			fileName;		/* The input file name */
} emsDataFile;

/* ----- Resulting Status Data ---------------------------- */
typedef struct emsResultStatusS FAR*emsResultStatusP; 
typedef struct emsResultStatusS {
    long			size;			/* Size of this data structure */
    LPSTR			desc;			/* Returned string for display with
										the result */
    LPSTR			error;			/* Place to return string with 
										error message */
    long			code;			/* Return for translator-specific 
										result code */
} emsResultStatus;


/* ========== FUNCTION PROTOTYPES ======================================== */

#ifdef __cplusplus
extern "C" {
#endif
	
/* ----- Get the API Version number this plugin implements --------------- */
long WINAPI ems_plugin_version(
    short FAR* apiVersion			/* Out: Plugin Version */
);

/* ----- Initialize plugin and get its basic info ------------------------ */
long WINAPI ems_plugin_init(
    void FAR*FAR* globals,			/* Out: Return for allocated 
										instance structure */
    short eudAPIVersion,			/* In: The API version eudora is 
										using */
    emsMailConfigP mailConfig,		/* In: Eudoras mail configuration */
    emsPluginInfoP pluginInfo		/* Out: Return Plugin Information */
);

/* ----- Get details about a translator in a plugin ---------------------- */
long WINAPI ems_translator_info(
    void FAR* globals,				/* Out: Return for allocated 
										instance structure */
    emsTranslatorP transInfo		/* In/Out: Return Translator 
										Information */
);

/* ----- Check and see if a translation can be performed ----------------- */
long WINAPI ems_can_translate(
    void FAR* globals,				/* Out: Return for allocated 
										instance structure */
    emsTranslatorP trans,			/* In: Translator Info */
    emsDataFileP inTransData,		/* In: What to translate */
    emsResultStatusP transStatus	/* Out: Translations Status 
										information */
);

/* ----- Actually perform a translation on a file ------------------------ */
long WINAPI ems_translate_file(
    void FAR* globals,				/* Out: Return for allocated 
										instance structure */
    emsTranslatorP trans,			/* In: Translator Info */
    emsDataFileP inFile,			/* In: What to translate */
    emsProgress progress,			/* Func to report progress/check 
										for abort */
    emsDataFileP outFile,			/* Out: Result of the translation */
    emsResultStatusP transStatus	/* Out: Translations Status 
										information */
);

/* ----- End use of a plugin and clean up -------------------------------- */
long WINAPI ems_plugin_finish(
    void FAR* globals	/* Out: Return for allocated instance structure */
);

short ems_allocate(
    void FAR** mem,             
	size_t size
);

/* ----- Free memory allocated by EMS plug-in ---------------------------- */
long WINAPI ems_free(
    void FAR* mem					/* Memory to free */
);

/* ----- Call the plug-ins configuration Interface ----------------------- */
long WINAPI ems_plugin_config(
    void FAR* globals,				/* Out: Return for allocated 
										instance structure */
    emsMailConfigP mailConfig		/* In: Eudora mail info */
);

/* ----- Manage properties for queued translations ----------------------- */
long WINAPI ems_queued_properties(
    void FAR* globals,				/* Out: Return for allocated instance structure */
    emsTranslatorP trans,			/* In/Out: The translator */
    long FAR* selected				/* In/Out: State of this translator */
);

/* ----- Info about menu hook to attach/insert composed object ----------- */
long WINAPI ems_attacher_info(
    void FAR* globals,				/* Out: Return for allocated instance structure */
    emsMenuP attachMenu				/* Out: The menu */
);

/* ----- Call an attacher hook to compose some special object ------------ */
long WINAPI ems_attacher_hook(
    void FAR* globals,				/* Out: Return for allocated 
										instance structure */
    emsMenuP attachMenu,			/* In: The menu */
    LPSTR attachDir,				/* In: Location to put attachments */
    long * numAttach,				/* Out: Number of files attached */
    emsDataFileP ** attachFiles		/* Out: Name of files written */
);

/* ----- Info about special menu items hooks ----------------------------- */
long WINAPI ems_special_info(
    void FAR* globals,				/* Out: Return for allocated instance 
											structure */
    emsMenuP specialMenu			/* Out: The menu */
);

/* ----- Call a special menu item hook ----------------------------------- */
long WINAPI ems_special_hook(
    void FAR* globals,				/* Out: Return for allocated 
										instance structure */
    emsMenuP specialMenu			/* In: The menu */
);

#ifdef __cplusplus
}
#endif

#endif /* __EMS_WIN__ */

