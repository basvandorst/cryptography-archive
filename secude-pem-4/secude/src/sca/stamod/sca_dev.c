/*
 *  STARMOD Release 1.1 (GMD)
 */
/*---------------------------------------------------------------------------+-----*/
/*							                     | GMD */
/*   SYSTEM   STARMOD -  Version 1.1		                             +-----*/
/*							                           */
/*---------------------------------------------------------------------------------*/
/*							                           */
/*    PACKAGE	SCAIF                                 VERSION 1.1	           */
/*					                       DATE June 1993      */
/*					                         BY Levona Eckstein*/
/*			       				                           */
/*    FILENAME     					                           */
/*	sca_dev.c                        		         		   */
/*							                           */
/*    DESCRIPTION	   				                           */
/*      This modul provides all functions for device handling, the                 */
/*      SCT display  and secure messaging between DTE and SCT                      */
/*      of the smartcard application interface                                     */
/*      and global functions for all sca functions                                 */
/*							                           */
/*    EXPORT		    DESCRIPTION 		                           */
/*      Functions for Device Handling						   */
/*	sca_init_icc()	      Request and initialize a smartcard	           */
/*							                           */
/*	sca_get_sct_info()    Get information about registered SCTs 	           */
/*							                           */
/*	sca_eject_icc()	      Eject smartcard	                                   */
/*							                           */
/*      Function for SCT-Display						   */
/*	sca_display()	      Print text on SCT display                            */
/*      									   */
/*      Function for Secure Messaging						   */
/*	sca_gen_sessionkey()  Generate sessionkey(s)                               */
/*                                                                                 */
/*	sca_del_sessionkey()  Delete sessionkey(s)                                 */
/*                                                                                 */
/*	sca_set_mode()	      Set security mode                                    */
/*                                                                                 */
/*      sca_errno             global error variable                                */
/*                                                                                 */
/*      sca_errmsg            global pointer to error message                      */
/*							                           */
/*      Function for DTE/SCT communication handling			           */
/*	sca_reset	      reset function					   */
/*									           */
/*	sca_get_sctport       get port information				   */
/*									           */
/*      Global functions                                                           */
/*      STAGetSCTAlgId	      check alg_id and get SCT specific alg_id             */
/*                                                                                 */
/*      STACheckSCT_ICC()     check SCT and ICC                                    */
/*                                                                                 */
/*      STACheckKeyAttrList() check key attribute list                             */
/*                                                                                 */
/*      STACheckSecMess()     check security mode(s) for command and response      */
/*                                                                                 */
/*      STAErrAnalyse()       error analyse and handling                           */
/*                                                                                 */
/*	STACreateTrans        create icc command and send it via S_TRANS           */
/* 										   */
/*      STACreateHead	      create header of icc command			   */
/*										   */
/*      STAGetVerifyDataBody  create body for the  verify data installation        */
/*										   */
/*      STAGetBits	      get bits						   */
/*									 	   */
/*	STACheckRSAKeyId      create key_id of private RSA key stored in the ICC   */
/* 										   */
/*	STAGetSCTState        get actual state of the SCT                          */
/* 										   */
/*    IMPORT		    DESCRIPTION 		                           */
/*                                                                                 */
/*	all functions and variables which are described in stamod.h                */
/*                                                                                 */
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
/*   forward global function declarations		       */
/*-------------------------------------------------------------*/
char            STAGetSCTAlgId();
int		STACheckSCT_ICC();
int		STACheckKeyAttrList();
int		STACheckSecMess();
void            STAErrAnalyse();
int		STACreateTrans();
int		STACreateHead();
int      	STAGetVerifyDataBody();
unsigned int	STAGetBits();
int		STAGetSCTState();


/*-------------------------------------------------------------*/
/*   type definitions					       */
/*-------------------------------------------------------------*/
#define BITNULL		(BitString *)0


/*-------------------------------------------------------------*/
/*   global variable definitions			       */
/*-------------------------------------------------------------*/
unsigned int	sca_errno;		/* error number set bei STARMOD */
char		*sca_errmsg;		/* pointer to error message set */
					/* by STARMOD			*/


/*-------------------------------------------------------------*/
/*   local Variable definitions			               */
/*-------------------------------------------------------------*/
static Boolean  	icc_expect;	/* = TRUE indicates: ICC expected    */
 					/* = FALSE indicates: ICC not needed */


/* sct states or smartcard inserted or not */
static Boolean sct_states[] = {
/*state	smartcard inserted	*/
/*00*/	FALSE,	
/*01*/	FALSE,	
/*02*/	FALSE,	
/*03*/	FALSE,	
/*04*/	TRUE,	
/*05*/	FALSE,	
/*06*/	TRUE,	
/*07*/	TRUE,	
/*08*/	TRUE,	
/*09*/	TRUE,	
/*10*/	FALSE,	
/*11*/	TRUE,	
/*12*/	FALSE,	
/*13*/	TRUE,	
/*14*/	TRUE,	
/*15*/	TRUE,	
/*16*/	TRUE,	
/*17*/	TRUE,	
/*18*/	TRUE,	
/*19*/	TRUE,	
/*20*/	TRUE,	
/*21*/	TRUE,	
/*22*/	TRUE,	
/*23*/	TRUE,	
/*24*/	TRUE,	
/*25*/	TRUE,	
/*26*/	TRUE,	
/*27*/	TRUE,	
/*28*/	TRUE,	
/*29*/	TRUE,	
/*30*/	TRUE,	
/*31*/	TRUE,	
/*32*/	TRUE,	
/*33*/	TRUE,	
/*34*/	FALSE,	
/*35*/	TRUE,	
/*36*/	TRUE,	
/*37*/	TRUE,	
/*38*/	TRUE,	
/*39*/	TRUE
};


/*-------------------------------------------------------------*/
/*   local procedure definitions	                       */
/*-------------------------------------------------------------*/



/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_init_icc	          VERSION   1.1	    	       */
/*				     DATE   March 1993	       */
/*			      	       BY   L. Eckstein        */
/*							       */
/* DESCRIPTION						       */
/*  Request and initialize a smartcard.		               */
/*  Sca_init_icc has to be the first function to be called     */
/*  before the communication with the smartcard is             */
/*  possible.                                                  */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   display_text              text which shall be             */
/*                             displayed on the SCT-           */
/*                             display or the NULL-Pointer     */
/*   time_out                  Time-out in seconds             */
/*							       */
/*   atr_info		       ATR Information notification    */
/* OUT							       */
/*   atr		       ATR			       */ 
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
/*  aux_free2_OctetString				       */
/*							       */
/*  STAErrAnalyse	                                       */
/*							       */
/*							       */
/*-------------------------------------------------------------*/
int
sca_init_icc(sct_id, display_text, time_out, atr_info, atr)
	int             sct_id;
	char           *display_text;
	int             time_out;
	ATRInfo		atr_info;
	OctetString	*atr;
{
	int             rc;
	OctetString      octet_string;
	char		*proc="sca_init_icc";

	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;


#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_init_icc *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id                  : %d\n", sct_id);
	fprintf(stdout, "display_text            : %s\n", display_text);
	fprintf(stdout, "time_out(sec)           : %d\n", time_out);
#endif
/*****************************************************************************/
/*
 *      Check input parameters
 */
	if (display_text != NULL) {
		if (strlen(display_text) > MAXL_SCT_DISPLAY)
			goto parerrcase; 
	}
	if ((time_out < 0) || (time_out > MAX_TIME)) 
		goto parerrcase;

	if ((atr_info < NO_ATR) || (atr_info > ATR_HISTORICAL)) 
		goto parerrcase; 
 

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
	Gsct_cmd = S_REQUEST_ICC;
	Gsct_request.rq_p2.time = time_out;
	if (display_text == NULL)
		Gsct_request.rq_datafield.outtext = NULLOCTETSTRING;
	else {
		octet_string.noctets = strlen(display_text);
		octet_string.octets  = display_text;
		Gsct_request.rq_datafield.outtext = &octet_string;
	}

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
 *      Normal End
 */


	if (atr_info != NO_ATR) {
		atr->octets = Gsct_response.octets;
		atr->noctets= Gsct_response.noctets;
	}
	else {
		if (atr) {
			atr->octets = NULL;
			atr->noctets= 0;
		} 
		Gsct_response.noctets = 1;
		aux_free2_OctetString(&Gsct_response);
	}
 



#ifdef TEST
	fprintf(stdout, "TRACE of the output parameters : \n");
	if (atr) {
		fprintf(stdout, "atr                     : \n");
		fprintf(stdout, "    noctets             : %d\n", atr->noctets);
		if (atr->octets != NULL) {
			fprintf(stdout, "    octets              : \n");
			aux_fxdump(stdout, atr->octets, atr->noctets, 0);
		}
		else
			fprintf(stdout, "    octets              : NULL\n");
	}
	else
		fprintf(stdout, "atr                     : NULL\n");
	fprintf(stdout, "\n***** Normal end of   sca_init_icc *****\n\n");
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


}				/* end sca_init_icc */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_init_icc	       */
/*-------------------------------------------------------------*/



