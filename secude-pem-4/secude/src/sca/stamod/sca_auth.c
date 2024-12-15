/*
 *  STARMOD Release 1.1 (GMD)
 */
/*---------------------------------------------------------------------------+-----*/
/*							                     | GMD */
/*   SYSTEM   STARMOD  -  Version 1.1		                             +-----*/
/*							                           */
/*---------------------------------------------------------------------------------*/
/*							                           */
/*    PACKAGE	SCAIF                                        VERSION 1.1	   */
/*					                       DATE June    1993   */
/*					                         BY Levona Eckstein*/
/*			       				                           */
/*    FILENAME     					                           */
/*	sca_auth.c                       		         		   */
/*							                           */
/*    DESCRIPTION	   				                           */
/*      This modul provides all functions for user authentication and device       */
/*      authentication of the smartcard application interface (SCA-IF).            */
/*							                           */
/*    EXPORT		    DESCRIPTION 		                           */
/*      Functions for User Authentication					   */
/*							                           */
/*	sca_inst_verification_data() Install verification data on the smartcard    */
/*							                           */
/*	sca_verify_pin()       PIN authentication		 	           */
/*							                           */
/*	sca_change_pin()       Change PIN 			                   */
/*							                           */
/*	sca_unblock_pin()      Unblock a blocked PIN                               */
/*							                           */
/*							                           */
/*      Functions for Device Authentication					   */
/*							                           */
/*	sca_get_card_data()    Get Card data                                       */
/*							                           */
/*	sca_auth()    	       Device authentication                               */
/*							                           */
/*      									   */
/*                                                                                 */
/*    IMPORT		    DESCRIPTION 		                           */
/*	all functions and variables which are described in stamod.h                */
/*      									   */
/*    INTERNAL                                                                     */
/*                                                                                 */
/*                                                                                 */
/*                                                                                 */
/*---------------------------------------------------------------------------------*/

/*-------------------------------------------------------------*/
/*   include-Files					       */
/*-------------------------------------------------------------*/
#include "sca.h"
#include "stamod.h"
#ifndef _MACC7_
#include <sys/types.h>
#include <sys/stat.h>
#else
#include <stdlib.h>
#endif /* !MACC7 */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>




/*-------------------------------------------------------------*/
/*   forward global declarations			       */
/*-------------------------------------------------------------*/

/*-------------------------------------------------------------*/
/*   globale variable definitions			       */
/*-------------------------------------------------------------*/
static char    INIT_APPL_ID[6] = {0x00,0x00,0x00,0x00,0x00,0x00};
					   /* Initial application identifier */


/*-------------------------------------------------------------*/
/*   type definitions					       */
/*-------------------------------------------------------------*/



/*-------------------------------------------------------------*/
/*   local Variable definitions			               */
/*-------------------------------------------------------------*/
static Boolean  icc_expect;	/* = TRUE indicates: ICC expected    */
				/* = FALSE indicates: ICC not needed */
static KindOfKey kind_of_key;	/* {USER_KEY, PIN_KEY, PUK_KEY,      */
				/* DEVICE}                           */

/*-------------------------------------------------------------*/
/*   macro definitions					       */
/*-------------------------------------------------------------*/
#define ALLOC_CHAR(v,s)  {if (0 == (v = malloc(s))) {sca_errno = EMEMAVAIL; sca_errmsg = sca_errlist[sca_errno].msg; goto errcase;}}








