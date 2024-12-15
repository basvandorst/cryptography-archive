/*
 *  STARMOD Release 1.1 (GMD)
 */
/*-------------------------------------------------------+-----*/
/*                                                       | GMD */
/*   SYSTEM   STARMOD  -  Version 1.1                    +-----*/
/*                                                             */
/*-------------------------------------------------------------*/
/*                                                             */
/*    PACKAGE   ICCDE                   VERSION 1.1            */
/*                                         DATE January  1993  */
/*                                           BY Levona Eckstein*/
/*    FILENAME                                                 */
/*      iccde.c                                                */
/*                                                             */
/*    DESCRIPTION                                              */
/*      ICC  - Interface - Module                              */
/*                                                             */
/*    EXPORT                DESCRIPTION                        */
/*                                                             */
/*      icc_create               create ICC-Request-Apdu       */
/*                                                             */
/*      icc_check                check  ICC-Response-Apdu      */
/*                                                             */
/*      icc_OutSecMessConc       encrypt ICC-APDU              */
/*                                                             */
/*      icc_InSecMessConc        decrypt ICC-APDU              */
/*                                                             */
/*      icc_OutSecMessAuth       create ICC-APDU with MAC      */
/*                                                             */
/*      icc_InSecMessAuth        check ICC-APDU with MAC       */
/*                                                             */
/*      icc_giveParmList         return Parameter list for     */
/*				 icc command		       */	
/*                                                             */
/*      icc_e_KeyId             create key identifier          */
/*                                                             */
/*      icc_e_FileId            create file identifier         */
/*                                                             */
/*      icc_e_KeyAttrList       create the key header          */
/*                                                             */
/*      icc_check_KeyId         test KeyId-Parameter           */
/*                                                             */
/*      icc_if.h                                               */
/*                                                             */
/*      icc_errno                error-number                  */
/*                                                             */
/*      icc_errmsg               address of error message      */
/*                                                             */
/*                                                             */
/*    IMPORT                DESCRIPTION                        */
/*                                                             */
/*      aux_fxdump        dump buffer in File                  */
/*                                                             */
/*      sca_aux_icc_apdu       dump ICC-APDU in file           */
/*                                                             */
/*      sca_aux_icc_resp       dump ICC-RESP in file           */
/*                                                             */
/*                                                             */
/*    INTERNAL              DESCRIPTION                        */
/*                                                             */
/*          					               */
/*      e_OperationMode       create the Operation Mode Param. */
/*                      			               */
/*      e_AccessControl       create the ACV Param.            */
/*                      			               */
/*      e_TwoByte             create                           */
/*                            (HIGH/LOW-Byte) in icc_apdu      */
/*						               */
/*      e_AuthControlPar      create the acp parameter         */
/*                      			               */
/*      Ioput                 transmit one byte in icc_apdu    */
/*                                                             */
/*      Ioputbuff             transmit string in icc_apdu      */
/*                                                             */
/*      ICCalloc               allocate buffer                 */
/*                                                             */
/*      ICCcheck_len           check length of parameter       */
/*                                                             */
/*      ICCcheck_FileSel       test FileSel-Parameter          */
/*                                                             */
/*                                                             */
/*-------------------------------------------------------------*/

/*-------------------------------------------------------------*/
/*   include-Files                                             */
/*-------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "sca.h"
#include "stamod.h"


/*-------------------------------------------------------------*/
/*   extern declarations                                       */
/*-------------------------------------------------------------*/


extern void     aux_fxdump();
extern int      sca_aux_icc_apdu();
extern void     sca_aux_icc_resp();
extern void     aux_free2_OctetString();
extern void     aux_free2_BitString();
extern int	aux_cmp_ObjId();
extern int      des_encrypt();
extern int      des_decrypt();

/*-------------------------------------------------------------*/
/*   forward global declarations                               */
/*-------------------------------------------------------------*/
int             icc_create();
int             icc_check();
int             icc_OutSecMessConc();
int             icc_InSecMessConc();
int             icc_OutSecMessAuth();
int             icc_InSecMessAuth();
int		icc_giveParmList();
char            icc_e_KeyId();
char            icc_e_FileId();
void            icc_e_KeyAttrList();
int		icc_check_KeyId();

/*-------------------------------------------------------------*/
/*   forward local  declarations                               */
/*-------------------------------------------------------------*/
static char     e_OperationMode();
static char     e_AccessControl();
static void     e_TwoByte();
static char     e_AuthControlPar();
static void     Ioput();
static void     Ioputbuff();
static char    *ICCalloc();
static int      ICCcheck_len();
static int      ICCcheck_FileSel();
static int	sta_itoa();



/*-------------------------------------------------------------*/
/*   global  variable definitions                              */
/*-------------------------------------------------------------*/
unsigned int    icc_errno;	/* error variable              */
char           *icc_errmsg;	/* address of error message    */

ICC_ParmList icc_cmdlist[] = {
{ICC_SELECT,ICC_ISO,ICC_LEN_FILESEL,
 4,	ICC_PARM_filecat,
	ICC_PARM_fileinforeq,
	ICC_PARM_LEN,
	ICC_PARM_filesel,
   	ICC_PARM_ABSENT},
{ICC_CLOSE,ICC_NON_ISO,ICC_LEN_FILESEL,
 4,	ICC_PARM_filecat,
	ICC_PARM_context,
	ICC_PARM_LEN,
	ICC_PARM_filesel,
  	ICC_PARM_ABSENT},
{ICC_READB,ICC_ISO,ICC_LEN_LRDATA,
 2,	ICC_PARM_datasel,
	ICC_PARM_LEN,
	ICC_PARM_ABSENT,
	ICC_PARM_ABSENT,
  	ICC_PARM_ABSENT},
{ICC_READR,ICC_ISO,ICC_LEN_LRDATA,
 2,	ICC_PARM_datasel,
	ICC_PARM_LEN,
	ICC_PARM_ABSENT,
	ICC_PARM_ABSENT,
   	ICC_PARM_ABSENT},
{ICC_WRITEB,ICC_ISO,ICC_LEN_OCTET,
 3,	ICC_PARM_datasel,
	ICC_PARM_LEN,
	ICC_PARM_data,
	ICC_PARM_ABSENT,
   	ICC_PARM_ABSENT},
{ICC_WRITER,ICC_ISO,ICC_LEN_OCTET,
 3,	ICC_PARM_datasel,
	ICC_PARM_LEN,
	ICC_PARM_data,
	ICC_PARM_ABSENT,
   	ICC_PARM_ABSENT},
{ICC_CREATE,ICC_NON_ISO,ICC_LEN_FINFO,
 4,	ICC_PARM_filecat,
	ICC_PARM_datastruc,
	ICC_PARM_LEN,
	ICC_PARM_filecontrolinfo,
  	ICC_PARM_ABSENT},
{ICC_DELF,ICC_NON_ISO,ICC_LEN_FILESEL,
 4,	ICC_PARM_filecat,
	ICC_PARM_NOTUSED,
	ICC_PARM_LEN,
	ICC_PARM_filesel,
   	ICC_PARM_ABSENT},
{ICC_DELREC,ICC_NON_ISO,ICC_LEN_NOTUSED,
 2,	ICC_PARM_datasel,
	ICC_PARM_LEN,
	ICC_PARM_ABSENT,
	ICC_PARM_ABSENT,
   	ICC_PARM_ABSENT},
{ICC_SETKEY,ICC_NON_ISO,ICC_LEN_NOTUSED,
 3,	ICC_PARM_kid,
	ICC_PARM_conc_kid,
	ICC_PARM_LEN,
	ICC_PARM_ABSENT,
   	ICC_PARM_ABSENT},
{ICC_WR_KEY,ICC_NON_ISO,ICC_LEN_KEY,
 4,	ICC_PARM_INSTMODE,
	ICC_PARM_kid,
	ICC_PARM_LEN,
	ICC_PARM_keyattrlist,
        ICC_PARM_ABSENT},
{ICC_GET_CD,ICC_NON_ISO,ICC_LEN_FIX,
 3,	ICC_PARM_NOTUSED,
	ICC_PARM_NOTUSED,
	ICC_PARM_LEN,
	ICC_PARM_ABSENT,
  	ICC_PARM_ABSENT},
{ICC_GETCHAL,ICC_NON_ISO,ICC_LEN_FIX,
 3,	ICC_PARM_NOTUSED,
	ICC_PARM_NOTUSED,
	ICC_PARM_LEN,
	ICC_PARM_ABSENT,
   	ICC_PARM_ABSENT},
{ICC_CRYPT,ICC_NON_ISO,ICC_LEN_OCTET,
 4,	ICC_PARM_modi,
	ICC_PARM_kid,
	ICC_PARM_LEN,
	ICC_PARM_data,
   	ICC_PARM_ABSENT},
{ICC_KEYSTAT,ICC_NON_ISO,ICC_LEN_NOTUSED,
 3,	ICC_PARM_NOTUSED,
	ICC_PARM_kid,
	ICC_PARM_LEN,
	ICC_PARM_ABSENT,
   	ICC_PARM_ABSENT},
{ICC_LOCKKEY,ICC_NON_ISO,ICC_LEN_NOTUSED,
 3,	ICC_PARM_operation,
	ICC_PARM_kid,
	ICC_PARM_LEN,
	ICC_PARM_ABSENT,
   	ICC_PARM_ABSENT},
{ICC_REGISTER,ICC_NON_ISO,ICC_LEN_REG,
 4,	ICC_PARM_units,
	ICC_PARM_LEN,
	ICC_PARM_acv,
   	ICC_PARM_data,
	ICC_PARM_ABSENT},
{ICC_LOCKF,ICC_NON_ISO,ICC_LEN_FILEID,
 4,	ICC_PARM_LOCKCAT,
	ICC_PARM_operation,
	ICC_PARM_LEN,
   	ICC_PARM_fid,
	ICC_PARM_ABSENT},
{ICC_VERIFY,ICC_ISO,ICC_LEN_OCTET,
 4,	ICC_PARM_acp_verify,
	ICC_PARM_kid,
	ICC_PARM_LEN,
   	ICC_PARM_data,
	ICC_PARM_ABSENT},
{ICC_IN_AUTH,ICC_ISO,ICC_LEN_OCTET,
 4,	ICC_PARM_NOTUSED,
	ICC_PARM_kid,
	ICC_PARM_LEN,
   	ICC_PARM_data,
	ICC_PARM_ABSENT},
{ICC_EX_AUTH,ICC_ISO,ICC_LEN_OCTET,
 4,	ICC_PARM_NOTUSED,
	ICC_PARM_kid,
	ICC_PARM_LEN,
   	ICC_PARM_data,
	ICC_PARM_ABSENT},
{ICC_MU_AUTH,ICC_NON_ISO,ICC_LEN_OCTET,
 4,	ICC_PARM_acp_auth,
	ICC_PARM_kid,
	ICC_PARM_LEN,
   	ICC_PARM_data,
	ICC_PARM_ABSENT},
{ICC_VER_CHG,ICC_NON_ISO,ICC_LEN_PIN,
 4,	ICC_PARM_acp_verify,
	ICC_PARM_kid,
	ICC_PARM_LEN,
  	ICC_PARM_CHGPIN,
	ICC_PARM_ABSENT},
{ICC_EXCHAL,ICC_NON_ISO,ICC_LEN_OCTET,
 4,	ICC_PARM_direction,
	ICC_PARM_NOTUSED,
	ICC_PARM_LEN,
   	ICC_PARM_data,
	ICC_PARM_ABSENT}

};




