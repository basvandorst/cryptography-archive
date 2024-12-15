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
/*					                       DATE January 1993   */
/*					                         BY Levona Eckstein*/
/*			       				                           */
/*    FILENAME     					                           */
/*	sca_dkey.c                       		         		   */
/*							                           */
/*    DESCRIPTION	   				                           */
/*      This modul provides all functions for device key handling                  */
/*							                           */
/*    EXPORT		    DESCRIPTION 		                           */
/*      Functions for Device key handling					   */
/*							                           */
/*	sca_gen_dev_key()      Generate device key                                 */
/*      									   */
/*	sca_inst_dev_key()     Install device key on user smartcard                */
/*      									   */
/*	sca_del_dev_key()      Delete device key in SCT                            */
/*      									   */
/*	sca_enable_deletion()  Enable the deletion of own device keys              */
/*      									   */
/*	sca_activate_devkeyset() Set device key set in SCT                         */
/*      									   */
/*	sca_save_devkeyset()   Save device key(s) on keycard                       */
/*      									   */
/*	sca_load_devkeyset()   Load device key(s) from keycard                     */
/*      									   */
/*                                                                                 */
/*    IMPORT		    DESCRIPTION 		                           */
/*	all functions and variables which are described in stamod.h                */
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
/*   globale variable definitions			       */
/*-------------------------------------------------------------*/
static char    INIT_APPL_ID[6] = {0x00,0x00,0x00,0x00,0x00,0x00};
					   /* Initial application identifier */


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
/* PROC  sca_gen_dev_key         VERSION   1.1	    	       */
/*				     DATE   January 1993       */
/*			      	       BY   L.Eckstein         */
/*							       */
/* DESCRIPTION						       */
/*  Generate device key (DES )		                       */
/*							       */
/*						               */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   key_dev_sel               Structure which determines the  */
/*                             device key to be generated.     */
/*                             In the version 1.1 only the     */
/*			       following algorithm identifier  */ 
/*			       is possible:		       */
/*			       desCBC_ISO1		       */
/*                                                             */
/*   appl_id		       application identifier          */
/*							       */ 
/*							       */
/* OUT							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*				 EPARINC 		       */
/*				 EALGO 		               */
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
/*  STAGetSCTAlgId              ERROR-Codes		       */
/*                               EALGO                         */
/*                               EKEYLEN                       */
/*							       */
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
/*							       */
/*  STAErrAnalyse		         	               */
/*							       */
/*  aux_free2_OctetString				       */
/*							       */
/*-------------------------------------------------------------*/
int
sca_gen_dev_key(sct_id, key_dev_sel, appl_id)
	int             sct_id;
	KeyDevSel       *key_dev_sel;
   	OctetString     *appl_id;
{
	int             i;
	int             rc;
	KeyAlgId        sct_algid;
	DevKeyInfo      sct_devkeyinfo;
	char		*proc="sca_gen_user_key";


	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;

#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_gen_dev_key *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id		        : %d\n", sct_id);
	print_KeyDevSel(key_dev_sel, S_GEN_DEV_KEY);
	print_OctetString("appl_id       		:\n",
			  appl_id);
#endif


/*****************************************************************************/
/*
 *      Check input parameters
 */
	if (!key_dev_sel) 
		goto parerrcase;

	if (icc_check_KeyId(key_dev_sel->key_id) == -1)
		goto parerrcase;



	if ((sct_algid = STAGetSCTAlgId(key_dev_sel->key_algid)) == -1)
		return (-1);
	if ((sct_algid != S_DES_CBC) ||
	    (aux_cmp_ObjId(key_dev_sel->key_algid->objid,desCBC_ISO1->objid))) {
		sca_errno = EALGO;
		sca_errmsg = sca_errlist[sca_errno].msg;
		return (-1);
	}



	if  ((key_dev_sel->key_purpose != PURPOSE_SECCONC) &&
	     (key_dev_sel->key_purpose != PURPOSE_SECAUTH) &&
	     (key_dev_sel->key_purpose != PURPOSE_AUTH))
		goto parerrcase;

	if  ((appl_id == NULLOCTETSTRING) ||
	     (appl_id->noctets != 6) ||
             (appl_id->octets == NULL)) 
		goto parerrcase;

	if (memcmp(appl_id->octets,INIT_APPL_ID,6) == 0) 
		goto parerrcase;






/*****************************************************************************/
/*
 *      call STACheckSCT_ICC
 */
	if (STACheckSCT_ICC(sct_id, FALSE, proc) == -1)
		return (-1);

/*****************************************************************************/
/*
 *	Generate key (S_GEN_DEV_KEY)
 *      Prepare parameters for the SCT Interface
 */
	Gsct_cmd = S_GEN_DEV_KEY;
	Gsct_request.rq_p1.kid = key_dev_sel->key_id;
	Gsct_request.rq_p2.algid = sct_algid;
	sct_devkeyinfo.purpose = key_dev_sel->key_purpose;
	sct_devkeyinfo.appl_id = appl_id;
	Gsct_request.rq_datafield.dev_key_info = &sct_devkeyinfo;



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
	fprintf(stdout, "\n***** Normal end of   sca_gen_dev_key *****\n\n");
#endif

	return (0);


/*****************************************************************************/
/*
 *      Parameter error
 */
parerrcase:
	sca_errno = EPARINC;
	sca_errmsg = sca_errlist[sca_errno].msg;
	return (-1);




}				/* end sca_gen_dev_key */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_gen_dev_key        */
/*-------------------------------------------------------------*/


