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
/*                                         DATE July 1993      */
/*                                           BY Levona Eckstein*/
/*                                                             */
/*    FILENAME                                                 */
/*      sctmem.c                                               */
/*                                                             */
/*    DESCRIPTION                                              */
/*      SCT - Memory - Module                                  */
/*                                                             */
/*    EXPORT                DESCRIPTION                        */
/*      MEMCreateList()       create SCT-list out of install-  */
/*                            File                             */
/*      MEMGetIdElem()        get SCT-Element                  */
/*                            search-string = SCT-identifier   */
/*      MEMGetOrgElem()       get original SCT-Element out     */
/*                            of install-file                  */
/*      MEMPrintElem()        print sct-element if TRACE    */
/*                                                             */
/*                                                             */
/*                                                             */
/*    INTERNAL              DESCRIPTION                        */
/*     del_sctlist               delete sct-list               */
/*     init_elem                 initialize sct element        */
/*     pr_sctlist                print sct-list if TRACE       */
/*                                                             */
/*    IMPORT                DESCRIPTION                        */
/*     sct_errno                 error variable from sct_if.c  */
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
#include "install.h"

#ifdef _MACC7_
#include "Mac.h"
#include "baud.h"
#endif /* MACC7 */



/*-------------------------------------------------------------*/
/*   extern declarations                                       */
/*-------------------------------------------------------------*/
extern unsigned int sct_errno;	/* error variable               */
extern char    *sct_errmsg;

#ifdef MEMTRACE
extern FILE    *sct_trfp;	/* Filepointer of trace file    */

#endif


/*-------------------------------------------------------------*/
/*   forward declarations                                      */
/*-------------------------------------------------------------*/
static void     del_sctlist();
static void     init_elem();
static void     pr_sctlist();
void            MEMPrintElem();

/*-------------------------------------------------------------*/
/*   type definitions                                          */
/*-------------------------------------------------------------*/
#ifdef _BSD42_
/*#define B19200 14 */
#include <sys/ttydev.h>

#elif _POSIX_
#include <termio.h>

#endif



/*-------------------------------------------------------------*/
/*   globale variable definitions                              */
/*-------------------------------------------------------------*/
struct s_portparam *p_lhead;



/*-------------------------------------------------------------*/
/*   lokale Variable definitions                               */
/*-------------------------------------------------------------*/
static struct s_portparam *p_llast;
static struct s_portparam *p_elem;