/*-------------------------------------------------------------*/
/*   local  Variable definitions                               */
/*-------------------------------------------------------------*/

#ifdef ICCTRACE 
static Boolean   first = FALSE;	/* FLAG, if Trace-File open     */

FILE           *icc_trfp;	/* Filepointer of trace file    */

#endif

static char MF_NAME[2] = {0x3F, 0x00};

/*-------------------------------------------------------------*/
/*   type definitions                                          */
/*-------------------------------------------------------------*/
#define ICCHEAD                 icc_cmd->icc_header

#define HEADLEN                 5  /* CLA + INS + P1 + P2 + 1 Byte LEN */

#define SOURCE_DTE              0x01     /* Source Address = DTE */

#define BLANK                   0x20

/*-------------------------------------------------------------*/
/*   macro definitions					       */
/*-------------------------------------------------------------*/
#define ALLOC_CHAR(v,s)  {if (0 == (v = malloc(s))) {icc_errno = EMEMAVAIL; icc_errmsg = sca_errlist[icc_errno].msg; goto errcase;}}



/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  icc_create           VERSION   1.1               */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Create ICC-Command                                    */
/*  This procedure allocates the buffer for the           */
/*  ICC-Command and generates the ICC-APDU.               */
/*  If sec_mode = TRUE, then the SSC will be set in the   */
/*  APDU.						  */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   icc_cmd                    ICC-Command-Structure     */
/*                                                        */
/*   sec_mode  		       Flag, if secure messaging  */
/*							  */
/*   ssc		       Send Sequence Counter      */
/*			       only used in case of       */
/*			       sec_mode = TRUE            */
/* OUT                                                    */
/*   icc_apdu                   Pointer of ICC-Command-APDU*/
/*			       Construction:              */
/*			       ______________________     */
/*			      | CLA,INS,P1,P2,L,DATA |    */
/*			       ______________________     */
/*			       or:			  */
/*			       __________________________ */
/*			      | CLA,INS,P1,P2,L,SSC,DATA |*/
/*			       __________________________ */
/*			      The memory for icc_apdu->octets*/
/*			      will be allocated by this   */
/*			      programm and must be set    */
/*			      free by the calling program */
/*			      only in case of no error    */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   0                         o.k                        */
/*   -1                        Error                      */
/*				EINS			  */
/*			        EPARINC                   */
/*						          */
/* CALLED FUNCTIONS					  */
/*   icc_e_KeyId                                          */
/*   icc_e_FileId              Error                      */
/*				EPARINC 		  */ 
/*   e_OperationMode                                      */
/*   e_AccessControl                                      */
/*   e_TwoByte                                            */
/*   e_AuthControlPar                                     */
/*   Ioput                                                */
/*   Ioputbuff                                            */
/*   ICCalloc		       Error                      */
/*			        EMEMAVAIL                 */
/*   ICCcheck_len              Error                      */
/*				ETOOLONG                  */ 
/*   ICCcheck_FileSel          Error                      */
/*				EPARINC 		  */ 
/*				ETOOLONG                  */ 
/*   icc_check_KeyId           Error                      */
/*				EPARINC 		  */ 
/*   sca_aux_icc_apdu                                     */
/*--------------------------------------------------------*/
int
icc_create(icc_cmd, sec_mode, ssc, icc_apdu)
	struct s_command *icc_cmd;	/* ICC-Command    	    */
	Boolean         sec_mode;
	int             ssc;
	OctetString     *icc_apdu;	/* Structure of ICC-APDU    */
{
  	char           *scapdu = NULL;
	unsigned int    lscapdu = HEADLEN;
	char           *ptr = NULL;
	char            oldpin[9];
	char            newpin[9];
	char            finfolen;
	char            kid;
	char            fid;
	char		p2;
	char		delrec_acv, read_acv, write_acv, delfile_acv, acv, acp;
	unsigned int    len = 0;
	unsigned int    class;
	unsigned int    i;
	char            op_mode;
	int		index, listlen, file_len;
	Boolean		ins_found = FALSE;
	char		file_name[30];
	char		pid[10];
	char		*proc="icc_create";

	icc_apdu->noctets = 0;
	icc_apdu->octets = NULL;

#ifdef ICCTRACE 
	if (!first) {
		strcpy(file_name,"ICC");
		sta_itoa(pid,getpid());
		strcat(file_name,pid);
		strcat(file_name,".TRC");
		icc_trfp = fopen(file_name, "wt");
		first = TRUE;
	};
#endif

	icc_errno = 0;

/*****************************************************************************/
/*
 *      test security mode for command; if TRUE => add 1 byte to length for SSC
 *	
	if (sec_mode == TRUE) {
		lscapdu++;
		if (ssc != 0)
			ssc = ssc % 256;
	};
	
/*****************************************************************************/
/*
 *      search inscode in cmdlist
 */
	listlen = sizeof(icc_cmdlist) / sizeof(ICC_ParmList);
	for (index = 0; index < listlen; index++) {

		if (icc_cmdlist[index].inscode == 
		    icc_cmd->icc_header.inscode) {
			ins_found = TRUE;
			break;
		}
	}
/*****************************************************************************/
/*
 *      test, if inscode found
 */

	if (ins_found == FALSE) {
		icc_errno = EINS;
		icc_errmsg = sca_errlist[icc_errno].msg;
		return (-1);
	}

/*****************************************************************************/
/*
 *      allocate memory for icc_apdu depending on parameter length
 */
	switch (icc_cmdlist[index].parmlen) {
		case ICC_LEN_FILESEL:

		        /* In case of 
		           filecat == MF / PARENT_DF/
				      CHILD_DF => the file_name is checked
		   		   == EF       => the file_id is checked 
			*/
 
			if (ICCcheck_FileSel(icc_cmd->icc_header.inscode,
					     icc_cmd->filecat,
					     icc_cmd->filesel,
					     &len,&fid) == -1)
		  		return(-1);


			lscapdu += len;
			break;

		case ICC_LEN_LRDATA:

			len = icc_cmd->lrddata;
			break;

		case ICC_LEN_OCTET:

			if ((icc_cmdlist[index].inscode	== ICC_EXCHAL) &&
			    (icc_cmd->direction == GET_CHALLENGE))
				len = 8;
			else {
				len = icc_cmd->data->noctets;
				lscapdu += len;
			}
			break;

		case ICC_LEN_FINFO:
			if (ICCcheck_len(icc_cmd->filecontrolinfo->addinfo->noctets,
			       		MAX_ADDINFO))
				return (-1);


			if (ICCcheck_FileSel(icc_cmd->icc_header.inscode,
		    		             icc_cmd->filecat,
					icc_cmd->filecontrolinfo->file_sel,
		                    	     &file_len,&fid) == -1)
		 	        return(-1);

			len = file_len + UNITLEN + SSDLEN + OPLEN + SIZELEN + 1 +
			      icc_cmd->filecontrolinfo->addinfo->noctets + 1;

			lscapdu += len;
			break;

		case ICC_LEN_NOTUSED:
			len = 0;
			break;

		case ICC_LEN_FIX:
			len = CDLEN;
			break;

		case ICC_LEN_KEY:
			len = KEYHEAD + icc_cmd->data->noctets;
			lscapdu += len;
			break;

		case ICC_LEN_REG:
			if ((icc_cmd->data == NULL) ||
		            (icc_cmd->data->octets == NULL) ||
			    (icc_cmd->data->noctets == 0) ||
			    (icc_cmd->data->noctets > MAX_FILENAME) ||
			    (*icc_cmd->data->octets == 0x20))
				goto parerrcase;

			len = REGACVLEN + icc_cmd->data->noctets;
			lscapdu += len;
			break;

		case ICC_LEN_FILEID:
			len = FIDLEN;
			lscapdu += len;
			break;

		case ICC_LEN_PIN:
			len = PINLEN * 2;
			lscapdu += len;
			break;



	};

	if ((scapdu = ICCalloc(lscapdu)) == NULL)
		return (-1);

	ptr = scapdu + 2;

/*****************************************************************************/
/*
 *      create sc apdu
 */
	for (i=0; i<icc_cmdlist[index].parm_no; i++) {
		switch (icc_cmdlist[index].parmtype[i]) {

		case ICC_PARM_filecat:
		        if ((icc_cmdlist[index].inscode == ICC_SELECT)) {
				if ((icc_cmd->filecat > EF) ||
		    		    (icc_cmd->filecat < PARENT_DF)) 
					goto parerrcase;

			}
			else {
				if (icc_cmdlist[index].inscode == ICC_CLOSE) {
					switch (icc_cmd->context) {
					case CLOSE_CREATE: 
						if ((icc_cmd->filecat < MF) ||
		    		    		    (icc_cmd->filecat > EF))
							goto parerrcase;
						break;
					case CLOSE_SELECT: 
						if ((icc_cmd->filecat < PARENT_DF) ||
		    		    		    (icc_cmd->filecat > CHILD_DF))
							goto parerrcase;
						break;
					}
				}
				else
					if ((icc_cmd->filecat > EF) ||
			    		    (icc_cmd->filecat < MF))
						goto parerrcase;
			};

		        if (icc_cmdlist[index].inscode == ICC_SELECT) {
				switch (icc_cmd->filecat) {
				case EF:
					Ioput(&ptr, SEL_EF);
					break;
				case CHILD_DF:
					Ioput(&ptr, SEL_CHILD_DF);
					break;
				case PARENT_DF:
					Ioput(&ptr, SEL_PARENT_DF);
					break;
				}
			}
			else
				Ioput(&ptr, (char) icc_cmd->filecat);
			break;

		case ICC_PARM_fileinforeq:
			if ((icc_cmd->fileinforeq != NONE_INFO) &&
		            (icc_cmd->fileinforeq != COMP_INFO))
				goto parerrcase;

			Ioput(&ptr, (char) icc_cmd->fileinforeq);
			break;

		case ICC_PARM_LEN:
			Ioput(&ptr, len);
			if (sec_mode == TRUE)
				Ioput(&ptr, ssc);

			break;

		case ICC_PARM_filesel:
			if (icc_cmd->filecat != EF)
				Ioputbuff(&ptr, icc_cmd->filesel->file_name->octets,
					        len);
			else {
				Ioput(&ptr, icc_cmd->filesel->file_id->file_qualifier);
							/* file qualifier */
				Ioput(&ptr, fid);	/* file index */
			}
			break;

		case ICC_PARM_context:
			if ((icc_cmd->context < CLOSE_CREATE) ||
			    (icc_cmd->context > CLOSE_SELECT))
				goto parerrcase;
			Ioput(&ptr, (char) icc_cmd->context);
			break;

		case ICC_PARM_datasel:
			switch (icc_cmdlist[index].inscode) {
			case ICC_READB:
			case ICC_WRITEB:
				if (icc_cmd->datasel->data_struc != TRANSPARENT)
					goto parerrcase;
				e_TwoByte(&ptr,icc_cmd->datasel->data_ref.string_sel);

				break;

			case ICC_READR:
			case ICC_WRITER:
				if ((icc_cmd->datasel->data_struc > CYCLIC) ||
		    	            (icc_cmd->datasel->data_struc < LIN_FIX) ||
		    	            ((icc_cmd->datasel->data_struc == LIN_FIX) &&
		     	     	     ((icc_cmd->datasel->data_ref.record_sel.record_id < 0x01) ||
		      	      	      (icc_cmd->datasel->data_ref.record_sel.record_id > 0xFD))) ||
		                    ((icc_cmd->datasel->data_struc == LIN_VAR) &&
		              	     ((icc_cmd->datasel->data_ref.record_sel.record_id < 0x01) ||
		               	      (icc_cmd->datasel->data_ref.record_sel.record_id > 0xFE))) ||
		              	    ((icc_cmdlist[index].inscode == ICC_READR) && (icc_cmd->datasel->data_struc == CYCLIC) &&
		               	     ((icc_cmd->datasel->data_ref.element_sel.element_ref < 0x01) ||
		                      (icc_cmd->datasel->data_ref.element_sel.element_ref > 0xFD))) ||
		                    (icc_cmd->short_index < 0) ||
		           	    (icc_cmd->short_index > 0x0F))
					goto parerrcase;

				switch (icc_cmd->datasel->data_struc) {
				case CYCLIC:
					Ioput(&ptr,
					icc_cmd->datasel->data_ref.element_sel.element_ref);
	
					if (icc_cmdlist[index].inscode == ICC_READR)
						p2 = (((char) icc_cmd->short_index & 0xFF) << 3) |
			     			      ((char) RECORD_NUMBER & 0xFF);
					else
						p2 = (((char) icc_cmd->short_index & 0xFF) << 3) |
			      			      ((char) APPEND & 0xFF);
							break;

				case LIN_FIX:
					Ioput(&ptr,
                                        icc_cmd->datasel->data_ref.record_sel.record_id);
	
					if (icc_cmdlist[index].inscode == ICC_READR)
						p2 = (((char) icc_cmd->short_index & 0xFF) << 3) |
			      			      ((char) RECORD_NUMBER & 0xFF);
					else
						p2 = (((char) icc_cmd->short_index & 0xFF) << 3) |
			      			      ((char) icc_cmd->datasel->data_ref.record_sel.write_mode & 0xFF);
					break;

				case LIN_VAR:
					Ioput(&ptr,
 					icc_cmd->datasel->data_ref.record_sel.record_id);
					if (icc_cmdlist[index].inscode == ICC_READR)
						p2 = (((char) icc_cmd->short_index & 0xFF) << 3) |
			      			      ((char) RECORD_ID & 0xFF);
					else
						p2 = (((char) icc_cmd->short_index & 0xFF) << 3) |
			      			      ((char) icc_cmd->datasel->data_ref.record_sel.write_mode & 0xFF);
					break;
				};
				Ioput(&ptr, p2);
				break;

			case ICC_DELREC:
				if ((icc_cmd->datasel->data_struc > LIN_VAR) ||
		    	            (icc_cmd->datasel->data_struc < LIN_FIX) ||
		    	            ((icc_cmd->datasel->data_struc == LIN_FIX) &&
		     	             ((icc_cmd->datasel->data_ref.record_sel.record_id < 0x01) ||
		     		      (icc_cmd->datasel->data_ref.record_sel.record_id > 0xFD))) ||
		    	            ((icc_cmd->datasel->data_struc == LIN_VAR) &&
		     		     ((icc_cmd->datasel->data_ref.record_sel.record_id < 0x01) ||
		      		      (icc_cmd->datasel->data_ref.record_sel.record_id > 0xFE))) ||
		    	            (icc_cmd->short_index < 0) ||
		    	            (icc_cmd->short_index > 0x0F)) 
					goto parerrcase;


				switch (icc_cmd->datasel->data_struc) {
				case LIN_FIX:
					Ioput(&ptr,
                                       icc_cmd->datasel->data_ref.record_sel.record_id);
	
					p2 = (((char) icc_cmd->short_index & 0xFF) << 3) |
			      		      ((char) RECORD_NUMBER & 0xFF);
					break;

				case LIN_VAR:
					Ioput(&ptr,
 					icc_cmd->datasel->data_ref.record_sel.record_id);
					p2 = (((char) icc_cmd->short_index & 0xFF) << 3) |
			      		      ((char) RECORD_ID & 0xFF);
					break;
				};
				Ioput(&ptr, p2);

				break;
			};

			break;

		case ICC_PARM_data:
			if ((icc_cmdlist[index].inscode	== ICC_EXCHAL) &&
			    (icc_cmd->direction == GET_CHALLENGE))
				break;
			else
				Ioputbuff(&ptr, icc_cmd->data->octets, icc_cmd->data->noctets);
			break;

		case ICC_PARM_datastruc:


		    	if ((icc_cmd->filecat == EF) &&
		     	    ((icc_cmd->datastruc > TRANSPARENT) ||
		             (icc_cmd->datastruc < LIN_FIX)))
				goto parerrcase;

			if (icc_cmd->filecat == EF)
				Ioput(&ptr, (char) icc_cmd->datastruc);
			else
				Ioput(&ptr, ICC_NOTUSED);
			break;

		case ICC_PARM_filecontrolinfo:
                	delrec_acv = e_AccessControl(icc_cmd->filecontrolinfo->delrec_acv);

                	read_acv = e_AccessControl(icc_cmd->filecontrolinfo->racv);
                	write_acv = e_AccessControl(icc_cmd->filecontrolinfo->wacv);
                	delfile_acv = e_AccessControl(icc_cmd->filecontrolinfo->delfile_acv);


		    	if ((icc_cmd->filecontrolinfo->units <= 0) ||

		            ((icc_cmd->filecat == EF) &&
		             (icc_cmd->filecontrolinfo->file_sel->file_id->file_type == WEF) &&
		             (icc_cmd->filecontrolinfo->readwrite > WRITE_ONLY))) 
				goto parerrcase;

			e_TwoByte(&ptr, icc_cmd->filecontrolinfo->units);
			Ioput(&ptr, delrec_acv);
			Ioput(&ptr, read_acv);
			Ioput(&ptr, write_acv);
			Ioput(&ptr, delfile_acv);
			op_mode = e_OperationMode(icc_cmd->filecontrolinfo);

			Ioput(&ptr, op_mode);
			if (icc_cmd->filecat == EF)
				Ioput(&ptr, icc_cmd->filecontrolinfo->recordsize);
			else
				Ioput(&ptr, ICC_NOTUSED);

			Ioput(&ptr, file_len);
			if (icc_cmd->filecat != EF)
				Ioputbuff(&ptr, icc_cmd->filecontrolinfo->file_sel->file_name->octets,
					        file_len);
			else {
				Ioput(&ptr, 0x00);	/* file qualifier */
				Ioput(&ptr, fid);	/* file index */
			}

			Ioput(&ptr, icc_cmd->filecontrolinfo->addinfo->noctets);
			Ioputbuff(&ptr, icc_cmd->filecontrolinfo->addinfo->octets,
			 	        icc_cmd->filecontrolinfo->addinfo->noctets);

			break;

		case ICC_PARM_NOTUSED:
			Ioput(&ptr, ICC_NOTUSED);
			break;

		case ICC_PARM_kid:
			if (icc_cmdlist[index].inscode != ICC_SETKEY) { 
				if(icc_check_KeyId(icc_cmd->kid) == -1)
					goto parerrcase;
			}
			kid = icc_e_KeyId(icc_cmd->kid);
			Ioput(&ptr, kid);
			break;
			
		case ICC_PARM_conc_kid:
			if (icc_cmdlist[index].inscode != ICC_SETKEY) { 
				if(icc_check_KeyId(icc_cmd->kid) == -1)
					goto parerrcase;
			}

			kid = icc_e_KeyId(icc_cmd->conc_kid);
			Ioput(&ptr, kid);
			break;

		case ICC_PARM_modi:
			Ioput(&ptr, icc_cmd->modi);
			break;

		case ICC_PARM_INSTMODE:
			if (!icc_cmd->keyattrlist)
				goto parerrcase;

			Ioput(&ptr, (int) icc_cmd->keyattrlist->key_inst_mode);
			break;

		case ICC_PARM_keyattrlist:
			if (!icc_cmd->keyattrlist)
				goto parerrcase;

			icc_e_KeyAttrList(&ptr, icc_cmd->keyattrlist, icc_cmd->key_algid);
			Ioput(&ptr, icc_cmd->data->noctets);
			Ioputbuff(&ptr, icc_cmd->data->octets, icc_cmd->data->noctets);
			break;

		case ICC_PARM_operation:
		    	if ((icc_cmd->operation != CO_UNLOCK) &&
		            (icc_cmd->operation != CO_LOCK))
				goto parerrcase;
			Ioput(&ptr, icc_cmd->operation);
			break;

		case ICC_PARM_units:
			e_TwoByte(&ptr, icc_cmd->units);
			break;

		case ICC_PARM_acv:
                	acv = e_AccessControl(icc_cmd->acv);
			Ioput(&ptr, acv);
			break;

		case ICC_PARM_LOCKCAT:
			Ioput(&ptr, LOCK_CAT);
			break;

		case ICC_PARM_fid:
			if ((!icc_cmd->fid) ||
	    		    (icc_cmd->fid->file_type != XEF) &&
	    		    (icc_cmd->fid->file_type != WEF) &&
	    		    (icc_cmd->fid->file_type != ISF))
				goto parerrcase;

			if ((fid = icc_e_FileId(icc_cmd->fid)) == -1)
				goto parerrcase;
			Ioput(&ptr, 0x00);
			Ioput(&ptr, fid);
			break;

		case ICC_PARM_acp_verify:
			icc_cmd->acp_verify.key_search = 0x00;
			acp = e_AuthControlPar(icc_cmd->acp_verify);
			Ioput(&ptr, acp);
			break;

		case ICC_PARM_acp_auth:
			acp = e_AuthControlPar(icc_cmd->acp_auth);
			Ioput(&ptr, acp);
			break;

		case ICC_PARM_CHGPIN:
			if ((icc_cmd->data->noctets > PINLEN) ||
		    	    (icc_cmd->new_pin->noctets > PINLEN)) 
				goto parerrcase;
			for (i = 0; i < 8; i++) {
				oldpin[i] = BLANK;
				newpin[i] = BLANK;
			}

			memcpy(oldpin, icc_cmd->data->octets, icc_cmd->data->noctets);
			memcpy(newpin, icc_cmd->new_pin->octets, icc_cmd->new_pin->noctets);
		 	Ioputbuff(&ptr, oldpin, PINLEN);
			Ioputbuff(&ptr, newpin, PINLEN);
			break;

 		case ICC_PARM_direction:
			if ((icc_cmd->direction < GET_CHALLENGE) || 
		            (icc_cmd->direction > EXCHANGE_CHALLENGE))
				goto parerrcase;
			Ioput(&ptr, (int) icc_cmd->direction);
			break;


		case ICC_PARM_ABSENT:
			break;

		}; /* end of switch */
	}; /* end of for */


/*****************************************************************************/
/*
 *      create CLASS / INS in scapdu and return pointer
 */
	class = icc_cmdlist[index].cmd_class; 
	if (ICCHEAD.security_mess.command != ICC_SEC_NORMAL)
		class |= (unsigned) ICCHEAD.security_mess.command << 2;
	if (ICCHEAD.security_mess.response != ICC_SEC_NORMAL)
		class |= (unsigned) ICCHEAD.security_mess.response;

	*scapdu = class;
	*(scapdu + 1) = ICCHEAD.inscode;




#ifdef ICCTRACE 
	sca_aux_icc_apdu(icc_trfp, scapdu, lscapdu);
#endif

	icc_apdu->noctets = lscapdu;
	icc_apdu->octets = scapdu;
	return (0);



/*****************************************************************************/
/*
 *      In error case release all allocated storage
 */
parerrcase:
	icc_errno = EPARINC;
	icc_errmsg = sca_errlist[icc_errno].msg;

	if (scapdu)
	  free(scapdu);

	return (-1);


}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      icc_create              */
/*-------------------------------------------------------------*/



