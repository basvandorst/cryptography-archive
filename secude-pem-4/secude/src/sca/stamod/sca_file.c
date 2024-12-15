/*
 *  STARMOD Release 1.1 (GMD)
 */

/*---------------------------------------------------------------------------+-----*/
/*							                     | GMD */
/*   SYSTEM   STARMOD  -  Version 1.1		                             +-----*/
/*							                           */
/*---------------------------------------------------------------------------------*/
/*							                           */
/*    PACKAGE	SCAIF                                       VERSION 1.1	           */
/*					                       DATE June 1993      */
/*					                         BY Levona Eckstein*/
/*			       				                           */
/*    FILENAME     					                           */
/*	sca_file.c                          		         		   */
/*							                           */
/*    DESCRIPTION	   				                           */
/*      This modul provides all functions for file handling and data access        */
/*      on the smartcard  of the smartcard application interface                   */
/*							                           */
/*    EXPORT		    DESCRIPTION 		                           */
/*	sca_trans()	      Transfer of a smartcard command   	           */
/*							                           */
/*	sca_create_file()     Create file on the smartcard                         */
/*							                           */
/*	sca_register()        Registering of applications       	           */
/*							                           */
/*	sca_select_file()     Select file on the smartcard	                   */
/*										   */
/*      sca_close_file()      Close file on the smartcard                          */
/*										   */
/*      sca_lock_file()       Lock working elementary file on the smartcard        */
/*										   */
/*      sca_unlock_file()     Unlock WEF on the smartcard                          */
/*										   */
/*      sca_delete_file()     Delete file on the smartcard                         */
/*										   */
/*                                                                                 */
/*							                           */
/*                                                                                 */
/*    IMPORT		    DESCRIPTION 		                           */
/*	all functions and variables which are described in stamod.h                */
/*                                                                                 */
/*                                                                                 */
/*    INTERNAL                                                                     */
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
/*   macro definitions					       */
/*-------------------------------------------------------------*/
#define ALLOC_CHAR(v,s)  {if (0 == (v = malloc(s))) {sca_errno = EMEMAVAIL; sca_errmsg = sca_errlist[sca_errno].msg; goto errcase;}}



/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_trans                VERSION   1.1	    	       */
/*				     DATE   June 1993	       */
/*			      	       BY   Levona Eckstein    */
/*							       */
/* DESCRIPTION						       */
/*  Transfer of a smartcard command.         	               */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   icc_command                Smartcard command              */
/*							       */
/*   trans_mode		       Transfer mode                   */
/*			       (TRANSP,SECURE)   	       */
/*							       */
/* OUT							       */
/*   icc_response               Response of the smartcard      */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*			         EPARINC                        */
/*                               EMEMAVAIL                     */
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
/*   malloc macro in MF_check.h			               */ 
/*-------------------------------------------------------------*/
int 
sca_trans(sct_id, icc_command, trans_mode, icc_response)
	int             sct_id;
	OctetString    *icc_command;
	TransMode       trans_mode;
	OctetString    *icc_response;
{
	int             i;
	char		*proc="sca_trans";

	sca_errno = M_NOERR;
	sca_errmsg = NULL;


#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_trans *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id                  : %d\n", sct_id);
	fprintf(stdout, "icc_command             : \n");
	fprintf(stdout, "    noctets             : %d\n", icc_command->noctets);
	fprintf(stdout, "    octets              : \n");
	aux_fxdump(stdout, icc_command->octets, icc_command->noctets, 0);
	print_TransMode(trans_mode);
#endif

/*****************************************************************************/
/*
 *      check input parameter
 */

	if ((icc_command == NULLOCTETSTRING) ||
	    (icc_command->octets == NULL) ||
            (icc_command->noctets == 0))
		goto parerrcase;

	if (icc_response == NULL)
		goto parerrcase;

	if ((trans_mode != TRANSP) &&
            (trans_mode != SECURE))
		goto parerrcase;

/*
 *      the correctness of the icc command specific parameters are checked 
 * 	in the function icc_create
 */



/*****************************************************************************/
/*
 *      call STACheckSCT_ICC
 */

	if (STACheckSCT_ICC(sct_id, TRUE, proc))
		return (-1);


/*****************************************************************************/
/*
 *      create SCT command S_TRANS
 */
	Gsct_cmd = S_TRANS;
	Gsct_request.rq_p1.secmode = trans_mode;
	Gsct_request.rq_datafield.sccommand = icc_command;

	/* call STACreateTrans			  */
	if (STACreateTrans(sct_id, FALSE))
		return (-1);

/*****************************************************************************/
/*
 *      normal end => transfer response message  from ICC into icc_response 
 */
	icc_response->noctets = Gsct_response.noctets;
	ALLOC_CHAR(icc_response->octets, icc_response->noctets);

	for (i = 0; i < icc_response->noctets; i++)
		icc_response->octets[i] = Gsct_response.octets[i];
	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);