/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_inst_dev_key         VERSION   1.1	    	       */
/*				     DATE   January 1993       */
/*			      	       BY   L.Eckstein         */
/*							       */
/* DESCRIPTION						       */
/*  Install device key (DES )	on ICC	                       */
/*							       */
/*						               */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   key_dev_sel               Structure which determines the  */
/*                             device key to be installed.     */
/*			       The parameters		       */
/*			       key_status                      */
/*			       key_id or key_purpose           */
/*			       are evaluated.		       */
/*                                                             */
/*   appl_id		       application identifier          */
/*							       */ 
/*   key_attr_list             Structure which contains        */
/*                             additional information for      */
/*                             storing the device key on       */
/*			       the ICC                         */
/*                                                             */
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
/*  STACheckKeyAttrList        ERROR-Codes		       */
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
/*  STAErrAnalyse		                               */
/*							       */
/*							       */
/*  aux_free2_OctetString				       */
/*							       */
/*-------------------------------------------------------------*/
int
sca_inst_dev_key(sct_id, key_dev_sel, appl_id, key_attr_list)
	int             sct_id;
	KeyDevSel      *key_dev_sel;
	OctetString    *appl_id;
	KeyAttrList    *key_attr_list;
{
	int             i;
	int             rc;
	DevSelKey       sct_devinstkey;
	InstDevKey      inst_dev_key;
	OctetString     init_appl_id;
	KeyId		key_id;
	ICC_SecMess     sec_mess;
	char		*proc="sca_inst_dev_key";

	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;





#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_inst_dev_key *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id		        : %d\n", sct_id);
	print_KeyDevSel(key_dev_sel, S_INST_DEV_KEY);
	print_OctetString("appl_id       		:\n",
			  appl_id);
	print_KeyAttrList(key_attr_list);
#endif



/*****************************************************************************/
/*
 *      Check input parameters
 */
	if (key_dev_sel == NULL)
		goto parerrcase;
	/* 
	 * if key_dev_sel->key_status == ANY => check appl_id   
	 * and key_id					        	
	 * otherwise appl_id ist set to 0 and    	        
	 * key_dev_sel->key_purpose will be checked             	
 	 */
	switch (key_dev_sel->key_status) {
	case DEV_ANY:
		/*
	 	 * Check key_id				               
	 	 */
		if (icc_check_KeyId(key_dev_sel->key_id) == -1)
			goto parerrcase;

		key_id.key_level  = key_dev_sel->key_id->key_level;
		key_id.key_number = key_dev_sel->key_id->key_number;



		/* 
		 * check appl_id; must be unequal to the INITIAL-Devicekeyset                                        
		 */
		if  ((appl_id == NULLOCTETSTRING) ||
	             (appl_id->noctets != 6) ||
                     (appl_id->octets == NULL))
			goto parerrcase;

		if (memcmp(appl_id->octets,INIT_APPL_ID,6) == 0)
			goto parerrcase;
		sct_devinstkey.pval.kid = key_dev_sel->key_id;
		inst_dev_key.appl_id = appl_id;
		break;

	case DEV_OWN:
		/* 
		 * check key_purpose                                   
		 */
		switch (key_dev_sel->key_purpose ) {
		case PURPOSE_SECCONC:
			key_id.key_level  = FILE_MASTER;
			key_id.key_number = 3;
			break;
		case PURPOSE_SECAUTH:
			key_id.key_level  = FILE_MASTER;
			key_id.key_number = 2;
			break;
		case PURPOSE_AUTH:
			key_id.key_level  = FILE_MASTER;
			key_id.key_number = 1;
			break;
		default:
			goto parerrcase;
		}
			
			
 		sct_devinstkey.pval.purpose = key_dev_sel->key_purpose;

		/* 
		 * check appl_id ; must be the NULL-Pointer                                       
		 */
		if  (appl_id != NULLOCTETSTRING) 
			goto parerrcase;


		/* 
		 * set initial application identifier                   
		 */
		init_appl_id.noctets = 6;
		init_appl_id.octets  = INIT_APPL_ID;
		inst_dev_key.appl_id = &init_appl_id;
		break;
	default:
		goto parerrcase;
	}

	/* 
	 * key shall be installed on the ICC,                   
	 * then - check key attribute list                     
	 */

	if (STACheckKeyAttrList(DEVICE_KEY, key_attr_list) == -1)
		return (-1);
	inst_dev_key.key_attr_list = key_attr_list;



/*****************************************************************************/
/*
 *      call STACheckSCT_ICC
 */
	if (STACheckSCT_ICC(sct_id, TRUE, proc) == -1)
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
		PAR_KID 	= &key_id;

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
 *	Generate key (S_INST_DEV_KEY)
 *      Prepare parameters for the SCT Interface
 */
	Gsct_cmd = S_INST_DEV_KEY;
	Gsct_request.rq_p1.dev_sel_key = &sct_devinstkey;
	Gsct_request.rq_datafield.inst_dev_key = &inst_dev_key;



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
	fprintf(stdout, "\n***** Normal end of   sca_inst_dev_key *****\n\n");
#endif

	return (0);

/*****************************************************************************/
/*
 *      Parameter error
 */
parerrcase:
	sca_errno = EPARINC;
	sca_errmsg = sca_errlist[sca_errno].msg;
	return (-1);



}				/* end sca_inst_dev_key */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_inst_dev_key        */
/*-------------------------------------------------------------*/



