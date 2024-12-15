/*
 *  SecuDE Release 4.3 (GMD)
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

/*-------------secure-cryptographic-fuctions and i/o functions------*/
/*------------------------------------------------------------------*/
/* GMD Darmstadt Institute for System Technic (F2.G3)               */
/* Rheinstr. 75 / Dolivostr. 15                                     */
/* 6100 Darmstadt                                                   */
/* Project ``Secure DFN'' 1990                                      */
/* Grimm/Nausester/Schneider/Viebeg/Vollmer et alii                 */
/*------------------------------------------------------------------*/
/*                                                                  */
/* PACKAGE   secure             VERSION   3.0                       */
/*                                      DATE   27.11.1990           */
/*                                        BY   Tr./Na./Gr./WS       */
/* DESCRIPTION                                                      */
/*   This modul presents the cryptographic functions and            */
/*   the basic i/o functions of the ``sec-interface''.              */
/*   More basic algorithmic functions of RSA, DSA and DES           */
/*   are called (imported) here.                                    */
/*                                                                  */
/* EXPORT                                                           */
/*   sec_chpin          sec_close       sec_create                  */
/*   sec_decrypt        sec_del_key     sec_delete                  */
/*   sec_encrypt        sec_gen_key     sec_get_EncryptedKey        */
/*   sec_get_key        sec_hash        sec_get_keysize             */
/*   sec_open           sec_print_toc   sec_put_EncryptedKey        */
/*   sec_put_key        sec_read        sec_read_toc                */
/*   sec_rename         sec_sign        sec_string_to_key           */
/*   sec_verify         sec_write       sec_write_toc               */
/*   sec_checkSK        sec_read_PSE    sec_unblock_SCpin 	    */
/*   sec_write_PSE   	sec_pin_check	                            */
/*   get_unixname						    */
/*   sec_psetest        sec_pse_config  sec_scttest		    */
/*                                                                  */
/* STATIC                                                           */
/*   chk_parm                                                       */
/*   fsize                                                          */
/*   get_keyinfo_from_key                                           */
/*   get2_keyinfo_from_key                                          */
/*   get_keyinfo_from_keyref                                        */
/*   get2_keyinfo_from_keyref                                       */
/*   object_reencrypt                                               */
/*   pin_check                                                      */
/*   put_keyinfo_according_to_key                                   */
/*   pse_name                                                       */
/*   read_toc                                                       */
/*   write_toc                                                      */
/*                                                                  */
/*   handle_in_SCTSC()						    */
/*                                                                  */
/* IMPORT                                                           */
/*   rsa_encrypt        aux_cpy_KeyInfo                             */
/*   rsa_decrypt        aux_cpy2_KeyInfo                            */
/*   hash_sqmodn        aux_cpy2_AlgId                              */
/*   rsa_sign           aux_cmp_AlgId                               */
/*   rsa_verify         aux_cmp_ObjId                               */
/*   rsa_set_key        aux_free_OctetString                        */
/*   des_ebc_encrypt    aux_free_KeyInfo                            */
/*   des_cbc_encrypt    d2_KeyInfo                                  */
/*   rsa_gen_key        e_KeyInfo                                   */
/*   e_DName            d_DName                                     */
/*   sec_read_pin                                                   */
/*   aux_current_UTCTime                                            */
/*   aux_readable_UTCTime                                           */
/*								    */
/*   SECSC-IF:							    */
/*     secsc_close()     	Close application on the SC.        */
/*     secsc_chpin()     	Change PIN for application on SC.   */
/*     secsc_create()     	Create file (WEF) on the SC.        */
/*     secsc_delete()     	Delete file (WEF) on the SC.        */
/*     secsc_del_key()     	Delete key stored in an SCT.        */
/*     secsc_decrypt()     	Decrypt octetstring within SCT/SC.  */
/*     secsc_encrypt()     	Encrypt octetstring within SCT/SC.  */
/*     secsc_gen_key()     	Generate key and install key on SCT */
/*				or SC.				    */
/*     secsc_get_EncryptedKey() Encrypt key within SCT/SC.	    */
/*     secsc_open()  	        Open application on the SC.         */
/*     secsc_put_EncryptedKey() Decrypt key within SCT/SC.	    */
/*     secsc_read()  	        Read data from file (WEF) on SC     */
/*				into octetstring.		    */
/*     secsc_sc_eject()         Eject SC(s).			    */
/*     secsc_sign()  	        Sign octetstring with key from SC.  */
/*     secsc_verify()  	        Verify signature (with SCT).        */
/*     secsc_unblock_SCpin()    Unblock blocked PIN of the SC-app.  */
/*     secsc_write()  	        Write octetstring into file (WEF)   */
/*				on SC.				    */
/*     aux_AppName2SCApp()	Get information about an SC app.    */
/*     aux_AppObjName2SCObj()	Get information about an SC object  */
/*     check_SCapp_configuration()				    */
/*				Check consistency of configuration  */
/*				data for an SC-application.	    */
/*     get_pse_pin_from_SC()	Read the PIN for the SW-PSE from    */
/*				the SC and sets it in 		    */
/*			        "sct_status_list[]".		    */
/*     handle_SC_app()	        If application not open, open it.   */
/*     SC_configuration()	Perform SC configuration (get data  */
/*				form file ".scinit".		    */
/*     SCT_configuration()	Perform SCT configuration (get data */
/*   				from a prior process).              */
/*								    */
/*------------------------------------------------------------------*/


#define DIRMASK 0700		/* directory creation mask */
#define OBJMASK 0600		/* object creation mask */

#include "secure.h"

#ifndef MAC
#include <sys/types.h>
#include <sys/stat.h>
#else
#include <stdlib.h>
#include <string.h>
#include "MacTypes.h"
#include "Mac.h"
#endif /* MAC */

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

/*
 *    Local variables, but global within secure.c
 */


#ifdef __STDC__

static int	update_toc	(PSESel *pse_sel, struct PSE_Objects *objp, char *objname);
static int	chk_parm	(PSESel *pse_sel);
static off_t	fsize	(int fd);
static OctetString	*get_encodedkeyinfo_from_keyref	(KeyRef keyref);
static int	get2_keyinfo_from_keyref	(KeyInfo *keyinfo, KeyRef keyref);
static int	object_reencrypt	(PSESel *pse_sel, char *newpin, Boolean psepin);
static RC	open_object	(PSESel *pse_sel, int flag);
static char	*pin_check	(PSESel *pse_sel, char *obj, char *pin, int err_ind, Boolean interactive);
static RC	read_object	(PSESel *pse_sel, int fd, OctetString *content);
static PSEToc  *read_toc(PSESel	 *pse_sel);
static PSESel	*set_key_pool	(KeyRef keyref);
static char	*key_pool_pw	();
static void	strzero	(register char *string);
static char	*pse_name	(char *app, char *object);
static int	write_toc	(PSESel *pse_sel, PSEToc *toc, int flags);
static struct PSE_Objects	**locate_toc	(char *objname);



#else
static int	update_toc	();
static int	chk_parm	();
static off_t	fsize	();
static OctetString	*get_encodedkeyinfo_from_keyref	();
static int	get2_keyinfo_from_keyref	();
static int	object_reencrypt	();
static RC	open_object	();
static char	*pin_check	();
static RC	read_object	();
static PSEToc  *read_toc();
static PSESel	*set_key_pool	();
static char	*key_pool_pw	();
static void	strzero	();
static char	*pse_name	();
static int	write_toc	();
static struct   PSE_Objects	**locate_toc	();
#endif


extern AlgId    desCBC_pad_aid;	/* from sec_init.c */
extern ObjId   *RSA_SK_OID, *DSA_SK_OID, *DES_OID, *IDEA_OID, *DES3_OID, *Uid_OID;


static          pse_pw;
static KeyInfo  got_key;
static PSEToc  *psetoc;
static char     text[128];
static AlgId   *sec_io_algid = &desCBC_pad_aid;	/* Default Algid for the
						 * encryption of the PSE */
static ObjId    dummy_oid;

static PSESel   sec_key_pool = {
	".key_pool", CNULL, { CNULL, CNULL }, 0
};




#ifdef SCA
static PSEToc  *sc_toc;

PSELocation     pse_location;
int             call_secsc = FALSE;

#endif



typedef enum {
	F_null, F_encrypt, F_decrypt,
	F_hash, F_sign, F_verify
} FTYPE;

static FTYPE    sec_state = F_null;

/***************************************************************************************
 *                                     sec_chpin                                       *
 ***************************************************************************************/

/***************************************************************
 *
 * Procedure sec_chpin
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_chpin(
	PSESel	 *pse_sel,
	char	 *newpin
)

#else

RC sec_chpin(
	pse_sel,
	newpin
)
PSESel	 *pse_sel;
char	 *newpin;

#endif

{
	struct PSE_Objects *nxt;
	char           *object, pwcontent[32];
	int             fd, ret, n, old_pin = TRUE;

	char           *proc = "sec_chpin";

	if (chk_parm(pse_sel)) {
		AUX_ADD_ERROR;
		return (-1);
	}
#ifdef SCA
/************************  S C  -  P A R T  *******************************************/

	/*
	 * Check whether SC available and application = SC-application.
	 */

	if ((pse_location = sec_psetest(pse_sel->app_name)) == ERR_in_psetest) {
		AUX_ADD_ERROR;
		return (-1);
	}
	if (pse_location == SCpse) {

		/*
		 * SC available and application = SC application.
		 */

		/*
		 * secsc_chpin(): Change PIN for an object:  => an error will
		 * be returned
		 * 
		 * Change PIN for application: => Only the PIN for the
		 * application on the SC is changed. The PIN for the SW-PSE
		 * is not changed. => Parameter "newpin" is not evaluated.
		 */

#ifdef SECSCTEST
		fprintf(stderr, "Call of secsc_chpin\n");
#endif
		if (secsc_chpin(pse_sel)) {
			AUX_ADD_ERROR;
			return (-1);
		}
		return (0);	/* PIN on the SC has been changed */

	}			/* if (SC available && app = SC-app) */
	/**************************************************************************************/

	/*
	 * The following is only performed, if the SC is not available or the
	 * application is not an SC-application.
	 */
#endif				/* SCA */


	if (!newpin) newpin = "";

	if (pse_sel->object.name && strlen(pse_sel->object.name)) {

		/*
		 * Change object PIN
		 */

		if (object_reencrypt(pse_sel, newpin, FALSE) < 0) {
			AUX_ADD_ERROR;
			return (-1);
		}
	} else {

		/*
		 * Change PSE PIN
		 */

		/* Read toc */

		if (!(psetoc = chk_toc(pse_sel, FALSE))) {
			AUX_ADD_ERROR;
			return (-1);
		}
		if (pse_sel->pin && strlen(pse_sel->pin)) old_pin = TRUE;
		else old_pin = FALSE;

		nxt = psetoc->obj;
		while (nxt) {
			if (!(object = pse_name(pse_sel->app_name, nxt->name))) {
				AUX_ADD_ERROR;
				return (-1);
			}

			/*
			 * reencrypt all .sf objects which have no .pw, if
			 * old_pin == TRUE, or all objects without .sf
			 * suffix, if old_pin == FALSE
			 */

			strcat(object, ".sf");

			if ((fd = open(object, O_RDONLY)) > 0) {
				close(fd);
				object[strlen(object) - 3] = '\0';
				strcat(object, ".pw");
				if ((fd = open(object, O_RDONLY)) < 0) {
					pse_sel->object.name = nxt->name;
					ret = object_reencrypt(pse_sel, newpin, TRUE);
					if (ret < 0) {
						if (object)
							free(object);
						AUX_ADD_ERROR;
						return (-1);
					}
				} else {

					/*
					 * here we have an object with .pw,
					 * i.e. it has a pin which is
					 * different from the old PSE pin. If
					 * its pin is equal to the new pin,
					 * remove .pw
					 */

					if ((n = read_dec(fd, pwcontent, sizeof(pwcontent), newpin)) > 0) {
						pwcontent[n] = '\0';
						if (!strcmp(pwcontent, newpin)) {
							/* yes, remove .pw */
							close_dec(fd);
							unlink(object);
						}
					}
					close_dec(fd);
				}
				close(fd);
			} else if (old_pin == FALSE) {
				object[strlen(object) - 3] = '\0';
				if ((fd = open(object, O_RDONLY)) > 0) {
					close(fd);
					pse_sel->object.name = nxt->name;
					ret = object_reencrypt(pse_sel, newpin, TRUE);
					if (ret < 0) {
						if (object) free(object);
						AUX_ADD_ERROR;
						return (-1);
					}
				}
			}
			if (object) free(object);
			nxt = nxt->next;
		}

		/* Create pse.pw with encrypted newpin */

		if (pse_sel->pin && strlen(pse_sel->pin)) {
			if (!(object = pse_name(pse_sel->app_name, "pse.pw"))) {
				AUX_ADD_ERROR;
				return (-1);
			}
			unlink(object);
			if (object)	free(object);
		}
		if (strlen(newpin)) {
			if (!(object = pse_name(pse_sel->app_name, "pse.pw"))) {
				AUX_ADD_ERROR;
				return (-1);
			}
#ifndef MAC
			if ((fd = open(object, O_WRONLY | O_CREAT | O_EXCL, OBJMASK)) < 0) 
#else
			if ((fd = open(object, O_WRONLY | O_CREAT | O_EXCL)) < 0) 
#endif /* MAC */
			{
				aux_add_error(ESYSTEM, "can't create object", (char *) object, char_n, proc);
				if (object) free(object);
				return (-1);
			}
			chmod(object, OBJMASK);
			strcpy(text, newpin);	/* save pin because write_enc
						 * encrypts inline */
			if (write_enc(fd, text, strlen(newpin), newpin) < 0) {
				if(LASTERROR != ESYSTEM) AUX_ADD_ERROR;
				else aux_add_error(ESYSTEM, "can't write object", (char *) object, char_n, proc);

				if (object) free(object);
				close_enc(fd);
				return (-1);
			}
			if (object) free(object);
			close_enc(fd);
		}
		/* delete old Toc/Toc.sf */

		if (!(object = pse_name(pse_sel->app_name, "Toc"))) {
			AUX_ADD_ERROR;
			return (-1);
		}
		unlink(object);
		strcat(object, ".sf");
		unlink(object);
		if (object) free(object);

		/* Create Toc.sf with (encrypted) toc */

		if(pse_sel->pin) strzfree(&(pse_sel->pin));
		pse_sel->pin = aux_cpy_String(newpin);
		if (write_toc(pse_sel, psetoc, O_WRONLY | O_CREAT | O_EXCL) < 0) {
			strzfree(&(pse_sel->pin));
			AUX_ADD_ERROR;
			return (-1);
		}
	}
	return (0);
}


/***************************************************************************************
 *                                     sec_close                                       *
 ***************************************************************************************/

/***************************************************************
 *
 * Procedure sec_close
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_close(
	PSESel	 *pse_sel
)

#else

RC sec_close(
	pse_sel
)
PSESel	 *pse_sel;

#endif

{
	char           *proc = "sec_close";

	if (chk_parm(pse_sel)) {
		AUX_ADD_ERROR;
		return (-1);
	}
#ifdef SCA
/************************  S C  -  P A R T  *******************************************/

	/*
	 * Check whether SC available and application = SC-application.
	 */

	if ((pse_location = sec_psetest(pse_sel->app_name)) == ERR_in_psetest) {
		AUX_ADD_ERROR;
		return (-1);
	}
	if (pse_location == SCpse) {

		/*
		 * SC available and application = SC application.
		 */

#ifdef SECSCTEST
		fprintf(stderr, "Call of secsc_close \n");
#endif


		if ((!pse_sel->object.name) || (!strlen(pse_sel->object.name)) ||
		  ((pse_sel->object.name && strlen(pse_sel->object.name)) &&
		   (aux_AppObjName2SCObj(pse_sel->app_name, pse_sel->object.name)))) {

			if (secsc_close(pse_sel)) {
				AUX_ADD_ERROR;
				return (-1);
			}
			if (pse_sel->object.name && strlen(pse_sel->object.name)) {
				return (0);	/* object on SC has been
						 * closed */
			}
		}
	}			/* end if (SC available && app = SC-app) */
	/**************************************************************************************/

	/*
	 * If an application shall be closed, the following is performed in
	 * any case. If an object shall be closed, the following is only
	 * performed, if - the SC is not available or - the application is
	 * not an SC-application or - the object to be closed is not an
	 * SC-object.
	 */
#endif				/* SCA */


	if (!pse_sel->object.name) {
		aux_free_PSEToc(&psetoc);
#ifdef SCA
		aux_free_PSEToc(&sc_toc);
#endif
		strzfree(&(pse_sel->pin));
	}
	else strzfree(&(pse_sel->object.pin));

	return (0);
}


/***************************************************************************************
 *                                     sec_create                                      *
 ***************************************************************************************/

/***************************************************************
 *
 * Procedure sec_create
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_create(
	PSESel	 *pse_sel
)

#else

RC sec_create(
	pse_sel
)
PSESel	 *pse_sel;

#endif

{
	char           *dirname, *object, buf[32], *zwpin;
	int             fd, ret, toc_flag;
	struct PSE_Objects *nxt, *pre, *new;
	int             free_owner, maxref = -1, ref;
	PSEToc         *sctoc = (PSEToc *) 0;


	char           *proc = "sec_create";

	if (chk_parm(pse_sel)) {
		AUX_ADD_ERROR;
		return (-1);
	}
#ifdef SCA
/************************  S C  -  P A R T  *******************************************/

	/*
	 * Check whether SC available and application = SC-application.
	 */

	if ((pse_location = sec_psetest(pse_sel->app_name)) == ERR_in_psetest) {
		AUX_ADD_ERROR;
		return (-1);
	}
	if (pse_location == SCpse) {

		/*
		 * SC available and application = SC application.
		 */

		/* If SC application not open => open it */
		if (handle_SC_app(pse_sel->app_name)) {
			AUX_ADD_ERROR;
			return (-1);
		}

		if(pse_sel->object.name && strlen(pse_sel->object.name)) {

			/*
			 * If object = SC object    => create object on SC
			 */

			if (aux_AppObjName2SCObj(pse_sel->app_name, pse_sel->object.name)) {
				/* create object on SC */
#ifdef SECSCTEST
				fprintf(stderr, "Call of secsc_create (obj)\n");
#endif
				if (secsc_create(pse_sel)) {
					AUX_ADD_ERROR;
					return (-1);
				}
				/* update or create SC toc */
				if (update_SCToc(pse_sel, 0, 0)) {	
					AUX_ADD_ERROR;
					return (-1);
				}

				return (0);	/* object on SC has been
						 * created */
			} 


			/* 
			 *
			 *  An object on the SW-PSE shall be created!
			 *
			 *  => Get the PIN for the SW-PSE from the SC.
			 */
		
			strrep(&(pse_sel->pin), get_pse_pin_from_SC(pse_sel->app_name));
			if(!pse_sel->pin) {
				aux_add_error(EPSEPIN, "Can't get PIN for SW-PSE extension from SC", CNULL, 0, proc);
				return (-1);
			}
			if(pse_sel->object.name && strlen(pse_sel->object.name)) {
				strrep(&(pse_sel->object.pin), pse_sel->pin);
			}

		}
		else {

			/*
			 *  Create application on SC
			 */

			/*
			 *  If application on SC is virgin (no toc on SC)
			 *     => check consistency of configuration data of SC application
			 *     => install toc on SC
			 *  If application on SC is not virgin (toc exists on SC)
			 *     => return (-1)
			 */
		
			sctoc = read_SCToc(pse_sel);
			if (!sctoc) {

				/*
				 *  No toc => application on SC is virgin
				 */

				/*
				 *  Check consistency of the configuration data for the SC-application
				 */

				if (check_SCapp_configuration(pse_sel->app_name, sec_onekeypair)) {
					AUX_ADD_ERROR;
					return (-1);
				}

				/*
				 *  Install Toc on SC
				 */

				sctoc = create_SCToc(pse_sel);
				if (!sc_toc) {
					AUX_ADD_ERROR;
					return (-1);
				}
				else {
					/*
					 *  If onekeypair, set status byte in Toc 
					 */

					sctoc->status = 0;
					if (sec_onekeypair == TRUE)

						sctoc->status |= ONEKEYPAIRONLY;
					if (write_SCToc(pse_sel, sctoc) < 0) {
						AUX_ADD_ERROR;
						return (-1);
					}
				}
				aux_free_error();
			}				
			else {

				/*
				 *  Toc on SC exists => application on SC is not virgin
				 */

				aux_add_error(ECREATEAPP, "Application on SC exists already", (char *) pse_sel->app_name, char_n, proc);
				return(-1);
			}

			return(0);

		}
	}			/* if (SC available && app = SC-app) */


	
/**************************************************************************************/

	/*
	 * The following is only performed, 
	 * if an application | object on the SW-PSE shall be created.
	 * 
	 * --------------------------------------------------------------
	 * If the SC is available and an application on the SC could be 
	 * opened, the PIN read from the SC is used as PIN for the SW-PSE 
	 * application/object.
	 */
#endif				/* SCA */


	if (pse_sel->object.name && strlen(pse_sel->object.name)) {

		/*
		 * Check whether a keypool object with the highest existing
		 * number + 1 shall be created
		 */

		if (pse_sel == &sec_key_pool && strcmp(pse_sel->object.name, "-1") == 0)
			maxref = 0;

		/*
		 * Create an object on an existing PSE
		 */

		/* Read toc */

		if (!(psetoc = chk_toc(pse_sel, FALSE))) {
#ifdef SCA

			if ((pse_location = sec_psetest(pse_sel->app_name)) == ERR_in_psetest) {
				AUX_ADD_ERROR;
				return (-1);
			}
			if (pse_location == SCpse) {
				aux_free_error();
				/* 
				 *
				 *  An object on the SW-PSE shall be created!
				 *
				 *  => Get the PIN for the SW-PSE from the SC.
				 */
		
				strrep(&(pse_sel->pin), get_pse_pin_from_SC(pse_sel->app_name));
				if(!pse_sel->pin) {
					aux_add_error(EPSEPIN, "Can't get PIN for SW-PSE extension from SC", CNULL, 0, proc);
					return (-1);
				}
				if(pse_sel->object.name && strlen(pse_sel->object.name)) {
					strrep(&(pse_sel->object.pin), pse_sel->pin);
				}

				if(create_PSE(pse_sel) < 0) {
					AUX_ADD_ERROR;
					return (-1);
				}

				toc_flag = O_WRONLY | O_CREAT | O_EXCL;
			}
			else {
#endif
				AUX_ADD_ERROR;
				return (-1);
#ifdef SCA
			}
#endif
		}
		else toc_flag = O_WRONLY;

		/* allocate memory for new element */

		new = (struct PSE_Objects *) calloc(1, sizeof(struct PSE_Objects));
		if (!new) {
			aux_add_error(EMALLOC, "new", CNULL, 0, proc);
			return (-1);
		}
		/* Check whether pse_sel->object.name already exists */

		if (!(nxt = psetoc->obj)) psetoc->obj = new;
		else {
			while (nxt) {
				if (maxref >= 0) {	/* create new keypoool
							 * object as maxref + 1 */
					strcpy(buf, nxt->name);
					buf[strlen(buf) - 3] = '\0';	/* cut suffix .sf */
					sscanf(buf, "%X", &ref);
					if (ref > maxref) maxref = ref;
				} else {
					if (strcmp(nxt->name, pse_sel->object.name) == 0) {
						/* yes */
						free(new);
						aux_add_error(ECREATEOBJ, "Object exists already: can't create", (char *) pse_sel->object.name, char_n, proc);
						return (-1);
					}
				}
				pre = nxt;
				nxt = nxt->next;
			}
			pre->next = new;
		}

		if (maxref >= 0) {
			maxref++;
			pse_sel = set_key_pool(maxref);
		}
		/* append new object */

		nxt = new;
		if (!(nxt->name = aux_cpy_String(pse_sel->object.name))) {
			aux_add_error(EMALLOC, "next->name", CNULL, 0, proc);
			return (-1);
		}
		nxt->create = aux_current_UTCTime();
		nxt->update = aux_current_UTCTime();
		nxt->noOctets = 0;
		nxt->status = 0;
		nxt->next = (struct PSE_Objects *) 0;


		/* Ask for object PIN, if not present */

		if(pse_sel->pin && !pse_sel->object.pin) pse_sel->object.pin = aux_cpy_String(pse_sel->pin);


		if (!pse_sel->object.pin) {
			pse_sel->object.pin = sec_read_pin("PIN for", pse_sel->object.name, TRUE);
			if (!pse_sel->object.pin) {
				AUX_ADD_ERROR;
				return (-1);
			}
		}


		/*
		 * Create object.pw with encrypted PIN if PIN exists and is
		 * different from PSE pin
		 */

		if (!(object = pse_name(pse_sel->app_name, pse_sel->object.name))) {
			AUX_ADD_ERROR;
			return (-1);
		}
		if (strlen(pse_sel->object.pin)) {
			if (!pin_check(pse_sel, "pse", pse_sel->object.pin, FALSE, TRUE)) {
				strcat(object, ".pw");
#ifndef MAC
				if ((fd = open(object, O_WRONLY | O_CREAT | O_EXCL, OBJMASK)) < 0) 
#else
				if ((fd = open(object, O_WRONLY | O_CREAT | O_EXCL)) < 0) 
#endif /* MAC */
				{
					aux_add_error(ESYSTEM, "can't create object", (char *) object, char_n, proc);
					if (object) free(object);
					return (-1);
				}
				chmod(object, OBJMASK);
				strcpy(text, pse_sel->object.pin);	/* save pin because
									 * write_enc encrypts
									 * inline */
				if (write_enc(fd, text, strlen(pse_sel->object.pin), pse_sel->object.pin) < 0) {
					if(LASTERROR != ESYSTEM) AUX_ADD_ERROR;
					else aux_add_error(ESYSTEM, "can't write object", (char *) object, char_n, proc);
					if (object) free(object);
					close_enc(fd);
					return (-1);
				}
				close_enc(fd);
				object[strlen(object) - 3] = '\0';
			}
			strcat(object, ".sf");
		}
		/* Create object */

#ifndef MAC
		if ((fd = open(object, O_WRONLY | O_CREAT | O_EXCL, OBJMASK)) < 0) 
#else
		if ((fd = open(object, O_WRONLY | O_CREAT | O_EXCL)) < 0) 
#endif /* MAC */
		{
			aux_add_error(ESYSTEM, "can't create object", (char *) object, char_n, proc);
			if (object) free(object);
			return (-1);
		}
		close(fd);
		chmod(object, OBJMASK);
		if (object) free(object);
	} 
	else {
		if(create_PSE(pse_sel) < 0) {
			AUX_ADD_ERROR;
			return(-1);
		}
		toc_flag = O_WRONLY | O_CREAT | O_EXCL;
	}


	/* Write toc */

	if (pse_sel == &sec_key_pool && pse_sel->object.name) {
		/* sec_write follows, write toc there */
		if (maxref > 0)	return (maxref);
		else return (0);
	} 
	else {
		if (!pse_sel->object.name || !strlen(pse_sel->object.name)) {

			/*
			 *  If pse has been created, set status byte in Toc 
			 */

			psetoc->status = 0;
			if (sec_onekeypair == TRUE)
				psetoc->status |= ONEKEYPAIRONLY;
		}

		ret = write_toc(pse_sel, psetoc, toc_flag);
		if (ret) AUX_ADD_ERROR;
		return (ret);
	}
}


