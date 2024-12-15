/*
 *  STARMOD Release 1.1 (GMD)
 */
/*-------------------------------------------------------+-----*/
/*							 | GMD */
/*   SYSTEM   STAPAC  -  Version 1.0			 +-----*/
/*							       */
/*-------------------------------------------------------------*/
/*							       */
/*    PACKAGE	STARMOD			VERSION 1.0	       */
/*					   DATE January 1993   */
/*					     BY Levona Eckstein*/
/*						Ursula Viebeg  */ 
/*							       */
/*    FILENAME			                 	       */
/*      stamod.h    		         		       */
/*							       */
/*    DESCRIPTION					       */
/*      This file contains all structures and types for the    */
/*	STAPAC Application Interface         		       */
/*-------------------------------------------------------------*/
#ifndef _STARMOD_
#define _STARMOD_


/*
 *   structures and types for the sct interface
 */


/*
 *   sca.h defines:
 *         KeyAttrList,
 * 	   TransMode,
 *         KeyPurpose, 
 *         KeyLevel,
 *         KeyId,
 *         ICC_SecMessMode,
 *         SCT_SecMessMode,
 *	   KeyDevStatus,
 *	      
 *   secure.h (included in sca.h) defines:
 *
 *         Boolean,
 *	   More,  
 *	   OctetString
 *
 */


/* Returncodes from the sctinterface - Procedure                       */
#define S_NOERR                 0       /* no error                    */
#define S_WAIT                  1       /* SCT waiting                  */
#define S_KEYREPL               2       /* Key in SCT replaced          */
#define S_SIGOK                 3       /* Signatur correct, but Key too*/
                                        /* short                        */
#define S_PINOFF                4       /* PIN-CHECK off    90 01       */
#define S_PINON                 5       /* PIN-CHECK on                 */

#define S_ERR                   -1      /* error (errornumber in sct_errno */


/* SW1 = no error                                                       */
#define OKSCT                   0x40
#define OKICC                   0x90
#define DATAINC			0x03



/* Instructioncodes of the SCT-Commands                         */
/* local SCT - commands     */
#define S_REQUEST_ICC           0x01    /* Request Smartcard    */
#define S_EJECT_ICC             0x03    /* Eject Smartcard      */
#define S_STATUS                0x02    /* Status               */
#define S_DISPLAY               0x04    /* display text on SCT  */
#define S_RESET                 0x05    /* RESET SCT            */
#define S_VERSION               0xF0    /* Version of SCT       */
#define S_STATINF               0xF1    /* Version of SCT       */

/* SC - TRANSFER - Commands */
#define S_TRANS                 0x11    /* transport of SC-Commands */

/* Cryptology - commands    */
#define S_GEN_USER_KEY          0x21    /* generate user-key    */
#define S_INST_USER_KEY         0x22    /* install user-key     */
#define S_DEL_USER_KEY          0x23    /* delete  user-key     */
#define S_GET_RNO               0x24    /* get random number    */
#define S_RSA_SIGN              0x25    /* create signature     */
#define S_RSA_VERIFY            0x26    /* verify signature     */
#define S_DES_ENC               0x27    /* DES-encryption       */
#define S_RSA_ENC               0x28    /* RSA-encryption       */
#define S_DES_DEC               0x29    /* DES-decryption       */
#define S_RSA_DEC               0x2A    /* RSA-decryption       */
#define S_ENC_DES_KEY           0x2B    /* encrypt DES-KEY      */
#define S_DEC_DES_KEY           0x2C    /* decrypt DES-KEY      */

/* Device key   - commands  */
#define S_GEN_DEV_KEY		0x31	/* generate device keys */
#define S_INST_DEV_KEY		0x32	/* install device keys  */
#define S_DEL_DEV_KEY		0x33	/* delete device keys   */
#define S_COMP_DEV_KEY		0x34	/* compare device key   */
#define S_SET_APPL_ID		0x35	/* set device key set   */

/* Authenticate - commands  */
#define S_INST_VER_DATA         0x41    /* install verification data  */
#define S_MODIFY_VER_DATA       0x42    /* modify verification data   */
#define S_AUTH                  0x43    /* authentication       */
#define S_PERFORM_VERIFY        0x44    /* perform verification */

/* Secure messaging  - commands  */
#define S_GEN_SESSION_KEY       0x52    /* install session key  */
#define S_DEL_SESSION_KEY       0x53    /* delete  session key  */


