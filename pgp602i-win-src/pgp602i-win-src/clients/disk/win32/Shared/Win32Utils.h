//////////////////////////////////////////////////////////////////////////////
// Win32Utils.h
//
// Declarations for Win32Utils.cpp.
//////////////////////////////////////////////////////////////////////////////

// $Id: Win32Utils.h,v 1.1.2.16.2.2 1998/10/22 00:41:55 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_Win32Utils_h	// [
#define Included_Win32Utils_h


////////////
// Constants
////////////

static LPCSTR	kPGPdiskNormalMsgBoxTitle	= "PGPdisk";
static LPCSTR	kPGPdiskErrorMsgBoxTitle	= "PGPdisk Error";

// From Lmcons.h.

#define NERR_Success	0       /* Success */

#define	LM20_NNLEN		12		// LM 2.0 Net name length
#define	MAXCOMMENTSZ	256		// Multipurpose comment length
#define	SHPWLEN			8		// Share password length (bytes)


////////
// Types
////////

// PGDMessageBoxStyle defines message box styles.

enum PGDMessageBoxStyle {kPMBS_Ok, kPMBS_OkCancel, kPMBS_YesNo, 
	kPMBS_YesNoCancel};

// PGDMessageBoxFocus defines the initial focus on message box buttons.

enum PGDMessageBoxFocus {kPMBF_YesButton, kPMBF_NoButton, kPMBF_OkButton, 
	kPMBF_CancelButton};

// UserResponse defines message box return values.

enum UserResponse {kUR_OK, kUR_Cancel, kUR_Yes, kUR_No};

// BareNameExtFlag is used by the GetBareName function.

enum BareNameExtFlag {kBNE_Default, kBNE_HideExt, kBNE_ShowExt};

// From Svrapi.h.

struct share_info_50 
{
	char			shi50_netname[LM20_NNLEN+1];
	unsigned char 	shi50_type; 
    unsigned short	shi50_flags;
	char FAR *		shi50_remark;
	char FAR *		shi50_path;
	char			shi50_rw_password[SHPWLEN+1];
	char			shi50_ro_password[SHPWLEN+1];

};

// From Lmshare.h.

typedef struct _SHARE_INFO_2 
{
    LPTSTR  shi2_netname;
    DWORD   shi2_type;
    LPTSTR  shi2_remark;
    DWORD   shi2_permissions;
    DWORD   shi2_max_uses;
    DWORD   shi2_current_uses;
    LPTSTR  shi2_path;
    LPTSTR  shi2_passwd;

} SHARE_INFO_2, *PSHARE_INFO_2, *LPSHARE_INFO_2;


// From Lmwksta.h

typedef struct _WKSTA_INFO_100
{
    DWORD   wki100_platform_id;
    LPTSTR  wki100_computername;
    LPTSTR  wki100_langroup;
    DWORD   wki100_ver_major;
    DWORD   wki100_ver_minor;

} WKSTA_INFO_100, *PWKSTA_INFO_100, *LPWKSTA_INFO_100;


/////////////////////
// Exported functions
/////////////////////

void			RegisterPGPdiskMsgBoxParent(CWnd *pWnd);

UserResponse	PGPdiskMessageBox(LPCSTR message, 
					CWnd *pParent = NULL, 
					LPCSTR title = kPGPdiskNormalMsgBoxTitle, 
					PGDMessageBoxStyle style = kPMBS_Ok, 
					PGDMessageBoxFocus focus = kPMBF_YesButton);

UserResponse	ReportError(PGDMajorError perr, 
					DualErr derr = DualErr::NoError, 
					PGPUInt8 drive = kInvalidDrive, 
					PGDMessageBoxStyle style = kPMBS_Ok, 
					PGDMessageBoxFocus focus = kPMBF_YesButton);

UserResponse	DisplayMessage(PGPUInt32 stringId, 
					PGDMessageBoxStyle style = kPMBS_Ok, 
					PGDMessageBoxFocus focus = kPMBF_YesButton);

PGPInt32		FindMenuItemPosition(CMenu *pMenu, LPCSTR menuString);

DualErr			ConvertPathToLong(CString *path);
DualErr			GetRoot(LPCSTR path, CString *root);
DualErr			MakeRoot(PGPUInt8 drive, CString *root);
DualErr			StripRoot(LPCSTR path, CString *nonRoot);
DualErr			GetServer(LPCSTR path, CString *server);
DualErr			GetShare(LPCSTR path, CString *share);
DualErr			GetCurrentDirectory(CString *outDir);
DualErr			GetDirectory(LPCSTR path, CString *dir);
DualErr			GetDirectorySmart(LPCSTR path, CString *dir);
DualErr			GetBareName(LPCSTR path, CString *bareName, 
					BareNameExtFlag bneFlag = kBNE_Default);

DualErr			CanonicalizeVolumeName(LPCSTR inName, CString *outName);
DualErr			VerifyAndCanonicalizePath(LPCSTR inPath, 
					CString *outPath);
DualErr			ResolveShortcut(LPCSTR shortcutPath, CString *fullPath);

DualErr			BroadcastDriveMessage(PGPUInt8 drive, WPARAM msg);

DualErr			GetLocalComputerName(CString *compName);
DualErr			TranslateDriveToUNC(LPCSTR inPath, CString *outPath);
DualErr			TranslateUNCToLocal(LPCSTR inPath, CString *outPath,
					PGPBoolean *isLoopedBack);

DualErr			CheckIfLoopedBack(LPCSTR inFilePath, CString *outFilePath, 
					PGPBoolean *isLoopedBack);

#endif // Included_Win32Utils_h
