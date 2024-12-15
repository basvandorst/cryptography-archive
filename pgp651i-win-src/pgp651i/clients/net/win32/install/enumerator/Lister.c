/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	

	$Id: Lister.c,v 1.64.4.1 1999/06/14 05:26:54 philipn Exp $
____________________________________________________________________________*/

/*
 * Lister - Lists all network adapters on the machine
 *          the user selects the desired adapter(s)
 *          then this information is passed on.
 *
 */


/*
 * This is for reference
 *# NetBT\Linkage
 * LanmanServer\Linkage
 * LanmanWorkstation\Linkage
 * RemoteAccess\Linkage
 * NetBIOS\Linkage
 * Tcpip\Linkage
 */
#pragma message( "Compiling " __FILE__ ) 
#pragma message( "Last modified on " __TIMESTAMP__ ) 

#include <windows.h>
#include <windowsx.h>
#include <winreg.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h> 
#include <mbstring.h>
#include "resource.h"
#include "Define.h"
#include "Lister.h"
#include "globals.h"

const		DWORD INFINSTALL_PRIMARYINSTALL		= 0x00000001;
const		DWORD INFINSTALL_INPROCINTERP		= 0x00000002;
const		DWORD INFINSTALL_SUPPORTED			= 0x00000003;

/*
 * Program Entry Point
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	HKEY		hMainKey	= HKEY_LOCAL_MACHINE;
	HKEY		hOpenKey	= NULL;
	HWND		hwnd;
	MSG			msg;
	WNDCLASSEX	wndclass;
	UINT		nBufferLengthMax = 1024;
	LPBYTE		pData		= NULL;
	DWORD		dwSize;


	/*Ensure Common control Dll is loaded*/
	InitCommonControls();

	bSansCmdLine = FALSE;
	bDelReg		 = FALSE;
	bRunOnce	 = FALSE;

	if (IsCorrectVersion() == FALSE)
	{
		MessageBox (GetFocus(), WRONGOS, szAppName, 0 | MB_ICONERROR);
		exit(0);
	}/*if*/

	/*check the command line*/
	bSilentInstall = FALSE;

	/*void cmdline is ran from runonce
	  this is to fix the problem with 
	  the sys32 dir. poping up when 
	  the runonce contains a value
	  but no data*/
	if (strcmp(szCmdLine, "void") == 0)
		exit(0);
	if (strcmp(szCmdLine, INSTALL) == 0)
		goto begin;
	if (strcmp(szCmdLine, "") == 0)
	{
		bSansCmdLine = TRUE;
		goto begin;
	}/*if*/
	if (strcmp(szCmdLine, "runonce") == 0)
	{
		bSansCmdLine = TRUE;
		bRunOnce = TRUE;
		goto begin;
	}/*if*/
	else if (strcmp(szCmdLine, INSTALLSILENT) == 0)
	{
		bSilentInstall = TRUE;
		goto begin;
	}/*elseif*/
	else if (strcmp(szCmdLine, BINDREVIEW) == 0)
	{
		hwnd = GetDesktopWindow();
		BindReview(hwnd);
		goto done;
	}/*elseif*/
	else if (strcmp(szCmdLine, ADDINF) == 0)
	{
		RemoveInf();
		AddInf();
		goto done;
	}/*elseif*/
	else if (strcmp(szCmdLine, REMOVEINF) == 0)
	{
		RemoveInf();
		goto done;
	}/*elseif*/
	else if (strstr(szCmdLine, DELREG) != NULL)
	{
		RemoveInf();
		PutSAinRunKey();
		dwSize = 1024;
		pData = (LPBYTE) calloc(sizeof(BYTE), dwSize);
		if (RegOpenKeyEx(hMainKey, RUNONCE, 0, KEY_ALL_ACCESS, &hOpenKey)
		== ERROR_SUCCESS)	
			if (RegQueryValueEx(hOpenKey, "PGPnetNeedsReboot", 0,
							  NULL, pData, &dwSize) != ERROR_SUCCESS)
			{
				NeedReboot();
				bDelReg = TRUE;
				RemoveInfFile();
				RemoveNT();
				RemoveInf();
			}/*if*/
		free (pData);
		RegCloseKey(hOpenKey);
		goto done;
	}/*elseif*/
	else if (strcmp(szCmdLine, UNINSTALL) == 0)		     
	{
		if (bIsNT)
		{
			RemoveInf();
			RemoveInfFile();
			RemoveNT();
		}
		else
		{
			Remove95(CLASSNETTRANS, 0);
			Remove95(ROOTNET, 1);
			Remove95(CLASSNET, 0);
		}/*else*/
		goto done;
	}/*elseif*/

//usageerror
	MessageBox (GetFocus(), USAGEERROR, "", 0 | MB_ICONERROR);
	goto done;
	
		
begin: 
	/*Get global instance handle*/
	g_hInstance = hInstance;

	/* Setup new window class */
	ZeroMemory(&wndclass, sizeof(wndclass));

	wndclass.cbSize			= sizeof(wndclass);
	wndclass.style			= CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc	= WndProc;
	wndclass.cbClsExtra		= 0;
	wndclass.cbWndExtra		= 0;
	wndclass.hInstance		= hInstance;
	wndclass.hIcon			= LoadIcon(hInstance, "lister");
	wndclass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName	= NULL;
	wndclass.lpszClassName	= szAppName;
	wndclass.hIconSm		= LoadIcon(hInstance, "lister");

	/* Register the window class */
	RegisterClassEx(&wndclass);

    //Is there another version of this application running? If so, 
    //  we just bring it to the foreground.
    hwnd = FindWindow (szAppName, NULL);
    if (hwnd) 
    {
        //Found another one!
        if (IsIconic(hwnd)) 
        {
            ShowWindow(hwnd, SW_RESTORE);
        }/*if*/
        SetForegroundWindow (hwnd);
        return FALSE;
    }/*if*/

	/* Create a window using the new window class */
	hwnd = CreateWindow(szAppName,				// window class name
				szAppName,						// window caption
				WS_OVERLAPPEDWINDOW,			// window style
				CW_USEDEFAULT,					// initial x position
				CW_USEDEFAULT,					// initial y position
				CW_USEDEFAULT,					// initial x size
				CW_USEDEFAULT,					// initial y size
				NULL,							// parent window handle
				NULL,							// window menu handle
				hInstance,						// program instance handle
				NULL);							// creation parameters

	/* Start the message loop */
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}/*while*/
	return msg.wParam;

done:
	return FALSE;
}/*end*/


/*
 * Window Procedure
 */
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	/* Use message cracker macros to process messages */
	switch (iMsg)
	{
		case WM_CREATE:
			return HANDLE_WM_CREATE(hwnd, wParam, lParam, Lister_OnCreate);

		case WM_DESTROY:
			return HANDLE_WM_DESTROY(hwnd, wParam, lParam, Lister_OnDestroy);

        case WM_QUIT:
			//Cleanup
			ImageList_Destroy(g_hImages);
			PostQuitMessage(0);
            return TRUE;
	}/*switch*/
	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}/*end*/


/*
 * ListerDlgProc Dialog Procedure
 */