/* Keycard      - commands  */
#define S_WRITE_KEYCARD         0x61    /* write keycard        */
#define S_READ_KEYCARD          0x62    /* read  keycard        */








typedef enum {S_DISENGAGEABLE_PIN, S_PUK, S_DES_CBC, S_NON_DISENGAGEABLE_PIN, S_RSA_F4, 
	      S_DES_3_CBC=0x06, NOT_DEFINED} KeyAlgId;
typedef enum {S_CHG_PIN,S_UNBLOCK_PIN} ModifyMode;


/* Structure - definitions              */

typedef struct s_sctinfo  {
        unsigned int  apdusize;
        Boolean           port_open;
        Boolean           icc_request;
        Boolean           integrity_key;
        Boolean           concealed_key;
	SCT_SecMess	  secure_messaging;
	char      	  first_pad_byte;  /* coding of the first 
					  	    padding byte in the ICC
						  */  
	char      	  next_pad_byte;   /* coding of the following 
					  	    padding bytes in the ICC
						  */  
        Boolean		  always;  	   /* TRUE = padding is always
						    done 
					      FALSE= padding is only done
						    if the plaintext is no
						    multiple of 8
						  */  
               } SCTInfo;



typedef struct s_public{
         OctetString   *modulus;
         } Public;



typedef struct s_enc     {
         Public     *public;
         OctetString *plaintext;
         } Enc;

typedef struct s_enc_des_key     {
         Public     *public;
         OctetString *padding_string;
         } EncDesKey;

typedef struct s_verify {
         Public       *public;
         OctetString   *signature;
         OctetString   *hash;
         } Verify;

typedef struct s_deskey {
         int        algid;
         OctetString *chiffrat;
        } DESKey;

typedef struct s_dev_key_info {		
	 KeyPurpose     purpose;
         OctetString    *appl_id;
        } DevKeyInfo;

typedef struct s_inst_dev_key {		
         OctetString    *appl_id;
	 KeyAttrList   *key_attr_list;
        } InstDevKey;

typedef struct s_dev_sel_key {
        union {
                KeyId         *kid;
                KeyPurpose    purpose;
              }pval;
        }DevSelKey;

typedef struct s_ver_record {
	KeyAlgId     key_algid;		
        KeyAttrList *key_attr_list;
        OctetString  *ver_data;
        } VERRecord;


typedef struct Write_Keycard {
	OctetString	*appl_id;
	KeyId    	**keyid_list;		
        }WriteKeycard;

                



