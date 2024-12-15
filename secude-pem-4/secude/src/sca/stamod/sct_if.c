/*
 *  STARMOD Release 1.1 (GMD)
 */
/*-------------------------------------------------------+-----*/
/*							 | GMD */
/*   SYSTEM   STARMOD  -  Version 1.1			 +-----*/
/*							       */
/*-------------------------------------------------------------*/
/*							       */
/*    PACKAGE	SCT			VERSION 1.1	       */
/*					   DATE January  1993  */
/*					     BY Levona Eckstein*/
/*							       */
/*    FILENAME					               */
/*	sct_if.c		         		       */
/*							       */
/*    DESCRIPTION					       */
/*	SCT - Interface - Module			       */
/*							       */
/*    EXPORT		    DESCRIPTION 		       */
/*	sct_reset()	      Reset SmartCard terminal	       */
/*							       */
/*	sct_interface()       Send Command / receive response  */
/*							       */
/*	sct_perror()	      Print error message	       */
/*							       */
/*	sct_info()	      information about sct/sc	       */
/*							       */
/*	sct_list()	      list of installed sct's          */
/*							       */
/*      sct_close()           close port of SCT                */
/*							       */
/*      sct_secure()          set sessionkey                   */
/*							       */
/*      sct_init_secure()     delete sessionkey                */
/*							       */
/*      sct_setmode()         set security mode DTE-SCT        */
/*							       */
/*      sct_checksecmess()    check secure messaging           */
/*							       */
/*      sct_getsecmess()      get secure messaging             */
/*							       */
/*      sct_get_errmsg()      get SCT error message            */
/*              					       */
/*      sct_init_port()       initialize port memory           */
/*              					       */
/*      sct_get_port()        get port memory                  */
/*              					       */
/*      sct_set_portpar()     set port parameter               */
/*                            in this version, only icc_request*/
/*  			      will be set		       */ 
/*              					       */
/*	sct_errno	      error number		       */
/*							       */
/*	sct_errmsg	      address of error message	       */
/*							       */
/*	sctif.h					               */
/*							       */
/*	sctrc.h 					       */
/*							       */
/*                                                             */
/*							       */
/*							       */
/*    IMPORT		    DESCRIPTION 		       */
/*							       */
/*	aux_free2_OctetString  release byte-buffer	       */
/*							       */
/*	SCTDEcreate	      create S-Command		       */
/*							       */
/*	SCTDEerr	      error-handling		       */
/*							       */
/*	SCTDEcheck	      check 1 or 3 octets	       */
/*							       */
/*	SCTDEresponse         analyse the response	       */
/*							       */
/*	MEMGetOrgElem	      read org. element out of install */
/*			      file			       */
/*							       */
/*	MEMGetIdElem	      read element out of sct-list     */
/*							       */
/*	MEMCreateList	      create sct-list		       */
/*							       */
/*      MEMPrintElem          print sct-element if TRACE       */
/*                                                             */
/*	COMinit 	      Port-Initialisation	       */
/*							       */
/*	COMreset	      Port-Reset		       */
/*							       */
/*	COMtrans	      Transfer of S-APDU               */
/*							       */
/*	COMclose	      Close port		       */
/*							       */
/*    INTERNAL              DESCRIPTION                        */
/*	sct_status	      send S_STATUS      	       */
/*	time_int	      Signal - Routine      	       */
/*-------------------------------------------------------------*/

/*-------------------------------------------------------------*/
/*   include-Files					       */
/*-------------------------------------------------------------*/
#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include "sca.h"
#include "stamod.h"

#ifdef _BSD42_
/*#define B19200 14 */
#include <sys/ttydev.h>

#elif _POSIX_
#include <sgtty.h>

#endif

/*-------------------------------------------------------------*/
/*   extern declarations				       */
/*-------------------------------------------------------------*/
extern unsigned int tp1_err;	/* error-variable from transmission module */
extern int      COMinit();
extern int      COMreset();
extern int      COMtrans();
extern int      COMclose();

extern struct s_portparam *p_lhead;	/* Begin of sct_list from sctmem.c */
extern struct s_portparam *MEMGetIdElem();
extern int      MEMCreateList();
extern int      MEMGetOrgElem();
extern void     MEMPrintElem();

extern void     aux_free2_OctetString();

extern char    *SCTDEcreate();
extern int      SCTDEerr();
extern int      SCTDEcheck();
extern int      SCTDEresponse();



/*-------------------------------------------------------------*/
/*   type definitions					       */
/*-------------------------------------------------------------*/


#define BITNULL		(BitString *)0



/*-------------------------------------------------------------*/
/*   globale variable definitions			       */
/*-------------------------------------------------------------*/
unsigned int    sct_errno;	/* error variable		 */
char           *sct_errmsg;	/* error message		 */

/*-------------------------------------------------------------*/
/*   lokale Variable definitions			       */
/*-------------------------------------------------------------*/
/* SW2 = SCT wait */
#define SCTWAIT                 0x01


#ifdef SCTTRACE
static Boolean     first = FALSE;	/* FLAG, if Trace-File open	 */
FILE           *sct_trfp;	/* Filepointer of trace file    */

#endif
static Boolean     resfirst = FALSE;	/* FLAG, if sct_reset already called */

static char     fermat_f4[3] = {'\001', '\000', '\001'};	/* public exponent                  */
static int      fermat_f4_len = 3;

static unsigned char	transport_key[] =  {0xA2, 0x6F, 0xA6, 0xA2, 0xF8, 0x5E, 0x83, 0x8D,
				    	    0xBE, 0x62, 0x34, 0xC1, 0xCB, 0xF7, 0x2F, 0xBF,
				            0xBD, 0x94, 0x73, 0xEF, 0xBC, 0x46, 0xD6, 0xB4,
				            0xDB, 0x24, 0x31, 0x39, 0xDE, 0x0C, 0x68, 0x49,
				            0x8A, 0x67, 0xD5, 0xFD, 0x2A, 0x6D, 0x3A, 0x70,
				            0x2A, 0xE4, 0xDC, 0xD1, 0x07, 0x87, 0xD5, 0x31,
				            0x0E, 0x27, 0x0C, 0x45, 0x6A, 0x28, 0x1A, 0xE1,
				            0x22, 0x93, 0xC7, 0xD0, 0xD5, 0xF6, 0xEF, 0x6D};
static int	transport_keylen = 64;




static int sct_status();
static int sta_itoa();

/*--------------------------------------------------------*/
/*                                                  | GMD */
/* Signal - Routine                                 +-----*/
/*                                                        */
/*--------------------------------------------------------*/
static
void time_int(dummy)
int	dummy;
{
/* Signal SIGALRM received      */
}