/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_inst_verification_data VERSION   1.1	       */
/*				     DATE   March   1993       */
/*			      	       BY   L.Eckstein         */
/*							       */
/* DESCRIPTION						       */
/*  Install verification data  on the smartcard.               */
/*  A smartcard must be inserted.		               */
/*  In Version 1.1 the installation of a PIN or PUK will be    */
/*  supported						       */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   key_id	               key_id of the verification data */
/*                             to be installed	               */
/*                                                             */
/*   verify_data	       Structure which determines the  */
/*                             verification data to be installed*/
/*                                                             */
/*   key_attr_list             Structure which contains        */
/*                             additional information for      */
/*                             storing the verification data   */
/*                             on the ICC                      */
/*							       */
/* OUT							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*				 EPARINC  		       */
/*							       */
/* CALLED FUNCTIONS					       */
/*  STACheckSCT_ICC            ERROR-Codes		       */
/*			         ENOCARD		       */
/*                               ESCTLIST                      */
/*			         ESIDUNK		       */
/*				 EBAUD                         */
/*				 ENOSHELL		       */
/*                               EOPERR                        */
/*			         EEMPTY                        */
/*                               EMEMAVAIL                     */
/*                               ECLERR                        */
/*                               ERDERR                        */
/*                               EINVARG		       */
/*                               ETOOLONG		       */
/*			         ENOINTKEY		       */
/*				 ENOCONCKEY		       */ 
/*                               T1 - ERROR                    */
/*                               sw1/sw2 from SCT response     */
/*							       */
/*  STACheckKeyAttrList        ERROR-Codes		       */
/*			         EPARINC 		       */
/*							       */
/*  icc_check_KeyId             ERROR-Codes		       */
/*			         EPARINC		       */
/*							       */
/*  STAGetVerifyDataBody        ERROR-Codes                    */
/*                               EMEMAVAIL		       */
/*							       */
/*  sct_interface	       ERROR-Codes	               */
/*                              EINVARG			       */
/*                              ETOOLONG		       */
/*                              EMEMAVAIL		       */
/*                              ESIDUNK                        */
/*				EINS			       */
/*				EDESENC  		       */
/*				EMAC     		       */
/*				EALGO    		       */
/*				ENOINTKEY	               */
/*				ENOCONCKEY	               */
/*                              sw1/sw2 from SCT response      */
/*				EDESDEC  		       */
/*				ESSC			       */
/*				ERECMAC 	               */
/*                              T1 - ERROR                     */
/*							       */
/*  STAErrAnalyse		         	               */
/*							       */
/*							       */
/*  aux_free2_OctetString				       */
/*							       */
/*-------------------------------------------------------------*/
int
sca_inst_verification_data(sct_id, key_id, verify_data, key_attr_list)
	int             	sct_id;
	KeyId          		*key_id;
	VerifyDataStruc 	*verify_data;
	KeyAttrList    		*key_attr_list;
{
	int             rc;
     	VERRecord       verinfo;
	OctetString     data_body;
	ICC_SecMess     sec_mess;
	char		*proc="sca_inst_verification_data";

	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;
	icc_expect = TRUE;

#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_inst_verification_data *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id		        : %d\n", sct_id);
	print_KeyId(key_id);
	print_VerifyDataStruc(verify_data);
	print_KeyAttrList(key_attr_list);
	fprintf(stdout, "\n\n");
#endif



/*****************************************************************************/
/*
 *      check parameter
 */

	/*
	 * Check key_id				               
	 */
	if (icc_check_KeyId(key_id) == -1)
		goto parerrcase;

	/*
	 * Check verification data record
	 */
	if (verify_data->verify_data_type == PIN) {
		/*	
		 * check PIN
		
 */
		kind_of_key = PIN_KEY;
		if ((verify_data->VerifyDataBody.pin_info.min_len < 0) ||
		    (verify_data->VerifyDataBody.pin_info.min_len > MAXL_PIN))
			goto parerrcase;
		if ((verify_data->VerifyDataBody.pin_info.pin == NULLOCTETSTRING) ||
		    (verify_data->VerifyDataBody.pin_info.pin->noctets <= 0) ||
		    (verify_data->VerifyDataBody.pin_info.pin->octets == NULL))
			goto parerrcase;
		else if (verify_data->VerifyDataBody.pin_info.pin->noctets > MAXL_PIN)
			goto parerrcase;

	} else if (verify_data->verify_data_type == PUK) {
		/*	
		 * check PUK 
		 */
		kind_of_key = PUK_KEY;
		if ((verify_data->VerifyDataBody.puk_info.puk == NULLOCTETSTRING) ||
		    (verify_data->VerifyDataBody.puk_info.puk->noctets <= 0) ||
		    (verify_data->VerifyDataBody.puk_info.puk->octets == NULL))
			goto parerrcase;
		else if (verify_data->VerifyDataBody.puk_info.puk->noctets > MAXL_PIN)
			goto parerrcase;

		if (icc_check_KeyId(verify_data->VerifyDataBody.puk_info.pin_key_id) == -1)
			goto parerrcase;



	} else 
		goto parerrcase;		/* no PIN, no PUK */

	/*
	 * Check key_attr_list
	 */
	if (STACheckKeyAttrList(kind_of_key, key_attr_list) == -1)
		return (-1);

/*****************************************************************************/
/*
 *      call STACheckSCT_ICC
 */
	if (STACheckSCT_ICC(sct_id, icc_expect, proc) == -1)
		return (-1);

/*****************************************************************************/
/*
 *      if key_attr_list->key_inst_mode = INST, then test, if key
 *      already installed (send ICC command KEYSTATUS
 */
	if (key_attr_list->key_inst_mode == INST) {

		/*
 		 *      create ICC command 
 		 */
      		sec_mess.command = ICC_SEC_NORMAL;
      		sec_mess.response= ICC_SEC_NORMAL;
		if (STACreateHead(ICC_KEYSTAT, &sec_mess))
			return (-1);

		/* set parameters			  */
		PAR_KID 	= key_id;

		/* call STACreateTrans			  */
		rc = STACreateTrans(sct_id, TRUE);
		if (rc == -1) {
			switch (sca_errno) {
			  case ERID:
				/*
				 * Key not installed 
				 */
				break;
			  default:
				sca_errno = EINV_ACCESS;
				sca_errmsg = sca_errlist[sca_errno].msg;
				return(-1);
			}
		}
		else {


			/*
 			 *       key already exists => release response storage
 			 */
			Gsct_response.noctets = 1;
			aux_free2_OctetString(&Gsct_response);
			sca_errno = EINV_ACCESS;
			sca_errmsg = sca_errlist[sca_errno].msg;
			return (-1);
		}
	}






/*****************************************************************************/
/*
 *      Prepare parameters for the SCT Interface
 */
	Gsct_cmd = S_INST_VER_DATA;
	Gsct_request.rq_p1.kid = key_id;

	if (verify_data->verify_data_type == PIN) {
		if (verify_data->VerifyDataBody.pin_info.disengageable == TRUE)
			verinfo.key_algid = S_DISENGAGEABLE_PIN;
		else
			verinfo.key_algid = S_NON_DISENGAGEABLE_PIN;
	}
	else
		verinfo.key_algid = S_PUK;

	verinfo.key_attr_list = key_attr_list;

	rc = STAGetVerifyDataBody(verify_data, &data_body);
	if (rc == -1)
		return (-1);
	verinfo.ver_data = &data_body;

#ifdef TEST
	fprintf(stdout, "verinfo.ver_data->octets: \n");
	aux_fxdump(stdout, verinfo.ver_data->octets, verinfo.ver_data->noctets, 0);
	fprintf(stdout, "\n");
#endif

        Gsct_request.rq_datafield.ver_record = &verinfo;

/*****************************************************************************/
/*
 *      Call SCT Interface
 */
	rc = sct_interface(sct_id, Gsct_cmd, &Gsct_request, &Gsct_response);
	if (rc < 0) {
		sca_errno = sct_errno;
		sca_errmsg = sct_errmsg;
		STAErrAnalyse(sct_id);
		data_body.noctets = 1;
		aux_free2_OctetString(&data_body);
		return (-1);
	}
/*****************************************************************************/
/*
 *      Normal End	 (Release storage)
 */

	data_body.noctets = 1;
	Gsct_response.noctets = 1;
	aux_free2_OctetString(&data_body);
	aux_free2_OctetString(&Gsct_response);

#ifdef TEST
	fprintf(stdout, "\n***** Normal end of   sca_inst_verification_data *****\n\n");
#endif

	return (sca_errno);

/*****************************************************************************/
/*
 *      Parameter error
 */
parerrcase:
	sca_errno = EPARINC;
	sca_errmsg = sca_errlist[sca_errno].msg;
	return (-1);



}				/* end sca_inst_verification_data */

