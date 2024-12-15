/*
 *  STARMOD Release 1.1 (GMD)
 */

/*-------------------------------------------------------+-----*/
/*                                                       | GMD */
/*   SYSTEM   STARMOD  -  Version 1.1                    +-----*/
/*                                                             */
/*-------------------------------------------------------------*/
/*                                                             */
/*    PACKAGE   SCT                     VERSION 1.1            */
/*                                         DATE January 1993   */
/*                                           BY Levona Eckstein*/
/*                                                             */
/*    FILENAME                                                 */
/*      sctde.c                                                */
/*                                                             */
/*    DESCRIPTION                                              */
/*      Encoding and decoding procedures for the               */
/*      SCT commands and responses                             */
/*                                                             */
/*    IMPORT                DESCRIPTION                        */
/*                                                             */
/*      sca_aux_sct_apdu      dump sct-apdu in file            */
/*                                                             */
/*      aux_free2_OctetString  release octets - pointer        */
/*                                                             */
/*      icc_e_KeyAttrList     create datafield for             */
/*                            S_INST_USER_KEY, S_INST_DEV_KEY  */
/*                            S_INST_VER_DATA                  */
/*                                                             */
/*      icc_e_KeyId             create key identifier          */
/*                                                             */
/*    EXPORT                DESCRIPTION                        */
/*      SCTDEcreate           create S-Command                 */
/*                                                             */
/*      SCTDEerr              error-handling                   */
/*                                                             */
/*      SCTDEcheck            check 1 or 3 octets              */
/*                                                             */
/*      SCTDEresponse         analyse response                 */
/*                                                             */
/*                                                             */
/*    INTERNAL              DESCRIPTION                        */
/*      SCTOutSecMessINT      create SCT-APDU with CCS         */
/*                                                             */
/*      SCTOutSecMessCONC     encrypt SCT command              */
/*                                                             */
/*      SCTInSecMessINT       check SCT-APDU with CCS          */
/*                                                             */
/*      SCTInSecMessCONC      decrypt SCT-APDU                 */
/*                                                             */
/*      SCTalloc              allocate buffer for command      */
/*                                                             */
/*      SCToctetstring        create datafield of command      */
/*                                                             */
/*      SCTparam              create parameter in command      */
/*                                                             */
/*      SCTplength            create lengthfield in apdu       */
/*                                                             */
/*      SCTppublic            create parameter 'public' in cmd */
/*                                                             */
/*      SCTwithNMdata         create command with not mandatory*/
/*                            datafield                        */
/*      SCTwithMdata          create command with mandatory    */
/*                            datafield                        */
/*      SCTnodata             create command with no datafield */
/*                                                             */
/*      LofPublic             calculate length of public       */
/*                            structure                        */
/*      SCTssc                in case of secure messaging      */
/*                            create ssc field in apdu         */
/*                                                             */
/*                                                             */
/*      SCTverifyinfo         create datafield for             */
/*                            S_INST_VER_DATA                  */
/*                                                             */
/*      SCTclass              create SCT class-byte            */
/*                                                             */
/*      ICCclass              create ICC class-byte            */
/*                                                             */
/*      SCTelemlen            eleminate length field in        */
/*                            response buffer                  */
/*                                                             */
/*-------------------------------------------------------------*/

/*-------------------------------------------------------------*/
/*   include-Files                                             */
/*-------------------------------------------------------------*/
#include <stdio.h>
#include "sca.h"
#include "stamod.h"



/*-------------------------------------------------------------*/
/*   extern declarations                                       */
/*-------------------------------------------------------------*/

extern void     sca_aux_sct_resp();
extern int      sca_aux_sct_apdu();
extern int      cr_sctlist();

extern unsigned int tp1_err;	/* error-variable from transmission module */
extern unsigned int sct_errno;	/* error variable               */
extern char    *sct_errmsg;	/* address of error message */
extern void     aux_free2_OctetString();


#ifdef SCTTRACE
extern FILE    *sct_trfp;	/* Filepointer of trace file    */

#endif

/*-------------------------------------------------------------*/
/*   globale forward declarations                              */
/*-------------------------------------------------------------*/
char           *SCTDEcreate();
int             SCTDEcheck();
int             SCTDEresponse();
int		SCTDEerr();
/*-------------------------------------------------------------*/
/*   internal forward declarations                             */
/*-------------------------------------------------------------*/
static int      	SCTOutSecMessINT();
static int      	SCTOutSecMessCONC();
static int      	SCTInSecMessINT();
static int      	SCTInSecMessCONC();
static void     	SCToctetstring();
static void     	SCTparam();
static void     	SCTplength();
static void     	SCTbinval();
static void     	SCTppublic();
static char    		*SCTwithNMdata();
static char   		*SCTwithMdata();
static char    		*SCTnodata();
static char    		*SCTalloc();
static unsigned int 	LofPublic();
static void    		SCTssc();
static void     	SCTverifyinfo();
static unsigned int 	SCTclass();
static unsigned int 	ICCclass();
static void		SCTelemlen();

/*-------------------------------------------------------------*/
/*   type definitions                                          */
/*-------------------------------------------------------------*/

#define LHEADER                 4       /* Length of Header     */
                                        /* CLA+INS+P1+P2        */
#define LEN1                    1       /* 1 Byte, if L < 255   */
#define LEN3                    3       /* 3 Byte, if L >=255   */

#define S_NOTUSED               0x00

#define NON_INTER               0x80    /* Non interindustry command */
                                        /* set for class - byte */


#define RQP1                    request->rq_p1
#define RQP2                    request->rq_p2
#define RQDATA                  request->rq_datafield
#define RQDATENC                request->rq_datafield.enc
#define RQDATDES                request->rq_datafield.enc_des_key
#define RQDATVERIFY             request->rq_datafield.verify
#define RQDATDESKEY             request->rq_datafield.deskey
#define RQDATDEV                request->rq_datafield.dev_key_info
#define RQDATVER                request->rq_datafield.ver_record
#define RQDATSESS               request->rq_datafield.session_key
#define RQDATWRITE              request->rq_datafield.write_keycard
#define RQDATKEYATTR            request->rq_datafield.keyattrlist
#define RQDATINSTKEY            request->rq_datafield.inst_dev_key
#define RQDATAPPLID             request->rq_datafield.appl_id

static char 	INITIAL_APPL_ID[6] = {0x00,0x00,0x00,0x00,0x00,0x00};
static Boolean	no_secure_cmd = FALSE;


