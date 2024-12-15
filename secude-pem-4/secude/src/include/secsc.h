/*
 *  SecuDE Release 4.1 (GMD), STARCOS Version 1.1
 */

/******************************************************************
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

/*-----------------------------------------------------------------------*/
/* INCLUDE FILE  secsc.h                                                 */
/* Definition of structures and types for the 				 */
/* SEC-IF - SCA-IF interface module (secsc).				 */
/*-----------------------------------------------------------------------*/

#ifndef _SECSC_
#define _SECSC_

/*
 *
 *   secure.h defines:
 *          AlgId               (typedef struct AlgId)
 *          BitString           (typedef struct Bitstring)
 *          Boolean             (typedef char)
 *          EncryptedKey        (typedef struct EncryptedKey)
 *          ENCRYPTED           (typedef struct BitString)
 *          EncryptedKey        (typedef struct EncryptedKey) 
 *          KeyBits             (typedef struct KeyBits)
 *          More                (typedef enum { SEC_END, SEC_MORE })
 *          ObjId               (typedef struct ObjId)
 *          OctetString         (typedef struct Octetstring)
 *          Signature           (typedef struct Signature)
 *
 *   sca.h defines:
 *           Structures and types of the SCA-IF
 */

#include "sca.h"

/*-----------------------------------------------------------------------*/
/*     G l o b a l s     						 */
/*-----------------------------------------------------------------------*/

extern Boolean SC_verify, SC_encrypt, SC_ignore, SC_ignore_SWPSE;

extern int     SC_timer;	/* During this time interval (in seconds) the 	*/
				/* SCT accepts the insertion of an SC.		*/
				/* The timer starts after the user has been 	*/
				/* requested to insert the SC.			*/
				/* If the variable is set to 0, no timer is     */
				/* specified.					*/

/*
 *  External initialization of SC_timer:
 *  
  int		SC_timer =	SC_WAITTIME;
 */



/*-----------------------------------------------------------------------*/
/*     E r r o r  -  c o d e s						 */
/*-----------------------------------------------------------------------*/
#define ERR_flag	-1
#define EOF_flag	-2
#define EOF_with_ERR	-3
  
#define NOERR		 0

/*
 *   Error codes defined in secure.h
 */	


/*-----------------------------------------------------------------------*/
/*     D e f i n i t i o n s      					 */
/*-----------------------------------------------------------------------*/
#define USER_BELL	 "\007" /* Control character  to "ring the bell" */
 


/*-----------------------------------------------------------------------*/
/*     T e x t  - ,  C h a r a c t e r  -  D e f i n i t i o n s  	 */
/*-----------------------------------------------------------------------*/
#define SC_CONFIG_name   ".scinit1"
#define	SCT_CONFIG_name	 ".sctinit1."		/* Name of the SCT configuration file   */
						/* (one file per SCT)			*/
#define DEF_SC_APP_ID    "D27600000701"		/* Default application identifier, is	*/
						/* used, if no app_id is defined in 	*/
						/* SCINITFILE.				*/
#define SC_PROCESS_KEY	 "SC_PROCESS_KEY"	/* Name of an environment variable	*/

#define APP_KEY_WORD     "APPLICATION"
#define APP_ID_KEY_WORD  "APPLICATION_ID"
#define OBJ_KEY_WORD     "OBJECT"
#define IGN_KEY_WORD	 "IGNORE"
#define SC_ENC_KEY_WORD	 "SC_ENCRYPT"
#define SC_VER_KEY_WORD	 "SC_VERIFY"
#define TRUE_WORD	 "TRUE"
#define FALSE_WORD	 "FALSE"
#define SC_KEY_WORD	 "SC_KEY"
#define SC_FILE_WORD	 "SC_FILE"
#define NORM_WORD	 "NORM"
#define INTG_WORD	 "INTG"
#define AUTH_WORD	 "AUTH"
#define CONC_WORD	 "CONC"
#define COMB_WORD	 "COMB"

