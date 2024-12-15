/*
 *  STARMOD Release 1.1 (GMD)
 */
/*---------------------------------------------------------------------------+-----*/
/*							                     | GMD */
/*   SYSTEM   STARMOD  -  Version 1.1		                             +-----*/
/*							                           */
/*---------------------------------------------------------------------------------*/
/*							                           */
/*    PACKAGE	SCAIF                                   VERSION 1.1	           */
/*					                       DATE March  1993    */
/*					                         BY Levona Eckstein*/
/*			       				                           */
/*    FILENAME     					                           */
/*	sca_data.c                          		         		   */
/*							                           */
/*    DESCRIPTION	   				                           */
/*      This modul provides all functions for file handling and data access        */
/*      on the smartcard  of the smartcard application interface                   */
/*							                           */
/*    EXPORT		    DESCRIPTION 		                           */
/*      sca_read_binary()     Read data from EF on the smartcard                   */
/*										   */
/*      sca_read_record()     Read record from EF on the smartcard                 */
/*										   */
/*      sca_write_binary()    Write data in EF on the smartcard			   */
/*										   */
/*      sca_write_record()    Write record in EF on the smartcard		   */
/*    									           */
/*      sca_delete_record()   Delete record in EF on the smartcard		   */
/*										   */
/*      sca_lock_key()        Lock key on smartcard				   */
/*   										   */
/*      sca_unlock_key()      Unlock key on the smartcard			   */
/*										   */
/*							                           */
/*                                                                                 */
/*							                           */
/*                                                                                 */
/*    IMPORT		    DESCRIPTION 		                           */
/*	all functions and variables which are described in stamod.h                */
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
/* PROC  sca_read_binary          VERSION   1.1	    	       */
/*				     DATE   March 1993	       */
/*			      	       BY   Levona Eckstein    */
/*							       */
/* DESCRIPTION						       */
/*  Read data from elementary file on the smartcard.           */
/*  In Version 1.1 this function will only be allowed on       */
/*  TRANSPARENT files					       */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   short_index	       short index		       */
/*			       				       */
/*   offset                    Offset of the data to be read   */
/*			                         	       */
/*   noctets                   Number of octets to be read     */
/*			                 		       */
/*   sec_mess		       security modes                  */
/* OUT							       */
/*							       */
/*   in_data                   Buffer where the returned data  */
/*			       will be stored		       */
/*			       in_data->octets will be allocated*/
/*			       by the called program           */
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*				 EWRONG_OFFSET		       */
/*                               EMEMAVAIL                     */
/* NOTE:						       */
/* In case of error already                                    */
/* received data are returned				       */
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
/*                              sw1/sw2 from ICC  response     */
/*							       */
/*   malloc macro in MF_check.h				       */ 
/*   free macro in MF_check.h				       */ 
/*-------------------------------------------------------------*/
int 
sca_read_binary(sct_id, short_index, offset, noctets, in_data, sec_mess)
	int             sct_id;
	unsigned int	short_index;
	unsigned int    offset;
	unsigned int    noctets;
	OctetString     *in_data;
	ICC_SecMess      *sec_mess;
{
	int             i, rc, act_offset, act_len, rec_len, max_len, act_pos;
	DataSel		data_sel;
   	int  		end_flag   = 0;
   	int  		error_flag = 0;
   	Buffer		*p_head, *p_last, *dp_tail;
	unsigned int	offset_high, offset_new, offset_p1;
	Boolean		ef_id;
	char		*proc="sca_read_binary";
	
	sca_errno = M_NOERR;
	sca_errmsg = NULL;


#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_read_binary *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id                  : %d\n", sct_id);
	fprintf(stdout, "short_index             : %d\n",short_index);
	fprintf(stdout, "offset                  : %d\n", offset);
	fprintf(stdout, "noctets                 : %d\n", noctets);
	print_ICC_SecMess(sec_mess);
#endif



/*
 *      the correctness of the icc command specific parameters are checked 
 * 	in the function icc_create
 */

/*****************************************************************************/
/*
 *      check parameter and initialize return parameter
 */
	if (in_data == NULLOCTETSTRING) {
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		return (-1);
	}

	if (short_index != 0) {
		ef_id = TRUE;
		offset_high = (0x80 | short_index);
	}
	else
		ef_id = FALSE;


	in_data->noctets	= 0;
	in_data->octets		= NULL;


/*****************************************************************************/
/*
 *      call STACheckSCT_ICC
 */

	if (STACheckSCT_ICC(sct_id, TRUE,proc))
		return (-1);


/*****************************************************************************/
/*
 *      create ICC command READB
 */
	if (STACreateHead(ICC_READB, sec_mess))
		return (-1);


	/* set parameters			  */

	data_sel.data_struc = TRANSPARENT;

	PAR_DATASEL		= &data_sel;

	/*
	 *  compute max_len		       
	 */
	switch (sec_mess->response) {
	case ICC_SEC_NORMAL:
		max_len = RLEN_DATA_NORMAL;
		break;
	case ICC_AUTHENTIC:
		max_len = RLEN_DATA_AUTH;
		break;
	case ICC_CONCEALED:
		max_len = RLEN_DATA_CONC;
		break;
	case ICC_COMBINED:
		max_len = RLEN_DATA_COMB;
		break;
	}

	rec_len	= noctets;		
	act_pos = 0;
	
	if ((noctets == 0) ||
	    (noctets > max_len)) 
		act_len = max_len;
	else
		act_len = noctets;

	act_offset = offset;
	p_head	   = BUFFERNULL;

        /*      
         * allocate buffer for incoming data
	 * in case of noctets  = 0 => the input buffer is a chained list          
         */      
        if (noctets == 0) {
           	/*
           	 * create first element                  
           	 */
          
          	if ((p_head = (Buffer *) malloc(sizeof(Buffer))) == BUFFERNULL) {
             		sca_errno = EMEMAVAIL;
             		sca_errmsg = sca_errlist[sca_errno].msg;
			p_head = BUFFERNULL;
             		in_data->noctets = 0;
			return(-1); 
           	}
           	p_head->next = BUFFERNULL;
           	p_last = p_head;
		p_last->data.octets = NULL;
		p_last->data.noctets= 0;
        }
	else {
    		if ((in_data->octets = (char *) malloc(noctets)) == NULL) {
          		sca_errno = EMEMAVAIL;
          		sca_errmsg = sca_errlist[sca_errno].msg;
          		return(-1);
    		}
	}
	




   	while (!end_flag)
   	{   

         	/*
         	 * call STACreateTrans			  
         	 */
		if (ef_id == TRUE) {
			offset_new = (offset_high * 256) + act_offset;
			offset_p1  = offset_new >> 8;
			if (offset_p1 != offset_high) {
             			sca_errno = EWRONG_OFFSET;
             			sca_errmsg = sca_errlist[sca_errno].msg;
				error_flag = 1;
				if (noctets == 0)
					goto create_indata;
				else {
					if (in_data->noctets == 0) {
						free (in_data->octets);
						in_data->octets = NULL;
					}

					return(-1);
				} 
			}

			data_sel.data_ref.string_sel 	= offset_new;
		}
		 
		else
			data_sel.data_ref.string_sel 	= act_offset;

		PAR_LRDDATA			= act_len;
         	rc = STACreateTrans(sct_id,TRUE);
         
         	if (rc == -1) {
           		switch(sca_errno) {
             		case EEND_OF_FILE:
				/* RC = 62 82 */
				if (noctets != 0)
					error_flag = 1;
                     		break;
             		case EWRONG_OFFSET:
				/* RC = 6B 00 */
				if (noctets != 0)
					error_flag = 1;
                     		break;

             		case EDATA_INCON: 
                        	/* Data inconsistency */
				/* RC = 62 81         */
                        	error_flag = 1;
                     		break;
             		default:
                     		/*
                     		 * error while reading file              
                     		 */
				Gsct_response.noctets = 1;
                     		aux_free2_OctetString(&Gsct_response);
				error_flag = 1;
				if (noctets == 0)
					goto create_indata;
				else {
					if (in_data->noctets == 0) {
						free (in_data->octets);
						in_data->octets = NULL;
					}

					return(-1);
				} 
				break;
            		}
			end_flag = 1;
         	};

		if (!end_flag) {
			act_offset += act_len;
			rec_len    -= act_len;
			if ((noctets != 0) &&
			    (rec_len == 0))
				end_flag = 1;
			else
				if ((noctets == 0) ||
	    			    (rec_len > max_len)) 
					act_len = max_len;
				else
					act_len = rec_len;
		}



        	/*
         	 * set data.octets = Gsct_response.octets      
         	 */
		if (noctets == 0) {
			in_data->noctets += Gsct_response.noctets;
         		p_last->data.octets  = Gsct_response.octets;
			p_last->data.noctets = Gsct_response.noctets;
			Gsct_response.octets = NULL;
			Gsct_response.noctets= 0; 
 
			if (!end_flag) {
           			/*
           			 * create next Buffer element            
           			 */
  
           			if ((p_last->next = (Buffer *) malloc(sizeof(Buffer))) == BUFFERNULL) {
             				sca_errno = EMEMAVAIL;
             				sca_errmsg = sca_errlist[sca_errno].msg;
					p_last->next = BUFFERNULL;
             				error_flag = 1;
					goto create_indata; 
           			}
           			p_last = p_last->next;
				p_last->data.octets = NULL;
				p_last->data.noctets= 0;
           			p_last->next = BUFFERNULL;
			}
		}
		else {
		   	in_data->noctets += Gsct_response.noctets;
			for (i=0; i<Gsct_response.noctets; i++) 
           			in_data->octets[act_pos++] = Gsct_response.octets[i];
			Gsct_response.noctets = 1;
			aux_free2_OctetString(&Gsct_response);
		}
					    




 
   	}



create_indata:
	if (noctets == 0) {
		if (in_data->noctets != 0) {

			/* data already received */

    			if ((in_data->octets = (char *) malloc(in_data->noctets)) == NULL) {
          			sca_errno = EMEMAVAIL;
          			sca_errmsg = sca_errlist[sca_errno].msg;
    				dp_tail = p_head;

    				while(dp_tail != BUFFERNULL) {
     					p_head = p_head->next;
        				free(dp_tail->data.octets);
        				free(dp_tail);

    					dp_tail = p_head;
	
    				}
          			return(-1);
    			}
	
    			dp_tail = p_head;

    			while(dp_tail != BUFFERNULL) {
				for (i=0; i<dp_tail->data.noctets; i++) 
           				in_data->octets[act_pos++] = dp_tail->data.octets[i];
     				p_head = p_head->next;
        			free(dp_tail->data.octets);
        			free(dp_tail);

    				dp_tail = p_head;
	
    			}
		}
		else {
			/* no data received => first call */
			if (p_last->data.octets != NULL)
				free(p_last->data.octets);
			free(p_last);
		} 
	}
	
#ifdef TEST
	fprintf(stdout, "TRACE of the output parameters : \n");
	fprintf(stdout, "in_data                 : \n");
	fprintf(stdout, "    noctets             : %d\n", in_data->noctets);
	fprintf(stdout, "    octets              : \n");
	aux_fxdump(stdout, in_data->octets, in_data->noctets, 0);
	fprintf(stdout, "\n***** Normal end of   sca_read_binary *****\n\n");
#endif
	if (error_flag)
		return (-1);
	else
		return (0);




}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_read_binary	       */
/*-------------------------------------------------------------*/