BOOL CALLBACK ListerDlgProc (HWND hwndListerDlg, UINT imsg, WPARAM wParam, LPARAM lParam)
{
	HWND	hListControl = NULL;
	HWND	hOkButton	 = NULL;
	HWND	hHelpButton  = NULL;

	switch (imsg)
    {
		case WM_INITDIALOG:
			HANDLE_WM_INITDIALOG(hwndListerDlg, wParam, lParam, Lister_InitDialog);
			return TRUE;

		case WM_COMMAND:
			HANDLE_WM_COMMAND(hwndListerDlg, wParam, lParam, Lister_OnCommand);
			return TRUE;

		case WM_PAINT:
			//load the question mark icon for the help button
			ghIcon = LoadImage(g_hInstance, MAKEINTRESOURCE (IDI_QMARK), IMAGE_ICON,
					0, 0, LR_DEFAULTCOLOR);
			hHelpButton = GetDlgItem(hwndListerDlg, IDB_HELP);
			SendMessage(hHelpButton, BM_SETIMAGE, (WPARAM)IMAGE_ICON ,(LPARAM)(HANDLE)ghIcon);
			return DefWindowProc(hwndListerDlg, imsg, wParam, lParam);

		case WM_NOTIFY:
			if ((((LPNMHDR) lParam)->code) == LVN_ITEMCHANGED)
			{
				hListControl = GetDlgItem(hwndListerDlg, IDC_ADAPTERLIST);
				hOkButton = GetDlgItem(hwndListerDlg, IDB_PGPNETOK);

				if (SetSelectedItems (hListControl, FALSE))
				{
					Button_Enable(hOkButton, TRUE);
				}/*if*/
				else
				{
					Button_Enable(hOkButton, FALSE);
				}/*else*/
			}/*if*/
			return TRUE;
				
        case WM_QUIT:
			//Cleanup
			ImageList_Destroy(g_hImages);
			EndDialog(hwndListerDlg,0);
		   	PostQuitMessage(0);
            return TRUE;
	
		default:
			return FALSE;
    }/*switch*/
return FALSE;
}/*end*/


/*
 * HelpDlgProc Dialog Procedure
 */
BOOL CALLBACK HelpDlgProc (HWND hwndHelpDlg, UINT imsg, WPARAM wParam, LPARAM lParam)
{
	LPTSTR lpBuffer;

	switch (imsg)
    {
		case WM_COMMAND:
			switch (wParam)
			{
				case IDB_HELPOK:
					EndDialog(hwndHelpDlg,0);
					return TRUE;

			}/*switch*/

		case WM_PAINT:
			lpBuffer = malloc(1024);
			LoadString(g_hInstance,IDS_HELPSTRING, lpBuffer, 1024);
			//SetText
			SetDlgItemText(hwndHelpDlg, IDC_HELPTEXT, lpBuffer);
			free (lpBuffer);
			return DefWindowProc(hwndHelpDlg, imsg, wParam, lParam);

		default:
			return FALSE;
    }/*switch*/
return FALSE;
}/*end*/


/*
 * RebindDlgProc Dialog Procedure
 */
BOOL CALLBACK RebindDlgProc (HWND hwndRebindDlg, UINT imsg, WPARAM wParam, LPARAM lParam)
{
	LPTSTR lpBuffer;


	switch (imsg)
    {
		case WM_COMMAND:
			switch (wParam)
			{
				case IDB_REBINDOK:
					RemoveInf();
					EndDialog(hwndRebindDlg,0);
					RemoveInfFile();
					bDelReg = TRUE;
					RemoveNT();
					RebootYesNo();
					exit(0);
					return TRUE;

				case IDB_REBINDCANCEL:
					EndDialog(hwndRebindDlg,0);
					exit(0);
					return TRUE;
			}/*switch*/

		case WM_PAINT:
			lpBuffer = malloc(1024);

			LoadString(g_hInstance,IDS_REBINDSTRING, lpBuffer, 1024);
			//SetText
			SetDlgItemText(hwndRebindDlg, IDC_REBINDTEXT, lpBuffer);

			LoadString(g_hInstance,IDS_REBINDSTRINGB, lpBuffer, 1024);
			//SetText
			SetDlgItemText(hwndRebindDlg, IDC_REBINDTEXTB, lpBuffer);

			free (lpBuffer);
			return DefWindowProc(hwndRebindDlg, imsg, wParam, lParam);
			
		default:
			return FALSE;
    }/*switch*/
return FALSE;
}/*end*/


/*
 * WM_CREATE Message Handler Function
 */
BOOL Lister_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
	unsigned int RC;


	if ((bIsNT) && (bSansCmdLine))
	{
		RC = CheckNTBindStatus ();

		switch (RC)
		{
			case 1: //no "PGPnetNTSafeToRebind" key exists
				DialogBox(g_hInstance, MAKEINTRESOURCE (IDD_REBINDDIALOG), hwnd, (DLGPROC) RebindDlgProc);
				exit(0);

			case 2: //"PGPnetNeedsReboot
				RebootYesNo ();
				exit(0);

			case 3: //Good to go ...
				break;
			
			default:
				MessageBox (GetFocus(), "Error checking bind status. You need to reinstall product.",
							TITLE, 0 | MB_ICONERROR);
				exit(0);
				break;
		}/*switch*/
	}/*if*/

	DialogBox(lpCreateStruct->hInstance, MAKEINTRESOURCE (IDD_LISTERDLG), hwnd, (DLGPROC) ListerDlgProc);
	FORWARD_WM_CREATE(hwnd, lpCreateStruct, DefWindowProc);
	return TRUE;
}/*end*/


/*
 * WM_DESTROY Message Handler Function
 */
void Lister_OnDestroy(HWND hwnd)
{
	PostQuitMessage(0);
	return;
}/*end*/


/*
 * ListerInitDialog Message Handler Function
 */