/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_del_dev_key         VERSION   1.1	    	       */
/*				     DATE   January 1993       */
/*			      	       BY   L.Eckstein         */
/*							       */
/* DESCRIPTION						       */
/*  Delete device key (DES )	in SCT	                       */
/*							       */
/*						               */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   key_dev_sel               Structure which determines the  */
/*                             device key to be installed.     */
/*			       The parameters		       */
/*			       key_status                      */
/*			       key_id or key_purpose           */
/*			       are evaluated.		       */
/*                                                             */
/*                                                             */
/*   appl_id		       application identifier          */
/*							       */ 
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
/*							       */
/*  STAErrAnalyse		                               */
/*							       */
/*							       */
/*  aux_free2_OctetString				       */
/*							       */
/*-------------------------------------------------------------*/
int
sca_del_dev_key(sct_id, key_dev_sel, appl_id)
	int             sct_id;
	KeyDevSel      *key_dev_sel;
	OctetString	*appl_id;
{
	int             i;
	int             rc;
	DevSelKey      sct_devinstkey;
	char		*proc="sca_del_dev_key";

	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;

#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_del_dev_key *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id		        : %d\n", sct_id);
	print_KeyDevSel(key_dev_sel, S_DEL_DEV_KEY);
	print_OctetString("appl_id       		:\n",
			  appl_id);
#endif


/*****************************************************************************/
/*
 *      Check input parameters
 */


	/* 
	 * check appl_id                                        
	 */
	if  ((appl_id == NULLOCTETSTRING) ||
	     (appl_id->noctets != 6) ||
             (appl_id->octets == NULL))
		goto parerrcase;

	if (key_dev_sel == NULL) 
		goto parerrcase;
	switch (key_dev_sel->key_status) {
	case DEV_ANY:

		sct_devinstkey.pval.kid = key_dev_sel->key_id;
		break;

	case DEV_OWN:
		/* 
		 * check key_purpose                                   
		 */
		if  ((key_dev_sel->key_purpose != PURPOSE_SECCONC) &&
	     	     (key_dev_sel->key_purpose != PURPOSE_SECAUTH) &&
	     	     (key_dev_sel->key_purpose != PURPOSE_AUTH))
			goto parerrcase;
 		sct_devinstkey.pval.purpose = key_dev_sel->key_purpose;


		if (memcmp(appl_id->octets,INIT_APPL_ID,6) == 0)
			goto parerrcase;
		break;
	default:
		goto parerrcase;
	}


 
/*****************************************************************************/
/*
 *      call STACheckSCT_ICC
 */
	if (STACheckSCT_ICC(sct_id, FALSE, proc) == -1)
		return (-1);


/*****************************************************************************/
/*
 *	Generate key (S_DEL_DEV_KEY)
 *      Prepare parameters for the SCT Interface
 */
	Gsct_cmd = S_DEL_DEV_KEY;
	Gsct_request.rq_p1.dev_sel_key = &sct_devinstkey;
	Gsct_request.rq_p2.status = key_dev_sel->key_status;
	Gsct_request.rq_datafield.appl_id = appl_id;



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
	fprintf(stdout, "\n***** Normal end of   sca_del_dev_key *****\n\n");
#endif

	return (0);

/*****************************************************************************/
/*
 *      Parameter error
 */
parerrcase:
	sca_errno = EPARINC;
	sca_errmsg = sca_errlist[sca_errno].msg;
	return (-1);



}				/* end sca_del_dev_key */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_del_dev_key        */
/*-------------------------------------------------------------*/