/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_read_record         VERSION   1.1	    	       */
/*				     DATE   March 1993	       */
/*			      	       BY   Levona Eckstein    */
/*							       */
/* DESCRIPTION						       */
/*  Read record from EF on the smartcard.        	       */
/*  This function will only be allowed on LIN FIX, LIN VAR and */
/*  CYCLIC EFs						       */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   short_index	       short index		       */
/*							       */
/*   data_struc		       data structure		       */
/*			       (FIX, VAR, CYCLIC)	       */
/*							       */			 
/*   rec_id                    Record identifier               */
/*							       */
/*   noctets                   number of data which shall be   */
/*			       read			       */	
/*							       */
/*   sec_mess		       security modes                  */
/*							       */
/*							       */
/* OUT							       */
/*							       */
/*   in_data                   Buffer where the returned data  */
/*			       will be stored		       */
/*			       in_data->octets will be allocated*/
/*			       by the called program           */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       record  read                    */
/*  -1			       error			       */
/*				 EPARINC 		       */
/*				 EMEMAVAIL		       */
/* NOTE:						       */
/* In case of error = EEND_OF_FILE or EDATA_INCON already      */
/* received data are returned				       */
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
/*                               EPARINC  		       */
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
/*-------------------------------------------------------------*/
int 
sca_read_record(sct_id, short_index, data_struc, rec_id, noctets, in_data, sec_mess)
	int             sct_id;
	unsigned int	short_index;
	DataStruc       data_struc;
	unsigned int	rec_id;
	unsigned int	noctets;
	OctetString	*in_data;
	ICC_SecMess      *sec_mess;
{

	int             rc, i;
	DataSel		data_sel;
	char		*proc="sca_read_record";

	sca_errno = M_NOERR;
	sca_errmsg = NULL;


#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_read_record *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id                  : %d\n", sct_id);
	fprintf(stdout, "short_index             : %d\n", short_index);
	print_DataStruc(data_struc);
	fprintf(stdout, "rec_id                  : %d\n", rec_id);
	fprintf(stdout, "noctets                 : %d\n", noctets);
	print_ICC_SecMess(sec_mess);
#endif


/*
 *      the correctness of the icc command specific parameters are checked 
 * 	in the function icc_create
 */

/*****************************************************************************/
/*
 *      initialize return parameter
 */
	if (in_data == NULLOCTETSTRING) {
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		return (-1);
	}

	in_data->noctets	= 0;
	in_data->octets		= NULL;





/*****************************************************************************/
/*
 *      call STACheckSCT_ICC
 */

	if (STACheckSCT_ICC(sct_id, TRUE, proc))
		return (-1);


/*****************************************************************************/
/*
 *      create ICC command READR
 */
	if (STACreateHead(ICC_READR, sec_mess))
		return (-1);


	/* set parameters			  */
	data_sel.data_struc = data_struc;
	if (data_struc != CYCLIC)
		data_sel.data_ref.record_sel.record_id = rec_id;
	else
		data_sel.data_ref.element_sel.element_ref = rec_id;


	PAR_DATASEL		= &data_sel;
	PAR_SHORT_INDEX		= short_index;
	PAR_LRDDATA		= noctets;



/*****************************************************************************/
/*
 *      call STACreateTrans
 */
	rc = STACreateTrans(sct_id, TRUE);

         if (rc == -1) {
           	switch(sca_errno) {
             	case EEND_OF_FILE:
			/* RC = 62 82 */
			goto create_indata;
			break;

             	case EDATA_INCON: 
                        /* Data inconsistency */
			/* RC = 62 81         */
			goto create_indata;
			break;
             	default:
                     	/*
                     	 * error while reading record => no data received              
                     	 */
			Gsct_response.noctets = 1;
                     	aux_free2_OctetString(&Gsct_response);
			return(-1);
			break;
            	}
         };
	


create_indata:

/*****************************************************************************/
/*
 *      allocate in_data.octets
 */
	ALLOC_CHAR(in_data->octets, Gsct_response.noctets);

	for (i = 0; i < Gsct_response.noctets; i++)
		*(in_data->octets + i) = Gsct_response.octets[i];

	in_data->noctets = Gsct_response.noctets;

	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);




