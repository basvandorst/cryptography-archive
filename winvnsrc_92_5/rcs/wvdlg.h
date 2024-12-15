head	1.26;
access;
symbols
	V80:1.5
	V76d:1.1;
locks; strict;
comment	@ * @;


1.26
date	94.08.24.18.00.29;	author jcooper;	state Exp;
branches;
next	1.25;

1.25
date	94.08.23.23.13.19;	author martin;	state Exp;
branches;
next	1.24;

1.24
date	94.08.11.00.09.17;	author jcooper;	state Exp;
branches;
next	1.23;

1.23
date	94.08.04.05.48.33;	author dumoulin;	state Exp;
branches;
next	1.22;

1.22
date	94.07.25.22.41.22;	author pearse_w_r;	state Exp;
branches;
next	1.21;

1.21
date	94.07.25.18.51.48;	author jcooper;	state Exp;
branches;
next	1.20;

1.20
date	94.06.23.23.06.21;	author dumoulin;	state Exp;
branches;
next	1.19;

1.19
date	94.05.23.18.37.00;	author jcooper;	state Exp;
branches;
next	1.18;

1.18
date	94.05.02.20.50.48;	author jglasser;	state Exp;
branches;
next	1.17;

1.17
date	94.05.02.20.20.38;	author rushing;	state Exp;
branches;
next	1.16;

1.16
date	94.03.01.19.10.19;	author rushing;	state Exp;
branches;
next	1.15;

1.15
date	94.02.24.21.28.18;	author jcoop;	state Exp;
branches;
next	1.14;

1.14
date	94.01.24.17.39.47;	author jcoop;	state Exp;
branches;
next	1.13;

1.13
date	94.01.15.23.53.22;	author jcoop;	state Exp;
branches;
next	1.12;

1.12
date	94.01.12.19.27.55;	author mrr;	state Exp;
branches;
next	1.11;

1.11
date	93.12.08.01.29.07;	author rushing;	state Exp;
branches;
next	1.10;

1.10
date	93.12.07.19.47.23;	author rushing;	state Exp;
branches;
next	1.9;

1.9
date	93.08.25.18.54.36;	author mbretherton;	state Exp;
branches;
next	1.8;

1.8
date	93.07.13.16.03.04;	author riordan;	state Exp;
branches;
next	1.7;

1.7
date	93.06.22.19.09.14;	author rushing;	state Exp;
branches;
next	1.6;

1.6
date	93.06.22.18.14.53;	author rushing;	state Exp;
branches;
next	1.5;

1.5
date	93.05.28.17.18.00;	author rushing;	state Exp;
branches;
next	1.4;

1.4
date	93.05.18.22.10.45;	author rushing;	state Exp;
branches;
next	1.3;

1.3
date	93.05.13.19.58.51;	author SOMEONE;	state Exp;
branches;
next	1.2;

1.2
date	93.05.13.16.17.30;	author rushing;	state Exp;
branches;
next	1.1;

1.1
date	93.02.16.20.54.42;	author rushing;	state Exp;
branches;
next	;


desc
@winvn version 0.76 placed into RCS
@