typedef struct s_request  {
         union p1 {
               KeyId           *kid;    /* S_GEN_USER_KEY, S_INST_USER_KEY,*/
				        /* S_DEL_USER_KEY, 	        */
				        /* S_GEN_DEV_KEY,               */
                                        /* S_RSA_SIGN, S_INST_VER_DATA  */
                                        /* S_MODIFY_VER_DATA            */
					/* S_PERFORM_VERIFY		*/
                                        /* S_RSA_DEC,                 */
                                        /* S_DES_ENC, S_DES_DEC,        */
                                        /* S_DEC_DES_KEY, S_ENC_DES_KEY,*/
	       KeyLevel	       level;   /* S_SET_APPL_ID		*/
               TransMode       secmode; /* S_TRANS                      */
               unsigned int    lrno;    /* S_GET_RNO                    */
	       SCT_SecMessMode sec_mode;       /* S_GEN_SESSION_KEY     */
					       /* S_DEL_SESSION_KEY     */
               DevSelKey      *dev_sel_key;    /* S_DEL_DEV_KEY         */
					       /* S_INST_DEV_KEY        */
                                              
                  }rq_p1;
         union p2 {
               unsigned int    time;    /* S_REQUEST_ICC,S_DISPLAY      */
	       Boolean         signal;  /* S_EJECT_ICC			*/
               KeyAlgId        algid;   /* S_GEN_USER_KEY               */
                                        /* S_GEN_DEV_KEY                */
                                        /* S_GEN_SESSION_KEY            */
               More            more;    /* S_RSA_ENC, S_RSA_DEC,        */
                                        /* S_DES_ENC, S_DES_DEC         */
               KeyId           *kid;    /* S_DEC_DES_KEY                */
               AuthProcId      acp;     /* S_AUTH                       */
               KeyDevStatus    status;  /* S_DEL_DEV_KEY                */
                                        /* S_READ_KEYCARD               */
	       ModifyMode      modify_mode;	/* S_MODIFY_VER_DATA       */
                  }rq_p2;

         union datafield {
               OctetString     *outtext;        /* S_REQUEST_ICC, S_DISPLAY,
						   S_EJECT_ICC */
               unsigned int     keylen;         /* S_GEN_USER_KEY          */
               DevKeyInfo      *dev_key_info;   /* S_GEN_DEV_KEY           */
               KeyAttrList     *keyattrlist;	/* S_INST_USER_KEY	   */
  	       InstDevKey      *inst_dev_key;   /* S_INST_DEV_KEY          */
               OctetString     *sig_string;     /* S_RSA_SIGN              */
               VERRecord       *ver_record;     /* S_INST_VER_DATA         */
               Enc             *enc;            /* S_RSA_ENC               */
               OctetString     *chiffrat;       /* S_RSA_DEC, S_DES_DEC    */
               OctetString     *plaintext;      /* S_DES_ENC               */
	       OctetString     *appl_id;        /* S_DEL_DEV_KEY	   */
						/* S_COMP_DEV_KEY	   */
						/* S_SET_APPL_ID	   */
               DESKey          *deskey;         /* S_DEC_DES_KEY           */
               EncDesKey       *enc_des_key;    /* S_ENC_DES_KEY           */
               OctetString     *sccommand;      /* S_TRANS                 */
               Verify          *verify;         /* S_RSA_VERIFY            */
               ICC_SecMess     *sec_mode; 	/* S_MODIFY_VER_DATA   */
						/* S_PERFORM_VERIFY		*/ 
               OctetString     *session_key;    /* S_GEN_SESSION_KEY       */
               WriteKeycard    *write_keycard;  /* S_WRITE_KEYCARD  ?????? */
             } rq_datafield;
         } Request;




/* NULL-Pointer - Definitions           */
#define PUBNULL         (Public  *)0
#define ENCNULL         (Enc  *)0
#define VERNULL         (Verify  *)0
#define DESKNULL        (DESKey  *)0
#define ENCDESKNULL     (EncDesKey  *)0
#define REQNULL         (Request *)0
#define DEVNULL		(DevKeyInfo *)0
#define INSTDEVKEYNULL 	(InstDevKey *)0
#define VERDATNULL	(VERRecord *)0
#define WRITENULL	(WriteKeycard *)0



#define SAD                     2       /* source address       */
#define DAD                     1       /* destination address  */


/*
 *   defines structures and types for the port memory
 */
#define LSCTNAME        8       /* max. length of sct name */
#define LPORTNAME       24      /* max. length of port name */

#define B_19200        19200

#ifdef _DOS_
#define STOP_1      0x00
#define STOP_2      0x01
#define DATA_8      0x03
#define DATA_7      0x02
#define PARNONE     0
#define PARODD      1
#define PAREVEN     2
#endif
#ifdef _BSD42_
#define STOP_1      0x00
#define STOP_2      64
#define DATA_8      48
#define DATA_7      32
#define PARNONE     0
#define PAREVEN     256
#endif
#ifdef _POSIX_
#define STOP_1      0x00
#define STOP_2      64
#define DATA_8      48
#define DATA_7      32
#define PARNONE     0
#define PAREVEN     256
#endif
#ifdef _MACC7_
#include <Serial.h>
#define STOP_1      stop10
#define STOP_2      stop20
#define DATA_8      data8
#define DATA_7      data7
#define PARNONE     noParity
#define PARODD      oddParity
#define PAREVEN     evenParity
#endif


#define PORTNULL        (struct s_portparam *)0

struct s_portparam {
                      char      port_name[LPORTNAME+1];     /* + 1 for \0 */
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
                      Chain     chaining;        /* initialized with C_ON in */
					         /* sctmem.c -> init_elem    */

                      int       ns;		 /* set in t1.c		     */
                      int       rsv;		 /* set in t1.c              */
                      int       sad;		 /* set in scctint.c -> sct_reset*/
                      int       dad;		 /* set in sctint.c -> sct_reset*/
                      int       port_id;         /* = FD of OPEN PORT */
                      int       first;		 /* set in t1.c              */
		      Boolean   integrity;
		      Boolean   concealed;
		      KeyInfo   integrity_key;   /* temporary session_key */
		      KeyInfo   concealed_key;   /* temporary session_key */
                      int       ssc;             /* send sequence counter */
		      unsigned int icc_request;  	
                      SCT_SecMess secure_messaging; /* secure messaging 
						    between dte and sct */
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
                      struct s_portparam *p_next;
                 };