#define COMMENT		 '#'
#define BLANK_CHAR	 ' '
#define TAB		 '\t'
#define COMMA		 ','
#define EQUAL		 '='
#define CR_CHAR		 '\r'			/* carriage return */


 

/*-----------------------------------------------------------------------*/
/*     M a x  - M i n  -  Definitions					 */
/*-----------------------------------------------------------------------*/
#define PSE_PIN_L	  8	/* length of the PSE-PIN stored on the SC 	*/
#define MAX_PIN_L	  8	/* max length of the PIN to be installed on a 	*/
				/* keycard.					*/
#define MAXL_APPNAME  	256	/* max length of the name of an application.	*/
#define APPID_L		  6	/* length of the application identifier 	*/
#define UNIT_SIZE	 32	/* Size of a file on the SC is specified in     */
				/* units					*/
#define MAX_SC_SPACE 	100     /* Max. number of free units on the SC		*/   
#define MAX_TOC_ENTRY    50     /* Max. number of bytes neccessary for one 	*/
				/* Toc entry (in ASN.1 encoding.		*/
#define MAX_PIN_FAIL	  3	/* Max number of faulty PIN authentications	*/
#define MAX_SCRESET_FAIL  3	/* Max number of failed SC resets		*/
#define WEF_LEN_BYTES	  2	/* No. of bytes used to store the length of     */
                                /* a WEF on the SC.				*/
#define SC_WAITTIME	 20	/* After the request an SC is accepted within 	*/
				/* this time (in seconds).			*/
#define MAX_RECORD      512	/* max length of record in configuration file	*/
#define NO_OF_SM	  4	/* max number of secure messaging parameters    */
				/* (SCT/SC) for one object in configuration file*/
#define	MAXSCTID_LEN	  2	/* No. of ASCII characters used for the sct_id.	*/
#define PROCESS_KEY_LEN	  8	/* length of the process key			*/
#define MAX_LEN_PROC_KEY 64	/* Max length of the process key.		*/
#define MIN_SHORT_INDEX   1	/* Min. value of short_index (part of FileId)	*/
#define MAX_SHORT_INDEX  14	/* Max. value of short_index (part of FileId)	*/
				/* FileId is used to address a file on the SC.	*/
				/* short_index = 15 is reserved for an XEF 	*/
#define MIN_KEY_NUMBER    1	/* Min. value of key_number (part of KeyId)	*/
#define MAX_KEY_NUMBER   31	/* Max. value of key_number (part of KeyId)	*/
				/* KeyId is used to address a key on the SC.	*/
#define MAX_DEVKEY_NO	  3	/* Max. number of device keys			*/

/*-----------------------------------------------------------------------*/
/*     Max. number of list entries					 */
/*-----------------------------------------------------------------------*/
#define MAX_SCAPP 	20	/* max # of applications stored on the smartcard*/
				/* List: sc_app_list[]				*/
#define MAX_SCOBJ 	22	/* max # of objects stored on the smartcard	*/
				/* List: sc_obj_list[]				*/
#define MAX_SCTNO 	20	/* max # of SCTs				*/
				/* List: sct_stat_list[]			*/



/*-----------------------------------------------------------------------*/
/*     Texts for the SCT-display					 */
/*-----------------------------------------------------------------------*/
/*				  12345678901234561234567890123456	 */
#define SCT_TEXT_PIN_LOCKED	 "PIN on SC locked                "
#define SCT_TEXT_PUK_LOCKED	 "PUK on SC locked		  "
#define SCT_TEXT_PIN_INVALID	 "  PIN invalid    		  "
#define SCT_TEXT_PUK_INVALID 	 "  PUK invalid                   "
#define SCT_TEXT_NEW_PIN_INV	 " New PIN invalid                "
#define SCT_TEXT_NEW_PIN_CPIN	 " Clear PIN not      allowed     "
#define SCT_TEXT_RESET_SC_ERR	 "  Reset of SC       failed!     "
#define SCT_TEXT_SCT_CHECK	 "   SCT check                    "
#define SCT_TEXT_REQ_N_KEYCARD	 "Please insert a  new smartcard  "    
#define SCT_TEXT_REQ_O_KEYCARD	 "Please insert a     keycard     "    