#ifdef TEST
	fprintf(stdout, "TRACE of the output parameters : \n");
	fprintf(stdout, "icc_response            : \n");
	fprintf(stdout, "    noctets             : %d\n", icc_response->noctets);
	fprintf(stdout, "    octets              : \n");
	aux_fxdump(stdout, icc_response->octets, icc_response->noctets, 0);
	fprintf(stdout, "\n***** Normal end of   sca_trans *****\n\n");
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

/*****************************************************************************/
/*
 *      Memory error
 */
errcase:
	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);
	return (-1);


}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_trans	       */
/*-------------------------------------------------------------*/



/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_create_file          VERSION   1.1	    	       */
/*				     DATE   June 1993	       */
/*			      	       BY   Levona Eckstein    */
/*							       */
/* DESCRIPTION						       */
/*  Create file on the smartcard.        	               */
/*  Sca_create_file creates a new file (MF,DF,CHILD_DF or EF)  */
/*  on the smartcard. 					       */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   file_cat                  File category(MF,DF,CHILD_DF,EF)*/
/*							       */
/*   data_struc                Data structure of the EF        */
/* 			       (LIN_FIX,LIN_VAR,CYCLIC,        */
/*			        TRANSPARENT)		       */
/*   file_control_info         File control information        */
/*						  	       */
/*   sec_mess		       security modes                  */
/*							       */
/* OUT							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
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
/*							       */
/*							       */
/*-------------------------------------------------------------*/
int 
sca_create_file(sct_id, file_cat, data_struc, file_control_info,
		sec_mess)
	int             sct_id;
	FileCat         file_cat;
	DataStruc       data_struc;
	FileControlInfo *file_control_info;
	ICC_SecMess      *sec_mess;
{
	char		*proc="sca_create_file";

	sca_errno = M_NOERR;
	sca_errmsg = NULL;


#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_create_file *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id		        : %d\n", sct_id);
	print_FileCat(file_cat);

	print_DataStruc(data_struc);
	fprintf(stdout, "                          set to 0x00 in case of MF,DF or CHILD_DF\n");

	print_FileControlInfo(file_cat, file_control_info);
	print_ICC_SecMess(sec_mess);

#endif



/*
 *      the correctness of the icc command specific parameters are checked 
 * 	in the function icc_create
 */




/*****************************************************************************/
/*
 *      call STACheckSCT_ICC
 */

	if (STACheckSCT_ICC(sct_id, TRUE, proc))
		return (-1);

/*****************************************************************************/
/*
 *      create ICC command CREATE
 */
	/* create header                       */
	if (STACreateHead(ICC_CREATE, sec_mess))
		return (-1);

	/* set parameters			  */
	PAR_FILECAT	 = file_cat;
	if (file_cat != EF) {
		data_struc = 0x00;
	}
	PAR_DATASTRUC	 = data_struc;
	PAR_FILECONTROL	 = file_control_info;


	/* call STACreateTrans			  */
	if (STACreateTrans(sct_id, TRUE))
		return (-1);

/*****************************************************************************/
/*
 *       normal end => release response storage
 */
	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);

#ifdef TEST
	fprintf(stdout, "\n***** Normal end of   sca_create_file *****\n\n");
#endif
	return (0);


}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_create_file	       */
/*-------------------------------------------------------------*/