1.26
log
@misc encoding/decoding changes
@
text
@/*
 *
 * $Id: wvdlg.h 1.25 1994/08/23 23:13:19 martin Exp $
 * $Log: wvdlg.h $
 * Revision 1.25  1994/08/23  23:13:19  martin
 * new group list box
 *
 * Revision 1.24  1994/08/11  00:09:17  jcooper
 * Enhancements to Mime and article encoding/encoding
 *
 * Revision 1.23  1994/08/04  05:48:33  dumoulin
 * Added support for scrollable version box
 *
 * Revision 1.22  1994/07/25  22:41:22  pearse_w_r
 * ShowUnreadOnly option
 *
 * Revision 1.21  1994/07/25  18:51:48  jcooper
 * execution of decoded files
 *
 * Revision 1.20  1994/06/23  23:06:21  dumoulin
 * sorted identifiers by number and added new status fields to print dialog
 *
 * Revision 1.19  1994/05/23  18:37:00  jcooper
 * new attach code, session [dis]connect
 *
 * Revision 1.18  1994/05/02  20:50:48  jglasser
 * oops forgot one
 *
 * Revision 1.17  1994/05/02  20:20:38  rushing
 * changes from jody glasser
 *
 * Revision 1.16  1994/03/01  19:10:19  rushing
 * ThreadFullSubject option added
 *
 * Revision 1.15  1994/02/24  21:28:18  jcoop
 * jcoop changes
 *
 * Revision 1.14  1994/01/24  17:39:47  jcoop
 * 90.2 changes
 *
 * Revision 1.13  1994/01/15  23:53:22  jcoop
 * constants for new jcoop dialogs - attachments, etc
 *
 * Revision 1.12  1994/01/12  19:27:55  mrr
 * mrr mods 4
 * 
 * Revision 1.11  1993/12/08  01:29:07  rushing
 * new version box and cr lf consistency
 *
 * Revision 1.10  1993/12/07  19:47:23  rushing
 * added IDD_SHOWVERSION
 *
 * Revision 1.9  1993/08/25  18:54:36  mbretherton
 * MRB merge, mail & post logging
 *
 * Revision 1.8  1993/07/13  16:03:04  riordan
 * MRR mods
 *
 * Revision 1.7  1993/06/22  19:09:14  rushing
 * mail force type selection via dialog
 *
 * Revision 1.6  1993/06/22  18:14:53  rushing
 * added ID's for MailForceType
 *
 * Revision 1.5  1993/05/28  17:18:00  rushing
 * Added Misc... option to enable threading
 *
 * Revision 1.4  1993/05/18  22:10:45  rushing
 * smtp support
 *
 * Revision 1.3  1993/05/13  19:58:51  SOMEONE
 * fancy 'from' in group window
 *
 * Revision 1.2  1993/05/13  16:17:30  rushing
 * article fetch limit dialog box support
 *
 * Revision 1.1  1993/02/16  20:54:42  rushing
 * Initial revision
 *
 *
 */

#include "version.h" 

#define ID_OK                           1
#define ID_CANCEL                       2 
#define ID_OK_SAVE                      11
#define OK                              108
#define IDD_OK_SAVE                     109
 
#define IDD_COM1                        300
#define IDD_COM2                        301
#define IDD_7EVEN                       302
#define IDD_8NONE                       303
#define IDD_SPEED                       304
#define ID_COMMTCP                      305
#define ID_COMMSERIAL                   306

#define ID_CONFIG_NNTP_SERVER           310
#define ID_CONFIG_NNTP_PORT             311
#define ID_CONFIG_NAME                  320
#define ID_CONFIG_EMAIL                 321
#define ID_CONFIG_ORG                   322
#define ID_CONFIG_SHOWUNSUB             324
#define ID_CONFIG_SHOWREAD              325
#define ID_CONFIG_CONFIRM_BATCH         326
#define ID_CONFIG_CONFIRM_EXIT          327
#define ID_CONFIG_CONFIRM_REPLY_TO      328
#define ID_CONFIG_CONNECT_AT_START      329
#define ID_DOLIST_BASE                  330
#define ID_DOLIST_YES                   331
#define ID_DOLIST_ASK                   332
#define ID_DOLIST_NO                    333
#define ID_CONFIG_THREADS               334
#define ID_BROWSE                       335
#define ID_CONFIG_THREADS_FULL_SUBJECT  336	/* SMR 940301 */
#define ID_CONFIG_GROUP_MULTI_SELECT    337     /* jlg 28-apr-94 */
#define ID_CONFIG_SHOWUNREADONLY        338     /* wrp 05-JUL-94 */

#define ID_CONFIG_NEW_WND_GROUP         340
#define ID_CONFIG_NEW_WND_ARTICLE       341
#define ID_CONFIG_APPEND                342
#define ID_CONFIG_DO_LIST               344
#define ID_CONFIG_ASK_COMM              350
#define ID_CONFIG_ARTICLE_FIXED_FONT    352


#define IDD_FILENAME                    501
#define IDD_APPEND                      502
#define IDD_VERSION_LISTBOX             508     /* JD 8/3/94 */
#define IDD_VERSION_NUMBER              509     /* JD 8/3/94 */
#define IDD_SEARCH_STRING               510
#define IDD_SUBJECT_STRING              511
#define ID_CONFIG_FULLNAMEFROM          512
#define IDD_SUBJECT_PROMPT              513
#define ID_CONFIG_SMTP_SERVER           514
#define IDD_MAIL_SELECT_NONE            515
#define IDD_MAIL_SELECT_MAPI            516
#define IDD_MAIL_SELECT_SMTP            517
#define IDD_MAIL_SELECT_AUTO            518
#define IDD_REPLY_TO                    519

#define ID_CONFIG_AUTH_USERNAME         521
#define ID_CONFIG_AUTH_PASSWORD         522

#define IDD_DEMANDLOGON                 525
#define IDM_SHOW_VERSION                526

