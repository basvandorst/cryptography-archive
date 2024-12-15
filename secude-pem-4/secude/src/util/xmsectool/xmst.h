/*
 *  SecuDE Release 4.2 (GMD)
 */
/********************************************************************
 * Copyright (C) 1991, GMD. All rights reserved.                    *
 *                                                                  *
 *                                                                  *
 *                         NOTICE                                   *
 *                                                                  *
 *    Acquisition, use, and distribution of this module             *
 *    and related materials are subject to restrictions             *
 *    mentioned in each volume of the documentation.                *
 *                                                                  *
 ********************************************************************/

/*-----------------------xmst.h-------------------------------------*/
/*                                                                  */
/*------------------------------------------------------------------*/
/* GMD Darmstadt Institute for System Technic (I2)                  */
/* Rheinstr. 75 / Dolivostr. 15                                     */
/* 6100 Darmstadt                                                   */
/* Project ``Secure DFN'' 1990 / "SecuDE" 1991/92/93                */
/* Grimm/Nausester/Schneider/Viebeg/Vollmer 	                    */
/* Luehe/Surkau/Reichelt/Kolletzki		                    */
/*------------------------------------------------------------------*/
/* PACKAGE   util            VERSION   4.2                          */
/*                              DATE   01.11.1993                   */
/*                                BY   ws                           */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* PROGRAM   xmst            VERSION   2.2                          */
/*                              DATE   01.11.1993                   */
/*                                BY   Kolletzki                    */
/*                                                                  */
/*                            REVIEW                                */
/*                              DATE                                */
/*                                BY                                */
/*                                                                  */
/* FILE      xmst.h                                                 */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* DESCRIPTION                                                      */
/*	Defines constants 					    */
/*	Includes further files 					    */
/*	Declares functions & externals 		 		    */
/*                                                                  */
/*------------------------------------------------------------------*/




/*
 *	Defines
 */
#define NL '\012'
#define ALL 6
#define ENC 5
#define TIMELEN 40
#define OPEN_TO_READ	0
#define OPEN_TO_WRITE	1


/*
 *	System Includes
 */
#include <stdio.h>
#include <sys/param.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#ifndef MAC
#include <sys/types.h>
#include <sys/stat.h>
#else
#include <unix.h>
#include <console.h>
#include "Mac.h"
#endif


/*
 *	Motif Includes (include them before SecuDE stuff!)
 */
#include <Xm/XmAll.h>		/* Motif Toolkit */
#include <Mrm/Mrm.h>		/* Mrm Toolkit */


/*
 *	SecuDE Includes
 */
#include "af.h"
#include "cadb.h"
#ifdef MFCHECK
#include "MF_check.h"
#endif

/*
 *	XMST Version
 */
#include "version.h"
 
 

/*
 ******************************
 *	Motif Declaration Stuff
 ******************************
 */

extern XrmDatabase	config_database;

/*
 *	Self defined resources
 */
#define XmstNbusycursor		"busycursor"
#define XmstNfocuscolor		"focuscolor"
#define XmstNactivelistcolor	"activelistcolor"

struct applrestype {
	Cursor		busycursor;
	Pixel		focuscolor;
	Pixel		activelistcolor;
};
typedef struct applrestype ApplResType;

/* global variables that contain resource values */
extern char		*external_editor;
extern Cursor		busy_cursor;
extern Pixel		focus_color;
extern Pixel		active_list_color;


struct pklistclientdata {				/* LIST_CLIENT_DATA type for pk/ek-list */
	DName		*subject;
	DName		*issuer;
	OctetString	*serial;
};
typedef struct pklistclientdata PKList_client_data;

/*
 *	LIST CLIENT DATA type
 *	This is my quick & dirty solution for a XmList item client data structure.
 */
extern XtPointer	pselist_clientdata[];
extern XtPointer	pklist_clientdata[];
extern XtPointer	eklist_clientdata[];
extern XtPointer	aliaslist_clientdata[];

/*
 *	UIL
 */
extern MrmHierarchy	applicationHierarchy;
extern char		*applicationUidVector[];
extern MrmType		opaqueClass;

/*
 *	Display/Screen/Shells/Widgets
 */
extern Display		*display;
extern Screen		*screen;
extern XtAppContext	applicationContext;
extern Widget		applicationShell;
extern Widget		stMainShell, atMainShell, dtMainShell;
extern Widget		atToplevel, dtToplevel;
extern Widget		initialAuthToggle, initialNiceToggle;
extern Widget		pseList, pkList, ekList;
extern Widget		activeList;