/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_enable_deletion     VERSION   1.1	    	       */
/*				     DATE   July    1993       */
/*			      	       BY   L.Eckstein         */
/*							       */
/* DESCRIPTION						       */
/*  Enable the deletion of own device keys stored in the SCT.  */
/*  Note:					 	       */
/*  In case of sca_errno = ??, the purposes of the keys, which */
/*  are identical are returned.				       */ 	
/*							       */
/*						               */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   appl_id		       application identifier          */
/*							       */ 
/*							       */
/* OUT							       */
/*   key_set                   purposes of all deletable device*/
/*                             keys.                           */
/*                                                             */
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
/*  STAErrAnalyse		                               */
/*							       */
/*							       */
/*  aux_free2_OctetString				       */
/*							       */
/*-------------------------------------------------------------*/
int
sca_enable_deletion(sct_id, appl_id, key_set)
int             sct_id;
OctetString     *appl_id;
KeyPurpose	key_set[];
{

	int             i;
	int             rc;
	KeyId           key_id;
	ICC_SecMess	sec_mess;
	OctetString     init_appl_id;
	char		*proc="sca_enable_deletion";

	rc = 0;


	sca_errno = M_NOERR;
	sca_errmsg = NULL;

#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_enable_deletion *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id		        : %d\n", sct_id);
	print_OctetString("appl_id       		:\n",
			  appl_id);
#endif


/*****************************************************************************/
/*
 *      initialize return parameters
 */
	key_set[0] = '\0';

/*****************************************************************************/
/*
 *      Check input parameters
 */
	/* 
	 * check appl_id                                        
	 */
	if  ((appl_id == NULLOCTETSTRING) ||
	     (appl_id->noctets != 6) ||
             (appl_id->octets == NULL))
		goto parerrcase;

	if (memcmp(appl_id->octets,INIT_APPL_ID,6) == 0)
		goto parerrcase;


 
/*****************************************************************************/
/*
 *      call STACheckSCT_ICC 
 */
	if (STACheckSCT_ICC(sct_id, TRUE, proc) == -1)
		return (-1);


/*****************************************************************************/
/*
 *	1. step: activate INITIAL - devicekeyset
 */

	/* 
	 * set initial application identifier                   
	 */
	init_appl_id.noctets = 6;
	init_appl_id.octets  = INIT_APPL_ID;
	Gsct_request.rq_datafield.appl_id = &init_appl_id;

	Gsct_request.rq_p1.level = FILE_MASTER;

	Gsct_cmd = S_SET_APPL_ID;



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


/*****************************************************************************/
/*
 *	2. step: mutual authentication
 */
	Gsct_cmd = S_AUTH;
	Gsct_request.rq_p2.acp = MU_AUTH;
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

/*****************************************************************************/
/*
 *      3. step: pin verification mit KID = X'24' and 
 *		 secure messaging = 'concealed + auth' for command and response
 */
	Gsct_cmd = S_PERFORM_VERIFY;
	key_id.key_level = FILE_MASTER;
	key_id.key_number= 4;
	sec_mess.command = ICC_COMBINED;
	sec_mess.response= ICC_COMBINED;
	Gsct_request.rq_p1.kid = &key_id;
	Gsct_request.rq_datafield.sec_mode = &sec_mess;

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



/*****************************************************************************/
/*
 *	4. step: compare device keys (S_COMP_DEV_KEY)
 */
	Gsct_cmd = S_COMP_DEV_KEY;
	Gsct_request.rq_datafield.appl_id = appl_id;


	rc = sct_interface(sct_id, Gsct_cmd, &Gsct_request, &Gsct_response);
	if (rc < 0) {
		sca_errno = sct_errno;
		sca_errmsg = sct_errmsg;
		if ((!Gsct_response.noctets) ||
		    (Gsct_response.noctets > 3)) {
			/* length = 0 */ 
			STAErrAnalyse(sct_id);
			return (-1);
		};

	};
	for (i=0; i<Gsct_response.noctets; i++) 
		key_set[i] = *(Gsct_response.octets + i);
	key_set[i] = '\0';
	

	/*
 	 *     Release storage
         */

	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);