/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_get_sct_info         VERSION   1.1	    	       */
/*				     DATE   March 1993	       */
/*			      	       BY   L.Eckstein         */
/*							       */
/* DESCRIPTION						       */
/*  Get information about registered SCT's.		       */
/*  If sct_id = 0 => number of registered SCTs will be returned*/
/*  If sct_id <> 0=> the following returncodes will be         */
/*		     possible    			       */
/*		     M_SCT_NOT_ACTIVE			       */
/*		     M_NO_CARD_INSERTED			       */
/*		     M_CARD_INSERTED			       */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*							       */
/* OUT							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   n	         	       number of registered SCT's      */
/*			       in case of sct_id == 0	       */
/*							       */
/*			       returncode in case of 	       */
/*   			       sct_id <> 0		       */
/*   M_SCT_NOT_ACTIVE			      		       */
/*   M_NO_CARD_INSERTED	        	       		       */
/*   M_CARD_INSERTED	                 		       */
/*							       */
/*  -1			       error			       */
/*							       */
/* CALLED FUNCTIONS					       */
/*  sct_list                   ERROR-Codes		       */
/*				 ENOSHELL		       */
/*                               EOPERR 		       */
/*                               EEMPTY 		       */
/*				 EMEMAVAIL		       */
/*				 ECLERR			       */
/*			         ESIDUNK		       */
/*							       */
/*  sct_info		       ERROR-Codes		       */
/*			         ESIDUNK		       */
/*                               ESCTLIST                      */
/*							       */
/* STAGetSCTState					       */
/* sct_set_portpar					       */
/*							       */
/*							       */
/*-------------------------------------------------------------*/
int
sca_get_sct_info(sct_id)
	int             sct_id;
{
	int             rc, state;

	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;

#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_get_sct_info *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id                  : %d\n", sct_id);
#endif

/*****************************************************************************/
/*
 *      Information about all registered SCTs 
 */
	if (sct_id == 0) {
		rc = sct_list();
		if (rc < 0) {
			sca_errno = sct_errno;
			sca_errmsg = sct_errmsg;
			return (-1);
		}
#ifdef TEST
		fprintf(stdout, "TRACE of the output parameters : \n");
		fprintf(stdout, "No. of SCT's 		 : %d\n", rc);
		fprintf(stdout, "\n***** Normal end of   sca_get_sct_info *****\n\n");
#endif

		return (rc);
	}


/*****************************************************************************/
/*
 *      Informations about a special sct
 */
	/*
	 * call sct_interface		  
	 * send S_STATINF 		  
	 */
	if ((rc = STAGetSCTState(sct_id, &state)) == -1)
		
		rc = M_SCT_NOT_ACTIVE;
	else {


		if (sct_states[state] == TRUE) 
			sct_set_portpar(sct_id, TRUE);
		else
			sct_set_portpar(sct_id, FALSE);




		rc = sct_info(sct_id, &sctinfo);
		if (rc < 0) {
			sca_errno = sct_errno;
			sca_errmsg = sct_errmsg;
			return (-1);
		}
		if (sctinfo.port_open == FALSE) 	/* if port not open       */
			rc = M_SCT_NOT_ACTIVE;
		else	
			if (sctinfo.icc_request == TRUE) /* card inserted	  */
				rc = M_CARD_INSERTED;
			else
				rc = M_NO_CARD_INSERTED;
	}


#ifdef TEST
	fprintf(stdout, "TRACE of the returncode : \n");
	switch (rc) {
	case M_SCT_NOT_ACTIVE:
		fprintf(stdout, "returncode		 : M_SCT_NOT_ACTIVE\n");
		break;
	case M_CARD_INSERTED:
		fprintf(stdout, "returncode		 : M_CARD_INSERTED\n");
		break;
	case M_NO_CARD_INSERTED:
		fprintf(stdout, "returncode		 : M_NO_CARD_INSERTED\n");
		break;
	}

	fprintf(stdout, "\n***** Normal end of   sca_get_sct_info *****\n\n");
#endif
	return(rc);

	

}				/* end sca_get_sct_info */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_get_sct_info       */
/*-------------------------------------------------------------*/




/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_eject_icc	          VERSION   1.1	    	       */
/*				     DATE   June 1993	       */
/*			      	       BY   L. Eckstein        */
/*							       */
/* DESCRIPTION						       */
/*  Eject smartcard.				               */
/*  A smartcard must be inserted 			       */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   display_text              text which shall be             */
/*                             displayed on the SCT-           */
/*                             display or the NULL-Pointer     */
/*   alarm                     = TRUE - acoustic alarm signal  */
/*                             = FALSE - no alarm signal       */
/* OUT							       */
/*							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*			         EPARINC	               */ 
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
/*  aux_free2_OctetString				       */
/*							       */
/*  STAErrAnalyse	                                       */
/*							       */
/*							       */
/*-------------------------------------------------------------*/
int
sca_eject_icc(sct_id, display_text, alarm)
	int             sct_id;
	char           *display_text;
	Boolean         alarm;
{
	int             rc;
	OctetString      octet_string;
	char		*proc="sca_eject_icc";


	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;
	icc_expect = TRUE;	/* this function needs the ICC */

#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_eject_icc *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id                  : %d\n", sct_id);
	fprintf(stdout, "display_text		 : %s\n", display_text);
	if (alarm == TRUE)
		fprintf(stdout, "alarm 			 : SWITCHED ON\n");
	else
		fprintf(stdout, "alarm 			 : SWITCHED OFF\n");
#endif

/*****************************************************************************/
/*
 *      Check input parameters
 */

	if (display_text != NULL) {  
		if (strlen(display_text) > MAXL_SCT_DISPLAY)
			goto parerrcase; 
	}
	if ((alarm != TRUE) && (alarm != FALSE))
		goto parerrcase; 

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
	Gsct_cmd = S_EJECT_ICC;
	Gsct_request.rq_p2.signal = alarm;
	if (display_text == NULL)
		Gsct_request.rq_datafield.outtext = NULLOCTETSTRING;
	else {
		octet_string.noctets = strlen(display_text);
		octet_string.octets  = display_text;
		Gsct_request.rq_datafield.outtext = &octet_string;
	}

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
 *       Normal End	 (Release storage)
 */
	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);

#ifdef TEST
	fprintf(stdout, "\n***** Normal end of   sca_eject_icc *****\n\n");
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



}				/* end sca_eject_icc */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_eject_icc	       */
/*-------------------------------------------------------------*/





/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_display	          VERSION   1.1	    	       */
/*				     DATE   June 1993	       */
/*			      	       BY   L. Eckstein        */
/*							       */
/* DESCRIPTION						       */
/*  Print text on SCT-display			               */
/*  A smartcard is not expected.			       */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   display_text              text which shall be             */
/*                             displayed on the SCT-           */
/*                             display or the NULL-Pointer     */
/*   time_out                  Time-out in seconds             */
/*							       */
/* OUT							       */
/*							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*				 EPARINC	               */
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
/*  aux_free2_OctetString				       */
/*							       */
/*  STAErrAnalyse	                                       */
/*							       */
/*							       */
/*							       */
/*							       */
/*							       */
/*-------------------------------------------------------------*/
int
sca_display(sct_id, display_text, time_out)
	int             sct_id;
	char           *display_text;
	int             time_out;
{
	int             rc;
	OctetString      octet_string;
	char		*proc="sca_display";

	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;
	icc_expect = FALSE;	/* this function doesn't need a ICC */

#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_display *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id                  : %d\n", sct_id);
	fprintf(stdout, "display_text            : %s\n", display_text);
	fprintf(stdout, "time_out(sec)           : %d\n", time_out);
#endif

/*****************************************************************************/
/*
 *      Check input parameters
 */

	if (display_text != NULL) { 
		if (strlen(display_text) > MAXL_SCT_DISPLAY)
			goto parerrcase; 
	}
	if ((time_out < 0) || (time_out > MAX_TIME)) 
		goto parerrcase;

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

	Gsct_cmd = S_DISPLAY;
	Gsct_request.rq_p2.time = time_out;
	if (display_text == NULL)
		Gsct_request.rq_datafield.outtext = NULLOCTETSTRING;
	else {
		octet_string.noctets = strlen(display_text);
		octet_string.octets  = display_text;
		Gsct_request.rq_datafield.outtext = &octet_string;
	}

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
 *      Normal End	  (Release storage)
 */
	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);



