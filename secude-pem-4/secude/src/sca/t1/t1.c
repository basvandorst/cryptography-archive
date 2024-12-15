/*
 *  STARMOD Release 1.1 (GMD)
 */

/*-------------------------------------------------------+-----*/
/*                                                       | GMD */
/*   SYSTEM   STARMOD  -  Version 1.1                    +-----*/
/*                                                             */
/*-------------------------------------------------------------*/
/*    Modifications to Version 1.0:                            */
/*    1.) The S-Block S(WTX, ..) will be supported.            */
/*    2.) The Error Detection Code CRC will be supported       */
/*-------------------------------------------------------------*/
/*                                                             */
/*                                                             */
/*    PACKAGE   T1                      VERSION 1.1            */
/*                                         DATE January 1993   */
/*                                           BY Zhou, Eckstein */
/*                                                             */
/*    FILENAME                                                 */
/*    t1.c                                                     */
/*                                                             */
/*    DESCRIPTION      Procedures necessary for communicating  */
/*                     with Smartcard Terminal.                */
/*                                                             */
/*    Notice:                                                  */
/*    1) For exchange of transmission control information,     */
/*       it supports                                           */
/*       - resynchronization   S(RESYNCH, ...)                 */
/*       - change of the BWT   S(WTX, ...)                     */
/*         If the Smartcard Terminal requires more than BWT to */
/*         process the previously received I-Block, it sends   */
/*         S(WTX, request). The PC acknowledges by             */
/*         S(WTX, response).                                   */
/*    2) For Error Detecte Code, LRC or CRC can be used.       */
/*    3) At the start of the prptpcol the first block to be    */
/*       sent to SCT is a S(RESYNCH,request) (see COMtrans)    */
/*       The numer of repetitions is one.                      */
/*       If an error occurs, the SCT should be reset physically*/
/*       Possible error: the SCT is not connected to the PC.   */
/*                                                             */
/*    Restriction:                                             */
/*     In the current version only 10 chain block will be      */
/*     supported                                               */
/*                                                             */
/*    EXPORT                                                   */
/*      COMinit()             Initiation of serial port        */
/*                                                             */
/*      COMreset()            Re-initiation of port            */
/*                                                             */
/*      COMtrans()            Communication with SCT           */
/*                                                             */
/*      COMclose()            Close serial port                */
/*                                                             */
/*      tp1_err               Error number                     */
/*                                                             */
/*    USES                                                     */
/*      Module from assembly  If define DOS                    */
/*      routine siofunc.asm                                    */
/*                                                             */
/*      RS232_INIT()          Initiate RS232 interface         */
/*                                                             */
/*      SENDSTR()             To send string to RS232 interface*/
/*                                                             */
/*      RECESTR()             To receive string from RS232     */
/*                                                             */
/*    INTERNAL                                                 */
/*      COMsend()             To send a TPDU-request           */
/*                                                             */
/*      COMrece()             To receive a TPDU-response       */
/*                                                             */
/*      State_reset()         To reset sending and receiving   */
/*                            state variables                  */
/*                                                             */
/*      INITforSELECT()       If define _BSD42_ or             */
/*                            _POSIX_                          */
/*                            Initiation for select system call*/
/*                                                             */
/*      blk_process()         To process TPDU-response and     */
/*                            produce an appropriate block     */
/*                                                             */
/*      Resynch()             To resynchronize protocol        */
/*                                                             */
/*      blkvalidity()         To check validity of a block     */
/*                                                             */
/*      err_handle()          To produce an appropriate R_BLOCK*/
/*                                                             */
/*      divi_blk()            To divide a block into small     */
/*                            blocks                           */
/*                                                             */
/*      free_chain            release allocated buffer in      */
/*                            divi_blk                         */
/*                                                             */
/*      Is_lastblkWTXResp()   To check whether the last sent   */
/*                            block was a S(WTX,response).     */
/*                                                             */
/*      Is_ResynchResp()      Is a S(RESYNCH,response)         */
/*                                                             */
/*      getbits()             To read bits  in a integer       */
/*                                                             */
/*      xdmp_tpdu()           print TPDU (only in case of TRACE)*/
/*                                                             */
/*    EXTERNAL                                                 */
/*      aux_fxdump()          only in case of TRACE            */
/*-------------------------------------------------------------*/


/*-------------------------------------------------------------*/
/*   include-Files                                             */
/*-------------------------------------------------------------*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#ifdef _BSD42_
#include <sys/types.h>
#include <termio.h>
#include <sys/ttold.h>          /* defines the TIOCEXCL  ioctl call */
#include <sys/time.h>
#endif  /* from _BSD42_ */

#ifdef _MACC7_
#include <files.h>
#include <time.h>
#include <types.h>
#include <devices.h>
#endif  /* from _MACC7_ */


#ifdef _POSIX_
#include <sys/types.h>
#include <sgtty.h>
#include <sys/time.h>
#endif  /* from _POSIX_ */


#include "t1if.h"
#include "sca.h"
#include "stamod.h"

#ifdef _DOSCTI_
#include "tsi_func.h"
#endif

/*-------------------------------------------------------------*/
/*   extern function definitions                               */
/*-------------------------------------------------------------*/

#ifdef T1TRACE
extern void     aux_fxdump();
#endif

/*   Declare extern DOS assembly module                        */
#ifdef _DOS_
extern int far  RS232_INIT();                           /* To initiate rs232
                                                           interface
                                                        */
extern int far  SENDSTR(int, char far *, int);          /* To send protocol block
                                                           to rs232 interface
                                                        */
extern int far  RECESTR(int, char far *, int, int);     /* To receive protocol
                                                           block from rs232
                                                           interface
                                                        */
#endif  /* from _DOS_ */




/*-------------------------------------------------------------*/
/*   forward global function declarations                      */
/*-------------------------------------------------------------*/
#ifdef _MACC7_
/* is maybe obsolete */
int COMinit(struct s_portparam *portpar);
int COMreset(struct s_portparam *portpar);
int COMclose(struct s_portparam *portpar);
int COMtrans(struct s_portparam *, char *, int , char[], int *);
#endif  /* from _MACC7_ */

/*-------------------------------------------------------------*/
/*   global variable definitions                               */
/*-------------------------------------------------------------*/
int             tp1_err = 0;    /* To include error code        */

#ifdef T1TRACE
FILE           *tp1_trfp = NULL;/* File descriptor of TRACE     */
#endif

/*-------------------------------------------------------------*/
/*   type definitions                                          */
/*-------------------------------------------------------------*/
#define IFSPC 254       /*   information field size for PC     */

#ifdef _POSIX_
#ifndef TIOCEXCL
#define TIOCEXCL        _IO('t', 13)            /* set exclusive use of tty */
#endif
#ifdef FD_ZERO
#undef FD_ZERO
#endif
#define FD_ZERO(s)          ((s)->fds_bits[0] = 0)
#endif  /* from _POSIX_ */



#ifdef _MACC7_
#define MacOk 0
#endif



/*-------------------------------------------------------------*/
/*   local variable definitions                                */
/*-------------------------------------------------------------*/
static int      act_bwt;        /* new; Version 1.1      */

#ifdef _BSD42_
static struct termio o_par, n_par;
#endif  /* from _BSD42_ */

#ifdef _POSIX_
static struct sgttyb o_par, n_par;
#endif  /* from _POSIX_ */

#ifdef _MACC7_
static ParamBlockRec InBlock, OutBlock;
#endif  /* from _MACC7_ */

/*-------------------------------------------------------------*/
/*   forward static function declarations                      */
/*-------------------------------------------------------------*/
static void  State_reset();
static void  free_chain();

#ifdef T1TRACE
static void  xdmp_tpdu();
#endif

#if defined(_BSD42_) || defined(_POSIX_)
static void  INITforSELECT();
#endif  /* from _BSD42_   or _POSIX_ */



/*-------------------------------------------------------------*/
/*                                                       | GMD */
/*                                                       +-----*/
/*    Procedure COMinit                 VERSION 1.0            */
/*                                         DATE                */
/*                                           BY Zhou           */
/*                                                             */
/*                                                             */
/*    DESCRIPTION             Initiation of the serial port    */
/*                                                             */
/*    INOUT                                                    */
/*      portpar               port parameter                   */
/*                            port_id will be set.             */
/*    OUT                                                      */
/*                                                             */
/*    USES                                                     */
/*     Module from assembly                                    */
/*     routine siofunc.asm:                                    */
/*     RS232_INIT()           if DOS                           */
/*                            ERROR                            */
/*                            OPEN_ERR                         */
/*                                                             */
/*    RETURN                                                   */
/*       0                    Ok                               */
/*      -1                    Error                            */
/*                            OPEN_ERR                         */
/*                            INVALID_PORT                     */
/*-------------------------------------------------------------*/
int
COMinit(portpar)
        struct s_portparam *portpar;
{
        int             fd;
        int             data_stru;
        int             ret;
        char            file_name[30];
        char            pid[10];
#ifdef _DOSCTI_
        uchar           port;
#endif
#ifdef T1TRACE
        if (tp1_trfp == NULL) {
                strcpy(file_name,"T1");
                sta_itoa(pid,getpid());
                strcat(file_name,pid);
                strcat(file_name,".TRC");
                tp1_trfp = fopen(file_name, "w");
        }
#endif

/*****************************************************************************/
/*
 *      Open port on DOS-System
 */
#ifdef _DOS_