/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  MEMCreateList       VERSION   1.1                */
/*                              DATE   June 1993          */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Create SCT-List                                       */
/*  In the environment must exists the shell-variable     */
/*  "STAMOD".                                             */
/*  This shell-variable contains the name of the          */
/*  installation - File                                   */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*                                                        */
/*                                                        */
/* OUT                                                    */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   0                         o.k.                       */
/*  -1                         error                      */
/*				ENOSHELL		  */
/*                              EOPERR                    */
/*			        EEMPTY                    */
/*                              EMEMAVAIL                 */
/*                              ECLERR                    */
/* CALLED FUNCTIONS					  */
/*   del_sctlist				          */
/*   malloc macro in MF_check.h				  */ 
/*--------------------------------------------------------*/
int 
MEMCreateList()
{
	char           *fileptr;
	FILE           *fd;
	struct s_record genrecord;
	int             lindex = 0;
	int             i;
	char		*proc="MEMCreateList";

/*****************************************************************************/
/*
 *      Open file for read  
 */
	/* read shell - variable               */
#ifdef _MACC7_
    if ((fileptr = MacGetEnv("STAMOD")) == NULL) {
#else
    if ((fileptr = getenv("STAMOD")) == NULL) {
#endif /* !MACC7 */
		sct_errno = ENOSHELL;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (S_ERR);
	};

	/* open File for read */

	if ((fd = fopen(fileptr, "r")) == NULL) {
		sct_errno = EOPERR;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (S_ERR);
	};

/*****************************************************************************/
/*
 *      create SCT-List  
 */
	if (fread(&genrecord, sizeof(struct s_record), 1, fd) == 0) {
		sct_errno = EEMPTY;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (S_ERR);
	};

	do {
		if (lindex == 0) {	/* create first element */
			p_lhead = (struct s_portparam *) malloc(sizeof(struct s_portparam));
			p_llast = p_lhead;
		} else {	/* next element         */
			p_llast->p_next = (struct s_portparam *) malloc(sizeof(struct s_portparam));
			p_llast = p_llast->p_next;
		};


		if (p_llast != PORTNULL) {
			memcpy(p_llast->port_name, genrecord.port_name, LPORTNAME);
			p_llast->port_name[LPORTNAME] = '\0';
			for (i = 0; i < LPORTNAME; i++) {
				if (p_llast->port_name[i] == 0x20) {
					p_llast->port_name[i] = '\0';
					break;
				};
			};


			/*
			 * Initialize element					 
			 */
			init_elem(p_llast, &genrecord, TRUE);
			p_llast->p_next = PORTNULL;


			/*
			 * create next element					 
			 */
			lindex++;
		} else {
			sct_errno = EMEMAVAIL;
			sct_errmsg = sca_errlist[sct_errno].msg;
			/* delete sct-list */
			fclose(fd);
			del_sctlist();
			return (S_ERR);
		};

	} while (fread(&genrecord, sizeof(struct s_record), 1, fd) != 0);

/*****************************************************************************/
/*
 *      close Installation File  
 */
	if (fclose(fd) != 0) {
		sct_errno = ECLERR;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (S_ERR);
	};

#ifdef MEMTRACE
	/* print list */
	fprintf(sct_trfp, "FUNCTION MEMCreateList: \n");
	pr_sctlist(sct_trfp);
#endif

	return (0);

}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      MEMCreateList          */
/*-------------------------------------------------------------*/






/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  MEMGetIdElem        VERSION   1.1                */
/*                              DATE   June 1993          */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Get sct-element with search-string = sct_id           */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*  fd                        filedescriptor              */
/*                                                        */
/* OUT                                                    */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*  Pointer                   Pointer of sct-element      */
/*  POINTNULL                 error                       */
/*                             ESIDUNK                    */
/*                                                        */
/* CALLED FUNCTIONS					  */
/*--------------------------------------------------------*/
struct s_portparam *
MEMGetIdElem(sct_id)
	int             sct_id;
{
	int             i;

	p_elem = PORTNULL;

	if (sct_id == 0) {
		sct_errno = ESIDUNK;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (p_elem);
	};

	p_elem = p_lhead;
	if (p_elem == PORTNULL) {
		sct_errno = ESIDUNK;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (p_elem);
	};

	for (i = 0; i < sct_id - 1; i++) {
		p_elem = p_elem->p_next;


		if (p_elem == PORTNULL) {
			sct_errno = ESIDUNK;
			sct_errmsg = sca_errlist[sct_errno].msg;
			return (p_elem);
		};
	};

#ifdef MEMTRACE
	fprintf(sct_trfp, "FUNCTION MEMGetIdElem: \n");
	MEMPrintElem(sct_trfp, p_elem);
#endif
	return (p_elem);
}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      MEMGetIdElem           */
/*-------------------------------------------------------------*/



/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  MEMGetOrgElem()     VERSION   1.1                */
/*                              DATE   June 1993          */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Read original SCT-Element out of Install-File.        */
/*  In the environment must exists the shell-variable     */
/*  "STAMOD".                                             */
/*  This shell-variable contains the name of the          */
/*  installation - File                                   */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*  Pointer                   Pointer of old listelement  */
/*                                                        */
/*                                                        */
/* OUT                                                    */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*   0                         o.k.                       */
/*  -1                         error                      */
/*			        ESIDUNK                   */
/*                              ENOSHELL                  */
/*                              EOPERR                    */
/*                              EEMPTY                    */
/*                              ERDERR                    */
/*                              ECLERR                    */
/*                                                        */
/*                                                        */
/* CALLED FUNCTIONS					  */
/* init_elem					          */
/* free macro in MF_check.h				  */ 
/*--------------------------------------------------------*/
int 
MEMGetOrgElem(sct_id, oldelem)
	int             sct_id;
	struct s_portparam *oldelem;
{
	char           *fileptr;
	FILE           *fd;
	struct s_record genrecord;
	int             rc;
	long            offset;
	char		*proc="MEMGetOrgElem";


	if (sct_id == 0) {
		sct_errno = ESIDUNK;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (S_ERR);
	};
/*****************************************************************************/
/*
 *      Open file for read 
 */
	/* read shell - variable               */
	if ((fileptr = getenv("STAMOD")) == NULL) {
		sct_errno = ENOSHELL;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (S_ERR);
	};

	/* open File for read */

	if ((fd = fopen(fileptr, "r")) == NULL) {
		sct_errno = EOPERR;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (S_ERR);
	};

/*****************************************************************************/
/*
 *      read install-file, until element found  
 */

	offset = (long) (sizeof(struct s_record) * (sct_id - 1));
	if (fseek(fd, offset, 0) != 0) {
		sct_errno = EEMPTY;
		sct_errmsg = sca_errlist[sct_errno].msg;
		fclose(fd);
		return (S_ERR);
	};


	if (fread(&genrecord, sizeof(struct s_record), 1, fd) == 0) {
		sct_errno = ERDERR;
		sct_errmsg = sca_errlist[sct_errno].msg;
		fclose(fd);
		return (S_ERR);
	};


/*****************************************************************************/
/*
 *      Initialize element  
 */
	init_elem(oldelem, &genrecord, FALSE);

/*****************************************************************************/
/*
 *      close Installation File  
 */
	if (fclose(fd) != 0) {
		sct_errno = ECLERR;
		sct_errmsg = sca_errlist[sct_errno].msg;
		return (-1);
	};

#ifdef MEMTRACE
	/* print element */
	fprintf(sct_trfp, "FUNCTION MEMGetOrgElem: \n");
	MEMPrintElem(sct_trfp, oldelem);
#endif

	return (0);

}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      MEMGetOrgElem          */
/*-------------------------------------------------------------*/



/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  del_sctlist         VERSION   1.1                */
/*                              DATE   June 1993          */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Delete sct-List                                       */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*                                                        */
/*                                                        */
/* OUT                                                    */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*                                                        */
/* CALLED FUNCTION				          */
/* free macro in MF_check.h				  */ 
/*--------------------------------------------------------*/
static void 
del_sctlist()
{
	struct s_portparam *dp_tail;
	char		   *proc="del_sctlist"; 

	dp_tail = p_lhead;

	while (dp_tail != PORTNULL) {
		p_lhead = p_lhead->p_next;
		if (dp_tail)
			free(dp_tail);
		dp_tail = p_lhead;
	}

	p_lhead = PORTNULL;

}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      del_sctlist            */
/*-------------------------------------------------------------*/



/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  init_elem           VERSION   1.1                */
/*                              DATE   June 1993          */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Initialize sct element                                */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*  Pointer                   Pointer of listelement      */
/*                                                        */
/*  Pointer                   Pointer of genrecord        */
/*                                                        */
/*  first                     call  of init_elem          */
/*			      TRUE : call of MEMCreateList*/
/*			      FALSE: call of MEMGetOrgElem*/
/*                                                        */
/*                                                        */
/* OUT                                                    */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*--------------------------------------------------------*/
static void 
init_elem(sct_elem, genrecord, first)
	struct s_portparam *sct_elem;
	struct s_record *genrecord;
	Boolean         first;
{
	int             div = 19200;
	int             baud;
	int             index;


	sct_elem->bwt = genrecord->bwt;
	sct_elem->cwt = genrecord->cwt;
#ifdef _DOS_
	sct_elem->baud = (div / genrecord->baud) * 6;
#else
#ifdef _BSD42_
	index = 0;
	baud = genrecord->baud;
	while ((div / baud) != 1) {
		baud = baud * 2;
		index++;
	}
	sct_elem->baud = B19200 - index;
#else
#ifdef _MACC7_
      	sct_elem->baud = MacBaud(p_elem->baud);
      
#endif /* _MACC7_ */
#endif
#endif


	if (genrecord->databits == 7)
		sct_elem->databits = DATA_7;
	else
		sct_elem->databits = DATA_8;
	if (genrecord->stopbits == 1)
		sct_elem->stopbits = STOP_1;
	else
		sct_elem->stopbits = STOP_2;
	if (sct_elem->databits == DATA_8)
		sct_elem->parity = PARNONE;
	else
		sct_elem->parity = genrecord->parity;
	sct_elem->dataformat = genrecord->dataformat;
	sct_elem->tpdusize = genrecord->tpdu_size;
	sct_elem->apdusize = genrecord->apdu_size;
	sct_elem->edc = genrecord->edc;
	sct_elem->chaining = C_ON;
	sct_elem->port_id = 0;
	sct_elem->ssc = 0;
	sct_elem->icc_request = 0;
	if (first == TRUE) {
		sct_elem->secure_messaging.response = SCT_SEC_NORMAL;
		sct_elem->secure_messaging.command = SCT_SEC_NORMAL;
		sct_elem->integrity = FALSE;
		sct_elem->concealed = FALSE;
		sct_elem->integrity_key.subjectkey.bits = NULL;
		sct_elem->integrity_key.subjectkey.nbits = 0;
		sct_elem->concealed_key.subjectkey.bits = NULL;
		sct_elem->concealed_key.subjectkey.nbits = 0;
	};
	sct_elem->integrity_key.subjectAI = NULL;
	sct_elem->concealed_key.subjectAI = NULL;

	if (sct_elem->integrity_key.subjectkey.bits)
		free(sct_elem->integrity_key.subjectkey.bits);
	sct_elem->integrity_key.subjectkey.bits = NULL;
	sct_elem->integrity_key.subjectkey.nbits = 0;

	if (sct_elem->concealed_key.subjectkey.bits)
		free(sct_elem->concealed_key.subjectkey.bits);
	sct_elem->concealed_key.subjectkey.bits = NULL;
	sct_elem->concealed_key.subjectkey.nbits = 0;
	sct_elem->first_pad_byte = 0x00;
	sct_elem->next_pad_byte  = 0x00;
	sct_elem->always         = FALSE;

	
}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E      init_elem              */
/*-------------------------------------------------------------*/



/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC   pr_sctlist         VERSION   1.1                */
/*                              DATE   June 1993          */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Print  sct-List                                       */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*                                                        */
/*                                                        */
/* OUT                                                    */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*--------------------------------------------------------*/
static void 
pr_sctlist(dump_file)
	FILE           *dump_file;
{

	struct s_portparam *dp_tail;
	int             lindex = 1;

	dp_tail = p_lhead;

	while (dp_tail != PORTNULL) {

		fprintf(dump_file, "\n%d. Listenelement \n", lindex);
		MEMPrintElem(dump_file, dp_tail);

		dp_tail = dp_tail->p_next;
		lindex++;
	}



}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E       pr_sctlist            */
/*-------------------------------------------------------------*/



/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC   MEMPrintElem       VERSION   1.1                */
/*                              DATE   June 1993          */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Print  one listelement                                */
/*                                                        */
/*                                                        */
/* IN                        DESCRIPTION                  */
/*  Pointer                    Pointer of listelement     */
/*                                                        */
/* OUT                                                    */
/*                                                        */
/*                                                        */
/*                                                        */
/*                                                        */
/* RETURN                    DESCRIPTION                  */
/*                                                        */
/* CALLED FUNCTIONS					  */
/*   aux_ObjId2Name    				          */ 
/*--------------------------------------------------------*/
void 
MEMPrintElem(dump_file, elem)
	FILE           *dump_file;
	struct s_portparam *elem;
{
	char	       *aux_ptr;


	if (elem != PORTNULL) {
		fprintf(dump_file, "\nListenelement \n");
		fprintf(dump_file, "  PORT_NAME    = %s\n", elem->port_name);
		fprintf(dump_file, "  BWT          = %d\n", elem->bwt);
		fprintf(dump_file, "  CWT          = %d\n", elem->cwt);
		fprintf(dump_file, "  BAUD         = %d\n", elem->baud);
		fprintf(dump_file, "  DATABITS     = %d\n", elem->databits);
		fprintf(dump_file, "  STOPBITS     = %d\n", elem->stopbits);
		fprintf(dump_file, "  PARITY       = %d\n", elem->parity);
		fprintf(dump_file, "  DATAFORMAT   = %d\n", elem->dataformat);
		fprintf(dump_file, "  TPDUSIZE     = %d\n", elem->tpdusize);
		fprintf(dump_file, "  APDUSIZE     = %d\n", elem->apdusize);
		fprintf(dump_file, "  EDC          = %d\n", elem->edc);
		fprintf(dump_file, "  PROTTYPE     = %d\n", elem->protocoltype);
		fprintf(dump_file, "  CHAINING     = %d\n", elem->chaining);
		fprintf(dump_file, "  NS           = %d\n", elem->ns);
		fprintf(dump_file, "  RSV          = %d\n", elem->rsv);
		fprintf(dump_file, "  SAD          = %d\n", elem->sad);
		fprintf(dump_file, "  DAD          = %d\n", elem->dad);

		fprintf(dump_file, "  PORT_ID       = %d\n", elem->port_id);
		fprintf(dump_file, "  FIRST         = %d\n", elem->first);
		fprintf(dump_file, "  INTEGRITY     = %d\n", elem->integrity);
		fprintf(dump_file, "  CONCEALED     = %d\n", elem->concealed);
		fprintf(dump_file, "  INTEGRITY_KEY = \n");
		if (elem->integrity_key.subjectAI != NULL) {
			aux_ptr = aux_ObjId2Name(elem->integrity_key.subjectAI->objid);
			fprintf(dump_file, "     subjectAI         = %s\n",
			        aux_ptr);
			free(aux_ptr);
		}


		if (elem->integrity_key.subjectkey.bits != NULL) {
			fprintf(dump_file, "     subjectkey.nbits  = %d\n", elem->integrity_key.subjectkey.nbits);
			fprintf(dump_file, "     subjectkey.bits   = \n");
			aux_fxdump(dump_file, elem->integrity_key.subjectkey.bits,
				 elem->integrity_key.subjectkey.nbits / 8, 0);
		} else {
			fprintf(dump_file, "     subjectkey.bits   = NULL\n");
		}

		fprintf(dump_file, "  CONCEALED_KEY = \n");
		if (elem->concealed_key.subjectAI != NULL) {
			aux_ptr = aux_ObjId2Name(elem->concealed_key.subjectAI->objid);
			fprintf(dump_file, "     subjectAI         = %s\n",
			        aux_ptr);
			free(aux_ptr);
		}
		if (elem->concealed_key.subjectkey.bits != NULL) {
			fprintf(dump_file, "     subjectkey.nbits  = %d\n", elem->concealed_key.subjectkey.nbits);
			fprintf(dump_file, "     subjectkey.bits   = \n");
			aux_fxdump(dump_file, elem->concealed_key.subjectkey.bits,
				 elem->concealed_key.subjectkey.nbits / 8, 0);
		} else {
			fprintf(dump_file, "     subjectkey.bits   = NULL\n");
		}

		fprintf(dump_file, "  SSC             = %x\n", elem->ssc);
		fprintf(dump_file, "  SECMESS_COMMAND = %d\n", elem->secure_messaging.command);
		fprintf(dump_file, "  SECMESS_RESPONSE= %d\n", elem->secure_messaging.response);
		fprintf(dump_file, "  ICC_REQUEST      = %d\n\n", elem->icc_request);
		fprintf(dump_file, "  FIRST_PAD_BYTE   = %x\n\n", elem->first_pad_byte);
		fprintf(dump_file, "  NEXT_PAD_BYTE    = %x\n\n", elem->next_pad_byte);
		fprintf(dump_file, "  ALWAYS           = %x\n\n", elem->always);
	}
}

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E       MEMPrintElem          */
/*-------------------------------------------------------------*/

/*-------------------------------------------------------------*/
/* E N D   O F   P R O C E D U R E       s c t m e m           */
/*-------------------------------------------------------------*/