#ifdef TEST
	fprintf(stdout, "\n***** Normal end of   sca_display *****\n\n");
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



}				/* end sca_display */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_display	       */
/*-------------------------------------------------------------*/


/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_set_mode	          VERSION   1.1	    	       */
/*				     DATE   June 1993	       */
/*			      	       BY   L.Eckstein         */
/*							       */
/* DESCRIPTION						       */
/*  Set security mode    			               */
/*  A smartcard is not expected.			       */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   sec_mess		       security modes                  */
/*							       */
/* OUT							       */
/*							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*				EPARINC			       */
/*			        ENOINTKEY		       */
/*			        ENOCONCKEY                     */
/*							       */
/* CALLED FUNCTIONS					       */
/*  sct_reset		       ERROR-Codes		       */
/*				 EBAUD                         */
/*				 ENOSHELL		       */
/*                               EOPERR                        */
/*			         EEMPTY                        */
/*                               EMEMAVAIL                     */
/*                               ECLERR                        */
/*                               ESIDUNK                       */
/*                               ERDERR                        */
/*                               EINVARG		       */
/*                               ETOOLONG		       */
/*                               T1 - ERROR                    */
/*                               sw1/sw2 from SCT response     */
/*  sct_info		       ERROR-Codes		       */
/*			         ESIDUNK		       */
/*                               ESCTLIST                      */
/*							       */
/*  sct_setmode 	       ERROR-Codes	               */
/*			         ESIDUNK		       */
/*                               ESCTLIST                      */
/*							       */
/*  STAErrAnalyse					       */
/*-------------------------------------------------------------*/
int
sca_set_mode(sct_id, sct_sec_mess)
	int             sct_id;
	SCT_SecMess     *sct_sec_mess;
{
	int             rc;
	char		*proc = "sca_set_mode";

	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;
	icc_expect = FALSE;	/* this function doesn't need a ICC */

#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_set_mode *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id		        : %d\n", sct_id);
	print_SCT_SecMess(sct_sec_mess);
#endif

/*****************************************************************************/
/*
 *      Check input parameter sec_mess
 */
	if ((sct_sec_mess->command != SCT_SEC_NORMAL) &&
	    (sct_sec_mess->command != SCT_INTEGRITY) &&
	    (sct_sec_mess->command != SCT_CONCEALED) &&
	    (sct_sec_mess->command != SCT_COMBINED)) {
		sca_errno = EPARINC;
		sca_errmsg= sca_errlist[sca_errno].msg;
		return (-1);
	}
	if ((sct_sec_mess->response != SCT_SEC_NORMAL) &&
	    (sct_sec_mess->response != SCT_INTEGRITY) &&
	    (sct_sec_mess->response != SCT_CONCEALED) &&
	    (sct_sec_mess->response != SCT_COMBINED)) {
		sca_errno = EPARINC;
		sca_errmsg= sca_errlist[sca_errno].msg;
		return (-1);
	}


/*****************************************************************************/
/*
 *      if port_open = FALSE then reset SCT                  
 */
	rc = sct_info(sct_id, &sctinfo);
	if (rc < 0) {
		sca_errno = sct_errno;
		sca_errmsg = sct_errmsg;
		return (-1);
	}
	if (sctinfo.port_open == FALSE) {
		/* if port not open       */
		rc = sct_reset(sct_id, proc);
		if (rc < 0) {
			sca_errno = sct_errno;
			sca_errmsg = sct_errmsg;
			STAErrAnalyse(sct_id);
			return (-1);
		}
	}

/*****************************************************************************/
/*
 *      check sessionkey
 */

	if ((sct_sec_mess->command != SCT_SEC_NORMAL) ||
	    (sct_sec_mess->response != SCT_SEC_NORMAL)) {

		/*
		 * test, if sessionkey available		
		 */

		switch (sct_sec_mess->command) {
		case SCT_INTEGRITY:
			if (sctinfo.integrity_key == FALSE) {
				sca_errno = ENOINTKEY;
				sca_errmsg= sca_errlist[sca_errno].msg;
				return (-1);
			}
			break;
		case SCT_CONCEALED:
			if (sctinfo.concealed_key == FALSE) {
				sca_errno = ENOCONCKEY;
				sca_errmsg= sca_errlist[sca_errno].msg;
				return (-1);
			}
			break;
		case SCT_COMBINED:
			if (sctinfo.integrity_key == FALSE) {
				sca_errno = ENOINTKEY;
				sca_errmsg= sca_errlist[sca_errno].msg;
				return (-1);
			}
			if (sctinfo.concealed_key == FALSE) {
				sca_errno = ENOCONCKEY;
				sca_errmsg= sca_errlist[sca_errno].msg;
				return (-1);
			}
			break;
		}

		switch (sct_sec_mess->response) {
		case SCT_INTEGRITY:
			if (sctinfo.integrity_key == FALSE) {
				sca_errno = ENOINTKEY;
				sca_errmsg= sca_errlist[sca_errno].msg;
				return (-1);
			}
			break;
		case SCT_CONCEALED:
			if (sctinfo.concealed_key == FALSE) {
				sca_errno = ENOCONCKEY;
				sca_errmsg= sca_errlist[sca_errno].msg;
				return (-1);
			}
			break;
		case SCT_COMBINED:
			if (sctinfo.integrity_key == FALSE) {
				sca_errno = ENOINTKEY;
				sca_errmsg= sca_errlist[sca_errno].msg;
				return (-1);
			}
			if (sctinfo.concealed_key == FALSE) {
				sca_errno = ENOCONCKEY;
				sca_errmsg= sca_errlist[sca_errno].msg;
				return (-1);
			}
			break;
		}



	}

/*****************************************************************************/
/*
 *      Store security mode in port-memory
 */
	rc = sct_setmode(sct_id, sct_sec_mess);
	if (rc < 0) {
		sca_errno = sct_errno;
		sca_errmsg = sct_errmsg;
		return (-1);
	}
#ifdef TEST
	fprintf(stdout, "\n***** Normal end of   sca_set_mode *****\n\n");
#endif

	return (0);


}				/* end sca_set_mode */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_set_mode	       */
/*-------------------------------------------------------------*/

/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_gen_sessionkey	  VERSION   1.1	    	       */
/*				     DATE   August 1993	       */
/*			      	       BY   L.Eckstein         */
/*							       */
/* DESCRIPTION						       */
/*  Generate Sessionkey for Secure Messaging between DTE and   */
/*  SCT.		    			               */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   session_key	       specification of the            */
/*			       sessionkey(s).		       */
/*                             Algorithm Identifier            */
/*			       The following values are        */
/*			       possible:		       */
/*			       desCBC_ISO0	               */
/*			       desCBC3_ISO0	               */
/*							       */
/* OUT							       */
/*							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*				EPARINC			       */
/*							       */
/* CALLED FUNCTIONS					       */
/* STACheckSCT_ICC		ERROR-Codes		       */
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
/*  sct_secure		        ERROR-Codes		       */
/*                             	 ESCTLIST                      */
/*                               EPARINC                       */
/*			         EGENSESS		       */
/*			       	 EMEMAVAIL		       */
/*			         EKEY			       */
/* 			         ERSAENC		       */
/*                               EALGO			       */
/*                               ESIDUNK                       */
/*                               EINVARG		       */
/*                               ETOOLONG		       */
/*			         EINS			       */
/*			         EDESDEC  		       */
/*			         ESSC			       */
/*			         EDESENC  		       */
/*			         ERECMAC 	               */
/*                               T1 - ERROR                    */
/*                               sw1/sw2 from SCT response     */
/*							       */
/*  STAErrAnalyse					       */
/*-------------------------------------------------------------*/
int
sca_gen_sessionkey(sct_id, session_key)
	int             sct_id;
	SessionKey	*session_key;
{
	int             rc;
	char		*proc="sca_gen_sessionkey";

	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;
	icc_expect = FALSE;	

#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_gen_sessionkey *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id		        : %d\n", sct_id);
	print_SessionKey(session_key);
#endif

/*****************************************************************************/
/*
 *      Check input parameter session_key
 */
	if ((!session_key) ||
	    ((!session_key->integrity_key) && (!session_key->concealed_key))) {
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		return (-1);
	}



/*****************************************************************************/
/*
 *      call STACheckSCT_ICC
 */
	if (STACheckSCT_ICC(sct_id, FALSE, proc) == -1)
		return (-1);


/*****************************************************************************/
/*
 *      call sct_secure to generate session_key and to store it in the prot-memory
 */
	if (session_key->integrity_key) {
		rc = sct_secure(sct_id,SCT_INTEGRITY,session_key->integrity_key);
		if (rc < 0) {
			sca_errno = sct_errno;
			sca_errmsg = sct_errmsg;
			STAErrAnalyse(sct_id);
			return (-1);
		}
	}

	if (session_key->concealed_key) {
		rc = sct_secure(sct_id,SCT_CONCEALED,session_key->concealed_key);
		if (rc < 0) {
			sca_errno = sct_errno;
			sca_errmsg = sct_errmsg;
			STAErrAnalyse(sct_id);
			return (-1);
		}
	}

#ifdef TEST
	fprintf(stdout, "\n***** Normal end of   sca_gen_sessionkey *****\n\n");
#endif

	return (0);


}				/* end sca_gen_sessionkey */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_gen_sessionkey     */
/*-------------------------------------------------------------*/