        if (!strncmp("COM1", portpar->port_name, 4))
                fd = 1;
        else if (!strncmp("COM2", portpar->port_name, 4))
                fd = 2;
        else {
                portpar->port_id = -1;
                tp1_err = INVALID_PORT;
#ifdef T1TRACE
                fprintf(tp1_trfp, "+++++ ERROR : INVALID_PORT\n");
#endif
                return (-1);
        }

        data_stru = portpar->databits | portpar->stopbits | (int) portpar->parity;
        if ((ret = RS232_INIT(fd, portpar->baud, data_stru)) != TP1_OK) {
                tp1_err = OPEN_ERR;
#ifdef T1TRACE
                fprintf(tp1_trfp, "+++++ ERROR : OPEN_ERR\n");
#endif
                return (-1);
        }
#endif  /* from _DOS_ */

/*****************************************************************************/
/*
 *      Open port on DOS-System with PC/CTI functions
 */
#ifdef _DOSCTI_


        if (!strncmp("COM1", portpar->port_name, 4))
                port = TSI_COM1;
        else if (!strncmp("COM2", portpar->port_name, 4))
                port = TSI_COM2;
        else {
                portpar->port_id = -1;
                tp1_err = INVALID_PORT;
#ifdef T1TRACE
                fprintf(tp1_trfp, "+++++ ERROR : INVALID_PORT\n");
#endif
                return (-1);
        }

        if ((ret = tsi_init(&fd,port,TSI_CT_CCR2,TSI_T1,
                            19200l,8,2,'N')) != TSI_OK)      {
                tp1_err=OPEN_ERR;
#ifdef T1TRACE
                fprintf(tp1_trfp,"COMinit :: ERROR: %s\n",tsi_get_msg(ret));
                fprintf(tp1_trfp,"tsi_close called\n");
#endif
                tsi_close(TSI_CLOSE_ALL);
                return(-1);
        }

        fd++;                /* To set port_id */

#endif  /* from _DOSCTI_ */
/*****************************************************************************/
/*
 *      Open port on SUN-System
 */
#ifdef _BSD42_

        if ((fd = open(portpar->port_name, O_RDWR | O_NDELAY)) == -1) {
                portpar->port_id = -1;
                if (errno == 16)
                        /* Device busy */
                        tp1_err = DEV_BUSY;
                else
                        tp1_err = OPEN_ERR;
#ifdef T1TRACE
                fprintf(tp1_trfp, "+++++ ERROR : OPEN_ERR\n");
#endif
                return (-1);
        }

        /*
         *  Set "exclusive-use" mode:
         *       no further opens are permitted until the port has been closed
         */
        ioctl(fd, TIOCEXCL);

        ioctl(fd, TCGETA, &o_par);
        ioctl(fd, TCGETA, &n_par);
        n_par.c_iflag = 0;
        n_par.c_oflag = 0;
        n_par.c_cflag = portpar->baud | portpar->databits | portpar->stopbits
                | (int) portpar->parity | CREAD | CLOCAL;
        n_par.c_lflag = 0;
        ioctl(fd, TCSETAW, &n_par);





#endif  /* from _BSD42_  */


/*****************************************************************************/
/*
 *      Open port on HP-System
 */
#ifdef _POSIX_

        if ((fd = open(portpar->port_name, O_RDWR | O_NDELAY)) == -1) {
                portpar->port_id = -1;
                tp1_err = OPEN_ERR;
#ifdef T1TRACE
                fprintf(tp1_trfp, "+++++ ERROR : OPEN_ERR\n");
#endif
                return (-1);
        }

        /*
         *  Set "exclusive-use" mode:
         *       no further opens are permitted until the file has been closed
         */
        ioctl(fd, TIOCEXCL, &o_par);


        ioctl(fd, TIOCGETP, &o_par);
        ioctl(fd, TIOCGETP, &n_par);

        n_par.sg_ispeed = portpar->baud;
        n_par.sg_ospeed = portpar->baud;
        n_par.sg_flags = RAW;
        ioctl(fd, TIOCSETP, &n_par);
#endif/* from _POSIX_ */

/*****************************************************************************/
/*
 *      Open port on MAC-System
 */
#ifdef _MACC7_
     fd = 1;

     InBlock.ioParam.ioCompletion = NULL;
     InBlock.ioParam.ioNamePtr = (StringPtr) "\p.AIn";
     InBlock.ioParam.ioPermssn = fsRdPerm;

     ret = PBOpen(&InBlock, false);

     OutBlock.ioParam.ioCompletion = NULL;
     OutBlock.ioParam.ioNamePtr = (StringPtr) "\p.AOut";
     OutBlock.ioParam.ioPermssn = fsWrPerm;

     ret = PBOpen(&OutBlock, false);

     ret = SerReset(InBlock.ioParam.ioRefNum,
        portpar->baud | portpar->parity | portpar->databits | portpar->stopbits);
     ret = SerReset(OutBlock.ioParam.ioRefNum,
        portpar->baud | portpar->parity | portpar->databits | portpar->stopbits);

     if (ret != MacOk)
        {
        tp1_err = OPEN_ERR;
        portpar->port_id = -1;
#ifdef T1TRACE
        fprintf(tp1_trfp, "+++++ ERROR : OPEN_ERR\n");
#endif /* T1TRACE */
        return (-1);
        }

#endif  /* from _MACC7_ */



/*****************************************************************************/
/*
 *      port is opened; set port_id in port-element
 */

#ifdef T1TRACE
        fprintf(tp1_trfp, "\nCOMinit :: \n\tSCT Id=%d\n", fd);
#endif
        portpar->port_id = fd;  /* To set port_id */
        portpar->first = 0;
        return (0);
}


/*-------------------------------------------------------------*/
/*                                                       | GMD */
/*                                                       +-----*/
/*    Procedure COMreset                VERSION 1.0            */
/*                                         DATE                */
/*                                           BY Zhou           */
/*                                                             */
/*                                                             */
/*    DESCRIPTION             Initiation of the serial port    */
/*                                                             */
/*    INOUT                                                    */
/*      portpar               Port parameter                   */
/*                                                             */
/*    OUT                                                      */
/*                                                             */
/*    RETURN                                                   */
/*       0                    Ok                               */
/*      -1                    Error                            */
/*                            OPEN_ERR                         */
/*                                                             */
/*-------------------------------------------------------------*/
int
COMreset(portpar)
        struct s_portparam *portpar;
{
        int             data_stru;
        int             ret;
        int             fd;


        fd = portpar->port_id;
        if (fd <= 0) {
                tp1_err = NO_PORT;
#ifdef T1TRACE
                fprintf(tp1_trfp, "+++++ ERROR : NO_PORT\n");
#endif
                return (-1);
        }


/*****************************************************************************/
/*
 *      reset port on DOS-System
 */
#ifdef _DOS_
        data_stru = portpar->databits | portpar->stopbits | (int) portpar->parity;
        if ((ret = RS232_INIT(portpar->port_id, portpar->baud, data_stru)) != TP1_OK) {
                tp1_err = OPEN_ERR;
#ifdef T1TRACE
                fprintf(tp1_trfp, "+++++ ERROR : OPEN_ERR\n");
#endif
                return (-1);
        }
#endif  /* from _DOS_ */

/*****************************************************************************/
/*
 *      reset port on SUN-System
 */

#ifdef _BSD42_
        n_par.c_iflag = 0;
        n_par.c_oflag = 0;
        n_par.c_cflag = portpar->baud | portpar->databits | portpar->stopbits
                | (int) portpar->parity | CREAD | CLOCAL;
        n_par.c_lflag = 0;
        ioctl(portpar->port_id, TCSETAW, &n_par);
#endif  /* from _BSD42__  */


/*****************************************************************************/
/*
 *      reset port on HP-System
 */
#ifdef _POSIX_
        n_par.sg_ispeed = portpar->baud;
        n_par.sg_ospeed = portpar->baud;
        n_par.sg_flags = RAW;
        ioctl(portpar->port_id, TIOCSETP, &n_par);
#endif  /* from _POSIX_  */

/*****************************************************************************/
/*
 *      reset port on MAC-System
 */
#ifdef _MACC7_

        InBlock.ioParam.ioCompletion = NULL;
        InBlock.ioParam.ioNamePtr = (StringPtr) "\p.AIn";
        InBlock.ioParam.ioPermssn = fsRdPerm;

        ret = PBOpen(&InBlock, false);

         OutBlock.ioParam.ioCompletion = NULL;
         OutBlock.ioParam.ioNamePtr = (StringPtr) "\p.AOut";
         OutBlock.ioParam.ioPermssn = fsWrPerm;

        ret = PBOpen(&OutBlock, false);

        ret = SerReset(InBlock.ioParam.ioRefNum,
        portpar->baud | portpar->parity | portpar->databits | portpar->stopbits);
        ret = SerReset(OutBlock.ioParam.ioRefNum,
        portpar->baud | portpar->parity | portpar->databits | portpar->stopbits);