#ifdef TEST
	fprintf(stdout, "TRACE of the output parameters : \n");
	fprintf(stdout, "in_data                 : \n");
	fprintf(stdout, "    noctets             : %d\n", in_data->noctets);
	fprintf(stdout, "    octets              : \n");
	aux_fxdump(stdout, in_data->octets, in_data->noctets, 0);
	fprintf(stdout, "\n***** Normal end of   sca_read_record *****\n\n");
#endif

	return(0);


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
/* E N D   O F	 P R O C E D U R E	sca_read_record	       */
/*-------------------------------------------------------------*/


/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_write_binary        VERSION   1.1	    	       */
/*				     DATE   March 1993	       */
/*			      	       BY   Levona Eckstein    */
/*							       */
/* DESCRIPTION						       */
/*  Write data in elementary file on the smartcard.            */
/*  In the current version this function is only allowed on    */
/*  a transparent file					       */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   short_index	       short index		       */
/*						  	       */
/*   offset                    Offset where the data shall     */
/*			       be written		       */
/*   out_data                  Data to be written              */
/*			                 		       */
/*   sec_mess		       security modes                  */
/* OUT							       */
/*   noctets		       number of actual written data   */	
/*							       */
/*							       */
/* RETURN		     DESCRIPTION	      	       */
/*   0	         	       o.k			       */
/*  -1			       error			       */
/*			         EPARINC 		       */
/*				 EWRONG_OFFSET		       */
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
/*-------------------------------------------------------------*/
int 
sca_write_binary(sct_id, short_index, offset,out_data, sec_mess, noctets)
	int             sct_id;
	unsigned int    short_index;
	unsigned int	offset;
	OctetString     *out_data;
	ICC_SecMess      *sec_mess;
	unsigned int	*noctets;
{
	int             i, act_offset, trans_len, data_len, rc, max_len;
	DataSel		data_sel;
	OctetString	data_octet;
	char		*ptr;
	unsigned int	offset_high, offset_new, offset_p1;
	Boolean		ef_id;
	char		*proc="sca_write_binary";

	sca_errno = M_NOERR;
	sca_errmsg = NULL;


#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_write_binary *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id                  : %d\n", sct_id);
	fprintf(stdout, "short_index             : %d\n",short_index);
	fprintf(stdout, "offset                  : %d\n", offset);
	fprintf(stdout, "out_data                : \n");
	fprintf(stdout, "    noctets             : %d\n", out_data->noctets);
	fprintf(stdout, "    octets              : \n");
	aux_fxdump(stdout, out_data->octets, out_data->noctets, 0);
	print_ICC_SecMess(sec_mess);
#endif

/*****************************************************************************/
/*
 *      check parameter
 */
	if ((out_data == NULLOCTETSTRING) ||
	    (out_data->noctets == 0) ||
            (out_data->octets == NULL)) {
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		return (-1);
	}

	if (short_index != 0) {
		ef_id = TRUE;
		offset_high = (0x80 | short_index);
	}
	else
		ef_id = FALSE;



/*****************************************************************************/
/*
 *      call STACheckSCT_ICC
 */

	if (STACheckSCT_ICC(sct_id, TRUE, proc))
		return (-1);


/*****************************************************************************/
/*
 *      compute maximal datalength
 */

	switch (sec_mess->command) {
	case ICC_SEC_NORMAL:
		max_len = SLEN_DATA_NORMAL;
		break;
	case ICC_AUTHENTIC:
		max_len = SLEN_DATA_AUTH;
		break;
	case ICC_CONCEALED:
		max_len = SLEN_DATA_CONC;
		break;
	case ICC_COMBINED:
		max_len = SLEN_DATA_COMB;
		break;
	}



	act_offset	= offset;
    	ptr 		= out_data->octets;
    	data_len	= out_data->noctets;
	*noctets	= 0;

    	if ((data_len / max_len) == 0)
       		trans_len = data_len % max_len;
    	else
       		trans_len = max_len;
	
/*****************************************************************************/
/*
 *      create ICC command WRITEF
 */
	if (STACreateHead(ICC_WRITEB, sec_mess))
		return (-1);


	/* set parameters			  */
	data_sel.data_struc	= TRANSPARENT;
	PAR_DATASEL		= &data_sel;
	PAR_DATA		= &data_octet;

    	while (data_len != 0) {
      	 	data_len = data_len - trans_len;
		if (ef_id == TRUE) {
			offset_new = (offset_high * 256) + act_offset;
			offset_p1  = offset_new >> 8;
			if (offset_p1 != offset_high) {
             			sca_errno = EWRONG_OFFSET;
             			sca_errmsg = sca_errlist[sca_errno].msg;
				return(-1);
			}

			data_sel.data_ref.string_sel 	= offset_new;
		}
		 
		else
			data_sel.data_ref.string_sel 	= act_offset;


       		data_octet.noctets 		= trans_len;
       		data_octet.octets		= ptr;

		/*
		 * call STACreateTrans			  
		 */
		rc = STACreateTrans(sct_id, TRUE);
		/*
		 * release response storage       		
		 */
		Gsct_response.noctets = 1;
		aux_free2_OctetString(&Gsct_response);

       		if (rc < 0) {
         		/*
         		 * Error-Handling                   
         		 */
           		switch(sca_errno) {
             		case EDATA_INCON: 
			case EEEPROM_KFPC:
			case ESYSTEM_ERR:
                        	/* data may be written */
                        	*noctets += trans_len;
	         		return(-1);
                     		break;
			default:
				return(-1);
			}
		

       		}
       		else {
         		act_offset += trans_len;
         		ptr += trans_len;
			*noctets += trans_len;

         		if ((data_len / max_len) == 0)
           			trans_len = data_len % max_len;
         		else
           			trans_len = max_len;

       		}; /* end else */
     	}; /* end while */



#ifdef TEST
	fprintf(stdout, "TRACE of the output parameters : \n");
	fprintf(stdout, "noctets                 : %d\n",*noctets);
	fprintf(stdout, "\n***** Normal end of   sca_write_binary *****\n\n");
#endif
	return (0);



}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_write_binary       */
/*-------------------------------------------------------------*/