/*------------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_inst_verification_data  */
/*------------------------------------------------------------------*/

/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_verify_pin             VERSION   1.1	       */
/*				     DATE   March   1993       */
/*			      	       BY   L. Eckstein        */
/*							       */
/* DESCRIPTION						       */
/*  PIN authentication			                       */
/*  A smartcard must be inserted.		               */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   key_id	               key_id of the PIN to be         */
/*                             changed		               */
/*                                                             */
/*   sec_mess	               Specification of the security   */
/*			       mode(s) for the command and     */
/*			       response exchange between SCT   */
/*			       and smartcard.                  */
/*							       */
/* OUT							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*				 EPARINC 		       */
/*							       */
/* CALLED FUNCTIONS					       */
/*  STACheckSCT_ICC            ERROR-Codes		       */
/*			         ENOCARD		       */
/*                               ESCTLIST                      */
/*			         ESIDUNK		       */
/*				 EBAUD                         */
/*				 ENOSHELL		       */
/*                               EOPERR                        */
/*			         EEMPTY                        */
/*                               EMEMAVAIL                     */
/*                               ECLERR                        */
/*                               ERDERR                        */
/*                               EINVARG		       */
/*                               ETOOLONG		       */
/*			         ENOINTKEY		       */
/*				 ENOCONCKEY		       */ 
/*                               T1 - ERROR                    */
/*                               sw1/sw2 from SCT response     */
/*							       */
/*  icc_check_KeyId           ERROR-Codes		       */
/*			         EPARINC		       */
/*							       */
/*  STACheckSecMess             ERROR-Codes		       */
/*			         EPARINC 		       */
/*							       */
/*  sct_interface	       ERROR-Codes	               */
/*                              EINVARG			       */
/*                              ETOOLONG		       */
/*                              EMEMAVAIL		       */
/*                              ESIDUNK                        */
/*				EINS			       */
/*				EDESENC  		       */
/*				EMAC     		       */
/*				EALGO    		       */
/*				ENOINTKEY	               */
/*				ENOCONCKEY	               */
/*                              sw1/sw2 from SCT response      */
/*				EDESDEC  		       */
/*				ESSC			       */
/*				ERECMAC 	               */
/*                              T1 - ERROR                     */
/*							       */
/*  STAErrAnalyse		        	               */
/*							       */
/*							       */
/*  aux_free2_OctetString				       */
/*							       */
/*-------------------------------------------------------------*/
int
sca_verify_pin(sct_id, key_id, sec_mess)
	int             sct_id;
	KeyId          *key_id;
	ICC_SecMess        *sec_mess;
{

	int             rc;
	char		*proc="sca_verify_pin";

	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;
	icc_expect = TRUE;

#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_verify_pin *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id		        : %d\n", sct_id);
	print_KeyId(key_id);
	print_ICC_SecMess(sec_mess);
	fprintf(stdout, "\n\n");
#endif



/*****************************************************************************/
/*
 *      check parameter
 */
	/*
	 * Check key_id				               
	 */
	if (icc_check_KeyId(key_id) == -1) {
		sca_errno = icc_errno;
		sca_errmsg = icc_errmsg;
		return(-1);
	}


	/*
 	 *      check sec_mess
 	 */
	if (STACheckSecMess(sec_mess) == -1)
		return (-1);


/*****************************************************************************/
/*
 *      call STACheckSCT_ICC
 */
	if (STACheckSCT_ICC(sct_id, icc_expect, proc) == -1)
		return (-1);


/*****************************************************************************/
/*
 *      Prepare parameters for the SCT Interface
 */
	Gsct_cmd = S_PERFORM_VERIFY;
	Gsct_request.rq_p1.kid = key_id;
	Gsct_request.rq_datafield.sec_mode = sec_mess;

/*****************************************************************************/
/*
 *      Call SCT Interface
 */
	rc = sct_interface(sct_id, Gsct_cmd, &Gsct_request, &Gsct_response);
	if (rc < 0) {
		sca_errno = sct_errno;
		sca_errmsg = sct_errmsg;
		STAErrAnalyse(sct_id);
		return (-1);
	}

/*****************************************************************************/
/*
 *      Normal End	 (Release storage)
 */
	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);

#ifdef TEST
	fprintf(stdout, "\n***** Normal end of   sca_verify_pin *****\n\n");
