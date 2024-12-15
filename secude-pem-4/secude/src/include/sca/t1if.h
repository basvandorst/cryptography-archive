/*
 *  STARMOD Release 1.1 (GMD)
 */
/*-------------------------------------------------------+-----*/
/*							 | GMD */
/*   SYSTEM   STARMOD  -  Version 1.1			 +-----*/
/*							       */
/*-------------------------------------------------------------*/
/*							       */
/*    PACKAGE	T1                VERSION 1.0	               */
/*				   DATE         November 1991  */
/*			           MODIFICATION Version 1.1    */
/*						January 1993   */
/*			           BY           Levona Eckstein*/
/*							       */
/*    FILENAME			                 	       */
/*      t1if.h   		         		       */
/*							       */
/*    DESCRIPTION					       */
/*      This file contains all local define's and structures   */
/*	for the t1-programm	         		       */
/*      It is only used by T1.c				       */
/*-------------------------------------------------------------*/

#define    REQUEST      0xc0      /* For S_block */
#define    RESPONSE     0xe0      /* For S_block */
#define    RESYCH       0x00      /* For S_block */
#define    IFS          0x01
#define    ABORT        0x02
#define    WTX          0x03

#define    CHKERR       0x01
#define    FMATERR      0x02

#define    NS_0         0x00      /* Sending sequence number */
#define    NS_1         0x40

#define    T_MORE       0x20      /* More bit */
#define    NMORE        0x00

#define    NR_0         0x00      /* Receiving sequence number */
#define    NR_1         0x10

#define    BLKREPEAT    3
#define    LRC_LEN      1
#define    CRC_LEN      2

#define    I_BLOCK      0
#define    R_BLOCK      2
#define    S_BLOCK      3

#define    NEXTBLOCK     -3
#define    ERROR         -1

#define    CONTINUE      -2

#define    BLKHDLEN    3          /* Node addr byte + PCB + Length byte */
#define    BLKLPOS     2

#define    NOT(x)       ( x == 0 ) ? 1 : 0
#define    BLOCK_WAIT     1
#define    CHAR_WAIT      2

/* returncodes codes */
#define    TP1_OK           0
#define    DEV_BUSY      0x19	  /* port locked		        */
#define    RD_ERR        0x20
#define    WR_ERR        0x21
#define    EDC_ERR       0x22     /* To indicate invalid block received */
#define    MEMO_ERR      0x23     /* Memory alloc error  */
#define    OPEN_ERR      0x24     /* Faile for opening file or initiate port*/
#define    NO_PORT       0x25     /* no port open */
#define    BLK_TIMEOUT   0x26     /* Faile for receiving a block from SCT
                                     within the block waiting time */
#define    CHAR_TIMEOUT  0x27     /* Character is not come within
                                     character waiting time        */
#define    INVALID_LEN   0x28     /* Length of data field of a block is
                                     incompatiable with that allowed by PC */
#define    INVAL_TPDULEN 0x29     /* Length of TPDU-request is too long */
#define    INVALID_PORT  0x30     /* Not available port */
#define    SELECT_ERR    0x31     /* Error from system call    */
#define    PROT_RESYNCH  0x32     /* Protocol has been resynchronized. and
                                     communication can be started again with
                                     new protocol parameter state.        */
#define    SCT_RESET     0x33     /* Smart card terminal should be reset
                                     physically due to errors unrecoverable */

#define    SYNTAX_ERR    0x34



typedef struct {
        int ms;                   /* More bit */
        int nr;                   /* Receiving sequence number */
        int blktype;              /* Type of block */
        int S_respbit;            /* Response bit of S_blocks */
        int S_ctl;                /* Control function of S_blocks */
        int I_inflen;             /* Length of data field of I_block */
        int S_inflen;             /* Length of data field of S_block */
        char *I_rqstr;            /* Data field of I_block */
        char *S_rqstr;            /* Data field of S_block; Version 1.1*/
        } BLOCKstru;

typedef struct {
        char *sub_rqstr[10];      /* Point to each chained block */
        int  sublen[10];          /* Length of each chained block */
        int  sequence;            /* Order of chained blocks */
        int  amount;              /* Amount of chained blocks */
        } CHAINstru;