#define IDD_MAILLOG                     530
#define IDD_POSTLOG		        531  
#define IDD_MAILNAME                    532
#define IDD_POSTNAME                    533


#define IDD_CODING_TYPE                 550		/* en/decode JSC 9/12/93 */
#define IDD_ATTACH_NOW		        551
#define IDD_ATTACH_NEXT		        552
#define IDD_CONTENT_TYPE	        553
#define IDD_ARTICLE_SPLIT_LENGTH        554
#define IDD_CUSTOM_TABLE                555
#define IDD_SUBJECT_TEMPLATE	        556
#define IDD_REVIEW                      557
#define IDD_NO_REVIEW                   558
#define IDD_MIME_BOUNDARY               559
#define IDD_MAKE_DEFAULT                560
#define IDD_GENERATE_MIME		568
#define IDD_MIME_USAGE_SUGGESTIONS	569

#define IDD_DECODE_PATH                 561
#define IDD_DUMB_DECODE                 562
#define IDD_VERBOSE_STATUS              563
#define IDD_ENABLE_SIG                  564
#define IDD_EXECUTE_DECODED             567

#define IDD_SAVE_NEWSRC                 565
#define IDD_SAVE_CONFIG                 566

#define IDD_DELETE                      570
#define IDD_ADD                         571
#define IDD_MAIL_DEST                   572
#define IDD_MAIL_LIST                   573


#define IDD_PRINT                       600
#define IDD_PRINTDEVICE                 601     /* JD 930205 */
#define IDD_PRINTPORT                   602     /* JD 930205 */
#define IDD_PRINTTITLE                  603     /* JD 930205 */
#define IDD_PRINTSTATUS			604     /* JD 940623 */
#define IDD_PRINTSUBJECT		605	/* JD 940623 */

#define ID_100_ARTS                     650
#define ID_250_ARTS                     651
#define ID_500_ARTS                     652
#define ID_THRESHOLD_ALL                654
#define IDD_ART_THRESHOLD               655
#define IDD_ARTS_TO_GRAB                656
#define ID_THRESHOLD_UNREAD             657

#define IDD_HIERARCHY_LISTBOX           700
#define IDD_SUBSCRIBED_GROUP_LISTBOX    701
#define IDD_UNSUBSCRIBED_GROUP_LISTBOX  702


@


1.25
log
@new group list box
@
text
@d3 1
a3 1
 * $Id: wvdlg.h 1.24 1994/08/11 00:09:17 jcooper Exp $
d5 3
d167 1
@


1.24
log
@Enhancements to Mime and article encoding/encoding
@
text
@d3 1
a3 1
 * $Id: wvdlg.h 1.23 1994/08/04 05:48:33 dumoulin Exp $
d5 3
a126 1
#define IDD_GROUP_LISTBOX               507
d194 5
@


1.23
log
@Added support for scrollable version box
@
text
@d3 1
a3 1
 * $Id: wvdlg.h 1.22 1994/07/25 22:41:22 pearse_w_r Exp dumoulin $
d5 3
d161 1
@


1.22
log
@ShowUnreadOnly option
@
text
@d3 1
a3 1
 * $Id: wvdlg.h 1.21 1994/07/25 18:51:48 jcooper Exp pearse_w_r $
d5 3
d111 2
a112 2
#define ID_CONFIG_NEW_WND_GROUP         340 
#define ID_CONFIG_NEW_WND_ARTICLE       341      
d122 2
a123 2


@


1.21
log
@execution of decoded files
@
text
@d3 1
a3 1
 * $Id: wvdlg.h 1.20 1994/06/23 23:06:21 dumoulin Exp $
d5 3
d106 1
@


1.20
log
@sorted identifiers by number and added new status fields to print dialog
@
text
@d3 1
a3 1
 * $Id: wvdlg.h 1.19 1994/05/23 18:37:00 jcooper Exp dumoulin $
d5 3
d156 1
@


1.19
log
@new attach code, session [dis]connect
@
text
@d3 1
a3 1
 * $Id: wvdlg.h 1.18 1994/05/02 20:50:48 jglasser Exp $
