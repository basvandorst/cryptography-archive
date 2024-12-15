
/*-------------------------------------------------------+-----*/
/*                                                       | GMD */
/*   SYSTEM   STAMOD  -  Version 1.0                     +-----*/
/*                                                             */
/*-------------------------------------------------------------*/
/*                                                             */
/*    PACKAGE   TRPROT                  VERSION 1.00           */
/*                                         DATE                */
/*                                           BY Zhou           */
/*                                                             */
/*    FILENAME                                                 */
/*      t1test.c                                               */
/*                                                             */
/*    DESCRIPTION                                              */
/*      Transmission Protocol Interface Test Program           */
/*                                                             */
/*    EXPORT                DESCRIPTION                        */
/*      COMinit()             initiate serial port             */
/*                                                             */
/*      COMtrans()            communication                    */
/*                                                             */
/*      err                   error number                     */
/*                                                             */
/*                                                             */
/*                                                             */
/*    USES                  DESCRIPTION                        */
/*                                                             */
/*                                                             */
/*                                                             */
/*    INTERNAL              DESCRIPTION                        */
/*      prot_init             reset protocol parameters        */
/*      Resynch()             resynchronasation of the protocol*/
/*                                                             */
/*                                                             */
/* Notice:                                                     */
/* 1) For exchange of transmission control information,        */
/*    it supports only resynchronisation.   S(RESYNCH, ...)    */
/* 2) For Error Detecte Code, LRC is used currently.           */
/*    Late, CRC will be used.                                  */
/* 3) Currently, only serial port COM1 is used.                */
/*-------------------------------------------------------------*/

/*-------------------------------------------------------------*/
/*   include-Files                                             */
/*-------------------------------------------------------------*/
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <termio.h> 
#include <sys/ttold.h> 		/* defines the TIOCEXCL  ioctl call */
#include <sys/time.h>



#include "t1if.h"
#include "sca.h"
#include "stamod.h"



/*-------------------------------------------------------------*/
/*   globale variable definitions                              */
/*-------------------------------------------------------------*/
unsigned int    tp1_err = 0;	/* to include error code       */



/*-------------------------------------------------------------*/
/*   forward declarations                                      */
/*-------------------------------------------------------------*/
static void     prot_init();

/*-------------------------------------------------------------*/
/*   Preprocessor - Definitionen                               */
/*-------------------------------------------------------------*/

#define T1      0x81		/* Protocoltype = t1 */



/*-------------------------------------------------------+-----*/
/*                                                       | GMD */
/*   SYSTEM   STAMOD  -  Version 1.0                     +-----*/
/*                                                             */
/*-------------------------------------------------------------*/
/*                                                             */
/*    Procedure COMinit                 VERSION 1.00           */
/*                                         DATE                */
/*                                           BY Zhou           */
/*                                                             */
/*                                                             */
/*    DESCRIPTION                                              */
/*      Initialization of the serial port                      */
/*                                                             */
/*                                                             */
/*                                                             */
/*    IN                    DESCRIPTION                        */
/*      portname              name of the port                 */
/*                                                             */
/*      portpar               default portparameter            */
/*                                                             */
/*                                                             */
/*    OUT                   DESCRIPTION                        */
/*                                                             */
/*    RETURN                DESCRIPTION                        */
/*      0                     filedescriptor of open port      */
/*      -1                    error                            */
/*-------------------------------------------------------------*/
int
COMinit(portpar)
	struct s_portparam *portpar;
{
	int             fd;
	int             data_structure;


	/* To set protocol parameters to their initial state */
	portpar->protocoltype = T1;
	portpar->chaining = C_ON;

	portpar->port_id = 1;

	return (0);
}