/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_del_sessionkey	  VERSION   1.1	    	       */
/*				     DATE   August 1993	       */
/*			      	       BY   L.Eckstein         */
/*							       */
/* DESCRIPTION						       */
/*  Delete Sessionkey for Secure Messaging between DTE and     */
/*  SCT.		    			               */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   session_key	       specification of the            */
/*			       sessionkey(s).		       */
/*                             Algorithm Identifier            */
/*			       The following values are        */
/*			       possible:		       */
/*			       desCBC_ISO0	               */
/*			       desCBC3_ISO0	               */
/*							       */
/* OUT							       */
/*							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*				EPARINC			       */
/*			        EALGO                          */
/* CALLED FUNCTIONS					       */
/*   sct_init_secure          ERROR-Codes                      */
/*                             ESCTLIST                        */
/*                             EPARINC                         */
/*			       ENOINTKEY	               */
/*			       ENOCONCKEY		       */
/*                             ESIDUNK                         */
/*                             EINVARG			       */
/*                             ETOOLONG		               */
/*                             EMEMAVAIL		       */
/*			       EINS			       */
/*			       EDESDEC  		       */
/*			       ESSC			       */
/*			       EDESENC  		       */
/*			       ERECMAC 	                       */
/*			       EALGO	                       */
/*                             T1 - ERROR                      */
/*                             sw1/sw2 from SCT response       */
/*   sct_getsecmess           ERROR-Codes                      */
/*                             ESCTLIST                        */
/*                              ESIDUNK                        */
/*  sct_setmode 	       ERROR-Codes	               */
/*			         ESIDUNK		       */
/*                               ESCTLIST                      */
/*   STAErrAnalyse					       */
/*   aux_cmp_ObjId					       */
/*-------------------------------------------------------------*/
int
sca_del_sessionkey(sct_id, session_key)
	int             sct_id;
	SessionKey	*session_key;
{
	int             rc;
	SCT_SecMess	org_sec_mess, new_sec_mess;
	Boolean		new;

	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;

#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_del_sessionkey *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id		        : %d\n", sct_id);
	print_SessionKey(session_key);
#endif

/*****************************************************************************/
/*
 *      Check input parameter session_key
 */
	if ((!session_key) ||
	    ((!session_key->integrity_key) && (!session_key->concealed_key))) {
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		return (-1);
	}

	if (session_key->integrity_key) {
		if (!session_key->integrity_key->objid) {
			sca_errno = EALGO;
			sca_errmsg = sca_errlist[sca_errno].msg;
			return (-1);
		}

		if ((aux_cmp_ObjId(session_key->integrity_key->objid,desCBC_ISO0->objid)) &&
 		    (aux_cmp_ObjId(session_key->integrity_key->objid,desCBC3_ISO0->objid))) {
			sca_errno = EALGO;
			sca_errmsg = sca_errlist[sca_errno].msg;
			return (-1);
		}
	};

	if (session_key->concealed_key) {
		if (!session_key->concealed_key->objid) {
			sca_errno = EALGO;
			sca_errmsg = sca_errlist[sca_errno].msg;
			return (-1);
		}

		if ((aux_cmp_ObjId(session_key->concealed_key->objid,desCBC_ISO0->objid)) &&
 		    (aux_cmp_ObjId(session_key->concealed_key->objid,desCBC3_ISO0->objid))) {
			sca_errno = EALGO;
			sca_errmsg = sca_errlist[sca_errno].msg;
			return (-1);
		}
	};

/*****************************************************************************/
/*
 *      Get security mode of port-memory
 */

		rc = sct_getsecmess(sct_id, &org_sec_mess);
		if (rc < 0) {
			sca_errno = sct_errno;
			sca_errmsg = sct_errmsg;
			return (-1);
		}

		new = FALSE;
		new_sec_mess.command = org_sec_mess.command;
		new_sec_mess.response= org_sec_mess.response;




/*****************************************************************************/
/*
 *      call sct_init_secure to delete session_key 
 */
	if (session_key->integrity_key) {
		rc = sct_init_secure(sct_id,SCT_INTEGRITY);
		if (rc < 0) {
			sca_errno = sct_errno;
			sca_errmsg = sct_errmsg;
			STAErrAnalyse(sct_id);
			return (-1);
		}


		switch (org_sec_mess.command) {
		case SCT_INTEGRITY:
			new_sec_mess.command = SCT_SEC_NORMAL;
			new = TRUE;
			break;
		case SCT_COMBINED:
			new_sec_mess.command = SCT_CONCEALED;
			new = TRUE;
			break;
		}

		switch (org_sec_mess.response) {
		case SCT_INTEGRITY:
			new_sec_mess.response = SCT_SEC_NORMAL;
			new = TRUE;
			break;
		case SCT_COMBINED:
			new_sec_mess.response = SCT_CONCEALED;
			new = TRUE;
			break;
		}

 
	}

	if (session_key->concealed_key) {
		if (new == TRUE) {

		/*
 	 	 *      Store new security mode in port-memory
 	 	 */
			rc = sct_setmode(sct_id, &new_sec_mess);
			if (rc < 0) {
				sca_errno = sct_errno;
				sca_errmsg = sct_errmsg;
				return (-1);
			}
		}

		rc = sct_init_secure(sct_id,SCT_CONCEALED);
		if (rc < 0) {
			sca_errno = sct_errno;
			sca_errmsg = sct_errmsg;
			STAErrAnalyse(sct_id);
			return (-1);
		}
	}

/*****************************************************************************/
/*
 *      Store new security mode in port-memory
 */
	rc = sct_setmode(sct_id, org_sec_mess);
	if (rc < 0) {
		sca_errno = sct_errno;
		sca_errmsg = sct_errmsg;
		return (-1);
	}


#ifdef TEST
	fprintf(stdout, "\n***** Normal end of   sca_del_sessionkey *****\n\n");
#endif

	return (0);


}				/* end sca_del_sessionkey */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_del_sessionkey     */
/*-------------------------------------------------------------*/

