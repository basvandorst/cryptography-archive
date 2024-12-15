head	 1.3;
branch   ;
access   ;
symbols  V80:1.1;
locks    ; strict;
comment  @ * @;


1.3
date	 94.08.25.00.53.42;  author rushing;  state Exp;
branches ;
next	 1.2;

1.2
date	 93.12.08.01.29.07;  author rushing;  state Exp;
branches ;
next	 1.1;

1.1
date	 93.03.30.21.09.07;  author rushing;  state Exp;
branches ;
next	 ;


desc
@MAPI merge
@


1.3
log
@mapi32.dll for win32
@
text
@
/*
 * $Log: wvmapi.h $
 * Revision 1.2  1993/12/08  01:29:07  rushing
 * new version box and cr lf consistency
 *
 * $Id: wvmapi.h 1.2 1993/12/08 01:29:07 rushing Exp rushing $
 * Revision 1.1  1993/03/30  21:09:07  rushing
 * Initial revision
 *
 *
 */

#define SZ_MAPILOGON "MAPILogon"
#define SZ_MAPILOGOFF "MAPILogoff"
#define SZ_MAPIADDRESS "MAPIAddress"
#define SZ_MAPISENDMAIL "MAPISendMail"
#define SZ_MAPIFREEBUFFER "MAPIFreeBuffer"

/* following functions not yet required in winvn
#define SZ_MAPISENDDOC "MAPISendDocuments"
#define SZ_MAPIFINDNEXT "MAPIFindNext"
#define SZ_MAPIREADMAIL "MAPIReadMail"
#define SZ_MAPISAVEMAIL "MAPISaveMail"
#define SZ_MAPIDELMAIL "MAPIDeleteMail"
#define SZ_MAPIDETAILS "MAPIDetails"
#define SZ_MAPIRESOLVENAME "MAPIResolveName"
*/

/* This isn't necessarily correct, because MAPI32 isn't
 * available with win32s.  <sigh>
 */

#ifdef WIN32
#define MAPIDLL "MAPI32.DLL"
#else
#define MAPIDLL "MAPI.DLL"
#endif

#define ERR_LOAD_LIB  0x02
#define ERR_LOAD_FUNC 0x04

/*  not sure about this
    #ifdef MAIN */ 

ULONG (FAR PASCAL *lpfnMAPILogon)(ULONG, LPSTR, LPSTR, FLAGS, ULONG, LPLHANDLE);

ULONG (FAR PASCAL *lpfnMAPILogoff)(LHANDLE, ULONG, FLAGS,ULONG);

ULONG (FAR PASCAL *lpfnMAPISendMail)(LHANDLE, ULONG, lpMapiMessage, FLAGS,
                                     ULONG);

ULONG (FAR PASCAL *lpfnMAPISendDocuments)(ULONG, LPSTR, LPSTR, LPSTR, ULONG);

ULONG (FAR PASCAL *lpfnMAPIFindNext)(LHANDLE, ULONG, LPSTR, LPSTR, FLAGS,
                                     ULONG, LPSTR);

ULONG (FAR PASCAL *lpfnMAPIReadMail)(LHANDLE, ULONG, LPSTR, FLAGS, ULONG,
                                     lpMapiMessage FAR *);

ULONG (FAR PASCAL *lpfnMAPISaveMail)(LHANDLE, ULONG, lpMapiMessage, FLAGS,
                                     ULONG, LPSTR);

ULONG (FAR PASCAL *lpfnMAPIDeleteMail)(LHANDLE, ULONG, LPSTR, FLAGS, ULONG);

ULONG (FAR PASCAL *lpfnMAPIFreeBuffer)(LPVOID);

ULONG (FAR PASCAL *lpfnMAPIAddress)(LHANDLE, ULONG, LPSTR, ULONG, LPSTR,
				    ULONG, lpMapiRecipDesc, FLAGS, ULONG,
                                    LPULONG, lpMapiRecipDesc FAR *);

ULONG (FAR PASCAL *lpfnMAPIDetails)(LHANDLE, ULONG,lpMapiRecipDesc, FLAGS,
                                    ULONG);

ULONG (FAR PASCAL *lpfnMAPIResolveName)(LHANDLE, ULONG, LPSTR, FLAGS,
						                ULONG, lpMapiRecipDesc FAR *);
/* likewise here 

#else
extern ULONG (FAR PASCAL *lpfnMAPILogon)(ULONG, LPSTR, LPSTR, FLAGS, ULONG,
                                         LPLHANDLE);

extern ULONG (FAR PASCAL *lpfnMAPILogoff)(LHANDLE, ULONG, FLAGS,ULONG);

extern ULONG (FAR PASCAL *lpfnMAPISendMail)(LHANDLE, ULONG, lpMapiMessage,
                                            FLAGS, ULONG);

extern ULONG (FAR PASCAL *lpfnMAPISendDocuments)(ULONG, LPSTR, LPSTR, LPSTR,
                                                 ULONG);

extern ULONG (FAR PASCAL *lpfnMAPIFindNext)(LHANDLE, ULONG, LPSTR, LPSTR,
                                            FLAGS, ULONG, LPSTR);

extern ULONG (FAR PASCAL *lpfnMAPIReadMail)(LHANDLE, ULONG, LPSTR, FLAGS,
                                            ULONG, lpMapiMessage FAR *);

extern ULONG (FAR PASCAL *lpfnMAPISaveMail)(LHANDLE, ULONG, lpMapiMessage,
                                            FLAGS, ULONG, LPSTR);

extern ULONG (FAR PASCAL *lpfnMAPIDeleteMail)(LHANDLE, ULONG, LPSTR, FLAGS,
                                              ULONG);

extern ULONG (FAR PASCAL *lpfnMAPIFreeBuffer)(LPVOID);

extern ULONG (FAR PASCAL *lpfnMAPIAddress)(LHANDLE, ULONG, LPSTR, ULONG,
                                           LPSTR, ULONG, lpMapiRecipDesc,
                                           FLAGS, ULONG, LPULONG,
                                           lpMapiRecipDesc FAR *);

extern ULONG (FAR PASCAL *lpfnMAPIDetails)(LHANDLE, ULONG,lpMapiRecipDesc,
                                           FLAGS, ULONG);

extern ULONG (FAR PASCAL *lpfnMAPIResolveName)(LHANDLE, ULONG, LPSTR,
                                               FLAGS, ULONG,
                                               lpMapiRecipDesc FAR *);
#endif

*/ 

int FAR PASCAL InitMAPI(void);
int FAR PASCAL DeInitMAPI(void);
@


1.2
log
@new version box and cr lf consistency
@
text
@d4 4
a7 1
 * $Id: wvmapi.h 1.1 1993/03/30 21:09:07 rushing Exp rushing $
d30 7
d38 1
@



1.1
log
@Initial revision
@
text
@d3 4
a6 2
 * $Id$
 * $Log$
d8 1
d110 1
a110 1
int FAR PASCAL DeInitMAPI(void);@