/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  SCTDEcreate         VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Create s_apdu                                         */
/*  All parameters must be checked in calling procedure   */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   p_elem                   pointer of portparam struct.*/
/*							  */
/*   command                  instruction code            */
/*                                                        */
/*   request                  address of request structure*/
/*                                                        */
/* OUT                                                    */
/*   lapdu                    length of apdu              */
/*                                                        */
/*   flag                     flag for S_STATUS           */
/*                            set by S_REQUEST_ICC        */
/*                                   S_PERFORM_VERIFY     */
/*                                   S_MODIFY_VER_DATA    */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   pointer                   o.k (address of apdu )     */
/*                                                        */
/*   NULL                      Error                      */
/*                             EINS                       */
/*						          */
/* CALLED FUNCTIONS					  */
/*   SCTalloc                  Error                      */
/*				EMEMAVAIL		  */
/*   SCTOutSecMessINT	       Error			  */ 
/*				EMEMAVAIL		  */
/*				EDESENC  		  */
/*				EMAC     		  */
/*				EALGO    		  */
/*				ENOINTKEY	          */
/*   SCTOutSecMessCONC	       Error			  */ 
/*				EMEMAVAIL		  */
/*				EDESENC  		  */
/*				EALGO    		  */
/*				ENOCONCKEY	          */
/*   SCToctetstring                                       */
/*   SCTparam                                             */
/*   SCTplength                                           */
/*   SCTppublic                                           */
/*   SCTwithNMdata             Error                      */
/*				EMEMAVAIL		  */
/*   SCTwithMdata              Error                      */
/*				EPARINC                   */
/*				EMEMAVAIL                 */
/*   SCTnodata                 Error                      */
/*				EMEMAVAIL                 */
/*   LofPublic                                            */
/*   SCTssc                                               */
/*   SCTverifyinfo                                        */
/*   SCTclass                                             */
/*   ICCclass                                             */
/*   icc_e_KeyAttrList                                    */
/*   sca_aux_sct_apdu                                     */
/*   free macro in MF_check.h				  */
/*--------------------------------------------------------*/
char           *
SCTDEcreate(p_elem, command, request, lapdu, flag)
	struct s_portparam *p_elem;	/* portparam structure */
	unsigned int    command;/* instruction code */
	Request        *request;/* request structure */
	unsigned int   *lapdu;	/* length of apdu   */
	Boolean        *flag;	/* flag for S_STATUS */
{
	char           *s_apdu;
	char           *ptr;
	char		kid, kid2;
	unsigned int    ldata = 0;
	unsigned int    lenofpublic;
	unsigned int    class;
	unsigned int    purpose, i;
	int             rc;
	OctetString     in_apdu;
	OctetString     out_apdu;
	char		*proc="SCTDEcreate";

	*flag = FALSE;
	sct_errno = 0;
	no_secure_cmd = FALSE;

/*****************************************************************************/
/*
 *      Build S_APDU without CLASS / INS
 *	The correctness of the parameters will be checked by the sca-functions   
 */

	switch (command) {
		/*--------------------------*/
		/* create S_STATINF         */
		/*--------------------------*/
	case S_STATINF:
		no_secure_cmd = TRUE;

		if ((s_apdu = SCTalloc(ldata, &p_elem->secure_messaging,
				       lapdu)) == NULL)
			return (NULL);

		ptr = s_apdu + 2;
		*ptr++ = S_NOTUSED;
		*ptr++ = S_NOTUSED;
		*ptr = ldata;
		if (p_elem->secure_messaging.command != SCT_SEC_NORMAL)
			*lapdu = *lapdu - 1;
		break;
		/*--------------------------*/
		/* create S_REQUEST_ICC     */
		/*--------------------------*/
	case S_REQUEST_ICC:

		s_apdu = SCTwithNMdata(p_elem, S_NOTUSED, RQP2.time,
				       RQDATA.outtext, lapdu);

		*flag = TRUE;
		break;


		/*--------------------------*/
		/* create S_DISPLAY         */
		/*--------------------------*/
	case S_DISPLAY:

		s_apdu = SCTwithNMdata(p_elem, S_NOTUSED, RQP2.time,
				       RQDATA.outtext, lapdu);
		break;


		/*--------------------------*/
		/* create S_EJECT_ICC       */
		/*--------------------------*/
	case S_EJECT_ICC:
		s_apdu = SCTwithNMdata(p_elem, S_NOTUSED, (unsigned) RQP2.signal,
				       RQDATA.outtext, lapdu);
		break;

		/*---------------------------------------------------------*/
		/* create S_STATUS- is only be called by the procedure     */
		/* sct_interface                                           */
		/*---------------------------------------------------------*/
	case S_STATUS:
		s_apdu = SCTnodata(p_elem, S_NOTUSED, S_NOTUSED, lapdu);
		break;
		/*---------------------------------------------------------*/
		/* create S_RESET - is only be called by the procedure     */
		/* sct_reset                                               */
		/* this command will always be send in plaintext           */
		/*---------------------------------------------------------*/
	case S_RESET:
		no_secure_cmd = TRUE;

		if ((s_apdu = SCTalloc(ldata, &p_elem->secure_messaging,
				       lapdu)) == NULL)
			return (NULL);

		ptr = s_apdu + 2;
		*ptr++ = S_NOTUSED;
		*ptr++ = S_NOTUSED;
		*ptr = ldata;
		if (p_elem->secure_messaging.command != SCT_SEC_NORMAL)
			*lapdu = *lapdu - 1;
		break;

		/*---------------------------*/
		/* create S_VERSION          */
		/*---------------------------*/
	case S_VERSION:
		s_apdu = SCTnodata(p_elem, S_NOTUSED, S_NOTUSED, lapdu);
		break;

		/*------------------------------------------------------*/
		/* create S_TRANS                                       */
		/* the secure - parameter defines the secure messaging  */
		/* between SCT and SC                                   */
		/*------------------------------------------------------*/
	case S_TRANS:
		s_apdu = SCTwithMdata(p_elem, (unsigned) RQP1.secmode, S_NOTUSED,
				      RQDATA.sccommand, lapdu);
		break;

		/*--------------------------*/
		/* create S_GEN_USER_KEY    */
		/*--------------------------*/
	case S_GEN_USER_KEY:
		if (RQP2.algid == S_RSA_F4) {
			if (RQDATA.keylen < 255)
				ldata = 1;
			else
				ldata = 2;
		} else
			ldata = 0;

		if ((s_apdu = SCTalloc(ldata, &p_elem->secure_messaging,
				       lapdu)) == NULL)
			return (NULL);

		ptr = s_apdu + 2;

		kid = icc_e_KeyId(RQP1.kid);
		SCTparam(&ptr, kid, RQP2.algid);
		SCTplength(&ptr, ldata);
		SCTssc(&ptr, p_elem);
		if (ldata > 0)
			SCTbinval(&ptr, RQDATA.keylen);
		break;

		/*----------------------------*/
		/* create S_INST_USER_KEY     */
		/*----------------------------*/
	case S_INST_USER_KEY:
		ldata = 5;

		if ((s_apdu = SCTalloc(ldata, &p_elem->secure_messaging,
				       lapdu)) == NULL)
			return (NULL);
		ptr = s_apdu + 2;

		kid = icc_e_KeyId(RQP1.kid);
		SCTparam(&ptr, kid, S_NOTUSED);

		SCTplength(&ptr, ldata);

		SCTssc(&ptr, p_elem);

		SCTbinval(&ptr, RQDATKEYATTR->key_inst_mode);

		icc_e_KeyAttrList(&ptr, RQDATKEYATTR, NOT_DEFINED); 
		break;



		/*--------------------------*/
		/* create S_DEL_USER_KEY    */
		/*--------------------------*/
	case S_DEL_USER_KEY:
		kid = icc_e_KeyId(RQP1.kid);
		s_apdu = SCTnodata(p_elem, kid, S_NOTUSED, lapdu);
		break;

		/*----------------------------*/
		/* create S_GET_RNO           */
		/*----------------------------*/
	case S_GET_RNO:
		s_apdu = SCTnodata(p_elem, RQP1.lrno, S_NOTUSED, lapdu);
		break;

		/*----------------------------*/
		/* create S_RSA_SIGN          */
		/*----------------------------*/
	case S_RSA_SIGN:
		kid = icc_e_KeyId(RQP1.kid);
		s_apdu = SCTwithMdata(p_elem, kid, S_NOTUSED,
				      RQDATA.sig_string, lapdu);

		break;

		/*----------------------------*/
		/* create S_RSA_VERIFY        */
		/*----------------------------*/
	case S_RSA_VERIFY:

		ldata = LofPublic(RQDATVERIFY->public);

		ldata += RQDATVERIFY->signature->noctets;
		ldata++;
		if (RQDATVERIFY->signature->noctets >= 255)
			ldata += 2;


		ldata += RQDATVERIFY->hash->noctets;
		ldata++;
		if (RQDATVERIFY->hash->noctets >= 255)
			ldata += 2;


		if ((s_apdu = SCTalloc(ldata, &p_elem->secure_messaging,
				       lapdu)) == NULL)
			return (NULL);
		ptr = s_apdu + 2;

		SCTparam(&ptr, S_NOTUSED, S_NOTUSED);

		SCTplength(&ptr, ldata);

		SCTssc(&ptr, p_elem);

		SCTppublic(&ptr, RQDATVERIFY->public);

		SCTplength(&ptr, RQDATVERIFY->signature->noctets);
		SCToctetstring(&ptr, RQDATVERIFY->signature);

		SCTplength(&ptr, RQDATVERIFY->hash->noctets);
		SCToctetstring(&ptr, RQDATVERIFY->hash);


		break;

		/*----------------------------*/
		/* create S_DES_ENC           */
		/*----------------------------*/
	case S_DES_ENC:
		kid = icc_e_KeyId(RQP1.kid);
		s_apdu = SCTwithMdata(p_elem, kid, (unsigned) RQP2.more,
				      RQDATA.plaintext, lapdu);
		break;


		/*----------------------------*/
		/* create S_RSA_ENC           */
		/*----------------------------*/
	case S_RSA_ENC:
		ldata = LofPublic(RQDATENC->public);
		if (ldata == 1)
			ldata = 0;

		lenofpublic = ldata;
		ldata += RQDATENC->plaintext->noctets;

		if ((s_apdu = SCTalloc(ldata, &p_elem->secure_messaging,
				       lapdu)) == NULL)
			return (NULL);
		ptr = s_apdu + 2;

		SCTparam(&ptr, S_NOTUSED, (unsigned) RQP2.more);

		SCTplength(&ptr, ldata);

		SCTssc(&ptr, p_elem);

		if (lenofpublic > 1)
			SCTppublic(&ptr, RQDATENC->public);

		SCToctetstring(&ptr, RQDATENC->plaintext);
		break;

		/*----------------------------*/
		/* create S_RSA_DEC           */
		/* create S_DES_DEC           */
		/*----------------------------*/
	case S_RSA_DEC:
	case S_DES_DEC:
		kid = icc_e_KeyId(RQP1.kid);
		s_apdu = SCTwithMdata(p_elem, kid, (unsigned) RQP2.more,
				      RQDATA.chiffrat, lapdu);
		break;


		/*----------------------------*/
		/* create S_ENC_DES_KEY       */
		/*----------------------------*/
	case S_ENC_DES_KEY:
		ldata = LofPublic(RQDATDES->public);
		if (RQDATDES->padding_string != NULLOCTETSTRING)
			ldata += RQDATDES->padding_string->noctets;
		ldata++;

		if ((s_apdu = SCTalloc(ldata, &p_elem->secure_messaging,
				       lapdu)) == NULL)
			return (NULL);
		ptr = s_apdu + 2;

		kid = icc_e_KeyId(RQP1.kid);
		SCTparam(&ptr, kid, S_NOTUSED);

		SCTplength(&ptr, ldata);

		SCTssc(&ptr, p_elem);

		SCTppublic(&ptr, RQDATDES->public);
		if (RQDATDES->padding_string != NULLOCTETSTRING)
		{
			SCTplength(&ptr,
				   RQDATDES->padding_string->noctets);
			SCToctetstring(&ptr, 
				   RQDATDES->padding_string);
		}
		else
			SCTplength(&ptr,S_NOTUSED);

		break;


		/*----------------------------*/
		/* create S_DEC_DES_KEY       */
		/*----------------------------*/
	case S_DEC_DES_KEY:
		ldata = 1 + RQDATDESKEY->chiffrat->noctets;

		if ((s_apdu = SCTalloc(ldata, &p_elem->secure_messaging,
				       lapdu)) == NULL)
			return (NULL);
		ptr = s_apdu + 2;

		kid = icc_e_KeyId(RQP1.kid);
		kid2= icc_e_KeyId(RQP2.kid);
		SCTparam(&ptr, kid, kid2);

		SCTplength(&ptr, ldata);

		SCTssc(&ptr, p_elem);

		*ptr++ = RQDATDESKEY->algid;
		SCToctetstring(&ptr, RQDATDESKEY->chiffrat);

		break;

		/*----------------------------*/
		/* create S_GEN_DEV_KEY       */
		/*----------------------------*/
	case S_GEN_DEV_KEY:
		ldata = 1 + RQDATDEV->appl_id->noctets;


		if ((s_apdu = SCTalloc(ldata, &p_elem->secure_messaging,
				       lapdu)) == NULL)
			return (NULL);
		ptr = s_apdu + 2;

		kid = icc_e_KeyId(RQP1.kid);
		SCTparam(&ptr, kid, RQP2.algid);

		SCTplength(&ptr, ldata);

		SCTssc(&ptr, p_elem);

		SCToctetstring(&ptr, RQDATDEV->appl_id);
		*ptr = RQDATDEV->purpose;

		break;

		/*----------------------------*/
		/* create S_INST_DEV_KEY      */
		/*----------------------------*/
	case S_INST_DEV_KEY:
		ldata = 5 + RQDATINSTKEY->appl_id->noctets;

		if ((s_apdu = SCTalloc(ldata, &p_elem->secure_messaging,
				       lapdu)) == NULL)
			return (NULL);
		ptr = s_apdu + 2;

		if (memcmp(RQDATINSTKEY->appl_id->octets,INITIAL_APPL_ID,6) !=0) {
			kid = icc_e_KeyId(RQP1.dev_sel_key->pval.kid);
			SCTparam(&ptr, kid, S_NOTUSED);
		}
		else
			SCTparam(&ptr, RQP1.dev_sel_key->pval.purpose, S_NOTUSED);

		SCTplength(&ptr, ldata);

		SCTssc(&ptr, p_elem);

		SCToctetstring(&ptr, RQDATINSTKEY->appl_id);
		SCTbinval(&ptr, RQDATINSTKEY->key_attr_list->key_inst_mode);

		icc_e_KeyAttrList(&ptr, RQDATINSTKEY->key_attr_list,
			          NOT_DEFINED); 

		break;

		/*--------------------------*/
		/* create S_DEL_DEV_KEY     */
		/*--------------------------*/
	case S_DEL_DEV_KEY:
		ldata = RQDATAPPLID->noctets;

		if ((s_apdu = SCTalloc(ldata, &p_elem->secure_messaging,
				       lapdu)) == NULL)
			return (NULL);
		ptr = s_apdu + 2;
		if (RQP2.status == DEV_OWN) {
			SCTparam(&ptr,RQP1.dev_sel_key->pval.purpose, RQP2.status);
		} else {
			kid = icc_e_KeyId(RQP1.dev_sel_key->pval.kid);
			SCTparam(&ptr, kid, RQP2.status);
		}

		SCTplength(&ptr, ldata);

		SCTssc(&ptr, p_elem);

		SCToctetstring(&ptr, RQDATAPPLID);
		break;

		/*--------------------------*/
		/* create S_COMP_DEV_KEY    */
		/*--------------------------*/
	case S_COMP_DEV_KEY:
		ldata = RQDATAPPLID->noctets;

		if ((s_apdu = SCTalloc(ldata, &p_elem->secure_messaging,
				       lapdu)) == NULL)
			return (NULL);
		ptr = s_apdu + 2;
		SCTparam(&ptr, S_NOTUSED, S_NOTUSED);

		SCTplength(&ptr, ldata);

		SCTssc(&ptr, p_elem);

		SCToctetstring(&ptr, RQDATAPPLID);
		break;

		/*--------------------------*/
		/* create S_SET_APPL_ID    */
		/*--------------------------*/
	case S_SET_APPL_ID:
		ldata = RQDATAPPLID->noctets;

		if ((s_apdu = SCTalloc(ldata, &p_elem->secure_messaging,
				       lapdu)) == NULL)
			return (NULL);
		ptr = s_apdu + 2;
		SCTparam(&ptr, RQP1.level, S_NOTUSED);

		SCTplength(&ptr, ldata);

		SCTssc(&ptr, p_elem);

		SCToctetstring(&ptr, RQDATAPPLID);
		break;


		/*-----------------------------------*/
		/* create S_INST_VERIFICATION_DATA   */
		/*-----------------------------------*/
	case S_INST_VER_DATA:
		ldata = 6 + RQDATVER->ver_data->noctets;

		if ((s_apdu = SCTalloc(ldata, &p_elem->secure_messaging,
				       lapdu)) == NULL)
			return (NULL);
		ptr = s_apdu + 2;

		kid = icc_e_KeyId(RQP1.kid);
		SCTparam(&ptr, kid, S_NOTUSED);

		SCTplength(&ptr, ldata);

		SCTssc(&ptr, p_elem);

		SCTverifyinfo(&ptr, RQDATVER);

		break;

		/*--------------------------------*/
		/* create S_MODIFY_VER_DATA       */
		/* create S_PERFORM_VER_DAT       */
		/* the parameter 2 will be coded  */
		/* by this procedure              */
		/*--------------------------------*/
	case S_MODIFY_VER_DATA:
	case S_PERFORM_VERIFY:
		class = ICCclass(RQDATA.sec_mode);
		ldata = 1;

		if ((s_apdu = SCTalloc(ldata, &p_elem->secure_messaging,
				       lapdu)) == NULL)
			return (NULL);

		ptr = s_apdu + 2;

		kid = icc_e_KeyId(RQP1.kid);
		if (command == S_MODIFY_VER_DATA)

			SCTparam(&ptr, kid, RQP2.modify_mode);

		else
			SCTparam(&ptr, kid, S_NOTUSED);

		SCTplength(&ptr, ldata);

		SCTssc(&ptr, p_elem);

		SCTbinval(&ptr, class);
		*flag = TRUE;
		break;



		/*----------------------------*/
		/* create S_AUTH              */
		/*----------------------------*/
	case S_AUTH:
		s_apdu = SCTnodata(p_elem, S_NOTUSED, RQP2.acp, lapdu);
		break;



		/*----------------------------*/
		/* create S_GEN_SESSION_KEY   */
		/*----------------------------*/
	case S_GEN_SESSION_KEY:

		ldata = RQDATSESS->noctets;

		if ((s_apdu = SCTalloc(ldata, &p_elem->secure_messaging,
				       lapdu)) == NULL)
			return (NULL);


		ptr = s_apdu + 2;

		SCTparam(&ptr, RQP1.sec_mode, RQP2.algid);

		SCTplength(&ptr, ldata);

		SCToctetstring(&ptr, RQDATSESS);
		break;

		/*----------------------------*/
		/* create S_DEL_SESSION_KEY   */
		/*----------------------------*/
	case S_DEL_SESSION_KEY:
		s_apdu = SCTnodata(p_elem, RQP1.sec_mode, S_NOTUSED, lapdu);
		break;


		/*----------------------------*/
		/* create S_WRITE_KEYCARD    */
		/*---------------------------*/
	case S_WRITE_KEYCARD:
		i = 0;
		while (*(RQDATWRITE->keyid_list+i) != KEYIDNULL) {
		     i++; 
		     ldata++;
		}

		ldata += RQDATWRITE->appl_id->noctets;
		if ((s_apdu = SCTalloc(ldata, &p_elem->secure_messaging,
				       lapdu)) == NULL)
			return (NULL);
		ptr = s_apdu + 2;


		SCTparam(&ptr, S_NOTUSED, S_NOTUSED);
		SCTplength(&ptr, ldata);

		SCTssc(&ptr, p_elem);

		SCToctetstring(&ptr, RQDATWRITE->appl_id);
		i = 0;
		while (*(RQDATWRITE->keyid_list+i) != KEYIDNULL) {
			kid = icc_e_KeyId(*(RQDATWRITE->keyid_list+i));
			SCTbinval(&ptr, kid);
			i++;
		}

		break;

		/*----------------------------*/
		/* create S_READ_KEYCARD      */
		/*----------------------------*/
	case S_READ_KEYCARD:
		s_apdu = SCTnodata(p_elem, S_NOTUSED, RQP2.status, lapdu);
		break;




		/*----------------------------*/
		/* DEFAULT                    */
		/*----------------------------*/
	default:
		sct_errno = EINS;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (NULL);
		break;
	};

	if (s_apdu == NULL) {
		*flag = FALSE;
		return (NULL);
	}


	/*------------------------------------*/
	/* create CLASS / INS       in s_apdu */
	/*------------------------------------*/
	if (no_secure_cmd == TRUE)
		class = NON_INTER;
	else
		class = SCTclass(&p_elem->secure_messaging);
	*s_apdu = class;
	*(s_apdu + 1) = command;


#ifdef SCTTRACE
	sca_aux_sct_apdu(sct_trfp, s_apdu, *lapdu);
#endif


/*****************************************************************************/
/*
 *      Execute Secure Messaging   
 */
	if (no_secure_cmd == FALSE) {
		switch (p_elem->secure_messaging.command) {
		case SCT_INTEGRITY:
			in_apdu.noctets = *lapdu;
			in_apdu.octets = s_apdu;
			out_apdu.noctets = 0;
			out_apdu.octets = NULL;
			rc = SCTOutSecMessINT(&p_elem->integrity_key, &in_apdu, &out_apdu);
			if (s_apdu)
				free(s_apdu);
			if (rc < 0)
				return (NULL);

			s_apdu = out_apdu.octets;
			*lapdu = out_apdu.noctets;
			break;
		case SCT_CONCEALED:
			in_apdu.noctets = *lapdu;
			in_apdu.octets = s_apdu;
			out_apdu.noctets = 0;
			out_apdu.octets = NULL;
			rc = SCTOutSecMessCONC(&p_elem->concealed_key, &in_apdu, &out_apdu);
			if (s_apdu)
				free(s_apdu);
			if (rc < 0)
				return (NULL);

			s_apdu = out_apdu.octets;
			*lapdu = out_apdu.noctets;
			break;
		case SCT_COMBINED:
			in_apdu.noctets = *lapdu;
			in_apdu.octets = s_apdu;
			out_apdu.noctets = 0;
			out_apdu.octets = NULL;
			rc = SCTOutSecMessINT(&p_elem->integrity_key, &in_apdu, &out_apdu);
			if (s_apdu)
				free(s_apdu);
			if (rc < 0)
				return (NULL);

			in_apdu.noctets = 0;
			in_apdu.octets = NULL;
			rc = SCTOutSecMessCONC(&p_elem->concealed_key, &out_apdu, &in_apdu);
			if (out_apdu.octets)
				free(out_apdu.octets);
			if (rc < 0)
				return (NULL);

			s_apdu = in_apdu.octets;
			*lapdu = in_apdu.noctets;
			break;



		}
	}
	return (s_apdu);

}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      SCTDEcreate            */
/*-------------------------------------------------------------*/