/***************************************************************************************
 *                                     create_PSE                                      *
 ***************************************************************************************/

/***************************************************************
 *
 * Procedure create_PSE
 *
 ***************************************************************/
#ifdef __STDC__

RC create_PSE(
	PSESel	 *pse_sel
)

#else

RC create_PSE(
	pse_sel
)
PSESel	 *pse_sel;

#endif

{
	char           *dirname, *object, buf[32], *zwpin, *dd;
	int             fd, ret, toc_flag;
	struct PSE_Objects *nxt, *pre, *new;
	int             free_owner, maxref = -1, ref;

	char           *proc = "create_PSE";

		/*
		 * Create a PSE
		 */

		/* check conflict with key_pool */

		if (!strcmp(pse_sel->app_name, ".key_pool")) {
			if (!(pse_sel->pin) || strcmp(pse_sel->pin, key_pool_pw())) {
				aux_add_error(EINVALID, "name .key_pool not allowed", CNULL, 0, proc);
				return (-1);
			}
		}
		/* Build directory name of the PSE */

		if (!(dirname = pse_name(pse_sel->app_name, CNULL))) {
			AUX_ADD_ERROR;
			return (-1);
		}
		/* Make directory of the PSE */

		if (mkdir(dirname, DIRMASK) < 0) {
			if (errno == EEXIST) {
				aux_add_error(ECREATEAPP, "PSE exists already", (char *) dirname, char_n, proc);
			}
			else {
				aux_add_error(ESYSTEM, "mkdir failed for", (char *) dirname, char_n, proc);
			}
			if (dirname) free(dirname);
			return (-1);
		}
		chmod(dirname, DIRMASK);

		/* Ask for PIN, if not present */

		if (!pse_sel->pin) {
			pse_sel->pin = sec_read_pin("PIN for", pse_sel->app_name, TRUE);
			if (!pse_sel->pin) {
				AUX_ADD_ERROR;
				rmdir(dirname);
				if (dirname) free(dirname);
				return (-1);
			}
		}
		/* Create pse.pw with encrypted PIN if PIN exists */

		if (strlen(pse_sel->pin)) {
			if (!(object = pse_name(pse_sel->app_name, "pse.pw"))) {
				AUX_ADD_ERROR;
				return (-1);
			}
#ifndef MAC
			if ((fd = open(object, O_WRONLY | O_CREAT | O_EXCL, OBJMASK)) < 0) 
#else
			if ((fd = open(object, O_WRONLY | O_CREAT | O_EXCL)) < 0) 
#endif /* MAC */
			{
				aux_add_error(ESYSTEM, "can't create object", (char *) object, char_n, proc);
				if (object) free(object);
				rmdir(dirname);
				if (dirname) free(dirname);
				return (-1);
			}
			chmod(object, OBJMASK);
			strcpy(text, pse_sel->pin);	/* save pin because
							 * write_enc encrypts
							 * inline */
			if (write_enc(fd, text, strlen(pse_sel->pin), pse_sel->pin) < 0) {
				if(LASTERROR != ESYSTEM) AUX_ADD_ERROR;
				else aux_add_error(ESYSTEM, "can't write object", (char *) object, char_n, proc);
				close_enc(fd);
				unlink(object);
				rmdir(dirname);
				if (object) free(object);
				if (dirname) free(dirname);
				return (-1);
			}
			close_enc(fd);
		}
		/* Build initial toc */

		if (!(psetoc = chk_toc(pse_sel, TRUE))) {
			AUX_ADD_ERROR;
			unlink(object);
			rmdir(dirname);
			if (object) free(object);
			if (dirname) free(dirname);
			return (-1);
		}
		if (!(psetoc->owner = (char *) malloc(128))) {
			aux_add_error(EMALLOC, "psetoc->owner", CNULL, 0, proc);
			unlink(object);
			rmdir(dirname);
			if (object) free(object);
			if (dirname) free(dirname);
			return (-1);
		}
		strcpy(psetoc->owner, get_unixname());
		psetoc->create = aux_current_UTCTime();
		psetoc->update = aux_current_UTCTime();
		psetoc->obj = (struct PSE_Objects *) 0;


		if (object) free(object);
		if (dirname) free(dirname);

		return(0);
}

/***************************************************************
 *
 * Procedure sec_decrypt_PKCS
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_decrypt_PKCS(
	BitString	 *in_bits,
	OctetString	 *out_octets,
	More		  more,
	Key		 *key,
	Boolean		  sc
)

#else

RC sec_decrypt_PKCS(
	in_bits,
	out_octets,
	more,
	key,
	sc
)
BitString	 *in_bits;
OctetString	 *out_octets;
More		  more;
Key		 *key;
Boolean		  sc;

#endif

{
	char           		*proc = "sec_decrypt_PKCS";
	static BitString      	input;
	static int		keysize = 0, sum = 0;
	int			n, data_size, PSlength, i;
	BitString		data_block;
	OctetString		rsa_output;
	char			*bb, *dd;
	More			more_blocks;

	if(!keysize) {

/* initialisation if first call */
		keysize = RSA_PARM(rsaEncryption->param);
		if (!(rsa_output.octets = malloc(keysize/8))) {
			aux_add_error(EMALLOC, "out_octets.octets", CNULL, 0, proc);
			return (-1);
		}
		input.bits = 0;
		input.bits = CNULL;
	}

/* add input stream to the rest of the last call */

	if (aux_append_BitString(&input, in_bits) < 0) {
		AUX_ADD_ERROR;
		in_bits->nbits *= 8; 
		return (-1);
	}






	

	data_block.bits = input.bits;

	while(input.nbits > 0 && (input.nbits >= keysize || more == SEC_END)) {
/* encrypt each block */

		if(input.nbits > keysize) {
			data_block.nbits = keysize;
			more_blocks = SEC_MORE;
		}
		else {
			data_block.nbits = input.nbits;
			more_blocks = SEC_END;
		}


		rsa_output.noctets = 0;



/* do RSA on SmartCard or with software */

#ifdef SCA
		if(sc) {

#ifdef SECSCTEST
			fprintf(stderr, "Call of secsc_decrypt\n");
#endif
			if ((n = secsc_decrypt(&data_block, &rsa_output, more_blocks, key)) == -1) {
				AUX_ADD_ERROR;
				return (-1);
			}

		}
#endif
		if(!sc) {

#ifdef SECSCTEST
			fprintf(stderr, "input for rsa_decrypt\n");
               	 	aux_xdump(data_block.bits, data_block.nbits / 8, 0);
	        	fprintf(stderr, "\n");
#endif

			n = rsa_decrypt(&data_block, &rsa_output, more_blocks, keysize);

		}
		/* 
		 * Restore D from out_octets which should be 
		 * 0x02 || PS || 0x00 || D, PS should be all non-zero.  
		 */

		PSlength = strlen(rsa_output.octets + 1);
		if(rsa_output.octets[0] != 2 || PSlength > rsa_output.noctets - 2) {
			aux_add_error(EDECRYPTION, "decrypted block wrong (PKCS#1 BT 02)", CNULL, 0, proc);
			return(-1);
		}


		bcopy(rsa_output.octets + 2 + PSlength, out_octets->octets + out_octets->noctets, rsa_output.noctets - 2 - PSlength);
		out_octets->noctets += rsa_output.noctets - 2 - PSlength;

/* set next data block */
		input.nbits -= data_block.nbits;
		data_block.bits += data_block.nbits/8;
	}










/* delete the processed data blocks */
	if(input.nbits) {
		bcopy(data_block.bits, input.bits, input.nbits/8);
		input.bits = realloc(input.bits, input.nbits/8);
	}
	else if(input.bits) {
		free(input.bits);
		input.bits = CNULL;
	}


	sum += n;
	if(more == SEC_END) {
		keysize = 0;
		free(rsa_output.octets);
		return(sum);
	}
	return(n);
}
/***************************************************************
 *
 * Procedure sec_decrypt
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_decrypt(
	BitString	 *in_bits,
	OctetString	 *out_octets,
	More		  more,
	Key		 *key
)

#else

RC sec_decrypt(
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
	int             n, i, rc, PSlength, no_dec;
	char           *dd, *bb;
	static AlgEnc   algenc;
	static AlgSpecial     algspecial;


	char           *proc = "sec_decrypt";

	public_modulus_length = 0;

#ifdef SCA
/************************  S C  -  P A R T  *******************************************/


	/*
	 * Check whether decryption shall be done within the SCT/SC.
	 * If the key is selected with object name, the PIN for the
	 * SW-PSE is read from the SC.
	 */

	if ((call_secsc = handle_in_SCTSC(key, FALSE)) == -1) {
		AUX_ADD_ERROR;
		return (-1);
	}
	if (call_secsc == TRUE) {

		/* decrypt within SCT/SC */
#ifdef SECSCTEST
		fprintf(stderr, "Call of secsc_decrypt\n");
#endif
		if ((no_dec = secsc_decrypt(in_bits, out_octets, more, key)) == -1) {
			AUX_ADD_ERROR;
			return (-1);
		}
		return (no_dec);	/* Decryption was successful! */

	}		/* if (call_secsc == TRUE) */
	/**************************************************************************************/

	/*
	 * The following is only performed, if - the SC is not available or -
	 * the decryption key doesn't address a key within SCT/SC
	 * 
	 * 
	 * --------------------------------------------------------------
	 * If the SC is available and an application on the SC could be 
	 * opened, the PIN read from the SC is used as PIN for the SW-PSE 
	 * application/object.
	 */
#endif				/* SCA */



	if (sec_state == F_null) {
		if (get2_keyinfo_from_key(&got_key, key) < 0) {
			AUX_ADD_ERROR;
			return -1;
		}
		algenc = aux_ObjId2AlgEnc(got_key.subjectAI->objid);
		algspecial = aux_ObjId2AlgSpecial(got_key.subjectAI->objid);
		if (algenc == RSA) {
			public_modulus_length = RSA_PARM(got_key.subjectAI->param);
			rc = rsa_set_key((char *)&got_key.subjectkey, 1);
			if (rc != 0) {
				AUX_ADD_ERROR;
				return -1;
			}
		}
		if(key->alg) {
			if(aux_ObjId2AlgEnc(key->alg->objid) != algenc) {
				aux_add_error(EINVALID, "Invalid algorithm for decryption", (char *) key->alg, AlgId_n, proc);
				return -1;
			}
			algspecial = aux_ObjId2AlgSpecial(key->alg->objid);
		}
		if(sec_time) {
			if(algenc == RSA) rsa_sec = rsa_usec = 0;
			if(algenc == IDEA) idea_sec = idea_usec = 0;
			else des_sec = des_usec = 0;
		}
		sec_state = F_decrypt;
	} else if (sec_state != F_decrypt) {
		aux_add_error(EDECRYPT, "wrong sec_state", CNULL, 0, proc);
		return -1;
	}
	if(sec_verbose) {
		fprintf(stderr, "Input to sec_decrypt:\n");
		aux_fprint_BitString(stderr, in_bits);
	}
	switch (algenc) {
	case RSA:

		if(sec_time) gettimeofday(&sec_tp1, &sec_tzp1);

		if(algspecial == PKCS_BT_02) {

			/*  Here goes PKCS#1 ...   */

			if ((no_dec = sec_decrypt_PKCS(in_bits, out_octets, more, key, FALSE)) == -1) { /* FALSE : decrypt with software */
				AUX_ADD_ERROR;
				return (-1);
			}
		} 
		else {


#ifdef SECSCTEST
			fprintf(stderr, "input for rsa_decrypt\n");
                	aux_xdump(in_bits->bits, in_bits->nbits / 8, 0);
	        	fprintf(stderr, "\n");
#endif

			n = rsa_decrypt(in_bits, out_octets, more, RSA_PARM(got_key.subjectAI->param));

#ifdef SECSCTEST
			fprintf(stderr, "rsa_output + PKCS blocking \n");
                	aux_xdump(out_octets->octets, out_octets->noctets, 0);
	        	fprintf(stderr, "\n");
#endif

		}

		if(sec_time) {
			gettimeofday(&sec_tp2, &sec_tzp2);
			rsa_usec = (rsa_sec + (sec_tp2.tv_sec - sec_tp1.tv_sec)) * 1000000 + 	rsa_usec + (sec_tp2.tv_usec - sec_tp1.tv_usec);
			rsa_sec = rsa_usec/1000000;
			rsa_usec = rsa_usec % 1000000;
		}
		break;
	case DES:
	case DES3:
		if(sec_time) gettimeofday(&sec_tp1, &sec_tzp1);

		n = des_decrypt(in_bits, out_octets, more, &got_key);

		if(sec_time) {
			gettimeofday(&sec_tp2, &sec_tzp2);
			des_usec = (des_sec + (sec_tp2.tv_sec - sec_tp1.tv_sec)) * 1000000 + 	des_usec + (sec_tp2.tv_usec - sec_tp1.tv_usec);
			des_sec = des_usec/1000000;
			des_usec = des_usec % 1000000;
		}
		break;
	case IDEA:
		if(sec_time) gettimeofday(&sec_tp1, &sec_tzp1);

		n = idea_decrypt(in_bits, out_octets, more, &got_key);

		if(sec_time) {
			gettimeofday(&sec_tp2, &sec_tzp2);
			idea_usec = (idea_sec + (sec_tp2.tv_sec - sec_tp1.tv_sec)) * 1000000 + 	idea_usec + (sec_tp2.tv_usec - sec_tp1.tv_usec);
			idea_sec = idea_usec/1000000;
			idea_usec = idea_usec % 1000000;
		}
		break;
	default:
		aux_add_error(EALGID, "invalid or unknown alg_id", CNULL, 0, proc);
		return -1;
	}
	if (more == SEC_END) {
		algenc = NoAlgEnc;
		aux_free2_KeyInfo(&got_key);
		sec_state = F_null;
	}
	if (n < 0) {
		AUX_ADD_ERROR;
		return (-1);
	}
	return (n);
}


/***************************************************************************************
 *                                 sec_del_key                                         *
 ***************************************************************************************/

/***************************************************************
 *
 * Procedure sec_del_key
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_del_key(
	KeyRef	  keyref
)

#else

RC sec_del_key(
	keyref
)
KeyRef	  keyref;

#endif

{
	int             rc;
	PSESel         *pse_sel;
	register char  *dd;

#ifdef SCA
	Key		key;

#endif


	char           *proc = "sec_del_key";

#ifdef SCA
	if ((keyref == -1) || (keyref == 0)) 
#else
	if (keyref <= 0) 
#endif				/* SCA */
	{
		aux_add_error(EINVALID, "invalid key reference", CNULL, 0, proc);
		return (-1);
	}
#ifdef SCA
/************************  S C  -  P A R T  *******************************************/


	/*
	 * Check whether key stored within the SCT/SC shall be deleted.
	 */

	key.keyref  = keyref;
	key.pse_sel = (PSESel *) 0;
        key.key     = (KeyInfo *)0;

	if ((call_secsc = handle_in_SCTSC(&key, FALSE)) == -1) {
		AUX_ADD_ERROR;
		return (-1);
	}
	if (call_secsc == TRUE) {
			
		/* Delete key in SC/SCT */
#ifdef SECSCTEST
		fprintf(stderr, "Call of secsc_del_key\n");
#endif
		if (secsc_del_key(keyref)) {
			AUX_ADD_ERROR;
			return (-1);
		}
		return (0);	/* key has been deleted */
	}		
	/**************************************************************************************/

	/*
	 * The following is only performed, if - the SC is not available or -
	 * the key to be deleted is stored in the key_pool
	 * 
	 * 
	 */
#endif				/* SCA */



	pse_sel = set_key_pool(keyref);

	rc = sec_delete(pse_sel);
	if(pse_sel->pin) strzfree(&(pse_sel->pin));
	if (rc)	AUX_ADD_ERROR;
	return (rc);
}


/***************************************************************************************
 *                                     sec_delete                                      *
 ***************************************************************************************/

#ifndef MAC
#include <dirent.h>
#endif /* !MAC */

/***************************************************************
 *
 * Procedure sec_delete
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_delete(
	PSESel	 *pse_sel
)

#else

RC sec_delete(
	pse_sel
)
PSESel	 *pse_sel;

#endif

{
	struct PSE_Objects *nxt, *pre, *new;
	int             fd, ret;
	char            *object, *cmd, *o;
#ifndef MAC
	struct dirent   *dp, *readdir();
	DIR             * dir, *opendir();
#endif /* !MAC */


	char           *proc = "sec_delete";

	if (chk_parm(pse_sel)) {
		AUX_ADD_ERROR;
		return (-1);
	}
#ifdef SCA
/************************  S C  -  P A R T  *******************************************/

	/*
	 * Check whether SC available and application = SC-application.
	 */

	if ((pse_location = sec_psetest(pse_sel->app_name)) == ERR_in_psetest) {
		AUX_ADD_ERROR;
		return (-1);
	}
	if (pse_location == SCpse) {

		/*
		 * SC available and application = SC application.
		 */

		/* If SC application not open => open it */
		if (handle_SC_app(pse_sel->app_name)) {
			AUX_ADD_ERROR;
			return (-1);
		}

		if (pse_sel->object.name && strlen(pse_sel->object.name)) {

			/*
			 * If object = SC object    => delete object on SC
			 */

			if (aux_AppObjName2SCObj(pse_sel->app_name, pse_sel->object.name)) {
				/* delete object on SC */
#ifdef SECSCTEST
				fprintf(stderr, "Call of secsc_delete (obj)\n");
#endif
				if (secsc_delete(pse_sel)) {
					AUX_ADD_ERROR;
					return (-1);
				}
				/* delete object from SC toc */
				if (delete_SCToc(pse_sel)) {	
					AUX_ADD_ERROR;
					return (-1);
				}

				return (0);	/* object on SC has been
						 * deleted */
			}
		}

	}			/* if (SC available && app = SC-app) */

	/**************************************************************************************/

	/*
	 * The following is only performed, 
	 * if an application | object on the SW-PSE shall be deleted.
	 * 
	 * --------------------------------------------------------------
	 * If the SC is available and an application on the SC could be 
	 * opened, the PIN read from the SC is used as PIN for the SW-PSE 
	 * application/object.
	 */
#endif				/* SCA */



	if ((fd = open_object(pse_sel, O_WRONLY | O_TRUNC)) == -1) {
		AUX_ADD_ERROR;
		return (-1);
	}
	close(fd);

	if (!pse_sel->object.name || !strlen(pse_sel->object.name)) {
#ifndef MAC 
		/* delete PSE */

		if (!(object = pse_name(pse_sel->app_name, ""))) {
			AUX_ADD_ERROR;
			return (-1);
		}
		if (!(dir = opendir(object))) {
			if (object) free(object);
			aux_add_error(ESYSTEM, "Can't access PSE", (char *)  object, char_n, proc);
			return (-1);
		}
		o = &object[strlen(object)];
		if (*o != PATH_SEPARATION_CHAR) {
			*o++ = PATH_SEPARATION_CHAR;
			*o = '\0';
		}
		while ((dp = readdir(dir))) {
			strcpy(o, dp->d_name);
			unlink(object);
		}
		closedir(dir);
		*o-- = '\0';
		if (*o == PATH_SEPARATION_CHAR)
			*o = '\0';
		if ((ret = rmdir(object)) < 0) {
			aux_add_error(ESYSTEM, "Can't remove PSE", (char *) object, char_n, proc);
		}
		if (object) free(object);
		return (ret);
#endif /* !MAC */
	}

	/* delete PSE object */

	/*
	 * Check whether pse_sel->object.name exists in toc (we have the toc
	 * from sec_open)
	 */

	nxt = psetoc->obj;
	pre = (struct PSE_Objects *) 0;
	while (nxt) {
		if (strcmp(nxt->name, pse_sel->object.name) == 0) {

			/* yes */

			/* chain out */

			if (pre) pre->next = nxt->next;
			else {
				psetoc->obj = nxt->next;
				if(!psetoc->obj) {

					/* last object removed from SW-PSE
                                         * delete entire SW-PSE if application is SC app */
#ifdef SCA
					if ((pse_location = sec_psetest(pse_sel->app_name)) == ERR_in_psetest) {
						AUX_ADD_ERROR;
						return (-1);
					}
					if (pse_location == SCpse) {
						if(pse_sel->object.name) free(pse_sel->object.name);
						pse_sel->object.name = CNULL;
						strzfree(&(pse_sel->object.pin));
						ret = sec_delete(pse_sel);
						strzfree(&(pse_sel->pin));
						aux_free_PSEToc(&psetoc);
						if(ret == 0) return(0);
					}
#endif
				}
			}

			/* Write toc */

			ret = write_toc(pse_sel, psetoc, O_WRONLY | O_TRUNC);
			if (ret < 0) {
				AUX_ADD_ERROR;
				return (-1);
			}
			/* unlink all corresponding files) */

			if (!(object = pse_name(pse_sel->app_name, pse_sel->object.name))) {
				AUX_ADD_ERROR;
				return (-1);
			}
			unlink(object);
			strcat(object, ".sf");
			unlink(object);
			object[strlen(object) - 3] = '\0';
			strcat(object, ".pw");
			unlink(object);
			if(object) free(object);
			return (0);
		}
		pre = nxt;
		nxt = nxt->next;
	}
	aux_add_error(EOBJNAME, "object is not in toc", (char *) pse_sel->object.name, char_n, proc);
	return (-1);
}


/***************************************************************************************
 *                               sec_rename                                            *
 ***************************************************************************************/

/***************************************************************
 *
 * Procedure sec_rename
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_rename(
	PSESel	 *pse_sel,
	char	 *objname
)

#else

RC sec_rename(
	pse_sel,
	objname
)
PSESel	 *pse_sel;
char	 *objname;

#endif

{
#ifdef MAC
	FILE           *f;
#endif /* MAC */
	char           *object, *newobj;
	struct PSE_Objects **toc_obj;
	int             rc;
	char           *proc = "sec_rename";

	if (chk_parm(pse_sel)) {
		AUX_ADD_ERROR;
		return (-1);
	}
	if (!pse_sel->object.name || !*(pse_sel->object.name)) {
		aux_add_error(EINVALID, "object missing", (char *) pse_sel, PSESel_n, proc);
		return -1;
	}
	/* locate object in toc */
	if (!(psetoc = chk_toc(pse_sel, FALSE))) {
		AUX_ADD_ERROR;
		return (-1);
	}
	toc_obj = locate_toc(pse_sel->object.name);
	if (!toc_obj || !*toc_obj) {
		aux_add_error(EOBJNAME, "object not in toc ", (char *) pse_sel, PSESel_n, proc);
		return -1;
	}
	rc = -1;
	if (strcmp(pse_sel->object.name, objname)) {	/* objects differ */
		object = pse_name(pse_sel->app_name, pse_sel->object.name);
		newobj = pse_name(pse_sel->app_name, objname);
		if (!object || !newobj) {
			AUX_ADD_ERROR;
			goto rename_err;
		}
/* object wird in newobject umbenannt. Falls es nicht klappt und Fehler = ENOENT ist,
   wird object.sf in newobject.sf umbenannt. Wenn das geklappt hat, wird noch
   object.pw in newobject.pw umbenannt. AS */
   
#ifndef MAC
		if (link(object, newobj) < 0) {
			if (errno != ENOENT) {
				aux_add_error( ESYSTEM, "Can't link file", (char *) newobj, char_n, proc);
				goto rename_err;
			}
			/* object possibly secure file */
			strcat(object, ".sf");
			strcat(newobj, ".sf");
			if (link(object, newobj) < 0) {
				aux_add_error( ESYSTEM, "Can't link file", (char *) newobj, char_n, proc);
				goto rename_err;
			}
			unlink(object);
			strcpy( object + strlen(object) - 2, "pw");
			strcpy( newobj + strlen(newobj) - 2, "pw");
			link(object, newobj);	/* ignore any failure */
			unlink(object);
		} else {
			unlink(object);
		}
#else
        if (rename(object, newobj) != 0)
           {
           if ( ( (f = fopen(object, "r")), errno) != ENOENT )
              {  
              aux_add_error( ESYSTEM, "Can't rename file", (char *) newobj, char_n, proc);
              if (f) fclose(f);
              goto rename_err;
              }
           if (f) fclose(f);
           
           /* object possibly secure file */
           strcat(object,".sf");
           strcat(newobj,".sf");
           if (rename(object, newobj) != 0)
              {
              aux_add_error( ESYSTEM, "Can't rename file", (char *) newobj, char_n, proc);
              goto rename_err;
              }
              
           strcpy(object + strlen(object) - 2, "pw");
           strcpy(newobj + strlen(newobj) - 2, "pw");
           rename(object, newobj);     /* ignore any failure */
           }
#endif /* MAC */

		if (update_toc(pse_sel, *toc_obj, objname)) {
			AUX_ADD_ERROR;
			goto rename_err;
		}
	} /* else :  objects are same, do not complain ignore
	   * instead */


	rc = 0;

	/*------ error handling ------*/