/*
 *  structures and types for the icc module
 */


/*
 *   sca.h defines:
 *
 *	   ChallengeMode, 
 *         DataStruc,
 *         FileInfoReq,  
 *         FileSel, 
 *         FileCat,
 *         FileCloseContext,
 *         FileType,
 *         FileControlInfo, 
 *         KeyId,
 *	   KeyAttrList
 *
 *   secure.h (included in sca.h) defines:
 *
 *         Boolean, 
 *         OctetString,
 * 
 *   sctif.h defines:
 *
 *         KeyAlgId 
 */


/*--------------------------------*/
/* define sc-command-instruction  */
/* code                           */
/*--------------------------------*/
#define ICC_SELECT       0xA4		/* SELECT FILE	 */
#define ICC_CLOSE        0xAC		/* CLOSE FILE	 */
#define ICC_EXECUTE	0xAE		/* EXECUTE	 */
					/* nicht in Version 1.1 */
 
#define ICC_READB        0xB0		/* READ BINARY   */
#define ICC_READR        0xB2		/* READ RECORD   */

#define ICC_WRITEB       0xD0		/* WRITE BINARY  */
#define ICC_WRITER       0xD2		/* WRITE RECORD  */

#define ICC_CREATE       0xE0		/* CREATE FILE	 */
#define ICC_DELF         0xE4		/* DELETE FILE   */
#define ICC_DELREC       0xE6		/* DELETE RECORD */

#define ICC_SETKEY	0xF0		/* SET KEY	 */
#define ICC_WR_KEY       0xF4		/* WRITE_KEY     */
#define ICC_GET_CD       0xF6		/* GET CARD DATA */
#define ICC_CRYPT        0xF8		/* CRYPT	 */
#define ICC_LOCKKEY      0xFC		/* LOCK KEY	 */
#define ICC_KEYSTAT	0xF2		/* KEY STATUS	 */

#define ICC_REGISTER     0x52		/* REGISTER DF	 */

/* KLAEREN */
#define ICC_COMPUTE	0x54		/* COMPUTE	 - noch nicht spezifiziert */

#define ICC_LOCKF        0x76		/* LOCK FILE     */

#define ICC_VERIFY	0x20		/* VERIFY	 */
#define ICC_VER_CHG	0x24		/* VERIFY AND CHANGE */

#define ICC_EXCHAL       0x80		/* EXCHANGE CHALLENGE */
#define ICC_EX_AUTH	0x82		/* EXTERNAL AUTHENTICATE */
#define ICC_GETCHAL	0x84		/* GET CHALLENGE */
#define ICC_IN_AUTH	0x88		/* INTERNAL AUTHENTICATE */
#define ICC_MU_AUTH	0x8A		/* MUTUAL AUTHENTICATE */




/*--------------------------------*/
/* define sc-command-parameter    */
/* length                         */
/*--------------------------------*/
#define SSDLEN          4       /* length of              */
                                /* security status description           */
#define RNDLEN          8       /* max. length of Random number = 8      */
#define CDLEN           8       /* max. length of Card Data = 8          */
#define AUTHRELLEN      8       /* max. length of auth-releated-info = 8 */
#define PINLEN          8       /* max. length of PIN = 8                */
#define DATLEN          254     /* max. length of data                   */
#define KEYLEN          254     /* max. length of key-data               */
#define REGACVLEN       1       /* length of Security Status Description */
                                /* of the REGISTER-Command               */
#define UNITLEN         2       /* max. length of UNITS                  */
#define OPLEN           1       /* max. length of Operation mode         */
#define SIZELEN         1       /* max. length of Record or Element Size */
#define FIDLEN          2       /* max. length of File Identifier   */
#define KEYHEAD         5       /* max. length of Key Header        */
#define RECORD_NUMBER   0x04
#define RECORD_ID	0x00
#define APPEND		0x06

#define SLEN_DATA_NORMAL	90	   /* max. length of data-field(send) 
					      in normal mode */