        if (ret != MacOk) {
                tp1_err = OPEN_ERR;
                portpar->port_id = -1;
#ifdef T1TRACE
                fprintf(tp1_trfp, "+++++ ERROR : OPEN_ERR\n");
#endif /* T1TRACE */
                return (-1);
        }

#endif  /* from _MACC7_  */


/*****************************************************************************/
/*
 *      end of COMreset
 */
#ifdef T1TRACE
#ifdef _MACC7_
#else
        fprintf(tp1_trfp, "\nCOMreset :: SCT Id=%d\n", portpar->port_id);
#endif
#endif
        return (0);





}


/*-------------------------------------------------------------*/
/*                                                       | GMD */
/*                                                       +-----*/
/*                                                             */
/*    Procedure COMclose                VERSION 1.0            */
/*                                         DATE                */
/*                                           BY Zhou           */
/*                                                             */
/*                                                             */
/*    DESCRIPTION             Reseting the serial port         */
/*                                                             */
/*    INOUT                                                    */
/*      portpar               port parameter                   */
/*                            port_id will be set.             */
/*                                                             */
/*    OUT                                                      */
/*                                                             */
/*    RETURN                                                   */
/*       0                    Ok                               */
/*-------------------------------------------------------------*/
int
COMclose(portpar)
        struct s_portparam *portpar;
{
        int             ret;
        int             fd;


        fd = portpar->port_id;
        if (fd <= 0)
                return(0);
#ifdef T1TRACE
#ifdef _MACC7_
        fprintf(tp1_trfp, "\nCOMclose :: SCT In=%d, Out=%d\n",
        InBlock.ioParam.ioRefNum, OutBlock.ioParam.ioRefNum);
#else
     fprintf(tp1_trfp,"\nCOMclose :: SCT Id=%d\n",fd);
#endif /* _MACC7_ */
#endif

/*****************************************************************************/
/*
 *      close port on SUN-System
 */
#ifdef _BSD42_
        ioctl(fd, TCSETAW, &o_par);

        close(fd);
#endif  /* from _BSD42_ */

/*****************************************************************************/
/*
 *      close port on HP-System
 */
#ifdef _POSIX_
        ioctl(fd, TIOCSETP, &o_par);
        close(fd);
#endif  /* from _POSIX_ */

/*****************************************************************************/
/*
 *      close port on MAC-System
 */
#ifdef _MACC7_

        ret = PBClose(&InBlock, false);
#ifdef T1TRACE
        fprintf(tp1_trfp, "\tret PBClose(In) = %d", ret);
#endif

        ret = PBClose(&OutBlock, false);
#ifdef T1TRACE
        fprintf(tp1_trfp, "\tret PBClose(Out) = %d\n", ret);
#endif

#endif /* _MACC7_ */

/*****************************************************************************/
/*
 *      close port on DOS-System with PC/CTI-functions
 */
#ifdef _DOSCTI_

        tsi_close(TSI_CLOSE_ALL);
#endif  /* from _DOSCTI_ */



/*****************************************************************************/
/*
 *      return
 */
        portpar->port_id = 0;

        return (0);

}


/*-------------------------------------------------------------*/
/*                                                       | GMD */
/*                                                       +-----*/
/*                                                             */
/*    Procedure  COMtrans          VERSION 1.0                 */
/*                                  MODIFICATION Version 1.1   */
/*                                               January 1993  */
/*                                  DATE                       */
/*                                  BY           Zhou, Eckstein*/
/*                                                             */
/*                                                             */
/*    DESCRIPTION                                              */
/*    To communicating with SCT for transfering APDUs          */
/*    At the start of the protocol the first block to be sent  */
/*    to the SCT is the S(RESYNCH,request).                    */
/*    The numer of repetitions is one.                         */
/*    If an error occurs, the SCT should be reset physically   */
/*    Possible error: the SCT is not connected to the PC.      */
/*                                                             */
/*                                                             */
/*    IN                                                       */
/*      portpar             SCT parameter                      */
/*      APDU_reqstr         Request string                     */
/*      req_len             Length of APDU_reqstr              */
/*                                                             */
/*    INOUT                                                    */
/*                                                             */
/*    OUT                                                      */
/*      resp_len            Length of a complete APDU-response */
/*      APDU_respstr        Response string                    */
/*                                                             */
/*    USES                                                     */
/*     COMsend()            To send a TPDU request             */
/*                            ERROR                            */
/*                            WR_ERR                           */
/*     COMrece()            To receive a TPDU response         */
/*                          Errors of this procedure will not  */
/*                          returned by COMtrans               */
/*                            ERROR                            */
/*                            CHAR_TIMEOUT                     */
/*                            BLK_TIMEOUT                      */
/*                            RD_ERR (only if not DOS)         */
/*                            SELECT_ERR (only if _BSD42_,     */
/*                                        or _POSIX_)          */
/*     blk_process()        To process response, and produce   */
/*                          an appropriate request             */
/*                            ERROR                            */
/*                            SYNTAX_ERR                       */
/*     Resynch()            To resynchronize protocol          */
/*                            ERROR                            */
/*                            SCT_RESET                        */
/*                            PROT_RESYNCH                     */
/*     err_handle()         To indicate errors                 */
/*     divi_blk()           To get data for chaining           */
/*                            ERROR                            */
/*                            MEMO_ERR                         */
/*     free_chain()         release allocated buffer in        */
/*                          divi_blk                           */
/*                                                             */
/*    RETURN                                                   */
/*     0                    Successful                         */
/*    -1                    Error occured                      */
/*                          tp1_err is set to one of the       */
/*                          following values:                  */
/*                          INVAL_TPDULEN                      */
/*                          SCT_RESET                          */
/*                          PROT_RESYNCH                       */
/*                          WR_ERR                             */
/*                          MEMO_ERR                           */
/*                                                             */
/*-------------------------------------------------------------*/
int
COMtrans(portpar, APDU_reqstr, req_len, APDU_respstr, resp_len)
        struct s_portparam *portpar;
        char           *APDU_reqstr;
        int             req_len;
        char            APDU_respstr[];
        int            *resp_len;
{
#ifdef _DOSCTI_
        unsigned char   nadc;
        unsigned char  *resp_data, nad_resp;
#endif
        int             repeat;
        int             ret, i, resp_datlen;
        int             ret_send;
        int             ifsd;
        int             tpdulen;
        int             fd;
        char            rpstr[512];
        char            t1_errno;
        BLOCKstru       block;
        CHAINstru       chaindata;
#ifdef _BSD42_
#ifdef TIME
        struct timeval  day_time, day_zone;
#endif
#endif

/*****************************************************************************/
/*
 *      in the current version only 10 chain block will be supported
 *      initialize chaindata , return parameter and block structure
 */



        fd = portpar->port_id;
        if (fd <= 0) {
                tp1_err = NO_PORT;
#ifdef T1TRACE
                fprintf(tp1_trfp, "+++++ ERROR : NO_PORT\n");
#endif
                return (-1);
        }



        *resp_len = 0;

 /*****************************************************************************/
/*
 *      COMtrans on DOS-System with PC/CTI-functions
 */
#ifdef _DOSCTI_

        fd = portpar->port_id - 1;
        nadc = (portpar->dad << 4) & 0xf0 | portpar->sad & 0x0f;


#ifdef T1TRACE

        fprintf(tp1_trfp, "\nTPDU_request: ");
        fprintf(tp1_trfp, "    I_BLOCK\n");
        fprintf(tp1_trfp, "    NAD(0x%02x) PCB(0xXX) LEN(0x%02x)\n",
                nadc,
                req_len & 0xFF);

        fprintf(tp1_trfp, "    INF:\n");
        aux_fxdump(tp1_trfp,APDU_reqstr,req_len  & 0xFF, 0);

        fprintf(tp1_trfp, "    EDC (LEN = 1): ??\n");
#endif
        ret = tsi_api(fd,nadc,&nad_resp,req_len,APDU_reqstr,
                     &resp_datlen,&resp_data);

        if (ret != TSI_OK) {
                tp1_err=WR_ERR;
#ifdef T1TRACE
                fprintf(tp1_trfp,"COMtrans :: ERROR: %d\n",ret);
                fprintf(tp1_trfp,"COMtrans :: ERROR: %s\n",tsi_get_msg(ret));
                fprintf(tp1_trfp,"tsi_close called\n");
#endif
                portpar->port_id = 0;
                tsi_close(TSI_CLOSE_ALL);
                return(-1);
        }
        else {
                for (i=0; i<resp_datlen; i++)
                   APDU_respstr[i] = resp_data[i];
                *resp_len = resp_datlen;

#ifdef T1TRACE

        fprintf(tp1_trfp, "\nTPDU_response: ");
        fprintf(tp1_trfp, "    I_BLOCK\n");
        fprintf(tp1_trfp, "    NAD(0x%02x) PCB(0xXX) LEN(0x%02x)\n",
                nad_resp,
                *resp_len & 0xFF);

        fprintf(tp1_trfp, "    INF:\n");
        aux_fxdump(tp1_trfp,APDU_respstr,*resp_len  & 0xFF, 0);

        fprintf(tp1_trfp, "    EDC (LEN = 1): ??\n");

#endif
        }

        return(0);
#endif  /* from _DOSCTI_ */



        block.I_rqstr = NULL;
        block.S_rqstr = NULL;


        for (i = 0; i < 10; i++)
                chaindata.sub_rqstr[i] = NULL;
        chaindata.amount = 0;
        chaindata.sequence = 0;


        tp1_err = 0;
        if (!portpar->first) {
                ret = Resynch(portpar, 1);
                if (ret != 0)
                        return (-1);
                portpar->first = 1;
        }
        ifsd = portpar->tpdusize - ((portpar->edc == E_LRC) ? 4 : 5);
        tpdulen = req_len + BLKHDLEN + ((portpar->edc == E_LRC) ? 1 : 2);

/*****************************************************************************/
/*
 *      Should chaining function be used ?
 */
        if (tpdulen > portpar->tpdusize) {
                if (portpar->chaining == C_OFF) {
                        tp1_err = INVAL_TPDULEN;
#ifdef T1TRACE
                        fprintf(tp1_trfp, "+++++ ERROR : INVAL_TPDULEN\n");
#endif
                        return (-1);
                }

                /*
                 *      To prepare for chaining
                */
                if (req_len > 10 * ifsd) {      /* new; Version 1.1 */
                        tp1_err = INVAL_TPDULEN;
#ifdef T1TRACE
                        fprintf(tp1_trfp, "+++++ ERROR : INVAL_TPDULEN\n");
#endif
                        return (-1);
                }
                chaindata.amount = divi_blk(ifsd, APDU_reqstr, req_len, &chaindata);
                if (chaindata.amount < 0)
                        return (-1);

                /*
                 *      To send first chained block
                 */
                block.I_rqstr = chaindata.sub_rqstr[0];
                block.I_inflen = chaindata.sublen[0];

                /*
                 *      To set more bit in protocol control byte of I_BLOCK
                 */
                block.ms = T_MORE;
        } else {
                /*
                 *  Chaining will not be used
                 */
                block.I_rqstr = APDU_reqstr;
                block.I_inflen = req_len;
                block.ms = 0;
        }

/*****************************************************************************/
/*
 *      send  and receive TPDU
 */

        block.blktype = I_BLOCK;
        act_bwt = portpar->bwt; /* new; Version 1.1 */


        do {
                repeat = 0;
                do {
                        /*
                         *      send TPDU
                         */
                        if ((ret_send = COMsend(portpar, &block, &t1_errno)) == -1) {

                                /*
                                 * release allocated buffer in divi_blk; new
                                 * Version 1.1
                                 */
                                free_chain(&chaindata);
                                return (-1);
                        }

#ifdef _BSD42_
#ifdef TIME
                        gettimeofday(&day_time, &day_zone);

                        fprintf(tp1_trfp,"Before COMRECE - timer.tv_sec = %ld\n",day_time.tv_sec);
                        fprintf(tp1_trfp,"timer.tv_usec = %ld\n",day_time.tv_usec);
#endif
#endif
                        /*
                         *      receive TPDU
                         */


                        ret = COMrece(portpar->port_id, rpstr,
                                      act_bwt, portpar->cwt, portpar->edc,
                                      &t1_errno);
                        if (ret == 0)
                                ret = blk_process(rpstr, &portpar->ns,
                                                  &portpar->rsv, &block,
                                                  &chaindata, APDU_respstr,
                                                  resp_len);

#ifdef _BSD42_
#ifdef TIME
                        gettimeofday(&day_time, &day_zone);

                        fprintf(tp1_trfp,"After  COMRECE - timer.tv_sec = %ld\n",day_time.tv_sec);
                        fprintf(tp1_trfp,"timer.tv_usec = %ld\n",day_time.tv_usec);
#endif
#endif

#ifdef T1TRACE
                        fprintf(tp1_trfp, "\nret = %d and repeat = %d\n",ret,repeat);
#endif


                        if ((ret != 0) && (ret != NEXTBLOCK) && (repeat == BLKREPEAT - 1)) {
#ifdef T1TRACE
                                fprintf(tp1_trfp, "\nPC begins resynchronizing SCT\n");
#endif
                                ret = Resynch(portpar, BLKREPEAT);


                                /*
                                 * release allocated buffer in divi_blk; new
                                 * Version 1.1
                                 */
                                free_chain(&chaindata);
                                return (-1);
                        }
                        if ((ret != 0) && (ret != NEXTBLOCK)) {

#ifdef T1TRACE
                                fprintf(tp1_trfp, "\nerror - handling ?\n");
#endif
                                repeat++;
                                if (ret == -1)
                                        err_handle(portpar->rsv, &block.blktype, &block.nr);
                        }





                } while ((repeat < BLKREPEAT) && (ret != 0));
        } while (ret != 0);

/*****************************************************************************/
/*
 *      normal end => release allocated buffer in divi_blk; new Version 1.1
 */

        free_chain(&chaindata);


        return (0);

}


