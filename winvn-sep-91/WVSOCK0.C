/*--  First line of WVSock.c ------------------------------------ */
#define _NEAR
#define _CDECL
#undef LINT_ARGS

#include "windows.h"
#include "wvglob.h"
#include "winvn.h"
#ifndef MAC
#include <pctcp/types.h>
#include <pctcp/pctcp.h>
#include <pctcp/sockets.h>
#include <pctcp/options.h>
#include <pctcp/error.h>
#else
#include <SerialDvr.h>
#include <MacTCPCommonTypes.h>
#include <AddressXlation.h>
#include <TCPPB.h>
#endif
#ifndef MAC
#include <memory.h>
#include <dos.h>

#ifdef NET_FAR
extern int far net_connect(int nd, int type, struct addr far *addr);
extern int far net_read(int nd, char far *buf, unsigned len, struct addr far *from, unsigned flags);
extern int far net_write(int nd, char far *buf, unsigned len, unsigned flags);
extern int far net_release(int nd);
extern long far inet_addr(char far *cp);
#else
long inet_addr(char *cp);
#endif
#endif

union {
   struct {
      unsigned int toff;
      unsigned int tseg;
   } structaddr;
   char far * far * TCPpos;
} TCPaddr;

#ifdef MAC
void ClearParamBlock(char *paramBlock,int  nbytes);
#endif

#ifdef MAC
#define COMMBUFSIZE   1600
#define TCPBUFSIZE   16384
#else
#define COMMBUFSIZE  1600
#endif
#define USE_NET 1
#define SERIALOUTBUFSIZE  1024

#ifdef TSRBUF
char far *CommBuff;
#define INTBUFPTR (0x85*4)
#else
char CommBuff[COMMBUFSIZE];
#endif
int  CommBuffIdx=COMMBUFSIZE+1;
int  CharsInCommBuff=0;
int  CharsInSkLine=70;
#define DEBUGSIZE 200
char DebugKeyBuf[DEBUGSIZE];
int  DebugKeyIdx=0;

#ifndef MAC
struct addr a;
struct addr from;
int NNTPSock;
int DebugSock = 0;
char mymesbuf[80];

union REGS inregs, outregs;
struct SREGS segregs;
#else
StreamPtr NNTPSock;
rdsEntry myRDS[2];
wdsEntry myWDS[2];
BOOL prevTCPError = FALSE;
#endif

#ifdef MAC
int serialIn = AinRefNum;
int serialOut = AoutRefNum;
int whichPort = sPortA;
int serConfig = stop10+evenParity+data7+baud9600;
Handle hSerBuf;
char *lpSerBuf;
Size SerBufSize = COMMBUFSIZE+2;
OSErr myOSErr;

short int SockRefNum;
TCPiopb TCPparamBlock;
TCPiopb TCPReadparamBlock;
#endif

/*-- function SetupSock --------------------------------------------
 *
 *   Setups up BSD-style socket to the NNTP server.
 *
 *   Returns 1 upon success, else 0.
 */
int
SetupSock()
{
   int protocol = 0;
   int nntpport = 119;
   int j;
   int result;
   char mes[100];
   char buf[60];
   long destaddr;
#ifndef MAC
   destaddr = inet_addr(NNTPIPAddr);

   /* Set up name structure */

   memcpy(&a.fhost,&destaddr,sizeof(a.fhost));
   a.lsocket = 0;
   a.fsocket = NNTPPort;
   a.protocol = 0;

   LockData(0);
   if(DebugSock) {
		MessageBox(hWndConf,"Ready to call net_connect","in SetupSock",MB_OK);
   }
#ifdef USE_NET
   NNTPSock = net_connect(-1,STREAM,&a);
#else
   inregs.h.ah = 0x13;  /* net_connect */
   inregs.x.bx = 0xffff;  /* nd = -1 */
   segregs.ds =  HIWORD((char far *)(&a));
   inregs.x.si = LOWORD((char far *)(&a));
   inregs.x.dx = STREAM;

   NNTPSock = int86x(0x61,&inregs,&outregs,&segregs);

#endif
   if(DebugSock || NNTPSock == -1) {
      sprintf(mes,"net_connect returned %d; neterrno=%d",NNTPSock,neterrno);
      MessageBox(hWndConf,mes,"Error from 'net_connect'",MB_OK|MB_ICONHAND);
#if 0
      return(0);
#endif
   }
#endif
   return(1);
}