/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  icc_check            VERSION   1.1               */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Check  ICC-Response (without Secure Messaging)        */
/*  The Response must have the structur:		  */
/*       ______________________                           */
/*      | DATA,SW1,SW2         |                          */
/*       ______________________                           */
/*  Check the SW1 / SW2 - Byte.                           */
/*  In case of O.K., icc_check remove                     */
/*  the SW1 and the SW2 - field out of the response-buffer.*/
/*  The response-buffer contains only the datafield       */
/*  without SW1 / SW2.                                    */
/*  If SW1/SW2 indicates an error, icc_check      returns */
/*  the value -1 and in icc_errno  the error number.      */
/*                                                        */
/*                                                        */
/*                                                        */
/* INOUT                     DESCRIPTION                  */
/*   icc_response               ICC-Response-buffer       */
/*                                                        */
/* OUT                                                    */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   0                         o.k                        */
/*   -1                        Error                      */
/*                              sw/sw2 of ICC response    */
/*							  */
/* CALLED FUNCTIONS					  */
/*  aux_free2_OctetString				  */
/*  sca_aux_icc_resp					  */
/*--------------------------------------------------------*/
int
icc_check(icc_response)
	OctetString     *icc_response;	/* Structure of ICC-Response-Apdu */
{
	unsigned int    sw1;
	unsigned int    sw2;
	unsigned int    index = 0;
	unsigned int    listlen = 0;

#ifdef ICCTRACE 
	sca_aux_icc_resp(icc_trfp, icc_response->octets, icc_response->noctets);
#endif



/*****************************************************************************/
/*
 *      check SW1/SW2
 *	if sw1 indicates an error, then search in sca_errlist sw1/sw2   
 *	and return index in icc_errno  to calling procedure                 
 */
	sw1 = *(icc_response->octets + (icc_response->noctets - 2)) & 0xFF;
	sw2 = *(icc_response->octets + (icc_response->noctets - 1)) & 0xFF;
	/* delete sw1/sw2 in response-buffer */
	*(icc_response->octets + (icc_response->noctets - 2)) = 0x00;
	*(icc_response->octets + (icc_response->noctets - 1)) = 0x00;
	icc_response->noctets -= 2;


	if ((sw1 != OKICC) ||
	    ((sw1 == OKICC) && (sw2 == DATAINC))) {

		while ((sca_errlist[index].sw1 != 00) || (sca_errlist[index].sw2 != 0x00)) {

			if (sca_errlist[index].sw1 == sw1 && sca_errlist[index].sw2 == sw2) {
				icc_errno = index;
				icc_errmsg = sca_errlist[index].msg;
				return (-1);
			}
			index++;
		}
		/* sw1 + sw2 not found */
		icc_errno = index;	/* last element in error-list */
		icc_errmsg = sca_errlist[index].msg;
		icc_response->noctets = 1;
		aux_free2_OctetString(icc_response);
		return (-1);
	};

	return (0);

}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      icc_check               */
/*-------------------------------------------------------------*/


