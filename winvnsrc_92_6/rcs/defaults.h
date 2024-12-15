head	 1.16;
branch   ;
access   ;
symbols  ;
locks    ; strict;
comment  @ * @;


1.16
date	 94.09.16.01.10.59;  author jcooper;  state Exp;
branches ;
next	 1.15;

1.15
date	 94.08.24.18.40.15;  author mrr;  state Exp;
branches ;
next	 1.14;

1.14
date	 94.08.24.17.59.26;  author jcooper;  state Exp;
branches ;
next	 1.13;

1.13
date	 94.08.11.00.09.17;  author jcooper;  state Exp;
branches ;
next	 1.12;

1.12
date	 94.07.25.22.41.22;  author pearse_w_r;  state Exp;
branches ;
next	 1.11;

1.11
date	 94.07.25.18.51.48;  author jcooper;  state Exp;
branches ;
next	 1.10;

1.10
date	 94.06.02.00.17.07;  author rushing;  state Exp;
branches ;
next	 1.9;

1.9
date	 94.05.23.19.55.59;  author rushing;  state Exp;
branches ;
next	 1.8;

1.8
date	 94.05.23.19.51.51;  author rushing;  state Exp;
branches ;
next	 1.7;

1.7
date	 94.05.23.18.37.00;  author jcooper;  state Exp;
branches ;
next	 1.6;

1.6
date	 94.05.02.19.53.51;  author rushing;  state Exp;
branches ;
next	 1.5;

1.5
date	 94.03.01.19.10.19;  author rushing;  state Exp;
branches ;
next	 1.4;

1.4
date	 94.02.24.21.21.55;  author jcoop;  state Exp;
branches ;
next	 1.3;

1.3
date	 94.02.16.20.59.17;  author rushing;  state Exp;
branches ;
next	 1.2;

1.2
date	 94.02.04.02.18.41;  author rushing;  state Exp;
branches ;
next	 1.1;

1.1
date	 94.01.24.18.15.23;  author jcoop;  state Exp;
branches ;
next	 ;


desc
@initial version
@


1.16
log
@new for SmartFiler, cc-by-mail, coding status always-on-top, and
keep-article-headers-visible-during-coding (!)
@
text
@/*
 * DEFAULTS.H
 * Defaults for profile
 * JSC 1/18/94
 * 
 * $Id: defaults.h 1.15 1994/08/24 18:40:15 mrr Exp $
 * 
 */

// personal
#define DEF_UserName "Your Name Here"
#define DEF_MailAddr "you@@somehost.somedomain"
#define DEF_Organization "Your Organization"
#define DEF_ReplyTo ""

// comm
#define DEF_UsingSocket TRUE
#define DEF_NNTPHost "NewServerName"
#define DEF_SMTPHost "MailServerName"
#define DEF_NNTPService "nntp"
#define DEF_DebugComm TRUE
#define DEF_GenSockDLL "gwinsock.dll"
#define DEF_ForceXhdr FALSE

// auth
#define DEF_NNTPUserName ""
#define DEF_NNTPPassword ""                        
#define DEF_AuthReqPost FALSE
#define DEF_AuthReqMail FALSE

// pref
#define DEF_EnableThreading FALSE
#define DEF_EnableSignature FALSE
#define DEF_SignatureFile ""
#define DEF_MailFileName "mailrc"
#define DEF_MailLog FALSE
#define DEF_MailLogFile  "MAIL.LOG"
#define DEF_PostLog FALSE
#define DEF_PostLogFile  "POST.LOG"
#define DEF_DoList ID_DOLIST_ASK-ID_DOLIST_BASE
#define DEF_ConnectAtStartup TRUE
#define DEF_FullNameFrom TRUE
#define DEF_ArticleThreshold 300
#define DEF_MailDemandLogon TRUE
#define DEF_MailForceType -1
#define DEF_CcByMail FALSE
#define DEF_ShowUnsubscribed TRUE
#define DEF_ShowReadArticles TRUE
#define DEF_ConfirmBatchOps TRUE
#define DEF_ConfirmExit TRUE
#define DEF_ConfirmReplyTo TRUE
#define DEF_NewWndGroup TRUE
#define DEF_NewWndArticle FALSE
#define DEF_SaveArtAppend TRUE
#define DEF_ThreadFullSubject FALSE
#define DEF_GroupMultiSelect TRUE
#define DEF_ShowUnreadOnly FALSE
 