/*-------------------------------------------------------------*/
/*                                                       | GMD */
/*                                                       +-----*/
/*    Procedure State_reset             VERSION 1.0            */
/*                                         DATE                */
/*                                           BY Zhou           */
/*                                                             */
/*                                                             */
/*    DESCRIPTION           To reset sending and receiving     */
/*                          state variables                    */
/*                                                             */
/*    IN                                                       */
/*                                                             */
/*    INOUT                                                    */
/*                                                             */
/*    OUT                                                      */
/*      ssv                 Sending state variable             */
/*      rsv                 Receiving state variable           */
/*                                                             */
/*    RETURN                                                   */
/*-------------------------------------------------------------*/
static
void
State_reset(ssv, rsv)
        int            *ssv;
        int            *rsv;
{
        *ssv = 0;
        *rsv = 1;
}


/*-------------------------------------------------------------*/
/*                                                       | GMD */
/*                                                       +-----*/
/*                                                             */
/*    Procedure  COMsend          VERSION 1.0                  */
/*                                 DATE                        */
/*                                 MODIFICATION  Version 1.1   */
/*                                               January 1993  */
/*                                 BY            Zhou, Eckstein*/
/*                                                             */
/*                                                             */
/*    DESCRIPTION             To send a TPDU-request           */
/*                                                             */
/*    IN                                                       */
/*     portpar                Port parameters                  */
/*     block                  Protocol block                   */
/*     t1_errno               error number for R_BLOCK         */
/*                            is set in COMrece                */
/*                                                             */
/*    OUT                                                      */
/*                                                             */
/*    USES                                                     */
/*     Moudle from assembly                                    */
/*     routine siofunc.asm:                                    */
/*                                                             */
/*     SENDSTR()              if DOS                           */
/*                            ERROR                            */
/*                            WR_ERR                           */
/*                                                             */
/*    RETURN                                                   */
/*       0                    Successful                       */
/*      -1                    Error occured                    */
/*                            WR_ERR                           */
/*-------------------------------------------------------------*/
static
int
COMsend(portpar, block, t1_errno)
        struct s_portparam *portpar;
        BLOCKstru          *block;      /* BLOCKstru is defined in t1if.h */
        char               *t1_errno;
{
        char            outstr[512];
        char           *piostr;
        int             i;
        int             k;
        int             ret;
        int             blklen, bcc, bcclen, edclen;

        piostr = &outstr[0];
        bcclen = 0;

/*****************************************************************************/
/*
 *      Node address :: DAD + SAD
 */

        *piostr++ = (portpar->dad << 4) & 0xf0 | portpar->sad & 0x0f;

        switch (block->blktype) {

/*****************************************************************************/
/*
 *      I_BLOCK
 */
        case I_BLOCK:
                /*
                 *   Protocol control byte of I_BLOCK :: NS + MS
                 */
                *piostr++ = portpar->ns | block->ms;

                /*
                 *   Length field of I_BLOCK
                 */
                *piostr++ = block->I_inflen;
                bcclen = block->I_inflen;

                /*
                 *   Information field
                 */
                for (i = 0; i < block->I_inflen; i++)
                        *piostr++ = *(block->I_rqstr + i);
                break;

/*****************************************************************************/
/*
 *      S_BLOCK
 */
        case S_BLOCK:
                /*
                 *   Protocol control byte of S_BLOCK
                 */
                *piostr++ = block->S_respbit | block->S_ctl;

                /*
                 *   Length field of S_BLOCK
                 */
                *piostr++ = block->S_inflen;
                bcclen = block->S_inflen;

                /*
                 *   Information field
                 */
                for (i = 0; i < block->S_inflen; i++)
                        *piostr++ = *(block->S_rqstr + i);
                break;

/*****************************************************************************/
/*
 *      R_BLOCK
 */
        case R_BLOCK:

                /*
                 *   Protocol control byte of R_BLOCK
                 */
                *piostr++ = 0x80 | block->nr | *t1_errno;

                /*
                 *   Length field of R_BLOCK is 0
                 */
                *piostr++ = 0x00;
                break;
        }


/*****************************************************************************/
/*
 *      Computing checksum  (modification; Version 1.1 )
 */
        switch (portpar->edc) {
        case E_LRC:
                bcclen += BLKHDLEN;
                bcc = 0;
                for (i = 0; i < bcclen; i++)
                        bcc = bcc ^ outstr[i];
                *piostr++ = bcc;
                *piostr = '\0';
                blklen = bcclen + LRC_LEN;
                edclen = 1;
                break;
        case E_CRC:
                /* noch zu implementieren */
                break;

        }


#ifdef T1TRACE
        xdmp_tpdu(tp1_trfp, outstr, blklen, 0, edclen);
#endif

/*****************************************************************************/
/*
 *      To send TPDU-request
 */


#if defined(_DOS_) || defined(_POSIX_) || defined(_MACC7_)
        for (i = 0; i < blklen; i++) {
#ifdef _DOS_
                if ((ret = SENDSTR(portpar->port_id, &outstr[i], 1)) != TP1_OK)
#endif /* from _DOS_ */


#ifdef _POSIX_
                if ((ret = write(portpar->port_id, &outstr[i], 1)) == -1)
#endif  /* from _POSIX_ */

#ifdef _MACC7_
                OutBlock.ioParam.ioBuffer = &outstr[i];
                OutBlock.ioParam.ioReqCount = 1;
                if ( (ret = PBWrite(&OutBlock, false)) != MacOk)
#endif /* from _MACC7_ */

                {
                        tp1_err = WR_ERR;
#ifdef T1TRACE
                        fprintf(tp1_trfp, "+++++ ERROR : WR_ERR\n");
#endif
                        return (-1);
                }

        }
#endif


#ifdef _BSD42_
                if ((ret = write(portpar->port_id, outstr, blklen)) == -1)


                {
                        tp1_err = WR_ERR;
#ifdef T1TRACE
                        fprintf(tp1_trfp, "+++++ ERROR : WR_ERR\n");
#endif
                        return (-1);
                }


#endif /* from _BSD42_ */




/*****************************************************************************/
/*
 *      After DTE has sent S(WTX,response) act_bwt is set to the
 *      new value    (new; Version 1.1 )
 */
        if (Is_lastblkWTXResp(block->blktype, block->S_ctl, block->S_respbit)) {
                act_bwt = act_bwt * (*(block->S_rqstr));
        }


        return (0);
}