/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  icc_OutSecMessAuth   VERSION   1.1               */
/*                              DATE   June   1993        */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Create ICC-COMMAND-APDU  with MAC                     */
/*  This procedure can be called in case of               */
/*  secure messaging = AUTHENTIC.                         */
/*  In case of secure messaging=COMBINED, this procedure  */
/*  must be called before calling the procedure           */
/*  icc_OutSecMessConc.                                   */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   sec_key		       Secure Messaging key	  */
/*                             Algorithm Identifier       */
/*			       The following values are   */
/*			       possible:		  */
/*			       desCBC_ISO1	          */
/*							  */
/*   in_apdu		       Pointer of ICC-APDU        */
/*                             The ICC-APDU must have the */
/*                             structur:                  */
/*			       __________________________ */
/*			      | CLA,INS,P1,P2,L,SSC,DATA |*/
/*			       __________________________ */
/*                            (= output of the procedure  */
/*				 icc_create)		  */
/*                                                        */
/*							  */
/*   maclen		       Length of MAC (0 - 8)      */
/*			       In the current Version     */
/*  			       only 4 is allowed          */
/* OUT                                                    */
/*   out_apdu                  Pointer of SEC-APDU        */
/*			       out_apdu->octets will be    */
/*			       allocated by the called    */
/*			       program			  */
/*			       and must be set free by the*/
/*			       calling program            */
/*			       The APDU has the structure:*/
/*		           ______________________________ */
/*			  | CLA,INS,P1,P2,L,SSC,DATA,MAC |*/
/*		           ______________________________ */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   0                         o.k                        */
/*   -1                        Error                      */
/*				EMEMAVAIL		  */
/*				EDESENC  		  */
/*				EPARINC     		  */
/*				EALGO    		  */
/*						          */
/* CALLED FUNCTIONS					  */
/*   des_encrypt                                          */
/*   aux_fxdump                                           */
/*   aux_free2_BitString                                  */
/*   aux_cmp_ObjId                                        */
/*   malloc macro in MF_check.h				  */ 
/*							  */
/*--------------------------------------------------------*/
int
icc_OutSecMessAuth(sec_key, in_apdu, out_apdu, maclen)
	KeyInfo      	*sec_key;	/* secure messaging key  */
	OctetString     *in_apdu;	/* ICC-APDU		 */
	OctetString     *out_apdu;	/* ICC-SEC-APDU		 */
	int             maclen;		/* Length of MAC	 */
{
	OctetString     in_octets;
	char           *ptr, *mac_ptr;
	int             i;
	int             memolen;
	BitString       out_bits;
	char           *proc="icc_OutSecMessAuth"; 


	out_apdu->noctets = 0;
	out_apdu->octets = NULL;
	out_bits.nbits   = 0;
        out_bits.bits    = NULL;

/*****************************************************************************/
/*
 *	test parameter maclen and sec_key     
 */
	if (maclen != 4) {
		icc_errno = EPARINC;
		icc_errmsg = sca_errlist[icc_errno].msg;
		return (-1);
	}
	if ((!sec_key->subjectAI) || (!sec_key->subjectAI->objid)) {
		icc_errno = EALGO;
		icc_errmsg = sca_errlist[icc_errno].msg;
		return (-1);
	}
	if (aux_cmp_ObjId(sec_key->subjectAI->objid,desCBC_ISO1->objid)) {
		icc_errno = EALGO;
		icc_errmsg = sca_errlist[icc_errno].msg;
		return (-1);
	};
	if ((sec_key->subjectkey.nbits <= 0) ||
	    (sec_key->subjectkey.bits == NULL)) {
		icc_errno = EPARINC;
		icc_errmsg = sca_errlist[icc_errno].msg;
		return (-1);
	};




/*****************************************************************************/
/*
 * 	encrypt data (CLA,INS,P1,P2,L,SSC,DATA) with Secure Messaging Key       
 */

	in_octets.noctets = in_apdu->noctets;
	in_octets.octets = in_apdu->octets;

#ifdef ICCTRACE 
	fprintf(icc_trfp, "TRACE in icc_OutSecMessAuth\n");
	fprintf(icc_trfp, "   in_octets.noctets = %d\n", in_octets.noctets);
	fprintf(icc_trfp, "   in_octets.octets  = \n");
	aux_fxdump(icc_trfp, in_octets.octets, in_octets.noctets, 0);
#endif



	/*
	 * allocate memory for out_bits             
	 * the memory must be a multiple of 8 Bytes 
	 * in case of a multiple - 8 bytes are extra
	 * padded				    
	 */

	if ((in_octets.noctets % 8) != 0)
		memolen = (in_octets.noctets - (in_octets.noctets % 8)) + 8;
	else
		memolen = in_octets.noctets + 8;


#ifdef ICCTRACE 
	fprintf(icc_trfp, "   allocate out_bits = %d\n", memolen);
#endif

	ALLOC_CHAR(out_bits.bits, memolen);	/* will be set free in this
						 * proc. */


	/*
	 * encrypt data
	 */

	memolen = des_encrypt(&in_octets, &out_bits, SEC_END, sec_key);
	if (memolen == -1) {
		icc_errno = EDESENC;
		icc_errmsg = sca_errlist[icc_errno].msg;
		out_bits.nbits = 1;
		aux_free2_BitString(&out_bits);
		return (-1);
	}
#ifdef ICCTRACE 
	fprintf(icc_trfp, "   out_bits.nbits    = %d\n", out_bits.nbits);
	fprintf(icc_trfp, "   out_bits.bits     = \n");
	aux_fxdump(icc_trfp, out_bits.bits, out_bits.nbits / 8, 0);
#endif

	memolen = in_octets.noctets + maclen;

/*****************************************************************************/
/*
 *	create output parameter      
 */
	ALLOC_CHAR(out_apdu->octets, memolen);


	out_apdu->noctets = memolen;
	ptr = out_apdu->octets;
	for (i = 0; i < in_octets.noctets; i++) {
		*ptr = *(in_octets.octets + i);
		ptr++;
	}

	/*
	 * take the first 4 octets of the last block
         */

	if ((out_bits.nbits / 8) > 8)
		mac_ptr = out_bits.bits + ((out_bits.nbits / 8) - 8);
	else
		mac_ptr = out_bits.bits;

	for (i = 0; i < maclen; i++) {
		*ptr = *(mac_ptr + i);
		ptr++;
	};
	out_bits.nbits = 1;
	aux_free2_BitString(&out_bits);

#ifdef ICCTRACE 
	fprintf(icc_trfp, "   out_apdu->noctets  = %d\n", out_apdu->noctets);
	fprintf(icc_trfp, "   out_apdu->octets   = \n");
	aux_fxdump(icc_trfp, out_apdu->octets, out_apdu->noctets, 0);
	fprintf(icc_trfp, "TRACE-END in icc_OutSecMessAuth\n");
#endif

	return (0);



/*****************************************************************************/
/*
 *      Memory error
 */
errcase:

	out_bits.nbits = 1;
	aux_free2_BitString(&out_bits);
	return (-1);


}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      icc_OutSecMessAuth     */
/*-------------------------------------------------------------*/