/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_reset	          VERSION   1.1	    	       */
/*				     DATE   August 1993	       */
/*			      	       BY   L. Eckstein        */
/*							       */
/* DESCRIPTION						       */
/*  Reset smartcard terminal     		               */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   mode	               OPEN: open port                 */
/*			       CLOSE:close port		       */	
/*   sct_port                  SCT information                 */
/*			       mode =OPEN		       */
/*			       If the NULL pointer is delivered*/
/*			       STARMOD sends the S_RESET       */
/*			       command                         */
/*							       */
/* OUT							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*   M_NO_CARD_INSERTED	       icc_request in sct_port is set  */
/*			       to FALSE  	       	       */
/*   M_CARD_INSERTED	       icc_request in sct_port is set  */
/*			       to TRUE          	       */
/*   M_REMOVE_CARD	       icc_request in sct_port is set  */
/*			       to FALSE          	       */
/*  -1			       error			       */
/*				 EPARINC 		       */
/*			         EMEMAVAIL		       */	 
/*							       */
/* CALLED FUNCTIONS					       */
/*  sct_reset		       ERROR-Codes		       */
/*				 EBAUD                         */
/*				 ENOSHELL		       */
/*                               EOPERR                        */
/*			         EEMPTY                        */
/*                               EMEMAVAIL                     */
/*                               ECLERR                        */
/*                               ESIDUNK                       */
/*                               ERDERR                        */
/*                               EINVARG		       */
/*                               ETOOLONG		       */
/*                               T1 - ERROR                    */
/*                               sw1/sw2 from SCT response     */
/*							       */
/*							       */
/*  sct_init_port	        ERROR_Codes                    */
/*				ENOSHELL		       */
/*                              EOPERR                         */
/*			        EEMPTY                         */
/*                              EMEMAVAIL                      */
/*                              ECLERR                         */
/*                              ESIDUNK                        */
/*                              ERDERR                         */
/*                              ECLERR                         */
/*                              T1 - ERROR                     */
/*							       */
/*  sct_close   		ERROR-Codes		       */
/*			        ESIDUNK                        */
/*                              ENOSHELL                       */
/*                              EOPERR                         */
/*                              EEMPTY                         */
/*                              ERDERR                         */
/*                              ECLERR                         */
/*							       */
/*  STAErrAnalyse	                                       */
/*  STAGetSCTState		ERROR-Codes                    */
/*				  ESTATE0		       */ 
/*							       */
/*							       */
/*-------------------------------------------------------------*/
int
sca_reset(sct_id, mode, sct_port)
	int             sct_id;
	ComMode		mode;
	SCTPort 	*sct_port;
{
	int             rc, state, listlen;
	char		*proc="sca_reset";

	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;


#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_reset *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id                  : %d\n", sct_id);
	print_ComMode(mode);
	if (mode == COM_OPEN)
		print_SCTPort(sct_port);
#endif

 

/*****************************************************************************/
/*
 *      call sct_reset or sct_init_port
 */
	switch (mode) {
	case COM_OPEN:
		if (sct_port == NULL) {
			rc = sct_reset(sct_id, proc);
			if (rc < 0) {
				sca_errno = sct_errno;
				sca_errmsg = sct_errmsg;
				STAErrAnalyse(sct_id);
				return (-1);
			}
		}
		else {
			rc = sct_init_port(sct_id, sct_port, TRUE, proc);
			if (rc < 0) {
				sca_errno = sct_errno;
				sca_errmsg = sct_errmsg;
				STAErrAnalyse(sct_id);
				return (-1);
			}

			/*
	 		 * call sct_interface		  
	 		 * send S_STATINF 		  
	 		 */
			if ((rc = STAGetSCTState(sct_id, &state)) == -1)
				return(-1);

			if (state == 7) {
				/* the smartcard must be removed */
				sct_port->icc_request = FALSE;
				sca_errno = M_REMOVE_CARD;

				rc = sct_init_port(sct_id, sct_port, FALSE, proc);
				if (rc < 0) {
					sca_errno = sct_errno;
					sca_errmsg = sct_errmsg;
					STAErrAnalyse(sct_id);
					return (-1);
				}
			}
			else {
				if (sct_states[state] != sct_port->icc_request) {
					sct_port->icc_request = sct_states[state];
					if (sct_states[state] == TRUE)
						sca_errno = M_CARD_INSERTED;
					else
						sca_errno = M_NO_CARD_INSERTED;


					rc = sct_init_port(sct_id, sct_port, FALSE, proc);
					if (rc < 0) {
						sca_errno = sct_errno;
						sca_errmsg = sct_errmsg;
						STAErrAnalyse(sct_id);
						return (-1);
					}

				}
			};


		};
		
		break;
	case COM_CLOSE:
		rc = sct_close(sct_id, proc);
		if (rc < 0) {
			sca_errno = sct_errno;
			sca_errmsg = sct_errmsg;
			return (-1);
		}


		break;
	default:
		goto parerrcase;
	};

			 
#ifdef TEST

	fprintf(stdout, "\n***** Normal end of   sca_reset *****\n\n");
#endif
	return(sca_errno);


/*****************************************************************************/
/*
 *      Parameter error
 */
parerrcase:
	sca_errno = EPARINC;
	sca_errmsg = sca_errlist[sca_errno].msg;
	return (-1);


}				/* end sca_reset */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_reset	       */
/*-------------------------------------------------------------*/


/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_get_sctport	  VERSION   1.1	    	       */
/*				     DATE   August 1993	       */
/*			      	       BY   L. Eckstein        */
/*							       */
/* DESCRIPTION						       */
/*  Get SCT port information     		               */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*							       */
/* OUT							       */
/*   sct_port                  SCT information                 */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*				 EPARINC 		       */	 
/*							       */
/* CALLED FUNCTIONS					       */
/*  sct_get_port		ERROR-Codes		       */
/*                               ESIDUNK                       */
/*							       */
/*							       */
/*  STAErrAnalyse	                                       */
/*							       */
/*							       */
/*-------------------------------------------------------------*/
int
sca_get_sctport(sct_id, sct_port)
	int             sct_id;
	SCTPort 	*sct_port;
{
	int             rc;
	char		*proc="sca_get_sctport";

	rc = 0;
	sca_errno = M_NOERR;
	sca_errmsg = NULL;


#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_get_sctport *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id                  : %d\n", sct_id);
#endif

 

/*****************************************************************************/
/*
 *      Check input parameter
 */
	if (sct_port == NULL)
		goto parerrcase;

	rc = sct_get_port(sct_id, sct_port);
	if (rc < 0) {
		sca_errno = sct_errno;
		sca_errmsg = sct_errmsg;
		STAErrAnalyse(sct_id);
		return (-1);
	}
#ifdef TEST
	fprintf(stdout, "TRACE of the output parameters : \n");
	print_SCTPort(sct_port);
	fprintf(stdout, "\n***** Normal end of   sca_get_sctport *****\n\n");
#endif
	return(0);


/*****************************************************************************/
/*
 *      Parameter error
 */
parerrcase:
	sca_errno = EPARINC;
	sca_errmsg = sca_errlist[sca_errno].msg;
	return (-1);


}				/* end sca_get_sctport */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_get_sctport	       */
/*-------------------------------------------------------------*/




/*****************************************************************************/
/*	global functions for the sca-functions                               */

/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  STACheckSCT_ICC             VERSION   1.1	       */
/*				     DATE   June 1993	       */
/*			      	       BY   Levona Eckstein    */
/*							       */
/* DESCRIPTION						       */
/*  Check, if SCT initialized and ICC inserted 	               */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*   icc_expect		       = TRUE => ICC is expected       */
/*   			       = FALSE => ICC is not expected  */
/*                                                             */
/* OUT							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*                               ENOCARD		       */
/*							       */
/* CALLED FUNCTIONS					       */
/*  sct_info                   ERROR-Codes		       */
/*                               ESCTLIST                      */
/*			         ESIDUNK		       */
/*							       */
/*  sct_reset		       ERROR-Codes		       */
/*				 EBAUD                         */
/*				 ENOSHELL		       */
/*                               EOPERR                        */
/*			         EEMPTY                        */
/*                               EMEMAVAIL                     */
/*                               ECLERR                        */
/*                               ESIDUNK                       */
/*                               ERDERR                        */
/*                               EINVARG		       */
/*                               ETOOLONG		       */
/*                               T1 - ERROR                    */
/*                               sw1/sw2 from SCT response     */
/*							       */
/*  sct_checksecmess            ERROR-Codes		       */
/*                               ESCTLIST                      */
/*			         ESIDUNK		       */
/*				 ENOINTKEY		       */
/*				 ENOCONCKEY		       */	
/*  STAErrAnalyse					       */       
/*  STAGetSCTState		ERROR-Codes                    */
/*				  ESTATE0		       */     
/*							       */
/*-------------------------------------------------------------*/
int
STACheckSCT_ICC(sct_id, icc_expect, proc)
	int             sct_id;
	Boolean         icc_expect;
	char		*proc;
{
	int             rc, state;


/*****************************************************************************/
/*
 *      if port_open = FALSE then reset SCT                  
 */
	rc = sct_info(sct_id, &sctinfo);
	if (rc < 0) {
		sca_errno = sct_errno;
		sca_errmsg = sct_errmsg;
		return (-1);
	}
	if (sctinfo.port_open == FALSE) {
		/* if port not open       */
		rc = sct_reset(sct_id, proc);
		if (rc < 0) {
			sca_errno = sct_errno;
			sca_errmsg = sct_errmsg;
			STAErrAnalyse(sct_id);
			return (-1);
		}
	}
	else {


		/*
 		 *      test secure messaging
 		 */

		if (sct_checksecmess(sct_id) == -1) {
			sca_errno = sct_errno;
			sca_errmsg = sct_errmsg;
			return (-1);
		}
	}





	/*
	 * call sct_interface		  
	 * send S_STATINF 		  
	 */
	if ((rc = STAGetSCTState(sct_id, &state)) == -1)
		return(-1);







/*****************************************************************************/
/*
 *      function only allowed, if smartcard already requested.                
 *      Therefore test, if smartcard is inserted				 
 */

	if ((icc_expect == TRUE) && (sctinfo.icc_request == FALSE)) {
		/* error => smartcard not requested */
		sca_errno = ENOCARD;
		sca_errmsg= sca_errlist[sca_errno].msg;
		return (-1);
	}


	return (0);
}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	STACheckSCT_ICC	       */
/*-------------------------------------------------------------*/





