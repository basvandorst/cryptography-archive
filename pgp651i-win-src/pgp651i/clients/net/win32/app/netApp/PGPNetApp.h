/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPnetApp.h - PGPnet GUI application internal header file
	

	$Id: PGPNetApp.h,v 1.57.4.1 1999/06/08 16:32:06 pbj Exp $
____________________________________________________________________________*/

#ifndef _PGPNETAPP_H
#define _PGPNETAPP_H

// includes
#include "commctrl.h"
#include "pgpPubTypes.h"
#include "pgpErrors.h"
#include "pgpUtilities.h"
#include "pgpMem.h"

#include "pgpNetPrefs.h"
#include "pgpNetConfig.h"

// compilation flags
#define PGPNET_SECURE_HOSTS_BEHIND_GATEWAYS		TRUE
#define PGPNET_ALLOW_DH768						FALSE

// PGPNetHostEntry userVal flags
#define PGPNET_HOST_MODIFIED	0x0001
#define PGPNET_HOST_SAEXISTS	0x0002

// control positions
#define HOR_TAB_OFFSET			6
#define TOP_TAB_OFFSET			6 
#define BOTTOM_TAB_OFFSET		4

#define ONOFFBOX_HEIGHT			40
#define ONOFFBOX_WIDTH			120
#define HOR_ON_OFFSET			20
#define HOR_OFF_OFFSET			70
#define TOP_ONOFFBOX_OFFSET		3
#define TOP_ONOFF_OFFSET		8
#define BOTTOM_ONOFF_OFFSET		6

// messages
#define PGPNET_M_INTERNALCOPYDATA	WM_APP+200
#define PGPNET_M_ACTIVATEPAGE		WM_APP+201
#define PGPNET_M_RELOADKEYRINGS		WM_APP+202
#define PGPNET_M_SETFOCUS			WM_APP+203
#define PGPNET_M_UPDATELIST			WM_APP+204
#define PGPNET_M_CLEARSALIST		WM_APP+205

// option panel indices
#define PGPNET_GENOPTIONS	0
#define PGPNET_AUTHOPTIONS	1
#define PGPNET_ADVOPTIONS	2
#define PGPNET_NUMOPTIONS	3

// WPARAMs of WM_APP
#define	PGPNET_STARTUP		0
#define PGPNET_PAGESELECT	1
#define PGPNET_SHOWWINDOW	2

// host image index definitions
#define IDX_SSCRYPTO		0
#define IDX_PGPCRYPTO		1
#define IDX_X509CRYPTO		2
#define	IDX_GATEWAY			3
#define	IDX_SECUREHOST		4
#define	IDX_INSECUREHOST	5
#define	IDX_SECURESUBNET	6
#define	IDX_INSECURESUBNET	7
#define IDX_EXISTINGSA		8
#define	NUM_HOST_BITMAPS	9

#define IDXB_SSCRYPTO		(1<<IDX_SSCRYPTO)
#define IDXB_PGPCRYPTO		(1<<IDX_PGPCRYPTO)
#define IDXB_X509CRYPTO		(1<<IDX_X509CRYPTO)

// key image index definitions
#define	IDX_NONE			-1
#define IDX_RSAPUBKEY		0
#define IDX_RSAPUBDISKEY	1
#define IDX_RSAPUBREVKEY	2
#define IDX_RSAPUBEXPKEY	3
#define IDX_RSASECKEY		4
#define IDX_RSASECDISKEY	5
#define IDX_RSASECREVKEY	6
#define IDX_RSASECEXPKEY	7
#define IDX_RSASECSHRKEY	8
#define IDX_DSAPUBKEY		9
#define IDX_DSAPUBDISKEY	10
#define IDX_DSAPUBREVKEY	11
#define IDX_DSAPUBEXPKEY	12
#define IDX_DSASECKEY		13
#define IDX_DSASECDISKEY	14
#define IDX_DSASECREVKEY	15
#define IDX_DSASECEXPKEY	16
#define IDX_DSASECSHRKEY	17
#define IDX_RSAUSERID		18
#define IDX_DSAUSERID		19
#define IDX_INVALIDUSERID	20
#define IDX_PHOTOUSERID		21
#define IDX_CERT			22
#define IDX_REVCERT			23
#define IDX_EXPCERT			24
#define IDX_BADCERT			25
#define IDX_EXPORTCERT		26
#define IDX_METACERT		27
#define IDX_TRUSTEDCERT		28
#define IDX_X509CERT		29
#define IDX_X509EXPCERT		30
#define IDX_X509REVCERT		31
#define NUM_KEY_BITMAPS		32

// typedefs
typedef struct {
	PNCONFIG			pnconfig;
	PGPKeySetRef		keysetMain;
	BOOL				bPGPKey;
	BOOL				bX509Key;
	BOOL				bUpdateHostList;
	HWND				hwndHostTreeList;
	HWND				hwndStatusDlg;
	HWND				hwndAuthDlg;
	HWND				hwndHostDlg;
} APPOPTIONSSTRUCT, *PAPPOPTIONSSTRUCT;

typedef struct {
	HWND				hwndIkeTimeExpire;
	HWND				hwndIpsecTimeExpire;
	HWND				hwndPGPKeyList;
	HWND				hwndX509KeyList;
	INT					iSelectedCertItem;
	INT					iSelectedCertParam;
	HIMAGELIST			hil;

	PNCONFIG			pnconfig;
	BOOL				bUserCancel;
	BOOL				bPGPKey;
	BOOL				bX509Key;
	BOOL				bKeyringsChanged;
} OPTIONSSTRUCT, *POPTIONSSTRUCT;


