/*
 *  STARMOD Release 1.1 (GMD)
 */
/*---------------------------------------------------------------------------+-----*/
/*							                     | GMD */
/*   SYSTEM   STARMOD  -  Version 1.1		                             +-----*/
/*							                           */
/*---------------------------------------------------------------------------------*/
/*							                           */
/*    PACKAGE	PRINT                                        VERSION 1.1	   */
/*					                       DATE June 1993      */
/*					                         BY Levona Eckstein*/
/*			       				                           */
/*    FILENAME     					                           */
/*	sca_prnt.c                       		         		   */
/*							                           */
/*    DESCRIPTION	   				                           */
/*      This modul provides a function which prints the error message.	           */
/*							                           */
/*    EXPORT		    DESCRIPTION 		                           */
/*	sca_print_errmsg()     print error message    			           */
/*							                           */
/*      all print_... functions							   */
/*      									   */
/*      									   */
/*                                                                                 */
/*    IMPORT		    DESCRIPTION 		                           */
/*							                           */
/*	all functions and variables which are described in stamod.h                */

/*                                                                                 */
/*    INTERNAL                                                                     */
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
/*   forward global function and  variable declarations	       */
/*-------------------------------------------------------------*/

#ifdef TEST
char            text1[27] = "file_type               : ";
char            text2[23] = "file_type           : ";
void            print_FileCat();
void            print_FileType();
void            print_DataStruc();
void            print_FileControlInfo();
void            print_SCT_SecMess();
void            print_ICC_SecMess();
void            print_FileSel();
void            print_FileId();
void            print_FileInfoReq();
void            print_FileCloseContext();
void            print_KeyId();
void            print_TransMode();
void            print_KeyPurpose();
void            print_KeyDevSel();
void            print_KeyAttrList();
void            print_VerifyDataStruc();
void            print_KeyIdList();
void		print_AccessControlValue();
void 		print_WriteMode();
void		print_Boolean();
void		print_OctetString();
void		print_Destination();
void		print_ChallengeMode();
void		print_KeySel();
void		print_More();
void 		print_Signature();
void		print_HashPar();
void		print_KeyInfo();
void  		print_KeyLevel();
void		print_KeyDevList();
void		print_EncryptedKey();
void	 	print_VerificationKey();
#endif





/*-------------------------------------------------------------*/
/*   type definitions					       */
/*-------------------------------------------------------------*/




/*-------------------------------------------------------------*/
/*   local variable definitions			               */
/*-------------------------------------------------------------*/




/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_print_errmsg         VERSION   1.1	    	       */
/*				     DATE   June 1993	       */
/*			      	       BY   U.Viebeg           */
/*							       */
/* DESCRIPTION						       */
/*  Print the given text together with the global error        */
/*  message on stderr.                 			       */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   msg		       text, which is printed 	       */
/*                             together with error message     */
/*                                                             */
/*							       */
/* OUT							       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*				 wrong sca_errno	       */
/*							       */
/* CALLED FUNCTIONS					       */
/*  sct_perror                ERROR-Codes		       */
/*			         0  (ok)		       */
/*							       */
/*							       */
/*-------------------------------------------------------------*/
int 
sca_print_errmsg(msg)
	char           *msg;
{
	int             rc;

	rc = 0;



#ifdef TEST
	fprintf(stdout, "\n*****  sca_print_errmsg ********************************************\n\n");
	fprintf(stdout, "input-parameters:\n");
	if ((msg != NULL) && (strlen(msg) > 0)) {
		fprintf(stdout, "msg:     %s\n", msg);
	}
	fprintf(stdout, "\n\n");
#endif

	if (sca_errno <= 0) {	/* no error number set */
		if (msg && strlen(msg))
			fprintf(stdout, "%s: ", msg);
		fprintf(stdout, "No error set \n");
	} else {
			if (msg && strlen(msg))
				fprintf(stdout, "%s: ", msg);
			fprintf(stdout, "%s\n", sca_errlist[sca_errno].msg);
	}		/* end else */


#ifdef TEST
	fprintf(stdout, "\n***** Normal end of   sca_print_errmsg *********************************************\n\n");
#endif
	return (0);





}				/* end  sca_print_errmsg */

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_print_errmsg       */
/*-------------------------------------------------------------*/

#ifdef TEST


/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  print_routines           VERSION   1.1	    	       */
/*				     DATE   June 1993	       */
/*			      	       BY   Levona Eckstein    */
/*							       */
/* DESCRIPTION						       */
/*  Trace functions                     	               */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*-------------------------------------------------------------*/
void 
print_FileCat(file_cat)
	FileCat         file_cat;
{
	switch (file_cat) {
	case MF:
		fprintf(stdout, "FileCat  		: MF\n");
		break;
	case PARENT_DF:
		fprintf(stdout, "FileCat  		: PARENT_DF\n");
		break;
	case CHILD_DF:
		fprintf(stdout, "FileCat  		: CHILD_DF\n");
		break;
	case EF:
		fprintf(stdout, "FileCat  		: EF\n");
		break;
	default:
		fprintf(stdout, "FileCat  		: undefined\n");
		break;
	};
}
void 
print_FileType(file_type, msg)
	FileType        file_type;
	char           *msg;
{


	if (msg == NULL)
		fprintf(stdout, "%s", text1);
	else {
		fprintf(stdout, "%s", msg);
		fprintf(stdout, "%s", text2);
	}

	switch (file_type) {
	case XEF:
		fprintf(stdout, "XEF\n");
		break;
	case WEF:
		fprintf(stdout, "WEF\n");
		break;
	case ACF:
		fprintf(stdout, "ACF\n");
		break;
	case ISF:
		fprintf(stdout, "ISF\n");
		break;
	default:
		fprintf(stdout, "undefined\n");
		break;
	};
}