BOOL Lister_InitDialog(HWND hwndDlg, HWND hwnd, LPARAM lParam)
{
	//common
	HKEY			hMainKey	= HKEY_LOCAL_MACHINE;
	HKEY			hNewKey		= NULL;
	HKEY			hkey		= NULL;
	HKEY			phkResult	= NULL;
	LONG			lResult		= ERROR_SUCCESS;
	LPBYTE			pValue		= NULL;
	LPSTR			pszText		= NULL;
	DWORD			dwType;
	DWORD			dwSize;
	UINT			i;
	UINT			nCount;
	BOOL			brc;
	//List view
	LV_ITEM			lviKey;
	HWND			hListControl;
	HWND			hOkButton;


	i = 0;
	//Create the list view
	hListControl = GetDlgItem(hwndDlg, IDC_ADAPTERLIST);
	
	//Create the image list
	brc = CreateImageList();
	if (brc != TRUE)
		MessageBox (GetFocus(), ERR_IMGLISTCREATE, "", 0 | MB_ICONERROR);

	ListView_SetImageList(hListControl, g_hImages, LVSIL_SMALL);
	
	lviKey.mask     = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE ;
	lviKey.iItem    = 0;
	lviKey.iSubItem = 0;
	lviKey.iImage   = 2;
	
	nCount = 0;

	if (bIsNT)
	{
		//Running NT
		lResult = RegOpenKeyEx(hMainKey, TCPIP_SERVICES_KEY, 0, KEY_READ, &hNewKey);
		if (lResult != ERROR_SUCCESS)
		{
			MessageBox (GetFocus(), ERR_OPENREG, "", 0 | MB_ICONERROR);
			return FALSE;
		}/*if*/

		dwSize = 1024;
		pValue = (LPBYTE) calloc(sizeof(BYTE), dwSize);

		lResult = RegQueryValueEx(hNewKey, BINDINGS, 0, &dwType, pValue, &dwSize);
		if (lResult != ERROR_SUCCESS)
		{
			MessageBox (GetFocus(), ERR_READREG, "", 0 | MB_ICONERROR);
			RegCloseKey(hNewKey);
			free(pValue);
			return FALSE;
		}/*if*/

		while (i<dwSize)
		{	
			pszText			= GetTxtAfterLastSlash ((char *) &(pValue[i]));
			lviKey.pszText  = ResolveRealName (pszText);
			
			if ((lviKey.pszText != NULL) && 
				(strstr(lviKey.pszText, "PGPnet") == NULL))
			{
				nCount++;
				ListView_InsertItem(hListControl, &lviKey);
			}

			i += strlen((char *) &(pValue[i])) + 2;
			free(lviKey.pszText);
			free(pszText);
		}/*while*/
		free(pValue);
		RegCloseKey(hNewKey);
	}/*if*/
	else
	{
		//Running 95 or 98
		HKEY			hNewKeyb		= NULL;
		HKEY			hDeviceDesc		= NULL;
		HKEY			hPGPnetkey		= NULL;
		RegSearchState state;
		char			szSubKey[256];
		char			*szName;
		char			*szValue;
		LPTSTR 			pSubkeyBuffer	= NULL;	
		LPBYTE			pData			= NULL;
		DWORD			dwDisposition;
		unsigned long	lpcbMaxValueLen;


		state.hKey = HKEY_LOCAL_MACHINE;
		state.nextState = NULL;
		state.dwKeyIndex = 0;
		state.dwValueIndex = 0;
		state.bMatch = FALSE;
		
		szName = "Class";
		szValue = "Net";
		strcpy(szSubKey, "Enum");

		nCount = 0;
		if (RegOpenKeyEx(state.hKey, "Enum", 0, KEY_READ, &(state.hKey))
			== ERROR_SUCCESS)
		{
			while (RegistrySearch(szName, szValue, szSubKey, &state))
			{
				//Check to see if adapter is bound to TCPIP
				if (AmIBoundToTcpip95 (szSubKey))
				{
					//create the adapter reg inside the PGPnet key
					RegCreateKeyEx(HKEY_LOCAL_MACHINE, PATHTOADAPTERLIST,
					0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hPGPnetkey,
					&dwDisposition);
					
					//Get the DeviceDesc from the Reg. key and
					//Insert the item into List control
					RegOpenKeyEx (HKEY_LOCAL_MACHINE, szSubKey, 0, KEY_READ, &hNewKeyb);
					RegQueryInfoKey (hNewKeyb, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
									NULL, &lpcbMaxValueLen, NULL, NULL);
					pData = malloc(lpcbMaxValueLen + 1);
					lResult = RegQueryValueEx(hNewKeyb, "DeviceDesc", 0,
											  &dwType, pData, &lpcbMaxValueLen);

					//Write the adapter entry under PGPnetkey (REALNAME -> REG KEY)
					RegSetValueEx(hPGPnetkey, pData , 0, REG_SZ, szSubKey,(strlen (szSubKey)+1));

					if (strstr(pData, "PGPnet") == NULL)
					{
						lviKey.pszText  = pData;
						lviKey.iItem	= nCount;
						ListView_Arrange(hListControl, LVA_ALIGNLEFT);
						ListView_InsertItem(hListControl, &lviKey);
						nCount++;
					}/*if*/
					
					/*cleanup*/
					if(hPGPnetkey)
						RegCloseKey(hPGPnetkey);
					if(hNewKeyb)
						RegCloseKey(hNewKeyb);
					if(pData)
						free (pData);
				}/*if*/
				
			}/*while*/
			RegCloseKey(state.hKey);
		}/*if*/
		FreeRegSearchState(&state);
	}/*else*/
	
	//if only 1 adapter select it by default
	if (nCount == 1)
	{
		//there is only 1 adapter bound to TCP/IP
		ListView_SetItemState(hListControl, 0, LVIS_SELECTED, LVIS_SELECTED);
		hOkButton = GetDlgItem(hwndDlg, IDB_PGPNETOK);
		Button_Enable(hOkButton, TRUE);
		
		//if this is a silent install and we just press OK
		if (bSilentInstall)
			PostMessage(hwndDlg, WM_COMMAND, 1004, 0);
	}/*if*/
	return TRUE;
}/*end*/


/*
 * ListerOnCommand [WM_COMMAND]
 */