d5 3
d65 1
a65 1
#include "version.h"
d67 31
a97 58
#define ID_CANCEL                      2
#define ID_COMMSERIAL                  306
#define ID_COMMTCP                     305
#define ID_CONFIG_APPEND               342
#define ID_CONFIG_ARTICLE_FIXED_FONT   352
#define ID_CONFIG_ASK_COMM             350
#define ID_CONFIG_DO_LIST              344
#define ID_CONFIG_EMAIL                321
#define ID_CONFIG_NAME                 320
#define ID_CONFIG_NEW_WND_ARTICLE      341
#define ID_CONFIG_NEW_WND_GROUP        340
#define ID_CONFIG_NNTP_PORT            311
#define ID_CONFIG_NNTP_SERVER          310
#define ID_CONFIG_ORG                  322
#define ID_CONFIG_SHOWUNSUB            324
#define ID_CONFIG_SHOWREAD             325
#define ID_CONFIG_CONFIRM_BATCH        326
#define ID_CONFIG_CONFIRM_EXIT         327
#define ID_CONFIG_CONFIRM_REPLY_TO     328
#define ID_CONFIG_CONNECT_AT_START     329

#define ID_DOLIST_ASK                  332
#define ID_DOLIST_BASE                 330
#define ID_DOLIST_NO                   333
#define ID_DOLIST_YES                  331
#define ID_OK                          1
#define ID_OK_SAVE                     11
#define ID_BROWSE                      335
#define IDD_7EVEN                      302
#define IDD_8NONE                      303
#define IDD_APPEND                     502
#define IDD_COM1                       300
#define IDD_COM2                       301
#define IDD_FILENAME                   501
#define IDD_GROUP_LISTBOX              507
#define IDD_OK_SAVE                    109
#define IDD_SEARCH_STRING              510
#define IDD_SPEED                      304

#define IDD_PRINT                      600
#define IDD_PRINTDEVICE                601     /* JD 930205 */
#define IDD_PRINTPORT                  602     /* JD 930205 */
#define IDD_PRINTTITLE                 603     /* JD 930205 */

#define IDD_SUBJECT_STRING          511
#define IDD_REPLY_TO                519
#define IDD_ART_THRESHOLD           655
#define IDD_ARTS_TO_GRAB            656
#define ID_THRESHOLD_ALL            654
#define ID_THRESHOLD_UNREAD         657
#define ID_100_ARTS                 650
#define ID_250_ARTS                 651
#define ID_500_ARTS                 652
#define OK                          108
#define ID_CONFIG_FULLNAMEFROM      512
#define IDD_SUBJECT_PROMPT          513
#define ID_CONFIG_SMTP_SERVER       514
#define ID_CONFIG_THREADS           334
d99 79
a177 41
#define ID_CONFIG_GROUP_MULTI_SELECT    337  /* jlg 28-apr-94 */
#define IDD_MAIL_SELECT_NONE        515
#define IDD_MAIL_SELECT_MAPI        516
#define IDD_MAIL_SELECT_SMTP        517
#define IDD_MAIL_SELECT_AUTO        518

#define IDD_DEMANDLOGON             525
#define IDM_SHOW_VERSION            526

#define IDD_MAILLOG                 530
#define IDD_POSTLOG         531  
#define IDD_MAILNAME                532
#define IDD_POSTNAME                533

#define ID_CONFIG_AUTH_USERNAME     521
#define ID_CONFIG_AUTH_PASSWORD     522

#define IDD_CODING_TYPE             550		/* en/decode JSC 9/12/93 */
#define IDD_ATTACH_NOW		    551
#define IDD_ATTACH_NEXT		    552
#define IDD_CONTENT_TYPE	    553
#define IDD_ARTICLE_SPLIT_LENGTH    554
#define IDD_CUSTOM_TABLE            555
#define IDD_SUBJECT_TEMPLATE	    556
#define IDD_REVIEW                  557
#define IDD_NO_REVIEW               558
#define IDD_MIME_BOUNDARY           559
#define IDD_MAKE_DEFAULT            560

#define IDD_DECODE_PATH             561
#define IDD_DUMB_DECODE             562
#define IDD_VERBOSE_STATUS          563
#define IDD_ENABLE_SIG              564

#define IDD_SAVE_NEWSRC             565
#define IDD_SAVE_CONFIG             566

#define IDD_DELETE                  570
#define IDD_ADD                     571
#define IDD_MAIL_DEST               572
#define IDD_MAIL_LIST               573
@


1.18
log
@oops forgot one
@
text
@d3 1
a3 1
 * $Id: wvdlg.h 1.17 1994/05/02 20:20:38 rushing Exp jglasser $
d5 3
d83 2
d147 9
a155 7
#define IDD_REVIEW_ATTACH           557
#define IDD_MIME_BOUNDARY           558

#define IDD_DECODE_PATH             559
#define IDD_DUMB_DECODE             560
#define IDD_VERBOSE_STATUS          561
#define IDD_ENABLE_SIG              562
d157 2
a158 2
#define IDD_SAVE_NEWSRC             563
#define IDD_SAVE_CONFIG             564
@