void 
print_DataStruc(data_struc)
	DataStruc       data_struc;
{
	switch (data_struc) {
	case LIN_FIX:
		fprintf(stdout, "DataStruc  		: LIN_FIX\n");
		break;
	case LIN_VAR:
		fprintf(stdout, "DataStruc  		: LIN_VAR\n");
		break;
	case CYCLIC:
		fprintf(stdout, "DataStruc  		: CYCLIC\n");
		break;
	case TRANSPARENT:
		fprintf(stdout, "DataStruc  		: TRANSPARENT\n");
		break;
	default:
		fprintf(stdout, "DataStruc  		: undefined\n");
		break;
	};
}
void 
print_Destination(destination)
	Destination destination;
{
	switch (destination) {
	case SCT:
		fprintf(stdout, "Destination             : SCT\n");
		break;
	case ICC:
		fprintf(stdout, "Destination             : ICC\n");
		break;
	default:
		fprintf(stdout, "Destination             : undefined\n");
		break;
	};
}
void 
print_ChallengeMode(mode)
	ChallengeMode	mode;
{
	switch (mode) {
	case GET_CHALLENGE:
		fprintf(stdout, "ChallengeMode           : GET_CHALLENGE\n");
		break;
	case SEND_CHALLENGE:
		fprintf(stdout, "ChallengeMode           : SEND_CHALLENGE\n");
		break;
	case EXCHANGE_CHALLENGE:
		fprintf(stdout, "ChallengeMode           : EXCHANGE_CHALLENGE\n");
		break;
	default:
		fprintf(stdout, "ChallengeMode           : undefined\n");
		break;
	};
}


void 
print_FileControlInfo(file_cat, file_control_info)
	FileCat         file_cat;
	FileControlInfo *file_control_info;
{

	fprintf(stdout, "FileControlInfo         :\n");
	fprintf(stdout, "    units               : %d\n", file_control_info->units);
	print_AccessControlValue("    delrec_acv          :\n", file_control_info->delrec_acv);
	print_AccessControlValue("    racv                :\n",file_control_info->racv);
	print_AccessControlValue("    wacv                :\n",file_control_info->wacv);
	print_AccessControlValue("    delfile_acv         :\n", file_control_info->delfile_acv);

	switch (file_control_info->access_sec_mode) {
	case ACCESS_NORMAL:
		fprintf(stdout, "    access_sec_mode     : ACCESS_NORMAL\n");
		break;
	case ACCESS_AUTH:
		fprintf(stdout, "    access_sec_mode     : ACCESS_AUTH\n");
		break;
	case ACCESS_CONC:
		fprintf(stdout, "    access_sec_mode     : ACCESS_CONC\n");
		break;
	case ACCESS_COMBINED:
		fprintf(stdout, "    access_sec_mode     : ACCESS_COMBINED\n");
		break; 
	default:
		fprintf(stdout, "    access_sec_mode     : undefined\n");
		break;
	};

	switch (file_control_info->readwrite) {
	case READ_WRITE:
		fprintf(stdout, "    readwrite           : READ_WRITE\n");
		break;
	case WORM:
		fprintf(stdout, "    readwrite           : WORM\n");
		break;
	case READ_ONLY:
		fprintf(stdout, "    readwrite           : READ_ONLY\n");
		break;
	case WRITE_ONLY:
		fprintf(stdout, "    readwrite           : WRITE_ONLY\n");
		break;
	default:
		fprintf(stdout, "    readwrite           : undefined\n");
		break;
	};
	print_Boolean(  "    execute             : ",file_control_info->execute);
	print_Boolean(  "    not_erasable        : ",file_control_info->not_erasable);
	fprintf(stdout, "    recordsize          : %d\n", file_control_info->recordsize);
	print_FileSel("    file_sel            :\n", file_cat,
		      file_control_info->file_sel);
	print_OctetString("addinfo                 : ",file_control_info->addinfo);

}
void
print_Boolean(text,value)
	char	*text;
	Boolean	value;
{
	switch (value) {
	case TRUE:
		fprintf(stdout,"%sTRUE\n",text);
		break;
	case FALSE:
		fprintf(stdout,"%sFALSE\n",text);
		break;
	default:
		fprintf(stdout,"%sundefined\n",text);
		break;
	}
}
void
print_AccessControlValue(text,acv)
	char			*text;
	AccessControlValue	acv;
{
	fprintf(stdout,text);
	switch(acv.compare_mode) {
	case ST_EQ:
		fprintf(stdout, "           compare_mode : ST_EQ\n");
		break;
	case ST_LT:
		fprintf(stdout, "           compare_mode : ST_LT\n");
		break;
	case ST_GE:
		fprintf(stdout, "           compare_mode : ST_GE\n");
		break;
	case ST_NOT_EQ:
		fprintf(stdout, "           compare_mode : ST_NOT_EQ\n");
		break;
	default:
		fprintf(stdout, "           compare_mode : undefined\n");
		break;
	}
	fprintf(stdout, "           state_number : %d\n",acv.state_number);
}	