// coding
#define DEF_DecodePath ""
#define DEF_CodingStatusVerbose FALSE
#define DEF_BlockCodingStatusAlwaysOnTop FALSE
#define DEF_DumbDecode FALSE
#define DEF_ReviewAttach TRUE
#define DEF_ArticleSplitLength 50000
#define DEF_EncodingTable ""
#define DEF_EncodingType "Base-64"
#define DEF_AttachInNewArt FALSE
#define DEF_ExecuteDecodedFiles FALSE
#define DEF_KeepArticleHeaderVisible FALSE
#define DEF_UseSmartFiler TRUE

#define DEF_GenerateMIME TRUE
#define DEF_MIMEUsageSuggestions TRUE
#define DEF_MIMEUUType "x-uue"
#define DEF_MIMEXXType "x-xxe"
#define DEF_MIMECustomType "x-custom3to4"
#define DEF_MIMEBoundary  "*-*-*- Next Section -*-*-*"
#define DEF_BlankBeforeMIME FALSE
#define DEF_DefaultContentType "Other"

// interface
#define DEF_ListFontFace "Courier"
#define DEF_ListFontSize 10
#define DEF_ListFontStyle "Regular"
#define DEF_ArticleFontFace "Courier"
#define DEF_ArticleFontSize 10
#define DEF_ArticleFontStyle "Regular"
#define DEF_StatusFontFace "Times New Roman"
#define DEF_StatusFontSize 11
#define DEF_StatusFontStyle "Regular"
#define DEF_PrintFontFace "Arial"
#define DEF_PrintFontSize 12
#define DEF_NetUnSubscribedColor "0,0,200"              // blue
#define DEF_NetSubscribedColor "0,0,0"                  // black
#define DEF_ArticleUnSeenColor "0,0,0"
#define DEF_ArticleSeenColor "0,0,200"
#define DEF_ArticleTextColor "0,0,0"
#define DEF_StatusTextColor "0,0,0"
#define DEF_ArticleBackgroundColor "255,255,255"        // white
#define DEF_ListBackgroundColor "255,255,255"
#define DEF_StatusBackgroundColor "255,255,255"
#define DEF_ThumbTrack TRUE

// smart filer 
#define DEF_OnDupeName 0
#define DEF_OnNameTooLong 0
#define DEF_MaxFileNameLen 8
#define DEF_MaxFileExtLen 3
#define DEF_EnableExtensionConversion 1@


1.15
log
@authorization enables post/mail
@
text
@d6 1
a6 1
 * $Id: defaults.h 1.14 1994/08/24 17:59:26 jcooper Exp $
d46 1
d62 1
d70 2
d104 7
@


1.14
log
@misc encoding/decoding changes
@
text
@d6 1
a6 1
 * $Id: defaults.h 1.13 1994/08/11 00:09:17 jcooper Exp $
d27 3
a29 1
#define DEF_NNTPPassword ""  
@


1.13
log
@Enhancements to Mime and article encoding/encoding
@
text
@d6 1
a6 1
 * $Id: defaults.h 1.12 1994/07/25 22:41:22 pearse_w_r Exp $
d68 1
@


1.12
log
@ShowUnreadOnly option
@
text
@d6 1
a6 1
 * $Id: defaults.h 1.11 1994/07/25 18:51:48 jcooper Exp $
d63 8
a72 1
#define DEF_EncodingType "UU"
a73 2
#define DEF_AttachInNewArt FALSE
#define DEF_ExecuteDecodedFiles FALSE
@


1.11
log
@execution of decoded files
@
text
@d6 1
a6 1
 * $Id: defaults.h 1.10 1994/06/02 00:17:07 rushing Exp $
d54 1
d82 2
a83 2
#define DEF_NetUnSubscribedColor "0,0,200"		// blue
#define DEF_NetSubscribedColor "0,0,0"			// black
d88 1
a88 1
#define DEF_ArticleBackgroundColor "255,255,255" 	// white
@


1.10
log
@put id key back in, remove log messages
@
text
@d6 1
a6 1
 * $Id: $
d67 1
@


1.9
log
@default 'connect at startup' to true.
@
text
@d5 3
@


1.8
log
@NNTPPort becomes NNTPService, and it's a string.
@
text
@d36 1
a36 1
#define DEF_ConnectAtStartup FALSE
@


1.7
log
@new attach code, session [dis]connect
@
text
@d17 1
a17 1
#define DEF_NNTPPort 119
@


1.6
log
@changes from jody glasser
@
text
@d36 1
d61 3
@


1.5
log
@ThreadFullSubject option added
@
text
@d49 1
@


1.4
log
@jcoop changes
@
text
@d48 1
@


1.3
log
@Added .ini param to force use of XHDR over XOVER
@
text
@d11 1
d30 1
d43 2
d50 1
@


1.2
log
@added GenSockDLL .ini parameter
@
text
@d19 1
@


1.1
log
@Initial revision
@
text
@d18 1
@