/*-----------------------------------------------------------------------*/
/*     Type definitions							 */
/*-----------------------------------------------------------------------*/
typedef	enum   {ALL_SCTS, 
	        CURRENT_SCT}		SCTSel;
typedef struct  SCT_Status		SCTStatus;
typedef struct  SC_Sel           	SCSel;
typedef	struct  SCAppEntry		SCAppEntry;
typedef	struct  DevKeyEntry		DevKeyEntry;
typedef	struct  SCObjEntry		SCObjEntry;
typedef	enum   {SC_KEY_TYPE, 
		SC_FILE_TYPE}		SCObjType;
typedef	struct  SCAFctPar		SCAFctPar;
typedef enum   {SYSTEM_CONF, 
		USER_CONF}		WhichSCConfig;



/*-----------------------------------------------------------------------*/
/*     Selection of the SCT and the SC					 */
/*-----------------------------------------------------------------------*/

struct SC_Sel {
       int      sct_id;         		/* selected sct number   	*/

};
extern SCSel sc_sel;

/*
 *  External initialization of sc_sel:
 *  
 #	1    -> select first SCT in the installation file (list of the connected SCTs)
  SCSel		sc_sel =	{ 1 };
 */





/*
 *  Definitions of security mode(s) used for the communication between DTE-SCT
 */

#define SCT_NORM	SCT_SEC_NORMAL			/* Normal mode			*/
#define SCT_INTG	SCT_INTEGRITY			/* Authentic mode		*/
#define SCT_CONC	SCT_CONCEALED			/* Concealed mode		*/
#define SCT_COMB	SCT_COMBINED			/* Authentic and concealed mode	*/

/*
 *  Definitions of security mode(s) used for the communication between SCT-SC
 */

#define SC_NORM		ICC_SEC_NORMAL			/* Normal mode			*/
#define SC_AUTH		ICC_AUTHENTIC			/* Authentic mode		*/
#define SC_CONC		ICC_CONCEALED			/* Concealed mode		*/
#define SC_COMB		ICC_COMBINED			/* Authentic and concealed mode	*/


/*
 *  Definitions of NULL-Pointers
 */

#define NULLSCAPPENTRY  (SCAppEntry *) 0 
#define NULLSCOBJENTRY  (SCObjEntry *) 0 




/*-----------------------------------------------------------------------*/
/*     SCT-List								 */
/*     Contains current status information for max. 20 SCTs		 */
/*-----------------------------------------------------------------------*/
   
struct SCT_Status {
	Boolean	    config_done;		/* = TRUE,  configuration done, e.g.    */
						/*          process data read.		*/ 
						/* = FALSE, configuration not yet done. */
	Boolean     available;			/* = TRUE , SCT is available		*/
						/* = FALSE, SCT is not available	*/
        SCT_SecMess sm_SCT;			/* Security mode for the command and	*/
						/* response exchange between DTE and SCT*/
	char	    app_name[MAXL_APPNAME+1];	/* name of the application on the SC,   */
						/* which has been opened, else = CNULL  */
	char	    sw_pse_pin[PSE_PIN_L+1];	/* PIN for the application on the 	*/
						/* SW-PSE, else = CNULL  		*/
	Boolean	    user_auth_done;		/* = TRUE,  if user authentication has  */
						/*          been performed.		*/ 
						/* = FALSE, if user authentication has  */
						/*          not been performed.		*/ 
	SCTPort	    SCA_config_data;		/* Used to initialize the STARMOD modul */
						/* with internal configuration data from*/
						/* a previous process.			*/
};
	


extern SCTStatus		sct_stat_list[MAX_SCTNO+1];	





/*-----------------------------------------------------------------------*/
/*     P S E  -  O b j e c t s, if a smartcard environment is available  */
/*-----------------------------------------------------------------------*/

/*
 *  Names of PSE-Objects additional to the PSE-Objects defined in filename.h
 */