void 
print_WriteMode(write_mode)
	WriteMode	write_mode;
{
	switch (write_mode) {
	case REC_UPDATE:
		fprintf(stdout, "WriteMode 		: REC_UPDATE\n");
		break;
	case REC_APPEND:
		fprintf(stdout, "WriteMode 		: REC_APPEND\n");
		break;
	default:
		fprintf(stdout, "WriteMode 		: undefined\n");
		break;
	};
}
void 
print_FileSel(text,file_cat, file_sel)
	char		*text;
	FileCat         file_cat;
	FileSel        *file_sel;
{
	int i;

	fprintf(stdout,"%s",text);
	if (file_cat != EF) 
			print_OctetString("    file_name           : ",
				  	file_sel->file_name);
	else
		print_FileId(file_sel->file_id);
}
void
print_OctetString(text,octet)
	char		*text;
	OctetString	*octet;
{
	int	i;

	fprintf(stdout,"%s",text);
	if ((octet == NULLOCTETSTRING) ||
	    (octet->octets == NULL) ||
	    (octet->noctets == 0))
		fprintf(stdout,"NULL\n");
	else {
		fprintf(stdout,"\nnoctets                 : %d\n",octet->noctets);
		fprintf(stdout,"octets                  : \n");
		aux_fxdump(stdout, octet->octets, octet->noctets, 0);
	}
}
void
print_More(more)
	More more;
{

	if (more == SEC_MORE)
		fprintf(stdout, "More                    : SEC_MORE\n");
	else {
		if (more == SEC_END)
			fprintf(stdout, "More                    : SEC_END\n");
		else
			fprintf(stdout, "More                    : undefined\n");
	}
}


void 
print_FileId(file_id)
	FileId         *file_id;
{

	fprintf(stdout, "FileId                  :\n");
	fprintf(stdout, "    file_qualifier      : %x\n", file_id->file_qualifier);
	switch (file_id->file_level) {
	case MF_LEVEL:
		fprintf(stdout, "    file_level          : MF_LEVEL\n");
		break;
	case DF_LEVEL:
		fprintf(stdout, "    file_level          : DF_LEVEL\n");
		break;
	case CHILD_LEVEL:
		fprintf(stdout, "    file_level          : CHILD_LEVEL\n");
		break;
	default:
		fprintf(stdout, "    file_level          : undefined\n");
		break;
	};

	print_FileType(file_id->file_type, "    ");
	fprintf(stdout, "    short_index         : %d\n", file_id->short_index);
}

