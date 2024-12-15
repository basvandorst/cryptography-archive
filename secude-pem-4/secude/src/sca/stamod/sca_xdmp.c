/*
 *  STAMOD Release 1.1 (GMD)
 */
/*-------------------------------------------------------+-----*/
/*                                                       | GMD */
/*   SYSTEM   STARMOD  -  Version 1.1                    +-----*/
/*                                                             */
/*-------------------------------------------------------------*/
/*                                                             */
/*    PACKAGE   XDUMP                     VERSION 1.1          */
/*                              	   DATE   January 1993 */
/*                                	     BY   L.Eckstein   */
/*                                                             */
/*    FILENAME                                                 */
/*      sca_xdmp.c                                             */
/*                                                             */
/*    DESCRIPTION                                              */
/*      Auxiliary functions for trace                          */
/*                                                             */
/*                                                             */
/*    EXPORT                DESCRIPTION                        */
/*      sca_aux_sct_apdu     print SCT-APDU in file            */
/*                                                             */
/*      sca_aux_sct_resp     print SCT-RESP in file            */
/*                                                             */
/*      sca_aux_icc_apdu      print SC-APDU in file            */
/*                                                             */
/*      sca_aux_icc_resp      print SC-RESP in file            */
/*                                                             */
/*-------------------------------------------------------------*/

/*-------------------------------------------------------------*/
/*   include-Files                                             */
/*-------------------------------------------------------------*/
#ifndef _MACC7_
#include <sys/types.h>
#include <sys/stat.h>
#else 
#include "MacTypes.h"
#endif /* !MACC7 */


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "sca.h"
#include "stamod.h"

/*-------------------------------------------------------------*/
/*   type definitions					       */
/*-------------------------------------------------------------*/

#define  INSANZ 	28
#define  ICC_INSANZ	17

/*-------------------------------------------------------------*/
/*   global variable definitions			       */
/*-------------------------------------------------------------*/
static struct sct_ins {
	int             ins_code;
	char            ins_text[30];
}               SCTins[] = {

{S_REQUEST_ICC, 	"S_REQUEST_ICC"},
{S_STATUS, 		"S_STATUS"}, 
{S_EJECT_ICC,		"S_EJECT_ICC"}, 
{S_DISPLAY,		"S_DISPLAY"},	 
{S_RESET,		"S_RESET"},
{S_VERSION,		"S_VERSION"}, 
{S_TRANS,		"S_TRANS"}, 
{S_GEN_USER_KEY,	"S_GEN_USER_KEY"}, 
{S_INST_USER_KEY,	"S_INST_USER_KEY"}, 
{S_DEL_USER_KEY,	"S_DEL_USER_KEY"},
{S_GET_RNO,		"S_GET_RNO"}, 
{S_RSA_SIGN,		"S_RSA_SIGN"}, 
{S_RSA_VERIFY,		"S_RSA_VERIFY"}, 
{S_DES_ENC,		"S_DES_ENC"}, 
{S_RSA_ENC,		"S_RSA_ENC"},
{S_DES_DEC,		"S_DES_DEC"}, 
{S_RSA_DEC,		"S_RSA_DEC"}, 
{S_ENC_DES_KEY,		"S_ENC_DES_KEY"}, 
{S_DEC_DES_KEY,		"S_DEC_DES_KEY"}, 
{S_GEN_DEV_KEY,		"S_GEN_DEV_KEY"}, 
{S_INST_DEV_KEY,	"S_INST_DEV_KEY"}, 
{S_DEL_DEV_KEY,		"S_DEL_DEV_KEY"}, 
{S_COMP_DEV_KEY,	"S_COMP_DEV_KEY"}, 
{S_SET_APPL_ID,		"S_SET_APPL_ID"},
{S_INST_VER_DATA,	"S_INST_VERIFICATION_DATA"}, 
{S_MODIFY_VER_DATA,	"S_MODIFY_VERIFICATION_DATA"}, 
{S_AUTH,		"S_AUTHENTICATE"}, 
{S_PERFORM_VERIFY,	"S_PERFORM_VERIFICATION"}, 
{S_GEN_SESSION_KEY,	"S_GEN_SESSION_KEY"}, 
{S_DEL_SESSION_KEY,	"S_DEL_SESSION_KEY"}, 
{S_WRITE_KEYCARD,	"S_WRITE_KEYCARD"}, 
{S_READ_KEYCARD,	"S_READ_KEYCARD"},
{S_STATINF,     	"S_STATINF"}
};