/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  SCTDEerr            VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Error - handling                                      */
/*  Search in sca_errlist -  sw1 / sw2;                   */
/*  return index in sct_errno                             */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   sw1                       SW1                        */
/*                                                        */
/*   sw2                       SW2                        */
/*                                                        */
/* OUT                                                    */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   -1                       error                       */
/*				T1 errors		  */
/*				SCT errors		  */
/*                                                        */
/* CALLED FUNCTIONS					  */
/*--------------------------------------------------------*/
int
SCTDEerr(sw1, sw2)
	unsigned int    sw1;
	unsigned int    sw2;
{


	unsigned int    index = 0;
	unsigned int    listlen = 0;


	while ((sca_errlist[index].sw1 != 0x00) || (sca_errlist[index].sw2 != 0x00)) { 

		if ((sca_errlist[index].sw1 == sw1) && (sca_errlist[index].sw2 == sw2)) {
			sct_errno = index;
			sct_errmsg = sca_errlist[index].msg;
			return (S_ERR);
		}
		index++;
	}
	/* sw1 + sw2 not found */
	sct_errno = index;	/* last element in error-list */
	sct_errmsg = sca_errlist[index].msg;
	return (S_ERR);

}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      SCTDEerr               */
/*-------------------------------------------------------------*/




/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  SCTDEcheck          VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Check field, if 1 or 3 Bytes and return integer value */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* INOUT                     DESCRIPTION                  */
/*  buffer                    pointer to buffer           */
/*                                                        */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   size                      integer value              */
/*                                                        */
/*                                                        */
/* CALLED FUNCTIONS					  */
/*--------------------------------------------------------*/
int
SCTDEcheck(buffer)
	char          **buffer;
{
	char           *p;
	int             size;


	p = *buffer;
	if ((size = ((int) *p++) & 0xFF) >= 255) {
		size = ((((int) *p++) & 0xff) << 8);
		size += (((int) *p++) & 0xFF);
	};
	*buffer = p;
	return (size);
}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      SCTDEcheck             */
/*-------------------------------------------------------------*/



