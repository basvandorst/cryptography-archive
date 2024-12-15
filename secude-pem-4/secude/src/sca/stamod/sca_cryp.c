/*
 *  STARMOD Release 1.1 (GMD)
 */
/*---------------------------------------------------------------------------+-----*/
/*							                     | GMD */
/*   SYSTEM   STARMOD  -  Version 1.1		                             +-----*/
/*							                           */
/*							                           */
/*							                           */
/*							                           */
/*---------------------------------------------------------------------------------*/
/*							                           */
/*    PACKAGE	SCAIF                                         VERSION 1.1	   */
/*					                       DATE July   1993    */
/*					                         BY Levona Eckstein*/
/*					                                           */
/*			       				                           */
/*    FILENAME     					                           */
/*	sca_cryp.c                       		         		   */
/*							                           */
/*    DESCRIPTION	   				                           */
/*      This modul provides all cryptographic functions of the smartcard 	   */
/*	application interface.          				           */
/*										   */
/*    EXPORT		    DESCRIPTION 		                           */
/*	sca_gen_user_key()     Generate user key (DES or RSA)		           */
/*							                           */
/*	sca_inst_user_key()    Install user key (DES or RSA) in the smartcard      */
/*							                           */
/*	sca_challenge()        Get and/or send random number	                   */
/*							                           */
/*	sca_del_user_key()     Delete user key stored in an SCT	 	           */
/*							                           */
/*	sca_sign() 	       Sign octetstring	                                   */
/*										   */
/*	sca_verify_sig()       Verify digital signature				   */
/*							                           */
/*	sca_encrypt()	       Encrypt octetstring				   */
/*							                           */
/*	sca_decrypt()	       Decrypt octetstring				   */
/*							                           */
/*	sca_hash()	       Hash octetstring					   */
/*							                           */
/*	sca_enc_des_key()      Encrypt a DES key with the RSA algorithm            */
/*							                           */
/*	sca_dec_des_key()      Decrypt an rsa-encrypted DES key			   */
/*							                           */
/*                                                                                 */
/*    IMPORT		    DESCRIPTION 		                           */
/*	all functions and variables which are described in stamod.h                */
/*							                           */
/*    INTERNAL                                                                     */
/*      STAcompare()           compares two strings (independent of '\0')          */
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
/*   local function declarations			       */
/*-------------------------------------------------------------*/
static int      STAcompare();





/*-------------------------------------------------------------*/
/*   type definitions					       */
/*-------------------------------------------------------------*/
typedef enum {
	F_null, F_encrypt, F_decrypt,
	F_hash, F_sign, F_verify
}               FTYPE;


/*-------------------------------------------------------------*/
/*   macro definitions					       */
/*-------------------------------------------------------------*/
#define ALLOC_CHAR(v,s)  {if (0 == (v = malloc(s))) {sca_errno = EMEMAVAIL; sca_errmsg = sca_errlist[sca_errno].msg; goto errcase;}}

#define ALLOC_OCTET(v,t)        {if (0 == (v = (t *)malloc(sizeof(t)))) {sca_errno =  EMEMAVAIL; sca_errmsg = sca_errlist[sca_errno].msg; goto errcase;}}



/*-------------------------------------------------------------*/
/*   local  variable definitions			       */
/*-------------------------------------------------------------*/
static Boolean  icc_expect;	/* = TRUE indicates: ICC expected    */

 /* = FALSE indicates: ICC not needed */
static char     fermat_f4[3] = {'\001', '\000', '\001'};	/* public exponent                  */
static int      fermat_f4_len = 3;



static FTYPE    act_function = F_null;	/* used if encrypt, decrypt, sign   */
					/* or verify are called with more = */
					/* SEC_MORE                         */






/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_gen_user_key         VERSION   1.1	    	       */
/*				     DATE   June 1993	       */
/*			      	       BY   L. Eckstein        */
/*							       */
/* DESCRIPTION						       */
/*  Generate user key (DES or RSA)		               */
/*  Note:						       */
/*  In the current version only the RSA keysizes 512 and 1024  */
/*  will be supported.					       */	
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   key_sel                   Structure which determines the  */
/*                             generated key.                  */
/*                                                             */
/*							       */
/* OUT							       */
/*   key_sel->key_bits         In case of RSA, the public key  */
/*			       is returned. Memory is provided */
/*			       by this function and must be    */
/*                             released by calling routine.    */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*   1                           M_KEYREPL                     */
/*  -1			       error			       */
/*				 EPARINC 		       */
/*				 EMEMAVAIL		       */
/*				 EKEYLEN		       */
/*				 EALGO			       */
/*							       */
/* CALLED FUNCTIONS					       */
/*  STACheckSCT_ICC               ERROR-Codes		       */
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
/*  STAGetSCTAlgId              ERROR-Codes		       */
/*                               EALGO                         */
/*                               EKEYLEN                       */
/*							       */
/*  STACheckRSAKeyId          ERROR-Codes		       */
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
/*  STAErrAnalyse		         	               */
/*							       */
/*  aux_free2_OctetString				       */
/*  aux_cmp_ObjId					       */
/*							       */
/*   malloc macro in MF_check.h			               */ 
/*-------------------------------------------------------------*/
int
sca_gen_user_key(sct_id, key_sel)
	int             sct_id;
	KeySel         *key_sel;
{
	int             i, algorithm;
	int		*ptr_parm_int;
	int             rc;
	KeyAlgId        sct_algid;	/* SCT specific alg_id	    */
	KeyId		icc_keyid;
	char		*proc="sca_gen_user_key";

	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;

#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_gen_user_key *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id		        : %d\n", sct_id);
	print_KeySel(key_sel,FALSE);

#endif



/*****************************************************************************/
/*
 *      Initialize return parameter 
 */
	if ((key_sel ) && (key_sel->key_bits)) {
		key_sel->key_bits->part1.noctets = 0;
		key_sel->key_bits->part1.octets  = NULL;
		key_sel->key_bits->part2.noctets = 0;
		key_sel->key_bits->part2.octets  = NULL;
		key_sel->key_bits->part3.noctets = 0;
		key_sel->key_bits->part3.octets  = NULL;
		key_sel->key_bits->part4.noctets = 0;
		key_sel->key_bits->part4.octets  = NULL;
	}




/*****************************************************************************/
/*
 *      Check input parameters
 */
	if (!key_sel) 
		goto parerrcase;

	if ((sct_algid = STAGetSCTAlgId(key_sel->key_algid)) == -1)
		return (-1);


	/*
	 * if sct_algid = S_RSA_F4,
	 * only the algorithm identifier rsa and sqmodnWithRsa is allowed,
	 * the keysizes 512 and 1024 are only allowed and			       
	 * key_sel->key_bits must be valid for           
	 * the returned public key.     	              
	 */
	if (sct_algid == S_RSA_F4) {
		if ((aux_cmp_ObjId(key_sel->key_algid->objid,rsa->objid)) &&
		    (aux_cmp_ObjId(key_sel->key_algid->objid,sqmodnWithRsa->objid))) {
			sca_errno = EALGO;
			sca_errmsg = sca_errlist[sca_errno].msg;
			return (-1);
		}


		if  (key_sel->key_bits == NULL)
			goto parerrcase;


		ptr_parm_int = (int *) (key_sel->key_algid->param);

		if ((*ptr_parm_int != 512) &&
		    (*ptr_parm_int != 1024)) {
			sca_errno = EKEYLEN;
			sca_errmsg= sca_errlist[sca_errno].msg;
			return (-1);
		}






	    	if (STACheckRSAKeyId(key_sel->key_id, &icc_keyid) == -1)
			return(-1);

 


	}


/*****************************************************************************/
/*
 *      call STACheckSCT_ICC
 */
	if (STACheckSCT_ICC(sct_id, FALSE, proc) == -1)
		return (-1);

/*****************************************************************************/
/*
 *      Generate key (S_GEN_USER_KEY)
 *	Prepare parameters for the SCT Interface           
 * 	At the SCA-IF the keysize is given in bits, at the SCT-IF the keysize
 *	is delivered in octets,therefore the keysize in bits is divided by 8.
 */
	Gsct_cmd = S_GEN_USER_KEY;
	Gsct_request.rq_p2.algid = sct_algid;
	if (sct_algid == S_RSA_F4) {
		Gsct_request.rq_p1.kid = &icc_keyid;
		Gsct_request.rq_datafield.keylen = RSA_PARM(key_sel->key_algid->param) / 8;
	}
	else {
		Gsct_request.rq_p1.kid = key_sel->key_id;
		Gsct_request.rq_datafield.keylen = 0;
	}

#ifdef TEST
	if (sct_algid == S_RSA_F4)
		fprintf(stdout, "keysize of RSA key: %d\n",
			RSA_PARM(key_sel->key_algid->param));
#endif



/*****************************************************************************/
/*
 *      call SCT Interface
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
 *      Prepare return parameters
 */
	/*
	 * If an existing key in the SCT has been overwritten  
	 * then return (warning)			       
	 */
	if (rc == S_KEYREPL)
		sca_errno = M_KEYREPL;

	/*
	 * If sct_algid = S_RSA_F4  			       
	 * then get modulus from SCT response and              
	 * construct public key (modulus, Fermat-F4)           
	 * and return pk in key_sel->key_bits                  
	 */
	if (sct_algid == S_RSA_F4) {
		key_sel->key_bits->part1.noctets = Gsct_response.noctets;
		ALLOC_CHAR(key_sel->key_bits->part1.octets, Gsct_response.noctets);

		for (i = 0; i < Gsct_response.noctets; i++)
			key_sel->key_bits->part1.octets[i] = Gsct_response.octets[i];

		/* get fermat-f4 as public exponent */
		key_sel->key_bits->part2.noctets = 3;
		ALLOC_CHAR(key_sel->key_bits->part2.octets, 3);
		memcpy(key_sel->key_bits->part2.octets, fermat_f4, 3);

#ifdef TEST
		fprintf(stdout, "TRACE of the output parameters : \n");
		fprintf(stdout, "key_sel->key_bits->part1 (modulus) :\n");
		fprintf(stdout, "    noctets                        : %d\n",
			 	key_sel->key_bits->part1.noctets );
		fprintf(stdout, "    octets                         : \n");
		aux_fxdump(stdout, key_sel->key_bits->part1.octets,
				   key_sel->key_bits->part1.noctets, 0);
		fprintf(stdout, "\n");
		fprintf(stdout, "key_sel->key_bits->part2 (exponent):\n");
		fprintf(stdout, "    noctets                        : %d\n",
				key_sel->key_bits->part2.noctets);
		fprintf(stdout, "    octets                         : \n");
		aux_fxdump(stdout, key_sel->key_bits->part2.octets,
				   key_sel->key_bits->part2.noctets, 0);
		fprintf(stdout, "\n");
#endif
	}


/*****************************************************************************/
/*
 *      Release storage
 */

	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);


#ifdef TEST
	fprintf(stdout, "\n***** Normal end of   sca_gen_user_key *****\n\n");
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



/*****************************************************************************/
/*
 *      Memory error
 */
errcase:
	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);
	return (-1);

}				/* end sca_gen_user_key */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_gen_user_key       */
/*-------------------------------------------------------------*/