/*-- function MRRReadComm ---------------------------------------
 *
 *  Reads characters from either a serial line or TCP socket.
 */
int
MRRReadComm()
{
   int bytesread, ch;
   int myerr;
   long int lCount;
   char mesbuf[80];
#ifdef MAC
   static BOOL waiting=TRUE;
   BOOL doread=FALSE;
#endif

   if(CommBuffIdx < CharsInCommBuff) {
   } else if(UsingSocket) {
#ifndef MAC
#ifdef USE_NET
#if 0
      MessageBox(hWndConf,"About to net_read","in MRRReadComm",MB_OK);
#endif
      bytesread = net_read(NNTPSock,CommBuff,COMMBUFSIZE,&from,NET_FLG_NONBLOCKING);
#if 0
      MessageBox(hWndConf,"Just did net_read","in MRRReadComm",MB_OK);
#endif
#else
      inregs.h.ah = 0x1b;
      inregs.x.bx = NNTPSock;
      segregs.ds  = HIWORD((char far *)CommBuff);
      inregs.x.si = LOWORD((char far *)CommBuff);
      inregs.x.cx = COMMBUFSIZE;
      segregs.es =  HIWORD((char far *)(NULL));
      inregs.x.di = LOWORD((char far *)(NULL));
      inregs.x.dx = NET_FLG_NONBLOCKING;

#if 1
      myerr = int86x(0x61,&inregs,&outregs,&segregs);
#else
      outregs.x.cflag = 1;
      myerr = NET_ERR_WOULD_BLOCK;
#endif
      if(outregs.x.cflag) {
         bytesread = -1;
      } else {
         bytesread = outregs.x.cx;
      }
      neterrno = myerr & 0xff;
#endif
   /* ifdef USE_NET */
      if(!bytesread || (bytesread == (-1) && neterrno==NET_ERR_WOULD_BLOCK)) {
         return(-1);
      } else if(bytesread > 0) {
	 if(DebugSock) {
	    sprintf(mymesbuf,"bytesread=%d '%.30s'",bytesread,CommBuff);
	    MessageBox(hWndConf,mymesbuf,"Read bytes in MRRReadComm",MB_OK);
	 }
         CharsInCommBuff = bytesread;
         CommBuffIdx = 0;
      } else {
         sprintf(mymesbuf,"NNTPSock=%d  bytesread=%d  neterrno=%d",NNTPSock,bytesread,neterrno);
         MessageBox(hWndConf,mymesbuf,"in MRRReadComm, probs",MB_OK|MB_ICONHAND);
         return(-1);
      }
#else
  /* ifndef MAC */
      /* Mac code to read from socket. */

      if(TCPReadparamBlock.ioResult == inProgress) {
         return(-1);
      } else if(TCPReadparamBlock.ioResult == commandTimeout) {
         doread = TRUE;
         TCPReadparamBlock.ioResult = 0;
      } else if(TCPReadparamBlock.ioResult < 0 && !prevTCPError) {
         sprintf(mesbuf,"TCPRcv returned ioResult = %d",TCPReadparamBlock.ioResult);
         MessageBox(hWndConf,mesbuf,"Serious Communication error",MB_OK|MB_ICONHAND);
         prevTCPError = TRUE;
      } else if(waiting) {
         bytesread = TCPReadparamBlock.csParam.receive.rcvBuffLen;
         CharsInCommBuff = bytesread;
         CommBuffIdx = 0;
         waiting = FALSE;
      } else {
         waiting = TRUE;
         doread = TRUE;
      }
      if(doread) {
         TCPReadparamBlock.csCode = TCPRcv;
/*       TCPReadparamBlock.csParam.receive.commandTimeoutValue = 1; */
         TCPReadparamBlock.csParam.receive.rcvBuff = CommBuff;
         TCPReadparamBlock.csParam.receive.rcvBuffLen = COMMBUFSIZE;
            myOSErr = PBControl(&TCPReadparamBlock,TRUE);
         if(myOSErr != noErr &&  myOSErr != commandTimeout) {
            sprintf(mesbuf,"TCPRcv ret %d, bytes=%d mes='%.45s'",myOSErr,bytesread,CommBuff);
            MessageBox(hWndConf,mesbuf,"",MB_OK|MB_ICONHAND);
         }
         return(-1);
      }
#endif
   } else {
#ifndef MAC
      bytesread = ReadComm(CommDevice,CommBuff,COMMBUFSIZE);
#else
      SerGetBuf(serialIn,&lCount);
      FSRead(serialIn,&lCount,CommBuff);
      bytesread = lCount;
#endif
      if(bytesread <= 0) {
         return(-1);
      } else {
         CharsInCommBuff = bytesread;
         CommBuffIdx = 0;
      }
   }
   return(0xff & CommBuff[CommBuffIdx++]);
}