/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_register             VERSION   1.1	    	       */
/*				     DATE   June 1993	       */
/*			      	       BY   Levona Eckstein    */
/*							       */
/* DESCRIPTION						       */
/*  Registering of applications.         	               */
/*  With sca_register a DF which shall be created must be      */
/*  registered on the smartcard.			       */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   DF_name                   Name of the DF                  */
/*							       */
/*   memory_units	       Number of units required for    */
/*			       this DF           	       */
/*   sec_status                Security status to protect the  */
/*			       creation			       */
/*   sec_mess		       security modes                  */
/*							       */
/* OUT							       */
/*							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
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
/*  STACreateHead         	ERROR-Codes    		       */
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
/*							       */
/*-------------------------------------------------------------*/
int 
sca_register(sct_id, DF_name, memory_units, sec_status,
	     sec_mess)
	int             	sct_id;
	OctetString    		*DF_name;
	int            		memory_units;
	AccessControlValue	sec_status;
	ICC_SecMess     	*sec_mess;
{
	char			*proc="sca_register";

	sca_errno = M_NOERR;
	sca_errmsg = NULL;


#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_register *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id                  : %d\n", sct_id);
	print_OctetString("DF_name                 : ",DF_name);
	fprintf(stdout, "memory_units            : %d\n", memory_units);
	print_AccessControlValue("sec_status              : \n",sec_status);
	print_ICC_SecMess(sec_mess);

#endif



/*
 *      the correctness of the icc command specific parameters are checked 
 * 	in the function icc_create
 */


/*****************************************************************************/
/*
 *      call STACheckSCT_ICC
 */

	if (STACheckSCT_ICC(sct_id, TRUE, proc))
		return (-1);


/*****************************************************************************/
/*
 *      create ICC command REGISTER
 */
	/* create header                       */
	if (STACreateHead(ICC_REGISTER, sec_mess))
		return (-1);


	/* set parameters			  */
	PAR_UNITS = memory_units;
	PAR_ACV   = sec_status;
	PAR_DATA  = DF_name;

	/* call STACreateTrans			  */
	if (STACreateTrans(sct_id, TRUE))
		return (-1);

/*****************************************************************************/
/*
 *      normal end => release response storage
 */
	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);

#ifdef TEST
	fprintf(stdout, "\n***** Normal end of   sca_register *****\n\n");
#endif
	return (0);


}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_register	       */
/*-------------------------------------------------------------*/


/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_select_file          VERSION   1.1	    	       */
/*				     DATE   June 1993	       */
/*			      	       BY   Levona Eckstein    */
/*							       */
/* DESCRIPTION						       */
/*  Select file on the smartcard.         	               */
/*  Sca_select_file will be used to select the file (MF,DF or  */
/*  SF) on the smartcard and to set the smartcard in the       */
/*  context of this file.				       */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   file_cat                  File category(DF,CHILD_DF,EF)   */
/*							       */
/*   file_sel                  File name / File identifier     */
/*			                         	       */
/*   file_info_req             File information requested      */
/*			       (NONE or COMP)		       */
/*   sec_mess		       security modes                  */
/* OUT							       */
/*							       */
/*   file_info                 Returned file information       */
/*			       file_info->addinfo->octets will */
/*			       be allocated by the called program */ 	
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*				 EPARINC 		       */
/*                               EMEMAVAIL                     */
/*                               ERESPDAT                      */
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
/*							       */
/*							       */
/*   malloc macro in MF_check.h			               */ 
/*-------------------------------------------------------------*/
int 
sca_select_file(sct_id, file_cat, file_sel, file_info_req,
		file_info, sec_mess)
	int             sct_id;
	FileCat         file_cat;
	FileSel        *file_sel;
	FileInfoReq     file_info_req;
	FileInfo       *file_info;
	ICC_SecMess     *sec_mess;
{

	int             i;
	ICC_SecMess	icc_sec_mess;
	char		*proc="sca_select_file";

	sca_errno = M_NOERR;
	sca_errmsg = NULL;



#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_select_file *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id                  : %d\n", sct_id);
	print_FileCat(file_cat);
	print_FileSel("file_sel                : \n",file_cat, file_sel);
	print_FileInfoReq(file_info_req);
	print_ICC_SecMess(sec_mess);
#endif

/*
 *      the correctness of the icc command specific parameters are checked 
 * 	in the function icc_create
 */



/*****************************************************************************/
/*
 *      call STACheckSCT_ICC 
 */
	if (STACheckSCT_ICC(sct_id, TRUE, proc))
		return (-1);


/*****************************************************************************/
/*
 *      create ICC command SELECT
 */

	/* create header                       */
	icc_sec_mess.command = sec_mess->command;
	icc_sec_mess.response= sec_mess->response;
 
	if ((file_cat == PARENT_DF) ||
	    (file_cat == CHILD_DF)) {
		/*
		 * test security mode for the response
		 */
		if (icc_sec_mess.response != ICC_SEC_NORMAL)
			icc_sec_mess.response = ICC_SEC_NORMAL;
	} 
		
  
	if (STACreateHead(ICC_SELECT, &icc_sec_mess))
		return (-1);


	/* set parameters			  */
	PAR_FILECAT	 = file_cat;
	PAR_FILEINFO	 = file_info_req;
	PAR_FILESEL	 = file_sel;


	/* call STACreateTrans			  */
	if (STACreateTrans(sct_id, TRUE))
		return (-1);


/*****************************************************************************/
/*
 *      normal end => get file information and release response storage
 */

	/*
	 * if file_info_req == COMP_INFO => get file information 
	 */
	if (file_info_req == COMP_INFO) {

		file_info->addinfo->noctets = 0;
		file_info->addinfo->octets  = NULL;

		if (Gsct_response.noctets != 5) {	/* addinfo = 4 + 1 byte FSTAT */
			sca_errno = ERESPDAT;
			sca_errmsg = sca_errlist[sca_errno].msg;
			Gsct_response.noctets = 1;
			aux_free2_OctetString(&Gsct_response);
			return (-1);
		} else {
			/* create file_info->file_status */
			file_info->file_status.install_status = STAGetBits((unsigned) Gsct_response.octets[0], 2, 1);
			file_info->file_status.file_memory = STAGetBits((unsigned) Gsct_response.octets[0], 1, 6);
			file_info->file_status.file_access = STAGetBits((unsigned) Gsct_response.octets[0], 1, 7);

			/* create file_info->addinfo      */
			file_info->addinfo->noctets = Gsct_response.noctets - 1;
			ALLOC_CHAR(file_info->addinfo->octets,
				   file_info->addinfo->noctets);
			for (i = 0; i < file_info->addinfo->noctets; i++)
				*(file_info->addinfo->octets+i) = Gsct_response.octets[i+1];


		}
	}

	/*
	 * release response storage
	 */

	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);