rename_err:
	if (object)
		free(object);
	if (newobj)
		free(newobj);
	return rc;
}


/***************************************************************************************
 *                               update_toc                                            *
 ***************************************************************************************/

/***************************************************************
 *
 * Procedure update_toc
 *
 ***************************************************************/
#ifdef __STDC__

static int update_toc(
	PSESel			 *pse_sel,
	struct PSE_Objects	 *objp,
	char			 *objname
)

#else

static int update_toc(
	pse_sel,
	objp,
	objname
)
PSESel			 *pse_sel;
struct PSE_Objects	 *objp;
char			 *objname;

#endif

{
	RC              rc;
	char           *proc = "update_toc";

	if (psetoc && objp && objp->name && objname) {
		free(objp->name);
		objp->name = (char *) malloc(strlen(objname) + 1);
		if (!objp->name) {
			aux_add_error(EMALLOC, "objp->name", CNULL, 0, proc);
			return -1;
		}
		strcpy(objp->name, objname);
		rc = write_toc(pse_sel, psetoc, O_WRONLY | O_TRUNC);
		if (rc)
			AUX_ADD_ERROR;
		return (rc);
	} else {
		aux_add_error(EINVALID, "invalid parameter", CNULL, 0, proc);
		return -1;
	}
}
#ifdef SCA

/***************************************************************************************
 *                               sec_sc_eject (if SCA is defined)
 ***************************************************************************************/
/***************************************************************
 *
 * Procedure sec_sc_eject
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_sc_eject(
	SCTSel	sct_sel
)

#else

RC sec_sc_eject(
	sct_sel
)
SCTSel	  sct_sel;
#endif

{
	int             rc = 0;

	char           *proc = "sec_sc_eject";

        if ((rc = secsc_sc_eject(sct_sel)) == -1)  {
		AUX_ADD_ERROR;
	}

	return (rc);
}
#else
/***************************************************************************************
 *                               sec_sc_eject (empty function if SCA is not defined)
 ***************************************************************************************/

#ifdef __STDC__

RC sec_sc_eject(
	int	sct_sel
)

#else

RC sec_sc_eject(
	sct_sel
)
int	  sct_sel;
#endif


{
	return (0);

}

#endif

/***************************************************************************************
 *                               sec_encrypt                                           *
 ***************************************************************************************/

/***************************************************************
 *
 * Procedure sec_encrypt_PKCS
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_encrypt_PKCS(
	OctetString	 *in_octets,
	BitString	 *out_bits,
	More		  more,
	Key		 *key,
	Boolean		  sc
)

#else

RC sec_encrypt_PKCS(
	in_octets,
	out_bits,
	more,
	key,
	sc
)
OctetString	 *in_octets;
BitString	 *out_bits;
More		  more;
Key		 *key;
Boolean		  sc;

#endif

{
	char           		*proc = "sec_encrypt_PKCS";
	static OctetString      input;
	static int		max_data_size = 0, sum = 0;
	int			n, blocksize, data_size;
	OctetString		data_block, *rsa_input;
	More			more_blocks;

/* initialisation if first call */

	if(!max_data_size) {
		blocksize = (RSA_PARM(rsaEncryption->param) - 1) / 8;
		max_data_size = blocksize - 10; /* This means 8 octets random padding string (PS)  PKCS : 0x00 0x02 PS 0x00 D */
		if (max_data_size <= 0) {
			aux_add_error(EINVALID, "blocksize to small for PKCS with 8 octets padding string (PS)", CNULL, 0, proc);
			return (-1);
		}
		input.noctets = 0;
		input.octets = CNULL;
	}

/* add input stream to the rest of the last call */

	if (aux_append_OctetString(&input, in_octets) < 0) {
		AUX_ADD_ERROR;
		return (-1);
	}


	data_block.octets = input.octets;

	while(input.noctets > 0 && (input.noctets >= max_data_size || more == SEC_END)) {
/* encrypt each block */

		if(input.noctets > max_data_size) {
			data_block.noctets = max_data_size;
			more_blocks = SEC_MORE;
		}
		else {
			data_block.noctets = input.noctets;
			more_blocks = SEC_END;
		}



/* do the PKCS padding of one block */
		if ((rsa_input = aux_create_PKCSBlock(PKCS_BT_02, &data_block)) == NULLOCTETSTRING) {
			AUX_ADD_ERROR;
			return (-1);
		}

#ifdef SECSCTEST
		fprintf(stderr, "rsa_input + PKCS blocking \n");
                aux_xdump(rsa_input->octets, rsa_input->noctets, 0);
		fprintf(stderr, "\n");
#endif

/* do RSA on SmartCard or with software */

#ifdef SCA
		if(sc) {

				if ((n = secsc_encrypt(rsa_input, out_bits, more_blocks, key)) == -1) {
					AUX_ADD_ERROR;
					return (-1);
				}

		}
#endif
		if(!sc) {
				if(sec_verbose) {
					fprintf(stderr, "RSA input block (PKCS #2):\n");
					aux_fprint_OctetString(stderr, rsa_input);
				}
				n = rsa_encrypt(rsa_input, out_bits, more_blocks, public_modulus_length);
#ifdef SECSCTEST
				fprintf(stderr, "encrypted block \n");
                                aux_xdump(out_bits->bits, out_bits->nbits / 8, 0);
				fprintf(stderr, "\n");
#endif

		}
		aux_free_OctetString(&rsa_input);


/* set next data block */
		input.noctets -= data_block.noctets;
		data_block.octets += data_block.noctets;
	}


/* delete the processed data blocks */
	if(input.noctets) {
		bcopy(data_block.octets, input.octets, input.noctets);
		input.octets = realloc(input.octets, input.noctets);
	}
	else if(input.octets) { 
		free(input.octets);
		input.octets = CNULL;
	}


	sum += n;
	if(more == SEC_END) {
		max_data_size = 0;
		return(sum);
	}
	return(n);

}
/***************************************************************
 *
 * Procedure sec_encrypt
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_encrypt(
	OctetString	 *in_octets,
	BitString	 *out_bits,
	More		  more,
	Key		 *key
)

#else

RC sec_encrypt(
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

	int             n, rc;
	static AlgEnc   algenc = NoAlgEnc;
	static AlgSpecial     algspecial = NoAlgSpecial;
	OctetString     *rsa_input;

#ifdef SCA
	int             no_enc;

#endif

	char           *proc = "sec_encrypt";


#ifdef SCA
/************************  S C  -  P A R T  *******************************************/


	/*
	 * Check whether encryption shall be done within the
	 * SCT/SC - If the key is selected with object name,
	 * the PIN for the SW-PSE is read from the SC.
	 */

	if ((call_secsc = handle_in_SCTSC(key, SC_encrypt)) == -1) {
		AUX_ADD_ERROR;
		return (-1);
	}

	if (call_secsc == TRUE) {

		/* encrypt within SCT/SC */
#ifdef SECSCTEST
		fprintf(stderr, "Call of secsc_encrypt\n");
#endif

		if ((no_enc = secsc_encrypt(in_octets, out_bits, more, key)) == -1) {
			AUX_ADD_ERROR;
			return (-1);
		}
		return (no_enc);	/* encryption done within SC/SCT */

	}		/* if (call_secsc == TRUE) */
	/**************************************************************************************/

	/*
	 * The following is performed in case of
	 * 
	 * RSA, if: - global variable "SC_encrypt" is FALSE or - the SC is not
	 * available or - the key is not delivered by the calling routine
	 * 
	 * 
	 * DES/DES3, if - the SC is not available or - encryption key doesn't
	 * address a key within SCT/SC
	 * 
	 * --------------------------------------------------------------
	 * If the SC is available and an application on the SC could be 
	 * opened, the PIN read from the SC is used as PIN for the SW-PSE 
	 * application/object.
	 */
#endif				/* SCA */



	if (sec_state == F_null) {
		if (get2_keyinfo_from_key(&got_key, key) < 0) {
			AUX_ADD_ERROR;
			return -1;
		}
		public_modulus_length = sec_get_keysize(&got_key);
		algenc = aux_ObjId2AlgEnc(got_key.subjectAI->objid);
		algspecial = aux_ObjId2AlgSpecial(got_key.subjectAI->objid);
		if (algenc == RSA) {
			rc = rsa_set_key((char *)&got_key.subjectkey, 1);
			if (rc != 0) {
				AUX_ADD_ERROR;
				return -1;
			}
		}
		if(key->alg) {
			if(aux_ObjId2AlgEnc(key->alg->objid) != algenc) {
				aux_add_error(EINVALID, "Invalid algorithm for encryption in key->alg", (char *) key->alg, AlgId_n, proc);
				return -1;
			}
			algspecial = aux_ObjId2AlgSpecial(key->alg->objid);
		}
		if(sec_time) {
			if(algenc == RSA) rsa_sec = rsa_usec = 0;
			if(algenc == IDEA) idea_sec = idea_usec = 0;
			else des_sec = des_usec = 0;
		}
		sec_state = F_encrypt;
	} else if (sec_state != F_encrypt) {
		aux_add_error(EENCRYPT, "wrong sec_state", CNULL, 0, proc);
		return -1;
	}

	if(sec_verbose) {
		fprintf(stderr, "Input to sec_encrypt:\n");
		aux_fprint_OctetString(stderr, in_octets);
	}

	switch (algenc) {

	case RSA:

		if(sec_time) gettimeofday(&sec_tp1, &sec_tzp1);

		if(algspecial == PKCS_BT_02) {

			/*  Here goes PKCS#1 ...   */
			n = sec_encrypt_PKCS(in_octets, out_bits, more, key, FALSE); /* FALSE: not on SC */
		}
		else n = rsa_encrypt(in_octets, out_bits, more, public_modulus_length);
		if(sec_time) {
			gettimeofday(&sec_tp2, &sec_tzp2);
			rsa_usec = (rsa_sec + (sec_tp2.tv_sec - sec_tp1.tv_sec)) * 1000000 + 	rsa_usec + (sec_tp2.tv_usec - sec_tp1.tv_usec);
			rsa_sec = rsa_usec/1000000;
			rsa_usec = rsa_usec % 1000000;
		}
		break;
	case DES:
	case DES3:
		if(sec_time) gettimeofday(&sec_tp1, &sec_tzp1);
		n = des_encrypt(in_octets, out_bits, more, &got_key);
		if(sec_time) {
			gettimeofday(&sec_tp2, &sec_tzp2);
			des_usec = (des_sec + (sec_tp2.tv_sec - sec_tp1.tv_sec)) * 1000000 + 	des_usec + (sec_tp2.tv_usec - sec_tp1.tv_usec);
			des_sec = des_usec/1000000;
			des_usec = des_usec % 1000000;
		}
		break;
	case IDEA:
		if(sec_time) gettimeofday(&sec_tp1, &sec_tzp1);
		n = idea_encrypt(in_octets, out_bits, more, &got_key);
		if(sec_time) {
			gettimeofday(&sec_tp2, &sec_tzp2);
			idea_usec = (idea_sec + (sec_tp2.tv_sec - sec_tp1.tv_sec)) * 1000000 + 	idea_usec + (sec_tp2.tv_usec - sec_tp1.tv_usec);
			idea_sec = idea_usec/1000000;
			idea_usec = idea_usec % 1000000;
		}
		break;
	default:
		aux_add_error(EALGID, "invalid or unknown alg_id", CNULL, 0, proc);
		return -1;
	}

	if (more == SEC_END) {
		algenc = NoAlgEnc;
		aux_free2_KeyInfo(&got_key);
		sec_state = F_null;
	}
	if (n < 0) {
		AUX_ADD_ERROR;
		return (-1);
	}
	return (n);
}


/***************************************************************************************
 *                               sec_gen_key                                           *
 ***************************************************************************************/

/***************************************************************
 *
 * Procedure sec_gen_key
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_gen_key(
	Key	 *key,
	Boolean	  replace
)

#else

RC sec_gen_key(
	key,
	replace
)
Key	 *key;
Boolean	  replace;

#endif

{
	int             rc, i;
	BitString      *public_keybits, *secret_keybits;
	KeyInfo  	subjectprivatekeyinfo;
	ObjId          *keytype;
	AlgId          *SKalgid;
	ObjId          *af_get_objoid();
	int             keysize;


	char           *proc = "sec_gen_key";

	public_modulus_length = 0;


#ifdef SCA
/************************  S C  -  P A R T  *******************************************/


	/*
	 * Check whether key generation shall be done within the
	 * SCT/SC - If the key is selected with object name, the PIN
	 * for the SW-PSE is read from the SC.
	 */

	if ((call_secsc = handle_in_SCTSC(key, FALSE)) == -1) {
		AUX_ADD_ERROR;
		return (-1);
	}

	if (call_secsc == TRUE) {

		/* generate key for SCT/SC */
#ifdef SECSCTEST
		fprintf(stderr, "Call of secsc_gen_key\n");
#endif
		if (secsc_gen_key(key, replace)) {
			AUX_ADD_ERROR;
			return (-1);
		}

		/*
		 *  Update of SCToc is done in secsc_gen_key()	
		 */		

		return (0);	/* key has been generated for SC/SCT */

	}		/* if (call_secsc == TRUE) */
	/**************************************************************************************/

	/*
	 * The following is only performed, if - the SC is not available or -
	 * the key to be generated shall not be stored within SCT/SC
	 * 
	 * 
	 * --------------------------------------------------------------
	 * If the SC is available and an application on the SC could be 
	 * opened, the PIN read from the SC is used as PIN for the SW-PSE 
	 * application/object.
	 */
#endif				/* SCA */



	if ((key->keyref > 0) || (key->pse_sel != (PSESel * ) 0)) {
		if (key->pse_sel != (PSESel * ) 0) {
			if (sec_create(key->pse_sel) < 0) {
				if (replace == FALSE) {
					aux_add_error(EINVALID, "Can't create object because object exists and replace flag is not set", (char *) key->pse_sel, PSESel_n, proc);
					return -1;
				}
			}
		}
		else if(key->keyref > 0) {
			if(get2_keyinfo_from_keyref(&subjectprivatekeyinfo, key->keyref) == 0) {
				aux_free2_KeyInfo(&subjectprivatekeyinfo);
				if(replace == FALSE) {
					aux_add_error(EINVALID, "Can't create object because object exists and replace flag is not set", (char *) key->pse_sel, PSESel_n, proc);
					return -1;
				}
			}
		}

	}

	if(key->pse_sel) {
		keytype = af_get_objoid(key->pse_sel->object.name);
		if(!aux_cmp_ObjId(keytype, Uid_OID)) aux_free_ObjId(&keytype);
	}
	else keytype = (ObjId *)0;

	switch (aux_ObjId2AlgEnc(key->key->subjectAI->objid)) {
	case RSA:
		keysize = RSA_PARM(key->key->subjectAI->param);
		if ((key->keyref == 0) && (key->pse_sel == (PSESel * ) 0)) {
			aux_add_error(EINVALID, "invalid key", CNULL, 0, proc);	/* secret key would be
										 * lost */
			return -1;
		}
		rc = rsa_gen_key(keysize, &secret_keybits, &public_keybits);

		if (rc) {
			AUX_ADD_ERROR;
			return -1;
		}
		/* public key */

		key->key->subjectkey.nbits = public_keybits->nbits;
		key->key->subjectkey.bits = public_keybits->bits;
		if(!keytype) keytype = RSA_SK_OID;
		SKalgid = rsa;
		break;

	case DSA:
		keysize = sec_dsa_keysize;
		if ((key->keyref == 0) && (key->pse_sel == (PSESel * ) 0)) {
			aux_add_error(EINVALID, "invalid key", CNULL, 0, proc);	/* secret key would be
										 * lost */
			return -1;
		}
		rc = dsa_gen_key(keysize, &secret_keybits, &public_keybits);

		if (rc) {
			AUX_ADD_ERROR;
			return -1;
		}
		/* public key */

		key->key->subjectkey.nbits = public_keybits->nbits;
		key->key->subjectkey.bits = public_keybits->bits;
		if(!keytype) keytype = DSA_SK_OID;
		if(sec_dsa_predefined) SKalgid = dsaSK;
		else SKalgid = dsa;
		break;

	case IDEA:
		secret_keybits = sec_random_bstr(128);

		if(!keytype) keytype = IDEA_OID;
		SKalgid = key->key->subjectAI;
		break;
	case DES:
		while (1) {
			secret_keybits = sec_random_bstr(64);

			/* check for bad DES keys */
			for (i = 0; i < no_of_bad_des_keys; i++) {
				if (bcmp(secret_keybits->bits, bad_des_keys[i], 8) == 0)
					break;
			}
			if (i == no_of_bad_des_keys)
				break;
		}
		if(!keytype) keytype = DES_OID;
		SKalgid = key->key->subjectAI;
		break;
	case DES3:
		while (1) {
			secret_keybits = sec_random_bstr(128);

			/* check for bad DES keys */
			for (i = 0; i < no_of_bad_des_keys; i++) {
				if (bcmp(secret_keybits->bits, bad_des_keys[i], 8) == 0)
					break;
				if (bcmp(secret_keybits->bits + 8, bad_des_keys[i], 8) == 0)
					break;
			}
			if (i == no_of_bad_des_keys)
				break;
		}
		if(!keytype) keytype = DES3_OID;
		SKalgid = key->key->subjectAI;
		break;
	default:
		aux_add_error(EALGID, "unknown alg_id", (char *) key->key->subjectAI, AlgId_n, proc);
		return -1;
	}

	/* secret RSA or DSA key or DES key */

	subjectprivatekeyinfo.subjectAI = aux_cpy_AlgId(SKalgid);
	if(aux_ObjId2ParmType(subjectprivatekeyinfo.subjectAI->objid) == PARM_INTEGER) {
		*(int *)subjectprivatekeyinfo.subjectAI->param = keysize;
	}
	subjectprivatekeyinfo.subjectkey.nbits = secret_keybits->nbits;
	subjectprivatekeyinfo.subjectkey.bits = secret_keybits->bits;

	/* store key */

		
	if (put_keyinfo_according_to_key(&subjectprivatekeyinfo, key, keytype) < 0) {
		aux_free2_KeyInfo(&subjectprivatekeyinfo);
		AUX_ADD_ERROR;
		return -1;
	}
	for(i = 0; i < secret_keybits->nbits / 8; i++) secret_keybits->bits[i] = 0;
	aux_free2_KeyInfo(&subjectprivatekeyinfo);
	return 0;
}


/***************************************************************************************
 *                               sec_get_EncryptedKey                                 *
 ***************************************************************************************/

/***************************************************************
 *
 * Procedure sec_get_EncryptedKey
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_get_EncryptedKey(
	EncryptedKey	 *encrypted_key,
	Key		 *plain_key,
	Key		 *encryption_key
)

#else

RC sec_get_EncryptedKey(
	encrypted_key,
	plain_key,
	encryption_key
)
EncryptedKey	 *encrypted_key;
Key		 *plain_key;
Key		 *encryption_key;

#endif

{
	OctetString     in_octets;
	KeyInfo        *plain_keyinfo, *encryption_keyinfo;

	AlgEnc          plainkey_algenc;
	int             i, j, len_plain_key, pos_plain_key, len_extended_key;
	OctetString    *extended_key = (OctetString * ) 0;


	char           *proc = "sec_get_EncryptedKey";

	if (!plain_key || !encryption_key || !encrypted_key) {
		aux_add_error(EINVALID, "key missing", CNULL, 0, proc);
		return (-1);
	}
#ifdef SCA
/************************  S C  -  P A R T  *******************************************/


	/*
	 * Check whether key to be encrypted (plain_key) is stored in
	 * the SCT/SC - If the key is selected with object name, the
	 * PIN for the SW-PSE is read from the SC.
	 */

	if ((call_secsc = handle_in_SCTSC(plain_key, FALSE)) == -1) {
		AUX_ADD_ERROR;
		return (-1);
	}
	if (call_secsc == TRUE) {

		/* key to be encrypted is stored within SCT/SC */

		/*
		 * in this case the encryption key must be delivered
		 * in encryption_key->key
		 */

		if (!encryption_key->key) {
			aux_add_error(ESCNOTSUPP, "plainkey = SCT/SC-key && encryption key not delivered in key!", CNULL, 0, proc);
			return (-1);
		}
#ifdef SECSCTEST
		fprintf(stderr, "Call of secsc_get_EncryptedKey\n");
#endif
		if (secsc_get_EncryptedKey(encrypted_key, plain_key, encryption_key)) {
			AUX_ADD_ERROR;
			return (-1);
		}
		return (0);	/* Encryption of key was successful! */

	}		/* if (call_secsc == TRUE) */
	/**************************************************************************************/

	/*
	 * The following is only performed, if the SC is not available or 
	 * plain_key doesn't address a key within SCT/SC
	 * 
	 * 
	 * --------------------------------------------------------------
	 * If the SC is available and an application on the SC could be 
	 * opened, the PIN read from the SC is used as PIN for the SW-PSE 
	 * application/object.
	 */
#endif				/* SCA */



	if (!(plain_keyinfo = get_keyinfo_from_key(plain_key))) {
		AUX_ADD_ERROR;
		return (-1);
	}
	if (!(encryption_keyinfo = get_keyinfo_from_key(encryption_key))) {
		AUX_ADD_ERROR;
		aux_free_KeyInfo(&plain_keyinfo);
		return (-1);
	}
	if (!(encrypted_key->encryptionAI = aux_cpy_AlgId(encryption_keyinfo->subjectAI))) {
		AUX_ADD_ERROR;
		aux_free_KeyInfo(&encryption_keyinfo);
		aux_free_KeyInfo(&plain_keyinfo);
		return (-1);
	}
	if (!(encrypted_key->subjectAI = aux_cpy_AlgId(plain_keyinfo->subjectAI))) {
		AUX_ADD_ERROR;
		aux_free_KeyInfo(&encryption_keyinfo);
		aux_free_KeyInfo(&plain_keyinfo);
		return (-1);
	}
	if (!(encrypted_key->subjectkey.bits = (char *) malloc((plain_keyinfo->subjectkey.nbits / 8) + 128))) {
		aux_add_error(EMALLOC, "encrypted_key->subjectkey.bits", CNULL, 0, proc);
		aux_free_KeyInfo(&encryption_keyinfo);
		aux_free_KeyInfo(&plain_keyinfo);
		return (-1);
	}
	encrypted_key->subjectkey.nbits = 0;

#ifdef SECSCTEST
		fprintf(stderr, "plain_key:\n");
		aux_fxdump(stderr, plain_keyinfo->subjectkey.bits, plain_keyinfo->subjectkey.nbits / 8, 0);
		fprintf(stderr, "\n");
#endif

	plainkey_algenc = aux_ObjId2AlgEnc(plain_keyinfo->subjectAI->objid);

	if (((plainkey_algenc != DES) && (plainkey_algenc != DES3)) || 
	     (aux_ObjId2AlgEnc(encryption_keyinfo->subjectAI->objid) != RSA) || 
	     (aux_ObjId2AlgSpecial(encryption_keyinfo->subjectAI->objid) == PKCS_BT_02) || 
	     (aux_ObjId2AlgSpecial(encryption_key->alg->objid) == PKCS_BT_02) || 
	     ((RSA_PARM(encryption_keyinfo->subjectAI->param) % 8) != 0)) {

		/*
		 * No padding before encryption.
		 */

		in_octets.noctets = (plain_keyinfo->subjectkey.nbits / 8);
		if (plain_keyinfo->subjectkey.nbits % 8)
			in_octets.noctets++;
		in_octets.octets = plain_keyinfo->subjectkey.bits;
	}
	else {

		/* 
		 * Extend DES key to be encrypted before encryption.
		 */ 

		len_extended_key = (RSA_PARM(encryption_keyinfo->subjectAI->param) / 8) - 1;
		if ((extended_key = sec_random_ostr(len_extended_key)) == NULLOCTETSTRING) {
			AUX_ADD_ERROR;
			return (-1);
		}

		len_plain_key = plain_keyinfo->subjectkey.nbits / 8;
		pos_plain_key = len_extended_key - len_plain_key;

		j = pos_plain_key;
		for (i = 0; i < len_plain_key && j < len_extended_key;)
			extended_key->octets[j++] = plain_keyinfo->subjectkey.bits[i++];

#ifdef SECSCTEST
		fprintf(stderr, "extended_key->octets (random string with key):\n");
		aux_fxdump(stderr, extended_key->octets, extended_key->noctets, 0);
		fprintf(stderr, "\n");
#endif
		in_octets.noctets = extended_key->noctets;
		in_octets.octets = extended_key->octets;

	}


	/* 
	 * Encrypt plain key.
	 */ 

	if (sec_encrypt(&in_octets, &(encrypted_key->subjectkey), SEC_END, encryption_key) < 0) {

		AUX_ADD_ERROR;

		if (extended_key)
			aux_free_OctetString(&extended_key);
		aux_free_KeyInfo(&encryption_keyinfo);
		aux_free_KeyInfo(&plain_keyinfo);
		return (-1);
	}
	
	if (extended_key)
		aux_free_OctetString(&extended_key);
	aux_free_KeyInfo(&encryption_keyinfo);
	aux_free_KeyInfo(&plain_keyinfo);

	return (0);
}