/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  icc_OutSecMessConc  VERSION   1.1                */
/*                              DATE   June  1993         */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Encrypt ICC-COMMAND-APDU (without CLA-Byte)           */
/*  This Procedure can be called in case of               */
/*  secure messaging = CONCEALED and in case of           */
/*  secure messaging = COMBINED after calling the         */
/*  procedure icc_OutSecMessAuth.                         */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   sec_key		       Secure Messaging key	  */
/*                             Algorithm Identifier       */
/*			       The following values are   */
/*			       possible:		  */
/*			       desCBC_ISO1	          */
/*							  */
/*   in_apdu		       Pointer of ICC-APDU        */
/*                             The ICC-APDU must have the */
/*                             structur:                  */
/*			       __________________________ */
/*			      | CLA,INS,P1,P2,L,SSC,DATA |*/
/*			       __________________________ */
/*			      (= output of the procedure  */
/*				 icc_create)		  */
/*			       or			  */
/*		           ______________________________ */
/*			  | CLA,INS,P1,P2,L,SSC,DATA,MAC |*/
/*		           ______________________________ */
/*			       (= output of the procedure */
/*				  icc_OutSecMessAuth)     */
/*							  */
/*							  */
/* OUT                                                    */
/*   out_apdu                  Pointer of SEC-APDU        */
/*			       out_apdu->octets will be   */
/*			       allocated by the called    */
/*			       program			  */
/*			       and must be set free by the*/
/*			       calling program            */
/*                             The SEC-APDU has the       */
/*                             structure:                 */
/*		           _____________________          */
/*			  | CLA,ENCRYPTED DATA  |         */
/*		           _____________________          */
/*							  */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   0                         o.k                        */
/*   -1                        Error                      */
/*				EMEMAVAIL		  */
/*				EDESENC  		  */
/*				EALGO    		  */
/*				EPARINC    		  */
/*						          */
/* CALLED FUNCTIONS					  */
/*   des_encrypt                                          */
/*   aux_fxdump                                           */
/*   aux_free2_BitString                                  */
/*   aux_cmp_ObjId                                        */
/*   malloc macro in MF_check.h				  */ 
/*							  */
/*--------------------------------------------------------*/
int
icc_OutSecMessConc(sec_key, in_apdu, out_apdu)
	KeyInfo      	*sec_key;	/* secure messaging key  */
	OctetString     *in_apdu;	/* ICC-APDU		 */
	OctetString     *out_apdu;	/* ICC-SEC-APDU		 */
{
	OctetString     in_octets;
	char           *ptr;
	int             i;
	int             memolen;
	BitString       out_bits;
        char           *proc="icc_OutSecMessConc";


	out_apdu->noctets = 0;
	out_apdu->octets = NULL;
	in_octets.noctets = in_apdu->noctets;
	in_octets.octets = in_apdu->octets;
	out_bits.nbits = 0;
	out_bits.bits  = NULL;



/*****************************************************************************/
/*
 *	test parameter sec_key     
 */
	if ((!sec_key->subjectAI) || (!sec_key->subjectAI->objid)) {
		icc_errno = EALGO;
		icc_errmsg = sca_errlist[icc_errno].msg;
		return (-1);
	}
	if (aux_cmp_ObjId(sec_key->subjectAI->objid,desCBC_ISO1->objid)) {
		icc_errno = EALGO;
		icc_errmsg = sca_errlist[icc_errno].msg;
		return (-1);
	};
	if ((sec_key->subjectkey.nbits <= 0) ||
	    (sec_key->subjectkey.bits == NULL)) {
		icc_errno = EPARINC;
		icc_errmsg = sca_errlist[icc_errno].msg;
		return (-1);
	};


	

/*****************************************************************************/
/*
 * 	encrypt data (INS,P1,P2,L,SSC,DATA/MAC) with Secure Messaging Key       
 */
     

	in_octets.noctets -= 1;
	in_octets.octets++;

#ifdef ICCTRACE 
	fprintf(icc_trfp, "TRACE in icc_OutSecMessConc\n");
	fprintf(icc_trfp, "   in_octets.noctets = %d\n", in_octets.noctets);
	fprintf(icc_trfp, "   in_octets.octets  = \n");
	aux_fxdump(icc_trfp, in_octets.octets, in_octets.noctets, 0);
#endif


	/*
	 * allocate memory for out_bits             
	 * the memory must be a multiple of 8 Bytes 
	 * in case of a multiple - 8 bytes are extra
	 * padded				    
	 */

	if ((in_octets.noctets % 8) != 0)
		memolen = (in_octets.noctets - (in_octets.noctets % 8)) + 8;
	else
		memolen = in_octets.noctets + 8;


#ifdef ICCTRACE 
	fprintf(icc_trfp, "   allocate out_bits = %d\n", memolen);
#endif

	ALLOC_CHAR(out_bits.bits, memolen);	/* will be set free in this
						 * proc. */
	/*
	 * encrypt data
	 */

	memolen = des_encrypt(&in_octets, &out_bits, SEC_END, sec_key);
	if (memolen == -1) {
		icc_errno = EDESENC;
		icc_errmsg = sca_errlist[icc_errno].msg;
		out_bits.nbits = 1;
		aux_free2_BitString(&out_bits);
		return (-1);
	}
#ifdef ICCTRACE 
	fprintf(icc_trfp, "   out_bits.nbits    = %d\n", out_bits.nbits);
	fprintf(icc_trfp, "   out_bits.bits     = \n");
	aux_fxdump(icc_trfp, out_bits.bits, out_bits.nbits / 8, 0);
#endif


/*****************************************************************************/
/*
 * 	create output parameter     
 */
	memolen = (out_bits.nbits / 8) + 1;

	ALLOC_CHAR(out_apdu->octets, memolen);

	out_apdu->noctets = memolen;
	ptr = out_apdu->octets;
	*ptr = *in_apdu->octets;	/* transfer CLA-Byte */
	ptr++;
	for (i = 0; i < (out_bits.nbits / 8); i++) {
		*ptr = *(out_bits.bits + i);
		ptr++;
	};
	out_bits.nbits = 1;
	aux_free2_BitString(&out_bits);

#ifdef ICCTRACE 
	fprintf(icc_trfp, "   out_apdu->noctets  = %d\n", out_apdu->noctets);
	fprintf(icc_trfp, "   out_apdu->octets   = \n");
	aux_fxdump(icc_trfp, out_apdu->octets, out_apdu->noctets, 0);
	fprintf(icc_trfp, "TRACE-END in icc_OutSecMessConc\n");
#endif


	return (0);


/*****************************************************************************/
/*
 *      Memory error
 */
errcase:

	out_bits.nbits = 1;
	aux_free2_BitString(&out_bits);
	return (-1);




}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      icc_OutSecMessConc     */
/*-------------------------------------------------------------*/


