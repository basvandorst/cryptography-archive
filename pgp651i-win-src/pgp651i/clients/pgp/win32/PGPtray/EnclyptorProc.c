/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: EnclyptorProc.c,v 1.56 1999/04/13 17:29:55 wjb Exp $
____________________________________________________________________________*/
#include "precomp.h"
#include "PThotkey.h"
#include "PTnet.h"
#include "pgpNetIPC.h"
#include <commctrl.h>

BOOL PopupTaskbarMenu(HWND hwndTarget, BOOL InPGPOperation);
UINT nLaunchKeysTimer=0;
BOOL PGPdiskExists=FALSE;
BOOL PGPnetExists=FALSE;
BOOL PGPtoolsExists=FALSE;
BOOL PGPkeysExists=FALSE;
HIMAGELIST hIml=NULL;

#define PGPNET_TIMER_ID			1234
#define PGPNET_TIMER_MS			10000

#define OE_REG_KEY	"Software\\Network Associates\\PGP\\OE"

typedef struct
{
	char szString[20];
	UINT uIcon;
} MENUINFO, *PMENUINFO;

MENUINFO miNet,miDisk,miTools,miKeys;

INT 
PTMessageBox (
		HWND	hwnd, 
		INT		iCaption, 
		INT		iText, 
		UINT	uStyle)
{
	CHAR	szCaption[64];
	CHAR	szText[256];

	LoadString (g_hinst, iCaption, szCaption, sizeof(szCaption));
	LoadString (g_hinst, iText, szText, sizeof(szText));

	return (MessageBox (hwnd, szText, szCaption, uStyle));
}

BOOL CheckForExistanceOfEXE(char *szEXEname)
{
	char szEXEPath[MAX_PATH];
	FILE *ftest;

	PGPpath(szEXEPath);
	strcat(szEXEPath,szEXEname);

	ftest=fopen(szEXEPath,"rb");
	
	if(ftest!=NULL)
	{
		fclose(ftest);
		return TRUE;
	}

	return FALSE;
}

BOOL CheckIfOKToClose(HWND hwnd)
{
	HKEY	hkey;
	INT		ids;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, OE_REG_KEY, 0, 
						KEY_ALL_ACCESS, &hkey) == ERROR_SUCCESS)
	{
		ids = IDS_CHECKIFOKTOCLOSEOE;
		RegCloseKey(hkey);
	}
	else
		ids = IDS_CHECKIFOKTOCLOSE;

	if (PTMessageBox(hwnd, IDS_CAPTION, ids, 
			MB_OKCANCEL|MB_ICONEXCLAMATION) == IDOK)
		return TRUE;
	else
		return FALSE;
}

#define NUM_BITMAPS 4