#define SLEN_DATA_AUTH		85	   /* max. length of data-field(send) 
					      in authentic mode */
#define SLEN_DATA_CONC		82	   /* max. length of data-field(send) 
					      in concealed mode */
#define SLEN_DATA_COMB		78	   /* max. length of data-field(send) 
					      in combinded mode */
#define RLEN_DATA_NORMAL	92	   /* max. length of data-field(receive) 
					      in normal mode */
#define RLEN_DATA_AUTH 		88	   /* max. length of data-field(receive) 
					      in authentic mode */
#define RLEN_DATA_CONC		84	   /* max. length of data-field(receive) 
					      in concealed mode */
#define RLEN_DATA_COMB		80	   /* max. length of data-field(receive) 
					      in normal mode */

/*
 *    Parameter types (parmtype member) 
 */
typedef struct ICC_ParmList          ICC_ParmList;

typedef enum {
        ICC_PARM_ABSENT,
	ICC_PARM_filecat,
	ICC_PARM_fileinforeq,
	ICC_PARM_filesel,
	ICC_PARM_context,
	ICC_PARM_datasel,
	ICC_PARM_short_index,
	ICC_PARM_lrddata,
	ICC_PARM_LEN,
        ICC_PARM_data,
	ICC_PARM_datastruc,
	ICC_PARM_filecontrolinfo,
	ICC_PARM_kid,
	ICC_PARM_conc_kid,
	ICC_PARM_keyattrlist,
	ICC_PARM_key_algid,
	ICC_PARM_modi,
	ICC_PARM_operation,
	ICC_PARM_LOCKCAT,
	ICC_PARM_units,
	ICC_PARM_acv,
	ICC_PARM_fid,
	ICC_PARM_acp_verify,
	ICC_PARM_acp_auth,
	ICC_PARM_CHGPIN,
	ICC_PARM_new_pin,
	ICC_PARM_INSTMODE,
	ICC_PARM_direction,
	ICC_PARM_NOTUSED
} ICC_ParmType;


typedef enum {
	ICC_LEN_ABSENT,
	ICC_LEN_FILESEL,
	ICC_LEN_LRDATA,
	ICC_LEN_OCTET,
	ICC_LEN_FINFO,
	ICC_LEN_NOTUSED,
	ICC_LEN_FIX,
	ICC_LEN_KEY,
	ICC_LEN_REG,
	ICC_LEN_FILEID,
	ICC_LEN_PIN

} ICC_ParmLen;


typedef enum {ICC_ISO=0x00,ICC_NON_ISO=0x20} CmdClass;


struct ICC_ParmList {
        unsigned int	inscode;
	CmdClass        cmd_class;
	ICC_ParmLen	parmlen;
	unsigned int	parm_no;
        ICC_ParmType     parmtype[5];
};


/*--------------------------------*/
/* define sc-command-parameter    */
/* values                         */
/*--------------------------------*/
#define ICC_NOTUSED      0x00
#define LOCK_CAT        0x03      /* EF */

typedef enum {ICC_ENC,ICC_DEC,ICC_MAC} Modi;
typedef enum {SEL_EF, SEL_CHILD_DF, SEL_PARENT_DF=0x04} SelectionControl; 
typedef enum {CO_UNLOCK, CO_LOCK=0xFF} Operation;
typedef enum {AUTH_PIN=0x20, AUTH_PUK=0x30, AUTH_DES=0x40} AuthMethod;
typedef enum {KEY_ISF=0x05,KEY_EEPROM=0x06} KeySearch;
 
typedef struct Auth_control_par {
		AuthMethod auth_method;
		KeySearch  key_search;
} AuthControlPar;




typedef struct Record_Sel {			  /* for LIN_FIX,LIN_VAR         */
        unsigned int  record_id;                  /* Record identifier           */
	WriteMode     write_mode;		  /* append or create record     
						     will only be interpreted if
						     a record will be written    */
						  
						     
}                      RecordSel;

typedef struct Element_Sel {			  /* for CYCLIC                  */
        unsigned int  element_ref;                /* Element reference           */
}                      ElementSel;

typedef unsigned int  StringSel;		  /* for TRANSPARENT             */



typedef struct Data_Sel {
        DataStruc   data_struc;                   
        union { 				  /* data access to an:          */
	        RecordSel   record_sel;	          /* record oriented file        */
                ElementSel  element_sel;          /* cyclic file                 */
                StringSel   string_sel;           /* transparent file            */ 
              }data_ref; 
}                      DataSel;