BOOL Lister_OnCommand(HWND hwndDlg, int id, HWND hwndCommand, UINT Event)
{
	HKEY			hkey			= NULL;
	HKEY			hMainKey		= HKEY_LOCAL_MACHINE;
	HKEY			hOpenKey		= NULL;
	DWORD			dwDisposition;
	int				cchTextMax		= 1024;
	HWND			hListControl	= NULL;
	BOOL			brc				= TRUE;
	LPTSTR			lpBuffer		= NULL;
	LPTSTR 			pszText			= NULL;
	char  			*RealMac		= NULL;
	int				Response;
	int				Count;
	char*			Path;
	
	
	switch (id)
    {
		case IDB_HELP:
			ShowWindow(hwndDlg, SW_HIDE);
			DialogBox(g_hInstance, MAKEINTRESOURCE (IDD_HELPDIALOG), hwndDlg, (DLGPROC) HelpDlgProc);
 			ShowWindow(hwndDlg, SW_SHOW);
			return TRUE;

		case IDB_PGPNETOK:

			hListControl = GetDlgItem(hwndDlg, IDC_ADAPTERLIST);

			if (bIsNT)
			{
				//delete PGPnetNTSafeToRebind entry
				if (RegOpenKeyEx(hMainKey, PATHTOPGPNETREGKEY, 0, KEY_ALL_ACCESS, &hOpenKey)
					== ERROR_SUCCESS)	
				{
					RegDeleteValue(hOpenKey, "PGPnetNTSafeToRebind");
					RegCloseKey (hOpenKey);
				}/*if*/

				//delete Run entry
				if (RegOpenKeyEx(hMainKey, RUNKEY, 0, KEY_ALL_ACCESS, &hOpenKey)
					== ERROR_SUCCESS)	
				{
					RegDeleteValue(hOpenKey, "ZZRunSetAdapter");
					RegCloseKey (hOpenKey);
				}/*if*/

				//Set Runonce entry
				if (RegOpenKeyEx(hMainKey, RUNONCE, 0, KEY_ALL_ACCESS, &hOpenKey)
					== ERROR_SUCCESS)	
				{
					Path = SetExePath();
					strcat(Path, "\\setadapter.exe void");
					GetShortPathName(Path, Path, strlen(Path) + 1);

					RegSetValueEx(hOpenKey, "PGPnetNeedsReboot" , 0, REG_SZ, Path, strlen(Path)+1);
					RegCloseKey (hOpenKey);
					free(Path);
				}/*if*/

				RemoveInf();
				/*We are running NT*/
				SetbWan (hListControl);
				if (SetSelectedItems (hListControl, FALSE))
				{	
					SetCursor(LoadCursor(NULL, IDC_WAIT));
					ShowWindow(hwndDlg, SW_HIDE);
					if (InstallInfFile(hwndDlg))
					{	
						brc = SetSelectedItems (hListControl, TRUE);
						brc = BindReview (hwndDlg);
					}/*if*/
					if(DisableExtraProtocols())
						brc = BindReview (hwndDlg);
					if (bSansCmdLine) 
					{
						RebootYesNo();
					}/*if*/
					SetCursor(LoadCursor(NULL, IDC_ARROW));

					/*If this is not WAN, Set DependOnServiceValue*/
					if(!bWan)
					{
						if (RegOpenKeyEx(hMainKey, PGPMACMPKEY, 0, KEY_ALL_ACCESS, &hOpenKey)
							== ERROR_SUCCESS)
						{
							/*this is REG_MULTI_SZ add the final null*/
							Count = strlen (lpbActualServiceName);
							lpbActualServiceName[Count] = '\0';
							lpbActualServiceName[Count+1] = '\0';

							Count = strlen(lpbActualServiceName) + 2;

							if(lpbActualServiceName)
								RegSetValueEx(hOpenKey, "DependOnService" , 0, REG_MULTI_SZ, lpbActualServiceName,
												Count);
							RegCloseKey (hOpenKey);
							if(lpbActualServiceName)
								free(lpbActualServiceName);
						}/*if*/
					}/*if*/
					brc = SendMessage(hwndDlg, WM_QUIT, (WPARAM)NULL,(LPARAM)NULL);
					AddInf();				
					return TRUE;
				}/*if*/
			}/*if*/
			else
			{
				SetCursor(LoadCursor(NULL, IDC_WAIT));
				/*We are running 95/98*/
				Remove95(CLASSNETTRANS, 0);
				Remove95(ROOTNET, 1);
				Remove95(CLASSNET, 0);
				SetSelectedItems95(hListControl, &RealMac);
				//configure the driver
				ConfigSystemNetTrans();
				ConfigSystemNet();
				ConfigEnumNet();
				ConfigEnumNetwork();
				SetRealMac(RealMac);
				free (RealMac);
				Sleep (1500);
				SetCursor(LoadCursor(NULL, IDC_ARROW));
				ShowWindow(hwndDlg, SW_HIDE);
				SetbWan(NULL);
				if (bSansCmdLine) 
				{
					RebootYesNo();
				}/*if*/				
				brc = SendMessage(hwndDlg, WM_QUIT, (WPARAM)NULL,(LPARAM)NULL);
			}/*else*/

			return TRUE;

		case IDB_PGPNETCANCEL:
			/*if this was ran without command line, it is ok to*/
			/*just quit.*/
			if (bSansCmdLine) 
			{
				SendMessage(hwndDlg, WM_QUIT, (WPARAM)NULL,(LPARAM)NULL);
				return TRUE;
			}/*if*/

			Response = MessageBox (GetFocus(), CONFIRM, "PGPnet",MB_YESNO | MB_ICONQUESTION);
			switch (Response)
			{
			case IDYES:
				MessageBox (GetFocus(), PGPNETCANCELED, "PGPnet", 0 | MB_ICONINFORMATION);

				/*Write the PGPnetCancelled entry*/
				RegCreateKeyEx(HKEY_LOCAL_MACHINE, PATHTOPGPNETREGKEY,
							0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey,
							&dwDisposition);
				RegSetValueEx(hkey, "PGPnetCancelled", 0, REG_SZ, "UserCancelled",
							14);
				RegCloseKey(hkey);
				SendMessage(hwndDlg, WM_QUIT, (WPARAM)NULL,(LPARAM)NULL);
			case IDNO:
				return FALSE;
			}/*switch*/

			return TRUE;

		default:
			return FALSE;
    }/*switch*/
	return TRUE;
}/*end*/


/*
 * 	CreateImageList 
 */