/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_inst_user_key        VERSION   1.1	    	       */
/*				     DATE   March 1993	       */
/*			      	       BY   L. Eckstein        */
/*							       */
/* DESCRIPTION						       */
/*  Install user key (DES or RSA)		               */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   key_sel                   Structure which determines the  */
/*                             key to be installed.            */
/*			       only the parameter              */
/*			       key_sel->key_id                 */
/*                             is evaluated                    */
/*                                                             */
/*   key_attr_list             Structure which contains        */
/*                             additional information for      */
/*                             storing the generated key on    */
/*			       the ICC                         */
/*							       */
/* OUT							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*				 EMEMAVAIL		       */
/*				 EPARINC  		       */ 
/*							       */
/* CALLED FUNCTIONS					       */
/*  STACheckSCT_ICC               ERROR-Codes		       */
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
/*			         EPARINC		       */
/*							       */
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
/*  STAErrAnalyse		        	               */
/*							       */
/*  aux_free2_OctetString				       */
/*  aux_cmp_ObjId					       */
/*							       */
/*-------------------------------------------------------------*/
int
sca_inst_user_key(sct_id, key_sel, key_attr_list)
	int             sct_id;
	KeySel         *key_sel;
	KeyAttrList    *key_attr_list;
{
	int             i, algorithm;
	int             rc;
	KeyAlgId        sct_algid;	/* SCT specific alg_id	    */
	KeyId		icc_keyid;
	Boolean		des_key;
	ICC_SecMess     sec_mess;
	char		*proc="sca_inst_user_key";

	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;

#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_inst_user_key *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id		        : %d\n", sct_id);
	if (key_sel) {
		fprintf(stdout, "Key_sel->key_id	: \n");
		print_KeyId(key_sel->key_id);
	}
	else
		fprintf(stdout, "KeySel  	        : NULL\n");
	print_KeyAttrList(key_attr_list);
#endif




/*****************************************************************************/
/*
 *      Check input parameters
 */


	if ((!key_sel) ||
	    (!key_sel->key_id)) {
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		return (-1);
	}
 

	if (STACheckKeyAttrList(USER_KEY, key_attr_list) == -1)
		return (-1);

	/*
	 * in case of RSA set key_id to the corresponding value  
	 */
	des_key = FALSE;
	switch (key_sel->key_id->key_level) {
	case FILE_MF:
		icc_keyid.key_level = 0x04;
		break;
	case FILE_DF:
		icc_keyid.key_level = 0x05;
		break;
	case FILE_CHILD:
		icc_keyid.key_level = 0x06;
		break;
	default:
		des_key = TRUE;
		icc_keyid.key_level = key_sel->key_id->key_level;
		break;
	}

	icc_keyid.key_number = key_sel->key_id->key_number;





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
	if ((key_attr_list->key_inst_mode == INST) &&
	    (des_key == TRUE)) {

		/*
 		 *      create ICC command 
 		 */
      		sec_mess.command = ICC_SEC_NORMAL;
      		sec_mess.response= ICC_SEC_NORMAL;
		if (STACreateHead(ICC_KEYSTAT, &sec_mess))
			return (-1);

		/* set parameters			  */
		PAR_KID 	= &icc_keyid;

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
	Gsct_cmd = S_INST_USER_KEY;
	Gsct_request.rq_datafield.keyattrlist = key_attr_list;
	Gsct_request.rq_p1.kid = &icc_keyid;

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
	fprintf(stdout, "\n***** Normal end of   sca_inst_user_key *****\n\n");
#endif

	return (sca_errno);


}				/* end sca_inst_user_key */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_inst_user_key      */
/*-------------------------------------------------------------*/

/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_challenge	          VERSION   1.1	    	       */
/*				     DATE   March 1993	       */
/*			      	       BY   L. Eckstein        */
/*							       */
/* DESCRIPTION						       */
/*  Get random number from SCT			               */
/*  A smartcard will not expected.			       */
/*  OR							       */
/*  Get and/or send  random number from / to smartcard         */
/*  A smartcard will expected			               */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   destination  	       Identifier from where the       */
/*			       random number shall be got      */
/*							       */
/*   mode                      in case of destination = ICC    */
/*			       this parameter specifies if a   */
/*                             random number shall be got and/ */
/*			       or sent			       */
/*							       */
/*   send_challenge            Data to be written              */
/*			       (will only be evaluated in case */
/*			        of destination=ICC    and      */
/*			        mode=SEND_/EXCHANGE_CHALLENGE) */
/*							       */
/*   sec_mess		       security modes                  */
/*			       (will only be evaluated in case */
/*				of destination = ICC   )       */  
/* OUT							       */
/*   rec_challenge             Buffer where the returned data  */
/*			       will be stored		       */
/*			       rec_challenge->octets is        */
/*			       provides by this function       */
/*							       */
/*							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*                               EPARINC 		       */
/*                               EMEMAVAIL		       */
/*							       */
/* CALLED FUNCTIONS					       */
/*  STACheckSCT_ICC               ERROR-Codes		       */
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
/*  STACreateHead         	ERROR-Codes    		       */
/*                               EPARINC 		       */
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
/*							       */
/*  aux_free2_OctetString				       */
/*							       */
/*  STAErrAnalyse		        	               */
/*							       */
/*							       */
/*   malloc macro in MF_check.h			               */ 
/*-------------------------------------------------------------*/
int
sca_challenge(sct_id, destination, mode, rec_challenge, send_challenge, sec_mess)
	int             sct_id;
	Destination	destination;
	ChallengeMode  	mode;
	OctetString	*rec_challenge;
	OctetString	*send_challenge;
	ICC_SecMess      *sec_mess;

{
	int             rc, i;
	char		*proc="sca_challenge";

	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;

#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_challenge *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id		        : %d\n", sct_id);
	print_Destination(destination);
	print_ChallengeMode(mode);
	if ((destination == ICC) && (mode != GET_CHALLENGE)) 
		print_OctetString("send_challenge		:\n",
			   	  send_challenge);
	if (destination == ICC)
		print_ICC_SecMess(sec_mess);

#endif

/*****************************************************************************/
/*
 *      Check input parameters
 */
	if ((destination != SCT) && (destination != ICC))
		goto parerrcase;


	if (destination == ICC) {

		if ((mode != GET_CHALLENGE) &&
            	    (mode != SEND_CHALLENGE) &&
            	    (mode != EXCHANGE_CHALLENGE))
			goto parerrcase;
 
		if ((mode != SEND_CHALLENGE) &&
	    	    (rec_challenge == NULLOCTETSTRING))
			goto parerrcase;

		if ((mode != GET_CHALLENGE) &&
	            ((send_challenge == NULLOCTETSTRING) ||
	     	     (send_challenge->noctets != 8) ||
	    	     (send_challenge->octets == NULL)))
			goto parerrcase;
		icc_expect = TRUE;
	}
	else {
		if ((rec_challenge == NULLOCTETSTRING) || 
	            (rec_challenge->noctets < 1) ||
	            (rec_challenge->noctets > 240))
			goto parerrcase;

		icc_expect = FALSE;
	}


	rec_challenge->octets = NULL;


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
	switch (destination) {
	case SCT:
		Gsct_cmd = S_GET_RNO;
		Gsct_request.rq_p1.lrno = rec_challenge->noctets;

		/*
		 * Call SCT Interface     			       
		 */
		rc = sct_interface(sct_id, Gsct_cmd, &Gsct_request, &Gsct_response);
		if (rc < 0) {
			sca_errno = sct_errno;
			sca_errmsg = sct_errmsg;
			STAErrAnalyse(sct_id);
			return (-1);
		}
		/*
		 * Normal End => generate octets and Release Gsct_response 
		 */
		ALLOC_CHAR(rec_challenge->octets, Gsct_response.noctets);

		rec_challenge->noctets = Gsct_response.noctets;
		for (i = 0; i < rec_challenge->noctets; i++)
			*(rec_challenge->octets + i) = *(Gsct_response.octets + i);

		Gsct_response.noctets = 1;
		aux_free2_OctetString(&Gsct_response);

#ifdef TEST
		fprintf(stdout, "TRACE of the output parameters : \n");
		fprintf(stdout, "rec_challenge           : \n");
		fprintf(stdout, "    noctets             : %d\n",
		        rec_challenge->noctets);
		fprintf(stdout, "    octets              : \n");
		aux_fxdump(stdout, rec_challenge->octets, rec_challenge->noctets, 0);
		fprintf(stdout, "\n***** Normal end of   sca_challenge *****\n\n");
#endif


		return (0);

	case	ICC:
		if (mode == GET_CHALLENGE) {
			/*
			 * create ICC command GET CHALLENGE     
			 */
			if (STACreateHead(ICC_GETCHAL, sec_mess))
				return (-1);
		}
		else {
			/*
			 * create ICC command EXCHANGE CHALLENGE
			 */
			if (STACreateHead(ICC_EXCHAL, sec_mess))
				return (-1);
			PAR_DIRECTION		 = mode;
			PAR_DATA		 = send_challenge;

		}


		/* call STACreateTrans			  */
		if (STACreateTrans(sct_id, TRUE))
			return (-1);

		if (mode != SEND_CHALLENGE) {
			/*
			 * Normal End => generate octets and Release response     
			 */
			ALLOC_CHAR(rec_challenge->octets, Gsct_response.noctets);
			rec_challenge->noctets = Gsct_response.noctets;
			for (i = 0; i < rec_challenge->noctets; i++)
				*(rec_challenge->octets + i) = *(Gsct_response.octets + i);

#ifdef TEST
			fprintf(stdout, "TRACE of the output parameters : \n");
			fprintf(stdout, "rec_challenge           : \n");
			fprintf(stdout, "    noctets             : %d\n",
		        	rec_challenge->noctets);
			fprintf(stdout, "    octets              : \n");
			aux_fxdump(stdout, rec_challenge->octets,
					   rec_challenge->noctets, 0);
#endif
		}

		/*
		 * normal end => release response storage       
		 */
		Gsct_response.noctets = 1;
		aux_free2_OctetString(&Gsct_response);

#ifdef TEST
		fprintf(stdout, "\n***** Normal end of   sca_challenge *****\n\n");
#endif
		return(0);
		break;




	} /* end of switch */

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






}				/* end sca_challenge */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_challenge	       */
/*-------------------------------------------------------------*/





/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_del_user_key         VERSION   1.1	    	       */
/*				     DATE   March 1993	       */
/*			      	       BY   L. Eckstein        */
/*							       */
/* DESCRIPTION						       */
/*  Delete user key, which is stored in the SCT		       */
/*							       */
/*  A smartcard is not expected.			       */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   key_sel                   Structure which determines the  */
/*                             key to be deleted.              */
/*			       only the parameter              */
/*			       key_sel->key_id                 */
/*                             is evaluated                    */
/*                                                             */
/*							       */
/* OUT							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*				EPARINC			       */
/*							       */
/* CALLED FUNCTIONS					       */
/*  STACheckSCT_ICC               ERROR-Codes		       */
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
/*  STAErrAnalyse		        	               */
/*							       */
/*  aux_free2_OctetString				       */
/*  aux_cmp_ObjId					       */
/*							       */
/*-------------------------------------------------------------*/
int
sca_del_user_key(sct_id, key_sel)
	int             sct_id;
	KeySel          *key_sel;
{
	int             i;
	int             rc;
	KeyAlgId        sct_algid;	/* SCT specific alg_id	    */
	KeyId		icc_keyid;
	char		*proc="sca_del_user_key";

	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;

#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_del_user_key *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id		        : %d\n", sct_id);

	if (key_sel) {
		fprintf(stdout, "Key_sel->key_id	: \n");
		print_KeyId(key_sel->key_id);
	}
	else
		fprintf(stdout, "KeySel  	        : NULL\n");

#endif

/*****************************************************************************/
/*
 *      Check input parameters
 */

	if ((!key_sel) ||
	    (!key_sel->key_id)) {
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		return(-1);
	}


	/*
	 * in case of RSA set key_id to the corresponding value  
	 */
	switch (key_sel->key_id->key_level) {
	case FILE_MF:
		icc_keyid.key_level = 0x04;
		break;
	case FILE_DF:
		icc_keyid.key_level = 0x05;
		break;
	case FILE_CHILD:
		icc_keyid.key_level = 0x06;
		break;
	default:
		icc_keyid.key_level = key_sel->key_id->key_level;
		break;
	}

	icc_keyid.key_number = key_sel->key_id->key_number;



/*****************************************************************************/
/*
 *      call STACheckSCT_ICC
 */
	if (STACheckSCT_ICC(sct_id, FALSE, proc) == -1)
		return (-1);

/*****************************************************************************/
/*
 *      Prepare parameters for the SCT Interface
 */

	Gsct_cmd = S_DEL_USER_KEY;
	Gsct_request.rq_p1.kid = &icc_keyid;

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
	fprintf(stdout, "\n***** Normal end of   sca_del_user_key *****\n\n");
#endif

	return (sca_errno);

}				/* end sca_del_user_key */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_del_user_key       */
/*-------------------------------------------------------------*/

/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_sign                 VERSION   1.1	    	       */
/*				     DATE   July 1993	       */
/*			      	       BY   L. Eckstein        */
/*							       */
/* DESCRIPTION						       */
/*  Sign octetstring 				               */
/*  The signature algorithm is taken from                      */
/*  signature_key->key_algid.				       */
/*  If key_algid = NULL the algorithm 'sqmodnWithRsa' and      */
/*  keysize = 512 bit is automatically used.                   */  
/*							       */
/*  Smartcard must be inserted.				       */
/*							       */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   out_octets		       Octetstring of the data to be   */
/*                             signed.                         */
/*                                                             */
/*   signature_key             Structure of the sigature key   */
/*			       signature_key->key_algid must be*/
/*			       set to NULL or specifies a      */
/*			       signature algorithm.	       */
/*			       In case of NULL the algorithm   */
/*			       identifier Square-Mod-NwithRsa  */
/*			       will be used.		       */
/*			       signature_key->key_id must      */
/*			       specify a key stored in the ICC */ 	 		
/*                                                             */
/*   more		       = SEC_MORE -> more data is expected */
/*   			       = SEC_END  -> Last data for this*/
/*				             sign process.     */
/*                                                             */
/*   hash_par                  Additional algorithm (hash_alg) */
/*                             specific parameters or the NULL */
/*                             pointer.                        */
/*			       Is only evaluated in case of    */
/*			       signature algorithm =           */
/*			       Square-Mod-NwithRsa.            */
/*							       */
/* OUT							       */
/*   signature                 Algorithm to be used and        */
/*			       returned signature (memory is   */
/*                             provided by the called program) */
/*                                                             */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*				 EPARINC		       */
/*				 EALGO  		       */
/*				 EMEMAVAIL 		       */
/*				 EHASHFUNC		       */
/*				 EFUNCTION		       */
/*				 EKEY   		       */
/*				 EPKCS   		       */
/*							       */
/* CALLED FUNCTIONS					       */
/*  STACheckSCT_ICC             ERROR-Codes		       */
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
/*  STACheckRSAKeyId          ERROR-Codes		       */
/*			         EPARINC		       */
/*							       */
/*  rsa_set_key		       ERROR-Codes		       */
/*				 -1 -> EKEY                    */
/*							       */
/*  hash_sqmodn 	       ERROR-Codes     	               */
/*				 -1 -> EHASHFUNC               */
/*                                                             */
/*  md2_hash	               ERROR-Codes      	       */
/*				 -1 -> EHASHFUNC               */
/*                                                             */
/*  md4_hash	       	       ERROR_Codes      	       */
/*				 -1 -> EHASHFUNC               */
/*                                                             */
/*  md5_hash	               ERROR-Codes      	       */
/*				 -1 -> EHASHFUNC               */
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
/*  STAErrAnalyse		        	               */
/*							       */
/*  aux_free2_OctetString				       */
/*  aux_free_OctetString		    	               */
/*  aux_ObjId2Name					       */
/*  aux_ObjId2AlgHash					       */	
/*  aux_ObjId2AlgType					       */	
/*  aux_ObjId2AlgSpecial				       */	
/*  aux_ObjId2ParmType					       */
/*  aux_create_PKCS_MIC_D				       */
/*  aux_create_PKCSBlock				       */	
/*							       */
/*   malloc macro in MF_check.h			               */ 
/*-------------------------------------------------------------*/
int
sca_sign(sct_id, out_octets, signature_key, hash_par, more, signature)
	int             sct_id;
	OctetString    *out_octets;
	KeySel         *signature_key;
	HashPar        *hash_par;
	More            more;
	Signature      *signature;
{

	int             	rc, i;
	char			*aux_ptr;
	static Boolean		already_call;
	static KeyId		icc_keyid;	
	static AlgHash  	alghash;
	static AlgSpecial  	algspecial;
	static int		hash_length;
	static OctetString 	*hash_result;
	static AlgId		*sign_algid;
	OctetString		dummy_octets;
	OctetString     	*encodedDigest;
	char			*proc="sca_sign";





	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;
	

#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_sign *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id		        : %d\n", sct_id);
	print_OctetString("out_octets    		:\n",
			   out_octets);


	if (act_function == F_null) {	/* first call of sca_sign */
		fprintf(stdout, "First call of sca_sign / trace signature_key \n");
		if (signature_key) {
			fprintf(stdout, "signature_key           : \n");

			if ((!signature_key->key_algid) ||
		   	    (!signature_key->key_algid->objid))
				fprintf(stdout, "key_algid               : NULL\n");
			else {
				aux_ptr = aux_ObjId2Name(signature_key->key_algid->objid);
				fprintf(stdout, "key_algid               : %s\n", aux_ptr);
				free(aux_ptr);
			}
			print_KeyId(signature_key->key_id);


		}
		else
			fprintf(stdout, "signature_key           : NULL\n");

		print_HashPar(hash_par);

	}
	else {
		fprintf(stdout, "Next call of sca_sign / trace internal signature_key \n");
		fprintf(stdout, "signature_key           : \n");

		aux_ptr = aux_ObjId2Name(sign_algid->objid);
		fprintf(stdout, "key_algid               : %s\n", aux_ptr);
		free(aux_ptr);
		print_KeyId(&icc_keyid);
		fprintf(stdout, "hash_par                : is not evaluated\n");


	}

	
	print_More(more);

#endif


/*****************************************************************************/
/*
 *      if first call of sign function then
 *	1) check signature_key-> key_id
 *	2) check signature_key->key_algid  
 * 	3) if algorithm = RSA-sqmodn then               
 *  	   a) check hash-parameter         
 * 	   b) set key (modulus) for hash-function 
 *         if algorithm = MDxWithRSAEncryption
 *	   a) check signature->key_bits->nbits                    
 * 	4) allocate storage for hash result 
 *	else
 *	   check input parameter       
 */
	if (act_function == F_null) {	/* first call of sca_sign */
		/*
		 * initialize static variables
		 */
		already_call = FALSE;
		alghash      = NoAlgHash;
		sign_algid   = NULL;
		hash_length  = 0;
		hash_result  = NULLOCTETSTRING;
		encodedDigest= NULLOCTETSTRING; 

		/*
	 	 * check signature_key            
	 	 * if  signature_key->key_algid = NULL pointer               
	 	 * then take sqmodnWithRsa  
	 	 * to the corresponding values	       
	 	 */
		if ((!signature_key) ||
		    (STACheckRSAKeyId(signature_key->key_id, &icc_keyid) == -1)) {
			sca_errno = EPARINC;
			sca_errmsg = sca_errlist[sca_errno].msg;
			goto errcase;
		}



		if (signature_key->key_algid == NULL) {
			sign_algid        = sqmodnWithRsa;
			/*
		         * set hash_length on default value = 512 Bit
			 */ 
			hash_length 	  = 64;
		}
 
		else {
			if (!signature_key->key_algid->objid) {
				sca_errno = EPARINC;
				sca_errmsg = sca_errlist[sca_errno].msg;
				goto errcase;
			}

			if (aux_ObjId2AlgType(signature_key->key_algid->objid) != SIG) {
				sca_errno = EALGO;
				sca_errmsg = sca_errlist[sca_errno].msg;
				goto errcase;
			}

			if ((aux_ObjId2ParmType(signature_key->key_algid->objid) == PARM_NULL) &&
		    	    (signature_key->key_algid->param)) {
				sca_errno = EPARINC;
				sca_errmsg = sca_errlist[sca_errno].msg;
				goto errcase;
			}

			sign_algid = signature_key->key_algid;

		}


		/*
	  	 * get hash algorithm
		 */

		alghash = aux_ObjId2AlgHash(sign_algid->objid);
		algspecial = aux_ObjId2AlgSpecial(sign_algid->objid);
	

		if (alghash == SQMODN) {
			if (hash_par == NULL) {
				sca_errno = EPARINC;
				sca_errmsg = sca_errlist[sca_errno].msg;
				goto errcase;
			}
			if (hash_par->sqmodn_par.part1.octets == NULL) {
				sca_errno = EPARINC;
				sca_errmsg = sca_errlist[sca_errno].msg;
				goto errcase;
			}
			/* set key in an internal function for hash-function */
			rc = rsa_set_key(&hash_par->sqmodn_par, 0);
			if (rc < 0) {
				sca_errno = EKEY;
				sca_errmsg = sca_errlist[sca_errno].msg;
				goto errcase;
			}
			if (signature_key->key_algid != NULL)
				/*
		         	 * set hash_length on alg_id specific value
			 	 */ 
 				hash_length = (RSA_PARM(sign_algid->param) + 7) / 8;
		}
		else
			/*
			 * MD2, MD4, MD5 => length of hash string = 128 bit
			 */
			hash_length = 16;


#ifdef TEST
		fprintf(stdout,"hash_length = %d\n",hash_length);
#endif

		/*
         	 * allocate storage for hash result
		 */
		ALLOC_OCTET(hash_result, OctetString);
		hash_result->noctets = 0;
		ALLOC_CHAR(hash_result->octets, hash_length);

		act_function = F_sign;




	}
	/* end if (act_function == F_null) */
	else {
	 	/* not first call of sca_sign */ 
		if (act_function != F_sign) {
			sca_errno = EFUNCTION;	/* wrong function call */
			sca_errmsg = sca_errlist[sca_errno].msg;
			goto errcase;
		}
	}



/*****************************************************************************/
/*
 *      Check input parameters
 */

	if ((!out_octets) ||
	    ((out_octets->noctets > 0) &&
	     (!out_octets->noctets))) { 
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		goto errcase;
	}


	if ((more != SEC_END) && (more != SEC_MORE)) {
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		goto errcase;
	}


/*****************************************************************************/
/*
 * 	Call hash function depending on algorithm          
 */
	switch (alghash) {
	case SQMODN:
		rc = hash_sqmodn(out_octets, hash_result, more, hash_length * 8);
		break;
	case MD2:
		rc = md2_hash(out_octets, hash_result, more);
		break;
	case MD4:
		rc = md4_hash(out_octets, hash_result, more);
		break;
	case MD5:
		rc = md5_hash(out_octets, hash_result, more);
		break;
	default:
		sca_errno = EALGO;
		sca_errmsg = sca_errlist[sca_errno].msg;
		goto errcase;


	}			/* end switch */
	if (rc < 0) {
		sca_errno = EHASHFUNC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		goto errcase;
	}
	else {
		/*
		 * in case of SEC_MORE, mark, that the hash function was
		 * already successfully called
		 */
		if (more == SEC_MORE)
			already_call = TRUE;
	}



/*****************************************************************************/
/*
 *      Now hashing is done			      
 * 	If last call of sca_sign ( more = SEC_END)  then call sct-interface
 */

	if (more == SEC_END) {	/* last call of sca_sign */
		already_call = FALSE;	/* hash function correctly ended */

		/* 
		 * check output parameter for signature    	          
		 */
		if (!signature) {
			sca_errno = EPARINC;
			sca_errmsg = sca_errlist[sca_errno].msg;
			goto errcase;
		}

		/*
 		 *      call STACheckSCT_ICC
 		 */
		if (STACheckSCT_ICC(sct_id, TRUE, proc) == -1)
			goto errcase;

		if(algspecial == PKCS_BT_01) {
			/*
			 * hash-value must have the same length as the modulus
			 * therefore the global variable public_modulus_length    
			 * (declared in sec_init.c) is set 
			 * it is used in aux_create_PKCSBlock to compute the
			 * blocksize
			 */

			if ((signature_key->key_bits == NULL) ||
			    (signature_key->key_bits->part1.noctets == 0))
				public_modulus_length = 512;	/* default value */
			else
				public_modulus_length = signature_key->key_bits->part1.noctets * 8;

			if (hash_result->noctets < public_modulus_length / 8) {

				/*  Here goes PKCS#1 ...   */

				encodedDigest = aux_create_PKCS_MIC_D(hash_result, sign_algid);
				if (!encodedDigest) {
					sca_errno = EPKCS;
					sca_errmsg = sca_errlist[sca_errno].msg;
					goto errcase;
				}

				if(hash_result) aux_free_OctetString(&hash_result);		
				hash_result = aux_create_PKCSBlock(1, encodedDigest);
				if (!hash_result) {
					sca_errno = EPKCS;
					sca_errmsg = sca_errlist[sca_errno].msg;
					goto errcase;
				}
				if(encodedDigest) aux_free_OctetString(&encodedDigest);
			}

       		}


#ifdef TEST
		fprintf(stdout, "hash result: \n");
		aux_fxdump(stdout, hash_result->octets, hash_result->noctets, 0);
#endif
		/*
		 * Prepare parameters for the SCT Interface          
		 */
		Gsct_cmd = S_RSA_SIGN;
		Gsct_request.rq_p1.kid = &icc_keyid;
		Gsct_request.rq_datafield.sig_string = hash_result;

		/*
		 * Call SCT Interface     			      
		 */
		rc = sct_interface(sct_id, Gsct_cmd, &Gsct_request, &Gsct_response);
		if (rc < 0) {
			sca_errno = sct_errno;
			sca_errmsg = sct_errmsg;
			STAErrAnalyse(sct_id);
			goto errcase;
		}


		/*
         	 * allocate storage for signature->bits
		 */
		signature->signature.nbits = 0;
		ALLOC_CHAR(signature->signature.bits,Gsct_response.noctets - 1);


		/*
		 * get returned signature    		      
		 */
		signature->signature.nbits = 8 * (Gsct_response.noctets - 1);
		for (i = 0; i < (Gsct_response.noctets - 1); i++) {
			signature->signature.bits[i] = Gsct_response.octets[i + 1];
		}

		signature->signAI = sign_algid;



#ifdef TEST
	fprintf(stdout, "TRACE of the output parameters : \n");
	fprintf(stdout, "signature               : \n");
	aux_ptr = aux_ObjId2Name(signature->signAI->objid);
	fprintf(stdout, "    signAI              : %s\n", aux_ptr);
	free(aux_ptr);
	fprintf(stdout, "    nbits               : %d\n",signature->signature.nbits);
	fprintf(stdout, "    bits                : \n");
	aux_fxdump(stdout, signature->signature.bits, signature->signature.nbits / 8, 0);
	fprintf(stdout, "keylength from the SCT  : %\n");
	aux_fxdump(stdout, &Gsct_response.octets[0], 1, 0);
	fprintf(stdout, "\n***** Normal end of   sca_sign *****\n\n");
#endif

		/*
		 * Normal End	 (Release storage)		       
		 */
		if (hash_result) {
			aux_free_OctetString(&hash_result);
		}

		Gsct_response.noctets = 1;
		aux_free2_OctetString(&Gsct_response);

		act_function = F_null;



	}			/* end if (more == SEC_END) */



	return (0);

/*****************************************************************************/
/*
 *      In error case release all allocated storage 			      
 */
errcase:
	act_function = F_null;
	
	/*
	 * if a hash function was already successfully called by MORE,
	 * this function has to be ended
	 */
	if (already_call) {
		dummy_octets.noctets = 0;
		switch (alghash) {
		case SQMODN:
			rc = hash_sqmodn(&dummy_octets, hash_result,
					SEC_END, hash_length * 8);
			break;
		case MD2:
			rc = md2_hash(&dummy_octets, hash_result, SEC_END);
			break;
		case MD4:
			rc = md4_hash(&dummy_octets, hash_result, SEC_END);
			break;
		case MD5:
			rc = md5_hash(&dummy_octets, hash_result, SEC_END);
			break;
		case SHA:
			rc = sha_hash(&dummy_octets, hash_result, SEC_END);
			break;
		}
		already_call = FALSE;
	}
	alghash = NoAlgHash;
	if (hash_result) {
		aux_free_OctetString(&hash_result);
	}
	hash_length  = 0;
	if(encodedDigest) aux_free_OctetString(&encodedDigest);


	return(-1);



}				/* end sca_sign */


/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_sign    	       */
/*-------------------------------------------------------------*/



/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_verify_sig               VERSION   1.1	       */
/*				     DATE   July 1993	       */
/*			      	       BY   L. Eckstein        */
/*							       */
/* DESCRIPTION						       */
/*  Verify a digital signature.			               */
/*  The signature algorithm is taken from signature->signAI.   */
/*							       */
/*  A smartcard is not expected.			       */
/*							       */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   out_octets		       Octetstring of the data to be   */
/*                             signed.                         */
/*                                                             */
/*   signature                 Signature to be verified and    */
/*			       corresponding algorithm         */
/*                                                             */
/*   verification_key          Structure which identifies the  */
/*                             verification key.               */
/*                             In the current version only the */
/*			       delivery of a public RSA key is */
/*			       supported (key_sel->key_bits).  */
/*                                                             */
/*   hash_par                  Additional algorithm (hash_alg) */
/*                             specific parameters or the NULL */
/*                             pointer.                        */
/*                                                             */
/*   more		       = SEC_MORE -> more data is expected */
/*   			       = SEC_END  -> Last data for this    */
/*				         verification process. */
/*                                                             */
/*							       */
/* OUT							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*   1				 M_SIGOK		       */
/*  -1			       error			       */
/*				 EPARINC		       */
/*				 EALGO 		               */
/*				 EKEY   		       */
/*				 EHASHFUNC		       */
/*				 EMEMAVAIL		       */
/*				 EFUNCTION		       */
/*				 EPKCS   		       */
/*							       */
/* CALLED FUNCTIONS					       */
/*  STACheckSCT_ICC               ERROR-Codes		       */
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
/*  rsa_set_key		       ERROR-Codes		       */
/*				 -1 -> M_EHASHPAR              */
/*							       */
/*  hash_sqmodn 	       ERROR-Codes     	               */
/*				 -1 -> M_EHASH                 */
/*                                                             */
/*  md2_hash	               ERROR-Codes      	       */
/*				 -1 -> M_EHASH                 */
/*                                                             */
/*  md4_hash	       	       ERROR_Codes      	       */
/*				 -1 -> M_EHASH                 */
/*                                                             */
/*  md5_hash	               ERROR-Codes      	       */
/*				 -1 -> M_EHASH                 */
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
/*  STAcompare						       */
/*							       */
/*  aux_free2_OctetString				       */
/*  aux_free_OctetString		    	               */
/*  aux_ObjId2Name					       */
/*  aux_ObjId2AlgType					       */
/*  aux_ObjId2ParmType					       */
/*  aux_ObjId2AlgHash					       */
/*  aux_ObjId2AlgSpecial				       */
/*  aux_create_PKCS_MIC_D				       */
/*  aux_create_PKCSBlock				       */	
/*							       */
/*   free macro in MF_check.h			               */ 
/*   malloc macro in MF_check.h			               */ 
/*-------------------------------------------------------------*/
int
sca_verify_sig(sct_id, out_octets, signature, verification_key, hash_par, more)
	int             sct_id;
	OctetString    *out_octets;
	Signature      *signature;
	KeySel         *verification_key;
	HashPar        *hash_par;
	More            more;

{
	int             	rc, i;
	static Boolean		already_call;
	char			*aux_ptr;
	static AlgHash  	alghash;
	static AlgSpecial  	algspecial;
	static int		hash_length;
	static OctetString 	*hash_result;
	static AlgId		*sign_algid;
	static OctetString      rsa_modulus;	
	OctetString		dummy_octets;
 	OctetString      	sig;	
	OctetString     	*encodedDigest;

	Verify			verify;
	Public			public;

	char			*proc="sca_verify_sig";

	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;

#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_verify_sig *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id		        : %d\n", sct_id);
	print_OctetString("out_octets    		:\n",
			   out_octets);


	if (act_function == F_null) {	/* first call of sca_verify_sig */
		fprintf(stdout, "First call of sca_verify_sig \n");
		fprintf(stdout, "the parameters signature->signAI, verification_key and hash_par are evaluated\n");
		if (more == SEC_END) 
			print_Signature(signature,0);
		else
			print_Signature(signature,1);
			
		fprintf(stdout, "the parameters verification_key and hash_par are evaluated\n");
		fprintf(stdout, "verification_key        : \n");
		print_VerificationKey(verification_key);

		print_HashPar(hash_par);

	}
	else {
		fprintf(stdout, "Next call of sca_verify_sig  \n");
		fprintf(stdout, "trace the internal signature algorithm \n");

		aux_ptr = aux_ObjId2Name(sign_algid->objid);
		fprintf(stdout, "signature algorithm    : %s\n", aux_ptr);
		free(aux_ptr);

		if (more == SEC_END) 
			print_Signature(signature,2);
		else
			fprintf(stdout, "Signature              : is not evaluated in case of more != SEC_END\n");


		fprintf(stdout, "trace the internal verification_key \n");
		fprintf(stdout, "verification_key        : \n");
		print_OctetString("modulus          	 :\n",
			   	   &rsa_modulus);

	}

	
	print_More(more);

#endif


/*****************************************************************************/
/*
 *      if first call of verify_sig function then
 *	1) check verification_key->key_bits  
 * 	2) if algorithm = RSA-sqmodn then               
 *  	   a) check hash-parameter         
 * 	   b) set key (modulus) for hash-function                     
 * 	3) allocate storage for hash result 
 *	else
 *	   check input parameter       
 */

	if (act_function == F_null) {	/* first call of sca_verify_sig */
		/*
	         * initialize static variables
		 */
		already_call = FALSE;
		alghash      = NoAlgHash;
		sign_algid   = NULL;
		hash_length  = 0;
		rsa_modulus.noctets = 0;
		rsa_modulus.octets  = NULL;
		hash_result  = NULLOCTETSTRING;
		encodedDigest= NULLOCTETSTRING;
		


		/*
	 	 * check verification_key            
	 	 */
		if (!verification_key) {
			sca_errno = EPARINC;
			sca_errmsg = sca_errlist[sca_errno].msg;
			goto errcase;
		} else if ((!verification_key->key_bits) ||
			   ((verification_key->key_bits->part1.noctets != 32) &&
			    (verification_key->key_bits->part1.noctets != 64) &&
			    (verification_key->key_bits->part1.noctets != 128)) ||
			   (!verification_key->key_bits->part1.octets)) {
				sca_errno = EPARINC;
				sca_errmsg = sca_errlist[sca_errno].msg;
				goto errcase;
		}


		if (verification_key->key_bits->part2.octets) {
	    		if ((verification_key->key_bits->part2.noctets != fermat_f4_len ) ||
                    	    (STAcompare(verification_key->key_bits->part2.octets,
                     	        fermat_f4,fermat_f4_len)) != 0) {
        			/* exponent not = Fermatzahl F4 */
				sca_errno = EPARINC;
				sca_errmsg = sca_errlist[sca_errno].msg;
				return (-1);
			}
		}

		/*
		 * save rsa modulus fur further use
		 */
		rsa_modulus.noctets = verification_key->key_bits->part1.noctets;
		ALLOC_CHAR(rsa_modulus.octets, rsa_modulus.noctets);
		for (i=0; i<rsa_modulus.noctets; i++)
			*(rsa_modulus.octets+i) = *(verification_key->key_bits->part1.octets+i);

		/*
	 	 * check signature algorithm , must be SIG           
	 	 */
		if ((!signature) ||
		    (!signature->signAI) ||
		    (!signature->signAI->objid)) {
			sca_errno = EPARINC;
			sca_errmsg = sca_errlist[sca_errno].msg;
			goto errcase;
		}

		if (aux_ObjId2AlgType(signature->signAI->objid) != SIG) {
			sca_errno = EALGO;
			sca_errmsg = sca_errlist[sca_errno].msg;
			goto errcase;
		}

		if ((aux_ObjId2ParmType(signature->signAI->objid) == PARM_NULL) &&
		    	    (signature->signAI->param)) {
				sca_errno = EPARINC;
				sca_errmsg = sca_errlist[sca_errno].msg;
				goto errcase;
		}

		sign_algid = signature->signAI;


		/*
	  	 * get hash algorithm
		 */

		alghash = aux_ObjId2AlgHash(sign_algid->objid);
		algspecial = aux_ObjId2AlgSpecial(sign_algid->objid);
		if (alghash == SQMODN) {
			if (hash_par == NULL) {
				sca_errno = EPARINC;
				sca_errmsg = sca_errlist[sca_errno].msg;
				goto errcase;
			}
			if (hash_par->sqmodn_par.part1.octets == NULL) {
				sca_errno = EPARINC;
				sca_errmsg = sca_errlist[sca_errno].msg;
				goto errcase;
			}
			/* set key in an internal function for hash-function */
			rc = rsa_set_key(&hash_par->sqmodn_par, 0);
			if (rc < 0) {
				sca_errno = EKEY;
				sca_errmsg = sca_errlist[sca_errno].msg;
				goto errcase;
			}
			/*
		         * set hash_length on alg_id specific value
			 */ 
 			hash_length = (RSA_PARM(sign_algid->param) + 7) / 8;
		}
		else
			/*
			 * MD2, MD4, MD5 => length of hash string = 128 bit
			 */
			hash_length = 16;


#ifdef TEST
		fprintf(stdout,"hash_length = %d\n",hash_length);
#endif

		/*
         	 * allocate storage for hash result
		 */
		ALLOC_OCTET(hash_result, OctetString);
		hash_result->noctets = 0;
		ALLOC_CHAR(hash_result->octets, hash_length);

		act_function = F_verify;



	}
	/* end if (act_function == F_null) */
	else {
	 	/* not first call of sca_verify_sig */ 
		if (act_function != F_verify) {
			sca_errno = EFUNCTION;	/* wrong function call */
			sca_errmsg = sca_errlist[sca_errno].msg;
			goto errcase;
		}
	}




/*****************************************************************************/
/*
 *      Check input parameters
 */

/*****************************************************************************/
/*
 *      Check input parameters
 */

	if ((!out_octets) ||
	    ((out_octets->noctets > 0) &&
	     (!out_octets->noctets))) { 
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		goto errcase;
	}


	if ((more != SEC_END) && (more != SEC_MORE)) {
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		goto errcase;
	}






/*****************************************************************************/
/*
 *      Call hash function depending on algorithm
 */
	switch (alghash) {
	case SQMODN:
		rc = hash_sqmodn(out_octets, hash_result, more, hash_length * 8);
		break;
	case MD2:
		rc = md2_hash(out_octets, hash_result, more);
		break;
	case MD4:
		rc = md4_hash(out_octets, hash_result, more);
		break;
	case MD5:
		rc = md5_hash(out_octets, hash_result, more);
		break;
	default:
		sca_errno = EALGO;
		sca_errmsg = sca_errlist[sca_errno].msg;
		goto errcase;


	}			/* end switch */
	if (rc < 0) {
		sca_errno = EHASHFUNC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		goto errcase;
	}
	else {
		/*
		 * in case of SEC_MORE, mark, that the hash function was
		 * already successfully called
		 */
		if (more == SEC_MORE)
			already_call = TRUE;
	}


/*****************************************************************************/
/*
 *      Now hashing is done			      
 * 	If last call of sca_verify_sig ( more = SEC_END)  then call sct-interface
 */

	if (more == SEC_END) {	/* last call of sca_verify_sig */
		already_call = FALSE;

#ifdef TEST
		fprintf(stdout, "hash result: \n");
		aux_fxdump(stdout, hash_result->octets, hash_result->noctets, 0);
#endif

		/* 
		 * check signature    	          
		 */
		if (!signature->signature.bits) {
			sca_errno = EPARINC;
			sca_errmsg = sca_errlist[sca_errno].msg;
			goto errcase;
		}
		sig.noctets	= signature->signature.nbits / 8;
		sig.octets      = signature->signature.bits;

		/*
 		 *      call STACheckSCT_ICC
 		 */
		if (STACheckSCT_ICC(sct_id, FALSE, proc) == -1)
			goto errcase;

		if(algspecial == PKCS_BT_01) {
			/*
			 * hash-value must have the same length as the modulus
			 * therefore the global variable public_modulus_length    
			 * (declared in sec_init.c) is set 
			 * it is used in aux_create_PKCSBlock to compute the
			 * blocksize
			 */
			public_modulus_length = verification_key->key_bits->part1.noctets * 8;

			if (hash_result->noctets < public_modulus_length / 8) {

				/*  Here goes PKCS#1 ...   */

				encodedDigest = aux_create_PKCS_MIC_D(hash_result,
							              signature->signAI);
				if (!encodedDigest) {
					sca_errno = EPKCS;
					sca_errmsg = sca_errlist[sca_errno].msg;
					goto errcase;
				}

				if (hash_result) aux_free_OctetString(&hash_result);		
				hash_result = aux_create_PKCSBlock(1, encodedDigest);
				if (!hash_result) {
					sca_errno = EPKCS;
					sca_errmsg = sca_errlist[sca_errno].msg;
					goto errcase;
				}
				if(encodedDigest) aux_free_OctetString(&encodedDigest);
			}

       		}

		/*
		 * Prepare parameters for the SCT Interface          
		 */
      
		Gsct_cmd = S_RSA_VERIFY;
                public.modulus 		= &rsa_modulus;
                verify.public 		= &public;
                verify.signature 	= &sig;
                verify.hash 	        = hash_result;
                Gsct_request.rq_datafield.verify = &verify;




		/*
		 * Call SCT Interface     			      
		 */
		rc = sct_interface(sct_id, Gsct_cmd, &Gsct_request,
				   &Gsct_response);
		if (rc < 0) {
			sca_errno = sct_errno;
			sca_errmsg = sct_errmsg;
			STAErrAnalyse(sct_id);
			goto errcase;
		}

		/*
		 * rc == S_SIKOK (M_SIGOK) means   		       
		 * signature correct, keysize too short (256)    
		 */
		if (rc == S_SIGOK)
			sca_errno = M_SIGOK;

		/*
		 * Normal End	 (Release storage)		       
		 */
		Gsct_response.noctets = 1;
		rsa_modulus.noctets = 1;
		if (hash_result) {
			aux_free_OctetString(&hash_result);
		}
		aux_free2_OctetString(&Gsct_response);
		aux_free2_OctetString(&rsa_modulus);

		act_function = F_null;

	}			/* end if (more == SEC_END) */
#ifdef TEST
	fprintf(stdout, "\n***** Normal end of   sca_verify_sig *****\n\n");
#endif

	return (sca_errno);

/*****************************************************************************/
/*
 *      In error case release all allocated storage 			      
 */
errcase:
	act_function = F_null;
	
	/*
	 * if a hash function was already successfully called by MORE,
	 * this function has to be ended
	 */
	if (already_call) {
		dummy_octets.noctets = 0;
		switch (alghash) {
		case SQMODN:
			rc = hash_sqmodn(&dummy_octets, hash_result,
					SEC_END, hash_length * 8);
			break;
		case MD2:
			rc = md2_hash(&dummy_octets, hash_result, SEC_END);
			break;
		case MD4:
			rc = md4_hash(&dummy_octets, hash_result, SEC_END);
			break;
		case MD5:
			rc = md5_hash(&dummy_octets, hash_result, SEC_END);
			break;
		case SHA:
			rc = sha_hash(&dummy_octets, hash_result, SEC_END);
			break;
		}
		already_call = FALSE;
	}
	alghash = NoAlgHash;
	rsa_modulus.noctets = 1;
	if (hash_result) {
		aux_free_OctetString(&hash_result);
	}
	aux_free2_OctetString(&rsa_modulus);
	if(encodedDigest) aux_free_OctetString(&encodedDigest);
	hash_length  = 0;
	return(-1);




}				/* end sca_verify */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_verify_sig         */
/*-------------------------------------------------------------*/

/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_encrypt               VERSION   1.1	       */
/*				     DATE   August 1993	       */
/*			      	       BY   L.Eckstein         */
/*							       */
/* DESCRIPTION						       */
/*  Encrypt OctetString in SCT or ICC.			       */
/*							       */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   in_octets		       Octetstring of the data to be   */
/*                             encrypted.                      */
/*                                                             */
/*   out_octets->noctets       offset where the encrypted data */
/*			       shall be stored.		       */
/*                                                             */
/*   encryption_key            Structure which identifies the  */
/*                             encryption   key.               */
/*			       DES:			       */
/*			       The key is stored either in the */
/*			       SCT or in the ICC.	       */
/*			       RSA:			       */
/*                             In the current version only the */
/*			       delivery of a public RSA key is */
/*			       supported                       */
/*			       (encryption_key->key_bits).     */
/*   destination	       Call encryption function in SCT */
/*			       or in ICC		       */
/*			       in case of RSA, destination     */
/*			       must be set to SCT	       */		
/*   sec_mess                  Specification of the security   */
/*			       modes for the data exchange     */
/*			       between SCT and ICC.	       */
/*			       will only be evaluated in case  */
/*			       of destination = ICC            */	
/*							       */
/*   more		       = SEC_MORE -> more data are     */
/*				             expected          */
/*   			       = SEC_END  -> Last data for this*/
/*				         encryption   process. */
/*                                                             */
/* OUT							       */
/*   out_octets->octets        Encrypted data                  */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*				 EPARINC 		       */
/*				 EALGO 		               */
/*				 EMEMAVAIL		       */
/*				 EFUNCTION		       */
/*				 EPKCS   		       */
/*							       */
/* CALLED FUNCTIONS					       */
/*  STACheckSCT_ICC               ERROR-Codes		       */
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
/*							       */
/*  icc_check_KeyId           ERROR-Codes		       */
/*			         EPARINC		       */
/*							       */
/*  STACheckSecMess               ERROR-Codes		       */
/*				 EPARINC		       */
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
/*  STAcompare						       */
/*							       */
/*  STACreateHead         	ERROR-Codes    		       */
/*                               EPARINC 		       */
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
/*  aux_ObjId2AlgType    				       */
/*  aux_ObjId2AlgEnc    				       */
/*  aux_ObjId2AlgSpecial				       */
/*  aux_cmp_ObjId					       */
/*  aux_free2_OctetString				       */
/*  aux_free_OctetString				       */
/*  aux_create_PKCSBlock				       */	
/*							       */
/*   malloc macro in MF_check.h			               */ 
/*   free macro in MF_check.h			               */ 
/*							       */
/*  Achtung !!!!					       */
/*  In der Smartcard wird der CBC-Mode nur auf ein Kommando    */
/*  angewendet, d.h. Texte, die groesser als 80 bzw. 72 Bytes  */
/*  sind werden in der ICC nicht richtig behandelt, da bei     */
/*  jedem ICC_CRYPT-Kommando der erste Block wieder mit dem    */
/*  Initialvektor (0) verschluesselt wird und nicht wie beim   */
/*  CBC-Mode mit dem letzten Chiffratblock.		       */
/*  STARMOD fuehrt deshalb folgende Funktionen durch:          */
/*  Der Text wird in 80er bzw. 72er Bloecke aufgeteilt.        */
/*  Nach einem erfolgreichen ICC_CRYPT-Kommando werden die     */
/*  letzten 8 Bytes des empfangenen Chiffrats auf die ersten   */
/*  8 Bytes des naechsten Klartext-Blockes geodert.            */
/*  Im Falle von MORE=SEC_END wird in STARMOD entsprechend dem */
/*  Algorithmus Identifier gepaddet. 
/*							       */
/*-------------------------------------------------------------*/
int
sca_encrypt(sct_id, in_octets, out_octets, encryption_key, destination,
	   sec_mess, more)
	int             sct_id;
	OctetString    	*in_octets;
	OctetString    	*out_octets;
	KeySel         	*encryption_key;
	Destination     destination;
	ICC_SecMess     *sec_mess;
	More            more;
{
	static char     	cbc_initvek[8];
	static AlgEnc   	algenc    = NoAlgEnc;
	static AlgType  	algtype;
	static AlgSpecial	algspecial= NoAlgSpecial;
	static OctetString     	modulus;	
	static Destination 	enc_destination;
	static int		out_enc_len = 0;
	static Boolean		already_call, elem_last_block;
	static int		blocksize = 0;
	static int      	max_length, first_len, next_len;		
	static int      	out_rest_len = 0;
	static char    		*out_rest = NULL;
	int			rc, padding_len, offset, j, out_newlen;
	int			i, act_length, enc_len, out_conc_len;
	char			first_pad_byte, next_pad_byte;
	char			*in_ptr, *out_conc, *out_ptr;
	char           		*out_new;
	char			*dummy_data="E";
	Boolean			NOTENDE;
	Enc             	enc;
	Public          	public;
	OctetString     	trans;
	OctetString		*pkcs_block;
	More            	cmd_more;
	char			*proc="sca_encrypt";


	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;
	out_conc = NULL;
	out_new = NULL;
	pkcs_block = NULL;


#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_encrypt *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id                 : %d\n", sct_id);
	print_OctetString("in_octets		        :\n",
			   in_octets);
	if (out_octets)
		fprintf(stdout, "out_octets->noctets    : %d\n",
				out_octets->noctets);
	else
		fprintf(stdout, "out_octets             : NULL\n");

	if (act_function == F_null) {	/* first call of sca_encrypt */
		fprintf(stdout, "First call of sca_encrypt / trace encryption_key \n");

		print_KeySel(encryption_key,TRUE);
		print_Destination(destination);
	}
	else {
		fprintf(stdout, "Next call of sca_encrypt  \n");
		fprintf(stdout, "encryption_key          : is not evaluated\n");
		print_Destination(enc_destination);
	}

	if (destination == ICC)
		print_ICC_SecMess(sec_mess);
	else
		fprintf(stdout, "sec_mess                : is not evaluated in case of destination != ICC\n");
	print_More(more);

#endif


/*****************************************************************************/
/*
 *      Check input parameters
 */

	if ((in_octets == NULL) ||
	    (in_octets->noctets == 0) ||
	    (in_octets->octets == NULL)) {
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		goto errcase;
	}


	if ((out_octets == NULL) ||
	    (out_octets->octets == NULL)) {
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		goto errcase;
	}


	if ((more != SEC_END) && (more != SEC_MORE)) {
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		goto errcase;
	}





/*****************************************************************************/
/*
 *      if first call => check destination, encryption_key
 *			 and smartcard
 */


	if (act_function == F_null) {	/* first call of sca_encrypt */

		modulus.noctets	= 0;
		modulus.octets	= NULL;
		out_rest_len 	= 0;
		out_rest 	= NULL;
		already_call	= FALSE;

		if (encryption_key == NULL) {
			sca_errno = EPARINC;
			sca_errmsg = sca_errlist[sca_errno].msg;
			goto errcase;
		}

		if ((!encryption_key->key_algid) || 
		    (!encryption_key->key_algid->objid)) {
			sca_errno = EPARINC;
			sca_errmsg= sca_errlist[sca_errno].msg;
			return (-1);
		}

		if (destination == SCT) 
			icc_expect = FALSE;
		else {
			if (destination == ICC)
				icc_expect = TRUE;
		     	else {
				sca_errno = EPARINC;
				sca_errmsg = sca_errlist[sca_errno].msg;
				return(-1);
		     	}
		} 

		/*
 		 *      call STACheckSCT_ICC
 		 */
		if (STACheckSCT_ICC(sct_id, icc_expect, proc) == -1)
			goto errcase;

		enc_destination   		= destination;
		algspecial = aux_ObjId2AlgSpecial(encryption_key->key_algid->objid);
	 	algtype    = aux_ObjId2AlgType(encryption_key->key_algid->objid);
	 	algenc 	   = aux_ObjId2AlgEnc(encryption_key->key_algid->objid);


	 	if (algtype != ASYM_ENC && algtype != SYM_ENC) {
			sca_errno = EALGO;
			sca_errmsg = sca_errlist[sca_errno].msg;
			goto errcase;
		}


		switch(algenc) {

		case RSA:

			/*
	 		 * check destination                                   
	 		 */

			if (enc_destination != SCT) {
				sca_errno = EPARINC;
				sca_errmsg = sca_errlist[sca_errno].msg;
				goto errcase;
			} 
			/*
			 * check key selection in case of RSA                  
			 * check key (key must be a public RSA key)            
			 */

			if ((!encryption_key->key_bits) ||
	    		    (!encryption_key->key_bits->part1.octets) ||
	    		    (encryption_key->key_bits->part1.noctets == 0)) {
				sca_errno = EPARINC;
				sca_errmsg = sca_errlist[sca_errno].msg;
				goto errcase;
			}

			if (encryption_key->key_bits->part2.octets) {
	    			if ((encryption_key->key_bits->part2.noctets != fermat_f4_len ) ||
                    			(STAcompare(encryption_key->key_bits->part2.octets,
                     	        		    fermat_f4,fermat_f4_len)) != 0) {
        				/* exponent not = Fermatzahl F4 */
					sca_errno = EPARINC;
					sca_errmsg = sca_errlist[sca_errno].msg;
					goto errcase;
				}
			}

			modulus.noctets = encryption_key->key_bits->part1.noctets;
			modulus.octets  = encryption_key->key_bits->part1.octets;

			first_len  = SCT_RSA_ENC_FIRST_DATLEN - (modulus.noctets + 1);
			/*
			 * the blocksize is set to the modulus_length (in bytes) - 1
			 */
			blocksize = modulus.noctets - 1;
#ifdef TEST
			fprintf(stdout, "blocksize              : %d\n", blocksize);
#endif

			/*
			 * in case of RSAEncryption the length of in_octets->noctets must be
			 * less blocksize - 2 and more must be set to SEC_END.
			 */

	    		if (!aux_cmp_ObjId(encryption_key->key_algid->objid,
			                   rsaEncryption->objid)) {
				if ((more != SEC_END) ||
				    (in_octets->noctets > blocksize - 2)) {
 					sca_errno = EPARINC;
					sca_errmsg = sca_errlist[sca_errno].msg;
					goto errcase;
				}
			}


			Gsct_cmd = S_RSA_ENC;
			Gsct_request.rq_p1.kid 		= 0x00;
			public.modulus    		= &modulus;
			enc.public        		= &public;
			Gsct_request.rq_datafield.enc 	= &enc;
			next_len 	  		= SCT_RSA_ENC_NEXT_DATLEN;

			break;

		case DES:
		case DES3:
			/*
			 * if destination == ICC => only DES is allowed 
			 */
			if ((enc_destination == ICC) && (algenc == DES3)) {
				sca_errno = EALGO;
				sca_errmsg = sca_errlist[sca_errno].msg;
				goto errcase;
			}
			if (enc_destination == ICC) {
				/*
	 		 	 * Check key_id
				 */
				if (icc_check_KeyId(encryption_key->key_id) == -1) {
					sca_errno = icc_errno;
					sca_errmsg = icc_errmsg;
					goto errcase;
				}


				/*
	 		 	 * Check sec_mess
				 */
				if (STACheckSecMess(sec_mess) == -1)
					goto errcase;
				/*
 				 *      compute maximal datalength
 				 */

				switch (sec_mess->command) {
				case ICC_SEC_NORMAL:
					first_len = ICC_CRYPT_NORMDATLEN;
					break;
				case ICC_AUTHENTIC:
					first_len = ICC_CRYPT_AUTHDATLEN;
					break;
				case ICC_CONCEALED:
					first_len = ICC_CRYPT_CONCDATLEN;
					break;
				case ICC_COMBINED:
					first_len = ICC_CRYPT_COMBDATLEN;
					break;
				}

				next_len 	= first_len;


				/*
		 		 * check padding method in ICC
				 * sctinfo was set in STACheckSCT_ICC
		 		 */


				switch (algspecial) {
				case WITH_PEM_PADDING:
				case WITH_B1_PADDING:
					if (((sctinfo.first_pad_byte & 0xFF) == 0x80) &&
				             (sctinfo.next_pad_byte  == 0x00) &&
				             (sctinfo.always == TRUE)) {
						if ((more != SEC_END) ||
						    (in_octets->noctets >= first_len)) {
							sca_errno = EPARINC;
							sca_errmsg = sca_errlist[sca_errno].msg;
							goto errcase;
						}
			
  
						elem_last_block = TRUE;
					}
					else {

						if ((sctinfo.first_pad_byte != 0x00) ||
				            	    (sctinfo.next_pad_byte  != 0x00) ||
				                    (sctinfo.always == TRUE)) {
							sca_errno = EICCPADD;
							sca_errmsg = sca_errlist[sca_errno].msg;
							goto errcase;
						};
						elem_last_block = FALSE;
					}
					break;

				case WITH_B0_PADDING:
					if (((sctinfo.first_pad_byte & 0xFF) == 0x80) &&
				             (sctinfo.next_pad_byte  == 0x00) &&
				             (sctinfo.always == TRUE)) {
						if ((more != SEC_END) ||
						    (in_octets->noctets > first_len)) {
							sca_errno = EPARINC;
							sca_errmsg = sca_errlist[sca_errno].msg;
							goto errcase;
						}
			
  
						elem_last_block = TRUE;
					}
					else {

						if ((sctinfo.first_pad_byte != 0x00) ||
				            	    (sctinfo.next_pad_byte  != 0x00) ||
				                    (sctinfo.always == TRUE)) {
							sca_errno = EICCPADD;
							sca_errmsg = sca_errlist[sca_errno].msg;
							goto errcase;
						};
						elem_last_block = FALSE;
					}
					break;
				}
				/*
		 	 	 * initialize cbc_initvek with zero 
			 	 */
				for (i = 0; i < 8; i++)
					cbc_initvek[i] = 0x00;

				out_conc_len = in_octets->noctets;
				ALLOC_CHAR(out_conc, out_conc_len);

				for (i = 0; i < out_conc_len; i++)
					*(out_conc + i) = *(in_octets->octets + i);


			} else {
				/*
				 * set maximal data length depending on 
				 * secure messaging between DTE and SCT
				 * in case of NORMAL, INTEGRITY and CONCEALED
				 * the max. data length = 240
				 * else max. data length = 240 - 4 byte CCS
				 * sctinfo was set in STACheckSCT_ICC
				 */
				if (sctinfo.secure_messaging.command == SCT_COMBINED) { 
					first_len 		= SCT_DES_ENC_DEC_DATLEN - 4;
					next_len 		= SCT_DES_ENC_DEC_DATLEN - 4;
				}
				else {
					first_len 		= SCT_DES_ENC_DEC_DATLEN;
					next_len 		= SCT_DES_ENC_DEC_DATLEN;
				}

				Gsct_cmd 		= S_DES_ENC;
				Gsct_request.rq_p1.kid 	= encryption_key->key_id;
				blocksize  		= 8;
#ifdef TEST
				fprintf(stdout, "blocksize              : %d\n", blocksize);
#endif
			};
			break;
		default:
			sca_errno = EALGO;
			sca_errmsg = sca_errlist[sca_errno].msg;
			goto errcase;

		} /* end switch */




		act_function = F_encrypt;



	} 
	else {
		/*	
		 * next call of sca_encrypt  
		 */
		if (algenc == RSA) {
			modulus.noctets = 0;
			modulus.octets 	= NULL;
			public.modulus    		= &modulus;
			enc.public        		= &public;
			Gsct_request.rq_datafield.enc 	= &enc;
			first_len 	= SCT_RSA_ENC_NEXT_DATLEN;
		}

		if (act_function != F_encrypt) {
			sca_errno = EFUNCTION;	/* wrong function call */
			sca_errmsg = sca_errlist[sca_errno].msg;
			goto errcase;
		};
		switch (enc_destination) {
		case ICC:
			if (out_rest_len > 0) {
				out_conc_len = in_octets->noctets + out_rest_len;
			} else
				out_conc_len = in_octets->noctets;

			ALLOC_CHAR(out_conc, out_conc_len);

			for (i = 0; i < out_rest_len; i++)
				*(out_conc + i) = *(out_rest + i);

			free(out_rest);
			out_rest = NULL;

			offset = out_rest_len;
			for (i = 0; i < in_octets->noctets; i++) {
				*(out_conc + offset) = *(in_octets->octets + i);
				offset++;

			}
			out_rest_len = 0;
			break;
		} /* end switch enc_destination */
	}





/*****************************************************************************/
/*
 * 	prepare outgoing data in case of more=SEC_END 
 */
	switch(enc_destination) {
		case SCT:
			out_enc_len += in_octets->noctets;
			if (more == SEC_END) {
				switch (algspecial) {
				case WITH_B1_PADDING:
				case WITH_PEM_PADDING:
					/*
					 * padding is done by STARMOD
					 */
					padding_len = out_enc_len % blocksize;
					if (padding_len == 0)
						padding_len = blocksize;
					else
						padding_len = blocksize - padding_len;

#ifdef TEST
					fprintf(stdout, "padding_len            : %d\n", padding_len);
#endif
					act_length = in_octets->noctets + padding_len;
					ALLOC_CHAR(out_conc, act_length);
					for (i=0; i<in_octets->noctets; i++)
						*(out_conc + i) = *(in_octets->octets + i);
					offset = in_octets->noctets;
					if (algspecial == WITH_B1_PADDING) {
						*(out_conc + offset++) = 0x80;
						for (i=0; i<padding_len - 1; i++)
							*(out_conc + offset++) = 0x00;
					}
					else {
						for (i=0; i<padding_len; i++)
							*(out_conc + offset++) = padding_len;
					}
					in_ptr = out_conc;
#ifdef TEST
					fprintf(stdout, "act_length             : %d\n",
						 act_length);
					aux_fxdump(stdout, out_conc, act_length, 0);
#endif
					break;

				case WITH_B0_PADDING:
					act_length = in_octets->noctets;
					in_ptr     = in_octets->octets;
					break;

				case PKCS_BT_02:

					/*
	 				 * in case of PKCS => create Padding string
			 		 * therefore the global variable public_modulus_length    
			 		 * (declared in sec_init.c) is set 
			 		 * it is used in aux_create_PKCSBlock to compute the
			 		 * blocksize
			 		 */
					public_modulus_length = encryption_key->key_bits->part1.noctets * 8;

					pkcs_block = aux_create_PKCSBlock(2, in_octets);
					if (!pkcs_block) {
						sca_errno = EPKCS;
						sca_errmsg = sca_errlist[sca_errno].msg;
						goto errcase;
					}
					act_length = pkcs_block->noctets;
					in_ptr = pkcs_block->octets;
#ifdef TEST
					fprintf(stdout, "pkcs_block->noctets             : %d\n",
						 pkcs_block->noctets);
					aux_fxdump(stdout, pkcs_block->octets, pkcs_block->noctets, 0);
#endif
					break;

				default:
					in_ptr 	   = in_octets->octets;
					act_length = in_octets->noctets;
					break;
				} /* end switch algspecial*/
			} /* end if more = SEC_END */
			else {
				in_ptr 	   = in_octets->octets;
				act_length = in_octets->noctets;
			} /* more = SEC_MORE */
			break;

		case ICC:

			if (more == SEC_END) {
				switch (algspecial) {
				case WITH_B0_PADDING:
					if ((out_conc_len % 8) > 0) {
						/* allocate buffer, Padding with Zero */
						out_newlen = (out_conc_len -
						     	     (out_conc_len % 8)) + 8;
						padding_len = 8 - (out_conc_len % 8);
					}
					else {
						out_newlen = out_conc_len;
						padding_len = 0;
					}

					first_pad_byte = 0x00;
					next_pad_byte  = 0x00;

					break;
				case WITH_B1_PADDING:
					/* allocate buffer, Padding with b'1...0' */
					if ((out_conc_len % 8) > 0) {
					  	out_newlen = (out_conc_len -
						     	     (out_conc_len % 8)) + 8;
						padding_len = 8 - (out_conc_len % 8);
					}
					else {
						out_newlen = out_conc_len + 8;
						padding_len = 8;
					}
					first_pad_byte = 0x80;
					next_pad_byte  = 0x00;
					break;
					
				case WITH_PEM_PADDING:
					/* allocate buffer, Padding with PEM */
					if ((out_conc_len % 8) > 0) {
						out_newlen = (out_conc_len -
						     	     (out_conc_len % 8)) + 8;
						padding_len = 8 - (out_conc_len % 8);
					}
					else {
						out_newlen = out_conc_len + 8;
						padding_len = 8;
					}

					first_pad_byte = padding_len;
					next_pad_byte  = padding_len;
					break;
				} /* end switch algspecial */
				
				offset = 0;
				ALLOC_CHAR(out_new, out_newlen);
				for (i = 0; i < out_conc_len; i++) 
					*(out_new + offset++) = *(out_conc + i);

				if (padding_len > 0) {
					*(out_new + offset++) = first_pad_byte;
					padding_len--;

					for (i=0; i<padding_len; i++)
						*(out_new + offset++) = next_pad_byte;
				}
				free(out_conc);
				out_conc_len = 0;
				out_conc = NULL;
				act_length = out_newlen;
				in_ptr = out_new;


			} 
			else {/* more == SEC_MORE */
				if ((out_conc_len % first_len) > 0) {
					/* save rest for next transfer */
					out_rest_len = out_conc_len % first_len;
					act_length = out_conc_len - out_rest_len;

					ALLOC_CHAR(out_rest, out_rest_len);

					offset = out_conc_len - out_rest_len;
					for (i = 0; i < out_rest_len; i++) {
						*(out_rest + i) = *(out_conc + offset);
						offset++;
					}
					if (act_length == 0) {
						free(out_conc);
						return (0);
					}
					in_ptr = out_conc;
				} 
				else {
					in_ptr = out_conc;
					act_length = out_conc_len;
				}
			}
			break;


	} /* end switch enc_destination */







/*****************************************************************************/
/*
 *      Prepare parameters for the SCT Interface			      
 * 	
 */
	max_length = first_len;

#ifdef TEST
	fprintf(stdout, "FIRST_LEN = %d\n", first_len);
#endif

	NOTENDE = TRUE;
	out_ptr = out_octets->octets + out_octets->noctets;
	enc_len = 0;
	do {
		if (act_length <= max_length) {
			trans.noctets = act_length;
			cmd_more = more;

			NOTENDE = FALSE;
		} else {
			trans.noctets = max_length;
			cmd_more = SEC_MORE;
			max_length = next_len;
		}
		switch (enc_destination) {
		case SCT:

			Gsct_request.rq_p2.more = cmd_more;
			trans.octets = in_ptr;

			if (Gsct_cmd == S_RSA_ENC)
				Gsct_request.rq_datafield.enc->plaintext = &trans;
			else
				Gsct_request.rq_datafield.plaintext = &trans;
			/*
			 * Call SCT Interface     			  
			 */
			rc = sct_interface(sct_id, Gsct_cmd, &Gsct_request,
					   &Gsct_response);
			if (rc < 0) {
				sca_errno = sct_errno;
				sca_errmsg = sct_errmsg;
				STAErrAnalyse(sct_id);
				goto errcase;
			};
			modulus.noctets = 0;
			/*
		 	 * in case of SEC_MORE, mark, that the function was
		 	 * already successfully called
		 	 */
			if (cmd_more == SEC_MORE)
				already_call = TRUE;


			break;

		default:
			/*
			 * create ICC command CRYPT           
			 */

			/* in_ptr[0..7] XOR cbc_initvek[0..7] for CBC-Mode */
			for (i = 0; i < 8; i++)
				*(in_ptr + i) = *(in_ptr + i) ^ cbc_initvek[i];


			if (STACreateHead(ICC_CRYPT, sec_mess))
				goto errcase;


			/* set parameters			  */
			PAR_KID  = encryption_key->key_id;
			PAR_MODI = ICC_ENC;

			trans.octets  = in_ptr;
			PAR_DATA      = &trans;



			/* call STACreateTrans			  */
			if (STACreateTrans(sct_id, TRUE))
				goto errcase;


			/* save last 8 Bytes of ciphertext in cbc_initvek */
			j = 1;
			for (i = 7; i >= 0; i--) {
				cbc_initvek[i] = *(Gsct_response.octets + Gsct_response.noctets - j);
				j++;
			}


			break;

		}
		in_ptr += trans.noctets;
		act_length -= trans.noctets;

		if ((enc_destination == ICC) &&
		    (NOTENDE == FALSE) &&
	            (elem_last_block == TRUE)) {
			/*
			 * eleminate last encrypted block
			 */
			Gsct_response.noctets -= 8;
		}

		for (i = 0; i < Gsct_response.noctets; i++) {
			*out_ptr = *(Gsct_response.octets + i);
			out_ptr++;
		}
		out_octets->noctets += Gsct_response.noctets;
		enc_len += Gsct_response.noctets;

		/*
		 * Normal End	 (Release storage)		
		 */
		Gsct_response.noctets = 1;
		aux_free2_OctetString(&Gsct_response);

	} while (NOTENDE);

/*****************************************************************************/
/*
 *      Normal End	 (Release storage)			      
 * 	
 */

	if(out_new) 	free(out_new);
	if(out_conc) 	free(out_conc);
	if(pkcs_block) {
		aux_free_OctetString(&pkcs_block);
	}
		

/*****************************************************************************/
/*
 *      if last call of encryption function			      
 * 	
 */
	if (more == SEC_END) {	/* last call of sca_encrypt */
		algenc 		= NoAlgEnc;
		act_function 	= F_null;
		algspecial	= NoAlgSpecial;
		modulus.noctets = 0;
		modulus.octets  = NULL;
		out_enc_len     = 0;



	};


#ifdef TEST
	offset = out_octets->noctets - enc_len;
	fprintf(stdout, "TRACE of the output parameters : \n");
	fprintf(stdout, "out_octets              : \n");
	fprintf(stdout, "    enc_len             : %d\n", enc_len);
	fprintf(stdout, "    octets              : \n");
	aux_fxdump(stdout, out_octets->octets + offset, enc_len, 0);

	fprintf(stdout, "\n***** Normal end of   sca_encrypt *****\n\n");
#endif


	return (enc_len);

/*****************************************************************************/
/*
 *      In error case release all allocated storage			      
 * 	
 */
errcase:
	algenc 		= NoAlgEnc;
	algspecial	= NoAlgSpecial;
	modulus.noctets = 0;
	modulus.octets  = NULL;
	act_function 	= F_null;
	out_enc_len     = 0;
	if(out_conc) 	free(out_conc);
	if(pkcs_block)  {
		aux_free_OctetString(&pkcs_block);
	}
  	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);

	
	/*
	 * if the encrypt function in the SCT was already successfully called
 	 * by MORE, this function has to be ended
	 */
	if ((already_call == TRUE) && (sca_errno != ESTATE0)) {
		Gsct_request.rq_p2.more = SEC_END;
		trans.noctets = 1;
		trans.octets  = dummy_data;

		if (Gsct_cmd == S_RSA_ENC)
			Gsct_request.rq_datafield.enc->plaintext = &trans;
		else
			Gsct_request.rq_datafield.plaintext = &trans;
		/*
		 * Call SCT Interface     			  
		 */
		rc = sct_interface(sct_id, Gsct_cmd, &Gsct_request,
			           &Gsct_response);
  		Gsct_response.noctets = 1;
		aux_free2_OctetString(&Gsct_response);
	}
	


	already_call = FALSE;
	if(out_new) free(out_new);
	if(out_rest) free(out_rest);
	out_rest = NULL;
	out_rest_len = 0;
	return (-1);


}				/* end sca_encrypt */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_encrypt 	       */
/*-------------------------------------------------------------*/