/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  icc_InSecMessAuth   VERSION   1.1                */
/*                              DATE   June     1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Check MAC  and SSC of a received ICC-RESPONSE-APDU    */
/*  This procedure can be called in case of               */
/*  secure messaging = AUTHENTIC or in case of            */
/*  secure messaging = COMBINED after calling the         */
/*  procedure icc_InSecMessConc.                          */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   sec_key		       Secure Messaging key	  */
/*                             Algorithm Identifier       */
/*			       The following values are   */
/*			       possible:		  */
/*			       desCBC_ISO1	          */
/*							  */
/*   ssc		       Send Sequence Counter      */
/*							  */
/*   in_apdu		       Pointer of SEC-APDU        */
/*                             The ICC-APDU must have the */
/*                             structur:                  */
/*			       ________________________   */
/*			      | SSC,DATA,MAC,SW1,SW2   |  */
/*			       ________________________   */
/*                                                        */
/*							  */
/*   maclen		       Length of MAC (0 - 8)      */
/*			       In the current Version     */
/*  			       only 4 is allowed          */
/*                                                        */
/* OUT                                                    */
/*   out_apdu                  Pointer of ICC-APDU        */
/*			       (without SSC and MAC)      */
/*			       out_apdu->octets will be   */
/*			       allocated by the called    */
/*			       program			  */
/*			       and must be set free by the*/
/*			       calling program            */
/*			       The APDU has the structure:*/
/*		                _________________         */
/*			       | DATA,SW1,SW2    |        */
/*		                _________________         */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   0                         o.k                        */
/*   -1                        Error                      */
/*				EMEMAVAIL		  */
/*				EDESENC  		  */
/*				ESSC			  */
/*				ERECMAC			  */
/*				EPARINC			  */
/*				EALGO	          	  */
/*						          */
/* CALLED FUNCTIONS					  */
/*   des_encrypt                                          */
/*   aux_fxdump                                           */
/*   aux_free2_OctetString				  */
/*   aux_free2_BitString				  */
/*   aux_cmp_ObjId                                        */
/*   malloc macro in MF_check.h				  */ 
/*							  */
/*--------------------------------------------------------*/
int
icc_InSecMessAuth(sec_key, ssc, in_apdu, out_apdu, maclen)
	KeyInfo     	*sec_key;	/* secure messaging key  */
	int             ssc;		/* Send sequence Counter */
	OctetString     *in_apdu;	/* SEC-APDU		 */
	OctetString     *out_apdu;	/* ICC-APDU		 */
	int             maclen;		/* Length of MAC	 */
{
	OctetString     in_octets;
	char           *ptr, *apdu_ptr, *mac_ptr;
	int             i;
	int             memolen;
	BitString       out_bits;
	int             rec_ssc;
	char           *mac_field;
        char           *proc="icc_InSecMessAuth";


	out_bits.nbits	= 0;
	out_bits.bits	= NULL;
	in_octets.noctets = 0;
	in_octets.octets  = NULL;
	

/*****************************************************************************/
/*
 * 	test parameter maclen and sec_key       
 */
	if (maclen != 4) {
		icc_errno = EPARINC;
		icc_errmsg = sca_errlist[icc_errno].msg;
		return (-1);
	}
	if ((!sec_key->subjectAI) || (!sec_key->subjectAI->objid)) {
		icc_errno = EALGO;
		icc_errmsg = sca_errlist[icc_errno].msg;
		return (-1);
	}
	if (aux_cmp_ObjId(sec_key->subjectAI->objid,desCBC_ISO1->objid)) {
		icc_errno = EALGO;
		icc_errmsg = sca_errlist[icc_errno].msg;
		return (-1);
	};
	if ((sec_key->subjectkey.nbits <= 0) ||
	    (sec_key->subjectkey.bits == NULL)) {
		icc_errno = EPARINC;
		icc_errmsg = sca_errlist[icc_errno].msg;
		return (-1);
	};


/*****************************************************************************/
/*
 *	 encrypt data (SSC,DATA,SW1,SW2) with Secure Messaging Key        
 */
	out_apdu->noctets = 0;
	out_apdu->octets = NULL;

#ifdef ICCTRACE 
	fprintf(icc_trfp, "TRACE in icc_InSecMessAuth\n");
	fprintf(icc_trfp, "   in_apdu->noctets   = %d\n", in_apdu->noctets);
	fprintf(icc_trfp, "   in_apdu->octets    = \n");
	aux_fxdump(icc_trfp, in_apdu->octets, in_apdu->noctets, 0);
	fprintf(icc_trfp, "   sec_key->subjectkey.nbits    = %d\n", sec_key->subjectkey.nbits);
	fprintf(icc_trfp, "   sec_key->subjectkey.bits     = \n");
	aux_fxdump(icc_trfp, sec_key->subjectkey.bits, sec_key->subjectkey.nbits / 8, 0);
#endif



	if (in_apdu->noctets == 2) {
		/* only SW1,SW2 received */
		out_apdu->noctets = in_apdu->noctets;


		ALLOC_CHAR(out_apdu->octets,out_apdu->noctets);	

		/* copy SW1,SW2 into out_apdu->octets */
		ptr = out_apdu->octets;
		for (i = 0; i < 2; i++)
			*ptr++ = *(in_apdu->octets + i);

#ifdef ICCTRACE 
		fprintf(icc_trfp, "   out_apdu->noctets  = %d\n", out_apdu->noctets);
		fprintf(icc_trfp, "   out_apdu->octets   = \n");
		aux_fxdump(icc_trfp, out_apdu->octets, out_apdu->noctets, 0);
		fprintf(icc_trfp, "TRACE-END in icc_InSecMessAuth\n");
#endif

		return (0);
	}

	in_octets.noctets = in_apdu->noctets - maclen;	
	mac_field = in_apdu->octets + (in_apdu->noctets - (maclen + 2));

	ALLOC_CHAR(in_octets.octets,in_octets.noctets);	/* will be set free in
							 * this proc. */

	/* copy SSC,DATA,SW1,SW2 from in_apdu->octets into in_octets.octets */
	ptr = in_octets.octets;
	apdu_ptr = in_apdu->octets;
	*ptr = *apdu_ptr++;			/* SSC		 */
	rec_ssc = *ptr & 0xFF;
	ptr++;

	for (i = 0; i < in_octets.noctets - 3; i++) {	/* Data		 */
		*ptr = *apdu_ptr++;
		ptr++;
	}

	apdu_ptr = in_apdu->octets + (in_apdu->noctets - 2);
	*ptr++ = *apdu_ptr++;	/* SW1		 */
	*ptr = *apdu_ptr;	/* SW2		 */




#ifdef ICCTRACE 
	fprintf(icc_trfp, "   in_octets.noctets = %d\n", in_octets.noctets);
	fprintf(icc_trfp, "   in_octets.octets  = \n");
	aux_fxdump(icc_trfp, in_octets.octets, in_octets.noctets, 0);
	fprintf(icc_trfp, "   rec_ssc           = %x\n", rec_ssc);
	fprintf(icc_trfp, "   akt_ssc           = %x\n", (ssc & 0xFF) % 256);
	fprintf(icc_trfp, "   mac_field         = \n");
	aux_fxdump(icc_trfp, mac_field, maclen, 0);
#endif






	/*
	 * allocate memory for out_bits             
	 * the memory must be a multiple of 8 Bytes 
	 * in case of a multiple - 8 bytes are extra
	 * padded				    
	 */

	if ((in_octets.noctets % 8) != 0)
		memolen = (in_octets.noctets - (in_octets.noctets % 8)) + 8;
	else
		memolen = in_octets.noctets + 8;


#ifdef ICCTRACE 
	fprintf(icc_trfp, "   allocate out_bits = %d\n", memolen);
#endif

	ALLOC_CHAR(out_bits.bits, memolen);	/* will be set free in this
						 * proc. */


	/*
	 * encrypt data
	 */
	memolen = des_encrypt(&in_octets, &out_bits, SEC_END, sec_key);
	if (memolen == -1) {
		icc_errno = EDESENC;
		icc_errmsg = sca_errlist[icc_errno].msg;
		in_octets.noctets = 1;
		aux_free2_OctetString(&in_octets);
		out_bits.nbits = 1;
		aux_free2_BitString(&out_bits);
		return (-1);
	}
#ifdef ICCTRACE 
	fprintf(icc_trfp, "   out_bits.nbits    = %d\n", out_bits.nbits);
	fprintf(icc_trfp, "   out_bits.bits     = \n");
	aux_fxdump(icc_trfp, out_bits.bits, out_bits.nbits / 8, 0);
#endif

/*****************************************************************************/
/*
 *      check SSC
 */
	if (rec_ssc != ((ssc & 0xFF) % 256)) {
		icc_errno = ESSC;
		icc_errmsg = sca_errlist[icc_errno].msg;
		in_octets.noctets = 1;
		aux_free2_OctetString(&in_octets);
		out_bits.nbits = 1;
		aux_free2_BitString(&out_bits);

		return (-1);
	}
/*****************************************************************************/
/*
 *      check MAC							 
 *	if only 1 block encrypted => take the first 4 Bytes for MAC   
 *	else take the last 4 octets of the last block		
 */

	if ((out_bits.nbits / 8) > 8)
		mac_ptr = out_bits.bits + ((out_bits.nbits / 8) - 8);
	else
		mac_ptr = out_bits.bits;

#ifdef ICCTRACE 
	fprintf(icc_trfp, "   mac_ptr           = \n");
	aux_fxdump(icc_trfp, mac_ptr, maclen, 0);
#endif


	for (i = 0; i < maclen; i++) {
		if (mac_field[i] != *(mac_ptr + i)) {
			icc_errno = ERECMAC;
			icc_errmsg = sca_errlist[icc_errno].msg;
			in_octets.noctets = 1;
			aux_free2_OctetString(&in_octets);
			out_bits.nbits = 1;
			aux_free2_BitString(&out_bits);
			return (-1);
		}
	}



/*****************************************************************************/
/*
 *      create output parameter							 
 */
	ALLOC_CHAR(out_apdu->octets , in_octets.noctets - 1); /* without SSC Bytes */	
	out_apdu->noctets = in_octets.noctets - 1;
	ptr = out_apdu->octets;
	for (i = 1; i < in_octets.noctets; i++) {
		*ptr = *(in_octets.octets + i);
		ptr++;
	}
	in_octets.noctets = 1;
	aux_free2_OctetString(&in_octets);
	out_bits.nbits = 1;
	aux_free2_BitString(&out_bits);

#ifdef ICCTRACE 
	fprintf(icc_trfp, "   out_apdu->noctets  = %d\n", out_apdu->noctets);
	fprintf(icc_trfp, "   out_apdu->octets   = \n");
	aux_fxdump(icc_trfp, out_apdu->octets, out_apdu->noctets, 0);
	fprintf(icc_trfp, "TRACE-END in icc_InSecMessAuth\n");
#endif

	return (0);

/*****************************************************************************/
/*
 *      Memory error
 */
errcase:

	in_octets.noctets = 1;
	out_bits.nbits = 1;
	aux_free2_OctetString(&in_octets);
	aux_free2_BitString(&out_bits);
	return (-1);

}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      icc_InSecMessAuth      */
/*-------------------------------------------------------------*/


/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  icc_InSecMessConc   VERSION   1.1                */
/*                              DATE   June     1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Decrypt SEC-RESPONSE-APDU                             */
/*  This procedure can be called in case of               */
/*  secure messaging = CONCEALED or in case of            */
/*  secure messaging = COMBINED before calling the        */
/*  procedure icc_InSecMessAuth.		          */
/*  In case of secure messaging = CONCEALED this procedure*/
/*  also checks the SSC.				  */
/*  In case of secure messaging = COMBINED the calling    */
/*  procedure must set the parameter maclen.              */
/*  This parameter will be need to allocate the           */
/*  buffer out_apdu->octets correctly.			  */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   sec_key		       Secure Messaging key	  */
/*                             Algorithm Identifier       */
/*			       The following values are   */
/*			       possible:		  */
/*			       desCBC_ISO1	          */
/*							  */
/*   sec_mode  		       security mode              */
/*							  */
/*   ssc		       Send Sequence Counter      */
/*			       only used in case of       */
/*			       sec_mode = CONCEALED       */
/*							  */
/*   in_apdu		       Pointer of SEC-APDU        */
/*			       The SEC-APDU have the      */
/*                             structure	          */
/*                             _________________          */
/*			      | ENCRYPTED DATA  |         */
/*		               _________________          */
/*			       or			  */
/*		                _________________         */
/*			       | SW1,SW2         |        */
/*		                _________________         */
/*							  */
/* OUT                                                    */
/*   out_apdu                  Pointer of ICC-APDU        */
/*			       out_apdu->octets will be   */
/*			       allocated by the called    */
/*			       program			  */
/*			       and must be set free by the*/
/*			       calling program            */
/*			       The APDU has the structure:*/
/*		                _________________         */
/*			       | DATA,SW1,SW2    |        */
/*		                _________________         */
/*                             or                         */
/*			       ________________________   */
/*			      | SSC,DATA,MAC,SW1,SW2   |  */
/*			       ________________________   */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   0                         o.k                        */
/*   -1                        Error                      */
/*				EMEMAVAIL		  */
/*				EDESDEC  		  */
/*				ESSC			  */
/*				EALGO	                  */
/*				EPARINC    		  */
/*						          */
/* CALLED FUNCTIONS					  */
/*   des_decrypt                                          */
/*   aux_fxdump                                           */
/*   aux_free2_OctetString				  */
/*   aux_cmp_ObjId                                        */
/*   malloc macro in MF_check.h				  */ 
/*							  */
/*--------------------------------------------------------*/
int
icc_InSecMessConc(sec_key, sec_mode, ssc, in_apdu, out_apdu)
	KeyInfo      	*sec_key;	/* secure messaging key  */
	ICC_SecMessMode  sec_mode;	/* security mode         */
	int             ssc;		/* Send sequence Counter */
	OctetString     *in_apdu;	/* SEC-APDU		 */
	OctetString     *out_apdu;	/* ICC-APDU		 */
{
	OctetString     out_octets;
	char           *ptr, *apdu_ptr;
	int             i;
	int             memolen;
	BitString       in_bits;
	int             rec_ssc;
	char		*proc="icc_InSecMessConc";


	out_octets.noctets = 0;
	out_octets.octets  = NULL;


/*****************************************************************************/
/*
 * 	test parameter sec_key     
 */
	if ((!sec_key->subjectAI) || (!sec_key->subjectAI->objid)) {
		icc_errno = EALGO;
		icc_errmsg = sca_errlist[icc_errno].msg;
		return (-1);
	}

	if (aux_cmp_ObjId(sec_key->subjectAI->objid,desCBC_ISO1->objid)) {
		icc_errno = EALGO;
		icc_errmsg = sca_errlist[icc_errno].msg;
		return (-1);
	};
	if ((sec_key->subjectkey.nbits <= 0) ||
	    (sec_key->subjectkey.bits == NULL)) {
		icc_errno = EPARINC;
		icc_errmsg = sca_errlist[icc_errno].msg;
		return (-1);
	};


/*****************************************************************************/
/*
 *      decrypt data with Secure Messaging Key
 */

#ifdef ICCTRACE 
	fprintf(icc_trfp, "TRACE in icc_InSecMessConc\n");
	fprintf(icc_trfp, "   in_apdu->noctets   = %d\n", in_apdu->noctets);
	fprintf(icc_trfp, "   in_apdu->octets    = \n");
	aux_fxdump(icc_trfp, in_apdu->octets, in_apdu->noctets, 0);
#endif

	if (in_apdu->noctets == 2) {
		/* only SW1,SW2 received */
		out_apdu->noctets = in_apdu->noctets;

		ALLOC_CHAR(out_apdu->octets, out_apdu->noctets);

		/* copy SW1,SW2 into out_apdu->octets */
		ptr = out_apdu->octets;
		for (i = 0; i < 2; i++) {
			*ptr = *(in_apdu->octets + i);
			ptr++;
		}

#ifdef ICCTRACE 
		fprintf(icc_trfp, "   out_apdu->noctets  = %d\n", out_apdu->noctets);
		fprintf(icc_trfp, "   out_apdu->octets   = \n");
		aux_fxdump(icc_trfp, out_apdu->octets, out_apdu->noctets, 0);
		fprintf(icc_trfp, "TRACE-END in icc_InSecMessConc\n");
#endif

		return (0);
	}

	/* allocate memory for out_octets  */

	ALLOC_CHAR(out_octets.octets,in_apdu->noctets);	/* will be set free in
							 * this proc. */


	in_bits.nbits = in_apdu->noctets * 8;
	in_bits.bits = in_apdu->octets;
	memolen = des_decrypt(&in_bits, &out_octets, SEC_END, sec_key);

	if (memolen == -1) {
		icc_errno = EDESDEC;
		icc_errmsg = sca_errlist[icc_errno].msg;
		out_octets.noctets = 1;
		aux_free2_OctetString(&out_octets);
		return (-1);
	}
#ifdef ICCTRACE 
	fprintf(icc_trfp, "   out_octets.noctets= %d\n", out_octets.noctets);
	fprintf(icc_trfp, "   out_octets.octets = \n");
	aux_fxdump(icc_trfp, out_octets.octets, out_octets.noctets, 0);
#endif

	if (sec_mode == ICC_CONCEALED) {
		/*
		 * CONCEALED-Mode -> Test SSC; return DATA,SW1,SW2 
		 * allocate out_data->octets                      
		 */
		out_apdu->noctets = out_octets.noctets - 1;	
		ALLOC_CHAR(out_apdu->octets, out_apdu->noctets);
		/*
		 * copy DATA,SW1,SW2 from out_octets.octets into
		 * out_apdu->octets
		 */
		ptr = out_apdu->octets;
		apdu_ptr = out_octets.octets;
		rec_ssc = *apdu_ptr++ & 0xFF;		/* SSC		 */

#ifdef ICCTRACE 
		fprintf(icc_trfp, "   rec_ssc           = %x\n", rec_ssc);
		fprintf(icc_trfp, "   akt_ssc           = %x\n", (ssc & 0xFF) % 256);
#endif

		/* check SSC	 */
		if (rec_ssc != ((ssc & 0xFF) % 256)) {
			icc_errno = ESSC;
			icc_errmsg = sca_errlist[icc_errno].msg;
			out_octets.noctets = 1;
			out_apdu->noctets = 1;
			aux_free2_OctetString(&out_octets);
			aux_free2_OctetString(out_apdu);
			return (-1);
		}
		for (i = 0; i < out_octets.noctets - 1; i++) {	/* Data, SW1, SW2 */
			*ptr = *apdu_ptr++;
			ptr++;
		}

	} else {
		/*
		 * COMBINED-Mode-> return SSC,DATA,MAC,SW1,SW2     
		 * allocate out_data->octets                       
		 */
		out_apdu->noctets = out_octets.noctets;

		out_apdu->octets = out_octets.octets;	

	}
	out_octets.noctets = 1;
	aux_free2_OctetString(&out_octets);

#ifdef ICCTRACE 
	fprintf(icc_trfp, "   out_apdu->noctets  = %d\n", out_apdu->noctets);
	fprintf(icc_trfp, "   out_apdu->octets   = \n");
	aux_fxdump(icc_trfp, out_apdu->octets, out_apdu->noctets, 0);
	fprintf(icc_trfp, "TRACE-END in icc_InSecMessConc\n");
#endif

	return (0);


/*****************************************************************************/
/*
 *      Memory error
 */
errcase:

	out_octets.noctets = 1;
	aux_free2_OctetString(&out_octets);
	return (-1);

}