/*-------------------------------------------------------------*/
/*                                                       | GMD */
/*                                                       +-----*/
/*    Procedure  COMrece         VERSION 1.0                   */
/*                                 DATE                        */
/*                                 MODIFICATION Version 1.1    */
/*                                              January 1993   */
/*                                 BY           Zhou, Eckstein */
/*                                                             */
/*                                                             */
/*    DESCRIPTION             To receive a TPDU-response       */
/*                                                             */
/*    IN                                                       */
/*      fd                    port identifier                  */
/*      bwt                   block waiting time               */
/*      cwt                   char  waiting time               */
/*      edc                   error detection code             */
/*    OUT                                                      */
/*      rpstr                 TPDU-response string             */
/*      t1_errno              error numer for R_BLOCK          */
/*                                                             */
/*    USES                                                     */
/*     Module from assembly                                    */
/*     routine siofunc.asm:                                    */
/*     RECESTR()              if DOS                           */
/*                            ERROR                            */
/*                            CHAR_TIMEOUT                     */
/*                            BLK_TIMEOUT                      */
/*                                                             */
/*     blkvalidity()          To check validity of response    */
/*                            ERROR                            */
/*                            EDC_ERR                          */
/*                                                             */
/*     select                 if _BSD42_ or _POSIX_            */
/*                            select system call               */
/*    RETURN                                                   */
/*       0                    Successful                       */
/*      -1                    Error occured                    */
/*                            CHAR_TIMEOUT                     */
/*                            BLK_TIMEOUT                      */
/*                            RD_ERR (only if not DOS)         */
/*                            SELECT_ERR (only if _BSD42_,     */
/*                                        or _POSIX_)          */
/*-------------------------------------------------------------*/
static
int
COMrece(fd, rpstr, bwt, cwt, edc, t1_errno)
        int             fd;
        unsigned char  *rpstr;
        int             bwt;    /* block waiting time */
        int             cwt;    /* chars waiting time */
        EdcType         edc;    /* new; Version 1.1   */
        char            *t1_errno;
{

        int             i, ret;
        int             resplen;
        int             rdind;
        int             callrd;
        int             length;
        int             index;
        int             wt;     /* Waiting time */

#ifdef _BSD42_
#ifdef TIME
        struct timeval  day_time, day_zone;
#endif
#endif  /* from _BSD42_  */


#if defined(_BSD42_) || defined(_POSIX_)
        int             wait_mark;
        int             width, nfds;
        fd_set          readfds, writefds, exceptfds;
        struct timeval  timeout;
        timerclear(&timeout);
#endif  /* from _BSD42_ or _POSIX_ */





#ifdef _MACC7_
        clock_t Start;
#endif /* from _MACC7_ */



        resplen = BLKHDLEN;
        callrd = 2;
        index = 0;
        *t1_errno = 0x02;

        for (rdind = 0; rdind < callrd; rdind++) {
                for (i = 0; i < resplen; i++) {

/*****************************************************************************/
/*
 *      receive TPDU on DOS-System
 */
#ifdef _DOS_
                        wt = (!i && !rdind) ? (bwt * 18 + 1) : ((cwt * 18) / 10 + 1);
                        if ((ret = RECESTR(fd, rpstr + index + i, 1, wt)) != TP1_OK) {
#ifdef T1TRACE
                                if (ret == BLK_TIMEOUT)
                                        fprintf(tp1_trfp, "+++++ ERROR : BLK_TIMEOUT\n");
                                else
                                        fprintf(tp1_trfp, "+++++ ERROR : CHAR_TIMEOUT\n");
#endif
                                return (-1);
                        }
#endif  /* from _DOS_ */

/*****************************************************************************/
/*
 *      receive TPDU on SUN-System or HP-System
 */
#if defined(_BSD42_) || defined(_POSIX_)
                        /*
                         *   To check whether data on line is ready within
                         *   waiting time
                         */
                        if (!i && !rdind) {
                                wt = bwt;
                                wait_mark = BLOCK_WAIT;
                        } else {
                                wt = cwt;
                                wait_mark = CHAR_WAIT;
                        }
                        INITforSELECT(fd, wait_mark, wt, &readfds,
                                      &writefds, &exceptfds, &timeout);
                        width = getdtablesize();
                        nfds = select(width, &readfds, &writefds,
                                      &exceptfds, &timeout);
                        if (nfds == 0) {
                                /*
                                 * Time is out !
                                 */
                                ret = (!i && !rdind) ? BLK_TIMEOUT : CHAR_TIMEOUT;
#ifdef T1TRACE
                                if (ret == BLK_TIMEOUT)
                                        fprintf(tp1_trfp, "+++++ ERROR : BLK_TIMEOUT\n");
                                else
                                        fprintf(tp1_trfp, "+++++ ERROR : CHAR_TIMEOUT\n");
#endif
                                return (-1);
                        } else if (nfds == -1) {
                                /*
                                 * Error occured from select system call
                                 */
                                ret = SELECT_ERR;
#ifdef T1TRACE
                                fprintf(tp1_trfp, "+++++ ERROR : SELECT_ERR\n");
#endif
                                return (-1);
                        }

#ifdef TIME
                        gettimeofday(&day_time, &day_zone);

                        fprintf(tp1_trfp,"Before READ - timer.tv_sec = %ld\n",day_time.tv_sec);
                        fprintf(tp1_trfp,"timer.tv_usec = %ld\n",day_time.tv_usec);
#endif

                        if ((ret = read(fd, rpstr + index + i, 1)) != 1) {

#ifdef T1TRACE
                                fprintf(tp1_trfp, "+++++ ERROR : RD_ERR\n");
#endif
                                return (-1);
                        }

#ifdef TIME
                        gettimeofday(&day_time, &day_zone);

                        fprintf(tp1_trfp,"AFTER READ - timer.tv_sec = %ld\n",day_time.tv_sec);
                        fprintf(tp1_trfp,"timer.tv_usec = %ld\n",day_time.tv_usec);
#endif

#endif  /* from _BSD42_ or _POSIX_*/

/*****************************************************************************/
/*
 *      receive TPDU on MAC-System
 */
#ifdef _MACC7_

                        /* Erst mal ermitteln, wieviele Clocks die BWT bzw. CWT
                         * sind
                         * Ganz zu Anfang gilt die Block-Waiting-Time,
                         * zwischen Chars eines Blocks gilt die
                         * Character-Waiting-Time
                         */

                        if( !i && !rdind )
                                wt = bwt * CLOCKS_PER_SEC;
                        else
                                wt = cwt / 10.0 * CLOCKS_PER_SEC;

                        /* naechstes Zeichen nach rpstr+index+i lesen */
                        InBlock.ioParam.ioCompletion = NULL;
                        InBlock.ioParam.ioBuffer = (Ptr)rpstr + index + i;
                        InBlock.ioParam.ioReqCount = 1;

                        /* Asynchronen Read-Request abschicken */
                        ret = PBRead(&InBlock, true);

                        /* Abwarten, bis Daten angekommen sind.
                         * Wenn's zulange dauert, Fehler!
                         */

                        /* Start-Clocks merken */
                        Start = clock();
                        /* und warten */
                        while ((InBlock.ioParam.ioResult == 1) &&
                               (clock() - Start <= wt)) ;

                         /* TimeOut? */

                        if (InBlock.ioParam.ioResult == 1) {
                                ret = (!i && !rdind) ? BLK_TIMEOUT : CHAR_TIMEOUT;
#ifdef T1TRACE
                                if (ret == BLK_TIMEOUT)
                                        fprintf(tp1_trfp, "+++++ ERROR : BLK_TIMEOUT\n");
                                else
                                        fprintf(tp1_trfp, "+++++ ERROR : CHAR_TIMEOUT\n");
#endif
                                /* Timeout, daher Read-Request noch killen */

                                ret = PBKillIO(&InBlock, false);
#ifdef T1TRACE
                                fprintf(tp1_trfp, "ret PBKillIO(In) = %d\n", ret);
#endif

                                return (-1);
                        } /* if TimeOut */

                        /* Lesefehler? */

                        if ((InBlock.ioParam.ioResult != MacOk) ||
                            (InBlock.ioParam.ioActCount == 0)) {

#ifdef T1TRACE
                                fprintf(tp1_trfp, "+++++ ERROR : RD_ERR\n");
#endif /* T1TRACE */

                                return (-1);
                        } /* if Lesefehler */

#endif /* from _MACC7_ */

                }               /* end of FOR(i) */


                if (*(rpstr + BLKLPOS) > IFSPC) {
#ifdef T1TRACE
                        fprintf(tp1_trfp, "+++++ ERROR : INVALID_LEN\n");
#endif
                        return (-1);
                }
                resplen = *(rpstr + BLKLPOS) + 1;
                index = 3;
        }                       /* end of FOR(rdind) */


#ifdef T1TRACE
        if (edc == E_LRC)
                xdmp_tpdu(tp1_trfp, rpstr, resplen + 3, 1, 1);
        else
                xdmp_tpdu(tp1_trfp, rpstr, resplen + 3, 1, 2);

#endif

/*****************************************************************************/
/*
 *      check if received response is a valid block
 */


        ret = blkvalidity(rpstr, edc);
        if (ret == ERROR)
                *t1_errno = 0x01;
        return (ret);
}