void 
print_KeyId(key_id)
	KeyId          *key_id;
{
	if (!key_id)
		fprintf(stdout, "KeyId                   : NULL\n");
	else {

		fprintf(stdout, "KeyId                   :\n");
		switch (key_id->key_level) {
		case FILE_MF:
			fprintf(stdout, "    key_level           : FILE_MF\n");
			break;
		case FILE_DF:
			fprintf(stdout, "    key_level           : FILE_DF\n");
			break;
		case FILE_CHILD:
			fprintf(stdout, "    key_level           : FILE_CHILD\n");
			break;
		case FILE_MASTER:
			fprintf(stdout, "    key_level           : FILE_MASTER\n");
			break;
		case FILE_CURRENT:
			fprintf(stdout, "    key_level           : FILE_CURRENT\n");
			break;
		case FILE_PARENT:
			fprintf(stdout, "    key_level           : FILE_PARENT\n");
			break;
		default:
			fprintf(stdout, "    key_level           : undefined\n");
			break;
		};

		fprintf(stdout, "    key_number          : %d\n", key_id->key_number);
	}
}
void 
print_FileCloseContext(file_close_context)
	FileCloseContext file_close_context;
{

	switch (file_close_context) {
	case CLOSE_CREATE:
		fprintf(stdout, "FileCloseContext        : CLOSE_CREATE\n");
		break;
	case CLOSE_SELECT:
		fprintf(stdout, "FileCloseContext        : CLOSE_SELECT\n");
		break;
	default:
		fprintf(stdout, "FileCloseContext        : undefined\n");
		break;
	};

}
void 
print_TransMode(trans_mode)
	TransMode       trans_mode;
{

	switch (trans_mode) {
	case TRANSP:
		fprintf(stdout, "TransMode               : TRANSP\n");
		break;
	case SECURE:
		fprintf(stdout, "TransMode               : SECURE\n");
		break;
	default:
		fprintf(stdout, "TransMode               : undefined\n");
		break;
	};

}
void 
print_ICC_SecMess(sec_mess)
	ICC_SecMess        *sec_mess;
{
	if (!sec_mess) {
		fprintf(stdout, "ICC_SecMess             : NULL\n");
	}
	else {
		fprintf(stdout, "ICC_SecMess             :\n");
		switch (sec_mess->command) {
		case ICC_SEC_NORMAL:
			fprintf(stdout, "    command             : ICC_SEC_NORMAL\n");
			break;
		case ICC_AUTHENTIC:
			fprintf(stdout, "    command             : ICC_AUTHENTIC\n");
			break;
		case ICC_CONCEALED:
			fprintf(stdout, "    command             : ICC_CONCEALED\n");
			break;
		case ICC_COMBINED:
			fprintf(stdout, "    command             : ICC_COMBINED\n");
			break;
		default:
			fprintf(stdout, "    command             : undefined\n");
			break;
		};

		switch (sec_mess->response) {
		case ICC_SEC_NORMAL:
			fprintf(stdout, "    response            : ICC_SEC_NORMAL\n");
			break;
		case ICC_AUTHENTIC:
			fprintf(stdout, "    response            : ICC_AUTHENTIC\n");
			break;
		case ICC_CONCEALED:
			fprintf(stdout, "    response            : ICC_CONCEALED\n");
			break;
		case ICC_COMBINED:
			fprintf(stdout, "    response            : ICC_COMBINED\n");
			break;
		default:
			fprintf(stdout, "    response            : undefined\n");
			break;
		};
	};

}
void 
print_SCT_SecMess(sec_mess)
	SCT_SecMess        *sec_mess;
{

	if (!sec_mess) {
		fprintf(stdout, "SCT_SecMess             : NULL\n");
	}
	else {
		fprintf(stdout, "SCT_SecMess             :\n");
		switch (sec_mess->command) {
		case SCT_SEC_NORMAL:
			fprintf(stdout, "    command             : SCT_SEC_NORMAL\n");
			break;
		case SCT_INTEGRITY:
			fprintf(stdout, "    command             : SCT_INTEGRITY\n");
			break;
		case SCT_CONCEALED:
			fprintf(stdout, "    command             : SCT_CONCEALED\n");
			break;
		case SCT_COMBINED:
			fprintf(stdout, "    command             : SCT_COMBINED\n");
			break;
		default:
			fprintf(stdout, "    command             : undefined\n");
			break;
		};

		switch (sec_mess->response) {
		case SCT_SEC_NORMAL:
			fprintf(stdout, "    response            : SCT_SEC_NORMAL\n");
			break;
		case SCT_INTEGRITY:
			fprintf(stdout, "    response            : SCT_INTEGRITY\n");
			break;
		case SCT_CONCEALED:
			fprintf(stdout, "    response            : SCT_CONCEALED\n");
			break;
		case SCT_COMBINED:
			fprintf(stdout, "    response            : SCT_COMBINED\n");
			break;
		default:
			fprintf(stdout, "    response            : undefined\n");
			break;
		};
	};

}

void 
print_FileInfoReq(info_req)
	FileInfoReq     info_req;
{

	switch (info_req) {
	case NONE_INFO:
		fprintf(stdout, "FileInfoReq             : NONE_INFO\n");
		break;
	case COMP_INFO:
		fprintf(stdout, "FileInfoReq             : COMP_INFO\n");
		break;
	default:
		fprintf(stdout, "FileInfoReq             : undefined\n");
		break;
	};


}

void 
print_AuthProcId(proc_id)
	AuthProcId     proc_id;
{

	switch (proc_id) {
	case IN_AUTH:
		fprintf(stdout, "AuthProcId              : IN_AUTH\n");
		break;
	case EX_AUTH:
		fprintf(stdout, "AuthProcId              : EX_AUTH\n");
		break;
	case MU_AUTH:
		fprintf(stdout, "AuthProcId              : MU_AUTH\n");
		break;
	case MU_AUTHWithINITIAL:
		fprintf(stdout, "AuthProcId              : MU_AUTHWithINITIAL\n");
		break;
	default:
		fprintf(stdout, "AuthProcId              : undefined\n");
		break;
	};


}
void 
print_AuthObjectId(object_id)
	AuthObjectId	object_id;
{

	switch (object_id) {
	case DTE_ICC:
		fprintf(stdout, "AuthObjectId            : DTE_ICC\n");
		break;
	case SCT_ICC:
		fprintf(stdout, "AuthObjectId            : SCT_ICC\n");
		break;
	default:
		fprintf(stdout, "AuthObjectId            : undefined\n");
		break;
	};


}

void 
print_KeyPurpose(key_purpose)
	KeyPurpose  key_purpose;
{
	switch (key_purpose) {
	case PURPOSE_SECCONC:
		fprintf(stdout, "KeyPurpose              : PURPOSE_SECCONC\n");
		break;
	case PURPOSE_SECAUTH:
		fprintf(stdout, "KeyPurpose              : PURPOSE_SECAUTH\n");
		break;
	case PURPOSE_AUTH:
		fprintf(stdout, "KeyPurpose              : PURPOSE_AUTH\n");
		break;
	default:
		fprintf(stdout, "KeyPurpose              : undefined\n");
		break;
	};


}