#endif

	return (sca_errno);

}				/* end sca_verify_pin */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_verify_pin	       */
/*-------------------------------------------------------------*/




/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_change_pin	          VERSION   1.1	    	       */
/*				     DATE   March   1993       */
/*			      	       BY   L. Eckstein        */
/*							       */
/* DESCRIPTION						       */
/*  Change PIN value on the smartcard                          */
/*  A smartcard must be inserted.		               */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   key_id	               key_id of the PIN to be         */
/*                             changed		               */
/*                                                             */
/*   sec_mess	               Specification of the security   */
/*			       mode(s) for the command and     */
/*			       response exchange between SCT   */
/*			       and smartcard.                  */
/*							       */
/* OUT							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*				 EPARINC 		       */
/*							       */
/* CALLED FUNCTIONS					       */
/*  STACheckSCT_ICC            ERROR-Codes		       */
/*			         ENOCARD		       */
/*                               ESCTLIST                      */
/*			         ESIDUNK		       */
/*				 EBAUD                         */
/*				 ENOSHELL		       */
/*                               EOPERR                        */
/*			         EEMPTY                        */
/*                               EMEMAVAIL                     */
/*                               ECLERR                        */
/*                               ERDERR                        */
/*                               EINVARG		       */
/*                               ETOOLONG		       */
/*			         ENOINTKEY		       */
/*				 ENOCONCKEY		       */ 
/*                               T1 - ERROR                    */
/*                               sw1/sw2 from SCT response     */
/*  icc_check_KeyId           ERROR-Codes		       */
/*			         EPARINC		       */
/*							       */
/*  STACheckSecMess             ERROR-Codes		       */
/*			         EPARINC 		       */
/*							       */
/*  sct_interface	       ERROR-Codes	               */
/*                              EINVARG			       */
/*                              ETOOLONG		       */
/*                              EMEMAVAIL		       */
/*                              ESIDUNK                        */
/*				EINS			       */
/*				EDESENC  		       */
/*				EMAC     		       */
/*				EALGO    		       */
/*				ENOINTKEY	               */
/*				ENOCONCKEY	               */
/*                              sw1/sw2 from SCT response      */
/*				EDESDEC  		       */
/*				ESSC			       */
/*				ERECMAC 	               */
/*                              T1 - ERROR                     */
/*							       */
/*  STAErrAnalyse		         	               */
/*							       */
/*							       */
/*  aux_free2_OctetString				       */
/*							       */
/*-------------------------------------------------------------*/
int
sca_change_pin(sct_id, key_id, sec_mess)
	int             sct_id;
	KeyId          *key_id;
	ICC_SecMess     *sec_mess;
{
	int             rc;
	char		*proc="sca_change_pin";

	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;
	icc_expect = TRUE;

#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_change_pin *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id		        : %d\n", sct_id);
	print_KeyId(key_id);
	print_ICC_SecMess(sec_mess);
	fprintf(stdout, "\n\n");
#endif


/*****************************************************************************/
/*
 *      check parameter
 */

	/*
	 * Check key_id				               
	 */
	if (icc_check_KeyId(key_id) == -1) {
		sca_errno = icc_errno;
		sca_errmsg = icc_errmsg;
		return(-1);
	}


	/*
 	 *      check sec_mess
 	 */
	if (STACheckSecMess(sec_mess) == -1)
		return (-1);

/*****************************************************************************/
/*
 *      call STACheckSCT_ICC
 */
	if (STACheckSCT_ICC(sct_id, icc_expect, proc) == -1)
		return (-1);


/*****************************************************************************/
/*
 *      Prepare parameters for the SCT Interface
 */
	Gsct_cmd = S_MODIFY_VER_DATA;
	Gsct_request.rq_p1.kid = key_id;
	Gsct_request.rq_p2.modify_mode = S_CHG_PIN;
        Gsct_request.rq_datafield.sec_mode = sec_mess;

/*****************************************************************************/
/*
 *      Call SCT Interface
 */
	rc = sct_interface(sct_id, Gsct_cmd, &Gsct_request, &Gsct_response);
	if (rc < 0) {
		sca_errno = sct_errno;
		sca_errmsg = sct_errmsg;
		STAErrAnalyse(sct_id);
		return (-1);
	}
/*****************************************************************************/
/*
 *      Normal End	 (Release storage)
 */
	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);

#ifdef TEST
	fprintf(stdout, "\n***** Normal end of   sca_change_pin *****\n\n");
#endif

	return (sca_errno);

}				/* end sca_change_pin */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_change_pin	       */
/*-------------------------------------------------------------*/