BOOL CreateImageList ()
{
	HBITMAP			hBmp	= NULL;
	HWND			hHelpButton = NULL;
	int				iNumBits= 0;
	HDC				hDC     = NULL;

	hDC = GetDC (NULL);		/*DC for desktop*/
	iNumBits = GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
	ReleaseDC (NULL, hDC);

	/*Check the resolution we are running at*/
	if (iNumBits <= 8) 
	{
		/*Low Res*/
		g_hImages = ImageList_Create (16, 16, ILC_COLOR | ILC_MASK, 
					NUM_BITMAPS, 0); 
		hBmp      = LoadBitmap (g_hInstance, MAKEINTRESOURCE (IDB_IMAGES4BIT));
		ImageList_AddMasked (g_hImages, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
		return TRUE;

	}/*if*/
	else 
	{
		/*High res.*/
		g_hImages = ImageList_Create (16, 16, ILC_COLOR24 | ILC_MASK, 
					NUM_BITMAPS, 0); 
		hBmp      = LoadBitmap (g_hInstance, MAKEINTRESOURCE (IDB_IMAGES24BIT));
		ImageList_AddMasked (g_hImages, hBmp, RGB(255, 0, 255));
		DeleteObject (hBmp);
		return TRUE;
	}/*else*/

	return FALSE;	
}/*end*/


/*
 * 	GetRealNameFromServiceName
 */
LPTSTR ResolveRealName(LPTSTR ServiceName)
{
	HKEY			hMainKey		= HKEY_LOCAL_MACHINE;
	HKEY			hNewKey			= NULL;
	HKEY			hNewKeyb		= NULL;
	LPBYTE			pData			= NULL;
	LPTSTR 			pSubkeyBuffer	= NULL;	
	LPTSTR			KeyTitle        = NULL;
	unsigned long	nSubkeyNameLen;
	unsigned long   MaxValueNameLen;
	unsigned long   MaxDataNameLen;
	unsigned long	dwBufSize;
	DWORD			nSubkeys;
	DWORD			dwIndex;
	DWORD			dwSize;
	DWORD			dwType;
	LONG			lResult			= ERROR_SUCCESS;
	char			*RegKeyBuffer = NULL;

	/*Open key*/
   	if (RegOpenKey(hMainKey, NETWORKCARDS_KEY, &hNewKey) == ERROR_SUCCESS)
	{
		/*Determine number of keys to enumerate*/
		RegQueryInfoKey(hNewKey, NULL, NULL, NULL, &nSubkeys, 
						&nSubkeyNameLen, NULL, NULL, NULL,
						NULL, NULL, NULL);

		/*Retrieve Registry values*/
		pSubkeyBuffer = malloc(nSubkeyNameLen + 1);

		RegKeyBuffer = (char *)malloc(strlen(NETWORKCARDS_KEY) + 4);

		for (dwIndex = 0; dwIndex < nSubkeys; dwIndex++)
		{
			dwBufSize = nSubkeyNameLen + 1;
			lResult = RegEnumKeyEx(hNewKey, dwIndex, pSubkeyBuffer, &dwBufSize, 
							       NULL, NULL, NULL, NULL);
			strcpy(RegKeyBuffer, NETWORKCARDS_KEY);
			strcat(RegKeyBuffer, "\\");
			strcat(RegKeyBuffer, pSubkeyBuffer);

			/*Try to open the key*/
			if (RegOpenKey(hMainKey, RegKeyBuffer, &hNewKeyb) == ERROR_SUCCESS)
			{
				/*Query NETWORKCARDS_KEYVALUE*/
				RegQueryInfoKey(hNewKeyb, NULL, NULL, NULL, NULL, 
							    NULL, NULL, NULL, &MaxValueNameLen,
							    &MaxDataNameLen, NULL, NULL);
				pData = (LPBYTE) calloc(sizeof(BYTE), MaxDataNameLen + 1);
				dwSize = MaxDataNameLen;
				lResult = RegQueryValueEx(hNewKeyb, NETWORKCARDS_KEYVALUE, 0,
										  &dwType, pData, &dwSize);
				/*Check if this is the key we are looking for*/
				if (strcmp(ServiceName, pData) == 0)
				{ 
					free(pData);
					pData  = (LPBYTE) calloc(sizeof(BYTE), MaxDataNameLen);
					dwSize = MaxDataNameLen;
					lResult= RegQueryValueEx(hNewKeyb, NETWORKCARDTITLE, 0,
											  NULL, pData, &dwSize);
					if (lResult != ERROR_SUCCESS)
					{
						MessageBox (GetFocus(), ERR_READREG, "", 0 | MB_ICONERROR);
						RegCloseKey(hNewKeyb);
						free(pData);
						free (RegKeyBuffer);
						free(pSubkeyBuffer);
						RegCloseKey(hNewKey);
						return FALSE;
					}/*if*/
					RegCloseKey(hNewKeyb);
					free (RegKeyBuffer);
					free(pSubkeyBuffer);
					RegCloseKey(hNewKey);/*NETWORKCARDS_KEY*/
					return pData;
				}/*if*/
				free(pData);
				RegCloseKey(hNewKeyb);
			}/*if*/
		}/*for*/
		RegCloseKey(hNewKey);/*NETWORKCARDS_KEY*/
	}/*if*/
	free (RegKeyBuffer);
	free(pSubkeyBuffer);
	return FALSE;
}/*end*/


/*
 * 	GetServiceNameFromRealName
 */
LPTSTR ResolveServiceName(LPTSTR RealName)
{
	HKEY			hMainKey		= HKEY_LOCAL_MACHINE;
	HKEY			hNewKey			= NULL;
	HKEY			hNewKeyb		= NULL;
	LPBYTE			pData			= NULL;
	LPTSTR 			pSubkeyBuffer	= NULL;	
	LPTSTR			KeyTitle        = NULL;
	unsigned long	nSubkeyNameLen;
	unsigned long   MaxValueNameLen;
	unsigned long   MaxDataNameLen;
	unsigned long	dwBufSize;
	DWORD			nSubkeys;
	DWORD			dwIndex;
	DWORD			dwSize;
	DWORD			dwType;
	LONG			lResult			= ERROR_SUCCESS;
	char      		RegKeyBuffer[sizeof(NETWORKCARDS_KEY) + 3];


	/*Open key*/
   	if (RegOpenKey(hMainKey, NETWORKCARDS_KEY, &hNewKey) == ERROR_SUCCESS)
	{
		/*Determine number of keys to enumerate*/
		RegQueryInfoKey(hNewKey, NULL, NULL, NULL, &nSubkeys, 
						&nSubkeyNameLen, NULL, NULL, NULL,
						NULL, NULL, NULL);

		/*Retrieve Registry values*/
		pSubkeyBuffer = malloc(nSubkeyNameLen + 1);
		for (dwIndex = 0; dwIndex < nSubkeys; dwIndex++)
		{
			dwBufSize = nSubkeyNameLen;
			lResult = RegEnumKeyEx(hNewKey, dwIndex, pSubkeyBuffer, &dwBufSize, 
							       NULL, NULL, NULL, NULL);
			strcpy(RegKeyBuffer, NETWORKCARDS_KEY);
			strcat(RegKeyBuffer, "\\");
			strcat(RegKeyBuffer, pSubkeyBuffer);

			/*Try to open the key*/
			if (RegOpenKey(hMainKey, RegKeyBuffer, &hNewKeyb) == ERROR_SUCCESS)
			{
				/*Query NETWORKCARDS_KEYVALUE*/
				RegQueryInfoKey(hNewKeyb, NULL, NULL, NULL, NULL, 
							    NULL, NULL, NULL, &MaxValueNameLen,
							    &MaxDataNameLen, NULL, NULL);

				pData = (LPBYTE) calloc(sizeof(BYTE), MaxDataNameLen + 1);
			
				dwSize = MaxDataNameLen + 1;
				lResult = RegQueryValueEx(hNewKeyb, NETWORKCARDTITLE, 0,
										  &dwType, pData, &dwSize);
				/*Check if this is the key we are looking for*/
				if (strcmp(RealName, pData) == 0)
				{ 
					free(pData);
					pData  = (LPBYTE) calloc(sizeof(BYTE), MaxDataNameLen + 1);

					/*Get actual service name, this is used to set Depend
					  on service value to control load order*/
					if(!bWan)
					{
						lpbActualServiceName = (LPBYTE) calloc(sizeof(BYTE), MaxDataNameLen + 2);
						dwSize = MaxDataNameLen + 2;
						lResult= RegQueryValueEx(hNewKeyb, REALSERVICENAMEKEY, 0,
											  NULL, lpbActualServiceName, &dwSize);
					}/*if*/
					
					dwSize = MaxDataNameLen + 1;
					/*get the instance name ie: PGPMacMP#*/
					lResult= RegQueryValueEx(hNewKeyb, NETWORKCARDS_KEYVALUE, 0,
											  NULL, pData, &dwSize);
					if (lResult != ERROR_SUCCESS)
					{
						MessageBox (GetFocus(), ERR_READREG, "", 0 | MB_ICONERROR);
						RegCloseKey(hNewKeyb);
						free(pData);
						free(pSubkeyBuffer);
						RegCloseKey(hNewKey);
						return FALSE;
					}/*if*/
					RegCloseKey(hNewKeyb);
					free(pSubkeyBuffer);
					RegCloseKey(hNewKey);/*NETWORKCARDS_KEY*/
					return pData;
				}/*if*/
				free(pData);
				RegCloseKey(hNewKeyb);
			}/*if*/
		}/*for*/
		RegCloseKey(hNewKey);/*NETWORKCARDS_KEY*/
	}/*if*/
	return FALSE;
}/*end*/


/*
 * 	GetTxtAfterLastSlash
 */
LPTSTR GetTxtAfterLastSlash(LPTSTR Text)
{
	int		i				= 0;
	LPSTR	pszText			= NULL;
	
	while (*Text != '\0')
	{
		*Text ++;
	}/*while*/

	/* *ServiceName is now at the end of the string*/
	while (*Text != '\\')
	{
		*Text --;
	}/*while*/

	*Text ++;/*We do not want the '/'*/
	i = strlen(Text);
	pszText = malloc (i + 1);
	strcpy(pszText,Text);
	return pszText;
}/*end*/	


/*
 * 	GetTxtAfterFirstSlash
 */
LPTSTR GetTxtAfterFirstSlash(LPTSTR Text)
{
	int		i				= 0;
	LPSTR	pszText			= NULL;
	
	while (*Text != '\\')
	{
		*Text ++;
	}/*while*/

	*Text ++;/*We do not want the '/'*/
	i = strlen(Text);
	pszText = malloc (i + 1);
	strcpy(pszText,Text);
	return pszText;
}/*end*/


/*
 * 	SetSelectedItems
 */
BOOL SetSelectedItems(HWND hwnd, BOOL SetBindings)
{
	HKEY	hkey;
	DWORD	dwDisposition;
	UINT	NumberOfItems;
	int		Flag;
	DWORD	dwIndex			= 0;
	int     cchTextMax		= 1024;
	LPSTR	pszText			= NULL;
	LPSTR	pszServiceName	= NULL;

	NumberOfItems = ListView_GetSelectedCount(hwnd);

	if (NumberOfItems == 0)
		return FALSE;
	
	NumberOfItems = ListView_GetItemCount(hwnd);

	pszText		= malloc(cchTextMax);

	for (dwIndex = 0; dwIndex < NumberOfItems; dwIndex++)
	{
		ListView_GetItemText(hwnd, dwIndex, 0, pszText, cchTextMax);
		Flag = ListView_GetItemState(hwnd, dwIndex, LVNI_SELECTED);		

		if (Flag == 2)
		{
			if(SetBindings)
			{
				pszServiceName = ResolveServiceName(pszText);
				
				/*Write the FullSecuredCard reg entry*/
				RegCreateKeyEx(HKEY_LOCAL_MACHINE, PATHTOPGPNETREGKEY,
							0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey,
							&dwDisposition);
				RegSetValueEx(hkey, "FullSecuredCard", 0, REG_SZ, pszText,
							strlen(pszText) + 1);
				RegCloseKey(hkey);
				/*Write the SecuredCard reg entry*/
				RegCreateKeyEx(HKEY_LOCAL_MACHINE, PATHTOPGPNETREGKEY,
							0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey,
							&dwDisposition);
				RegSetValueEx(hkey, "SecuredCard", 0, REG_SZ, pszServiceName,
							strlen(pszServiceName) + 1);
				RegCloseKey(hkey);
				free(pszServiceName);
			}/*if*/
		}/*if*/
	}/*for*/
	free (pszText);
	return TRUE;
}/*end*/


/*
 * 	SetSelectedItems95
 */
BOOL SetSelectedItems95(HWND hwnd, LPSTR *RealMac)
{
	HKEY	hkey;
	DWORD	dwDisposition;
	UINT	NumberOfItems;
	int		Flag;
	DWORD	dwIndex			= 0;
	int     cchTextMax		= 1024;
	LPSTR	pszText			= NULL;
	LPSTR	pszServiceName	= NULL;

	NumberOfItems = ListView_GetSelectedCount(hwnd);

	if (NumberOfItems == 0)
		return FALSE;
	
	NumberOfItems = ListView_GetItemCount(hwnd);

	pszText		= malloc(cchTextMax);

	for (dwIndex = 0; dwIndex < NumberOfItems; dwIndex++)
	{
		ListView_GetItemText(hwnd, dwIndex, 0, pszText, cchTextMax);
		Flag = ListView_GetItemState(hwnd, dwIndex, LVNI_SELECTED);		

		if (Flag == 2)
		{
			/*Write the FullSecuredCard reg entry*/
			RegCreateKeyEx(HKEY_LOCAL_MACHINE, PATHTOADAPTERLIST,
						0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey,
						&dwDisposition);
			RegSetValueEx(hkey, "FullSecuredCard", 0, REG_SZ, pszText,
					strlen(pszText) + 1);
			RegCloseKey(hkey);
			*RealMac = (char *) malloc (strlen(pszText) + 1);
			ZeroMemory(*RealMac, (strlen(pszText) + 1));
			strcpy(*RealMac, pszText);
		}/*if*/
	}/*for*/
	free (pszText);
	return TRUE;
}/*end*/


/*
 * 	SetbWan
 */
BOOL SetbWan(HWND hwnd)
{
	/*NT*/
	UINT			NumberOfItems;
	int				Flag;
	DWORD			dwIndex			= 0;
	int				cchTextMax		= 1024;
	LPSTR			pszText			= NULL;
	LPSTR			pszServiceName	= NULL;
	/*9598*/
	HKEY			hMainKey		= HKEY_LOCAL_MACHINE;
	HKEY			hNewKeyb		= NULL;
	HKEY			hkey			= NULL;
	unsigned long   MaxValueNameLen;
	unsigned long   MaxDataNameLen;
	LPBYTE			pData			= NULL;
	LPBYTE			pDatab			= NULL;
	DWORD			dwType;
	DWORD			dwSize;
	DWORD			dwDisposition;


	if (bIsNT)
	{
		NumberOfItems = ListView_GetSelectedCount(hwnd);

		if (NumberOfItems == 0)
			return FALSE;
		
		NumberOfItems = ListView_GetItemCount(hwnd);

		pszText	      = malloc(cchTextMax);

		for (dwIndex = 0; dwIndex < NumberOfItems; dwIndex++)
		{
			ListView_GetItemText(hwnd, dwIndex, 0, pszText, cchTextMax);
			Flag = ListView_GetItemState(hwnd, dwIndex, LVNI_SELECTED);		

			if (Flag == 2)
			{
				bWan = TRUE;
				if (strstr (pszText, "WAN") == NULL)
					if (strstr (pszText, "Wan") == NULL)
						if (strstr (pszText, "wan") == NULL)
							bWan = FALSE;
			}/*if*/
		}/*for*/
		free (pszText);
		return TRUE;
	}/*if*/
	else/*running 9598*/
	{
		/*Try to open the key*/
		if (RegOpenKey(hMainKey, PATHTOADAPTERLIST, &hNewKeyb) == ERROR_SUCCESS)
		{
			/*Query NETWORKCARDS_KEYVALUE*/
			RegQueryInfoKey(hNewKeyb, NULL, NULL, NULL, NULL, 
							NULL, NULL, NULL, &MaxValueNameLen,
							&MaxDataNameLen, NULL, NULL);
			pData = (LPBYTE) calloc(sizeof(BYTE), MaxDataNameLen + 1);
			pDatab = (LPBYTE) calloc(sizeof(BYTE), MaxDataNameLen + 1);
			dwSize = MaxDataNameLen;
			RegQueryValueEx(hNewKeyb, "FullSecuredCard", 0,
									  &dwType, pData, &dwSize);
			dwSize = MaxDataNameLen;
			RegQueryValueEx(hNewKeyb, (LPSTR)pData, 0,
									  &dwType, pDatab, &dwSize);

			RegCloseKey(hNewKeyb);

			if(strstr((LPSTR)pDatab, "Enum\\Root\\Net") == NULL)
				bWan = FALSE;
			else
				bWan = TRUE;

			free(pData);
			free(pDatab);
				
			/*WAN*/
			if (bWan)
			{
				RegCreateKeyEx(HKEY_LOCAL_MACHINE, PATHTOPGPNETREGKEY,
							0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey,
							&dwDisposition);
				RegSetValueEx(hkey, "CardType", 0, REG_SZ, "NDISWAN",
							8);
				RegCloseKey(hkey);
			}/*if*/
			/*LAN*/
			else
			{
				RegCreateKeyEx(HKEY_LOCAL_MACHINE, PATHTOPGPNETREGKEY,
							0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey,
							&dwDisposition);
				RegSetValueEx(hkey, "CardType", 0, REG_SZ, "NDIS",
							5);
				RegCloseKey(hkey);
			}/*else*/
		}/*if*/
		return TRUE;
	}/*else*/
}/*end*/

/*
 *	Check for correct version of Windows
 */
BOOL IsCorrectVersion(VOID) 
{
	OSVERSIONINFO osid;

	osid.dwOSVersionInfoSize = sizeof (osid);
	GetVersionEx (&osid);
	bIsNT = FALSE;

	switch (osid.dwPlatformId) 
	{
		case VER_PLATFORM_WIN32s :
			return FALSE;

		/*Windows 95*/
		case VER_PLATFORM_WIN32_WINDOWS :
 			return TRUE;

		/*Windows NT*/
		case VER_PLATFORM_WIN32_NT :
		{
			if (osid.dwMajorVersion >= 4) 
			{
				bIsNT = TRUE;
				return TRUE;
			}/*if*/
			return FALSE;
		}/*case*/
		default:
			return FALSE;
	}/*switch*/
}/*end*/


/*
 *  Invokes a binding review 	
 */
BOOL BindReview (HWND hwnd)
{
	FARPROC		fpExportedFunction	= NULL;
	HMODULE		hNetcfgDll			= NULL;
	LPCTSTR		lpModuleName		= NETCFGDLL;
	BOOL		brc;
	int			tResult;

	hNetcfgDll = LoadLibrary(lpModuleName); 

	if (hNetcfgDll == NULL)
		return FALSE;

	fpExportedFunction = (NetReviewFunc *) GetProcAddress(hNetcfgDll, SZNETREVIEW);
	if(fpExportedFunction == NULL)
	{
		brc = FreeLibrary(hNetcfgDll);
		return FALSE;
	}/*if*/
	tResult = (*fpExportedFunction) (hwnd, 0);
	brc = FreeLibrary(hNetcfgDll);
	return TRUE;
}/*end*/

/*
 *  Installs an NT INF file (assumed in same directory)	
 */
BOOL InstallInfFile (HWND hwnd)
{
	HKEY		hkey;
	FARPROC		fpExportedFunction	= NULL;
	HMODULE		hNetcfgDll			= NULL;
	LPCTSTR		lpModuleName		= NETCFGDLL;
	UINT		nBufferLengthMax	= 1024;
	BOOL		brc;
	DWORD		dwReturn;
	DWORD		dwDisposition;
	int			tResult; 
	wchar_t		*pwcPathofInf		= NULL;
	wchar_t		*pwcPathtoInf		= NULL;
	LPTSTR		PathofInf;
	LPTSTR		PathtoInf;
	int			i;

	/*Get current dir.*/

	PathofInf = SetExePath();
	PathtoInf = (char *)malloc(strlen(PathofInf) + 20);

	strcat(PathofInf, "\\");
	strcpy(PathtoInf, PathofInf);
	strcat(PathtoInf, INFNAME);
	
	//Get the short path (no spaces)
	GetShortPathName(PathofInf, PathofInf, strlen(PathofInf) + 1); 
	GetShortPathName(PathtoInf, PathtoInf, strlen(PathtoInf) + 1);

	SetCurrentDirectory(PathofInf);

	/*Convert path of inf to wide char*/
	pwcPathofInf = malloc((strlen(PathofInf) + 1)*2);
	i = mbstowcs(pwcPathofInf, PathofInf, strlen(PathofInf) + 1);
	/*Convert path to the inf to wide char*/
	pwcPathtoInf = malloc((strlen(PathtoInf) + 1)*2);
	i = mbstowcs(pwcPathtoInf, PathtoInf, strlen(PathtoInf) + 1);

	hNetcfgDll = LoadLibrary(lpModuleName); 

	if (hNetcfgDll == NULL)
		return FALSE;

	fpExportedFunction = (NetInstallFunc *) GetProcAddress(hNetcfgDll, SZNETINSTALL);

	if(fpExportedFunction == NULL)
	{
		brc = FreeLibrary(hNetcfgDll);
		return FALSE;
	}/*if*/
	
	/*WAN*/
	if (bWan)
	{
		RegCreateKeyEx(HKEY_LOCAL_MACHINE, PATHTOPGPNETREGKEY,
					0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey,
					&dwDisposition);
		RegSetValueEx(hkey, "CardType", 0, REG_SZ, "NDISWAN",
					8);
		RegCloseKey(hkey);
		tResult = (*fpExportedFunction) (hwnd, L"PgpMac" , pwcPathtoInf, pwcPathofInf, "", INFINSTALL_SUPPORTED | INFINSTALL_INPROCINTERP,
								&dwReturn);
	}/*if*/
	/*LAN*/
	else
	{
		RegCreateKeyEx(HKEY_LOCAL_MACHINE, PATHTOPGPNETREGKEY,
					0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey,
					&dwDisposition);
		RegSetValueEx(hkey, "CardType", 0, REG_SZ, "NDIS",
					5);
		RegCloseKey(hkey);
		tResult = (*fpExportedFunction) (hwnd, L"PgpMac" , pwcPathtoInf, pwcPathofInf, "", INFINSTALL_SUPPORTED | INFINSTALL_INPROCINTERP,
									&dwReturn);
	}/*else*/

	free (PathofInf);
	free (PathtoInf);
	free (pwcPathofInf);
	free (pwcPathtoInf);
	brc = FreeLibrary(hNetcfgDll);
	if (tResult != 0)
		return FALSE;
	return TRUE;
}/*end*/

/*
 *  Removes an NT INF file (assumed in same directory)	
 */
BOOL RemoveInfFile ()
{
	FARPROC		fpExportedFunction	= NULL;
	HMODULE		hNetcfgDll			= NULL;
	LPCTSTR		lpModuleName		= NETCFGDLL;
	LPTSTR		lpBuffer			= NULL;
	UINT		nBufferLengthMax	= 1024;
	BOOL		brc;
	DWORD		dwReturn;
	int			tResult; 
	HWND		hDesktop;

	RemoveInf();

	hDesktop = GetDesktopWindow();
	
	hNetcfgDll = LoadLibrary(lpModuleName); 

	if (hNetcfgDll == NULL)
		return FALSE;

	fpExportedFunction = (NetRemoveFunc *) GetProcAddress( hNetcfgDll, SZNETREMOVE );

	if(fpExportedFunction == NULL)
	{
		brc = FreeLibrary(hNetcfgDll);
		return FALSE;
	}/*if*/

	tResult = (*fpExportedFunction) (hDesktop, L"PgpMac", INFINSTALL_SUPPORTED | INFINSTALL_INPROCINTERP,
									  &dwReturn);

	brc = BindReview (hDesktop);

	brc = FreeLibrary(hNetcfgDll);
	if (tResult != 0)
		return FALSE;
	return TRUE;
}/*end*/

/*
 * 	DisableBinding
 */
BOOL DisableBinding(LPTSTR Binding)
{
	HKEY	hkey;
	LPTSTR  lpFullBinding;
	DWORD	MaxData = 1024;

	/*Allocate memory*/
	lpFullBinding =  malloc(strlen(KEYSTUB) + strlen(Binding) + 1);
	/*Build value to write to disable key*/
	strcpy(lpFullBinding, KEYSTUB);
	strcat(lpFullBinding, Binding);

	/*Open Tcpip disable key key*/
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, KEYTCPIPDISABLE, 0, KEY_SET_VALUE, &hkey);

	/*Set key value*/
	RegSetValueEx(hkey, BIND, 0, REG_MULTI_SZ, lpFullBinding,
				strlen(lpFullBinding) + 1);
	RegCloseKey(hkey);
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, KEYNETBTDISABLE, 0, KEY_SET_VALUE, &hkey);
	RegSetValueEx(hkey, BIND, 0, REG_MULTI_SZ, lpFullBinding,
				strlen(lpFullBinding) + 1);
	RegCloseKey(hkey);
	free (lpFullBinding);
	free (Binding);
	return TRUE;
}/*end*/