void 
print_KeyDevSel(key_dev_sel, cmd)
	KeyDevSel      *key_dev_sel;
	int		cmd;
{

	char	*aux_ptr;
	char    *proc="print_KeyDevSel";



	if (!key_dev_sel)
		fprintf(stdout, "KeyDevSel               : NULL\n");
	else {
		fprintf(stdout, "KeyDevSel               : \n");
		switch (cmd) {
		case S_GEN_DEV_KEY:
			if (key_dev_sel->key_algid) { 
				aux_ptr = aux_ObjId2Name(key_dev_sel->key_algid->objid);
				fprintf(stdout, "    key_algid           : %s\n", aux_ptr);
				free(aux_ptr);
			}
			else
				fprintf(stdout, "    key_algid           : NULL\n");
			print_KeyId(key_dev_sel->key_id);
			print_KeyPurpose(key_dev_sel->key_purpose);
			break;
		case S_INST_DEV_KEY:
		case S_DEL_DEV_KEY:
			fprintf(stdout, "    key_algid           : not evaluated\n");
			switch (key_dev_sel->key_status) {
			case DEV_OWN:
				fprintf(stdout, "    key_status          : DEV_OWN\n");
				fprintf(stdout, "    key_id              : not evaluated\n");
				print_KeyPurpose(key_dev_sel->key_purpose);
				break;
			case DEV_ANY:
				fprintf(stdout, "    key_status          : DEV_ANY\n");
				print_KeyId(key_dev_sel->key_id);
				fprintf(stdout, "    key_purpose         : not evaluated\n");
				break;
			default:
				fprintf(stdout, "    key_status          : undefined\n");
				break;
			}
			break;
		}


	}			/* key_dev_sel <> NULL */

}


void 
print_KeyIdList(key_dev_list)
	KeyId     *key_dev_list[];
{

	if (!key_dev_list)
		fprintf(stdout, "KeyIdList               : NULL\n");
	else {
		fprintf(stdout, "KeyIdList               : \n");
		if (key_dev_list[0]) {
			fprintf(stdout, "key_1                   : \n");
			print_KeyId(key_dev_list[0]);

			if (key_dev_list[1]) {
				fprintf(stdout, "key_2                   : \n");
				print_KeyId(key_dev_list[1]);

				if (key_dev_list[2]) {
					fprintf(stdout, "key_3                   : \n");
					print_KeyId(key_dev_list[2]);
				}
				else
					fprintf(stdout, "key_3                   : NULL\n");

			}
			else 
				fprintf(stdout, "key_2                   : NULL\n");
			
		}
		else
			fprintf(stdout, "key_1                   : NULL\n");


	}
}





void 
print_KeyAttrList(key_attr_list)
	KeyAttrList    *key_attr_list;
{
	if (!key_attr_list)
		fprintf(stdout, "KeyAttrList             : NULL\n");
	else {
		fprintf(stdout, "KeyAttrList             : \n");
		switch (key_attr_list->key_inst_mode) {
		case INST:
			fprintf(stdout, "    key_inst_mode       : INST\n");
			break;
		case REPL:
			fprintf(stdout, "    key_inst_mode       : REPL\n");
			break;
		default:
			fprintf(stdout, "    key_inst_mode       : undefined\n");
			break;
		}

		fprintf(stdout, "    key_attr            : \n");
		print_Boolean(  "       key_repl_flag    : ",
				key_attr_list->key_attr.key_repl_flag);
		print_Boolean(  "       key_chg_flag     : ",
				key_attr_list->key_attr.key_chg_flag);
		print_Boolean(  "       key_foreign_flag : ",
				key_attr_list->key_attr.key_foreign_flag);

		fprintf(stdout, "       key_purpose      : ");
		switch (key_attr_list->key_attr.key_purpose) {
		case PURPOSE_CIPHER:
			fprintf(stdout, "PURPOSE_CIPHER\n");
			break;
		case PURPOSE_AUTH:
			fprintf(stdout, "PURPOSE_AUTH\n");
			break;
		case PURPOSE_SECCONC:
			fprintf(stdout, "PURPOSE_SECCONC\n");
			break;
		case PURPOSE_SECAUTH:
			fprintf(stdout, "PURPOSE_SECAUTH\n");
			break;
		default:
			fprintf(stdout, " undefined\n");
			break;
		}



		switch (key_attr_list->key_attr.key_presentation) {
		case KEY_GLOBAL:
			fprintf(stdout, "       key_presenation  : KEY_GLOBAL\n");
			break;
		case KEY_LOCAL:
			fprintf(stdout, "       key_presenation  : KEY_LOCAL\n");
			break;
		default:
			fprintf(stdout, "       key_presenation  : value not defined\n");
			break;
		}



		fprintf(stdout, "       MAC_length       : %d\n",
			key_attr_list->key_attr.MAC_length);

		fprintf(stdout, "    key_fpc             : %d\n", key_attr_list->key_fpc);

		fprintf(stdout, "    key_status          : \n");
		print_Boolean(  "       PIN_check        : ",
				key_attr_list->key_status.PIN_check);

		print_Boolean(  "       key_state        : ",
				key_attr_list->key_status.key_state);



	}


}