/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  SCTDEresponse       VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/* Execute secure messaging for response and check        */
/* sw1 / sw2 .                                            */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*  p_elem                    pointer of portparam struct.*/
/*                                                        */
/*  command		      executed command		  */
/*							  */
/*  response		      pointer of response buffer  */
/*                                                        */
/* OUT                                                    */
/*  sw1		              sw1 - value                 */
/*                                                        */
/*  sw2		              sw2 - value                 */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   0                         o.k.                       */
/*                                                        */
/*   -1                        error                      */
/*                              sw1/sw2 from SCT response */
/*                                                        */
/* CALLED FUNCTIONS                                       */
/*   sca_aux_sct_resp                                     */
/*   SCTelemlen                                           */
/*   aux_free2_OctetString                                */
/*   SCTInSecMessCONC	       error			  */
/*				EMEMAVAIL		  */
/*				EDESDEC  		  */
/*				ESSC			  */
/*				EALGO	                  */
/*				ENOCONCKEY		  */
/*   SCTInSecMessINT	       error                      */	
/*				EMEMAVAIL		  */
/*				EDESENC  		  */
/*				ESSC			  */
/*				ERECMAC 	          */
/*				EALGO	                  */
/*				ENOINTKEY		  */
/*                                                        */
/*   SCTDEerr						  */
/*                                                        */
/*--------------------------------------------------------*/
int
SCTDEresponse(p_elem, command, response, sw1, sw2)
	struct s_portparam *p_elem;
	unsigned int    command;
	OctetString    *response;
	unsigned int   *sw1;
	unsigned int   *sw2;
{

	KeyInfo         sec_key;
	OctetString     out_apdu;
	int             rc;
	char		*proc="SCTDEresponse";



/*****************************************************************************/
/*
 *      test secure messaging for response   
 */

	if ((p_elem->secure_messaging.response != SCT_SEC_NORMAL) &&
	    (no_secure_cmd == FALSE)) {

		switch (p_elem->secure_messaging.response) {
		case SCT_INTEGRITY:
			out_apdu.noctets = 0;
			out_apdu.octets = NULL;
			rc = SCTInSecMessINT(&p_elem->integrity_key,
					      	      p_elem->ssc, response,
					              &out_apdu, 4);
			break;
		case SCT_CONCEALED:
			out_apdu.noctets = 0;
			out_apdu.octets = NULL;
			rc = SCTInSecMessCONC(&p_elem->concealed_key,
						      SCT_CONCEALED,
					      	      p_elem->ssc, response,
					              &out_apdu);
			break;
		case SCT_COMBINED:
			out_apdu.noctets = 0;
			out_apdu.octets = NULL;
			rc = SCTInSecMessCONC(&p_elem->concealed_key,
						      SCT_COMBINED,
					      	      p_elem->ssc, response,
					              &out_apdu);
			if (rc < 0)
				break;
				
			response->noctets = 1;
			aux_free2_OctetString(response);
			response->noctets = out_apdu.noctets;
			response->octets = out_apdu.octets;
			out_apdu.noctets = 0;
			out_apdu.octets = NULL;
			rc = SCTInSecMessINT(&p_elem->integrity_key,
					      	      p_elem->ssc, response,
					              &out_apdu, 4);
			break;



		}

		response->noctets = 1;
		aux_free2_OctetString(response);
		/*
		 * set ssc                            
		 */
		p_elem->ssc++;
		if (rc < 0) {
			if (command == S_EJECT_ICC)
				p_elem->icc_request = FALSE;
			out_apdu.noctets = 1;
			aux_free2_OctetString(&out_apdu);
			return (-1);
		}
		response->noctets = out_apdu.noctets;
		response->octets = out_apdu.octets;
	}

#ifdef SCTTRACE
	sca_aux_sct_resp(sct_trfp, response->octets, response->noctets);
#endif


/*****************************************************************************/
/*
 *      eleminate Length-field in Response   
 */
	SCTelemlen(response);


/*****************************************************************************/
/*
 *      check SW1/SW2   
 *	if sw1 indicates an error, then	 search in sca_errlist sw1/sw2
 *	and return index in sct_errno to calling procedure
 *	the response buffer will be set free, if length = 0  
 */
	*sw1 = *(response->octets + (response->noctets - 2)) & 0xFF;
	*sw2 = *(response->octets + (response->noctets - 1)) & 0xFF;

	/* delete sw1/sw2 in response-buffer */
	*(response->octets + (response->noctets - 2)) = 0x00;
	*(response->octets + (response->noctets - 1)) = 0x00;
	response->noctets -= 2;


	if ((*sw1 != OKICC) && (*sw1 != OKSCT)) {
		if (response->noctets == 0) {
			response->noctets = 1;
			aux_free2_OctetString(response);
		}
		return (SCTDEerr(*sw1, *sw2));
	};

	return (S_NOERR);
}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      SCTDEresponse          */
/*-------------------------------------------------------------*/

/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  SCTOutSecMessINT     VERSION   1.1               */
/*                              DATE   June   1993        */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Create SCT-COMMAND-APDU  with CCS                     */
/*  This procedure can be called in case of               */
/*  secure messaging = INTEGRITY.                         */
/*  In case of secure messaging=COMBINED, this procedure  */
/*  must be called before calling the procedure           */
/*  SCTOutSecMessCONC.                                    */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   sec_key		       Secure Messaging key	  */
/*                             Algorithm Identifier       */
/*			       The following values are   */
/*			       possible:		  */
/*			       desCBC_ISO0	          */
/*			       desCBC3_ISO0	          */
/*							  */
/*   in_apdu		       Pointer of SCT-APDU        */
/*                             The SCT-APDU must have the */
/*                             structur:                  */
/*			       __________________________ */
/*			      | CLA,INS,P1,P2,L,SSC,DATA |*/
/*			       __________________________ */
/*                            (= output of the procedure  */
/*				 SCTcreate)		  */
/*                                                        */
/*							  */
/* OUT                                                    */
/*   out_apdu                  Pointer of SEC-APDU        */
/*			       out_apdu->octets will be   */
/*			       allocated by the called    */
/*			       program			  */
/*			       and must be set free by the*/
/*			       calling program            */
/*			       The APDU has the structure:*/
/*		           ______________________________ */
/*			  | CLA,INS,P1,P2,L,SSC,DATA,CCS |*/
/*		           ______________________________ */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   0                         o.k                        */
/*   -1                        Error                      */
/*				EMEMAVAIL		  */
/*				EDESENC  		  */
/*				EMAC     		  */
/*				EALGO    		  */
/*				ENOINTKEY	          */
/*						          */
/* CALLED FUNCTIONS					  */
/*   des_encrypt                                          */
/*   aux_fxdump                                           */
/*   aux_free2_BitString                                  */
/*   aux_cmp_ObjId                                        */
/*   malloc macro in MF_check.h				  */ 
/*							  */
/*--------------------------------------------------------*/
static int
SCTOutSecMessINT(sec_key, in_apdu, out_apdu)
	KeyInfo      	*sec_key;	/* secure messaging key  */
	OctetString     *in_apdu;	/* ICC-APDU		 */
	OctetString     *out_apdu;	/* ICC-SEC-APDU		 */
{
	OctetString     in_octets;
	char           *ptr, *mac_ptr;
	int             i;
	int             memolen;
	BitString       out_bits;
	char           *proc="SCTOutSecMessINT"; 


	out_apdu->noctets = 0;
	out_apdu->octets = NULL;

/*****************************************************************************/
/*
 *	test parameter sec_key     
 */

	if ((sec_key->subjectkey.nbits <= 0) ||
	    (sec_key->subjectkey.bits == NULL)) {
		sct_errno = ENOINTKEY;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (-1);
	};

	if ((!sec_key->subjectAI) || (!sec_key->subjectAI->objid)) {
		sct_errno = EALGO;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (-1);
	}
	if ((aux_cmp_ObjId(sec_key->subjectAI->objid,desCBC_ISO0->objid)) &&
 	    (aux_cmp_ObjId(sec_key->subjectAI->objid,desCBC3_ISO0->objid))) {
		sct_errno = EALGO;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (-1);
	};



/*****************************************************************************/
/*
 * 	encrypt data (CLA,INS,P1,P2,L,SSC,DATA) with Secure Messaging Key       
 */

	in_octets.noctets = in_apdu->noctets;
	in_octets.octets = in_apdu->octets;

#ifdef SCTTRACE 
	fprintf(sct_trfp, "TRACE in SCTOutSecMessINT\n");

	fprintf(sct_trfp, "   in_octets.noctets = %d\n", in_octets.noctets);
	fprintf(sct_trfp, "   in_octets.octets  = \n");
	aux_fxdump(sct_trfp, in_octets.octets, in_octets.noctets, 0);
	fprintf(sct_trfp, "   sec_key->subjectkey.nbits     = %d\n", sec_key->subjectkey.nbits);
	fprintf(sct_trfp, "   sec_key->subjectkey.bits      = \n");
	aux_fxdump(sct_trfp, sec_key->subjectkey.bits, sec_key->subjectkey.nbits / 8, 0);

#endif



	/*
	 * allocate memory for out_bits             
	 * the memory must be a multiple of 8 Bytes
         */

	if ((in_octets.noctets % 8) != 0)
		memolen = (in_octets.noctets - (in_octets.noctets % 8)) + 8;
	else
		memolen = in_octets.noctets;

	out_bits.nbits = 0;

#ifdef SCTTRACE 
	fprintf(sct_trfp, "   allocate out_bits = %d\n", memolen);
#endif

	out_bits.bits = malloc(memolen);	/* will be set free in this
						 * proc. */
	if (out_bits.bits == NULL) {
		sct_errno = EMEMAVAIL;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (-1);
	}


	/*
	 * encrypt data
	 */

	memolen = des_encrypt(&in_octets, &out_bits, SEC_END, sec_key);
	if (memolen == -1) {
		sct_errno = EDESENC;
		sct_errmsg = sca_errlist[sct_errno].msg;
		out_bits.nbits = 1;
		aux_free2_BitString(&out_bits);
		return (-1);
	}
#ifdef SCTTRACE 
	fprintf(sct_trfp, "   out_bits.nbits    = %d\n", out_bits.nbits);
	fprintf(sct_trfp, "   out_bits.bits     = \n");
	aux_fxdump(sct_trfp, out_bits.bits, out_bits.nbits / 8, 0);
#endif

	memolen = in_octets.noctets + 4;   /* 4 = length of CCS */

/*****************************************************************************/
/*
 *	create output parameter      
 */
	out_apdu->octets = malloc(memolen);
	if (out_apdu->octets == NULL) {
		sct_errno = EMEMAVAIL;
		sct_errmsg = sca_errlist[sct_errno].msg;
		out_bits.nbits = 1;
		aux_free2_BitString(&out_bits);
		return (-1);
	}

	out_apdu->noctets = memolen;
	ptr = out_apdu->octets;
	for (i = 0; i < in_octets.noctets; i++) {
		*ptr = *(in_octets.octets + i);
		ptr++;
	}

	/*
         * take the first 4 Bytes of the last block for CCS 
         */

	if ((out_bits.nbits / 8) > 8)
		mac_ptr = out_bits.bits + ((out_bits.nbits / 8) - 8);
	else
		mac_ptr = out_bits.bits;

	for (i = 0; i < 4; i++) {
		*ptr = *(mac_ptr + i);
		ptr++;
	};
	out_bits.nbits = 1;
	aux_free2_BitString(&out_bits);

#ifdef SCTTRACE 
	fprintf(sct_trfp, "   out_apdu->noctets  = %d\n", out_apdu->noctets);
	fprintf(sct_trfp, "   out_apdu->octets   = \n");
	aux_fxdump(sct_trfp, out_apdu->octets, out_apdu->noctets, 0);
	fprintf(sct_trfp, "TRACE-END in SCTOutSecMessINT\n");
#endif

	return (0);


}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      SCTOutSecMessINT     */
/*-------------------------------------------------------------*/