/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_write_record         VERSION   1.1	    	       */
/*				     DATE   March 1993	       */
/*			      	       BY   Levona Eckstein    */
/*							       */
/* DESCRIPTION						       */
/*  Write record in an EF on the smartcard.                    */
/*  This function will only be allowed on LIN FIX, LIN VAR and */
/*  CYCLIC EFs						       */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   short_index	       short index		       */
/*							       */
/*   data_struc		       data structure		       */
/*			       (FIX, VAR, CYCLIC)	       */
/*							       */			 
/*   rec_id                    Record identifier               */
/*							       */
/*   write_mode                UPDATE / APPEND                 */
/*							       */
/*   out_data                  Data to be written              */
/*			                 		       */
/*   sec_mess		       security modes                  */
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
/*-------------------------------------------------------------*/
int 
sca_write_record(sct_id, short_index, data_struc, rec_id, write_mode, out_data, sec_mess)
	int             sct_id;
	unsigned int	short_index;
	DataStruc	data_struc;
	unsigned int	rec_id;
	WriteMode	write_mode;
	OctetString     *out_data;
	ICC_SecMess      *sec_mess;
{
	DataSel		data_sel;
	char		*proc="sca_write_record";

	sca_errno = M_NOERR;
	sca_errmsg = NULL;


#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_write_record *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id                  : %d\n", sct_id);
	fprintf(stdout, "short_index             : %d\n", short_index);
	fprintf(stdout, "rec_id                  : %d\n", rec_id);
	print_WriteMode(write_mode);
	fprintf(stdout, "out_data                : \n");
	fprintf(stdout, "    noctets             : %d\n", out_data->noctets);
	fprintf(stdout, "    octets              : \n");
	aux_fxdump(stdout, out_data->octets, out_data->noctets, 0);
	print_ICC_SecMess(sec_mess);
#endif

/*****************************************************************************/
/*
 *      check parameter
 */


	if ((out_data == NULLOCTETSTRING) ||
	    (out_data->noctets == 0) ||
            (out_data->octets == NULL)) {
		sca_errno = EPARINC;
		sca_errmsg = sca_errlist[sca_errno].msg;
		return (-1);
	}



/*****************************************************************************/
/*
 *      call STACheckSCT_ICC
 */

	if (STACheckSCT_ICC(sct_id, TRUE, proc))
		return (-1);


/*****************************************************************************/
/*
 *      create ICC command WRITEF
 */
	if (STACreateHead(ICC_WRITER, sec_mess))
		return (-1);


	/* set parameters			  */
	data_sel.data_struc	=	data_struc;
	if (data_struc != CYCLIC) {
		data_sel.data_ref.record_sel.record_id	= rec_id;
		data_sel.data_ref.record_sel.write_mode = write_mode;
	}
	else
		data_sel.data_ref.element_sel.element_ref   = 0x00;
	

	PAR_DATASEL		= &data_sel;
	PAR_SHORT_INDEX		= short_index;
	PAR_DATA		= out_data;

	/* call STACreateTrans			  */
	if (STACreateTrans(sct_id, TRUE))
		return(-1);


/*****************************************************************************/
/*
 *      normal end => release storage 
 */
	Gsct_response.noctets = 1;
	aux_free2_OctetString(&Gsct_response);

#ifdef TEST
	fprintf(stdout, "\n***** Normal end of   sca_write_record *****\n\n");
#endif
	return (0);






}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_write_record       */
/*-------------------------------------------------------------*/


