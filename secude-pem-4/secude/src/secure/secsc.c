/*
 *  SecuDE Release 4.3 (GMD),  STARCOS Version 1.1
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

/*----Tranformation functions between the SEC-IF and the SCA-IF-----*/
/*------------------------------------------------------------------*/
/* GMD Darmstadt Institute for System Technic (I2)                  */
/* Rheinstr. 75 / Dolivostr. 15                                     */
/* 6100 Darmstadt                                                   */
/* Project ``SECUDE'' 1993                                          */
/*------------------------------------------------------------------*/
/*                                                                  */
/* PACKAGE   secsc              VERSION   4.3                       */
/*                                      DATE   9.9.93	            */
/*                                        BY   U. Viebeg            */
/* DESCRIPTION                                                      */
/*   This modul provides all functions needed for the               */
/*   transformation between the SEC-IF (project SECUDE) 	    */
/*   and the SCA-IF (project STARCOS).                              */
/*								    */
/*   Via the SEC-IF the communication with the SC (smartcard) and   */
/*   the access to the SC is possible.				    */
/*								    */
/*   Sign/Verify:						    */
/*   The hash-functions are performed in the DTE, the crypto-	    */
/*   functions are performed within the STE (SCT / SC).	            */
/*								    */
/*   PIN for the SW-PSE:					    */
/*   The PIN of the SW-PSE is stored in an WEF on the SC, this file */
/*   can only be read.   					    */
/*								    */
/*                                                                  */
/*   Application:						    */
/*   "sc_app_list[].app_id" is mapped on DF-name for the SC.  	    */
/*                                                                  */
/*   Object:							    */
/*   "pse_sel->object.name" is mapped on key_id or file_id for the  */
/*   SC using the global table "sc_app_list[].sc_obj_list[]".	    */
/*								    */
/*   Security mode:						    */
/*   If an SC-object is to be read/written, the security mode for   */
/*   the communication DTE/SCT and SCT/SC is set according to the   */
/*   parameters of this SC-object (sc_app_list[].sc_obj_list[]).    */
/*   In all other cases the security mode specified for the SCA-IF  */
/*   function, which shall be called, is set (sca_fct_list[]).	    */
/*                                                                  */
/*   Only WEFs whose data structure is TRANSPARENT are accessable   */
/*   via the SEC-IF.						    */
/*								    */
/*   PIN-authentication:					    */
/*   In the following cases the PIN authentication is performed:    */
/*		- The PIN for the SW-PSE shall be read from the SC. */
/*		- A key from the SC shall be used.	            */
/*		- A file on the SC shall be created or deleted.	    */
/*		- Data shall be written into a file on the SC.	    */
/*								    */
/*                                                                  */
/* EXPORT                                                           */
/*   Functions which handle with keycards/device key sets:	    */
/*   delete_devkeyset()  	Delete a device key set stored in   */
/*				the SCT. 			    */
/*   gen_devkeyset()		Generate a device key set in the    */
/*				SCT.				    */		
/*   load_devkeyset()		Load a device key set from a        */
/*				keycard into the SCT.		    */
/*   pre_devkeyset()		Function to be called before using  */
/*				a device key set.		    */
/*   re_devkeyset()		Function to be called after having  */
/*				used a device key set.		    */
/*   request_keycard()		Eject an inserted smartcard and	    */
/*				request a keycard.		    */
/*   write_on_keycards()	Write the device key set on one or  */
/*				more keycards.			    */
/*								    */
/*								    */
/*   secsc_close()   	  	Close application on the SC.	    */
/*   secsc_chpin()     		Change PIN for application on SC.   */
/*   secsc_create()     	Create file (WEF) on the SC.        */
/*   secsc_decrypt()     	Decrypt bitstring within SCT/SC.    */
/*   secsc_delete()     	Delete file (WEF) on the SC.        */
/*   secsc_del_key()     	Delete key stored in an SCT.	    */
/*   secsc_encrypt()     	Encrypt octetstring within SCT/SC.  */
/*   secsc_gen_key()  	        Generate DES or RSA key.            */
/*   secsc_get_EncryptedKey()   Encrypt key within SCT/SC.	    */
/*   secsc_open()  	        Open application on the SC.         */
/*   secsc_put_EncryptedKey()   Decrypt key within SCT/SC.	    */
/*   secsc_read()  	        Read data from file (WEF) on SC     */
/*				into octetstring.		    */
/*   secsc_sc_eject()  	        Eject SC(s).			    */
/*   secsc_sign()  	        Sign octetstring with key from SC.  */
/*   secsc_unblock_SCpin()      Unblock blocked PIN of the SC-app.  */
/*   secsc_verify()  	        Verify a digital signature.	    */
/*   secsc_write()  	        Write octetstring into file (WEF)   */
/*				on SC.				    */
/*								    */
/*  Auxiliary Functions                                             */
/*   aux_AppName2SCApp()	Get information about an SC app.    */
/*   aux_AppObjName2SCObj()	Get information about an SC object  */
/*			        belonging to an application on SC.  */
/*   get_pse_pin_from_SC()	Read the PIN for the SW-PSE from    */
/*				the SC and sets it in 		    */
/*			        "sct_stat_list[]".		    */
/*   handle_SC_app()		If application not open, open it.   */
/*   release_SCT()		Release port to SCT (initialize	    */
/*				communication with the SCT).	    */
/*   reset_SCT()		Delete SCT configuration data and   */
/*			        reset SCT.			    */
/*   SC_configuration()		Perform SC configuration (get data  */
/*				form file ".scinit".		    */
/*   SCT_configuration()	Perform SCT configuration (get data */
/*   				from a prior process).              */
/*								    */
/*                                                                  */
/* STATIC                                                           */
/*   activate_devkeyset()	Activate DevKeySet for a certain    */
/*				application.			    */
/*   analyse_sca_err()		Analyse of an error-number 	    */
/*				returned by an SCA-IF function.     */
/*   aux_FctName2FctPar()	Get security parameter for an SCA-  */
/*				Function.			    */
/*   aux_purpose2name()		Transform purpose into a printable  */
/*				character representation.	    */ 
/*   bell_function()		"Ring the bell" to require user     */
/*                              input at the SCT.		    */
/*   check_pin()		Check the entered PIN (keycard).    */
/*   copy_SCT_entry()		Copy values of SCT_entry.	    */
/*   copy_SCT_port()		Copy values of SCTPort.	    	    */
/*   delete_old_SCT_config()    Delete old SCT configuration file.  */
/*   device_authentication() 	Perform device authentication 	    */
/*				according to the add. file info of  */
/*				the selected application on the SC. */
/*   display_on_SCT()		Display string on SCT-display.	    */
/*   eject_sc()			Handle the ejection of the SC.	    */
/*   enter_app_in_sctlist()	Enter information about app in      */
/*				sct_list for current SCT.           */
/*   fprint_KeyId()		Display values of KeyId on stderr.  */
/*   fprint_KeyPurpose()	Display values of KeyPurpose on     */
/*				stderr.				    */
/*   gen_process_key()		Generate and set new process key.   */
/*   get_connected_SCT()	Get first SCT of the registered SCTs*/
/*				which is connected to the DTE       */
/*			        (not used)			    */
/*   get_appid()		Get application id for a given 	    */
/*				application name.		    */
/*   get_keyid_for_obj()	Get keyid for object.		    */
/*   get_DecSK_name()		Get name of decryption key on SC.   */
/*   get_devkeyset_info()	Provides infos about the device     */
/*				key set used within this software.  */
/*   get_process_key()		Get process key for encryption /    */
/*				decryption of SCT config.	    */
/*   get_SC_fileid()		Compose structure FileId (for a WEF */
/*				on the SC).			    */
/*   get_SC_keyid()		Compose structure KeyId (for a key  */
/*				on the SC).			    */
/*   get_SCT_config_fname()	Get name of SCT configuartion file. */
/*   getbits	                Get n bits of byte x from 	    */
/*			        position p.			    */
/*   handle_gen_DecSK()		Special handling of objects	    */
/*				"DecSK_new", "DecSKold", resp..	    */
/*   handle_key_sc_app()	Handle SC-application for the       */
/*                              selected key.			    */
/*   int2ascii()		Transform an integer value into a   */
/*			        NULL terminated ASCII character     */
/*				string.				    */
/*   init_SCT_config()		Initialize SCT configuration list.  */
/*   int_to_keyid()		Transform integer value into        */
/*				structure KeyId (for a key on       */
/*				the SC).	   		    */
/*   int_to_fileid()		Transform integer value into        */
/*				structure FileId (for a WEF on      */
/*				the SC).			    */
/*   is_SC_app_open()		Return whether application on SC    */
/*				has been opened.                    */
/*   itos()   			Transform integer to char-string.   */
/*   keyref_to_keyid()	        Transform keyref into structure     */
/*                              keyid.				    */
/*   key_to_keyid()	        Get key_id from key.		    */
/*   open_SC_application() 	Require SC, open SC application,    */
/*				perform device authenticationn.     */
/*   perform_SCT_config()	Restore or reset (initialize) SCT   */
/*				configuration.			    */
/*   prepare_sec_mess()		- Generate session key,	    	    */
/*				- Set security mode for the 	    */
/*				  communication between DTE/SCT.    */
/*   print_SCT_entry()		Print values of SCT_entry.	    */
/*   print_SCT_port()		Print values of SCTPort.    	    */
/*   read_SCT_config()		Read and decrypt SCT configuration  */
/*				data for the specified SCT.	    */
/*   request_sc()		Request and initialize a smartcard. */
/*   save_devkeyset()		Save a device key set on a keycard. */
/*   save_portparam()		Get port parameter from STARMOD and */
/*				save into SCT configuration file.   */
/*   set_fct_sec_mode()		Set security mode for communication */
/*				between DTE/SCT depending on the    */
/*                	        SCA-function to be called.          */
/*   stoi()			Transform char-string to integer.   */
/*   user_authentication()      Perform user authentication         */
/*				(PIN or PUK).		            */
/*   write_SCT_config()		Encrypt and write SCT configuration */
/*				data for the specified SCT.         */
/*                                                                  */
/* IMPORT              		              		            */
/*  Functions of SCA-IF:  			                    */
/*   sca_activate_devkeyset()	Activate device key set in the SCT. */
/*   sca_auth()			Device authentication  		    */
/*   sca_change_pin()		Change PIN on the smartcard.	    */
/*   sca_close_file()		Close file on the smartcard.        */
/*   sca_create_file()		Create file on the smartcard.       */
/*   sca_dec_des_key()		Decrypt an rsa encrypted DES key.   */
/*   sca_decrypt()		Decrypt octetstring.   	            */
/*   sca_delete_file()		Delete file on the smartcard.       */
/*   sca_del_dev_key()		Delete a device key stored in the   */
/*				SCT.				    */
/*   sca_del_user_key()		Delete user key in an SCT.	    */
/*   sca_display()		Display text on SCT-display.        */
/*   sca_eject_icc()		Eject smartcard. 	            */
/*   sca_enable_deletion()	Compare 'own' device key set in the */
/*				SCT with the device key set in a    */
/*				keycard and set the delete flag for */
/*				this set.			    */
/*   sca_enc_des_key()		Encrypt DES key with RSA. 	    */
/*   sca_encrypt()		Encrypt octetstring. 		    */
/*   sca_gen_dev_key()		Generate a device key.		    */
/*   sca_gen_user_key()		Generate user key in the SCT.       */
/*   sca_gen_sessionkey()	Generate session key(s) for secure  */
/*				messaging between DTE and SCT.	    */
/*   sca_get_sctport()		Get the actual DTE-SCT communication*/
/*				parameters.			    */
/*   sca_get_sct_info()		Get information about registered    */
/*				SCTs.				    */
/*   sca_init_icc()		Request and initialize a smartcard. */
/*   sca_inst_user_key()	Install user key on the smartcard.  */
/*   sca_load_devkeyset()	Load device key set from a keycard  */
/*				into the SCT.			    */
/*   sca_read_binary()		Read data from transparent 	    */
/*				elementary file on the smartcard.   */
/*   sca_reset()		Reset communication between DTE and */
/*				SCT.				    */
/*   sca_save_devkeyset()	Save device key set on a keycard.   */
/*   sca_select_file()		Select file on the smartcard.       */
/*   sca_set_mode()		Set security mode.  		    */
/*   sca_sign()			Sign octetstring.  		    */
/*   sca_verify_pin()		PIN authentication  		    */
/*   sca_verify_sig()		Verify a digital signature.	    */
/*   sca_unblock_pin()          Unblock blocked PIN of the SC-app.  */
/*   sca_write_binary()		Write data in transparent WEF on    */
/*				the SC.	    			    */
/*                                                                  */
/*								    */
/*  SC_CONFIGURATION:						    */
/*   display_SC_configuration() Display the actual SC               */
/*				configuration ("sc_app_list[]").    */
/*   read_SC_configuration()    Read SC configuration file into     */
/*				global structure "sc_app_list[]".   */
/*								    */
/*                                                                  */
/*  Auxiliary Functions of SECUDE		                    */
/*   aux_xdump()		dump buffer			    */
/*   aux_fxdump()		dump buffer in file		    */
/*   aux_add_error()		Add error to error stack.	    */
/*   aux_free2_OctetString()	Release the octets-buffer in        */
/*			    	structure OctetString		    */
/*   aux_new_OctetString()	Create new OctetString.		    */
/*   aux_cmp_UTCTime()		Compare two time-values (UTCTime).  */
/*   aux_cpy_String()		Copy string.	 		    */
/*   aux_free_KeyBits()		Release members of struct 	    */
/*				KeyBits and KeyBits.		    */
/*   aux_ObjId2AlgEnc()		Map object identifier on 	    */
/*				algorithm encryption method.	    */
/*   aux_ObjId2AlgHash()	Map object identifier on     	    */
/*				algorithm hash method.		    */
/*   aux_ObjId2AlgType()	Map object identifier on 	    */
/*				algorithm type.			    */
/*   aux_ObjId2ParmType()	Map object identifier on 	    */
/*				type of parameter.		    */
/*   d_KeyBits()		Decode given BitString into 	    */
/*				structure KeyBits.		    */
/*   e2_KeyBits()		Encode given structure KeyBits into */
/*				a given structure BitString.	    */
/*   get_update_time_SCToc()	Get update time of object in SC-Toc.*/
/*   is_in_SCToc()	        Check whether object in SCToc.	    */
/*   sec_psetest()		Check whether SC available and      */
/*				application is an SC app.	    */		
/*   sec_scttest()		Check whether SCT is available.     */
/*   strzfree()			Free string.			    */
/*   update_SCToc()		Update entry in SCToc.		    */
/*								    */
/*                                                                  */
/*  Global Variables                                                */
/*   sca_errno			Global error variable set by STAPAC */
/*   sca_errmsg			Global pointer to error message set */
/*                              by STAPAC 			    */
/*   sct_stat_list[]		Current status information          */
/*				about the SCTs (secure messaging    */
/*				parameter, app_name).   	    */
/*   sca_fct_list[]		List of the SCA-functions           */
/*				and the belonging secure messaging  */
/*				parameters.			    */
/*   sc_app_list[]		List of the applications available  */
/*				on the SC, including the list of    */
/*				all objects (app specific),         */
/*				-which shall be stored on the SC or */
/*				-which are stored on the SC.	    */
/*   sc_sel			Selection of SC and SCT	:	    */
/*                              sc_sel.sct_id - select SCT          */
/*------------------------------------------------------------------*/

#ifdef SCA

#include "filename.h"		/* Names of PSE Objects 
				   ("DecSKnew_name", "DecSKold_name", 
				    "SK_new_name", "SK_old_name" */
#include "secsc.h"		/* definitions for this module	 */


#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>


#define FILEMASK 0600		/* file creation mask (for SCT configuration files) */



#ifdef __STDC__
static int	activate_devkeyset	(int sct_id, char *app_name);
static int	analyse_sca_err		(int sct_id, char *sca_fct_name);
static char	*aux_purpose2name	(KeyPurpose purpose);
static int	check_pin		(char *pin);
static int	open_SC_application	(int sct_id, char *app_name);
static int	request_sc		(int sct_id, char *display_text, int time_out);
static int	eject_sc		(int sct_id, char *display_text, Boolean alarm);
static int	is_SC_app_open		(int sct_id, char *app_name);
static int	enter_app_in_sctlist	(int sct_id, char *app_name);
static void 	fprint_KeyId		(KeyId *key_id);
static void 	fprint_KeyPurpose	(KeyPurpose key_purpose);
static void	display_on_SCT		(int sct_id, char *display_text);
static int	handle_key_sc_app	(int sct_id, Key *key, KeyId *key_id);
static int      get_connected_SCT	();
static int      get_devkeyset_info	(char *app_name, KeyDevSel *key_dev_info[], unsigned int *no_of_devkeys);
static char	*get_process_key	();
static void	init_SCT_config		(int sct_id);
static int	perform_SCT_config	(int sct_id);
static SCTStatus *read_SCT_config	(int sct_id);
static char	*get_SCT_config_fname	(int sct_id);
static void 	copy_SCT_entry		(SCTStatus *from, SCTStatus *to);
static void 	copy_SCT_port		(SCTPort *from, SCTPort *to);
static void 	print_SCT_entry		(SCTStatus *sct_entry);
static void 	print_SCT_port		(SCTPort *sct_port);

static int	write_SCT_config	(int sct_id);
static int	delete_old_SCT_config	(int sct_id);
static int	prepare_sec_mess	(int sct_id, SCT_SecMess *new_sm_SCT, Boolean reset_sm);
static int	set_fct_sec_mode	(int sct_id, char *SCA_fct_name, ICC_SecMess *sm_SC);
static int	device_authentication	(unsigned int sct_id, char *app_name, AddInfo *add_info);
static int	user_authentication	(unsigned int sct_id, char *app_name, VerifyDataType pin_type);
static int	handle_gen_DecSK	(Key *key, Boolean replace, char **new_DecSK_name, Boolean *new_replace_value);
static OctetString *get_appid		(char *app_name);
static void	get_SC_fileid		(unsigned int sc_id, FileId *file_id);
static void	get_SC_keyid		(unsigned int sc_id, Boolean SC_internal, KeyId *key_id);
static int	getbits			(unsigned x, unsigned p, unsigned n);
static int	int2ascii		(char s[], int n);
static int	get_keyid_for_obj	(char *app_name, char *obj_name, Boolean SC_internal, KeyId *key_id);
static int	keyref_to_keyid		(int keyref, Boolean SC_internal, KeyId *key_id);
static int	itos			(unsigned int int_value, char *char_string, unsigned int max_len);
static int 	save_devkeyset		(OctetString *app_id, KeyDevSel *key_dev_info[], unsigned int no_of_devkeys, OctetString	*keycard_pin);
static int	save_portparam		(int sct_id);
static int	stoi			(char *char_string, unsigned int *int_value, unsigned int max_len);
static void	bell_function	();
static int	key_to_keyid		(Key *key, AlgId *key_alg, KeyId *key_id, Boolean special_DecSK_selection);

#else


static int	activate_devkeyset();
static int      analyse_sca_err();
static SCAFctPar *aux_FctName2FctPar();
static char	*aux_purpose2name();
static void     bell_function();
static int	check_pin();
static void 	copy_SCT_entry();
static void 	copy_SCT_port();
static int      delete_old_SCT_config();
static int      device_authentication();
static void     display_on_SCT();
static int      eject_sc();
static int      enter_app_in_sctlist();
static void 	fprint_KeyId();
static void 	fprint_KeyPurpose();
static int	gen_process_key();
static OctetString *get_appid();
static int      get_connected_SCT();
static char     *get_DecSK_name();
static int	get_devkeyset_info();
static int      get_keyid_for_obj();
static char 	*get_process_key();
static void     get_SC_fileid();
static void     get_SC_keyid();
static char 	*get_SCT_config_fname();
static int      getbits();
static int      handle_gen_DecSK();
static int      handle_key_sc_app();
static int      int2ascii();
static void	init_SCT_config();
static void     int_to_fileid();
static void     int_to_keyid();
static int      is_SC_app_open();
static int      itos();
static int      key_to_keyid();
static int      keyref_to_keyid();
static int     	open_SC_application();
static int	perform_SCT_config();
static int      prepare_sec_mess();
static void 	print_SCT_entry();
static void 	print_SCT_port();
static SCTStatus *read_SCT_config();
static int      request_sc();
static int	save_devkeyset();
static int	save_portparam();
static int      set_fct_sec_mode();
static int      stoi();
static int      user_authentication();
static int	write_SCT_config();


/*
 *    Extern declarations
 */

extern int      sca_activate_devkeyset();
extern int      sca_auth();
extern int      sca_change_pin();
extern int      sca_close_file();
extern int      sca_create_file();
extern int      sca_dec_des_key();
extern int      sca_decrypt();
extern int      sca_delete_file();
extern int	sca_del_dev_key();
extern int      sca_del_user_key();
extern int      sca_eject_icc();
extern int	sca_enable_deletion();
extern int      sca_enc_des_key();
extern int      sca_encrypt();
extern int	sca_gen_dev_key();
extern int	sca_gen_sessionkey();
extern int      sca_gen_user_key();
extern int      sca_get_sct_info();
extern int	sca_get_sctport();
extern int      sca_init_icc();
extern int      sca_inst_user_key();
extern int	sca_load_devkeyset();
extern int      sca_read_binary();
extern int	sca_save_devkeyset();
extern int      sca_select_file();
extern int      sca_set_mode();
extern int      sca_sign();
extern int      sca_unblock_pin();
extern int      sca_verify_pin();
extern int      sca_verify_sig();
extern int      sca_write_binary();

#endif /* not __STDC__ */




extern unsigned int sca_errno;	/* error number set by STAPAC       */
extern char    *sca_errmsg;	/* pointer to error message set by  */
				/* STAPAC                           */



/*
 *    Local variables, but global within secsc.c
 */

static unsigned int secsc_errno;/* internal error-number	       */
static int      sca_rc;		/* return code of the SCA-functions */
static unsigned int i;



typedef enum {
	F_null, F_encrypt, F_decrypt,
	F_hash, F_sign, F_verify
}


                FTYPE;
static FTYPE    sec_state = F_null;







/*--------------------------------------------------------------*/
/*						                */
/* PROC  secsc_create					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  a) Create an object on the SC:				*/
/*     Assumption: the belonging application has been opened by */
/*     the calling routine.					*/
/*								*/
/*     1) Get parameters for the object to be created from the	*/
/*        global variable "sc_app_list[].sc_obj_list[]".	*/
/*        Case 1: Object on the SC is a key			*/
/*		  => return(ESCNOTSUPP)				*/
/*	  Case 2: Object on the SC is a file:			*/
/*     2) The user authentication is performed.			*/
/*     3) Parameters for "Create WEF on SC":			*/
/*			   - "sc_obj_list[]" delivers:		*/
/*			      - number of bytes for the file	*/
/*			      - short index of the WEF on the SC*/
/*			   - "sca_fct_list[]" delivers:		*/
/*			      - sec_mess			*/
/*		           - constant values for:		*/
/*			      - file_cat = EF			*/
/*			      - data_struc = TRANSPARENT	*/
/*??????????????????????????????????????????????????????????????*?
/*			      - delete record acv = 0x80	*/
/*			      - read acv = 0x80			*/
/*			      - write acv = 0x80		*/
/*			      - delete file acv = 0x80		*/
/*??????????????????????????????????????????????????????????????*/
/*			      - access_sec_mode = ACCESS_NORMAL */
/*			      - access to file : read / write	*/
/*			      - file is erasable		*/
/*			      - add_info = "    "		*/
/*     4) Init. the number of relevant data (first octets in    */
/*        the WEF with 0.					*/
/*								*/
/*  b) Create an application:					*/
/*     In this case the error ESCNOTSUPP is returned.		*/
/*     The creation of an application on the SC is not 		*/
/*     supported.						*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   pse_sel	 	       					*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0			       o.k.				*/
/*   -1			       Error				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sca_close_file()		Close file on the SC. 		*/
/*				(close creation process)	*/
/*   sca_create_file()		Create file on the SC. 		*/
/*								*/
/*   aux_AppObjName2SCObj()	Get information about an SC     */
/*        			object belonging to an 		*/
/*                              application on SC. 		*/
/*   get_SC_fileid()		Compose structure FileId (for a */
/*				WEF on the SC).			*/
/*   set_fct_sec_mode()		Set security mode for comm.     */
/*				between DTE/SCT depending on the*/
/*                	        SCA-function to be called.      */
/*   user_authentication()      Perform user authentication 	*/
/*				(PIN).				*/
/*                                                              */
/*   analyse_sca_err()		Analyse of an error-number 	*/
/*				returned by an SCA-IF function. */
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure secsc_create
 *
 ***************************************************************/
#ifdef __STDC__

RC secsc_create(
	PSESel	 *pse_sel
)

#else

RC secsc_create(
	pse_sel
)
PSESel	 *pse_sel;

#endif

{

	/* Variables for the SCA-IF */
	unsigned int    sct_id;
	FileCat         file_cat;
	DataStruc       data_struc;
	FileControlInfo file_control_info;
	ICC_SecMess     sm_SC;	/* sec. mode for communication SCT/SC	 */
	unsigned int	offset;
	FileCloseContext file_close_context;

	FileId          file_id;
	OctetString     out_data;
	unsigned int	no_of_written_octets;


	/* Variables for internal use */
	SCObjEntry     *sc_obj_entry;
	FileSel         file_sel;
	AddInfo		add_info;
	int             rest;
	char            WEF_len[WEF_LEN_BYTES];	/* The length of the data to be
						 * written is stored in the first
						 * bytes of the WEF on the SC.	
				 		 */


	char           *proc = "secsc_create";

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	secsc_errno = NOERR;
	sca_rc = 0;
	sct_id = sc_sel.sct_id;


	if (pse_sel->object.name && strlen(pse_sel->object.name)) {

		/*
		 * Create object on the SC
		 */

		/* get information about the object to be created */
		sc_obj_entry = aux_AppObjName2SCObj(pse_sel->app_name, pse_sel->object.name);
		if (sc_obj_entry == NULLSCOBJENTRY) {
			aux_add_error(EINVALID, "Cannot get configuration info for SC-object to be created", (char *) pse_sel, PSESel_n, proc);
			return (-1);
		}
		if (sc_obj_entry->type == SC_KEY_TYPE) {
			/* object is a key */
			aux_add_error(ESCNOTSUPP, "Cannot create object 'key' on SC.", (char *) pse_sel, PSESel_n, proc);
			return (-1);
		}


		/*
		 *  For creation an object the user authentication is required.
		 */	

		if (user_authentication(sct_id, pse_sel->app_name, PIN)) {
			aux_add_error(LASTERROR, "User (PIN) authentication (prior to creation of an SC-object) failed", (char *) pse_sel, PSESel_n, proc);
			return (-1);
		}


		/*
		 *  Object to be created is a file
		 */

		/* set security mode for SCA-function */
		if (set_fct_sec_mode(sct_id, "sca_create_file", &sm_SC)) {
			aux_add_error(LASTERROR, "Set security mode (prior to creation of an SC-object) failed", (char *) pse_sel, PSESel_n, proc);
			return (-1);
		}
		/* Create WEF on the SC */
		rest = sc_obj_entry->size % UNIT_SIZE;
		if (rest > 0)
			file_control_info.units = sc_obj_entry->size / UNIT_SIZE + 1;
		else
			file_control_info.units = sc_obj_entry->size / UNIT_SIZE;

/*?????????????????????????????????????????????????????????????????????????????????????*/
/* 
   Die folgenden Werte sind abhaengig davon, ob mutual authentication zwischen SCT und SC
   im Automaten auf der SC verlangt ist oder nicht, das erkennt die SECSC-Software 
   an den add_info vom sca_select_file(). D.h. man sollte die add_info global machen oder
   in die sct_stat_list eintragen. Ueber den Aufbau der add_info muss man sich noch mal
   Gedanken machen. 
   Als erste Variante wird die device-authentication nicht mit beruecksichtigt, d.h.
   es werden feste Werte eingesetzt.
*/

		file_control_info.delrec_acv.compare_mode = ST_GE;
		file_control_info.delrec_acv.state_number = 0;
		file_control_info.racv.compare_mode = ST_GE;
		file_control_info.racv.state_number = 0;
		file_control_info.wacv.compare_mode = ST_GE;
		file_control_info.wacv.state_number = 0;
		file_control_info.delfile_acv.compare_mode = ST_GE;
		file_control_info.delfile_acv.state_number = 0;
/*?????????????????????????????????????????????????????????????????????????????????????*/


		file_control_info.access_sec_mode = ACCESS_NORMAL;
		file_control_info.readwrite = READ_WRITE;
		file_control_info.execute = FALSE;
		file_control_info.not_erasable = FALSE;	/* => file is erasable */
		file_control_info.recordsize = 0;
		file_control_info.file_sel = &file_sel;
		file_control_info.file_sel->file_name = NULLOCTETSTRING;
		file_control_info.file_sel->file_id = &file_id;
		get_SC_fileid(sc_obj_entry->sc_id, file_control_info.file_sel->file_id);
		file_control_info.addinfo = &add_info;
		file_control_info.addinfo->noctets = 4;
		file_control_info.addinfo->octets = "    ";

		sca_rc = sca_create_file(sct_id,
					 file_cat = EF,
					 data_struc = TRANSPARENT,
					 &file_control_info,
					 &sm_SC);

		if (sca_rc < 0) {
			secsc_errno = analyse_sca_err(sct_id, "sca_create_file");
			aux_add_error(secsc_errno, "Creation of an SC-object failed", (char *) pse_sel, PSESel_n, proc);
			return (-1);
		}


/*
 *   Intermediate result: WEF on the SC has been created!
 *
 *            Next to do: Initialize the number of relevant octets in the WEF with 0.
 */

		/* get length (= 0) of object as char-string */
		if (itos(0, &WEF_len[0], WEF_LEN_BYTES)) {
			aux_add_error(EINVALID, "Internal error (get WEF length)", CNULL, 0, proc);
			return (-1);
		}

		out_data.noctets = WEF_LEN_BYTES;
		out_data.octets = &WEF_len[0];
		offset = 0;
#ifdef SECSCTEST
		fprintf(stderr, "\n\nWritten length:\n");
		aux_fxdump(stderr, out_data.octets, out_data.noctets, 0);
		fprintf(stderr, "\n");
#endif

		sca_rc = sca_write_binary(sct_id,
				          0,
					  offset,
					  &out_data,
					  &sc_obj_entry->sm_SC_write,
					  &no_of_written_octets);

		if ((sca_rc < 0) || (no_of_written_octets != WEF_LEN_BYTES)) {
			secsc_errno = analyse_sca_err(sct_id, "sca_write_binary");
			aux_add_error(secsc_errno, "Initialization of the no. of octets in the created SC-object failed", (char *) pse_sel, PSESel_n, proc);
			return (-1);
		}





/*
 *   Intermediate result: WEF on the SC has been created!
 *
 *            Next to do: Close creation process.
 */


		/* set security mode for SCA-function */
		if (set_fct_sec_mode(sct_id, "sca_close_file", &sm_SC)) {
			aux_add_error(LASTERROR, "Set security mode (prior to close creation of an SC-object) failed", (char *) pse_sel, PSESel_n, proc);
			return (-1);
		}
		/* Close creation process */
		sca_rc = sca_close_file(sct_id,
					file_cat = EF,
					file_control_info.file_sel,
					file_close_context = CLOSE_CREATE,
					&sm_SC);
		if (sca_rc < 0) {
			secsc_errno = analyse_sca_err(sct_id, "sca_close_file");
			aux_add_error(secsc_errno, "Close creation of an SC-object failed", (char *) pse_sel, PSESel_n, proc);
			return (-1);
		}
	}
	 /* end if */ 
	else {

		/*
		 * Creation of an application on the SC is not supported
		 */

		aux_add_error(ESCNOTSUPP, "Creation of app on SC not supported", (char *) pse_sel, PSESel_n, proc);
		return (-1);
	}			/* end else */

	return (0);


}				/* end secsc_create() */





/*--------------------------------------------------------------*/
/*						                */
/* PROC  secsc_open					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  If the application (pse_sel->app_name) is not open and no 	*/
/*  other application on the SC is open, it will be opened.	*/
/*								*/
/*  Open an application means:					*/
/*     1. get required SC,					*/
/*     2. open application on SC: 				*/
/*	     - select DF with the app_id identified by 		*/
/*	       pse_sel->app_name,				*/
/*           - perform device authentication according to the	*/
/*	       "additional file information" from the SC	*/
/*	       (the "add_file_info" are returned by the SC, when*/
/*	       an application (DF) on the SC is selected),	*/
/*     4. set application to open for the current SCT:		*/
/*	     - enter app_name into the sct_stat_list for the 	*/
/*	       current SCT.					*/
/*								*/
/*								*/
/*  Open an object means:					*/
/*     If object is in SCToc, "pse_sel->object.pin" is set to 	*/
/*     CNULL and 0 is returned.					*/
/*     Otherwise -1 is returned.				*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   pse_sel	 	       					*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0			       o.k.				*/
/*   -1			       Error				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   handle_SC_app()		If application not open, open it*/
/*   is_in_SCToc()	        Check whether object in SCToc.  */
/*			         		       		*/
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure secsc_open
 *
 ***************************************************************/
#ifdef __STDC__

RC secsc_open(
	PSESel	 *pse_sel
)

#else

RC secsc_open(
	pse_sel
)
PSESel	 *pse_sel;

#endif

{

	/* Variables for the SCA-IF */
	unsigned int    sct_id;

	int             obj_in_SCToc;

	char           *proc = "secsc_open";

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	secsc_errno = NOERR;
	sca_rc = 0;
	sct_id = sc_sel.sct_id;



	/* If application (app_name) not open, open it */
	if (handle_SC_app(pse_sel->app_name)) {
		AUX_ADD_ERROR;
		return (-1);
	}

	if (pse_sel->object.name && strlen(pse_sel->object.name)) {

		/*
		 * Check, whether object in SCToc
		 */

		if ((obj_in_SCToc = is_in_SCToc(pse_sel)) == -1) {
			aux_add_error(EOBJNAME, "Check 'is obj in SCToc' failed", (char *) pse_sel, PSESel_n, proc);
			return (-1);
		}
		if (obj_in_SCToc == FALSE) {
			aux_add_error(EOBJNAME, "SC-Object is not in SCToc", (char *) pse_sel, PSESel_n, proc);
			return (-1);
		}

		/*
		 * Open an object on the SC => dummy function
		 */

		pse_sel->object.pin = CNULL;

		return (0);

	}			/* end if */
	return (0);


}				/* end secsc_open() */






/*--------------------------------------------------------------*/
/*						                */
/* PROC  secsc_close					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  If application not open (i.e. no application open or other  */
/*  application open)  =>   return (error).			*/
/*								*/
/*  a) Close an object on the SC:				*/
/*     1. Check whether object in SCToc.			*/
/*        If not => return(error)				*/
/*     2. As an object (file, key) on the SC cannot be closed 	*/
/*        with this function, this call is ignored. 		*/
/*        In this case "pse_sel->object.pin" is set to CNULL 	*/
/*        and 0	is returned.					*/
/*								*/
/*  b) Close an application on the SC:				*/
/*     - DF-name = app_id belonging to the given app_name.	*/
/*     - close application on the SC.				*/
/*       All files (EFs) belonging to this application are      */
/*       closed automatically.					*/
/*     - set application to close for SCT			*/
/*     Storage of "pse_sel->pin" (PIN for the SW-PSE) is 	*/
/*     released in the calling routine "sec_close()".		*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   pse_sel	 	       					*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0			       o.k.				*/
/*   -1			       Error				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sca_close_file()		Close file on SC. 	        */
/*								*/
/*   get_appid()		Get application id for a given  */
/*				application name.		*/
/*   is_in_SCToc()	        Check whether object in SCToc.  */
/*   is_SC_app_open()		Return whether application has  */
/*				been opened.                    */
/*   set_fct_sec_mode()		Set security mode for comm.     */
/*				between DTE/SCT depending on the*/
/*                	        SCA-function to be called.      */
/*                                                              */
/*   analyse_sca_err()		Analyse of an error-number 	*/
/*				returned by an SCA-IF function. */
/*   aux_add_error()		Add error to error stack.	*/
/*                                                              */
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure secsc_close
 *
 ***************************************************************/
#ifdef __STDC__

RC secsc_close(
	PSESel	 *pse_sel
)

#else

RC secsc_close(
	pse_sel
)
PSESel	 *pse_sel;

#endif

{

	/* Variables for the SCA-IF */
	unsigned int    sct_id;
	FileCat         file_cat;
	FileSel         file_sel;
	FileCloseContext file_close_context;
	ICC_SecMess     sm_SC;	/* sec. mode for communication SCT/SC	 */
	char           *display_text;
	Boolean         alarm;


	/* Variables for internal use */
	int             obj_in_SCToc;
	int		rc;
	char            *proc = "secsc_close";

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	secsc_errno = NOERR;
	sca_rc = 0;
	sct_id = sc_sel.sct_id;


	
	rc = is_SC_app_open(sct_id, pse_sel->app_name);
	if (rc != TRUE) {
		if (rc == FALSE)
			aux_add_error(EAPPNAME, "Application to be closed has not been opened!", (char *) pse_sel->app_name, char_n, proc);
		else
			AUX_ADD_ERROR;
		return (-1);
	}


	if (pse_sel->object.name && strlen(pse_sel->object.name)) {

		/*
		 * Check, whether object to be closed is in SCToc
		 */

		if ((obj_in_SCToc = is_in_SCToc(pse_sel)) == -1) {
			aux_add_error(EOBJNAME, "Check 'is obj in SCToc' failed", (char *) pse_sel->object.name, char_n, proc);
			return (-1);
		}
		if (obj_in_SCToc == FALSE) {
			aux_add_error(EOBJNAME, "SC-Object to be closed does not exist", (char *) pse_sel->object.name, char_n, proc);
			return (-1);
		}


		/*
		 * Close an object => dummy function
		 */

		pse_sel->object.pin = CNULL;
		return (0);

	}
	else {

		/*
		 * Close an application!
		 *	       - close application on the SC. 
		 *             - set application to close for SC
		 */


		/* set security mode for SCA-function */
		if (set_fct_sec_mode(sct_id, "sca_close_file", &sm_SC)) {
			aux_add_error(LASTERROR, "Set security mode (prior to close the SC-application) failed", (char *) pse_sel, PSESel_n, proc);
			return (-1);
		}

		file_sel.file_name = get_appid(pse_sel->app_name);
		if (!(file_sel.file_name)) {
			aux_add_error(EINVALID, "Internal error (can't get app_id for app_name)", (char *) pse_sel->app_name, char_n, proc);
			return (-1);
		}

		/* close application on the SC */
		sca_rc = sca_close_file(sct_id,
					file_cat = PARENT_DF,
					&file_sel,
					file_close_context = CLOSE_SELECT,
					&sm_SC	
					);

		if (sca_rc < 0) {
			secsc_errno = analyse_sca_err(sct_id, "sca_close_file");
			aux_add_error(secsc_errno, "Close SC-application failed", (char *) (char *) pse_sel->app_name, char_n, proc);
			aux_free_OctetString(&file_sel.file_name);
			return (-1);
		}

		aux_free_OctetString(&file_sel.file_name);

		/* set application to CLOSE for the current SCT */
		if (enter_app_in_sctlist(sct_id, CNULL)) {
			AUX_ADD_ERROR;
			return (-1);
		}
	}			

	return (0);

}				/* end secsc_close() */





/*--------------------------------------------------------------*/
/*						                */
/* PROC  secsc_delete					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  a) Delete an object on the SC:				*/
/*     Assumption: the belonging application has been opened by */
/*     the calling routine.					*/
/*								*/
/*     1) Get parameters for the object to be deleted from the	*/
/*        global variable "sc_app_list[].sc_obj_list[]".	*/
/*        Case 1: Object on the SC is a key			*/
/*		  => return(ESCNOTSUPP)				*/
/*		     Deletion of a key on the SC is not         */
/*		     supported.					*/
/*	  Case 2: Object on the SC is a file:			*/
/*     2) The user authentication is performed.			*/
/*     3) Delete WEF on SC:					*/
/*			   - "sc_obj_list[]" delivers:		*/
/*			      - short index of the WEF on the SC*/
/*			   - "sca_fct_list[]" delivers:		*/
/*			      - sec_mess			*/
/*		           - constant values for:		*/
/*			      - file_cat = EF			*/
/*								*/
/*  b) Delete an application:					*/
/*     In this case the error ESCNOTSUPP is returned.		*/
/*     The deletion of an application on the SC is not 		*/
/*     supported.						*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   pse_sel	 	       					*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0			       o.k.				*/
/*   -1			       Error				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sca_delete_file()		Delete (WEF) file on the SC. 	*/
/*							       	*/
/*   aux_AppObjName2SCObj()	Get information about an SC     */
/*        			object belonging to an 		*/
/*                              application on SC. 		*/
/*   get_SC_fileid()		Compose structure FileId (for a */
/*				WEF on the SC).			*/
/*   set_fct_sec_mode()		Set security mode for comm.     */
/*				between DTE/SCT depending on the*/
/*                	        SCA-function to be called.      */
/*   user_authentication()      Perform user authentication     */
/*				(PIN).				*/
/*                                                              */
/*   analyse_sca_err()		Analyse of an error-number 	*/
/*				returned by an SCA-IF function. */
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure secsc_delete
 *
 ***************************************************************/
#ifdef __STDC__

RC secsc_delete(
	PSESel	 *pse_sel
)

#else

RC secsc_delete(
	pse_sel
)
PSESel	 *pse_sel;

#endif

{

	/* Variables for the SCA-IF */
	unsigned int    sct_id;
	FileCat         file_cat;
	ICC_SecMess     sm_SC;	/* sec. mode for communication SCT/SC	 */
	FileSel         file_sel;
	FileId          file_id;


	/* Variables for internal use */
	SCObjEntry     *sc_obj_entry;


	char           *proc = "secsc_delete";

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	secsc_errno = NOERR;
	sca_rc = 0;
	sct_id = sc_sel.sct_id;


	if (pse_sel->object.name && strlen(pse_sel->object.name)) {

		/*
		 * Delete object on the SC
		 */

		/* get information about the object to be deleted */
		sc_obj_entry = aux_AppObjName2SCObj(pse_sel->app_name, pse_sel->object.name);
		if (sc_obj_entry == NULLSCOBJENTRY) {
			aux_add_error(EINVALID, "Cannot get configuration info for SC-object to be deleted",  (char *) pse_sel, PSESel_n, proc);
			return (-1);
		}
		if (sc_obj_entry->type == SC_KEY_TYPE) {

			/*
			 * Deletion of a key on the SC is not supported
			 */
			aux_add_error(ESCNOTSUPP, "Deletion of key on SC not supported", (char *) pse_sel, PSESel_n, proc);
			return (-1);
		}


		/*
		 *  For deletion an object the user authentication is required.
		 */	

		if (user_authentication(sct_id, pse_sel->app_name, PIN)) {
			aux_add_error(LASTERROR, "User (PIN) authentication (prior to deletion of an SC-object) failed", (char *) pse_sel, PSESel_n, proc);
			return (-1);
		}


		/*
		 * Object to be deleted is a file
		 */

		/* set security mode for SCA-function */
		if (set_fct_sec_mode(sct_id, "sca_delete_file", &sm_SC)) {
			aux_add_error(LASTERROR, "Set security mode (prior to deletion of an SC-object) failed", (char *) pse_sel, PSESel_n, proc);
			return (-1);
		}
		/* Delete WEF on the SC */
		file_sel.file_id = &file_id;
		get_SC_fileid(sc_obj_entry->sc_id, file_sel.file_id);

		sca_rc = sca_delete_file(sct_id,
					 file_cat = EF,
					 &file_sel,
					 &sm_SC);
		if (sca_rc < 0) {
			secsc_errno = analyse_sca_err(sct_id, "sca_delete_file");
			aux_add_error(secsc_errno, "Deletion of an SC-object failed", (char *) pse_sel, PSESel_n, proc);
			return (-1);
		}
	}
	 /* end if */ 
	else {

		/*
		 * Deletion of an application on the SC is not supported
		 */

		aux_add_error(ESCNOTSUPP, "Deletion of app on SC not supported", (char *) pse_sel, PSESel_n, proc);
		return (-1);
	}			/* end else */

	return (0);

}				/* end secsc_delete() */







/*--------------------------------------------------------------*/
/*						                */
/* PROC  secsc_write					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Write octetstring into WEF on the SC:			*/
/*								*/
/*  Assumption: the belonging application has been opened by 	*/
/*  the calling routine.					*/
/*								*/
/*     1) If object_name missing   				*/
/*	  => return(error)					*/
/*     else:							*/
/*     2) Get parameters for the object to be written from the	*/
/*        global variable "sc_app_list[].sc_obj_list[]".	*/
/*        Case 1: Object on the SC is a key			*/
/*		  => return(error)				*/
/*	  Case 2: Object on the SC is a file:			*/
/*     3) Check whether no. of octets to be written fits to the */
/*        size of the file.					*/
/*     4) The user authentication is performed.			*/
/*     5) Select WEF on smartcard				*/
/*			   - "sc_obj_list" delivers:		*/
/*			      - short index of the WEF on the SC*/
/*			      - sec_mess parameter		*/
/*     6) Parameters for "Write into WEF on SC":		*/
/*			   - "sc_obj_list" delivers:		*/
/*			      - sec_mess parameter		*/
/*								*/
/*  The function "sca_write_binary()" is called two times.	*/
/*  1. Store length:						*/
/*  The length of the data to be written is stored in the first */
/*  bytes of the WEF on the SC.	The no. of bytes which are used	*/
/*  for the length are specified in WEF_LEN_BYTES (constant).	*/
/*								*/
/*  2. Store the data:						*/
/*  In case of an error (e.g. EWRONG_OFFSET) the length of the  */
/*  relevant data is set to 0 (write the value 0 into the first */
/*  octets of the file.						*/
/*								*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   pse_sel	 	       					*/
/*   content		       Data to be written.		*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0			       o.k.				*/
/*   -1			       Error				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sca_select_file()		Select file on the smartcard.   */
/*   sca_write_binary()		Write data in transparent WEF   */
/*				on the SC.	    	        */
/*   aux_AppObjName2SCObj()	Get information about an SC     */
/*        			object belonging to an 		*/
/*                              application on SC. 		*/
/*   itos()			Transform integer to char-string*/
/*   prepare_sec_mess()		- Generate session key,	    	*/
/*				- Set security mode for the 	*/
/*				  communication between DTE/SCT.*/
/*   user_authentication ()     Perform user authentication	*/
/*				(PIN).				*/
/*                                                              */
/*   analyse_sca_err()		Analyse of an error-number 	*/
/*				returned by an SCA-IF function. */
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure secsc_write
 *
 ***************************************************************/
#ifdef __STDC__

RC secsc_write(
	PSESel		 *pse_sel,
	OctetString	 *content
)

#else

RC secsc_write(
	pse_sel,
	content
)
PSESel		 *pse_sel;
OctetString	 *content;

#endif

{

	/* Variables for the SCA-IF */
	unsigned int    sct_id;
	FileCat		file_cat;
	FileSel         file_sel;
	FileId          file_id;
	FileInfoReq     file_info_req;
	FileInfo        file_info;	/* return parameter of
					 * sca_select_file, */
	AddInfo		addinfo;
	unsigned int	offset;
	OctetString     out_data;
	ICC_SecMess     sm_SC;	/* sec. mode for communication SCT/SC	 */
	SCT_SecMess     sm_SCT;	/* sec. mode for communication DTE/SCT	 */
	unsigned int    no_of_written_octets;



	/* Variables for internal use */
	SCObjEntry     *sc_obj_entry;
	char            WEF_len[WEF_LEN_BYTES];	/* The length of the data to be
						 * written is stored in the first
						 * bytes of the WEF on the SC.	
				 		 */
	char		err_msg[256];


	char           *proc = "secsc_write";

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	secsc_errno = NOERR;
	sca_rc = 0;
	sct_id = sc_sel.sct_id;



	if ((!pse_sel->object.name) || (!strlen(pse_sel->object.name))) {
		aux_add_error(EINVALID, "Object name missing", (char *) pse_sel, PSESel_n, proc);
		return (-1);
	}

	/* get information about the object to be written */
	sc_obj_entry = aux_AppObjName2SCObj(pse_sel->app_name, pse_sel->object.name);
	if (sc_obj_entry == NULLSCOBJENTRY) {
		aux_add_error(EINVALID, "Cannot get configuration info for SC-object to be written", pse_sel->object.name, char_n, proc);
		return (-1);
	}
	if (sc_obj_entry->type == SC_KEY_TYPE) {
		/* object is a key */
		aux_add_error(ESCNOTSUPP, "SC-Key cannot be written", pse_sel->object.name, char_n, proc);
		return (-1);
	}


	/* 
	 *    Check whether no. of octets to be written fits to the size of the file.
	 */
	
	if (content->noctets + WEF_LEN_BYTES > sc_obj_entry->size) {
		sprintf(err_msg, "Size of file is %d, required size of file is %d (incl. length field)!", sc_obj_entry->size , content->noctets + WEF_LEN_BYTES);
		aux_add_error(EINVALID, "File on SC is too small for data", (char *) err_msg, char_n, proc);
		return (-1);
	}
	

	/*
	 *  Intermediate result:   Object to be written is a file, size of 
	 *			   file is sufficient for the data.
	 *  Next to do:            - Perform the user authentication.
	 *			   - Select WEF on the smartcard.
	 */	

	if (user_authentication(sct_id, pse_sel->app_name, PIN)) {
		aux_add_error(LASTERROR, "User (PIN) authentication (prior to write into an SC-object) failed", (char *) pse_sel->object.name, char_n, proc);
		return (-1);
	}		

	/* set security mode for SCA-function */
	if (set_fct_sec_mode(sct_id, "sca_select_file", &sm_SC)) {
		aux_add_error(LASTERROR, "Set security mode (prior to select an SC-object) failed", (char *) pse_sel->object.name, char_n, proc);
		return (-1);
	}

	file_sel.file_id = &file_id;
	get_SC_fileid(sc_obj_entry->sc_id, file_sel.file_id);

	file_info.addinfo = &addinfo;
	/* select application on the SC */
	sca_rc = sca_select_file(sct_id,
				 file_cat = EF,
				 &file_sel,
				 file_info_req = NONE_INFO,
				 &file_info,
				 &sm_SC	/* sec_mode for SCT/SC */
				);
	if (sca_rc < 0) {
		secsc_errno = analyse_sca_err(sct_id, "sca_select_file");
		aux_add_error(secsc_errno, "Cannot select SC-object to be written ", (char *) pse_sel->object.name, char_n, proc);
		return (-1);
	}


	/*
	 *  Intermediate result:   WEF on the smartcard has been selected.
	 *  Next to do:            - Set security mode for the communication DTE-SCT
	 *			     according to the parameters of the object to be
	 *			     written.
	 *			   - First write the length of the data to be written
	 *			     into the first octets of the file.
	 *			   - Write the data itself.
	 */	

	
	/* set security mode for writing an SC-object */
	sm_SCT.command = sc_obj_entry->sm_SCT;
	sm_SCT.response = SCT_SEC_NORMAL;
	if (prepare_sec_mess(sct_id, &sm_SCT, FALSE)) {
		aux_add_error(LASTERROR, "Prepare secure messaging (prior to write an SC-object) failed", (char *) pse_sel->object.name, char_n, proc);
		return (-1);
	}

	/* get length of object as char-string */
	if (itos(content->noctets, &WEF_len[0], WEF_LEN_BYTES)) {
		aux_add_error(EINVALID, "Internal error (get WEF length)", CNULL, 0, proc);
		return (-1);
	}

	/* first write length of object into WEF on SC */
	out_data.noctets = WEF_LEN_BYTES;
	out_data.octets = &WEF_len[0];
	offset = 0;
#ifdef SECSCTEST
	fprintf(stderr, "\n\nWritten length:\n");
	aux_fxdump(stderr, out_data.octets, out_data.noctets, 0);
	fprintf(stderr, "\n");
#endif

	sca_rc = sca_write_binary(sct_id,
				  0,
				  offset,
				  &out_data,
				  &sc_obj_entry->sm_SC_write,
			          &no_of_written_octets
				  );

	if ((sca_rc < 0) || (no_of_written_octets != WEF_LEN_BYTES)) {
		secsc_errno = analyse_sca_err(sct_id, "sca_write_binary");
		if (sca_errno == EWRONG_OFFSET) {
			sprintf(err_msg, "Write error: Not enough space in file (Required size of file is %d)", content->noctets + WEF_LEN_BYTES);
			aux_add_error(secsc_errno, "Write to SC-object failed", (char *) err_msg, char_n, proc);
		        /* set number of relevant octets in the WEF to 0 */
			/* get length (= 0) of object as char-string */
			if (itos(0, &WEF_len[0], WEF_LEN_BYTES)) {
				aux_add_error(EINVALID, "Internal error (get WEF length)", CNULL, 0, proc);
				return (-1);
			}
			out_data.noctets = WEF_LEN_BYTES;
			out_data.octets = &WEF_len[0];
			offset = 0;
			sca_rc = sca_write_binary(sct_id,
				  		  0,
					          offset,
					          &out_data,
					          &sc_obj_entry->sm_SC_write,
					          &no_of_written_octets);
		}
		else	
			aux_add_error(secsc_errno, "Write to SC-object failed", (char *)  pse_sel->object.name, char_n, proc);
		return (-1);
	}


#ifdef SECSCTEST
	fprintf(stderr, "\n\nWritten data:\n");
	aux_fxdump(stderr, content->octets, content->noctets, 0);
	fprintf(stderr, "\n");
#endif
	/* write the data itself */
	offset = WEF_LEN_BYTES;
	sca_rc = sca_write_binary(sct_id,
				  0,
				  offset,
				  content,
			          &sc_obj_entry->sm_SC_write,
				  &no_of_written_octets);

	if ((sca_rc < 0) || (no_of_written_octets != content->noctets)) {
		secsc_errno = analyse_sca_err(sct_id, "sca_write_binary");
		if (sca_errno == EWRONG_OFFSET) {
			sprintf(err_msg, "Write error: Not enough space in file (Required size of file is %d)", content->noctets + 2);
			aux_add_error(secsc_errno, "Write to SC-object failed", (char *) err_msg, char_n, proc);
		        /* set number of relevant octets in the WEF to 0 */
			/* get length (= 0) of object as char-string */
			if (itos(0, &WEF_len[0], WEF_LEN_BYTES)) {
				aux_add_error(EINVALID, "Internal error (get WEF length)", CNULL, 0, proc);
				return (-1);
			}
			out_data.noctets = WEF_LEN_BYTES;
			out_data.octets = &WEF_len[0];
			offset = 0;
			sca_rc = sca_write_binary(sct_id,
				 		  0,
					          offset,
					          &out_data,
					          &sc_obj_entry->sm_SC_write,
					          &no_of_written_octets);
		}
		else	
			aux_add_error(secsc_errno, "Write to SC-object failed", (char *) pse_sel->object.name, char_n, proc);
		return (-1);
	}

	return (0);


}				/* end secsc_write() */





/*--------------------------------------------------------------*/
/*						                */
/* PROC  secsc_read					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Read data from WEF on the SC into octetstring:		*/
/*								*/
/*  Assumption: the belonging application has been opened by 	*/
/*  the calling routine.					*/
/*								*/
/*     1) If object_name missing   				*/
/*	  => return(error)					*/
/*     else:							*/
/*     2) Get parameters for the object to be written from the	*/
/*        global variable "sc_app_list[].sc_obj_list[]".	*/
/*        Case 1: Object on the SC is a key			*/
/*		  => return(error)				*/
/*	  Case 2: Object on the SC is a file:			*/
/*     3) Select WEF on smartcard				*/
/*			   - "sc_obj_list" delivers:		*/
/*			      - short index of the WEF on the SC*/
/*			      - sec_mess parameter		*/
/*     4) Parameters for "Read from WEF on SC":			*/
/*			   - "sc_obj_list" delivers:		*/
/*			      - sec_mess parameter		*/
/*								*/
/*								*/
/*  The function "sca_read_binary()" is called two times.	*/
/*  1. Read length:						*/
/*  The assumption is that the length of the WEF (no. of 	*/
/*  relevant data) is stored in the first bytes of the WEF. The */
/*  no. of bytes which are used	for this length are specified   */
/*  in WEF_LEN_BYTES (constant).				*/
/*  If the length of the file is 0, "content->noctets" is set   */
/*  0 and 0 is returned.					*/
/*								*/
/*  2. Read data:						*/
/*  According to the read number of relevant data the data are	*/
/*  read from the file.						*/
/*  The read data are returned in content->octets. 		*/
/*  Secsc_read provides content->noctets and allocates the      */
/*  necessary memory in content->octets.			*/
/* 								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   pse_sel	 	       					*/
/*							       	*/
/* OUT							       	*/
/*   content		       Data read from the SC.		*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0			       o.k.				*/
/*   -1			       Error				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sca_select_file()		Select file on the smartcard.   */
/*   sca_read_binary()		Read data from transparent 	*/
/*				WEF on the smartcard.   	*/
/*   aux_AppObjName2SCObj()	Get information about an SC     */
/*        			object belonging to an 		*/
/*                              application on SC. 		*/
/*   prepare_sec_mess()		- Generate session key,	    	*/
/*				- Set security mode for the 	*/
/*				  communication between DTE/SCT.*/
/*   stoi()			Transform char-string to integer*/
/*   user_authentication()      Perform user authentication     */
/*				(PIN).				*/
/*                                                              */
/*   analyse_sca_err()		Analyse of an error-number 	*/
/*				returned by an SCA-IF function. */
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure secsc_read
 *
 ***************************************************************/
#ifdef __STDC__

RC secsc_read(
	PSESel		 *pse_sel,
	OctetString	 *content
)

#else

RC secsc_read(
	pse_sel,
	content
)
PSESel		 *pse_sel;
OctetString	 *content;

#endif

{

	/* Variables for the SCA-IF */
	unsigned int    sct_id;
	FileCat		file_cat;
	FileSel         file_sel;
	FileId          file_id;
	FileInfoReq     file_info_req;
	FileInfo        file_info;	/* return parameter of
					 * sca_select_file, */
	AddInfo		addinfo;
	int             data_length;
	unsigned int	offset;
	OctetString     in_data;
	SCT_SecMess     sm_SCT;	/* sec. mode for communication DTE/SCT	 */
	ICC_SecMess     sm_SC;	/* sec. mode for communication SCT/SC	 */
	unsigned int    WEF_len;/* The length of the data to be read     */
				/* is stored in the first bytes of the   */
				/* WEF on the SC.	 		 */


	/* Variables for internal use */
	SCObjEntry     *sc_obj_entry;
	int		repeat_times;

	char           *proc = "secsc_read";

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	secsc_errno = NOERR;
	sca_rc = 0;
	sct_id = sc_sel.sct_id;


	if ((!pse_sel->object.name) || (!strlen(pse_sel->object.name))) {
		aux_add_error(EINVALID, "Object name missing", (char *) pse_sel, PSESel_n, proc);
		return (-1);
	}

	/* get information about the object to be read */
	sc_obj_entry = aux_AppObjName2SCObj(pse_sel->app_name, pse_sel->object.name);
	if (sc_obj_entry == NULLSCOBJENTRY) {
		aux_add_error(EINVALID, "Cannot get configuration info for SC-object to be read", pse_sel->object.name, char_n, proc);
		return (-1);
	}
	if (sc_obj_entry->type == SC_KEY_TYPE) {
		/* object is a key */
		aux_add_error(ESCNOTSUPP, "SC-Key cannot be read", pse_sel->object.name, char_n, proc);
		return (-1);
	}


	/*
	 *  Intermediate result:   Object to be read is a file
	 *  Next to do:            - Select WEF on the smartcard.
	 */	

	/* set security mode for SCA-function */
	if (set_fct_sec_mode(sct_id, "sca_select_file", &sm_SC)) {
		aux_add_error(LASTERROR, "Set security mode (prior to select an SC-object) failed", (char *) pse_sel->object.name, char_n, proc);
		return (-1);
	}

	file_sel.file_id = &file_id;
	get_SC_fileid(sc_obj_entry->sc_id, file_sel.file_id);

	/* select application on the SC */
	file_info.addinfo = &addinfo;
	sca_rc = sca_select_file(sct_id,
				 file_cat = EF,
				 &file_sel,
				 file_info_req = NONE_INFO,
				 &file_info,
				 &sm_SC	/* sec_mode for SCT/SC */
				);
	if (sca_rc < 0) {
		secsc_errno = analyse_sca_err(sct_id, "sca_select_file");
		aux_add_error(secsc_errno, "Cannot select SC-object to be read ", (char *) pse_sel->object.name, char_n, proc);
		return (-1);
	}


	/*
	 *  Intermediate result:   WEF on the smartcard has been selected.
	 *  Next to do:            - Set security mode for the communication DTE-SCT
	 *			     according to the parameters of the object to be
	 *			     read.
	 *			   - First read the length of the data to be read
	 *			     from the first octets of the file.
	 *			   - Read the data itself.
	 */	


	/* set security mode for reading an SC-object */
	sm_SCT.command = SCT_SEC_NORMAL;
	sm_SCT.response = sc_obj_entry->sm_SCT;
	if (prepare_sec_mess(sct_id, &sm_SCT, FALSE)) {
		aux_add_error(LASTERROR, "Prepare secure messaging (prior to read an SC-object) failed", (char *) pse_sel->object.name, char_n, proc);
		return (-1);
	}

	/* first read length of object from WEF on SC */
	offset = 0;
	data_length = WEF_LEN_BYTES;
	repeat_times = 0;

repeat_read:
	sca_rc = sca_read_binary(sct_id,
				 0,
			         offset,
			         data_length,
			         &in_data,
			         &sc_obj_entry->sm_SC_read);
	if (sca_rc < 0) {

		if (sca_errno == ECMD_NOT_ALLOWED) {
			/* user authentication required. */
			if (user_authentication(sct_id, pse_sel->app_name, PIN)) {
				aux_add_error(LASTERROR, "User (PIN) authentication (prior to read from an SC-object) failed", (char *) pse_sel->object.name, char_n, proc);
				return (-1);
			}
			if (repeat_times == 0) {
				/* repeat read data from SC */
				repeat_times++;
				goto repeat_read;
			}
			else {
				aux_add_error(ESC, "Read SC-object not allowed in actual state of SC.", (char *) pse_sel->object.name, char_n, proc);
				return (-1);
			}		
		}
		else {
			secsc_errno = analyse_sca_err(sct_id, "sca_read_binary");
			aux_add_error(secsc_errno, "Read of SC-object failed", (char *) pse_sel->object.name, char_n, proc);
			return (-1);
		}
	}

#ifdef SECSCTEST
	fprintf(stderr, "\n\nRead length:\n");
	aux_fxdump(stderr, in_data.octets, in_data.noctets, 0);
	fprintf(stderr, "\n");
#endif

	/* get length of object as integer */
	if (stoi(in_data.octets, &WEF_len, WEF_LEN_BYTES)) {
		aux_add_error(EINVALID, "Internal error (get WEF length)", CNULL, 0, proc);
		return (-1);
	}
	aux_free2_OctetString(&in_data);


	/*
	 * Read octetstring from WEF on SC according to the read length
	 */

	if (!(content)) {
		aux_add_error(EINVALID, "invalid input value (content)", CNULL, 0, proc);
		return (-1);
	}
	
	/* if length of file is 0, file is empty */
	if (WEF_len == 0) {
		content->noctets = 0;
		content->octets = (char *)0;
		return (0);
	}

	offset = WEF_LEN_BYTES;
	data_length = WEF_len;

	sca_rc = sca_read_binary(sct_id,
				 0,
				 offset,
				 data_length,
				 content,
				 &sc_obj_entry->sm_SC_read);
	if (sca_rc < 0) {
		secsc_errno = analyse_sca_err(sct_id, "sca_read_binary");
		aux_add_error(secsc_errno, "Read of SC-object failed", (char *) pse_sel->object.name, char_n, proc);
		return (-1);
	}
#ifdef SECSCTEST
	fprintf(stderr, "\n\nRead data:\n");
	aux_fxdump(stderr, content->octets, content->noctets, 0);
	fprintf(stderr, "\n");
#endif


	return (0);


}				/* end secsc_read() */





/*--------------------------------------------------------------*/
/*						                */
/* PROC  secsc_chpin					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  a) Change PIN for object on SC:				*/
/*     In this case the error ESCNOTSUPP is returned.		*/
/*     This function is not supported.				*/
/*								*/
/*  b) Change PIN for an application on the SC:			*/
/*     1) If the belonging application is not open, it will	*/
/*	  be opened.						*/
/*     2) Get parameters for the object SC_PIN from the		*/
/*        global variable "sc_app_list[].sc_obj_list[]".	*/
/*        - SC_PIN is an SC-object, which is determined by an 	*/
/*          entry in "sc_obj_list[]".				*/
/*     3) If user enters an incorrect PIN, the PIN-change       */
/*        is repeated two times.				*/
/*								*/
/*  If the user authentication was successful, parameter	*/
/*  "user_auth_done" in "sct_stat_list[sct_id]" is set to TRUE	*/
/*								*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   pse_sel	 	       					*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0			       o.k.				*/
/*   -1			       Error				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sca_change_pin()		Change PIN on the smartcard.    */
/*   sca_display()		Display text on SCT-display.    */
/*							       	*/
/*   aux_AppObjName2SCObj()	Get information about an SC     */
/*        			object belonging to an 		*/
/*                              application on SC. 		*/
/*   bell_function()		"Ring the bell" to require user */
/*                              input at the SCT.		*/
/*   get_SC_keyid()		Compose structure KeyId (for a  */
/*				key on the SC).			*/
/*   handle_SC_app()		If application not open, open it*/
/*   prepare_sec_mess()		- Generate session key,	    	*/
/*				- Set security mode for the 	*/
/*				  communication between DTE/SCT.*/
/*   write_SCT_config()		Encrypt and write SCT configuration   */
/*				data for the specified SCT.	*/
/*   analyse_sca_err()		Analyse of an error-number 	*/
/*				returned by an SCA-IF function. */
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure secsc_chpin
 *
 ***************************************************************/
#ifdef __STDC__

RC secsc_chpin(
	PSESel	 *pse_sel
)

#else

RC secsc_chpin(
	pse_sel
)
PSESel	 *pse_sel;

#endif

{

	/* Variables for the SCA-IF */
	unsigned int    sct_id;
	Boolean		SC_internal;
	KeyId           key_id;
	ICC_SecMess     sm_SC;	/* sec. mode for communication SCT/SC	 */
	SCT_SecMess     sm_SCT;	/* sec. mode for communication DTE/SCT	 */
	int             time_out;
	char           *display_text;
	int             chgpin_rc;
	int             chgpin_errno;
	char           *chgpin_errmsg;
	int             chgpin_attempts = 0;	/* no. of attempts to change
						 * the PIN	 */


	/* Variables for internal use */
	SCObjEntry     *sc_obj_entry;



	char           *proc = "secsc_chpin";

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	secsc_errno = NOERR;
	sca_rc = 0;
	sct_id = sc_sel.sct_id;


	if (pse_sel->object.name && strlen(pse_sel->object.name)) {

		/*
		 * Change PIN for an object (WEF, key) on the SC is not
		 * supported
		 */
		aux_add_error(ESCNOTSUPP, "Change PIN for object not supported", pse_sel->object.name, char_n, proc);
		return (-1);

	}
	 /* end if */ 
	else {

		/*
		 * Change PIN for the application on the SC
		 */

		/* If belonging application not open, open it */
		if (handle_SC_app(pse_sel->app_name)) {
			AUX_ADD_ERROR;
			return (-1);
		}
		/* get information about the object SC_PIN */
		sc_obj_entry = aux_AppObjName2SCObj(pse_sel->app_name, SC_PIN_name);
		if (sc_obj_entry == NULLSCOBJENTRY) {
			aux_add_error(EINVALID, "Cannot get configuration info for SC-object", SC_PIN_name, char_n, proc);
			return (-1);
		}
		if (sc_obj_entry->type != SC_KEY_TYPE) {

			/*
			 * SC_PIN has to be a key on the SC
			 */
			aux_add_error(EINVALID, "SC_PIN has to be a key on the SC", CNULL, 0, proc);
			return (-1);
		}

		/*
		 * set security mode for writing an SC-object (object SC_PIN
		 * is written from the SCT to the SC
		 */
		sm_SCT.command = sc_obj_entry->sm_SCT;
		sm_SCT.response = SCT_SEC_NORMAL;
		if (prepare_sec_mess(sct_id, &sm_SCT, FALSE)) {
			aux_add_error(LASTERROR, "Prepare secure messaging (prior to change PIN) failed", (char *) pse_sel, PSESel_n, proc);
			return (-1);
		}
		get_SC_keyid(sc_obj_entry->sc_id, SC_internal = TRUE, &key_id);


		/*
		 * change PIN  (if user enters invalid PIN / new PIN, repeat
		 * 2 times)
		 */
		do {
			bell_function();
			chgpin_rc = sca_change_pin(sct_id,
						   &key_id,
						   &sc_obj_entry->sm_SC_write);
			chgpin_errno = sca_errno;
			chgpin_errmsg = sca_errmsg;
			chgpin_attempts++;

			if (chgpin_rc < 0) {
				switch (sca_errno) {

				case ELAST_ATTEMPT:
				case EKEY_LOCKED:
					aux_add_error(ESCPINLOCK, "sca_change_pin", (char *) (char *) sca_errmsg, char_n, proc);

					/* display message on SCT-Display */
					sca_rc = sca_display(sct_id,
							     display_text = SCT_TEXT_PIN_LOCKED,
							     time_out = 0);
					if (sca_rc < 0) {
						fprintf(stderr, "Cannot display error text: %s on SCT display\n", SCT_TEXT_PIN_LOCKED);
						return (-1);
					}
					return (-1);	/* PIN on SC is locked */
					break;

				case EAUTH_1ATTEMPT:
					fprintf(stderr, "\n\nAuthentication failed; 1 attempt left\n\n");
					break;
				case EAUTH_2ATTEMPT:
					fprintf(stderr, "\n\nAuthentication failed; 2 attempts left\n\n");
					break;
				case EAUTH_3ATTEMPT:
					fprintf(stderr, "\n\nAuthentication failed; 3 attempts left\n\n");
					break;
				case EAUTH_4ATTEMPT:
					fprintf(stderr, "\n\nAuthentication failed; 4 attempts left\n\n");
					break;
				case EAUTH_5ATTEMPT:
					fprintf(stderr, "\n\nAuthentication failed; 5 attempts left\n\n");
					break;
				case EAUTH_6ATTEMPT:
					fprintf(stderr, "\n\nAuthentication failed; 6 attempts left\n\n");
					break;
				case EAUTH_7ATTEMPT:
					fprintf(stderr, "\n\nAuthentication failed; 7 or more attempts left\n\n");
					sca_rc = sca_display(sct_id,
							     display_text = SCT_TEXT_PIN_INVALID,
							     time_out = 0);
					if (sca_rc < 0) {
						fprintf(stderr, "Cannot display error text: %s on SCT display\n", SCT_TEXT_PIN_INVALID);
						return (-1);
					}
					break;

				case EINV_LEN:
					sca_rc = sca_display(sct_id,
							     display_text = SCT_TEXT_NEW_PIN_INV,
							     time_out = 0);
					if (sca_rc < 0) {
						fprintf(stderr, "Cannot display error text: %s on SCT display\n", SCT_TEXT_NEW_PIN_INV);
						return (-1);
					}
					break;


				case ECPIN:
					sca_rc = sca_display(sct_id,
						  	     display_text = SCT_TEXT_NEW_PIN_CPIN,
						  	     time_out = 0);
					if (sca_rc < 0) {
						fprintf(stderr, "Cannot display error text: %s on SCT display\n", SCT_TEXT_NEW_PIN_CPIN);
						return (-1);
					}
					break;

				default:
					secsc_errno = analyse_sca_err(sct_id, "sca_change_pin");
					aux_add_error(secsc_errno, "Change PIN for SC-application failed", (char *)  pse_sel, PSESel_n, proc);
					return (-1);

				}	/* end switch */
			}
			 /* end if */ 
			else {
				/* PIN change was successful */
/*??????????????????????????????????????????????????????????????????????????????????*/
/*				Da ein erfolgreiches change PIN auch gleichzeitig ein 
				verify PIN bewirkt, koennte hier auch 
				sct_stat_list[].user_auth_done auf TRUE gesetzt sein.
				Dazu muss aber erst der Zustandsautomat entsprechend
				geaendert werden.
			        sct_stat_list[sct_id].user_auth_done = TRUE;
*/
/*??????????????????????????????????????????????????????????????????????????????????*/

				/* change PIN was successful => PIN authentication successful*/

				sct_stat_list[sct_id].user_auth_done = TRUE;

				/*
				 *  Save the changed SCT configuration data
				 */

				if ((write_SCT_config(sct_id)) < 0) {
					AUX_ADD_ERROR;
					return (-1);
				}

				return (0);
			}

		}		/* end do */
		while (chgpin_attempts < MAX_PIN_FAIL);

		/* after 3 unsuccessful attempts: PIN change fails */
		sca_errno = chgpin_errno;
		sca_errmsg = chgpin_errmsg;
		secsc_errno = analyse_sca_err(sct_id, "sca_change_pin");
		aux_add_error(secsc_errno, "Change PIN for SC-application failed", (char *)  pse_sel, PSESel_n, proc);
		return (-1);
	}			/* end else */


}				/* end secsc_chpin() */






/*--------------------------------------------------------------*/
/*						                */
/* PROC  secsc_gen_key					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*   Generate and install key on SCT/SC:			*/
/*								*/
/*   1) Algorithm:						*/
/*      If key->key->subjectAI != NULL				*/
/*	   => take this algorithm				*/
/*      else take algorithm specified in key->alg.		*/
/*								*/
/*   2) Handling of new and old decryption key:			*/
/*								*/			
/*      a) If "key->pse_sel->object.name" = "DecSKold", an      */
/*         error is returned.					*/
/*      b) If "key->pse_sel->object.name" = "DecSKnew" and 	*/ 
/*         the object "DecSKold" is an object on the SC, 	*/
/*         => the generated key is installed under the name of 	*/
/*            the oldest decryption key stored on the SC. 	*/
/*      c) If "key->pse_sel->object.name" = "DecSKnew" and 	*/ 
/*         the object "DecSKold" is not an object on the SC, 	*/
/*         => the generated key is installed under the name of 	*/
/*            "DecSKnew".					*/
/*								*/
/*      The objects "SKnew" and "SKold" are treated in the same	*/
/*	way.							*/
/*								*/
/*								*/
/*   3) Get key_id from key (keyref or pse_sel).		*/
/*  								*/
/*   4) Handle SC application for the key, if key = key on SC.  */
/*      (function handle_key_sc_app)				*/
/*								*/
/*--------------------------------------------------------------*/
/*								*/
/*   5) Generate the key(s) in the SCT with function 		*/
/*      "sca_gen_user_key()".					*/
/*								*/
/*   6) If the key has to be installed on the SC:		*/
/*	 1. Check algorithm, only the installation of RSA- and 	*/
/*          DES-keys is allowed.				*/
/*	 2. Set the attribute list for the key			*/
/*       3. Call SCA-IF function "sca_inst_user_key()"		*/
/*	 4. Delete the remaining copy of the key within the SCT.*/   
/*	 5. Update SC Toc					*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   key	 	       selection of the key 		*/
/*   replace		       = FALSE => Install key		*/
/*   			       = TRUE  => Replace key		*/
/*							       	*/
/* OUT							       	*/
/*   key->key->subjectkey      In case of RSA the public key is */
/*			       returned.			*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0			       o.k.				*/
/*   -1			       Error				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sca_del_user_key()		Delete user key in an SCT.	*/
/*   sca_gen_user_key()		Generate user key in the SCT.   */
/*   sca_inst_user_key()	Install user key on the SC.     */
/*								*/
/*   aux_ObjId2AlgEnc()		Map object identifier on 	*/
/*				algorithm encryption method.	*/
/*   e2_KeyBits()		Encode given structure KeyBits  */
/*				into a given structure BitString*/
/*   handle_gen_DecSK()		Special handling of new and 	*/
/*				old decryption key.		*/
/*   handle_key_sc_app()	Handle SC-application for the   */
/*                              selected key.			*/
/*   key_to_keyid()	        Get key_id from key.            */
/*   set_fct_sec_mode()		Set security mode for comm.     */
/*				between DTE/SCT depending on the*/
/*                	        SCA-function to be called.      */
/*   update_SCToc()		Update entry in SCToc.	        */
/*                                                              */
/*   analyse_sca_err()		Analyse of an error-number 	*/
/*				returned by an SCA-IF function. */
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure secsc_gen_key
 *
 ***************************************************************/
#ifdef __STDC__

RC secsc_gen_key(
	Key	 *key,
	Boolean	  replace
)

#else

RC secsc_gen_key(
	key,
	replace
)
Key	 *key;
Boolean	  replace;

#endif

{

	/* Variables for the SCA-IF */
	unsigned int    sct_id;
	KeySel          key_sel;
	KeyId           key_id;
	KeyBits         key_bits;
	KeyAttrList     key_attr_list;
	ICC_SecMess     sm_SC;	/* sec. mode for communication SCT/SC	 */


	/* Variables for internal use */
	char           *proc = "secsc_gen_key";
	char	       *new_DecSK_name;
	Boolean		internal_replace;
	char	       *save_obj_name;
	Boolean	       	special_DecSK_selection;
	Key	       	DecSK_key;
	PSESel	       	DecSK_pse_sel;
	AlgEnc	       	algenc;
	int		repeat_times;
	Boolean		inst_key_failed = FALSE;

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	secsc_errno = NOERR;
	sca_rc = 0;
	sct_id = sc_sel.sct_id;
	internal_replace = replace;


	/*
	 *  Get algorithm of key to be generated
	 */

	if ((key->key) && (key->key->subjectAI))
		key_sel.key_algid = key->key->subjectAI;
	else if (key->alg)
		key_sel.key_algid = key->alg;
	else {
		aux_add_error(EALGID, "Algorithm for key generation missing", CNULL, 0, proc);
		return (-1);
	}

	key_sel.key_id = &key_id;

	/*
	 *  Handling of new and old decryption key:
	 */

	if (handle_gen_DecSK(key, replace, &new_DecSK_name, &internal_replace)) {
		AUX_ADD_ERROR;
		return (-1);
	}

	if (new_DecSK_name) {

		/*
		 *  Object name of DecSK has been mapped on a new name
		 *	get keyid for this name.
		 */

		DecSK_key.pse_sel = &DecSK_pse_sel;
		DecSK_key.pse_sel->app_name     =  key->pse_sel->app_name;
		DecSK_key.pse_sel->object.name  =  new_DecSK_name;
		DecSK_key.keyref = 0;
		DecSK_key.key = ((KeyInfo *) 0);
		DecSK_key.alg = key_sel.key_algid;

		if (key_to_keyid(&DecSK_key, key_sel.key_algid, key_sel.key_id, special_DecSK_selection = FALSE)) {
			AUX_ADD_ERROR;
			strzfree(&new_DecSK_name);
			return (-1);
		}
	}
	else {
		if (key_to_keyid(key, key_sel.key_algid, key_sel.key_id, special_DecSK_selection = FALSE)) {
			AUX_ADD_ERROR;
			return (-1);
		}
	}

	/*
	 * Handle SC application for key:
	 */

	if (handle_key_sc_app(sct_id, key, key_sel.key_id)) {
		AUX_ADD_ERROR;
		return (-1);
	}


/*
 *   Intermediate result: 1) Key-id of the key to be generated and installed
 *                           is stored in "key_sel.key_id" !
 * 			  2) If key shall be installed on the SC,
 *			        the belonging application on the SC is open.
 *
 *
 *            Next to do: Generate key in the SCT with function "sca_gen_user_key()".
 */


	key_sel.key_bits = &key_bits;	/* for a returned public RSA key */


	/* set security mode for SCA-function */
	if (set_fct_sec_mode(sct_id, "sca_gen_user_key", &sm_SC)) {
		aux_add_error(LASTERROR, "Set security mode (prior to the generation of an SC-object (key)) failed", CNULL, 0, proc);
		return (-1);
	}

	sca_rc = sca_gen_user_key(sct_id, &key_sel);
	if (sca_rc < 0) {
		secsc_errno = analyse_sca_err(sct_id, "sca_gen_user_key");
		aux_add_error(secsc_errno, "Cannot generate key", (char *) sca_errmsg, char_n, proc);
		return (-1);
	}

/*
 *   Intermediate result: The key/keypair has been generated!
 *
 *            Next to do: If algid = RSA
 *                           Transform public key (structure KeyBits) into
 *                                     public key (structure KeyInfo) and
 *			     return public key in "key->key->subjectkey".
 *
 */

	algenc = aux_ObjId2AlgEnc(key_sel.key_algid->objid);
	if (algenc == RSA) {


		if (!key->key) {
			aux_add_error(EINVALID, "No memory for public RSA key!", CNULL, 0, proc);
			return (-1);

		}
#ifdef SECSCTEST
		fprintf(stderr, "key_sel.key_bits: \n");
		aux_fprint_KeyBits(stderr, key_sel.key_bits);
		fprintf(stderr, " \n");
#endif
		if (e2_KeyBits(key_sel.key_bits, &key->key->subjectkey)) {
			aux_add_error(EENCODE, "Cannot encode public RSAkey (e2_KeyBits)", CNULL, 0, proc);
			return (-1);
		}
#ifdef SECSCTEST
		fprintf(stderr, "&key->key->subjectkey: \n");
		aux_fxdump(stderr, key->key->subjectkey.bits, key->key->subjectkey.nbits / 8, 0);
		fprintf(stderr, " \n");
#endif
		/* release storage for public key (structure KeyBits) */
		aux_free2_KeyBits(key_sel.key_bits);
	}			



/*
 *   Intermediate result: The key/keypair has been generated and is stored in the SCT!
 *
 *            Next to do: If key shall be installed on the SC (level != FILE_DUMMY)
 *			  1. Check algorithm 
 *			     (Only the installation of RSA- and DES-keys is allowed)
 *			  2. Set the attribute list for the key
 *		          3. Call SCA-IF function "sca_inst_user_key"
 *			  4. Update SC Toc
 */


	/*
	 *  key level != FILE_DUMMY means that key shall be stored on the SC 
	 */

	if (key_sel.key_id->key_level != FILE_DUMMY) {

		/*
		 *  Check algorithm 
		 */

		if ((algenc != DES) && (algenc != RSA)) {
			aux_add_error(ESCNOTSUPP, "Algorithm not supported by the smartcard", (char *) key_sel.key_algid, AlgId_n, proc);
			return (-1);
		}

		if (internal_replace == FALSE)
			key_attr_list.key_inst_mode = INST;
		else
			key_attr_list.key_inst_mode = REPL;

		key_attr_list.key_attr.key_repl_flag = TRUE;	/* key is replaceable */
		key_attr_list.key_attr.key_chg_flag = TRUE;	/* key is changeable  */
		key_attr_list.key_attr.key_foreign_flag = TRUE; /* key is replaceable by
								   a key with a different
								   algorithm.         */		
		key_attr_list.key_attr.key_purpose = PURPOSE_CIPHER;
		key_attr_list.key_attr.key_presentation = KEY_LOCAL;
		key_attr_list.key_attr.MAC_length = 4;

		key_attr_list.key_fpc = 0;
		key_attr_list.key_status.PIN_check = FALSE;
		key_attr_list.key_status.key_state = FALSE; 

		/* set security mode for SCA-function */
		if (set_fct_sec_mode(sct_id, "sca_inst_user_key", &sm_SC)) {
			aux_add_error(LASTERROR, "Set security mode (prior to the installation of an SC-object (key)) failed", CNULL, 0, proc);
			return (-1);
		}

/*????????????????????????????????????????????????????????????????????*/
/*		solange SCT beim installieren eines RSA keys auf SC
		zu lange braucht.
*/
/*????????????????????????????????????????????????????????????????????*/
		repeat_times = 0;
repeat_install:
		sca_rc = sca_inst_user_key(sct_id, &key_sel, &key_attr_list);
		if (sca_rc < 0) {
			if ((sca_errno == EPROTRESYNCH) && (repeat_times == 0)) {
				repeat_times++;
				goto repeat_install;				
			}
			else {
				if (((sca_errno == EKEYNOTREPL) || (sca_errno == EINV_ACCESS)) && (key_attr_list.key_inst_mode == INST)) {
					aux_add_error(EINVALID, "Key to be installed exists already!", CNULL, 0, proc);
				}
				else  {
					if (((sca_errno == EKEYUNKNOWN) || (sca_errno == EINV_ACCESS)) && (key_attr_list.key_inst_mode == REPL)) {
						aux_add_error(EINVALID, "Key to be replaced doesn't exist!", CNULL, 0, proc);
					}
					else {
						secsc_errno = analyse_sca_err(sct_id, "sca_inst_user_key");
						aux_add_error(secsc_errno, "Cannot install generated key on the SC", (char *) sca_errmsg, char_n, proc);
					}
				}
				inst_key_failed = TRUE;
			}
		}



		/*
		 *   Intermediate result: It doesn't matter whether the key has been 
		 *			  installed on the SC or not!
		 *			  A copy of the key still remains in the SCT
		 *
		 *            Next to do: Delete this key stored in the SCT.
		 */

	
		/* set security mode for SCA-function */
		if (set_fct_sec_mode(sct_id, "sca_del_user_key", &sm_SC)) {
			aux_add_error(LASTERROR, "Set security mode (prior to delete copy of key  in the SCT) failed", CNULL, 0, proc);
			return (-1);
		}
/*????????????????????????????????????????????????????????????????????*/
/*		solange SCT beim installieren eines RSA keys auf SC
		zu lange braucht.
*/
/*????????????????????????????????????????????????????????????????????*/
		repeat_times = 0;
repeat_delete:
		sca_rc = sca_del_user_key(sct_id,
					  &key_sel);

		if (sca_rc < 0) {
			if ((sca_errno == EPROTRESYNCH) && (repeat_times == 0)) {
				repeat_times++;
				goto repeat_delete;				
			}
			else {
				secsc_errno = analyse_sca_err(sct_id, "sca_del_user_key");
				aux_add_error(secsc_errno, "Cannot delete copy of key in the SCT", (char *) sca_errmsg, char_n, proc);
				return (-1);
			}
		}

		
		/* 
		 *  If the installation of the key on the SC failed, 
		 *     return with an error code
		 */

		if (inst_key_failed == TRUE) 
			return (-1);


		/*
		 *  Update SC Toc
		 */

		if ( (key->pse_sel != (PSESel *) 0) && (key->pse_sel->object.name != CNULL) ) {

			if (new_DecSK_name) {

				/* 
				 *  Update the entry in SCToc for the name
				 *   of the old decryption key. 
				 */

				save_obj_name = key->pse_sel->object.name;
				key->pse_sel->object.name = new_DecSK_name;
#ifdef SECSCTEST
				fprintf(stderr,"Update new_DecSK_name: %s\n", new_DecSK_name);
#endif

				update_SCToc(key->pse_sel, 93, 0);
				key->pse_sel->object.name = save_obj_name;
  				strzfree(&new_DecSK_name);
			}
			else {

#ifdef SECSCTEST
				fprintf(stderr,"Update: %s\n", key->pse_sel->object.name);
#endif
				update_SCToc(key->pse_sel, 93, 0);
			}
		}

#ifdef SECSCTEST
		sec_print_toc(stderr, key->pse_sel);
#endif




	} 


	return (0);


}				/* end secsc_gen_key() */







/*--------------------------------------------------------------*/
/*						                */
/* PROC  secsc_sign					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*   Sign octetstring with key from the SC.			*/
/*								*/
/*   1) Get and check signature algorithm			*/
/*								*/
/*   2) Get key_id from key (keyref or pse_sel).		*/
/*   3) Check key_id:						*/
/*      Level of key_id must be a level on the SC, 		*/
/*      else return(error).					*/
/*   4) Handle SC application for the key, if key = key on SC.  */
/*      (function handle_key_sc_app)				*/
/*								*/
/*--------------------------------------------------------------*/
/*								*/
/*								*/
/*   5) Get parameter for hash-function				*/
/*   6) Call "sca_sign" to sign octetstring with key from SC.	*/
/*      Parameters for "Sign octetstring":			*/
/*			   - "sc_app_list[].sc_obj_list[]" or 	*/
/*			     key_ref delivers:			*/
/*			      - key identifier			*/
/*			   - "sca_fct_list[]" delivers:		*/
/*			      - sec_mess			*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   in_octets		       Data to be signed.		*/
/*   signature							*/
/*   more							*/
/*   key		       Structure which identifies the   */
/*			       signature key.		 	*/
/*   hash_input		       Add. hash-alg specific parameters*/
/*								*/
/*							       	*/
/* OUT							       	*/
/*   signature->signature      Returned signature. Memory is 	*/
/*			       provided by "sca_sign".		*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0			       o.k.				*/
/*   -1			       Error				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sca_sign()			Sign octetstring. 		*/
/*								*/
/*   aux_free_KeyBits()		Release members of struct 	*/
/*				KeyBits and KeyBits.		*/
/*   aux_ObjId2AlgHash()	Map object identifier on 	*/
/*				algorithm hash method.		*/
/*   aux_ObjId2AlgType()	Map object identifier on 	*/
/*				algorithm type.			*/
/*   aux_ObjId2ParmType()	Map object identifier on 	*/
/*				type of parameter.		*/
/*   d_KeyBits()		Decode given BitString into 	*/
/*				structure KeyBits.		*/
/*   handle_key_sc_app()	Handle SC-application for the   */
/*                              selected key.			*/
/*   key_to_keyid()	        Get key_id from key.            */
/*   set_fct_sec_mode()		Set security mode for comm.     */
/*				between DTE/SCT depending on the*/
/*                	        SCA-function to be called.      */
/*                                                              */
/*   analyse_sca_err()		Analyse of an error-number 	*/
/*				returned by an SCA-IF function. */
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure secsc_sign
 *
 ***************************************************************/
#ifdef __STDC__

RC secsc_sign(
	OctetString	 *in_octets,
	Signature	 *signature,
	More		  more,
	Key		 *key,
	HashInput	 *hash_input
)

#else

RC secsc_sign(
	in_octets,
	signature,
	more,
	key,
	hash_input
)
OctetString	 *in_octets;
Signature	 *signature;
More		  more;
Key		 *key;
HashInput	 *hash_input;

#endif

{

	/* Variables for the SCA-IF */
	unsigned int    sct_id;
	static KeySel   key_sel;
	KeyId		key_id;
	HashPar        *hash_par;
	HashPar         hash_par1;
	static KeyBits *sqmodn_par;
	static ICC_SecMess  sm_SC;	/* sec. mode for communication SCT/SC	 */


	/* Variables for internal use */
	AlgEnc          algenc;
	AlgType         algtype;
	static AlgHash  alghash;
	ParmType        parmtype;
	int             rc;
	Boolean	        special_DecSK_selection;

	char           *proc = "secsc_sign";

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	secsc_errno = NOERR;
	sca_rc = 0;
	sct_id = sc_sel.sct_id;



	if (sec_state == F_null) {
		/* first call of this signature process */

		/*
		 * Get signature algorithm
		 */

		if(key->alg) {
			if (!signature->signAI) { /* for compatibility with older SecuDE versions, accept
						     the sign algorithm also from parameter signature */
				signature->signAI = aux_cpy_AlgId(key->alg);
			}
		}
			
		if ((signature->signAI == NULLALGID) || (signature->signAI->objid == NULLOBJID)) {
			/* default signature AI = md5WithRsaEncryption */
			signature->signAI = aux_cpy_AlgId(md5WithRsaEncryption);
		}


		/*
		 * Check signature algorithm:
		 */

		algenc = aux_ObjId2AlgEnc(signature->signAI->objid);
		if (algenc != RSA) {
			aux_add_error(ESCNOTSUPP, "Algorithm not supported by the smartcard", (char *) signature->signAI, AlgId_n, proc);
			return (-1);
		}

		algtype = aux_ObjId2AlgType(signature->signAI->objid);
		if (algtype != SIG) {
			aux_add_error(EINVALID, "wrong signAI in signature", (char *) signature->signAI, AlgId_n, proc);
			return (-1);
		}

		if (((parmtype = aux_ObjId2ParmType(signature->signAI->objid)) == PARM_NULL) &&
		    (signature->signAI->param)) {
			aux_add_error(EINVALID, "wrong parameter in signature algorithm", (char *) signature->signAI, AlgId_n, proc);
			return (-1);
		}


		key_sel.key_id = &key_id;
		if (key_to_keyid(key, signature->signAI, key_sel.key_id, special_DecSK_selection = TRUE)) {
			AUX_ADD_ERROR;
			return (-1);
		}

		key_sel.key_algid = aux_cpy_AlgId(signature->signAI);
	


		/*
		 *  key level = FILE_DUMMY means that key is stored in the SCT 
		 */
		if (key_sel.key_id->key_level == FILE_DUMMY) {
			aux_add_error(EINVALID, "Signature key must be key on the SC!", CNULL, 0, proc);
			aux_free_AlgId(&key_sel.key_algid);
			return (-1);
		}

		/*
		 * Handle SC application for key:
		 */

		if (handle_key_sc_app(sct_id, key, key_sel.key_id)) {
			AUX_ADD_ERROR;
			aux_free_AlgId(&key_sel.key_algid);
			return (-1);
		}
/*
 *   Intermediate result: 1) Key-id of the signature key is stored in key_sel.key_id!
 * 			  2) Signature key is a key on the SC.
 * 			  3) The application on the SC is open.
 *
 *
 *            Next to do: 1. Get parameter for hash-function,
 *		          2. Call SCA-IF function "sca_sign"
 */

		/*
		 * Get parameter for hash-function:
		 */

		alghash = aux_ObjId2AlgHash(signature->signAI->objid);
		if (alghash == SQMODN) {
			/* decode given BitString into structure KeyBits */
			sqmodn_par = d_KeyBits(&hash_input->sqmodn_input);
			if (!sqmodn_par) {
				aux_add_error(EDECODE, "Cannot decode hash_input", CNULL, 0, proc);
				aux_free_AlgId(&key_sel.key_algid);
				return (-1);
			}
		}


		/* set security mode for SCA-function */
		if (set_fct_sec_mode(sct_id, "sca_sign", &sm_SC)) {
			aux_add_error(LASTERROR, "Set security mode (prior to sign octetstring) failed", CNULL, 0, proc);
			goto errcase;
		}
		sec_state = F_sign;
	}
	 /* end if (sec_state == F_null) */ 
	else if (sec_state != F_sign) {
		aux_add_error(ESIGN, "wrong sec_state", CNULL, 0, proc);
		goto errcase;
	}

	/*
	 * The following is performed in any case
	 */


	/*
	 * Get parameter for hash-function:
	 */

	switch (alghash) {
	case SQMODN:
		if (!sqmodn_par) {
			aux_add_error(EINVALID, "Hash_input missing for algorithm SQMODN", CNULL, 0, proc);
			return (-1);
		}
		hash_par1.sqmodn_par.part1.noctets = sqmodn_par->part1.noctets;
		hash_par1.sqmodn_par.part1.octets = sqmodn_par->part1.octets;
		hash_par1.sqmodn_par.part2.noctets = sqmodn_par->part2.noctets;
		hash_par1.sqmodn_par.part2.octets = sqmodn_par->part2.octets;
		hash_par1.sqmodn_par.part3.noctets = 0;
		hash_par1.sqmodn_par.part4.noctets = 0;
		hash_par = &hash_par1;
		break;
	case MD2:
	case MD4:
	case MD5:
	case SHA:
		hash_par = (HashPar * ) 0;
		break;
	default:
		aux_add_error(EALGID, "Unknown hash alg_id", CNULL, 0, proc);
		return (-1);
	}			/* end switch */


	sca_rc = sca_sign(sct_id,
			  in_octets,
			  &key_sel,
			  hash_par,
			  more,
		  	  signature);


	if (sca_rc < 0) {
		secsc_errno = analyse_sca_err(sct_id, "sca_sign");
		aux_add_error(secsc_errno, "Cannot sign octetstring", (char *) sca_errmsg, char_n, proc);
		goto errcase;
	}

	/*
	 * if more == SEC_END, release storage
	 */

	if (more == SEC_END) {
		if ((alghash == SQMODN) && (sqmodn_par))
			aux_free_KeyBits(&sqmodn_par);

		sec_state = F_null;

#ifdef SECSCTEST
		fprintf(stderr, "signature->signature.bits: \n");
		aux_fxdump(stderr, signature->signature.bits, signature->signature.nbits / 8, 0);
		fprintf(stderr, " \n");
#endif

	}			/* end if (more == SEC_END) */
	return (0);


errcase:
	if ((alghash == SQMODN) && (sqmodn_par))
		aux_free_KeyBits(&sqmodn_par);
			
	aux_free_AlgId(&key_sel.key_algid);

	sec_state = F_null;

	return (-1);



}				/* end secsc_sign() */





/*--------------------------------------------------------------*/
/*						                */
/* PROC  secsc_verify					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*   Verify a given digital signature within the SCT.		*/
/*								*/
/*   1) Check key:						*/
/*      Key must be delivered in key->key.			*/
/*      No application check will be done,  the verification is */
/*      done within the SCT.					*/
/*								*/
/*   2) Algorithm:						*/
/*      If signature->signAI->objid == NULL			*/
/*         take default_sign_alg: md5WithRsaEncryption and 	*/
/*         return it in signature->signAI->objid		*/
/*         (memory is allocated)				*/
/*								*/
/*       Check signAI:						*/
/*        if alg != signature algorithm				*/
/*            => return(error)					*/
/*								*/
/*       Check verification key:				*/
/*        if key->key->subjectAI->objid != NULL			*/
/*	        => encryption method of verification key must 	*/
/*                 be = encryption algorithm of signAI		*/
/*        if key->key->subjectAI->objid == NULL			*/
/*	        => return error				 	*/
/*								*/
/*	Check algorithm:					*/
/*	If a parameter is specified for an algorithm which has 	*/
/*	no parameter,  an error is returned.			*/
/*								*/
/*   3) Get parameter for hash-function				*/
/*   4) Transform structure KeyInfo to structure KeyBit		*/
/*   5) Call "sca_verify_sig" to verify given signature within 	*/
/*      the SCT:						*/
/*      Parameters for "Verify signature":			*/
/* 			   - "sca_fct_list[]" delivers:		*/
/*			      - sec_mess			*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   in_octets		       Data to be verified.		*/
/*   signature		      					*/
/*   more							*/
/*   key		       Structure which identifies the   */
/*			       verification key.	 	*/
/*   hash_input		       Add. hash-alg specific parameters*/
/*								*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0			       Signature ok.			*/
/*   -1			       Invalid signature.		*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sca_verify_sig()		Verify a digital signature. 	*/
/*								*/
/*   aux_free_KeyBits()		Release members of struct 	*/
/*				KeyBits and KeyBits.		*/
/*   aux_ObjId2AlgEnc()		Map object identifier on 	*/
/*				algorithm encryption method.	*/
/*   aux_ObjId2AlgHash()	Map object identifier on 	*/
/*				algorithm hash method.		*/
/*   aux_ObjId2AlgType()	Map object identifier on 	*/
/*				algorithm type.			*/
/*   aux_ObjId2ParmType()	Map object identifier on 	*/
/*				type of parameter.		*/
/*   d_KeyBits()		Decode given BitString into 	*/
/*				structure KeyBits.		*/
/*   set_fct_sec_mode()		Set security mode for comm.     */
/*				between DTE/SCT depending on the*/
/*                	        SCA-function to be called.      */
/*                                                              */
/*   analyse_sca_err()		Analyse of an error-number 	*/
/*				returned by an SCA-IF function. */
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure secsc_verify
 *
 ***************************************************************/
#ifdef __STDC__

RC secsc_verify(
	OctetString	 *in_octets,
	Signature	 *signature,
	More		  more,
	Key		 *key,
	HashInput	 *hash_input
)

#else

RC secsc_verify(
	in_octets,
	signature,
	more,
	key,
	hash_input
)
OctetString	 *in_octets;
Signature	 *signature;
More		  more;
Key		 *key;
HashInput	 *hash_input;

#endif

{

	/* Variables for the SCA-IF */
	unsigned int    sct_id;
	HashPar        *hash_par;
	static KeySel   key_sel;
	static AlgId   *key_alg;
	HashPar         hash_par1;
	static KeyBits *sqmodn_par;
	static ICC_SecMess  sm_SC;	/* sec. mode for communication SCT/SC	 */


	/* Variables for internal use */
	AlgEnc          keyalgenc;
	AlgType         algtype;
	AlgEnc          algenc;
	static AlgHash  alghash;
	ParmType        parmtype;
	int             rc;

	char           *proc = "secsc_verify";

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	secsc_errno = NOERR;
	sca_rc = 0;
	sct_id = sc_sel.sct_id;



	if (sec_state == F_null) {
		/* first call of this verification process */

		if ((!key) || (!key->key)) {
			aux_add_error(EINVALID, "Verification key missing!", CNULL, 0, proc);
			return (-1);
		}


		/*
		 * Check verification key
		 */

		if ((key->key->subjectAI != NULLALGID) && (key->key->subjectAI->objid != NULLOBJID)) {
			keyalgenc = aux_ObjId2AlgEnc(key->key->subjectAI->objid);
			if (keyalgenc != RSA) {
				aux_add_error(ESCNOTSUPP, "Algorithm not supported by the smartcard", (char *) signature->signAI, AlgId_n, proc);
				return (-1);
			}
		} else {
			aux_add_error(EINVALID, "alg missing in parameter key->key->subjectAI", (char *) key->key->subjectAI, AlgId_n, proc);
			return -1;
		}


		/*
		 * Get signature algorithm
		 */

		if ((signature->signAI == NULLALGID) || (signature->signAI->objid == NULLOBJID)) {
			/* default signature AI = md5WithRsaEncryption */
			signature->signAI = aux_cpy_AlgId(md5WithRsaEncryption);
		}

		algtype = aux_ObjId2AlgType(signature->signAI->objid);
		algenc = aux_ObjId2AlgEnc(signature->signAI->objid);
		if (algtype != SIG) {
			aux_add_error(EINVALID, "wrong signAI in signature", (char *) signature->signAI, AlgId_n, proc);
			return (-1);
		}


		if (algenc != keyalgenc) {
			aux_add_error(EINVALID, "alg of signature doesn't fit to verification key", (char *) key->key->subjectAI, AlgId_n, proc);
			return -1;
		}


		/*
		 * Check algorithm:
		 */

		if (((parmtype = aux_ObjId2ParmType(signature->signAI->objid)) == PARM_NULL) &&
		    (signature->signAI->param)) {
			aux_add_error(EINVALID, "wrong parameter in signature algorithm", (char *) signature->signAI, AlgId_n, proc);
			return (-1);
		}

		/*
		 * Get parameter for hash-function:
		 */

		alghash = aux_ObjId2AlgHash(signature->signAI->objid);
		if (alghash == SQMODN) {
			/* decode given BitString into structure KeyBits */
			sqmodn_par = d_KeyBits(&hash_input->sqmodn_input);
			if (!sqmodn_par) {
				aux_add_error(EDECODE, "Cannot decode hash_input", CNULL, 0, proc);
				return (-1);
			}
		}

		/*
		 * Transform structure KeyInfo to structure KeyBits
		 */

		if ((key_sel.key_bits = d_KeyBits(&key->key->subjectkey)) == (KeyBits * ) 0) {
			aux_add_error(EDECODE, "Cannot decode encryptionkey", CNULL, 0, proc);
			return (-1);
		}
		key_alg = aux_cpy_AlgId(key->key->subjectAI);


		/* set security mode for SCA-function */
		if (set_fct_sec_mode(sct_id, "sca_verify_sig", &sm_SC)) {
			aux_add_error(LASTERROR, "Set security mode (prior to verify signature) failed", CNULL, 0, proc);
			goto errcase;
		}
		sec_state = F_verify;
	}
	 /* end if (sec_state == F_null) */ 
	else if (sec_state != F_verify) {
		aux_add_error(EVERIFY, "wrong sec_state", CNULL, 0, proc);
		goto errcase;
	}

	/*
	 * The following is performed in any case
	 */

	/*
	 * Get parameter for hash-function:
	 */

	switch (alghash) {
	case SQMODN:
		if (!sqmodn_par) {
			aux_add_error(EINVALID, "Hash_input missing for algorithm SQMODN", CNULL, 0, proc);
			return (-1);
		}
		hash_par1.sqmodn_par.part1.noctets = sqmodn_par->part1.noctets;
		hash_par1.sqmodn_par.part1.octets = sqmodn_par->part1.octets;
		hash_par1.sqmodn_par.part2.noctets = sqmodn_par->part2.noctets;
		hash_par1.sqmodn_par.part2.octets = sqmodn_par->part2.octets;
		hash_par = &hash_par1;
		hash_par1.sqmodn_par.part3.noctets = 0;
		hash_par1.sqmodn_par.part4.noctets = 0;
		break;
	case MD2:
	case MD4:
	case MD5:
	case SHA:
		hash_par = (HashPar * ) 0;
		break;
	default:
		aux_add_error(EALGID, "invalid hash alg_id", CNULL, 0, proc);
		return (-1);
	}			/* end switch */


	key_sel.key_algid = key_alg;


	sca_rc = sca_verify_sig(sct_id,
			        in_octets,
			        signature,
			        &key_sel,
			        hash_par,
			        more);


	if (sca_rc < 0) {
		secsc_errno = analyse_sca_err(sct_id, "sca_verify_sig");
		aux_add_error(secsc_errno, "Cannot verify signature ", (char *) sca_errmsg, char_n, proc);
		goto errcase;
	}

	/*
	 * if more == SEC_END, release storage
	 */

	if (more == SEC_END) {
		if ((alghash == SQMODN) && (sqmodn_par))
			aux_free_KeyBits(&sqmodn_par);
		if (key_sel.key_bits)
			aux_free_KeyBits(&key_sel.key_bits);
		if (key_alg)
			aux_free_AlgId(&key_alg);

		sec_state = F_null;

	}			/* end if (more == SEC_END) */
	return (0);


errcase:
	if ((alghash == SQMODN) && (sqmodn_par))
		aux_free_KeyBits(&sqmodn_par);
	if (key_sel.key_bits)
		aux_free_KeyBits(&key_sel.key_bits);
	if (key_alg)
		aux_free_AlgId(&key_alg);

	sec_state = F_null;

	return (-1);


}				/* end secsc_verify() */




/*--------------------------------------------------------------*/
/*						                */
/* PROC  secsc_encrypt					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*   Encrypt octetstring within the SCT or the SC.		*/
/*								*/
/*   1) Algorithm:						*/
/*      If key->alg != NULL					*/
/*	      => take this algorithm at the SCA-IF		*/
/*      else 							*/
/*         if key->key != NULL					*/
/*            => take key->key->subjectAI		        */
/*         else 						*/
/*            return(error)				        */
/*								*/
/*   2) Depending on encryption algorithm:			*/
/*   2.a) RSA:							*/
/*        1) Check key:						*/
/*           Key must be delivered in key->key.			*/
/*           No application check will be done,  the encryption	*/
/*           is done within the SCT.				*/
/*	  2) Transform structure KeyInfo to structure KeyBits	*/
/*	  3) The encryption process is performed within the SCT */
/*           (destination(SCA_IF) = SCT)			*/		
/*								*/
/*								*/
/*   2.b) DES/DES3:						*/
/*        1) Check key:						*/
/*           An error is returned, if the DES-key is delivered  */
/*           in key->key.					*/
/*        2) Get key_id from key (keyref or pse_sel).		*/
/*        3) Handle SC application for the key, if key = key on */
/*           SC (function handle_key_sc_app). 			*/
/*	  4) The encryption process is performed either by SCT  */
/*           or by the SC. It depends on the level of the key   */
/*	     where (destination (SCA_IF)) the encryption is 	*/
/*	     performed.						*/
/*								*/
/*								*/
/*   3) If "out_bits->nbits" is not a multiple of 8,		*/
/*         => return(error)					*/
/*      else 							*/
/*         set "out_octets->octets" = "out_bits->bits" 		*/
/*         set "out_octets->noctets" = "out_bits->nbits" / 8	*/
/*								*/
/*								*/
/*   4) Call "sca_encrypt" to encrypt octetstring within in 	*/
/*      SCT/SC.							*/
/*								*/
/*      Parameters for "Encryption":				*/
/* 			   - "sca_fct_list[]" delivers:		*/
/*			      - sec_mess			*/
/*								*/
/*								*/
/*   5) Set "out_bits->nbits" = "out_octets->noctets" * 8	*/
/*      return(return of sca_encrypt * 8)			*/
/*								*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   in_octets		       Data to be encrypted.		*/
/*   out_bits		       Encrypted data.			*/
/*   more							*/
/*   key		       Structure which identifies the   */
/*			       encryption key.	 		*/
/*								*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   nn			       no of encrypted bits.		*/
/*   -1			       error				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sca_encrypt()		Encrypt octetstring. 		*/
/*								*/
/*   aux_free_KeyBits()		Release members of struct 	*/
/*				KeyBits and KeyBits.		*/
/*   aux_ObjId2AlgEnc()		Map object identifier on 	*/
/*				algorithm encryption method.	*/
/*   aux_ObjId2AlgType()	Map object identifier on 	*/
/*				algorithm type.			*/
/*   d_KeyBits()		Decode given BitString into 	*/
/*				structure KeyBits.		*/
/*   handle_key_sc_app()	Handle SC-application for the   */
/*                              selected key.			*/
/*   key_to_keyid()	        Get key_id from key.            */
/*   set_fct_sec_mode()		Set security mode for comm.     */
/*				between DTE/SCT depending on the*/
/*                	        SCA-function to be called.      */
/*                                                              */
/*   analyse_sca_err()		Analyse of an error-number 	*/
/*				returned by an SCA-IF function. */
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure secsc_encrypt
 *
 ***************************************************************/
#ifdef __STDC__

RC secsc_encrypt(
	OctetString	 *in_octets,
	BitString	 *out_bits,
	More		  more,
	Key		 *key
)

#else

RC secsc_encrypt(
	in_octets,
	out_bits,
	more,
	key
)
OctetString	 *in_octets;
BitString	 *out_bits;
More		  more;
Key		 *key;

#endif

{

	/* Variables for the SCA-IF */
	unsigned int    sct_id;
	static OctetString *out_octets;
	static AlgId   *key_alg = NULLALGID;
	static KeySel   key_sel;
	static KeyId	key_id;
	static Destination  destination;/* indicates where the encryption shall be performed */
	static ICC_SecMess  sm_SC;	/* sec. mode for communication SCT/SC	 */


	/* Variables for internal use */
	AlgEnc          algenc = NoAlgEnc;
	AlgType         algtype;
	int             no_enc;
	Boolean	        special_DecSK_selection;

	char           *proc = "secsc_encrypt";

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	secsc_errno = NOERR;
	sca_rc = 0;
	sct_id = sc_sel.sct_id;


	if (sec_state == F_null) {
		/* first call of this encryption process */

		if (!key) {
			aux_add_error(EINVALID, "Encryption key missing!", CNULL, 0, proc);
			return (-1);
		}

		/*
		 * Get algorithm of the encryption key
		 */

		if (key->key) {
			key_alg = key->key->subjectAI;
			algenc = aux_ObjId2AlgEnc(key_alg->objid);
		}
		if (key->alg) {
			if (algenc != NoAlgEnc) {
				if (aux_ObjId2AlgEnc(key->alg->objid) != algenc) {
					aux_add_error(EINVALID, "Invalid algorithm in key->alg", (char *) key->alg, AlgId_n, proc);
					return -1;
				}
			}
			key_alg = key->alg;
			algenc = aux_ObjId2AlgEnc(key_alg->objid);
		} 

		if (key_alg == NULLALGID) {
			aux_add_error(EINVALID, "Algorithm missing", CNULL, 0, proc);
			return (-1);
		}

		/*
		 * Algorithm = encryption algorithm?
		 */

		if (((algtype = aux_ObjId2AlgType(key_alg->objid)) != SYM_ENC) &&
		    ((algtype = aux_ObjId2AlgType(key_alg->objid)) != ASYM_ENC)) {
			aux_add_error(EALGID, "Invalid encryption algorithm", CNULL, 0, proc);
			return (-1);
		}

		/*
		 * Get key depending on the algorithm
		 */

		switch (algenc) {
		case RSA:
			if (!key->key) {
				aux_add_error(EINVALID, "RSA encryption key missing!", CNULL, 0, proc);
				return (-1);
			}

			/*
			 * Transform structure KeyInfo to structure KeyBits
			 */
#ifdef SECSCTEST
			fprintf(stderr, "SECSC_ENCRYPT: &key->key->subjectkey: \n");
			aux_fxdump(stderr, key->key->subjectkey.bits, key->key->subjectkey.nbits / 8, 0);
			fprintf(stderr, " \n");
#endif

			if ((key_sel.key_bits = d_KeyBits(&key->key->subjectkey)) == (KeyBits * ) 0) {
				aux_add_error(EDECODE, "Cannot decode encryptionkey", CNULL, 0, proc);
				return (-1);
			}

			/* 
			 *  RSA-encryption is performed within the SCT
			 */
			destination = SCT;
			break;
		case DES:
		case DES3:
			if (key->key) {
				aux_add_error(EINVALID, "Delivery of DES key to SCT/SC not allowed!", CNULL, 0, proc);
				return (-1);
			}
			key_sel.key_id = &key_id;
			if (key_to_keyid(key, key_alg, key_sel.key_id, special_DecSK_selection = FALSE)) {
				AUX_ADD_ERROR;
				return (-1);
			}


			/*
			 * Handle SC application for key:
			 */

			if (handle_key_sc_app(sct_id, key, key_sel.key_id)) {
				AUX_ADD_ERROR;
				return (-1);
			}

			/*
			 *  Where the encryption process shall be performed depends
			 *  on the key_level.
			 *  key level == FILE_DUMMY means that key is stored in the SCT 
			 */

			if (key_sel.key_id->key_level == FILE_DUMMY) 
				destination = SCT;
			else	destination = ICC;


			break;
		default:
			aux_add_error(EALGID, "unknown alg_id", CNULL, 0, proc);
			return (-1);
		}		/* end switch */


		if ((out_octets = (OctetString *) malloc(sizeof(OctetString))) == NULLOCTETSTRING) {
			aux_add_error(EMALLOC, "out_octets", CNULL, 0, proc);
			goto errcase;
		}
		/* set security mode for SCA-function */
		if (set_fct_sec_mode(sct_id, "sca_encrypt", &sm_SC)) {
			aux_add_error(LASTERROR, "Set security mode (prior to encrypt octetstring) failed", CNULL, 0, proc);
			goto errcase;
		}
		sec_state = F_encrypt;
	}
	 /* end if (sec_state == F_null) */ 
	else if (sec_state != F_encrypt) {
		aux_add_error(EENCRYPT, "wrong sec_state", CNULL, 0, proc);
		goto errcase;
	}

	/*
	 * The following is performed in any case
	 */

	/*
	 * The SCA-software works with octets, the SEC-software expects the
	 * encrypted output as a bitstring
	 */


	if ((out_bits->nbits > 0) && ((out_bits->nbits % 8) != 0)) {
		aux_add_error(ESCNOTSUPP, "out_bits->nbits not a multiple of 8!", CNULL, 0, proc);
		goto errcase;
	}
	out_octets->octets = out_bits->bits;
	if (out_bits->nbits > 0)
		out_octets->noctets = out_bits->nbits / 8;
	else
		out_octets->noctets = out_bits->nbits;

#ifdef SECSCTEST
	fprintf(stderr, "in_octets: \n");
	aux_fxdump(stderr, in_octets->octets, in_octets->noctets, 0);
	fprintf(stderr, " \n");
#endif

	key_sel.key_algid = key_alg;

	no_enc = sca_encrypt(sct_id,
			     in_octets,
			     out_octets,
			     &key_sel,
			     destination,
			     &sm_SC,
			     more);


	if (no_enc < 0) {
		secsc_errno = analyse_sca_err(sct_id, "sca_encrypt");
		aux_add_error(secsc_errno, "Cannot encrypt octetstring", (char *) sca_errmsg, char_n, proc);
		goto errcase;
	}

	/*
	 * The SCA-software works with octets, the SEC-software expects the
	 * encrypted output as a bitstring
	 */

	out_bits->nbits += no_enc * 8;


	/*
	 * if more == SEC_END, release storage of key_bits
	 */

	if (more == SEC_END) {
		free(out_octets);
		if (key_sel.key_bits)
			aux_free_KeyBits(&key_sel.key_bits);
		sec_state = F_null;
	}
#ifdef SECSCTEST
	fprintf(stderr, "SECSC_ENCRYPT: no of encrypted data: %d\n", no_enc);
	fprintf(stderr, "out_octets: \n");
	aux_fxdump(stderr, out_octets->octets, no_enc, 0);
	fprintf(stderr, " \n");
	fprintf(stderr, "out_bits->nbits: %d\n", out_bits->nbits);
	fprintf(stderr, "out_octets->noctets: %d\n", out_octets->noctets);
#endif



	/*
	 * normal end :
	 */

	return (no_enc * 8);



	/*
	 * error case => release storage:
	 */

errcase:
	if (out_octets)
		free(out_octets);
	if (key_sel.key_bits)
		aux_free_KeyBits(&key_sel.key_bits);

	sec_state = F_null;
	return (-1);


}				/* end secsc_encrypt() */






/*--------------------------------------------------------------*/
/*						                */
/* PROC  secsc_decrypt					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*   Decrypt bitstring within the SCT or the SC.		*/
/*								*/
/*   1) Algorithm:						*/
/*      if key->alg == NULL					*/
/*         return(error)					*/
/*	else take this algorithm				*/
/*								*/
/*   2) Key selection:						*/
/*      1) An error is returned, if the key is delivered 	*/
/*         in "key->key".					*/
/*      2) Get key_id from key (keyref or pse_sel).		*/
/*	   Special selection of an decryption key is done in	*/
/*	   function "key_to_keyid()".				*/
/*      3) Handle SC application for the key, if key = key on   */
/*         SC (function handle_key_sc_app). 			*/
/*								*/
/*   3) Depending on decryption algorithm:			*/
/*   3.a) RSA:							*/
/*	  The encryption process is performed within the SCT 	*/
/*        (destination(SCA_IF) = SCT)				*/		
/*								*/
/*   3.b) DES/DES3:						*/
/*	  The decryption process is performed either by the SCT	*/
/*        or by the SC. It depends on the level of the key   	*/
/*	  where (destination (SCA_IF)) the decryption is 	*/
/*	  performed.						*/
/*								*/
/*   4) If "in_bits->nbits" is not a multiple of 8,		*/
/*         => return(error)					*/
/*      else 							*/
/*         set "in_octets->octets" = "in_bits->bits" 		*/
/*         set "in_octets->noctets" = "in_bits->nbits" / 8	*/
/*								*/
/*								*/
/*   5) Call "sca_decrypt" to decrypt octetstring within in 	*/
/*      SCT/SC.							*/
/*								*/
/*      Parameters for "Decryption":				*/
/* 			   - "sca_fct_list[]" delivers:		*/
/*			      - sec_mess			*/
/*								*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   in_bits		       Data to be Decrypted.		*/
/*   out_octets		       Decrypted data.			*/
/*   more							*/
/*   key		       Structure which identifies the   */
/*			       decryption key.		 	*/
/*								*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   nn			       no of decrypted octets.		*/
/*   -1			       error				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sca_decrypt()		Decrypt octetstring. 		*/
/*								*/
/*   aux_free_KeyBits()		Release members of struct 	*/
/*				KeyBits and KeyBits.		*/
/*   aux_ObjId2AlgEnc()		Map object identifier on 	*/
/*				algorithm encryption method.	*/
/*   aux_ObjId2AlgType()	Map object identifier on 	*/
/*				algorithm type.			*/
/*   handle_key_sc_app()	Handle SC-application for the   */
/*                              selected key.			*/
/*   key_to_keyid()	        Get key_id from key.            */
/*   set_fct_sec_mode()		Set security mode for comm.     */
/*				between DTE/SCT depending on the*/
/*                	        SCA-function to be called.      */
/*                                                              */
/*   analyse_sca_err()		Analyse of an error-number 	*/
/*				returned by an SCA-IF function. */
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure secsc_decrypt
 *
 ***************************************************************/
#ifdef __STDC__

RC secsc_decrypt(
	BitString	 *in_bits,
	OctetString	 *out_octets,
	More		  more,
	Key		 *key
)

#else

RC secsc_decrypt(
	in_bits,
	out_octets,
	more,
	key
)
BitString	 *in_bits;
OctetString	 *out_octets;
More		  more;
Key		 *key;

#endif

{

	/* Variables for the SCA-IF */
	unsigned int    sct_id;
	static OctetString *in_octets;
	static AlgId   *key_alg = NULLALGID;
	static KeyBits *key_bits;
	static KeySel   key_sel;
	static KeyId	key_id;
	static Destination  destination;
	static ICC_SecMess  sm_SC;	/* sec. mode for communication SCT/SC	 */


	/* Variables for internal use */
	AlgType         algtype;
	AlgEnc		algenc;
	int             no_dec;
	Boolean	        special_DecSK_selection;

	char           *proc = "secsc_decrypt";

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	secsc_errno = NOERR;
	sca_rc = 0;
	sct_id = sc_sel.sct_id;


	if (sec_state == F_null) {
		/* first call of this decryption process */

		if (!key) {
			aux_add_error(EINVALID, "Decryption key missing!", CNULL, 0, proc);
			return (-1);
		}

		/*
		 * Get algorithm of the decryption key
		 */

		if (!key->alg) {
			aux_add_error(EINVALID, "Algorithm missing", CNULL, 0, proc);
			return (-1);
		} 
		else {
			key_alg = key->alg;
		}


		/*
		 * Algorithm = decryption algorithm?
		 */

		if (((algtype = aux_ObjId2AlgType(key_alg->objid)) != SYM_ENC) &&
		    ((algtype = aux_ObjId2AlgType(key_alg->objid)) != ASYM_ENC)) {
			aux_add_error(EALGID, "Invalid encryption algorithm", CNULL, 0, proc);
			return (-1);
		}

		/*
		 * Key selection:
		 */

		if (key->key) {
			aux_add_error(EINVALID, "Delivery of decryption key to SCT/SC not allowed!", CNULL, 0, proc);
			return (-1);
		}
		key_bits = (KeyBits * ) 0;

		key_sel.key_id = &key_id;
		if (key_to_keyid(key, key_alg, key_sel.key_id, special_DecSK_selection = TRUE)) {
			AUX_ADD_ERROR;
			return (-1);
		}

		/*
		 * Handle SC application for key:
		 */

		if (handle_key_sc_app(sct_id, key, key_sel.key_id)) {
			AUX_ADD_ERROR;
			return (-1);
		}

		/*
		 *  Where the decryption process shall be performed depends
		 *  on the decryption algorithm and on the key_level:
		 *  RSA: RSA decryption is performed within the SCT.
		 *  DES: If the key is stored in the SCT (key level == FILE_DUMMY)  
		 *  	 =>	decryption is performed within the SCT.
		 *       If the key is stored in the SC (key level != FILE_DUMMY)  
		 *  	 =>	decryption is performed within the SC (ICC).
		 */

		algenc = aux_ObjId2AlgEnc(key_alg->objid);

		switch (algenc) {
		case RSA:
			destination = SCT;
			break;
		case DES:
		case DES3:

			if (key_sel.key_id->key_level == FILE_DUMMY) 
				destination = SCT;
			else	destination = ICC;


			break;
		default:
			aux_add_error(EALGID, "unknown alg_id", (char *) key_alg, AlgId_n, proc);
			return (-1);
		}		/* end switch */



		if ((in_octets = (OctetString *) malloc(sizeof(OctetString))) == NULLOCTETSTRING) {
			aux_add_error(EMALLOC, "in_octets", CNULL, 0, proc);
			goto errcase;
		}

		/* set security mode for SCA-function */
		if (set_fct_sec_mode(sct_id, "sca_decrypt", &sm_SC)) {
			aux_add_error(LASTERROR, "Set security mode (prior to decrypt octetstring) failed", CNULL, 0, proc);
			goto errcase;
		}
		sec_state = F_decrypt;
	}
	 /* end if (sec_state == F_null) */ 
	else if (sec_state != F_decrypt) {
		aux_add_error(EDECRYPT, "wrong sec_state", CNULL, 0, proc);
		goto errcase;
	}

	/*
	 * The following is performed in any case
	 */

	/*
	 * The SCA-software works with octets, the SEC-software delivers the
	 * input data in an bitstring
	 */

	if ((in_bits->nbits > 0) && ((in_bits->nbits % 8) != 0)) {
		aux_add_error(ESCNOTSUPP, "in_bits->nbits not a multiple of 8!", CNULL, 0, proc);
		goto errcase;
	}
	in_octets->octets = in_bits->bits;
	if (in_bits->nbits > 0)
		in_octets->noctets = in_bits->nbits / 8;
	else
		in_octets->noctets = in_bits->nbits;

#ifdef SECSCTEST
	fprintf(stderr, "in_octets: \n");
	aux_fxdump(stderr, in_octets->octets, in_octets->noctets, 0);
	fprintf(stderr, " \n");
#endif


	key_sel.key_algid = key_alg;
	key_sel.key_bits = key_bits;	/* is set to NULL */

	no_dec = sca_decrypt(sct_id,
			     in_octets,
			     out_octets,
			     &key_sel,
			     destination,
			     &sm_SC,
			     more);


	if (no_dec < 0) {
		secsc_errno = analyse_sca_err(sct_id, "sca_decrypt");
		aux_add_error(secsc_errno, "Cannot decrypt bitstring", (char *) sca_errmsg, char_n, proc);
		goto errcase;
	}

	/*
	 * if more == SEC_END, release storage
	 */

	if (more == SEC_END) {
		if (in_octets)
			free(in_octets);
		sec_state = F_null;
	}
#ifdef SECSCTEST
	fprintf(stderr, "SECSC_DECRYPT: no of decrypted data: %d\n", no_dec);
	fprintf(stderr, "out_octets: \n");
	aux_fxdump(stderr, out_octets->octets, no_dec, 0);
	fprintf(stderr, " \n");
	fprintf(stderr, "out_octets->noctets: %d\n", out_octets->noctets);
#endif



	/*
	 * normal end :
	 */

	return (no_dec);



	/*
	 * error case => release storage:
	 */

errcase:
	if (in_octets)
		free(in_octets);

	sec_state = F_null;
	return (-1);


}				/* end secsc_decrypt() */





/*--------------------------------------------------------------*/
/*						                */
/* PROC  secsc_get_EncryptedKey				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*   Encrypt an DES key with RSA within SCT/SC.			*/
/*   - The key to be encrypted (plain_key) must be a key stored */
/*     within the SCT (selected with key_ref or object.name).	*/
/*   - The encryption key (encryption_key) must be delivered in */
/*     "encryption_key->key" and encryption alg must be RSA.	*/
/*								*/
/*   1) Check "plain_key":					*/
/*      -  An error is returned, if the key is delivered  	*/
/*         in key->key.						*/
/*      -  Get key_id from key (keyref or pse_sel).		*/
/*								*/
/*   2) "encryption_key":					*/
/*      1) Check encryption key and encryption alg:		*/
/*         If "encryption_key->key" == NULL or  		*/
/*            "encryption_key->key->subjectAI" != rsa		*/
/*	      => return(error)					*/
/*      2) Transform structure KeyInfo to structure KeyBits	*/
/*								*/
/*   3)	No application check will be done, the encryption is	*/
/*      performed within the SCT.				*/
/*								*/
/*   4) Call "sca_enc_des_key" to encrypt the DES key within  	*/
/*      the SCT.						*/
/*								*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   encrypted_key	       Encrypted key.			*/
/*   plain_key		       Key to be encrypted.		*/
/*   encryption_key	       Encryption key.			*/
/*								*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*    0			       ok				*/
/*   -1			       error				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sca_enc_des_key()		Encrypt DES key with RSA.	*/
/*								*/
/*   aux_free_KeyBits()		Release members of struct 	*/
/*				KeyBits and KeyBits.		*/
/*   aux_ObjId2AlgEnc()		Map object identifier on 	*/
/*				algorithm encryption method.	*/
/*   aux_ObjId2AlgType()	Map object identifier on 	*/
/*				algorithm type.			*/
/*   d_KeyBits()		Decode given BitString into 	*/
/*				structure KeyBits.		*/
/*   key_to_keyid()	        Get key_id from key.            */
/*   set_fct_sec_mode()		Set security mode for comm.     */
/*				between DTE/SCT depending on the*/
/*                	        SCA-function to be called.      */
/*                                                              */
/*   analyse_sca_err()		Analyse of an error-number 	*/
/*				returned by an SCA-IF function. */
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure secsc_get_EncryptedKey
 *
 ***************************************************************/
#ifdef __STDC__

RC secsc_get_EncryptedKey(
	EncryptedKey	 *encrypted_key,
	Key		 *plain_key,
	Key		 *encryption_key
)

#else

RC secsc_get_EncryptedKey(
	encrypted_key,
	plain_key,
	encryption_key
)
EncryptedKey	 *encrypted_key;
Key		 *plain_key;
Key		 *encryption_key;

#endif

{

	/* Variables for the SCA-IF */
	unsigned int    sct_id;
	KeySel          plain_keysel;
	KeyId		plain_keyid;
	KeySel          encryption_keysel;
	AlgId          *encryptionkey_alg;
	AlgId          *encryptedkey_alg;
	ICC_SecMess     sm_SC;	/* sec. mode for communication SCT/SC	 */


	/* Variables for internal use */
	AlgEnc          algenc;
	AlgType         algtype;
	Boolean	        special_DecSK_selection;

	char           *proc = "secsc_get_EncryptedKey";

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	secsc_errno = NOERR;
	sca_rc = 0;
	sct_id = sc_sel.sct_id;



	if (!plain_key || !encryption_key || !encrypted_key) {
		aux_add_error(EINVALID, "key missing", CNULL, 0, proc);
		return (-1);
	}

	/*
	 * Check "plain_key"
	 */

	if (plain_key->key) {
		aux_add_error(EINVALID, "Delivery of plain key to SCT/SC not allowed!", CNULL, 0, proc);
		return (-1);
	}

	if ((plain_key->alg == NULLALGID) || (plain_key->alg->objid == NULLOBJID)) {
		aux_add_error(EINVALID, "Algorithm missing for plain key.", CNULL, 0, proc);
		return (-1);
	}

	plain_keysel.key_algid = plain_key->alg;

	plain_keysel.key_id = &plain_keyid;
	if (key_to_keyid(plain_key, plain_key->alg, plain_keysel.key_id, special_DecSK_selection = FALSE)) {
		AUX_ADD_ERROR;
		return (-1);
	}

	/*
	 *  key level != FILE_DUMMY means that key is stored in the SC
	 */
	if (plain_keysel.key_id->key_level != FILE_DUMMY) {
		aux_add_error(EINVALID, "Plain key is not a key within the SCT!", CNULL, 0, proc);
		return (-1);
	}


/*
 *   Intermediate result: Plain key is ok!
 *
 *            Next to do: - Check encryption key and encryption alg
 *       		  - Transform structure KeyInfo to structure KeyBits
 */


	if ((!encryption_key->key) ||
	    (!encryption_key->key->subjectAI) ||
	    (!encryption_key->key->subjectAI->objid)) {
		aux_add_error(EINVALID, "No encryption key/algorithm!", CNULL, 0, proc);
		return (-1);
	} else {
		algenc = aux_ObjId2AlgEnc(encryption_key->key->subjectAI->objid);
		algtype = aux_ObjId2AlgType(encryption_key->key->subjectAI->objid);
		if ((algenc != RSA) || (algtype != ASYM_ENC)) {
			aux_add_error(EINVALID, "Invalid encryption algorithm!", (char *) encryption_key->key->subjectAI, AlgId_n, proc);
			return (-1);
		}
	}

	if (encryption_key->alg) {
		if (aux_ObjId2AlgEnc(encryption_key->alg->objid) != algenc) {
			aux_add_error(EINVALID, "Invalid algorithm in encryption_key->alg", (char *) encryption_key->alg, AlgId_n, proc);
			return -1;
		}
		encryption_keysel.key_algid = encryption_key->alg;
	}
	else 
		encryption_keysel.key_algid = encryption_key->key->subjectAI;

	/*
	 * Transform structure KeyInfo to structure KeyBits
	 */

	if ((encryption_keysel.key_bits = d_KeyBits(&encryption_key->key->subjectkey)) == (KeyBits * ) 0) {
		aux_add_error(EDECODE, "Cannot decode encryptionkey", CNULL, 0, proc);
		return (-1);
	}



/*
 *   Intermediate result: Plain key and encryption key are ok!
 *
 *            Next to do: Call "sca_enc_des_key"
 */

	/* set security mode for SCA-function */
	if (set_fct_sec_mode(sct_id, "sca_enc_des_key", &sm_SC)) {
		aux_add_error(LASTERROR, "Set security mode (prior to get encrypted DES key) failed", CNULL, 0, proc);
		goto errcase;
	}
	sca_rc = sca_enc_des_key(sct_id,
				 &encryption_keysel,
				 &plain_keysel,
				 encrypted_key);

	if (sca_rc < 0) {
		secsc_errno = analyse_sca_err(sct_id, "sca_enc_des_key");
		aux_add_error(secsc_errno, "Cannot encrypt DES key", (char *) sca_errmsg, char_n, proc);
		goto errcase;
	}


	/*
	 *  STARMOD does not allocate storage for the AlgIds of encrypted key
         *    => copies are created here.
	 */

	encryptionkey_alg = aux_cpy_AlgId(encrypted_key->encryptionAI);
	encrypted_key->encryptionAI = encryptionkey_alg;

	encryptedkey_alg = aux_cpy_AlgId(encrypted_key->subjectAI);
	encrypted_key->subjectAI = encryptedkey_alg;



	/*
	 * normal end, release storage:
	 */

	if (encryption_keysel.key_bits)
		aux_free_KeyBits(&encryption_keysel.key_bits);

	return (0);




	/*
	 * error case => release storage:
	 */

errcase:
	if (encryption_keysel.key_bits)
		aux_free_KeyBits(&encryption_keysel.key_bits);

	return (-1);


}				/* end secsc_get_EncryptedKey() */





/*--------------------------------------------------------------*/
/*						                */
/* PROC  secsc_put_EncryptedKey				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*   Decrypt an rsa encrypted DES key and store within SCT/SC.	*/
/*   - The decryption key ("decryption_key") must be a private 	*/
/*     RSA key stored on the SC (selected with key_ref or 	*/
/*     object.name).						*/
/*   - The resulting key (decrypted DES key) is stored under    */
/*     the key_id specified by "plain_key". "plain_key" must    */
/*     address a key in the SCT or on the SC (selected with 	*/
/*     key_ref or object.name).					*/
/*   - The encrypted key ("encrypted_key") must be delivered in	*/
/*     "encrypted_key->subjectkey",				*/
/*     "encrypted_key->subjectAI" must be DES or DES3,		*/
/*     "encrypted_key->encryptionAI" must be RSA.		*/
/*								*/
/*   1) Check "encrypted_key":					*/
/*      If "encrypted_key->subjectkey" == NULL or		*/
/*         "encrypted_key->subjectAI" != DES or DES3 or 	*/
/*         "encrypted_key->encryptionAI" != RSA		 	*/
/*         return(error)					*/
/*								*/
/*   2) Check "plain_key":					*/
/*      1) Get key_id from key (keyref or pse_sel).		*/
/*      2) Handle SC application for the key, if key = key on   */
/*         SC (function handle_key_sc_app). 			*/
/*								*/
/*   3) Check "decryption_key":					*/
/*      1) An error is returned, if the key is delivered 	*/
/*         in "decryption_key->key".				*/
/*      2) Get key_id from key (keyref or pse_sel).		*/
/*	   Special selection of an decryption key is done in	*/
/*	   function "key_to_keyid()".				*/
/*      3) Handle SC application for the key, if key = key on   */
/*         SC (function handle_key_sc_app). 			*/
/*								*/
/*   4) Call "sca_dec_des_key" to decrypt the rsa encrypted DES */
/*      key within the SCT. The key is stored temporarily 	*/
/*      within the SCT.						*/
/*								*/
/*   5) If the key has to be installed on the SC:		*/
/*	 1. Check algorithm, only the installation of RSA- and 	*/
/*          DES-keys is allowed.				*/
/*	 2. Set the attribute list for the key			*/
/*       3. Call SCA-IF function "sca_inst_user_key()"		*/
/*	 4. Delete the remaining copy of the key within the SCT.*/   
/*								*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   encrypted_key	       Encrypted key.			*/
/*   plain_key		       Key to be encrypted.		*/
/*   decryption_key	       Decryption key.			*/
/*   replace		       if FALSE do not overwrite key    */
/*								*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*    0			       ok				*/
/*   -1			       error				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sca_dec_des_key()		Encrypt DES key with RSA.	*/
/*   sca_del_user_key()		Delete user key in an SCT.	*/
/*   sca_inst_user_key()	Install user key on the SC.     */
/*								*/
/*   aux_free_KeyBits()		Release members of struct 	*/
/*				KeyBits and KeyBits.		*/
/*   aux_ObjId2AlgEnc()		Map object identifier on 	*/
/*				algorithm encryption method.	*/
/*   aux_ObjId2AlgType()	Map object identifier on 	*/
/*				algorithm type.			*/
/*   handle_key_sc_app()	Handle SC-application for the   */
/*                              selected key.			*/
/*   key_to_keyid()	        Get key_id from key.            */
/*   set_fct_sec_mode()		Set security mode for comm.     */
/*				between DTE/SCT depending on the*/
/*                	        SCA-function to be called.      */
/*                                                              */
/*   analyse_sca_err()		Analyse of an error-number 	*/
/*				returned by an SCA-IF function. */
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure secsc_put_EncryptedKey
 *
 ***************************************************************/
#ifdef __STDC__

RC secsc_put_EncryptedKey(
	EncryptedKey	 *encrypted_key,
	Key		 *plain_key,
	Key		 *decryption_key,
	Boolean		  replace
)

#else

RC secsc_put_EncryptedKey(
	encrypted_key,
	plain_key,
	decryption_key,
	replace
)
EncryptedKey	 *encrypted_key;
Key		 *plain_key;
Key		 *decryption_key;
Boolean		  replace;

#endif

{

	/* Variables for the SCA-IF */
	unsigned int    sct_id;
	KeySel          plain_keysel;
	KeyId           plain_keyid;
	KeySel          decryption_keysel;
	KeyId           decryption_keyid;
	AlgId          *enc_alg;
	KeyAttrList     key_attr_list;
	ICC_SecMess     sm_SC;	/* sec. mode for communication SCT/SC	 */


	/* Variables for internal use */
	AlgEnc          algenc;
	AlgEnc		plainkey_algenc;
	AlgType         algtype;
	Boolean	        special_DecSK_selection;

	char           *proc = "secsc_put_EncryptedKey";

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	secsc_errno = NOERR;
	sca_rc = 0;
	sct_id = sc_sel.sct_id;



	if (!plain_key || !decryption_key || !encrypted_key) {
		aux_add_error(EINVALID, "key missing", CNULL, 0, proc);
		return (-1);
	}

	/*
	 * Check encrypted key
	 */

	if ((encrypted_key == (EncryptedKey * ) 0) ||
	    (encrypted_key->encryptionAI == NULLALGID) ||
	    (encrypted_key->encryptionAI->objid == NULLOBJID) ||
	    (encrypted_key->subjectAI == NULLALGID) ||
	    (encrypted_key->subjectAI->objid == NULLOBJID) ||
	    (encrypted_key->subjectkey.nbits == 0) ||
	    (encrypted_key->subjectkey.bits == CNULL)) {
		aux_add_error(EINVALID, "Invalid encrypted_key!", CNULL, 0, proc);
		return (-1);
	}
	algenc = aux_ObjId2AlgEnc(encrypted_key->encryptionAI->objid);
	algtype = aux_ObjId2AlgType(encrypted_key->encryptionAI->objid);
	if ((algenc != RSA) || (algtype != ASYM_ENC)) {
		aux_add_error(EINVALID, "Invalid encryption algorithm!", CNULL, 0, proc);
		return (-1);
	}
	algenc = aux_ObjId2AlgEnc(encrypted_key->subjectAI->objid);
	algtype = aux_ObjId2AlgType(encrypted_key->subjectAI->objid);
	if (((algenc != DES) && (algenc != DES3)) || (algtype != SYM_ENC)) {
		aux_add_error(EINVALID, "Invalid encryption algorithm!", CNULL, 0, proc);
		return (-1);
	}

	/*
	 * Check "plain_key"
	 */

	if (plain_key->key) {
		aux_add_error(EINVALID, "Delivery of plain key to SCT/SC not allowed!", CNULL, 0, proc);
		return (-1);
	}
	plain_keysel.key_id = &plain_keyid;
	if (key_to_keyid(plain_key, encrypted_key->subjectAI, plain_keysel.key_id, special_DecSK_selection = FALSE)) {
		AUX_ADD_ERROR;
		return (-1);
	}

	/*
	 * Check algorithm of "plain_key": 
	 *     1. Algorithm of plainkey should be DES or DES3.
	 *     2. As the SC doesn't support DES3, a DES3 key 
	 *	  should not be installed on the SC.
	 */

	plainkey_algenc = aux_ObjId2AlgEnc(plain_key->alg->objid);
	if ((plainkey_algenc != DES) && (plainkey_algenc != DES3)) {
		aux_add_error(EINVALID, "Algorithm of plain key should be DES or DES3!", (char *) plain_key->alg, AlgId_n, proc);
		return (-1);
	}
	else {
		if ((plainkey_algenc == DES3) && (plain_keysel.key_id->key_level != FILE_DUMMY)) {
			aux_add_error(ESCNOTSUPP, "Algorithm not supported by the smartcard", (char *) plain_key->alg, AlgId_n, proc);
			return (-1);
		}
	}

	plain_keysel.key_algid = plain_key->alg;



	/*
	 * Handle SC application for plain_key:
	 */

	if (handle_key_sc_app(sct_id, plain_key, plain_keysel.key_id)) {
		AUX_ADD_ERROR;
		return (-1);
	}

/*
 *   Intermediate result: Encrypted key and plain key are ok!
 *
 *            Next to do: - Check decryption key
 */


	if (decryption_key->key) {
		aux_add_error(EINVALID, "Delivery of decryption key to SCT/SC not allowed!", CNULL, 0, proc);
		return (-1);
	}
	decryption_keysel.key_id = &decryption_keyid;
	if (key_to_keyid(decryption_key, encrypted_key->encryptionAI, decryption_keysel.key_id, special_DecSK_selection = TRUE)) {
		AUX_ADD_ERROR;
		return (-1);
	}

	/*
	 * Handle SC application for decryption key:
	 */

	if (handle_key_sc_app(sct_id, decryption_key, decryption_keysel.key_id)) {
		AUX_ADD_ERROR;
		return (-1);
	}


/*
 *   Intermediate result: Plain key, decryption key and encrypted key are ok!
 *
 *            Next to do: - call "sca_dec_des_key()" to decrypt the encrypted key.
 *			    The result is stored temporarily in the SCT.
 */


	/* set security mode for SCA-function */
	if (set_fct_sec_mode(sct_id, "sca_dec_des_key", &sm_SC)) {
		aux_add_error(LASTERROR, "Set security mode (prior to decrypt encrypted DES key) failed", CNULL, 0, proc);
		return (-1);
	}

	sca_rc = sca_dec_des_key(sct_id,
				 encrypted_key,
				 &plain_keysel,
				 &decryption_keysel);


	if (sca_rc < 0) {
		secsc_errno = analyse_sca_err(sct_id, "sca_dec_des_key");
		aux_add_error(secsc_errno, "Cannot decrypt encrypted DES key", (char *) sca_errmsg, char_n, proc);
		return (-1);
	}


/*
 *   Intermediate result: The plain key is stored in the SCT!
 *
 *            Next to do: If the key shall be installed on the SC (level != FILE_DUMMY)
 *			  1. Check algorithm 
 *			     (Only the installation of DES-keys is allowed)
 *			  2. Set the attribute list for the key
 *		          3. Call SCA-IF function "sca_inst_user_key"
 */


	/*
	 *  key level != FILE_DUMMY means that key shall be stored on the SC 
	 */

	if (plain_keysel.key_id->key_level != FILE_DUMMY) {

		/*
		 *  Check algorithm 
		 */

		if (plainkey_algenc != DES) {
			aux_add_error(ESCNOTSUPP, "Algorithm not supported by the smartcard", (char *) plain_key->alg, AlgId_n, proc);
			return (-1);
		}

		if (replace == FALSE)
			key_attr_list.key_inst_mode = INST;
		else
			key_attr_list.key_inst_mode = REPL;

		key_attr_list.key_attr.key_repl_flag = TRUE;	/* key is replaceable */
		key_attr_list.key_attr.key_chg_flag = TRUE;
		key_attr_list.key_attr.key_foreign_flag = TRUE;
		key_attr_list.key_attr.key_purpose = PURPOSE_CIPHER;
		key_attr_list.key_attr.key_presentation = KEY_LOCAL;
		key_attr_list.key_attr.MAC_length = 4;

		key_attr_list.key_fpc = 0;
		key_attr_list.key_status.PIN_check = FALSE;
		key_attr_list.key_status.key_state = FALSE; /* key is not locked */

		/* set security mode for SCA-function */
		if (set_fct_sec_mode(sct_id, "sca_inst_user_key", &sm_SC)) {
			aux_add_error(LASTERROR, "Set security mode (prior to install decrypted DES key) failed", CNULL, 0, proc);
			return (-1);
		}

		sca_rc = sca_inst_user_key(sct_id, &plain_keysel, &key_attr_list);
		if (sca_rc < 0) {
/*???????????????????????????????????????????????????????????????????????????????????????*/
			if ((sca_errno == EFILE) && (key_attr_list.key_inst_mode == INST)) {
				aux_add_error(EINVALID, "Key to be installed exists already!", CNULL, 0, proc);
			}
			else 
			if ((sca_errno == EFILE) && (key_attr_list.key_inst_mode == REPL)) {
				aux_add_error(EINVALID, "Key to be replaced doesn't exist!", CNULL, 0, proc);
			}
/*???????????????????????????????????????????????????????????????????????????????????????*/
			else {
				secsc_errno = analyse_sca_err(sct_id, "sca_inst_user_key");
				aux_add_error(secsc_errno, "Cannot install decrypted DES key", (char *)  sca_errmsg, char_n, proc);
			}
			return (-1);
		}



		/*
		 *   Intermediate result: The key has been installed on the SC!
		 *			  A copy of this key still remains in the SCT
		 *
		 *            Next to do: Delete this key stored in the SCT.
		 */

	
		/* set security mode for SCA-function */
		if (set_fct_sec_mode(sct_id, "sca_del_user_key", &sm_SC)) {
			aux_add_error(LASTERROR, "Set security mode (prior to delete copy of key  in the SCT) failed", CNULL, 0, proc);
			return (-1);
		}
		sca_rc = sca_del_user_key(sct_id,
					  &plain_keysel);


		if (sca_rc < 0) {
			secsc_errno = analyse_sca_err(sct_id, "sca_del_user_key");
			aux_add_error(secsc_errno, "Cannot delete copy of key in the SCT", (char *) sca_errmsg, char_n, proc);
			return (-1);
		}


	} 
	return (0);

}				/* end secsc_put_EncryptedKey() */






/*--------------------------------------------------------------*/
/*						                */
/* PROC  secsc_del_key					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*   Delete a key stored in the SCT.				*/
/*								*/
/*   The key to be deleted must be a key stored in the SCT.     */
/*								*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   keyref     	       Reference to an existing key.	*/
/*								*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*    0			       ok				*/
/*   -1			       error				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sca_del_user_key()		Delete user key in an SCT.	*/
/*								*/
/*   keyref_to_keyid()	        Transform keyref into structure */
/*                              keyid.				*/
/*   set_fct_sec_mode()		Set security mode for comm.     */
/*				between DTE/SCT depending on the*/
/*                	        SCA-function to be called.      */
/*                                                              */
/*   analyse_sca_err()		Analyse of an error-number 	*/
/*				returned by an SCA-IF function. */
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure secsc_del_key
 *
 ***************************************************************/
#ifdef __STDC__

RC secsc_del_key(
	KeyRef	  keyref
)

#else

RC secsc_del_key(
	keyref
)
KeyRef	  keyref;

#endif

{

	/* Variables for the SCA-IF */
	unsigned int    sct_id;
	KeySel          key_sel;
	KeyId		key_id;
	ICC_SecMess     sm_SC;	/* sec. mode for communication SCT/SC	 */


	/* Variables for internal use */
	Boolean		SC_internal = FALSE;	/* only keys stored in the SCT can be deleted */

	char           *proc = "secsc_del_key";

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	secsc_errno = NOERR;
	sca_rc = 0;
	sct_id = sc_sel.sct_id;

	key_sel.key_id = &key_id;
	if (keyref_to_keyid(keyref, SC_internal, key_sel.key_id)) {
		AUX_ADD_ERROR;
		return (-1);
	}

	/*
	 *  key level != FILE_DUMMY means that key is stored in the SC
	 */
	if (key_sel.key_id->key_level != FILE_DUMMY) {
		aux_add_error(EINVALID, "Only a key within the SCT can be deleted!", CNULL, 0, proc);
		return (-1);
	}

	/*
	 *  Set the alg-id of this key to default value: desCBC
	 */
	
	key_sel.key_algid = desCBC;

	
	/* set security mode for SCA-function */
	if (set_fct_sec_mode(sct_id, "sca_del_user_key", &sm_SC)) {
		aux_add_error(LASTERROR, "Set security mode (prior to delete key in the SCT) failed", CNULL, 0, proc);
		return (-1);
	}
	sca_rc = sca_del_user_key(sct_id,
				  &key_sel);


	if (sca_rc < 0) {
		secsc_errno = analyse_sca_err(sct_id, "sca_del_user_key");
		aux_add_error(secsc_errno, "Cannot delete key in the SCT", (char *) sca_errmsg, char_n, proc);
		return (-1);
	}
	return (0);


}				/* end secsc_del_key() */






/*--------------------------------------------------------------*/
/*						                */
/* PROC  secsc_unblock_SCpin				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Unblock blocked PIN on the SC with the PUK.			*/
/*								*/
/*  1. Call "open_SC_application" to open the application on 	*/
/*     the SC and to perform the device authentication.		*/
/*  2. Perform user authentication with "pin_type" = PUK to    	*/
/*     unblock the PIN on the SC.				*/
/*								*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   pse_sel	 	       					*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0			       o.k.				*/
/*   -1			       Error				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   open_SC_application() 	Require SC, open SC application,*/
/*				perform device authentication.  */
/*   user_authentication()      Perform user authentication	*/
/*				(PUK).				*/
/*			         		       		*/
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure secsc_unblock_SCpin
 *
 ***************************************************************/
#ifdef __STDC__

RC secsc_unblock_SCpin(
	PSESel	 *pse_sel
)

#else

RC secsc_unblock_SCpin(
	pse_sel
)
PSESel	 *pse_sel;

#endif

{

	/* Variables for the SCA-IF */
	unsigned int    sct_id;

	int             obj_in_SCToc;


	char           *proc = "secsc_unblock_SCpin";

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	secsc_errno = NOERR;
	sca_rc = 0;
	sct_id = sc_sel.sct_id;



	if (open_SC_application(sct_id, pse_sel->app_name)) {
		aux_add_error(EAPPNAME, "Open application (prior to unblock PIN on SC) failed", (char *) pse_sel->app_name, char_n, proc);
		return (-1);
	}


	if (user_authentication(sct_id, pse_sel->app_name, PUK)) {
		aux_add_error(LASTERROR, "User (PUK) authentication (for unblocking PIN) failed", (char *) pse_sel, PSESel_n, proc);
		return (-1);
	}

	return (0);


}				/* end secsc_unblock_SCpin() */





/*--------------------------------------------------------------*/
/*						                */
/* PROC  secsc_sc_eject					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*   Case 1: Input parameter "sct_sel" == ALL_SCTS: 		*/
/*             =>  eject all SCs, which have been requested	*/
/*                 (in this session) and set application to 	*/
/*	           close for the SCTs.			 	*/
/*								*/
/*   Case 2: Input parameter "sct_sel" == CURRENT_SCT:		*/
/*		=> Eject without check whether SC is inserted.	*/
/*		=> Send to the current SCT an EJECT command by  */
/*                 by calling the function sca_eject.		*/
/*	 	   The sct_id of the current SCT is the value   */
/*		   of the global variable "sc_sel.sct_id".	*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   sct_sel	 	       					*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0			       o.k.				*/
/*   -1			       Error				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sca_get_sct_info()		Get information about  		*/
/*				registered SCTs.		*/
/*   sec_scttest()		Check whether SCT is available. */
/*   eject_sc()		        Handle ejection of the SC.      */
/*                                                              */
/*   analyse_sca_err()		Analyse of an error-number 	*/
/*				returned by an SCA-IF function. */
/*   aux_add_error()		Add error to error stack.	*/
/*                                                              */
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure secsc_sc_eject
 *
 ***************************************************************/
#ifdef __STDC__

RC secsc_sc_eject(
	SCTSel	  sct_sel
)

#else

RC secsc_sc_eject(
	sct_sel
)
SCTSel	  sct_sel;

#endif

{

	/* Variables for the SCA-IF */
	unsigned int    sct_id;
	OctetString     sc_info;	/* historical characters 		 */
	char           *display_text;
	Boolean         alarm;


	/* Variables for internal use */
	int             no_of_SCTs;	/* number os registered SCTs		 */
	int             count_SCTs;
	SCTDevice	SCT_available;

	char           *proc = "secsc_sc_eject";

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	secsc_errno = NOERR;
	sca_rc = 0;
	sct_id = sc_sel.sct_id;


	switch (sct_sel) {

	case ALL_SCTS:

		/*
		 * Send EJECT command to all registered SCTs, in which an SC is inserted.
		 */

		/* get number of registered SCTs */
		no_of_SCTs = sca_get_sct_info(0);

		for (count_SCTs = 1; count_SCTs <= no_of_SCTs; count_SCTs++) {

			/* check: SC in SCT? */
			sca_rc = sca_get_sct_info(count_SCTs);

			if (sca_rc == M_CARD_INSERTED) {
				/* SC is inserted => eject it */
				if (eject_sc(count_SCTs, display_text = CNULL, alarm = FALSE)) {
					AUX_ADD_ERROR;
					return (-1);
				}
			}

		}

		break;

	case CURRENT_SCT:

		sct_id = sc_sel.sct_id;

		if ((SCT_available = sec_scttest(CNULL)) < 0) {
			AUX_ADD_ERROR;
			return (-1);
		}

		if (SCT_available == SCTDev_not_avail) {

			aux_add_error(LASTERROR, "Cannot eject smartcard, SCT is not available", CNULL, 0, proc);
			return (-1);
		}


		/*
		 * Send EJECT command to current SCT.
		 */

		if (eject_sc(sct_id, display_text = CNULL, alarm = FALSE)) {
			AUX_ADD_ERROR;
			return (-1);
		}
		break;

	default:
		aux_add_error(EINVALID, "Invalid input for sc_eject", CNULL, 0, proc);
		return (-1);

	}			/* end switch */


	return (0);

}				/* end secsc_sc_eject() */







/*--------------------------------------------------------------*/
/*						                */
/* PROC  analyse_sca_err				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Analyse of an error-number returned by an SCA-IF function.  */
/*								*/
/*  - "sca_errno" is the global error variable set by an SCA-IF	*/
/*    function.							*/
/*  - "sca_errmsg" is the global pointer to error message set   */
/*    by an SCA-IF function.					*/
/*     								*/
/*  1. If "sca_errno" indicates that the SC is ejected/removed: */
/*  	- set SC to "not inserted" in sct_stat_list		*/
/*  	- set application to CLOSE for the current SCT and 	*/
/*        write configuration data,				*/
/*  	- reset STARMOD with new SCT configuration data.	*/
/*								*/
/*  2. Eject SC, if reset of the SC failed.			*/
/*								*/
/*  3. Return error number ESC to indicate that a STAPAC -   	*/
/*     function has produced this error.			*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   sct_id 		        SCT identifier			*/
/*   sca_fct_name	        Name of the called SCA-IF 	*/
/*				function			*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   ESC		       Error number to be used by the   */
/*                             calling routine.			*/
/*   EDEVLOCK		       This number will be returned if  */
/*			       SCA-IF has returned EDEVBUSY	*/
/*								*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   enter_app_in_sctlist()	Enter information about app in  */
/*				sct_list for current SCT.       */
/*   eject_sc()			Handle the ejection of the SC.  */
/*   init_SCT_config()		Initialize SCT configuration    */
/*				list.				*/
/*   save_portparam()		Get port parameter from STARMOD */
/*				and save into SCT configuration */
/*				file.				*/
/*   sca_reset()		Reset communication between DTE */
/*				and SCT.		        */
/*   sca_display()		Display text on SCT-display.    */
/*			         		       		*/
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure analyse_sca_err
 *
 ***************************************************************/
#ifdef __STDC__

static int analyse_sca_err(
	int	  sct_id,
	char     *sca_fct_name
)

#else

static int analyse_sca_err(
	sct_id,
	sca_fct_name
)
int	  sct_id;
char	 *sca_fct_name;

#endif

{

	int             ret_sca_errno;
	char           *ret_sca_errmsg;
	int		rc;
	char		sca_errno_text[256];
	SCT_SecMess     sm_SCT;	/* sec. mode for communication DTE/SCT	 */
	static	Boolean prevent_loop = FALSE;

	int		local_rc = ESC;

	char           *proc = "analyse_sca_err";

	

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	
	if (prevent_loop == TRUE)
		/*
		 *  "analyse_sca_err()" has been called from a function
		 *   which has been called from "analyse_sca_err()".
		 *   To avoid an endless loop, "analyse_sca_err()" does 
		 *   nothing in this case.
		 */
		return (ESC);


	/*
	 *  Add error number/message from the SCA-IF to global error stack
	 */

	sprintf (sca_errno_text, "(%d) %s", sca_errno, sca_errmsg);
	aux_add_error(ESCA, (char *) "SCA-IF error: ", (char *) sca_errno_text, char_n, sca_fct_name);


	/*
	 *  Save the original error from SCA-IF
	 */

	ret_sca_errno = sca_errno;
	ret_sca_errmsg = sca_errmsg;



	if ((!strcmp(sca_fct_name, "sca_reset")) && (sca_errno == ESCTRESET)) {
		aux_add_error(LASTERROR, "Reset SCT failed: SCT is not active (e.g. SCT is not connected), sct_id: ", (char *) sct_id, int_n, proc);
		return (ESC);
	}


	/*
	 * Handling of severe SCA-IF errors
	 */

	switch (sca_errno) {

		case ENOCARD:
		case ERESET:
		case EICCTIMEOUT:
		case EICCREMOVED:
		case EUSERBREAK:
		case EUSTIMEOUT:

			/*
			 *  SC has been ejected/removed or user break or 
			 *  no answer from user
			 *  In any case: the SC is not longer inserted.
			 */

			prevent_loop = TRUE;
			init_SCT_config(sct_id);	/* initialize the internal SCT 
							   configuration data */

	       		delete_old_SCT_config(sct_id);  /* delete the SCT configuration
							   data file */

			/*
			 * Eject SC, if reset of the SC failed.
			 */

			if (sca_errno == ERESET) {
				eject_sc(sct_id, CNULL, FALSE);
				sca_display(sct_id, SCT_TEXT_RESET_SC_ERR, 0);
			}
			prevent_loop = FALSE;

			break;


		case EDEVBUSY:

			/* 
			 *  SCT port is busy, port is locked
			 */

			return (EDEVLOCK);
	
			break;


		case ENOINTKEY:
		case ENOCONCKEY:

			/*
			 *  Error codes indicate that something is 
			 *	wrong with the secure messaging keys (DTE/SCT)
			 */

			/*
			 *  Reset secure messaging (DTE/SCT)
			 */

			sm_SCT.command = SCT_SEC_NORMAL;
			sm_SCT.response = SCT_SEC_NORMAL;
			prevent_loop = TRUE;
			if (prepare_sec_mess(sct_id, &sm_SCT, TRUE)) {
				aux_add_error(LASTERROR, "Reset secure messaging (DTE/SCT) failed", CNULL, 0, proc);
				prevent_loop = FALSE;
				return (ESC);
			}
			prevent_loop = FALSE;

			break;




		default:
			break;

	} /* end switch */
	



	/*
	 *  Handle error codes which indicate that SCT port has been closed
	 */
	
	if ((sca_errno >= EWRITE) && (sca_errno <= ESCTRESET) ||
	    (sca_errno == ESTATE0)) {

		/*
		 *  Error codes indicate that - SCT should be resetted
		 *			      - error on T1-level
		 *  Port has been closed by STARMOD
		 */

		prevent_loop = TRUE;
		if (reset_SCT(sct_id)) {
			if (aux_last_error() == EDEVLOCK) 
				aux_add_error(LASTERROR, "Cannot reset SCT (SCT device is busy)", CNULL, 0, proc);
			else
				aux_add_error(LASTERROR, "Cannot reset SCT (e.g. SCT is not connected), sct_id: ", (char *) sct_id, int_n, proc);

			prevent_loop = FALSE;
			return (ESC);
		}
		prevent_loop = FALSE;
	}




	/*
	 *  Restore the original error from SCA-IF
	 */

	sca_errno = ret_sca_errno;
	sca_errmsg = ret_sca_errmsg;




	local_rc = ESC;	/* default value */

	/*
	 *  Depending on the called SCA-IF function the rc is set
	 */

	if (!strcmp(sca_fct_name, "sca_reset")) {

		switch (sca_errno) {
			case ENOSHELL:	
			case EOPERR:
			case EEMPTY:
			case ERDERR:
			case ECLERR:
			case ESIDUNK:
				local_rc = ESCTINST;
				break;
			default:
				local_rc = ESC;
		}
	}
	else  
	if (!strcmp(sca_fct_name, "sca_verify_pin")) {

		switch (sca_errno) {
			case EKEY_LOCKED:
			case ELAST_ATTEMPT:
				local_rc = ESCPINLOCK;
				break;
			case EAUTH_1ATTEMPT:
			case EAUTH_2ATTEMPT:
			case EAUTH_3ATTEMPT:
			case EAUTH_4ATTEMPT:
			case EAUTH_5ATTEMPT:
			case EAUTH_6ATTEMPT:
			case EAUTH_7ATTEMPT:
			case ECPIN:
				local_rc = ESCAUTH;
				break;
			default:
				local_rc = ESC;
		}
	}
	else  
	if (!strcmp(sca_fct_name, "sca_unblock_pin")) {

		switch (sca_errno) {
			case EKEY_LOCKED:
			case ELAST_ATTEMPT:
				local_rc = ESCPUKLOCK;
				break;
			case EAUTH_1ATTEMPT:
			case EAUTH_2ATTEMPT:
			case EAUTH_3ATTEMPT:
			case EAUTH_4ATTEMPT:
			case EAUTH_5ATTEMPT:
			case EAUTH_6ATTEMPT:
			case EAUTH_7ATTEMPT:
			case ECPIN:
				local_rc = ESCAUTH;
				break;
			case EINV_LEN:
				local_rc = ESCPININV;
				break;
			default:
				local_rc = ESC;
		}
	}
	else  
	if (!strcmp(sca_fct_name, "sca_select_file")) {

		switch (sca_errno) {
			case EFILE:
				local_rc = EOBJNAME;	/* e.g. file (WEF/DF) on SC not found */
				break;
			default:
				local_rc = ESC;
		}
	}
	else
	if (!strcmp(sca_fct_name, "sca_read_binary")) {

		switch (sca_errno) {
			case EFILE:
				local_rc = EOBJNAME;	/* e.g. file (WEF) on SC not found */
				break;
			default:
				local_rc = ESC;
		}
	}
	else
	if (!strcmp(sca_fct_name, "sca_write_binary")) {

		switch (sca_errno) {
			case EFILE:
				local_rc = EOBJNAME;	/* e.g. file (WEF) on SC not found */
				break;
			default:
				local_rc = ESC;
		}
	}
	else
			local_rc = ESC;
		


	return (local_rc);


}				/* analyse_sca_err */





/*--------------------------------------------------------------*/
/*						                */
/* PROC  open_SC_application				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Require SC, select SC application, perform device 		*/
/*  authentication according to the additional file information,*/
/*  which are returned by function "sca_select_file()". 	*/
/*								*/
/*     1. If no SC inserted, request new SC.			*/
/*     2. Activate device key set				*/
/*	  In this version the app_id dependent device key 	*/
/*	  set is activated independed whether secure messaging 	*/
/*	  between SCT/SC is required or not.			*/						/*     3. Select application on SC, 				*/
/*           - DF-name = app_id belonging to the given app_name.*/
/*     4. Perform device authentication,			*/
/*	       No user authentication is performed!		*/	
/*     5. Set application to OPEN for current SCT (enter 	*/
/*	  app_name into the "sct_stat_list[]").			*/
/*								*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   sct_id	 	       SCT identifier			*/
/*   app_name	 	       application name			*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0			       o.k.				*/
/*   -1			       Error				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   get_appid()		Get application id for a given  */
/*				application name.		*/
/*   activate_devkeyset()	Activate DevKeySet for a certain*/
/*				application.			*/
/*   sca_get_sct_info()		Get information about SCT(s).   */
/*   sca_select_file()		Select file on the smartcard.   */
/*								*/
/*   device_authentication()    Perform device authentication 	*/
/*				according to the add. file info */
/*				of the selected SC-application.	*/
/*   enter_app_in_sctlist()	Enter information about app in  */
/*				sct_list for current SCT.       */
/*   set_fct_sec_mode()		Set security mode for comm.     */
/*				between DTE/SCT depending on the*/
/*                	        SCA-function to be called.      */
/*   aux_free2_OctetString()	Release the octets-buffer in    */
/*			    	structure OctetString		*/
/*   request_sc()		Request and initialize a 	*/
/*				smartcard.			*/
/*   reset_SCT()		Delete SCT configuration data   */
/*			        and reset SCT.	    	        */
/*                                                              */
/*   analyse_sca_err()		Analyse of an error-number 	*/
/*				returned by an SCA-IF function. */
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*--------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure open_SC_application
 *
 ***************************************************************/
#ifdef __STDC__

static int open_SC_application(
	int	  sct_id,
	char	 *app_name
)

#else

static int open_SC_application(
	sct_id,
	app_name
)
int	  sct_id;
char	 *app_name;

#endif

{

	/* Variables for the SC-IF */
	OctetString     sc_info;	/* historical characters*/
	char           *display_text;
	Boolean         alarm;
	int             time_out;	/* time_out in seconds	*/
	FileCat         file_cat;
	FileSel         file_sel;
	FileInfoReq     file_info_req;
	FileInfo        file_info;	/* return parameter of
					 * sca_select_file, */
	AddInfo		addinfo;
	ICC_SecMess     sm_SC;	/* sec. mode for communication SCT/SC	 */
	SCT_SecMess     sm_SCT;	/* sec. mode for communication DTE/SCT	 */

	/* Variables for internal use */
	Boolean         SC_in_SCT;
	SCObjEntry     *sc_obj_entry;


	char           *proc = "open_SC_application";

	secsc_errno = NOERR;
	sca_rc = 0;
	SC_in_SCT = FALSE;


#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
	fprintf(stderr, "                Application-Name: %s\n", app_name);
#endif


	/*
	 *  Check: SC in SCT? 
	 */

	sca_rc = sca_get_sct_info(sct_id);
	if (sca_rc == M_CARD_INSERTED) {
		SC_in_SCT = TRUE;	/* SC is inserted */
	} else if (sca_rc == M_NO_CARD_INSERTED)
		SC_in_SCT = FALSE;	/* no SC is inserted */
	else if (sca_rc == M_SCT_NOT_ACTIVE) {
		if (reset_SCT(sct_id)) {
			if (aux_last_error() == EDEVLOCK) 
				aux_add_error(LASTERROR, "Cannot reset SCT (SCT device is busy)", CNULL, 0, proc);
			else
				aux_add_error(LASTERROR, "Reset SCT (prior to open application) failed (e.g. SCT is not connected), sct_id:", (char *) sct_id, int_n, proc);

			return (-1);
		}
		SC_in_SCT = FALSE;	/* no SC is inserted */
	}
	else {

		secsc_errno = analyse_sca_err(sct_id, "sca_get_sct_info");	/* error */
		aux_add_error(secsc_errno, "Cannot get information about SCT, sct_id:", (char *) sct_id, int_n, proc);
		return (-1);
	}

		
	/*
	 *  If no SC inserted, request new SC.
	 */

	if (SC_in_SCT == FALSE) {
		if (request_sc(sct_id, display_text = CNULL, time_out = SC_timer)) {
			aux_add_error(LASTERROR, "Request SC (prior to open application) failed", CNULL, 0, proc);
			return (-1);
		}
	}



	/* 
	 *  Activate Device Key Set for this application
	 */

	if (activate_devkeyset(sct_id, app_name)) {
		AUX_ADD_ERROR;
		return (-1);
	}



	/* 
	 *  Next to do:		     - open application, 
	 *                           - perform device authentication,
	 *			     - set application to OPEN for current SCT.
	 */

	/* 
	 *  Get application_id belonging to the application name,
	 *  the application id is the file_name (DF) on the SC
	 */

	file_sel.file_name = get_appid(app_name);
	if (!(file_sel.file_name)) {
		aux_add_error(EINVALID, "Internal error (can't get app_id for app_name)", (char *) app_name, char_n, proc);
		return (-1);
	}


	/* set security mode for SCA-function */
	if (set_fct_sec_mode(sct_id, "sca_select_file", &sm_SC)) {
		aux_add_error(LASTERROR, "Set security mode (prior to select application on SC) failed", app_name, char_n, proc);
		return (-1);
	}


	/* select application on the SC */
	file_info.addinfo = &addinfo;
	file_info.addinfo->octets = CNULL;
	file_info.addinfo->noctets = 0;

	sca_rc = sca_select_file(sct_id,
				 file_cat = PARENT_DF,
				 &file_sel,
				 file_info_req = COMP_INFO,
				 &file_info,
				 &sm_SC	/* sec_mode for SCT/SC */
				);
	if (sca_rc < 0) {
		secsc_errno = analyse_sca_err(sct_id, "sca_select_file");
		if (secsc_errno == EOBJNAME)
			secsc_errno = EAPPNAME;
		aux_add_error(secsc_errno, "Cannot open application (app_id) on SC", (char *) sca_errmsg, char_n, proc);
		aux_free_OctetString(&file_sel.file_name);
		return (-1);
	}
	aux_free_OctetString(&file_sel.file_name);


	/* 
	 *  Perform the authentication according to the returned file_info
	 */

	if (device_authentication(sct_id, app_name, file_info.addinfo)) {
		AUX_ADD_ERROR;
		return (-1);
	}
	if (file_info.addinfo->octets != CNULL) 
		free(file_info.addinfo->octets);


	/*
	 *   Intermediate result: - The application on the SC has been selected
	 *			  - Device authentication was successful 
	 *
	 *            Next to do: set application to OPEN for the current SCT
	 */


	/* set application to OPEN for the current SCT, SW-PSE-PIN is set to CNULL */
	if (enter_app_in_sctlist(sct_id, app_name)) {
		AUX_ADD_ERROR;
		return (-1);
	}

	return (0);

}				/* end open_SC_application() */





/*--------------------------------------------------------------*/
/*						                */
/* PROC  get_pse_pin_from_SC				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*  								*/
/*  Return the PIN for the SW-PSE.				*/							/*								*/
/*  Assumption: the belonging application has been opened by 	*/
/*  the calling routine.					*/
/*								*/
/*  If the PIN is already stored in the global list		*/
/*  "sct_stat_list[sc_sel.sct_id]", this value is returned.	*/
/*  Otherwise this function reads the PIN for the SW-PSE from	*/
/*  the smartcard, returns this PIN and sets it into 		*/
/*  "sct_stat_list[]"  for the current SCT.			*/
/*								*/
/*  The SW-PSE-PIN is an SC-object, which is determined by an 	*/
/*  entry in "sc_app_list[].sc_obj_list[]".			*/
/*								*/
/*  Before reading the SW-PSE-PIN from the SC a user 		*/
/*  authentication is neccessary.				*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   app_name	 	       application name			*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   Ptr to the PIN    	       o.k			       	*/
/*   CNULL		       Error			       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sca_read_binary()		Read data from transparent 	*/
/*				WEF on the smartcard.   	*/
/*								*/
/*   aux_AppObjName2SCObj()	Get information about an SC     */
/*        			object belonging to an 		*/
/*                              application on SC. 		*/
/*   aux_cpy_String()		Copy string.			*/
/*   aux_free2_OctetString()	Release the octets-buffer in    */
/*			    	structure OctetString		*/
/*   enter_app_in_sctlist()	Enter information about app in  */
/*				sct_list for current SCT.       */
/*   prepare_sec_mess()		- Generate session key,	    	*/
/*				- Set security mode for the 	*/
/*				  communication between DTE/SCT.*/
/*   set_fct_sec_mode()		Set security mode for comm.     */
/*				between DTE/SCT depending on the*/
/*                	        SCA-function to be called.      */
/*   user_authentication()      Perform user authentication	*/
/*				(PIN).				*/
/*   write_SCT_config()		Encrypt and write SCT configuration   */
/*				data for the specified SCT.	*/
/*                                                              */
/*   analyse_sca_err()		Analyse of an error-number 	*/
/*				returned by an SCA-IF function. */
/*   aux_add_error()		Add error to error stack.	*/
/*                                                              */
/*--------------------------------------------------------------*/


/***************************************************************
 *
 * Procedure get_pse_pin_from_SC
 *
 ***************************************************************/
#ifdef __STDC__

char *get_pse_pin_from_SC(
	char	 *app_name
)

#else

char *get_pse_pin_from_SC(
	app_name
)
char	 *app_name;

#endif

{

	/* Variables for the SC-IF */
	int		sct_id;
	OctetString	ostring;
	unsigned int	offset;
	ICC_SecMess     sm_SC;	/* sec. mode for communication SCT/SC	 */
	SCT_SecMess     sm_SCT;	/* sec. mode for communication DTE/SCT	 */
	int             data_length;
	OctetString     in_data;

	/* Variables for internal use */
	SCObjEntry     *sc_obj_entry;
	char            sw_pse_pin[PSE_PIN_L];


	char           *proc = "get_pse_pin_from_SC";

	secsc_errno = NOERR;
	sca_rc = 0;


#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
	fprintf(stderr, "                   Application-Name: %s\n", app_name);
#endif

	sct_id = sc_sel.sct_id;


	/*
	 *  If the SW-PSE-PIN has already been read from the SC
	 *     return this value.
	 */

	if (strlen(sct_stat_list[sct_id].sw_pse_pin))	
		return (aux_cpy_String(sct_stat_list[sct_id].sw_pse_pin));


/*
 *   Intermediate result: The SW-PSE_PIN has not been read from the SC
 *
 *            Next to do: - Perform the user authentication, if not yet done.
 *			  - Read SW-PSE-PIN from SC.
 *			  - Set SW-PSE-PIN in the sct_stat_list for the curent SCT.
 */


	/*
	 *  Perform user authentication if not yet done.
	 */	

	if (user_authentication(sct_id, app_name, PIN)) {
		aux_add_error(LASTERROR, "User (PIN) authentication (prior to read PSE_PIN from SC) failed", (char *) app_name, char_n, proc);
		return (CNULL);
	}

	/* get information about the object PSE_PIN */
	sc_obj_entry = aux_AppObjName2SCObj(app_name, PSE_PIN_name);
	if (sc_obj_entry == NULLSCOBJENTRY) {
		aux_add_error(EINVALID, "Cannot get configuration info for SC-object", PSE_PIN_name, char_n, proc);
		return (CNULL);
	}
	if (sc_obj_entry->type == SC_KEY_TYPE) {
		aux_add_error(EINVALID, "PSE_PIN has to be a file on the SC", CNULL, 0, proc);
		return (CNULL);
	}



	/*
	 *  Intermediate result:   User authentication was successful.
	 *  Next to do:            - Set security mode for the communication DTE-SCT
	 *			   - read SW-PSE-PIN from SC 
	 */

	/* set security mode for reading an SC-object */
	sm_SCT.command = SCT_SEC_NORMAL;
	sm_SCT.response = sc_obj_entry->sm_SCT;
	if (prepare_sec_mess(sct_id, &sm_SCT, FALSE)) {
		aux_add_error(LASTERROR, "Prepare secure messaging (prior to read PSE PIN) failed", app_name, char_n, proc);
		return (CNULL);
	}

	/* read SW-PSE-PIN from SC */
	offset = 0;
	data_length = PSE_PIN_L;
	sca_rc = sca_read_binary(sct_id,
				 sc_obj_entry->sc_id,
			         offset,
			         data_length,
			         &in_data,
			         &sc_obj_entry->sm_SC_read);
	if (sca_rc < 0) {
		secsc_errno = analyse_sca_err(sct_id, "sca_read_binary");
		aux_add_error(secsc_errno, "Cannot read PSE PIN from the SC", (char *) sca_errmsg, char_n, proc);
		return (CNULL);
	}

	/* check PIN length */
	if (in_data.noctets != PSE_PIN_L) {
		aux_add_error(EPIN, "Invalid PSE-PIN read from SC", CNULL, 0, proc);
		return (CNULL);
	}
	for (i = 0; i < in_data.noctets; i++)
		sw_pse_pin[i] = in_data.octets[i];
	sw_pse_pin[i] = '\0';

	aux_free2_OctetString(&in_data);


	/* set the SW-PSE-PIN for the current SCT */
	if (strlen(sw_pse_pin))
		strcpy(sct_stat_list[sct_id].sw_pse_pin, sw_pse_pin);


	/*
	 *  Save the changed SCT configuration data
	 */

	if ((write_SCT_config(sct_id)) < 0) {
		AUX_ADD_ERROR;
		return (CNULL);
	}


#ifdef SECSCTEST
	fprintf(stderr, "PIN for SW-PSE: \n");
	aux_fxdump(stderr, sct_stat_list[sct_id].sw_pse_pin, strlen(sct_stat_list[sct_id].sw_pse_pin), 0);
	fprintf(stderr, "\n");
#endif

	return (aux_cpy_String(sct_stat_list[sct_id].sw_pse_pin));


}				/* end get_pse_pin_from_SC() */








/*--------------------------------------------------------------*/
/*						                */
/* PROC  request_sc					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Request SC with function "sca_init_icc()".			*/
/*  If this function returns ERESET (reset of the SC was not 	*/
/*  successful), the request for the SC is repeated two times.	*/
/*								*/
/*  If the request of an SC was successful, the actual SCA-IF	*/
/*  configuration data are required and written into the	*/
/*  SCT configuration file.					*/
/*								*/
/*     								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   sct_id	       	       SCT identifier			*/
/*   display_text	       Text to be displayed on the SCT-	*/
/*			       display				*/
/*   time_out		       Time_out in seconds.		*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0         	               ok			  	*/
/*   -1			       Error				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*								*/
/*   sca_init_icc()		Request and initialize a 	*/
/*				smartcard.			*/
/*   bell_function()		"Ring the bell" to require user */
/*                              input at the SCT.		*/
/*   display_on_SCT()		Display text on SCT-display.    */
/*   reset_SCT()		Delete SCT configuration data   */
/*			        and reset SCT.	    	        */
/*   save_portparam()		Get port parameter from STARMOD */
/*				and save into SCT configuration */
/*				file.				*/
/*   set_fct_sec_mode()		Set security mode for comm.     */
/*				between DTE/SCT depending on the*/
/*                	        SCA-function to be called.      */
/*                                                              */
/*   analyse_sca_err()		Analyse of an error-number 	*/
/*				returned by an SCA-IF function. */
/*   aux_add_error()		Add error to error stack.	*/
/*                                                              */
/*--------------------------------------------------------------*/


/***************************************************************
 *
 * Procedure request_sc
 *
 ***************************************************************/
#ifdef __STDC__

static int request_sc(
	int	  sct_id,
	char	 *display_text,
	int	  time_out
)

#else

static int request_sc(
	sct_id,
	display_text,
	time_out
)
int	  sct_id;
char	 *display_text;
int	  time_out;

#endif

{
	ATRInfo		atr_info;
	OctetString	atr;
	SCTPort	        sca_config_data;

	
	ICC_SecMess     sm_SC;	/* sec. mode for communication SCT/SC	 */

	int             req_attempts = 0;	/* no. of attempts to request
						 * SC	 */

	char           *proc = "request_sc";

	secsc_errno = NOERR;
	sca_rc = 0;

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	/* set security mode for SCA-function */
	if (set_fct_sec_mode(sct_id, "sca_init_icc", &sm_SC)) {
		aux_add_error(LASTERROR, "Set security mode (prior to request the SC) failed", CNULL, 0, proc);
		return (-1);
	}
	/* request SC  (if reset of the SC failed, repeat 2 times) */
	do {
		/* request new SC */
		bell_function();
		sca_rc = sca_init_icc(sct_id, 
				      display_text, 
				      time_out,
				      atr_info = NO_ATR,
				      &atr
				     );

		req_attempts++;

		if ((sca_rc < 0) && (sca_errno != ECARDINS)) {

			/*
			 *  SC not inserted
			 */

			switch (sca_errno) {

			case ERESET:
/*?????????????????????*/
/*  
 *  Hier eventuell wenn sca_init_icc mehrmals nicht geklappt hat, dann ein
 *  sca_reset(OPEN, NULL) aufrufen, um alles in den Urzustand zu versetzen.
 */
/*?????????????????????*/
				break;

			case ESCTRESET:
			case ENOCARD:
			case ECARDREM:
				/*
				 *  SC has been ejected but not removed
				 *  => reset SCT and try again.
				 */

				if (reset_SCT(sct_id)) {
					if (aux_last_error() == EDEVLOCK) 
						aux_add_error(LASTERROR, "Cannot reset SCT (SCT device is busy)", CNULL, 0, proc);
					else
						aux_add_error(LASTERROR, "Cannot reset SCT (e.g. SCT is not connected)", CNULL, 0, proc);
					return (-1);
				}
				break;
				
				
			default:
				secsc_errno = analyse_sca_err(sct_id, "sca_init_icc");
				aux_add_error(secsc_errno, "Cannot request SC / no smartcard inserted", (char *) sca_errmsg, char_n, proc);
				return (-1);

			}	/* end switch */
		}
		 /* end if */ 
		else {

			/*
			 *  SC inserted (request SC was successful)
			 *   => set SCT-display to blanks 
			 */

			display_on_SCT(sct_id, "");

			/* 
			 *  Get and save SCT configuration data from the SCA-IF
			 */

			sca_rc = save_portparam(sct_id);
			if (sca_rc < 0) {
				AUX_ADD_ERROR;
				return (-1);
			}



			return (0);
		}

	}			/* end do */
	while (req_attempts < MAX_SCRESET_FAIL);

	/* after 3 unsuccessful attempts: request SC fails */
	secsc_errno = analyse_sca_err(sct_id, "sca_init_icc");
	aux_add_error(secsc_errno, "Cannot request SC", (char *) sca_errmsg, char_n, proc);
	return (-1);

}				/* request_sc */





/*--------------------------------------------------------------*/
/*						                */
/* PROC  display_on_SCT					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Display delivered string on specified SCT.			*/
/*								*/
/*  As the SCT does not use any timer for this function, no 	*/
/*  time-out is specified.					*/
/*								*/
/*     								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   sct_id	       	       SCT identifier			*/
/*   display_text	       Text to be displayed on the SCT-	*/
/*			       display				*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sca_display()		Display text on SCT-display.    */
/*								*/
/*                                                              */
/*--------------------------------------------------------------*/


/***************************************************************
 *
 * Procedure display_on_SCT
 *
 ***************************************************************/
#ifdef __STDC__

static void display_on_SCT(
	int	  sct_id,
	char	 *display_text
)

#else

static void display_on_SCT(
	sct_id,
	display_text
)
int	  sct_id;
char	 *display_text;

#endif

{



	char           *proc = "display_on_SCT";



#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif
	sca_display(sct_id, display_text, 0);


}				/* display_on_SCT */







/*--------------------------------------------------------------*/
/*						                */
/* PROC  eject_sc					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Eject SC and set application to CLOSE for the current SCT.  */
/*  The SCT configuration data file for the specified SCT is 	*/
/*  deleted.							*/
/*								*/
/*     								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   sct_id	       	       SCT identifier			*/
/*   display_text	       Text to be displayed on the SCT-	*/
/*			       display				*/
/*   alarm		       Switch for the acoustic alarm	*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0         	               ok			  	*/
/*   -1			       Error				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sca_eject_icc()		Eject smartcard. 	        */
/*								*/
/*   bell_function()		"Ring the bell" .		*/
/*   delete_old_SCT_config()    Delete old SCT configuration    */
/*				file.				*/
/*   enter_app_in_sctlist()	Enter information about app in  */
/*				sct_list for current SCT.       */
/*   release_SCT()		Release port to SCT (initialize */
/*				communication with the SCT).	*/
/*   set_fct_sec_mode()		Set security mode for comm.     */
/*				between DTE/SCT depending on the*/
/*                	        SCA-function to be called.      */
/*                                                              */
/*   analyse_sca_err()		Analyse of an error-number 	*/
/*				returned by an SCA-IF function. */
/*   aux_add_error()		Add error to error stack.	*/
/*                                                              */
/*--------------------------------------------------------------*/


/***************************************************************
 *
 * Procedure eject_sc
 *
 ***************************************************************/
#ifdef __STDC__

static int eject_sc(
	int	  sct_id,
	char	 *display_text,
	Boolean	  alarm
)

#else

static int eject_sc(
	sct_id,
	display_text,
	alarm
)
int	  sct_id;
char	 *display_text;
Boolean	  alarm;

#endif

{

	/* Variables for the SCA-IF */
	ICC_SecMess     sm_SC;		/* sec. mode for communication SCT/SC	 */


	char           *proc = "eject_sc";

	secsc_errno = NOERR;
	sca_rc = 0;


#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	/* set security mode for SCA-function */
	if (set_fct_sec_mode(sct_id, "sca_eject_icc", &sm_SC)) {
		aux_add_error(LASTERROR, "Set security mode (prior to eject the SC) failed", CNULL, 0, proc);
		return (-1);
	}

	/* eject inserted SC */
	sca_rc = sca_eject_icc(sct_id, display_text, alarm);
	if (sca_rc < 0) {
		secsc_errno = analyse_sca_err(sct_id, "sca_eject_icc");
		aux_add_error(secsc_errno, "Cannot eject SC", (char *) sca_errmsg, char_n, proc);
		return (-1);
	}

	/* If eject SC was successful => ring the bell */
	bell_function();

	/* set application to CLOSE for the current SCT */
	if (enter_app_in_sctlist(sct_id, CNULL)) {
		AUX_ADD_ERROR;
		return (-1);
	}
			
	if (delete_old_SCT_config(sct_id)) {
		AUX_ADD_ERROR;
		return (-1);
	}

	if (release_SCT(sct_id)) {
		AUX_ADD_ERROR;
		return (-1);
	}
return (0);
}				/* eject_sc */




/*--------------------------------------------------------------*/
/*						                */
/* PROC  aux_FctName2FctPar				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Returns parameters belonging to the specified SCA-function. */
/*								*/
/*  - Global variable "sca_fct_list" contains a list of the 	*/
/*    SCA-functions and the belonging parameters.		*/
/*     								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   SCA_fct_name	       Name of the SCA-function		*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   NULL         	       No entry for "SCA_fct_name" in  	*/
/*			       "sca_fct_list".			*/
/*   SCAFctPar *	       Pointer to the parameters of the */
/*			       specified SCA-function.	       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*			         		       		*/
/*--------------------------------------------------------------*/

static
/***************************************************************
 *
 * Procedure aux_FctName2FctPar
 *
 ***************************************************************/
#ifdef __STDC__

SCAFctPar *aux_FctName2FctPar(
	char	 *SCA_fct_name
)

#else

SCAFctPar *aux_FctName2FctPar(
	SCA_fct_name
)
char	 *SCA_fct_name;

#endif

{
	register SCAFctPar *f = &sca_fct_list[0];
	char           *proc = "aux_FctName2FctPar";


	if (!SCA_fct_name)
		return ((SCAFctPar *) 0);

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
	fprintf(stderr, "                Funktions-Name: %s\n", SCA_fct_name);
#endif

	while (f->fct_name) {
		if (strcmp(SCA_fct_name, f->fct_name) == 0)
			return (f);
		f++;
	}
	return ((SCAFctPar *) 0);


}				/* end aux_FctName2FctPar */




/*--------------------------------------------------------------*/
/*						                */
/* PROC  aux_AppObjName2SCObj				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Returns parameters belonging to the specified SC_object     */
/*  which belongs to the specified SC-application.		*/
/*								*/
/*  - Global variable "sc_app_list" contains a list of the 	*/
/*    applications available on the SC.				*/
/*    Part of "sc_app_list" is "sc_obj_list" which contains  	*/
/*    depending on the application a list of the objects (incl. */
/*    parameters), which shall be stored on the SC or which are */
/*    stored on the SC.						*/
/*     								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   app_name		       Name of the application 		*/
/*   obj_name		       Name of the object 		*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   NULL         	       No entry for "app_name" in  	*/
/*			       "sc_app_list" or 		*/
/*              	       no entry for "obj_name" in  	*/
/*			       "sc_obj_list".			*/
/*   SCObjEntry *	       Pointer to the parameters of the */
/*			       specified SC-Object.	       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*			         		       		*/
/*--------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure aux_AppObjName2SCObj
 *
 ***************************************************************/
#ifdef __STDC__

SCObjEntry *aux_AppObjName2SCObj(
	char	 *app_name,
	char	 *obj_name
)

#else

SCObjEntry *aux_AppObjName2SCObj(
	app_name,
	obj_name
)
char	 *app_name;
char	 *obj_name;

#endif

{
	register SCAppEntry *a = &sc_app_list[0];
	SCObjEntry     *o;

	char           *proc = "aux_AppObjName2SCObj";


	if ((!app_name) || (!obj_name))
		return (NULLSCOBJENTRY);

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: aux_AppObjName2SCObj \n");
	fprintf(stderr, "                Application-Name: %s\n", app_name);
	fprintf(stderr, "                Object-Name: %s\n", obj_name);
#endif

	while (a->app_name) {
		if (strcmp(app_name, a->app_name) == 0) {
			/* get obj */
			o = &a->sc_obj_list[0];

			while (o->name) {
				if (strcmp(obj_name, o->name) == 0)
					return (o);
				o++;
			}
		}		/* end if */
		a++;
	}			/* end while */

	return (NULLSCOBJENTRY);

}				/* end aux_AppObjName2SCObj */




/*--------------------------------------------------------------*/
/*						                */
/* PROC  aux_AppName2SCApp				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Returns parameters belonging to the specified 		*/
/*  SC_application, if the specified application is available   */
/*  on the SC.							*/
/*								*/
/*  - Global variable "sc_app_list" contains a list of the 	*/
/*    applications available on the SC.				*/
/*     								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   app_name		       Name of the application 		*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   NULL         	       No entry for "app_name" in  	*/
/*			       "sc_app_list".			*/
/*   SCAppEntry *	       Pointer to the parameters of the */
/*			       specified SC-Application	       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*			         		       		*/
/*--------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure aux_AppName2SCApp
 *
 ***************************************************************/
#ifdef __STDC__

SCAppEntry *aux_AppName2SCApp(
	char	 *app_name
)

#else

SCAppEntry *aux_AppName2SCApp(
	app_name
)
char	 *app_name;

#endif

{
	register SCAppEntry *a = &sc_app_list[0];
	char           *proc = "aux_AppName2SCApp";

	if (!app_name)
		return (NULLSCAPPENTRY);

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: aux_AppName2SCApp \n");
	fprintf(stderr, "                Application-Name: %s\n", app_name);
#endif

	while (a->app_name) {
		if (strcmp(app_name, a->app_name) == 0)
			return (a);
		a++;
	}
	return (NULLSCAPPENTRY);

}				/* end aux_AppName2SCApp */




/*--------------------------------------------------------------*/
/*						                */
/* PROC  get_appid					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Get application identifier for a given application name and */
/*  return a copy.						*/
/*								*/
/*  - Global variable "sc_app_list" contains a list of the 	*/
/*    applications available on the SC.				*/
/*     								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   app_name		       Name of the application 		*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   NULLOCTETSTRING  	       - Missing app_name		*/
/*			       - No entry for "app_name" in  	*/
/*			         "sc_app_list"			*/
/*			       - Missing app_id in "sc_app_list"*/
/*   OctetString *	       Pointer to the app_id		*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   aux_AppName2SCApp()	Get information about an SC app.*/
/*   aux_new_OctetString()	Create new OctetString.		*/
/*			         		       		*/
/*--------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure get_appid
 *
 ***************************************************************/
#ifdef __STDC__

static OctetString *get_appid(
	char	 *app_name
)

#else

static OctetString *get_appid(
	app_name
)
char	 *app_name;

#endif

{
	SCAppEntry     *sc_app_entry;
	OctetString    *app_id;
	unsigned int	i;


	char           *proc = "get_appid";

	if (!app_name)
		return (NULLOCTETSTRING);

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s \n", proc);
	fprintf(stderr, "                Application-Name: %s\n", app_name);
#endif

	sc_app_entry = aux_AppName2SCApp(app_name);
	if (sc_app_entry == NULLSCAPPENTRY) 
		return 	(NULLOCTETSTRING);
	
	if ((sc_app_entry->app_id == NULLOCTETSTRING) || 
	    (sc_app_entry->app_id->noctets == 0) ||
	    (sc_app_entry->app_id->octets == CNULL))
		return 	(NULLOCTETSTRING);
	else {
		/*
		 * Make a copy of the app_id and return the copy
                 */
		app_id = aux_new_OctetString(sc_app_entry->app_id->noctets);
		if (app_id == NULLOCTETSTRING) {
			aux_add_error(EMALLOC, "Copy of app_id", CNULL, 0, proc);
			return (NULLOCTETSTRING);
		}
		app_id->noctets = sc_app_entry->app_id->noctets;
		for (i = 0; i < sc_app_entry->app_id->noctets; i++)
			app_id->octets[i] = sc_app_entry->app_id->octets[i];
	}

#ifdef SECSCTEST
	fprintf(stderr, "	       \nAPPLICATION_ID=:\n");
	fprintf(stderr, "	       ");
	aux_fxdump(stderr, app_id->octets, app_id->noctets, 0);
	fprintf(stderr, "\n");
#endif

	return (app_id);

}				/* end get_appid */




/*--------------------------------------------------------------*/
/*						                */
/* PROC  enter_app_in_sctlist				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Enter the given application name into "sct_stat_list" for	*/
/*  the specified SCT. 						*/
/*  The values of "user_auth_done", "sw_pse_pin" and "sm_SCT" 	*/
/*  are set to initial values.					*/
/*								*/
/*  To delete an application from this list, the input parameter*/
/*  "app_name" must be set to CNULL.				*/
/*								*/
/*								*/
/*  - Global variable "sct_stat_list[sct_id]" contains 		*/
/*    current information for the SCT specified by sct_id.	*/
/*     								*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   sct_id		       SCT identifier			*/
/*   app_name		       Name of the application to be    */
/*			       set in "sct_stat_list". 		*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0			       ok				*/
/*   -1		               Error			      	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   write_SCT_config()		Encrypt and write SCT 		*/
/*			        configuration data for the 	*/
/*			        specified SCT.			*/
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*--------------------------------------------------------------*/


/***************************************************************
 *
 * Procedure enter_app_in_sctlist
 *
 ***************************************************************/
#ifdef __STDC__

static int enter_app_in_sctlist(
	int	  sct_id,
	char	 *app_name
)

#else

static int enter_app_in_sctlist(
	sct_id,
	app_name
)
int	  sct_id;
char	 *app_name;

#endif

{

	char           *proc = "enter_app_in_sctlist";


#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	/* check the given sct_id */
	if ((sct_id >= MAX_SCTNO) || (sct_id <= 0)) {
		aux_add_error(EINVALID, "Invalid value for sct_id", CNULL, 0, proc);
		return (-1);
	}
	/* check the given app_name */
	if (app_name) {
		if (strlen(app_name) > MAXL_APPNAME) {
			aux_add_error(EAPPNAME, "application name too long", CNULL, 0, proc);
			return (-1);
		}
	}


	/*
	 *  Set parameters to their initial values
	 */

	sct_stat_list[sct_id].user_auth_done = FALSE;
	strcpy(sct_stat_list[sct_id].sw_pse_pin, "");
	sct_stat_list[sct_id].sm_SCT.command  = SCT_SEC_NORMAL;
	sct_stat_list[sct_id].sm_SCT.response = SCT_SEC_NORMAL;


	if (app_name)
		strcpy(sct_stat_list[sct_id].app_name, app_name);
	else
		strcpy(sct_stat_list[sct_id].app_name, "");


#ifdef SECSCTEST
	fprintf(stderr, "                app_name in sct_stat_list[%d]: %s\n", sct_id, &sct_stat_list[sct_id].app_name[0]);
#endif


	/*
	 *  Save the changed SCT configuration data
	 */

	if ((write_SCT_config(sct_id)) < 0) {
		AUX_ADD_ERROR;
		return (-1);
	}


	return (0);

}				/* end enter_app_in_sctlist */





/*--------------------------------------------------------------*/
/*						         	*/
/* PROC  is_SC_app_open					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*  This function returns whether an application on the SC is 	*/
/*  open.							*/
/*								*/
/*  1. Check whether SC in SCT!					*/
/*     if no SC is inserted => return(FALSE)			*/
/*  2. Check whether application on the SC has been opened via  */
/*     the specified SCT. 					*/
/*     If "app_name" is set to NULL, this function checks,      */
/*        whether any application has been opened.		*/
/*     If "app_name" is not set to NULL, this function checks 	*/
/*        whether this application has been opened.		*/
/*								*/
/*  An application is open, if its name has been set in the 	*/
/*  "sct_stat_list" for this SCT. 				*/
/*								*/
/*  - Global variable "sct_stat_list[sct_id]" contains 		*/
/*    current application name.					*/
/*     								*/
/*  Observe that:						*/
/*  If another application is open an error (EAPPNAME) is 	*/
/*  returned.							*/
/*								*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   sct_id		       SCT identifier			*/
/*   app_name		       Name of the application or NULL 	*/
/*			       for any application.		*/
/*								*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   TRUE		       The specified application or any */
/*			       application (if app_name = CNULL)*/
/*			       is open. 			*/
/*   FALSE		       No application on the SC is open.*/
/*			       e.g. no SC inserted.		*/
/*   -1		               error				*/
/*			       e.g. another application is open.*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sca_get_sct_info()		Get information about SCT(s).   */
/*--------------------------------------------------------------*/


/***************************************************************
 *
 * Procedure is_SC_app_open
 *
 ***************************************************************/
#ifdef __STDC__

static int is_SC_app_open(
	int	  sct_id,
	char	 *app_name
)

#else

static int is_SC_app_open(
	sct_id,
	app_name
)
int	  sct_id;
char	 *app_name;

#endif

{


	char           *proc = "is_SC_app_open";

	sca_rc = 0;


#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
	if (app_name)
		fprintf(stderr, "                Application name: %s\n", app_name);
	else
		fprintf(stderr, "                no app_name specified:\n");
#endif

	/* check the current sct_id */
	if ((sct_id >= MAX_SCTNO) || (sct_id <= 0)) {
		aux_add_error(EINVALID, "Invalid value for sct_id", CNULL, 0, proc);
		return (-1);
	}

	/* check whether SC is inserted */
	sca_rc = sca_get_sct_info(sct_id);
	if (sca_rc < 0) {
		secsc_errno = analyse_sca_err(sct_id, "sca_get_sct_info");	/* error */
		aux_add_error(secsc_errno, "Cannot get information about SCT, sct_id:", (char *) sct_id, int_n, proc);
		return (-1);
	}

	if (sca_rc == M_NO_CARD_INSERTED)
		return (FALSE);		/* SC is not inserted => no app open */
	else {
		if (sca_rc == M_SCT_NOT_ACTIVE) {
			if (reset_SCT(sct_id)) {
				if (aux_last_error() == EDEVLOCK) 
					aux_add_error(LASTERROR, "Cannot reset SCT (SCT device is busy)", CNULL, 0, proc);
				else
					aux_add_error(LASTERROR, "Reset SCT failed (e.g. SCT is not connected), sct_id:", (char *) sct_id, int_n, proc);

				return (-1);
			}
			return (FALSE);		/* SC is not inserted => no app open */
		}
	}



	/* If no application name has been entered in global list, no application is open */
	if ((!sct_stat_list[sct_id].app_name) ||
	    (!(strlen(sct_stat_list[sct_id].app_name)))) {
		return (FALSE);
	}


/*
 *   Intermediate result: An application is open for the current SCT:
 *
 *            Next to do: If app_name  = NULL => any application is open => return(0).
 *			  If app_name != NULL => compare this name with
 *						 the name in the sct_stat_list.
 */


	if (!app_name) {
		return (TRUE);
	}
	else {
		if (strcmp(app_name, sct_stat_list[sct_id].app_name) == 0)
			return (TRUE);
		else {
			aux_add_error(EAPPNAME, "Another SC application is open", sct_stat_list[sct_id].app_name, char_n, proc);
			return (-1);
		}
	}

}				/* end is_SC_app_open */





/*--------------------------------------------------------------*/
/*						         	*/
/* PROC  handle_SC_app					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  The calling routine has checked that the application is an	*/
/*  SC application.						*/
/*								*/
/*  If the application on the SC has not been opened via the    */
/*  actual SCT (sc_sel.sct_id) and no other application on the  */
/*  SC is open, the specified application will be opened.	*/
/*								*/
/*  Observe that: 					        */
/*  If another application was open, the SC will automatically  */
/*  be ejected.					                */
/*								*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   pse_sel		       Structure which identifies the 	*/
/*			       PSE object.			*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0			       Application has been opened	*/
/*   -1		               Application could not been 	*/
/*			       opened.				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   open_SC_application() 	Require SC, open SC application,*/
/*				perform device authenticationn. */
/*   is_SC_app_open()		Return whether application has  */
/*				been opened.                    */
/*--------------------------------------------------------------*/


/***************************************************************
 *
 * Procedure handle_SC_app
 *
 ***************************************************************/
#ifdef __STDC__

int handle_SC_app(
	char	 *app_name
)

#else

int handle_SC_app(
	app_name
)
char	 *app_name;

#endif

{

	SCObjEntry     *sc_obj_entry;
	int		rc;
	unsigned int	sct_id;

	char           *proc = "handle_SC_app";
	secsc_errno = NOERR;


#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
	if (app_name)
		fprintf(stderr, "                Application name: %s\n", app_name);
	else
		fprintf(stderr, "                no app_name specified:\n");
#endif


	sct_id = sc_sel.sct_id;

	/* has the application been opened? */
	rc = is_SC_app_open(sct_id, app_name);
	if (rc == -1) {
		AUX_ADD_ERROR;
		if (aux_last_error() == EAPPNAME) {
			/*
			 *  Another SC-application is open, eject SC
			 */

			fprintf(stderr, "\nAnother SC-application is open, the SC will be ejected\n\n");
		        eject_sc(sct_id, CNULL, FALSE);
			return (-1);
		}
		return (-1);
	}

	if (rc == FALSE) {

		/* application not open => open it */

		if (open_SC_application(sct_id, app_name)) {
			aux_add_error(EAPPNAME, LASTTEXT, app_name, char_n, proc);
			return (-1);
		}
	}

	return (0);

}				/* end handle_SC_app */





/*--------------------------------------------------------------*/
/*						         	*/
/* PROC  handle_key_sc_app				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*   Handle SC application for specified key, if key is a key   */
/*   on the SC.							*/
/*      If key shall be installed on the  SC,			*/
/*         => Case 1: Selection of key with object name:	*/
/*                    Assumption: the belonging application has */
/*		      been opened by the calling routine.	*/
/*         => Case 2: Selection of key with key reference:	*/
/*		      Check whether any application is 		*/
/*		      open. If not, return(error).		*/
/*         => Case 3: return(error).				*/
/*								*/
/*  If a key on the SC shall be accessed, the user 		*/
/*  authentication is performed.				*/
/*     								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   sct_id		       SCT identifier			*/
/*   key		       Structure which identifies the   */
/*			       key.				*/
/*   key_id		       Pointer to key identifier used 	*/
/*			       at the SCA-IF			*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0			        ok				*/
/*   -1		                error				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   is_SC_app_open()		Return whether application has  */
/*				been opened.                    */
/*   user_authentication()      Perform user authentication	*/
/*				(PIN).				*/
/*			         		       		*/
/*   analyse_sca_err()		Analyse of an error-number 	*/
/*				returned by an SCA-IF function. */
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*--------------------------------------------------------------*/


/***************************************************************
 *
 * Procedure handle_key_sc_app
 *
 ***************************************************************/
#ifdef __STDC__

static int handle_key_sc_app(
	int	  sct_id,
	Key	 *key,
	KeyId	 *key_id
)

#else

static int handle_key_sc_app(
	sct_id,
	key,
	key_id
)
int	  sct_id;
Key	 *key;
KeyId	 *key_id;

#endif

{

	int		rc;

	char           *proc = "handle_key_sc_app";


#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif


	if ((key->keyref == 0) && (key->pse_sel != (PSESel * ) 0)) {

		/*
		 * Select key with object name,
		 *  the belonging application has been opened in the calling routine
		 */

	} else {
		if (key->keyref != 0) {

			/*
			 * Select key with keyref
			 */

			/*
			 *  key level != FILE_DUMMY means that key is stored in the SC 
			 */
			if (key_id->key_level != FILE_DUMMY) {
				rc = is_SC_app_open(sct_id, CNULL);
				if (rc != TRUE) {
					if (rc == FALSE)
						aux_add_error(EAPPNAME, "No application is open (Cannot access a key on the SC)!", CNULL, 0, proc);
					else
						AUX_ADD_ERROR;
					return (-1);
				}
			}
		} else {
			aux_add_error(EINVALID, "Key missing!", CNULL, 0, proc);
			return (-1);
		}		/* end else */

	}			/* end else */



	/*
	 *  If a key on the SC shall be accessed, the user authentication is required.
	 */	

	/*
	 *  key level != FILE_DUMMY means that key is stored in the SC 
	 */
	if (key_id->key_level != FILE_DUMMY) {

		if ((key->pse_sel != (PSESel * ) 0) &&
		    (key->pse_sel->app_name)) {
			if (user_authentication(sct_id, key->pse_sel->app_name, PIN)) {
				aux_add_error(LASTERROR, "User (PIN) authentication (prior to an access to a key on the SC) failed", (char *) key->pse_sel, PSESel_n, proc);
				return (-1);
			}
		}
		else {
			/*
			 * key selected with keyref
			 */

			if (user_authentication(sct_id, sct_stat_list[sct_id].app_name, PIN)) {
				aux_add_error(LASTERROR, "User (PIN) authentication (prior to an access to a key on the SC) failed", (char *) sct_stat_list[sct_id].app_name, char_n, proc);
				return (-1);
			}

		}

	}
		

	return (0);

}				/* end handle_key_sc_app */



/*--------------------------------------------------------------*/
/*						                */
/* PROC  SC_configuration				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Read the SC configuration file, if not yet done.	        */
/*  This function returns whether an SC configuration file      */
/*  could be successfully read (TRUE | FALSE).			*/ 
/*								*/
/*								*/
/*  First this function tries to read an SC configuration file  */
/*  stored under the home directory of the user.		*/
/*  If no configuration file exists under this directory, this  */
/*  function tries to read one under a system directory.	*/
/*								*/
/*  If no configuration file could be found, FALSE is returned. */
/*  In this case the global list "sc_app_list[]" is left 	*/
/*  unchanged.							*/
/*								*/
/*								*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   TRUE		       SC is available and 		*/
/*			       configuration was successful.	*/
/*   FALSE		       No SC configuration file found   */
/*			       => SC not available.		*/
/*  -1			       Error			       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   display_SC_configuration() Display the actual SC           */
/*				configuration ("sc_app_list[]").*/
/*   read_SC_configuration()   read SC-configuration file	*/
/*                                                              */
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure SC_configuration
 *
 ***************************************************************/
#ifdef __STDC__

int SC_configuration(
)

#else

int SC_configuration(
)

#endif

{
	static Boolean  config_done = FALSE;
	static	int	SC_available = FALSE;

	int             rc;

	char           *proc = "SC_configuration";


#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	if (config_done == FALSE) {

		rc = read_SC_configuration(USER_CONF);
		if (rc < 0) {
			AUX_ADD_ERROR;	
			return (-1);
		}
		if ((rc == 0) && (sc_app_list[0].app_name == CNULL)) {

			/*
			 * There is no SC configuration file under the user directory 
	                 *   => take SC configuration file under the system dirctory
			 */

			rc = read_SC_configuration(SYSTEM_CONF);
			if (rc < 0) {
				AUX_ADD_ERROR;	
				return (-1);
			}
		       if ((rc == 0) && (sc_app_list[0].app_name != CNULL)) 
			 	SC_available = TRUE;

		}
		else
		       SC_available = TRUE;
			

#ifdef SECSCTEST
		if (sc_app_list[0].app_name == CNULL) 
			fprintf(stderr, "\n              No SC configuration file found => no smartcard available\n\n");
		else 
			display_SC_configuration();
#endif

		config_done = TRUE;

	}  /* end if (config_done == FALSE) */

	return (SC_available);


}  			/* end SC_configuration */






/*--------------------------------------------------------------*/
/*						         	*/
/* PROC  get_connected_SCT				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Get the first SCT (sct_id) of the registered SCTs, which is */
/*  actually connected to the DTE.				*/
/*								*/
/*  An SCT is registered, if there is an entry for this SCT in  */
/*  the file whose name is set in the global environment 	*/
/*  variable STAMOD.						*/
/*  								*/
/*  An SCT is connected, if the function "sca_display" for this */
/*  SCT is successful.						*/
/*							        */
/*  If  - the SCT-file doesn't exist,				*/
/*      - no SCTs are registered,				*/
/*      - no SCT is connected to the DTE,			*/
/*  => return(-1)						*/
/*     								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   sct_id (>0)	       Id. of the connected SCT.	*/
/*   -1		               No SCT connected.		*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sca_display()		Display text on SCT-display.    */
/*   sca_get_sct_info()		Get information about  		*/
/*				registered SCTs.		*/
/*--------------------------------------------------------------*/


/***************************************************************
 *
 * Procedure get_connected_SCT
 *
 ***************************************************************/
#ifdef __STDC__

static int get_connected_SCT(
)

#else

static int get_connected_SCT(
)

#endif

{


	int             no_of_SCTs;
	int             count_SCTs;

	char           *proc = "get_connected_SCT";

	secsc_errno = NOERR;
	sca_rc = 0;


#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	/* get number of registered SCTs */

	no_of_SCTs = sca_get_sct_info(0);

	if (no_of_SCTs < 1)
		return (-1);


	/* check registered SCTs */

	for (count_SCTs = 1; count_SCTs <= no_of_SCTs; count_SCTs++) {

		sca_rc = sca_display(count_SCTs, SCT_TEXT_SCT_CHECK, 3);

		if (sca_rc >= 0) {

			/* SCT is connected */
			return (count_SCTs);
		}
	}

	return (-1);

}				/* end get_connected_SCT */





/*--------------------------------------------------------------*/
/*						         	*/
/* PROC  release_SCT					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Release(close) port to the specified SCT to give another	*/
/*  process the possibility to access this SCT.			*/
/*								*/
/*  If the actual process wants again to access this SCT, the	*/
/*  SCT configuration will be done again.			*/
/*								*/
/*								*/
/*  1) Check value of sct_id.					*/
/*								*/
/*  2) Set configuration done to "not done" for the specified	*/
/*     SCT.							*/
/*								*/
/*  3) Call "sca_reset()" to close port to the specified SCT.	*/
/*  								*/
/*     								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0			       ok				*/
/*   -1		               error				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sca_reset()		Reset communication between DTE */
/*				and SCT.		        */
/*--------------------------------------------------------------*/


/***************************************************************
 *
 * Procedure release_SCT
 *
 ***************************************************************/
#ifdef __STDC__

int release_SCT(
	int	  sct_id
)

#else

int release_SCT(
	sct_id
)
int	  sct_id;

#endif


{

	int	rc;


	char           *proc = "release_SCT";

	secsc_errno = NOERR;
	sca_rc = 0;


#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif


	/* check the specified sct_id */
	if ((sct_id >= MAX_SCTNO) || (sct_id <= 0)) {
		aux_add_error(EINVALID, "Invalid value for sct_id", CNULL, 0, proc);
		return (-1);
	}


	sct_stat_list[sct_id].config_done = FALSE;


	rc = sca_reset(sct_id, 
		       COM_CLOSE,
		       (SCTPort *) 0);
	if (rc < 0) {
		secsc_errno = analyse_sca_err(sct_id, "sca_reset");
		aux_add_error(secsc_errno, "Cannot reset SCT (COM_CLOSE)", (char *) sca_errmsg, char_n, proc);
		return (-1);
	}


	return (0);

}				/* end release_SCT */





/*--------------------------------------------------------------*/
/*						         	*/
/* PROC  reset_SCT					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Delete SCT configuration data and reset SCT.	        */
/*								*/
/*								*/
/*  1) Check value of sct_id.					*/
/*								*/
/*  2) Delete old SCT configuration data.			*/
/*								*/
/*  2) Call "sca_reset()" to reset port to the specified SCT 	*/
/*     and the SCT itself.					*/
/*     An inserted SC will be ejected.				*/
/*								*/
/*  3) Call "save_portparam()" to get and save SCT communication*/
/*     parameters.						*/
/*								*/
/*  4) Reset secure messaging (DTE/SCT).			*/
/*     								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0			       ok				*/
/*   -1		               error				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   delete_old_SCT_config()    Delete old SCT configuration    */
/*				file.				*/
/*   init_SCT_config()		Initialize SCT configuration    */
/*				list.				*/
/*   prepare_sec_mess()		- Generate session key,	    	*/
/*				- Set security mode for the 	*/
/*				  communication between DTE/SCT.*/
/*   save_portparam()		Get port parameter from STARMOD */
/*				and save into SCT configuration */
/*				file.				*/
/*   sca_reset()		Reset communication between DTE */
/*				and SCT.		        */
/*--------------------------------------------------------------*/


/***************************************************************
 *
 * Procedure reset_SCT
 *
 ***************************************************************/
#ifdef __STDC__

int reset_SCT(
	int	  sct_id
)

#else

int reset_SCT(
	sct_id
)
int	  sct_id;

#endif


{

	int	rc;
	SCTPort	        sca_config_data;
	SCT_SecMess 	sm_SCT;



	char           *proc = "reset_SCT";

	secsc_errno = NOERR;
	sca_rc = 0;


#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif


	/* check the specified sct_id */
	if ((sct_id >= MAX_SCTNO) || (sct_id <= 0)) {
		aux_add_error(EINVALID, "Invalid value for sct_id", CNULL, 0, proc);
		return (-1);
	}

	init_SCT_config(sct_id);	/* initialize the internal SCT 
					   configuration data */

	if (delete_old_SCT_config(sct_id)) {
		AUX_ADD_ERROR;
		return (-1);
	}


	/*
	 *  Reset (initialize SCT)
	 */

	sca_rc = sca_reset(sct_id, COM_OPEN, (SCTPort *) 0);
	if (sca_rc < 0) {
		secsc_errno = analyse_sca_err(sct_id, "sca_reset");
		if (secsc_errno == EDEVLOCK) 
			aux_add_error(secsc_errno, "Cannot reset SCT (SCT device is busy)", CNULL, 0, proc);
		else
			aux_add_error(secsc_errno, "Cannot reset SCT (e.g. SCT is not connected)",  CNULL, 0, proc);
		return (-1);
	}
	
	
	/* 
	 *  Get and save SCT configuration data from the SCA-IF
	 */

	sca_rc = save_portparam(sct_id);
	if (sca_rc < 0) {
		AUX_ADD_ERROR;
		return (-1);
	}

	/*
	 *  Reset secure messaging (DTE/SCT)
	 */

	sm_SCT.command = SCT_SEC_NORMAL;
	sm_SCT.response = SCT_SEC_NORMAL;
	if (prepare_sec_mess(sct_id, &sm_SCT, TRUE)) {
		aux_add_error(LASTERROR, "Reset secure messaging (DTE/SCT) failed", CNULL, 0, proc);
		return (-1);
	}


	return (0);

}				/* end reset_SCT */




/*--------------------------------------------------------------*/
/*						         	*/
/* PROC  SCT_configuration				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Perform the configuration for the selected SCT and check 	*/
/*  whether SCT is available:					*/
/*								*/
/*								*/
/*  1) Check value of sct_id (global variable: sc_sel.sct_id).	*/
/*								*/
/*  2) If this function is called for the first time, the	*/
/*     "sct_stat_list[]" is initialized.			*/
/*								*/
/*  3) If the configuration for the specified SCT has not been 	*/
/*     done:							*/
/*     -  Perform SCT configuration (inclusive check whether SCT*/
/*	  is available):					*/
/*								*/
/*	  If SCT configuration data of a previous process are	*/
/*	  available, the internal program status reached by the */
/*	  previous process will be restored.			*/
/*	  Otherwise the communication with SCT/SC and the 	*/
/*	  internal program status will be resetted to initial	*/
/*	  values.						*/
/*								*/
/*  4) Return whether SCT is available (TRUE | FALSE)		*/ 
/*  								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   TRUE		       SCT is available and 		*/
/*			       configuration was successful.	*/
/*   FALSE		       SCT is not available. 		*/
/*   -1		               error during SCT configuration	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   init_SCT_config()		Initialize SCT configuration    */
/*				list.				*/
/*   perform_SCT_config()	Restore or reset (init) SCT	*/
/*				configuration.			*/
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure SCT_configuration
 *
 ***************************************************************/
#ifdef __STDC__

int SCT_configuration(
)

#else

int SCT_configuration(
)

#endif

{

	unsigned int    sct_id;
	static Boolean	first = TRUE;
	int		SCT_available;

	char            *proc = "SCT_configuration";


	secsc_errno = NOERR;
	sct_id = sc_sel.sct_id;


#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
	if (sct_stat_list[sct_id].config_done == FALSE)
		fprintf(stderr, "                config_done = FALSE\n");
	else	fprintf(stderr, "                config_done = TRUE\n");
	if (sct_stat_list[sct_id].available == FALSE)
		fprintf(stderr, "                available = FALSE\n");
	else	fprintf(stderr, "                available = TRUE\n");

#endif


	/* check the specified sct_id */
	if ((sct_id >= MAX_SCTNO) || (sct_id <= 0)) {
		aux_add_error(EINVALID, "Invalid value for sct_id", CNULL, 0, proc);
		return (-1);
	}



	if (first == TRUE) {
		init_SCT_config(0);	/* initialize the SCT configuration */
		first = FALSE;
	}



	if (sct_stat_list[sct_id].config_done == FALSE)  {
	
		/*
		 *  Configuration for the specified SCT has not been done.
	 	 *
	         *  => Perform SCT configuration
		 *        
	         */

		if ((SCT_available = perform_SCT_config (sct_id)) == -1) {
			AUX_ADD_ERROR;
			sct_stat_list[sct_id].available = FALSE;
			sct_stat_list[sct_id].config_done = FALSE;
			return (-1);
		}

		sct_stat_list[sct_id].available = SCT_available;

		sct_stat_list[sct_id].config_done = TRUE;

	} /* end SCT configuration not yet done */


#ifdef SECSCTEST
	if (sct_stat_list[sct_id].available == TRUE)
		fprintf(stderr, "SCT with sct_id = %d is available\n", sct_id);
	else    fprintf(stderr, "SCT with sct_id = %d is not available\n", sct_id);
#endif

	return(sct_stat_list[sct_id].available);

}				/* end SCT_configuration */



/*--------------------------------------------------------------*/
/*						         	*/
/* PROC  get_process_key				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Compose process key and return pointer to the process key. 	*/
/*??????????????????????????????????????????????????????????????*/
/* Diesen festen String ersetzen durch einen vom Installierer bestimmten
geheimen code und Benutzer spezifischen Daten, entweder konkateniert
man diese Strings oder verschluesselt den einen Wert mit dem anderen.
Wichtig ist, dass der vom Installierer vorgegebene Code nicht im Klartext
im Programm auftaucht, sondern beim compilieren eingelesen wird.
*/
/*??????????????????????????????????????????????????????????????*/
/*							        */
/*  The allocated storage has to be released by the calling 	*/
/*  routine.							*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*							        */
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   CNULL		       error				*/
/*   != CNULL		       pointer to process key	 	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   aux_cpy_String()		Copy string.			*/
/*			         		       		*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure get_process_key
 *
 ***************************************************************/
#ifdef __STDC__

static char *get_process_key(
)

#else

static char *get_process_key(
)

#endif

{

	static char		*process_key = CNULL;

	char           *proc = "get_process_key";


#ifdef SECSCTEST
/*	fprintf(stderr, "SECSC-Function: %s\n", proc); */
#endif

	if (!process_key) {

		if (!(process_key = (char *) malloc(MAX_LEN_PROC_KEY))) {
			aux_add_error(EMALLOC, "process key", CNULL, 0, proc);
			return (CNULL);
		}

		strcpy(process_key, get_unixname());
		sprintf(process_key + strlen(process_key), "%d", 3 * getuid() - 100);
		strcat(process_key, ".&%)#(#$");

	}

	return (aux_cpy_String(process_key));

}				/* end get_process_key */




/*--------------------------------------------------------------*/
/*						         	*/
/* PROC  init_SCT_config				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*   Initialize the whole or one entry of the SCT configuration */
/*   list.							*/
/*								*/
/*   The configuration data for the STARMOD modul are not 	*/
/*   initialized.						*/
/*							        */
/*							        */
/* IN			     DESCRIPTION		       	*/
/*	sct_id			= 0 means: the whole list is	*/
/*				           initialized.		*/
/*				> 0 means: the entry for this   */
/*					   SCT is initialized.	*/
/*							        */
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*			         		       		*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure init_SCT_config
 *
 ***************************************************************/
#ifdef __STDC__

static void init_SCT_config( 
	int	  sct_id
)

#else

static void init_SCT_config(
	sct_id
)
int	  sct_id;


#endif

{

	unsigned int id;
	unsigned int max_id;

	char           *proc = "init_SCT_config";


#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif


	if (sct_id == 0) {
		id = 0;
		max_id = MAX_SCTNO;
	}
	else {
		id = sct_id;
		max_id = sct_id;
	}

	for (; id <= max_id; id++) {
		sct_stat_list[id].config_done     	= FALSE;
		sct_stat_list[id].available       	= FALSE;
		sct_stat_list[id].sm_SCT.command  	= SCT_SEC_NORMAL; 
		sct_stat_list[id].sm_SCT.response 	= SCT_SEC_NORMAL; 
		strcpy(sct_stat_list[id].app_name, "");
		strcpy(sct_stat_list[id].sw_pse_pin, "");
		sct_stat_list[id].user_auth_done  	= FALSE;

	}


}				/* end init_SCT_config */




/*--------------------------------------------------------------*/
/*						         	*/
/* PROC  perform_SCT_config				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*							       	*/
/*  Perform SCT_configuration					*/
/*							       	*/
/*							       	*/
/*  TRUE will be returned, if					*/
/*      - sct_id correct,					*/
/*      - SCT configuration was successful and			*/
/*      - function "sca_display" for this SCT was successful.	*/
/*								*/
/*  FALSE will be returned, if					*/
/*	- the file specified by the global environment variable */
/*        STAMOD doesn't exist,					*/
/*      - no SCTs are registered,				*/
/*	- the specified sct_id is greater than the no. of 	*/
/*        registered SCTs or					*/
/*	- "sca_reset(init)" returns error != EDEVBUSY		*/
/*     								*/
/*  -1 will be returned, if 					*/
/*      - "sca_reset()" returns error == EDEVBUSY, 		*/
/*      - "sca_display()" was not successful and the error 	*/
/*        number of the SCA-IF (sca_errno) is set to EDEVBUSY.  */
/*        This means that the device for the SCT could not be   */
/*        opened (device busy).					*/
/*	- "sca_reset(restore)" returns error 			*/
/*								*/
/*							       	*/
/*   1. Check sct_id						*/
/*								*/
/*   2. Restore or reset (initialize) SCT configuration:	*/
/*      If SCT configuration data of a previous process are	*/
/*      available, the internal program status reached by the	*/
/*      previous process will be restored.			*/
/*      Otherwise the communication with SCT/SC and the 	*/
/*      internal program status will be initialized.		*/
/*								*/
/*      Case A: 						*/
/*      Restore program status using SCT configuration data:	*/
/*      This is done, if - a process key is available,		*/
/*		         - SCT configuration file exists and	*/
/*		         - the read and decrypted data are 	*/
/*			   correct.				*/
/*      In this case:						*/
/*		   - STARMOD is resetted with the SCA specific 	*/
/*		     configuration data (part of "sct_stat_list"*/
/*		     by calling sca_reset().			*/
/*		   - sca_reset() compares these process data	*/
/*		     with the real situation, e.g. asks the SCT */
/*		     whether an SC is inserted or not. If 	*/
/*		     necessary the process data will be changed.*/
/*		   - If SCT is in state 0, the SCT will be 	*/
/*		     resetted => no SC inserted. The security   */
/*		     status reached by the previous process 	*/
/*		     will be lossed.				*/
/*		   - The SCT configuration data are stored in 	*/
/*		     the "sct_stat_list[sct_id]".		*/
/*		   - Depending on the return code of sca_reset()*/
/*		     the sct_stat_list[] will be adapted to 	*/
/*		     the real situation, i.e. if no card 	*/
/*		     inserted, no SC-app is open.		*/
/*		   - Restore the security status reached by the */
/*		     previous process, e.g. generate session 	*/
/*		     key, activate device key set....		*/
/*		   - (Implicit) write SCT configuration data    */
/*		     into file.					*/
/*								*/
/*								*/
/*      Case B: 						*/
/*      Reset SCT (Initialize communication with SCT):		*/
/*      This is done, if - no process key is available,		*/
/*		         - SCT configuration file doesn't exist */
/*		         - or the data are not correct.		*/
/*      In this case:						*/
/*		   - If the values are not correct, the SCT  	*/
/*         	     configuration file will be deleted.	*/
/*		   - STARMOD is resetted without process data 	*/
/*		     by calling sca_reset().			*/
/*		   - sca_reset() resets the communication with  */
/*		     the SCT and returns the SCA specific 	*/
/*		     process data.				*/
/*		   - Write SCT configuration data into file.	*/
/*		   - Restore secure messaging (DTE/SCT).	*/
/*								*/
/*								*/
/*   3. Check whether SCT is available.				*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   sct_id		       Identifier of the SCT for which	*/
/*			       the configuration shall be done. */
/*							        */
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   TRUE		       SCT is available and 		*/
/*			       configuration was successful.	*/
/*   FALSE		       SCT is not available. 		*/
/*   -1		               error during SCT configuration	*/
/*							       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   activate_devkeyset()	Activate DevKeySet for a certain*/
/*				application.			*/
/*   copy_SCT_entry()		Copy values of SCT_entry.       */
/*   prepare_sec_mess()		- Generate session key,	    	*/
/*				- Set security mode for the 	*/
/*				  communication between DTE/SCT.*/
/*   reset_SCT()		Delete SCT configuration data   */
/*			        and reset SCT.	    	        */
/*   sca_reset()		Reset communication between DTE */
/*				and SCT.		        */
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure perform_SCT_config
 *
 ***************************************************************/
#ifdef __STDC__

static int perform_SCT_config(
	int	  sct_id
)

#else

static int perform_SCT_config(
	sct_id
)
int	  sct_id;

#endif

{



	static int             no_of_SCTs;
	static Boolean         first_call = TRUE;

	SCTStatus      *previous_SCT_config = ((SCTStatus *) 0);
	int		reset_rc = 0;
	SCT_SecMess 	sm_SCT;



	char           *proc = "perform_SCT_config";

	secsc_errno = NOERR;

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif



	/*
	 *  Check sct_id
	 */

	if (first_call == TRUE) {

		/* get number of registered SCTs */
		no_of_SCTs = sca_get_sct_info(0);
		if (no_of_SCTs < 0) {
			secsc_errno = analyse_sca_err(sct_id, "sca_get_sct_info");
			aux_add_error(secsc_errno, "Cannot get SCT info", (char *) sca_errmsg, char_n, proc);
			return (FALSE);
		}
		first_call = FALSE;
	}

	if (no_of_SCTs < 1) {
#ifdef SECSCTEST
		fprintf(stderr, "No SCT registered\n");
#endif
		return (FALSE);
	}

	if (no_of_SCTs < sct_id) {
#ifdef SECSCTEST
		fprintf(stderr, "Specified SCT (SCT_id: %d) is not registered\n", sct_id);
#endif
		return (FALSE);
	}





/********************************************************************************/
/*
 *   Perform SCT configuration
 *   Case A	
 */

	previous_SCT_config = read_SCT_config(sct_id);

	if (previous_SCT_config != (SCTStatus *) 0) {

		/*
		 *  Restore previous SCT configuration
		 */

		reset_rc = sca_reset(sct_id, 
				   COM_OPEN,
				   &(previous_SCT_config->SCA_config_data));
		if (reset_rc < 0) {
			secsc_errno = analyse_sca_err(sct_id, "sca_reset");
			free(previous_SCT_config);
			if (sca_errno == ESTATE0) {

				/*
				 *  SCT was in state 0 (before reset), 
			         *   the assumption is that the SCT has been
				 *   resetted by "analyse_sca_err"
				 */
#ifdef SECSCTEST
				fprintf(stderr, "Reset with previous SCT configuration data failed (SCT in state 0), SCT has been resetted\n");
#endif
				previous_SCT_config = (SCTStatus *) 0;
				goto check_sct;
			}
			else
				aux_add_error(secsc_errno, "Reset with previous SCT configuration data failed", (char *) sca_errmsg, char_n, proc);
			return (-1);
		}
#ifdef SECSCTEST
		if (reset_rc == M_REMOVE_CARD) 
			fprintf(stderr, "RC from sca_reset: M_REMOVE_CARD\n");
		else if (reset_rc == M_NO_CARD_INSERTED) 
			fprintf(stderr, "RC from sca_reset: M_NO_CARD_INSERTED\n");
		else if (reset_rc == M_CARD_INSERTED) 
			fprintf(stderr, "RC from sca_reset: M_CARD_INSERTED\n");
		else 	fprintf(stderr, "RC from sca_reset: PORT DATA OK\n");
		fprintf(stderr, "\nReturned data from sca_reset:\n");
		print_SCT_port(&(previous_SCT_config->SCA_config_data));
		fprintf(stderr, "\n");
#endif


		copy_SCT_entry(previous_SCT_config, &sct_stat_list[sct_id]);
		
		if ((reset_rc == M_REMOVE_CARD) || (reset_rc == M_NO_CARD_INSERTED)) {
			/*
			 *  No card inserted => set application to close in SCT list
			 */
	      		enter_app_in_sctlist(sct_id, CNULL);
		}


		/*
		 *  restore previous security status
		 */ 

		sm_SCT.command = sct_stat_list[sct_id].sm_SCT.command;
		sm_SCT.response = sct_stat_list[sct_id].sm_SCT.response;
		sct_stat_list[sct_id].sm_SCT.command = SCT_SEC_NORMAL;
		sct_stat_list[sct_id].sm_SCT.response = SCT_SEC_NORMAL;
		if (prepare_sec_mess(sct_id, &sm_SCT, TRUE)) {
			aux_add_error(LASTERROR, "Restore secure messaging depending on previous security status failed", CNULL, 0, proc);
			free(previous_SCT_config);
			return (-1);
		}
			

		/*
		 *  If an app is open, 
		 *     the device key set for this application will be activated
		 */

		if ((sct_stat_list[sct_id].app_name) &&
	 	    (strlen(sct_stat_list[sct_id].app_name))) {
			if (activate_devkeyset(sct_id, sct_stat_list[sct_id].app_name)) {
				AUX_ADD_ERROR;
				return (-1);
			}
		}



		free(previous_SCT_config);
		return(TRUE);
	}


/********************************************************************************/
/*
 *   Perform SCT configuration
 *   Case B	
 */


	if (previous_SCT_config == (SCTStatus *) 0) {

		/*
		 *  Reset SCT (initialize SCT configuration)
		 */

		if (reset_SCT(sct_id)) {
			if (aux_last_error() == EDEVLOCK) 
				aux_add_error(LASTERROR, "Cannot reset SCT (SCT device is busy)", CNULL, 0, proc);
			else
				aux_add_error(LASTERROR, "Cannot reset SCT (e.g. SCT is not connected)", CNULL, 0, proc);
			return (-1);
		}
	}


check_sct:

	/* check specified SCT (try to display text on SCT-display) */

	sca_rc = sca_display(sct_id, SCT_TEXT_SCT_CHECK, 3);

	if (sca_rc >= 0) {

		/* SCT is connected */
		return (TRUE);
	}
	else {
		/* SCT is not available */
		if (sca_errno == EDEVBUSY) {
			aux_add_error(EDEVLOCK, "SCT device is busy (port locked)", CNULL, 0, proc);
			return (-1);
		}
	}

	return(FALSE);





}				/* end perform_SCT_config */





/*--------------------------------------------------------------*/
/*						         	*/
/* PROC  read_SCT_config				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*								*/
/*   Case 1:  SCT configuration file exists:			*/
/*   1. Get process key			        	        */
/*   2. This file is read and decrypted, the values are checked:*/
/*      If the values are correct, the resulting data are 	*/
/*         returned.						*/
/*	If the values are not correct, the SCT configuration 	*/
/*         file is deleted and the NULL-pointer is returned.	*/
/*								*/
/*   Case 2:  SCT configuration file does not exist:		*/				
/*   The NULL-pointer is returned.				*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   sct_id		       Identifier of the SCT for which	*/
/*			       the configuration shall be done. */
/*							        */
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   previous_SCT_config       Pointer to previous SCT 	        */
/*			       configuration data.		*/
/*   NULL-Pointer	       No (correct) previous SCT 	*/
/*			       configuration data.		*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   aux_AppName2SCApp()	Get information about an SC app.*/
/*   delete_old_SCT_config()    Delete old SCT configuration    */
/*				file.				*/
/*   get_SCT_config_fname()	Get name of SCT configuration 	*/
/*				file.				*/
/*   get_process_key()		Get process key for encryption/ */
/*				decryption of SCT config.	*/
/*			         		       		*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure read_SCT_config
 *
 ***************************************************************/
#ifdef __STDC__

static SCTStatus *read_SCT_config(
	int	  sct_id
)

#else

static SCTStatus *read_SCT_config(
	sct_id
)
int	  sct_id;

#endif

{

	char           *config_file_name = "";
	int            fd_SCT_config;
	SCTStatus      *sct_entry;
	char	       *process_key;


	char           *proc = "read_SCT_config";

	secsc_errno = NOERR;

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif



/********************************************************************************/
/*
 *      Get name of SCT configuration file:
 */

	config_file_name = get_SCT_config_fname (sct_id);

	if (config_file_name == CNULL) {
		AUX_ADD_ERROR;
		return ((SCTStatus *) 0);
	}

#ifdef SECSCTEST
	fprintf(stderr, "                Name of SCT configuration file: %s\n", config_file_name);
#endif

/********************************************************************************/
/*
 *	Open SCT configuration file
 */

	if ((fd_SCT_config = open(config_file_name, O_RDONLY)) < 0) {
#ifdef SECSCTEST
		fprintf(stderr, "                SCT Configuration file missing, default values are used.\n", config_file_name);
#endif
		free(config_file_name);
		return ((SCTStatus *) 0);
	}

/********************************************************************************/
/*
 *      Get encryption key for the SCT configuration data
 */

	process_key = get_process_key();

	if (process_key == CNULL) {
		/* No key available => error */
		AUX_ADD_ERROR;
		free(config_file_name);
		return ((SCTStatus *) 0);
	}



/********************************************************************************/
/*
 *	Read and decrypt SCT configuration file 
 */

	if (!(sct_entry = (SCTStatus *) malloc(sizeof(SCTStatus)))) {
		aux_add_error(EMALLOC, "sct_entry", CNULL, 0, proc);
		free(config_file_name);
		free(process_key);
		return ((SCTStatus *) 0);
	}

	if ((read_dec(fd_SCT_config, (char *) sct_entry, sizeof(SCTStatus), process_key)) <= 0) {
#ifdef SECSCTEST
		fprintf(stderr, "SCT configuration file %s invalid, is deleted!\n", config_file_name);
#endif
	        close_dec(fd_SCT_config);
	        delete_old_SCT_config(sct_id);
		free(config_file_name);
		free(process_key);
		free(sct_entry);
		return ((SCTStatus *) 0);
	}
	close_dec(fd_SCT_config);
	free(process_key);


/********************************************************************************/
/*
 *	Check read values:
 *		If values are correct, the read information are stored into sct status list.
 *              Otherwise the read file is deleted.
 */


	if ((sct_entry->user_auth_done != TRUE) &&
	    (sct_entry->user_auth_done != FALSE))
		secsc_errno = ESCPROCDATA;
	else if ((sct_entry->sm_SCT.command != SCT_SEC_NORMAL) &&
	         (sct_entry->sm_SCT.command != SCT_INTEGRITY) &&
	         (sct_entry->sm_SCT.command != SCT_CONCEALED) &&
	         (sct_entry->sm_SCT.command != SCT_COMBINED)) 
			secsc_errno = ESCPROCDATA;
	     else if ((sct_entry->sm_SCT.response != SCT_SEC_NORMAL) &&
	              (sct_entry->sm_SCT.response != SCT_INTEGRITY) &&
	              (sct_entry->sm_SCT.response != SCT_CONCEALED) &&
	              (sct_entry->sm_SCT.response != SCT_COMBINED)) 
			secsc_errno = ESCPROCDATA;
	          else  if ((sct_entry->config_done != TRUE) &&
	                    (sct_entry->config_done != FALSE))
				secsc_errno = ESCPROCDATA;
	   		else if ((sct_entry->available != TRUE) &&
	              	         (sct_entry->available != FALSE)) 
					secsc_errno = ESCPROCDATA;

	if ((sct_entry->SCA_config_data.parity != P_NONE) &&
	    (sct_entry->SCA_config_data.parity != P_ODD) &&
	    (sct_entry->SCA_config_data.parity != P_EVEN))
		secsc_errno = ESCPROCDATA;
	else if ((sct_entry->SCA_config_data.edc != E_LRC) &&
		 (sct_entry->SCA_config_data.edc != E_CRC))
			secsc_errno = ESCPROCDATA;
	     else if ((sct_entry->SCA_config_data.chaining != C_OFF) &&
		      (sct_entry->SCA_config_data.chaining != C_ON))
			secsc_errno = ESCPROCDATA;
		  else if ((sct_entry->SCA_config_data.always != TRUE) &&
			   (sct_entry->SCA_config_data.always != FALSE))
				secsc_errno = ESCPROCDATA;

	if ((sct_entry->SCA_config_data.apdusize <= 0) ||
	    (sct_entry->SCA_config_data.tpdusize <= 0))
				secsc_errno = ESCPROCDATA;


	if (secsc_errno != NOERR) {
		aux_add_error(secsc_errno, "SCT configuration file invalid, is deleted!", config_file_name, char_n, proc);
	        delete_old_SCT_config(sct_id);
		free(config_file_name);
		free(sct_entry);
		return ((SCTStatus *) 0);
	}	
	else {

#ifdef SECSCTEST
		fprintf(stderr, "\nRead SCT configuration file for SCT: %d\n", sct_id);

		if (fd_SCT_config >= 0) {
			print_SCT_entry(sct_entry);
		}
#endif
	free(config_file_name);
	return(sct_entry);
	}

}				/* end read_SCT_config */




/*--------------------------------------------------------------*/
/*						         	*/
/* PROC  copy_SCT_entry					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Copy parameter values of the first given structure to the   */
/*  second given structure.					*/
/*								*/
/*  								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   *from_SCT_entry	       					*/
/*   *to_SCT_entry	       					*/
/*							       	*/
/* OUT							       	*/
/*   to_SCT_entry	       					*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   copy_SCT_port()		Copy values of SCTPort.	        */
/*			         		       		*/
/*--------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure copy_SCT_entry
 *
 ***************************************************************/
#ifdef __STDC__

static void copy_SCT_entry(
	SCTStatus	  *from,
	SCTStatus	  *to
)

#else

static void copy_SCT_entry(
	from,
	to

)
SCTStatus	  *from;
SCTStatus	  *to;

#endif

{


	char           *proc = "copy_SCT_entry";


	from->app_name[MAXL_APPNAME] = '\0';
	from->sw_pse_pin[PSE_PIN_L] = '\0';

	to->user_auth_done 		 	= from->user_auth_done;
	to->available      		 	= from->available;
	to->sm_SCT.command 		 	= from->sm_SCT.command;
	to->sm_SCT.response		 	= from->sm_SCT.response;
	strcpy (to->app_name,from->app_name);
	strcpy (to->sw_pse_pin, from->sw_pse_pin);
	to->config_done    		 	= from->config_done;
	to->available      		 	= from->available;

	copy_SCT_port(&from->SCA_config_data, &to->SCA_config_data);


}				/* end copy_SCT_entry */




/*--------------------------------------------------------------*/
/*						         	*/
/* PROC  copy_SCT_port					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Copy parameter values of the first given structure to the   */
/*  second given structure.					*/
/*								*/
/*  								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   *from_SCT_port	       					*/
/*   *to_SCT_port	       					*/
/*							       	*/
/* OUT							       	*/
/*   to_SCT_entry	       					*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*			         		       		*/
/*--------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure copy_SCT_port
 *
 ***************************************************************/
#ifdef __STDC__

static void copy_SCT_port(
	SCTPort	  *from,
	SCTPort	  *to
)

#else

static void copy_SCT_port(
	from,
	to

)
SCTPort	  *from;
SCTPort	  *to;

#endif

{

	char           *proc = "copy_SCT_port";

	to->bwt	 		= from->bwt;
	to->cwt	 		= from->cwt;
	to->baud	 	= from->baud;
	to->databits	 	= from->databits;
	to->stopbits	 	= from->stopbits;
	to->parity	 	= from->parity;
	to->dataformat  	= from->dataformat;
	to->tpdusize	 	= from->tpdusize;
	to->apdusize	 	= from->apdusize;
	to->edc	 		= from->edc;
	to->protocoltype 	= from->protocoltype;
	to->chaining	 	= from->chaining;
	to->icc_request 	= from->icc_request;
	to->first_pad_byte 	= from->first_pad_byte;
	to->next_pad_byte 	= from->next_pad_byte;
	to->always	 	= from->always;


}				/* end copy_SCT_port */


/*--------------------------------------------------------------*/
/*						         	*/
/* PROC  print_SCT_entry				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Print parameter values of the given structure SCTStatus.    */
/*								*/
/*  								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   *sct_entry		       					*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   print_SCT_port()		Print values of SCTPort.        */
/*			         		       		*/
/*--------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure print_SCT_entry
 *
 ***************************************************************/
#ifdef __STDC__

static void print_SCT_entry(
	SCTStatus	  *sct_entry
)

#else

static void print_SCT_entry(
	sct_entry

)
SCTStatus	  *sct_entry;

#endif

{

	char           *proc = "print_SCT_entry";


	fprintf(stderr, "app_name       : %s\n", sct_entry->app_name);
	fprintf(stderr, "sw_pse_pin     : %s\n", sct_entry->sw_pse_pin);
	if (sct_entry->user_auth_done == TRUE) 
		fprintf(stderr, "auth_done      : TRUE\n");
	else	fprintf(stderr, "auth_done      : FALSE\n");
	if (sct_entry->config_done == TRUE) 
		fprintf(stderr, "config_done    : TRUE\n");
	else	fprintf(stderr, "config_done    : FALSE\n");
	if (sct_entry->available == TRUE) 
		fprintf(stderr, "available      : TRUE\n");
	else	fprintf(stderr, "available      : FALSE\n");
	if (sct_entry->sm_SCT.command == SCT_SEC_NORMAL) 
		fprintf(stderr, "command        : SCT_SEC_NORMAL\n");
	if (sct_entry->sm_SCT.command == SCT_INTEGRITY) 
		fprintf(stderr, "command        : SCT_INTEGRITY\n");
	if (sct_entry->sm_SCT.command == SCT_CONCEALED) 
		fprintf(stderr, "command        : SCT_CONCEALED\n");
	if (sct_entry->sm_SCT.command == SCT_COMBINED) 
		fprintf(stderr, "command        : SCT_COMBINED\n");
	if (sct_entry->sm_SCT.response == SCT_SEC_NORMAL) 
		fprintf(stderr, "response       : SCT_SEC_NORMAL\n");
	if (sct_entry->sm_SCT.response == SCT_INTEGRITY) 
		fprintf(stderr, "response       : SCT_INTEGRITY\n");
	if (sct_entry->sm_SCT.response == SCT_CONCEALED) 
		fprintf(stderr, "response       : SCT_CONCEALED\n");
	if (sct_entry->sm_SCT.response == SCT_COMBINED) 
		fprintf(stderr, "response       : SCT_COMBINED\n");

	fprintf(stderr, "\nSTARMOD data: \n");

	print_SCT_port(&sct_entry->SCA_config_data);

}				/* end print_SCT_entry */




/*--------------------------------------------------------------*/
/*						         	*/
/* PROC  print_SCT_port					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Print parameter values of the given structure SCTPort.      */
/*								*/
/*  								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   *sct_port		       					*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*			         		       		*/
/*--------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure print_SCT_port
 *
 ***************************************************************/
#ifdef __STDC__

static void print_SCT_port(
	SCTPort	  *sct_port
)

#else

static void print_SCT_port(
	sct_port

)
SCTPort	  *sct_port;

#endif

{

	char           *proc = "print_SCT_port";

	fprintf(stderr, "bwt            : %d\n", sct_port->bwt);
	fprintf(stderr, "cwt            : %d\n", sct_port->cwt);
	fprintf(stderr, "baud           : %d\n", sct_port->baud);
	fprintf(stderr, "databits       : %d\n", sct_port->databits);
	fprintf(stderr, "stopbits       : %d\n", sct_port->stopbits);	
	if (sct_port->parity == P_NONE)
		fprintf(stderr, "parity         : P_NONE\n");	
	if (sct_port->parity == P_ODD)
		fprintf(stderr, "parity         : P_ODD\n");	
	if (sct_port->parity == P_EVEN)
		fprintf(stderr, "parity         : P_EVEN\n");	
	fprintf(stderr, "dataformat     : %d\n", sct_port->dataformat); 
	fprintf(stderr, "tpdusize       : %d\n", sct_port->tpdusize);	
	fprintf(stderr, "apdusize       : %d\n", sct_port->apdusize);	
	if (sct_port->edc == E_LRC)
		fprintf(stderr, "edc            : E_LRC\n");
	else    fprintf(stderr, "edc            : E_CRC\n");

	fprintf(stderr, "protocoltype   : %d\n", sct_port->protocoltype	);
	if (sct_port->chaining == C_OFF)
		fprintf(stderr, "chaining       : C_OFF\n");
	else 	fprintf(stderr, "chaining       : C_ON\n");
	
	fprintf(stderr, "icc_request    : %d\n", sct_port->icc_request);
	fprintf(stderr, "first_pad_byte : %x\n", sct_port->first_pad_byte);
	fprintf(stderr, "next_pad_byte  : %x\n", sct_port->next_pad_byte);
	fprintf(stderr, "always         : %x\n", sct_port->always);	




}				/* end print_SCT_port */





/*--------------------------------------------------------------*/
/*						         	*/
/* PROC  save_portparam					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Get port parameter from STARMOD and save into SCT 		*/
/*  configuration file. 					*/
/*								*/
/*								*/
/*  1) Call "sca_get_sctport()" to get SCT communication 	*/
/*     parameters.						*/
/*								*/
/*  2) Store returned SCT parameters.				*/
/*								*/
/*     								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0			       ok				*/
/*   -1		               error				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   copy_SCT_port()		Copy values of SCTPort.	        */
/*   sca_get_sctport()		Get the actual DTE-SCT 		*/
/*				communication parameters.       */
/*   write_SCT_config()		Encrypt and write SCT 		*/
/*				configuration data for the 	*/
/*				specified SCT.			*/
/*			         		       		*/
/*--------------------------------------------------------------*/


/***************************************************************
 *
 * Procedure save_portparam
 *
 ***************************************************************/
#ifdef __STDC__

static int save_portparam(
	int	  sct_id
)

#else

static int save_portparam(
	sct_id
)
int	  sct_id;

#endif


{

	int	rc;
	SCTPort	        sca_config_data;


	char           *proc = "save_portparam";

	secsc_errno = NOERR;
	sca_rc = 0;


#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif


	/* check the specified sct_id */
	if ((sct_id >= MAX_SCTNO) || (sct_id <= 0)) {
		aux_add_error(EINVALID, "Invalid value for sct_id", CNULL, 0, proc);
		return (-1);
	}
	
	
	/* 
	 *  Get SCT configuration data from the SCA-IF
	 */

	sca_rc = sca_get_sctport(sct_id, &sca_config_data);
	if (sca_rc < 0) {
		secsc_errno = analyse_sca_err(sct_id, "sca_get_sctport");
		aux_add_error(secsc_errno, "Cannot get SCT port paramater", (char *) sca_errmsg, char_n, proc);
		return (-1);
	}

	copy_SCT_port(&sca_config_data, &sct_stat_list[sct_id].SCA_config_data);

	if ((write_SCT_config(sct_id)) < 0) {
		AUX_ADD_ERROR;
		return (-1);
	}


	return (0);

}				/* end save_portparam */





/*--------------------------------------------------------------*/
/*						         	*/
/* PROC  write_SCT_config				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Encrypt and write the SCT configuration data for the 	*/
/*  specified SCT into a file.					*/
/*								*/
/*  1. Get process key			        	        */
/*  2. The data for the specified SCT ("sct_stat_list[sct_id]") */
/*     are encrypted with the process key and written into a 	*/
/*     file.							*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   sct_id		       Identifier of the SCT for which	*/
/*			       the SCT configuration data shall */
/*			       be written.			*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0			        ok				*/
/*   -1			        error				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   copy_SCT_entry()		Copy values of SCT_entry.       */
/*   get_SCT_config_fname()	Get name of SCT configuration 	*/
/*				file.				*/
/*   get_process_key()		Get process key for encryption/ */
/*				decryption of SCT config.	*/
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*--------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure write_SCT_config
 *
 ***************************************************************/
#ifdef __STDC__

static int write_SCT_config(
	int	  sct_id
)

#else

static int write_SCT_config(
	sct_id
)
int	  sct_id;

#endif

{


	char	       *process_key;
	char           *config_file_name = "";
	int            fd_SCT_config;
	SCTStatus      *sct_entry;

	char           *proc = "write_SCT_config";

	secsc_errno = NOERR;


#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif




/********************************************************************************/
/*
 *      Get name of SCT configuration file:
 */

	config_file_name = get_SCT_config_fname (sct_id);

	if (config_file_name == CNULL) {
		AUX_ADD_ERROR;
		return (-1);
	}

#ifdef SECSCTEST
	fprintf(stderr, "                Name of SCT configuration file: %s\n", config_file_name);
#endif


/********************************************************************************/
/*
 *	Open SCT configuration file
 */

	if ((fd_SCT_config = open(config_file_name, O_WRONLY | O_CREAT, FILEMASK)) < 0) {
		aux_add_error(ESYSTEM, "Cannot open SCT configuration file!", config_file_name, char_n, proc);
		free(config_file_name);
		return (-1);
	}

	chmod(config_file_name, FILEMASK);
	free(config_file_name);
	

		
/********************************************************************************/
/*
 *      Get encryption key for the SCT configuration data
 */

	process_key = get_process_key();

	if (process_key == CNULL) {
		/* No key available => error */
		AUX_ADD_ERROR;
		return (-1);
	}



/********************************************************************************/
/*
 *	Encrypt and write SCT configuration data
 */


	if (!(sct_entry = (SCTStatus *) malloc(sizeof(SCTStatus)))) {
		aux_add_error(EMALLOC, "sct_entry", CNULL, 0, proc);
		free(process_key);
		return (-1);
	}
	copy_SCT_entry(&sct_stat_list[sct_id], sct_entry);

	if ((write_enc(fd_SCT_config, (char *) sct_entry, sizeof(SCTStatus), process_key)) < 0) {
		if (LASTERROR != ESYSTEM) 
			AUX_ADD_ERROR;
		else 
			aux_add_error(ESYSTEM, "Cannot write SCT configuration file", CNULL, 0, proc);
		close_enc(fd_SCT_config);
		free(process_key);
		free(sct_entry);
		return (-1);
	}

	close_enc(fd_SCT_config);
	free(process_key);    
	free(sct_entry);


	return(0);

}				/* end write_SCT_config */




/*--------------------------------------------------------------*/
/*						         	*/
/* PROC  delete_old_SCT_config				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Delete old SCT specific configuration file. 		*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   sct_id		       Identifier of the SCT for which	*/
/*			       the configuration file shall be  */
/*			       deleted.				*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0			       ok (file deleted or file does 	*/
/*			           not exist)			*/ 
/*   -1		               error				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   get_SCT_config_fname()	Get name of SCT configuration 	*/
/*				file.				*/
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*--------------------------------------------------------------*/
/***************************************************************
 *
 * Procedure delete_old_SCT_config
 *
 ***************************************************************/
#ifdef __STDC__

static int delete_old_SCT_config(
	int	  sct_id
)

#else

static int delete_old_SCT_config(
	sct_id
)
int	  sct_id;

#endif

{


	char           *config_file_name = "";
	char           *proc = "delete_old_SCT_config";

	secsc_errno = NOERR;


#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif




/********************************************************************************/
/*
 *      Get name of SCT configuration files:
 */

	config_file_name = get_SCT_config_fname (sct_id);

	if (config_file_name == CNULL) {
		AUX_ADD_ERROR;
		return (-1);
	}
#ifdef SECSCTEST
	fprintf(stderr, "                Name of SCT configuration file: %s\n", config_file_name);
#endif


/********************************************************************************/
/*
 *	Delete old SCT configuration files
 */


	if (unlink(config_file_name)) {
		if (errno != ENOENT) {
			aux_add_error(ESYSTEM, "Cannot delete SCT configuration file!", config_file_name, char_n, proc);
			free(config_file_name);
			return (-1);
		}
	}

	free(config_file_name);
			
			
	return(0);

}				/* end delete_old_SCT_config */



/*--------------------------------------------------------------*/
/*						         	*/
/* PROC  gen_process_key				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Generate random string and set this string into the 	*/
/*  environment variable "SC_PROCESS_KEY". 		 	*/
/*							        */
/*							        */
/* IN			     DESCRIPTION		       	*/
/*							        */
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0		       	       ok				*/
/*   -1		               error			 	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sec_random_str()	       Generate random character string.*/
/*   strzfree()		       Free string.   		    	*/
/*			         		       		*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure gen_process_key
 *
 ***************************************************************/
#ifdef __STDC__

static int gen_process_key(
)

#else

static int gen_process_key(
)

#endif

{

	char		*setenv_cdo;
	char		*process_key;
	int		nchar;

	char           *proc = "gen_process_key";


#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	if (!(process_key = sec_random_str(PROCESS_KEY_LEN, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"))) {
		AUX_ADD_ERROR;
		return (-1);
	}

	nchar = strlen(process_key) + strlen(SC_PROCESS_KEY) + 16;

	if (!(setenv_cdo = (char *) malloc(nchar))) {
		aux_add_error(EMALLOC, "setenv command", CNULL, 0, proc);
		return (-1);
	}
	
	strcpy(setenv_cdo, SC_PROCESS_KEY);
	strcat(setenv_cdo, "=");
	strcat(setenv_cdo, process_key);

#ifdef SECSCTEST
	fprintf(stderr, "setenv command: %s\n", setenv_cdo);
#endif

	if (putenv(setenv_cdo)) {
		aux_add_error(ESYSTEM, "putenv command", setenv_cdo, char_n, proc);
		return (-1);
	} 

	free (process_key);

	return(0);	


}				/* end gen_process_key */



/*--------------------------------------------------------------*/
/*						         	*/
/* PROC  get_SCT_config_fname				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Compose and return name of configuration file for the 	*/
/*  specified SCT: 						*/
/*								*/
/*  Structure:							*/
/*  Home directory || SCT_CONFIG_name || sct_id			*/
/*							        */
/*							        */
/*  The calling routine has to release the allocated memory.	*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   sct_id		       Identifier of the SCT.		*/
/*							        */
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   <> CNULL		       ptr to name of file		*/
/*   CNULL		       error			 	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   int2ascii()		Transform an integer value into */
/*			        a NULL terminated ASCII         */
/*				character string.	        */
/*   aux_cpy_String()		Copy string.			*/
/*			         		       		*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure get_SCT_config_fname
 *
 ***************************************************************/
#ifdef __STDC__

static char *get_SCT_config_fname(
	int	  sct_id
)

#else

static char *get_SCT_config_fname(
	sct_id
)
int	  sct_id;

#endif

{
	char           *homedir = "";
	static char    *config_file_name = "";
	char           *sca_config_file_name = "";
	static int     old_sctid = 0;
	char	       sct_id_ascii[MAXSCTID_LEN];

	char           *proc = "get_SCT_config_fname";

	secsc_errno = NOERR;

#ifdef SECSCTEST
/*	fprintf(stderr, "SECSC-Function: %s\n", proc); */
#endif


	if ((old_sctid != sct_id) || 
	    (!(config_file_name)) || (strlen(config_file_name) == 0)) {

		if ((config_file_name) && (strlen(config_file_name) != 0)) 
			free(config_file_name);

	    /*
	     *  Compose configuration file name 
	     */

		homedir = getenv("HOME");
		if (!homedir) {
			aux_add_error(ESYSTEM, "Getenv failed for variable HOME.", CNULL, 0, proc);
			return (CNULL);
		}
		config_file_name = (char *) malloc(strlen(homedir) + strlen(SCT_CONFIG_name) + 16);
		if (!config_file_name) {
			aux_add_error(EMALLOC, "SCT configuration file", CNULL, 0, proc);
			return (CNULL);
		}
		strcpy(config_file_name, homedir);
		if (strlen(homedir))
			if (config_file_name[strlen(config_file_name) - 1] != PATH_SEPARATION_CHAR)
				strcat(config_file_name, PATH_SEPARATION_STRING);
		strcat(config_file_name, SCT_CONFIG_name);


		if (int2ascii(&sct_id_ascii[0], sct_id)) {
			AUX_ADD_ERROR;
			free(config_file_name);
			return (CNULL);
		}
		strcat(config_file_name, sct_id_ascii);

		old_sctid = sct_id;
	}


#ifdef SECSCTEST
/*	fprintf(stderr, "                Name of SCT configuration file: %s\n", config_file_name);*/
#endif

	return(aux_cpy_String(config_file_name));

}			/* get_SCT_config_fname */





/*--------------------------------------------------------------*/
/*						                */
/* PROC  activate_devkeyset				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*							       	*/
/*   Activate DevKeySet for a certain application.    	        */
/*     								*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   sct_id		       SCT identifier			*/
/*   app_name		       Name of the application, for	*/
/*			       which the device key set shall   */
/*			       be activated.			*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*  -1			       Error			       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   get_appid()		Get application id for a given  */
/*				application name.		*/
/*   sca_activate_devkeyset()	Activate device key set in the 	*/
/*				SCT. 				*/
/*   analyse_sca_err()		Analyse of an error-number 	*/
/*				returned by an SCA-IF function. */
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*--------------------------------------------------------------*/


/***************************************************************
 *
 * Procedure activate_devkeyset
 *
 ***************************************************************/
#ifdef __STDC__

static int activate_devkeyset(
	int	  	sct_id,
	char	 	*app_name
)

#else

static int activate_devkeyset(
	sct_id,
	app_name
)
int	  	sct_id;
char	*app_name;

#endif

{

	OctetString     *app_id;
	KeyLevel 	key_level;




	char           *proc = "activate_devkeyset";

	secsc_errno = NOERR;
	sca_rc = 0;

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	/* check the given sct_id */
	if ((sct_id >= MAX_SCTNO) || (sct_id <= 0)) {
		aux_add_error(EINVALID, "Invalid value for sct_id", CNULL, 0, proc);
		return (-1);
	}


	/* 
	 *  Get application_id belonging to the application name
	 */

	app_id = get_appid(app_name);
	if (!(app_id)) {
		aux_add_error(EINVALID, "Internal error (can't get app_id for app_name)", (char *) app_name, char_n, proc);
		return (-1);
	}


	/* 
	 *  Activate Device Key Set for this application
	 */

#ifndef INITDEVSET
	sca_rc = sca_activate_devkeyset(sct_id, app_id, key_level = FILE_CURRENT);
#else
	sca_rc = sca_activate_devkeyset(sct_id, NULLOCTETSTRING, key_level = FILE_MASTER);
/* with Set in EEPROM */
/*	sca_rc = sca_activate_devkeyset(sct_id, NULLOCTETSTRING, key_level = INITKEY); */
#endif

	if (sca_rc < 0) {
		secsc_errno = analyse_sca_err(sct_id, "sca_activate_devkeyset");
		aux_add_error(secsc_errno, "Cannot activate device key set", (char *) sca_errmsg, char_n, proc);
		aux_free_OctetString(&app_id);
		return (-1);
	}

	aux_free_OctetString(&app_id);

	return(0);

}	/* activate_devkeyset */






/*--------------------------------------------------------------*/
/*						                */
/* PROC  prepare_sec_mess				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*							       	*/
/*   1.	Generate session key(s) for the communication between 	*/
/*      DTE and SCT, if secure messaging is required for the    */
/*      communication between SCT and DTE              		*/
/*      ( Is done once for each process )			*/
/*							       	*/
/*   2. If the current security mode is unequal to the required	*/
/*  	value:	-  set security mode to the required value by   */
/*		   calling "sca_set_mode" and			*/
/*		-  change value in "sct_stat_list"		*/
/*								*/
/*  - Global variable "sct_stat_list[sc_sel.sct_id]" contains   */
/*    current security mode for the SCT specified by     	*/
/*    sc_sel.sct_id						*/
/*     								*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   sct_id		       SCT identifier			*/
/*   new_sm_SCT		       Required security 		*/
/*			       mode for the communication 	*/
/*			       between DTE and SCT. Separat     */
/*			       values for command and response.	*/
/*   reset_sm		       = TRUE means: Session keys 	*/
/*					     are generated.	*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*  -1			       Error			       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sca_gen_sessionkey()	Generate session key(s) for     */
/*				secure messaging between DTE    */
/*				and SCT.			*/
/*   sca_set_mode()		Set security mode.  		*/
/*   write_SCT_config()		Encrypt and write SCT 		*/
/*				configuration data for the 	*/
/*				specified SCT.			*/
/*                                                              */
/*   analyse_sca_err()		Analyse of an error-number 	*/
/*				returned by an SCA-IF function. */
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*--------------------------------------------------------------*/


/***************************************************************
 *
 * Procedure prepare_sec_mess
 *
 ***************************************************************/
#ifdef __STDC__

static int prepare_sec_mess(
	int	  	sct_id,
	SCT_SecMess 	*new_sm_SCT,
	Boolean		reset_sm
)

#else

static int prepare_sec_mess(
	sct_id,
	new_sm_SCT,
	reset_sm
)
int	  	sct_id;
SCT_SecMess	*new_sm_SCT;
Boolean		reset_sm;

#endif

{

	static 	Boolean		session_key_generated   = FALSE;
		KeyLevel 	key_level;
		SessionKey	session_key;




	char           *proc = "prepare_sec_mess";

	secsc_errno = NOERR;
	sca_rc = 0;

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	/* check the given sct_id */
	if ((sct_id >= MAX_SCTNO) || (sct_id <= 0)) {
		aux_add_error(EINVALID, "Invalid value for sct_id", CNULL, 0, proc);
		return (-1);
	}


	/*
	 *  Generate session key(s) for the communication 
	 *	     between DTE and SCT, if not yet done
	 */

	if (((session_key_generated == FALSE) || (reset_sm == TRUE)) &&
	    ((new_sm_SCT->command != SCT_SEC_NORMAL) || 
	     (new_sm_SCT->command != SCT_SEC_NORMAL))) {
		session_key.integrity_key = desCBC_ISO0;
		session_key.concealed_key = desCBC_ISO0;
		sca_rc = sca_gen_sessionkey(sct_id, &session_key);
		if (sca_rc < 0) {
			secsc_errno = analyse_sca_err(sct_id, "sca_gen_sessionkey");
			aux_add_error(secsc_errno, "Cannot generate session keys for secure messaging (DTE/SC)", CNULL, 0, proc);
			return (-1);
		}
		session_key_generated = TRUE;
	}



	/* if actual value = new value => do nothing */
	if ((sct_stat_list[sct_id].sm_SCT.command == new_sm_SCT->command) &&
	    (sct_stat_list[sct_id].sm_SCT.response == new_sm_SCT->response))
		return (0);

	/* set security mode to new value */
	sca_rc = sca_set_mode(sct_id, new_sm_SCT);
	if (sca_rc < 0) {
		secsc_errno = analyse_sca_err(sct_id, "sca_set_mode");
		aux_add_error(secsc_errno, "Cannot set security mode for secure messaging (DTE/SC)", CNULL, 0, proc);
		return (-1);
	}
	/* change entry in sct_stat_list */
	sct_stat_list[sct_id].sm_SCT.command = new_sm_SCT->command;
	sct_stat_list[sct_id].sm_SCT.response = new_sm_SCT->response;


	/*
	 *  Save the changed SCT configuration data
	 */

	if ((write_SCT_config(sct_id)) < 0) {
		AUX_ADD_ERROR;
		return (-1);
	}


	return (0);

}				/* end prepare_sec_mess */




/*--------------------------------------------------------------*/
/*						                */
/* PROC  set_fct_sec_mode				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Depending on the specified SCA-function:			*/
/*	- the security mode for the communication between DTE 	*/
/*	  and the specified SCT is set and			*/
/*	- the security mode for the communication between SCT	*/
/*	  and SC is returned.					*/
/*  The name of the SCA-function is delivered in an input 	*/
/*  parameter.   						*/
/*								*/
/*  - Global variable "sca_fct_list" contains a list of the 	*/
/*    SCA-functions and the belonging secure messaging values.  */
/*     								*/
/*  If the delivered function name is not stored within       	*/
/*  "sca_fct_list", the security mode will not be changed and 	*/
/*  the sec-mode for SCT/SC is set to SEC_NORMAL		*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   sct_id		       SCT identifier			*/
/*   SCA_fct_name	       Name of the SCA-function		*/
/*   sm_SC		       Pointer to the sec-mode for the	*/
/*			       communication SCT/SC		*/
/*							       	*/
/* OUT							       	*/
/*   sm_SC		       Sec-mode for the			*/
/*			       communication SCT/SC		*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*  -1			       Error			       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   aux_FctName2FctPar		Get security mode for the SCA-  */
/*				function			*/
/*   prepare_sec_mess()		- Generate session key,	    	*/
/*				- Set security mode for the 	*/
/*				  communication between DTE/SCT.*/
/*                                                              */
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*--------------------------------------------------------------*/


/***************************************************************
 *
 * Procedure set_fct_sec_mode
 *
 ***************************************************************/
#ifdef __STDC__

static int set_fct_sec_mode(
	int	 	 sct_id,
	char	 	 *SCA_fct_name,
	ICC_SecMess	 *sm_SC
)

#else

static int set_fct_sec_mode(
	sct_id,
	SCA_fct_name,
	sm_SC
)
int		  sct_id;
char		 *SCA_fct_name;
ICC_SecMess	 *sm_SC;

#endif

{
	SCAFctPar      *sca_fct_par;

	char           *proc = "set_fct_sec_mode";


#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	if (!SCA_fct_name) {
		sm_SC->command = ICC_SEC_NORMAL;
		sm_SC->response = ICC_SEC_NORMAL;
		return (0);
	}
	sca_fct_par = aux_FctName2FctPar(SCA_fct_name);
	if (!sca_fct_par) {
		sm_SC->command = ICC_SEC_NORMAL;
		sm_SC->response = ICC_SEC_NORMAL;
		return (0);
	}
	if (prepare_sec_mess(sct_id, &sca_fct_par->sm_SCT, FALSE)) {
		AUX_ADD_ERROR;
		return (-1);
	}
	sm_SC->command = sca_fct_par->sm_SC.command;
	sm_SC->response = sca_fct_par->sm_SC.response;
	return (0);

}				/* set_fct_sec_mode */





/*--------------------------------------------------------------*/
/*						                */
/* PROC  get_devkeyset_info				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Provides infos about the device key set used within this 	*/
/*  software.							*/
/*  In the current version predefined values are used.		*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   app_name			Application name		*/
/*							        */
/*							        */
/* OUT			     DESCRIPTION		       	*/
/*   key_dev_info[]		List of pointers. For each 	*/
/*				device key (incl. its info.)    */
/*				one pointer.			*/
/*				No memory is allocated. 	*/
/*   no_of_devkeys		Number of relevant entries in 	*/
/*				the list "key_dev_info[]".	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	        o.k			       	*/
/*  -1			        Error			       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*                                                              */
/*   aux_add_error()		Add error to error stack	*/
/*   aux_AppObjName2SCObj()	Get information about an SC     */
/*        			object belonging to an 		*/
/*                              application on SC. 		*/
/*                                                              */
/*--------------------------------------------------------------*/




/***************************************************************
 *
 * Procedure get_devkeyset_info
 *
 ***************************************************************/
#ifdef __STDC__

static int get_devkeyset_info(
	char		 *app_name,
	KeyDevSel	 *key_dev_info[],
	unsigned int 	 *no_of_devkeys
)

#else

static int get_devkeyset_info(
	app_name,
	key_dev_info,
	no_of_devkeys
)
char		 *app_name;
KeyDevSel	 *key_dev_info[];
unsigned int 	 *no_of_devkeys;

#endif

{

		
		SCObjEntry     *sc_obj_entry;
		unsigned int	n;
		Boolean		SC_internal;
		Boolean		new_app = FALSE;
	static	char		old_app_name[MAXL_APPNAME] = {'\0'};

	static 	KeyDevSel	old_key_dev_info[MAX_DEVKEY_NO+1];
	static	KeyId		old_devkey_id[MAX_DEVKEY_NO+1];

	char           *proc = "get_devkeyset_info";


	secsc_errno = NOERR;
	sca_rc = 0;

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif


	if (!app_name) {
		aux_add_error(EINVALID, "No application name specified.", CNULL, 0, proc);
		return (-1);
	}

	if ((!old_app_name) || (strcmp(app_name, old_app_name))) 
		new_app = TRUE;


	for (n = 0; default_devkeylist[n].name; n++) {

		if (new_app == TRUE) {

			/*
			 *  Get information for new app_name
			 */

			sc_obj_entry = aux_AppObjName2SCObj(app_name, default_devkeylist[n].name);
			if (sc_obj_entry == NULLSCOBJENTRY) {
				aux_add_error(EINVALID, "Cannot get configuration info for a Device key", default_devkeylist[n].name, char_n, proc);
				return (-1);
			}

			old_key_dev_info[n].key_algid   = desCBC_ISO1;
			old_key_dev_info[n].key_status  = DEV_ANY;
	
		
			old_key_dev_info[n].key_id = &old_devkey_id[n];
			get_SC_keyid(sc_obj_entry->sc_id, SC_internal = TRUE, old_key_dev_info[n].key_id);
			old_key_dev_info[n].key_purpose = default_devkeylist[n].purpose;
		}

		key_dev_info[n] = &old_key_dev_info[n];

	}

	strcpy (old_app_name, app_name);
	*no_of_devkeys = n;
	

	return (0);

}				/* get_devkeyset_info */


/*--------------------------------------------------------------*/
/*						                */
/* PROC  pre_devkeyset				       		*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Function to be called before using a device key set.	*/
/*								*/
/*  This function						*/
/*		- checks whether the application is an SC app,	*/
/*		- provides the application id belonging to the  */
/*		  given application name,			*/
/*		- provides infos about the device key set 	*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   app_name			Application name		*/
/*   app_id			Pointer to app_id		*/
/*   no_of_devkeys		Pointer to number of device keys*/
/*							       	*/
/* OUT							       	*/
/*   *app_id			application identifier		*/
/*				(has to be released by the 	*/
/*				 calling routine)		*/
/*   key_dev_info[]		List of pointers. For each 	*/
/*				device key (incl. its info.)    */
/*				one pointer.			*/
/*				No memory is allocated. 	*/
/*   *no_of_devkeys		Number of relevant entries in 	*/
/*				the list "key_dev_info[]".	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*  -1			       Error			       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   get_appid()		Get application id for a given  */
/*				application name.		*/
/*   get_devkeyset_info()	Provides infos about the device */
/*				key set used within this 	*/
/*				software.			*/
/*   sec_psetest()		Check whether SC available and  */
/*				application is an SC app.	*/		
/*   set_fct_sec_mode()		Set security mode for comm.     */
/*				between DTE/SCT depending on the*/
/*                	        SCA-function to be called.      */
/*                                                              */
/*   aux_add_error()		Add error to error stack	*/
/*                                                              */
/*--------------------------------------------------------------*/




/***************************************************************
 *
 * Procedure pre_devkeyset
 *
 ***************************************************************/
#ifdef __STDC__

int pre_devkeyset(
	char		 *app_name,
	OctetString	 **app_id,
	KeyDevSel	 *key_dev_info[],
	unsigned int 	 *no_of_devkeys
)

#else

int pre_devkeyset(
	app_name,
	app_id,
	key_dev_info,
	no_of_devkeys
)
char		 *app_name;
OctetString	 **app_id;
KeyDevSel	 *key_dev_info[];
unsigned int 	 *no_of_devkeys;

#endif

{

	unsigned int	sct_id;
	PSELocation     pse_location;
	

	


	char           *proc = "pre_devkeyset";

	secsc_errno = NOERR;
	sca_rc = 0;
	sct_id = sc_sel.sct_id;

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif


	/*
	 *   Check the input parameter 
	 */

	if (!app_name) {
		aux_add_error(EINVALID, "Missing application name", CNULL, 0, proc);
		return (-1);
	}
	if (!app_id) {
		aux_add_error(EINVALID, "Missing pointer to application id", CNULL, 0, proc);
		return (-1);
	}


	if((pse_location = sec_psetest(app_name)) == ERR_in_psetest) {
		AUX_ADD_ERROR;
		return (-1);
	}

	if (pse_location == SWpse) {
		aux_add_error(EINVALID, "Application is not an SC application\n", app_name, char_n, proc);
		return (-1);
	}



	if (get_devkeyset_info(app_name, key_dev_info, no_of_devkeys)) {
		AUX_ADD_ERROR;
		return (-1);
	}

	*app_id = get_appid(app_name);
	if (!(*app_id)) {
		aux_add_error(EINVALID, "Internal error (can't get app_id for app_name)", (char *) app_name, char_n, proc);
		return (-1);
	}

	return (0);

}	/* pre_devkeyset */



/*--------------------------------------------------------------*/
/*						                */
/* PROC  re_devkeyset				       		*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Function to be called after having used a device key set.	*/
/*								*/
/*  As functios which handle with device key sets implicitly 	*/
/*  activate the INIT-set of the SCT, the Device Key Set for 	*/
/*  the actual application has to be activated again.		*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   app_name			Application name		*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*  -1			       Error			       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   activate_devkeyset()	Activate DevKeySet for a certain*/
/*				application.			*/
/*                                                              */
/*   aux_add_error()		Add error to error stack	*/
/*                                                              */
/*--------------------------------------------------------------*/




/***************************************************************
 *
 * Procedure re_devkeyset
 *
 ***************************************************************/
#ifdef __STDC__

int re_devkeyset(
	char		 *app_name
)

#else

int re_devkeyset(
	app_name
)
char		 *app_name;

#endif

{

	unsigned int	sct_id;
	

	


	char           *proc = "re_devkeyset";

	secsc_errno = NOERR;
	sca_rc = 0;
	sct_id = sc_sel.sct_id;

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif


	/*
	 *   Check the input parameter 
	 */

	if (!app_name) {
		aux_add_error(EINVALID, "Missing application name", CNULL, 0, proc);
		return (-1);
	}



	/* 
	 *  Activate the Device Key Set for the actual application.
	 */

	if (activate_devkeyset(sct_id, app_name)) {
		AUX_ADD_ERROR;
		return (-1);
	}


	return (0);

}	/* re_devkeyset */



/*--------------------------------------------------------------*/
/*						                */
/* PROC  request_keycard			       		*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  A keycard is requested (Before that an inserted SC will 	*/
/*  be ejected). 						*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   new_keycard		= TRUE means, that an uninit.	*/
/*				       shall be requested.	*/
/*				= FALSE means, that an already  */
/*				       written keycard shall be */
/*				       requested.		*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*  -1			       Error			       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   bell_function()		"Ring the bell" to require user */
/*                              input at the SCT.		*/
/*   eject_sc()		        Handle ejection of the SC.      */
/*   request_sc()		Request and initialize a 	*/
/*				smartcard.			*/
/*   reset_SCT()		Delete SCT configuration data   */
/*			        and reset SCT.	    	        */
/*   sca_get_sct_info()		Get information about  		*/
/*				registered SCTs.		*/
/*   set_fct_sec_mode()		Set security mode for comm.     */
/*				between DTE/SCT depending on the*/
/*                	        SCA-function to be called.      */
/*                                                              */
/*   aux_add_error()		Add error to error stack	*/
/*                                                              */
/*--------------------------------------------------------------*/




/***************************************************************
 *
 * Procedure request_keycard
 *
 ***************************************************************/
#ifdef __STDC__

int request_keycard(
	Boolean		new_keycard
)

#else

int request_keycard(
	new_keycard
)
Boolean		new_keycard;
#endif

{

	unsigned int	sct_id;
	Boolean         SC_in_SCT;
	char           *display_text;
	Boolean         alarm;
	int             time_out;

	unsigned int	i,n;
	char		err_text[256];
	

	


	char           *proc = "request_keycard";

	secsc_errno = NOERR;
	sca_rc = 0;
	sct_id = sc_sel.sct_id;
	SC_in_SCT = FALSE;

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif



	/*
	 *  Check whether an SC is inserted. 
	 */

	sca_rc = sca_get_sct_info(sct_id);
	if (sca_rc == M_CARD_INSERTED) {
		SC_in_SCT = TRUE;	/* SC is inserted */
	} else if (sca_rc == M_NO_CARD_INSERTED)
		SC_in_SCT = FALSE;	/* no SC is inserted */
	else if (sca_rc == M_SCT_NOT_ACTIVE) {
		if (reset_SCT(sct_id)) {
			if (aux_last_error() == EDEVLOCK) 
				aux_add_error(LASTERROR, "Cannot reset SCT (SCT device is busy)", CNULL, 0, proc);
			else
				aux_add_error(LASTERROR, "Cannot reset SCT (e.g. SCT is not connected)", CNULL, 0, proc);
			return (-1);
		}
		SC_in_SCT = FALSE;	/* no SC is inserted */
	}
	else {

		secsc_errno = analyse_sca_err(sct_id, "sca_get_sct_info");	/* error */
		aux_add_error(secsc_errno, "Cannot get information about SCT, sct_id:", (char *) sct_id, int_n, proc);
		return (-1);
	}


	/*
	 *  An already inserted SC will be ejected. 
	 */

	if (SC_in_SCT == TRUE) {
		if (eject_sc(sct_id, display_text = CNULL, alarm = FALSE)) {
			if (aux_last_error() == EDEVLOCK) 
				aux_add_error(LASTERROR, "Cannot eject smartcard (SCT device is busy)", CNULL, 0, proc);
			else
				aux_add_error(LASTERROR, "Cannot eject smartcard (prior to request a keycard)", CNULL, 0, proc);
			return (-1);
		}
	}


	/*
	 *  Request a keycard. 
	 */

	if (new_keycard == TRUE) {
		fprintf(stderr, "Please insert an uninitialized smartcard into the SCT.\n");
		display_text = SCT_TEXT_REQ_N_KEYCARD;
	}
	else   {
		fprintf(stderr, "Please insert a complete keycard into the SCT.\n");
		display_text = SCT_TEXT_REQ_O_KEYCARD;
	}


	if (request_sc(sct_id, display_text, time_out = SC_timer)) {
		aux_add_error(LASTERROR, "Request keycard failed", CNULL, 0, proc);
		return (-1);
	}


}	/* request_keycard */







/***************************************************************
 *
 * Procedure check_pin
 *
 ***************************************************************/
#ifdef __STDC__

static int check_pin(
	char		 *pin
)

#else

static int check_pin(
	pin
)
char		 *pin;

#endif
{
	int 	len, i;
	char 	err_text[256];


	char           *proc = "check_pin";


	len = strlen(pin);
	if (len > MAX_PIN_L) {

		sprintf(err_text, "PIN is too long, max. length is %d\n", MAX_PIN_L);
		aux_add_error(EINVALID, "Invalid PIN", (char *) err_text, char_n, proc);
		return(-1);
	}

	for(i = 0; i < len; i++){
		if(!isdigit(pin[i])){
			sprintf(err_text, "For a PIN only digits are allowed!\n", pin[i]);
			aux_add_error(EINVALID, "Invalid PIN", (char *) err_text, char_n, proc);
			return(-1);
		}
	}
	return(0);
}



/*--------------------------------------------------------------*/
/*						                */
/* PROC  write_on_keycards				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Writes the device key set on one or more keycards.		*/
/*  								*/
/*  1. If no PIN value is given, ask for it interactively.	*/
/*								*/
/*  2. In a loop:						*/
/*     - Request a keycard (uninitialized smartcard)		*/
/*     - Initialize the inserted smartcard as a keycard.	*/
/*     - Write the device key set on the inserted keycard.	*/
/*     - Eject the keycard.					*/
/*     - Ask whether a further keycard shall be written.	*/
/*     - Ask for a new PIN.					*/
/*							        */
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   app_id		        Application identifier.		*/
/*   key_dev_info[]		List of pointers. For each 	*/
/*				device key (incl. its info.)    */
/*				one pointer.			*/
/*				No memory is allocated. 	*/
/*   no_of_devkeys		Number of relevant entries in 	*/
/*				the list "key_dev_info[]".	*/
/*   pin_arg			PIN to be installed on the 	*/
/*				keycard.			*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*				The keycard could be written.   */
/*  -1			       Error			       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   check_pin()		Check the entered PIN (keycard).*/
/*   eject_sc()		        Handle ejection of the SC.      */
/*   request_keycard()		Eject an inserted smartcard and	*/
/*				request a keycard.		*/
/*   save_devkeyset()		Save a device key set on a 	*/
/*				keycard.			*/
/*   sca_get_sct_info()		Get information about registered*/
/*				SCTs.				*/
/*                                                              */
/*   aux_add_error()		Add error to error stack	*/
/*                                                              */
/*--------------------------------------------------------------*/




/***************************************************************
 *
 * Procedure write_on_keycards
 *
 ***************************************************************/
#ifdef __STDC__

int write_on_keycards(
	OctetString	 *app_id,
	KeyDevSel	 *key_dev_info[],
	unsigned int 	 no_of_devkeys,
	char		 *pin
)

#else

int write_on_keycards(
	app_id,
	key_dev_info,
	no_of_devkeys,
	pin
)
OctetString	 *app_id;
KeyDevSel	 *key_dev_info[];
unsigned int 	 no_of_devkeys;
char		 *pin;

#endif

{

	unsigned int	sct_id;
	Boolean		next_keycard = TRUE;
	OctetString	pin_ostr;
	char		user_in[128];
	char		err_text[256];
	char           *display_text;
	Boolean         alarm;



		


	char           *proc = "write_on_keycards";

	secsc_errno = NOERR;
	sca_rc = 0;
	sct_id = sc_sel.sct_id;

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif



	pin_ostr.noctets = 0;
	pin_ostr.octets = CNULL;


	/*
	 *  If no PIN is given, ask for it interactively
	 */

	if (!pin) {
		if (!(pin_ostr.octets = sec_read_pin("the PIN to be installed on the keycard", CNULL, TRUE))) {
			aux_add_error(ESYSTEM, "Can't read PIN", CNULL, 0, proc);
			return(-1);
		}
		if ( (!pin_ostr.octets) || (strlen(pin_ostr.octets) == 0) ) {
			aux_add_error(EINVALID, "Without a PIN the device key set cannot be installed on the keycard.",CNULL, 0, proc);
			return(-1);
		}
		pin_ostr.noctets = strlen(pin_ostr.octets);
	}
	else {
	
		if (!(pin_ostr.octets = (char *) malloc(128))) {
			aux_add_error(EMALLOC, "Cannot allocate memory for pin.", CNULL, 0, proc);	
			return (-1);
		}
		pin_ostr.noctets = strlen(pin);
		strcpy (pin_ostr.octets, pin);
	}

	if (check_pin(pin_ostr.octets)) {
		AUX_ADD_ERROR;
		goto err_exit;
	}
	


	while (next_keycard == TRUE) {

		/*
		 *  Request a keycard ( an already inserted smartcard will be ejected)
		 */

		if ( request_keycard(TRUE) < 0 ) {
			AUX_ADD_ERROR;
			goto err_exit;
		}


		/*
		 *  Initialize the smartcard as a keycard (STARMAC scriptfile)
		 */

		/*
		   ?????????????????????????????????????????????????
		*/



		/*
		 *  Save the device key set on the inserted keycard
		 */

		if ( save_devkeyset(app_id, key_dev_info, no_of_devkeys, &pin_ostr) < 0 ) {
			AUX_ADD_ERROR;
			goto err_exit;
		}



		/*
		 *  Eject keycard. 
		 */

		if (eject_sc(sct_id, display_text = CNULL, alarm = FALSE)) {
			AUX_ADD_ERROR;
			goto err_exit;
		}


		/*
		 *  A next keycard?
		 */

		fprintf(stderr, "Save device key set on a next keycard? (n/y)) \n");
		gets(user_in);
		if ((strlen(user_in) > 0) && (!strncmp(user_in, "y",1))) {
			next_keycard = TRUE;
			free(pin_ostr.octets);
			if (!(pin_ostr.octets = sec_read_pin("the PIN to be installed on the keycard", CNULL, TRUE))) {
				aux_add_error(ESYSTEM, "Can't read PIN", CNULL, 0, proc);
				goto err_exit;
			}
			if ( (!pin_ostr.octets) || (strlen(pin_ostr.octets) == 0) ) {
				aux_add_error(EINVALID, "Without a PIN the device key set cannot be installed on the keycard.",CNULL, 0, proc);
				goto err_exit;
			}
			pin_ostr.noctets = strlen(pin_ostr.octets);
			if (check_pin(pin_ostr.octets)) {
				AUX_ADD_ERROR;
				goto err_exit;
			}
		}
		else 
			next_keycard = FALSE;
	
		
	}	/* while */


	free(pin_ostr.octets);
	return (0);



err_exit:

	free(pin_ostr.octets);
	if ((sca_rc = sca_get_sct_info(sct_id)) == M_CARD_INSERTED) {
		/* SC is inserted => eject it */
		if (eject_sc(sct_id, display_text = CNULL, alarm = TRUE)) {
			AUX_ADD_ERROR;
			return(-1);
		}
	}

	return(-1);


}				/* write_on_keycards */






/*--------------------------------------------------------------*/
/*						                */
/* PROC  delete_devkeyset				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Delete DevKeySet stored in the SCT.				*/
/*  								*/
/*  This function tries to delete all keys of the device key 	*/
/*  set.							*/
/*  Error cases:						*/
/*  If one or more keys are unknown (EKEYUNKNOWN) to the SCT,	*/
/*  the other device key(s) will be deleted. In any other error */
/*  situation this function ends immediately with "return(-1)".	*/
/*							        */
/*  If at least one key of the DevKeySet is unknown, the Error-	*/
/*  Number in the error-stack is set to ESCDEVKEY (Device Key  	*/
/*  unknown) and -1 is returned.				*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   app_id		       Application identifier.		*/
/*   key_dev_info[]		List of pointers. For each 	*/
/*				device key (incl. its info.)    */
/*				one pointer.			*/
/*				No memory is allocated. 	*/
/*   no_of_devkeys		Number of relevant entries in 	*/
/*				the list "key_dev_info[]".	*/
/*   keyset_status	       DEV_OWN specifies a devkeyset	*/
/*			         which is used by the SCT for   */
/*				 authentication and secure 	*/
/*				 messaging with the SC.		*/
/*			       DEV_ANY specifies a devkeyset    */
/*			 	 which can only be used for the */
/*				 distribution of this set (on   */
/*				 keycards, on SCs,resp.).	*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*				All device keys could be deleted*/
/*  -1			       Error			       	*/
/*				One or more device keys could	*/
/*				not be deleted.			*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sca_del_dev_key()		Delete a device key stored in   */
/*				the SCT.		        */
/*   set_fct_sec_mode()		Set security mode for comm.     */
/*				between DTE/SCT depending on the*/
/*                	        SCA-function to be called.      */
/*                                                              */
/*   aux_add_error()		Add error to error stack	*/
/*                                                              */
/*--------------------------------------------------------------*/




/***************************************************************
 *
 * Procedure delete_devkeyset
 *
 ***************************************************************/
#ifdef __STDC__

int delete_devkeyset(
	OctetString	 *app_id,
	KeyDevSel	 *key_dev_info[],
	unsigned int 	 no_of_devkeys,
	KeyDevStatus	 keyset_status
)

#else

int delete_devkeyset(
	app_id,
	key_dev_info,
	no_of_devkeys,
	keyset_status
)
OctetString	 *app_id;
KeyDevSel	 *key_dev_info[];
unsigned int 	 no_of_devkeys;
KeyDevStatus	 keyset_status;

#endif

{

	unsigned int	sct_id;
	unsigned int	i;
	ICC_SecMess     sm_SC;	/* sec. mode for communication SCT/SC	 */
	unsigned int	no_of_delkeys = 0;
	char		err_text[256];


		


	char           *proc = "delete_devkeyset";

	secsc_errno = NOERR;
	sca_rc = 0;
	sct_id = sc_sel.sct_id;

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif




	/*
	 *   Check the input parameter 
	 */

	if ((!app_id) || (app_id->noctets == 0) || (!app_id->octets)) {
		aux_add_error(EINVALID, "Missing application identifier", CNULL, 0, proc);
		return (-1);
	}

	if ((keyset_status != DEV_OWN) &&
	     (keyset_status != DEV_ANY)) {
		aux_add_error(EINVALID, "Invalid value for Device Key Set status!", CNULL, 0, proc);
		return (-1);
	}



	/* set security mode for SCA-function */
	if (set_fct_sec_mode(sct_id, "sca_del_dev_key", &sm_SC)) {
		aux_add_error(LASTERROR, "Set security mode (prior to delete device key) failed", CNULL, 0, proc);
		aux_free_OctetString(&app_id);
		return (-1);
	}


	for (i = 0; i < no_of_devkeys; i++) {
	
		key_dev_info[i]->key_status = keyset_status;
	
		sca_rc = sca_del_dev_key(sct_id,
			  	  	 key_dev_info[i],
			          	 app_id);

	
		if (sca_rc < 0) {

#ifdef SECSCTEST
			fprintf(stderr, "\nPurpose of key which could not be deleted:  ");
			fprint_KeyPurpose (key_dev_info[i]->key_purpose);
			fprintf(stderr, "\n");
#endif
			secsc_errno = analyse_sca_err(sct_id, "sca_del_dev_key");
			sprintf (err_text, "%s (%s key)", sca_errmsg,  aux_purpose2name(key_dev_info[i]->key_purpose));
			aux_add_error(secsc_errno, "Cannot delete device key", (char *) err_text, char_n, proc);

			if (sca_errno != EKEYUNKNOWN) {	
				aux_free_OctetString(&app_id);
				return (-1);
			}
		}
		else {

#ifdef SECSCTEST
			fprintf(stderr, "\nPurpose of deleted key:  ");
			fprint_KeyPurpose (key_dev_info[i]->key_purpose);
			fprintf(stderr, "\n");
#endif
			no_of_delkeys++;
		}

	}


	if (no_of_delkeys == no_of_devkeys)
		return (0);
	else {
		aux_add_error(ESCDEVKEY, "Device key(s) for application unknown in SCT", CNULL, 0, proc);
		return(-1);

	}

}				/* delete_devkeyset */







/*--------------------------------------------------------------*/
/*						                */
/* PROC  gen_devkeyset				       		*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Generate DevKeySet (ANY) for application in the SCT.	*/
/*								*/
/*  If the generation of one device key fails, this function    */
/*  tries to delete the already generated keys and returns -1.  */
/*  								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   app_id		        Application identifier.		*/
/*   key_dev_info[]		List of pointers. For each 	*/
/*				device key (incl. its info.)    */
/*				one pointer.			*/
/*   no_of_devkeys		Number of relevant entries in 	*/
/*				the list "key_dev_info[]".	*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*  -1			       Error			       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sca_del_dev_key()		Delete a device key stored in   */
/*				the SCT.		        */
/*   sca_gen_dev_key()		Generate a device key.	        */
/*   set_fct_sec_mode()		Set security mode for comm.     */
/*				between DTE/SCT depending on the*/
/*                	        SCA-function to be called.      */
/*                                                              */
/*   aux_add_error()		Add error to error stack	*/
/*                                                              */
/*--------------------------------------------------------------*/



/***************************************************************
 *
 * Procedure gen_devkeyset
 *
 ***************************************************************/
#ifdef __STDC__

int gen_devkeyset(
	OctetString	 *app_id,
	KeyDevSel	 *key_dev_info[],
	unsigned int 	 no_of_devkeys
)

#else

int gen_devkeyset(
	app_id,
	key_dev_info,
	no_of_devkeys
)
OctetString	 *app_id;
KeyDevSel	 *key_dev_info[];
unsigned int 	 no_of_devkeys;

#endif

{

	unsigned int	sct_id;
	unsigned int	no_of_genkeys = 0;
	unsigned int	i;
	ICC_SecMess     sm_SC;	/* sec. mode for communication SCT/SC	 */
	char		err_text[256];
	


	
	char           *proc = "gen_devkeyset";

	secsc_errno = NOERR;
	sca_rc = 0;
	sct_id = sc_sel.sct_id;

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif



	if ((!app_id) || (app_id->noctets == 0) || (!app_id->octets)) {
		aux_add_error(EINVALID, "Missing application identifier", CNULL, 0, proc);
		return (-1);
	}



	/* set security mode for SCA-function */
	if (set_fct_sec_mode(sct_id, "sca_gen_dev_key", &sm_SC)) {
		aux_add_error(LASTERROR, "Set security mode (prior to generate device key) failed", CNULL, 0, proc);
		return (-1);
	}


	for (no_of_genkeys = 0; no_of_genkeys < no_of_devkeys; no_of_genkeys++) {
	
		key_dev_info[no_of_genkeys]->key_status = DEV_ANY;
		sca_rc = sca_gen_dev_key(sct_id,
			  	  	 key_dev_info[no_of_genkeys],
			          	 app_id);

	
		if (sca_rc < 0) {
#ifdef SECSCTEST
			fprintf(stderr, "\nPurpose of key which could not be generated:  ");
			fprint_KeyPurpose (key_dev_info[no_of_genkeys]->key_purpose);
			fprintf(stderr, "\n");
#endif

			secsc_errno = analyse_sca_err(sct_id, "sca_gen_dev_key");
			sprintf (err_text, "%s (%s key)", sca_errmsg,  aux_purpose2name(key_dev_info[no_of_genkeys]->key_purpose));
			aux_add_error(secsc_errno, "Cannot generate device key", (char *) err_text, char_n, proc);

			break;
		}
#ifdef SECSCTEST
		fprintf(stderr, "\nPurpose of generated key:  ");
		fprint_KeyPurpose (key_dev_info[no_of_genkeys]->key_purpose);
		fprintf(stderr, "\n");
#endif

	}


	if (no_of_genkeys >= no_of_devkeys) {
		return (0);
	}
	else {
		/*
		 *  Not all device keys could be generated
		 *  => delete the keys which have already been generated
		 */

		if (no_of_genkeys > 0) {

			for (i = 0; i < no_of_genkeys; i++) {

#ifdef SECSCTEST
				fprintf(stderr, "\nPurpose of key which has been generated and will now be deleted:  ");
				fprint_KeyPurpose (key_dev_info[i]->key_purpose);
				fprintf(stderr, "\n");
#endif
				sca_rc = sca_del_dev_key(sct_id,
			  	  			 key_dev_info[i],
				          	 	 app_id);
				
				if (sca_rc < 0) {
					secsc_errno = analyse_sca_err(sct_id, "sca_del_dev_key");
					sprintf (err_text, "%s (%s key)", sca_errmsg,  aux_purpose2name(key_dev_info[i]->key_purpose));
					aux_add_error(secsc_errno, "Cannot delete generated device key", (char *) err_text, char_n, proc);
					if (sca_errno != EKEYUNKNOWN) 	
						return (-1);
				}
		
			}
		}

		return(-1);
	}

}				/* gen_devkeyset */






/*--------------------------------------------------------------*/
/*						                */
/* PROC  load_devkeyset				       		*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Load DevKeySet from a keycard into the SCT.			*/
/*								*/
/*  Load devkeyset:						*/
/*     If keyset_status = ANY 					*/
/*     => a device key set destined for the distribution of 	*/
/*        this set shall be loaded into the SCT.		*/
/*        'ANY' device keys are referenced by their key_id.	*/
/*    								*/
/*     If keyset_status = OWN 					*/
/*     => a device key set destined for secure messaging and	*/
/*        authentication between SCT and SC shall be loaded 	*/
/*        into the SCT.						*/
/*        'OWN' device keys are referenced by their purpose.	*/
/*								*/
/*  Depending on the keyset_status the key_id(s) / purpose(s)   */
/*  of the installed key(s) and the belonging application id 	*/
/*  are returned by the SCA-IF. These returned values are 	*/
/*  compared with the required values.				*/
/*  								*/
/*  In case of an error the installed key(s) will be deleted.	*/ 
/*								*/
/*								*/
/*  								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   app_id		        Application identifier.		*/
/*   key_dev_info[]		List of pointers. For each 	*/
/*				device key (incl. its info.)    */
/*				one pointer.			*/
/*   no_of_devkeys		Number of relevant entries in 	*/
/*				the list "key_dev_info[]".	*/
/*   keyset_status	       DEV_OWN specifies a devkeyset	*/
/*			         which is used by the SCT for   */
/*				 authentication and secure 	*/
/*				 messaging with the SC.		*/
/*			       DEV_ANY specifies a devkeyset    */
/*			 	 which can only be used for the */
/*				 distribution of this set (on   */
/*				 keycards, on SCs,resp.).	*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*  -1			       Error			       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   fprint_KeyId()		Display values of KeyId on 	*/
/*				stderr.				*/
/*   fprint_KeyPurpose()	Display values of KeyPurpose on */
/*				stderr.				*/
/*   sca_del_dev_key()		Delete a device key stored in   */
/*				the SCT.		        */
/*   sca_enable_deletion()	Compare 'own' device key set in */
/*				the SCT with the device key set */
/*				in a keycard and set the delete */
/*				flag for this set.	        */
/*   sca_load_devkeyset()	Load device key set from a   	*/
/*				keycard into the SCT.	        */
/*   set_fct_sec_mode()		Set security mode for comm.     */
/*				between DTE/SCT depending on the*/
/*                	        SCA-function to be called.      */
/*                                                              */
/*   aux_add_error()		Add error to error stack	*/
/*                                                              */
/*--------------------------------------------------------------*/




/***************************************************************
 *
 * Procedure load_devkeyset
 *
 ***************************************************************/
#ifdef __STDC__

int load_devkeyset(
	OctetString	 *app_id,
	KeyDevSel	 *key_dev_info[],
	unsigned int 	 no_of_devkeys,
	KeyDevStatus	 keyset_status
)

#else

int load_devkeyset(
	app_id,
	key_dev_info,
	no_of_devkeys,
	keyset_status
)
OctetString	 *app_id;
KeyDevSel	 *key_dev_info[];
unsigned int 	 no_of_devkeys;
KeyDevStatus	 keyset_status;
#endif

{

	unsigned int	sct_id;

	KeyDevSel	key_dev_sel;
	unsigned int	i,n;
	OctetString	sca_app_id;
	ICC_SecMess     sm_SC;	/* sec. mode for communication SCT/SC	 */
	KeyDevList	load_key_set;
	KeyPurpose	purpose_set[MAX_DEVKEY_NO + 1];
	KeyId		*keyid_list[MAX_DEVKEY_NO + 1];
	Boolean		devkey_found;
	char		err_text[256];
	

	


	char           *proc = "load_devkeyset";

	secsc_errno = NOERR;
	sca_rc = 0;
	sct_id = sc_sel.sct_id;

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif


	/*
	 *   Check the input parameter 
	 */

	if ((!app_id) || (app_id->noctets == 0) || (!app_id->octets)) {
		aux_add_error(EINVALID, "Missing application identifier", CNULL, 0, proc);
		return (-1);
	}

	if ((keyset_status != DEV_OWN) &&
	     (keyset_status != DEV_ANY)) {
		aux_add_error(EINVALID, "Invalid value for Device Key Set status!", CNULL, 0, proc);
		return (-1);
	}




	/* set security mode for SCA-function */
	if (set_fct_sec_mode(sct_id, "sca_load_devkeyset", &sm_SC)) {
		aux_add_error(LASTERROR, "Set security mode (prior to load device key set) failed", CNULL, 0, proc);
		aux_free_OctetString(&app_id);
		return (-1);
	}

	
	load_key_set.key_status = keyset_status;
	if (keyset_status == DEV_OWN)
		load_key_set.list_ref.purpose_list = purpose_set;
	else 
		load_key_set.list_ref.keyid_list = keyid_list;


	sca_rc = sca_load_devkeyset(sct_id,
				    &sca_app_id, 
				    &load_key_set);

	if (sca_rc < 0) {
		secsc_errno = analyse_sca_err(sct_id, "sca_load_devkeyset");
		aux_add_error(secsc_errno, "Cannot load device key set", (char *) sca_errmsg, char_n, proc);
		keyid_list[0] = KEYIDNULL;
		goto err2_case;
	}


	/*
	 *  Check whether the devkeyset (incl. app_id) which has been installed
	 *  is correct.
	 */

	/* check the app_id */
	if ((sca_app_id.noctets != app_id->noctets) || (!sca_app_id.octets) ||
	     (strncmp(sca_app_id.octets, app_id->octets, app_id->noctets))) {
		aux_add_error(ESCDEVKEY, "Application id of the DevKeySet to be loaded is invalid", CNULL, 0, proc);
		goto err_case;
	}

	

	for (n = 0; n < no_of_devkeys; n++) {
		
		devkey_found = FALSE;
		i = 0;

		if (keyset_status == DEV_OWN) {

			/*
			 *  Compare the returned purpose(s) with the required values
		 	 */
		
			while ((*(load_key_set.list_ref.purpose_list + i) != NULL) &&
		    	   (devkey_found == FALSE)) {

#ifdef SECSCTEST
				fprintf(stderr, "\nPurpose of loaded key:  ");
				fprint_KeyPurpose (purpose_set[i]);
				fprintf(stderr, "\n");
#endif
				if (purpose_set[i] == key_dev_info[n]->key_purpose)
					devkey_found = TRUE;

				i++;
			}
		}
		else {

			/*
			 *  Compare the returned KeyId(s) with the required values
		 	 */

			while ((keyid_list[i] != KEYIDNULL) && (devkey_found == FALSE)) {
#ifdef SECSCTEST
				fprintf(stderr, "\nKeyId of loaded key:  ");
				fprint_KeyId (keyid_list[i]);
				fprintf(stderr, "\n");
#endif
				if ((keyid_list[i]->key_level == key_dev_info[n]->key_id->key_level) &&
				    (keyid_list[i]->key_number == key_dev_info[n]->key_id->key_number))

					devkey_found = TRUE;
		
				i++;
			}


		}  /* end else */

		if (devkey_found != TRUE) {
			if (key_dev_info[n]->key_purpose == PURPOSE_AUTH)
				strcpy (err_text,"Authentication key");
			else if (key_dev_info[n]->key_purpose == PURPOSE_SECAUTH)
				strcpy (err_text, "Secure Messaging (Authentic)");
				else if (key_dev_info[n]->key_purpose == PURPOSE_SECCONC)
					strcpy (err_text, "Secure Messaging (Concealed)");

			aux_add_error(ESCDEVKEY, "Required device key has not been loaded into the SCT", (char *) err_text, char_n, proc);
			goto err_case;
		}

	}  /* end for */


	aux_free2_OctetString(&sca_app_id);

	if (keyset_status == DEV_ANY) {
		i = 0;
		while (keyid_list[i] != KEYIDNULL) {
			free(keyid_list[i]);
			i++;
		}
	}


	return(0);	
	
		

err_case:

	/*
	 *  Delete the keys, which have been loaded from a keycard in the SCT
	 */
		
	/* set security mode for SCA-function */
	if (set_fct_sec_mode(sct_id, "sca_del_dev_key", &sm_SC)) {
		aux_add_error(LASTERROR, "Set security mode (prior to delete loaded device key set) failed", CNULL, 0, proc);
		goto err2_case;
	}

	key_dev_sel.key_status = keyset_status;

	
	i = 0;

	if (keyset_status == DEV_OWN) {

		/*
		 *  Enable deletion of an 'own' device key set
		 */

		/* set security mode for SCA-function */
		if (set_fct_sec_mode(sct_id, "sca_enable_deletion", &sm_SC)) {
			aux_add_error(LASTERROR, "Set security mode (prior to enable deletion of loaded device key set) failed", CNULL, 0, proc);
			goto err2_case;
		}

		sca_rc = sca_enable_deletion(sct_id,
			          	     sca_app_id,
		  	  	 	     purpose_set,
		          	 	     &sm_SC);
			
		if (sca_rc < 0) {
			secsc_errno = analyse_sca_err(sct_id, "sca_enable_deletion");
			aux_add_error(secsc_errno, "Deletion of loaded device key failed.", (char *) sca_errmsg, char_n, proc);
			goto err2_case;
		}
		
		/*
		 *  Delete DEV-OWN key(s)
		 */
		
		while (*(load_key_set.list_ref.purpose_list + i) != NULL) {

			key_dev_sel.key_purpose = purpose_set[i];
			sca_rc = sca_del_dev_key(sct_id,
			  	  	 	 &key_dev_sel,
			          	 	 sca_app_id);
			
			if (sca_rc < 0) {
				secsc_errno = analyse_sca_err(sct_id, "sca_del_dev_key");
				aux_add_error(secsc_errno, "Deletion of loaded device key failed.", (char *) sca_errmsg, char_n, proc);
				if (sca_errno != EKEYUNKNOWN) {	
					goto err2_case;
				}
			}

			i++;
		}
	}
	else {

		/*
		 *  Delete DEV-ANY key(s)
		 */

		while (keyid_list[i] != KEYIDNULL) {

			key_dev_sel.key_id = keyid_list[i];
			sca_rc = sca_del_dev_key(sct_id,
			  	  	 	 &key_dev_sel,
			          	 	 sca_app_id);
			
			if (sca_rc < 0) {
				secsc_errno = analyse_sca_err(sct_id, "sca_del_dev_key");
				aux_add_error(secsc_errno, "Deletion of loaded device key failed.", (char *) sca_errmsg, char_n, proc);
				if (sca_errno != EKEYUNKNOWN) {	
					goto err2_case;
				}
			}
		
			i++;
		}


	}  /* end else */


err2_case:

	aux_free2_OctetString(&sca_app_id);

	if (keyset_status == DEV_ANY) {
		i = 0;
		while (keyid_list[i] != KEYIDNULL) {
			free(keyid_list[i]);
			i++;
		}
	}

	return(-1);

}				/* load_devkeyset */








/*--------------------------------------------------------------*/
/*						                */
/* PROC  save_devkeyset					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Save DevKeySet ('any') on a keycard.			*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   app_id		        Application identifier.		*/
/*   key_dev_info[]		List of pointers. For each 	*/
/*				device key (incl. its info.)    */
/*				one pointer.			*/
/*   no_of_devkeys		Number of relevant entries in 	*/
/*				the list "key_dev_info[]".	*/
/*   keycard_pin		PIN, which is installed on the 	*/
/*				keycard and which is used to 	*/
/*				protect the read access on the 	*/
/*				keycard.			*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*  -1			       Error			       	*/
/*				The keycard is useless for the	*/
/*				user.				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sca_save_devkeyset()	Save device key set on a 	*/
/*				keycard.			*/
/*   set_fct_sec_mode()		Set security mode for comm.     */
/*				between DTE/SCT depending on the*/
/*                	        SCA-function to be called.      */
/*                                                              */
/*   aux_add_error()		Add error to error stack	*/
/*                                                              */
/*--------------------------------------------------------------*/




/***************************************************************
 *
 * Procedure save_devkeyset
 *
 ***************************************************************/
#ifdef __STDC__

static int save_devkeyset(
	OctetString	 *app_id,
	KeyDevSel	 *key_dev_info[],
	unsigned int 	 no_of_devkeys,
	OctetString	 *keycard_pin
)

#else

static int save_devkeyset(
	app_id,
	key_dev_info,
	no_of_devkeys,
	keycard_pin
)
OctetString	 *app_id;
KeyDevSel	 *key_dev_info[];
unsigned int 	 no_of_devkeys;
OctetString	 *keycard_pin;

#endif

{

	unsigned int	sct_id;

	unsigned int	i;
	ICC_SecMess     sm_SC;	/* sec. mode for communication SCT/SC	 */
	VerifyDataStruc verify_data;
	KeyAttrList	key_attr_list;
	KeyId		*keyid_list[MAX_DEVKEY_NO + 1];


		


	char           *proc = "save_devkeyset";

	secsc_errno = NOERR;
	sca_rc = 0;
	sct_id = sc_sel.sct_id;

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif


	if ((!app_id) || (app_id->noctets == 0) || (!app_id->octets)) {
		aux_add_error(EINVALID, "Missing application identifier", CNULL, 0, proc);
		return (-1);
	}

	if ((!keycard_pin) || (!keycard_pin->octets)) {
		aux_add_error(EINVALID, "Missing input parameter (app_name,keycard_pin)", CNULL, 0, proc);
		return (-1);
	}




	/* set security mode for SCA-function */
	if (set_fct_sec_mode(sct_id, "sca_save_dev_key", &sm_SC)) {
		aux_add_error(LASTERROR, "Set security mode (prior to save device key set) failed", CNULL, 0, proc);
		return (-1);
	}


	verify_data.verify_data_type = PIN;
	verify_data.VerifyDataBody.pin_info.min_len = 4;
	verify_data.VerifyDataBody.pin_info.pin  = keycard_pin;
	verify_data.VerifyDataBody.pin_info.disengageable  = FALSE;
		
	key_attr_list.key_inst_mode = INST;
	key_attr_list.key_attr.key_repl_flag = FALSE;	 /* PIN is not replaceable */
	key_attr_list.key_attr.key_chg_flag = TRUE;	 /* PIN is changeable  */
	key_attr_list.key_attr.key_foreign_flag = FALSE; /* PIN is not replaceable by
							    a key with a different
							    algorithm.         */		
	key_attr_list.key_attr.key_purpose = PURPOSE_AUTH;
	key_attr_list.key_attr.key_presentation = KEY_LOCAL;
	key_attr_list.key_attr.MAC_length = 4;

	key_attr_list.key_fpc = 4;
	key_attr_list.key_status.PIN_check = TRUE;
	key_attr_list.key_status.key_state = FALSE; 



	for (i = 0; i < no_of_devkeys; i++) 	
		keyid_list[i] = key_dev_info[i]->key_id;
	keyid_list[i] = KEYIDNULL;


	sca_rc = sca_save_devkeyset(sct_id,
			  	    &verify_data,
				    &key_attr_list,
				    app_id,
				    keyid_list);

	
	if (sca_rc < 0) {
		secsc_errno = analyse_sca_err(sct_id, "sca_save_devkeyset");
		aux_add_error(secsc_errno, "Cannot save device key set", (char *) sca_errmsg, char_n, proc);
		return (-1);
	}


	return (0);


}				/* save_devkeyset */






/*--------------------------------------------------------------*/
/*						                */
/* PROC  aux_purpose2name				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Transform purpose (type KeyPurpose) into a printable	*/
/*  character representation.					*/
/*							        */
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   purpose		       					*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   Character String	        Printable character 		*/
/*				representation of the given 	*/
/*				purpose.		       	*/
/*   CNULL			Given purpose unknown.	       	*/
/*							       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*                                                              */
/*--------------------------------------------------------------*/




/***************************************************************
 *
 * Procedure aux_purpose2name
 *
 ***************************************************************/
#ifdef __STDC__

static char  *aux_purpose2name(
		KeyPurpose	  purpose
)

#else

static char  *aux_purpose2name(
		purpose
)
KeyPurpose		 purpose;

#endif

{

	

	char           *proc = "aux_purpose2name";


#ifdef SECSCTEST
/*	fprintf(stderr, "SECSC-Function: %s\n", proc); */
#endif

	if (purpose == PURPOSE_AUTH)
		return ("authentication");
	else

	if (purpose == PURPOSE_SECAUTH)
		return ("secure messaging (authentic)");
	else

	if (purpose == PURPOSE_SECCONC)
		return ("secure messaging (concealed)");
	else 
		return (CNULL);

}				/* aux_purpose2name */









/*--------------------------------------------------------------*/
/*						                */
/* PROC  device_authentication				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Perform device authentication.				*/
/*								*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*  In the current version the device authentication is done in */
/*  any case, the addinitional file information are not 	*/
/*  evaluated.							*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   sct_id		       SCT identifier			*/
/*   app_name		       Application name.		*/
/*   add_info		       additional file information	*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*  -1			       Error			       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sca_auth()			Device authentication  		*/
/*   set_fct_sec_mode()		Set security mode for comm.     */
/*				between DTE/SCT depending on the*/
/*                	        SCA-function to be called.      */
/*                                                              */
/*   aux_add_error()		Add error to error stack	*/
/*                                                              */
/*--------------------------------------------------------------*/




/***************************************************************
 *
 * Procedure device_authentication
 *
 ***************************************************************/
#ifdef __STDC__

static int device_authentication(
	unsigned int	  sct_id,
	char		 *app_name,
	AddInfo		 *add_info
)

#else

static int device_authentication(
	sct_id,
	app_name,
	add_info
)
unsigned int	  sct_id;
char		 *app_name;
AddInfo		 *add_info;

#endif

{

	KeyId           key_id;
	Boolean		SC_internal;
	ICC_SecMess     sm_SC;	/* sec. mode for communication SCT/SC	 */
	int             auth_rc = 0;
	AuthProcId	auth_proc_id;
	AuthObjectId	auth_object_id;
		


	char           *proc = "device_authentication";

	secsc_errno = NOERR;
	sca_rc = 0;

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);

	fprintf(stderr, "                Additional file information:\n");
	fprintf(stderr, "                ");
	aux_xdump(add_info->octets, add_info->noctets, 0);
	fprintf(stderr, "\n");
#endif




	/* set security mode for SCA-function */
	if (set_fct_sec_mode(sct_id, "sca_auth", &sm_SC)) {
		aux_add_error(LASTERROR, "Set security mode (prior to the device authentiation) failed", CNULL, 0, proc);
		return (-1);
	}

	auth_rc = sca_auth(sct_id,
			  auth_proc_id = MU_AUTH,
			  auth_object_id = SCT_ICC,
			  KEYIDNULL,
			  NULLOCTETSTRING,
			  KEYINFONULL,
			  &sm_SC,
			  NULLOCTETSTRING);




/* with Set in EEPROM */
/*	auth_rc = sca_auth(sct_id,
			  auth_proc_id = MU_AUTHWithINITIAL,
			  auth_object_id = SCT_ICC,
			  KEYIDNULL,
			  NULLOCTETSTRING,
			  KEYINFONULL,
			  &sm_SC,
			  NULLOCTETSTRING);
*/


	if (auth_rc < 0) {
		secsc_errno = analyse_sca_err(sct_id, "sca_auth");
		aux_add_error(secsc_errno, "Device authentication failed", (char *) sca_errmsg, char_n, proc);
		return (-1);
	}

	return (0);

}				/* device_authentication */




/*--------------------------------------------------------------*/
/*						                */
/* PROC  user_authentication				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  According to parameter pin_type (PIN, PUK) the user 	*/
/*  authentication is performed.				*/
/*								*/
/*  If pin_type == PIN =>  PIN authentication:			*/
/*	    If not yet done the user authentication is 		*/ 
/*	    performed:						*/
/*	    The PIN is handled like an object (name = SC_PIN).	*/
/*	    The key_id of the PIN is taken from the    		*/
/*          sc_app_list[].sc_obj_list[].			*/
/*          If user enters an incorrect PIN, the PIN-check is   */
/*	    repeated two times.					*/
/*								*/
/*  If pin_type == PUK =>  Unblock PIN with PUK:		*/
/*	    The PUK is handled like an object (name = SC_PUK).	*/
/*	    The key_id of the PUK is taken from the    		*/
/*          sc_app_list[].sc_obj_list[].			*/
/*          If user enters an incorrect PUK, the unblock_PIN is */
/*	    repeated two times.					*/
/*								*/
/*  If the user authentication was successful, parameter	*/
/*  "user_auth_done" in "sct_stat_list[sct_id]" is set to TRUE	*/
/*								*/
/*								*/
/*  - Global variable "sct_stat_list[sct_id]" contains 		*/
/*    current information for the SCT specified by sct_id.	*/
/*							        */
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   sct_id		       SCT identifier			*/
/*   app_name		       Application name.		*/
/*   pin_type		       {PIN, PUK}			*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*  -1			       Error			       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   sca_verify_pin()		PIN authentication  		*/
/*   sca_display()		Display text on SCT-display.    */
/*   sca_unblock_pin()		Unblock PIN with PUK.		*/
/*                                                              */
/*   aux_AppObjName2SCObj()	Get information about an SC     */
/*        			object belonging to an 		*/
/*                              application on SC. 		*/
/*   bell_function()		"Ring the bell" to require user */
/*                              input at the SCT.		*/
/*   get_SC_keyid()		Compose structure KeyId (for a  */
/*				key on the SC).			*/
/*   prepare_sec_mess()		- Generate session key,	    	*/
/*				- Set security mode for the 	*/
/*				  communication between DTE/SCT.*/
/*   write_SCT_config()		Encrypt and write SCT configuration data  */
/*				for the specified SCT.		*/
/*                                                              */
/*   analyse_sca_err()		Analyse of an error-number 	*/
/*				returned by an SCA-IF function. */
/*   aux_add_error()		Add error to error stack	*/
/*                                                              */
/*--------------------------------------------------------------*/




/***************************************************************
 *
 * Procedure user_authentication
 *
 ***************************************************************/
#ifdef __STDC__

static int user_authentication(
	unsigned int	  sct_id,
	char		 *app_name,
	VerifyDataType	  pin_type
)

#else

static int user_authentication(
	sct_id,
	app_name,
	pin_type
)
unsigned int	  sct_id;
char		 *app_name;
VerifyDataType	  pin_type;

#endif

{
	KeyId           key_id;
	Boolean		SC_internal;
	SCT_SecMess     sm_SCT;	/* sec. mode for communication DTE/SCT	 */
	SCObjEntry     *sc_obj_entry;
	int             time_out;
	char           *display_text;
	int             auth_rc = 0;
	int             auth_errno;
	char           *auth_errmsg;

	char           *err_text_lock;
	char           *err_text_inv;
	char           *add_error_text;
	int             errno_lock;

	int             auth_attempts;	/* no. of attempts to check
					 * the PIN	 */

	char           *proc = "user_authentication";

	secsc_errno = NOERR;
	sca_rc = 0;

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
	if (pin_type == PIN) 
		fprintf(stderr, "                   with PIN.\n");
	if (pin_type == PUK) 
		fprintf(stderr, "                   with PUK.\n");

#endif

/*
 *      Perform the PIN | PUK authentication.
 *			  (If necessary: three times)
 */


	switch (pin_type) {

	case (PIN):
		if (sct_stat_list[sct_id].user_auth_done == TRUE) {
			/* 
			 *  The user authentication has already been performed.
			 */
			return (0);
		}
	
		/* PIN Authentication */

		/* get information about the object SC_PIN */
		sc_obj_entry = aux_AppObjName2SCObj(app_name, SC_PIN_name);
		if (sc_obj_entry == NULLSCOBJENTRY) {
			aux_add_error(EINVALID, "Cannot get configuration info for SC-object", SC_PIN_name, char_n, proc);
			return (-1);
		}
		err_text_lock = SCT_TEXT_PIN_LOCKED;
		err_text_inv  = SCT_TEXT_PIN_INVALID;
		errno_lock    = ESCPINLOCK;
		add_error_text = "PIN authentication failed";
		break;

	case (PUK):
		/* Unblock PIN with PUK */

		/* get information about the object SC_PUK */
		sc_obj_entry = aux_AppObjName2SCObj(app_name, SC_PUK_name);
		if (sc_obj_entry == NULLSCOBJENTRY) {
			aux_add_error(EINVALID, "Cannot get configuration info for SC-object", SC_PUK_name, char_n, proc);
			return (-1);
		}
		err_text_lock = SCT_TEXT_PUK_LOCKED;
		err_text_inv  = SCT_TEXT_PUK_INVALID;
		errno_lock    = ESCPUKLOCK;
		add_error_text = "Unblocking PIN failed";
		break;

	default:
		aux_add_error(EINVALID, "Invalid value for pin_type", CNULL, 0, proc);
		return (-1);

	}			/* end case */

	/* set security mode for writing an SC-object */
	sm_SCT.command = sc_obj_entry->sm_SCT;
	sm_SCT.response = SCT_SEC_NORMAL;
	if (prepare_sec_mess(sct_id, &sm_SCT, FALSE)) {
		aux_add_error(LASTERROR, "Prepare secure messaging (prior to user authentication", CNULL, 0, proc);
		return (-1);
	}
	get_SC_keyid(sc_obj_entry->sc_id, SC_internal = TRUE, &key_id);

	/* check PIN/PUK  (if user enters invalid PIN/PUK, repeat 2 times) */
	auth_attempts = 0;	
	do {
		bell_function();
		if (pin_type == PIN)
			auth_rc = sca_verify_pin(sct_id,
						 &key_id,
						 &sc_obj_entry->sm_SC_write);
		else
			auth_rc = sca_unblock_pin(sct_id,
						  &key_id,
						  &sc_obj_entry->sm_SC_write);

		auth_errno = sca_errno;
		auth_errmsg = sca_errmsg;
		auth_attempts++;

		if (auth_rc < 0) {
			switch (sca_errno) {

			case EKEY_LOCKED:
			case ELAST_ATTEMPT:
				aux_add_error(errno_lock, add_error_text, (char *) sca_errmsg, char_n, proc);

				/* display message on SCT-Display */
				sca_rc = sca_display(sct_id,
					       	     display_text = err_text_lock,
						     time_out = 0);
				if (sca_rc < 0) {
					fprintf(stderr, "Cannot display error text: %s on SCT display\n", err_text_lock);
					return (-1);
				}
				return (-1);	/* PIN | PUK on SC is locked */
				break;

			case EAUTH_1ATTEMPT:
			case EAUTH_2ATTEMPT:
			case EAUTH_3ATTEMPT:
			case EAUTH_4ATTEMPT:
			case EAUTH_5ATTEMPT:
			case EAUTH_6ATTEMPT:
			case EAUTH_7ATTEMPT:
				if (sca_errno == EAUTH_1ATTEMPT)
					fprintf(stderr, "\n\nAuthentication failed; 1 attempt left\n\n");
				else if (sca_errno == EAUTH_2ATTEMPT)
					fprintf(stderr, "\n\nAuthentication failed; 2 attempts left\n\n");
				else if (sca_errno == EAUTH_3ATTEMPT)
					fprintf(stderr, "\n\nAuthentication failed; 3 attempts left\n\n");
				else if (sca_errno == EAUTH_4ATTEMPT)
					fprintf(stderr, "\n\nAuthentication failed; 4 attempts left\n\n");
				else if (sca_errno == EAUTH_5ATTEMPT)
					fprintf(stderr, "\n\nAuthentication failed; 5 attempts left\n\n");
				else if (sca_errno == EAUTH_6ATTEMPT)
					fprintf(stderr, "\n\nAuthentication failed; 6 attempts left\n\n");
				else if (sca_errno == EAUTH_7ATTEMPT)
					fprintf(stderr, "\n\nAuthentication failed; 7 or more attempts left\n\n");
				sca_rc = sca_display(sct_id,
						     display_text = err_text_inv,
						     time_out = 0);
				if (sca_rc < 0) {
					fprintf(stderr, "Cannot display error text: %s on SCT display\n", err_text_inv);
					return (-1);
				}
				break;

			case EINV_LEN:
				sca_rc = sca_display(sct_id,
						     display_text = SCT_TEXT_NEW_PIN_INV,
						     time_out = 0);
				if (sca_rc < 0) {
					fprintf(stderr, "Cannot display error text: %s on SCT display\n", SCT_TEXT_NEW_PIN_INV);
					return (-1);
				}
				break;

			case ECPIN:
				sca_rc = sca_display(sct_id,
						     display_text = SCT_TEXT_NEW_PIN_CPIN,
						     time_out = 0);
				if (sca_rc < 0) {
					fprintf(stderr, "Cannot display error text: %s on SCT display\n", SCT_TEXT_NEW_PIN_CPIN);
					return (-1);
				}
				break;

			default:
				if (pin_type == PIN)
					secsc_errno = analyse_sca_err(sct_id, "sca_verify_pin");
				else 
					secsc_errno = analyse_sca_err(sct_id, "sca_unblock_pin");
				aux_add_error(secsc_errno, add_error_text, CNULL, 0, proc);
				return (-1);

			}	/* end switch */
		}
		 /* end if */ 
		else {
			/* PIN authentication | unblocking PIN was successful */

			sct_stat_list[sct_id].user_auth_done = TRUE;

			/*
			 *  Save the changed SCT configuration data
			 */

			if ((write_SCT_config(sct_id)) < 0) {
				AUX_ADD_ERROR;
				return (-1);
			}

			return (0);
		}

	}			/* end do */
	while (auth_attempts < MAX_PIN_FAIL);

	/* after 3 unsuccessful attempts: authentication fails */
	sca_errno = auth_errno;
	sca_errmsg = auth_errmsg;
	if (pin_type == PIN)
		secsc_errno = analyse_sca_err(sct_id, "sca_verify_pin");
	else 
		secsc_errno = analyse_sca_err(sct_id, "sca_unblock_pin");
	aux_add_error(secsc_errno, add_error_text, "after 3 attempts", char_n, proc);
	return (-1);

}				/* user_authentication */










/*--------------------------------------------------------------*/
/*						                */
/* PROC  handle_gen_DecSK				       	*/
/*   								*/
/* DESCRIPTION						       	*/
/*								*/
/*  This routine checks under which name the decryption key to 	*/
/*  be generated shall be installed on the SC.			*/
/*  If this routine sets the output parameter "new_DecSKname"	*/
/*  to CNULL, the key shall be installed under the name given   */
/*  in "key".							*/
/*  Otherwise the calling routine has to store the key under    */
/*  the name returned in "new_DecSKname".			*/
/*							       	*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   key	 	       Structure to identify the key.	*/
/*   replace		       = FALSE => Install new key	*/
/*   			       = TRUE  => Replace existing key	*/
/*   new_DecSKname	       **char				*/
/*   new_replace_value	       Pointer on type Boolean.		*/	
/*								*/
/*							       	*/
/*							       	*/
/* OUT							       	*/
/*   *new_DecSKname	       = CNULL  => no change of object 	*/
/*			                   name			*/
/*			       != CNULL => object name, under   */
/*					   which the new decryption key */
/*					   shall be installed on the SC.*/ 
/*  *new_replace_value	       replace value for the installation of    */
/*			       the decryption key under the name in     */
/*			       new_DecSKname.		        	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0			       o.k.				*/
/*   -1			       Error				*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   get_update_time_SCToc()	Get update time of object in 	*/
/*				SC-Toc.				*/
/*   aux_AppObjName2SCObj()	Get information about an SC     */
/*        			object belonging to an 		*/
/*                              application on SC. 		*/
/*   aux_cmp_UTCTime()		Compare two time-values  	*/
/*				(UTCTime).			*/
/*   aux_cpy_String()		Copy string.			*/
/*                                                              */
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*							       	*/
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure handle_gen_DecSK
 *
 ***************************************************************/
#ifdef __STDC__

static int handle_gen_DecSK(
	Key	 *key,
	Boolean	  replace,
	char	**new_DecSK_name,
	Boolean	 *new_replace_value
)

#else

static int handle_gen_DecSK(
	key,
	replace,
	new_DecSK_name,
	new_replace_value
)
Key	 *key;
Boolean	  replace;
char	**new_DecSK_name;
Boolean	 *new_replace_value;

#endif

{

	UTCTime	       *DecSKnew_update = (UTCTime *)0;
	UTCTime	       *DecSKold_update = (UTCTime *)0;
	char	       *save_obj_name;
	char 	       *old_key_name;

	char           *proc = "handle_gen_DecSK";

#ifdef SECSCTEST
	fprintf(stderr, "%s\n", proc);
#endif


	*new_DecSK_name = CNULL;


	if ((key->keyref != 0) || (key->pse_sel == (PSESel * ) 0) ||
	    (!key->pse_sel->object.name) || (!strlen(key->pse_sel->object.name)))

		return(0);


	/*
	 * Select key with object name
	 */

	if (!(strcmp(key->pse_sel->object.name, DecSKold_name)) ||
	    !(strcmp(key->pse_sel->object.name, SKold_name))) {
		aux_add_error(EINVALID, "Not allowed to generate old decryption key!", CNULL, 0, proc);
		return (-1);
	}

	if (!(strcmp(key->pse_sel->object.name, DecSKnew_name)) ||
	    !(strcmp(key->pse_sel->object.name, SKnew_name))) {

		/*
		 *  Get update time of DecSKnew (SKnew) on the SC
		 */

		if (get_update_time_SCToc(key->pse_sel, &DecSKnew_update)) {
			aux_add_error(EOBJNAME, "Error in SCToc", key->pse_sel->object.name, char_n, proc);
			return (-1);
		}
		if (DecSKnew_update == (UTCTime *)0) {

			/* 
			 *  DecSKnew (SKnew) not yet installed on the SC 
			 *     => if replace = TRUE, return(error)  
			 *                           (There is no key, which can be replaced)
			 *     => if replace = FALSE, key can be installed under the name of
			 *                            DecSKnew | SKnew.
			 */

			if (replace == TRUE) {
				aux_add_error(EINVALID, "Key to be replaced doesn't exist!", CNULL, 0, proc);
				return (-1);
			}
		}	
		else {

			/* 
			 *  DecSKnew (SKnew) is already installed on the SC 
			 *     => if replace = FALSE, return(error)
			 *     => if replace = TRUE and DecSKold (SKold) is an object on the SC, check DecSKold
			 */
	
			if (replace == FALSE) {
				aux_add_error(EINVALID, "Key exists already and replace flag is not set!", (char *) key->pse_sel, PSESel_n, proc);
				return (-1);
			}

			/*
			 *  DecSKold (SKold) an object on the SC ?
			 */

			if (!(strcmp(key->pse_sel->object.name, DecSKnew_name)))
				old_key_name = DecSKold_name;
			else    old_key_name = SKold_name;
 
			if ((aux_AppObjName2SCObj(key->pse_sel->app_name, old_key_name)) != NULLSCOBJENTRY) {

				/*
	 			 *  Get update time of DecSKold (SKold) on the SC
				 */

				save_obj_name = key->pse_sel->object.name;
				key->pse_sel->object.name = old_key_name;
				if (get_update_time_SCToc(key->pse_sel, &DecSKold_update)) {
					aux_add_error(EOBJNAME, "Error in SCToc", key->pse_sel->object.name, char_n, proc);
					return (-1);
				}
				key->pse_sel->object.name = save_obj_name;

				if (DecSKold_update == (UTCTime *)0) {

					/* 
					 *  DecSKold (SKold) not yet installed on the SC 
					 *     => - Key shall be installed under the name of DecSKold (SKold).
					 *        - For the SC this is a new key, therefore the parameter
					 *          new_replace_value is set to FALSE.
					 */

					*new_DecSK_name = aux_cpy_String(old_key_name);
					*new_replace_value = FALSE;
				}
				else {

					/* 
					 *  Both DecSKnew (SKnew) and DecSKold  (SKold) are
					 *   already installed on the SC 
					 *     => Get the name of the oldest decryption key.
					 */

						
					if ((aux_cmp_UTCTime(DecSKnew_update, DecSKold_update)) == 1) {

						/* 
						 *  DecSKold (SKold) is older than 
					         *    DecSKnew (SKnew)
						 *     => Key shall be installed under the 
						 *        name of DecSKold (SKold).
						 */ 

						*new_DecSK_name = aux_cpy_String(old_key_name);
					}
				}
			}
		}
	}		/* object_name = DecSKnew_name (SKnew_name) */


	return(0);


}				/* handle_gen_DecSK */






/*--------------------------------------------------------------*/
/*						                */
/* PROC  get_DecSK_name					       	*/
/*   								*/
/* DESCRIPTION						       	*/
/*								*/
/*  Returns the object name under which the decryption key	*/
/*  is stored on the SC.			 		*/
/*								*/
/*  The value of the input parameter "pse_sel->object.name"  	*/
/*  is "DecSK_new_name", "DecSKold_name", "SK_new_name",	*/
/*  "SK_old_name", resp..					*/
/*							       	*/
/*  "DecSKnew" ("SKnew"):					*/
/*  If the given object name is set to "DecSKnew" ("SKnew"), 	*/
/*  this routine returns the name of the decryption key on the  */
/*  SC, which has been changed last.				*/
/*  								*/
/*  "DecSKold" ("SKold"):					*/
/*  If the given object name is set to "DecSKold" ("SKold"),    */
/*  this routine returns the name of the oldest decryption key  */
/*  on the SC.							*/
/*  								*/
/*  This routine gets the entries of "DecSKnew" (SKnew") and 	*/
/*  "DecSKold" ("SKold") from SCToc and checks/compares the 	*/
/*  update-time of the keys to decide which key is the new one  */
/*  and which is the old one.					*/
/*							       	*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   pse_sel	       						*/
/*							       	*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   CNULL		       Error				*/
/*   ptr. to object name       Name under which the decryption  */
/*			       key is stored on the SC.		*/
/*							       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   get_update_time_SCToc()	Get update time of object in 	*/
/*				SC-Toc.				*/
/*   aux_AppObjName2SCObj()	Get information about an SC     */
/*        			object belonging to an 		*/
/*                              application on SC. 		*/
/*   aux_cmp_UTCTime()		Compare two time-values  	*/
/*				(UTCTime).			*/
/*   aux_cpy_String()		Copy string.			*/
/*                                                              */
/*   analyse_sca_err()		Analyse of an error-number 	*/
/*				returned by an SCA-IF function. */
/*   aux_add_error()		Add error to error stack.	*/
/*			         		       		*/
/*							       	*/
/*--------------------------------------------------------------*/
static
/***************************************************************
 *
 * Procedure get_DecSK_name
 *
 ***************************************************************/
#ifdef __STDC__

char *get_DecSK_name(
	PSESel	 *pse_sel
)

#else

char *get_DecSK_name(
	pse_sel
)
PSESel	 *pse_sel;

#endif

{

	UTCTime	       *DecSKnew_update = (UTCTime *)0;
	UTCTime	       *DecSKold_update = (UTCTime *)0;
	char	       *save_obj_name;
	char 	       *old_key_name;
	char 	       *new_key_name;


	char           *proc = "get_DecSK_name";

#ifdef SECSCTEST
	fprintf(stderr, "%s\n", proc);
#endif


	if (!(strcmp(pse_sel->object.name, DecSKnew_name))) {
		new_key_name = DecSKnew_name;
		old_key_name = DecSKold_name;
	}
	else {
		new_key_name = SKnew_name;
		old_key_name = SKold_name;
	}
 


	/*
	 *  Get update time of DecSKnew (SKnew) on the SC
	 */

	save_obj_name = pse_sel->object.name;
	pse_sel->object.name = new_key_name;
	if (get_update_time_SCToc(pse_sel, &DecSKnew_update)) {
		aux_add_error(EOBJNAME, "Error in SCToc", pse_sel->object.name, char_n, proc);
		return (CNULL);
	}
	pse_sel->object.name = save_obj_name;

	if (DecSKnew_update == (UTCTime *)0) {

		/* 
		 *  DecSKnew not yet installed on the SC 
		 */

		aux_add_error(EINVALID, "No decryption key stored on the SC!", pse_sel->object.name, char_n, proc);
		return (CNULL);

	}


	/*
	 *  Intermediate result:  DecSKnew is installed on the SC 
	 *
	 *           Next to do:  If DecSKold is an object on the SC
	 *			  => Get update time of DecSKold on the SC
	 */

	if ((aux_AppObjName2SCObj(pse_sel->app_name, old_key_name)) != NULLSCOBJENTRY) {

		save_obj_name = pse_sel->object.name;
		pse_sel->object.name = old_key_name;
		if (get_update_time_SCToc(pse_sel, &DecSKold_update)) {
			aux_add_error(EOBJNAME, "Error in SCToc", pse_sel->object.name, char_n, proc);
			return (CNULL);
		}
		pse_sel->object.name = save_obj_name;
	}



	if (!(strcmp(pse_sel->object.name, new_key_name))) {

		/*
		 *  Search for the decryption key which has been changed last.
		 */

		if (DecSKold_update == (UTCTime *)0) {

			/* 
			 *  DecSKold is not installed on the SC 
			 *    => DecSKnew has been changed last 
			 */

			return (aux_cpy_String(new_key_name));
		}
		
		/*
		 *  Intermediate result:  Both DecSKnew and DecSKold are installed on the SC 
		 *
		 *           Next to do:  Return the name of the key which has been changed last
		 */

						
		if ((aux_cmp_UTCTime(DecSKnew_update, DecSKold_update)) == 1) {

			/* 
			 *  DecSKnew is the key which has been changed last
			 */ 

			return (aux_cpy_String(new_key_name));
		}
		else  {

			/* 
			 *  DecSKold is the key which has been changed last
			 */ 

			return(aux_cpy_String(old_key_name));
		}

	}

	else {
		if (!(strcmp(pse_sel->object.name, old_key_name))) {

			/*
			 *  Search for the oldest decryption key stored on the SC.
			 */

			if (DecSKold_update == (UTCTime *)0) {

				/* 
				 *  DecSKold is not installed on the SC 
				 *    => DecSKnew has been changed last 
				 */

				aux_add_error(EINVALID, "No old decryption key on the SC!", CNULL, 0, proc);
				return (CNULL);
			}
		
		
			/*
			 *  Intermediate result:  Both DecSKnew and DecSKold are installed on the SC. 
			 *
			 *           Next to do:  Return the name of the key which is older.
			 */

						
			if ((aux_cmp_UTCTime(DecSKnew_update, DecSKold_update)) == 1) {

				/* 
				 *  DecSKold is older 
				 */ 

				return(aux_cpy_String(old_key_name));
			}
			else  {

				/* 
				 *  DecSKnew is older
				 */ 

				return(aux_cpy_String(new_key_name));
			}
		}
		else {
			aux_add_error(EINVALID, "Wrong name for an decryption key", pse_sel->object.name, char_n, proc);
			return (CNULL);
		}
	}



}				/* get_DecSK_name */







/*--------------------------------------------------------------*/
/*						                */
/* PROC  int_to_fileid					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Transform an integer value into the structure FileId, which */
/*  is used at the SCA-IF(STARMOD version 1.0)		        */
/*  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*  
	Not used
*/
/*  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*								*/
/*  Bit-Structure of a FileId (one byte): 			*/
/*         B'nnnnttll' (n=no., t=file-type, l=file-level)	*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   fileid_int	 	       Integer value of the file_id	*/
/*   file_id		       Pointer to the structure FileId 	*/
/*							       	*/
/* OUT							       	*/
/*   file_id		       File Identifier		 	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   getbits()	                Get n bits of byte x from 	*/
/*			        position p.			*/
/*                                                              */
/*--------------------------------------------------------------*/



static
/***************************************************************
 *
 * Procedure int_to_fileid
 *
 ***************************************************************/
#ifdef __STDC__

void int_to_fileid(
	int	  fileid_int,
	FileId	 *file_id
)

#else

void int_to_fileid(
	fileid_int,
	file_id
)
int	  fileid_int;
FileId	 *file_id;

#endif

{
	char            fileid_char;
	char		*proc = "int_to_fileid";

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	fileid_char = (char) fileid_int;

/*	file_id->file_qualifier = 0x00;
	file_id->file_level  = getbits(fileid_char, 1, 2);
	file_id->file_type   = getbits(fileid_char, 3, 2);
	file_id->short_index = getbits(fileid_char, 7, 4);
*/


}




/*--------------------------------------------------------------*/
/*						                */
/* PROC  int_to_keyid					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Transform an integer value into the structure KeyId, which  */
/*  is used at the SCA-IF.				        */
/*  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*  
	Not used
*/
/*  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*								*/
/*  Bit-Structure of a KeyId (one byte): 			*/
/*         B'nnnnnnll' (n=no., l=file-level)			*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   keyid_int	 	       Integer value of the key_id	*/
/*   key_id		       Pointer to the structure KeyId 	*/
/*							       	*/
/* OUT							       	*/
/*   key_id		       Key Identifier		 	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   getbits()	                Get n bits of byte x from       */
/*			        position p.		        */
/*                                                              */
/*--------------------------------------------------------------*/


static
/***************************************************************
 *
 * Procedure int_to_keyid
 *
 ***************************************************************/
#ifdef __STDC__

void int_to_keyid(
	int	  keyid_int,
	KeyId	 *key_id
)

#else

void int_to_keyid(
	keyid_int,
	key_id
)
int	  keyid_int;
KeyId	 *key_id;

#endif

{
	char            keyid_char;
	char		*proc = "int_to_keyid";

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif

	keyid_char = (char) keyid_int;

/*	key_id->key_level = getbits(keyid_char, 1, 2);
	key_id->key_number = getbits(keyid_char, 7, 6);
*/


}				/* int_to_keyid */






/*--------------------------------------------------------------*/
/*						                */
/* PROC  get_SC_fileid					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Compose structure FileId (used at the SCA-IF) to address a 	*/
/*  WEF on the SC.						*/
/*  The elements of the structure FileId are set as follows:	*/
/*  - file_qualifier = 0x00					*/
/*  - file_level     = DF_LEVEL					*/
/*  - file_type      = WEF					*/
/*  - short_index    = sc_id					*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   sc_id	 	       Short index of the file on the SC*/
/*   file_id		       Pointer to the structure FileId 	*/
/*							       	*/
/* OUT							       	*/
/*   file_id		       File Identifier		 	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*--------------------------------------------------------------*/




/***************************************************************
 *
 * Procedure get_SC_fileid
 *
 ***************************************************************/
#ifdef __STDC__

static void get_SC_fileid(
	unsigned int	 sc_id,
	FileId	 	*file_id
)

#else

static void get_SC_fileid(
	sc_id,
	file_id
)
unsigned int 	sc_id;
FileId	 	*file_id;

#endif

{
	char		*proc = "get_SC_fileid";

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif


	file_id->file_qualifier	 = 0x00;
	file_id->file_level	 = DF_LEVEL;
	file_id->file_type	 = WEF;
	file_id->short_index	 = sc_id;


}




/*--------------------------------------------------------------*/
/*						                */
/* PROC  get_SC_keyid					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Compose structure KeyId (used at the SCA-IF) to address a 	*/
/*  key on the SC.						*/
/*								*/
/*  The elements of the structure KeyId are set as follows:	*/
/*  - key_number    = sc_id					*/
/*  - The value of key_level depends on the input parameter     */
/*    SC_internal.						*/
/*    key_level is set to FILE_CURRENT, if SC_internal == TRUE	*/
/*    key_level is set to FILE_DF,      if SC_internal == FALSE	*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   sc_id	 	       key_number of the key on the SC.	*/
/*   SC_internal	       TRUE  means that the key is used */
/*				     only within the SC. 	*/
/*				     These keys (e.g. PIN, PUK, */
/*				     DES-Keys) are stored in an */
/*				     ISF on the SC and cannot 	*/
/*				     be read out of the SC.	*/
/*   			       FALSE means that the key is 	*/
/*				     stored in an XEF and can be*/
/*				     read by the SCT. In the    */
/*				     current version RSA-keys   */
/*				     have to be transfered into */
/*				     the SCT, because the RSA	*/
/*				     alg. is not implemeneted   */
/*				     on the SC.			*/
/*								*/
/*   key_id		       Pointer to the structure KeyId 	*/
/*							       	*/
/* OUT							       	*/
/*   key_id		       Key Identifier		 	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*                                                              */
/*--------------------------------------------------------------*/



/***************************************************************
 *
 * Procedure get_SC_keyid
 *
 ***************************************************************/
#ifdef __STDC__

static void get_SC_keyid(
	unsigned int	sc_id,
	Boolean  	SC_internal,
	KeyId	 	*key_id
)

#else

static void get_SC_keyid(
	sc_id,
        SC_internal,
	key_id
)
unsigned int	sc_id;
Boolean  	SC_internal;
KeyId	 	*key_id;

#endif

{
	char		*proc = "get_SC_keyid";

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif


	key_id->key_number = sc_id;

	if (SC_internal == TRUE)
		key_id->key_level = FILE_CURRENT;
	else 
		key_id->key_level = FILE_DF;


}				/* get_SC_keyid */





/*--------------------------------------------------------------*/
/*						                */
/* PROC  get_keyid_for_obj				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Provide the keyid for an object:				*/
/*								*/
/*  1. Get parameters for the object from the global variable 	*/
/*     "sc_app_list[].sc_obj_list[]". 				*/
/*     If object is a file					*/
/*	  => return (error).					*/
/*  2. Get key_id for the object.				*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   app_name		       Name of the application		*/
/*   obj_name	 	       Name of the object		*/
/*   SC_internal	       TRUE  means that the key is used */
/*				     only within the SC. 	*/
/*				     These keys (e.g. PIN, PUK, */
/*				     DES-Keys) are stored in an */
/*				     ISF on the SC and cannot 	*/
/*				     be read out of the SC.	*/
/*   			       FALSE means that the key is 	*/
/*				     stored in an XEF and can be*/
/*				     read by the SCT. In the    */
/*				     current version RSA-keys   */
/*				     have to be transfered into */
/*				     the SCT, because the RSA	*/
/*				     alg. is not implemeneted   */
/*				     on the SC.			*/
/*								*/
/*   key_id		       Pointer to the structure KeyId 	*/
/*							       	*/
/* OUT							       	*/
/*   key_id		       Key Identifier		 	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*  -1			       Error			       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   aux_AppObjName2SCObj()	Get information about an SC     */
/*        			object belonging to an 		*/
/*                              application on SC. 		*/
/*   get_SC_keyid()		Compose structure KeyId (for a  */
/*				key on the SC).			*/
/*   aux_add_error()		Add error to error stack.	*/
/*                                                              */
/*--------------------------------------------------------------*/



/***************************************************************
 *
 * Procedure get_keyid_for_obj
 *
 ***************************************************************/
#ifdef __STDC__

static int get_keyid_for_obj(
	char	 *app_name,
	char	 *obj_name,
	Boolean  SC_internal,
	KeyId	 *key_id
)

#else

static int get_keyid_for_obj(
	app_name,
	obj_name,
	SC_internal,
	key_id
)
char	 *app_name;
char	 *obj_name;
Boolean  SC_internal;
KeyId	 *key_id;

#endif

{
	char		*proc = "get_keyid_for_obj";


	/* Variables for internal use */
	SCObjEntry     *sc_obj_entry;


#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif


	/* get information about the object */
	sc_obj_entry = aux_AppObjName2SCObj(app_name, obj_name);
	if (sc_obj_entry == NULLSCOBJENTRY) {
		aux_add_error(EINVALID, "Cannot get configuration info for SC-Object (key)", obj_name, char_n, proc);
		return (-1);
	}
	if (sc_obj_entry->type == SC_FILE_TYPE) {
		/* object is a file */
		aux_add_error(EINVALID, "Type of object is not key!", obj_name, char_n, proc);
		return (-1);
	}
	get_SC_keyid(sc_obj_entry->sc_id, SC_internal, key_id);

	return(0);

}				/* get_keyid_for_obj */




/*--------------------------------------------------------------*/
/*						                */
/* PROC  keyref_to_keyid				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Transform the key reference into the structure KeyId, which */
/*  is used at the SCA-IF.				        */
/*								*/
/*  The key reference (integer) of the SEC-IF can be used to 	*/
/*  address:							*/
/*      - a key stored on the SC 				*/
/*	  - in an ISF (SC_internal == TRUE)			*/
/*	  - in an XEF (SC_internal == FALSE)			*/
/*      - a key stored in the SCT, 				*/
/*	  =>  level of SCT-keys is set to FILE_DUMMY.		*/
/*	   							*/
/*								*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   keyref	 	       Key reference			*/
/*   SC_internal	       TRUE  means that the key is used */
/*				     only within the SC. 	*/
/*				     These keys (e.g. PIN, PUK, */
/*				     DES-Keys) are stored in an */
/*				     ISF on the SC and cannot 	*/
/*				     be read out of the SC.	*/
/*   			       FALSE means that the key is 	*/
/*				     stored in an XEF and can be*/
/*				     read by the SCT. In the    */
/*				     current version RSA-keys   */
/*				     have to be transfered into */
/*				     the SCT, because the RSA	*/
/*				     alg. is not implemeneted   */
/*				     on the SC.			*/
/*								*/
/*   key_id		       Pointer to the structure KeyId 	*/
/*							       	*/
/* OUT							       	*/
/*   key_id		       Key Identifier		 	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*  -1			       Error			       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   get_SC_keyid()		Compose structure KeyId (for a  */
/*				key on the SC).			*/
/*--------------------------------------------------------------*/



/***************************************************************
 *
 * Procedure keyref_to_keyid
 *
 ***************************************************************/
#ifdef __STDC__

static int keyref_to_keyid(
	int	  keyref,
	Boolean   SC_internal,
	KeyId	 *key_id
)

#else

static int keyref_to_keyid(
	keyref,
	SC_internal,
	key_id
)
int	  keyref;
Boolean   SC_internal;
KeyId	 *key_id;

#endif

{
	char           *proc = "keyref_to_keyid";

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif


	if ((keyref & SC_KEY) == SC_KEY) {
		key_id->key_number = keyref & ~SC_KEY;

		get_SC_keyid(key_id->key_number, SC_internal, key_id);

	}
	else {
		if ((keyref & SCT_KEY) == SCT_KEY) {
			key_id->key_level = FILE_DUMMY;
			key_id->key_number = keyref & ~SCT_KEY;
		}
		else {
			aux_add_error(EINVALID, "Invalid level of an SC-key!", (char *) keyref, int_n, proc);
			return (-1);
		}
	}
	return (0);


}				/* keyref_to_keyid */





/*--------------------------------------------------------------*/
/*						                */
/* PROC  key_to_keyid					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Return key_id according to the key selection in key:	*/
/*								*/
/*   Key algorithm:						*/
/*   If key_alg == DES, the key is supposed to be "SC internal".*/
/*   "SC internal" means that the key is stored in an ISF on the*/
/*   SC. Keys stored in an ISF cannot be read out of the SC.	*/
/*								*/
/*   RSA-keys (in an encrypted form) are stored in an XEF on 	*/
/*   the SC. These keys can be read by the SCT. In the current  */
/*   version RSA-keys have to be transfered into the SCT, 	*/
/*   because the RSA algorithm is not implemented on the SC.	*/
/*								*/
/*   The key_level (part of structure KeyId) depends on the     */
/*   information whether the key is stored in an ISF, XEF, resp.*/
/*   								*/
/*								*/
/*   Case 1: Selection with an object name:			*/
/*    If key->keyref == 0 && key->pse_sel != NULL		*/
/*	   a) If "special_DecSK_selection" is set to TRUE, 	*/
/*	      "DecSKnew" and "DecSKold" are selected according  */
/*	      to the update time of the keys on the SC, e.g.	*/
/*	      "DecSKnew" is the key which has been changed last.*/
/*	      "SKnew" and "SKold" are treated in the same way.	*/
/*         b) Get parameters for the object 		 	*/
/*            from the global variable "sc_app_list[].sc_obj_list[]". */
/*	      If object is a file				*/
/*		 => return (error).				*/
/*	   c) Get key_id for the object.			*/
/*								*/
/*   Case 2: Selection with the key reference:			*/
/*    If key->keyref != 0					*/
/*         a) Get key_id from keyref.				*/
/*								*/
/*   Case 3: return(error)					*/
/*								*/
/*  Structure KeyId is used at the SCA-IF to identify a key.	*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   key	 	       Structure which identifies a key	*/
/*   key_alg		       Key algorithm			*/
/*   key_id		       Pointer to the structure KeyId 	*/
/*   special_DecSK_selection   = TRUE 				*/
/*				 If object.name = DecSKnew (SKnew),  */
/*				 the key which has been 	*/
/*				 installed last is selected.	*/
/*				 If object.name = DecSKold (SKold),  */
/*				 the key which has been 	*/
/*				 installed prior is selected.	*/
/*			       = FALSE				*/
/*				 no special handling of the 	*/
/*				 Decryption secret keys		*/
/*								*/	 
/*							       	*/
/* OUT							       	*/
/*   key_id		       Key identifier		 	*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*  -1			       Error			       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*   aux_AppObjName2SCObj()	Get information about an SC     */
/*        			object belonging to an 		*/
/*                              application on SC. 		*/
/*   get_DecSK_name()		Get name of decryption key on SC*/
/*   get_keyid_for_obj()	Get keyid for object.	        */
/*   keyref_to_keyid()	        Transform keyref into structure */
/*                              keyid.				*/
/*                                                              */
/*   aux_add_error()		Add error to error stack.	*/
/*   aux_cpy_String()		Copy string.			*/
/*			         		       		*/
/*--------------------------------------------------------------*/



/***************************************************************
 *
 * Procedure key_to_keyid
 *
 ***************************************************************/
#ifdef __STDC__

static int key_to_keyid(
	Key	 *key,
	AlgId	 *key_alg,
	KeyId	 *key_id,
	Boolean	  special_DecSK_selection
)

#else

static int key_to_keyid(
	key,
	key_alg,
	key_id,
	special_DecSK_selection
)
Key	 *key;
AlgId	 *key_alg;
KeyId	 *key_id;
Boolean	  special_DecSK_selection;

#endif

{

	/* Variables for internal use */
	SCObjEntry     *sc_obj_entry;
	char	       *obj_name = CNULL;
	Boolean		SC_internal;
	AlgEnc		algenc;

	char           *proc = "key_to_keyid";

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif


	if (!key || !key_id) {
		aux_add_error(EINVALID, "Key || key_id missing", CNULL, 0, proc);
		return (-1);
	}

	algenc = aux_ObjId2AlgEnc(key_alg->objid);
	SC_internal = FALSE;
	if ((algenc == DES) || (algenc == DES3))
		SC_internal = TRUE;



	if ((key->keyref == 0) && (key->pse_sel != (PSESel * ) 0)) {

		/*
		 * Select key with object name
		 */

		if ((special_DecSK_selection == TRUE) &&
		    ((!(strcmp(key->pse_sel->object.name, DecSKnew_name))) || 
		     (!(strcmp(key->pse_sel->object.name, SKnew_name))) || 
		     (!(strcmp(key->pse_sel->object.name, DecSKold_name))) || 
		     (!(strcmp(key->pse_sel->object.name, SKold_name))))) {

			/*
			 *  Get the name of the decryption key on the SC
			 */

			if ( (obj_name = get_DecSK_name(key->pse_sel)) == CNULL ) {
				aux_add_error(LASTERROR, "Cannot get name of decryption key on the SC!", CNULL, 0, proc);
				return (-1);
			}
		}
		else {
			if ( (obj_name = aux_cpy_String (key->pse_sel->object.name)) == CNULL) {
				aux_add_error(EMALLOC, "obj_name", CNULL, 0, proc);
				return (-1);
			}
		}



		if (get_keyid_for_obj(key->pse_sel->app_name, obj_name, SC_internal, key_id)) {
			AUX_ADD_ERROR;
			return (-1);
		}

		free(obj_name);
		
	} else {
		if (key->keyref != 0) {

			/*
			 * Select key with keyref
			 */

			if (keyref_to_keyid(key->keyref, SC_internal, key_id)) {
				AUX_ADD_ERROR;
				return (-1);
			};

		} else {
			aux_add_error(EINVALID, "Key missing!", CNULL, 0, proc);
			return (-1);
		}		/* end else */

	}			/* end else */

	return (0);

}				/* key_to_keyid */





/*--------------------------------------------------------------*/
/*						                */
/* PROC  itos						       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Transform an integer value into a character string. The     */
/*  max. length of the char-string is specified by an input 	*/
/*  parameter.							*/
/*  The resulting character string is not NULL terminated.	*/
/*								*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   int_value	 	       Integer value to be transformed. */
/*   char_string 	       Pointer to char_string.		*/
/*   max_len		       Max. length of the char_string to*/
/*			       be returned.			*/
/*							       	*/
/* OUT							       	*/
/*   char_string 	       Character string.		*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*  -1			       Error			       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*                                                              */
/*--------------------------------------------------------------*/



/***************************************************************
 *
 * Procedure itos
 *
 ***************************************************************/
#ifdef __STDC__

static int itos(
	unsigned int	  int_value,
	char		 *char_string,
	unsigned int	  max_len
)

#else

static int itos(
	int_value,
	char_string,
	max_len
)
unsigned int	  int_value;
char		 *char_string;
unsigned int	  max_len;

#endif

{

	unsigned int    i;

	char           *proc = "itos";

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
#endif
	i = sizeof(int);
	if ((max_len <= 0) || (max_len > i)) {
		aux_add_error(EINVALID, "max. length wrong", CNULL, 0, proc);
		return (-1);
	}
	for (i = 0; i < max_len; i++) {

		char_string[max_len - i - 1] = (unsigned) int_value >> i * 8;

	}			/* end for */

#ifdef SECSCTEST
	fprintf(stderr, "char_string\n");
	aux_fxdump(stderr, char_string, max_len, 0);
	fprintf(stderr, "\n");
#endif

	return (0);

}				/* end itos */





/*--------------------------------------------------------------*/
/*						                */
/* PROC  stoi						       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Transform an character string into an integer value. The    */
/*  max. length of the char-string is specified by an input 	*/
/*  parameter.							*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   char_string 	       Pointer to char_string to be	*/
/*			       transformed.			*/
/*   int_value	 	       POinter to integer value.	*/
/*   max_len		       Max. length of the char_string.  */
/*							       	*/
/* OUT							       	*/
/*   int_value	 	       Integer value.			*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*  -1			       Error			       	*/
/*--------------------------------------------------------------*/



/***************************************************************
 *
 * Procedure stoi
 *
 ***************************************************************/
#ifdef __STDC__

static int stoi(
	char		 *char_string,
	unsigned int	 *int_value,
	unsigned int	  max_len
)

#else

static int stoi(
	char_string,
	int_value,
	max_len
)
char		 *char_string;
unsigned int	 *int_value;
unsigned int	  max_len;

#endif

{

	unsigned int    i;

	char           *proc = "stoi";

#ifdef SECSCTEST
	fprintf(stderr, "SECSC-Function: %s\n", proc);
	fprintf(stderr, "char_string to be transformed\n");
	aux_fxdump(stderr, char_string, max_len, 0);
	fprintf(stderr, "\n");
#endif
	i = sizeof(int);
	if ((max_len <= 0) || (max_len > i)) {
		aux_add_error(EINVALID, "max. length wrong", CNULL, 0, proc);
		return (-1);
	}
	if (!(int_value)) {
		aux_add_error(EINVALID, "Invalid input value", CNULL, 0, proc);
		return (-1);
	}
	*int_value = 0;
	for (i = 0; i < max_len; i++) {
		*int_value += (((unsigned int) char_string[i] & 0xFF) << (max_len - i - 1) * 8);
	}			/* end for */
#ifdef SECSCTEST
	fprintf(stderr, "int_value: %d, i: %d\n", *int_value, i);
#endif

	return (0);

}				/* end stoi */



/*--------------------------------------------------------------*/
/*						                */
/* PROC  int2ascii					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Transform an integer value into a NULL terminated ASCII 	*/
/*  character string. 						*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   s		 	       Pointer to char_string.		*/
/*   n		 	       Integer value to be transformed. */
/*							       	*/
/* OUT							       	*/
/*   char_string 	       Character string.		*/
/*							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   0	         	       o.k			       	*/
/*  -1			       Error			       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*                                                              */
/*--------------------------------------------------------------*/

/***************************************************************
 *
 * Procedure int2ascii
 *
 ***************************************************************/
#ifdef __STDC__

static int int2ascii(
	char	  s[],
	int	  n
)

#else

static int int2ascii(
	s,
	n
)
char	  s[];
int	  n;

#endif

{
    int c,i,j, sign;
	char           *proc = "int2ascii";

#ifdef SECSCTEST
/*	fprintf(stderr, "SECSC-Function: %s\n", proc);*/
#endif

	if (!(s)) {
		aux_add_error(EINVALID, "Invalid input value", CNULL, 0, proc);
		return (-1);
	}

	if((sign = n) < 0)
	      n = -n;
	i = 0;
    	do {			 /* generation from right to left */
	 	s[i++] = n % 10 + '0';
    	} while ((n /= 10) > 0);
    	s[i] = '\0';

    	/* reverse(s);*/
    	for (i=0, j=strlen(s)-1; i<j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
    	} /* end for */

#ifdef SECSCTEST
/*	fprintf(stderr, "char_string: %s\n", s);*/
#endif

	return(0);

} /* end int2ascii */



/*--------------------------------------------------------------*/
/*						                */
/* PROC  getbits					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Returns n bits of byte x from position p. The returned bits */
/*  are shifted to right. Position 0 is the right end of x.	*/
/*								*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*   x	 	       	       One byte	(8 bits)		*/
/*   p			       Position				*/
/*   n			       Number of bits			*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   				Return value			*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*--------------------------------------------------------------*/





/***************************************************************
 *
 * Procedure getbits
 *
 ***************************************************************/
#ifdef __STDC__

static int getbits(
	unsigned	  x,
	unsigned	  p,
	unsigned	  n
)

#else

static int getbits(
	x,
	p,
	n
)
unsigned	  x;
unsigned	  p;
unsigned	  n;

#endif

{

	return ((x >> (p + 1 - n)) & ~(~0 << n));

}				/* end getbits */





/*--------------------------------------------------------------*/
/*						                */
/* PROC  bell_function					       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  "Ring the bell" to require user input at the SCT.		*/
/*  If the Workstation is not equipped with an audible bell, the*/
/*  system flashes the window.					*/
/*  ('\07' is written to /dev/tty)				*/
/*								*/
/*							        */
/* IN			     DESCRIPTION		       	*/
/*							       	*/
/* OUT							       	*/
/*							       	*/
/* RETURN		     DESCRIPTION	      	       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*--------------------------------------------------------------*/





/***************************************************************
 *
 * Procedure bell_function
 *
 ***************************************************************/
#ifdef __STDC__

static void bell_function(
)

#else

static void bell_function(
)

#endif

{
	int             fd, rc;

	if ((fd = open("/dev/tty", O_RDWR)) != -1) {
		rc = write(fd, USER_BELL, 1);
	}
	close(fd);

}				/* end bell_function */





/***************************************************************
 *
 * Procedure fprint_KeyId
 *
 ***************************************************************/
#ifdef __STDC__

static void fprint_KeyId(
	KeyId          *key_id
)

#else

static void fprint_KeyId(
	key_id
)
KeyId          *key_id;

#endif

{
	if (!key_id)
		fprintf(stderr, "KeyId                   : NULL\n");
	else {

		fprintf(stderr, "KeyId                   :\n");
		switch (key_id->key_level) {
		case FILE_MF:
			fprintf(stderr, "    key_level           : FILE_MF\n");
			break;
		case FILE_DF:
			fprintf(stderr, "    key_level           : FILE_DF\n");
			break;
		case FILE_CHILD:
			fprintf(stderr, "    key_level           : FILE_CHILD\n");
			break;
		case FILE_MASTER:
			fprintf(stderr, "    key_level           : FILE_MASTER\n");
			break;
		case FILE_CURRENT:
			fprintf(stderr, "    key_level           : FILE_CURRENT\n");
			break;
		case FILE_PARENT:
			fprintf(stderr, "    key_level           : FILE_PARENT\n");
			break;
		default:
			fprintf(stderr, "    key_level           : undefined\n");
			break;
		};

		fprintf(stderr, "    key_number          : %d\n", key_id->key_number);
	}
}




/***************************************************************
 *
 * Procedure fprint_KeyPurpose
 *
 ***************************************************************/
#ifdef __STDC__
static void fprint_KeyPurpose(
	KeyPurpose          key_purpose
)

#else

static void fprint_KeyPurpose(
	key_purpose
)
KeyPurpose          key_purpose;

#endif

{
	if (key_purpose == PURPOSE_SECCONC) 
		fprintf(stderr, "KeyPurpose              : PURPOSE_SECCONC\n");
	else 
	if (key_purpose == PURPOSE_SECAUTH)
		fprintf(stderr, "KeyPurpose              : PURPOSE_SECAUTH\n");
	else 
	if (key_purpose == PURPOSE_AUTH)
		fprintf(stderr, "KeyPurpose              : PURPOSE_AUTH\n");
	else 
		fprintf(stderr, "KeyPurpose              : undefined\n");


}


#endif