/*
 * 	AmIBoundToTcpip95
 */
BOOL AmIBoundToTcpip95 (char *Key)
{
	HKEY	hkey = NULL;
	UINT	nValues;
	LPSTR	pValueBuffer;
	DWORD	dwIndex;
	DWORD	nValueNameLen;
	DWORD   nValueNameLenb;
	LONG	lResult;
	BOOL	Pass;
	char	*BaseKey = NULL;

	Pass = FALSE;
	if(!Key)
		return FALSE;
	BaseKey = (char *)malloc(strlen(Key) + 12);
	ZeroMemory(BaseKey, (strlen(Key) + 12)); 

	strcat(BaseKey, Key);	
	strcat(BaseKey, "\\Bindings");
	/*Open key*/
	if (RegOpenKey(HKEY_LOCAL_MACHINE, BaseKey, &hkey) == ERROR_SUCCESS)
	{
		/*Determine number of keys to enumerate*/
		RegQueryInfoKey(hkey, NULL, NULL, NULL, NULL, 
						NULL, NULL, &nValues, &nValueNameLen,
						NULL, NULL, NULL);

		/*Retrieve Registry values*/
		pValueBuffer = malloc(nValueNameLen + 1);
		for (dwIndex = 0; dwIndex < nValues; dwIndex++)
		{
			nValueNameLenb = nValueNameLen + 1;
			ZeroMemory(pValueBuffer,(nValueNameLen + 1)); 
			lResult = RegEnumValue( hkey, dwIndex, pValueBuffer, 
								&nValueNameLenb, NULL, NULL, NULL, NULL); 
			if (strstr(pValueBuffer, "MSTCP"))
				Pass = TRUE;
		}/*for*/
	free(pValueBuffer);
	RegCloseKey (hkey);
	}/*if*/
	free(BaseKey);

	if (Pass)
		return TRUE;
	return FALSE;
}/*end*/