/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_decrypt               VERSION   1.1	       */
/*				     DATE   August 1993	       */
/*			      	       BY   L.Eckstein         */
/*							       */
/* DESCRIPTION						       */
/*  Decrypt OctetString in SCT or ICC.			       */
/*							       */
/*							       */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   in_octets		       Octetstring of the encrypted    */
/*                             data                            */
/*                                                             */
/*   out_octets->noctets       offset where the decrypted data */
/*			       shall be stored.		       */
/*                                                             */
/*   decryption_key            Structure which identifies the  */
/*                             decryption   key.               */
/*			       DES:			       */
/*			       The key is stored either in the */
/*			       SCT or in the ICC.	       */
/*			       RSA:			       */
/*                             The privat RSA key must be      */
/*			       stored in the ICC.              */
/*							       */
/*   destination	       Call encryption function in SCT */
/*			       or in ICC		       */
/*			       in case of RSA, destination     */
/*			       must be set to ICC	       */
/*							       */
/*   sec_mess                  Specification of the security   */
/*			       modes for the data exchange     */
/*			       between SCT and ICC.	       */
/*							       */
/*   more		       = SEC_MORE -> more data are     */
/*				             expected          */
/*   			       = SEC_END  -> Last data for this*/
/*				         decryption   process. */
/*                                                             */
/* OUT							       */
/*   out_octets->octets        decrypted data                  */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*				 EPARINC 		       */
/*				 EALGO		       	       */
/*				 EMEMAVAIL		       */
/*				 EFUNCTION		       */
/* CALLED FUNCTIONS					       */
/*  STACheckSCT_ICC               ERROR-Codes		       */
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
/*  STACheckRSAKeyId          ERROR-Codes		       */
/*			         EPARINC		       */
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
/*  STACreateHead         	ERROR-Codes    		       */
/*                               EPARINC 		       */
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
/*  aux_ObjId2AlgType    				       */
/*  aux_ObjId2AlgEnc    				       */
/*  aux_ObjId2AlgSpecial				       */
/*  aux_cmp_ObjId					       */
/*  aux_free2_OctetString				       */
/*   malloc macro in MF_check.h			               */ 
/*   free macro in MF_check.h			               */ 
/*							       */
/*  Achtung !!!						       */
/*  Da in der Smartcard der CBC-Mode nur auf ein Kommando      */
/*  angewendet wird, d.h. Texte, die laenger als 80 bzw. 72    */
/*  Bytes sind werden in der ICC nicht richtig behandelt, da   */
/*  bei jedem ICC_CRYPT-Kommando der erste Block wieder mit dem*/
/*  Initialvektor (0) entschluesselt wird und nicht wie beim   */
/*  CBC-Mode mit dem letzten Chiffratblock des letzten         */
/*  ICC_CRYPT-Kdos werden von STARMOD folgende Funktionen      */
/*  durchgefuehrt:       				       */
/*  Der gesamte Text wird in 8 Byte-Bloecke versendet.         */
/*  Die zu versendenden 8 Chiffreoctets werden als Initialvektor*/
/*  gesichert.   					       */
/*  Die von der Karte empfangenen Bytes werden mit   	       */
/*  dem Initialvektor geodert => Klartext.                     */
/*  							       */
/*							       */
/*-------------------------------------------------------------*/
int
sca_decrypt(sct_id, in_octets, out_octets, decryption_key, destination, sec_mess, more)
	int             sct_id;
	OctetString    	*in_octets;
	OctetString    	*out_octets;
	KeySel         	*decryption_key;
	Destination     destination;
	ICC_SecMess     *sec_mess;
	More            more;
{
	static char     	cbc_initvek[8];
	static int      	FIRST_CRYPT_CMD;
	static AlgEnc   	algenc    = NoAlgEnc;
	static AlgType  	algtype;
	static AlgSpecial	algspecial= NoAlgSpecial;
	static Destination 	dec_destination;
	static int      	max_length;
	static int      	blksize;
	static int		send_data_len;
	static int      	out_rest_len = 0;
	static char    		*out_rest = NULL;
	static KeyId		icc_keyid;	
	int			*ptr_parm_int;
	static Boolean		already_call;
	int             	rc, i, act_length, dec_len, offset, out_conc_len;
	OctetString     	trans;
	Boolean			NOTENDE;
	More            	cmd_more;
	char            	*in_ptr, *out_ptr, *out_conc;
	char			dummy_data='E';
	char			c;
	char			*proc="sca_decrypt";



	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;
	out_conc = NULL;


#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_decrypt *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id                 : %d\n", sct_id);
	print_OctetString("in_octets		        :\n",
			   in_octets);
	if (out_octets)
		fprintf(stdout, "out_octets->noctets    : %d\n",
				out_octets->noctets);
	else
		fprintf(stdout, "out_octets             : NULL\n");



	if (act_function == F_null) {	/* first call of sca_decrypt */
		fprintf(stdout, "First call of sca_decrypt / trace decryption_key \n");

		print_KeySel(decryption_key,FALSE);
		print_Destination(destination);
	}
	else {
		fprintf(stdout, "Next call of sca_decrypt  \n");
		fprintf(stdout, "decryption_key          : is not evaluated\n");
		print_Destination(dec_destination);
	}

	if (destination == ICC)
		print_ICC_SecMess(sec_mess);
	else
		fprintf(stdout, "sec_mess                : is not evaluated in case of destination != ICC\n");
	print_More(more);

#endif


/*****************************************************************************/
/*
 *      Check input parameters
 */

	if ((in_octets == NULL) ||
	    (in_octets->noctets == 0) ||
	    (in_octets->octets == NULL)) {
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		goto errcase;
	}


	if ((out_octets == NULL) ||
	    (out_octets->octets == NULL)) {
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		goto errcase;
	}


	if ((more != SEC_END) && (more != SEC_MORE)) {
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		goto errcase;
	}





/*****************************************************************************/
/*
 *      if first call => check destination, decryption_key
 *			 and smartcard
 */


	if (act_function == F_null) {	/* first call of sca_decrypt */

		out_rest_len 	= 0;
		out_rest 	= NULL;
		already_call	= FALSE;
		blksize         = 0;
		send_data_len   = 0;

		if (decryption_key == NULL) {
			sca_errno = EPARINC;
			sca_errmsg = sca_errlist[sca_errno].msg;
			goto errcase;
		}

		if ((!decryption_key->key_algid) || 
		    (!decryption_key->key_algid->objid)) {
			sca_errno = EPARINC;
			sca_errmsg= sca_errlist[sca_errno].msg;
			return (-1);
		}


		if (destination == SCT) 
			icc_expect = FALSE;
		else {
			if (destination == ICC)
				icc_expect = TRUE;
		     	else {
				sca_errno = EPARINC;
				sca_errmsg = sca_errlist[sca_errno].msg;
				return(-1);
		     	}
		} 

		dec_destination   		= destination;
		/*
 		 *      call STACheckSCT_ICC
 		 */
		if (STACheckSCT_ICC(sct_id, icc_expect, proc) == -1)
			goto errcase;

		algspecial = aux_ObjId2AlgSpecial(decryption_key->key_algid->objid);
	 	algtype    = aux_ObjId2AlgType(decryption_key->key_algid->objid);
	 	algenc 	   = aux_ObjId2AlgEnc(decryption_key->key_algid->objid);


	 	if (algtype != ASYM_ENC && algtype != SYM_ENC) {
			sca_errno = EALGO;
			sca_errmsg = sca_errlist[sca_errno].msg;
			goto errcase;
		}


		switch(algenc) {

		case RSA:

			/*
	 		 * check destination                                   
	 		 */

			if (dec_destination != SCT) {
				sca_errno = EPARINC;
				sca_errmsg = sca_errlist[sca_errno].msg;
				goto errcase;
			} 
			/*
			 * check decryption_key->key_id                  
			 */

			if ((!decryption_key->key_id) ||
		    	    (STACheckRSAKeyId(decryption_key->key_id, 
					      &icc_keyid) == -1)) {

				sca_errno = EPARINC;
				sca_errmsg = sca_errlist[sca_errno].msg;
				goto errcase;
			}


			/*
			 * in case of RSAEncryption more must be set to SEC_END.
			 */

	    		if (!aux_cmp_ObjId(decryption_key->key_algid->objid,
			                   rsaEncryption->objid)) {
				if (more != SEC_END) {
 					sca_errno = EPARINC;
					sca_errmsg = sca_errlist[sca_errno].msg;
					goto errcase;
				}
			}


	    		if (!aux_cmp_ObjId(decryption_key->key_algid->objid,
			                   rsa->objid)) {
				/* 
				 * save keysize for errorcase
				 */
				ptr_parm_int = (int *) (decryption_key->key_algid->param);
				blksize      = (*ptr_parm_int / 8);
			}

			max_length  = SCT_RSA_DEC_DATLEN;

			Gsct_cmd = S_RSA_DEC;
			Gsct_request.rq_p1.kid = &icc_keyid;
			act_length = in_octets->noctets;
			in_ptr = in_octets->octets;
			break;

		case DES:
		case DES3:
			/*
			 * if destination == ICC => only DES is allowed 
			 */
			if ((dec_destination == ICC) && (algenc == DES3)) {
				sca_errno = EALGO;
				sca_errmsg = sca_errlist[sca_errno].msg;
				goto errcase;
			}
			if (dec_destination == ICC) {
				/*
	 		 	 * Check key_id
				 */
				if (icc_check_KeyId(decryption_key->key_id) == -1) {
					sca_errno = icc_errno;
					sca_errmsg = icc_errmsg;
					goto errcase;
				}
				icc_keyid.key_level  = decryption_key->key_id->key_level;

				icc_keyid.key_number = decryption_key->key_id->key_number;

				max_length = 8;
				icc_expect = TRUE;
				FIRST_CRYPT_CMD = TRUE;
				for (i = 0; i < 8; i++)
					cbc_initvek[i] = *(in_octets->octets + i);
				if (more == SEC_END) {
					if ((in_octets->noctets % 8) > 0) {
						sca_errno = EPARINC;
						sca_errmsg = sca_errlist[sca_errno].msg;
						goto errcase;
					}
					in_ptr = in_octets->octets;
					act_length = in_octets->noctets;

				}
				else {/* more == SEC_MORE */
					if ((in_octets->noctets % 8) > 0) {
						/* save rest for next transfer */
						out_rest_len = in_octets->noctets % 8;
						act_length = in_octets->noctets - out_rest_len;
						ALLOC_CHAR(out_rest, out_rest_len);

						offset = in_octets->noctets - out_rest_len;
						for (i = 0; i < out_rest_len; i++) {
							*(out_rest + i) = *(in_octets->octets + offset);
							offset++;
						}
						if (act_length == 0) {
							out_octets->noctets = 0;
							act_function = F_decrypt;
							return (0);
						}
						in_ptr = in_octets->octets;

					} 
					else {
						in_ptr = in_octets->octets;
						act_length = in_octets->noctets;
					}
				}
			} 
			else {
				/*
				 * set maximal data length depending on 
				 * secure messaging between DTE and SCT
				 * in case of NORMAL, INTEGRITY and CONCEALED
				 * the max. data length = 240
				 * else max. data length = 240 - 4 byte CCS
				 * sctinfo was set in STACheckSCT_ICC
				 */
				if (sctinfo.secure_messaging.command == SCT_COMBINED) 
					max_length = SCT_DES_ENC_DEC_DATLEN - 4;
				else
					max_length = SCT_DES_ENC_DEC_DATLEN;

				Gsct_cmd = S_DES_DEC;
				icc_keyid.key_level  = decryption_key->key_id->key_level;

				icc_keyid.key_number = decryption_key->key_id->key_number;
				Gsct_request.rq_p1.kid = &icc_keyid;
				act_length = in_octets->noctets;
				in_ptr = in_octets->octets;
				blksize= 8;

			};

			break;
		default:
			sca_errno = EALGO;
			sca_errmsg = sca_errlist[sca_errno].msg;
			goto errcase;

		} /* end switch */


		/*
		 * prepare outgoing data
		 */

		act_function = F_decrypt;



	} 
	else {			/* not first call of sca_decrypt */
		if (act_function != F_decrypt) {
			sca_errno = EFUNCTION;	/* wrong function call */
			sca_errmsg = sca_errlist[sca_errno].msg;
			goto errcase;
		};

		switch (dec_destination) {

		case SCT:
			in_ptr = in_octets->octets;
			act_length = in_octets->noctets;
			break;
		case ICC:
			if (out_rest_len > 0) {
				out_conc_len = in_octets->noctets + out_rest_len;
			} else
				out_conc_len = in_octets->noctets;

			ALLOC_CHAR(out_conc, out_conc_len);

			for (i = 0; i < out_rest_len; i++)
				*(out_conc + i) = *(out_rest + i);

			if(out_rest) free(out_rest);
			out_rest = NULL;

			offset = out_rest_len;
			for (i = 0; i < in_octets->noctets; i++) {
				*(out_conc + offset) = *(in_octets->octets + i);
				offset++;

			}
			out_rest_len = 0;
			if (more == SEC_END) {
				if ((out_conc_len % 8) > 0) {
					sca_errno = EPARINC;
					sca_errmsg = sca_errlist[sca_errno].msg;
					free(out_conc);
					goto errcase;
				} else {
					in_ptr = out_conc;
					act_length = out_conc_len;
				}
			} else {/* more == SEC_MORE */
				if ((out_conc_len % 8) > 0) {
					/* save rest for next transfer */
					out_rest_len = out_conc_len % 8;
					act_length = out_conc_len - out_rest_len;

					ALLOC_CHAR(out_rest, out_rest_len);

					offset = out_conc_len - out_rest_len;
					for (i = 0; i < out_rest_len; i++) {
						*(out_rest + i) = *(out_conc + offset);
						offset++;
					}
					if (act_length == 0) {
						if(out_conc) free(out_conc);
						return (0);
					}
					in_ptr = out_conc;
				} else {
					in_ptr = out_conc;
					act_length = out_conc_len;
				}
			}
			break;
		} /* end switch dec_destination */
	}





/*****************************************************************************/
/*
 *      Prepare parameters for the SCT Interface			      
 * 	
 */
	NOTENDE = TRUE;
	out_ptr = out_octets->octets + out_octets->noctets;
	dec_len = 0;
	do {
		if (act_length <= max_length) {
			trans.noctets = act_length;
			cmd_more = more;

			NOTENDE = FALSE;
		} else {
			trans.noctets = max_length;
			cmd_more = SEC_MORE;
		}
		switch (dec_destination) {
		case SCT:

			Gsct_request.rq_p2.more = cmd_more;
			trans.octets = in_ptr;

			Gsct_request.rq_datafield.chiffrat = &trans;
			/*-----------------------------------------------------*/
			/* Call SCT Interface     			   */
			/*-----------------------------------------------------*/
			rc = sct_interface(sct_id, Gsct_cmd, &Gsct_request, &Gsct_response);
			if (rc < 0) {
				sca_errno = sct_errno;
				sca_errmsg = sct_errmsg;
				STAErrAnalyse(sct_id);
				send_data_len 	    += trans.noctets;
				goto errcase;
			};

			/*
		 	 * in case of SEC_MORE, mark, that the function was
		 	 * already successfully called
		 	 */
			if (cmd_more == SEC_MORE) {
				already_call = TRUE;
			}
			


			break;
		default:
			/*-------------------------------------*/
			/* create ICC command CRYPT            */
			/*-------------------------------------*/
			/* create header                       */
			if (STACreateHead(ICC_CRYPT, sec_mess))
				goto errcase;


			/* set parameters			  */
			PAR_KID  = &icc_keyid;
			PAR_MODI = ICC_DEC;

			trans.octets = in_ptr;
			PAR_DATA     = &trans;



			/* call STACreateTrans			  */
			if (STACreateTrans(sct_id, TRUE))
				goto errcase;

			if (FIRST_CRYPT_CMD == FALSE) {
				for (i = 0; i < 8; i++)
					*(Gsct_response.octets + i) = *(Gsct_response.octets + i) ^ cbc_initvek[i];


				/* save cbc_initvek */

				for (i = 0; i < 8; i++)
					cbc_initvek[i] = *(in_ptr + i);
			} else
				FIRST_CRYPT_CMD = FALSE;

			break;

		}
		in_ptr += trans.noctets;
		act_length -= trans.noctets;
		for (i = 0; i < Gsct_response.noctets; i++) {
			*out_ptr = *(Gsct_response.octets + i);
			out_ptr++;
		}


#ifdef TEST
	fprintf(stdout, "TRACE of the decrypted string : \n");
	fprintf(stdout, "Gsct_response              : \n");
	fprintf(stdout, "    Gsct_response.noctets             : %d\n", Gsct_response.noctets);
	fprintf(stdout, "    Gsct_response.octets              : \n");
	aux_fxdump(stdout, Gsct_response.octets, Gsct_response.noctets, 0);

#endif

/*		out_octets->noctets += Gsct_response.noctets; */
		dec_len 	    += Gsct_response.noctets;
		send_data_len 	    += trans.noctets;

		/*
		 * Normal End	 (Release storage)		  
		 */
/*  		Gsct_response.noctets = 1; */
/*		aux_free2_OctetString(&Gsct_response); */

	} while (NOTENDE);

/*****************************************************************************/
/*
 *      Normal End	 (Release storage)			      
 * 	
 */
	if(out_conc) free(out_conc);

/*****************************************************************************/
/*
 *      if last call of decryption function			      
 * 	
 */
	if (more == SEC_END) {	/* last call of sca_decrypt */
		algenc 		= NoAlgEnc;
		act_function 	= F_null;
		blksize         = 0;
		send_data_len   = 0;
		already_call    = FALSE;
		/*
		 * remove padding bytes
		 */


		switch (algspecial) {
		case WITH_B1_PADDING:
			out_ptr--;
			c = *out_ptr;
			/* test last padding byte */
			if ((*out_ptr != 0x00) && 
			    ((*out_ptr & 0xFF) != 0x80)) {
				sca_errno = EPADD;
				sca_errmsg = sca_errlist[sca_errno].msg;
				goto errcase;
			}
			i = 1;
			while (c == 0x00) {
				i++;
				out_ptr--;
				c = *out_ptr;
			}
			    
			if ((*out_ptr & 0xFF) != 0x80) {
				sca_errno = EPADD;
				sca_errmsg = sca_errlist[sca_errno].msg;
				goto errcase;
			}
			out_octets->noctets -= i;
			dec_len -= i;
			break;

		case WITH_PEM_PADDING:
			out_ptr--;
			i = *out_ptr;

			if (i <= 0 || i > 8) {
				sca_errno = EPADD;
				sca_errmsg = sca_errlist[sca_errno].msg;
				goto errcase;
			}
			dec_len -= i;
			out_octets->noctets += dec_len;
			break;

		case PKCS_BT_02:
			out_ptr = out_octets->octets + out_octets->noctets;

			i = 0;
			c = *(Gsct_response.octets + i);
			while ((i < dec_len) && (c != 0x00)) {
				c = *(Gsct_response.octets + i);
				i++;

			}
			if ((i < 2) ||
			    (i == dec_len)) {
				sca_errno = EPADD;
				sca_errmsg = sca_errlist[sca_errno].msg;
				goto errcase;
			}
			dec_len -= i;
			out_octets->noctets += dec_len;
			while (i < Gsct_response.noctets) {
				*out_ptr = *(Gsct_response.octets + i);
#ifdef TEST
				fprintf(stdout, "    1 octet              : \n");
				aux_fxdump(stdout, out_ptr, 1, 0);
#endif
				
				out_ptr++;
				i++;
			}
			break;

		case NoAlgSpecial:
			out_octets->noctets += dec_len;
			break;

		};
		algspecial	= NoAlgSpecial;
	};

	aux_free2_OctetString(&Gsct_response); 



#ifdef TEST
	fprintf(stdout, "TRACE of the output parameters : \n");
	fprintf(stdout, "out_octets              : \n");
	fprintf(stdout, "    out_octets->noctets             : %d\n", out_octets->noctets);
	fprintf(stdout, "    dec_len             : %d\n", dec_len);
	fprintf(stdout, "    octets              : \n");
	aux_fxdump(stdout, out_octets->octets, dec_len, 0);

	fprintf(stdout, "\n***** Normal end of   sca_decrypt *****\n\n");
#endif

	return (dec_len);


/*****************************************************************************/
/*
 *      In error case release all allocated storage			      
 * 	
 */
errcase:
	algenc 		= NoAlgEnc;
	algspecial	= NoAlgSpecial;
	act_function 	= F_null;
	if(out_conc) free(out_conc);
  	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);

	
	/*
	 * if the decrypt function in the SCT was already successfully called
 	 * by MORE, this function has to be ended
	 */
	if ((already_call == TRUE) && (sca_errno != ESTATE0)) {
		Gsct_request.rq_p2.more = SEC_END;
		trans.noctets = blksize - (send_data_len % blksize);
#ifdef TEST
		fprintf(stdout, "send_data_len           :%d\n",send_data_len);
		fprintf(stdout, "blksize                 :%d\n",blksize);
		fprintf(stdout, "trans_noctets           :%d\n",trans.noctets);
#endif
		


		trans.octets = (char *) malloc(trans.noctets);
		if (trans.octets == NULL) {
			sca_errno = EMEMAVAIL;
			sca_errmsg = sca_errlist[sca_errno].msg; 
			return(-1);
		}
		for (i = 0; i < trans.noctets; i++)
			*(trans.octets + i) = dummy_data;

		Gsct_request.rq_datafield.chiffrat = &trans;
		/*
		 * Call SCT Interface     			  
		 */
		rc = sct_interface(sct_id, Gsct_cmd, &Gsct_request,
			           &Gsct_response);
		trans.noctets = 1;
  		Gsct_response.noctets = 1;
		aux_free2_OctetString(&trans);
		aux_free2_OctetString(&Gsct_response);
	}

	already_call = FALSE;
	blksize = 0;
	send_data_len = 0;
	

	if(out_rest) free(out_rest);
	out_rest = NULL;
	out_rest_len = 0;
	return (-1);


}				/* end sca_decrypt */