/*-------------------------------------------------------------*/
/*                                                       | GMD */
/*                                                       +-----*/
/*    Procedure  blkvalidity     VERSION 1.0                   */
/*                                 DATE                        */
/*                                 MODIFICATION Version 1.1    */
/*                                              January 1993   */
/*                                 BY           Zhou, Eckstein */
/*                                                             */
/*                                                             */
/*    DESCRIPTION                                              */
/*      To check the validity of TPDU received                 */
/*                                                             */
/*                                                             */
/*    IN                                                       */
/*      rpstr               TPDU-response string               */
/*      edc                 error detection code               */
/*                                                             */
/*    INOUT                                                    */
/*                                                             */
/*    OUT                                                      */
/*                                                             */
/*    RETURN                                                   */
/*       0                    Valid                            */
/*      -1                    Invalid                          */
/*                            EDC_ERR                          */
/*                                                             */
/*-------------------------------------------------------------*/
static
int
blkvalidity(rpstr, edc)
        unsigned char   rpstr[];
        EdcType         edc;    /* new; Version 1.1 */
{
        int             i;
        int             bcc, bcclen;
        int             ret;

        switch (edc) {
        case E_LRC:
                bcc = 0;
                bcclen = BLKHDLEN + rpstr[BLKLPOS];
                for (i = 0; i < bcclen; i++)
                        bcc = bcc ^ rpstr[i];

                if (bcc != rpstr[bcclen]) {
#ifdef T1TRACE
                        fprintf(tp1_trfp, "+++++ ERROR : EDC_ERR\n");
#endif
                        return (ERROR);
                } else
                        return (TP1_OK);
                break;

        case E_CRC:
                /* noch zu implementieren */
                break;
        }



}


/*-------------------------------------------------------------*/
/*                                                       | GMD */
/*                                                       +-----*/
/*    Procedure  err_handle             VERSION 1.0            */
/*                                         DATE                */
/*                                           BY Zhou           */
/*                                                             */
/*                                                             */
/*    DESCRIPTION           To recovey errors by producing     */
/*                          an appropriate R_BLOCK             */
/*                                                             */
/*    IN                                                       */
/*      rsv                 Receiving state variable           */
/*                                                             */
/*    INOUT                                                    */
/*                                                             */
/*    OUT                                                      */
/*      blktype             Type of protocol block             */
/*      nr                  Receiving sequence number          */
/*                                                             */
/*    RETURN                                                   */
/*      ERROR               Error indication                   */
/*-------------------------------------------------------------*/
static
int
err_handle(rsv, blktype, nr)    /* to retransmit last block */
        int             rsv;
        int            *blktype;
        int            *nr;
{
        *blktype = R_BLOCK;
        *nr = (!NOT(rsv)) ? 0 : NR_1;
        return (ERROR);
}


/*-------------------------------------------------------------*/
/*                                                       | GMD */
/*                                                       +-----*/
/*                                                             */
/*    Procedure  Is_lastblkWTXResp      VERSION 1.1            */
/*                                         DATE January 1993   */
/*                                           BY Eckstein       */
/*                                                             */
/*                                                             */
/*    DESCRIPTION      To check whether the last send block    */
/*                     was a S(WTX,response)                   */
/*                                                             */
/*    IN                                                       */
/*      blktype        Type of block                           */
/*      S_ctl          Control function of S_BLOCK             */
/*      S_respbit      Response indication of S_BLOCK          */
/*                                                             */
/*    INOUT                                                    */
/*                                                             */
/*    OUT                                                      */
/*                                                             */
/*    RETURN                                                   */
/*       0                    Negative                         */
/*       1                    Positive                         */
/*                                                             */
/*-------------------------------------------------------------*/
static
int
Is_lastblkWTXResp(blktype, S_ctl, S_respbit)
        int             blktype;
        int             S_ctl;
        int             S_respbit;
{
        if (blktype != S_BLOCK)
                return (0);
        if (S_ctl != WTX)
                return (0);
        if (S_respbit != RESPONSE)
                return (0);
        return (1);
}



/*-------------------------------------------------------------*/
/*                                                       | GMD */
/*                                                       +-----*/
/*    Procedure  INITforSELECT          VERSION 1.0            */
/*                                         DATE                */
/*                                           BY Zhou           */
/*                                                             */
/*                                                             */
/*    DESCRIPTION           Initiation for select system call  */
/*                                                             */
/*                                                             */
/*                                                             */
/*    IN                                                       */
/*      fd                  SCT identifier                     */
/*      wt                  Waiting time when receiving        */
/*                                                             */
/*    INOUT                                                    */
/*                                                             */
/*    OUT                                                      */
/*      readfds                                                */
/*      writefds                                               */
/*      exceptfds                                              */
/*      timeout                                                */
/*                                                             */
/*-------------------------------------------------------------*/
#if defined(_BSD42_) || defined(_POSIX_)
static
void
INITforSELECT(fd, waitmk, wt, readfds, writefds, exceptfds, timeout)
        int             fd;
        int             waitmk;
        int             wt;
        fd_set         *readfds, *writefds, *exceptfds;
        struct timeval *timeout;
{
        FD_ZERO(readfds);
        FD_ZERO(writefds);
        FD_ZERO(exceptfds);

/*****************************************************************************/
/*
 *      To check fd in read file descriptor set
 */

        FD_SET(fd, readfds);

/*****************************************************************************/
/*
 *      To set tioimout value
 */
        if (waitmk == BLOCK_WAIT) {
                timeout->tv_sec = wt;
                timeout->tv_usec = 0;
        } else {
                timeout->tv_sec = 0;
                timeout->tv_usec = wt * 100000;
        }
}

#endif


/*-------------------------------------------------------------*/
/*                                                       | GMD */
/*                                                       +-----*/
/*    Procedure  getbits                VERSION 1.0            */
/*                                         DATE                */
/*                                           BY Zhou           */
/*                                                             */
/*                                                             */
/*    DESCRIPTION           To read bits in an integer         */
/*                                                             */
/*                                                             */
/*                                                             */
/*    IN                                                       */
/*      x                   Unsigned integer to read           */
/*      p                   Position from which bits is read   */
/*      n                   Amount of bits to read             */
/*                                                             */
/*    INOUT                                                    */
/*                                                             */
/*    OUT                                                      */
/*                                                             */
/*    RETURN                Value has been read                */
/*                                                             */
/*-------------------------------------------------------------*/
static
int
getbits(x, p, n)
        unsigned        x, p, n;
{
        return ((x >> (p + 1 - n)) & ~(~0 << n));
}