void PGPtrayCreateImageList(HIMAGELIST *hIml)
{
	int iNumBits;
	HDC hDC;
	HBITMAP hBmp;

	// ImageList Init

	hDC = GetDC (NULL);		// DC for desktop
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

	if (iNumBits <= 8) {
		*hIml =	ImageList_Create (16, 16, ILC_COLOR|ILC_MASK, 
							NUM_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hinst, MAKEINTRESOURCE (IDB_EXEBITMAP4BIT));
		ImageList_AddMasked (*hIml, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}
	else {
		*hIml =	ImageList_Create (16, 16, ILC_COLOR24|ILC_MASK, 
							NUM_BITMAPS, 0); 
		hBmp = LoadBitmap (g_hinst, MAKEINTRESOURCE (IDB_EXEBITMAP24BIT));
		ImageList_AddMasked (*hIml, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
	}
}

void MeasureMenuItem(LPMEASUREITEMSTRUCT mi)
{
    // create the font we will use for the title
    HFONT hfont,oldhfont;
	HDC hdc;
	SIZE size;
	PMENUINFO pmi;
	
	pmi=(PMENUINFO)mi->itemData;
	hfont=GetStockObject(ANSI_VAR_FONT);
	hdc=GetDC(NULL);
	oldhfont=SelectObject(hdc,hfont);
	GetTextExtentPoint32(hdc,pmi->szString,strlen(pmi->szString),&size);
	SelectObject(hdc,oldhfont);
	DeleteObject(hfont);

	// add in the left margin for the menu item
	size.cx += GetSystemMetrics(SM_CXMENUCHECK)+8;

	// return the width and height
	mi->itemWidth = size.cx;
	mi->itemHeight = size.cy+6;
}

void DrawMenuItem(LPDRAWITEMSTRUCT di)
{
    // create the font we will use for the title
    HFONT hfont,oldhfont;
	HBRUSH hbgb;
	int mode;
	COLORREF text,back;
	PMENUINFO pmi;
	
	pmi=(PMENUINFO)di->itemData;

	hfont=GetStockObject(ANSI_VAR_FONT);

	if((BOOL)(di->itemState & ODS_SELECTED))
	{
		hbgb = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
		FillRect(di->hDC, &di->rcItem, hbgb);
		DeleteObject(hbgb);

        // Set the text background and foreground colors
        text=SetTextColor(di->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
        back=SetBkColor(di->hDC, GetSysColor(COLOR_HIGHLIGHT));
   	}
	else
	{
		hbgb = CreateSolidBrush(GetSysColor(COLOR_MENU));
		FillRect(di->hDC, &di->rcItem, hbgb);
		DeleteObject(hbgb);

        // Set the text background and foreground colors to the 
        // standard window colors
        text=SetTextColor(di->hDC, GetSysColor(COLOR_MENUTEXT));
        back=SetBkColor(di->hDC, GetSysColor(COLOR_MENU));
	}

	if((BOOL)(di->itemState & ODS_DISABLED))
	{
		SetTextColor(di->hDC, GetSysColor(COLOR_GRAYTEXT));
	}

	mode = SetBkMode(di->hDC, TRANSPARENT);

	ImageList_Draw(hIml,pmi->uIcon,
			di->hDC,
			di->rcItem.left,
			di->rcItem.top+1,
			ILD_TRANSPARENT);

	// add the menu margin offset
	di->rcItem.left += GetSystemMetrics(SM_CXMENUCHECK)+8;

	oldhfont = (HFONT)SelectObject(di->hDC, hfont);

	// draw the text left aligned and vertically centered
	DrawText(di->hDC,pmi->szString, -1, &di->rcItem, DT_SINGLELINE|DT_VCENTER|DT_LEFT);

	SelectObject(di->hDC, oldhfont);
	SetBkMode(di->hDC, mode);
	SetTextColor(di->hDC, text);
	SetBkColor(di->hDC, back);
}

LRESULT CALLBACK EnclyptorProc(HWND hwnd, UINT msg, 
							   WPARAM wParam, LPARAM lParam)
{
	static char HelpFile[MAX_PATH + 1] = "\0";
	static BOOL InPGPOperation = FALSE;
	static HWND hwndFocus;
	
	// See if user deselects caching via prefs
	CheckForPurge(msg,wParam);

	// check for hotkey-related messages
	PTCheckForHotKeyPrefsMsg(hwnd,msg);

	// check for PGPnet-related messages
	PTCheckForNetMsg(hwnd,msg,wParam,lParam);

	switch(msg)
	{
		case WM_CREATE:
		{
			InitCommonControls();

			// See if we have to grey PGPDisk Menu Item
			PGPdiskExists=CheckForExistanceOfEXE("PGPdisk.exe");
			PGPnetExists=CheckForExistanceOfEXE("PGPnet.exe");
			PGPtoolsExists=CheckForExistanceOfEXE("PGPtools.exe");
			PGPkeysExists=CheckForExistanceOfEXE("PGPkeys.exe");

			// PGPdisk only in biz and pp versions
#if !(PGP_BUSINESS_SECURITY || PGP_PERSONAL_PRIVACY)
			PGPdiskExists=FALSE;
#endif

			LoadString(g_hinst, IDS_PGPTOOLS, miTools.szString, sizeof(miTools.szString));
			LoadString(g_hinst, IDS_PGPNET, miNet.szString, sizeof(miNet.szString));
			LoadString(g_hinst, IDS_PGPDISK, miDisk.szString, sizeof(miDisk.szString));
			LoadString(g_hinst, IDS_PGPKEYS, miKeys.szString, sizeof(miKeys.szString));

			miKeys.uIcon=0;
			miTools.uIcon=1;
			miDisk.uIcon=2;
			miNet.uIcon=3;

			PGPtrayCreateImageList(&hIml);

			PTLoadAndSetHotKeys(hwnd);
///			PTSendLogOnOffMessage (hwnd, QUERYLOGON);
			// the service needs the following because it can't detect
			// logons under Win9x
			PTSendLogOnOffMessage (hwnd, LOGON);

			StartUpdateTimer(hwnd, &nLaunchKeysTimer);

			// start timer that periodically queries PGPnet service for status
			if (PGPnetExists)
			{
				SetTimer(hwnd, PGPNET_TIMER_ID, PGPNET_TIMER_MS, NULL);
				PostMessage (hwnd, WM_TIMER, 0, 0);
			}

			return 0;
		}

		case WM_HOTKEY:
		{
			PTProcessHotKey(hwnd, wParam);

			return TRUE;
		}

		case WM_TIMER :
		{
			PTUpdateTrayIconAndText(hwnd);

			return 0;
		}

		case WM_DRAWITEM: 
        {
	        DrawMenuItem((LPDRAWITEMSTRUCT) lParam);

            return TRUE;
        }

        case WM_MEASUREITEM:
		{
	        MeasureMenuItem((LPMEASUREITEMSTRUCT)lParam);

            return TRUE;
		}

		case WM_QUERYOPEN:
		{
			return 0;
		}

		case WM_TASKAREA_MESSAGE:
		{
			UINT uID = (UINT) wParam;
			UINT uMouseMsg = (UINT) lParam;

			if(uMouseMsg == WM_RBUTTONDOWN)
			{
				switch(uID)
				{
					case 1:
					{
						PopupTaskbarMenu(hwnd, InPGPOperation);
						return 0;
					}
				}
			}
			else if(uMouseMsg == WM_LBUTTONDOWN)
			{
				switch(uID)
				{
					case 1:
					{
						PopupTaskbarMenu(hwnd, InPGPOperation);
						return 0;
					}
				}
			}
			break;
		}

		case WM_COMMAND:
		{
			// Menu is greyed, but just in case hot keys are sending 
			// messages
			if(InPGPOperation)
				break;

			switch(wParam)
			{
				case ID_ENCRYPTCLIPBOARD:
				{
					InPGPOperation = TRUE;
					EncryptClipboard(hwnd, szApp, PGPsc, PGPtls, TRUE, FALSE);
					InPGPOperation = FALSE;
					break;
				}

				case ID_SIGNCLIPBOARD:
				{
					InPGPOperation = TRUE;
					EncryptClipboard(hwnd, szApp, PGPsc,PGPtls, FALSE, TRUE);
					InPGPOperation = FALSE;
					break;
				}

				case ID_ENCRYPTSIGNCLIPBOARD:
				{
					InPGPOperation = TRUE;
					EncryptClipboard(hwnd, szApp, PGPsc,PGPtls, TRUE, TRUE);
					InPGPOperation = FALSE;
					break;
				}

				case ID_DECRYPTVERIFYCLIPBOARD:
				{
					InPGPOperation = TRUE;
					DecryptClipboard(hwnd, szApp, PGPsc,PGPtls);
					InPGPOperation = FALSE;
					break;
				}

				case ID_ENCRYPTWINDOW:
				{
					InPGPOperation = TRUE;
					if(DoCopy(hwnd,PGPsc,TRUE,&hwndFocus))
					{
						if(EncryptClipboard(hwnd, szApp, PGPsc, PGPtls, TRUE, FALSE))
							DoPaste(TRUE,hwndFocus);
						else
							DoFocus(TRUE,hwndFocus);
					}
					InPGPOperation = FALSE;
					break;
				}

				case ID_SIGNWINDOW:
				{
					InPGPOperation = TRUE;
					if(DoCopy(hwnd,PGPsc,TRUE,&hwndFocus))
					{
						if(EncryptClipboard(hwnd, szApp, PGPsc,PGPtls, FALSE, TRUE))
							DoPaste(TRUE,hwndFocus);
						else
							DoFocus(TRUE,hwndFocus);
					}
					InPGPOperation = FALSE;
					break;
				}

				case ID_ENCRYPTSIGNWINDOW:
				{
					InPGPOperation = TRUE;
					if(DoCopy(hwnd,PGPsc,TRUE,&hwndFocus))
					{
						if(EncryptClipboard(hwnd, szApp, PGPsc,PGPtls, TRUE, TRUE))
							DoPaste(TRUE,hwndFocus);
						else
							DoFocus(TRUE,hwndFocus);
					}
					InPGPOperation = FALSE;
					break;
				}

				case ID_DECRYPTVERIFYWINDOW:
				{
					InPGPOperation = TRUE;
					if(DoCopy(hwnd,PGPsc,TRUE,&hwndFocus))
					{
						DecryptClipboard(hwnd, szApp, PGPsc,PGPtls);
						DoFocus(TRUE,hwndFocus);
					}
					InPGPOperation = FALSE;
					break;
				}

				case ID_WIPECLIP:
				{
					InPGPOperation = TRUE;
					ClipboardWipe(hwnd,PGPsc);
					InPGPOperation = FALSE;
					break;
				}
				
				case ID_VIEWCLIPBOARD:
				{
					InPGPOperation = TRUE;
					LaunchInternalViewer(PGPsc,hwnd);
					InPGPOperation = FALSE;
					break;
				}
	
				case ID_LAUNCHPGPKEYS:
				{
					InPGPOperation = TRUE;
					DoLaunchKeys(hwnd);
					InPGPOperation = FALSE;
					break;
				}

				case ID_LAUNCHPGPTOOLS:
				{
					InPGPOperation = TRUE;
					DoLaunchTools(hwnd);
					InPGPOperation = FALSE;
					break;
				}

				case ID_LAUNCHPGPDISK:
				{
					InPGPOperation = TRUE;
					DoLaunchDisk(hwnd);
					InPGPOperation = FALSE;
					break;
				}

				case ID_PGPNETHOSTS:
				{
					InPGPOperation = TRUE;
					PTNetLaunch(hwnd, PGPNET_HOSTPAGE);
					InPGPOperation = FALSE;
					break;
				}

				case ID_PGPNETLOG:
				{
					InPGPOperation = TRUE;
					PTNetLaunch(hwnd, PGPNET_LOGPAGE);
					InPGPOperation = FALSE;
					break;
				}

				case ID_PGPNETSTATUS:
				{
					InPGPOperation = TRUE;
					PTNetLaunch(hwnd, PGPNET_STATUSPAGE);
					InPGPOperation = FALSE;
					break;
				}

				case ID_PGPNETOPTIONS:
				{
					InPGPOperation = TRUE;
					PTNetLaunch(hwnd, PGPNET_OPTIONSHEET);
					InPGPOperation = FALSE;
					break;
				}

				case ID_PGPNETLOGON:
				{
					InPGPOperation = TRUE;
					PTSendLogOnOffMessage (hwnd, LOGON);
					InPGPOperation = FALSE;
					break;
				}

				case ID_PGPNETLOGOFF:
				{
					InPGPOperation = TRUE;
					PTSendLogOnOffMessage (hwnd, LOGOFF);
					InPGPOperation = FALSE;
					break;
				}

				case ID_PROPERTIES:
				{
					InPGPOperation = TRUE;
					PGPscPreferences(hwnd, PGPsc,PGPtls);
					InPGPOperation = FALSE;
					break;
				}

				case ID_QUIT_ENCLYPTOR:
				{
					if (CheckIfOKToClose(hwnd))
					{
						SetForegroundWindow(hwnd);
						PostMessage(hwnd, WM_CLOSE, 0, 0);
					}
					break;
				}

				case ID_HELP_TOPICS:
				{
					InPGPOperation = TRUE;
					PGPpath(HelpFile);
					strcat(HelpFile,"PGP.hlp");
					WinHelp(hwnd, HelpFile, HELP_FINDER, 0);
					InPGPOperation = FALSE;
					break;
				}
								
			}
			break;
		}
		
		case WM_ENDSESSION :
		{
			break;
		}

		case WM_CLOSE:
		{	
			KillTimer(hwnd, nLaunchKeysTimer);
			PTRemoveHotKeys(hwnd);
			PostQuitMessage(0);	
			return 0;
		}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void InsertOwnerDrawnMenuItem(HMENU hMenu,UINT pos,
							  UINT id,HMENU hSubMenu,PMENUINFO pmi)
{
	MENUITEMINFO mii;
	BOOL result;

	memset(&mii,0x00,sizeof(MENUITEMINFO));

	mii.cbSize=sizeof(MENUITEMINFO);
	mii.fMask=MIIM_TYPE|MIIM_DATA|MIIM_ID|MIIM_STATE|MIIM_SUBMENU;
	mii.fType=MFT_OWNERDRAW;
	mii.dwItemData=(DWORD)pmi;
	mii.wID=id;
	mii.fState=MFS_ENABLED;
	mii.hSubMenu=hSubMenu;

	result=InsertMenuItem(hMenu, 
		pos, 
		TRUE,  
		&mii); 
}

BOOL PopupTaskbarMenu(HWND hwndTarget, BOOL InPGPOperation)
{
	HMENU hMenu = NULL;
	HMENU hSubMenu = NULL;
	HMENU hMenuNet = NULL;
	HMENU hSubMenuNet = NULL;

	POINT pt;

	GetCursorPos( &pt );

	// If the cursor is at the top, we need to reverse the menu
	if(pt.y<200)
	{
		// load up menu
		hMenu = LoadMenu(g_hinst, MAKEINTRESOURCE(IDM_TASKBARMENUREV));
		hSubMenu = GetSubMenu(hMenu,0);

		// Make these owner drawn so we can get icons
		if (PGPnetExists) 
		{
			// load up PGPnets popup menu
			hMenuNet = LoadMenu(g_hinst, MAKEINTRESOURCE(IDM_PGPNETPOPUP));
			hSubMenuNet = GetSubMenu(hMenuNet,0);

			InsertOwnerDrawnMenuItem(
					hSubMenu,3,ID_LAUNCHPGPNET,hSubMenuNet,&miNet);
		}
		if (PGPdiskExists)
			InsertOwnerDrawnMenuItem(hSubMenu,3,ID_LAUNCHPGPDISK,NULL,&miDisk);
		if (PGPkeysExists)
			InsertOwnerDrawnMenuItem(hSubMenu,3,ID_LAUNCHPGPKEYS,NULL,&miKeys);
		if (PGPtoolsExists)
			InsertOwnerDrawnMenuItem(hSubMenu,3,ID_LAUNCHPGPTOOLS,NULL,&miTools);
	}
	else
	{
		// load up menu
		hMenu = LoadMenu(g_hinst, MAKEINTRESOURCE(IDM_TASKBARMENU));
		hSubMenu = GetSubMenu(hMenu,0);

		// Make these owner drawn so we can get icons
		if (PGPtoolsExists)
			InsertOwnerDrawnMenuItem(hSubMenu,5,ID_LAUNCHPGPTOOLS,NULL,&miTools);
		if (PGPkeysExists)
			InsertOwnerDrawnMenuItem(hSubMenu,5,ID_LAUNCHPGPKEYS,NULL,&miKeys);
		if (PGPdiskExists)
			InsertOwnerDrawnMenuItem(hSubMenu,5,ID_LAUNCHPGPDISK,NULL,&miDisk);
		if (PGPnetExists) 
		{
			// load up PGPnets popup menu
			hMenuNet = LoadMenu(g_hinst, MAKEINTRESOURCE(IDM_PGPNETPOPUP));
			hSubMenuNet = GetSubMenu(hMenuNet,0);

			InsertOwnerDrawnMenuItem(
					hSubMenu,5,ID_LAUNCHPGPNET,hSubMenuNet,&miNet);
		}
	}

	if(InPGPOperation || PTNetIsGUIDisabled ())
	{
		EnableMenuItem(hMenu, 
					   ID_ENCRYPTCLIPBOARD, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, 
					   ID_SIGNCLIPBOARD, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, 
					   ID_ENCRYPTSIGNCLIPBOARD, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, 
					   ID_DECRYPTVERIFYCLIPBOARD, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, 
					   ID_ENCRYPTWINDOW, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, 
					   ID_SIGNWINDOW, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, 
					   ID_ENCRYPTSIGNWINDOW, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, 
					   ID_DECRYPTVERIFYWINDOW, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, 
					   ID_PROPERTIES, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, 
					   ID_VIEWCLIPBOARD, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, 
					   ID_HELP_TOPICS, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, 
					   ID_LAUNCHPGPKEYS, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, 
					   ID_LAUNCHPGPDISK, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, 
					   ID_LAUNCHPGPTOOLS, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, 
					   ID_WIPECLIP, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, 
					   ID_LAUNCHPGPTOOLS, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenu, 
					   ID_QUIT_ENCLYPTOR, 
					   MF_BYCOMMAND | MF_GRAYED);

		// PGPnet submenu
		EnableMenuItem(hMenuNet, 
					   ID_PGPNETSTATUS, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenuNet, 
					   ID_PGPNETLOG, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenuNet, 
					   ID_PGPNETHOSTS, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenuNet, 
					   ID_PGPNETOPTIONS, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenuNet, 
					   ID_PGPNETLOGON, 
					   MF_BYCOMMAND | MF_GRAYED);
		EnableMenuItem(hMenuNet, 
					   ID_PGPNETLOGOFF, 
					   MF_BYCOMMAND | MF_GRAYED);
	}
	else
	{
		if (PTNetIsLogonDisabled ())
		{
			EnableMenuItem(hMenuNet, 
					   ID_PGPNETLOGON, 
					   MF_BYCOMMAND | MF_GRAYED);
		}

		if (PTNetIsLogoffDisabled ())
		{
			EnableMenuItem(hMenuNet, 
					   ID_PGPNETLOGOFF, 
					   MF_BYCOMMAND | MF_GRAYED);
		}
	}

	//  Calls to SetForegroundWindow and PostMessage to fix a bug
    //  documented in PSS ID Number: Q135788
	SetForegroundWindow(hwndTarget); 
	TrackPopupMenu( hSubMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON, 
					pt.x, pt.y, 0, hwndTarget, NULL);
	PostMessage( hwndTarget, WM_NULL, 0, 0 ) ;
	// Above fixes "three clicks" problem  -wjb

	DestroyMenu(hMenu);

	return TRUE;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