/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  SCTOutSecMessCONC    VERSION   1.1               */
/*                              DATE   June  1993         */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Encrypt SCT-COMMAND-APDU (without CLA-Byte)           */
/*  This Procedure can be called in case of               */
/*  secure messaging = CONCEALED and in case of           */
/*  secure messaging = COMBINED after calling the         */
/*  procedure SCTOutSecMessINT.                           */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   sec_key		       Secure Messaging key	  */
/*                             Algorithm Identifier       */
/*			       The following values are   */
/*			       possible:		  */
/*			       desCBC_ISO0	          */
/*			       desCBC3_ISO0	          */
/*							  */
/*   in_apdu		       Pointer of SCT-APDU        */
/*                             The SC-APDU must have the  */
/*                             structur:                  */
/*			       __________________________ */
/*			      | CLA,INS,P1,P2,L,SSC,DATA |*/
/*			       __________________________ */
/*			      (= output of the procedure  */
/*				 SCTcreate)		  */
/*							  */
/*			       or			  */
/*		           ______________________________ */
/*			  | CLA,INS,P1,P2,L,SSC,DATA,MAC |*/
/*		           ______________________________ */
/*			       (= output of the procedure */
/*				  SCTOutSecMessINT)       */
/*							  */
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
/* RETURN                    DESCRIPTION                  */
/*   0                         o.k                        */
/*   -1                        Error                      */
/*				EMEMAVAIL		  */
/*				EDESENC  		  */
/*				EALGO    		  */
/*				ENOCONCKEY	          */
/*						          */
/* CALLED FUNCTIONS					  */
/*   des_encrypt                                          */
/*   aux_fxdump                                           */
/*   aux_free2_BitString                                  */
/*   aux_cmp_ObjId                                        */
/*   malloc macro in MF_check.h				  */ 
/*							  */
/*--------------------------------------------------------*/
static int
SCTOutSecMessCONC(sec_key, in_apdu, out_apdu)
	KeyInfo        *sec_key;	/* secure messaging key  */
	OctetString    *in_apdu;	/* SCT-APDU		 */
	OctetString    *out_apdu;	/* SCT-SEC-APDU		 */
{
	OctetString     in_octets;
	char           *ptr;
	int             i;
	int             memolen;
	BitString       out_bits;
	char		*proc="SCTOutSecMessCONC";


	out_apdu->noctets = 0;
	out_apdu->octets = NULL;
	in_octets.noctets = in_apdu->noctets;
	in_octets.octets = in_apdu->octets;

/*****************************************************************************/
/*
 *	test parameter sec_key     
 */
	if ((sec_key->subjectkey.nbits <= 0) ||
	    (sec_key->subjectkey.bits == NULL)) {
		sct_errno = ENOCONCKEY;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (-1);
	};

	if ((!sec_key->subjectAI) || (!sec_key->subjectAI->objid)) {
		sct_errno = EALGO;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (-1);
	}
	if ((aux_cmp_ObjId(sec_key->subjectAI->objid,desCBC_ISO0->objid)) &&
 	    (aux_cmp_ObjId(sec_key->subjectAI->objid,desCBC3_ISO0->objid))) {
		sct_errno = EALGO;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (-1);
	};

	/*****************************************************************************/
/*
 *      encrypt data (INS,P1,P2,L,SSC,DATA/CCS) with Secure Messaging Key
 */
	in_octets.noctets -= 1;
	in_octets.octets++;

#ifdef SCTTRACE
	fprintf(sct_trfp, "TRACE in SCTOutSecMessCONC\n");
	fprintf(sct_trfp, "   in_octets.noctets = %d\n", in_octets.noctets);
	fprintf(sct_trfp, "   in_octets.octets  = \n");
	aux_fxdump(sct_trfp, in_octets.octets, in_octets.noctets, 0);
	fprintf(sct_trfp, "   sec_key->subjectkey.nbits     = %d\n", sec_key->subjectkey.nbits);
	fprintf(sct_trfp, "   sec_key->subjectkey.bits      = \n");
	aux_fxdump(sct_trfp, sec_key->subjectkey.bits, sec_key->subjectkey.nbits / 8, 0);

#endif

	/*
	 * allocate memory for out_bits             
	 * the memory must be a multiple of 8 Bytes 
	 */
	if ((in_octets.noctets % 8) != 0)
		memolen = (in_octets.noctets - (in_octets.noctets % 8)) + 8;
	else
		memolen = in_octets.noctets;

	out_bits.nbits = 0;

#ifdef SCTTRACE
	fprintf(sct_trfp, "   allocate out_bits = %d\n", memolen);
#endif

	out_bits.bits = malloc(memolen);	/* will be set free in this
						 * proc. */
	if (out_bits.bits == NULL) {
		sct_errno = EMEMAVAIL;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (-1);
	}



	/*
	 * encrypt data
	 */

	memolen = des_encrypt(&in_octets, &out_bits, SEC_END, sec_key);
	if (memolen == -1) {
		sct_errno = EDESENC;
		sct_errmsg = sca_errlist[sct_errno].msg;
		out_bits.nbits = 1;
		aux_free2_BitString(&out_bits);
		return (-1);
	}
#ifdef SCTTRACE
	fprintf(sct_trfp, "   out_bits.nbits    = %d\n", out_bits.nbits);
	fprintf(sct_trfp, "   out_bits.bits     = \n");
	aux_fxdump(sct_trfp, out_bits.bits, out_bits.nbits / 8, 0);
#endif


/*****************************************************************************/
/*
 *      create output parameter   
 */
	memolen = (out_bits.nbits / 8) + 1;

	out_apdu->octets = malloc(memolen);
	if (out_apdu->octets == NULL) {
		sct_errno = EMEMAVAIL;
		sct_errmsg = sca_errlist[sct_errno].msg;
		out_bits.nbits = 1;
		aux_free2_BitString(&out_bits);
		return (-1);
	}

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

#ifdef SCTTRACE
	fprintf(sct_trfp, "   out_apdu->noctets  = %d\n", out_apdu->noctets);
	fprintf(sct_trfp, "   out_apdu->octets   = \n");
	aux_fxdump(sct_trfp, out_apdu->octets, out_apdu->noctets, 0);
	fprintf(sct_trfp, "TRACE-END in SCTOutSecMessCONC\n");
#endif


	return (0);


}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      SCTOutSecMessCONC      */
/*-------------------------------------------------------------*/


