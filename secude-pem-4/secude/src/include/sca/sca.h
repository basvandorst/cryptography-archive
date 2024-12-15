
/*
 *  STARMOD Release 1.1 (GMD)
 */
/*-------------------------------------------------------+-----*/
/*							 | GMD */
/*   SYSTEM   STAPAC  -  Version 1.0			 +-----*/
/*							       */
/*-------------------------------------------------------------*/
/*							       */
/*    PACKAGE	SCA			VERSION 1.1	       */
/*					   DATE January 1993   */
/*					     BY Levona Eckstein*/
/*							       */
/*    FILENAME			                 	       */
/*      sca.h    		         		       */
/*							       */
/*    DESCRIPTION					       */
/*      This file contains all structures and types for the    */
/*	STAPAC Application Interface         		       */
/*-------------------------------------------------------------*/
#ifndef _SCA_
#define _SCA_
/*
 *   secure.h defines:
 *
 *         AlgId,
 *         BitString,  
 *         Boolean, 
 *         EncryptedKey,
 *         ENCRYPTED,
 *         KeyBits,
 *         More,
 *         ObjId,
 *         OctetString, 
 *         Signature,
 */

#ifndef _SECURE_
#include "secure.h"
#endif

/*-------------------------------------------------------------*/
/*   global variable definitions			       */
/*-------------------------------------------------------------*/
extern unsigned int	sca_errno;	/* error number set bei STARMOD */
extern char		*sca_errmsg;	/* pointer to error message set */
					/* by STARMOD			*/



/*------------------------------------------------------------------*/
/*  NULL - Pointer definitions                                      */
/*------------------------------------------------------------------*/
#define KEYATTRNULL	(KeyAttrList *)0
#define KEYIDNULL	(KeyId *)0
#define KEYINFONULL	(KeyInfo *)0


/*------------------------------------------------------------------*/
/*  MAX values for parameters of the application interface          */
/*------------------------------------------------------------------*/


#define MAX_FILENAME    8       /* max. length of file name for Parent DF        */
#define MAX_FILEID      2       /* max. length of file name for MF / child of DF */
#define MAX_ADDINFO     4       /* max. length of add info of file control info */


/*------------------------------------------------------------------*/
/*  Definitions                                                     */
/*------------------------------------------------------------------*/


#ifndef TRUE
#define TRUE		1
#endif
#ifndef FALSE
#define FALSE		0
#endif


/*------------------------------------------------------------------*/
/*  General type definitions                                        */
/*------------------------------------------------------------------*/

typedef enum {DTE_ICC,SCT_ICC}AuthObjectId;
					/* object to be authenticated     */
typedef enum {IN_AUTH=0x42,EX_AUTH, MU_AUTH=0x45,MU_AUTHWithINITIAL}AuthProcId;
					/* authentication procedure       */

/* ATR information notification   */
typedef enum {NO_ATR, ATR_HISTORICAL} ATRInfo;
typedef enum {SCT, ICC} Destination;
typedef enum {GET_CHALLENGE, SEND_CHALLENGE,  EXCHANGE_CHALLENGE} 
ChallengeMode;


/*------------------------------------------------------------------*/
/*  Definitions for secure messaging                                */
/*------------------------------------------------------------------*/
typedef enum {TRANSP, SECURE}   TransMode;        /* transfer mode for the     */ 
						  /* function sca_trans        */
                                                  /* of smartcard commands     */

typedef enum {ICC_SEC_NORMAL,ICC_AUTHENTIC,ICC_CONCEALED,ICC_COMBINED}
ICC_SecMessMode;
						  /* security mode for secure messaging
						     between SCT and ICC			*/
typedef enum {SCT_SEC_NORMAL,SCT_INTEGRITY,SCT_CONCEALED,SCT_COMBINED}
SCT_SecMessMode;
						  /* security mode for secure messaging
						     between DTE and SCT			*/


typedef struct ICC_Sec_Mess {
		ICC_SecMessMode command;
                ICC_SecMessMode response;
               } ICC_SecMess;

typedef struct SCT_Sec_Mess {
		SCT_SecMessMode command;
                SCT_SecMessMode response;
               } SCT_SecMess;