/*--------------------------------------------------------*/
/*						    | GMD */
/*						    +-----*/
/* PROC  sct_reset	     VERSION   1.1		  */
/*				DATE   January  1993	  */
/*				  BY   L.Eckstein,GMD	  */
/*							  */
/* DESCRIPTION						  */
/*  Reset Smartcard Terminal				  */
/*  If this function is called the first time, then	  */
/*  the installation file will be opened and the	  */
/*  sct-list for further use will be created.		  */
/*							  */
/*							  */
/*							  */
/*							  */
/*							  */
/* IN			     DESCRIPTION		  */
/*   sct_id		       SCT identifier		  */
/* OUT							  */
/*							  */
/*							  */
/*							  */
/* RETURN		     DESCRIPTION		  */
/*   0	         	       o.k			  */
/*  -1			       error			  */
/*				EBAUD                     */
/*                                                        */
/* CALLED FUNCTIONS					  */
/*  MEMCreateList	      error			  */
/*				ENOSHELL		  */
/*                              EOPERR                    */
/*			        EEMPTY                    */
/*                              EMEMAVAIL                 */
/*                              ECLERR                    */
/*  MEMGetIdElem              error			  */
/*                              ESIDUNK                   */
/*  MEMGetOrgElem	      error			  */
/*			        ESIDUNK                   */
/*                              ENOSHELL                  */
/*                              EOPERR                    */
/*                              EEMPTY                    */
/*                              ERDERR                    */
/*                              ECLERR                    */
/*  COMinit		      error			  */
/*                              T1 - ERROR                */
/*  COMclose		      error			  */
/*                              T1 - ERROR                */
/*  COMreset		      error			  */
/*                              T1 - ERROR                */
/*  sct_interface	      error			  */
/*                             EINVARG			  */
/*                             ETOOLONG		          */
/*                             EMEMAVAIL		  */
/*                             ESIDUNK                    */
/*			       EINS			  */
/*                             T1 - ERROR                 */
/*                             sw1/sw2 from SCT response  */
/*  aux_free2_OctetString				  */
/*  							  */
/*--------------------------------------------------------*/
int
sct_reset(sct_id, proc)
	int             sct_id;	/* SCT identifier 	 */
	char            *proc;
{



	int             i;
	struct s_portparam *p_elem;
	unsigned int    sw1 = 0;
	OctetString     response;
	Request         request;
	int             baud;
	int             databits;
	int             stopbits;
	char           *p;
	int             div = 19200;
	int             index;
	int             resetbaud;
	Boolean		getorg;
	char		file_name[30];
	char		pid[10];

	sct_errno = 0;

#ifdef SCTTRACE
	if (!first) {
		strcpy(file_name,"SCT");
		sta_itoa(pid,getpid());
		strcat(file_name,pid);
		strcat(file_name,".TRC");

		sct_trfp = fopen(file_name, "wt");
		first = TRUE;
	};
	fprintf(sct_trfp, "sct_reset called from : %s\n", proc);
#endif


/*****************************************************************************/
/*
 *      Create sct_list
 */

	if (!resfirst) {	
		if (MEMCreateList() == -1)
			return (S_ERR);
		resfirst = TRUE;
		getorg   = TRUE;
	}
	else
		getorg   = FALSE;




/*****************************************************************************/
/*
 *      test, if sct_id refers to a sct element
 */

	if ((p_elem = MEMGetIdElem(sct_id)) == PORTNULL)
		return (S_ERR);	/* ERROR: sct not in sct-list */

#ifdef ELEMTRACE
	fprintf(sct_trfp, "Element after MEMGetIdElem in sct_reset\n");
	MEMPrintElem(sct_trfp, p_elem);
#endif



/*****************************************************************************/
/*
 *      set sad, dad in sct_element and call COMinit
 */

	p_elem->sad = SAD;
	p_elem->dad = DAD;

	if (p_elem->port_id > 0) {
		COMclose(p_elem);
		getorg = TRUE;
	}

	if (getorg == TRUE) {
		if (MEMGetOrgElem(sct_id, p_elem) == -1)
			return (S_ERR);
		getorg = FALSE;
	}

	if ((COMinit(p_elem)) == -1)
		return (SCTDEerr(sw1, tp1_err));




/*****************************************************************************/
/*
 *      send RESET - Command
 */	

	if (sct_interface(sct_id, S_RESET, &request, &response) == -1) {
		response.noctets = 1;
		aux_free2_OctetString(&response);
		return (S_ERR);
	}


/*****************************************************************************/
/*
 *      set reset-values in p_elem
 */
	baud = p_elem->baud;
	databits = p_elem->databits;
	stopbits = p_elem->stopbits;
	p = response.octets;
	p_elem->dataformat = *p++;
	p_elem->protocoltype = *p++;
	p_elem->bwt = *p++;
	p_elem->cwt = *p++;
	p_elem->chaining = (Chain) * p++;

	p_elem->baud = 0;
	p_elem->baud = ((((int) *p++) & 0xff) << 8);
	p_elem->baud += (((int) *p++) & 0xFF);

#ifdef _DOS_
	p_elem->baud = (div / p_elem->baud) * 6;
#endif

/*****************************************************************************/
/*
 *      release response-buffer
 */


	response.noctets = 1;
	aux_free2_OctetString(&response);


/*****************************************************************************/
/*
 *      test baudrate
 */
#ifdef _BSD42_
	if (p_elem->baud < 2400) {
		COMclose(p_elem);

		if (MEMGetOrgElem(sct_id, p_elem) == -1)
			return (S_ERR);
		sct_errno = EBAUD;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (S_ERR);
	}
	index = 0;
	resetbaud = p_elem->baud;
	while ((div / resetbaud) != 1) {
		resetbaud = resetbaud * 2;
		index++;
	}
	p_elem->baud = B19200 - index;

#else
#ifdef _MACC7_
	if (p_elem->baud < 2400) {
      		COMclose(p_elem);

      		if (MEMGetOrgElem(sct_id,p_elem) == -1)
         		return(-1);
      		sct_errno = EBAUD;
      		sct_errmsg = sca_errlist[sct_errno].msg;
      		return(S_ERR);
      	}
   
      	p_elem->baud = MacBaud(p_elem->baud);
      
#endif /* _MACC7_ */
#endif

	if (*p++ == 0x07)
		p_elem->databits = DATA_7;
	else
		p_elem->databits = DATA_8;
	if (*p++ == 0x01)
		p_elem->stopbits = STOP_1;
	else
		p_elem->stopbits = STOP_2;

	p_elem->edc = (EdcType) * p++;
	p_elem->tpdusize = SCTDEcheck(&p);
	/* apdusize = tpdusize - length of TPDU-Header - Length of EDC */
	p_elem->apdusize = p_elem->tpdusize - 3 - ((p_elem->edc == E_LRC) ? 1 : 2);


/*****************************************************************************/
/*
 *      if baud / databits / stopbits changed, then call COMreset
 */

	if ((baud != p_elem->baud) || (databits != p_elem->databits) ||
	    (stopbits != p_elem->stopbits)) {

		if (COMreset(p_elem) == -1) {
			COMclose(p_elem);


			if (MEMGetOrgElem(sct_id, p_elem) == -1)
				return (S_ERR);
			return (SCTDEerr(sw1, tp1_err));
		}
	}


#ifdef ELEMTRACE
	MEMPrintElem(sct_trfp, p_elem);
	fprintf(sct_trfp, "END of sct_reset\n");
#endif


	return (S_NOERR);

}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sct_reset	       */
/*-------------------------------------------------------------*/





/*--------------------------------------------------------*/
/*						    | GMD */
/*						    +-----*/
/* PROC  sct_interface	     VERSION   1.1		  */
/*				DATE   January  1993	  */
/*				  BY   L.Eckstein,GMD	  */
/*							  */
/* DESCRIPTION						  */
/*  Creat the s_apdu.					  */
/*  The sct_interface checks the mandatory parameter	  */
/*  in the body and the validity of the Instruction-Code. */
/*  It constructs the CLASS-Byte and executes the secure  */
/*  messaging functions.				  */
/*  The memory for the s_apdu is provided by this program.*/
/*  Send the s_apdu to the SCT. 			  */
/*  Receive the response from the SCT.			  */
/*  The memory for the response-apdu is provided by this  */
/*  program (response.octets);				  */
/*  Execute the secure messaging functions.		  */
/*  Check the SW1 / SW2 - Byte. 			  */
/*  In case of O.K., sct_interface returns a pointer to   */
/*  the response-buffer response.octets. 		  */
/*  The response-buffer contains only the datafield	  */
/*  without SW1 / SW2.					  */
/*  If SW1/SW2 indicates an error, sct_interface returns  */
/*  the value -1 and in sct_errno the error number.	  */
/*							  */
/*							  */
/*							  */
/* IN			     DESCRIPTION		  */
/*   int sct_id 	       SCT-Identifier		  */
/*							  */
/*   command		       instruction code 	  */
/*							  */
/*   pointer		       request structure	  */
/*							  */
/*   pointer		       response structure	  */
/*							  */
/* OUT							  */
/*   pointer		       response.octets		  */
/*			       will be allocated by this  */
/*			       procedure and must be set  */
/*			       free by the calling proc.  */
/*			       but only in case of no err.*/
/*							  */
/*							  */
/*							  */
/* RETURN		     DESCRIPTION		  */
/*   0			       o.k.			  */
/*   1			       SCT waiting	          */
/*   2			       Key in SCT replaced	  */
/*   3			       Signature correct, but	  */
/*			       key to short		  */
/*   4			       PIN-CHECK off from SC	  */
/*   5			       PIN-CHECK on  from SC	  */
/*  -1			       error			  */
/*                             EINVARG			  */
/*                             ETOOLONG		          */
/*                             EMEMAVAIL		  */
/*							  */
/* CALLED FUNCTIONS					  */
/*  MEMGetIdElem	      error			  */
/*                              ESIDUNK                   */
/*  SCTDEcreate		      error			  */
/*				EINS			  */
/*			        EMEMAVAIL                 */
/*				EDESENC  		  */
/*				EMAC     		  */
/*				EALGO    		  */
/*				ENOINTKEY	          */
/*				ENOCONCKEY	          */
/*  SCTDEresponse	      error			  */
/*                              sw1/sw2 from SCT response */
/*				EMEMAVAIL		  */
/*				EDESDEC  		  */
/*				ESSC			  */
/*				EDESENC  		  */
/*				ERECMAC 	          */
/*				EALGO	                  */
/*				ENOCONCKEY		  */
/*				ENOINTKEY		  */
/*  sct_status   	      error			  */
/*			       EMEMAVAIL		  */
/*                             ETOOLONG                   */
/*			       EINS			  */
/*                             T1 - ERROR                 */
/*                             sw1/sw2 from SCT response  */
/*			       EDESDEC  		  */
/*			       EDESENC  		  */
/*			       ERECMAC 	                  */
/*			       EALGO	                  */
/*			       ESSC			  */
/*			       EMAC     		  */
/*			       ENOINTKEY	          */
/*			       ENOCONCKEY	          */
/*  SCTDEerr						  */
/*  aux_free2_OctetString				  */
/*  COMtrans		      error			  */
/*                             T1 - ERROR                 */
/*  malloc macro in MF_check.h				  */ 
/*  free   macro in MF_check.h				  */ 
/*							  */
/*--------------------------------------------------------*/
int
sct_interface(sct_id, command, request, response)
	int             sct_id;	/* sct_identifier   */
	unsigned int    command;/* instruction code */
	Request        *request;/* IN  - Puffer     */
	OctetString    *response;	/* Pointer of OUT - Puffer     */
{

	char           *s_apdu;
	unsigned int    lapdu;
	unsigned int    sw1;
	unsigned int    sw2;
	int             i;
	struct s_portparam *p_elem;
	Boolean            flag = FALSE;  /* FLAG, if S_STATUS must be send */
	char		file_name[30];
	char		pid[10];
	char		*proc="sct_interface";


	sct_errno = 0;
	response->octets = NULL;



#ifdef SCTTRACE
	if (!first) {
		strcpy(file_name,"SCT");
		sta_itoa(pid,getpid());
		strcat(file_name,pid);
		strcat(file_name,".TRC");

		sct_trfp = fopen(file_name, "wt");
		first = TRUE;
	};

#endif

/*****************************************************************************/
/*
 *       test sct_id  in sct_list 
 */

	if ((p_elem = MEMGetIdElem(sct_id)) == PORTNULL)
		return (-1);	/* ERROR: sct_id not in sct-list */

#ifdef ELEMTRACE
	fprintf(sct_trfp, "Element after MEMGetIdElem in sct_interface\n");
	MEMPrintElem(sct_trfp, p_elem);
#endif



/*****************************************************************************/
/*
 *      test request argument  
 */
	if (request == REQNULL) {
		sct_errno = EINVARG;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (S_ERR);

	};

/*****************************************************************************/
/*
 *      Create s_apdu  
 */
	if ((s_apdu = SCTDEcreate(p_elem, command, request, &lapdu, &flag)) == NULL)
		return (S_ERR);

	if (command != S_RSA_VERIFY) {
		/* 
		 * in case of S_RSA_VERIFY an apdulength > apdusize is possible
		 * this is the only command with chaining function
		 */
		if (lapdu > p_elem->apdusize) {	/* test apdusize */
			sct_errno = ETOOLONG;
			sct_errmsg = sca_errlist[sct_errno].msg;
			if (s_apdu)
				free(s_apdu);
			return (S_ERR);
		};
	}
	else {
		/* 
		 * in case of S_RSA_VERIFY set p_elem->chaining to C_ON
		 * after transmission the value C_OFF is set again
		 */
		p_elem->chaining = C_ON;
	}





/*****************************************************************************/
/*
 *      allocate response-buffer  
 */

	response->octets = malloc(p_elem->apdusize);

	if (response->octets == NULL) {
		sct_errno = EMEMAVAIL;
		sct_errmsg = sca_errlist[sct_errno].msg;
		if (s_apdu)
			free(s_apdu);
		return (S_ERR);
	};


	response->noctets = 0;
	for (i = 0; i < p_elem->apdusize; i++)
		*(response->octets + i) = 0x00;


/*****************************************************************************/
/*
 *      call transmission-procedure  
 */

	if (COMtrans(p_elem, s_apdu, lapdu, response->octets,
                     &response->noctets) == -1) {
		p_elem->chaining = C_OFF;
		if (s_apdu)
			free(s_apdu);
		response->noctets = 1;
		aux_free2_OctetString(response);
		return (SCTDEerr(0, tp1_err));
	}
/*****************************************************************************/
/*
 *      set p_elem->chaining to C_OFF and release s_apdu 
 */
	p_elem->chaining = C_OFF;
	if (s_apdu)
		free(s_apdu);



/*****************************************************************************/
/*
 *      analyse response  
 */

	if (SCTDEresponse(p_elem, command, response, &sw1, &sw2) == -1)
		return (S_ERR);


/*****************************************************************************/
/*
 *      if flag = TRUE, then send S_STATUS  
 */

	if (flag) {
		response->noctets = 1;
		aux_free2_OctetString(response);
		i = sct_status(command, p_elem, response);
		/*
		 * if i = -1 && command = S_REQUEST_ICC && sct_errno = ERESET
                 * set p_elem->icc_request = TRUE
		 */ 
		if ((i < 0) &&
		    (command == S_REQUEST_ICC) &&
		    (sct_errno == ERESET)) 
			p_elem->icc_request = TRUE;

		return (i);
	};

/*****************************************************************************/
/*
 *      if command = S_EJECT_ICC, then set
 *      p_elem->icc_request = FALSE
 */
	if (command == S_EJECT_ICC) 
		p_elem->icc_request = FALSE;

#ifdef ELEMTRACE
	MEMPrintElem(sct_trfp, p_elem);
	fprintf(sct_trfp, "END of sct_interface\n");
#endif

/*****************************************************************************/
/*
 *      no error => set sw2, if > 0  
 */
	if (sw2 == S_NOERR)
		return (sw2);

	if (sw1 == OKICC)
		sw2 += 3;

	return (sw2);

}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sct_interface	       */
/*-------------------------------------------------------------*/