/***************************************************************************************
 *                                 sec_get_key                                         *
 ***************************************************************************************/

/***************************************************************
 *
 * Procedure sec_get_key
 *
 ***************************************************************/
#ifdef __STDC__

int sec_get_key(
	KeyInfo	 *keyinfo,
	KeyRef	  keyref,
	Key	 *key
)

#else

int sec_get_key(
	keyinfo,
	keyref,
	key
)
KeyInfo	 *keyinfo;
KeyRef	  keyref;
Key	 *key;

#endif

{
	char           *proc = "sec_get_key";

	if (keyref)
		return (get2_keyinfo_from_keyref(keyinfo, keyref));
	else if (key)
		return (get2_keyinfo_from_key(keyinfo, key));
	else {
		aux_add_error(EINVALID, "don't know which key", CNULL, 0, proc);
		return (-1);
	}
}


/***************************************************************************************
 *                               sec_get_keysize                                       *
 ***************************************************************************************/

/***************************************************************
 *
 * Procedure sec_get_keysize
 *
 ***************************************************************/
#ifdef __STDC__

int sec_get_keysize(
	KeyInfo	 *keyinfo
)

#else

int sec_get_keysize(
	keyinfo
)
KeyInfo	 *keyinfo;

#endif

{

	KeyBits *keybits;
	int ret;

	keybits = d_KeyBits(&keyinfo->subjectkey);
	if(!keybits) return(0);
	ret = keybits->part1.noctets * 8;
	aux_free_KeyBits(&keybits);
	return(ret);
}
	
/***************************************************************************************
 *                                     sec_hash                                        *
 ***************************************************************************************/

/***************************************************************
 *
 * Procedure sec_hash
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_hash(
	OctetString	 *in_octets,
	OctetString	 *hash_result,
	More		  more,
	AlgId		 *alg_id,
	HashInput	 *hash_input
)

#else

RC sec_hash(
	in_octets,
	hash_result,
	more,
	alg_id,
	hash_input
)
OctetString	 *in_octets;
OctetString	 *hash_result;
More		  more;
AlgId		 *alg_id;
HashInput	 *hash_input;

#endif

{
	static int      keysize;
	static AlgHash  algorithm;
	int             rc;
	OctetString    *key_parm_octetstring;

	char           *proc = "sec_hash";

	if (sec_state == F_null) {

		/* first call of sec_hash */

		if(sec_time) {
			hash_sec  = hash_usec = 0;
		}
		algorithm = aux_ObjId2AlgHash(alg_id->objid);

		switch (algorithm) {
		case SQMODN:
			keysize = RSA_PARM(alg_id->param);
			rc = rsa_set_key((char *)&hash_input->sqmodn_input, 1);
			if (rc != 0) {
				AUX_ADD_ERROR;
				return -1;
			}
			if (!(hash_result->octets = malloc((*(alg_id->param) + 7) / 8))) {
				aux_add_error(EMALLOC, "hash_result->octets", CNULL, 0, proc);
				return (-1);
			}
			break;
		case MD2:
		case MD4:
		case MD5:
		case SHA:
			if (!(hash_result->octets = malloc(64))) {
				aux_add_error(EMALLOC, "hash_result->octets", CNULL, 0, proc);
				return (-1);
			}
			break;
		default:
			aux_add_error(EINVALID, "Invalid algorithm", (char *) alg_id, AlgId_n, proc);
			return (-1);

		}
		hash_result->noctets = 0;
		sec_state = F_hash;
	}

	if(sec_time) gettimeofday(&sec_tp1, &sec_tzp1);

	switch (algorithm) {
	case SQMODN:
		rc = hash_sqmodn(in_octets, hash_result, more, keysize);
		break;
	case MD2:
		rc = md2_hash(in_octets, hash_result, more);
		break;
	case MD4:
		rc = md4_hash(in_octets, hash_result, more);
		break;
	case MD5:
		rc = md5_hash(in_octets, hash_result, more);
		break;
	case SHA:
		rc = sha_hash(in_octets, hash_result, more);
		break;
	default:
		aux_add_error(EALGID, "invalid alg_id", CNULL, 0, proc);
		sec_state = F_null;
		return -1;
	}

	if(sec_time) {
		gettimeofday(&sec_tp2, &sec_tzp2);
		hash_usec = (hash_sec + (sec_tp2.tv_sec - sec_tp1.tv_sec)) * 1000000 + 	hash_usec + (sec_tp2.tv_usec - sec_tp1.tv_usec);
		hash_sec = hash_usec/1000000;
		hash_usec = hash_usec % 1000000;
	}

	if (more == SEC_END) {
		sec_state = F_null;
		algorithm = NoAlgHash;
	}
	if (rc)
		aux_add_error(EINVALID, "hash error", CNULL, 0, proc);
	return (rc);
}

/***************************************************************************************
 *                                     sec_keysize                                     *
 ***************************************************************************************/

/*
 *  sec_keysize returns the size of the modulus of the key in keyinfo
 *  which is supposed to be a public RSA key.
 */

/***************************************************************
 *
 * Procedure sec_keysize
 *
 ***************************************************************/
#ifdef __STDC__

int sec_keysize(
	KeyInfo	 *keyinfo
)

#else

int sec_keysize(
	keyinfo
)
KeyInfo	 *keyinfo;

#endif

{
	KeyBits *keybits;
	int keysize;

	keybits = d_KeyBits(&keyinfo->subjectkey);
	keysize = keybits->part1.noctets * 8;
	aux_free_KeyBits(&keybits);
	return(keysize);
}


/***************************************************************************************
 *                                     sec_open                                        *
 ***************************************************************************************/

/***************************************************************
 *
 * Procedure sec_open
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_open(
	PSESel	 *pse_sel
)

#else

RC sec_open(
	pse_sel
)
PSESel	 *pse_sel;

#endif

{
	int             fd;
	char           *proc = "sec_open";


#ifdef SCA
/************************  S C  -  P A R T  *******************************************/

	if (chk_parm(pse_sel)) {
		AUX_ADD_ERROR;
		return (-1);
	}

	/*
	 * Check whether SC available and application = SC-application.
	 */

	if ((pse_location = sec_psetest(pse_sel->app_name)) == ERR_in_psetest) {
		AUX_ADD_ERROR;
		return (-1);
	}
	if (pse_location == SCpse) {

		/*
		 * SC available and application = SC application.
		 */

		/* If SC application not open => open it */
		if (handle_SC_app(pse_sel->app_name)) {
			AUX_ADD_ERROR;
			return (-1);
		}

		if (pse_sel->object.name && strlen(pse_sel->object.name)) {

			/*
			 * If object = SC object    => open object on SC
			 */

			if (aux_AppObjName2SCObj(pse_sel->app_name, pse_sel->object.name)) {
				/* open object on SC */
#ifdef SECSCTEST
				fprintf(stderr, "Call of secsc_open (obj)\n");
#endif
				if (secsc_open(pse_sel)) {
					aux_add_error(EOBJNAME, "Can't open object on SC", (char *) pse_sel->object.name, char_n, proc);
					return (-1);
				}
				return (0);	/* object on SC has been
						 * opened */
			} 
		} 
		else {

			/*
                         * SC application has already been opened.
                         * 
                         * The application on the SW-PSE will be opened,
                         * if an object on the SW-PSE shall be opened.
                         * 
			 */
			return (0);

		}		

	}			/* if (SC available && app = SC-app) */
	/**************************************************************************************/

	/*
	 * The following is only performed, 
	 * if an object on the SW-PSE shall be opened.
	 * 
         * --------------------------------------------------------------
         * If the SC is available and an application on the SC could be 
         * opened, the PIN read from the SC is used as PIN for the SW-PSE 
         * application/object.
	 */
#endif				/* SCA */


	fd = open_object(pse_sel, O_RDONLY);
	if (fd == -1) {
#ifdef SCA
		if(pse_sel->object.name && LASTERROR == EAPPNAME && pse_location == SCpse) 
			aux_add_error(EOBJNAME, "object is not in toc", (char *) pse_sel->object.name, char_n, proc);
		else 
#endif
			AUX_ADD_ERROR;
		return (-1);
	} 
	else if (fd >= 0) close(fd);
	return (0);
}



/***************************************************************************************
 *                                     sec_print_toc                                   *
 ***************************************************************************************/

/***************************************************************
 *
 * Procedure sec_print_toc
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_print_toc(
	FILE	 *ff,
	PSESel	 *pse_sel
)

#else

RC sec_print_toc(
	ff,
	pse_sel
)
FILE	 *ff;
PSESel	 *pse_sel;

#endif

{
	PSEToc         *sctoc;
	char           *proc = "sec_print_toc";

	if (!pse_sel)
		pse_sel = set_key_pool(0);

	if (chk_parm(pse_sel)) {
		AUX_ADD_ERROR;
		return (-1);
	}

#ifdef SCA
	if((pse_location = sec_psetest(pse_sel->app_name)) == ERR_in_psetest) {
		AUX_ADD_ERROR;
		return (-1);
	}
	if(pse_location == SCpse) {
		sctoc = chk_SCToc(pse_sel);
		aux_free_error();
		if(pse_sel->object.name && strlen(pse_sel->object.name)) {
			strrep(&(pse_sel->object.pin), pse_sel->pin);
		}
	}
#endif

	/* Read toc */

	if (!(psetoc = chk_toc(pse_sel, FALSE))) {
#ifdef SCA
		if(pse_location == SWpse || SC_ignore_SWPSE == FALSE) {
			AUX_ADD_ERROR;
			return (-1);
		}
#else
		AUX_ADD_ERROR;
		return (-1);
#endif
	}
	fprintf(ff, "Table of Contents of PSE %s:\n", pse_sel->app_name);
#ifdef SCA
	if(pse_location == SCpse) {
		if (psetoc) aux_fprint_PSEToc(ff, sctoc, psetoc);
		else aux_fprint_PSEToc(ff, sctoc, sctoc);
	}
	else aux_fprint_PSEToc(ff, psetoc, (PSEToc * ) 0);
#else
	aux_fprint_PSEToc(ff, psetoc, (PSEToc * ) 0);
#endif

	return (0);
}


/***************************************************************************************
 *                                     sec_read_toc                                   *
 ***************************************************************************************/

/***************************************************************
 *
 * Procedure sec_read_toc
 *
 ***************************************************************/
#ifdef __STDC__

PSEToc *sec_read_toc(
	PSESel	 *pse_sel
)

#else

PSEToc *sec_read_toc(
	pse_sel
)
PSESel	 *pse_sel;

#endif

{
	char           *proc = "sec_read_toc";

	if (!pse_sel)
		pse_sel = set_key_pool(0);

	if (chk_parm(pse_sel)) {
		AUX_ADD_ERROR;
		return ((PSEToc *)0);
	}

#ifdef SCA
	if((pse_location = sec_psetest(pse_sel->app_name)) == ERR_in_psetest) {
		AUX_ADD_ERROR;
		return ((PSEToc *)0);
	}
	if(pse_location == SCpse) {
		sc_toc = chk_SCToc(pse_sel);
		if (!sc_toc) {
			AUX_ADD_ERROR;
			return ((PSEToc *)0);
		}

		if(pse_sel->object.name && strlen(pse_sel->object.name)) {
			strrep(&(pse_sel->object.pin), pse_sel->pin);
		}
	}
#endif

	/* Read toc */

	if (!(psetoc = chk_toc(pse_sel, FALSE))) {
#ifdef SCA
		if(pse_location == SWpse || SC_ignore_SWPSE == FALSE) {
			AUX_ADD_ERROR;
			return ((PSEToc *)0);
		}
#else
		AUX_ADD_ERROR;
		return ((PSEToc *)0);
#endif
	}

/* sc_toc and psetoc should be merged here! */
#ifdef SCA
	if(sc_toc) return(sc_toc);
	else return(psetoc);
#else
	return(psetoc);
#endif

}

/***************************************************************************************
 *                                     sec_read_tocs                                   *
 ***************************************************************************************/

/***************************************************************
 *
 * Procedure sec_read_tocs
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_read_tocs(
	PSESel	 *pse_sel,
	PSEToc	**SCtoc,
	PSEToc	**PSEtoc
)

#else

RC sec_read_tocs(
	pse_sel,
	SCtoc,
	PSEtoc
)
PSESel	 *pse_sel;
PSEToc	**SCtoc;
PSEToc	**PSEtoc;

#endif

{
	char           *proc = "sec_read_tocs";

	*SCtoc = (PSEToc *)0;

	if (!pse_sel)
		pse_sel = set_key_pool(0);

	if (chk_parm(pse_sel)) {
		AUX_ADD_ERROR;
		return (-1);
	}

#ifdef SCA
	if((pse_location = sec_psetest(pse_sel->app_name)) == ERR_in_psetest) {
		AUX_ADD_ERROR;
		return (-1);
	}
	if(pse_location == SCpse) {
		sc_toc = chk_SCToc(pse_sel);
		if(pse_sel->object.name && strlen(pse_sel->object.name)) {
			strrep(&(pse_sel->object.pin), pse_sel->pin);
		}
	}
#endif

	/* Read toc */

	if (!(psetoc = chk_toc(pse_sel, FALSE))) {
#ifdef SCA
		if(pse_location == SWpse || SC_ignore_SWPSE == FALSE) {
			AUX_ADD_ERROR;
			return (-1);
		}
#else
		AUX_ADD_ERROR;
		return (-1);
#endif
	}

/* sc_toc and psetoc should be merged here! */
#ifdef SCA
	if(sc_toc) *SCtoc = sc_toc;
#endif
	*PSEtoc = psetoc;
	return(0);

}
/***************************************************************************************
 *                            sec_put_EncryptedKey                                    *
 ***************************************************************************************/

/***************************************************************
 *
 * Procedure sec_put_EncryptedKey
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_put_EncryptedKey(
	EncryptedKey	 *encrypted_key,
	Key		 *plain_key,
	Key		 *decryption_key,
	Boolean		  replace
)

#else

RC sec_put_EncryptedKey(
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
	BitString       in_bits;
	static KeyInfo  plain_keyinfo;
	KeyBits        *kb;
	char           *zw;
	int             keysize;
	rsa_parm_type  *rsaparm;
	AlgEnc          plainkey_algenc;
	AlgEnc          deckey_algenc;
	AlgSpecial      deckey_algspecial;

	int		i, j, pos_plain_key, len_plain_key, len_extended_key;
	KeyInfo        *decryption_keyinfo;

	char           *proc = "sec_put_EncryptedKey";

	if (!plain_key || !decryption_key || !encrypted_key) {
		aux_add_error(EINVALID, "key missing", CNULL, 0, proc);
		return (-1);
	}
#ifdef SCA
/************************  S C  -  P A R T  *******************************************/


	/*
	 * For the decryption of the encrypted key within SCT/SC both
	 * the plain_key and the encryption key must address keys
	 * within SCT/SC
	 */

	/*
	 * 1. Check whether plain_key (where to store decrypted key)
	 * addresses a key within SCT/SC - If the key is selected
	 * with object name, the PIN for the SW-PSE is read from the
	 * SC.
	 */

	if ((call_secsc = handle_in_SCTSC(plain_key, FALSE)) == -1) {
		AUX_ADD_ERROR;
		return (-1);
	}
	if (call_secsc == TRUE) {

		/* plain_key within SCT/SC */

		/*
		 * in this case the decryption key must be a key
		 * within SCT/SC, too
		 */

		if ((call_secsc = handle_in_SCTSC(decryption_key, FALSE)) == -1) {
			AUX_ADD_ERROR;
			return (-1);
		}
		if (call_secsc == FALSE) {
			aux_add_error(ESCNOTSUPP, "The plainkey is a key within SCT/SC, but the decryption key is not a key within SCT/SC!", CNULL, 0, proc);
			return (-1);
		}
#ifdef SECSCTEST
		fprintf(stderr, "Call of secsc_put_EncryptedKey\n");
#endif
		if (secsc_put_EncryptedKey(encrypted_key, plain_key, decryption_key, replace)) {
			AUX_ADD_ERROR;
			return (-1);
		}
		return (0);	/* Decryption of key was successful! */

	}		/* if (call_secsc == TRUE) */
	/**************************************************************************************/

	/*
	 * The following is only performed, if the SC is not available or 
	 * plain_key does not address a key within SCT/SC.
	 * 
	 * 
	 * --------------------------------------------------------------
	 * If the SC is available and an application on the SC could be 
	 * opened, the PIN read from the SC is used as PIN for the SW-PSE 
	 * application/object.
	 */
#endif				/* SCA */




	if (!(plain_keyinfo.subjectAI = aux_cpy_AlgId(encrypted_key->subjectAI))) {
		AUX_ADD_ERROR;
		return (-1);
	}
	if (!(plain_keyinfo.subjectkey.bits = (char *) malloc((encrypted_key->subjectkey.nbits / 8) + 32))) {
		aux_add_error(EMALLOC, "plain_keyinfo.subjectkey.bits ", CNULL, 0, proc);
		aux_free2_KeyInfo(&plain_keyinfo);
		return (-1);
	}
	plain_keyinfo.subjectkey.nbits = 0;

	in_bits.nbits = encrypted_key->subjectkey.nbits;
	in_bits.bits = encrypted_key->subjectkey.bits;

	if (sec_decrypt(&in_bits, (OctetString *) &(plain_keyinfo.subjectkey), SEC_END, decryption_key) < 0) {

		AUX_ADD_ERROR;
		return (-1);
	}
	plain_keyinfo.subjectkey.nbits *= 8;

	plainkey_algenc = aux_ObjId2AlgEnc(plain_keyinfo.subjectAI->objid);
	if (plainkey_algenc == RSA) {
		zw = plain_keyinfo.subjectkey.bits;
		kb = d_KeyBits(&(plain_keyinfo.subjectkey));
		if(!kb) {
			free(zw);
			AUX_ADD_ERROR;
			return (-1);
		}

		if (e2_KeyBits(kb, &(plain_keyinfo.subjectkey)) < 0) {
			free(zw);
			aux_free_KeyBits(&kb);
			AUX_ADD_ERROR;
			return (-1);
		}
		free(zw);
		aux_free_KeyBits(&kb);
	} 
	else if (plainkey_algenc == DES)
		len_plain_key = 8;
	else if (plainkey_algenc == DES3)
		len_plain_key = 16;


	/*
	 * If algorithm of plain_key = DES or DES3 and 
	 *    algorithm of decryption_key = RSA and 
	 *    special block format of decryption key != PKCS_BT_02,
	 * 
	 * it is assumed that the DES key has been extended before encryption
	 *    (In this case the last 8 (16) octets contain the DES (DES3) key)
	 */

	/*
	 * Get algorithm of the decryption key
	 */

#ifdef SCA

	if ((call_secsc = handle_in_SCTSC(decryption_key, FALSE)) == TRUE) {
		/*
		 * Decryption key is a key stored on the SC
		 */
		deckey_algenc = aux_ObjId2AlgEnc(decryption_key->alg->objid);
		deckey_algspecial = aux_ObjId2AlgSpecial(decryption_key->alg->objid);
	}
	else {
#endif
		/*
		 * Decryption key is a key stored in the SW-PSE
		 */
		if (!(decryption_keyinfo = get_keyinfo_from_key(decryption_key))) {
			AUX_ADD_ERROR;
			aux_free2_KeyInfo(&plain_keyinfo);
			return (-1);
		}
		deckey_algenc  = aux_ObjId2AlgEnc(decryption_keyinfo->subjectAI->objid);
		deckey_algspecial = aux_ObjId2AlgSpecial(decryption_keyinfo->subjectAI->objid);

		aux_free_KeyInfo(&decryption_keyinfo);
#ifdef SCA
	}
#endif


	if (((plainkey_algenc == DES) || (plainkey_algenc == DES3)) &&
	     (deckey_algenc == RSA) && (deckey_algspecial != PKCS_BT_02)) {

#ifdef SECSCTEST
		fprintf(stderr, "extended key:\n");
		fprintf(stderr, "plain_keyinfo.subjectkey.nbits: %d\n", plain_keyinfo.subjectkey.nbits);
		aux_fxdump(stderr, plain_keyinfo.subjectkey.bits, plain_keyinfo.subjectkey.nbits / 8, 0);
		fprintf(stderr, "\n");
#endif

		len_extended_key = plain_keyinfo.subjectkey.nbits / 8;
		pos_plain_key = len_extended_key - len_plain_key;
	
		j = pos_plain_key;

		for (i=0; i<len_plain_key && j < len_extended_key;)
			plain_keyinfo.subjectkey.bits[i++] = plain_keyinfo.subjectkey.bits[j++];

	}

	plain_keyinfo.subjectkey.nbits = len_plain_key * 8;

#ifdef SECSCTEST
		fprintf(stderr, "plain_key:\n");
		fprintf(stderr, "plain_keyinfo.subjectkey.nbits: %d\n", plain_keyinfo.subjectkey.nbits);
		aux_fxdump(stderr, plain_keyinfo.subjectkey.bits, plain_keyinfo.subjectkey.nbits / 8, 0);
		fprintf(stderr, "\n");
#endif



	/* store key */
	if (put_keyinfo_according_to_key(&plain_keyinfo, plain_key, (ObjId *)0) < 0) {
		AUX_ADD_ERROR;
		return -1;
	}
	
	return (0);

}


/***************************************************************************************
 *                                 sec_put_key                                         *
 ***************************************************************************************/

/***************************************************************
 *
 * Procedure sec_put_key
 *
 ***************************************************************/
#ifdef __STDC__

KeyRef sec_put_key(
	KeyInfo	 *keyinfo,
	KeyRef	  keyref
)

#else

KeyRef sec_put_key(
	keyinfo,
	keyref
)
KeyInfo	 *keyinfo;
KeyRef	  keyref;

#endif

{
	PSESel         *pse_sel;
	PSEToc         *toc;
	struct PSE_Objects *nxt;
	char           *object, key[32], buf[32];
	int             fd, maxref, ref, rcode;
	OctetString    *encoded_key;


	char           *proc = "sec_put_key";

	pse_sel = set_key_pool(0);

	if (!(object = pse_name(pse_sel->app_name, ""))) {
		AUX_ADD_ERROR;
		if (pse_sel->pin) strzero(pse_sel->pin);
		return (-1);
	}
	if ((fd = open(object, O_RDONLY)) < 0) {	/* check whether
							 * key_pool exists */
		if (errno == ENOENT)
			if (sec_create(pse_sel) < 0) {	/* create one */
				AUX_ADD_ERROR;
				if (pse_sel->pin) strzero(pse_sel->pin);
				if (object) free(object);
				return (-1);
			}
	}
	close(fd);

	pse_sel = set_key_pool(keyref);

	if ((ref = sec_create(pse_sel)) < 0) {	/* create object in key_pool */
		AUX_ADD_ERROR;
		if (pse_sel->pin) strzero(pse_sel->pin);
		return (-1);
	}
	if (keyref < 0)	keyref = ref;
	encoded_key = e_KeyInfo(keyinfo);	/* encode keyinfo */
	if(encoded_key) {
		rcode = sec_write(pse_sel, encoded_key);	/* write it to object */
		if(encoded_key->octets) free(encoded_key->octets);
	}
	else {
		aux_add_error(EENCODE, "can't encode keyinfo", (char *) keyinfo, KeyInfo_n, proc);
		return (-1);
	}
	if (pse_sel->pin) strzero(pse_sel->pin);
	if (rcode < 0) {
		AUX_ADD_ERROR;
		return (-1);
	} 
	else return (keyref);
}


/***************************************************************************************
 *                                     sec_read                                        *
 ***************************************************************************************/

/***************************************************************
 *
 * Procedure sec_read
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_read(
	PSESel		 *pse_sel,
	OctetString	 *content
)

#else

RC sec_read(
	pse_sel,
	content
)
PSESel		 *pse_sel;
OctetString	 *content;

#endif

{
	int             fd, size, len;
	RC              rc;
	char           *proc = "sec_read";

	if (chk_parm(pse_sel)) {
		AUX_ADD_ERROR;
		return (-1);
	}
	if (!pse_sel->object.name || !strlen(pse_sel->object.name)) {
		aux_add_error(EINVALID, "object name missing", (char *) aux_cpy_PSESel(pse_sel), PSESel_n, proc);
		return (-1);
	}
	if (!content) {
		aux_add_error(EINVALID, "content is NULL", (char *) aux_cpy_PSESel(pse_sel), PSESel_n, proc);
		return (-1);
	}
#ifdef SCA
/************************  S C  -  P A R T  *******************************************/

	/*
	 * Check whether SC available and application = SC-application.
	 */

	if ((pse_location = sec_psetest(pse_sel->app_name)) == ERR_in_psetest) {
		AUX_ADD_ERROR;
		return (-1);
	}
	if (pse_location == SCpse) {

		/*
		 * SC available and application = SC application.
		 */

		/* If SC application not open => open it */
		if (handle_SC_app(pse_sel->app_name)) {
			AUX_ADD_ERROR;
			return (-1);
		}

		if (pse_sel->object.name && strlen(pse_sel->object.name)) {

			/*
			 * If object = SC object    => read from WEF on SC
			 */

			if (aux_AppObjName2SCObj(pse_sel->app_name, pse_sel->object.name)) {
				/* read from object on SC */
#ifdef SECSCTEST
				fprintf(stderr, "Call of secsc_read (obj)\n");
#endif
				if (secsc_read(pse_sel, content)) {
					AUX_ADD_ERROR;
					return (-1);
				}
				return (0);	/* read from SC-object
						 * successful */
			} 
		}

		/* 
		 *
		 *  An application | object on the SW-PSE shall be read!
		 *
		 *  => Get the PIN for the SW-PSE from the SC.
		 */
		
		strrep(&(pse_sel->pin), get_pse_pin_from_SC(pse_sel->app_name));
		if(!pse_sel->pin) {
			aux_add_error(EPSEPIN, "Can't get PIN for SW-PSE extension from SC", CNULL, 0, proc);
			return (-1);
		}
		if(pse_sel->object.name && strlen(pse_sel->object.name)) {
			strrep(&(pse_sel->object.pin), pse_sel->pin);
		}

	}			/* if (SC available && app = SC-app) */
	/**************************************************************************************/

	/*
	 * The following is only performed, 
	 * if the object to be read is an object on the SW-PSE.
	 * 
	 * --------------------------------------------------------------
	 * If the SC is available and an application on the SC could be 
	 * opened, the PIN read from the SC is used as PIN for the SW-PSE 
	 * application/object.
	 */