/*
 *	Pixmaps
 */
extern Pixmap		sectoolPixmap;
extern Pixmap		aliastoolPixmap;
extern Pixmap		dirtoolPixmap;
extern Pixmap		scPixmap;
extern Pixmap		psePixmap;
extern Pixmap		scpsePixmap;
extern Pixmap		onepairPixmap;
extern Pixmap		twopairsPixmap;

/*
 *	Args, Flags, Vectors
 */
extern Arg		args[];			/* handle with care! */
extern int 		sectool_cnt;
extern char 		**sectool_parm;
extern Boolean		debugmode_on;
extern Boolean		Debug;
extern Boolean		auto_save_configuration;
extern Boolean		auto_detect_sct;
extern Boolean		alias_save_required;
extern Boolean		confirm_overwrite;
extern Boolean 		verbose;
extern Boolean 		sectool_verbose;
extern char		*unix_home;
extern char		editor_tempfile[];
extern char		config_rcfile[];
extern char		system_aliasfile[];
extern char		dialog_message[];
extern char		*alias_to_add;
extern char		*alias_message[];
extern int		pin_failure_count;



/*
 *	Macros
 */

#define CPDSTRING(s)		XmStringCreateLtoR(s, XmSTRING_DEFAULT_CHARSET)
#define CPDSTRINGFIXED(s)	XmStringCreateLtoR(s, "listFixedFont")
#define CPDSTRINGBOLD(s)	XmStringCreateLtoR(s, "listFixedBoldFont")
#define STWIDGET(s)		name2widget(applicationShell, s)		
#define ATWIDGET(s)		name2widget(atToplevel, s)
#define DTWIDGET(s)		name2widget(dtToplevel, s)
#define ALARM()			XBell(display, 1)
#define ALARM3()		XBell(display, 1); XBell(display, 1); XBell(display, 1)
#define BUSY_CURSOR()		{ XDefineCursor(display, XtWindow(applicationShell), busy_cursor);		\
				  if (atToplevel) XDefineCursor(display, XtWindow(atToplevel), busy_cursor);	\
				  if (dtToplevel) XDefineCursor(display, XtWindow(dtToplevel), busy_cursor);	\
				  XmUpdateDisplay(applicationShell);						}
#define IDLE_CURSOR()		{ XDefineCursor(display, XtWindow(applicationShell), None);			\
				  if (atToplevel) XDefineCursor(display, XtWindow(atToplevel), None);		\
				  if (dtToplevel) XDefineCursor(display, XtWindow(dtToplevel), None);		\
				  XmUpdateDisplay(applicationShell);						}


/*
 *	Global defs & decs
 */
#define XMST_ARGS			11		/* Size of global Arg vector for single XSetValues */
#define XMST_PIN_LENGTH			256		/* Size of passwords */
#define XMST_PIN_FAILURES		3		/* you have three trials to enter ... */
#define XMST_SC_OBJECT			1		/* object is from sc */
#define XMST_PSE_OBJECT			2		/* object is from sw-pse */
#define XMST_MAX_LIST_ITEMS		400		/* number of list entries */
#define XMST_MAX_ALIAS_LIST_ITEMS	XMST_MAX_LIST_ITEMS - 1		/* number of alias list entries */
#define XMST_MAX_OTHERALIASES		100		/* number of other aliases in selection box */
#define XMST_PSELISTSTR_LENGTH		100		/* list string properties */
#define XMST_PKLISTSTR_LENGTH		100
#define XMST_PKSERIAL_LENGTH		15 		/* max. characters for Serialnumber in pklist */
#define XMST_PKSERIAL_NOCTETS		10 		/* max. number of octets that can be displayed */

#define XMST_ALIASLISTSTR_LENGTH	100

#define XMST_NONE			0
#define XMST_ALIAS_USER			1
#define XMST_ALIAS_SYSTEM		2
#define XMST_ALIAS_BOTH			3				
#define XMST_ALIAS_LOCALNAME		1		/* alias_type_setting values (index) */
#define XMST_ALIAS_RFCMAIL		2				
#define XMST_ALIAS_NEXTBEST		3				
#define XMST_ALIAS_X400MAIL		4	

#ifdef MS_DOS
#define XMST_TEMP_FILE		"sttxttmp"
#define XMST_CONFIG_FILE	"xmstrc"
#else
#define XMST_TEMP_FILE		".st_txt_tmp"
#define XMST_CONFIG_FILE	".xmstrc"
#endif