#ifdef TEST
	fprintf(stdout, "TRACE of the output parameters : \n");
	if (file_info_req != NONE_INFO) {
		fprintf(stdout, "file_status             : %d\n", sct_id);
		switch (file_info->file_status.install_status) {
		case REGISTERED:
			fprintf(stdout, "    install_status      : REGISTERED\n");
			break;
		case DELETED:
			fprintf(stdout, "    install_status      : DELETED\n");
			break;
		case DEL_PENDING:
			fprintf(stdout, "    install_status      : DEL_PENDING\n");
			break;
		case INSTALLED:
			fprintf(stdout, "    install_status      : INSTALLED\n");
			break;
		default:
			fprintf(stdout, "    install_status      : undefined\n");
			break;
		}
		switch (file_info->file_status.file_memory) {
		case MEM_CONSISTENT:
			fprintf(stdout, "    file_memory         : MEM_CONSISTENT\n");
			break;
		case MEM_INCONSISTENT:
			fprintf(stdout, "    file_memory         : MEM_INCONSISTENT\n");
			break;
		default:
			fprintf(stdout, "    file_memory         : undefined\n");
			break;
		}
		switch (file_info->file_status.file_access) {
		case FILE_UNLOCKED:
			fprintf(stdout, "    file_access         : FILE_UNLOCKED\n");
			break;
		case FILE_LOCKED:
			fprintf(stdout, "    file_access         : FILE_LOCKED\n");
			break;
		default:
			fprintf(stdout, "    file_access         : undefined\n");
			break;
		}

		print_OctetString("addinfo                 : ", file_info->addinfo);
	} else
		fprintf(stdout, "file_info_req           : no output data \n");
	fprintf(stdout, "\n***** Normal end of   sca_select_file *****\n\n");
#endif
	return (0);

/*****************************************************************************/
/*
 *      Memory error
 */
errcase:
	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);
	return (-1);



}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_select_file	       */
/*-------------------------------------------------------------*/


/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_close_file           VERSION   1.1	    	       */
/*				     DATE   June 1993	       */
/*			      	       BY   Levona Eckstein    */
/*							       */
/* DESCRIPTION						       */
/*  Close file on the smartcard.         	               */
/*  Sca_close_file closes the specified file and also all files*/
/*  belonging to the related file.			       */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   file_cat                  File category(MF,DF,CHILD_DF,EF)*/
/*							       */
/*   file_sel                  File name / File identifier     */
/*			                         	       */
/*   file_close_context        Close context                   */
/*			       (CLOSE_CREATE or CLOSE_SELECT)  */
 /*   sec_mess		       security modes                  *//* */