/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_delete_record        VERSION   1.1	    	       */
/*				     DATE   March 1993	       */
/*			      	       BY   Levona Eckstein    */
/*							       */
/* DESCRIPTION						       */
/*  Delete record in elementary file on the smartcard.         */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   short_index               File identifier                 */
/*							       */
/*   data_struc		       data structure		       */
/*			       (FIX, VAR)	               */
/*   rec_id                    Record identifier               */
/*			                         	       */
/*   sec_mess		       security modes                  */
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
/*                              sw1/sw2 from ICC  response     */
/*							       */
/*-------------------------------------------------------------*/
int 
sca_delete_record(sct_id, short_index, data_struc, rec_id, sec_mess)
	int             sct_id;
	unsigned int	short_index;
	DataStruc	data_struc;
	unsigned int    rec_id;
	ICC_SecMess      *sec_mess;
{
	DataSel		data_sel;
	char		*proc="sca_delete_record";

	sca_errno = M_NOERR;
	sca_errmsg = NULL;


#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_delete_record *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id                  : %d\n", sct_id);
	fprintf(stdout, "short_index             : %d\n", short_index);
	print_DataStruc(data_struc);
	fprintf(stdout, "rec_id                  : %d\n", rec_id);
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
 *      create ICC command DELREC
 */
	if (STACreateHead(ICC_DELREC, sec_mess))
		return (-1);


	/* set parameters			  */
	data_sel.data_struc	=	data_struc;
	data_sel.data_ref.record_sel.record_id	= rec_id;
	

	PAR_DATASEL		= &data_sel;
	PAR_SHORT_INDEX		= short_index;


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
	fprintf(stdout, "\n***** Normal end of   sca_delete_record *****\n\n");
#endif
	return (0);





}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_delete_record      */
/*-------------------------------------------------------------*/