#ifdef MAC

/*-- function write_wds --------------------------------------------------------
 */
write_wds(line,nchars)
char *line;
int nchars;
{
   char mesbuf[80];

      myWDS[0].length = nchars;
      myWDS[0].ptr = line;
      myWDS[1].length = 0;
      myWDS[1].ptr = (Ptr) 0;

      TCPparamBlock.csCode = TCPSend;
      TCPparamBlock.csParam.send.wdsPtr = (Ptr) &myWDS;
      myOSErr = PBControl(&TCPparamBlock,FALSE);
      if(myOSErr != noErr && !prevTCPError) {
         sprintf(mesbuf,"TCPSend returned OSErr = %d.",myOSErr);
         MessageBox(hWndConf,mesbuf,"Serious Communication error",MB_OK|MB_ICONHAND);
         prevTCPError = TRUE;
      }
}
#endif

/*-- function PutCommLine ----------------------------------------------
 *
 *  Output a line--either to a socket or to a serial port.
 *
 *  Entry   line   points to a buffer of characters.
 *          nchars is the number of characters to output.
 */
void
PutCommLine(line,nchars)
char *line;
unsigned int nchars;
{
   int retcode;
#ifndef MAC
   COMSTAT MyStat;
#else
#define MAXSENDLEN 256
   char mybuf[MAXSENDLEN];
   int j, mynchars;
   char *cdest, *csource;
#endif
   char mesbuf[80];
   BOOL Sent = FALSE;
   long int lCount = nchars;

   if(UsingSocket) {
#if 0
      MessageBox(hWndConf,"About to net_write","in PutCommLine",MB_OK);
#endif
#ifndef MAC
#ifdef USE_NET
      net_write(NNTPSock,line,nchars,0);
      net_write(NNTPSock,"\n",1,0);
#else
      inregs.h.ah = 0x1a;
      inregs.x.bx = NNTPSock;
      segregs.ds  = HIWORD((char far *)line);
      inregs.x.si = LOWORD((char far *)line);
      inregs.x.cx = nchars;
      inregs.x.dx = 0;

      retcode = int86x(0x61,&inregs,&outregs,&segregs);

      mymesbuf[0] = '\n';
      segregs.ds  = HIWORD((char far *)mymesbuf);
      inregs.x.si = LOWORD((char far *)mymesbuf);
      inregs.x.cx = 1;

      retcode = int86x(0x61,&inregs,&outregs,&segregs);
#endif
      if(DebugSock) {
	 MessageBox(hWndConf,"Just did net_write","in PutCommLine",MB_OK);
      }
#else
      if(nchars >= MAXSENDLEN) {
         sprintf(mesbuf,"nchars = %d",nchars);
         MessageBox(hWndConf,"Line too long in PutCommLine",mesbuf,MB_OK);
      }
      for(cdest=mybuf, csource=line, mynchars=nchars; mynchars; mynchars--) {
         *(cdest++) = *(csource++);
      }
      *(cdest++) = ' ';
      *(cdest++) = '\n';
      *cdest = '\0';
      write_wds(mybuf,nchars+2);
/*    write_wds(line,nchars);
      write_wds("\n",1);*/
/*    printf("Sent '%s', nchars=%d",mybuf,nchars); */
#endif

   } else {
      while (!Sent) {
#ifndef MAC
         GetCommError(CommDevice,&MyStat);
         if(SERIALOUTBUFSIZE - MyStat.cbOutQue > nchars+1) {
            WriteComm(CommDevice,line,nchars);
            WriteComm(CommDevice,"\r",1);
            Sent = TRUE;
         } else {
            MainLoopPass();
         }
#else
         FSWrite(serialOut,&lCount,line);
         lCount = 1;
         FSWrite(serialOut,&lCount,"\r");
         Sent = TRUE;
#endif
      }
   }
}

