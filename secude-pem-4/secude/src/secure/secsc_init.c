/*
 *  SecuDE Release 4.3 (GMD), STARCOS Version 1.1
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

/*------------------------------------------------------------------*/
/* FILE  secsc_init.c                                               */
/* Initialization of global variables of smartcard module           */
/*------------------------------------------------------------------*/

#ifdef SCA

#ifndef _SECSC_
#include "secsc.h"
#endif

Boolean         SC_verify = FALSE, SC_encrypt = FALSE, SC_ignore = FALSE;


/*
 *  Initialization of global variables:
 */

Boolean         SC_ignore_SWPSE = TRUE;	/* TRUE:  If the Software-PSE part	   */
					/* cannot be opened with the pin from	   */
					/* SC_PIN_name, sec_open ignores this	   */
					/* error. FALSE: sec_open returns -1	   */
					/* in this case          		   */

int		SC_timer = SC_WAITTIME;	/* During this time interval (in seconds)  */
					/* the SCT accepts the insertion of an SC. */



/*
 *  Initialization of sc_app_list[]:
 */

SCAppEntry      sc_app_list[] = {0};



/*
 *  Initialization of the device key list[]:
 */

DevKeyEntry	default_devkeylist[] =

  {
	AUTH_name,    PURPOSE_AUTH,
	SM_AUTH_name, PURPOSE_SECAUTH,
        SM_CONC_name, PURPOSE_SECCONC,
	CNULL
  };



/*
 *  Definition of sct_stat_list[]:
 */

SCTStatus      sct_stat_list[];



/*
 *  Initialization of sca_fct_list[]:
 */

SCAFctPar       sca_fct_list[] =

/*
   fct_name,		 	      sm_SCT,	            sm_SC
				 command,  response	command, response

*/
/* test values: without secure messaging */
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
	"sca_encrypt",		 SCT_NORM, SCT_NORM, 	SC_NORM, SC_NORM,
	"sca_del_dev_key",	 SCT_NORM, SCT_NORM, 	SC_NORM, SC_NORM,
	"sca_decrypt",		 SCT_NORM, SCT_NORM, 	SC_NORM, SC_NORM,
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

/* real values:

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
	"sca_del_dev_key",	 SCT_NORM, SCT_NORM, 	SC_NORM, SC_NORM,
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


/*
 *  Initialization for the SCT-/SC-Selection:
 *
 *	1      -> select first SCT in the installation file (list of the connected SCTs)
 */
SCSel           sc_sel = {1};





#else
int             secsc_dummy;

#endif