/*
 * 	RebootYesNo
 */
BOOL RebootYesNo ()
{
	int RC;

	if(bIsNT)
	{
		TOKEN_PRIVILEGES TpNew;
		HANDLE           hToken;
		LUID             ShutDownValue;

		OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES |
			TOKEN_QUERY, &hToken);
		LookupPrivilegeValue( (LPSTR)NULL, SE_SHUTDOWN_NAME, &ShutDownValue);

		TpNew.PrivilegeCount = 1;
		TpNew.Privileges[0].Luid = ShutDownValue;
		TpNew.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		AdjustTokenPrivileges( hToken, FALSE, &TpNew, sizeof(TpNew),
			(TOKEN_PRIVILEGES *)NULL, (DWORD *)NULL);
	}

	RC = MessageBox (GetFocus(), STDREBOOTMSG, "Network Settings Change", MB_ICONWARNING | MB_YESNO);
	switch (RC)
	{
		case IDYES:
			ExitWindowsEx (EWX_REBOOT, 0);
			return TRUE;

		case IDNO:
			return FALSE;

		default:
			return FALSE;
	}/*switch*/
}

/*
 * 	PutSAinRunKey
 */
BOOL PutSAinRunKey()
{
	char*			CommandLine;
	HKEY			hMainKey		= HKEY_LOCAL_MACHINE;
	HKEY			hOpenKey		= NULL;
	LPBYTE			pValue			= NULL;

	CommandLine = SetExePath();
	strcat (CommandLine, "\\SetAdapter.exe");
	GetShortPathName(CommandLine, CommandLine, strlen(CommandLine) + 1);
	strcat (CommandLine, " runonce");

	//Set Runonce entry
	if (RegOpenKeyEx(hMainKey, RUNKEY, 0, KEY_ALL_ACCESS, &hOpenKey)
		== ERROR_SUCCESS)	
	{
		RegSetValueEx(hOpenKey, "ZZRunSetAdapter" , 0, REG_SZ, CommandLine, strlen(CommandLine)+1);
		RegCloseKey (hOpenKey);
	}/*if*/
	free (CommandLine);
	return TRUE;
}