void 
print_VerifyDataStruc(verify_data)
	VerifyDataStruc       *verify_data;
{
	if (!verify_data)
		fprintf(stdout, "VerifyDataStruc         : NULL\n");
	else {
		fprintf(stdout, "VerifyDataStruc         : \n");
		switch (verify_data->verify_data_type) {
		case PIN:
			fprintf(stdout, "    verify_data_type    : PIN\n");
			fprintf(stdout, "    pin_info            :\n");
			fprintf(stdout, "       min_len          : %d\n",
				verify_data->VerifyDataBody.pin_info.min_len);
			print_OctetString("       pin              : ",
				  	verify_data->VerifyDataBody.pin_info.pin);


			print_Boolean(    "       disengageable    : ",
					verify_data->VerifyDataBody.pin_info.disengageable);
			break;
		case PUK:
			fprintf(stdout, "    verify_data_type    : PUK\n");
			fprintf(stdout, "    puk_info            :\n");
			print_OctetString("       puk              : ",
				  	verify_data->VerifyDataBody.puk_info.puk);

			fprintf(stdout, "       PIN_key_id       :\n");


			if (!verify_data->VerifyDataBody.puk_info.pin_key_id)
				fprintf(stdout, "       PIN_key_id       : NULL\n");
			else {

				fprintf(stdout, "       PIN_key_id       : \n");
				fprintf(stdout, "           key_level    : ");
				switch (verify_data->VerifyDataBody.puk_info.pin_key_id->key_level) {
				case FILE_CURRENT:
					fprintf(stdout, "FILE_CURRENT\n");
					break;
				case FILE_PARENT:
					fprintf(stdout, "FILE_PARENT\n");
					break;
				case FILE_MASTER:
					fprintf(stdout, "FILE_MASTER\n");
					break;
				default:
					fprintf(stdout, "undefined\n");
					break;
				};

				fprintf(stdout, "           key_number   : %d\n",
					 	verify_data->VerifyDataBody.puk_info.pin_key_id->key_number);
			}

			break;
		default:
			fprintf(stdout, "    verify_data_type    : value not supported\n");
			break;
		}
	}
}



void print_KeySel (key_sel, flag)
	KeySel	*key_sel;
	Boolean flag; 			/* if flag = FALSE and algid == RSA,
					   then print key_id
					   else print key_bits
					*/

{

	char	*aux_ptr;
	char    *proc="print_KeySel";


	if (key_sel != NULL) {
		fprintf(stdout, "KeySel                  : \n");
		if (key_sel->key_algid) { 
			aux_ptr = aux_ObjId2Name(key_sel->key_algid->objid);
			fprintf(stdout, "    key_algid           : %s\n", aux_ptr);
			free(aux_ptr);
			if(aux_ObjId2AlgEnc(key_sel->key_algid->objid) == RSA) {
				if (flag == TRUE) {
					if (key_sel->key_bits == NULL)
						fprintf(stdout, "    key_bits            : NULL\n");
					else {
						fprintf(stdout, "    key_bits            : \n");
						if (key_sel->key_bits->part1.octets == NULL)
							fprintf(stdout, "    part1               : NULL\n");
						else {
							fprintf(stdout, "    part1               : \n");
							fprintf(stdout, "        noctets         : %d\n",
					  		key_sel->key_bits->part1.noctets);
							fprintf(stdout, "        octets          : \n");
							aux_fxdump(stdout, key_sel->key_bits->part1.octets,
							key_sel->key_bits->part1.noctets, 0);
						}
						if (key_sel->key_bits->part2.octets == NULL)
							fprintf(stdout, "    part2               : NULL\n");
						else {
							fprintf(stdout, "    part2               : \n");
							fprintf(stdout, "        noctets         : %d\n",
					  		key_sel->key_bits->part2.noctets);
							fprintf(stdout, "        octets          : \n");
							aux_fxdump(stdout, key_sel->key_bits->part2.octets,
							key_sel->key_bits->part2.noctets, 0);
						}
					}
				}
				else 
					print_KeyId(key_sel->key_id);
			}
			else 
				print_KeyId(key_sel->key_id);
		}
		else
			fprintf(stdout, "    key_algid           : NULL\n");
	} 
	else 
		fprintf(stdout, "KeySel                  : NULL\n");

}

void print_Signature (signature, flag)
	Signature *signature;
	int	  flag;		/* 0  = print signAI and signature */
				/* 1  = print only signAI	   */
				/* 2  = print only signature       */

{

	char	*aux_ptr;
	char    *proc="print_Signature";


	if (signature) {
		fprintf(stdout, "Signature              : \n");
		if (flag < 2) {
			if(!signature->signAI) 
				fprintf(stdout, "    signAI              : NULL\n");
			else {
				aux_ptr = aux_ObjId2Name(signature->signAI->objid);
				fprintf(stdout, "    signAI              : %s\n", aux_ptr);
				free(aux_ptr);
			}
		}
		else
			fprintf(stdout, "    signAI              : is not evaluated\n");
		if (flag != 1) {
			if (signature->signature.bits) {
				fprintf(stdout, "    signature           : \n");
				fprintf(stdout, "       nbits            : %d\n",
					signature->signature.nbits);
				fprintf(stdout, "        bits            : \n");
				aux_fxdump(stdout, signature->signature.bits,
			   	   	   signature->signature.nbits / 8, 0);
			}
			else {

				fprintf(stdout, "    signature           : \n");
				fprintf(stdout, "        bits            : NULL\n");
			}
		}
		else
				fprintf(stdout, "    signature           : is not evaluated\n");
	}
	else
		fprintf(stdout, "signature              : NULL\n");


}