/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_unblock_pin	  VERSION   1.1	    	       */
/*				     DATE   March   1993       */
/*			      	       BY   L. Eckstein        */
/*							       */
/* DESCRIPTION						       */
/*  Unblock a blocked PIN		                       */
/*  A smartcard must be inserted.		               */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   key_id	               key_id of the PIN to be         */
/*                             changed		               */
/*                                                             */
/*   sec_mess	               Specification of the security   */
/*			       mode(s) for the command and     */
/*			       response exchange between SCT   */
/*			       and smartcard.                  */
/*							       */
/* OUT							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*				 EPARINC 		       */
/*							       */
/* CALLED FUNCTIONS					       */
/*  STACheckSCT_ICC            ERROR-Codes		       */
/*			         ENOCARD		       */
/*                               ESCTLIST                      */
/*			         ESIDUNK		       */
/*				 EBAUD                         */
/*				 ENOSHELL		       */
/*                               EOPERR                        */
/*			         EEMPTY                        */
/*                               EMEMAVAIL                     */
/*                               ECLERR                        */
/*                               ERDERR                        */
/*                               EINVARG		       */
/*                               ETOOLONG		       */
/*			         ENOINTKEY		       */
/*				 ENOCONCKEY		       */ 
/*                               T1 - ERROR                    */
/*                               sw1/sw2 from SCT response     */
/*							       */
/*  icc_check_KeyId           ERROR-Codes		       */
/*			         EPARINC		       */
/*							       */
/*  STACheckSecMess             ERROR-Codes		       */
/*			         EPARINC 		       */
/*							       */
/*  sct_interface	       ERROR-Codes	               */
/*                              EINVARG			       */
/*                              ETOOLONG		       */
/*                              EMEMAVAIL		       */
/*                              ESIDUNK                        */
/*				EINS			       */
/*				EDESENC  		       */
/*				EMAC     		       */
/*				EALGO    		       */
/*				ENOINTKEY	               */
/*				ENOCONCKEY	               */
/*                              sw1/sw2 from SCT response      */
/*				EDESDEC  		       */
/*				ESSC			       */
/*				ERECMAC 	               */
/*                              T1 - ERROR                     */
/*							       */
/*  STAErrAnalyse		        	               */
/*							       */
/*							       */
/*  aux_free2_OctetString				       */
/*							       */
/*-------------------------------------------------------------*/
int
sca_unblock_pin(sct_id, key_id, sec_mess)
	int             sct_id;
	KeyId          *key_id;
	ICC_SecMess     *sec_mess;
{
	int             rc;
	char		*proc="sca_unblock_pin";

	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;
	icc_expect = TRUE;

#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_unblock_pin *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id		        : %d\n", sct_id);
	print_KeyId(key_id);
	print_ICC_SecMess(sec_mess);
	fprintf(stdout, "\n\n");
#endif


/*****************************************************************************/
/*
 *      check parameter
 */
	/*
	 * Check key_id				               
	 */
	if (icc_check_KeyId(key_id) == -1) {
		sca_errno = icc_errno;
		sca_errmsg = icc_errmsg;
		return(-1);
	}


	/*
 	 *      check sec_mess
 	 */
	if (STACheckSecMess(sec_mess) == -1)
		return (-1);


/*****************************************************************************/
/*
 *      call STACheckSCT_ICC
 */
	if (STACheckSCT_ICC(sct_id, icc_expect, proc) == -1)
		return (-1);


/*****************************************************************************/
/*
 *      Prepare parameters for the SCT Interface
 */
	Gsct_cmd = S_MODIFY_VER_DATA;
	Gsct_request.rq_p1.kid = key_id;
	Gsct_request.rq_p2.modify_mode = S_UNBLOCK_PIN;
        Gsct_request.rq_datafield.sec_mode = sec_mess;

/*****************************************************************************/
/*
 *      Call SCT Interface
 */
	rc = sct_interface(sct_id, Gsct_cmd, &Gsct_request, &Gsct_response);
	if (rc < 0) {
		sca_errno = sct_errno;
		sca_errmsg = sct_errmsg;
		STAErrAnalyse(sct_id);
		return (-1);
	}
/*****************************************************************************/
/*
 *      Normal End	 (Release storage)
 */
	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);

#ifdef TEST
	fprintf(stdout, "\n***** Normal end of   sca_unblock_pin *****\n\n");
#endif

	return (sca_errno);

}				/* end sca_unblock_pin */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_unblock_pin	       */
/*-------------------------------------------------------------*/