/*--------------------------------*/
/* define sc-command-header       */
/*--------------------------------*/
struct s_header {
        ICC_SecMess   security_mess;
        unsigned int inscode;
        };




/*--------------------------------*/
/* define sccom-structure         */
/*--------------------------------*/
struct s_command {
        struct s_header icc_header;
	/* command parameter	*/
	FileCat		filecat;	/* must be set by
					   ICC_SELECT
					   ICC_DELFILE
					   (value: EF, CHILD_DF,
						   PARENT_DF)
					   ICC_CLOSE
					   ICC_CREATE
					   (value: MF, PARENT_DF,
						   CHILD_DF, EF)
					*/   
	FileInfoReq	fileinforeq;	/* must be set by
					   ICC_SELECT
					*/
	FileCloseContext context;	/* must be set by
					   ICC_CLOSE
					*/
	FileSel		*filesel;	/* must be set by
					   ICC_SELECT
					   ICC_DELFILE
					   ICC_CLOSE_FILE 
					*/
	DataSel		*datasel;	/* must be set by
					   ICC_READB
					   ICC_READR
					   ICC_DELREC
					   ICC_WRITEB
					   ICC_WRITER
					*/
					/* TRANSPARENT     =>string_sel is
							     interpreted */
					/* LIN_FIX,LIN_VAR =>record_sel is
							     interpreted */ 
					/* CYCLIC	   =>element_sel is
							     interpreted */
	unsigned int	short_index;	/* must be set by
					   ICC_READR
					   ICC_DELREC
					   ICC_WRITER
					*/
					/* this parameter will only be interpreted
					   in case of LIN_FIX,LIN_VAR and CYCLIC
				           Value = 0 means no short EF identifier 
					*/
	unsigned int	lrddata;	/* must be set by
					   ICC_READR
					   ICC_READB 
					*/
					/* length of data which shall be read
				           Value = 0 means the whole record 
					   shall be read 
					*/
	OctetString	*data;		/* must be set by
					   ICC_WRITEB
					   ICC_WRITER
					   ICC_CRYPT
					   ICC_WR_KEY (=key_body)
					   ICC_REGISTER (=file_name)
					   ICC_VERIFY (=auth_data)
					   ICC_IN_AUTH (=auth_data)
					   ICC_EX_AUTH (=auth_data)
					   ICC_MU_AUTH (=auth_data)
					   ICC_VER_CHG (=old_pin)
					   ICC_EXCHAL (=random number)


					*/
	OctetString	*new_pin;	/* must be set by
					   ICC_VER_CHG (=new_pin)
					*/
	FileControlInfo *filecontrolinfo; /* must be set by
						ICC_CREATE
					  */
	DataStruc	datastruc;	/* must be set by
					   ICC_CREATE
					*/
	KeyId		*kid;		/* must be set by
					   ICC_SETKEY (= auth_kid)
					   ICC_WR_KEY
					   ICC_CRYPT
					   ICC_LOCKK
					   ICC_KEYSTAT
					   ICC_VERIFY
					   ICC_IN_AUTH
					   ICC_EX_AUTH
					   ICC_MU_AUTH
					   ICC_VER_CHG
					   ICC_IN_AUTH
					*/
	KeyId		*conc_kid;	/* must be set by
					   ICC_SETKEY
					*/
	KeyAttrList	*keyattrlist;	/* must be set by
					   ICC_WR_KEY
					*/
	KeyAlgId	key_algid;	/* must be set by
					   ICC_WR_KEY
					*/
	Modi		modi;		/* must be set by
					   ICC_CRYPT
					*/
	Operation	operation;	/* must be set by
					   ICC_LOCKK
					   ICC_LOCKF
					*/
	unsigned int	units;		/* must be set by
					   ICC_REGISTER
					*/
	AccessControlValue acv;		/* must be set by
					   ICC_REGISTER
					*/
	FileId		*fid;		/* must be set by
					   ICC_LOCKF
					*/
	AuthControlPar	acp_verify;	/* must be set by
					   ICC_VERIFY
					   ICC_VER_CHG
					*/
	AuthControlPar	acp_auth;	/* must be set by
					   ICC_MU_AUTH
					*/
	ChallengeMode	direction;	/* must be set by
				  	   ICC_EXCHAL
					*/

	};