/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      icc_InSecMessConc      */
/*-------------------------------------------------------------*/

/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  icc_giveParmList     VERSION   1.1               */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Return Parameter list                                 */
/*                                                        */
/*                                                        */
/* IN / OUT                  DESCRIPTION                  */
/*   icc_parmlist               ICC-parameter list        */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   0                         o.k                        */
/*   -1                        Error                      */
/*			        EINS                      */
/*						          */
/* CALLED FUNCTIONS					  */
/*--------------------------------------------------------*/
int
icc_giveParmList (icc_parmlist)
	struct ICC_ParmList *icc_parmlist;	
{
	unsigned int    i, j;
	int		index, listlen;
	Boolean		ins_found = FALSE;

	
/*****************************************************************************/
/*
 *      search inscode in cmdlist
 */
	listlen = sizeof(icc_cmdlist) / sizeof(ICC_ParmList);
	for (index = 0; index < listlen; index++) {

		if (icc_cmdlist[index].inscode == 
		    icc_parmlist->inscode) {
			ins_found = TRUE;
			break;
		}
	}
/*****************************************************************************/
/*
 *      test, if inscode found
 */
	if (ins_found == FALSE) {
		icc_errno = EINS;
		icc_errmsg = sca_errlist[icc_errno].msg;
		return (-1);
	}
/*****************************************************************************/
/*
 *      fill up parameter list
 */
	icc_parmlist->parm_no = icc_cmdlist[index].parm_no;
	icc_parmlist->parmlen = 0;
	j = 0;
	for (i=0; i<icc_cmdlist[index].parm_no; i++) {
		/* some parameter types will not be returned */
	   	switch (icc_cmdlist[index].parmtype[i]) {
		case ICC_PARM_LOCKCAT:
		case ICC_PARM_NOTUSED:
		case ICC_PARM_LEN:
		case ICC_PARM_INSTMODE:
			icc_parmlist->parm_no--;
			break;
		default:
			if (icc_cmdlist[index].parmtype[i] == ICC_PARM_CHGPIN) {
				switch (icc_parmlist->inscode) {
				case ICC_VER_CHG:
					icc_parmlist->parmtype[j] = ICC_PARM_data;
					j++;
					icc_parmlist->parm_no++;	
					icc_parmlist->parmtype[j] = ICC_PARM_new_pin;
					break;
				}
			}
			else {
				if (icc_cmdlist[index].parmtype[i] == ICC_PARM_keyattrlist) {
					switch (icc_parmlist->inscode) {
					case ICC_WR_KEY:
						icc_parmlist->parm_no++;	
						icc_parmlist->parmtype[j] = ICC_PARM_key_algid;
						j++;
						icc_parmlist->parmtype[j] = icc_cmdlist[index].parmtype[i];
						j++;
						icc_parmlist->parm_no++;	
						icc_parmlist->parmtype[j] = ICC_PARM_data;						
						break;
					}
				}

				else
					icc_parmlist->parmtype[j] = icc_cmdlist[index].parmtype[i];
			}

			if (icc_cmdlist[index].parmtype[i] == ICC_PARM_datasel) {
				switch (icc_parmlist->inscode) {
				case ICC_READR:
				case ICC_WRITER:
				case ICC_DELREC:
					j++;
					icc_parmlist->parm_no++;	
					icc_parmlist->parmtype[j] = ICC_PARM_short_index;
					break;
				}
				if ((icc_parmlist->inscode == ICC_READR) || 
				    (icc_parmlist->inscode == ICC_READB)) {
					j++;
					icc_parmlist->parm_no++;	
					icc_parmlist->parmtype[j] = ICC_PARM_lrddata;
				}
			}
				
			j++;
			break;
		    
		};
	};
	return(0);
}
/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      icc_giveParmList        */
/*-------------------------------------------------------------*/

		 


/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  icc_e_KeyId         VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Create the key identifier byte                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   kid                       kid structure              */
/*                                                        */
/*                                                        */
/* OUT                                                    */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   char                      Value of Byte              */
/*						          */
/* CALLED FUNCTIONS					  */
/*--------------------------------------------------------*/
char
icc_e_KeyId(kid)
	KeyId          *kid;
{
	char            kidvalue = 0x00;

	if (!kid) 
		return(kidvalue);

	kidvalue = (((char) kid->key_number & 0xFF) << 3) |
		((char) kid->key_level & 0xff);

	return (kidvalue);


}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      icc_e_KeyId            */
/*-------------------------------------------------------------*/



/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  icc_e_FileId         VERSION   1.1               */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Create the second file identifier byte                */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   fid                       file id structure          */
/*                                                        */
/*                                                        */
/* OUT                                                    */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   char                      Value of Byte              */
/*						          */
/*   -1			       Error			  */
/*				 EPARINC		  */ 
/*						          */
/* CALLED FUNCTIONS					  */
/*--------------------------------------------------------*/
char
icc_e_FileId(fid)
	FileId         *fid;
{
	char            fidvalue;

	if ((!fid) ||
	    (fid->short_index > 15) ||
	    (fid->short_index < 1) ||
	    (fid->file_type > ISF) ||
	    (fid->file_type < XEF)) {
		icc_errno = EPARINC;
		icc_errmsg = sca_errlist[icc_errno].msg;
		return (-1);
	};

	fidvalue = (((char) fid->file_type & 0xFF) << 6) |
		(((char) fid->file_level & 0xFF) << 4) |
		((char) fid->short_index & 0xff);

	return (fidvalue);


}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      icc_e_FileId           */
/*-------------------------------------------------------------*/



/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  icc_e_KeyAttrList   VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  create parameter KATTR1,KATTR2,KFPC,KSTAT in APDU     */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   apdu                      Pointer of APDU-buffer     */
/*                                                        */
/*   keyattrlist               Pointer of KEYattrlist     */
/*                             structure                  */
/*   key_algid                 Algorithm identifier       */
/*   						          */
/* OUT                                                    */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*						          */
/* CALLED FUNCTIONS					  */
/*--------------------------------------------------------*/
void
icc_e_KeyAttrList(apdu, keyattrlist, key_algid)
	char          **apdu;
	KeyAttrList    *keyattrlist;
	KeyAlgId        key_algid;
{
	char           *p;
	unsigned        kattr;
	unsigned        bit;
	unsigned	purpose;
	Boolean		key_repl_flag, key_chg_flag, key_foreign_flag;

	p = *apdu;
	kattr = 0x00;

	if (!keyattrlist) {
		*p++ = (char) kattr;
		*p++ = (char) kattr;
		*p++ = (char) kattr;
		*p++ = (char) kattr;
	}
	else {

/*****************************************************************************/
/*
 *      create KATTR - Bytes
 */

	/* 1. Byte    */

	if (keyattrlist->key_attr.key_repl_flag == TRUE)
		key_repl_flag = FALSE;
	else
		key_repl_flag = TRUE;

	if (keyattrlist->key_attr.key_chg_flag == TRUE)
		key_chg_flag = FALSE;
	else
		key_chg_flag = TRUE;

	if (keyattrlist->key_attr.key_foreign_flag == TRUE)
		key_foreign_flag = FALSE;
	else
		key_foreign_flag = TRUE;



	kattr |= (unsigned) key_repl_flag << 7 |
		(unsigned) key_chg_flag << 6 |
		(unsigned) key_foreign_flag  << 5 |
		(unsigned) keyattrlist->key_attr.key_purpose;




	*p++ = (char) kattr;

	/* 2. Byte    */
	kattr = 0x00;
	bit = 0x00;
	bit = (unsigned) key_algid << 2;

	kattr = bit | (ICC_NOTUSED << 1) |
		(unsigned) keyattrlist->key_attr.key_presentation;


	if (keyattrlist->key_attr.MAC_length == 0)
		kattr |= (unsigned) (keyattrlist->key_attr.MAC_length) << 5;
	else
		kattr |= (unsigned) (keyattrlist->key_attr.MAC_length - 1) << 5;
	*p++ = (char) kattr;

/*****************************************************************************/
/*
 *      create KFPC - Byte
 */
	kattr = 0x00;
	if (keyattrlist->key_fpc != 0) {
		kattr = (unsigned) keyattrlist->key_fpc;
		kattr |= (unsigned) keyattrlist->key_fpc << 4;
	}
	*p++ = (char) kattr;

/*****************************************************************************/
/*
 *      create KSTAT - Byte
 */
	kattr = 0x00;
	kattr = (unsigned) keyattrlist->key_status.key_state << 1 |
		(unsigned) keyattrlist->key_status.PIN_check;
	*p++ = (char) kattr;

	}


	*apdu = p;
}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      icc_e_KeyAttrList      */
/*-------------------------------------------------------------*/