/*------------------------------------------------------------------*/
/*  Type Definitions for Smartcard Files                            */
/*------------------------------------------------------------------*/
typedef  enum {MF, PARENT_DF, CHILD_DF, EF} 	FileCat;           
						  /* file category               */


typedef  enum { LIN_FIX=1, LIN_VAR, CYCLIC, TRANSPARENT} DataStruc;
                                                  /*  structure of an elementary */
						  /*  file                       */ 

typedef enum {ACCESS_NORMAL, ACCESS_AUTH, ACCESS_CONC, ACCESS_COMBINED } AccessSecMode;
						  /* access security mode */

typedef enum {READ_WRITE,WORM,READ_ONLY,WRITE_ONLY} ReadWrite;
						  /* read/write access mode      */
                                                  /* (part of FileControlInfo)   */ 

typedef  enum {MF_LEVEL, DF_LEVEL, CHILD_LEVEL} FileLevel;
                                                  /* level of an elementary file */

typedef  enum {XEF, WEF, ACF, ISF} 		FileType;      
						  /* type of an elementary file  */

typedef enum {ST_EQ, ST_LT, ST_GE, ST_NOT_EQ} 	CompareMode;
						  /* compare mode		*/
						  /* the access is only allowed, if 
						     the actual state is
						     ST_EQ 	= equal the state number
						     ST_LT      = lower than the state number
						     ST_GE      = greater or equal the
								  state number
						     ST_NOT_EQ  = not equal the state number
						  */

typedef enum {FILE_UNLOCKED, FILE_LOCKED} 	FileAccess;
						  /* Status of file access       */

typedef enum {MEM_CONSISTENT, MEM_INCONSISTENT} FileMemory;
						  /* Status of file memory       */

typedef enum {REGISTERED,DELETED,DEL_PENDING,INSTALLED} InstallStatus;
						  /* Status of file installation */

typedef  enum {CLOSE_CREATE, CLOSE_SELECT} 	FileCloseContext;
                                                  /* finish a creation process   */
                                                  /* or close a selected file    */


typedef enum {REC_UPDATE=0x04, REC_APPEND=0x06} WriteMode;


typedef  enum {NONE_INFO, COMP_INFO=0x04} 	FileInfoReq;
                                                  /* file information requested  */



typedef  OctetString AddInfo;                     /* additional information      */


typedef struct Access_Control {                   /* Access control value        */ 
        CompareMode  compare_mode;                /* compare mode                */
        unsigned int state_number;                /* state_number 
						     (Value: 0 - 63 allowed)     */
}
                          AccessControlValue;
typedef struct File_Id {                          /* File_id for an elementary   */
                                                  /* file (EF)                   */
	char	      file_qualifier;		  /* file qualifier = 
						     0x00 in this version        */   
        FileLevel     file_level;                 /* level of the EF             */
        FileType      file_type;                  /* type of the EF              */
        unsigned int  short_index;                /* possible values: 1-15       */
}                          FileId;

typedef union File_Sel {
        OctetString    	*file_name;               /* to select an MF, DF or Child of DF  */
						  /* in case of a Child the length must be 2 */
        FileId   	*file_id;                 /* to select an EF  */
}                          FileSel;

typedef struct File_Cont_Info {                   /* file control information    */
        unsigned int		units;            /* number of units             */
        AccessControlValue 	delrec_acv;       /* delete record acv           */
        AccessControlValue 	racv;             /* read acv                    */
        AccessControlValue 	wacv;             /* write acv                   */
        AccessControlValue 	delfile_acv;      /* delete file acv             */
	AccessSecMode	     	access_sec_mode;  /* security mode for read / 
						     write access */	
        ReadWrite    	readwrite;                /* READ / WRITE - Mode         */
        Boolean      	execute;                  /* execute - mode              */
        Boolean      	not_erasable;             /* erase - flag                */
        unsigned int 	recordsize;               /* record size                 */
        FileSel      	*file_sel;                /* filename / file_id          */
        AddInfo  	*addinfo;                 /* additional information      */
}                      FileControlInfo;


typedef struct File_Status {                      /* file status                 */
        InstallStatus  install_status;
        FileMemory     file_memory;
        FileAccess     file_access;                    
}                          FileStatus;


typedef struct File_Info {                        /* file information            */ 
        FileStatus  file_status;                  /* file status                 */
        AddInfo     *addinfo;                     /* additional information      */
}
                          FileInfo;