// macro definitions
#define CKERR		if (IsPGPError (err)) goto done

// project prototypes

// PNaddwiz.c
BOOL
PNAddHostWizard (
		HWND		hwndParent,
		PPNCONFIG	ppnconfig,
		INT			iSelectedHost);

// PNadvanced.c
BOOL CALLBACK
PNAdvancedOptionsDlgProc (
		HWND		hwnd, 
		UINT		uMsg, 
		WPARAM		wParam, 
		LPARAM		lParam);

// PNedit.c
BOOL
PNEditHostEntry (
		HWND		hwndParent,
		PPNCONFIG	ppnconfig,
		INT			iIndex,
		BOOL		bHasChildren);

BOOL
PNIsSubnetMaskValid (
		PGPUInt32	uSubnetMask);

BOOL
PNIsHostAlreadyInList (
		HWND				hwnd,
		PGPNetHostType		hosttype,
		PGPUInt32			uIP,
		PGPUInt32			uSubnetMask,
		PGPNetHostEntry*	pHostList,
		PGPUInt32			uHostCount,
		PGPInt32			iIndex);

// PNhost.c
BOOL CALLBACK
PNHostDlgProc (
		HWND		hwnd, 
		UINT		uMsg, 
		WPARAM		wParam, 
		LPARAM		lParam);

BOOL
PNAddHostEntry (
		HWND		hwnd,
		PPNCONFIG	ppnconfig,
		BOOL		bGateway,
		INT			iChildOf,
		LPSTR		pszParent);

// PNlog.c
BOOL CALLBACK
PNLogDlgProc (
		HWND		hwnd, 
		UINT		uMsg, 
		WPARAM		wParam, 
		LPARAM		lParam);

// PNmisc.c
LPARAM
PNMessageBox (
		HWND		hWnd, 
		INT			iCaption, 
		INT			iMessage,
		ULONG		ulFlags);

PGPError
PNErrorBox (
		HWND		hWnd, 
		INT			iCode);

VOID
PNCommaCat (
		LPSTR		psz,
		INT			ids);

BOOL
PNLoadKeyRings (
		HWND			hwnd,
		PGPKeySetRef*	pkeyset);

UINT 
PNGetServiceStatus (
		HWND		hwnd);

BOOL 
PNSendServiceMessage (
		UINT		uMsg,
		WPARAM		wParam,
		LPARAM		lParam);

BOOL 
PNSendTrayAppMessage (
		UINT		uMsg,
		WPARAM		wParam,
		LPARAM		lParam);

VOID
PNHelpAbout (
		HWND		hwnd);

BOOL
PNGetSharedSecret (
		HWND		hwnd,
		LPSTR		pszSecret,
		UINT		uMaxSecretLength);

INT 
PNDetermineKeyIcon (
		PGPKeyRef	key, 
		BOOL*		pbItalics);

INT 
PNDetermineCertIcon (
		PGPSigRef	cert, 
		BOOL*		pbItalics);

VOID
PNConvertTimeToString (
		PGPTime		Time, 
		LPSTR		sz, 
		INT			iLen);
VOID
PNGetLocalHostInfo (
		DWORD*		pdwAddress,
		LPSTR		pszName, 
		INT			iLen);

BOOL
PNGetRemoteHostIP (
		LPSTR		psz, 
		DWORD*		pdwAddress);

VOID 
PNSaveConfiguration (
		HWND			hwnd,
		PPNCONFIG		pPNConfig,
		BOOL			bKeyringsUpdated);

VOID
PNSaveOnOffButtons (
		HWND			hwnd,
		PGPContextRef	context);

VOID
PNSetAuthKeyFlags (
		PGPContextRef	context,
		PPNCONFIG		ppnconfig,
		PBOOL			pbPGPKey,
		PBOOL			pbX509Key);

		
// PNmsgprc.c
LRESULT CALLBACK 
PGPnetWndProc (
		HWND		hwnd, 
		UINT		uMsg, 
		WPARAM		wParam, 
		LPARAM		lParam);

UINT
PNDisplayStatusBarText (VOID);

VOID
PNSetStatusSACount (
		INT			iCount);

VOID
PNIncStatusSACount (VOID);

VOID
PNDecStatusSACount (VOID);

VOID
PNLoadAppOptions (
		HWND	hwnd,
		BOOL	bInitialTime);

// PNoption.c
VOID
PNOptionsPropSheet (
		HWND	hwnd,
		UINT	uStartPage);

// PNreport.c
PGPError
PNSaveLogToFile (
		HWND	hwnd);

PGPError
PNSaveStatusToFile (
		HWND	hwnd);

// PNselect.c
PGPError 
PNSelectPGPKey (
		PGPContextRef	context,
		PGPKeySetRef	keysetMain,
		HWND			hwndParent,
		PGPUInt32*		puPGPAlg,
		PGPByte*		pexpkeyidPGP);

// PNstatus.c
BOOL CALLBACK
PNStatusDlgProc (
		HWND		hwnd, 
		UINT		uMsg, 
		WPARAM		wParam, 
		LPARAM		lParam);

BOOL
PNDoesHostHaveSA (
		BOOL		bBehindGateway,
		DWORD		dwHostAddress,
		DWORD		dwHostMask);

BOOL
PNRemoveHostSAs (
		DWORD		dwHostAddress,
		DWORD		dwHostMask);

#endif //_PGPNETAPP_H