/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  SCTInSecMessINT   VERSION   1.1                */
/*                              DATE   June     1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Check MAC  and SSC of a received SCT-RESPONSE-APDU    */
/*  This procedure can be called in case of               */
/*  secure messaging = AUTHENTIC or in case of            */
/*  secure messaging = COMBINED after calling the         */
/*  procedure SCTInSecMessCONC.                           */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   sec_key		       Secure Messaging key	  */
/*                             Algorithm Identifier       */
/*			       The following values are   */
/*			       possible:		  */
/*			       desCBC_ISO0	          */
/*			       desCBC3_ISO0	          */
/*							  */
/*							  */
/*   ssc		       Send Sequence Counter      */
/*							  */
/*   in_apdu		       Pointer of SEC-APDU        */
/*                             The ICC-APDU must have the */
/*                             structur:                  */
/*			       ________________________   */ 
/*			      | L,SSC,DATA,CCS,SW1,SW2 |  */
/*			       ________________________   */
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
/*			       | L,DATA,SW1,SW2  |        */ 	
/*		                _________________         */	
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   0                         o.k                        */
/*   -1                        Error                      */
/*				EMEMAVAIL		  */
/*				EDESENC  		  */
/*				ESSC			  */
/*				ERECMAC 	          */
/*				EALGO	                  */
/*				ENOINTKEY		  */
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
static int
SCTInSecMessINT(sec_key, ssc, in_apdu, out_apdu)
	KeyInfo      	*sec_key;	/* secure messaging key  */
	int             ssc;		/* Send sequence Counter */
	OctetString     *in_apdu;	/* SEC-APDU		 */
	OctetString     *out_apdu;	/* ICC-APDU		 */
{
	OctetString     in_octets;
	char           *ptr, *apdu_ptr, *mac_ptr;
	int             i, mac_len, data_len;
	int             memolen;
	BitString       out_bits;
	int             rec_ssc;
	char           *mac_field;
        char           *proc="SCTInSecMessINT";


/*****************************************************************************/
/*
 *	test parameter sec_key     
 */
	if ((sec_key->subjectkey.nbits <= 0) ||
	    (sec_key->subjectkey.bits == NULL)) {
		sct_errno = ENOINTKEY;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (-1);
	};

	if ((!sec_key->subjectAI) || (!sec_key->subjectAI->objid)) {
		sct_errno = EALGO;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (-1);
	}
	if ((aux_cmp_ObjId(sec_key->subjectAI->objid,desCBC_ISO0->objid)) &&
 	    (aux_cmp_ObjId(sec_key->subjectAI->objid,desCBC3_ISO0->objid))) {
		sct_errno = EALGO;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (-1);
	};


/*****************************************************************************/
/*
 *	 encrypt data (L,SSC,DATA,SW1,SW2) with Secure Messaging Key        
 */
	out_apdu->noctets = 0;
	out_apdu->octets = NULL;

#ifdef SCTTRACE 
	fprintf(sct_trfp, "TRACE in SCTInSecMessINT\n");
	fprintf(sct_trfp, "   in_apdu->noctets   = %d\n", in_apdu->noctets);
	fprintf(sct_trfp, "   in_apdu->octets    = \n");
	aux_fxdump(sct_trfp, in_apdu->octets, in_apdu->noctets, 0);
	fprintf(sct_trfp, "   sec_key->subjectkey.nbits     = %d\n", sec_key->subjectkey.nbits);
	fprintf(sct_trfp, "   sec_key->subjectkey.bits      = \n");
	aux_fxdump(sct_trfp, sec_key->subjectkey.bits, sec_key->subjectkey.nbits / 8, 0);
#endif



	if (in_apdu->noctets == 3) {
		/* only L=0,SW1,SW2 received */
		out_apdu->noctets = in_apdu->noctets;

		out_apdu->octets = malloc(out_apdu->noctets);	
		if (out_apdu->octets == NULL) {
			sct_errno = EMEMAVAIL;
			sct_errmsg = sca_errlist[sct_errno].msg;
			return (-1);
		}

		/* copy SW1,SW2 into out_apdu->octets */
		ptr = out_apdu->octets;
		for (i = 0; i < 3; i++)
			*ptr++ = *(in_apdu->octets + i);

#ifdef SCTTRACE 
		fprintf(sct_trfp, "   out_apdu->noctets  = %d\n", out_apdu->noctets);
		fprintf(sct_trfp, "   out_apdu->octets   = \n");
		aux_fxdump(sct_trfp, out_apdu->octets, out_apdu->noctets, 0);
		fprintf(sct_trfp, "TRACE-END in SCTInSecMessINT\n");
#endif

		return (0);
	}

	in_octets.noctets = (*in_apdu->octets & 0xFF) + 4;	/* 4 = L,SSC,SW1,SW2 */	
	mac_field = in_apdu->octets + ((*in_apdu->octets & 0xFF) + 2);
	mac_len   = 4;


	in_octets.octets = malloc(in_octets.noctets);	/* will be set free in
							 * this proc. */
	if (in_octets.octets == NULL) {
		sct_errno = EMEMAVAIL;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (-1);
	}

	/* copy L,SSC,DATA,SW1,SW2 from in_apdu->octets into in_octets.octets */
   	ptr      = in_octets.octets;
   	apdu_ptr = in_apdu->octets;

   	*ptr     = *apdu_ptr++;    		/* Length-field */
   	data_len = *ptr & 0xFF;
   	ptr++;

   	*ptr     = *apdu_ptr++;		/* SSC		*/
   	rec_ssc  = *ptr & 0xFF;
   	ptr++;

   	for (i=0; i<data_len; i++)		/* Data		*/
   	{
   	  	*ptr = *apdu_ptr++;
     		ptr++;
   	}

  	 apdu_ptr = in_apdu->octets + ((*in_apdu->octets & 0xFF) + 6);
   	*ptr++   = *apdu_ptr++;		/* SW1		*/
   	*ptr     = *apdu_ptr;		/* SW2		*/
    
   

#ifdef SCTTRACE 
	fprintf(sct_trfp, "   in_octets.noctets = %d\n", in_octets.noctets);
	fprintf(sct_trfp, "   in_octets.octets  = \n");
	aux_fxdump(sct_trfp, in_octets.octets, in_octets.noctets, 0);
	fprintf(sct_trfp, "   rec_ssc           = %x\n", rec_ssc);
	fprintf(sct_trfp, "   akt_ssc           = %x\n", (ssc & 0xFF) % 256);
	fprintf(sct_trfp, "   mac_field         = \n");
	aux_fxdump(sct_trfp, mac_field, mac_len, 0);
#endif




	/*
	 * allocate memory for out_bits             
	 * the memory must be a multiple of 8 Bytes 
	 */

	if ((in_octets.noctets % 8) != 0)
		memolen = (in_octets.noctets - (in_octets.noctets % 8)) + 8;
	else
		memolen = in_octets.noctets;

	out_bits.nbits = 0;

#ifdef SCTTRACE 
	fprintf(sct_trfp, "   allocate out_bits = %d\n", memolen);
#endif

	out_bits.bits = malloc(memolen);	/* will be set free in this
						 * proc. */
	if (out_bits.bits == NULL) {
		sct_errno = EMEMAVAIL;
		sct_errmsg = sca_errlist[sct_errno].msg;
		in_octets.noctets = 1;
		aux_free2_OctetString(&in_octets);
		return (-1);
	}


	/*
	 * encrypt data
	 */
	memolen = des_encrypt(&in_octets, &out_bits, SEC_END, sec_key);
	if (memolen == -1) {
		sct_errno = EDESENC;
		sct_errmsg = sca_errlist[sct_errno].msg;
		in_octets.noctets = 1;
		out_bits.nbits = 1;
		aux_free2_OctetString(&in_octets);
		aux_free2_BitString(&out_bits);
		return (-1);
	}
#ifdef SCTTRACE 
	fprintf(sct_trfp, "   out_bits.nbits    = %d\n", out_bits.nbits);
	fprintf(sct_trfp, "   out_bits.bits     = \n");
	aux_fxdump(sct_trfp, out_bits.bits, out_bits.nbits / 8, 0);
#endif

/*****************************************************************************/
/*
 *      check SSC
 */
	if (rec_ssc != ((ssc & 0xFF) % 256)) {
		sct_errno = ESSC;
		sct_errmsg = sca_errlist[sct_errno].msg;
		in_octets.noctets = 1;
		out_bits.nbits = 1;
		aux_free2_OctetString(&in_octets);
		aux_free2_BitString(&out_bits);

		return (-1);
	}
/*****************************************************************************/
/*
 *      check MAC							 
 *	if only 1 block encrypted => take the first 4 Bytes for MAC   
 *	else take the last 4 octets of the last block		
 */

      	/* check MAC	*/
	if ((out_bits.nbits / 8) > 8)
		mac_ptr = out_bits.bits + ((out_bits.nbits / 8) - 8);
	else
		mac_ptr = out_bits.bits;

#ifdef SCTTRACE 
	fprintf(sct_trfp, "   mac_ptr           = \n");
	aux_fxdump(sct_trfp, mac_ptr, mac_len, 0);
#endif


	for (i = 0; i < 4; i++) {
		if (mac_field[i] != *(mac_ptr + i)) {
			sct_errno = EMAC;
			sct_errmsg = sca_errlist[sct_errno].msg;
			in_octets.noctets = 1;
			out_bits.nbits = 1;
			aux_free2_OctetString(&in_octets);
			aux_free2_BitString(&out_bits);
			return (-1);
		}
	}



/*****************************************************************************/
/*
 *      create output parameter							 
 */
	out_apdu->octets = malloc(in_octets.noctets - 1); /* without SSC Bytes */	
	if (out_apdu->octets == NULL) {
		sct_errno = EMEMAVAIL;
		sct_errmsg = sca_errlist[sct_errno].msg;
		in_octets.noctets = 1;
		out_bits.nbits = 1;
		aux_free2_OctetString(&in_octets);
		aux_free2_BitString(&out_bits);
		return (-1);
	}

	out_apdu->noctets = in_octets.noctets - 1;
	ptr = out_apdu->octets;
      	*ptr++ = *in_octets.octets;
      	for (i=2; i<in_octets.noctets; i++)
      	{
         	*ptr = *(in_octets.octets + i);
          	ptr++;
      	}
	in_octets.noctets = 1;
	out_bits.nbits = 1;
	aux_free2_OctetString(&in_octets);
	aux_free2_BitString(&out_bits);

#ifdef SCTTRACE 
	fprintf(sct_trfp, "   out_apdu->noctets  = %d\n", out_apdu->noctets);
	fprintf(sct_trfp, "   out_apdu->octets   = \n");
	aux_fxdump(sct_trfp, out_apdu->octets, out_apdu->noctets, 0);
	fprintf(sct_trfp, "TRACE-END in SCTInSecMessINT\n");
#endif

	return (0);


}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      SCTInSecMessINT      */
/*-------------------------------------------------------------*/