/*-- function MRRCloseComm -----------------------------------------
 *
 *  Close the communications port, serial or TCP.
 */
void
MRRCloseComm()
{
   int timeout = 0;
   int retcode;
   char mesbuf[80];

   if(UsingSocket) {
#ifndef MAC
/*    setsockopt(NNTPSock,SOL_SOCKET,SO_LINGER,(char *)&timeout,2); */
      PutCommLine("QUIT",4);
 /*   close(NNTPSock); */
#ifdef USE_NET
      net_release(NNTPSock);
#else
      inregs.h.ah = 0x08;
      inregs.x.bx = NNTPSock;

      retcode = int86x(0x61,&inregs,&outregs,&segregs);
#endif
#else
      /* Close MacTCP socket */
      TCPparamBlock.csCode = TCPRelease;
      myOSErr = PBControl(&TCPparamBlock,FALSE);
      if(myOSErr != noErr) {
         sprintf(mesbuf,"TCPRelease returned OSErr = %d.",myOSErr);
         MessageBox(hWndConf,mesbuf,"",MB_OK|MB_ICONHAND);
      }
#endif
   } else {
#ifndef MAC
      CloseComm(CommDevice);
#else
      RAMSDClose(whichPort);
#endif
   }
}

#ifdef MAC
/*--- function ClearParamBlock -------------------------------------------
 *
 *    Zero out a parameter block for the PB (parameter block) routines on the Mac.
 */
void
ClearParamBlock(paramBlock,nbytes)
char *paramBlock;
int  nbytes;
{
   int j=0;

   for(;j<nbytes;j++) paramBlock[j] = 0;
}
#endif

/*--- function MRRInitComm --------------------------------------------
 *
 *  Initialize communications--including figuring out whether
 *  to use TCP or serial.
 */