static struct icc_ins {
	int             ins_code;
	char            ins_text[20];
}               SCins[] = {

{ICC_SELECT, 	"ICC_SELECT"},
{ICC_CLOSE, 	"ICC_CLOSE"}, 
{ICC_EXECUTE,	"ICC_EXECUTE"}, 
{ICC_READB,	"ICC_READB"},	 
{ICC_READR,	"ICC_READR"},
{ICC_WRITEB,	"ICC_WRITEB"}, 
{ICC_WRITER,	"ICC_WRITER"}, 
{ICC_CREATE,	"ICC_CREATE"}, 
{ICC_DELF,	"ICC_DELF"}, 
{ICC_DELREC,	"ICC_DELREC"},
{ICC_SETKEY,	"ICC_SETKEY"}, 
{ICC_WR_KEY,	"ICC_WR_KEY"}, 
{ICC_GET_CD,	"ICC_GET_CD"}, 
{ICC_CRYPT,	"ICC_CRYPT"}, 
{ICC_LOCKKEY,	"ICC_LOCKKEY"},
{ICC_KEYSTAT,	"ICC_KEYSTAT"}, 
{ICC_REGISTER,	"ICC_REGISTER"}, 
{ICC_COMPUTE,	"ICC_COMPUTE"}, 
{ICC_LOCKF,	"ICC_LOCKF"}, 
{ICC_VERIFY,	"ICC_VERIFY"}, 
{ICC_VER_CHG,	"ICC_VER_CHG"}, 
{ICC_EXCHAL,	"ICC_EXCHAL"}, 
{ICC_EX_AUTH,	"ICC_EX_AUTH"}, 
{ICC_GETCHAL,	"ICC_GETCHAL"},
{ICC_IN_AUTH,	"ICC_IN_AUTH"}, 
{ICC_MU_AUTH,	"ICC_MU_AUTH"}

};






#ifdef _DOS_
typedef unsigned char u_char;
typedef unsigned long u_long;

struct hex_overlay {
	u_long          b0, b1, b2, b3;
};

#endif

typedef
union {
	struct {
		u_long          b0, b1, b2, b3;
	}               w;	/* for xdump, fxdump */
	char            c[16];
}               XBUF;








/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  sca_aux_sct_apdu       VERSION   1.1             */
/*                              DATE   January 1993       */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Print SCT-APDU in TRACE-File SCTpid.TRC               */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* INPUT                     DESCRIPTION                  */
/*  dump_file                 File-pointer                */
/*                                                        */
/*  buffer                    information                 */
/*                                                        */
/*  len                       length of information       */
/*                                                        */
/*                                                        */
/*--------------------------------------------------------*/
int 
sca_aux_sct_apdu(dump_file, buffer, len)
	FILE           *dump_file;
	char           *buffer;
	int             len;
{
	unsigned int    index = 0;
	unsigned int	listlen = 0;
	unsigned int	offset;
		 int	len_field;

	fprintf(dump_file, "SCT_COMMAND: ");
	listlen = sizeof(SCTins) / sizeof(SCTins[0]);
	for (index = 0; index < listlen; index++) {

		if (SCTins[index].ins_code == (*(buffer + 1) & 0xFF)) {
			fprintf(dump_file, "%s\n", SCTins[index].ins_text);
			fprintf(dump_file, "      CL(0x%02x) INS(0x%02x) P1(0x%02x) P2(0x%02x) L(0x%02x)",
				*buffer & 0xFF,
				*(buffer + 1) & 0xFF,
				*(buffer + 2) & 0xFF,
				*(buffer + 3) & 0xFF,
				*(buffer + 4) & 0xFF);
			len_field = (int) (*(buffer + 4) & 0xFF);
			offset = 5;
			if (len_field == 255) {
				fprintf(dump_file, " L2(0x%02x) L3(0x%02x)\n",
				*(buffer + 5) & 0xFF,
				*(buffer + 6) & 0xFF);
				offset = 7;
			}
			else
				fprintf(dump_file,"\n");

			
			if ((*(buffer + 1) & 0xFF) == 0x11) {	/* = S_TRANS */
				fprintf(dump_file, "      DATA: ");
				sca_aux_icc_apdu(dump_file, buffer + offset, len - offset);
			} else {
				fprintf(dump_file, "      DATA:\n");
				aux_fxdump(dump_file, buffer + offset, len - offset, 0);
			}
			return (0);

		}
	}
	/* INS not found */
	fprintf(dump_file, "SCT_COMMAND: INS not defined\n");
	return (-1);



}