#endif				/* SCA */




	if ((fd = open_object(pse_sel, O_RDONLY)) < 0) {
		AUX_ADD_ERROR;
		return (-1);
	}
	rc = read_object(pse_sel, fd, content);
	if (rc)	{
		if(LASTERROR != EDECRYPTION) aux_add_error(LASTERROR, LASTTEXT, (char *) pse_sel->object.name, char_n, proc);
		else aux_add_error(EPIN, "can't read object because PIN is wrong", (char *) pse_sel->object.name, char_n, proc);
	}
	return (rc);
}


/***************************************************************************************
 *                                     sec_read_PSE                                    *
 ***************************************************************************************/

/***************************************************************
 *
 * Procedure sec_read_PSE
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_read_PSE(
	PSESel		 *pse_sel,
	ObjId		 *type,
	OctetString	 *value
)

#else

RC sec_read_PSE(
	pse_sel,
	type,
	value
)
PSESel		 *pse_sel;
ObjId		 *type;
OctetString	 *value;

#endif

{
	OctetString     content;
	OctetString    *result;

	char           *proc = "sec_read_PSE";

	if (!value || !type) {
		aux_add_error(EINVALID, "value or type = 0", CNULL, 0, proc);
		return (-1);
	}
	if (sec_read(pse_sel, &content) < 0) {
		AUX_ADD_ERROR;
		return (-1);
	}
	if ((result = d_PSEObject(type, &content)) == (OctetString *) 0) {
		aux_add_error(EDECODE, "Can't decode PSEObject", (char *)pse_sel, PSESel_n, proc);
		free(content.octets);
		return (-1);
	}
	free(content.octets);

	value->noctets = result->noctets;
	value->octets = result->octets;

	return (0);
}



/********************************************************************************
 *                              sec_write_toc
 *******************************************************************************/

/*
 *      sec_write_toc(pse_sel, toc) writes the toc specified by "toc" onto the PSE 
 *      specified by "pse_sel".
 *      It returns -1 in case of errors, 0 otherwise.
 */

/***************************************************************
 *
 * Procedure sec_write_toc
 *
 ***************************************************************/
#ifdef __STDC__

int sec_write_toc(
	PSESel	 *pse_sel,
	PSEToc	 *toc
)

#else

int sec_write_toc(
	pse_sel,
	toc
)
PSESel	 *pse_sel;
PSEToc	 *toc;

#endif

{
	int              ret;

	char           * proc = "sec_write_toc";

	if (! pse_sel || ! toc) {
		aux_add_error(EINVALID, "No pse_sel or toc parameter provided", CNULL, 0, proc);
		return (- 1);
	}

	ret = write_toc(pse_sel, toc, O_WRONLY | O_TRUNC);
	if(ret < 0) AUX_ADD_ERROR;

	return (ret);
}



/*******************************************************************************
 *                                    sec_psetest                              *
 *									       *
 *  Returns the location of the PSE: SWpse, SCpse			       *
 *									       *
 *									       *
 *  1) Perform SC configuration (read SC configuration file (".scinit")).      *
 *  									       *
 *  2) Check whether the application is an SC-application		       *
 *									       *
 *  3) In case of an SC application or if SC_verify/SC_encrypt is set to       *
 *     verification/encryption within the SCT, it is checked whether the SCT   *
 *     is available.							       *
 *									       *
 *									       *
 *  Return values:							       *
 *  SCpse:							               *
 *        - SCINITFILE is available and					       *
 *	  - Application is an SC-application and 			       *
 *        - SCT is available 	       					       *
 *									       *
 *  SWpse:						      		       *
 *        -  No SCINITFILE available 	or				       *
 *  	  -  Application is not an SC-application 	                       *
 *									       *
 *  ERR_in_psetest:							       *
 *        - error in SC configuration file  or   			       *
 *	  - error during SCT configuration 				       *
 *          (e.g.device unknown or device busy)  or	 	       	       *
 *        - SC_verify and/or SC_encrypt is/are set to TRUE, but the SCT is     *
 *          not available or						       *
 *	  - the application is an application on the SC, but the SCT is not    *
 *          available						               *
 *									       *
 *******************************************************************************/

/***************************************************************
 *
 * Procedure sec_psetest
 *
 ***************************************************************/
#ifdef __STDC__

PSELocation sec_psetest(
	char	 *app_name
)

#else

PSELocation sec_psetest(
	app_name
)
char	 *app_name;

#endif

{
#ifdef SCA

	int		SCT_available;
	int		SC_available;
	Boolean		SCapp = FALSE;
	SCAppEntry     *sc_app_entry;

	char           *proc = "sec_psetest";



	if (!app_name) {
		aux_add_error(EINVALID, "Application name missing", CNULL, 0, proc);
		return (ERR_in_psetest);
	}



	if (SC_ignore == TRUE)
		return (SWpse);



	/*
	 * Perform SC configuration
	 */

	if ((SC_available = SC_configuration()) == -1) {
		AUX_ADD_ERROR;
		return (ERR_in_psetest);
	}
	if (SC_available == FALSE)

		/*
		 * SC is not available (no SC configuration file found)
		 */
		return (SWpse);



	/*
	 *  Intermediate result :  SC configuration was successful
	 *  Next to do:		   Check whether app is an SC-application
	 */

	if (sc_app_entry = aux_AppName2SCApp(app_name)) {

		/*
		 * application to be tested is an SC-application.
		 */
		SCapp = TRUE;
	}



	/*
	 * If an SCT is required, check whether the SCT (sc_sel.sct_id) is available 
	 */

	if ((SC_verify == TRUE) || (SC_encrypt == TRUE) || (SCapp == TRUE)) {

		if ((SCT_available = SCT_configuration ()) == -1) {
			AUX_ADD_ERROR;
			return (ERR_in_psetest);
		}

		/*
		 * SCT is required, but not available
		 */
		if (SCT_available == FALSE) {
			if ((SC_verify == TRUE) || (SC_encrypt == TRUE)) 
				aux_add_error(ECONFIG, "Global variable SC_encrypt/SC_verify (SCINITFILE) is set to encryption/verification in the SCT. SCT is not available.", CNULL, 0, proc);
			if (SCapp == TRUE) 
				aux_add_error(ECONFIG, "Application is an application on the SC, but SCT is not available.", app_name, char_n, proc);
			return (ERR_in_psetest);
		}

	}

	if (SCapp == TRUE) SC_ignore_SWPSE = sc_app_entry->ignore_flag;


	/*
	 *  No error occured, return result
	 */

	if (SCapp == FALSE)
		return (SWpse);
	else 	return (SCpse);


#else
	return (SWpse);
#endif				/* SCA */

}				/* sec_psetest */




/*******************************************************************************
 *                                    sec_scttest                              *
 *									       *
 *  Check whether SCT is available:					       *
 *									       *
 *     - check whether selected SCT (sc_sel.sct_id) is available. This is done *
 *       internally by calling the STARMOD function "sca_display". 	       *
 *       If the actual process hasn't yet opened the device for the selected   *
 *       SCT, this is done automatically by the STARMOD modul. If open fails   *
 *       (device unknown or device busy), "sec_scttest()" returns 	       *
 *	 ERR_in_scttest.						       *
 *									       *
 *									       *
 *  Return values:     SCTDev_avail, SCTDev_not_avail, ERR_in_scttest          *
 *									       *
 *******************************************************************************/

/***************************************************************
 *
 * Procedure sec_scttest
 *
 ***************************************************************/
#ifdef __STDC__

SCTDevice sec_scttest(
)

#else

SCTDevice sec_scttest(
)

#endif

{
#ifdef SCA

	int		SC_available;
	int		SCT_available;

	char           *proc = "sec_scttest";



	if ((SC_available = SC_configuration()) == -1) {
		AUX_ADD_ERROR;
		return (ERR_in_scttest);
	}


	if ((SCT_available = SCT_configuration()) < 0) {
		AUX_ADD_ERROR;
		return (ERR_in_scttest);
	}

	if (SCT_available == TRUE)
		return (SCTDev_avail);
	else 	return (SCTDev_not_avail);

#else
	return (SCTDev_not_avail);
#endif				/* SCA */

}				/* sec_scttest */





/*******************************************************************************
 *                                    sec_pse_config                           *
 *									       *
 *									       *
 *									       *
 * 1) Perform SC configuration (read SC configuration file (".scinit")).       *
 *  									       *
 *									       *
 * 2) Check type of pse:						       *
 *									       *
 *    a) Check type of application (pse_sel->object == CNULL):		       *
 *									       *
 *	 Check whether application (pse_sel->app_name) is an SC-application.   *
 *       Possible return values: NOT_ON_SC, APP_ON_SC			       *
 *									       *
 *									       *
 *    b) Check type of object  (pse_sel->object != CNULL):		       *
 *									       *
 *       Check whether application and object are objects on the SC. If object *
 *	 is an object on the SC, it is returned whether this object is a key   *
 *	 or a file on the SC.						       *
 *       Possible return values: NOT_ON_SC, FILE_ON_SC, KEY_ON_SC	       *
 *									       *
 *									       *
 * In case of an error ERR_in_pseconfig will be returned.		       *
 *									       *
 *******************************************************************************/

/***************************************************************
 *
 * Procedure sec_pse_config
 *
 ***************************************************************/
#ifdef __STDC__

PSEConfig sec_pse_config(
	PSESel	 *pse_sel
)

#else

PSEConfig sec_pse_config(
	pse_sel
)
PSESel	 *pse_sel;

#endif

{
#ifdef SCA

	int		SC_available;
	SCAppEntry     *sc_app_entry;
	SCObjEntry     *sc_obj_entry;

	char           *proc = "sec_pse_config";


	if (chk_parm(pse_sel)) {
		AUX_ADD_ERROR;
		return (ERR_in_pseconfig);
	}
	

	/*
	 * Perform SC configuration
	 */

	if ((SC_available = SC_configuration()) == -1) {
		AUX_ADD_ERROR;
		return (ERR_in_pseconfig);
	}
	if (SC_available == FALSE)

		/*
		 * SC is not available (no SC configuration file found)
		 */
		return (NOT_ON_SC);



	/*
	 *  Intermediate result :  SC configuration was successful => SC is available
         *  Next to do:		   Check whether Application and/or object 
         *			   are supposed to be on the SC.
	 */
	

	if (pse_sel->object.name && strlen(pse_sel->object.name)) {

		/*
		 * Check type of object
		 */

		sc_obj_entry = aux_AppObjName2SCObj(pse_sel->app_name, pse_sel->object.name);

		if (sc_obj_entry == (SCObjEntry * ) 0) 	        
			return (NOT_ON_SC);

		else if (sc_obj_entry->type == SC_KEY_TYPE) 		
			return (KEY_ON_SC);
		     else    					
			return (FILE_ON_SC);

	}
	else {

		/*
		 * Check type of application
		 */

	        sc_app_entry = aux_AppName2SCApp(pse_sel->app_name);

		if (sc_app_entry == (SCAppEntry * ) 0) 	
			return (NOT_ON_SC);
		else  	return (APP_ON_SC);

	}


#else
	return (NOT_ON_SC);
#endif				/* SCA */

}				/* sec_pse_config */





/*******************************************************************************
 *                                    sec_set_sct                              *
 *******************************************************************************/
/***************************************************************
 *
 * Procedure sec_set_sct
 *
 ***************************************************************/
#ifdef __STDC__

void sec_set_sct(
	int	  sct_id
)

#else

void sec_set_sct(
	sct_id
)
int	  sct_id;

#endif

{
#ifdef SCA
	sc_sel.sct_id = sct_id;
#endif
}


/*******************************************************************************
 *                                    sec_sign                                 *
 *******************************************************************************/

/***************************************************************
 *
 * Procedure sec_sign
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_sign(
	OctetString	 *in_octets,
	Signature	 *signature,
	More		  more,
	Key		 *key,
	HashInput	 *hash_input
)

#else

RC sec_sign(
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
	static AlgEnc   algenc;
	static AlgHash  alghash;
	static AlgSpecial  algspecial;
	static OctetString *hash_result;
	OctetString     *rsa_input, *encodedDigest;
	AlgType         algtype;
	AlgEnc          keyalgenc;
	int             rc;

	char           *proc = "sec_sign";

	public_modulus_length = 0;

#ifdef SCA
/************************  S C  -  P A R T  *******************************************/


	/*
	 * Check whether signing shall be done within the SCT/SC - If
	 * the key is selected with object name, the PIN for the
	 * SW-PSE is read from the SC.
	 */

	if ((call_secsc = handle_in_SCTSC(key, FALSE)) == -1) {
		AUX_ADD_ERROR;
		return (-1);
	}
	if (call_secsc == TRUE) {

		/* sign with key from the SC */
#ifdef SECSCTEST
		fprintf(stderr, "Call of secsc_sign\n");
#endif
		if (secsc_sign(in_octets, signature, more, key, hash_input)) {
			AUX_ADD_ERROR;
			return (-1);
		}
		return (0);	/* Generation of signature was
					 * successful! */

	}		/* if (call_secsc == TRUE) */
	/**************************************************************************************/

	/*
	 * The following is only performed, if - the SC is not available or -
	 * signature key doesn't address a key within SCT/SC
	 * 
	 * 
	 * --------------------------------------------------------------
	 * If the SC is available and an application on the SC could be 
	 * opened, the PIN read from the SC is used as PIN for the SW-PSE 
	 * application/object.
	 */
#endif				/* SCA */



	if (sec_state == F_null) {
		if(sec_time) {
			hash_sec  = hash_usec = rsa_sec = rsa_usec = dsa_sec = dsa_usec = 0;
		}
		if (get2_keyinfo_from_key(&got_key, key) < 0) {
			AUX_ADD_ERROR;
			return -1;
		}
		keyalgenc = aux_ObjId2AlgEnc(got_key.subjectAI->objid);
		if(key->alg) {
			if (!signature->signAI) { /* for compatibility with older SecuDE versions, accept
						     the sign algorithm also from parameter signature */
				signature->signAI = aux_cpy_AlgId(key->alg);
			}
		}
			
		if ((signature->signAI == NULLALGID) || (signature->signAI->objid == NULLOBJID)) {
			/* default signature AI = md5WithRsaEncryption or dsaWithSHA 
			   depending of keyalgenc  */
			if(keyalgenc == RSA) signature->signAI = aux_cpy_AlgId(md5WithRsaEncryption);
			if(keyalgenc == DSA) signature->signAI = aux_cpy_AlgId(dsaWithSHA);
		}
		algenc = aux_ObjId2AlgEnc(signature->signAI->objid);
		alghash = aux_ObjId2AlgHash(signature->signAI->objid);
		algspecial = aux_ObjId2AlgSpecial(signature->signAI->objid);
		algtype = aux_ObjId2AlgType(signature->signAI->objid);
		if (algtype != SIG) {
			aux_add_error(EINVALID, "wrong signAI in signature", (char *) signature->signAI, AlgId_n, proc);
			return -1;
		}

		/* check required encryption method against that of the secret key */

		if (algenc != keyalgenc) {
			aux_add_error(EINVALID, "required algorithm doesn't fit to secret key", (char *) got_key.subjectAI, AlgId_n, proc);
			return -1;
		}
		if(algenc == RSA) public_modulus_length = RSA_PARM(got_key.subjectAI->param);
		if(algenc == DSA) {
			if(got_key.subjectAI->param) public_modulus_length = *(dsaSK_parm_type *)(got_key.subjectAI->param);
			else public_modulus_length = 320;
		} 
		if (!(hash_result = (OctetString *) malloc(sizeof(OctetString)))) {
			aux_add_error(EMALLOC, "hash_result", CNULL, 0, proc);
			return (-1);
		}
		hash_result->noctets = 0;
		switch (alghash) {
		case SQMODN:
			rc = rsa_set_key((char *)&hash_input->sqmodn_input, 1);
			if (rc != 0) {
				aux_add_error(EINVALID, "Can't set key for sqmodn", CNULL, 0, proc);
				return -1;
			}
			if (!(hash_result->octets = malloc((public_modulus_length + 7) / 8))) {
				aux_add_error(EMALLOC, "hash_result->octets", CNULL, 0, proc);
				return (-1);
			}
			break;
		case MD2:
		case MD4:
		case MD5:
		case SHA:
			if (!(hash_result->octets = malloc(64))) {
				aux_add_error(EMALLOC, "hash_result->octets", CNULL, 0, proc);
				return (-1);
			}
			break;
		}

		signature->signature.nbits = 0;
		if (!(signature->signature.bits = malloc(public_modulus_length / 8 + 16))) {
			aux_add_error(EMALLOC, "signature->signature.bits", CNULL, 0, proc);
			return (-1);
		}
		sec_state = F_sign;
	}

	if(sec_time) gettimeofday(&sec_tp1, &sec_tzp1);

	switch (alghash) {
	case SQMODN:
		rc = hash_sqmodn(in_octets, hash_result, more, public_modulus_length);
		break;
	case MD2:
		rc = md2_hash(in_octets, hash_result, more);
		break;
	case MD4:
		rc = md4_hash(in_octets, hash_result, more);
		break;
	case MD5:
		rc = md5_hash(in_octets, hash_result, more);
		break;
	case SHA:
		rc = sha_hash(in_octets, hash_result, more);
		break;
	default:
		aux_add_error(EALGID, "invalid alg_id for hash", CNULL, 0, proc);
		algenc = NoAlgEnc;
		alghash = NoAlgHash;
		aux_free_OctetString(&hash_result);
		aux_free2_KeyInfo(&got_key);
		sec_state = F_null;
		return -1;
	}

	if(sec_time) {
		gettimeofday(&sec_tp2, &sec_tzp2);
		hash_usec = (hash_sec + (sec_tp2.tv_sec - sec_tp1.tv_sec)) * 1000000 + 	hash_usec + (sec_tp2.tv_usec - sec_tp1.tv_usec);
		hash_sec = hash_usec/1000000;
		hash_usec = hash_usec % 1000000;
	}

	if (more == SEC_END) {
		if (rc == 0) {
			switch (algenc) {
			case RSA:

				if(sec_time) gettimeofday(&sec_tp1, &sec_tzp1);

				if(algspecial == PKCS_BT_01 || algspecial == PKCS_BT_TD) {

					/*  Here goes PKCS#1 ...   */

					encodedDigest = aux_create_PKCS_MIC_D(hash_result, signature->signAI);
					rsa_input = aux_create_PKCSBlock(algspecial, encodedDigest);
					aux_free_OctetString(&hash_result);		
					if(encodedDigest) aux_free_OctetString(&encodedDigest);
          			}
				else rsa_input = hash_result;

				rc = rsa_set_key((char *)&got_key.subjectkey, 1);
				if (rc < 0) aux_add_error(EINVALID, "Can't set rsa key", CNULL, 0, proc);
				else if(!rsa_input) aux_add_error(EINVALID, "aux_create_PKCSBlock failed", CNULL, 0, proc);  
				else {
					rc = rsa_sign(rsa_input, &(signature->signature));
					if (rc < 0) AUX_ADD_ERROR;
				}
				if(sec_time) {
					gettimeofday(&sec_tp2, &sec_tzp2);
					rsa_usec = (rsa_sec + (sec_tp2.tv_sec - sec_tp1.tv_sec)) * 1000000 + rsa_usec + (sec_tp2.tv_usec - sec_tp1.tv_usec);
					rsa_sec = rsa_usec/1000000;
					rsa_usec = rsa_usec % 1000000;
				}

				break;
			case DSA:

				if(sec_time) gettimeofday(&sec_tp1, &sec_tzp1);

				rc = dsa_set_key((char *)&got_key.subjectkey, public_modulus_length);
				if (rc < 0) aux_add_error(EINVALID, "Can't set dsa key", CNULL, 0, proc);
				else {
					rc = dsa_sign(hash_result, &(signature->signature));
					if (rc < 0) AUX_ADD_ERROR;
				}
				if(sec_time) {
					gettimeofday(&sec_tp2, &sec_tzp2);
					dsa_usec = (dsa_sec + (sec_tp2.tv_sec - sec_tp1.tv_sec)) * 1000000 + dsa_usec + (sec_tp2.tv_usec - sec_tp1.tv_usec);
					dsa_sec = dsa_usec/1000000;
					dsa_usec = dsa_usec % 1000000;
				}

				break;
			default: 
			        aux_add_error(EINVALID, "AlgEnc of algorithm wrong", CNULL, 0, proc);
				rc = -1;
				break;	
			}
		}
		else aux_add_error(EHASH, "hash failed", CNULL, 0, proc);
		if(algenc == RSA) aux_free_OctetString(&rsa_input);
		aux_free2_KeyInfo(&got_key);
		algenc = NoAlgEnc;
		alghash = NoAlgHash;
		sec_state = F_null;
	}
	return (rc);
}

/*******************************************************************************
 *                             sec_string_to_key                               *
 *******************************************************************************/

/***************************************************************
 *
 * Procedure sec_string_to_key
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_string_to_key(
	char	 *pin,
	Key	 *des_key
)

#else

RC sec_string_to_key(
	pin,
	des_key
)
char	 *pin;
Key	 *des_key;

#endif

{
	KeyInfo        *key_sec; 
	PSEToc         *toc;
	RC              rcode;
	char           *string_to_key();

	char           *proc = "sec_string_to_key";

	/*
	 * Build KeyInfo
	 */

	if (!(key_sec = (KeyInfo *) calloc(1, sizeof(KeyInfo)))) {
		aux_add_error(EMALLOC, "key_sec", CNULL, 0, proc);
		return (-1);
	}
	key_sec->subjectAI = aux_cpy_AlgId(sec_io_algid);

	/* Generate key from string */

	if (!(key_sec->subjectkey.bits = string_to_key(pin))) {
		aux_add_error(EMALLOC, "key_sec->subjectkey.bits", CNULL, 0, proc);
		aux_free_KeyInfo(&key_sec);
		return (-1);
	}
	key_sec->subjectkey.nbits = 64;

	/*
	 * Return the key in the form requested by des_key
	 */
	rcode = put_keyinfo_according_to_key(key_sec, des_key, (ObjId *)0);
	aux_free_KeyInfo(&key_sec);
	if (rcode) AUX_ADD_ERROR;
	return (rcode);
}


/***************************************************************************************
 *                                     sec_verify                                      *
 ***************************************************************************************/

/***************************************************************
 *
 * Procedure sec_verify
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_verify(
	OctetString	 *in_octets,
	Signature	 *signature,
	More		  more,
	Key		 *key,
	HashInput	 *hash_input
)

#else

RC sec_verify(
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

	static OctetString *hash_result;
	static AlgEnc   algenc;
	static AlgHash  alghash;
	static AlgSpecial     algspecial;
	static int      keysize;
	OctetString     *rsa_input, *encodedDigest;
	BitString       *b1;
	OctetString     *o1;
	AlgType         algtype;
	AlgEnc          keyalgenc;
	int             rc, i;

	char           *proc = "sec_verify";

#ifdef SCA
/************************  S C  -  P A R T  *******************************************/


	/*
	 * Check whether verification shall be done within the
	 * SCT/SC - If the key is selected with object name,
	 * the PIN for the SW-PSE is read from the SC.
	 */

	if ((call_secsc = handle_in_SCTSC(key, SC_verify)) == -1) {
		AUX_ADD_ERROR;
		return (-1);
	}

	if (call_secsc == TRUE) {

		/* verify within SCT/SC */

#ifdef SECSCTEST
		fprintf(stderr, "Call of secsc_verify(key)\n");
#endif
		if (secsc_verify(in_octets, signature, more, key, hash_input) < 0) {
			AUX_ADD_ERROR;
			return (-1);
		}
		return (0);	/* Verification positive! */

	}			/* end if (SC available && key->key) */
	/**************************************************************************************/

	/*
	 * The following is called, if : - global variable "SC_verify" is
	 * FALSE or - the SC is not available or - the key is not delivered
	 * by the calling routine
	 */