/*--------------------------------------------------------*/
/*						    | GMD */
/*						    +-----*/
/* PROC  sct_perror	     VERSION   1.1		  */
/*				DATE   January  1993	  */
/*				  BY   L.Eckstein,GMD	  */
/*							  */
/* DESCRIPTION						  */
/*   Print error message				  */
/*   In case of sct_errno > 0, sct_perror first prints	  */
/*   msg, then a colon	and a blank, and then the error   */
/*   message to stderr. If msg is NULL or "", only the    */
/*   error message is printed.				  */
/*							  */
/*							  */
/*							  */
/*							  */
/*							  */
/* IN			     DESCRIPTION		  */
/*   msg		       additional message	  */
/*							  */
/* OUT							  */
/*							  */
/* RETURN		     DESCRIPTION		  */
/*   0			       o.k.			  */
/*							  */
/*							  */
/* CALLED FUNCTIONS					  */
/*							  */
/*							  */
/*--------------------------------------------------------*/
int
sct_perror(msg)
	char           *msg;	/* additional message	 */
{

	if (msg && strlen(msg))
		fprintf(stderr, "%s: ", msg);
	fprintf(stderr, "%s\n", sca_errlist[sct_errno].msg);
	return (S_NOERR);
}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sct_perror	       */
/*-------------------------------------------------------------*/




/*--------------------------------------------------------*/
/*						    | GMD */
/*						    +-----*/
/* PROC  sct_info	     VERSION   1.1		  */
/*				DATE   January  1993	  */
/*				  BY   L.Eckstein,GMD	  */
/*							  */
/* DESCRIPTION						  */
/*   Information about SCT / SC 			  */
/*							  */
/*							  */
/*							  */
/*							  */
/*							  */
/* IN			     DESCRIPTION		  */
/*   sct_id		      SCT-Identifier		  */
/*							  */
/*  sctinfo		      Pointer to structure	  */
/*							  */
/*							  */
/* OUT							  */
/*							  */
/*							  */
/* RETURN		     DESCRIPTION		  */
/*  0			      o.k			  */
/*							  */
/*  -1			      ERROR              	  */
/*                             ESCTLIST                   */
/*							  */
/*							  */
/* CALLED FUNCTIONS					  */
/*  MEMGetIdElem	       error			  */
/*                              ESIDUNK                   */
/*  MEMCreateList	       error			  */
/*				ENOSHELL		  */
/*                              EOPERR                    */
/*			        EEMPTY                    */
/*                              EMEMAVAIL                 */
/*                              ECLERR                    */
/*--------------------------------------------------------*/
int
sct_info(sct_id, sctinfo)
	int             sct_id;	/* SCT - Identifier */
	SCTInfo        *sctinfo;/* SCTInfo structure */
{
	struct s_portparam *p_elem;
	char		file_name[30];
	char		pid[10];

	sct_errno = 0;

#ifdef SCTTRACE
	if (!first) {
		strcpy(file_name,"SCT");
		sta_itoa(pid,getpid());
		strcat(file_name,pid);
		strcat(file_name,".TRC");

		sct_trfp = fopen(file_name, "wt");
		first = TRUE;
	};

#endif
	if (!resfirst) {	
		if (MEMCreateList() == -1)
			return (S_ERR);
		resfirst = TRUE;
	}


	sct_errno = 0;
	sctinfo->apdusize = 0;
	sctinfo->port_open = FALSE;
	sctinfo->icc_request = FALSE;
	sctinfo->integrity_key = FALSE;
	sctinfo->concealed_key = FALSE;

	if (!resfirst) {	/* sct-list not created  */
		sct_errno = ESCTLIST;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (S_ERR);
	};

/*****************************************************************************/
/*
 *      test sct_id  in sct_list  
 */
	if ((p_elem = MEMGetIdElem(sct_id)) == PORTNULL)
		return (S_ERR);	/* ERROR: sct_id not in sct-list */

#ifdef ELEMTRACE
	fprintf(sct_trfp, "Element after MEMGetIdElem in sct_info\n");
	MEMPrintElem(sct_trfp, p_elem);
#endif


/*****************************************************************************/
/*
 * 	set sctinfo - structure  
 */
	sctinfo->apdusize = p_elem->apdusize;
	if (p_elem->port_id > 0)
		sctinfo->port_open = TRUE;
	if (p_elem->icc_request > 0)
		sctinfo->icc_request = TRUE;
	if (p_elem->integrity_key.subjectkey.bits != NULL)
		sctinfo->integrity_key = TRUE;

	if (p_elem->concealed_key.subjectkey.bits != NULL)
		sctinfo->concealed_key = TRUE;


	sctinfo->secure_messaging.command  = p_elem->secure_messaging.command;
	sctinfo->secure_messaging.response = p_elem->secure_messaging.response;
	sctinfo->first_pad_byte = p_elem->first_pad_byte;
	sctinfo->next_pad_byte  = p_elem->next_pad_byte;
	sctinfo->always         = p_elem->always;

	return (S_NOERR);

}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sct_info	       */
/*-------------------------------------------------------------*/