#ifdef TEST
	fprintf(stdout, "TRACE of the output parameters : \n");

	fprintf(stdout, "key_set                 : \n");
	for (i=0; i<4; i++) {
		if (key_set[i]) 
			print_KeyPurpose(key_set[i]);
		else {
			fprintf(stdout, "KeyPurpose              : NULL\n");
			break;
		}
	}

	fprintf(stdout, "\n***** Normal end of   sca_enable_deletion *****\n\n");
#endif


	return (rc);

/*****************************************************************************/
/*
 *      Parameter error
 */
parerrcase:
	sca_errno = EPARINC;
	sca_errmsg = sca_errlist[sca_errno].msg;
	return (-1);




}				/* end sca_enable_deletion */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_enable_deletion    */
/*-------------------------------------------------------------*/


/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_activate_devkeyset    VERSION   1.1	       */
/*				     DATE   August  1993       */
/*			      	       BY   L.Eckstein         */
/*							       */
/* DESCRIPTION						       */
/* Activate own device key set in SCT	                       */
/*							       */
/*						               */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   appl_id		       application identifier          */
/*							       */ 
/*   key_level                 level of the ICC key            */
/*                                                             */
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
/*							       */
/*  STAErrAnalyse		                               */
/*							       */
/*							       */
/*  aux_free2_OctetString				       */
/*							       */
/*-------------------------------------------------------------*/
int
sca_activate_devkeyset(sct_id, appl_id, key_level)
	int             sct_id;
	OctetString	*appl_id;
	KeyLevel	key_level;
{
	int 		rc;
	OctetString     init_appl_id;
	char		*proc="sca_activate_devkeyset";


	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;

#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_activate_devkeyset *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id		        : %d\n", sct_id);
	print_OctetString("appl_id       		:\n",
			  appl_id);
	print_KeyLevel(key_level);

#endif

/*****************************************************************************/
/*
 *      Check input parameters and set parameters for S_SET_APPL_ID
 */
	if (appl_id != NULLOCTETSTRING) {
		if ((appl_id->noctets != 6) ||
		    (appl_id->octets == NULL) ||
		    ((key_level != FILE_MASTER) &&
	     	     (key_level != FILE_CURRENT) &&
	     	     (key_level != FILE_PARENT) &&
		     (key_level != INITKEY))) 
			goto parerrcase;

		Gsct_request.rq_datafield.appl_id = appl_id;

	}
	else {
		/* 
		 * set initial application identifier                   
		 */
		init_appl_id.noctets = 6;
		init_appl_id.octets  = INIT_APPL_ID;
		Gsct_request.rq_datafield.appl_id = &init_appl_id;
	}

	Gsct_request.rq_p1.level = key_level;

		


 
/*****************************************************************************/
/*
 *      call STACheckSCT_ICC 
 */
	if (STACheckSCT_ICC(sct_id, FALSE, proc) == -1)
		return (-1);


/*****************************************************************************/
/*
 *	Set APPL_ID (S_SET_APPL_ID )
 */
	Gsct_cmd = S_SET_APPL_ID;



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
	fprintf(stdout, "\n***** Normal end of   sca_activate_devkeyset *****\n\n");
#endif

	return (0);

/*****************************************************************************/
/*
 *      Parameter error
 */
parerrcase:
	sca_errno = EPARINC;
	sca_errmsg = sca_errlist[sca_errno].msg;
	return (-1);





}				/* end sca_activate_devkeyset */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_activate_devkeyset */
/*-------------------------------------------------------------*/