#endif				/* SCA */



	if (sec_state == F_null) {
		if(sec_time) {
			hash_sec  = hash_usec = rsa_sec = rsa_usec = dsa_sec = dsa_usec = 0;
		}
		if (get2_keyinfo_from_key(&got_key, key) < 0) {
			AUX_ADD_ERROR;
			return -1;
		}
		i = sec_get_keysize(&got_key);
		public_modulus_length = i;
		sec_SignatureTimeDate = (UTCTime *)0;
		keyalgenc = aux_ObjId2AlgEnc(got_key.subjectAI->objid);
		if ((signature->signAI == NULLALGID) || (signature->signAI->objid == NULLOBJID)) {
			if(keyalgenc == RSA) signature->signAI = aux_cpy_AlgId(md5WithRsaEncryption);
			if(keyalgenc == DSA) signature->signAI = aux_cpy_AlgId(dsaWithSHA);
		}
		algenc = aux_ObjId2AlgEnc(signature->signAI->objid);
		alghash = aux_ObjId2AlgHash(signature->signAI->objid);
		algtype = aux_ObjId2AlgType(signature->signAI->objid);
		algspecial = aux_ObjId2AlgSpecial(signature->signAI->objid);
		if (algtype != SIG) {
			aux_add_error(EINVALID, "wrong signAI in signature", (char *) signature->signAI, AlgId_n, proc);
			return -1;
		}
		/* check encryption method of signature key */
		if (algenc != keyalgenc) {
			aux_add_error(EINVALID, "alg of signature doesn't fit to verification key", (char *) got_key.subjectAI, AlgId_n, proc);
			return -1;
		}
		if (!(hash_result = (OctetString *) malloc(sizeof(OctetString)))) {
			aux_add_error(EMALLOC, "hash_result", CNULL, 0, proc);
			return (-1);
		}
		hash_result->noctets = 0;
		switch (alghash) {
		case SQMODN:
			keysize = public_modulus_length;
			rc = rsa_set_key((char *)&hash_input->sqmodn_input, 1);
			if (rc != 0) {
				aux_add_error(EINVALID, "Can't set key for sqmodn", CNULL, 0, proc);
				return -1;
			}
			if (!(hash_result->octets = malloc((public_modulus_length + 7) / 8))) {
				aux_add_error(EMALLOC, "hash_result->octets", CNULL, 0, proc);
				return (-1);
			}
			break;
		case MD2:
		case MD4:
		case MD5:
		case SHA:
			if (!(hash_result->octets = malloc(64))) {
				aux_add_error(EMALLOC, "hash_result->octets", CNULL, 0, proc);
				return (-1);
			}
			break;
		}
		sec_state = F_verify;
	}

	if(sec_time) gettimeofday(&sec_tp1, &sec_tzp1);
	switch (alghash) {
	case SQMODN:
		rc = hash_sqmodn(in_octets, hash_result, more, keysize);
		break;
	case MD2:
		rc = md2_hash(in_octets, hash_result, more);
		break;
	case MD4:
		rc = md4_hash(in_octets, hash_result, more);
		break;
	case MD5:
		rc = md5_hash(in_octets, hash_result, more);
		break;
	case SHA:
		rc = sha_hash(in_octets, hash_result, more);
		break;
	default:
		aux_add_error(EALGID, "invalid alg_id", CNULL, 0, proc);
		algenc = NoAlgEnc;
		alghash = NoAlgHash;
		aux_free_OctetString(&hash_result);
		aux_free2_KeyInfo(&got_key);
		sec_state = F_null;
		return -1;
	}

	if(sec_time) {
		gettimeofday(&sec_tp2, &sec_tzp2);
		hash_usec = (hash_sec + (sec_tp2.tv_sec - sec_tp1.tv_sec)) * 1000000 + 	hash_usec + (sec_tp2.tv_usec - sec_tp1.tv_usec);
		hash_sec = hash_usec/1000000;
		hash_usec = hash_usec % 1000000;
	}

	if(sec_verbose) {
		fprintf(stderr, "Input to sec_verify:\n");
		aux_fprint_OctetString(stderr, in_octets);
		fprintf(stderr, "Hash value of input to sec_verify:\n");
		aux_fprint_OctetString(stderr, hash_result);
	}

	if (more == SEC_END) {

		if (rc == 0) {
			switch (algenc) {
			case RSA:
				if(sec_time) gettimeofday(&sec_tp1, &sec_tzp1);
				if(algspecial == PKCS_BT_01 || algspecial == PKCS_BT_TD) {

					/*  Here goes PKCS#1 ...   */

					encodedDigest = aux_create_PKCS_MIC_D(hash_result, signature->signAI);
					rsa_input = aux_create_PKCSBlock(PKCS_BT_01, encodedDigest);
					if(algspecial == PKCS_BT_TD) {
						o1 = (OctetString *)malloc(sizeof(OctetString));
						o1->noctets = 0;
						o1->octets = malloc(256);
						rc = rsa_set_key((char *)&got_key.subjectkey, 1);
						if (rc < 0) aux_add_error(EINVALID, "rsa_set_key failed", CNULL, 0, proc);
						rsa_encblock2OctetString(&(signature->signature), o1);
						for(i = 1; i < 20; i++) {
							if((unsigned char)o1->octets[i] == 255) {
								o1->octets[i] = '\0';
								sec_SignatureTimeDate = (UTCTime *)aux_cpy_String(&(o1->octets[1]));
								break;
							}
							rsa_input->octets[i] = o1->octets[i];
						}
						rsa_input->octets[0] = PKCS_BT_TD;
						aux_free_OctetString(&o1);
					}
					aux_free_OctetString(&hash_result);		
					if(encodedDigest) aux_free_OctetString(&encodedDigest);
          			}
				else rsa_input = hash_result;

				if(sec_verbose) {
					fprintf(stderr, "RSA block generated from hash-value:\n");
					aux_fprint_OctetString(stderr, rsa_input);
					fprintf(stderr, "Signature value (input to sec_verify):\n");
					aux_fprint_BitString(stderr, &(signature->signature));
					fprintf(stderr, "Public Key for RSA encryption of the signature:\n");
					print_keyinfo_flag |= (PK | KEYBITS);
					aux_fprint_KeyInfo(stderr, &got_key);
					fprintf(stderr, "RSA-encrypted signature (must be equal to the RSA block above):\n");
				}

				rc = rsa_set_key((char *)&got_key.subjectkey, 1);
				if (rc < 0) aux_add_error(EINVALID, "rsa_set_key failed", CNULL, 0, proc);
				else if(!rsa_input) aux_add_error(EINVALID, "aux_create_PKCSBlock failed", CNULL, 0, proc);  
				else {
					rc = rsa_verify(rsa_input, &(signature->signature));
					if (rc < 0) aux_add_error(EVERIFICATION, "verification failed", CNULL, 0, proc);
				}
				if(sec_time) {
					gettimeofday(&sec_tp2, &sec_tzp2);
					rsa_usec = (rsa_sec + (sec_tp2.tv_sec - sec_tp1.tv_sec)) * 1000000 + rsa_usec + (sec_tp2.tv_usec - sec_tp1.tv_usec);
					rsa_sec = rsa_usec/1000000;
					rsa_usec = rsa_usec % 1000000;
				}

				break;
			case DSA:
				if(sec_time) gettimeofday(&sec_tp1, &sec_tzp1);

				if(sec_verbose) {
					fprintf(stderr, "DSA input block:\n");
					aux_fprint_OctetString(stderr, hash_result);
					fprintf(stderr, "Signature value:\n");
					aux_fprint_BitString(stderr, &(signature->signature));
				}

				rc = dsa_set_key((char *)&got_key.subjectkey, public_modulus_length);
				if (rc < 0) aux_add_error(EINVALID, "dsa_set_key failed", CNULL, 0, proc);
				else {
					rc = dsa_verify(hash_result, &(signature->signature));
					if (rc < 0) aux_add_error(EVERIFICATION, "verification failed", CNULL, 0, proc);
				}
				if(sec_time) {
					gettimeofday(&sec_tp2, &sec_tzp2);
					dsa_usec = (dsa_sec + (sec_tp2.tv_sec - sec_tp1.tv_sec)) * 1000000 + dsa_usec + (sec_tp2.tv_usec - sec_tp1.tv_usec);
					dsa_sec = dsa_usec/1000000;
					dsa_usec = dsa_usec % 1000000;
				}

				break;
			default: 
			        aux_add_error(EINVALID, "AlgEnc of algorithm wrong", CNULL, 0, proc);
				rc = -1;
				break;	
			}
		}
		else aux_add_error(EHASH, "hash failed", CNULL, 0, proc);

		if(algenc == RSA) aux_free_OctetString(&rsa_input);
		aux_free2_KeyInfo(&got_key);
		algenc = NoAlgEnc;
		alghash = NoAlgHash;
		sec_state = F_null;
	}
	return (rc);
}


/****************************************************************************************
 *                                     sec_checkSK                                      *
 *	sec_checkSK does an encryption and decryption on a random data block		*
 *	and returns 0 for success, if the process was the identity; -1 otherwise	*
 ***************************************************************************************/

/***************************************************************
 *
 * Procedure sec_checkSK
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_checkSK(
	Key	 *sk,
	KeyInfo	 *pkinfo
)

#else

RC sec_checkSK(
	sk,
	pkinfo
)
Key	 *sk;
KeyInfo	 *pkinfo;

#endif

{
	OctetString    *rand_octs;
	Signature       sign_signature;
	HashInput      *hashinput;
	Key             pk;
	int             rc;

	char           *proc = "sec_checkSK";

	if (!sk || !pkinfo)
		goto checkerr;

	if ((aux_ObjId2AlgEnc(pkinfo->subjectAI->objid) != RSA) && (aux_ObjId2AlgEnc(pkinfo->subjectAI->objid) != DSA)) {
		aux_add_error(EALGID, "invalid alg_id", (char *) pkinfo->subjectAI, AlgId_n, proc);
		return -1;
	}
	/* get some random data of minimum keysize */
	rand_octs = sec_random_ostr(sizeof(int));
	if (!rand_octs) {
		AUX_ADD_ERROR;
		return -1;
	}

	pk.key = pkinfo;
	pk.keyref = 0;
	pk.pse_sel = (PSESel * ) 0;
	pk.alg = (AlgId *)0;

	if (aux_ObjId2AlgHash(pkinfo->subjectAI->objid) == SQMODN)
		hashinput = (HashInput *) & (pkinfo->subjectkey);
	else hashinput = (HashInput *)0;

	sign_signature.signAI = (AlgId *)0;

	if (sec_sign(rand_octs, &sign_signature, SEC_END, sk, hashinput) < 0) {
		aux_free2_Signature(&sign_signature);
		AUX_ADD_ERROR;
		return(-1);
	}

	rc = sec_verify(rand_octs, &sign_signature, SEC_END, &pk, hashinput);
	if(rc < 0){
		aux_free2_Signature(&sign_signature);
		AUX_ADD_ERROR;
		return(-1);
	}

	return (rc);

checkerr:
	aux_add_error(EINVALID, "SK or PKInfo is zero", CNULL, 0, proc);
	return -1;
}



#ifdef SCA
/***************************************************************************************
 *                                sec_unblock_SCpin                                    *
 ***************************************************************************************/

/***************************************************************
 *
 * Procedure sec_unblock_SCpin
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_unblock_SCpin(
	PSESel	 *pse_sel
)

#else

RC sec_unblock_SCpin(
	pse_sel
)
PSESel	 *pse_sel;

#endif

{
	char           *proc = "sec_unblock_SCpin";

	if (chk_parm(pse_sel)) {
		AUX_ADD_ERROR;
		return (-1);
	}

	/*
	 * Check whether SC available and application = SC-application.
	 */

	if ((pse_location = sec_psetest(pse_sel->app_name)) == ERR_in_psetest) {
		AUX_ADD_ERROR;
		return (-1);
	}
	if (pse_location == SCpse) {

		/*
		 * SC available and application = SC application.
		 */

#ifdef SECSCTEST
		fprintf(stderr, "Call of secsc_unblock_SCpin \n");
#endif
		if (secsc_unblock_SCpin(pse_sel)) {
			AUX_ADD_ERROR;
			return (-1);
		}

		return (0);	/* unblocking PIN on SC was successful */

	}
	 /* if (SC available && app = SC-app) */ 
	else {
		aux_add_error(EAPPNAME, "Can't select application on SC", pse_sel->app_name, char_n, proc);
		return (-1);
	}

}				/* sec_unblock_SCpin */

#endif				/* SCA */



/***************************************************************************************
 *                                     sec_write                                       *
 ***************************************************************************************/

/***************************************************************
 *
 * Procedure sec_write
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_write(
	PSESel		 *pse_sel,
	OctetString	 *content
)

#else

RC sec_write(
	pse_sel,
	content
)
PSESel		 *pse_sel;
OctetString	 *content;

#endif

{
	int             fd, len, ret;
	char           *object;
	struct PSE_Objects *nxt;

	char           *proc = "sec_write";

	if (chk_parm(pse_sel)) {
		AUX_ADD_ERROR;
		return (-1);
	}
	if (!pse_sel->object.name || !strlen(pse_sel->object.name)) {
		aux_add_error(EINVALID, "Obj name missing", CNULL, 0, proc);
		return (-1);
	}
	if (!content) {
		aux_add_error(EINVALID, "content is NULL", CNULL, 0, proc);
		return (-1);
	}
#ifdef SCA
/************************  S C  -  P A R T  *******************************************/

	/*
	 * Check whether SC available and application = SC-application.
	 */

	if ((pse_location = sec_psetest(pse_sel->app_name)) == ERR_in_psetest) {
		AUX_ADD_ERROR;
		return (-1);
	}
	if (pse_location == SCpse) {

		/*
		 * SC available and application = SC application.
		 */

		/* If SC application not open => open it */
		if (handle_SC_app(pse_sel->app_name)) {
			AUX_ADD_ERROR;
			return (-1);
		}

		if (pse_sel->object.name && strlen(pse_sel->object.name)) {

			/*
			 * If object = SC object    => write into WEF on SC
			 */


			if (aux_AppObjName2SCObj(pse_sel->app_name, pse_sel->object.name)) {
				/* write into object on SC */
#ifdef SECSCTEST
				fprintf(stderr, "Call of secsc_write (obj)\n");
#endif
				if (secsc_write(pse_sel, content)) {
					AUX_ADD_ERROR;
					return (-1);
				}
				/* update modification time in SC toc */
				if (update_SCToc(pse_sel, content->noctets, 0)) {
					AUX_ADD_ERROR;
					return (-1);
				}

				return (0);	/* write into SC-object
						 * successful */
			} 
		}

		/* 
		 *
		 *  An object on the SW-PSE shall be written!
		 *
		 *  => Get the PIN for the SW-PSE from the SC.
		 */
		
		strrep(&(pse_sel->pin), get_pse_pin_from_SC(pse_sel->app_name));
		if(!pse_sel->pin) {
			aux_add_error(EPSEPIN, "Can't get PIN for SW-PSE extension from SC", CNULL, 0, proc);
			return (-1);
		}
		if(pse_sel->object.name && strlen(pse_sel->object.name)) {
			strrep(&(pse_sel->object.pin), pse_sel->pin);
		}

	}			/* if (SC available && app = SC-app) */
	/**************************************************************************************/

	/*
	 * The following is only performed, 
	 * if the object to be written is an object on the SW-PSE.
	 * 
	 * --------------------------------------------------------------
	 * If the SC is available and an application on the SC could be 
	 * opened, the PIN read from the SC is used as PIN for the SW-PSE 
	 * application/object.
	 */
#endif				/* SCA */



	if ((fd = open_object(pse_sel, O_WRONLY | O_TRUNC)) < 0) {
		AUX_ADD_ERROR;
		return (-1);
	}
	/* update time stamp in toc which we have from open_object() */

	nxt = psetoc->obj;
	while (nxt) {
		if (strcmp(nxt->name, pse_sel->object.name) == 0) {

			/* yes */

			if (nxt->update)
				free(nxt->update);
			nxt->update = aux_current_UTCTime();
			nxt->noOctets = content->noctets;
			nxt->status = 0;

			/* Write toc */

			ret = write_toc(pse_sel, psetoc, O_WRONLY | O_TRUNC);
			if (ret < 0) {
				AUX_ADD_ERROR;
				return (-1);
			}
			break;
		}
		nxt = nxt->next;
	}

	/* write object */


	if (!(object = (char *) malloc(content->noctets))) {
		aux_add_error(EMALLOC, "object", CNULL, 0, proc);
		close_enc(fd);
		return (-1);
	}
	bcopy(content->octets, object, content->noctets);
	if (write_enc(fd, object, content->noctets, pse_sel->object.pin) < 0) {
		sprintf(text, "can't write %s", pse_sel->object.name);
		if(LASTERROR != ESYSTEM) AUX_ADD_ERROR;
		else aux_add_error(ESYSTEM, "can't write object", (char *) pse_sel->object.name, char_n, proc);
		close_enc(fd);
		free(object);
		return (-1);
	}
	close_enc(fd);
	free(object);
	return (0);
}


/***************************************************************************************
 *                                     sec_write_PSE                                   *
 ***************************************************************************************/

/***************************************************************
 *
 * Procedure sec_write_PSE
 *
 ***************************************************************/
#ifdef __STDC__

RC sec_write_PSE(
	PSESel		 *pse_sel,
	ObjId		 *type,
	OctetString	 *value
)

#else

RC sec_write_PSE(
	pse_sel,
	type,
	value
)
PSESel		 *pse_sel;
ObjId		 *type;
OctetString	 *value;

#endif

{
	OctetString    *content;
	ObjId          *af_get_objoid();

	char           *proc = "sec_write_PSE";

	if (!type)
		type = af_get_objoid(pse_sel->object.name);
	if ((content = e_PSEObject(type, value)) == (OctetString *) 0) {
		aux_add_error(EENCODE, "Can't encode PSEObject", CNULL, 0, proc);
		return (-1);
	}
	if (sec_write(pse_sel, content) < 0) {
		AUX_ADD_ERROR;
		return (-1);
	}
	return (0);
}



/***************************************************************************************
 *                                     sec_pin_check                                   *
 ***************************************************************************************/

/***************************************************************
 *
 * Procedure sec_pin_check
 *
 ***************************************************************/
#ifdef __STDC__

Boolean sec_pin_check(
	PSESel	 *pse_sel,
	char	 *obj,
	char	 *pin
)

#else

Boolean sec_pin_check(
	pse_sel,
	obj,
	pin
)
PSESel	 *pse_sel;
char	 *obj;
char	 *pin;

#endif

{
	char           *proc = "sec_pin_check";


	if (!pse_sel || !pin) return(FALSE);

	if (!pin_check(pse_sel, obj, pin, TRUE, FALSE)) return(FALSE);

	return(TRUE);
}










/* ************************ local functions: ********************************* */

/***************************** chk_parm *****************************************/


/***************************************************************
 *
 * Procedure chk_parm
 *
 ***************************************************************/
#ifdef __STDC__

static int chk_parm(
	PSESel	 *pse_sel
)

#else

static int chk_parm(
	pse_sel
)
PSESel	 *pse_sel;

#endif

{
	char           *proc = "chk_parm";

	if (!pse_sel) {
		aux_add_error(EINVALID, "pse_sel is NULL", CNULL, 0, proc);
		return (-1);
	}
	if (!pse_sel->app_name || !strlen(pse_sel->app_name)) {
		aux_add_error(EINVALID, "PSE name missing", (char *) pse_sel, PSESel_n, proc);
		return (-1);
	}
	return (0);
}


/******************************** fsize *****************************************
 *
 *      fsize(fd) returns the size of the file fd points to.
 *
 *******************************************************************************/


/***************************************************************
 *
 * Procedure fsize
 *
 ***************************************************************/
#ifdef __STDC__

static off_t fsize(
	int	  fd
)

#else

static off_t fsize(
	fd
)
int	  fd;

#endif

{
	struct stat     stat;
	char           *proc = "fsize";

	if (fstat(fd, &stat) == 0)
		return (stat.st_size);

	aux_add_error(ESYSTEM, "fstat failed", CNULL, 0, proc);
	return (-1);
}


/********************* get_encodedkeyinfo_from_keyref ***************************
 *
 *  given: keyref
 *  Return Value: OctetString with encoded keyinfo
 *
 *******************************************************************************/


/***************************************************************
 *
 * Procedure get_encodedkeyinfo_from_keyref
 *
 ***************************************************************/
#ifdef __STDC__

static OctetString *get_encodedkeyinfo_from_keyref(
	KeyRef	  keyref
)

#else

static OctetString *get_encodedkeyinfo_from_keyref(
	keyref
)
KeyRef	  keyref;

#endif

{
	static OctetString encoded_key;
	PSESel         *pse_sel;
	char           *proc = "sec_get_encodedkeyinfo_from_keyref";

	if (keyref <= 0) {
		aux_add_error(EINVALID, "invalid keyref ", CNULL, 0, proc);
		return ((OctetString *) 0);
	}
	pse_sel = set_key_pool(keyref);

	if (sec_read(pse_sel, &encoded_key) < 0) {	/* read object */
		AUX_ADD_ERROR;
		if (pse_sel->pin) strzero(pse_sel->pin);
		return ((OctetString *) 0);
	}
	if (pse_sel->pin) strzero(pse_sel->pin);
	return (&encoded_key);
}


/****************************** get_keyinfo_from_key ****************************
 *
 *      get_keyinfo_from_key(key) returns a KeyInfo from Key
 *
 *******************************************************************************/

/***************************************************************
 *
 * Procedure get_keyinfo_from_key
 *
 ***************************************************************/
#ifdef __STDC__

KeyInfo *get_keyinfo_from_key(
	Key	 *key
)

#else

KeyInfo *get_keyinfo_from_key(
	key
)
Key	 *key;

#endif

{
	OctetString     encoded_key;
	KeyInfo        *keyinfo;
	char           *proc = "get_keyinfo_from_key";

	if (!key) {
		aux_add_error(EINVALID, "key missing in get_keyinfo_from_key", CNULL, 0, proc);
		return ((KeyInfo *) 0);
	}
	keyinfo = (KeyInfo *) 0;

	if (key->keyref == 0 && !key->pse_sel)
		keyinfo = aux_cpy_KeyInfo(key->key);
	else {
		if (key->keyref == 0 && key->pse_sel) {
			if (sec_read_PSE(key->pse_sel, &dummy_oid, &encoded_key) >= 0) {
				keyinfo = d_KeyInfo(&encoded_key);
				free(encoded_key.octets);
			}
		} else {
			if (key->keyref > 0)
				keyinfo = get_keyinfo_from_keyref(key->keyref);
		}
	}

	return (keyinfo);
}


/***************************** get2_keyinfo_from_key ****************************
 *
 *    given:  key (and struct of keyinfo)
 *    return: keyinfo
 *    RC:     0 or -1
 *
 *******************************************************************************/

/***************************************************************
 *
 * Procedure get2_keyinfo_from_key
 *
 ***************************************************************/
#ifdef __STDC__

int get2_keyinfo_from_key(
	KeyInfo	 *keyinfo,
	Key	 *key
)

#else

int get2_keyinfo_from_key(
	keyinfo,
	key
)
KeyInfo	 *keyinfo;
Key	 *key;

#endif

{
	OctetString     pse_content;
	char           *proc = "get2_keyinfo_from_key";

	if (!key || !keyinfo) {
		aux_add_error(EINVALID, "key missing in get2_keyinfo_from_key", CNULL, 0, proc);
		return (-1);
	}
	if (key->keyref == 0) {
		if (key->pse_sel == (PSESel *) 0) {
			if (key->key == (KeyInfo *) 0) {
				aux_add_error(EINVALID, "invalid keyinfo ", CNULL, 0, proc);
				return (-1);
			}
			/* key is in key->key: */
			if (aux_cpy2_KeyInfo(keyinfo, key->key) < 0) {
				AUX_ADD_ERROR;
				return (-1);
			}
		} else {	/* key is in PSE-Object: */

			/*
			 * sec_read does sec_open in case of PSE if (
			 * sec_open (key->pse_sel) < 0 ) {
			 * aux_add_error(EINVALID,"can't open
			 * object",(char *) key->pse_sel,PSESel_n,proc); return (-1) ;
			 * }
			 */
			if (sec_read_PSE(key->pse_sel, &dummy_oid, &pse_content) < 0) {
				AUX_ADD_ERROR;
				return (-1);
			}

			/*
			 * if ( sec_close (key->pse_sel) < 0 ) { free
			 * (pse_content.octets) ;
			 * aux_add_error(EINVALID,"can't close
			 * object",(char *) key->pse_sel,PSESel_n,proc); return (-1) ;
			 * }
			 */
			if (d2_KeyInfo(&pse_content, keyinfo) < 0) {
				aux_add_error(EDECODE, "Can't decode keyinfo", (char *)key->pse_sel, PSESel_n, proc);	/* in fact: object is
												 * broken */
				free(pse_content.octets);
				return (-1);
			}
			free(pse_content.octets);
		}
	} else {		/* key is referenced: */
		if (get2_keyinfo_from_keyref(keyinfo, key->keyref) < 0) {
			AUX_ADD_ERROR;
			return (-1);
		}
	}
	return (0);
}


/***************************** get_keyinfo_from_keyref *************************
 *
 *    given:  keyref
 *    Return Value: keyinfo or NULL
 *
 *******************************************************************************/

/***************************************************************
 *
 * Procedure get_keyinfo_from_keyref
 *
 ***************************************************************/
#ifdef __STDC__

KeyInfo *get_keyinfo_from_keyref(
	KeyRef	  keyref
)

#else

KeyInfo *get_keyinfo_from_keyref(
	keyref
)
KeyRef	  keyref;

#endif

{
	OctetString    *encoded_key;
	KeyInfo        *keyinfo;
	char           *proc = "get_keyinfo_from_keyref";

	if (!(encoded_key = get_encodedkeyinfo_from_keyref(keyref))) {
		AUX_ADD_ERROR;
		return ((KeyInfo *) 0);
	}
	if (!(keyinfo = d_KeyInfo(encoded_key))) {
		aux_add_error(EDECODE, "Can't decode keyinfo of keypool", (char *)keyref, int_n, proc);
	}
	free(encoded_key->octets);

	return (keyinfo);
}


/***************************** get2_keyinfo_from_keyref *************************
 *
 *    given:  keyref
 *    return: keyinfo
 *    RC:     0 or -1
 *
 *******************************************************************************/


/***************************************************************
 *
 * Procedure get2_keyinfo_from_keyref
 *
 ***************************************************************/
#ifdef __STDC__

static int get2_keyinfo_from_keyref(
	KeyInfo	 *keyinfo,
	KeyRef	  keyref
)

#else