/* OUT							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
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
/*							       */
/*  STACreateHead         	ERROR-Codes    		       */
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
/*							       */
/*-------------------------------------------------------------*/
int 
sca_close_file(sct_id, file_cat, file_sel, file_close_context,
	       sec_mess)
	int             	sct_id;
	FileCat         	file_cat;
	FileSel        		*file_sel;
	FileCloseContext 	file_close_context;
	ICC_SecMess       	*sec_mess;
{
	ICC_SecMess     icc_sec_mess;
	char		*proc="sca_close_file";

	sca_errno = M_NOERR;
	sca_errmsg = NULL;


#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_close_file *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id		        : %d\n", sct_id);
	print_FileCat(file_cat);
	print_FileSel("file_sel                : \n",file_cat, file_sel);
	print_FileCloseContext(file_close_context);
	print_ICC_SecMess(sec_mess);
#endif


/*
 *      the correctness of the icc command specific parameters are checked 
 * 	in the function icc_create
 */


/*****************************************************************************/
/*
 *      call STACheckSCT_ICC 
 */
	if (STACheckSCT_ICC(sct_id, TRUE, proc))
		return (-1);


/*****************************************************************************/
/*
 *      create ICC command CLOSE
 */
	/* create header                       */
	icc_sec_mess.command = sec_mess->command;
	icc_sec_mess.response= sec_mess->response;
 
	if ((file_cat == PARENT_DF) ||
	    (file_cat == CHILD_DF)) {
		/*
		 * test security mode for the response
		 */
		if (icc_sec_mess.response != ICC_SEC_NORMAL)
			icc_sec_mess.response = ICC_SEC_NORMAL;
	} 

	if (STACreateHead(ICC_CLOSE, &icc_sec_mess))
		return (-1);


	/* set parameters			  */
	PAR_FILECAT	 = file_cat;
	PAR_FILECONTEXT	 = file_close_context;
	PAR_FILESEL	 = file_sel;


	/* call STACreateTrans			  */
	if (STACreateTrans(sct_id, TRUE))
		return (-1);

/*****************************************************************************/
/*
 *      normal end => release response storage
 */
	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);

#ifdef TEST
	fprintf(stdout, "\n***** Normal end of   sca_close_file *****\n\n");
#endif
	return (0);


}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_close_file	       */
/*-------------------------------------------------------------*/



/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_lock_file            VERSION   1.1	    	       */
/*				     DATE   June 1993	       */
/*			      	       BY   Levona Eckstein    */
/*							       */
/* DESCRIPTION						       */
/*  Lock working elementary file on the smartcard.             */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   file_id                   File identifier                 */
/*							       */
/*   sec_mess		       security modes                  */
/*							       */
/* OUT							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
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
/*							       */
/*-------------------------------------------------------------*/
int 
sca_lock_file(sct_id, file_id, sec_mess)
	int             sct_id;
	FileId         *file_id;
	ICC_SecMess     *sec_mess;
{
	char		*proc="sca_lock_file";

	sca_errno = M_NOERR;
	sca_errmsg = NULL;


#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_lock_file *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id                  : %d\n", sct_id);
	print_FileId(file_id);
	print_ICC_SecMess(sec_mess);
#endif


/*
 *      the correctness of the icc command specific parameters are checked 
 * 	in the function icc_create
 */


/*****************************************************************************/
/*
 *      call STACheckSCT_ICC
 */
	if (STACheckSCT_ICC(sct_id, TRUE, proc))
		return (-1);


/*****************************************************************************/
/*
 *      create ICC command LOCKF
 */
	/* create header                       */
	if (STACreateHead(ICC_LOCKF, sec_mess))
		return (-1);


	/* set parameters			  */
	PAR_FID		 = file_id;
	PAR_OPERATION	 =  CO_LOCK;


	/* call STACreateTrans			  */
	if (STACreateTrans(sct_id, TRUE))
		return (-1);

/*****************************************************************************/
/*
 *      normal end => release response storage
 */

	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);

#ifdef TEST
	fprintf(stdout, "\n***** Normal end of   sca_lock_file *****\n\n");
#endif
	return (0);






}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_lock_file	       */
/*-------------------------------------------------------------*/