/*--------------------------------------------------------*/
/*						    | GMD */
/*						    +-----*/
/* PROC  sct_secure	     VERSION   1.1		  */
/*				DATE   January  1993	  */
/*				  BY   L.Eckstein,GMD	  */
/*							  */
/* DESCRIPTION						  */
/*   Generate sessionkey and set it and the ssc in        */
/*   port-memory for secure messaging  between DTE and SCT*/
/*							  */
/*							  */
/*							  */
/*							  */
/*							  */
/* IN			     DESCRIPTION		  */
/*   sct_id		      SCT-Identifier		  */
/*							  */
/*							  */
/*   mode		      mode of the session key     */
/*							  */
/*   alg_id		      Algorithm identifier        */
/*			      In the current version, the */
/* 			      following values are        */
/*			      possible:                   */
/*			      desCBC_ISO0                 */
/*                            desCBC3_ISO0               1*/  
/* OUT							  */
/*							  */
/*							  */
/* RETURN		     DESCRIPTION		  */
/*  0			      o.k			  */
/*							  */
/*  -1			      ERROR             	  */
/*                             ESCTLIST                   */
/*                             EPARINC                    */
/*			       EGENSESS			  */
/*			       EMEMAVAIL		  */
/*			       EKEY			  */
/* 			       ERSAENC			  */
/*                             EALGO			  */
/*							  */
/*							  */
/* CALLED FUNCTIONS					  */
/*  MEMGetIdElem	      error			  */
/*                              ESIDUNK                   */
/*  sec_random_bstr					  */
/*  sct_interface	      error			  */
/*                              EINVARG			  */
/*                              ETOOLONG		  */
/*                              EMEMAVAIL		  */
/*                              ESIDUNK                   */
/*				EINS			  */
/*				EDESENC  		  */
/*				EMAC     		  */
/*				EALGO    		  */
/*				ENOINTKEY	          */
/*				ENOCONCKEY	          */
/*                              sw1/sw2 from SCT response */
/*				EDESDEC  		  */
/*				ESSC			  */
/*				ERECMAC 	          */
/*                              T1 - ERROR                */
/*  rsa_set_key						  */
/*  rsa_encrypt						  */
/*  aux_free_BitString					  */
/*  aux_free2_OctetString				  */
/*  aux_free2_BitString					  */
/*  aux_fxdump					          */
/*  aux_cmp_ObjId					  */ 
/*  malloc macro in MF_check.h				  */ 
/*--------------------------------------------------------*/
int
sct_secure(sct_id, mode, alg_id)
	int             sct_id;		/* SCT - Identifier */
	SCT_SecMessMode mode;		/* security mode    */
	AlgId		*alg_id;	/* encryption algorithm */
{
	struct s_portparam *p_elem;
	int             rc, i, ssc, key_pos, key_len;
	BitString      *sessionkey;	/* Structure will be allocated by
					 * sec_random_bstr */
					/* must be set free by aux_free_BitString */
	KeyBits         key_bits;
	OctetString     in;
	BitString       out;
	OctetString     enc_sess_key;
	int             keysize, memolen;

	AlgId          *subjectAI;
	KeyAlgId	sct_algid;
	Request         request;
	OctetString     response;
	char		*des_key;
	char		*proc="sct_secure";


	sct_errno = 0;


/*****************************************************************************/
/*
 *      test parameter mode  
 */
	if ((mode != SCT_INTEGRITY) &&
	    (mode != SCT_CONCEALED)) {
		sct_errno = EPARINC;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (S_ERR);
	};

/*****************************************************************************/
/*
 *	test parameter alg_id     
 */
	if ((!alg_id) || (!alg_id->objid)) {
		sct_errno = EALGO;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (-1);
	}

	if (!aux_cmp_ObjId(alg_id->objid,desCBC_ISO0->objid)) {
		/* algid = S_DES_CBC */ 
		sct_algid = S_DES_CBC;
		key_len = 8;
	}
	else {
 		if (!aux_cmp_ObjId(alg_id->objid,desCBC3_ISO0->objid)){
			/* algid = S_DES_3_CBC */
			sct_algid = S_DES_3_CBC;
			key_len = 16;
		}
		else {
			sct_errno = EALGO;
			sct_errmsg = sca_errlist[sct_errno].msg;
			return (-1);
		}
	};



/*****************************************************************************/
/*
 *      test sct_id in sct_list  
 */
	if (!resfirst) {	/* sct-list not created  */
		sct_errno = ESCTLIST;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (S_ERR);
	};

	if ((p_elem = MEMGetIdElem(sct_id)) == PORTNULL)
		return (S_ERR);	/* ERROR: sct_id not in sct-list */

#ifdef ELEMTRACE
	fprintf(sct_trfp, "Element after MEMGetIdElem in sct_secure\n");
	MEMPrintElem(sct_trfp, p_elem);
#endif



/*****************************************************************************/
/*
 *      generate DES-Sessionkey  
 */

	sessionkey = BITNULL;

	if ((sessionkey = sec_random_bstr(key_len * 8)) == BITNULL) {
		sct_errno = EGENSESS;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (-1);
	}

			
#ifdef SCTTRACE
	fprintf(sct_trfp, "sessionkey              : \n");
	fprintf(sct_trfp, "    nbits               : %d\n", sessionkey->nbits);
	fprintf(sct_trfp, "    bits                : \n");
	aux_fxdump(sct_trfp, sessionkey->bits, sessionkey->nbits / 8, 0);
#endif





/*****************************************************************************/
/*
 *      construct public key (modulus,Fermat-F4) 
 */

	key_bits.part1.noctets = transport_keylen;
	key_bits.part1.octets  = (char *) transport_key;
	key_bits.part2.noctets = fermat_f4_len;
	key_bits.part2.octets  = fermat_f4;
	key_bits.part3.noctets = 0;
	key_bits.part4.noctets = 0;

#ifdef SCTTRACE
	fprintf(sct_trfp, "Key_bits                : \n");
	fprintf(sct_trfp, "    part1.noctets       : %d\n", key_bits.part1.noctets);
	fprintf(sct_trfp, "    part1.octets        : \n");
	aux_fxdump(sct_trfp, key_bits.part1.octets, key_bits.part1.noctets, 0);
	fprintf(sct_trfp, "    part2.noctets       : %d\n", key_bits.part2.noctets);
	fprintf(sct_trfp, "    part2.octets        : \n");
	aux_fxdump(sct_trfp, key_bits.part2.octets, key_bits.part2.noctets, 0);
#endif


/*****************************************************************************/
/*
 *      set rsa public key in an internal function for hash-function 
 */

	rc = rsa_set_key(&key_bits, 0);
	if (rc < 0) {
		sct_errno = EKEY;
		sct_errmsg = sca_errlist[sct_errno].msg;
		if (sessionkey) {
			sessionkey->nbits = 1;
			aux_free_BitString(&sessionkey);
		}
		return (-1);
	}
/*****************************************************************************/
/*
 *      encrypt sessionkey with RSA-Publickey  
 *      allocate buffer for DES-Key              
 *      The Key must be set in the last 8 or 16 Bytes  
 *      The first Bytes are set to 0x00 
 */
 
        des_key = malloc(transport_keylen - 1);
        if ( des_key == NULL) {
		sct_errno = EMEMAVAIL;
		sct_errmsg = sca_errlist[sct_errno].msg;
		if (sessionkey) {
			sessionkey->nbits = 1;
			aux_free_BitString(&sessionkey);
		}
		return (-1);
        }

        for (i=0; i<transport_keylen - 1; i++)
           *(des_key+i) = 0x00;
        key_pos = transport_keylen - 1 - key_len;

        for (i=0; i<key_len; i++)
          *(des_key+key_pos+i) = *(sessionkey->bits +i);

	in.noctets = transport_keylen - 1;
	in.octets  = des_key;


	subjectAI = rsa;
	keysize = transport_keylen * 8;

#ifdef SCTTRACE
	fprintf(sct_trfp, "keysize                 : %d\n", keysize);
#endif

	out.nbits = 0;

	out.bits = malloc(transport_keylen);	/* will be set free in this proc. */

	if (out.bits == NULL) {
		sct_errno = EMEMAVAIL;
		sct_errmsg = sca_errlist[sct_errno].msg;
		if (sessionkey) {
			sessionkey->nbits = 1;
			aux_free_BitString(&sessionkey);
		}
		free(des_key);
		return (-1);
	}
	rc = rsa_encrypt(&in, &out, SEC_END, keysize);
	if (rc < 0) {
		sct_errno = ERSAENC;
		sct_errmsg = sca_errlist[sct_errno].msg;
		out.nbits = 1;
		aux_free2_BitString(&out);
		if (sessionkey) {
			sessionkey->nbits = 1;
			aux_free_BitString(&sessionkey);
		}
		free(des_key);
		return (-1);
	}

#ifdef SCTTRACE
	fprintf(sct_trfp, "encrypted sessionkey    : \n");
	fprintf(sct_trfp, "    nbits/8             : %d\n", out.nbits / 8);
	fprintf(sct_trfp, "    bits                : \n");
	aux_fxdump(sct_trfp, out.bits, out.nbits / 8, 0);
#endif


/*****************************************************************************/
/*
 *      Prepare parameters for the SCT Interface Cmd S_GEN_SESSION_KEY 
 */

	request.rq_p1.sec_mode = mode;
	request.rq_p2.algid    = sct_algid;
	enc_sess_key.noctets = out.nbits / 8;
	enc_sess_key.octets = out.bits;
	request.rq_datafield.session_key = &enc_sess_key;

/*****************************************************************************/
/*
 *      Call SCT Interface  
 */
	rc = sct_interface(sct_id, S_GEN_SESSION_KEY, &request, &response);
	if (rc < 0) {
		out.nbits = 1;
		aux_free2_BitString(&out);
		if (sessionkey) {
			sessionkey->nbits = 1;
			aux_free_BitString(&sessionkey);
		}
		response.noctets = 1;
		aux_free2_OctetString(&response);
		free(des_key);
		return (-1);
	}
	ssc = (*response.octets) & 0xFF;

#ifdef SCTTRACE
	fprintf(sct_trfp, "ssc                     : %x\n", ssc);
#endif

/*****************************************************************************/
/*
 *      Release storage  
 */
	response.noctets = 1;
	out.nbits = 1;
	aux_free2_OctetString(&response);
	aux_free2_BitString(&out);
	free(des_key);


/*****************************************************************************/
/*
 *      Store sessionkey and ssc in port-memory 
 */


	switch (mode) {
		case SCT_INTEGRITY:
			if (p_elem->integrity_key.subjectkey.bits != NULL) { 
			   free(p_elem->integrity_key.subjectkey.bits);
			   p_elem->integrity_key.subjectkey.bits = NULL;
			   p_elem->integrity_key.subjectkey.nbits = 0;
			}			
			p_elem->integrity = FALSE;

			/* allocate buffer for sessionkey;    */
			p_elem->integrity_key.subjectkey.bits = malloc(sessionkey->nbits / 8);
			if (p_elem->integrity_key.subjectkey.bits == NULL) {
				sct_errno = EMEMAVAIL;
				sct_errmsg = sca_errlist[sct_errno].msg;
				if (sessionkey) {
					sessionkey->nbits = 1;
					aux_free_BitString(&sessionkey);
				}
				return (-1);
			}
			for (i = 0; i < sessionkey->nbits / 8; i++)
				*(p_elem->integrity_key.subjectkey.bits + i) = *(sessionkey->bits + i);
			p_elem->integrity_key.subjectAI = alg_id;
			p_elem->integrity_key.subjectkey.nbits = sessionkey->nbits;
			p_elem->integrity = TRUE;
			break;

		case SCT_CONCEALED:
			if (p_elem->concealed_key.subjectkey.bits != NULL) {
			   free(p_elem->concealed_key.subjectkey.bits);
			   p_elem->concealed_key.subjectkey.bits = NULL;
			   p_elem->concealed_key.subjectkey.nbits = 0;
			}
			p_elem->concealed = FALSE;


			/* allocate buffer for sessionkey;    */
			p_elem->concealed_key.subjectkey.bits = malloc(sessionkey->nbits / 8);
			if (p_elem->concealed_key.subjectkey.bits == NULL) {
				sct_errno = EMEMAVAIL;
				sct_errmsg = sca_errlist[sct_errno].msg;
				if (sessionkey) {
					sessionkey->nbits = 1;
					aux_free_BitString(&sessionkey);
				}
				return (-1);
			}
			for (i = 0; i < sessionkey->nbits / 8; i++)
				*(p_elem->concealed_key.subjectkey.bits + i) = *(sessionkey->bits + i);
			p_elem->concealed_key.subjectAI = alg_id;
			p_elem->concealed_key.subjectkey.nbits = sessionkey->nbits;
			p_elem->concealed = TRUE;
			break;

	}

	if (sessionkey) {
		sessionkey->nbits = 1;
		aux_free_BitString(&sessionkey);
	}

	p_elem->ssc = ssc + 1;

#ifdef ELEMTRACE
	MEMPrintElem(sct_trfp, p_elem);
	fprintf(sct_trfp, "End of sct_secure\n");
#endif

	return (S_NOERR);

}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sct_secure	       */
/*-------------------------------------------------------------*/