/*------------------------------------------------------------------*/
/*  structure for key handling                                      */
/*------------------------------------------------------------------*/

typedef enum {INST,REPL} KeyInstMode;             /* the key is to be installed  */
                                                  /* or replaced                 */ 

typedef enum {FILE_DUMMY, FILE_MF, FILE_DF, FILE_CHILD, FILE_MASTER, FILE_CURRENT, FILE_PARENT, INITKEY} KeyLevel;
   					          /* specifies where the key is  */
                                                  /* stored                      */  

typedef enum {PURPOSE_CIPHER=0x0F,PURPOSE_SECCONC=0x17,
	      PURPOSE_SECAUTH=0x1D,PURPOSE_AUTH=0x1E} KeyPurpose; 
                     
typedef enum {KEY_GLOBAL,KEY_LOCAL} KeyPresent;   /* global use or local use     */
                                                  /* of the key                  */

typedef enum {PIN,PUK} VerifyDataType;            /* type of the verification data */ 


typedef enum {DEV_OWN, DEV_ANY } KeyDevStatus;    /* status of a device key      */
                                                  /* OWN=dev_key for local use   */
                                                  /* ANY=dev_key for distribution*/    


typedef struct Key_Status {                       /* ICC internal status of the   */
                                                  /* key/PIN                     */  
        Boolean   PIN_check;                      /* FALSE=inactive              */
                                                  /* TRUE =active                */
        Boolean   key_state;                      /* State of the key            */
						  /* TRUE(1) = key locked        */
						  /* FALSE(0)= key unlocked      */
}            KeyStatus;

typedef struct Key_attr {                         /* key attributes              */
        Boolean       key_repl_flag;              /* replaceable/not replaceable */
						  /* TRUE(1) = replaceable   */
						  /* FALSE(0)= not replaceable       */
        Boolean       key_chg_flag;               /* changeable/not changeable   */
						  /* TRUE(1) = changeable    */
						  /* FALSE(0)= not changeable       */
        Boolean       key_foreign_flag;           /* key is replaceable by a key */
						  /* with another algorithm      */
						  /* TRUE(1) = allowed       */
						  /* FALSE(0)= not allowed           */
        KeyPurpose    key_purpose;                /* purpose of the key          */ 
        KeyPresent    key_presentation;           /* key presentation            */ 
        unsigned int  MAC_length;                 /* length of MAC               */
} KeyAttr;

struct KEYAttrList {                             /* key attribute list for the ICC*/
        KeyInstMode   key_inst_mode;              /* installation or replacement  */
        KeyAttr       key_attr;                   /* several key attributes       */
        unsigned int  key_fpc;                    /* key fault presentation counter*/
        KeyStatus   key_status;                   /* ICC internal status of the key*/
};
  
typedef struct KEYAttrList KeyAttrList;

struct KEYId   {                                  /* key identifier              */
        KeyLevel     key_level;                   /* key level                   */
        unsigned int key_number;                  /* number in the range 1 to 31 */ 
};

typedef struct KEYId KeyId;

typedef struct Key_Sel {                          /* this structure identifies   */
                                                  /* a key                       */
        AlgId	    *key_algid;			  /* Alg-Id of the key	         */
	KeyBits     *key_bits;			  /* In case of rsa key this     */
                                                  /* structure contains the      */
                                                  /* public key                  */
						  /* (part1 = modulus m;	 */
						  /*  part2 = exponent e)	 */
        KeyId       *key_id;                      /* key identifier              */
}                          KeySel;

typedef struct PIN_Info {
        unsigned int min_len;			  /* minimum length of a new PIN */
                                                  /* value                       */
	OctetString  *pin;			  /* value of the PIN	         */
 	Boolean      disengageable;	          /* PIN  disengageable?         */
						  /* TRUE(1) = yes               */
						  /* FALSE(0)= no                */
}			   PINInfo;


typedef struct PUK_Info {
	OctetString  *puk;			  /* value of the PUK	         */
	KeyId	     *pin_key_id;		  /* key_id of the corresponding */
						  /* PIN		         */
}		           PUKInfo;