/* global function definitions  */
/* functions for the SCT interface (/stamod/sct_if.c) */
extern int      	sct_reset();
extern int      	sct_interface();
extern int      	sct_perror();
extern void     	sct_get_errmsg();
extern int      	sct_info();
extern int      	sct_list();
extern int      	sct_setmode();
extern int      	sct_close();
extern int      	sct_secure();
extern int      	sct_init_secure();
extern int      	sct_checksecmess();
extern int      	sct_getsecmess();
extern int      	sct_init_port();
extern int      	sct_get_port();
extern int      	sct_set_portpar();


/* functions for the ICC interface (/stamod/iccde.c) */
extern	int		icc_create();
extern	int		icc_check();
extern  char		icc_e_KeyId();
extern  char		icc_e_FileId();
extern  void		icc_e_KeyAttrList();
extern  int 		icc_check_KeyId();


/* auxiliary functions (/auxil/aux_...c) */
extern int      	aux_cmp_ObjId();
extern ParmType 	aux_ObjId2ParmType();
extern void     	aux_free2_OctetString();
extern void 		aux_free_OctetString();
extern AlgEnc		aux_ObjId2AlgEnc();
extern AlgHash		aux_ObjId2AlgHash();
extern AlgSpecial	aux_ObjId2AlgSpecial();
extern char		*aux_ObjId2Name();
extern AlgType		aux_ObjId2AlgType();
extern AlgMode		aux_ObjId2AlgMode();
extern void     	aux_fxdump();

/*   STARMOD-Functions (/stamod/sca_dev.c)		 */
extern char            	STAGetSCTAlgId();
extern int		STACheckSCT_ICC();
extern int		STACheckKeyAttrList();
extern int		STACheckSecMess();
extern void            	STAErrAnalyse();
extern int		STACreateTrans();
extern int		STACreateHead();
extern int		STAGetVerifyDataBody();
extern unsigned int	STAGetBits();
extern int		STACheckRSAKeyId();
extern int		STAGetSCTState();



#ifdef TEST
/* functions for TEST (/stamod/sca_prnt.c) */
extern void            	print_FileCat();
extern void            	print_FileType();
extern void           	print_DataStruc();
extern void           	print_FileControlInfo();
extern void           	print_SCT_SecMess();
extern void            	print_ICC_SecMess();
extern void            	print_FileSel();
extern void            	print_FileId();
extern void            	print_FileInfoReq();
extern void            	print_FileCloseContext();
extern void           	print_KeyId();
extern void           	print_TransMode();
extern void            	print_KeyPurpose();
extern void            	print_KeyDevSel();
extern void           	print_KeyAttrList();
extern void            	print_VerifyDataStruc();
extern void            	print_KeyIdList();
extern void		print_AccessControlValue();
extern void 		print_WriteMode();
extern void		print_Boolean();
extern void		print_OctetString();
extern void		print_Destination();
extern void		print_ChallengeMode();
extern void		print_KeySel();
extern void		print_More();
extern void		print_Signature();
extern void		print_HashPar();
extern void		print_KeyInfo();
extern void		print_KeyLevel();
extern void		print_KeyDevList();
extern void		print_EncryptedKey();
extern void		print_VerificationKey();
extern void		print_SessionKey();
extern void		print_ComMode();
extern void		print_SCTPort();

#endif

/* functions for dump (/stamod/sca_xdmp.c) */
extern int 		sca_aux_sct_apdu();
extern void 		sca_aux_sct_resp();
extern int 		sca_aux_icc_apdu();
extern void 		sca_aux_icc_resp();







/* global variable definitions - defined in sca_init.c */

extern unsigned int sct_errno;		/* error number set by SCT-Interface */
extern char    *sct_errmsg;		/* pointer to error msg set by      */
					/* SCT-Interface		    */
extern unsigned int icc_errno;		/* error number set by ICC-Interface */
extern char    *icc_errmsg;		/* pointer to error msg set by      */
					/* ICC-Interface		    */
extern Request  	Gsct_request;	/* body of the SCT commands         */
extern OctetString 	Gsct_response;	/* body of the response of the SCT  */
					/* command			    */
extern int      	Gsct_cmd;	/* INS-Code of the SCT command      */
extern struct s_command Gicc_param;	/* body of the icc command          */
extern OctetString 	Gicc_apdu;	/* created icc command		    */
extern SCTInfo  	sctinfo;	/* structure of sct_info-parameter  */