/*--------------------------------------------------------*/
/*						    | GMD */
/*						    +-----*/
/* PROC  sct_init_secure     VERSION   1.1		  */
/*				DATE   August  1993	  */
/*				  BY   L.Eckstein,GMD	  */
/*							  */
/* DESCRIPTION						  */
/*   Delete Sessionkey in port-memory                     */
/*							  */
/*							  */
/*							  */
/*							  */
/*							  */
/* IN			     DESCRIPTION		  */
/*   sct_id		      SCT-Identifier		  */
/*							  */
/*							  */
/*   mode		      mode of the session key     */
/*							  */  
/* OUT							  */
/*							  */
/*							  */
/* RETURN		     DESCRIPTION		  */
/*  0			      o.k			  */
/*							  */
/*  -1			      ERROR             	  */
/*                             ESCTLIST                   */
/*                             EPARINC                    */
/*			       ENOINTKEY	          */
/*			       ENOCONCKEY		  */
/*							  */
/*							  */
/* CALLED FUNCTIONS					  */
/*  MEMGetIdElem	      error			  */
/*                              ESIDUNK                   */
/*  sct_interface	      error			  */
/*                              EINVARG			  */
/*                              ETOOLONG		  */
/*                              EMEMAVAIL		  */
/*                              ESIDUNK                   */
/*				EINS			  */
/*				EDESENC  		  */
/*				EMAC     		  */
/*				EALGO    		  */
/*				ENOINTKEY	          */
/*				ENOCONCKEY	          */
/*                              sw1/sw2 from SCT response */
/*				EDESDEC  		  */
/*				ESSC			  */
/*				ERECMAC 	          */
/*                              T1 - ERROR                */
/*  aux_free2_OctetString				  */
/*  free macro in MF_check.h				  */ 
/*--------------------------------------------------------*/
int
sct_init_secure(sct_id, mode)
	int             sct_id;		/* SCT - Identifier */
	SCT_SecMessMode mode;		/* security mode    */
{
	struct s_portparam *p_elem;
	int             rc;

	Request         request;
	OctetString     response;
	char		*proc="sct_init_secure";


	sct_errno = 0;


/*****************************************************************************/
/*
 *      test parameter mode  
 */
	if ((mode != SCT_INTEGRITY) &&
	    (mode != SCT_CONCEALED)) {
		sct_errno = EPARINC;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (S_ERR);
	};




/*****************************************************************************/
/*
 *      test sct_id in sct_list  
 */
	if (!resfirst) {	/* sct-list not created  */
		sct_errno = ESCTLIST;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (S_ERR);
	};

	if ((p_elem = MEMGetIdElem(sct_id)) == PORTNULL)
		return (S_ERR);	/* ERROR: sct_id not in sct-list */

#ifdef ELEMTRACE
	fprintf(sct_trfp, "Element after MEMGetIdElem in sct_init_secure\n");
	MEMPrintElem(sct_trfp, p_elem);
#endif

	/*
 	 *      test secure messaging key available
 	 */

	switch (mode) {
		case SCT_INTEGRITY:
			if (p_elem->integrity == FALSE) {
				sct_errno = ENOINTKEY;
				sct_errmsg= sca_errlist[sct_errno].msg;
				return (-1);
			}
			break;
		case SCT_CONCEALED:
			if (p_elem->concealed == FALSE) {
				sct_errno = ENOCONCKEY;
				sct_errmsg= sca_errlist[sct_errno].msg;
				return (-1);
			}
			break;
	}




/*****************************************************************************/
/*
 *      Prepare parameters for the SCT Interface Cmd S_DEL_SESSION_KEY 
 *      only if port = open
 */

	if (p_elem->port_id > 0) {


		request.rq_p1.sec_mode = mode;

		/*
 		 *      Call SCT Interface  
 		 */
		rc = sct_interface(sct_id, S_DEL_SESSION_KEY, &request, &response);
		if (rc < 0) {
			response.noctets = 1;
			aux_free2_OctetString(&response);
			return (-1);
		}



		/*
 		 *      Release storage  
 		 */
		response.noctets = 1;
		aux_free2_OctetString(&response);


	}

	switch (mode) {
	case SCT_INTEGRITY:
		p_elem->integrity = FALSE;
		p_elem->integrity_key.subjectAI = NULL;
		if (p_elem->integrity_key.subjectkey.bits)
			free(p_elem->integrity_key.subjectkey.bits);
		p_elem->integrity_key.subjectkey.bits = NULL;
		p_elem->integrity_key.subjectkey.nbits = 0;
		break;
	case SCT_CONCEALED:
		p_elem->concealed = FALSE;
		p_elem->concealed_key.subjectAI = NULL;
		if (p_elem->concealed_key.subjectkey.bits)
			free(p_elem->concealed_key.subjectkey.bits);
		p_elem->concealed_key.subjectkey.bits = NULL;
		p_elem->concealed_key.subjectkey.nbits = 0;
		break;
	}



#ifdef ELEMTRACE
	MEMPrintElem(sct_trfp, p_elem);
	fprintf(sct_trfp, "End of sct_init_secure\n");
#endif

	return (S_NOERR);

}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sct_init_secure	       */
/*-------------------------------------------------------------*/






/*--------------------------------------------------------*/
/*						    | GMD */
/*						    +-----*/
/* PROC  sct_setmode	     VERSION   1.1		  */
/*				DATE   January  1993	  */
/*				  BY   L.Eckstein,GMD	  */
/*							  */
/* DESCRIPTION						  */
/*   Set security mode for DTE-SCT in port memory 	  */
/*							  */
/*							  */
/*							  */
/*							  */
/*							  */
/* IN			     DESCRIPTION		  */
/*   sct_id		      SCT-Identifier		  */
/*							  */
/*   sec_mess                 security mode               */
/*							  */
/* OUT							  */
/*							  */
/* RETURN		     DESCRIPTION		  */
/*  0			      o.k			  */
/*							  */
/*  -1			      error             	  */
/*                             ESCTLIST                   */
/*							  */
/*							  */
/* CALLED FUNCTIONS					  */
/*  MEMGetIdElem	       error			  */
/*                              ESIDUNK                   */
/*--------------------------------------------------------*/
int
sct_setmode(sct_id, sec_mess)
	int             sct_id;	/* SCT - Identifier */
	SCT_SecMess    *sec_mess;
{


	struct s_portparam *p_elem;

	sct_errno = 0;


/*****************************************************************************/
/*
 *      test sct_id  in sct_list  
 */

	if (!resfirst) {	/* sct-list not created  */
		sct_errno = ESCTLIST;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (S_ERR);
	};
	if ((p_elem = MEMGetIdElem(sct_id)) == PORTNULL)
		return (S_ERR);	/* ERROR: sct_id not in sct-list */

#ifdef ELEMTRACE
	fprintf(sct_trfp, "Element after MEMGetIdElem in sct_setmode\n");
	MEMPrintElem(sct_trfp, p_elem);
#endif

	p_elem->secure_messaging.command = sec_mess->command;
	p_elem->secure_messaging.response = sec_mess->response;

#ifdef ELEMTRACE
	MEMPrintElem(sct_trfp, p_elem);
	fprintf(sct_trfp, "END of sct_setmode\n");
#endif

	return (S_NOERR);

}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sct_setmode	       */
/*-------------------------------------------------------------*/