/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_unlock_file          VERSION   1.1	    	       */
/*				     DATE   June 1993	       */
/*			      	       BY   Levona Eckstein    */
/*							       */
/* DESCRIPTION						       */
/*  Unlock working elementary file on the smartcard.           */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   file_id                   File identifier                 */
/*							       */
/*   file_name                 File name                       */
/*			                         	       */
/*   sec_mess		       security modes                  */
/*							       */
/* OUT							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
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
/*							       */
/*-------------------------------------------------------------*/
int 
sca_unlock_file(sct_id, file_id, sec_mess)
	int             sct_id;
	FileId         *file_id;
	ICC_SecMess     *sec_mess;
{
	char		*proc="sca_unlock_file";

	sca_errno = M_NOERR;
	sca_errmsg = NULL;


#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_unlock_file *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id                  : %d\n", sct_id);
	print_FileId(file_id);
	print_ICC_SecMess(sec_mess);
#endif


/*
 *      the correctness of the icc command specific parameters are checked 
 * 	in the function icc_create
 */


/*****************************************************************************/
/*
 *      call STACheckSCT_ICC
 */

	if (STACheckSCT_ICC(sct_id, TRUE, proc))
		return (-1);


/*****************************************************************************/
/*
 *      create ICC command LOCKF
 */
	/* create header                       */
	if (STACreateHead(ICC_LOCKF, sec_mess))
		return (-1);


	/* set parameters			  */
	PAR_FID		 = file_id;
	PAR_OPERATION	 = CO_UNLOCK;


	/* call STACreateTrans			  */
	if (STACreateTrans(sct_id, TRUE))
		return (-1);

/*****************************************************************************/
/*
 *      normal end => release response storage
 */
	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);

#ifdef TEST
	fprintf(stdout, "\n***** Normal end of   sca_unlock_file *****\n\n");
#endif
	return (0);






}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_unlock_file	       */
/*-------------------------------------------------------------*/


/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_delete_file          VERSION   1.1	    	       */
/*				     DATE   June 1993	       */
/*			      	       BY   Levona Eckstein    */
/*							       */
/* DESCRIPTION						       */
/*  Delete file on the smartcard.         	               */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   file_cat                  File category(DF,CHILD_DF,EF)   */
/*							       */
/*   file_sel                  File name / File identifier     */
/*			                         	       */
/*   sec_mess		       security modes                  */
/*							       */
/* OUT							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
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
/*							       */
/*-------------------------------------------------------------*/
int 
sca_delete_file(sct_id, file_cat, file_sel, sec_mess)
	int             sct_id;
	FileCat         file_cat;
	FileSel        *file_sel;
	ICC_SecMess     *sec_mess;
{
	ICC_SecMess     icc_sec_mess;
	char		*proc="sca_delete_file";

	sca_errno = M_NOERR;
	sca_errmsg = NULL;


#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_delete_file *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id                  : %d\n", sct_id);
	print_FileCat(file_cat);
	print_FileSel("file_sel                : \n",file_cat, file_sel);
	print_ICC_SecMess(sec_mess);
#endif


/*
 *      the correctness of the icc command specific parameters are checked 
 * 	in the function icc_create
 */


/*****************************************************************************/
/*
 *      call STACheckSCT_ICC
 */
	if (STACheckSCT_ICC(sct_id, TRUE, proc))
		return (-1);


/*****************************************************************************/
/*
 *      create ICC command DELF
 */
	/* create header                       */
	icc_sec_mess.command = sec_mess->command;
	icc_sec_mess.response= sec_mess->response;
 
	if ((file_cat == PARENT_DF) ||
	    (file_cat == CHILD_DF) ||
	    ((file_cat == EF) && (file_sel->file_id->file_type == ISF))) {
		/*
		 * test security mode for the response
		 */
		if (icc_sec_mess.response != ICC_SEC_NORMAL)
			icc_sec_mess.response = ICC_SEC_NORMAL;
	} 

	if (STACreateHead(ICC_DELF, &icc_sec_mess))
		return (-1);


	/* set parameters			  */
	PAR_FILECAT	 = file_cat;
	PAR_FILESEL	 = file_sel;


	/* call STACreateTrans			  */
	if (STACreateTrans(sct_id, TRUE))
		return (-1);

/*****************************************************************************/
/*
 *       normal end => release response storage
 */
	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);

#ifdef TEST
	fprintf(stdout, "\n***** Normal end of   sca_delete_file *****\n\n");
#endif
	return (0);





}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_delete_file	       */
/*-------------------------------------------------------------*/