/*
 * 	PutSAinRunKey
 */
BOOL NeedReboot()
{
	HKEY			hMainKey		= HKEY_LOCAL_MACHINE;
	HKEY			hOpenKey		= NULL;
	LPSTR			Path;
	//Set Runonce entry
	if (RegOpenKeyEx(hMainKey, RUNONCE, 0, KEY_ALL_ACCESS, &hOpenKey)
		== ERROR_SUCCESS)	
	{
		Path = SetExePath();
		strcat(Path, "\\setadapter.exe void");
		GetShortPathName(Path, Path, strlen(Path) + 1);

		RegSetValueEx(hOpenKey, "PGPnetNeedsReboot" , 0, REG_SZ, Path, strlen(Path)+1);
		RegCloseKey (hOpenKey);
		free(Path);
	}/*if*/
	return TRUE;
}


/*
 * 	SetExePath
 */
char* SetExePath()
{
	char*		CommandLine;
	LPSTR		pstring;
	char*		pstringb;
	int			index;

	pstring = (char*)malloc(1024);
	ZeroMemory (pstring, 1024);
	GetModuleFileName(NULL, pstring, 1024);
	GetShortPathName(pstring, pstring, strlen(pstring) + 1);

	index = strlen(pstring);
	//malloc index + 10 (10 needed for runonce)
	CommandLine = (char*)malloc (index + 10);
	ZeroMemory (CommandLine, index + 10);
	pstringb = pstring;

	while (*pstring == '"')
	{
		*pstring ++;
	}

	*pstringb = *pstring;

	while (*pstringb != '\0')
	{
		*pstringb ++;
	}
	while (*pstringb != '\\')
	{
		*pstringb --;
	}

	for (index = 0; pstring != pstringb; index ++)
	{
		CommandLine[index] = pstring[0];
		pstring++;
	}
	CommandLine[index] = '\0';
	return CommandLine;
}



/*To install inf file:

fpNetInstall = (NetInstallFunc *) GetProcAddress( hNetcfgDll, SZNETINSTALL );

tResult = ( *fpNetInstall ) ( hWindow,
TZPRODUCTNAME,
szInfPath,
szSrcPath,
NULL,
INFINSTALL_SUPPORTED | INFINSTALL_INPROCINTERP,
&dwReturn );


char szMsg[200];
LoadString(g_hInst, IDS_WRONGOS, szMsg, sizeof(szMsg));
MessageBox (NULL, szMsg, g_szAppName, 
MB_OK|MB_ICONEXCLAMATION);
*/


