/*
 *******************************
 *	SecuDE Declaration Stuff
 *******************************
 */


typedef enum {
       ADDALIAS, ADDEK, ADDPCA, ADDPK, ALGS, ALIAS2DNAME, ALIASES,
#ifdef X500
       AUTHNONE, AUTHSIMPLE,
#ifdef STRONG
       AUTHSTRONG,
#endif
#endif
       CACRL, CAPRINTLOG, CASERIALNUMBERS, CAUSERS, CERTIFY,
       CERT2KEYINFO, CERT2PKROOT, CHECK, CHPIN, CLOSE, CREATE, DELALIAS, DELEK, DELETE,
       DELKEY, DELPCA, DELCRL, DELPK, DH1, DH2, DHINIT, DNAME2ALIAS,
#ifdef SCA
       EJECT,
#endif
       ENDE, ENTER, ERROR, EXIT, GENKEY, HELPCMD, INITCRL, ISSUEDCERTIFICATE, KEYTOC,
       MFLIST,
       OPEN, OWNER, PROLONG, PROTOTYPE, QM, QUIT, READ, REMOVE,
       RENAME, RESETERROR, RETRIEVE, REVOKE, SETPARM, SETSERIAL, SHOW,
       SPLIT, STRING2KEY, TOC,
#ifdef SCA
       TOGGLE,
#endif
       TRUSTPATH, VERBOSE, VERIFY, WRITE, XDUMP
} commands;


extern	int     		cmd;
extern	Boolean 		replace;
extern	char    		inp[256];
extern	char    		* cmdname, * helpname, * filename, * pin, * newpin, * algname, * objtype, * attrname;
extern	CertificateType 	certtype;
extern	KeyRef  		keyref;
extern	Key     		*key, *publickey, *secretkey;
extern	Boolean 		interactive;
extern	char 			*pname, *ppin;
extern	int 			pkeyref;
extern	char			*newstring;
extern	int            		fd1, fd2, fdin;
extern	int             	i, anz, n, k, algtype, found;
extern	time_t          	atime, etime;
extern	char	        	opt, x500;
extern	Boolean         	update, create, replace_afdb_cert;
extern	char            	*enc_file, *plain_file, *tbs_file, *sign_file, *hash_file;
extern	char            	*buf1, *buf2, *ii, *xx;
extern	char            	*par, *dd, *ptr, *cc, *afname, *newname, *objname, *number;
extern	char	        	*pgm_name;
extern	char 	        	*revlistpempath;
extern	char            	*pse_name, *pse_path, *ca_dir, *home;
extern	RC              	rcode_dir, rcode_afdb, rcode;
extern	OctetString     	octetstring, *ostr, *objectvalue, *tmp_ostr;
extern	ObjId    		*object_oid, oid;
extern	AlgId           	*algid;
extern	BitString       	bitstring, *bstr;
extern	HashInput       	hashinput;
extern	KeyInfo         	tmpkey, *keyinfo, *signpk, *encpk;
extern	FCPath          	*fcpath;
extern  Certificate          	*fcpath_cert;
extern	PKList          	*pklist;
extern	PKRoot          	*pkroot;
extern	Certificate     	*certificate;
extern	Certificates    	*certs;
extern	ToBeSigned 		*tbs;
extern	SET_OF_Certificate 	*certset, *soc, *tmp_soc;
extern	CertificatePair 	*cpair;
extern	SET_OF_CertificatePair 	*cpairset;
extern	SEQUENCE_OF_CRLEntry 	*revcertpemseq;
extern	Crl			* crlpse;
extern	CrlSet      		* crlset;
extern	Name            	*name, *alias, * issuer, * subject;
extern	DName			* dname, * issuer_dn, * subject_dn, * own_dname, * signsubject, * encsubject;
extern	EncryptedKey    	encryptedkey;
extern	rsa_parm_type   	*rsaparm;
extern	KeyType         	ktype;
extern	AlgEnc          	algenc;
extern	PSESel  		std_pse;
extern	PSESel          	*pse_sel;
extern	PSEToc          	*psetoc, *sctoc;
extern	struct PSE_Objects 	*pseobj;
extern	OctetString		* serial;
extern	SET_OF_IssuedCertificate *isscertset;
extern	SET_OF_Name		*nameset;
extern	UTCTime 		*lastUpdate, *nextUpdate;	
extern	FILE            	*logfile;
extern	AlgList         	*a;
extern	Boolean         	onekeypaironly;
extern	AliasFile		aliasfile;
extern	AliasList               * aliaslist;