/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_save_devkeyset        VERSION   1.1	       */
/*				     DATE   January 1993       */
/*			      	       BY   L.Eckstein         */
/*							       */
/* DESCRIPTION						       */
/*  Write PIN and device keys (STATUS=DEV_ANY) from SCT in Keycard */
/*							       */
/*						               */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*                                                             */
/*   verify_data	       Structure which determines the  */
/*                             PIN to be installed             */
/*                                                             */
/*   key_attr_list             Structure which contains        */
/*                             additional information for      */
/*                             storing the verification data   */
/*                             on the ICC                      */
/*							       */
/*   appl_id		       application identifier          */
/*							       */ 
/*   save_key_set              List of KIDs of the any device  */
/*                             keys, which shall be stored     */
/*			       in the keycard (terminated by   */
/*			       the NULL pointer)	       */
/*                                                             */
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
/*  STACheckKeyAttrList        ERROR-Codes		       */
/*			         EPARINC 		       */
/*							       */
/*  icc_check_KeyId             ERROR-Codes		       */
/*			         EPARINC		       */
/*							       */
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
/*							       */
/*  STAErrAnalyse		                               */
/*							       */
/*							       */
/*  aux_free2_OctetString				       */
/*							       */
/*-------------------------------------------------------------*/
int
sca_save_devkeyset(sct_id, verify_data, key_attr_list, appl_id, save_key_set)
	int             sct_id;
	VerifyDataStruc *verify_data;
	KeyAttrList	*key_attr_list;
	OctetString     *appl_id;	
	KeyId           *save_key_set[];
{
	int             rc, i;
     	VERRecord       verinfo;
	OctetString     data_body;
	WriteKeycard    sct_writekey;
	KeyId           key_id;
	OctetString     init_appl_id;
	char		*proc="sca_save_devkeyset";

	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;

#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_save_devkeyset *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id		        : %d\n", sct_id);
	print_VerifyDataStruc(verify_data);
	print_KeyAttrList(key_attr_list);
	print_OctetString("appl_id       		:\n",
			  appl_id);
	print_KeyIdList(save_key_set);   
#endif

/*****************************************************************************/
/*
 *      Check input parameters
 */
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
	}
	else
		goto parerrcase;

	/*
	 * Check key_attr_list
	 */
	if (STACheckKeyAttrList(PIN_KEY, key_attr_list) == -1)
		return (-1);

	/* 
	 * check appl_id                                        
	 */
	if  ((appl_id == NULLOCTETSTRING) ||
	     (appl_id->noctets != 6) ||
             (appl_id->octets == NULL))
		goto parerrcase;

	/* 
	 * check save_key_set                                        
	 */
	if ((!save_key_set) ||
	    (!save_key_set[0]))
		goto parerrcase;

	i = 0;
	while (save_key_set[i]) {
		/*
	 	 * Check key_id				               
	 	 */
		if (icc_check_KeyId(save_key_set[i]) == -1)
			goto parerrcase;
		i++;
	}

	if (i > 3)
		goto parerrcase;	/* more than 3 KIDs specified */
		


	
 /*****************************************************************************/
/*
 *      call STACheckSCT_ICC
 */
	if (STACheckSCT_ICC(sct_id, TRUE, proc) == -1)
		return (-1);

/*****************************************************************************/
/*
 *	1. step: activate INITIAL - devicekeyset
 */

	/* 
	 * set initial application identifier                   
	 */
	init_appl_id.noctets = 6;
	init_appl_id.octets  = INIT_APPL_ID;
	Gsct_request.rq_datafield.appl_id = &init_appl_id;

	Gsct_request.rq_p1.level = FILE_MASTER;

	Gsct_cmd = S_SET_APPL_ID;



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