void print_HashPar(hash_par)
	HashPar	*hash_par;
{
	if (hash_par) {
		fprintf(stdout, "HashPar  (modulus)      : \n");
		aux_fxdump(stdout, hash_par->sqmodn_par.part1.octets, hash_par->sqmodn_par.part1.noctets, 0);
	}
	else
		fprintf(stdout, "HashPar  (modulus)      : NULL\n");
}


void print_KeyInfo (key)
	KeyInfo	*key;

{

	char	*aux_ptr;
	char    *proc="print_KeyInfo";


	if (key != KEYINFONULL) {
		fprintf(stdout, "KeyInfo                : \n");

		if ((!key->subjectAI) ||
	            (!key->subjectAI->objid))
 			fprintf(stdout, "    subjectAI           : NULL\n");
		else {
			aux_ptr = aux_ObjId2Name(key->subjectAI->objid);
			fprintf(stdout, "    subjectAI           : %s\n", aux_ptr);
			free(aux_ptr);
		}

		fprintf(stdout, "    subjectkey.nbits    : %d\n",
			key->subjectkey.nbits);
		
		if (key->subjectkey.bits == NULL)
			fprintf(stdout, "    subjectkey.bits     : NULL\n");
		else {
			fprintf(stdout, "    subjectkey.bits     : \n");
 			aux_fxdump(stdout, key->subjectkey.bits,
				   key->subjectkey.nbits / 8, 0);
		}
	} 
	else fprintf(stdout, "KeyInfo	        : NULL\n");

}

void 
print_KeyLevel(key_level)
	KeyLevel          key_level;
{

		switch (key_level) {
		case FILE_MF:
			fprintf(stdout, "key_level               : FILE_MF\n");
			break;
		case FILE_DF:
			fprintf(stdout, "key_level               : FILE_DF\n");
			break;
		case FILE_CHILD:
			fprintf(stdout, "key_level               : FILE_CHILD\n");
			break;
		case FILE_MASTER:
			fprintf(stdout, "key_level               : FILE_MASTER\n");
			break;
		case FILE_CURRENT:
			fprintf(stdout, "key_level               : FILE_CURRENT\n");
			break;
		case FILE_PARENT:
			fprintf(stdout, "key_level               : FILE_PARENT\n");
			break;
		case INITKEY:
			fprintf(stdout, "key_level               : INITKEY\n");
			break;
		default:
			fprintf(stdout, "key_level               : undefined\n");
			break;
		};

}

void 
print_KeyDevList(key_dev_list, flag)
	KeyDevList      *key_dev_list;
	Boolean		flag;		/* FALSE: print only key_status;
					 * TRUE : print all elements
				 	 */
{
	int     i;

	char    *proc="print_KeyDevList";



	if (!key_dev_list)
		fprintf(stdout, "KeyDevList              : NULL\n");
	else {
		fprintf(stdout, "KeyDevList               : \n");
		switch (key_dev_list->key_status) {
		case DEV_OWN:
			fprintf(stdout, "key_status              : DEV_OWN\n");
			if (flag == TRUE) {
				fprintf(stdout, "purpose_list            : \n");
				i = 0;
				while (*(key_dev_list->list_ref.purpose_list + i) != '\0') {
					print_KeyPurpose(*(key_dev_list->list_ref.purpose_list + i));
					i++;
				}
			}
			break;
		case DEV_ANY:
			fprintf(stdout, "key_status              : DEV_ANY\n");
			if (flag == TRUE) {
				fprintf(stdout, "keyid_list              : \n");
				i = 0;
				while (*(key_dev_list->list_ref.keyid_list + i) != KEYIDNULL) {
					print_KeyId(*(key_dev_list->list_ref.keyid_list + i));
					i++;
				}
			}
			break;
		default:
			fprintf(stdout, "    key_status          : undefined\n");
			break;
		}
	}

}


void 
print_EncryptedKey(encrypted_key)
	EncryptedKey      *encrypted_key;
{
	char    *aux_ptr;
	char    *proc="print_EncryptedKey";

	if (encrypted_key) {

		fprintf(stdout, "encrypted_key           : \n");
		if (encrypted_key->encryptionAI) {
			aux_ptr = aux_ObjId2Name(encrypted_key->encryptionAI->objid);
			fprintf(stdout, "   encryptionAI         : %s\n", aux_ptr);
			free(aux_ptr);
		}
		else
			fprintf(stdout, "   encryptionAI         : NULL\n");

		if (encrypted_key->subjectAI) {
			aux_ptr = aux_ObjId2Name(encrypted_key->subjectAI->objid);
			fprintf(stdout, "   subjectAI            : %s\n", aux_ptr);
			free(aux_ptr);
		}
		else
			fprintf(stdout, "   subjectAI            : NULL\n");

		fprintf(stdout, "    subjectkey          : \n");
		fprintf(stdout, "      nbits             : %d\n", encrypted_key->subjectkey.nbits);
		if (encrypted_key->subjectkey.bits) {
			fprintf(stdout, "      bits              : \n");
			aux_fxdump(stdout, encrypted_key->subjectkey.bits,
		   	   	encrypted_key->subjectkey.nbits / 8, 0);
		}
		else
			fprintf(stdout, "      bits              : NULL\n");
	}
	else
		fprintf(stdout, "encrypted_key           : NULL\n");

}