#ifdef AFDBFILE
extern	char		 	afdb[256];
#endif

#ifdef X500
extern	int 		 	dsap_index;
extern	char			*callflag;
extern DName 			* directory_user_dname; 
extern int     			count;			
extern char 			** vecptr;   	
extern int     			af_dir_authlevel;	
extern char     		*env_af_dir_authlevel;		
#endif

#ifdef SCA
extern int			SCapp_available;		
#endif

extern char 			*passwd; 	/* store user-typed passwd here. */
extern Boolean			start_with_configuration;
	




/*
 *	Callbacks
 */
void st_destroy_cb();			/* sectool */
void st_quit_button_cb();	
void st_properties_cb();
void st_directory_button_cb();
void st_alias_button_cb();
void st_keypool_button_cb();
void st_debug_button_cb();
void st_viewer_button_cb();
void st_clipboard_button_cb();
void st_config_button_cb();
void st_xlock_button_cb();
void st_psetype_drawnbutton_cb();
void st_pse_check_button_cb();
void st_pse_changepin_button_cb();
void st_pselist_button_cb();
void st_pselist_action_cb();
void st_pklist_button_cb();
void st_pklist_action_cb();
void st_eklist_button_cb();
void st_eklist_action_cb();
void st_show_drawnbutton_cb();
void st_help_drawnbutton_cb();
void st_copy_drawnbutton_cb();
void st_paste_drawnbutton_cb();
void st_cut_drawnbutton_cb();
void st_undo_drawnbutton_cb();
void st_file_drawnbutton_cb();
void st_print_drawnbutton_cb();
void st_create_drawnbutton_cb();
void st_rename_drawnbutton_cb();
void st_activate_list();

void at_destroy_cb();			/* aliastool */
void at_close_button_cb();			
void at_quit_button_cb();
void at_sectool_button_cb();
void at_directory_button_cb();
void at_keypool_button_cb();
void at_debug_button_cb();
void at_viewer_button_cb();
void at_config_button_cb();
void at_xlock_button_cb();
void at_file_toggle_cb();
void at_type_toggle_cb();
void at_aliaslist_action_cb();
void at_find_cb();
void at_apply_drawnbutton_cb();
void at_reset_drawnbutton_cb();
void at_clear_drawnbutton_cb();
void at_insert_drawnbutton_cb();
void at_change_drawnbutton_cb();
void at_delete_drawnbutton_cb();
void at_otheraliases_pushbutton_cb();

void pin_dialog_cb();				/* dialogs */
void chpin_dialog_cb();
void addalias_dialog_cb();	
void ok_dialog_cb();	
void error_dialog_cb();	
void warning_dialog_cb();	
void question_dialog_cb();	
void debug_dialog_cb();
void viewer_dialog_cb();
void otheraliases_dialog_cb();
void objectselection_dialog_cb();
void file_dialog_cb();
void file_selection_dialog_cb();
void file_object_selection_dialog_cb();
void toolquit_dialog_cb();
void clipboard_dialog_cb();
void config_dialog_cb();
void config_accessdir_toggle_cb();
void config_useeditor_toggle_cb();

void check_passwd_cb();	
void textfield_focus_cb();	

Boolean debug_workproc();




/*
 *	other function declarations
 */

Key			*build_key_object();
void			store_objpin();

void			exit_sectool();
Widget			name2widget();
char			*search_add_alias();
void			init_all_clientdata();
void			init_clientdata();
Boolean			insert_clientdata();
Boolean			append_clientdata();
void			delete_clientdata();
int			count_clientdata();
XtPointer		get_clientdata();
char			*Serialnumber2String();
void			set_property_widgets();
void			set_properties();
void			close_tempfile();
void			remove_tempfile();
void			start_editor();
void 			get_configuration();
void 			put_configuration();
void 			reset_configuration();

void			at_init_tool();
void			at_check_aliaslist();
void			at_save_aliaslist();
void			at_fill_list();
void			at_fill_textfields();
void			pin_dialog_open();
void			chpin_dialog_open();
void			ok_dialog_open();
void			abort_dialog_open();
Boolean			cont_canc_dialog_open();
void			cont_quit_dialog_open();
Boolean			save_loose_dialog_open();
Boolean			cont_show_dialog_open();
void			otheraliases_dialog_open();
char			*objectselection_dialog_open();
void			toolquit_dialog_open();
Boolean			config_dialog_open();
void			file_dialog_open();
Boolean			signature_dialog_open();