/*****************************************************************************/
/*
 *	2. step: mutual authentication
 */
	Gsct_cmd = S_AUTH;
	Gsct_request.rq_p2.acp = MU_AUTH;
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

/*****************************************************************************/
/*
 *	3. step: install PIN (S_INST_VERIFICATION_DATA)
 */

	Gsct_cmd = S_INST_VER_DATA;
	key_id.key_level = FILE_MASTER;
	key_id.key_number= 4;
	Gsct_request.rq_p1.kid = &key_id;

	if (verify_data->VerifyDataBody.pin_info.disengageable == TRUE)
		verinfo.key_algid = S_DISENGAGEABLE_PIN;
	else
		verinfo.key_algid = S_NON_DISENGAGEABLE_PIN;

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
	/*
 	 *      Normal End	 (Release storage)
         */
	data_body.noctets = 1;
	Gsct_response.noctets = 1;
	aux_free2_OctetString(&data_body);
	aux_free2_OctetString(&Gsct_response);



/*****************************************************************************/
/* 
 *	4. step: store device keys on keycard (S_WRITE_KEYCARD)
 */
	Gsct_cmd = S_WRITE_KEYCARD;
	sct_writekey.appl_id = appl_id;
	sct_writekey.keyid_list = save_key_set;
	Gsct_request.rq_datafield.write_keycard = &sct_writekey;

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
	fprintf(stdout, "\n***** Normal end of   sca_save_devkeyset *****\n\n");
#endif

	return (0);

/*****************************************************************************/
/*
 *      Parameter error
 */
parerrcase:
	sca_errno = EPARINC;
	sca_errmsg = sca_errlist[sca_errno].msg;
	return (-1);



}				/* end sca_save_devkeyset */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_save_devkeyset      */
/*-------------------------------------------------------------*/