void print_VerificationKey (key_sel)
	KeySel	*key_sel;

{

	char	*aux_ptr;
	char    *proc="print_Verification_key";


	if (key_sel != NULL) {
		fprintf(stdout, "KeySel                  : \n");
			fprintf(stdout, "    key_algid           : is not evaluated\n");

		if (key_sel->key_bits == NULL)
			fprintf(stdout, "    key_bits            : NULL\n");
		else {
			fprintf(stdout, "    key_bits            : \n");
			fprintf(stdout, "    part1               : \n");
			fprintf(stdout, "        noctets         : %d\n",
				key_sel->key_bits->part1.noctets);
			if (key_sel->key_bits->part1.octets == NULL)
				fprintf(stdout, "        octets          : NULL\n");
			else {
				fprintf(stdout, "        octets          : \n");
				aux_fxdump(stdout, key_sel->key_bits->part1.octets,
				key_sel->key_bits->part1.noctets, 0);
			}
			fprintf(stdout, "    part2               : \n");
			fprintf(stdout, "        noctets         : %d\n",
				key_sel->key_bits->part2.noctets);
			if (key_sel->key_bits->part2.octets == NULL)
				fprintf(stdout, "        octets          : NULL\n");
			else {
				fprintf(stdout, "        octets          : \n");
				aux_fxdump(stdout, key_sel->key_bits->part2.octets,
				key_sel->key_bits->part2.noctets, 0);
			}
		}
	} 
	else 
		fprintf(stdout, "KeySel                  : NULL\n");

}

void print_SessionKey (session_key)
	SessionKey	*session_key;

{

	char	*aux_ptr;
	char    *proc="print_SessionKey";


	if (session_key != NULL) {
		fprintf(stdout, "SessionKey              : \n");
			if (session_key->integrity_key) { 
				aux_ptr = aux_ObjId2Name(session_key->integrity_key->objid);
				fprintf(stdout, "    integrity_key       : %s\n",aux_ptr);
				free(aux_ptr);
			}
			else
				fprintf(stdout, "    integrity_key       : NULL\n");

			if (session_key->concealed_key) { 
				aux_ptr = aux_ObjId2Name(session_key->concealed_key->objid);
				fprintf(stdout, "    concealed_key       : %s\n",aux_ptr);
				free(aux_ptr);
			}
			else
				fprintf(stdout, "    concealed_key       : NULL\n");
	} 
	else 
		fprintf(stdout, "SessionKey              : NULL\n");

}

void 
print_ComMode(mode)
	ComMode       mode;
{

	switch (mode) {
	case COM_OPEN:
		fprintf(stdout, "ComMode                 : COM_OPEN\n");
		break;
	case COM_CLOSE:
		fprintf(stdout, "ComMode                 : COM_CLOSE\n");
		break;
	default:
		fprintf(stdout, "ComMode                 : undefined\n");
		break;
	};

}

void 
print_SCTPort(elem)
	SCTPort		*elem;
{
	if (elem != NULL) {
		fprintf(stdout, "SCTPort                 : \n");
		fprintf(stdout, "  BWT                   : %x\n", elem->bwt);
		fprintf(stdout, "  CWT                   : %x\n", elem->cwt);
		fprintf(stdout, "  BAUD                  : %x\n", elem->baud);
		fprintf(stdout, "  DATABITS              : %x\n", elem->databits);
		fprintf(stdout, "  STOPBITS              : %x\n", elem->stopbits);
		fprintf(stdout, "  PARITY                : %x\n", elem->parity);
		fprintf(stdout, "  DATAFORMAT            : %x\n", elem->dataformat);
		fprintf(stdout, "  TPDUSIZE              : %x\n", elem->tpdusize);
		fprintf(stdout, "  APDUSIZE              : %x\n", elem->apdusize);
		fprintf(stdout, "  EDC                   : %x\n", elem->edc);
		fprintf(stdout, "  PROTTYPE              : %x\n", elem->protocoltype);
		fprintf(stdout, "  CHAINING              : %x\n", elem->chaining);
		fprintf(stdout, "  ICC_REQUEST           : %x\n\n", elem->icc_request);
		fprintf(stdout, "  FIRST_PAD_BYTE        : %x\n\n", elem->first_pad_byte);
		fprintf(stdout, "  NEXT_PAD_BYTE         : %x\n\n", elem->next_pad_byte);
		fprintf(stdout, "  ALWAYS                : %x\n\n", elem->always);
	}
	else
		fprintf(stdout, "SCTPort                 : NULL\n");
}





#endif



/*-------------------------------------------------------------*/
/* E N D   O F	 P A C K A G E	     PRINT      	       */
/*-------------------------------------------------------------*/