#define SC_PIN_name     "SC_PIN"  		 /* PIN for the smartcard	*/
#define SC_PUK_name     "SC_PUK"  		 /* PUK for the smartcard	*/
#define PSE_PIN_name    "PSE_PIN"  		 /* PIN for the SW-PSE		*/
#define SCToc_name      "SCToc"  		 /* Toc in the SC		*/

#define SM_AUTH_name   	"SM_Auth"  		 /* Secure messaging key for an */
						 /* authentic message transfer.	*/
#define SM_CONC_name   	"SM_Conc"  		 /* Secure messaging key for an */
						 /* concealed message transfer.	*/
#define AUTH_name     	"Auth"  		 /* Device key used for the 	*/
						 /* authentication between SCT  */
						 /* and SC. 			*/


/*-----------------------------------------------------------------------*/
/*									 */
/*  If a smartcard (SC) environment is available,			 */
/*     the following list is used to address objects stored on the SC.   */
/*  This list is integrated within the list of the SC-applications 	 */
/*									 */
/*-----------------------------------------------------------------------*/
   
struct SCObjEntry {
        char		*name;			/* SC object name			*/
	SCObjType	type;			/* Type of the object (key, file)	*/
	unsigned int	sc_id;			/* Identifier for a key/file on the SC	*/
						/* if type = SC_FILE, id = short_index	*/
						/* if type = SC_KEY,  id = key_number	*/
	unsigned int	size;		        /* size of a file on the SC             */
	SCT_SecMessMode	sm_SCT;			/* Security mode (one value) for the 	*/
						/* command and response exchange 	*/
						/* between DTE and SCT.			*/
	ICC_SecMess	sm_SC_read;		/* Security mode for the command and	*/
						/* response exchange between SCT/SC, if */
						/* the PSE-Object is read from the SC.	*/
						/* Separat values for cmd. and response.*/
	ICC_SecMess	sm_SC_write;		/* Security modes for the command and	*/
						/* response exchange between SCT/SC, if */
						/* the PSE-Object is written to the SC.	*/
						/* Separat values for cmd. and response.*/
};


/*-----------------------------------------------------------------------*/
/*									 */
/*  If a smartcard (SC) environment is available,			 */
/*     the following list contains the names of the applications and the */ 
/*     belonging objects stored on the SC.				 */
/*									 */
/*-----------------------------------------------------------------------*/
   
struct SCAppEntry {
        char		*app_name;		   /* SC application name	  	  */
	OctetString	*app_id;		   /* SC application identifier		  */
        Boolean		ignore_flag;		   /* TRUE:  If the Software-PSE part     */
						   /*        cannot be opened with the pin*/
						   /*	     from SC_PIN_name, sec_open	  */
						   /*	     ignores this error.	  */
                                      		   /* FALSE: sec_open returns -1 in this  */
						   /*	     case          		  */
	SCObjEntry	sc_obj_list[MAX_SCOBJ+1];  /* List of the objects belonging to    */
						   /* the application			  */

};

extern SCAppEntry	sc_app_list[MAX_SCAPP+1];	


/*
 *  External initialization of sc_app_list[]:
 *  
  SCAppEntry		sc_app_list[0] = {0};
 *
 */




/*-----------------------------------------------------------------------*/
/*									 */
/*  If a smartcard (SC) environment is available,			 */
/*     the following list contains the names of the device keys and the  */ 
/*     belonging purpose,which is used at the SCA-IF.			 */
/*									 */
/*-----------------------------------------------------------------------*/

struct DevKeyEntry {
        char		*name;			/* SC object name			*/
	KeyPurpose	purpose;		/* Purpose of the object		*/
};


extern DevKeyEntry	default_devkeylist[MAX_SCOBJ+1]; 

/*
 *  External initialization of default_devkeylist[]:
 *  

DevKeyEntry	default_devkeylist[MAX_DEVKEY_NO + 1] =
 {
	AUTH_name,    PURPOSE_AUTH,
	SM_AUTH_name, PURPOSE_SECAUTH,
        SM_CONC_name, PURPOSE_SECCONC,
	0
  }
 *
 */