static int get2_keyinfo_from_keyref(
	keyinfo,
	keyref
)
KeyInfo	 *keyinfo;
KeyRef	  keyref;

#endif

{
	OctetString    *encoded_key;
	char           *proc = "get2_keyinfo_from_keyref";

	if (!(encoded_key = get_encodedkeyinfo_from_keyref(keyref))) {
		AUX_ADD_ERROR;
		return (-1);
	}

	if (d2_KeyInfo(encoded_key, keyinfo) < 0) {	/* decode into keyinfo */
		aux_add_error(EDECODE, "Can't decode keyinfo of keypool", (char *)keyref, int_n, proc);
		free(encoded_key->octets);
		return (-1);
	}
	free(encoded_key->octets);
	return (0);
}


/***************************** get_unixname *************************************
 *
 *      get_unixname returns a pointer to a static char * containing
 *      the UNIX username generated from environment variable USER
 *
 *******************************************************************************/
#if defined(MS_DOS)
/***************************************************************
 *
 * Procedure get_unixname
 *
 ***************************************************************/
#ifdef __STDC__

char *get_unixname(
)

#else

char *get_unixname(
)

#endif

{
	return ("\0");
}
#elif defined(MAC)
/***************************************************************
 *
 * Procedure get_unixname
 *
 ***************************************************************/
#ifdef __STDC__

char *get_unixname(
)

#else

char *get_unixname(
)

#endif

{
   static char unixname[128];

   unixname[8] = '\0';
   return(strncpy(unixname, getlogin(), 8));
    
   /* ist getlogin() kuerzer als 8 Zeichen, haengt '\0' schon dran,
      andernfalls von Hand anhaengen. */
}
    
#else  /* UN*X */

/***************************************************************
 *
 * Procedure get_unixname
 *
 ***************************************************************/
#ifdef __STDC__

char *get_unixname(
)

#else

char *get_unixname(
)

#endif

{

   static char unixname[64];

	return ((char *)cuserid(unixname));
}

#endif

/***************************** is_key_pool *************************************
 *
 *      is_key_pool returns TRUE if the given address is the address of the
 *      static variable sec_key_pool
 *
 *******************************************************************************/

/***************************************************************
 *
 * Procedure is_key_pool
 *
 ***************************************************************/
#ifdef __STDC__

Boolean is_key_pool(
	PSESel	 *psesel
)

#else

Boolean is_key_pool(
	psesel
)
PSESel	 *psesel;

#endif

{
	if(psesel == &sec_key_pool) return(TRUE);
	else return(FALSE);
}


/***************************** object_reencrypt *********************************
 *
 *      object_reencrypt reencrypts object with newpin
 *
 *******************************************************************************/

/***************************************************************
 *
 * Procedure object_reencrypt
 *
 ***************************************************************/
#ifdef __STDC__

static int object_reencrypt(
	PSESel	 *pse_sel,
	char	 *newpin,
	Boolean	  psepin
)

#else

static int object_reencrypt(
	pse_sel,
	newpin,
	psepin
)
PSESel	 *pse_sel;
char	 *newpin;
Boolean	  psepin;

#endif

{
	OctetString     ostr;
	int             fd;
	char           *object, *o_pin, *zwpin;
	unsigned int    size;

	char           *proc = "object_reencrypt";

	if ((fd = open_object(pse_sel, O_RDWR)) < 0) {
		AUX_ADD_ERROR;
		return (-1);
	}
	o_pin = pse_sel->object.pin;
	if (!o_pin) o_pin = "";

	if (strlen(o_pin)) {
		if (!strcmp(o_pin, newpin)) {
			close(fd);
			return (0);
		}
	}

	if (read_object(pse_sel, fd, &ostr) < 0) {
		if(LASTERROR != EDECRYPTION) aux_add_error(LASTERROR, LASTTEXT, (char *) pse_sel->object.name, char_n, proc);
		else aux_add_error(EPIN, "can't read object because PIN is wrong", (char *) pse_sel->object.name, char_n, proc);
		close(fd);
		return (-1);
	}
	if (!(object = pse_name(pse_sel->app_name, pse_sel->object.name))) {
		AUX_ADD_ERROR;
		return (-1);
	}
	unlink(object);
	strcat(object, ".pw");
	unlink(object);
	object[strlen(object) - 3] = '\0';
	strcat(object, ".sf");
	unlink(object);
	object[strlen(object) - 3] = '\0';

	if (!psepin && strlen(newpin)) {
		if (!pin_check(pse_sel, "pse", newpin, FALSE, TRUE)) {
			strcat(object, ".pw");
#ifndef MAC
			if ((fd = open(object, O_WRONLY | O_CREAT | O_EXCL, OBJMASK)) < 0) 
#else
			if ((fd = open(object, O_WRONLY | O_CREAT | O_EXCL)) < 0) 
#endif /* MAC */
			{

				aux_add_error(ESYSTEM, "Can't create object", (char *) object, char_n, proc);
				if (object) free(object);
				return (-1);
			}
			chmod(object, OBJMASK);
			strcpy(text, newpin);	/* save pin because write_enc
						 * encrypts inline */
			if (write_enc(fd, text, strlen(newpin), newpin) < 0) {
				if(LASTERROR != ESYSTEM) AUX_ADD_ERROR;
				else aux_add_error(ESYSTEM, "can't write object", (char *) object, char_n, proc);
				if (object) free(object);
				close_enc(fd);
				return (-1);
			}
			close_enc(fd);
			object[strlen(object) - 3] = '\0';
		}
	}
	if (strlen(newpin)) strcat(object, ".sf");

	/* Create object */

#ifndef MAC
	if ((fd = open(object, O_WRONLY | O_CREAT | O_EXCL, OBJMASK)) < 0) 
#else
	if ((fd = open(object, O_WRONLY | O_CREAT | O_EXCL)) < 0) 
#endif /* MAC */
	{
		aux_add_error(ESYSTEM, "Can't create object", (char *) object, char_n, proc);
		if (object) free(object);
		return (-1);
	}
	chmod(object, OBJMASK);
	if (object) free(object);

	/* write reencrypted content */

	if (!(object = (char *) malloc(ostr.noctets))) {
		aux_add_error(EMALLOC, "object ", CNULL, 0, proc);
		close_enc(fd);
		return (-1);
	}
	bcopy(ostr.octets, object, ostr.noctets);
	if (write_enc(fd, object, ostr.noctets, newpin) < 0) {
		if(LASTERROR != ESYSTEM) AUX_ADD_ERROR;
		else aux_add_error(ESYSTEM, "can't write object", (char *) object, char_n, proc);
		close_enc(fd);
		free(ostr.octets);
		free(object);
		return (-1);
	}
	free(object);
	close_enc(fd);
	free(ostr.octets);
	return (0);
}


/*********************************** open_object ****************************************
 *
 *      open_object returns an open filedescriptor or zero. In addition, it leaves
 *      the toc in psetoc, if it was called with flag != O_RDONLY and RC is not -1.
 *
 ***************************************************************************************/


/***************************************************************
 *
 * Procedure open_object
 *
 ***************************************************************/
#ifdef __STDC__

static RC open_object(
	PSESel	 *pse_sel,
	int	  flag
)

#else

static RC open_object(
	pse_sel,
	flag
)
PSESel	 *pse_sel;
int	  flag;

#endif

{
	char           *object;
	int             fd_sf, fd_pw;
	struct PSE_Objects *nxt;

	char           *proc = "open_object";

	if (chk_parm(pse_sel)) {
		AUX_ADD_ERROR;
		return (-1);
	}
	/* Read toc */

	if (!(psetoc = chk_toc(pse_sel, FALSE))) {
		AUX_ADD_ERROR;
		return (-1);
	}
	fd_sf = -2;
	if (pse_sel->object.name) {

		/*
		 * Open an object of a PSE
		 */

		/* Check whether pse_sel->object.name exists */

		nxt = psetoc->obj;
		while (nxt) {
			if (strcmp(nxt->name, pse_sel->object.name) == 0) {
				/* yes */

				goto found;
			}
			nxt = nxt->next;
		}
		aux_add_error(EOBJNAME, "object is not in toc", (char *) pse_sel->object.name, char_n, proc);
		return (-1);
found:

		if (!(object = pse_name(pse_sel->app_name, pse_sel->object.name))) {
			AUX_ADD_ERROR;
			return (-1);
		}
		strcat(object, ".sf");
		pse_pw = TRUE;
		if ((fd_sf = open(object, flag)) < 0) {
			object[strlen(object) - 3] = '\0';
			pse_pw = FALSE;
			if ((fd_sf = open(object, flag)) < 0) {
				aux_add_error(EDAMAGE, "object is in toc, but can't open", (char *) pse_sel->object.name, char_n, proc);
				if (object) free(object);
				return (-1);
			}
		}
		if (pse_pw) {

			/*
			 * Check object PIN
			 */

#ifdef SCA
			if ((pse_location = sec_psetest(pse_sel->app_name)) == ERR_in_psetest) {
				AUX_ADD_ERROR;
				return (-1);
			}
			if (pse_location == SCpse) {

				/* open object of SC application on SW-PSE */

				/*
				 * Get the PIN for the SW-PSE from the SC.
				 */

				strrep(&(pse_sel->pin), get_pse_pin_from_SC(pse_sel->app_name));
				if(!pse_sel->pin) {
					aux_add_error(EPSEPIN, "Can't get PIN for SW-PSE extension from SC", CNULL, 0, proc);
					return (-1);
				}
				if(pse_sel->object.name && strlen(pse_sel->object.name)) {
					strrep(&(pse_sel->object.pin), pse_sel->pin);
				}
			}
#endif


			object[strlen(object) - 3] = '\0';
			strcat(object, ".pw");
			if ((fd_pw = open(object, O_RDONLY)) < 0) {

				/* object needs PSE PIN */
				if(pse_sel->object.pin) pse_sel->object.pin = pin_check(pse_sel, pse_sel->object.name, pse_sel->object.pin, TRUE, TRUE);
				else pse_sel->object.pin = aux_cpy_String(pin_check(pse_sel, pse_sel->object.name, pse_sel->pin, TRUE, TRUE));

			}
			else {
				/* object needs object PIN */

				pse_sel->object.pin = pin_check(pse_sel, pse_sel->object.name, pse_sel->object.pin, TRUE, TRUE);
				close(fd_pw);
			}
			if (!pse_sel->object.pin) {
				AUX_ADD_ERROR;
				if (object) free(object);
				close(fd_sf);
				return (-1);
			}
		}
/*		else pse_sel->object.pin = CNULL; */

		if (object)	free(object);

		/*
		 * free toc if open for reading, else toc will be free'd by
		 * the calling program
		 */

	}
	return (fd_sf);
}


/******************************* pin_check **************************************
 *
 *      pin_check(app, obj, pin, err_ind) checks whether pin is the PIN of the 
 *      given (app, obj), using the following strategy:
 *      
 *      It asks for the PIN if it is not present and returns pin
 *      to the PIN entered by the user. If obj == "pse", the PIN of the
 *      PSE is checked. If err_ind == FALSE, no aux_add_error( ) is called
 *      after PIN check failed.
 *
 *******************************************************************************/


/***************************************************************
 *
 * Procedure pin_check
 *
 ***************************************************************/
#ifdef __STDC__

static char *pin_check(
	PSESel	 *pse_sel,
	char	 *obj,
	char	 *pin,
	int	  err_ind,
	Boolean	  interactive
)

#else

static char *pin_check(
	pse_sel,
	obj,
	pin,
	err_ind,
	interactive
)
PSESel	 *pse_sel;
char	 *obj;
char	 *pin;
int	  err_ind;
Boolean	  interactive;

#endif

{
	char           *object, decrypted_pin[64];
	int             fd, len, fbz, free_pin;
	char 	       *objpin;
	char           *proc = "pin_check";

/*  
   fbz: maximum number of retries if the PIN is entered from /dev/tty (only if parameter pin is NULL)
*/
	if (pin && strlen(pin)) fbz = 0;
	else fbz = 2;

	if (!(object = pse_name(pse_sel->app_name, obj))) {
		AUX_ADD_ERROR;
		return (CNULL);
	}
	strcat(object, ".pw");
	if ((fd = open(object, O_RDONLY)) < 0) {

		if (object) free(object);
		if (errno == ENOENT && strcmp(obj, "pse")) {

			/* obj uses PSE pin instead of own PIN */

			if (!(object = pse_name(pse_sel->app_name, "pse.pw"))) {
				AUX_ADD_ERROR;
				return (CNULL);
			}
			if ((fd = open(object, O_RDONLY)) >= 0) goto opened;
			if (object) free(object);
		}
		if (errno == ENOENT) return (CNULL);
		if (err_ind) aux_add_error(ESYSTEM, "Can't open object", (char *) object, char_n, proc);
		return (CNULL);
	}
opened:
	free_pin = FALSE;

try_again:

	if ((!pin || !strlen(pin)) && interactive == FALSE) {
		aux_add_error(EPIN, "no pin given", CNULL, 0, proc);
		if (object) free(object);
		return(CNULL);
	}

	if (!pin || !strlen(pin)) {

		if (strcmp(obj, "pse") == 0) pin = sec_read_pin("PIN for", pse_sel->app_name, FALSE);
		else pin = sec_read_pin("PIN for", obj, FALSE);
		if (!pin) {
			if (object) free(object);
			AUX_ADD_ERROR;
			return (CNULL);
		}
		free_pin = TRUE;
	}
	if ((len = read_dec(fd, decrypted_pin, sizeof(decrypted_pin), pin)) < 0) {
		close_dec(fd);
		if(LASTERROR != EDECRYPTION) {
			AUX_ADD_ERROR;
			return(CNULL);
		}
		goto failed;
	}
	close_dec(fd);
	decrypted_pin[len] = '\0';
	if (strcmp(decrypted_pin, pin) == 0) {

		/*
		 * PIN o.k.
		 */

		strzero(decrypted_pin);
		if (object) free(object);
		return(pin);
	}
failed:

	if (free_pin) free(pin);
	if (fbz--) {
		fd = open(object, O_RDONLY);
		pin = CNULL;
		goto try_again;
	}
	if (err_ind) aux_add_error(EPIN, "PIN check failed", CNULL, 0, proc);
	if (object) free(object);
	return (CNULL);
}


/********************** put_keyinfo_according_to_key ****************************
 *
 *      put_keyinfo_according_to_key(keyinfo, key) puts keyinfo either to
 *      key->key, or stores it on the PSE or as keyref according to key->keyref
 *      and key->pse_sel;
 *
 *******************************************************************************/


/***************************************************************
 *
 * Procedure put_keyinfo_according_to_key
 *
 ***************************************************************/
#ifdef __STDC__

RC put_keyinfo_according_to_key(
	KeyInfo	 *keyinfo,
	Key	 *key,
	ObjId	 *objid
)

#else

RC put_keyinfo_according_to_key(
	keyinfo,
	key,
	objid
)
KeyInfo	 *keyinfo;
Key	 *key;
ObjId	 *objid;

#endif

{
	int             rcode;
	OctetString    *encoded_key;
	char           *proc = "put_keyinfo_according_to_key";

	if (!key || !keyinfo) {
		aux_add_error(EINVALID, "key missing", CNULL, 0, proc);
		return (-1);
	}
	if (key->keyref == 0 && !key->pse_sel) {

		/*
		 * Return key in key
		 */

		key->key = aux_cpy_KeyInfo(keyinfo);
		return (0);
	}
	if (key->keyref == 0 && key->pse_sel) {

		/*
		 * Write key to PSE-object
		 */

		if ((encoded_key = e_KeyInfo(keyinfo))) {
			rcode = sec_write_PSE(key->pse_sel, objid, encoded_key);
			if (rcode)
				AUX_ADD_ERROR;
			free(encoded_key->octets);
			return (rcode);
		} else {
			aux_add_error(EENCODE, "Can't encode keyinfo", CNULL, 0, proc);
			return (-1);
		}
	}
	if (key->keyref > 0 || key->keyref == -1) {

		/*
		 * Install key as key reference
		 */

		rcode = sec_put_key(keyinfo, key->keyref);
		if(rcode < 0)
			AUX_ADD_ERROR;
		if(rcode > 0)
			key->keyref = rcode;
		return (rcode);
	}
	return(0);
}


/***************************** sec_read_pin ****************************************
 *
 *      sec_read_pin(text, object, reenter) reads PIN for object from /dev/tty without
 *      echo and returns a pointer which can be freed afterwards with free().
 *      It prompts text, object and ": " to stderr. If reenter == TRUE, it asks
 *      to reenter the PIN.
 *
 *******************************************************************************/

/***************************************************************
 *
 * Procedure sec_read_pin
 *
 ***************************************************************/
#ifdef __STDC__

char *sec_read_pin(
	char	 *text,
	char	 *object,
	int	  reenter
)

#else

char *sec_read_pin(
	text,
	object,
	reenter
)
char	 *text;
char	 *object;
int	  reenter;

#endif

{
	char           *dd, *buf, prompt[64];
	char           *proc = "sec_read_pin";

	buf = (char *) malloc(16);
	if (!buf) {
		aux_add_error(EMALLOC, "buf", CNULL, 0, proc);
		return (CNULL);
	}
enter:
	strcpy(prompt, "  Enter ");
	if (text && strlen(text))
		strcat(prompt, text);
	if (object && strlen(object)) {
		strcat(prompt, " ");
		strcat(prompt, object);
	}
	if (strlen(prompt))
		strcat(prompt, ": ");

	if (!(dd = aux_getpass(&prompt[2]))) {
		aux_add_error(ESYSTEM, "Can't read PIN", CNULL, 0, proc);
		return (CNULL);
	}
	strcpy(buf, dd);
	if (reenter) {
		strcpy(prompt, "Re");
		prompt[2] = 'e';
		if (!(dd = aux_getpass(prompt))) {
			aux_add_error(ESYSTEM, "Can't read PIN", CNULL, 0, proc);
			return (CNULL);
		}
		if (strcmp(buf, dd))
			goto enter;
	}
	return (buf);
}


/****************************** read_object ************************************
 *
 *      read_object(pse_sel, fd, content) reads the the previously with open_object
 *      opened object from filedescriptor fd into OctetString content.
 *      It mallocs content->octets and closes fd.
 *
 *******************************************************************************/


/***************************************************************
 *
 * Procedure read_object
 *
 ***************************************************************/
#ifdef __STDC__

static RC read_object(
	PSESel		 *pse_sel,
	int		  fd,
	OctetString	 *content
)

#else

static RC read_object(
	pse_sel,
	fd,
	content
)
PSESel		 *pse_sel;
int		  fd;
OctetString	 *content;

#endif

{
	int    		size;
	int             len;

	char           *proc = "read_object";

	if ((size = fsize(fd)) < 0) {
		AUX_ADD_ERROR;
		close(fd);
		return (-1);
	}
	if(size) content->octets = (char *) malloc(size);
	else {
		content->octets = (char *)0;
		content->noctets = 0;
		close(fd);
		return(0);
	}
	if (!content->octets) {
		aux_add_error(EMALLOC, "content->octets ", CNULL, 0, proc);
		close(fd);
		return (-1);
	}
	if ((len = read_dec(fd, content->octets, (unsigned int)  size, pse_sel->object.pin)) < 0) {
		sprintf(text, "can't read %s", pse_sel->object.name);
		AUX_ADD_ERROR;
		close_dec(fd);
		free(content->octets);
		return (-1);
	}
	close_dec(fd);
	content->noctets = len;
	return (0);
}


/****************************** read_toc ****************************************
 *
 *      read_toc(pse_sel) reads the toc of the PSE specified by pse_sel
 *      and returns a pointer to PSEToc. It does all necessary PIN checking
 *      and questionning. After successful read_toc, pse_sel->pin contains
 *      the checked PIN of the PSE.
 *
 *******************************************************************************/


/***************************************************************
 *
 * Procedure read_toc
 *
 ***************************************************************/
#ifdef __STDC__

static PSEToc *read_toc(
	PSESel	 *pse_sel
)

#else

static PSEToc *read_toc(
	pse_sel
)
PSESel	 *pse_sel;

#endif

{
	int             fdtoc, len;
	int    		tocsize;
	OctetString     encoded_toc;
	char           *toc_name, *tmppin;

	char           *proc = "read_toc";

	if (psetoc) return (psetoc);

	if (!(toc_name = pse_name(pse_sel->app_name, "Toc.sf"))) {
		AUX_ADD_ERROR;
		return ( (PSEToc * ) 0);
	}
	pse_pw = TRUE;
	if ((fdtoc = open(toc_name, O_RDONLY)) < 0) {
		toc_name[strlen(toc_name) - 3] = '\0';
		pse_pw = FALSE;
		if ((fdtoc = open(toc_name, O_RDONLY)) < 0) {
			toc_name[strlen(toc_name) - 3] = '\0';
			if ((fdtoc = open(toc_name, O_RDONLY)) < 0) {
				/* can't open PSE directory */
				aux_add_error(EAPPNAME, "Can't open PSE", (char *) toc_name, char_n, proc);
			} 
			else {
				/* can open PSE directory, but no toc */
				aux_add_error(EOBJ, "Can't find TOC of PSE", (char *) toc_name, char_n, proc);
				close(fdtoc);
			}
			if (toc_name) free(toc_name);
			return ((PSEToc *) 0);
		}
	}
	if (pse_pw) {

		/*
		 * Check PIN of PSE
		 */

#ifdef SCA
		if((pse_location = sec_psetest(pse_sel->app_name)) == ERR_in_psetest) {
			AUX_ADD_ERROR;
			return ((PSEToc *) 0);
		}
		if(pse_location == SCpse) {
			strrep(&(pse_sel->pin), get_pse_pin_from_SC(pse_sel->app_name));
			if(!pse_sel->pin) {
				aux_add_error(EPSEPIN, "Can't get PIN for SW-PSE extension from SC", CNULL, 0, proc);
				return ((PSEToc *) 0);
			}
			if(pse_sel->object.name && strlen(pse_sel->object.name)) {
				strrep(&(pse_sel->object.pin), pse_sel->pin);
			}
		}
#endif
		if(!(pse_sel->pin = pin_check(pse_sel, "pse", pse_sel->pin, TRUE, TRUE))) {
			AUX_ADD_ERROR;
			close(fdtoc);
			if (toc_name) free(toc_name);
			return ((PSEToc *) 0);
		} 
		tmppin = pse_sel->pin;
	}
	else tmppin = CNULL;

	if ((tocsize = fsize(fdtoc)) < 0) {
		aux_add_error(LASTERROR, LASTTEXT, (char *) toc_name, char_n, proc);
		return ((PSEToc *) 0);
	}

	/*
	 * Read and decrypt encoded toc
	 */

	encoded_toc.octets = (char *) malloc(tocsize);
	if (!encoded_toc.octets) {
		aux_add_error(EMALLOC, "encoded_toc.octets ", CNULL, 0, proc);
		close(fdtoc);
		if (toc_name) free(toc_name);
		return ((PSEToc *) 0);
	}
	if ((len = read_dec(fdtoc, encoded_toc.octets, (unsigned int) tocsize, tmppin)) < 0) {
		sprintf(text, "can't read %s", toc_name);
		aux_add_error(LASTERROR, LASTTEXT, (char *) toc_name, char_n, proc);
		if (toc_name) free(toc_name);
		close_dec(fdtoc);
		return ((PSEToc *) 0);
	}
	close_dec(fdtoc);
	encoded_toc.noctets = len;
	if (!(psetoc = d_PSEToc(&encoded_toc))) {
		aux_add_error(EDECODE, "Can't decode toc", (char *) toc_name, char_n, proc);
	}
	if (toc_name) free(toc_name);
	free(encoded_toc.octets);
	return (psetoc);
}



/********************************** set_key_pool ***************************************
 *
 *      set_key_pool sets pse_sel to appropriate values to handle an object
 *      from the key_pool, and returns its address
 *
 ***************************************************************************************/


/***************************************************************
 *
 * Procedure set_key_pool
 *
 ***************************************************************/
#ifdef __STDC__

static PSESel *set_key_pool(
	KeyRef	  keyref
)

#else

static PSESel *set_key_pool(
	keyref
)
KeyRef	  keyref;

#endif

{
	PSESel         *pse_sel;
	static char     key[4];
	KeyInfo        *keyinfo;


	char           *proc = "set_key_pool";

	pse_sel = &sec_key_pool;

	pse_sel->pin = key_pool_pw();
	if (keyref > 0) {
		sprintf(key, "%X", keyref);	/* convert number to text */
		pse_sel->object.name = key;
		pse_sel->object.pin = pse_sel->pin;
	} else if (keyref < 0)
		pse_sel->object.name = "-1";
	else
		pse_sel->object.name = CNULL;
	return (pse_sel);
}



/***************************************************************
 *
 * Procedure key_pool_pw
 *
 ***************************************************************/
#ifdef __STDC__

static char *key_pool_pw(
)

#else

static char *key_pool_pw(
)

#endif

{
	static char       pin[64];
	char    	* proc = "key_pool_pw";

	if(!pin[0]){
		strcpy(pin, get_unixname());
		sprintf(pin + strlen(pin), "%d", 3 * getuid() - 100);
		strcat(pin, ".&%)#(#$");
	}

	return (aux_cpy_String(pin));
}


/********************************** strzero ********************************************
 *
 *      strzero(string) overwrites string with zeroes (until the first zero)
 *
 ***************************************************************************************/


/***************************************************************
 *
 * Procedure strzero
 *
 ***************************************************************/
#ifdef __STDC__

static void strzero(
	register char	 *string
)

#else

static void strzero(
	string
)
register char	 *string;

#endif

{
	while(*string) *string++ = '\0';
	return;
}


/***************************** pse_name ***************************************
 *
 *      pse_name(app, object) concatenates object to app, if app begins with /
 *      or $HOME/app otherwise, and returns a pointer to the concatenated name.
 *
 *******************************************************************************/