/*-------------------------------------------------------------*/
/*                                                       | GMD */
/*                                                       +-----*/
/*    Procedure  blk_process    VERSION 1.0                    */
/*                                  DATE                       */
/*                                  MODIFICATION Version 1.1   */
/*                                               January 1993  */
/*                                  BY           Zhou, Eckstein*/
/*                                                             */
/*                                                             */
/*    DESCRIPTION                                              */
/*      To process the block received, and produce             */
/*      appropriate block                                      */
/*                                                             */
/*    IN                                                       */
/*      rpstr                 Response string                  */
/*                                                             */
/*    INOUT                                                    */
/*      ssv                   Sending state variable           */
/*      rsv                   Receiving state variable         */
/*      block                 Protocol block data              */
/*      chaindata             Data for chaining                */
/*      outlen                Length of APDU                   */
/*                                                             */
/*    USES                                                     */
/*      getbits()             To get bits in an integer        */
/*      Is_lastblkWTXResp()   To check whether the last sent   */
/*                            block was a S(WTX,response)      */
/*                                                             */
/*    OUT                                                      */
/*      outdata               APDU response                    */
/*                                                             */
/*    RETURN                                                   */
/*        0                   Procession is finished           */
/*       -1                   Error occured                    */
/*                            SYNTAX_ERR                       */
/*                                                             */
/*       -2                   To continue receiving data       */
/*-------------------------------------------------------------*/
static
int
blk_process(rpstr, ssv, rsv, block, chaindata, outdata, outlen)
        char   rpstr[];
        int            *ssv;
        int            *rsv;
        BLOCKstru      *block;
        CHAINstru      *chaindata;
        unsigned char   outdata[];
        int            *outlen;
{
        int             i;
        int             Pcb;    /* Protocol control byte */
        int             BLK_TYPE;
        int             k;
        int             R_nr;   /* The N(R) of rpstr received */
        int             Mr;     /* More bit of PCB of respones */
        int             New_rsv;
        int             ret;
        int             inf_len;

        Pcb = rpstr[1];         /* Protocol control byte */
        BLK_TYPE = getbits(Pcb, 7, 2);
        switch (BLK_TYPE) {
/*****************************************************************************/
/*
 *       R_BLOCK received
 */

        case R_BLOCK:
                /*
                 * Length byte of R_BLOCK should be zero
                 */
                if (rpstr[BLKLPOS]) {
#ifdef T1TRACE

                        fprintf(tp1_trfp, "+++++ ERROR : SYNTAX_ERR (R_BLOCK with wrong length byte received)\n");
#endif
                        return (-1);
                }
                R_nr = (Pcb & NR_1) ? NS_1 : 0;
                if (Is_lastblkWTXResp(block->blktype, block->S_ctl, block->S_respbit)) {

                        /*
                         * After PC sent S(WTX,response), an I or S-block
                         * should be received; modification Version 1.1
                         */

#ifdef T1TRACE
                        fprintf(tp1_trfp, "+++++ ERROR : SYNTAX_ERR (R_BLOCK after S(WTX...) received)\n");
#endif
                        return (-1);

                }
                if (R_nr == *ssv) {
                        /*
                         * To retransmit last I-Block
                         */

                        block->blktype = I_BLOCK;
#ifdef T1TRACE

                        fprintf(tp1_trfp, "===== R_BLOCK with error received\n");
#endif
                        return (CONTINUE);
                } else if (block->ms) { /* With chain_function at PC    */

                        /*
                         * To ask to transmit I_block with sending sequence
                         * number equal to R_nr
                         */
                        block->blktype = I_BLOCK;
                        *ssv = (R_nr == 0) ? 0 : NS_1;
                        chaindata->sequence++;

                        /*
                         * Is the last chained block ? yes, More bit is
                         * cleared
                         */
                        if (chaindata->sequence == chaindata->amount - 1)
                                block->ms = 0;
                        else
                                block->ms = T_MORE;

                        block->I_rqstr = chaindata->sub_rqstr[chaindata->sequence];
                        block->I_inflen = chaindata->sublen[chaindata->sequence];
                        /*
                         * set *rsv to last received I_Block
                         */
                        *rsv = (R_nr == 0) ? 1 : 0;
#ifdef T1TRACE

                        fprintf(tp1_trfp, "===== rsv of errorfree R_BLOCK in chaining mode = %x\n",*rsv);
#endif

                        return (NEXTBLOCK);
                } else {
#ifdef T1TRACE
                        fprintf(tp1_trfp, "+++++ ERROR : SYNTAX_ERR (R_BLOCK in chaining mode received)\n");
#endif
                        return (-1);
                }
                break;

/*****************************************************************************/
/*
 *      S_BLOCK received
 */
        case S_BLOCK:

                i = getbits(Pcb, 2, 3);
                switch (i) {
                        /*
                         * S(RESYNCH ...) received
                         */
                case RESYCH:


                        /*
                         * S(RESYNCH,response) should be received in
                         * procedure Resynch()
                         * and S(RESYNCH,request) should only be sent by the
                         * DTE
                         */
#ifdef T1TRACE
                        fprintf(tp1_trfp, "+++++ ERROR : SYNTAX_ERR (S(RESYNCH,...) received)\n");
#endif
                        return (-1);
                        break;
                        /*
                         * S(WTX ...) received
                         */
                case WTX:       /* new; Version 1.1  */
                        /*
                         * ERROR, if S(WTX,response will be received or
                         * the length-byte is not 1
                         */
                        if ((Pcb & 0x20) ||
                            (rpstr[2] != 0x01)) {
#ifdef T1TRACE
                                fprintf(tp1_trfp, "+++++ ERROR : SYNTAX_ERR (wrong S(WTX,... received)\n");
#endif
                                return (-1);
                        }
                        /*
                         * To send S(WTX,response)
                         */
                        block->blktype = S_BLOCK;
                        block->S_respbit = RESPONSE;
                        block->S_ctl = WTX;
                        block->S_inflen = 1;
                        block->S_rqstr  =  &rpstr[3];
                        return (CONTINUE);
                        break;



                        /*
                         * Currently, other control functions are not
                         * supported
                         */

                default:        /* This is invalid rpstr */
#ifdef T1TRACE
                        fprintf(tp1_trfp, "+++++ ERROR : SYNTAX_ERR (wrong S_BLOCK received)\n");
#endif
                        return (-1);
                        break;
                }
                break;

/*****************************************************************************/
/*
 *      I_BLOCK received
 */
        case 1:
        case I_BLOCK:

                /* modification: Version 1.1     */
                if (block->ms == T_MORE)

                        /*
                         * Chaining is only possible in one direction at a
                         * time
                         */
                {
#ifdef T1TRACE
                        fprintf(tp1_trfp, "+++++ ERROR : SYNTAX_ERR (wrong I_BLOCK received)\n");
#endif

                        return (-1);
                }
                New_rsv = getbits(Pcb, 6, 1);
                Mr = getbits(Pcb, 5, 1);

                k = New_rsv - *rsv;
#ifdef T1TRACE

                fprintf(tp1_trfp, "===== New_rsv = %x and rsv = %x\n",New_rsv,
                        *rsv);
#endif

                inf_len = (int) (rpstr[2] & 0xFF);
                if ((k == 0) || (inf_len == 0))

                        /*
                         * It is correct that sending sequence number of
                         * block received is diffrent from that received last
                         * otherwise an error is occured
                         * or
                         * if the length of the INF-field is 0 an error
                         * is occured
                         */
                {
#ifdef T1TRACE
                        fprintf(tp1_trfp, "+++++ ERROR : SYNTAX_ERR (I_BLOCK with wrong send sequence number or inf_len = 0 received)\n");
#endif
                        return (-1);
                }
                /*
                 * To keep new receiving state variable
                 */
                *rsv = New_rsv;

                /*
                 * To saveing data of I_BLOCK
                 */
                for (i = 0; i < inf_len; i++) {
                        outdata[*outlen + i] = rpstr[3 + i];
                }

                *outlen += inf_len;



                if (Mr)
                        /*
                         * SCT sends data with chaining function
                         */
                {
                        block->blktype = R_BLOCK;
                        /*
                         * To expect next chained block from SCT with sending
                         * sequence number ns equal to new nr
                         */
                        block->nr = (!NOT(*rsv)) ? 0 : NR_1;

                        return (CONTINUE);
                } else {

                        /*
                         * The complete APDU response has been received. It
                         * is ready to send next APDU request for PC
                         */
                        *ssv = (!NOT(*ssv)) ? 0 : NS_1;

                        return (0);
                }
                break;

/*****************************************************************************/
/*
 *      wrong block received
 */
        default:
                /* new: Version 1.1      */
#ifdef T1TRACE
                fprintf(tp1_trfp, "+++++ ERROR : SYNTAX_ERR (wrong block received)\n");
#endif
                return (-1);
                break;


        }
}



/*-------------------------------------------------------------*/
/*                                                       | GMD */
/*                                                       +-----*/
/*    Procedure  Resynch         VERSION 1.0                   */
/*                                  MODIFICATION Version 1.1   */
/*                                               January 1993  */
/*                                  DATE                       */
/*                                  BY           Zhou, Eckstein*/
/*                                                             */
/*                                                             */
/*    DESCRIPTION           To resynchronize protocol by       */
/*                          sending S(RESYNCH,request)         */
/*                                                             */
/*    IN                                                       */
/*      rep_no              numer of repetitions               */
/*                                                             */
/*    INOUT                                                    */
/*      portpar             SCT parameters                     */
/*                                                             */
/*    OUT                                                      */
/*                                                             */
/*    RETURN                                                   */
/*       0                  Protocol is resynchronized         */
/*                          tp1_err = PROT_RESYNCH             */
/*                            Protocol is resynchronized       */
/*                            S(RESYNCH,response) was received */
/*                            The last I_Block was not correctly*/
/*                            processed                        */
/*                            The calling program should try it*/
/*                            again                            */
/*                                                             */
/*      -1                  Error                              */
/*                          SCT_RESET                          */
/*                                                             */
/*-------------------------------------------------------------*/
static
int
Resynch(portpar, rep_no)
        struct s_portparam *portpar;
        int             rep_no; /* new; Version 1.1      */
{

        int             repeat;
        int             ret;
        char            rpstr[255];
        BLOCKstru       block;
        char            t1_errno;


/*****************************************************************************/
/*
 *      To send S(RESYNCH,request)
 */
        block.blktype = S_BLOCK;
        block.S_respbit = REQUEST;
        block.S_inflen = 0;
        block.S_ctl = RESYCH;

        repeat = 0;
        do {
                if (repeat == rep_no) {
#ifdef T1TRACE
                        fprintf(tp1_trfp, "\nSCT should be reset physically!\n");
#endif
                        State_reset(&portpar->ns, &portpar->rsv);
                        tp1_err = SCT_RESET;
                        return (-1);
                }

                if ((ret = COMsend(portpar, &block, &t1_errno)) == -1)
                        return (-1);
                ret = COMrece(portpar->port_id, rpstr,
                           portpar->bwt, portpar->cwt, portpar->edc, &t1_errno);

                if (ret == 0)
                        ret = Is_ResynchResp(rpstr);

                repeat++;


        }
        while (ret == -1);
#ifdef T1TRACE
        fprintf(tp1_trfp, "\nProtocol is resynchronized\n");
#endif
        State_reset(&portpar->ns, &portpar->rsv);
        tp1_err = PROT_RESYNCH;
        return (0);
}