int
MRRInitComm()
{
   char mesbuf[80],mesbuf2[80],buf[80];
   char szComName[10];
   char *errptr;
   int retcode, j;
#ifdef MAC
   long msuinfo = 0x23080214L;
   long ebfs2 =   0x23080241L;
   cntrlParam paramBlock;
#endif

#ifdef TSRBUF
   TCPaddr.structaddr.toff = INTBUFPTR;
   TCPaddr.structaddr.tseg = 0;
   CommBuff = *(TCPaddr.TCPpos);
#endif

#ifndef MAC
#else
   UsingSocket = TRUE;
#endif

   if(UsingSocket) {
      IgnoreCommCh = '\r';
      EOLCommCh = '\n';
      retcode = SetupSock();
      if(!retcode) {
         MessageBox(hWndConf,"Can't set up socket","Initialization error",MB_OK|MB_ICONHAND);
      }
#ifdef MAC
      /* Open the driver. */
      ClearParamBlock(&paramBlock,sizeof(paramBlock));
      paramBlock.ioNamePtr = (StringPtr) "\p.IPP";
/*    paramBlock.ioPermssn = fsRdWrShPerm; */
      myOSErr = PBOpen(&paramBlock,FALSE);
      if(myOSErr != noErr) {
         sprintf(mesbuf,"PBOpen returned OSErr = %d.",myOSErr);
         MessageBox(hWndConf,mesbuf,"",MB_OK|MB_ICONHAND);
         prevTCPError = TRUE;
      }
      SockRefNum = paramBlock.ioRefNum;

      /* Create the socket. */
      ClearParamBlock(&TCPparamBlock,sizeof(TCPparamBlock));
      TCPparamBlock.ioCRefNum = SockRefNum;
      TCPparamBlock.csCode = TCPCreate;
      SerBufSize = TCPBUFSIZE;
      hSerBuf = NewHandle(SerBufSize);
      HLock(hSerBuf);
      lpSerBuf = *hSerBuf;
      TCPparamBlock.csParam.create.rcvBuff = (Ptr) lpSerBuf;
      TCPparamBlock.csParam.create.rcvBuffLen = TCPBUFSIZE;
      myOSErr = PBControl(&TCPparamBlock,FALSE);

      if(myOSErr != noErr) {
         sprintf(mesbuf,"TCPCreate returned OSErr = %d.",myOSErr);
         MessageBox(hWndConf,mesbuf,"",MB_OK|MB_ICONHAND);
      }
      NNTPSock = TCPparamBlock.tcpStream;

      /* Open the connection to the server. */

      TCPparamBlock.csCode = TCPActiveOpen;
      TCPparamBlock.csParam.open.remoteHost = (ip_addr) msuinfo; /* msuinfo */
      TCPparamBlock.csParam.open.remotePort = 119 /* 1041 */ ;
      myOSErr = PBControl(&TCPparamBlock,FALSE);
      if(myOSErr != noErr) {
         sprintf(mesbuf,"TCPActiveOpen returned OSErr = %d.",myOSErr);
         MessageBox(hWndConf,mesbuf,"",MB_OK|MB_ICONHAND);
      }

      /* Start the first read on the socket. */

      TCPReadparamBlock = TCPparamBlock;

      TCPReadparamBlock.csCode = TCPRcv;
/*    TCPReadparamBlock.csParam.receive.commandTimeoutValue = 1; */
      TCPReadparamBlock.csParam.receive.rcvBuff = CommBuff;
      TCPReadparamBlock.csParam.receive.rcvBuffLen = COMMBUFSIZE;
      myOSErr = PBControl(&TCPReadparamBlock,TRUE);
      if(myOSErr != noErr &&  myOSErr != commandTimeout) {
         sprintf(mesbuf,"TCPRcv ret %d",myOSErr);
         MessageBox(hWndConf,mesbuf,"",MB_OK|MB_ICONHAND);
      }
#endif
   } else {
      IgnoreCommCh = '\r';
      EOLCommCh = '\n';      /* Used to be reversed */

      /* Set up Comm parameters.                                        */

#ifndef MAC

      /*
       *  Open the Comm port.  I use a hard-coded communications rate
       *  and other parameters; obviously, this should be improved
       *  with a config file.
       */

      strncpy(szComName,szCommString,4);
      szComName[4] = '\0';
      CommDevice = OpenComm(szComName,4096,SERIALOUTBUFSIZE);
      if(CommDevice < 0) {
         sprintf(mesbuf,"OpenComm returned %d",CommDevice);
         strcpy(mesbuf2,"Error opening '");
         strcat(mesbuf2,szComName);
         strcat(mesbuf2,"'");
         MessageBox(hWndConf,mesbuf,mesbuf2,MB_OK|MB_ICONEXCLAMATION);
      } else {
          WinVnDoComm(szCommString);
      }
#else
      RAMSDOpen(whichPort);
      SerReset(serialIn,serConfig);

      hSerBuf = NewHandle(SerBufSize);
      HLock(hSerBuf);
      lpSerBuf = *hSerBuf;
      SerSetBuf(serialIn,lpSerBuf,(int)SerBufSize);

#endif
   }

   /* Put out an illegal command, just so we can watch the           */
   /* server come back with an "Unrecognized command" message.       */
   /* This ensures that the comm link is up and we're talking to NNTP*/

   CommLinePtr = CommLineIn;
   PutCommLine("Patty",5);
   CommState = ST_ESTABLISH_COMM;
   CommBusy = TRUE;
   return(0);
}