typedef struct Verify_Data_Struc {
	VerifyDataType	        verify_data_type; /* PIN or PUK	                 */
	union {
		PINInfo		pin_info;	  /* Body for a PIN	         */
		PUKInfo		puk_info;	  /* Body for a PUK	         */
	      }  VerifyDataBody;
}			   VerifyDataStruc;
				 

typedef struct Key_Dev_Sel {			  /* this structure identifies a */
					 	  /* a device Key			*/
        AlgId	    *key_algid;			  /* Alg-Id of the key	         */
	KeyDevStatus key_status;		  /* OWN/ANY				*/																		          
	KeyId	    *key_id;          		  /* If ANY, select by key_id		*/
	KeyPurpose  key_purpose;	  	  /* If OWN, select by purpose		*/
}			    KeyDevSel;

typedef struct Key_Dev_List {			  /* this structure identifies a */
					 	  /* a device key list			*/
	KeyDevStatus key_status;		  /* OWN/ANY				*/																		          
	union {
		KeyId	     **keyid_list;        /* If ANY, key_id list		*/
        	KeyPurpose    *purpose_list;	  /* If OWN, purpose list		*/
	      } list_ref;
}			    KeyDevList;


/*------------------------------------------------------------------*/
/*  structures for the hash functions                               */
/*------------------------------------------------------------------*/
typedef union Hashpar {
	KeyBits	        sqmodn_par;		  /* modulus of public rsa key   */

} HashPar;	

/*------------------------------------------------------------------*/
/*  structures for sessionkey                                       */
/*------------------------------------------------------------------*/
typedef struct Session_Key {
	AlgId		*integrity_key;
	AlgId		*concealed_key;

} SessionKey;


/*------------------------------------------------------------------*/
/*  structures for port memory                                      */
/*------------------------------------------------------------------*/
typedef enum {P_NONE,P_ODD,P_EVEN} ParityType;
typedef enum {E_LRC,E_CRC} EdcType;
typedef enum {C_OFF,C_ON} Chain;
typedef enum {COM_OPEN, COM_CLOSE} ComMode;
typedef struct SCT_Port {
                      int       bwt;
                      int       cwt;
                      int       baud;
                      int       databits;
                      int       stopbits;
                      ParityType parity;
                      int       dataformat;
                      int       tpdusize;
                      int       apdusize;
                      EdcType   edc;
                      int       protocoltype;    /* set, when S_RESET response*/
						 /* received		      */
                      Chain     chaining;        
		      unsigned int icc_request;  	
		      char      first_pad_byte;  /* coding of the first 
					  	    padding byte in the ICC
						  */  
		      char      next_pad_byte;   /* coding of the following 
					  	    padding bytes in the ICC
						  */  
		      Boolean	always;  	 /* TRUE = padding is always
						    done 
					  	    FALSE= padding is only done
						    if the plaintext is no
						    multiple of 8
						  */  
} SCTPort;
	

/*------------------------------------------------------------------*/
/*  R e t u r n c o d e s                                           */
/*------------------------------------------------------------------*/


/*  Return codes of sca-functions                                          */

#define  M_NOERR	     0
#define  M_SIGOK             1             /* signature correct,    */
                                           /* key too short         */
#define  M_KEYREPL           1             /* existing key replaced */
#define  M_KEYLEN            1       /* correct key length returned */

#define  M_SCT_NOT_ACTIVE    1	
#define  M_NO_CARD_INSERTED  2	
#define  M_CARD_INSERTED     3
#define  M_REMOVE_CARD       4




/*------------------------------------------------------------------*/
/*  E r r o r s                                                     */
/*------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/* Structur of SCA_ErrList                                             */
/*---------------------------------------------------------------------*/

struct SCA_ErrList {
        unsigned int sw1;
        unsigned int sw2;
        char msg[128];
        };

extern struct SCA_ErrList  sca_errlist[];


/* Errors from the transmission - module                               */
#define EDEVBUSY                1       /* Device busy  - port locked   */
#define EWRITE                  2       /* Write error                  */
#define EMEMO                   4       /* Memory error                 */
#define ET1OPEN                 5       /* Open error                   */
#define ET1PORT                 6       /* no port open                 */
#define ETPDULEN                10      /* TPDU-Length error            */
#define EINVALIDPORT            11      /* Not available port */
#define EPROTRESYNCH            13      /* Protocol has been resynchronized.
                                           and
                                     communication can be started again with
                                     new protocol parameter state.        */