/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_decrypt 	       */
/*-------------------------------------------------------------*/


/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_hash                VERSION   1.1	    	       */
/*				     DATE   August 1993	       */
/*			      	       BY   L.Eckstein         */
/*							       */
/* DESCRIPTION						       */
/*  Hash octetstring in the DTE			               */
/*							       */
/*  A smartcard is not expected.			       */
/*							       */
/*							       */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*			       In the current version the hash */
/*			       functions are implemented within*/
/*			       the PC, therefore this parameter*/
/*			       is not evaluated.	       */
/*                                                             */
/*   out_octets		       Data to be hashed               */
/*                                                             */
/*   alg_id                    Algorithm Identifier            */
/*			       The following values are        */
/*			       possible:		       */
/*			       sqmodn			       */
/*			       md2			       */
/*			       md4			       */
/*			       md5			       */
/*			       sha			       */ 
/*                                                             */
/*   hash_par                  Additional algorithm (hash_alg) */
/*                             specific parameters or the NULL */
/*                             pointer.                        */
/*			       If the hash function 'sqmodn'   */
/*			       shall be used, the public RSA   */
/*			       key must be delivered in this   */
/*			       parameter.		       */
/*							       */
/*   more		       = SEC_MORE -> more data         */
/*					     are expected      */
/*   			       = SEC_END  -> Last data for this*/
/*				             process.          */
/*                                                             */
/* OUT							       */
/*   hash_result               Hash result in case of more=SEC_END */
/*			       Memory is provided by the called */
/*			       program.			       */
/*                                                             */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*				 EALGO 		               */
/*				 EMEMAVAIL		       */
/*				 EFUNCTION		       */
/*				 EPARINC		       */
/*				 EHASHFUNC		       */
/*				 EKEY			       */ 
/*							       */
/* CALLED FUNCTIONS					       */
/*							       */
/*							       */
/*							       */
/*  rsa_set_key		       ERROR-Codes		       */
/*				 -1 -> EKEY                    */
/*							       */
/*  hash_sqmodn 	       ERROR-Codes     	               */
/*				 -1 -> EHASHFUNC               */
/*  md2_hash	               ERROR-Codes      	       */
/*				 -1 -> EHASHFUNC               */
/*  md4_hash	       	       ERROR_Codes      	       */
/*				 -1 -> EHASHFUNC               */
/*  md5_hash	               ERROR-Codes      	       */
/*				 -1 -> EHASHFUNC               */
/*  sha_hash	               ERROR-Codes      	       */
/*				 -1 -> EHASHFUNC               */
/*							       */
/*  aux_free_OctetString		    	               */
/*  aux_ObjId2AlgHash					       */
/*							       */
/*   malloc macro in MF_check.h			               */ 
/*-------------------------------------------------------------*/
int
sca_hash(sct_id, out_octets, alg_id, hash_par, more, hash_result)
	int             sct_id;
	OctetString    *out_octets;
	AlgId          *alg_id;
	HashPar        *hash_par;
	More            more;
	OctetString    *hash_result;
{
 	int             	rc, i, memolen;
	static AlgHash  	alghash = NoAlgHash;
	static Boolean		first_call   = TRUE;
	static Boolean		already_call;
	static OctetString 	*hash_cop_result;
	static AlgId		*hash_algid;
	OctetString		dummy_octets;
	char			*proc="sca_hash";

	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;

 


/*****************************************************************************/
/*
 *      set function specific variables
 */
	if (first_call) {
		if ((alg_id) && (alg_id->objid)) { 
			alghash = aux_ObjId2AlgHash(alg_id->objid);
			hash_algid = alg_id;
		}	
	}



#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_hash *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id		        : %d\n", sct_id);
	print_OctetString("out_octets    		:\n",
			   out_octets);


	if (alghash != NoAlgHash) {
		switch (alghash) {
		case SQMODN:
			fprintf(stdout, "alg_id                  : sqmodn\n");
			print_HashPar(hash_par);
			break;
		case MD2:
			fprintf(stdout, "alg_id                  : md2\n");
			fprintf(stdout, "hash_par                : is not evaluated\n");
			break;
		case MD4:
			fprintf(stdout, "alg_id                  : md4\n");
			fprintf(stdout, "hash_par                : is not evaluated\n");
			break;
		case MD5:
			fprintf(stdout, "alg_id                  : md5\n");
			fprintf(stdout, "hash_par                : is not evaluated\n");
			break;
		case SHA:
			fprintf(stdout, "alg_id                  : sha\n");
			fprintf(stdout, "hash_par                : is not evaluated\n");
			break;
		default:
			fprintf(stdout, "alg_id                  : not allowed\n");
			fprintf(stdout, "hash_par                : is not evaluated\n");
			break;
		}
	}
	else
			fprintf(stdout, "alg_id                  : NULL\n");
		
	print_More(more);

#endif

/*****************************************************************************/
/*
 *      initialize return parameter
 */
	if (hash_result) {
		hash_result->noctets = 0;
		hash_result->octets = NULL;
	}



/*****************************************************************************/
/*
 *      if first call of hash function then
 *	1) check algorithm identifier 
 * 	2) if algorithm = RSA-sqmodn then               
 *  	   a) check hash-parameter         
 * 	   b) set key (modulus) for hash-function                     
 * 	3) allocate storage for hash result 
 *	else
 *	   check input parameter       
 */

	if (act_function == F_null) {	/* first call of sca_hash */

		
		/*
 		 *      initialize static variables
 		 */
		already_call    = FALSE;
		hash_cop_result = NULLOCTETSTRING;

		if (alghash == SQMODN) {
			if ((hash_par == NULL) ||
 			    (hash_par->sqmodn_par.part1.octets == NULL)) {
				sca_errno = EPARINC;
				sca_errmsg = sca_errlist[sca_errno].msg;
				goto errcase;
			}

			/* set key in an internal function for hash-function */
			if ((rc = rsa_set_key(&hash_par->sqmodn_par, 0)) < 0) {
				sca_errno = EKEY;
				sca_errmsg = sca_errlist[sca_errno].msg;
				goto errcase;
			}
			memolen = (RSA_PARM(hash_algid->param) + 7) / 8;
		}
		else
			/*
			 * MD2, MD4, MD5 => length of hash string = 128 bit
			 * SHA		 => length of hash string = 160 bit
			 */
			memolen = 20;

		/*
		 * allocate storage for hash_cop_result  
		 */

#ifdef TEST
		fprintf(stdout, "memolen: %d \n", memolen);
#endif

		ALLOC_OCTET(hash_cop_result, OctetString);
		hash_cop_result->noctets = 0;
		hash_cop_result->octets  = NULL;
		ALLOC_CHAR(hash_cop_result->octets, memolen);

		act_function    =  F_hash;
		first_call	=  FALSE;

	}
	/* end if (act_function == F_null) */
	else
	 /* not first call of sca_hash */ 
		if (act_function != F_hash) {
			sca_errno = EFUNCTION;	/* wrong function call */
			sca_errmsg = sca_errlist[sca_errno].msg;
			goto errcase;
		}

/*****************************************************************************/
/*
 *      check input parameter
 */

	if ((out_octets == NULLOCTETSTRING) ||
	    (out_octets->octets == NULL)) {
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
 		goto errcase;
	}

	if ((more != SEC_END) && (more != SEC_MORE)) {
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
 		goto errcase;
	}


/*****************************************************************************/
/*
 * 	Call hash function depending on algorithm          
 */
	switch (alghash) {
	case SQMODN:
		rc = hash_sqmodn(out_octets, hash_cop_result, more,
				 RSA_PARM(hash_algid->param));
		break;
	case MD2:
		rc = md2_hash(out_octets, hash_cop_result, more);
		break;
	case MD4:
		rc = md4_hash(out_octets, hash_cop_result, more);
		break;
	case MD5:
		rc = md5_hash(out_octets, hash_cop_result, more);
		break;
	case SHA:
		rc = sha_hash(out_octets, hash_cop_result, more);
		break;
	default:
		sca_errno = EALGO;
		sca_errmsg = sca_errlist[sca_errno].msg;
		goto errcase;


	}			/* end switch */
	if (rc < 0) {
		sca_errno = EHASHFUNC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		goto errcase;
	}
	else {
		/*
		 * in case of SEC_MORE, mark, that the function was
		 * already successfully called
		 */
		if (more == SEC_MORE)
			already_call = TRUE;
	}

/*****************************************************************************/
/*
 * 	Now hashing is done         
 * 	If last call of sca_hash ( more = SEC_END) return hash_result
 */
	if (more == SEC_END) {	/* last call of sca_hash */
		already_call = FALSE;		/* hash function correctly ended */

		/* 
		 * check output parameter for hash    	          
		 */
		if (hash_result == NULLOCTETSTRING) {
			sca_errno = EPARINC;
			sca_errmsg = sca_errlist[sca_errno].msg;
			goto errcase;
		}

		/*
		 * allocate buffer for hash_result->octets and copy hash_cop_result
		 */
		ALLOC_CHAR(hash_result->octets, hash_cop_result->noctets);
		hash_result->noctets = hash_cop_result->noctets;
		for (i = 0; i < hash_result->noctets; i++)
			*(hash_result->octets + i) = *(hash_cop_result->octets + i);


		/*
		 * Release storage
		 */
		if (hash_cop_result) {
			aux_free_OctetString(&hash_cop_result);
		}

 

		alghash = NoAlgHash;
		act_function = F_null;
		first_call   = TRUE;

#ifdef TEST
		fprintf(stdout, "TRACE of the output parameters : \n");
		fprintf(stdout, "hash_result             : \n");
		fprintf(stdout, "    noctets             : %d\n", hash_result->noctets);
		fprintf(stdout, "    octets              : \n");
		aux_fxdump(stdout, hash_result->octets, hash_result->noctets, 0);
#endif


	}			/* end if (more == SEC_END) */
#ifdef TEST
	fprintf(stdout, "\n***** Normal end of   sca_hash *****\n\n");
#endif

	return (0);

/*****************************************************************************/
/*
 *      error
 */
errcase:
	act_function = F_null;
	first_call   = TRUE;
	
	/*
	 * if a hash function was already successfully called by MORE,
	 * this function has to be ended
	 */
	if (already_call) {
		dummy_octets.noctets = 0;
		switch (alghash) {
		case SQMODN:
			rc = hash_sqmodn(&dummy_octets, hash_cop_result,
					SEC_END,
				 	RSA_PARM(hash_algid->param));
			break;
		case MD2:
			rc = md2_hash(&dummy_octets, hash_cop_result, SEC_END);
			break;
		case MD4:
			rc = md4_hash(&dummy_octets, hash_cop_result, SEC_END);
			break;
		case MD5:
			rc = md5_hash(&dummy_octets, hash_cop_result, SEC_END);
			break;
		case SHA:
			rc = sha_hash(&dummy_octets, hash_cop_result, SEC_END);
			break;
		}
		already_call = FALSE;
	}
	alghash = NoAlgHash;
	if (hash_cop_result) {
		aux_free_OctetString(&hash_cop_result);
	}

	return(-1);


}				/* end sca_hash */


