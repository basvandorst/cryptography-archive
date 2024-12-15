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

/*-----------------------init.c-------------------------------------*/
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
/* FILE      init.c                                                 */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/* DESCRIPTION                                                      */
/*	Defines & declares all types & functions  		    */
/*                                                                  */
/*------------------------------------------------------------------*/




/*
 ************************
 *	includes	*
 ************************
 */

#include "xmst.h"



/*
 *************************************
 *	Declaration & Definition Stuff
 *************************************
 */

XrmDatabase	config_database;

/*
 *	Self defined X Resource values
 */
char		*external_editor;
Cursor		busy_cursor;
Pixel		focus_color;
Pixel		active_list_color;

/*
 *	LIST CLIENT DATA type
 *	This is my quick & dirty solution for XmList client data substitute.
 *	Hope there will be such an enhancement in the standard Motif lib in future...
 *
 *	Each XmList has an client data vector, element[0] is index of first unused entry.
 *	element[0] == 1 means vector stores no client data
 *	To simplify handling the last vector item is not used.
 *	The convenience functions xxx_clientdata() will only work for unique item values!
 *
 */
XtPointer	pselist_clientdata[XMST_MAX_LIST_ITEMS];
XtPointer	pklist_clientdata[XMST_MAX_LIST_ITEMS];
XtPointer	eklist_clientdata[XMST_MAX_LIST_ITEMS];
XtPointer	aliaslist_clientdata[XMST_MAX_LIST_ITEMS];

/*
 *	UIL
 */
MrmHierarchy	applicationHierarchy;
char		*applicationUidVector[] = {"xmst.uid"};
MrmType		opaqueClass;

/*
 *	Display/Screen/Shells/Widgets
 */
XtAppContext	applicationContext;
Display		*display;
Screen		*screen;
Widget		applicationShell;
Widget		stMainShell, atMainShell, dtMainShell;
Widget		atToplevel, dtToplevel;
Widget		initialAuthToggle, initialNiceToggle;
Widget		pseList, pkList, ekList;
Widget		activeList;


/*
 *	Pixmaps
 */
Pixmap		sectoolPixmap;
Pixmap		aliastoolPixmap;
Pixmap		dirtoolPixmap;
Pixmap		scPixmap;
Pixmap		psePixmap;
Pixmap		scpsePixmap;
Pixmap		onepairPixmap;
Pixmap		twopairsPixmap;

/*
 *	Args, Flags, Names
 */
Arg		args[11];				/* Global Arg vector for single XtSetValues: handle with care! */
int 		sectool_cnt;
char 		**sectool_parm;
Boolean		debugmode_on;				/* Flag for Debug Toggle Button state*/
Boolean		alias_save_required;			/* Alias Editor has made changes in Alias List */
Boolean 	verbose = FALSE;
Boolean 	sectool_verbose = FALSE;
Boolean		Debug = FALSE;				/* Flag for Debugger initial state */
Boolean		auto_save_configuration = FALSE;	/* save configuration in .xinitrc on exit */
Boolean		auto_detect_sct = FALSE;
Boolean		confirm_overwrite = FALSE;
char		*unix_home;
char		editor_tempfile[256];
char		config_rcfile[256];
char		system_aliasfile[256];
char		dialog_message[256];
char		*alias_to_add;
char		*alias_message[] = {	"Alias",
					"Internet Mail Address",
					"X.400 Address",
					"Local Name"			};
int		pin_failure_count;


/*
 *******************************
 *	SecuDE Declaration Stuff
 *******************************
 */


int     		cmd;
Boolean 		replace;
char    		inp[256];
char    		* cmdname, * helpname, * filename, * pin, * newpin, * algname, * objtype, * attrname;
CertificateType 	certtype;
KeyRef  		keyref;
Key     		*key, *publickey, *secretkey;
Boolean 		interactive = TRUE;
char 			*pname, *ppin;
int 			pkeyref;
char			*newstring;
int            		fd1, fd2, fdin;
int             	i, anz, n, k, algtype, found;
time_t          	atime, etime;
char	        	opt, x500 = TRUE;
Boolean         	update, create, replace_afdb_cert;
char            	*enc_file, *plain_file, *tbs_file, *sign_file, *hash_file;
char            	*buf1, *buf2, *ii, *xx;
char            	*par, *dd, *ptr, *cc, *afname, *newname, *objname, *number;
char	        	*pgm_name;
char 	        	*revlistpempath;
char            	*pse_name = CNULL, *pse_path = CNULL, *ca_dir = CNULL, *home;
RC              	rcode_dir, rcode_afdb, rcode;
OctetString     	octetstring, *ostr, *objectvalue, *tmp_ostr;
ObjId    		*object_oid, oid;
AlgId           	*algid;
BitString       	bitstring, *bstr;
HashInput       	hashinput;
KeyInfo         	tmpkey, *keyinfo, *signpk, *encpk;
FCPath          	*fcpath;
Certificate          	*fcpath_cert;
PKList          	*pklist;
PKRoot          	*pkroot;
Certificate     	*certificate;
Certificates    	*certs;
ToBeSigned 		*tbs;
SET_OF_Certificate 	*certset, *soc, *tmp_soc;
CertificatePair 	*cpair;
SET_OF_CertificatePair  *cpairset;
SEQUENCE_OF_CRLEntry 	*revcertpemseq;
Crl			* crlpse;
CrlSet      		* crlset;
Name            	*name, *alias, * issuer, * subject;
DName			* dname, * issuer_dn, * subject_dn, * own_dname, * signsubject, * encsubject;
EncryptedKey    	encryptedkey;
rsa_parm_type   	*rsaparm;
KeyType         	ktype;
AlgEnc          	algenc;
PSESel  		std_pse;
PSESel          	*pse_sel;
PSEToc          	*psetoc, *sctoc;
struct PSE_Objects 	*pseobj;
OctetString		*serial;
SET_OF_IssuedCertificate *isscertset;
SET_OF_Name		*nameset;
UTCTime 		*lastUpdate, *nextUpdate;	
AlgList         	*a;
Boolean         	onekeypaironly = FALSE;
AliasFile		aliasfile;
AliasList               *aliaslist;

#ifdef AFDBFILE
char		 	afdb[256];
#endif

#ifdef X500
int 		 	dsap_index;
char			*callflag;
DName 			*directory_user_dname;
int     		count;			
char 			**vecptr;   		
int     		af_dir_authlevel;		
char     		*env_af_dir_authlevel;		
#endif

#ifdef SCA
int			SCapp_available;		
#endif

char 			*passwd; 	/* store user-typed passwd here. */
Boolean			start_with_configuration = FALSE;