/*-----------------------------------------------------------------------*/
/*									 */
/*  If a smartcard (SC) environment is available, 			 */
/*     the SEC-IF function calls are transformed into SCA-IF fct. calls. */
/*  The SCA-IF functions are transformed into a command and response 	 */
/*  protocol, which is used between the DTE/SCT and SCT/SC.		 */
/*									 */
/*  The following list contains the SCA-Functions and their security 	 */
/*  mode(s) for the communication between DTE/SCT and SCT/SC, resp.:	 */
/*	 								 */
/*-----------------------------------------------------------------------*/

struct SCAFctPar {
        char		*fct_name;		/* SCA-Function name			*/
	SCT_SecMess	sm_SCT;			/* Security mode for the  command and	*/
						/* response exchange between DTE and SCT*/
	ICC_SecMess	sm_SC;			/* Security mode for the command and	*/
						/* response exchange between SCT and SC.*/
						/* Separat values for cmd. and resp..	*/
};


extern SCAFctPar	sca_fct_list[];	


/*
 *  External initialization of sca_fct_list[]:
 *  
  SCAFctPar	sca_fct_list[] =

#   fct_name,		 	      sm_SCT,	            sm_SC
#				 command,  response	command, response

  {
	"sca_init_icc", 	 SCT_NORM, SCT_NORM, 	SC_NORM, SC_NORM,
	"sca_get_sct_info",	 SCT_NORM, SCT_NORM, 	SC_NORM, SC_NORM,
	"sca_eject_icc",	 SCT_NORM, SCT_NORM, 	SC_NORM, SC_NORM,
	"sca_display",		 SCT_NORM, SCT_NORM, 	SC_NORM, SC_NORM,
	"sca_gen_user_key",	 SCT_NORM, SCT_NORM, 	SC_NORM, SC_NORM,
	"sca_inst_user_key",	 SCT_NORM, SCT_NORM, 	SC_NORM, SC_NORM,
	"sca_del_user_key",	 SCT_NORM, SCT_NORM, 	SC_NORM, SC_NORM,
	"sca_sign",		 SCT_NORM, SCT_NORM, 	SC_NORM, SC_NORM,
	"sca_verify_sig",	 SCT_NORM, SCT_NORM, 	SC_NORM, SC_NORM,
	"sca_encrypt",		 SCT_CONC, SCT_NORM, 	SC_CONC, SC_NORM,
	"sca_decrypt",		 SCT_NORM, SCT_CONC, 	SC_NORM, SC_CONC,
	"sca_enc_des_key",	 SCT_NORM, SCT_NORM, 	SC_NORM, SC_NORM,
	"sca_dec_des_key",	 SCT_NORM, SCT_NORM, 	SC_NORM, SC_NORM,
	"sca_activate_devkeyset",SCT_NORM, SCT_NORM, 	SC_NORM, SC_NORM,
	"sca_auth",		 SCT_NORM, SCT_NORM, 	SC_NORM, SC_NORM,
	"sca_gen_sessionkey",	 SCT_NORM, SCT_NORM, 	SC_NORM, SC_NORM,
	"sca_del_sessionkey",	 SCT_NORM, SCT_NORM, 	SC_NORM, SC_NORM,
	"sca_create_file",	 SCT_NORM, SCT_NORM, 	SC_NORM, SC_NORM,
	"sca_select_file",	 SCT_NORM, SCT_NORM, 	SC_NORM, SC_NORM,
	"sca_close_file",	 SCT_NORM, SCT_NORM, 	SC_NORM, SC_NORM,
	"sca_delete_file",	 SCT_NORM, SCT_NORM, 	SC_NORM, SC_NORM,
	"sca_set_mode",		 SCT_NORM, SCT_NORM, 	SC_NORM, SC_NORM,
	CNULL
  };
 */



/* Function prototypes  */

#include "secsc_prototypes.h"

#endif