/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_hash    	       */
/*-------------------------------------------------------------*/

/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_enc_des_key         VERSION   1.1	    	       */
/*				     DATE   August 1993	       */
/*			      	       BY   L.Eckstein         */
/*							       */
/* DESCRIPTION						       */
/*  Encrypt a DES key in the SCT with RSA		       */
/*							       */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   encryption_key            Structure which identifies the  */
/*			       the encryption key.             */
/*                                                             */
/*   plain_key                 Key identifier of the key to be */
/*                             encrypted                       */
/*							       */
/*							       */
/* OUT							       */
/*   encrypted_key             Structure which contains  the   */
/*			       AlgId of the encryption key, the*/
/*			       AlgId of the encrypted key and  */
/*			       the encrypted key.	       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*				 EALGO 		               */
/*				 EPARINC		       */
/*				 EMEMAVAIL		       */
/*				 EKEYINV		       */
/*							       */
/* CALLED FUNCTIONS					       */
/*  STACheckSCT_ICC               ERROR-Codes		       */
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
/*  STAcompare						       */
/*							       */
/*  aux_free2_OctetString				       */
/*  aux_free_BitString					       */
/*  aux_cmp_ObjId					       */
/*  aux_ObjId2AlgSpecial				       */
/*  sec_random_bstr					       */	
/*							       */
/*   malloc macro in MF_check.h			               */ 
/*-------------------------------------------------------------*/
int
sca_enc_des_key(sct_id, encryption_key, plain_key, encrypted_key)
	int             sct_id;
	KeySel         *encryption_key;
	KeySel         *plain_key;
	EncryptedKey   *encrypted_key;
{
	int             rc, i, des_keylen, random_len;
	char		*key_buffer;
	AlgSpecial   	algspecial;
	KeyAlgId	des_algid;
     	EncDesKey	enckey;
     	Public    	public;
	BitString	*random;
	OctetString	padding;
	char		*proc="sca_enc_des_key";




	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;
	key_buffer = NULL;

#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_enc_des_key *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id		        : %d\n", sct_id);
	fprintf(stdout, "encryption_key	        : \n");
	print_KeySel(encryption_key,TRUE);
	fprintf(stdout, "plain_key	        : \n");
	print_KeySel(plain_key,FALSE);

#endif

/*****************************************************************************/
/*
 *      check input parameter
 */

	/*
	 * check encryption key
	 * alg_id must be rsa or rsaEncryption
         * keysize is not check 
	 */


	if ((!encryption_key) ||
	    (!encryption_key->key_algid) || 
	    (!encryption_key->key_algid->objid)) {
		sca_errno = EALGO;
		sca_errmsg= sca_errlist[sca_errno].msg;
		return (-1);
	}

	if ((aux_cmp_ObjId(encryption_key->key_algid->objid,rsa->objid)) &&
	    (aux_cmp_ObjId(encryption_key->key_algid->objid,
			   rsaEncryption->objid))) {
		sca_errno = EALGO;
		sca_errmsg = sca_errlist[sca_errno].msg;
		return (-1);
	}
	if ((!encryption_key->key_bits) ||
	    (!encryption_key->key_bits->part1.octets) ||
	    (encryption_key->key_bits->part1.noctets == 0)) {
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		return (-1);
	}

	if (encryption_key->key_bits->part2.octets) {
	    	if ((encryption_key->key_bits->part2.noctets != fermat_f4_len ) ||
                    (STAcompare(encryption_key->key_bits->part2.octets,
                     	        fermat_f4,fermat_f4_len)) != 0) {
        		/* exponent not = Fermatzahl F4 */
			sca_errno = EPARINC;
			sca_errmsg = sca_errlist[sca_errno].msg;
			return (-1);
		}
	}



	/*
	 * check plain key
	 * key_id must be set
	 * alg_id must be NULL, desCBC or desCBC3
	 */
	if ((!plain_key ) ||
	    (!plain_key->key_id)) {
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		return (-1);
	}
	if (plain_key->key_algid) {
		if (plain_key->key_algid->objid) {
			if (!aux_cmp_ObjId(plain_key->key_algid->objid,
					   desCBC->objid)) {
				des_algid     = S_DES_CBC; 
				des_keylen = 8;	
			}
			else {
	    	     		if (!aux_cmp_ObjId(plain_key->key_algid->objid,
				    desCBC3->objid)) {
					des_algid     = S_DES_3_CBC;
					des_keylen = 16;
				} 
				else { 
					sca_errno = EALGO;
					sca_errmsg = sca_errlist[sca_errno].msg;
					return (-1);
				}
			}
		}
		else {
			sca_errno = EALGO;
			sca_errmsg = sca_errlist[sca_errno].msg;
			return (-1);
		}
	}

	/*
	 * check encrypted key
	 */
	if (!encrypted_key) { 
		sca_errno = EPARINC;
		sca_errmsg= sca_errlist[sca_errno].msg;
		return (-1);
	}
	else {
		/*
		 * initialize return parameter
		 */ 
		encrypted_key->encryptionAI     = NULL;
		encrypted_key->subjectAI        = NULL;
		encrypted_key->subjectkey.nbits = 0;
		encrypted_key->subjectkey.bits  = NULL;
	}

	

/*****************************************************************************/
/*
 *      call STACheckSCT_ICC
 */
	if (STACheckSCT_ICC(sct_id, FALSE, proc) == -1)
		return(-1);


/*****************************************************************************/
/*
 *      Get DES key (S_ENC_DES_KEY)
 *	Prepare parameters for the SCT Interface           
 */

 	Gsct_cmd = S_ENC_DES_KEY;
	Gsct_request.rq_p1.kid = plain_key->key_id;
        public.modulus = &encryption_key->key_bits->part1;
        enckey.public  = &public;
	
	

	/*
	 * check padding 
	 * if encryption_key->key_algid = RSA => Padding is done by the SCT
         * else padding is done in this function (PKCS)
	 * If the padding is done in this function and plain_key->algid = NULL
	 * then send a dummy S_ENC_DES_KEY-command to get the algorithm of 
	 * the DES key
	 */
	
	algspecial = aux_ObjId2AlgSpecial(encryption_key->key_algid->objid);

	if ((!plain_key->key_algid) &&
	    (algspecial == PKCS_BT_02)) {
		enckey.padding_string = NULLOCTETSTRING;
                Gsct_request.rq_datafield.enc_des_key = &enckey;
		/* 
	 	 * Call SCT Interface     			        
	 	 */
		rc = sct_interface(sct_id, Gsct_cmd, &Gsct_request,
				   &Gsct_response);
		if (rc < 0) {
			sca_errno = sct_errno;
			sca_errmsg = sct_errmsg;
			STAErrAnalyse(sct_id);
			return(-1);
		}
		if (*(Gsct_response.octets) == S_DES_CBC) {
			des_algid = S_DES_CBC;
			des_keylen = 8;	
		}		
		else {
			des_algid = S_DES_3_CBC;
			des_keylen = 16;	
		}


		/* 
	 	 *  Release storage 
	 	 */
  		Gsct_response.noctets = 1;
		aux_free2_OctetString(&Gsct_response);
	}

	/*
	 * in case of PKCS and RSA-Keylen = 1024 only des_algid = S_DES_3_CBC
	 * will be supported
	 */
	if ((algspecial == PKCS_BT_02) &&
	    (encryption_key->key_bits->part1.noctets >= 0x80) &&
	    (des_algid == S_DES_CBC)) {
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		return (-1);
	}


	/*
	 * in case of PKCS => create Padding string
	 */
	 if (algspecial == PKCS_BT_02) {

		random_len = encryption_key->key_bits->part1.noctets - 1 - des_keylen - 2;
		
		padding.noctets = 2 + random_len;
		ALLOC_CHAR(padding.octets, padding.noctets);

		/*
		 * generate random number
	         */
		if ((random = sec_random_bstr(random_len * 8)) == NULLBITSTRING) {
			sca_errno = EMEMAVAIL;
			sca_errmsg = sca_errlist[sca_errno].msg;
			return (-1);
		}
		*padding.octets	  = 0x02;	/* BT */
		for (i=0; i<random_len; i++) {
			if (!(*(random->bits + i))) 
				*(random->bits + i) = 0xff;
			*(padding.octets + i + 1) = *(random->bits + i);
		}
		*(padding.octets + random_len + 1) = 0x00;

#ifdef TEST
		fprintf(stdout,"random_len    = %d\n",random_len);
		fprintf(stdout,"random->nbits = %d\n",random->nbits);
		fprintf(stdout,"random->bits  = \n");

		aux_fxdump(stdout, random->bits,
				   random->nbits / 8, 0); 	

#endif
		random->nbits = 1;
		aux_free_BitString(&random);



	}
	else {
		padding.noctets = 0;
		padding.octets  = NULL;
	}

	enckey.padding_string = &padding;
        Gsct_request.rq_datafield.enc_des_key = &enckey;
	/* 
	 * Call SCT Interface     			        
	 */
	rc = sct_interface(sct_id, Gsct_cmd, &Gsct_request,&Gsct_response);
	if (rc < 0) {
		sca_errno = sct_errno;
		sca_errmsg = sct_errmsg;
		STAErrAnalyse(sct_id);
		padding.noctets = 1;
		aux_free2_OctetString(&padding);
		return(-1);
	}

	padding.noctets = 1;
	aux_free2_OctetString(&padding);

	/*
	 * check if received algid is identical with specified algid
	 */
	if (plain_key->key_algid) {
		if (*(Gsct_response.octets) != des_algid) {
			sca_errno = EKEYINV;
			sca_errmsg = sca_errlist[sca_errno].msg;
  			Gsct_response.noctets = 1;
			aux_free2_OctetString(&Gsct_response);
			return (-1);
		}
	}
 

	/*
	 * allocate buffer for encrypted_key->subjectkey.bits
	 */
	ALLOC_CHAR(key_buffer, Gsct_response.noctets - 1);

	for (i = 0; i < Gsct_response.noctets-1; i++)
		*(key_buffer + i) = *(Gsct_response.octets + i + 1);

	encrypted_key->encryptionAI     = encryption_key->key_algid;

	if (*(Gsct_response.octets) == S_DES_CBC) 
		encrypted_key->subjectAI = desCBC;
	else 
		encrypted_key->subjectAI = desCBC3;

	encrypted_key->subjectkey.nbits = (Gsct_response.noctets - 1) * 8;
	encrypted_key->subjectkey.bits  = key_buffer;



	/*
	 * Normal End	 (Release storage)		       
	*/
  	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);