/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  SCTInSecMessCONC   VERSION   1.1                */
/*                              DATE   June     1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Decrypt SEC-RESPONSE-APDU                             */
/*  This procedure can be called in case of               */
/*  secure messaging = CONCEALED or in case of            */
/*  secure messaging = COMBINED before calling the        */
/*  procedure SCTInSecMessINT.		                  */
/*  In case of secure messaging = CONCEALED this procedure*/
/*  also checks the SSC.				  */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   sec_key		       Secure Messaging key	  */
/*                             Algorithm Identifier       */
/*			       The following values are   */
/*			       possible:		  */
/*			       desCBC_ISO0	          */
/*			       desCBC3_ISO0	          */
/*							  */
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
/*			       | L = 0,SW1,SW2   |        */ 	
/*		                _________________         */
/*		                _________________         */
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
/*			      | L,SSC,DATA,CCS,SW1,SW2 |  */
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
/*				ENOCONCKEY		  */
/*						          */
/* CALLED FUNCTIONS					  */
/*   des_decrypt                                          */
/*   aux_fxdump                                           */
/*   aux_free2_OctetString				  */
/*   aux_cmp_ObjId                                        */
/*   malloc macro in MF_check.h				  */ 
/*							  */
/*--------------------------------------------------------*/
static int
SCTInSecMessCONC(sec_key, sec_mode, ssc, in_apdu, out_apdu)
	KeyInfo      	*sec_key;	/* secure messaging key  */
	ICC_SecMessMode  sec_mode;	/* security mode         */
	int             ssc;		/* Send sequence Counter */
	OctetString     *in_apdu;	/* SEC-APDU		 */
	OctetString     *out_apdu;	/* ICC-APDU		 */
{
	OctetString     out_octets;
	char           *ptr, *apdu_ptr;
	int             i, data_len;
	int             memolen;
	BitString       in_bits;
	int             rec_ssc;
	char		*proc="SCTInSecMessCONC";


/*****************************************************************************/
/*
 *	test parameter sec_key     
 */
	if ((sec_key->subjectkey.nbits <= 0) ||
	    (sec_key->subjectkey.bits == NULL)) {
		sct_errno = ENOCONCKEY;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (-1);
	};
	if ((!sec_key->subjectAI) || (!sec_key->subjectAI->objid)) {
		sct_errno = EALGO;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (-1);
	}
	if ((aux_cmp_ObjId(sec_key->subjectAI->objid,desCBC_ISO0->objid)) &&
 	    (aux_cmp_ObjId(sec_key->subjectAI->objid,desCBC3_ISO0->objid))) {
		sct_errno = EALGO;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (-1);
	};


/*****************************************************************************/
/*
 *      decrypt data with Secure Messaging Key
 */

#ifdef SCTTRACE 
	fprintf(sct_trfp, "TRACE in SCTInSecMessCONC\n");
	fprintf(sct_trfp, "   in_apdu->noctets   = %d\n", in_apdu->noctets);
	fprintf(sct_trfp, "   in_apdu->octets    = \n");
	aux_fxdump(sct_trfp, in_apdu->octets, in_apdu->noctets, 0);
	fprintf(sct_trfp, "   sec_key->subjectkey.nbits     = %d\n", sec_key->subjectkey.nbits);
	fprintf(sct_trfp, "   sec_key->subjectkey.bits      = \n");
	aux_fxdump(sct_trfp, sec_key->subjectkey.bits, sec_key->subjectkey.nbits / 8, 0);

#endif

	if (in_apdu->noctets == 3) {
		/* only L=0,SW1,SW2 received */
		out_apdu->noctets = in_apdu->noctets;

		out_apdu->octets = malloc(out_apdu->noctets);
		if (out_apdu->octets == NULL) {
			sct_errno = EMEMAVAIL;
			sct_errmsg = sca_errlist[sct_errno].msg;
			return (-1);
		}

		/* copy L,SW1,SW2 into out_apdu->octets */
		ptr = out_apdu->octets;
		for (i = 0; i < 3; i++) {
			*ptr = *(in_apdu->octets + i);
			ptr++;
		}

#ifdef SCTTRACE 
		fprintf(sct_trfp, "   out_apdu->noctets  = %d\n", out_apdu->noctets);
		fprintf(sct_trfp, "   out_apdu->octets   = \n");
		aux_fxdump(sct_trfp, out_apdu->octets, out_apdu->noctets, 0);
		fprintf(sct_trfp, "TRACE-END in SCTInSecMessCONC\n");
#endif

		return (0);
	}

	/* allocate memory for out_octets  */
	out_octets.noctets = 0;

	out_octets.octets = malloc(in_apdu->noctets);	/* will be set free in
							 * this proc. */
	if (out_octets.octets == NULL) {
		sct_errno = EMEMAVAIL;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (-1);
	}

	in_bits.nbits = in_apdu->noctets * 8;
	in_bits.bits = in_apdu->octets;

	/*
	 * decrypt data
	 */
	memolen = des_decrypt(&in_bits, &out_octets, SEC_END, sec_key);

	if (memolen == -1) {
		sct_errno = EDESDEC;
		sct_errmsg = sca_errlist[sct_errno].msg;
		out_octets.noctets = 1;
		aux_free2_OctetString(&out_octets);
		return (-1);
	}
#ifdef SCTTRACE 
	fprintf(sct_trfp, "   out_octets.noctets= %d\n", out_octets.noctets);
	fprintf(sct_trfp, "   out_octets.octets = \n");
	aux_fxdump(sct_trfp, out_octets.octets, out_octets.noctets, 0);
#endif

	if (sec_mode == SCT_CONCEALED) {
		/*
		 * CONCEALED-Mode -> Test SSC; return L,DATA,SW1,SW2 
		 * allocate out_data->octets                      
		 */
		out_apdu->noctets = (*out_octets.octets & 0xFF) + 3; /* 3 = L,Sw1,Sw2 */	
		out_apdu->octets = malloc(out_apdu->noctets);
		if (out_apdu->octets == NULL) {
			sct_errno = EMEMAVAIL;
			sct_errmsg = sca_errlist[sct_errno].msg;
			out_octets.noctets = 1;
			aux_free2_OctetString(&out_octets);
			return (-1);
		}

		/*
		 * copy L,DATA,SW1,SW2 from out_octets.octets into
		 * out_apdu->octets
		 */
       		ptr      = out_apdu->octets;
       		apdu_ptr = out_octets.octets;
       		*ptr     = *apdu_ptr++;    		/* Length-field */
       		data_len = *ptr & 0xFF;
       		ptr++;
       		rec_ssc  = *apdu_ptr++ & 0xFF;		/* SSC		*/




#ifdef SCTTRACE 
		fprintf(sct_trfp, "   rec_ssc           = %x\n", rec_ssc);
		fprintf(sct_trfp, "   akt_ssc           = %x\n", (ssc & 0xFF) % 256);
#endif

		/* check SSC	 */
		if (rec_ssc != ((ssc & 0xFF) % 256)) {
			sct_errno = ESSC;
			sct_errmsg = sca_errlist[sct_errno].msg;
			out_octets.noctets = 1;
			out_apdu->noctets = 1;
			aux_free2_OctetString(&out_octets);
			aux_free2_OctetString(out_apdu);
			return (-1);
		}
		for (i = 0; i < data_len+2; i++) {	/* Data, SW1, SW2 */
			*ptr = *apdu_ptr++;
			ptr++;

		}
		out_octets.noctets = 1;
		aux_free2_OctetString(&out_octets);
	} else {
		/*
		 * COMBINED-Mode-> return L,SSC,DATA,CCS,SW1,SW2     
		 */
		out_apdu->noctets = out_octets.noctets;

		out_apdu->octets = out_octets.octets;	

	}

#ifdef SCTTRACE 
	fprintf(sct_trfp, "   out_apdu->noctets  = %d\n", out_apdu->noctets);
	fprintf(sct_trfp, "   out_apdu->octets   = \n");
	aux_fxdump(sct_trfp, out_apdu->octets, out_apdu->noctets, 0);
	fprintf(sct_trfp, "TRACE-END in SCTInSecMessCONC\n");
#endif

	return (0);



}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      SCTInSecMessCONC      */
/*-------------------------------------------------------------*/


/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  SCTalloc            VERSION   1.1                */
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
/*   datalen                   length of datafield        */
/*                                                        */
/*   secure                    secure messaging           */
/*                                                        */
/* OUT                                                    */
/*   pdulen                     length of s_apdu          */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   pointer                   o.k.                       */
/*                                                        */
/*   NULL                      error                      */
/*                              EMEMAVAIL                 */
/*                                                        */
/* CALLED FUNCTIONS                                       */
/*   malloc macro in MF_check.h				  */ 
/*--------------------------------------------------------*/
static char    *
SCTalloc(datalen, secure, pdulen)
	unsigned int    datalen;
	SCT_SecMess        *secure;
	unsigned int   *pdulen;
{
	char           *buffer = NULL;
	unsigned int    modulus;
	char		*proc="SCTalloc";

	if (datalen < 255)
		*pdulen = datalen + LHEADER + LEN1;
	else
		*pdulen = datalen + LHEADER + LEN3;

/*****************************************************************************/
/*
 *       the length depends on secure messaging    
 */
	if (secure->command != SCT_SEC_NORMAL)
		(*pdulen)++;	/* 1 Byte for SSC */



	buffer = malloc(*pdulen);	/* if no error => return 		 */
	if (buffer == NULL) {
		sct_errno = EMEMAVAIL;
		sct_errmsg = sca_errlist[sct_errno].msg;
	}



	return (buffer);
}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      SCTalloc               */
/*-------------------------------------------------------------*/



/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  SCToctetstring       VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  create datafield in APDU                              */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   apdu                       Pointer of APDU-buffer    */
/*                                                        */
/*   data                       Pointer of data           */
/*                                                        */
/*   len                        length  of data           */
/*                                                        */
/* OUT                                                    */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*                                                        */
/* CALLED FUNCTIONS                                       */
/*--------------------------------------------------------*/
static void
SCToctetstring(apdu, data)
	char          **apdu;
	OctetString    *data;
{
	int             i;
	char           *p;


	p = *apdu;
	if (data != NULLOCTETSTRING) {
		for (i = 0; i < data->noctets; i++) {
			*p = *(data->octets + i);
			p++;
		};
	}
	*apdu = p;
}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      SCToctetstring          */
/*-------------------------------------------------------------*/



/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  SCTparam            VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  create Parameter in APDU                              */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   apdu                       Pointer of APDU-buffer    */
/*                                                        */
/*   p1                         first parameter           */
/*                                                        */
/*   p2                         second parameter          */
/*                                                        */
/* OUT                                                    */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*                                                        */
/* CALLED FUNCTIONS                                       */
/*--------------------------------------------------------*/
static void
SCTparam(apdu, p1, p2)
	char          **apdu;
	unsigned int    p1;
	unsigned int    p2;
{
	char           *p;


	p = *apdu;

	*p++ = p1;
	*p++ = p2;
	*apdu = p;
}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      SCTparam               */
/*-------------------------------------------------------------*/



/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  SCTplength          VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  create Length in APDU                                 */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   apdu                       Pointer of APDU-buffer    */
/*                                                        */
/*   datalen                    length of datafield       */
/*                                                        */
/* OUT                                                    */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*                                                        */
/* CALLED FUNCTIONS                                       */
/*--------------------------------------------------------*/
static void
SCTplength(apdu, ldata)
	char          **apdu;
	unsigned int    ldata;
{
	char           *p;


	p = *apdu;
	if (ldata < 255)
		*p++ = ldata;
	else {
		*p++ = 0xFF;
		*p++ = ldata >> 8;
		*p++ = ldata;
	};
	*apdu = p;
}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      SCTplength             */
/*-------------------------------------------------------------*/




/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  SCTbinval           VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  create binary value in APDU                           */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   apdu                       Pointer of APDU-buffer    */
/*                                                        */
/*   value                      integer value             */
/*                                                        */
/* OUT                                                    */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*                                                        */
/*                                                        */
/* CALLED FUNCTIONS                                       */
/*--------------------------------------------------------*/
static void
SCTbinval(apdu, binval)
	char          **apdu;
	unsigned int    binval;
{
	char           *p;


	p = *apdu;
	if (binval < 255)
		*p++ = binval;
	else {
		*p++ = binval >> 8;
		*p++ = binval;
	};
	*apdu = p;
}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      SCTbinval              */
/*-------------------------------------------------------------*/



/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  SCTppublic          VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  create parameter public in APDU                       */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   apdu                      Pointer of APDU-buffer     */
/*                                                        */
/*   public                    Pointer of public structure*/
/*                                                        */
/* OUT                                                    */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*                                                        */
/* CALLED FUNCTIONS                                       */
/*   SCTplength                                           */
/*   SCToctetstring                                        */
/*--------------------------------------------------------*/
static void
SCTppublic(apdu, public)
	char          **apdu;
	Public         *public;
{
	char           *p;


	p = *apdu;
	if (public != PUBNULL) {
		if (public->modulus == NULLOCTETSTRING)
			SCTplength(&p, S_NOTUSED);
		else
		{
			SCTplength(&p, public->modulus->noctets);
			SCToctetstring(&p, public->modulus);
		}
	};

	*apdu = p;
}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      SCTppublic             */
/*-------------------------------------------------------------*/