/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_lock_key             VERSION   1.1	    	       */
/*				     DATE   March 1993	       */
/*			      	       BY   Levona Eckstein    */
/*							       */
/* DESCRIPTION						       */
/*  Lock key on smartcard.              	               */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   key_id                    Key identifier                  */
/*							       */
 /*   sec_mess		       security modes                  */
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
/*-------------------------------------------------------------*/
int 
sca_lock_key(sct_id, key_id, sec_mess)
	int             sct_id;
	KeyId          *key_id;
	ICC_SecMess     *sec_mess;
{
	char		*proc="sca_lock_key";

	sca_errno = M_NOERR;
	sca_errmsg = NULL;

#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_lock_key *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id                  : %d\n", sct_id);
	print_KeyId(key_id);
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
 *      create ICC command LOCKKEY 
 */
	if (STACreateHead(ICC_LOCKKEY, sec_mess))
		return (-1);


	/* set parameters			  */
	if (key_id->key_level == INITKEY)
		key_id->key_number = 2;

	PAR_KID 	= key_id;
	PAR_OPERATION 	= CO_LOCK;


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
	fprintf(stdout, "\n***** Normal end of   sca_lock_key *****\n\n");
#endif
	return (0);







}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_lock_key	       */
/*-------------------------------------------------------------*/