/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_get_card_data	  VERSION   1.1	    	       */
/*				     DATE   March   1993       */
/*			      	       BY   L. Eckstein        */
/*							       */
/* DESCRIPTION						       */
/*  Get card data       		                       */
/*  A smartcard must be inserted.		               */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   sec_mess	               Specification of the security   */
/*			       mode(s) for the command and     */
/*			       response exchange between SCT   */
/*			       and smartcard.                  */
/*							       */
/* OUT							       */
/*   in_data	               card data                       */
/*			       in_data->octets will be         */
/*			       allocated by the called program */ 	
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*				 EPARINC 		       */
/*				 EMEMAVAIL		       */
/*							       */
/* CALLED FUNCTIONS					       */
/*  STACheckSCT_ICC            ERROR-Codes		       */
/*			         ENOCARD		       */
/*                               ESCTLIST                      */
/*			         ESIDUNK		       */
/*				 EBAUD                         */
/*				 ENOSHELL		       */
/*                               EOPERR                        */
/*			         EEMPTY                        */
/*                               EMEMAVAIL                     */
/*                               ECLERR                        */
/*                               ERDERR                        */
/*                               EINVARG		       */
/*                               ETOOLONG		       */
/*			         ENOINTKEY		       */
/*				 ENOCONCKEY		       */ 
/*                               T1 - ERROR                    */
/*                               sw1/sw2 from SCT response     */
/*							       */
/*  STACreateHead              ERROR-Codes    		       */
/*                               EPARINC 		       */
/*							       */
/*							       */
/*  STACreateTrans   	       ERROR-Codes	               */
/*                              EINVARG			       */
/*                              ETOOLONG		       */
/*                              EMEMAVAIL		       */
/*                              ESIDUNK                        */
/*				EINS			       */
/*				EDESENC  		       */
/*				EMAC     		       */
/*				EALGO    		       */
/*				ENOINTKEY	               */
/*				ENOCONCKEY	               */
/*                              sw1/sw2 from SCT response      */
/*				EDESDEC  		       */
/*				ESSC			       */
/*				ERECMAC 	               */
/*                              T1 - ERROR                     */
/*			        EPARINC                        */
/*                              sw1/sw2 from ICC  response     */
/*							       */
/*							       */
/*   malloc macro in MF_check.h				       */
/*							       */
/*							       */
/*  aux_free2_OctetString				       */
/*							       */
/*-------------------------------------------------------------*/
int
sca_get_card_data(sct_id, in_data, sec_mess)
	int             sct_id;
	OctetString	*in_data;
	ICC_SecMess     *sec_mess;
{
	int             rc, i;
	char		*proc="sca_get_card_data";

	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;
	icc_expect = TRUE;

#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_get_card_data *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id		        : %d\n", sct_id);
	print_ICC_SecMess(sec_mess);
#endif


/*****************************************************************************/
/*
 *      initialize return parameter
 */
	in_data->octets = NULL;
	in_data->noctets= 0;

/*
 *      the correctness of the icc command specific parameters are checked 
 * 	in the function icc_create
 */

/*****************************************************************************/
/*
 *      call STACheckSCT_ICC
 */
	if (STACheckSCT_ICC(sct_id, icc_expect, proc) == -1)
		return (-1);


/*****************************************************************************/
/*
 *      create ICC command GET CARD DATA
 */
	/* create header                       */
	if (STACreateHead(ICC_GET_CD, sec_mess))
		return (-1);


	/* call STACreateTrans			  */
	if (STACreateTrans(sct_id, TRUE))
		return (-1);



/*****************************************************************************/
/*
 *      normal end => transfer response message  from ICC into in_data 
 */
	in_data->noctets = Gsct_response.noctets;
	ALLOC_CHAR(in_data->octets, in_data->noctets);

	for (i = 0; i < in_data->noctets; i++)
		in_data->octets[i] = Gsct_response.octets[i];
	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);

#ifdef TEST
	fprintf(stdout, "TRACE of the output parameters : \n");
	fprintf(stdout, "in_data                 : \n");
	fprintf(stdout, "    noctets             : %d\n", in_data->noctets);
	fprintf(stdout, "    octets              : \n");
	aux_fxdump(stdout, in_data->octets, in_data->noctets, 0);
	fprintf(stdout, "\n***** Normal end of   sca_get_card_data *****\n\n");
#endif



	return (sca_errno);

/*****************************************************************************/
/*
 *      Memory error
 */
errcase:
	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);
	return (-1);


}				/* end sca_get_card_data */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_get_card_data      */
/*-------------------------------------------------------------*/