/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_load_devkeyset       VERSION   1.1	    	       */
/*				     DATE   August 1993        */
/*			      	       BY   L.Eckstein         */
/*							       */
/* DESCRIPTION						       */
/*  Read device keyset  from Keycard in SCT                    */
/*							       */
/*						               */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*                                                             */
/*							       */
/* IN/OUT						       */
/*   key_dev_sel               Structure which determines the  */
/*                             device key.                     */
/*			       key_status must be set by       */
/*			       application;		       */
/*			       In case of DEV_OWN => key_purpose   */
/*			       will be returned.	       */
/*			       In case of DEV_ANY => key_id  will  */
/*			       be returned.		       */
/*							       */
/* OUT						               */
/*   appl_id		       application identifier          */
/*							       */ 
/*                                                             */
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
/*							       */
/*  STAErrAnalyse		                               */
/*							       */
/*  STAGetBits						       */
/*							       */
/*   malloc macro in MF_check.h				       */
/*							       */
/*  aux_free2_OctetString				       */
/*							       */
/*-------------------------------------------------------------*/
int
sca_load_devkeyset(sct_id, appl_id, load_key_set )
	int             sct_id;
	OctetString	*appl_id;
	KeyDevList	*load_key_set;
{
	int             rc, i;
	KeyId		key_id;
	KeyId		*ptr_key_id;
	ICC_SecMess	sec_mess;
	OctetString     init_appl_id;
	char		*proc="sca_load_savekeyset";

	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;


#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_load_devkeyset *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id		        : %d\n", sct_id);
	print_KeyDevList(load_key_set, FALSE);
#endif


/*****************************************************************************/
/*
 *      Check input parameters and initialize return parameters
 */
	/* 
	 * check appl_id                                        
	 */
	if  (appl_id == NULLOCTETSTRING) 
		goto parerrcase;
	else {
		appl_id->noctets = 0;
		appl_id->octets  = NULL;
	}

	if ((!load_key_set) ||
	    ((load_key_set->key_status != DEV_OWN) &&
	     (load_key_set->key_status != DEV_ANY)))
		goto parerrcase;
	if (load_key_set->key_status == DEV_OWN)
		*(load_key_set->list_ref.purpose_list) = '\0';
	else
		*(load_key_set->list_ref.keyid_list) = KEYIDNULL;


/*****************************************************************************/
/*
 *      call STACheckSCT_ICC 
 */
	if (STACheckSCT_ICC(sct_id, TRUE, proc) == -1)
		return (-1);



/*****************************************************************************/
/*
 *	1. step: activate INITIAL - devicekeyset
 */

	/* 
	 * set initial application identifier                   
	 */
	init_appl_id.noctets = 6;
	init_appl_id.octets  = INIT_APPL_ID;
	Gsct_request.rq_datafield.appl_id = &init_appl_id;

	Gsct_request.rq_p1.level = FILE_MASTER;

	Gsct_cmd = S_SET_APPL_ID;



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



/*****************************************************************************/
/*
 *	2. step: mutual authentication
 */
	Gsct_cmd = S_AUTH;
	Gsct_request.rq_p2.acp = MU_AUTH;
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

/*****************************************************************************/
/*
 *      3. step: pin verification mit KID = X'24' and 
 *		 secure messaging = 'concealed + auth' for command and response
 */
	Gsct_cmd = S_PERFORM_VERIFY;
	key_id.key_level = FILE_MASTER;
	key_id.key_number= 4;
	sec_mess.command = ICC_COMBINED;
	sec_mess.response= ICC_COMBINED;
	Gsct_request.rq_p1.kid = &key_id;
	Gsct_request.rq_datafield.sec_mode = &sec_mess;

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



/*****************************************************************************/
/*
 *	4. step: read device keys (S_READ_KEYCARD)
 */
	Gsct_cmd = S_READ_KEYCARD;
	Gsct_request.rq_p2.status = load_key_set->key_status;


	rc = sct_interface(sct_id, Gsct_cmd, &Gsct_request, &Gsct_response);

	if (rc < 0) {
		sca_errno = sct_errno;
		sca_errmsg = sct_errmsg;
		if ((!Gsct_response.noctets) ||
		    (Gsct_response.noctets < 6)) {
			/* length = 0 or incorrect length */ 
			STAErrAnalyse(sct_id);
			return (-1);
		};

	};

	/*
	 * allocate appl_id->octets
	 */

	appl_id->noctets = 6;
	ALLOC_CHAR(appl_id->octets, appl_id->noctets);

	for (i=0; i < 6; i++) 
		*(appl_id->octets + i) = *(Gsct_response.octets + i);


	/*
	 * return purposes or key_ids
	 */
	switch (load_key_set->key_status) {
		case DEV_OWN:
			/* set purpose_list */
			for (i=0; i<Gsct_response.noctets - 6; i++) 
				*(load_key_set->list_ref.purpose_list + i) =
				*(Gsct_response.octets + 6 + i);

			*(load_key_set->list_ref.purpose_list + i) = '\0';
			break;

		case DEV_ANY:
			/*
			 * allocate buffer for keyid_list and set return parameter
			 */

			for (i=0; i<Gsct_response.noctets - 6; i++) {
				if ((ptr_key_id = (struct KEYId *) malloc(sizeof(struct KEYId))) == KEYIDNULL) {
					sca_errno = EMEMAVAIL;
					sca_errmsg = sca_errlist[sca_errno].msg;
					goto errcase;
				}

				ptr_key_id->key_level = 
					STAGetBits((unsigned) Gsct_response.octets[6+i],3, 2);
				ptr_key_id->key_number =
					STAGetBits((unsigned) Gsct_response.octets[6+i], 5, 7);
				*(load_key_set->list_ref.keyid_list + i) = ptr_key_id;
				*(load_key_set->list_ref.keyid_list + i + 1) = KEYIDNULL;
			}



			break;
	};

	/*
 	 *     Release storage
         */

	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);

#ifdef TEST
	fprintf(stdout, "TRACE of the output parameters : \n");
	print_OctetString("appl_id       		:\n",
			  appl_id);
	print_KeyDevList(load_key_set,TRUE);


	fprintf(stdout, "\n***** Normal end of   sca_load_devkeyset *****\n\n");
#endif


	return (rc);
/*****************************************************************************/
/*
 *      Memory error
 */
errcase:
	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);
	return (-1);



/*****************************************************************************/
/*
 *      Parameter error
 */
parerrcase:
	sca_errno = EPARINC;
	sca_errmsg = sca_errlist[sca_errno].msg;
	return (-1);




}				/* end sca_load_devkeyset */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_load_devkeyset     */
/*-------------------------------------------------------------*/

