/*--------------------------------------------------------*/
/*						    | GMD */
/*						    +-----*/
/* PROC  sct_checksecmess    VERSION   1.1		  */
/*				DATE   August  1993	  */
/*				  BY   L.Eckstein,GMD	  */
/*							  */
/* DESCRIPTION						  */
/*   Check secure messaging                     	  */
/*							  */
/*							  */
/*							  */
/*							  */
/*							  */
/* IN			     DESCRIPTION		  */
/*   sct_id		      SCT-Identifier		  */
/*							  */
/*							  */
/* OUT							  */
/*							  */
/* RETURN		     DESCRIPTION		  */
/*  0			      o.k			  */
/*							  */
/*  -1			      error             	  */
/*                             ESCTLIST                   */
/*			       ENOINTKEY		  */
/*			       ENOCONCKEY		  */	
/*							  */
/*							  */
/* CALLED FUNCTIONS					  */
/*  MEMGetIdElem	       error			  */
/*                              ESIDUNK                   */
/*--------------------------------------------------------*/
int
sct_checksecmess(sct_id)
	int             sct_id;	/* SCT - Identifier */
{


	struct s_portparam *p_elem;

	sct_errno = 0;


/*****************************************************************************/
/*
 *      test sct_id  in sct_list  
 */

	if (!resfirst) {	/* sct-list not created  */
		sct_errno = ESCTLIST;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (S_ERR);
	};
	if ((p_elem = MEMGetIdElem(sct_id)) == PORTNULL)
		return (S_ERR);	/* ERROR: sct_id not in sct-list */

#ifdef ELEMTRACE
	fprintf(sct_trfp, "Element after MEMGetIdElem in sct_checksecmess\n");
	MEMPrintElem(sct_trfp, p_elem);
#endif


	if (p_elem->secure_messaging.command != SCT_SEC_NORMAL) {

		/*
		 * test, if sessionkey available		
		 */
		switch (p_elem->secure_messaging.command) {
		case SCT_INTEGRITY:
			if (p_elem->integrity == FALSE) {
				sct_errno = ENOINTKEY;
				sct_errmsg= sca_errlist[sct_errno].msg;
				return (-1);
			}
			break;
		case SCT_CONCEALED:
			if (p_elem->concealed == FALSE) {
				sct_errno = ENOCONCKEY;
				sct_errmsg= sca_errlist[sct_errno].msg;
				return (-1);
			}
			break;
		case SCT_COMBINED:
			if (p_elem->integrity == FALSE) {
				sct_errno = ENOINTKEY;
				sct_errmsg= sca_errlist[sct_errno].msg;
				return (-1);
			}
			if (p_elem->concealed == FALSE) {
				sct_errno = ENOCONCKEY;
				sct_errmsg= sca_errlist[sct_errno].msg;
				return (-1);
			}
			break;
		}
	}

	if (p_elem->secure_messaging.response != SCT_SEC_NORMAL) {
		/*
		 * test, if sessionkey available		
		 */
		switch (p_elem->secure_messaging.response) {
		case SCT_INTEGRITY:
			if (p_elem->integrity == FALSE) {
				sct_errno = ENOINTKEY;
				sct_errmsg= sca_errlist[sct_errno].msg;
				return (-1);
			}
			break;
		case SCT_CONCEALED:
			if (p_elem->concealed == FALSE) {
				sct_errno = ENOCONCKEY;
				sct_errmsg= sca_errlist[sct_errno].msg;
				return (-1);
			}
			break;
		case SCT_COMBINED:
			if (p_elem->integrity == FALSE) {
				sct_errno = ENOINTKEY;
				sct_errmsg= sca_errlist[sct_errno].msg;
				return (-1);
			}
			if (p_elem->concealed == FALSE) {
				sct_errno = ENOCONCKEY;
				sct_errmsg= sca_errlist[sct_errno].msg;
				return (-1);
			}
			break;
		}
	}



	return (S_NOERR);

}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sct_checksecmess       */
/*-------------------------------------------------------------*/

/*--------------------------------------------------------*/
/*						    | GMD */
/*						    +-----*/
/* PROC  sct_getsecmess	     VERSION   1.1		  */
/*				DATE   December  1993	  */
/*				  BY   L.Eckstein,GMD	  */
/*							  */
/* DESCRIPTION						  */
/*   Get security mode for DTE-SCT out of port memory 	  */
/*							  */
/*							  */
/*							  */
/*							  */
/*							  */
/* IN			     DESCRIPTION		  */
/*   sct_id		      SCT-Identifier		  */
/*							  */
/*							  */
/* OUT							  */
/*   sec_mess                 security mode               */
/*							  */
/* RETURN		     DESCRIPTION		  */
/*  0			      o.k			  */
/*							  */
/*  -1			      error             	  */
/*                             ESCTLIST                   */
/*							  */
/*							  */
/* CALLED FUNCTIONS					  */
/*  MEMGetIdElem	       error			  */
/*                              ESIDUNK                   */
/*--------------------------------------------------------*/
int
sct_getsecmess(sct_id, sec_mess)
	int             sct_id;	/* SCT - Identifier */
	SCT_SecMess    *sec_mess;
{


	struct s_portparam *p_elem;

	sct_errno = 0;


/*****************************************************************************/
/*
 *      test sct_id  in sct_list  
 */

	if (!resfirst) {	/* sct-list not created  */
		sct_errno = ESCTLIST;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (S_ERR);
	};
	if ((p_elem = MEMGetIdElem(sct_id)) == PORTNULL)
		return (S_ERR);	/* ERROR: sct_id not in sct-list */

#ifdef ELEMTRACE
	fprintf(sct_trfp, "Element after MEMGetIdElem in sct_setmode\n");
	MEMPrintElem(sct_trfp, p_elem);
#endif

	sec_mess->command = p_elem->secure_messaging.command;
	sec_mess->response = p_elem->secure_messaging.response;


	return (S_NOERR);

}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sct_getsecmess	       */
/*-------------------------------------------------------------*/




/*--------------------------------------------------------*/
/*						    | GMD */
/*						    +-----*/
/* PROC  sct_list	     VERSION   1.1		  */
/*				DATE   January  1993	  */
/*				  BY   L.Eckstein,GMD	  */
/*							  */
/* DESCRIPTION						  */
/*   Information about installed sct's                    */
/*							  */
/*							  */
/*							  */
/*							  */
/* IN			     DESCRIPTION		  */
/*							  */
/*							  */
/* OUT							  */
/*							  */
/* RETURN		     DESCRIPTION		  */
/*  entryno		      Number of entries 	  */
/*  -1			      error			  */
/*							  */
/* CALLED FUNCTIONS					  */
/*  MEMCreateList	       error			  */
/*				ENOSHELL		  */
/*                              EOPERR                    */
/*			        EEMPTY                    */
/*                              EMEMAVAIL                 */
/*                              ECLERR                    */
/*--------------------------------------------------------*/
int
sct_list()
{
	struct s_portparam *dp_tail;
	int             lindex = 0;
	char		file_name[30];
	char		pid[10];

	sct_errno = 0;

#ifdef SCTTRACE
	if (!first) {
		strcpy(file_name,"SCT");
		sta_itoa(pid,getpid());
		strcat(file_name,pid);
		strcat(file_name,".TRC");

		sct_trfp = fopen(file_name, "wt");
		first = TRUE;
	};

#endif


/*****************************************************************************/
/*
 *      Create sct_list  
 */

	if (!resfirst) {	/* create sct-list */
		if (MEMCreateList() == -1)
			return (S_ERR);
		resfirst = TRUE;
	};
	dp_tail = p_lhead;

	while (dp_tail != PORTNULL) {
		dp_tail = dp_tail->p_next;
		lindex++;
	}


	return (lindex);

}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sct_list	       */
/*-------------------------------------------------------------*/