/***************************************************************
 *
 * Procedure pse_name
 *
 ***************************************************************/
#ifdef __STDC__

static char *pse_name(
	char	 *app,
	char	 *object
)

#else

static char *pse_name(
	app,
	object
)
char	 *app;
char	 *object;

#endif

{
	char           *homedir, *dirname;
	char           *proc = "pse_name";

	if (object == CNULL)
		object = "";

	if (*app != PATH_SEPARATION_CHAR) {
#ifndef MAC
		homedir = getenv("HOME");
#else
		homedir = MacGetEnv("HOME");
#endif /* MAC */
		if (!homedir) {
			aux_add_error(ESYSTEM, "Can't get environment variable HOME", CNULL, 0, proc);
			return (CNULL);
		}
	} else
		homedir = "";

	dirname = (char *) malloc(strlen(homedir) + strlen(app) + strlen(object) + 64);
	if (!dirname) {
		aux_add_error(EMALLOC, "dirname", CNULL, 0, proc);
		return (CNULL);
	}

	strcpy(dirname, homedir);
	if (strlen(homedir))
#ifndef MAC 
		if (dirname[strlen(dirname)-1] != PATH_SEPARATION_CHAR) 
			strcat(dirname, PATH_SEPARATION_STRING);
#else
		if (dirname[strlen(dirname)-1] != ':') 
			strcat(dirname, ":");
#endif /* MAC */
	strcat(dirname, app);
	if (strlen(object))
#ifndef MAC 
		if (dirname[strlen(dirname)-1] != PATH_SEPARATION_CHAR) 
			strcat(dirname, PATH_SEPARATION_STRING);
#else
		if (dirname[strlen(dirname)-1] != ':') 
			strcat(dirname, ":");
#endif /* MAC */
	strcat(dirname, object);
	return (dirname);
}



/******************************* write_toc ***************************************
 *
 *      write_toc(pse_sel, toc, flags) writes the toc of the PSE specified by
 *      pse_sel and toc. It opens the toc file according to flags. It returns -1
 *      in case of errors, 0 otherwise.
 *
 *******************************************************************************/


/***************************************************************
 *
 * Procedure write_toc
 *
 ***************************************************************/
#ifdef __STDC__

static int write_toc(
	PSESel	 *pse_sel,
	PSEToc	 *toc,
	int	  flags
)

#else

static int write_toc(
	pse_sel,
	toc,
	flags
)
PSESel	 *pse_sel;
PSEToc	 *toc;
int	  flags;

#endif

{
	OctetString    *encoded_toc;
	int             fd;
	char           *object;

	char           *proc = "write_toc";

	/* Open or Create Toc.sf with (encrypted) toc */

	if (pse_sel->pin && strlen(pse_sel->pin)) {
		if (!(object = pse_name(pse_sel->app_name, "Toc.sf"))) {
			AUX_ADD_ERROR;
			return (-1);
		}
	} else {
		if (!(object = pse_name(pse_sel->app_name, "Toc"))) {
			AUX_ADD_ERROR;
			return (-1);
		}
	}
#ifndef MAC
	if ((fd = open(object, flags, OBJMASK)) < 0) 
#else
	if ((fd = open(object, flags)) < 0) 
#endif /* MAC */
	{
		aux_add_error(ESYSTEM, "Can't open object", (char *) object, char_n, proc);
		if (object)
			free(object);
		return (-1);
	}
	chmod(object, OBJMASK);

	/* update time stamp */

	if (toc->update)
		free(toc->update);
	toc->update = aux_current_UTCTime();

	/* encode it */

	encoded_toc = e_PSEToc(toc);

	/* write it to PSE */

	if (write_enc(fd, encoded_toc->octets, encoded_toc->noctets, pse_sel->pin) < 0) {
		sprintf(text, "can't write %s", object);
		if(LASTERROR != ESYSTEM) AUX_ADD_ERROR;
		else aux_add_error(ESYSTEM, "can't write object", (char *) object, char_n, proc);
		free(encoded_toc->octets);
		if (object)
			free(object);
		close_enc(fd);
		return (-1);
	}
	if (object)
		free(object);
	free(encoded_toc->octets);
	close_enc(fd);
	return (0);
}



/***************************************************************
 *
 * Procedure chk_toc
 *
 ***************************************************************/
#ifdef __STDC__

PSEToc *chk_toc(
	PSESel	 *pse_sel,
	Boolean	  create
)

#else

PSEToc *chk_toc(
	pse_sel,
	create
)
PSESel	 *pse_sel;
Boolean	  create;

#endif

{
	static char     last_pse_app_name[128];
	char           *proc = "chk_toc";

	if (strcmp(last_pse_app_name, pse_sel->app_name) || create == TRUE) {
		strcpy(last_pse_app_name, pse_sel->app_name);
		if (psetoc) aux_free_PSEToc(&psetoc);
		if (create == TRUE) {
			if (!(psetoc = (PSEToc *) calloc(1, sizeof(PSEToc)))) {
				aux_add_error(EMALLOC, "psetoc", CNULL, 0, proc);
				return ( (PSEToc * ) 0);
			}
		} 
		else psetoc = read_toc(pse_sel);
		return (psetoc);
	} else {
		if (!psetoc) psetoc = read_toc(pse_sel);
		return (psetoc);
	}
}


/******************************* locate_toc ***************************************
 *
 *      locate_toc(object_name) locates the object entry in psetoc. It returns the
 *	end of the object list if the object does not exist in psetoc and returns
 *	the address of the next pointer if found.
 *
 *******************************************************************************/


/***************************************************************
 *
 * Procedure locate_toc
 *
 ***************************************************************/
#ifdef __STDC__

static struct PSE_Objects **locate_toc(
	char	 *objname
)

#else

static struct PSE_Objects **locate_toc(
	objname
)
char	 *objname;

#endif

{
	struct PSE_Objects **obj, *nxt;
	char           *proc = "locate_toc";

	if (!psetoc || !objname)
		return ( (struct PSE_Objects * * ) 0);
	obj = &psetoc->obj;
	for (nxt = *obj; nxt; obj = &nxt->next, nxt = *obj) {
		if (strcmp(nxt->name, objname))
			continue;
		else
			break;
	}

	return obj;
}

/******************************* strzfree ***************************************
 *
 *      strzfree (char **str) sets **str until the next '\0' to zero, frees *str
 *      and finally sets *str to zero. *str must be a null terminated string
 *      and must be obtained from a prior malloc(), calloc() etc.
 *      Intended use: Free memory which was used to store a PIN
 *
 *******************************************************************************/

/***************************************************************
 *
 * Procedure strzfree
 *
 ***************************************************************/
#ifdef __STDC__

void strzfree(
	char	**str
)

#else

void strzfree(
	str
)
char	**str;

#endif

{
	char *dd;

	dd = *str;
	if(!dd) return;

	while(*dd) *dd++ = '\0';
	free(*str);
	*str = 0;
	return;
}

/******************************* strrep ***************************************
 *
 *      strrep (char **str1, char *str2) frees *str1, if non-zero, and
 *      creates a newly malloc'ed copy of str2 in *str1. It returns zero
 *      on success and -1 if the malloc failed. *str1 and str2 are supposed
 *      to be null terminated strings.
 *
 *******************************************************************************/

/***************************************************************
 *
 * Procedure strrep
 *
 ***************************************************************/
#ifdef __STDC__

int strrep(
	char	**str1,
	char	 *str2
)

#else

int strrep(
	str1,
	str2
)
char	**str1;
char	 *str2;

#endif

{
	if(*str1) free(*str1);
	*str1 = aux_cpy_String(str2);
	if(*str1) return(0);
	else return(-1);
}

		

#ifdef SCA






/*--------------------------------------------------------------*/
/*						                */
/* PROC  handle_in_SCTSC				       	*/
/*							       	*/
/* DESCRIPTION						       	*/
/*								*/
/*  Depending on the parameter "key", it is checked whether a 	*/
/*  SECSC-function is to be called to perform the crypto-	*/
/*  function (e.g. sign) within the SCT/SC.		        */
/*  If a crypto function in the SCT/SC shall be performed, it   */
/*  is automatically checked whether the SCT/SC is available.	*/
/*								*/
/*								*/
/*  Object-name:						*/
/*  If the key is selected with "key->pse_sel" and the application */
/*  is an application on the SC, the SC-application is opened.	*/
/*								*/
/*  If the key is selected with an object name and this object  */
/*  addresses a key on the SW-PSE, the PIN for the SW-PSE is 	*/
/*  read from the SC and stored in "key->pse_sel->pin".		*/
/*								*/
/*  Keyref:							*/
/*  If the key is selected with "key->keyref", it is checked    */
/*  whether the key is stored in SCT/SC or in the SW-PSE.	*/
/*								*/
/*  Delivered key:						*/
/*  If the key is delivered in "key->key", SC_crypt == TRUE,    */
/*  it is checked whether SCT/SC available. 			*/
/*  If SCT/SC is not available, an error code will be returned, */
/*  otherwise TRUE will be returned.				*/
/*								*/
/*								*/
/* RETURN		     DESCRIPTION	      	       	*/
/*   TRUE         	       SECSC-function is to be called  	*/
/*   FALSE         	       Perform SW-PSE-software       	*/
/*  -1			       Error			       	*/
/*							       	*/
/* CALLED FUNCTIONS	     DESCRIPTION		       	*/
/*                                                              */
/*--------------------------------------------------------------*/


/***************************************************************
 *
 * Procedure handle_in_SCTSC
 *
 ***************************************************************/
#ifdef __STDC__

int handle_in_SCTSC(
	Key	 *key,
	Boolean	  SC_crypt
)

#else

int handle_in_SCTSC(
	key,
	SC_crypt
)
Key	 *key;
Boolean	  SC_crypt;

#endif

{

	PSELocation	pse_location;
	SCTDevice	SCT_available;
        AlgEnc 		alg;



	char           *proc = "handle_in_SCTSC";



	if (!key) {
		aux_add_error(EINVALID, "No key specified!", CNULL, 0, proc);
		return (-1);
	}


	if ((key->keyref == 0) && (key->pse_sel != (PSESel * ) 0) &&
	    (key->pse_sel->app_name)) {

		/*
		 *  Adress key with app_name and object_name
		 *
		 *   => Check whether application = SC application and
		 *      check whether SCT/SC available
		 */
				
		if ((pse_location = sec_psetest(key->pse_sel->app_name)) == ERR_in_psetest) {
			AUX_ADD_ERROR;
			return (-1);
		}
		if (pse_location == SWpse) 
			return(FALSE);
		

		/*
		 * Intermediate result:  1. SC available and 
		 *			 2. application = SC application.
		 *
		 * Next to do:	         Open application on SC if not yet done.
		 */

		if (handle_SC_app(key->pse_sel->app_name)) {
			AUX_ADD_ERROR;
			return (-1);
		}

		/*
		 * Check whether object = SC object
		 */

		if (aux_AppObjName2SCObj(key->pse_sel->app_name, key->pse_sel->object.name)) {

			/* 
			 *  Object = SC object => SECSC-function is to be called 
			 */

			return (TRUE);
		} 
		else {

			/* 
			 *  A key on the SW-PSE shall be accessed!
			 *
			 *  => Get the PIN for the SW-PSE from the SC.
			 */

			strrep(&(key->pse_sel->pin), get_pse_pin_from_SC(key->pse_sel->app_name));
			if(!key->pse_sel->pin) {
				aux_add_error(EPSEPIN, "Can't get PIN for SW-PSE extension from SC", CNULL, 0, proc);
				return (-1);
			}
			strrep(&(key->pse_sel->object.pin), key->pse_sel->pin);

			return (FALSE);
		}
	} else {

		if ((key->keyref != 0) && (key->keyref != -1)) {

			/*
			 * Address key with keyref !
			 * 
			 * => Check whether keyref indicates SC/SCT as level of the key 
			 */

			if (((key->keyref & SC_KEY) == SC_KEY) ||
			    ((key->keyref & SCT_KEY) == SCT_KEY)) {

				/* 
				 * Keyref indicates SC/SCT as level of the key
				 *
				 * => check whether SCT/SC available
				 */
				
				if ((SCT_available = sec_scttest()) < 0) {
					AUX_ADD_ERROR;
					return (-1);
				}
				if (SCT_available == SCTDev_avail) 
					return (TRUE);
				else {
					aux_add_error(EINVALID, "Key referenced within SCT/SC, SCT is not available.", CNULL, 0, proc);
					return (-1);
				}

			}
		}
		else {
			if (key->keyref == -1)
				return(FALSE);		
			else {

				if ((key->key != (KeyInfo *)0) && (SC_crypt == TRUE)) {

					/*
					 * Key delivered in key->key and 
					 *  crypto-function shall be performed in SCT/SC.
			 		 */

					if ((SCT_available = sec_scttest()) < 0) {
						AUX_ADD_ERROR;
						return (-1);
					}
					if (SCT_available == SCTDev_avail) 
						return (TRUE);
					else {
						aux_add_error(EINVALID, "Global variable SC_encrypt/SC_verify (SCINITFILE) is set to encryption/verification in the SCT. SCT is not available.", CNULL, 0, proc);
						return (-1);
					}

				}
			}
		}


	}		

	return (FALSE);

}				/* end handle_in_SCTSC */


/***************************************************************
 *
 * Procedure read_SCToc
 *
 ***************************************************************/
#ifdef __STDC__

PSEToc *read_SCToc(
	PSESel	 *pse_sel
)

#else

PSEToc *read_SCToc(
	pse_sel
)
PSESel	 *pse_sel;

#endif

{
	OctetString     ostr;
	PSEToc         *sc_toc;
	RC              rc;
	char           *obj;
	char           *proc = "read_SCToc";

	obj = pse_sel->object.name;
	pse_sel->object.name = SCToc_name;


	/* If SC application not open => open it */
	if (handle_SC_app(pse_sel->app_name)) {
		aux_add_error(EAPPNAME, "Can't open SC PSE", (char *) pse_sel->app_name, char_n, proc);
		pse_sel->object.name = obj;
		return ((PSEToc *) 0);
	}

	rc = secsc_read(pse_sel, &ostr);
	if (rc < 0) {
		aux_add_error(EOBJNAME, "Can't read SC toc", (char *) pse_sel, PSESel_n, proc);
		pse_sel->object.name = obj;
		return ((PSEToc *) 0);
	}
	sc_toc = d_PSEToc(&ostr);
	free(ostr.octets);
	if (!sc_toc) {
		aux_add_error(EDAMAGE, "Can't decode SC toc", (char *) pse_sel, PSESel_n, proc);
		pse_sel->object.name = obj;
		return ((PSEToc *) 0);
	}
	pse_sel->object.name = obj;
	return (sc_toc);
}

/***************************************************************
 *
 * Procedure write_SCToc
 *
 ***************************************************************/
#ifdef __STDC__

RC write_SCToc(
	PSESel	 *pse_sel,
	PSEToc	 *sc_toc
)

#else

RC write_SCToc(
	pse_sel,
	sc_toc
)
PSESel	 *pse_sel;
PSEToc	 *sc_toc;

#endif

{
	OctetString    *ostr;
	RC              rc;
	char           *obj;
	char           *proc = "write_SCToc";

	obj = pse_sel->object.name;
	pse_sel->object.name = SCToc_name;

	ostr = e_PSEToc(sc_toc);
	if (!ostr) {
		aux_add_error(EENCODE, "Can't encode SC toc", (char *) sc_toc, PSEToc_n, proc);
		pse_sel->object.name = obj;
		return (-1);
	}

	/* If SC application not open => open it */
	if (handle_SC_app(pse_sel->app_name)) {
		AUX_ADD_ERROR;
		pse_sel->object.name = obj;
		return (-1);
	}

	rc = secsc_write(pse_sel, ostr);
	aux_free_OctetString(&ostr);
	if (rc < 0) {
		AUX_ADD_ERROR;
		pse_sel->object.name = obj;
		return (-1);
	}
	pse_sel->object.name = obj;
	return (0);
}

/***************************************************************
 *
 * Procedure create_SCToc
 *
 ***************************************************************/
#ifdef __STDC__

PSEToc *create_SCToc(
	PSESel	 *pse_sel
)

#else

PSEToc *create_SCToc(
	pse_sel
)
PSESel	 *pse_sel;

#endif

{
	RC              rc;
	char           *obj;
	char           *proc = "create_SCToc";

	obj = pse_sel->object.name;
	pse_sel->object.name = SCToc_name;

	/* If SC application not open => open it */
	if (handle_SC_app(pse_sel->app_name)) {
		AUX_ADD_ERROR;
		pse_sel->object.name = obj;
		return ((PSEToc *) 0);
	}

	rc = secsc_create(pse_sel);
	if (rc < 0) {
		AUX_ADD_ERROR;
		pse_sel->object.name = obj;
		return ((PSEToc *) 0);
	}
	sc_toc = (PSEToc *) malloc(sizeof(PSEToc));
	if (!sc_toc) {
		aux_add_error(EMALLOC, "sc_toc", CNULL, 0, proc);
		pse_sel->object.name = obj;
		return ((PSEToc *) 0);
	}
	sc_toc->owner = (char *) malloc(128);
	if (!sc_toc->owner) {
		aux_add_error(EMALLOC, "sc_toc", CNULL, 0, proc);
		pse_sel->object.name = obj;
		return ((PSEToc *) 0);
	}
	strcpy(sc_toc->owner, get_unixname());
	sc_toc->create = aux_current_UTCTime();
	sc_toc->update = aux_current_UTCTime();
	sc_toc->obj = (struct PSE_Objects *) 0;
	pse_sel->object.name = obj;
	return (sc_toc);
}

/***************************************************************
 *
 * Procedure update_SCToc
 *
 ***************************************************************/
#ifdef __STDC__

RC update_SCToc(
	PSESel	 *pse_sel,
	int	  length,
	int	  st
)

#else

RC update_SCToc(
	pse_sel,
	length,
	st
)
PSESel	 *pse_sel;
int	  length;
int	  st;

#endif

{
	OctetString    *ostr;
	RC              rc;
	struct PSE_Objects *nxt, *pre;
	char           *object;
	char           *proc = "update_SCToc";

	object = pse_sel->object.name;

	/* Read SC toc */

	if (!(sc_toc = chk_SCToc(pse_sel))) {
		if (LASTERROR == EOBJNAME) {

			/* doesn't exist, create */

			sc_toc = create_SCToc(pse_sel);
			if (!sc_toc) {
				AUX_ADD_ERROR;
				return (-1);
			}
			aux_free_error();
		} else {
			AUX_ADD_ERROR;
			return (-1);
		}
	}
	/* Check whether pse_sel->object.name already exists */

	nxt = sc_toc->obj;
	pre = (struct PSE_Objects *) 0;
	while (nxt) {
		if (strcmp(nxt->name, object) == 0) {

			/* yes, change update time */

			if (nxt->update)
				free(nxt->update);
			nxt->update = aux_current_UTCTime();
			if (sc_toc->update)
				free(sc_toc->update);
			sc_toc->update = aux_current_UTCTime();
			if (strcmp(sc_toc->owner, get_unixname())) {
				free(sc_toc->owner);
				sc_toc->owner = (char *) malloc(128);
				if (!sc_toc->owner) {
					aux_add_error(EMALLOC, "sc_toc", CNULL, 0, proc);
					return (-1);
				}
				strcpy(sc_toc->owner, get_unixname());
			}
			nxt->noOctets = length;
			nxt->status = st;
			goto write_sc_toc;
		}
		pre = nxt;
		nxt = nxt->next;
	}

	/* append new object */

	/* allocate memory for new element */

	nxt = (struct PSE_Objects *) malloc(sizeof(struct PSE_Objects));
	if (!nxt) {
		aux_add_error(EMALLOC, "new", CNULL, 0, proc);
		return (-1);
	}
	if (!(nxt->name = (char *) malloc(strlen(object) + 1))) {
		aux_add_error(EMALLOC, "next->name", CNULL, 0, proc);
		return (-1);
	}
	strcpy(nxt->name, object);
	nxt->create = aux_current_UTCTime();
	nxt->update = aux_current_UTCTime();
	sc_toc->update = aux_current_UTCTime();
	if (strcmp(sc_toc->owner, get_unixname())) {
		free(sc_toc->owner);
		sc_toc->owner = (char *) malloc(128);
		if (!sc_toc->owner) {
			aux_add_error(EMALLOC, "sc_toc", CNULL, 0, proc);
			return (-1);
		}
		strcpy(sc_toc->owner, get_unixname());
	}
	nxt->noOctets = length;
	nxt->status = st;
	nxt->next = (struct PSE_Objects *) 0;
	if (pre)
		pre->next = nxt;
	else
		sc_toc->obj = nxt;

write_sc_toc:

	rc = write_SCToc(pse_sel, sc_toc);

	if (rc < 0)
		AUX_ADD_ERROR;

	return (rc);
}

/***************************************************************
 *
 * Procedure delete_SCToc
 *
 ***************************************************************/
#ifdef __STDC__

RC delete_SCToc(
	PSESel	 *pse_sel
)

#else

RC delete_SCToc(
	pse_sel
)
PSESel	 *pse_sel;

#endif

{
	OctetString    *ostr;
	RC              rc;
	struct PSE_Objects *nxt, *pre;
	char           *object;
	char           *proc = "delete_SCToc";

	object = pse_sel->object.name;

	/* Read SC toc */

	if (!(sc_toc = chk_SCToc(pse_sel))) {
		AUX_ADD_ERROR;
		return (-1);

	}
	/* Check whether pse_sel->object.name already exists */

	nxt = sc_toc->obj;
	pre = (struct PSE_Objects *) 0;
	while (nxt) {
		if (strcmp(nxt->name, object) == 0) {

			/* yes, chain out */

			if (pre)
				pre->next = nxt->next;
			else
				sc_toc->obj = nxt->next;
			if (nxt->name)
				free(nxt->name);
			if (nxt->create)
				free(nxt->create);
			if (nxt->update)
				free(nxt->update);
			free(nxt);
			goto write_sc_toc;
		}
		pre = nxt;
		nxt = nxt->next;
	}

	/* object doesn't exist */

	aux_add_error(EOBJNAME, "object does not exist", (char *) object, char_n, proc);
	return (-1);


write_sc_toc:

	if (sc_toc->update)
		free(sc_toc->update);
	sc_toc->update = aux_current_UTCTime();

	rc = write_SCToc(pse_sel, sc_toc);

	if (rc < 0)
		AUX_ADD_ERROR;

	return (rc);
}

/***************************************************************
 *
 * Procedure is_in_SCToc
 *
 ***************************************************************/
#ifdef __STDC__

Boolean is_in_SCToc(
	PSESel	 *pse_sel
)

#else

Boolean is_in_SCToc(
	pse_sel
)
PSESel	 *pse_sel;

#endif

{
	OctetString    *ostr;
	RC              rc;
	struct PSE_Objects *nxt, *pre;
	char           *object;
	char           *proc = "is_in_SCToc";

	object = pse_sel->object.name;

	/* Read SC toc */

	if (!(sc_toc = chk_SCToc(pse_sel))) {
		AUX_ADD_ERROR;
		return (-1);

	}
	/* Check whether pse_sel->object.name already exists */

	nxt = sc_toc->obj;
	pre = (struct PSE_Objects *) 0;
	while (nxt) {
		if (strcmp(nxt->name, object) == 0) {

			/* yes */
			return (TRUE);

		}
		pre = nxt;
		nxt = nxt->next;
	}

	/* object doesn't exist */

	return (FALSE);
}


/***************************************************************
 *
 * Procedure chk_SCToc
 *
 ***************************************************************/
#ifdef __STDC__

PSEToc *chk_SCToc(
	PSESel	 *pse_sel
)

#else

PSEToc *chk_SCToc(
	pse_sel
)
PSESel	 *pse_sel;

#endif

{
	static char     last_pse_app_name[128];
	char           *proc = "chk_SCToc";

	if (strcmp(last_pse_app_name, pse_sel->app_name)) {
		strcpy(last_pse_app_name, pse_sel->app_name);
		if (sc_toc) aux_free_PSEToc(&sc_toc);
		sc_toc = read_SCToc(pse_sel);
		return (sc_toc);
	} else {
		if (!sc_toc) sc_toc = read_SCToc(pse_sel);
		return (sc_toc);
	}
}



/*
 *  Get update time of object in SC-Toc
 */

/***************************************************************
 *
 * Procedure get_update_time_SCToc
 *
 ***************************************************************/
#ifdef __STDC__

RC get_update_time_SCToc(
	PSESel	 *pse_sel,
	UTCTime	**update_time
)

#else

RC get_update_time_SCToc(
	pse_sel,
	update_time
)
PSESel	 *pse_sel;
UTCTime	**update_time;

#endif

{
	struct PSE_Objects *nxt, *pre;
	char           *object;
	char           *proc = "get_update_time_SCToc";

	*update_time =  (UTCTime *)0;
	object = pse_sel->object.name;


	/* Read SC toc */

	if (!(sc_toc = chk_SCToc(pse_sel))) {
		if (LASTERROR == EOBJNAME) {

			/* doesn't exist, create */

			sc_toc = create_SCToc(pse_sel);
			if (!sc_toc) {
				AUX_ADD_ERROR;
				return (-1);
			}
			aux_free_error();
		} else {
			AUX_ADD_ERROR;
			return (-1);
		}
	}

	/* Search entry of object in SCToc */

	nxt = sc_toc->obj;
	pre = (struct PSE_Objects *) 0;
	while (nxt) {
		if (strcmp(nxt->name, object) == 0) {

			/*  entry of object found */
			*update_time = nxt->update;
			return(0);
		}
		pre = nxt;
		nxt = nxt->next;
	}

	return (0);

}		/* get_update_time_SCToc */



#endif				/* SCA */