/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_auth		  VERSION   1.1	    	       */
/*				     DATE   March   1993       */
/*			      	       BY   L. Eckstein        */
/*							       */
/* DESCRIPTION						       */
/*  Device authentication		                       */
/*  A smartcard must be inserted.		               */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   auth_proc_id              Identifier of the authentication*/
/*			       procedure:                      */
/*                                                             */
/*   auth_object_id            Identifier of the object to be  */
/*			       authenticated:                  */
/*   key_id		       key identifier                  */
/*			       will only be evaluated in case  */
/*			       of auth_object_id = DTE_ICC     */
/*			        	 		       */	
/*   auth_out_data	       outgoing authentication data    */
/*			       will only be evaluated in case  */
/*			       of auth_object_id = DTE_ICC     */
/*			        	 		       */	
/*   encryption_key            Structure which identifies the  */
/*                             encryption   key.               */
/*			       This parameter will only be     */
/*			       evaluated in case of            */
/*			       auth_object_id = DTE_ICC	and    */
/*			       auth_proc_id == EX_AUTH or MU_AUTH */         
/*			       In encryption_key->subjectAI the*/
/*			       algorithm identifier of the     */
/*			       encryption algorithm must be    */
/*			       specified.		       */
/*			       In the current Version only the */
/*			       algorithm identifier            */
/*			       desCBC_ISO0		       */
/*			       will be supported.              */
/*			       In encryption_key->subjectkey   */
/*			       the DES-Key must be delivered.  */
/*			       If this parameter is set to NULL*/
/*			       the encryption of auth_out_data */
/*			       is not done by this procedure   */ 
/*			  				       */      	 
/*   sec_mess		       security modes                  */
/*			       will only be evaluated in case  */
/*			       of auth_object_id = DTE_ICC     */
/*							       */
/* OUT							       */
/*   auth_in_data	       incoming authentication data    */
/*			       auth_in_data->octets will be    */
/*			       allocated by the called program */ 	
/*			       will only be evaluated in case  */
/*			       of auth_object_id = DTE_ICC and */
/*                             auth_proc_id != EX_AUTH         */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*			         EPARINC 		       */
/*				 EPAR 		       	       */
/*				 EMEMAVAIL 		       */
/*				 EALGO  		       */
/*				 EDESENC  		       */
/*				 EDESDEC  		       */
/*							       */
/* CALLED FUNCTIONS					       */
/*  STACheckSCT_ICC            ERROR-Codes		       */
/*			         ENOCARD		       */
/*                               ESCTLIST                      */
/*			         ESIDUNK		       */
/*				 EBAUD                         */
/*				 ENOSHELL		       */
/*                               EOPERR                        */
/*			         EEMPTY                        */
/*                               EMEMAVAIL                     */
/*                               ECLERR                        */
/*                               ERDERR                        */
/*                               EINVARG		       */
/*                               ETOOLONG		       */
/*			         ENOINTKEY		       */
/*				 ENOCONCKEY		       */ 
/*                               T1 - ERROR                    */
/*                               sw1/sw2 from SCT response     */
/*							       */
/*  icc_check_KeyId           ERROR-Codes		       */
/*			         EPARINC		       */
/*							       */
/*  sct_interface	       ERROR-Codes	               */
/*                              EINVARG			       */
/*                              ETOOLONG		       */
/*                              EMEMAVAIL		       */
/*                              ESIDUNK                        */
/*				EINS			       */
/*				EDESENC  		       */
/*				EMAC     		       */
/*				EALGO    		       */
/*				ENOINTKEY	               */
/*				ENOCONCKEY	               */
/*                              sw1/sw2 from SCT response      */
/*				EDESDEC  		       */
/*				ESSC			       */
/*				ERECMAC 	               */
/*                              T1 - ERROR                     */
/*							       */
/*  STAErrAnalyse		        	               */
/*							       */
/*  STACreateHead              ERROR-Codes    		       */
/*                               EPARINC 		       */
/*							       */
/*							       */
/*  STACreateTrans   	       ERROR-Codes	               */
/*                              EINVARG			       */
/*                              ETOOLONG		       */
/*                              EMEMAVAIL		       */
/*                              ESIDUNK                        */
/*				EINS			       */
/*				EDESENC  		       */
/*				EMAC     		       */
/*				EALGO    		       */
/*				ENOINTKEY	               */
/*				ENOCONCKEY	               */
/*                              sw1/sw2 from SCT response      */
/*				EDESDEC  		       */
/*				ESSC			       */
/*				ERECMAC 	               */
/*                              T1 - ERROR                     */
/*			        EPARINC                        */
/*                              sw1/sw2 from ICC  response     */
/*							       */
/*   malloc macro in MF_check.h				       */
/*   aux_free2_OctetString				       */
/*   aux_free2_BitString	         		       */
/*   des_encrypt                                               */
/*   aux_cmp_ObjId                                             */
/*							       */
/*-------------------------------------------------------------*/
int
sca_auth(sct_id, auth_proc_id, auth_object_id, key_id, auth_out_data,
	 encryption_key, sec_mess, auth_in_data)
	int             sct_id;
	AuthProcId      auth_proc_id;
	AuthObjectId    auth_object_id;
	KeyId		*key_id;
	OctetString	*auth_out_data;
	KeyInfo	        *encryption_key;
        ICC_SecMess     *sec_mess;
	OctetString	*auth_in_data;
{
	int             rc, i;
	Boolean		encryption = FALSE;
	BitString       out_bits;
	BitString       in_bits;
	OctetString	enc_data;
	char		*proc="sca_auth";


	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;
	icc_expect = TRUE;

#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_auth *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id		        : %d\n", sct_id);
	print_AuthProcId(auth_proc_id);
	print_AuthObjectId(auth_object_id);
	if (auth_object_id == DTE_ICC) {
		print_KeyId(key_id);
		print_OctetString("auth_out_data		:\n",
			          auth_out_data);
		print_ICC_SecMess(sec_mess);
		print_KeyInfo(encryption_key);
	}
#endif




/*****************************************************************************/
/*
 *      check parameter
 */
	if ((auth_proc_id != IN_AUTH) &&
	    (auth_proc_id != EX_AUTH) &&
	    (auth_proc_id != MU_AUTH) &&
	    (auth_proc_id != MU_AUTHWithINITIAL)) 
		goto parerrcase;

	if ((auth_object_id != DTE_ICC) &&
	    (auth_object_id != SCT_ICC))
		goto parerrcase;

	if ((auth_object_id == DTE_ICC) &&
	    (auth_proc_id == MU_AUTHWithINITIAL)) {			
		sca_errno = EPAR;
		sca_errmsg = sca_errlist[sca_errno].msg;
		return (-1);
	}

	if (auth_object_id == DTE_ICC) {

		/*
	 	 * Check key_id				               
	 	 */
		if (icc_check_KeyId(key_id) == -1)
			goto parerrcase;


		if (((auth_out_data == NULLOCTETSTRING) ||
                     (auth_out_data->octets == NULL)) ||
	            ((auth_proc_id != EX_AUTH) &&
                     (auth_in_data == NULLOCTETSTRING)))
			goto parerrcase;

		/* 
		 * check length of auth_out_data, depending on auth_proc_id
		 */
		 switch (auth_proc_id) {
			case IN_AUTH:
			case EX_AUTH:
				if (auth_out_data->noctets != 8)
					goto parerrcase;
				break;
			case MU_AUTH:
				if (auth_out_data-> noctets != 24)
					goto parerrcase;
			}

		/*
		 * check parameter encryption_key in case of auth_proc_id == EX_AUTH
		 * and auth_proc_id = MU_AUTH
		 */
		 if ((auth_proc_id == EX_AUTH) ||
		     (auth_proc_id == MU_AUTH)) { 
		 	if (encryption_key) {
				if ((!encryption_key->subjectAI) ||
			    	    (!encryption_key->subjectAI->objid)) {
					sca_errno = EALGO;
				  	sca_errmsg = sca_errlist[sca_errno].msg;
					return (-1);
				}
				if (aux_cmp_ObjId(encryption_key->subjectAI->objid,
				 	   	  desCBC_ISO0->objid)) {
					sca_errno = EALGO;
					sca_errmsg = sca_errlist[sca_errno].msg;
					return (-1);
				};
				if ((encryption_key->subjectkey.nbits <= 0) ||
			    	    (encryption_key->subjectkey.bits == NULL))
					goto parerrcase;
				encryption = TRUE;
			}
		}


	}

/*****************************************************************************/
/*
 *      call STACheckSCT_ICC
 */
	if (STACheckSCT_ICC(sct_id, icc_expect, proc) == -1)
		return (-1);


/*****************************************************************************/
/*
 *      Prepare parameters for the SCT Interface
 */
	if (auth_object_id == SCT_ICC) {
		Gsct_cmd = S_AUTH;
		Gsct_request.rq_p2.acp = auth_proc_id;
		/*
 		 *      Call SCT Interface
 		 */
		rc = sct_interface(sct_id, Gsct_cmd, &Gsct_request, &Gsct_response);
		if (rc < 0) {
			sca_errno = sct_errno;
			sca_errmsg = sct_errmsg;
			STAErrAnalyse(sct_id);
			return (-1);
		}
		/*
 		 *      Normal End	 (Release storage)
 		 */
		Gsct_response.noctets = 1;
		aux_free2_OctetString(&Gsct_response);

#ifdef TEST
		fprintf(stdout, "\n***** Normal end of   sca_auth *****\n\n");
#endif

		return (sca_errno);

	}
	else {
		/*
		 * create ICC command ..._AUTHENTICATE  
		 */
		switch (auth_proc_id) {
		case IN_AUTH:
			if (STACreateHead(ICC_IN_AUTH, sec_mess))
				return (-1);
			break;
		case EX_AUTH:
			if (STACreateHead(ICC_EX_AUTH, sec_mess))
				return (-1);
			break;
		case MU_AUTH:
			if (STACreateHead(ICC_MU_AUTH, sec_mess))
				return (-1);
			PAR_ACP.auth_method= AUTH_DES;
			PAR_ACP.key_search= KEY_ISF;
			break;
		}
		PAR_KID	 = key_id;
		if (encryption == FALSE) {
			PAR_DATA = auth_out_data;
		}
		else {

			/*
	 		 * allocate memory for out_bits             
	 		 * the memory must be a multiple of 8 Bytes 
	 		 */

			out_bits.nbits = 0;
			out_bits.bits  = NULL;

			ALLOC_CHAR(out_bits.bits, auth_out_data->noctets);
							/* will be set free in this
							 * proc. */

			/*
	 		 * encrypt auth_out_data
	 		 */
			if (des_encrypt(auth_out_data, &out_bits, SEC_END, 
				        encryption_key) == -1) {
				sca_errno = EDESENC;
				sca_errmsg = sca_errlist[sca_errno].msg;
				out_bits.nbits = 1;
				aux_free2_BitString(&out_bits);
				return (-1);
			}
#ifdef TEST 
	printf("   out_bits.nbits    = %d\n", out_bits.nbits);
	printf("   out_bits.bits     = \n");
	aux_fxdump(stdout, out_bits.bits, out_bits.nbits / 8, 0);
#endif

			enc_data.noctets = out_bits.nbits / 8;
			enc_data.octets  = out_bits.bits;
			PAR_DATA         = &enc_data;
		}

 
		/* call STACreateTrans			  */
		if (STACreateTrans(sct_id, TRUE)) {
			out_bits.nbits = 1;
			aux_free2_BitString(&out_bits);
			return (-1);
		}

		out_bits.nbits = 1;
		aux_free2_BitString(&out_bits);


		if (Gsct_response.noctets != 0) {

			/*
			 * allocate auth_in_data.octets		
			 */


			auth_in_data->noctets = 0;
			auth_in_data->octets  = NULL;

			ALLOC_CHAR(auth_in_data->octets, Gsct_response.noctets);

			if (encryption == FALSE) { 
				for (i = 0; i < Gsct_response.noctets; i++)
					*(auth_in_data->octets + i) = Gsct_response.octets[i];
				auth_in_data->noctets = Gsct_response.noctets;
			}
			else {
				in_bits.nbits = Gsct_response.noctets * 8;
				in_bits.bits =  Gsct_response.octets;
				rc = des_decrypt(&in_bits, auth_in_data,
					         SEC_END, encryption_key);

				if (rc == -1) {
					sca_errno = EDESDEC;
					sca_errmsg = sca_errlist[sca_errno].msg;
					Gsct_response.noctets = 1;
					auth_in_data->noctets = 1;
					aux_free2_OctetString(&Gsct_response);
					aux_free2_OctetString(auth_in_data);
					return (-1);
				}
			}

#ifdef TEST
			fprintf(stdout, "TRACE of the output parameters : \n");
			fprintf(stdout, "auth_in_data            : \n");
			fprintf(stdout, "    noctets             : %d\n", auth_in_data->noctets);
			fprintf(stdout, "    octets              : \n");
			aux_fxdump(stdout, auth_in_data->octets, auth_in_data->noctets, 0);
#endif
		}

		Gsct_response.noctets = 1;
		aux_free2_OctetString(&Gsct_response);





#ifdef TEST
		fprintf(stdout, "\n***** Normal end of   sca_auth *****\n\n");
#endif

		return(0);
	}

/*****************************************************************************/
/*
 *      Parameter error
 */
parerrcase:
	sca_errno = EPARINC;
	sca_errmsg = sca_errlist[sca_errno].msg;
	return (-1);

/*****************************************************************************/
/*
 *      Memory error
 */
errcase:
	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);
	return (-1);






}				/* end sca_auth */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_auth	       */
/*-------------------------------------------------------------*/