/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  SCTverifyinfo       VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  create parameter VERRecord  in APDU                   */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   apdu                      Pointer of APDU-buffer     */
/*                                                        */
/*   pininfo                   Pointer of PINinfo         */
/*                             structure                  */
/*                                                        */
/* OUT                                                    */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*                                                        */
/* CALLED FUNCTIONS                                       */
/*   SCTbinval                                            */
/*   icc_e_KeyAttrList                                    */
/*   SCTplength                                           */
/*   SCToctetstring                                       */
/*--------------------------------------------------------*/
static void
SCTverifyinfo(apdu, verinfo)
	char          **apdu;
	VERRecord      *verinfo;
{
	char           *p;

	p = *apdu;

	SCTbinval(&p, verinfo->key_attr_list->key_inst_mode);

	icc_e_KeyAttrList(&p, verinfo->key_attr_list, verinfo->key_algid);


	SCTplength(&p, verinfo->ver_data->noctets);


	SCToctetstring(&p, verinfo->ver_data);



	*apdu = p;
}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      SCTverifyinfo          */
/*-------------------------------------------------------------*/


/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  SCTssc              VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  if secure messaging -> then create ssc in APDU        */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   apdu                      Pointer of APDU-buffer     */
/*                                                        */
/*   p_elem                    Pointer of portparam       */
/*                             structure                  */
/*                                                        */
/* OUT                                                    */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*                                                        */
/* CALLED FUNCTIONS                                       */
/*--------------------------------------------------------*/
static void
SCTssc(apdu, p_elem)
	char          **apdu;
	struct s_portparam *p_elem;
{
	char           *p;

	p = *apdu;
	if (p_elem->secure_messaging.command != SCT_SEC_NORMAL) {
		if (p_elem->ssc != 0)
			p_elem->ssc = p_elem->ssc % 256;

		*p++ = p_elem->ssc;
		p_elem->ssc++;
	}
	*apdu = p;
}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      SCTssc                 */
/*-------------------------------------------------------------*/


/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  SCTclass            VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  generate class byte for the SCT commands      /       */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*                                                        */
/*   security                  Pointer of secure messaging*/
/*                             structure                  */
/*                                                        */
/* OUT                                                    */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   class-byte                value of the class-byte    */
/*                                                        */
/*                                                        */
/* CALLED FUNCTIONS                                       */
/*                                                        */
/*--------------------------------------------------------*/
static unsigned int
SCTclass(security)
	SCT_SecMess        *security;
{
	unsigned int    class;


	class = NON_INTER;

	if (security->command != SCT_SEC_NORMAL)
		class |= (unsigned) security->command << 2;

	if (security->response != SCT_SEC_NORMAL)
		class |= (unsigned) security->response;


	return (class);
}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      SCTclass               */
/*-------------------------------------------------------------*/

/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  ICCclass            VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  generate the security parameter for                   */
/*  S_MODIFY_VERIFICATION_DATA,                           */
/*  S_PERFORM_VERIFICATION                                */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*                                                        */
/*   icc_secmess               Pointer of icc secure      */
/*                             messaging structure        */
/*                                                        */
/* OUT                                                    */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   class-byte                value of the class-byte    */
/*                                                        */
/*                                                        */
/* CALLED FUNCTIONS                                       */
/*                                                        */
/*--------------------------------------------------------*/
static unsigned int
ICCclass(icc_secmess)
	ICC_SecMess        *icc_secmess;
{
	unsigned int    class;


	class = 0x00;

	if (icc_secmess->command != ICC_SEC_NORMAL)
		class |= (unsigned) icc_secmess->command << 2;

	if (icc_secmess->response != ICC_SEC_NORMAL)
		class |= (unsigned) icc_secmess->response;


	return (class);
}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      ICCclass               */
/*-------------------------------------------------------------*/


/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  SCTwithNMdata       VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  create APDU (P1,P2,Data); Datafield ist not mandatory */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   p_elem                   pointer of portparam struct.*/
/*                                                        */
/*   p1                         Parameter 1               */
/*                                                        */
/*   p2                         parameter 2               */
/*                                                        */
/*   data                       datafield (OctetString)   */
/*                                                        */
/*                                                        */
/* OUT                                                    */
/*   lapdu                      length of APDU-Buffer     */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   Pointer                   Pointer to APDU-buffer     */
/*                                                        */
/*   NULL                      error                      */
/*                                                        */
/* CALLED FUNCTIONS                                       */
/*   SCTalloc                  Error                      */
/*                              EMEMAVAIL;                */
/*   SCTparam                                             */
/*   SCTplength                                           */
/*   SCTssc                                               */
/*   SCToctetstring                                       */
/*--------------------------------------------------------*/
static char    *
SCTwithNMdata(p_elem, p1, p2, data, lapdu)
	struct s_portparam *p_elem;
	unsigned int    p1;
	unsigned int    p2;
	OctetString    *data;
	unsigned int   *lapdu;
{
	char           *apdu;
	char           *p;
	unsigned int    ldata;
	unsigned int    len;



	if ((data == NULLOCTETSTRING) || (data->octets == NULL))
		ldata = 0;
	else
		ldata = data->noctets;

	if ((apdu = SCTalloc(ldata, &p_elem->secure_messaging, &len)) != NULL) {

		p = apdu + 2;

		SCTparam(&p, p1, p2);

		SCTplength(&p, ldata);
		SCTssc(&p, p_elem);
		SCToctetstring(&p, data);

		*lapdu = len;
	};
	return (apdu);

}



/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E       SCTwithNMdata         */
/*-------------------------------------------------------------*/



/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  SCTwithMdata        VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  create APDU (P1,P2,Data); datafield ist mandatory     */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   p_elem                   pointer of portparam struct.*/
/*                                                        */
/*   p1                         Parameter 1               */
/*                                                        */
/*   p2                         parameter 2               */
/*                                                        */
/*   data                       datafield (OctetString)   */
/*                                                        */
/*                                                        */
/* OUT                                                    */
/*   lapdu                      length of APDU-Buffer     */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   Pointer                   Pointer to APDU-buffer     */
/*                                                        */
/*   NULL                      error                      */
/*			 	EPARINC			  */ 
/*                                                        */
/* CALLED FUNCTIONS                                       */
/*   SCTalloc                  Error                      */
/*                              EMEMAVAIL;                */
/*   SCTparam                                             */
/*   SCTplength                                           */
/*   SCTssc                                               */
/*   SCToctetstring                                        */
/*--------------------------------------------------------*/
static char    *
SCTwithMdata(p_elem, p1, p2, data, lapdu)
	struct s_portparam *p_elem;
	unsigned int    p1;
	unsigned int    p2;
	OctetString    *data;
	unsigned int   *lapdu;
{
	char           *apdu;
	char           *p;
	unsigned int    ldata;
	unsigned int    len;


	if ((data == NULLOCTETSTRING) || (data->octets == NULL)) {
		sct_errno = EPARINC;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (NULL);
	}


	ldata = data->noctets;


	if ((apdu = SCTalloc(ldata, &p_elem->secure_messaging, &len)) != NULL) {
		p = apdu + 2;

		SCTparam(&p, p1, p2);

		SCTplength(&p, ldata);
		SCTssc(&p, p_elem);
		SCToctetstring(&p, data);

		*lapdu = len;
	};
	return (apdu);

}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E       SCTwithMdata          */
/*-------------------------------------------------------------*/



/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC   SCTnodata          VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  create APDU without datafield                         */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   p_elem                   pointer of portparam struct.*/
/*                                                        */
/*   p1                         Parameter 1               */
/*                                                        */
/*   p2                         parameter 2               */
/*                                                        */
/*                                                        */
/*                                                        */
/* OUT                                                    */
/*   lapdu                      length of APDU-Buffer     */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   Pointer                   Pointer to APDU-buffer     */
/*                                                        */
/*   NULL                      error                      */
/*                                                        */
/* CALLED FUNCTIONS                                       */
/*   SCTalloc                  Error                      */
/*                              EMEMAVAIL;                */
/*   SCTssc                                               */
/*--------------------------------------------------------*/
static char    *
SCTnodata(p_elem, p1, p2, lapdu)
	struct s_portparam *p_elem;
	unsigned int    p1;
	unsigned int    p2;
	unsigned int   *lapdu;
{
	char           *apdu;
	char           *p;
	unsigned int    ldata = 0;
	unsigned int    len;

	if ((apdu = SCTalloc(ldata, &p_elem->secure_messaging, &len)) != NULL) {

		p = apdu + 2;
		*p++ = p1;
		*p++ = p2;
		*p++ = ldata;

		SCTssc(&p, p_elem);

		*lapdu = len;
	};
	return (apdu);

}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E       SCTnodata             */
/*-------------------------------------------------------------*/




/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  LofPublic           VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  calculate length of public - structure                */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*   public                    Pointer of public structure*/
/*                                                        */
/* OUT                                                    */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   len                       length of data             */
/*                                                        */
/*                                                        */
/* CALLED FUNCTIONS                                       */
/*                                                        */
/*--------------------------------------------------------*/
static unsigned int
LofPublic(public)
	Public         *public;
{
	unsigned int    len = 0;

	if (public == PUBNULL)
		return (len);

	if (public->modulus != NULLOCTETSTRING) {
		len += public->modulus->noctets;
		if (len >= 255)
			len += 2;
	};
	len++;

	return (len);
}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      LofPublic              */
/*-------------------------------------------------------------*/

/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  SCTelemlen          VERSION   1.1                */
/*                              DATE   January 1993       */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Eleminate Length field in response-buffer             */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* INOUT                     DESCRIPTION                  */
/*  resp                      response-structure          */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/*--------------------------------------------------------*/
static
void 
SCTelemlen(resp)
	OctetString     *resp;
{
	/*----------------------------------------------------------*/
	/* Definitions                                            */
	/*----------------------------------------------------------*/
	char           *p;
	int             offset;
	int             i;

	/*----------------------------------------------------------*/
	/* Statements                                             */
	/*----------------------------------------------------------*/
	p = resp->octets;
	offset = 1;

	if ((resp->noctets = ((int) *p) & 0xFF) >= 255) {
		p++;
		resp->noctets = ((((int) *p++) & 0xff) << 8);
		resp->noctets += (((int) *p) & 0xFF);
		offset = 3;
	}
	else
		resp->noctets = (((int) *p) & 0xFF);

	resp->noctets += 2;

	for (i = 0; i < resp->noctets; i++)
		*(resp->octets + i) = *(resp->octets + i + offset);

}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      SCTelemlen             */
/*-------------------------------------------------------------*/




/*-------------------------------------------------------------*/
/* E N D   O F   P A C K A G E       sctde                    */
/*-------------------------------------------------------------*/