/*--------------------------------------------------------*/
/*						    | GMD */
/*						    +-----*/
/* PROC  sct_close	     VERSION   1.1		  */
/*				DATE   January  1993	  */
/*				  BY   L.Eckstein,GMD	  */
/*							  */
/* DESCRIPTION						  */
/*   Close port of SCT          			  */
/*							  */
/*							  */
/*							  */
/*							  */
/*							  */
/* IN			     DESCRIPTION		  */
/*   sct_id		      SCT-Identifier		  */
/*							  */
/*							  */
/* OUT							  */
/*							  */
/*							  */
/* RETURN		     DESCRIPTION		  */
/*  0			      o.k			  */
/*							  */
/*  -1			      error             	  */
/*							  */
/*							  */
/* CALLED FUNCTIONS					  */
/*  MEMGetIdElem	      error			  */
/*                              ESIDUNK                   */
/*  MEMGetOrgElem	      error			  */
/*			        ESIDUNK                   */
/*                              ENOSHELL                  */
/*                              EOPERR                    */
/*                              EEMPTY                    */
/*                              ERDERR                    */
/*                              ECLERR                    */
/*  COMclose
/*--------------------------------------------------------*/
int
sct_close(sct_id, proc)
	int             sct_id;	/* SCT - Identifier */
	char		*proc;
{
	struct s_portparam *p_elem;
	char		file_name[30];
	char		pid[10];

#ifdef SCTTRACE
	if (!first) {
		strcpy(file_name,"SCT");
		sta_itoa(pid,getpid());
		strcat(file_name,pid);
		strcat(file_name,".TRC");

		sct_trfp = fopen(file_name, "wt");
		first = TRUE;
	};

	fprintf(sct_trfp, "sct_close called from : %s\n", proc);
#endif

/*****************************************************************************/
/*
 *      test, if sct_id refers to a sct element  
 */

	if ((p_elem = MEMGetIdElem(sct_id)) == PORTNULL)
		return (S_ERR);	/* ERROR: sct not in sct-list */

#ifdef ELEMTRACE
	fprintf(sct_trfp, "Element after MEMGetIdElem in sct_close\n");
	MEMPrintElem(sct_trfp, p_elem);
#endif

/*****************************************************************************/
/*
 *      call COMclose - procedure  
 */
	if (p_elem->port_id > 0)
		COMclose(p_elem);





/*****************************************************************************/
/*
 *      get original element out of installation file 
 */

	if (MEMGetOrgElem(sct_id, p_elem) == -1)
		return (S_ERR);

#ifdef ELEMTRACE
	MEMPrintElem(sct_trfp, p_elem);
	fprintf(sct_trfp, "END of sct_close\n");
#endif


	return (0);
}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sct_close	       */
/*-------------------------------------------------------------*/


/*--------------------------------------------------------*/
/*						    | GMD */
/*						    +-----*/
/* PROC  sct_init_port	     VERSION   1.1		  */
/*				DATE   August  1993	  */
/*				  BY   L.Eckstein,GMD	  */
/*							  */
/* DESCRIPTION						  */
/*  Initialize port memory				  */
/*							  */
/*							  */
/*							  */
/*							  */
/*							  */
/* IN			     DESCRIPTION		  */
/*   sct_id		       SCT identifier		  */
/*							  */
/*   port_elem		       port element      	  */
/*							  */
/*   flag		       TRUE = COMinit		  */
/*			       FALSE= without COMinit     */
/*				      and set only        */
/*				      icc_request         */
/*							  */
/* OUT							  */
/*							  */
/*							  */
/*							  */
/* RETURN		     DESCRIPTION		  */
/*   0	         	       o.k			  */
/*  -1			       error			  */
/*                                                        */
/* CALLED FUNCTIONS					  */
/*  MEMCreateList	      error			  */
/*				ENOSHELL		  */
/*                              EOPERR                    */
/*			        EEMPTY                    */
/*                              EMEMAVAIL                 */
/*                              ECLERR                    */
/*  MEMGetIdElem              error			  */
/*                              ESIDUNK                   */
/*  MEMGetOrgElem	      error			  */
/*			        ESIDUNK                   */
/*                              ENOSHELL                  */
/*                              EOPERR                    */
/*                              EEMPTY                    */
/*                              ERDERR                    */
/*                              ECLERR                    */
/*  COMinit		      error			  */
/*                              T1 - ERROR                */
/*  COMclose		      error			  */
/*  							  */
/*--------------------------------------------------------*/
int
sct_init_port(sct_id, port_elem, flag, proc)
	int             sct_id;	/* SCT identifier 	 */
	SCTPort         *port_elem;
	Boolean		flag;
	char            *proc;
{


	struct s_portparam *p_elem;
	unsigned int    sw1 = 0;
	char		file_name[30];
	char		pid[10];

	sct_errno = 0;

#ifdef SCTTRACE
	if (!first) {
		strcpy(file_name,"SCT");
		sta_itoa(pid,getpid());
		strcat(file_name,pid);
		strcat(file_name,".TRC");

		sct_trfp = fopen(file_name, "wt");
		first = TRUE;
	};

#endif


	if (flag == TRUE) {
		/*
 		 *      Create sct_list or get original element out of
		 *      installation file
 		 */

		if (!resfirst) {	
			if (MEMCreateList() == -1)
				return (S_ERR);
			resfirst = TRUE;

			if ((p_elem = MEMGetIdElem(sct_id)) == PORTNULL)
				return (S_ERR);	/* ERROR: sct not in sct-list */
		}
		else {
			if ((p_elem = MEMGetIdElem(sct_id)) == PORTNULL)
				return (S_ERR);	/* ERROR: sct not in sct-list */
			if (p_elem->port_id > 0) 
				COMclose(p_elem);
			if (MEMGetOrgElem(sct_id, p_elem) == -1)
				return (S_ERR);
		}




#ifdef ELEMTRACE
		fprintf(sct_trfp, "Original Element in sct_init_port\n");
		MEMPrintElem(sct_trfp, p_elem);
#endif


		/*
 		 *      initialize p_elem with port_elem
 		 */
        	p_elem->bwt 		= port_elem->bwt;
		p_elem->cwt     	= port_elem->cwt;
		p_elem->baud		= port_elem->baud;
		p_elem->databits	= port_elem->databits;
		p_elem->stopbits	= port_elem->stopbits;
		p_elem->parity  	= port_elem->parity;
		p_elem->dataformat 	= port_elem->dataformat;
		p_elem->tpdusize 	= port_elem->tpdusize;
		p_elem->apdusize 	= port_elem->apdusize;
		p_elem->edc		= port_elem->edc;
		p_elem->protocoltype 	= port_elem->protocoltype;
		p_elem->chaining	= port_elem->chaining;
		p_elem->icc_request 	= port_elem->icc_request;
		p_elem->first_pad_byte	= port_elem->first_pad_byte;
		p_elem->next_pad_byte	= port_elem->next_pad_byte;
		p_elem->always		= port_elem->always;


		/*
 		 *      set sad, dad in sct_element and call COMinit
 		 */

		p_elem->sad = SAD;
		p_elem->dad = DAD;


		if ((COMinit(p_elem)) == -1) {
			return (SCTDEerr(sw1, tp1_err));
		}

	}
	else {

		/*
 		 *      get port element and set icc_request on new value
 		 */


		if ((p_elem = MEMGetIdElem(sct_id)) == PORTNULL)
			return (S_ERR);	/* ERROR: sct not in sct-list */

		p_elem->icc_request 	= port_elem->icc_request;
	}

#ifdef ELEMTRACE
	MEMPrintElem(sct_trfp, p_elem);
	fprintf(sct_trfp, "END of sct_init_port\n");
#endif



	return (S_NOERR);

}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sct_init_port	       */
/*-------------------------------------------------------------*/


/*--------------------------------------------------------*/
/*						    | GMD */
/*						    +-----*/
/* PROC  sct_get_port	     VERSION   1.1		  */
/*				DATE   August  1993	  */
/*				  BY   L.Eckstein,GMD	  */
/*							  */
/* DESCRIPTION						  */
/*  Get port memory				          */
/*  (same as sct_reset but without sending S_RESET)       */
/*							  */
/*							  */
/*							  */
/*							  */
/*							  */
/* IN			     DESCRIPTION		  */
/*   sct_id		       SCT identifier		  */
/*							  */
/*   port_elem		       port element      	  */
/* OUT							  */
/*							  */
/*							  */
/*							  */
/* RETURN		     DESCRIPTION		  */
/*   0	         	       o.k			  */
/*  -1			       error			  */
/*                                                        */
/* CALLED FUNCTIONS					  */
/*  MEMGetIdElem              error			  */
/*                              ESIDUNK                   */
/*--------------------------------------------------------*/
int
sct_get_port(sct_id, port_elem)
	int             sct_id;	/* SCT identifier 	 */
	SCTPort         *port_elem;
{


	struct s_portparam *p_elem;
	unsigned int    sw1 = 0;

	sct_errno = 0;



/*****************************************************************************/
/*
 *      get port element 
 */


	if ((p_elem = MEMGetIdElem(sct_id)) == PORTNULL)
		return (S_ERR);	/* ERROR: sct not in sct-list */


#ifdef ELEMTRACE
	MEMPrintElem(sct_trfp, p_elem);
	fprintf(sct_trfp, "END of sct_get_port\n");
#endif


/*****************************************************************************/
/*
 *      initialize port_elem with p_elem
 */
        port_elem->bwt 		= p_elem->bwt;
	port_elem->cwt     	= p_elem->cwt;
	port_elem->baud		= p_elem->baud;
	port_elem->databits	= p_elem->databits;
	port_elem->stopbits	= p_elem->stopbits;
	port_elem->parity  	= p_elem->parity;
	port_elem->dataformat 	= p_elem->dataformat;
	port_elem->tpdusize 	= p_elem->tpdusize;
	port_elem->apdusize 	= p_elem->apdusize;
	port_elem->edc		= p_elem->edc;
	port_elem->protocoltype = p_elem->protocoltype;
	port_elem->chaining	= p_elem->chaining;
	port_elem->icc_request 	= p_elem->icc_request;
	port_elem->first_pad_byte	= p_elem->first_pad_byte;
	port_elem->next_pad_byte	= p_elem->next_pad_byte;
	port_elem->always		= p_elem->always;

	return(0);






}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sct_get_port	       */
/*-------------------------------------------------------------*/