#define ESCTRESET               14/* Smart card terminal should be reset
                                     communication can be started again with
                                     new protocol parameter state.        */



/* Errors      from the SmartCard - Terminal                           */
/*---------------------------------------------------------------------*/
/* SW1(41)=Parameter not correct or inconsistent                       */
/*---------------------------------------------------------------------*/
#define EINVCLASS               20      /* wrong CLASS                  41 00*/
#define EINVKID                 21      /* invalid  KID                 41 02*/
#define EINVALGID               22      /* invalid  algorithm identifier41 03*/
#define EOPMODE                 23      /* Operation Mode not allowed   41 04*/
#define EINVPAR                 24      /* parameter in body invalid    41 06*/
#define EDATALEN                25      /* incorrect datalength         41 07*/
#define EP1P2INC                26      /* P1 - P2 incorrect            41 09*/
#define EDATL2INC	        27      /* data length leve 2 incorrect 41 0A*/
#define ECOMCOUNT               28      /* communication counter incorrect 41 0B*/
#define ESTATE                  29      /* wrong transition state       41 0C*/
#define ECCS                    30      /* CCS invalid                  41 0D*/


/*---------------------------------------------------------------------*/
/* SW1(42)=key access error                                            */
/*---------------------------------------------------------------------*/
#define EKEYUNKNOWN             31      /* Key unknown                  42 00*/
#define EKEYALGINCON            32      /* KEY and ALGID inconsistent   42 01*/
#define EKEYNOTREPL             33      /* Key not replaceable          42 02*/
#define EKEYINFO                34      /* incorrect Key-Information at */
                                        /* keycard                      42 03*/
#define EINVAPPLID              35      /* Appl_id invalid              42 04*/
#define EKEYSET                 36      /* Key set inconsistent         42 05*/
#define EAPPLID                 37      /* Appl_id not found            42 06*/

/*---------------------------------------------------------------------*/
/* SW1(43)=Parameter- or Dataerror                                     */
/*---------------------------------------------------------------------*/
#define EDECLEN                 38      /* incorrect length of ciphertext43 02*/
#define EINVSIG                 39      /* invalid  signature           43 03*/
#define EKEYLENINV              40      /* Keylength invalid            43 04*/
#define ESCTNOMEM               41      /* no memory available          43 05*/
#define EAUTH                   42      /* Authentication failed        43 06*/
#define EEXECDEN                43      /* execution denied             43 08*/
#define ESECMESSKEY             44      /* secure messaging key undefined43 0A*/
#define EAUTHKEY                45      /* authentication key undefined 43 0B*/
#define EICCSEC                 46      /* ICC - secure messaging error 43 0C*/



/*---------------------------------------------------------------------*/
/* SW1(44)=Communication-error with smartcard                          */
/*---------------------------------------------------------------------*/
#define ENOCARD                 47      /* no smartcard                 44 00*/
#define ERESET                  48      /* reset of ICC not successful   44 01*/
#define EICCREMOVED             49      /* ICC removed                   44 02*/
#define EICCTIMEOUT             50      /* Timeout     - no response from44 03*/
                                        /* ICC                           */
#define EUSERBREAK              51      /* break from user              44 04*/
#define EUSTIMEOUT              52      /* Timeout     - no response from44 05*/
                                        /* user                          */
#define EICCDATL2               53      /* ICC - data length leve 2 incorrect              44 06*/

/*---------------------------------------------------------------------*/
/* SW1(45)=DSP (RSA) error                                             */
/*---------------------------------------------------------------------*/
#define EDSPERR                 54      /* internal address-error       45 00*/
#define EDSPTIME                55      /* DSP-Timeout                  45 01*/
#define EDSPPAR                 56      /* RSA-Parameter ERROR          45 02*/
#define ENODSP                  57      /* no DSP available             45 03*/
#define EDSPGENERR              58      /* DSP - general error          45 04*/
#define EDSPSYS                 59      /* system error                 45 FF*/

/*---------------------------------------------------------------------*/
/* Sessionkey error                                                    */
/*---------------------------------------------------------------------*/
#define ESESSKEY                60      /* sessionkey not found         41 0E*/