/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  icc_check_KeyId     VERSION   1.1                */
/*                              DATE   July  1993         */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Check key identifier                                  */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   key_id                    structur of key identifier */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   0                          o.k                       */
/*  -1                          error                     */
/*                               EPARINC                  */
/*						          */
/* CALLED FUNCTIONS					  */
/*--------------------------------------------------------*/
int
icc_check_KeyId(key_id)
	KeyId	*key_id;
{
	if (key_id == NULL) {
		icc_errno = EPARINC;
		icc_errmsg = sca_errlist[icc_errno].msg;
		return (-1);
	}

	if (((key_id->key_level != FILE_MASTER) &&
	     (key_id->key_level != FILE_CURRENT) &&
	     (key_id->key_level != FILE_PARENT)) ||
	    ((key_id->key_number < 1) ||
	     (key_id->key_number > 31))) {
		icc_errno = EPARINC;
		icc_errmsg = sca_errlist[icc_errno].msg;
		return (-1);
	}


	return (0);
}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      icc_check_KeyId        */
/*-------------------------------------------------------------*/






/*****************************************************************************/
/*	local functions for the iccde-functions                              */





/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  e_OperationMode     VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Transform the values for Operation Mode               */
/*  into one Byte and returns its value.                  */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   finfo                     Pointer to Filecontrolinfo */
/*                             structure                  */
/*                                                        */
/* OUT                                                    */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   char                      Value of Byte              */
/*						          */
/* CALLED FUNCTIONS					  */
/*--------------------------------------------------------*/
static char
e_OperationMode(finfo)
	FileControlInfo *finfo;
{
	char            opmode = 0x00;


	opmode = (((char) finfo->not_erasable & 0xFF) << 7) |
		 (((char) finfo->access_sec_mode & 0xFF) << 5) |
		 (((char) finfo->execute & 0xFF) << 2) |
		 ((char) finfo->readwrite & 0xFF);
	return (opmode);


}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      e_OperationMode        */
/*-------------------------------------------------------------*/


/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  e_AccessControl     VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Transform the values for AccessControlValue           */
/*  into one Byte and returns its value.                  */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   acv_struc                 Pointer to AccessControlValue */
/*                             structure                  */
/*                                                        */
/* OUT                                                    */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   char                      Value of Byte              */
/*						          */
/* CALLED FUNCTIONS					  */
/*--------------------------------------------------------*/
static char
e_AccessControl(acv_struc)
	AccessControlValue acv_struc;
{
	char            acv = 0x00;



	acv    = (((char) acv_struc.compare_mode & 0xFF) << 6) |
		 ((char) acv_struc.state_number & 0xFF);

	return (acv);


}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      e_AccessControl        */
/*-------------------------------------------------------------*/





/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  e_TwoByte           VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Create two octets in scapdu                            */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   unit                      units-value                */
/*                                                        */
/*                                                        */
/* INOUT                                                  */
/*   io                        Pointer of buffer          */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*						          */
/* CALLED FUNCTIONS					  */
/*--------------------------------------------------------*/
static void
e_TwoByte(io, units)
	char          **io;
	unsigned        units;
{
	char           *p;


	p = *io;
	if (units < 255) {
		*p++ = 0x00;
		*p++ = units;
	} else {
		*p++ = units >> 8;
		*p++ = units;
	};
	*io = p;


}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      e_TwoByte              */
/*-------------------------------------------------------------*/

/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  e_AuthControlPar    VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  create the acp parameter                              */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   acp                       acp-structure              */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   char                      Value of acp-byte          */
/*						          */
/* CALLED FUNCTIONS					  */
/*--------------------------------------------------------*/
static char
e_AuthControlPar(acp_struc)
	AuthControlPar acp_struc;
{
	char            acp = 0x00;



	acp    = ((char) acp_struc.auth_method & 0xFF)  |
		 ((char) acp_struc.key_search & 0xFF);

	return (acp);


}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      e_AuthControlPar       */
/*-------------------------------------------------------------*/




/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  Ioput               VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Put one byte in scabdu                                */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   byte                      Byte                       */
/*                                                        */
/*                                                        */
/* INOUT                                                  */
/*   io                        Pointer of buffer          */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*						          */
/* CALLED FUNCTIONS					  */
/*--------------------------------------------------------*/
static void
Ioput(io, byte_c)
	char          **io;
	unsigned        byte_c;
{
	char           *p;

	p = *io;
	*p = byte_c;
	p++;
	*io = p;
}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      Ioput                  */
/*-------------------------------------------------------------*/



/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  Ioputbuff           VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Put more than one byte in scapdu                      */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   buff                      Pointer of databuffer      */
/*                                                        */
/*   len                       Length of data             */
/*                                                        */
/*                                                        */
/* INOUT                                                  */
/*   io                        Pointer of scapdu          */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*						          */
/* CALLED FUNCTIONS					  */
/*--------------------------------------------------------*/
static void
Ioputbuff(io, buff, len)
	char          **io;
	char           *buff;
	unsigned        len;
{
	char           *p;
	int             i;

	p = *io;
	for (i = 0; i < len; i++) {
		*p = *buff;
		p++;
		buff++;
	}
	*io = p;
}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      Ioputbuff              */
/*-------------------------------------------------------------*/



/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  ICCalloc             VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Allocate buffer                                       */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   pdulen                     length of scapdu          */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   pointer                   o.k.                       */
/*                                                        */
/*   NULL                      error                      */
/*                              EMEMAVAIL                 */
/* CALLED FUNCTIONS					  */
/*   malloc macro in MF_check.h				  */ 
/*							  */
/*--------------------------------------------------------*/
static char    *
ICCalloc(pdulen)
	unsigned int    pdulen;
{
	char           *buffer = NULL;
	char		*proc="ICCalloc";


	buffer = malloc(pdulen);/* must be set free in calling procedure */
	if (buffer == NULL) {
		icc_errno = EMEMAVAIL;
		icc_errmsg = sca_errlist[icc_errno].msg;
	}


	return (buffer);
}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      ICCalloc                */
/*-------------------------------------------------------------*/





/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  ICCcheck_len          VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Check length of parameter                             */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   len                       length                     */
/*                                                        */
/*   maxlen                    maximal length             */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   0                          o.k                       */
/*  -1                          error                     */
/*                               ETOOLONG                 */
/*						          */
/* CALLED FUNCTIONS					  */
/*--------------------------------------------------------*/
static int
ICCcheck_len(len, maxlen)
	unsigned int    len;
{

	if ((len < 0) || (len > maxlen)) {
		icc_errno = ETOOLONG;
		icc_errmsg = sca_errlist[icc_errno].msg;
		return (-1);
	}
	return (0);
}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      ICCcheck_len            */
/*-------------------------------------------------------------*/

/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  ICCcheck_FileSel     VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Check FileSel Parameter                               */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   ins                       instruction code           */
/*                                                        */
/*   file_cat                  file category              */
/*                                                        */
/*   file_sel                  structur of file           */
/*                                                        */
/*                                                        */
/* OUT                        DESCRIPTION                 */
/*   len                       length of file name or     */
/*			       file identifier            */
/*							  */
/*   fid		       encoded file index in      */
/*			       case of file_cat = EF      */	
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   0                          o.k                       */
/*  -1                          error                     */
/*                               EPARINC                  */
/*						          */
/* CALLED FUNCTIONS					  */
/*   ICCcheck_len              Error                      */
/*			        ETOOLONG		  */
/*   icc_e_FileId              Error                      */
/*				EPARINC 		  */ 
/*--------------------------------------------------------*/
static int
ICCcheck_FileSel(ins, file_cat,file_sel,len,fid)
	unsigned int	ins;
	FileCat		file_cat;
	FileSel		*file_sel;
	unsigned int    *len;
	char		*fid;
{
	int	max_len;



		switch (file_cat) {
			case PARENT_DF:
				max_len = MAX_FILENAME;
				break;
			case CHILD_DF:
			case MF:
				max_len = MAX_FILEID;
				break;
		}

		if (file_cat != EF) {
			/*
			 * in case of MF only '3F 00' is allowed
			 * in case of CHILD_DF and PARENT_DF
			 * the first byte must be <> '20'
			 */
			if ((file_sel->file_name->octets == NULL) ||
			    (file_sel->file_name->noctets == 0) ||
			    (*file_sel->file_name->octets == 0x20)) {
				icc_errno = EPARINC;
				icc_errmsg = sca_errlist[icc_errno].msg;
				return (-1);
			}

			if (ICCcheck_len(file_sel->file_name->noctets, max_len) == -1)
				return (-1);

			if ((file_cat == MF) &&
			    (memcmp(file_sel->file_name->octets,MF_NAME,2) != 0)) {
				icc_errno = EPARINC;
				icc_errmsg = sca_errlist[icc_errno].msg;
				return (-1);
			}


			*len = file_sel->file_name->noctets;
		} 
		else {
			if ((*fid = icc_e_FileId(file_sel->file_id)) == -1)
				return (-1);

			if (ins == ICC_DELF) {
				if ((file_sel->file_id->file_type != XEF) &&
				    (file_sel->file_id->file_type != WEF) &&
				    (file_sel->file_id->file_type != ISF)) {
					icc_errno = EPARINC;
					icc_errmsg = sca_errlist[icc_errno].msg;
					return (-1);
				}

			}

			*len = 2;
		}

}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      ICCcheck_FileSel        */
/*-------------------------------------------------------------*/

static
int
sta_itoa(s,n)			     /* from integer to character string */
char s[];
int n;
{
    int c,i,j, sign;

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
} /* end sta_itoa */



/*-------------------------------------------------------------*/
/* E N D   O F   P A C K A G E       iccde                     */
/*-------------------------------------------------------------*/