/*--------------------------------------------------------*/
/*						    | GMD */
/*						    +-----*/
/* PROC  sct_set_portpar	  VERSION   1.1		  */
/*				DATE   December  1993	  */
/*				  BY   L.Eckstein,GMD	  */
/*							  */
/* DESCRIPTION						  */
/*  Set port parameter				          */
/*  (in this version, only icc_request will be set        */
/*							  */
/*							  */
/*							  */
/*							  */
/*							  */
/* IN			     DESCRIPTION		  */
/*   sct_id		       SCT identifier		  */
/*							  */
/*   port_elem		       port element      	  */
/* OUT							  */
/*							  */
/*							  */
/*							  */
/* RETURN		     DESCRIPTION		  */
/*   0	         	       o.k			  */
/*  -1			       error			  */
/*                                                        */
/* CALLED FUNCTIONS					  */
/*  MEMGetIdElem              error			  */
/*                              ESIDUNK                   */
/*--------------------------------------------------------*/
int
sct_set_portpar(sct_id, icc_request)
	int             sct_id;	/* SCT identifier 	 */
	Boolean         icc_request;
{


	struct s_portparam *p_elem;
	unsigned int    sw1 = 0;

	sct_errno = 0;



/*****************************************************************************/
/*
 *      get port element 
 */


	if ((p_elem = MEMGetIdElem(sct_id)) == PORTNULL)
		return (S_ERR);	/* ERROR: sct not in sct-list */


#ifdef ELEMTRACE
	MEMPrintElem(sct_trfp, p_elem);
	fprintf(sct_trfp, "END of sct_set_portpar\n");
#endif


/*****************************************************************************/
/*
 *      initialize port_elem->icc_request
 */
	if (icc_request == FALSE)
		p_elem->icc_request 	= 0;
	else
		p_elem->icc_request 	= 1;

	return(0);






}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sct_set_portpar	       */
/*-------------------------------------------------------------*/


/*--------------------------------------------------------*/
/*						    | GMD */
/*						    +-----*/
/* PROC  sct_get_errmsg	     VERSION   1.1		  */
/*				DATE   January  1993	  */
/*				  BY   L.Eckstein,GMD	  */
/*							  */
/* DESCRIPTION						  */
/*   Get pointer of error message     			  */
/*							  */
/*							  */
/*							  */
/*							  */
/*							  */
/* IN			     DESCRIPTION		  */
/*   error_no		      Error number		  */
/*							  */
/*							  */
/* OUT							  */
/*							  */
/*							  */
/* RETURN		     DESCRIPTION		  */
/*							  */
/* CALLED FUNCTIONS					  */
/*--------------------------------------------------------*/
void
sct_get_errmsg(error_no)
	unsigned int    error_no;
{

	sct_errno = error_no;
	sct_errmsg = sca_errlist[error_no].msg;
}

/*-------------------------------------------------------------*/
/* E N D   O F	 P R O C E D U R E	sct_get_errmsg	       */
/*-------------------------------------------------------------*/

/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  sct_status           VERSION   1.1                */
/*                              DATE   January  1993      */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Send   S_STATUS-command, until SW1 / SW2 <> 0x40/0x41 */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*  lastcmd                   last command                */
/*                                                        */
/*  p_elem                    pointer of portparam struct.*/
/*                                                        */
/*                                                        */
/* OUT                                                    */
/*  response.octets            pointer of response.octets   */
/*                                                        */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   0                         o.k.                       */
/*                                                        */
/*   -1                        error                      */
/*			       EMEMAVAIL                  */
/*                             ETOOLONG                   */
/*                                                        */
/* CALLED FUNCTIONS                                       */
/*  SCTDEcreate		      error			  */
/*				EINS			  */
/*			        EMEMAVAIL                 */
/*				EDESENC  		  */
/*				EMAC     		  */
/*				EALGO    		  */
/*				ENOINTKEY	          */
/*				ENOCONCKEY	          */
/*  COMtrans		      error			  */
/*                             T1 - ERROR                 */
/*  SCTDEresponse	      error			  */
/*                              sw1/sw2 from SCT response */
/*				EMEMAVAIL		  */
/*				EDESDEC  		  */
/*				ESSC			  */
/*				EDESENC  		  */
/*				ERECMAC 	          */
/*				EALGO	                  */
/*				ENOCONCKEY		  */
/*				ENOINTKEY		  */
/*  SCTDEerr						  */
/*  aux_free2_OctetString                                 */
/*  malloc macro in MF_check.h				  */ 
/*  free   macro in MF_check.h				  */ 
/*                                                        */
/*                                                        */
/*                                                        */
/*--------------------------------------------------------*/
static int
sct_status(lastcmd, p_elem, resp)
	unsigned int    lastcmd;
	struct s_portparam *p_elem;
	OctetString     *resp;
{

	char           *s_apdu;
	unsigned int    lapdu;
	unsigned int    sw1;
	unsigned int    sw2;
	int             i;
	Boolean         flag = FALSE;	/* FLAG, if S_STATUS must be send */
	Request         request;
	char		*proc="sct_status";

#ifdef _DOS_
	long            time1;
	long            time2;

#else
#if defined(_MACC7_) || defined(_POSIX_)
   time_t time1, time2;
#else 
	struct itimerval value;
	struct itimerval ovalue;
#endif
#endif

/*****************************************************************************/
/*
 *      Create s_apdu   
 */

	if ((s_apdu = SCTDEcreate(p_elem, S_STATUS, &request, &lapdu, &flag)) == NULL)
		return (S_ERR);

	if (lapdu > p_elem->apdusize) {	/* test apdusize */
		sct_errno = ETOOLONG;
		sct_errmsg = sca_errlist[sct_errno].msg;
		if (s_apdu)
			free(s_apdu);
		return (S_ERR);
	};




/*****************************************************************************/
/*
 *      allocate response-buffer  
 */

	resp->octets = malloc(p_elem->apdusize);

	if (resp->octets == NULL) {
		sct_errno = EMEMAVAIL;
		sct_errmsg = sca_errlist[sct_errno].msg;
		if (s_apdu)
			free(s_apdu);
		return (S_ERR);
	};

/*****************************************************************************/
/*
 *      wait 1 sec, till next S_STATUS will be send 
 *	repeat, until SW1 / SW2 <> SCT waiting
 */
	do {

#ifdef _DOS_
		time(&time1);
		do {
			time(&time2);
		} while ((time2 - time1) < 4);
#else
#if defined(_MACC7_) || defined(_POSIX_)
      time(&time1);
      do
         time(&time2);
      while ((time2 - time1) < 1);
#else

      signal(SIGALRM,time_int);
      getitimer(ITIMER_REAL,&value);
      value.it_value.tv_sec = 1;    
      setitimer(ITIMER_REAL,&value,&ovalue);
      pause();
#endif 
#endif

		resp->noctets = 0;
		for (i = 0; i < p_elem->apdusize; i++)
			*(resp->octets + i) = 0x00;


		/*
		 * call transmission-procedure        
		 */

		if (COMtrans(p_elem, s_apdu, lapdu, resp->octets, &resp->noctets) == -1) {
			if (s_apdu)
				free(s_apdu);
			resp->noctets = 1;
			aux_free2_OctetString(resp);
			return (SCTDEerr(0, tp1_err));
		}

		/*
		 * analyse response                   
		 */

		if (SCTDEresponse(p_elem, S_STATUS, resp, &sw1, &sw2) == -1) {
			resp->noctets = 1;
			aux_free2_OctetString(resp);
			if (s_apdu)
				free(s_apdu);
			return (S_ERR);
		}

		/*
		 * Create s_apdu,if command=concealed 
		 */
		if (sw1 == OKSCT && sw2 == SCTWAIT) {
			if (p_elem->secure_messaging.command != SCT_SEC_NORMAL) {
				if (s_apdu)
					free(s_apdu);
				if ((s_apdu = SCTDEcreate(p_elem, S_STATUS, &request, &lapdu, &flag)) == NULL) {
					resp->noctets = 1;
					aux_free2_OctetString(resp);
					return (S_ERR);
				}
			} else {

#ifdef SCTTRACE
				sca_aux_sct_apdu(sct_trfp, s_apdu, lapdu);
#endif
			}


			/*
			 * allocate new response-buffer, if response =
			 * CONCEALED
			 */

			if (p_elem->secure_messaging.response != SCT_SEC_NORMAL) {

				resp->noctets = 1;
				aux_free2_OctetString(resp);

				resp->octets = malloc(p_elem->apdusize);

				if (resp->octets == NULL) {
					sct_errno = EMEMAVAIL;
					sct_errmsg = sca_errlist[sct_errno].msg;
					if (s_apdu)
						free(s_apdu);
					return (S_ERR);
				};
			}
		}
	} while (sw1 == OKSCT && sw2 == SCTWAIT);

/*****************************************************************************/
/*
 *      S-STATUS ended;
 *	if lastcmd = S_REQUEST_ICC, then set in p_elem 
 *      icc_request = TRUE
 *      and out of the ATR 
 *	first_pad_byte 
 *	next_pad_byte
 *	always
 */

	if (lastcmd == S_REQUEST_ICC) { 
		p_elem->icc_request = TRUE;
		p_elem->first_pad_byte = *(resp->octets + 10);
		p_elem->next_pad_byte  = *(resp->octets + 11);
		p_elem->always         = (int) *(resp->octets + 12);
	}

	
/*****************************************************************************/
/*
 *      release s_apdu   
 */
	if (s_apdu)
		free(s_apdu);
	return (S_NOERR);
}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      sct_status              */
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
/* E N D   O F	 P A C K A G E	     sct_if		       */
/*-------------------------------------------------------------*/