/* Errors      from the Smartcard                                      */
/* SW1 = 6x          */

/*---------------------------------------------------------------------*/
/* command with warning executed                                       */
/*---------------------------------------------------------------------*/
#define EFILE_NOT_DF		62	/* FILE_NOT_DF		   60 00  */ 
#define ETOO_MANY_DATA		63	/* TOO_MANY_DATA	   61 00  */
#define EFILE_ALREADY		64	/* FILE_ALREADY_EXISTS	   62 00  */
#define EDATA_INCON		65	/* DATA_INCONSISTENT	   62 81  */
#define EEND_OF_FILE		66	/* NO_DATA		   62 82  */
					/* END_OF_FILE/RECORD REACHED     */
#define EFILE_LOCKED		67	/* FILE_LOCKED		   62 84  */
#define ESID_INC                68      /* SID_INCORRECT           62 85  */
#define ELAST_ATTEMPT		69	/* LAST_BLOCK		   63 00  */
 
 

/*---------------------------------------------------------------------*/
/* execution error (memory not changed)                                */
/*---------------------------------------------------------------------*/
#define EINV_ACF                70      /* NO_VALID_ACF            64 00  */


/*---------------------------------------------------------------------*/
/* execution error (memory  changed)                                   */
/*---------------------------------------------------------------------*/
#define EEEPROM_KFPC            71      /* EEPROM_ERROR            65 00  */
					/* KFPC_ERR		  	  */
/*---------------------------------------------------------------------*/
/* wrong length                                                        */
/*---------------------------------------------------------------------*/
#define EINV_LEN                72      /* INVALID_PIN_LEN         67 00  */
					/* NEW_PINLEN_INCORRECT		  */
					/* INVALID_LC_LE		  */
/*---------------------------------------------------------------------*/
/* checking error in case of secure messaging                          */
/*---------------------------------------------------------------------*/
#define EMAC                    73      /* WRONG_MAC               68 00  */
					/* WRONG_SSC			  */
/*---------------------------------------------------------------------*/
/* checking error (command not allowed)                                */
/*---------------------------------------------------------------------*/
#define ECMD_NOT_ALLOWED        74      /* RID_ALREADY_EXIST       69 00  */
					/* INV_COM_ACF			  */
					/* INV_STATE			  */
					/* INV_FROM			  */
				 	/* ACV_DENY			  */ 

#define EACCESS_NOT_ALLOWED     75      /* NO_WRITE_ISF_ACF	   69 81  */
					/* NO_READ_ISF_ACF		  */
					/* WRITE_PROTECTION		  */
					/* INV_ATTR			  */
					/* CLOSE_PEND_LEVEL		  */
					/* NO_LOCK_IAP			  */
#define EKEY_LOCKED		76	/* KEY_LOCKED		   69 83  */
#define EINV_ACCESS		77	/* WRONG_ALGO		   69 85  */
					/* NO_SEC_MESS_KEY	          */
					/* INVALID_KEY_SEL		  */
					/* NO_DELETE			  */
					/* FILE_ONLY_REGISTERED		  */
					/* MF_NOT_ACTIVE		  */
					/* INV_OP_MODE			  */
					/* EF_REGISTERED		  */
#define EINV_CMD_PAR		78	/* NO_RND_EXISTS	   6A 00  */
					/* INV_SPACE_PARAM		  */
					/* ACF_NO_EF			  */
					/* INV_NAME_LEN			  */
					/* INV_FID_NAME			  */
					/* SPECIFIED_SIZE_NOK		  */
					/* ELEM_SIZE_TOO_BIG		  */
					/* INV_NAME			  */
#define EFILE			79	/* NO_ACTUAL_DF		   6A 82  */
					/* NO_FILE_SELECTED		  */
					/* FILE_NOT_FOUND		  */
					/* DF_NOT_REGISTERED		  */
					/* FILE_NOT_MATCHED		  */
#define ERID			80	/* RID_NOT_FOUND	   6A 83  */
					/* INVALID_RID			  */
#define ESPACE			81	/* NOT_ENOUGH_SPACE	   6A 84  */
#define EINV_PAR		82	/* INV_PAR_1_2		   6A 86  */
					/* INV_CAT_TYP		 	  */