/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  STAGetSCTAlgId      VERSION   1.1                */
/*                              DATE   June 1993          */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Check alg_id of SCA-IF and                            */
/*  transform  alg_id of SCA-IF to SCT specific alg_id,   */
/*  which is used at the SCT-IF.		          */
/*  In case of RSA keysize is checked: allowed values are */
/*  256, 512 bits.               	                  */
/*                                                        */
/*  alg_id	is transformed to 	sct_algid:        */
/*  ------                              ----------        */
/*  ENC = RSA		->		S_RSA_F4          */
/*  ENC = DES &&                                          */  
/*  MODE = CBC		->		S_DES_CBC         */
/*  ENC = DES3 &&                                         */  
/*  MODE = CBC		->		S_DES_3_CBC       */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   algid                     algid structure            */
/*                                                        */
/*                                                        */
/* OUT                                                    */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   char                      Value of Byte              */
/*                                                        */
/*   -1                        error                      */
/*                               EALGO                    */
/*                               EKEYLEN                  */
/*                                                        */
/*                                                        */
/*  CALLED FUNCTIONS                                      */
/*--------------------------------------------------------*/
char
STAGetSCTAlgId(algid)
	AlgId          *algid;
{

	KeyAlgId        sct_algid;
	int             rc;

	sct_algid = -1;


	if ((!algid) || (!algid->objid)) {
		sca_errno = EPARINC;
		sca_errmsg= sca_errlist[sca_errno].msg;
		return (-1);
	}

	if(aux_ObjId2AlgEnc(algid->objid) == RSA) {
		/* if RSA, then check keysize */
		if (algid->param) {
			if ((RSA_PARM(algid->param) != 256) &&
			    (RSA_PARM(algid->param) != 512) &&
			    (RSA_PARM(algid->param) != 1024)) {
				sca_errno = EKEYLEN;
				sca_errmsg= sca_errlist[sca_errno].msg;
				return (-1);
			}
		}
		sct_algid = S_RSA_F4;
	}
	else {
		if ((aux_ObjId2AlgEnc(algid->objid) == DES) &&
		    (aux_ObjId2AlgMode(algid->objid) == CBC))
			sct_algid = S_DES_CBC;
        	else {
			if ((aux_ObjId2AlgEnc(algid->objid) == DES3) &&
		            (aux_ObjId2AlgMode(algid->objid) == CBC)) 	 
			sct_algid = S_DES_3_CBC;
			else {
				sca_errno = EALGO;
				sca_errmsg= sca_errlist[sca_errno].msg;
				return (-1);
			}
		}
	};


#ifdef TEST
	fprintf(stdout, "SCT algid: %d\n ", sct_algid);
#endif

	return (sct_algid);


}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      STAGetSCTAlgId         */
/*-------------------------------------------------------------*/





/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/*   STACheckKeyAttrList      VERSION   1.1	    	       */
/*				     DATE   June 1993	       */
/*			      	       BY   L. Eckstein        */
/*							       */
/* DESCRIPTION						       */
/*  Check key attribute list for user keys, device keys, PIN   */
/*  and PUK. 						       */
/*  In case of a user key,PIN or PUK the value of key_purpose  */
/*  is set by  this function.				       */
/*  The MAC_length is automatically set to 4.                  */
/*  If the key is not an authentication key, key_fpc will be   */
/*  set to 0 by this function.				       */
/*  In case of PIN or PUK, key_status.PIN_check is checked,    */
/*  otherwise key_status.PIN_check is set to FALSE.            */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   kind_of_key	       {USER_KEY, PIN_KEY, PUK_KEY     */
/*                              DEVICE_KEY}                    */
/*                                                             */
/*   key_attr_list	       pointer to the list of the key  */
/*                             attributes                      */
/*							       */
/*							       */
/* OUT							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*                               EPARINC 		       */
/*							       */
/* CALLED FUNCTIONS					       */
/*-------------------------------------------------------------*/
int
STACheckKeyAttrList(kind_of_key, key_attr_list)
	KindOfKey       kind_of_key;
	KeyAttrList    *key_attr_list;
{
#define PURPOSE   key_attr_list->key_attr.key_purpose
#define PRESENT   key_attr_list->key_attr.key_presentation
#define MACLEN    key_attr_list->key_attr.MAC_length

	int             rc;


	if (key_attr_list == KEYATTRNULL)
		goto parerrcase;


/*****************************************************************************/
/*
 *      Check key install mode
 */
	if ((key_attr_list->key_inst_mode != INST) &&
	    (key_attr_list->key_inst_mode != REPL))
		goto parerrcase;

/*****************************************************************************/
/*
 *      Check key purpose depending on kind of key	:     
 *
 * 	USER_KEY: key_purpose is set by this function:      
 * 	key_purpose = PURPOSE_CIPHER                        
 *                                                     
 * 	PIN_KEY:  key_purpose is set by this function:      
 * 	key_purpose = PURPOSE_AUTH                         
 *                                                    
 *	PUK_KEY:  see PIN_KEY			       
 *                                                     
 * 	DEVICE_KEY: 				               
 * 	At least one of the following three must be set :
 * 	1) key_purpose = PURPOSE_AUTH                       
 * 	2) key_purpose = PURPOSE_SECCONC                    
 * 	3) key_purpose = PURPOSE_SECAUTH                   
 */

	switch (kind_of_key) {
	case USER_KEY:
		PURPOSE = PURPOSE_CIPHER;
		break;
	case PIN_KEY:
	case PUK_KEY:
		PURPOSE = PURPOSE_AUTH;
		break;
	case DEVICE_KEY:
		if ((PURPOSE != PURPOSE_AUTH) &&
		    (PURPOSE != PURPOSE_SECAUTH) &&
		    (PURPOSE != PURPOSE_SECCONC))
			goto parerrcase; 
		break;
	default:
		goto parerrcase;
	}			/* end case */

/*****************************************************************************/
/*
 *      Check key presentation
 */
	if ((PRESENT != KEY_GLOBAL) && (PRESENT != KEY_LOCAL))
		goto parerrcase;

/*****************************************************************************/
/*
 *      Set MAC length
 */
	MACLEN = 4;

/*****************************************************************************/
/*
 *      Check key fault presentation counter	       
 * 	If key is not an authentication key, key_fpc      
 * 	is set to 0 by this function.                     
 */
	if (PURPOSE != PURPOSE_AUTH)
		key_attr_list->key_fpc = 0;
	else {
		if ((key_attr_list->key_fpc < 0) ||
		    (key_attr_list->key_fpc > MAX_KPFC))
			goto parerrcase;
 	}			/* end else */

/*****************************************************************************/
/*
 *      Check key status				      
 *	If kind_of_key is not PIN or PUK,key_status.PIN_check is set to FALSE.      
 */
	if ((kind_of_key == PIN_KEY) || (kind_of_key == PUK_KEY)) {
		if ((key_attr_list->key_status.PIN_check != FALSE) &&
		    (key_attr_list->key_status.PIN_check != TRUE))
			goto parerrcase;
	} else
		key_attr_list->key_status.PIN_check = FALSE;


	return (0);

/*****************************************************************************/
/*
 *      Parameter error
 */
parerrcase:
	sca_errno = EPARINC;
	sca_errmsg = sca_errlist[sca_errno].msg;
	return (-1);

}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	STACheckKeyAttrList    */
/*-------------------------------------------------------------*/





/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  STACheckSecMess           VERSION   1.1	       */
/*				     DATE   June 1993	       */
/*			      	       BY   L. Eckstein        */
/*							       */
/* DESCRIPTION						       */
/*  Check security modes for command and response   	       */
/*  between SCT and ICC					       */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sec_mess		       security mode(s)		       */
/*							       */
/*							       */
/* OUT							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*                               EPARINC 		       */
/*							       */
/* CALLED FUNCTIONS					       */
/*-------------------------------------------------------------*/
int
STACheckSecMess(sec_mess)
	ICC_SecMess        *sec_mess;
{

	int             rc;


	if (!sec_mess)
		goto parerrcase;

	if ((sec_mess->command != ICC_SEC_NORMAL) &&
	    (sec_mess->command != ICC_AUTHENTIC) &&
	    (sec_mess->command != ICC_CONCEALED) &&
	    (sec_mess->command != ICC_COMBINED))
		goto parerrcase;

	if ((sec_mess->response != ICC_SEC_NORMAL) &&
	    (sec_mess->response != ICC_AUTHENTIC) &&
	    (sec_mess->response != ICC_CONCEALED) &&
	    (sec_mess->response != ICC_COMBINED))
		goto parerrcase;
 	return (0);

/*****************************************************************************/
/*
 *      Parameter error
 */
parerrcase:
	sca_errno = EPARINC;
	sca_errmsg = sca_errlist[sca_errno].msg;
	return (-1);



}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	STACheckSecMess	       */
/*-------------------------------------------------------------*/



/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  STAErrAnalyse             VERSION   1.1	       */
/*				     DATE   July 1993	       */
/*			      	       BY   Levona Eckstein    */
/*							       */
/* DESCRIPTION						       */
/*  Error analyse and error handling		               */
/*  If an error > ESCTRESET is indicated, the function tests   */
/*  the state of the SCT.				       */
/*  In this case sca_errno is set as follows                   */
/*  - an ICC is already inserted, sca_errno is set to ECARDINS.*/
/*  - no ICC is inserted,         sca_errno is set to ENOCARD  */
/*  - the ICC must be removed,    sca_errno is set to ECARDREM */
/*  - the SCT is in state 0       sca_errno is set to ESTATE0  */
/*				  in STAGetSCTState	       */  
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/* OUT							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*							       */
/* CALLED FUNCTIONS					       */
/*  sct_interface	       ERROR-Codes	               */
/*                               EINVARG		       */
/*                               ETOOLONG		       */
/*                               EMEMAVAIL		       */
/*                               ESIDUNK                       */
/*			         EINS			       */
/*			         EPARINC                       */
/*			         EDESDEC  		       */
/*			         ESSC			       */
/*			         EDESENC  		       */
/*			         ERECMAC 	               */
/*			         EALGO	                       */
/*                               T1 - ERROR                    */
/*                               sw1/sw2 from SCT response     */
/*							       */
/*  aux_free2_OctetString				       */
/*  sct_close  						       */
/*  STAGetSCTState		ERROR-Codes                    */
/*				  ESTATE0		       */              
/*							       */
/*-------------------------------------------------------------*/

void
STAErrAnalyse(sct_id)
	int             sct_id;
{
	int		rc, state;
	unsigned int	save_errno;
	char	*proc="STAErrAnalyse";

	
	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);
	save_errno = sca_errno;


#ifdef TEST
	fprintf(stdout,"STAErrAnalyse called with sca_errno = %d\n",sca_errno);
#endif



	if ((sca_errno >= EDEVBUSY) && (sca_errno <= ESCTRESET)) {
		/*
		 * communication error
		 * port is closed
		 */
		if ((sca_errno != EPROTRESYNCH) && (sca_errno != EDEVBUSY)) {
			sct_close(sct_id, proc);
			sca_errno = save_errno;
			sca_errmsg = sca_errlist[sca_errno].msg; 
		}
	}
	else {

		if ((sca_errno >= ENOSHELL) && (sca_errno <= ERDERR)) {
			/*
		 	 * error on installation file
		 	 */
			sca_errno = save_errno;
			sca_errmsg = sca_errlist[sca_errno].msg; 
		}
		else {
			/*
		 	 * call sct_interface		  
		 	 * send S_STATINF 		  
		 	 */
			STAGetSCTState(sct_id, &state);
		}


	} /* end of else */
}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	STAErrAnalyse	       */
/*-------------------------------------------------------------*/