/*-------------------------------------------------------+-----*/
/*                                                       | GMD */
/*   SYSTEM   STAMOD  -  Version 1.0                     +-----*/
/*                                                             */
/*-------------------------------------------------------------*/
/*                                                             */
/*    Procedure  COMtrans               VERSION 1.00           */
/*                                         DATE                */
/*                                           BY Zhou           */
/*                                                             */
/*                                                             */
/*    DESCRIPTION                                              */
/*      This procedure will transfer APDU's                    */
/*                                                             */
/*                                                             */
/*    IN                    DESCRIPTION                        */
/*      portpar               portparameter / sctparameter     */
/*                                                             */
/*                                                             */
/*    OUT                   DESCRIPTION                        */
/*                                                             */
/*    USES                  DESCRIPTION                        */
/*     COMresynch()                                            */
/*     divi_blk()                                              */
/*     SendTo IFD()                                            */
/*     RECEIVEfromIFD()                                        */
/*     blk_process()                                           */
/*                                                             */
/*    RETURN                DESCRIPTION                        */
/*-------------------------------------------------------------*/
int
COMtrans(portpar, APDU_reqstr, req_len, APDU_respstr, resp_len)
	struct s_portparam *portpar;
	char           *APDU_reqstr;
	int             req_len;
	char            APDU_respstr[];
	int            *resp_len;

{
	int             i;
	unsigned int    sw1 = 0x40;
	unsigned int    sw2 = 0x00;

	if (*(APDU_reqstr + 1) == 0x05) {	/* not S_RESET */
		*resp_len = 19;
		*(APDU_respstr) = 0x10;
		*(APDU_respstr + 1) = 0x3B;
		*(APDU_respstr + 2) = 0x81;
		*(APDU_respstr + 3) = 0x05;
		*(APDU_respstr + 4) = 0x02;
		*(APDU_respstr + 5) = 0x00;
		*(APDU_respstr + 6) = 0x4B;
		*(APDU_respstr + 7) = 0x00;
		*(APDU_respstr + 8) = 0x08;
		*(APDU_respstr + 9) = 0x02;
		*(APDU_respstr + 10) = 0x00;
		*(APDU_respstr + 11) = 0xFF;
		*(APDU_respstr + 12) = 0x00;
		*(APDU_respstr + 13) = 0xFF;
		*(APDU_respstr + 14) = 0xFF;
		*(APDU_respstr + 15) = 0xFF;
		*(APDU_respstr + 16) = 0x00;
		*(APDU_respstr + 17) = 0x40;
		*(APDU_respstr + 18) = 0x00;
	} else {

		if (*(APDU_reqstr + 1) != 0x11) {	/* not S_TRANS */
			*resp_len = 16;
			*(APDU_respstr) = 0x0D;
			*(APDU_respstr + 1) = 0x53;
			*(APDU_respstr + 2) = 0x54;
			*(APDU_respstr + 3) = 0x41;
			*(APDU_respstr + 4) = 0x52;
			*(APDU_respstr + 5) = 0x43;
			*(APDU_respstr + 6) = 0x4F;
			*(APDU_respstr + 7) = 0x53;
			*(APDU_respstr + 8) = 0x11;
			*(APDU_respstr + 9) = 0x11;
			*(APDU_respstr + 10) = 0x01;
			*(APDU_respstr + 11) = 0x80;
			*(APDU_respstr + 12) = 0x00;
			*(APDU_respstr + 13) = 0x01;
			*(APDU_respstr + 14) = 0x40;
			*(APDU_respstr + 15) = 0x00;
		}
		else {
			*resp_len = 10;
			*(APDU_respstr) = 0x07;
			*(APDU_respstr + 1) = 0x31;
			*(APDU_respstr + 2) = 0x32;
			*(APDU_respstr + 3) = 0x33;
			*(APDU_respstr + 4) = 0x34;
			*(APDU_respstr + 5) = 0x35;
			*(APDU_respstr + 6) = 0x90;
			*(APDU_respstr + 7) = 0x00;
			*(APDU_respstr + 8) = 0x40;
			*(APDU_respstr + 9) = 0x00;
		}
	}
	return (0);

}




int
COMreset(portpar)
	struct s_portparam *portpar;
{
	int             fd;
	int             data_structure;



	return (0);
}

int
COMclose(portpar)
	struct s_portparam *portpar;
{

	return (0);
}