/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  sca_aux_sct_resp       VERSION   1.1             */
/*                              DATE   January 1993       */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Print SCT-RESP in TRACE-File SCTpid.TRC               */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* INPUT                     DESCRIPTION                  */
/*  dump_file                 File-pointer                */
/*                                                        */
/*  buffer                    information                 */
/*                                                        */
/*  len                       length of information       */
/*                                                        */
/*                                                        */
/*--------------------------------------------------------*/
void 
sca_aux_sct_resp(dump_file, buffer, len)
	FILE           *dump_file;
	char           *buffer;
	int             len;
{
	fprintf(dump_file, "SCT-RESPONSE\n");

	fprintf(dump_file, "      L(0x%02x)\n", *buffer & 0xFF);
	fprintf(dump_file, "      DATA:\n");
	aux_fxdump(dump_file, buffer + 1, len - 3, 0);
	fprintf(dump_file, "      SW1(0x%02x) SW2(0x%02x)\n",
		*(buffer + len - 2) & 0xFF,
		*(buffer + len - 1) & 0xFF);




}

/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  sca_aux_icc_apdu        VERSION   1.1            */
/*                              DATE   January 1993       */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Print SC-APDU in TRACE-File ICCpid.TRC                 */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* INPUT                     DESCRIPTION                  */
/*  dump_file                 File-pointer                */
/*                                                        */
/*  buffer                    information                 */
/*                                                        */
/*  len                       length of information       */
/*                                                        */
/*                                                        */
/*--------------------------------------------------------*/
int 
sca_aux_icc_apdu(dump_file, buffer, len)
	FILE           *dump_file;
	char           *buffer;
	int             len;
{
	unsigned int    index = 0;
	unsigned int    listlen; 

	fprintf(dump_file, "ICC_COMMAND: ");
	listlen = sizeof(SCins) / sizeof(SCins[0]);
	for (index = 0; index < listlen; index++) {

		if (SCins[index].ins_code == (*(buffer + 1) & 0xFF)) {
			fprintf(dump_file, "%s\n", SCins[index].ins_text);
			fprintf(dump_file, "      CL(0x%02x) INS(0x%02x) P1(0x%02x) P2(0x%02x) L(0x%02x)\n",
				*buffer & 0xFF,
				*(buffer + 1) & 0xFF,
				*(buffer + 2) & 0xFF,
				*(buffer + 3) & 0xFF,
				*(buffer + 4) & 0xFF);
			fprintf(dump_file, "      DATA:\n");
			aux_fxdump(dump_file, buffer + 5, len - 5, 0);
			return (0);

		}
	}
	/* INS not found */
	fprintf(dump_file, "ICC_COMMAND: INS not defined\n");
	fprintf(dump_file, "      DATA:\n");
	aux_fxdump(dump_file, buffer, len , 0);	

	return (-1);



}


/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  sca_aux_icc_resp        VERSION   1.1            */
/*                              DATE   January 1993       */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Print ICC-RESP in TRACE-File ICCpid.TRC                */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* INPUT                     DESCRIPTION                  */
/*  dump_file                 File-pointer                */
/*                                                        */
/*  buffer                    information                 */
/*                                                        */
/*  len                       length of information       */
/*                                                        */
/*                                                        */
/*--------------------------------------------------------*/
void 
sca_aux_icc_resp(dump_file, buffer, len)
	FILE           *dump_file;
	char           *buffer;
	int             len;
{
	fprintf(dump_file, "SC-RESPONSE\n");

	fprintf(dump_file, "      DATA:\n");
	aux_fxdump(dump_file, buffer, len - 2, 0);
	fprintf(dump_file, "      SW1(0x%02x) SW2(0x%02x)\n",
		*(buffer + len - 2) & 0xFF,
		*(buffer + len - 1) & 0xFF);




}

/*-------------------------------------------------------------*/
/* E N D   O F	 P A C K A G E	     XDUMP      	       */
/*-------------------------------------------------------------*/