/*-------------------------------------------------------------*/
/*                                                       | GMD */
/*                                                       +-----*/
/*    Procedure  divi_blk         VERSION 1.0                  */
/*                                  MODIFICATION Version 1.1   */
/*                                               January 1993  */
/*                                  DATE                       */
/*                                  BY           Zhou, Eckstein*/
/*                                                             */
/*                                                             */
/*    DESCRIPTION       To divide a langer string into small   */
/*                      strings                                */
/*                                                             */
/*    IN                                                       */
/*      ifsd            Max. length of each divided string     */
/*      rqstr           String to be divided                   */
/*      len             Length of the string                   */
/*                                                             */
/*    INOUT                                                    */
/*                                                             */
/*    OUT                                                      */
/*      chaindata       Data for chainging, CHAINstru is       */
/*                      defined in protocol.h                  */
/*    USES                                                     */
/*      free_chain      release allocated data in case of an   */
/*                      error                                  */
/*      malloc macro in MF_check.h                             */
/*    RETURN                                                   */
/*      sequence        The amount of strings                  */
/*      -1              Memory alloc error                     */
/*                      MEMO_ERR                               */
/*-------------------------------------------------------------*/
static
int
divi_blk(ifsd, rqstr, len, chaindata)
        int             ifsd;
        char           *rqstr;
        int             len;
        CHAINstru      *chaindata;
{
        int             i;
        int             length;
        int             index = 0;
        int             sequence = 0;
        char           *tmp, *p;
        char           *proc="divi_blk";

        length = len;



        do {
                if ((tmp = malloc(ifsd)) == NULL) {
                        tp1_err = MEMO_ERR;
                        /*
                         * release allocated buffer for chain data
                         * new; Version 1.1
                         */
                        if (sequence > 0) {
                                free_chain(chaindata);
                        }
#ifdef T1TRACE
                        fprintf(tp1_trfp, "+++++ ERROR : MEMO_ERR\n");
#endif
                        return (-1);
                }
                p = tmp;
                for (i = 0; i < ifsd; i++)
                        *tmp++ = *(rqstr + i + index);
                chaindata->sub_rqstr[sequence] = p;
                chaindata->sublen[sequence] = ifsd;
                sequence++;
                index += ifsd;
                length = length - ifsd;
        } while (length > ifsd);

        if ((tmp = malloc(length)) == NULL) {
                tp1_err = MEMO_ERR;
                /*
                 * release allocated buffer for chain data
                 * new; Version 1.1
                 */
                free_chain(chaindata);
#ifdef T1TRACE
                fprintf(tp1_trfp, "+++++ ERROR : MEMO_ERR\n");
#endif
                return (-1);
        }
        p = tmp;
        for (i = 0; i < length; i++)
                *tmp++ = *(rqstr + i + index);
        chaindata->sub_rqstr[sequence] = p;
        chaindata->sublen[sequence] = length;
        sequence++;
        chaindata->sequence = 0;
        return (sequence);
}



/*-------------------------------------------------------------*/
/*                                                       | GMD */
/*                                                       +-----*/
/*    Procedure  Is_ResynchResp         VERSION 1.0            */
/*                                         DATE                */
/*                                           BY Zhou           */
/*                                                             */
/*                                                             */
/*    DESCRIPTION       To check whether the block is a        */
/*                      S(RESYNCH,response)                    */
/*                                                             */
/*    IN                                                       */
/*      rpstr           String to be check                     */
/*                                                             */
/*    INOUT                                                    */
/*                                                             */
/*    OUT                                                      */
/*                                                             */
/*    RETURN                                                   */
/*       0              Positive                               */
/*      -1              Negative                               */
/*-------------------------------------------------------------*/
static
int
Is_ResynchResp(rpstr)
        char            rpstr[];
{
        if (getbits(rpstr[1], 7, 2) != S_BLOCK)
                return (-1);
        if (getbits(rpstr[1], 2, 3) != RESYCH)
                return (-1);
        if (getbits(rpstr[1], 5, 1) != 1)
                return (-1);
        return (0);
}



/*-------------------------------------------------------------*/
/*                                                       | GMD */
/*                                                       +-----*/
/*    Procedure  free_chain       VERSION 1.1                  */
/*                                  DATE  January 1993         */
/*                                  BY    Eckstein             */
/*                                                             */
/*                                                             */
/*    DESCRIPTION       release chaindata buffer               */
/*                      allocated in divi_blk                  */
/*                                                             */
/*    IN                                                       */
/*      chaindata       Data for chainging, CHAINstru is       */
/*                      defined in protocol.h                  */
/*                                                             */
/*    INOUT                                                    */
/*                                                             */
/*    OUT                                                      */
/*                                                             */
/*    RETURN                                                   */
/*                                                             */
/*    USES                                                     */
/*     free macro in MF_check.h                                */
/*-------------------------------------------------------------*/
static
void
free_chain(chaindata)
        CHAINstru      *chaindata;
{
        int             i;
        char            *proc="free_chain";

        for (i = 0; i < 10; i++) {
                if (chaindata->sub_rqstr[i] != NULL) {
                        free(chaindata->sub_rqstr[i]);
                        chaindata->sub_rqstr[i] = NULL;
                }
        }
}
static
int
sta_itoa(s,n)                        /* from integer to character string */
char s[];
int n;
{
    int c,i,j, sign;

    if((sign = n) < 0)
      n = -n;
    i = 0;
    do {                         /* generation from right to left */
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






#ifdef T1TRACE
/*--------------------------------------------------------*/
/*                                                  | GMD */
/*                                                  +-----*/
/* PROC  xdmp_tpdu           VERSION   1.1                */
/*                              DATE   January 1993       */
/*                                BY   L.Eckstein,GMD     */
/*                                                        */
/* DESCRIPTION                                            */
/*  Print TPDU       in TRACE-File                        */
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
/*  flag                      0 = request                 */
/*                            1 = response                */
/*                                                        */
/*  edclen                    1 = LRC                     */
/*                            2 = CRC                     */
/*--------------------------------------------------------*/
static
void
xdmp_tpdu(dump_file, buffer, len, flag, edclen)
        FILE           *dump_file;
        char           *buffer;
        int             len;
        int             flag;
        int             edclen;
{
        int             pcb;
        int             blk_type;
        int             s_block;

        if (flag == 0)
                fprintf(dump_file, "\nTPDU_request: ");
        else
                fprintf(dump_file, "TPDU_response: ");

        pcb = *(buffer + 1);
        blk_type = getbits(pcb, 7, 2);
        s_block = getbits(pcb, 2, 3);

        switch (blk_type) {
        case 1:
        case I_BLOCK:
                fprintf(dump_file, "    I_BLOCK\n");
                break;
        case R_BLOCK:
                fprintf(dump_file, "    R_BLOCK\n");
                break;
        case S_BLOCK:
                switch (s_block) {
                case RESYCH:
                        fprintf(dump_file, "    S(RESYNCH, ...)_BLOCK\n");
                        break;
                case WTX:
                        fprintf(dump_file, "    S(WTX, ...)_BLOCK\n");
                        break;
                case IFS:
                        fprintf(dump_file, "    S(IFS, ...)_BLOCK\n");
                        break;
                case ABORT:
                        fprintf(dump_file, "    S(ABORT, ...)_BLOCK\n");
                        break;
                default:
                        fprintf(dump_file, "    S_BLOCK (not supported)\n");
                        break;
                }

                break;
        default:
                fprintf(dump_file, "    BLOCK unknown\n");
                break;
        }
/*   aux_fxdump(stdout,buffer,len,0);  */
        fprintf(dump_file, "    NAD(0x%02x) PCB(0x%02x) LEN(0x%02x)\n",
                *buffer & 0xFF,
                *(buffer + 1) & 0xFF,
                *(buffer + 2) & 0xFF);

        fprintf(dump_file, "    INF:\n");
        aux_fxdump(dump_file, buffer + 3, *(buffer + 2) & 0xFF, 0);

        fprintf(dump_file, "    EDC (LEN = %d):\n", edclen);
        aux_fxdump(dump_file, buffer + 3 + (*(buffer + 2) & 0xFF), edclen, 0);


}

#endif