/* type definitions for the ICC-Interface */
#define GICCCMD                 Gicc_param.icc_header.inscode
#define GICCHEAD                Gicc_param.icc_header
#define PAR_FILECAT		Gicc_param.filecat
#define PAR_DATASTRUC		Gicc_param.datastruc
#define PAR_FILECONTROL		Gicc_param.filecontrolinfo
#define PAR_UNITS		Gicc_param.units
#define PAR_ACV			Gicc_param.acv
#define PAR_DATA		Gicc_param.data
#define PAR_FILEINFO		Gicc_param.fileinforeq
#define PAR_FILESEL		Gicc_param.filesel
#define PAR_FILECONTEXT		Gicc_param.context
#define PAR_OPERATION		Gicc_param.operation
#define PAR_FID			Gicc_param.fid
#define PAR_DIRECTION		Gicc_param.direction
#define PAR_KID                 Gicc_param.kid
#define PAR_MODI		Gicc_param.modi
#define PAR_ACP 		Gicc_param.acp_auth
#define PAR_DATASEL		Gicc_param.datasel
#define PAR_SHORT_INDEX		Gicc_param.short_index
#define PAR_LRDDATA		Gicc_param.lrddata





/*------------------------------------------------------------------*/
/*   Length for parameters of the application interface             */
/*------------------------------------------------------------------*/

#define  LEN_KEYHEAD        9              /* length of key_header */


/*------------------------------------------------------------------*/
/*  MAX values for parameters of the application interface          */
/*------------------------------------------------------------------*/

#define MAXL_SCT_DISPLAY    32             /* max length of SCT-display */

#define MAXL_DISPLAY_TEXT   64             /* max length of SCT-display-text */

#define MAXL_SU_PIN         16             /* max length of super pin */

#define MAXL_PIN            8              /* max length of pin */

#define MAXL_RNO            255            /* max length of random number */



#define MAX_TEXT_NO         255            /* max textno for SCT-display */

#define MAX_TIME            255            /* max value for time  (seconds) */

#define MAX_KEYID            31            /* max value for key_id.key_number */

#define MAX_KPFC             14            /* max value for key fault presentation counter */

#define MAX_EL_NO           255            /* max value for element_no */

#define MAX_NO              255            /* max value for number of elements */

#define MAX_REC_ID          254            /* max value for record_id */

#define MAX_REC_POS         255            /* max value for the position in record */

#define MAX_LSB             255            /* max value for lsb (transparent file) */

#define MAX_MSB             255            /* max value for msb (transparent file) */

#define MAX_FILEN           15             /* max value for filename */



/*------------------------------------------------------------------*/
/*  MIN values for parameters of the application interface          */
/*------------------------------------------------------------------*/

#define MINL_RSAKEY         512            /* min length of RSA-Keysize */



/*------------------------------------------------------------------*/
/*  Default values for parameters of the application interface      */
/*------------------------------------------------------------------*/

 


/*------------------------------------------------------------------*/
/*  Definitions for SCT-Interface                                   */
/*------------------------------------------------------------------*/

#define LEN_APDU_HEADER    5               	/* length of APDU-Header */
#define SCT_RSA_ENC_FIRST_DATLEN	183
#define SCT_RSA_ENC_NEXT_DATLEN	        120
#define SCT_DES_ENC_DEC_DATLEN		240
#define SCT_RSA_DEC_DATLEN		64
        
/*------------------------------------------------------------------*/
/*  Definitions for SC-Interface                                    */
/*------------------------------------------------------------------*/
#define ICC_CRYPT_NORMDATLEN			80
#define ICC_CRYPT_AUTHDATLEN			80
#define ICC_CRYPT_CONCDATLEN			80
#define ICC_CRYPT_COMBDATLEN			72




/*------------------------------------------------------------------*/
/*  type definitions		                                    */
/*------------------------------------------------------------------*/
typedef enum {USER_KEY, PIN_KEY, PUK_KEY, DEVICE_KEY} KindOfKey;
                                           /* used for routine:     */
                                           /* check_key_attr_list   */

typedef struct buffer {
        OctetString	data;
        struct buffer  *next;
}Buffer;

#define BUFFERNULL	(Buffer *)0


#endif