#ifdef TEST
	fprintf(stdout, "TRACE of the output parameters : \n");
	print_EncryptedKey(encrypted_key);
	fprintf(stdout, "\n***** Normal end of   sca_enc_des_key *****\n\n");
#endif

	return (sca_errno);

/*****************************************************************************/
/*
 *      In error case release all allocated storage 			      
 */
errcase:
  	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);
	return (-1);



}				/* end sca_enc_des_key */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_enc_des_key        */
/*-------------------------------------------------------------*/

/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_dec_des_key         VERSION   1.1	    	       */
/*				     DATE   August 1992	       */
/*			      	       BY   L.Eckstein         */
/*							       */
/* DESCRIPTION						       */
/*  Decrypt an rsa-encrypted DES key and store it in the SCT.  */
/*							       */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   encrypted_key             Structure which identifies      */
/*			       the encrypted key               */
/*                                                             */
/*   plain_key                 Key identifier under which the  */
/*                             decrytpted key is to be stored  */
/*							       */
/*   decryption_key            Key identifier of the decryption*/
/*                             key.                            */
/*			       In the current version only a   */
/*			       private RSA key stored in the ICC*/
/*			       can be used.		       */
/*							       */
/*							       */
/* OUT							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*   1                           M_KEYREPL                     */
/*  -1			       error			       */
/*				 EPARINC 		       */
/*				 EALGO 		               */
/*							       */
/* CALLED FUNCTIONS					       */
/*  STACheckSCT_ICC               ERROR-Codes		       */
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
/*  STACheckRSAKeyId          ERROR-Codes		       */
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
/*  aux_free2_OctetString				       */
/*  aux_cmp_ObjId					       */
/*							       */
/*-------------------------------------------------------------*/
int
sca_dec_des_key(sct_id, encrypted_key, plain_key, decryption_key)
	int             sct_id;
	EncryptedKey   *encrypted_key;
	KeySel         *plain_key;
	KeySel         *decryption_key;
{
	int             rc;
	DESKey          deskey;
	OctetString     enckey;
	KeyAlgId        des_algid;
	KeyId		icc_keyid;
	char		*proc="sca_dec_des_key";


	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;


#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_dec_des_key *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id		        : %d\n", sct_id);
	print_EncryptedKey(encrypted_key);
	if (plain_key) {
		fprintf(stdout, "plain_key		: only key_id is evaluated\n");
		print_KeyId(plain_key->key_id);
	}
	else
		fprintf(stdout, "plain_key		: NULL\n");

	if (decryption_key) {
		fprintf(stdout, "decryption_key	        : only key_id is evaluated\n");
		print_KeyId(decryption_key->key_id);
	}
	else
		fprintf(stdout, "decryption_key 	: NULL\n");

#endif

/*****************************************************************************/
/*
 *      check input parameter
 */


	if ((!encrypted_key) ||
	    (!encrypted_key->encryptionAI) ||
	    (!encrypted_key->encryptionAI->objid) ||
	    (!encrypted_key->subjectAI) ||
	    (!encrypted_key->subjectAI->objid) ||
	    (encrypted_key->subjectkey.nbits == 0) ||
	    (!encrypted_key->subjectkey.bits)) {
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		return (-1);
	}

	if ((aux_cmp_ObjId(encrypted_key->encryptionAI->objid,rsa->objid)) &&
	    (aux_cmp_ObjId(encrypted_key->encryptionAI->objid,
			   rsaEncryption->objid))) {
		sca_errno = EALGO;
		sca_errmsg = sca_errlist[sca_errno].msg;
		return (-1);
	}


	if (!aux_cmp_ObjId(encrypted_key->subjectAI->objid, desCBC->objid)) 
		des_algid     = S_DES_CBC; 
	else {
	    	if (!aux_cmp_ObjId(encrypted_key->subjectAI->objid,
			           desCBC3->objid)) 
			des_algid     = S_DES_3_CBC;
		else { 
			sca_errno = EALGO;
			sca_errmsg = sca_errlist[sca_errno].msg;
			return (-1);
		}
	}

	if ((!plain_key) ||
	    (!plain_key->key_id)) {
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		return (-1);
	}


	if ((!decryption_key) ||
	    (STACheckRSAKeyId(decryption_key->key_id, &icc_keyid) == -1)) {
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		return(-1);
	}




/*****************************************************************************/
/*
 *      call STACheckSCT_ICC
 */
	if (STACheckSCT_ICC(sct_id, TRUE, proc) == -1)
		return (-1);

/*****************************************************************************/
/*
 *      Put DES key (S_DEC_DES_KEY)
 *	Prepare parameters for the SCT Interface           
 */

 	Gsct_cmd = S_DEC_DES_KEY;
	Gsct_request.rq_p1.kid = &icc_keyid;
	Gsct_request.rq_p2.kid = plain_key->key_id;
	enckey.noctets         = encrypted_key->subjectkey.nbits / 8;
	enckey.octets          = encrypted_key->subjectkey.bits;
	deskey.algid           = des_algid;

	deskey.chiffrat        = &enckey;
	Gsct_request.rq_datafield.deskey = &deskey;



	/*
	 * Call SCT Interface     			        
	 */
	rc = sct_interface(sct_id, Gsct_cmd, &Gsct_request, &Gsct_response);
	if (rc < 0) {
		sca_errno = sct_errno;
		sca_errmsg = sct_errmsg;
		STAErrAnalyse(sct_id);
		return (-1);
	}

	/* 
	 * Normal End	 (Release storage)		        
	 */
  	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);

	/*
	 * If an existing key in the SCT has been overwritten  
	 * then return (warning)			       
	 */
	if (rc == S_KEYREPL)
		sca_errno = M_KEYREPL;




#ifdef TEST
	fprintf(stdout, "\n***** Normal end of   sca_dec_des_key *****\n\n");
#endif


	return (sca_errno);


}				/* end sca_dec_des_key */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_dec_des_key        */
/*-------------------------------------------------------------*/



/************************** local functions: ***********************************/

/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  STAcompare             VERSION   1.1             */
/*                              DATE   June 1993          */
/*                                BY   L. Eckstein        */
/*                                                        */
/* DESCRIPTION                                            */
/*  compares two strings (independent of '\0') and returns*/
/*  0 if strings are equal.                               */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   s                         first string               */
/*                                                        */
/*   t                         second string              */
/*                                                        */
/*   no                        number of chars to be      */
/*                             compared                   */
/*                                                        */
/* OUT                                                    */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*    0                        strings are equal          */
/*                                                        */
/*   any                       strings are not equal      */
/*                                                        */
/*--------------------------------------------------------*/

static
int
STAcompare(s, t, no)
	char           *s;
	char           *t;
	int             no;
{
	int             i;

	for (i = 0; i < no; i++) {
		if (s[i] != t[i])
			break;
	}
	if (i < no)
		return (s[i] - t[i]);
	else
		return (0);

}				/* end STAcompare */