1.17
log
@changes from jody glasser
@
text
@d3 1
a3 1
 * $Id: wvdlg.h 1.16 1994/03/01 19:10:19 rushing Exp rushing $
d5 3
d108 1
d118 1
a118 1
#define ID_CONFIG_GROUP_MULTI_SELECT    337  // jlg 28-apr-94
@


1.16
log
@ThreadFullSubject option added
@
text
@d3 1
a3 1
 * $Id: wvdlg.h 1.15 1994/02/24 21:28:18 jcoop Exp rushing $
d5 3
d114 1
@


1.15
log
@jcoop changes
@
text
@d3 1
a3 1
 * $Id: wvdlg.h 1.14 1994/01/24 17:39:47 jcoop Exp $
d5 3
d110 1
@


1.14
log
@90.2 changes
@
text
@d3 1
a3 1
 * $Id: wvdlg.h 1.13 1994/01/15 23:53:22 jcoop Exp $
d5 3
d69 2
d95 1
d141 4
@


1.13
log
@constants for new jcoop dialogs - attachments, etc
@
text
@d3 1
a3 1
 * $Id: wvdlg.h 1.12 1994/01/12 19:27:55 mrr Exp $
d5 3
d64 2
d134 1
@


1.12
log
@mrr mods 4
@
text
@d3 1
a3 1
 * $Id: wvdlg.h 1.11 1993/12/08 01:29:07 rushing Exp $
d5 3
d67 1
d112 17
@


1.11
log
@new version box and cr lf consistency
@
text
@d3 1
a3 1
 * $Id: wvdlg.h 1.10 1993/12/07 19:47:23 rushing Exp rushing $
d5 3
d57 1
d101 1
a101 1
#define IDD_POSTLOG		    531	
@


1.10
log
@added IDD_SHOWVERSION
@
text
@d3 1
a3 1
 * $Id: wvdlg.h 1.9 1993/08/25 18:54:36 mbretherton Exp rushing $
d5 3
d38 2
d94 1
a94 1
#define IDD_SHOWVERSION		    526
@


1.9
log
@MRB merge, mail & post logging
@
text
@d3 1
a3 1
 * $Id: wvdlg.h 1.8 1993/07/13 16:03:04 riordan Exp $
d5 3
d89 1
@


1.8
log
@MRR mods
@
text
@d3 1
a3 1
 * $Id: wvdlg.h 1.7 1993/06/22 19:09:14 rushing Exp $
d5 3
d85 7
d94 1
@


1.7
log
@mail force type selection via dialog
@
text
@d3 1
a3 1
 * $Id: wvdlg.h 1.6 1993/06/22 18:14:53 rushing Exp rushing $
d5 3
d82 2
@


1.6
log
@added ID's for MailForceType
@
text
@d3 1
a3 1
 * $Id: wvdlg.h 1.5 1993/05/28 17:18:00 rushing Exp rushing $
d5 3
d74 1
a74 1
#define IDD_MAIL_SELECT_SMTP        515
d76 3
a78 2
#define IDD_MAIL_SELECT_AUTO        517
#define IDD_MAIL_SELECT_NONE        518
@


1.5
log
@Added Misc... option to enable threading
@
text
@d3 1
a3 1
 * $Id: wvdlg.h 1.4 1993/05/18 22:10:45 rushing Exp rushing $
d5 3
d71 4
@


1.4
log
@smtp support
@
text
@d3 1
a3 1
 * $Id: wvdlg.h 1.3 1993/05/13 19:58:51 SOMEONE Exp rushing $
d5 3
d67 1
@


1.3
log
@fancy 'from' in group window
@
text
@d3 1
a3 1
 * $Id: wvdlg.h 1.2 1993/05/13 16:17:30 rushing Exp SOMEONE $
d5 3
d61 3
a63 1
#define ID_CONFIG_FULLNAMEFROM      653
@


1.2
log
@article fetch limit dialog box support
@
text
@d3 1
a3 1
 * $Id: wvdlg.h 1.1 1993/02/16 20:54:42 rushing Exp rushing $
d5 3
d51 6
a56 7
#define wvdish                   
#define IDD_ART_THRESHOLD           106
#define IDD_ARTS_TO_GRAB            101
#define ID_THRESHOLD_ALL            102
#define ID_100_ARTS                 103
#define ID_250_ARTS                 104
#define ID_500_ARTS                 105
d58 1
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
d48 8
a55 1
#define wvdish                      102
@