#define EWRONG_OFFSET		83	/* WRONG_OFFSET		   6B 00  */     
#define EICCIN                  84      /* INVALID_INS             6D 00  */
#define ESPIN_TO_PIN		85	/* SPIN_TO_NO_PIN          6F 80  */ 
#define ESYSTEM_ERR 		86	/* SYSTEM ERROR            6F 81  */ 
#define ECPIN   		87	/* CPIN NOT ALLOWED        6F 82  */ 
#define EMISS_SEC		88	/* MISSING SECMESS         69 82  */ 
#define EAUTH_1ATTEMPT		89	/* AUTH FAILED - 1 ATTEMPT 63 81  */ 
#define EAUTH_2ATTEMPT		90	/* AUTH FAILED - 2 ATTEMPT 63 82  */ 
#define EAUTH_3ATTEMPT		91	/* AUTH FAILED - 3 ATTEMPT 63 83  */ 
#define EAUTH_4ATTEMPT		92	/* AUTH FAILED - 4 ATTEMPT 63 84  */ 
#define EAUTH_5ATTEMPT		93	/* AUTH FAILED - 5 ATTEMPT 63 85  */ 
#define EAUTH_6ATTEMPT		94	/* AUTH FAILED - 6 ATTEMPT 63 86  */ 
#define EAUTH_7ATTEMPT		95	/* AUTH FAILED - 7 ATTEMPT 63 87  */ 
#define EINV_ATTR		96	/* INVALID FILE-ATTRIBUTS  69 01  */
#define EINV_RID		97	/* INVALID RID		   6F 87  */
#define EPINDIS			98	/* PIN-AUTH disengaged	   91 02  */
#define EPINAUTH		99	/* PIN-AUTH successful because    */
					/* PIN-AUTH disengaged	   91 04  */
#define EPINCHG			100	/* PIN not changed         91 08  */ 



#define ESCTLIST                101     /* SCT-List not created         */
#define ESSC                    102     /* wrong SSC                    */
#define EPARINC                 103     /* invalid parameter            */
#define EALGO                   104     /* invalid algorithm identifier */
#define EINVARG                 105     /* invalid argument             */
#define EMEMAVAIL               106     /* memory not available         */
#define ERECMAC                 107     /* received MAC invalid         */
#define ETOOLONG                108     /* apdu too long or             */
					/* length of parameter invalid  */
#define ENOSHELL                109     /* no shell-variable in env.    */
#define EOPERR                  110     /* can't open install-file      */
#define EEMPTY                  111     /* install-file empty           */
#define ECLERR                  112     /* install-file not successfully
			                   closed                       */
#define ERDERR                  113     /* Read error on install-file   */
#define ESIDUNK                 114     /* SCT_ID unknown               */
#define EINS                    115     /* invalid instruction          */
#define EBAUD                   116     /* Baudvalue not allowed        */
#define EDESENC                 117     /* Data can`t be encrypted      */
#define EDESDEC                 118     /* Data can`t be decrypted      */
#define EGENSESS                119     /* Can't generate sessionkey    */
#define ERSAENC                 120     /* Error after RSA encryption   */
#define EKEY                    121     /* Can't set RSA key            */
#define ESCT_SSC                122     /* WRONG_SSC  from SCT         */

#define  EPAR         		123     /* parameter inconsistent              */
#define  EKEYLEN                124     /* invalid key length                  */
#define  ERESPDAT               125     /* error in response data              */
#define  EFUNCTION              126     /* mixed function calls not allowed    */

#define  EREADKEY        	127     /* key from keycard not installed      */
	                                /* (more)                              */
#define  EHASHFUNC           	128     /* hash-function error                 */
#define  EKEYINV            	129     /* key invalid                         */
#define  ENOINTKEY            	130     /* no sessionkey for the integrity mode available                        */
#define  ENOCONCKEY            	131     /* no sessionkey for the concealed mode available                         */
#define  EPADD            	132     /* Padding error - decrypt               */
#define  EICCPADD            	133     /* Padding error - encrypt               */
#define  ECARDINS            	134     /* smartcard already inserted            */
#define  ECARDREM            	135     /* smartcard still inserted              */
#define  ESTATE0            	136     /* SCT in state 0                        */
#define  EPKCS            	137     /* error in PKCS routine                 */






#endif