/*-------------------------------------------------------------*/
/*						         | GMD */
/*						         +-----*/
/* PROC  sca_unlock_key           VERSION   1.1	    	       */
/*				     DATE   March 1993	       */
/*			      	       BY   Levona Eckstein    */
/*							       */
/* DESCRIPTION						       */
/*  Unlock key on the smartcard.         	               */
/*							       */
/*							       */
/* IN			     DESCRIPTION		       */
/*   sct_id		       SCT identifier		       */
/*                                                             */
/*   key_id                    Key identifier                  */
/*							       */
 /*   sec_mess		       security modes                  */
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
/*                              sw1/sw2 from ICC  response     */
/*							       */
/*-------------------------------------------------------------*/
int 
sca_unlock_key(sct_id, key_id, sec_mess)
	int             sct_id;
	KeyId          *key_id;
	ICC_SecMess     *sec_mess;
{
	char		*proc="sca_unlock_key";

	sca_errno = M_NOERR;
	sca_errmsg = NULL;

#ifdef TEST
	fprintf(stdout, "\n***** STARMOD-Routine sca_unlock_key *****\n\n");
	fprintf(stdout, "TRACE of the input parameters : \n");
	fprintf(stdout, "sct_id                  : %d\n", sct_id);
	print_KeyId(key_id);
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
 *      create ICC command LOCKKEY
 */
	if (STACreateHead(ICC_LOCKKEY, sec_mess))
		return (-1);


	/* set parameters			  */
	PAR_KID		 = key_id;
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
	fprintf(stdout, "\n***** Normal end of   sca_unlock_key *****\n\n");
#endif
	return (0);







}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sca_unlock_key	       */
/*-------------------------------------------------------------*/

