/*
 * DEFAULTS.H
 * Defaults for profile
 * JSC 1/18/94
 * 
 * $Id: defaults.h 1.13 1994/08/11 00:09:17 jcooper Exp $
 * 
 */

// personal
#define DEF_UserName "Your Name Here"
#define DEF_MailAddr "you@somehost.somedomain"
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
#define DEF_DumbDecode FALSE
#define DEF_ReviewAttach TRUE
#define DEF_ArticleSplitLength 50000
#define DEF_EncodingTable ""
#define DEF_EncodingType "Base-64"
#define DEF_AttachInNewArt FALSE
#define DEF_ExecuteDecodedFiles FALSE

#define DEF_GenerateMIME TRUE
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