/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  STACreateTrans         VERSION   1.1	    	       */
/*				     DATE   June 1993	       */
/*			      	       BY   Levona Eckstein    */
/*							       */
/* DESCRIPTION						       */
/*  Create ICC command and send it via S_TRANS 	               */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   flag                      if flag = TRUE =>               */
/*    				       - create icc_command    */
/*                                     - check icc_response    */
/*                             if flag = FALSE =>              */
/*                                   only in case of sca_trans */
/*				     don't check icc_response  */
/* OUT							       */
/*   common variable Gsct_response			       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*							       */
/* CALLED FUNCTIONS					       */
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
/*  aux_free2_OctetString				       */
/*							       */
/*  STAErrAnalyse	                                       */
/*							       */
/*  icc_create                 ERROR_Codes		       */
/*				 EINS			       */
/*			         EPARINC                       */
/*			         EMEMAVAIL                     */
/*				 ETOOLONG                      */ 
/*    							       */
/*  icc_check		       ERROR-Codes		       */
/*                               sw1/sw2 from ICC  response    */
/*							       */
/*							       */
/*-------------------------------------------------------------*/
int 
STACreateTrans(sct_id, flag)
	int             sct_id;
	Boolean         flag;
{
	Boolean         sec_mode;
	int             ssc = 0;

	sec_mode = FALSE;

	if (flag == TRUE) {

		/*
		 * call icc_create ; in case of no error Gicc_apdu.octets must be
		 * set free
		 */
		if (icc_create(&Gicc_param, sec_mode, ssc, &Gicc_apdu) == -1) {
			/* error  => release Gicc_apdu and return */
			sca_errno = icc_errno;
			sca_errmsg = icc_errmsg;
			return (-1);
		}
		/*
		 * prepare parameters for the S_TRANS-command 
		 */
		Gsct_cmd = S_TRANS;
		if ((GICCHEAD.security_mess.command  == ICC_SEC_NORMAL) &&
		    (GICCHEAD.security_mess.response == ICC_SEC_NORMAL)) 
			Gsct_request.rq_p1.secmode = TRANSP;
		else
			Gsct_request.rq_p1.secmode = SECURE;

		Gsct_request.rq_datafield.sccommand = &Gicc_apdu;
	}

/*****************************************************************************/
/*
 *      send ICC command via S_TRANS
 */



	/* call sct_interface		  */
	if (sct_interface(sct_id, Gsct_cmd, &Gsct_request, &Gsct_response) == -1) {
		sca_errno = sct_errno;
		sca_errmsg = sct_errmsg;
		if (flag == TRUE) {
			Gicc_apdu.noctets = 1;
			aux_free2_OctetString(&Gicc_apdu);
		}
		/* error => call err_analyse  */
		STAErrAnalyse(sct_id);
		return (-1);
	}

/*****************************************************************************/
/*
 *      normal end => release storage of Gicc_apdu.octets
 */
	Gicc_apdu.noctets = 1;
	aux_free2_OctetString(&Gicc_apdu);


	if (flag == TRUE) {
		/*
		 * check ICC response                   
		 */
		if (icc_check(&Gsct_response) == -1) {
			sca_errno = icc_errno;
			sca_errmsg = icc_errmsg;
			if ((GICCCMD == ICC_READB) ||
			    (GICCCMD == ICC_READR)) {
				/* check error in sca_read_binary or
				   sca_read_record                   */
				return (-1);
			}
			/* error => release buffer and return */
			Gsct_response.noctets = 1;
			aux_free2_OctetString(&Gsct_response);
			return (-1);
		}
	}
	return (0);
}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	STACreateTrans	       */
/*-------------------------------------------------------------*/


/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  STACreateHead            VERSION   1.1	    	       */
/*				     DATE   June 1993	       */
/*			      	       BY   Levona Eckstein    */
/*							       */
/* DESCRIPTION						       */
/*  Create Header of ICC command         	               */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   ins		       Instruction code		       */
/*                                                             */
/*   sec_mess                  security modes                  */
/*							       */
/* OUT							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*							       */
/* CALLED FUNCTIONS					       */
/*  STACheckSecMess	       ERROR-Codes	               */
/*                               EPARINC 		       */
/*-------------------------------------------------------------*/
int 
STACreateHead(ins, sec_mess)
	int             ins;
	ICC_SecMess      *sec_mess;
{

/*****************************************************************************/
/*
 *      check sec_mess
 */
	if (STACheckSecMess(sec_mess))
		return (-1);



/*****************************************************************************/
/*
 *      set instruction code
 */
	GICCCMD = ins;


/*****************************************************************************/
/*
 *      set security modes
 */
	GICCHEAD.security_mess.command = sec_mess->command;
	GICCHEAD.security_mess.response = sec_mess->response;



	return (0);

}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	STACreateHead          */
/*-------------------------------------------------------------*/



/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  STAGetBits              VERSION   1.1	    	       */
/*				     DATE   June 1993	       */
/*			      	       BY   Levona Eckstein    */
/*							       */
/* DESCRIPTION						       */
/*  Get n bits from position p.         	               */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   x  		       1 byte   		       */
/*                                                             */
/*   n                         number of bits                  */
/*							       */
/*   p                         position                        */
/* OUT							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
 /*   n-bit field of x that begins at position p                *//*-------------------------------------------------------------*/
unsigned int 
STAGetBits(x, n, p)
	unsigned int    x;
	unsigned int    n;
	unsigned int    p;

{

	return ((x >> (p + 1 - n)) & ~(~0 << n));
}

/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  STAGetVerifyDataBody     VERSION   1.1	    	       */
/*				     DATE   March   1993       */
/*			      	       BY   L. Eckstein        */
/*							       */
/* DESCRIPTION						       */
/*  Composes verification data structure to one character string*/
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   verify_data	       Structure which determines the  */
/*                             verification data to be installed*/
/*                                                             */
/* OUT							       */
/*   data_body		       data body           	       */
/*			       (data_body->octets must be      */
/*			        released by calling program.)  */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0  	         	OK			       */
/*                              EMEMAVAIL		       */
/*							       */
/* CALLED FUNCTIONS					       */
/*  e_keyId               	         		       */
/*							       */
/*   malloc macro in MF_check.h				       */ 
/*-------------------------------------------------------------*/
int
STAGetVerifyDataBody(verify_data, data_body)
	VerifyDataStruc *verify_data;
	OctetString     *data_body;

{
	int             body_ctr, pin_ctr;
	char		*proc="STAGetVerifyDataBody";

#define PIN_INFO verify_data->VerifyDataBody.pin_info
#define PUK_INFO verify_data->VerifyDataBody.puk_info

#define MAX_BODY 9


	data_body->noctets = MAX_BODY;
	if ((data_body->octets = (char *) malloc(data_body->noctets)) == NULL) {
		sca_errno = EMEMAVAIL;
		sca_errmsg = sca_errlist[sca_errno].msg;
		return (-1);
	}

	switch (verify_data->verify_data_type) {

/*****************************************************************************/
/*
 *      A PIN body on the smartcard consists of :           
 *	- one Byte: 	Minimum length,                        
 * 	- 8 octets:	pin value, padded with trailing  blanks	
 */
	case PIN:
		data_body->octets[0] = PIN_INFO.min_len;	/* add min length */

		body_ctr = 1;
		for (pin_ctr = 0; pin_ctr < PIN_INFO.pin->noctets;)
			data_body->octets[body_ctr++] = PIN_INFO.pin->octets[pin_ctr++];
		/* padding with blanks */
		for (; body_ctr <= MAXL_PIN; body_ctr++)
			data_body->octets[body_ctr] = ' ';
		break;


	case PUK:
/*****************************************************************************/
/*
 *      A PUK body on the smartcard consists of :           
 *	- one Byte: 	key_id,                        
 * 	- 8 octets:	puk value, padded with trailing blanks	
 */
		data_body->octets[0] = icc_e_KeyId(PUK_INFO.pin_key_id);	/* add key_id of pin */

		body_ctr = 1;
		for (pin_ctr = 0; pin_ctr < PUK_INFO.puk->noctets;)
			data_body->octets[body_ctr++] = PUK_INFO.puk->octets[pin_ctr++];
		/* padding with blanks */
		for (; body_ctr <= MAX_BODY; body_ctr++)
			data_body->octets[body_ctr] = ' ';
		break;

	}
			
	return (0);




}				/* end  STAGetVerifyDataBody */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	STAGetVerifyDataBody   */
/*-------------------------------------------------------------*/

/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  STACheckRSAKeyId         VERSION   1.1	    	       */
/*				     DATE   August 1993	       */
/*			      	       BY   Levona Eckstein    */
/*							       */
/* DESCRIPTION						       */
/*  Check key_id of private RSA key stored in the ICC and      */
/*  set ICC specific value.				       */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   key_id                    structur of key identifier      */
/*                                                             */
/* OUT							       */
/*   icc_keyid                structur of ICC key identifier   */
/*                                                             */
/* RETURN                    DESCRIPTION                       */
/*   0                          o.k                            */
/*  -1                          error                          */
/*                               EPARINC                       */
/*						               */
/*							       */
/*							       */
/*-------------------------------------------------------------*/

int
STACheckRSAKeyId(key_id, icc_keyid)
	KeyId	*key_id;
	KeyId	*icc_keyid;
{
	if (key_id == NULL) {
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		return (-1);
	}

	if (((key_id->key_level != FILE_MF) &&
	     (key_id->key_level != FILE_DF) &&
	     (key_id->key_level != FILE_CHILD)) ||
	    ((key_id->key_number < 1) ||
	     (key_id->key_number > 31))) {
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		return (-1);
	}

	/*
	 * set key_id to the corresponding value  
	 */
	switch (key_id->key_level) {
	case FILE_MF:
		icc_keyid->key_level = 0x04;
		break;
	case FILE_DF:
		icc_keyid->key_level = 0x05;
		break;
	case FILE_CHILD:
		icc_keyid->key_level = 0x06;
		break;
	}

	icc_keyid->key_number = key_id->key_number;

	return (0);
}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	STACheckRSAKeyId       */
/*-------------------------------------------------------------*/


/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  STAGetSCTState             VERSION   1.1	       */
/*				     DATE   December 1993      */
/*			      	       BY   Levona Eckstein    */
/*							       */
/* DESCRIPTION						       */
/*  Get actual state of the SCT and close the port in case     */
/*  of state = 0                                               */
/*  if the port will be closed sca_errno = ESTATE0 is returned */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/* OUT							       */
/*   state		       aktual state 		       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0				ok			       */
/*   -1			        ERROR			       */
/*				 ESTATE0		       */ 
/*							       */
/* CALLED FUNCTIONS					       */
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
/*  sct_set_portpar					       */
/*  aux_free2_OctetString				       */
/*							       */
/*-------------------------------------------------------------*/

int
STAGetSCTState(sct_id, state)
	int             sct_id;
	int		*state;
{

	int		rc;
	unsigned int	save_errno;

	char	*proc="STAGetSCTState";

	
	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);

	save_errno = sca_errno;


	/*
	 * call sct_interface		  
	 * send S_STATINF 		  
	 */

	rc = sct_interface(sct_id, S_STATINF, &Gsct_request,
		           	   &Gsct_response);
	if (rc < 0) {
		sca_errno = sct_errno;
		sca_errmsg = sct_errmsg;
		if ((sca_errno >= EDEVBUSY) && (sca_errno <= ESCTRESET)) {
			if ((sca_errno != EPROTRESYNCH) && (sca_errno != EDEVBUSY)) {
				save_errno = sca_errno;
				sct_close(sct_id, proc);
				sca_errno = save_errno;
				sca_errmsg = sca_errlist[sca_errno].msg; 
			}
		}
		return(-1);
	}

	/*
	 * test sct state
	 */
	*state = (int) *Gsct_response.octets / 16;
	*state = (*state * 10) + (int) *Gsct_response.octets % 16;
	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);

#ifdef TEST
	fprintf(stdout, "STAGetSCTState - actual state of the SCT: %d\n",*state);
#endif


	switch (*state) {
	case 0:
		/*
		 * SCT in state 0 => only S_RESET is allowed
		 */
		sct_close(sct_id, proc);
		sca_errno = ESTATE0;
		sca_errmsg = sca_errlist[sca_errno].msg;
		return(-1);

	case 5:
		/*
		 * call sct_interface		  
		 * S_STATUS must be send 
		 */
		rc = sct_interface(sct_id, S_STATUS, &Gsct_request,
				   &Gsct_response);
		if (rc < 0) {
			sca_errno = sct_errno;
			sca_errmsg = sct_errmsg;
			if ((sca_errno >= EDEVBUSY) && 
			    (sca_errno <= ESCTRESET)) {
				if ((sca_errno != EPROTRESYNCH) && (sca_errno != EDEVBUSY)) {
					save_errno = sca_errno;
					sct_close(sct_id, proc);
					sca_errno = save_errno;
					sca_errmsg = sca_errlist[sca_errno].msg;
				}
				return(-1);
			}
		}
		break;
	case 7:
		/*
		 * the smartcard must be removed 
		 */
		sca_errno = ECARDREM;
		sca_errmsg = sca_errlist[sca_errno].msg;
		break;

	default:
		if (sca_errno == EEXECDEN) {
			if (sct_states[*state] == TRUE) {
				sca_errno = ECARDINS;
				sct_set_portpar(sct_id, TRUE);
			}
			else {
				sca_errno = ENOCARD;
				sct_set_portpar(sct_id, FALSE);
			}

			sca_errmsg = sca_errlist[sca_errno].msg;
		}
		else {
			sca_errno = save_errno;
			sca_errmsg = sca_errlist[sca_errno].msg;
		}

		break;




	} /* end of switch */

	return(0);

}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	STAGetSCTState	       */
/*-------------------------------------------------------------*/







