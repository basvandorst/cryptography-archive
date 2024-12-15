/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: WFwiz.c,v 1.27.8.1 1998/11/12 03:13:32 heller Exp $
____________________________________________________________________________*/
#include "precomp.h"

BOOL CALLBACK IntroDlgProc(HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK InfoDlgProc(HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK StatusDlgProc(HWND,UINT,WPARAM,LPARAM);
BOOL CALLBACK FinishDlgProc(HWND,UINT,WPARAM,LPARAM);
void WipeThread(void* pvoid);

BOOL g_bWiping			= FALSE;
HWND g_status			= NULL;
HWND g_wipe_progress	= NULL;
HWND g_pass_progress	= NULL;
HWND g_pass_status		= NULL;
HFONT g_boldfont, g_defaultfont;

static HPALETTE 
CreateDIBPalette (LPBITMAPINFO lpbmi, 
				  LPINT lpiNumColors);

static HBITMAP 
LoadResourceBitmap (HINSTANCE hInstance, 
					LPSTR lpString,
					HPALETTE FAR* lppalette);

void FreeSpaceWipeWizard(HWND hwnd, void* PGPsc)
{
	PROPSHEETPAGE	psp[NUM_PAGES];
	PROPSHEETHEADER psh;
	int				index;
	wizard_data		data;
	LOGFONT			lf;
	PGPError		error;
	PGPPrefRef		prefRef;
	PGPMemoryMgrRef memoryMgr;
	PGPUInt32		passes = 1;
	HDC				hdc = NULL;
	int				bits, bmp;
	PGPContextRef	context;

	context=(PGPContextRef)PGPsc;

	memoryMgr = PGPGetContextMemoryMgr(context);

	error = PGPclOpenClientPrefs(	memoryMgr,
									&prefRef );

	if(IsntPGPError(error) )
	{
		PGPGetPrefNumber (	prefRef,
							kPGPPrefDiskWipePasses, 
							(PGPUInt32*)&passes);

		PGPclCloseClientPrefs (prefRef, FALSE);

	}

	// initialize the wizard data
	memset(&data, 0x00, sizeof(data));
	data.cookie = 0;
	data.passes = passes;
	data.hinst = g_hinst;
	data.context = context;

	// Determine which bitmap will be displayed in the wizard

	hdc = GetDC (NULL);		// DC for desktop
	bits = GetDeviceCaps (hdc, BITSPIXEL) * GetDeviceCaps (hdc, PLANES);
	ReleaseDC (NULL, hdc);

	if (bits <= 1)
		bmp = IDB_WIPEWIZ1;
	else if (bits <= 8) 
		bmp = IDB_WIPEWIZ4;
//	else if (bits <= 8)     // No pallette cycling -wjb
//		bmp = IDB_WIPEWIZ8;
	else 
		bmp = IDB_WIPEWIZ8; // save some space <grin>
		//bmp = IDB_WIPEWIZ24; 

	data.bmp = LoadResourceBitmap(g_hinst, MAKEINTRESOURCE(bmp),
						&(data.palette));


	// Set the PROPSHEETPAGE values common to all pages

	for (index=0; index < NUM_PAGES; index++)
	{
		psp[index].dwSize		= sizeof(PROPSHEETPAGE);
		psp[index].dwFlags		= PSP_DEFAULT;
		psp[index].hInstance	= g_hinst;
		psp[index].pszTemplate	= NULL;
		psp[index].hIcon		= NULL;
		psp[index].pszTitle		= NULL;
		psp[index].pfnDlgProc	= NULL;
		psp[index].lParam		= (LPARAM) &data;
		psp[index].pfnCallback	= NULL;
		psp[index].pcRefParent	= NULL;
	}

	// Set up the intro page
	psp[intro].pszTemplate	= MAKEINTRESOURCE(IDD_INTRO);
	psp[intro].pfnDlgProc	= (DLGPROC) IntroDlgProc;

	// Set up the info page
	psp[info].pszTemplate	= MAKEINTRESOURCE(IDD_INFO);
	psp[info].pfnDlgProc	= (DLGPROC) InfoDlgProc;

	// Set up the status page
	psp[status].pszTemplate	= MAKEINTRESOURCE(IDD_STATUS);
	psp[status].pfnDlgProc	= (DLGPROC) StatusDlgProc;

	// Set up the finish page
	psp[finish].pszTemplate	= MAKEINTRESOURCE(IDD_FINISH);
	psp[finish].pfnDlgProc	= (DLGPROC) FinishDlgProc;

	// Fill out the header
	psh.dwSize		= sizeof(PROPSHEETHEADER);
	psh.dwFlags		= PSH_WIZARD | PSH_PROPSHEETPAGE;
	psh.hwndParent	= hwnd;
	psh.hInstance	= g_hinst;
	psh.hIcon		= NULL;
	psh.pszCaption	= NULL;
	psh.nPages		= NUM_PAGES;
	psh.nStartPage	= intro;
	psh.ppsp		= psp;
	psh.pfnCallback	= NULL;

	// set some convenient globals
	g_defaultfont = GetStockObject(DEFAULT_GUI_FONT);

	GetObject(g_defaultfont, sizeof(LOGFONT), &lf);

	lf.lfWeight = FW_BOLD;

	g_boldfont = CreateFontIndirect(&lf);

	// Execute the Wizard - doesn't return until Cancel or Finish
	PropertySheet(&psh);

	DeleteObject(g_boldfont);

}

BOOL CALLBACK IntroDlgProc(	HWND hwnd, 
							UINT msg, 
							WPARAM wParam, 
							LPARAM lParam)
{
	BOOL bReturnValue = FALSE;
	static wizard_data* data = NULL;

	switch(msg)
	{
		case WM_INITDIALOG:
		{
			PROPSHEETPAGE* psp = (PROPSHEETPAGE*) lParam;
			HWND welcome = GetDlgItem(hwnd, IDC_WELCOME);
			HWND hwndParent = GetParent(hwnd);
			int x,y;
			RECT rect;

			data = (wizard_data*) psp->lParam;

			SendMessage(welcome, 
						WM_SETFONT, 
						(WPARAM)g_boldfont,
						MAKELPARAM(TRUE, 0));

			// center the wizard on the screen
			GetClientRect(hwndParent, &rect);

			x = (GetSystemMetrics (SM_CXFULLSCREEN) - rect.right) / 2;
			y = (GetSystemMetrics (SM_CYFULLSCREEN) - rect.bottom) / 2;

			SetWindowPos (	hwndParent, 
							HWND_TOP, 
							x, 
							y, 
							0,
							0,
							SWP_NOSIZE);

			bReturnValue = TRUE;
			break;
		}

		case WM_DESTROY:
		{
			break;
		}

		case WM_PAINT:
		{
			if (data->palette)
			{
				PAINTSTRUCT ps;
				HDC	hDC = BeginPaint (hwnd, &ps);
				SelectPalette (hDC, data->palette, FALSE);
				RealizePalette (hDC);
				EndPaint (hwnd, &ps);
				bReturnValue = TRUE;
			}

			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR pnmh;

			pnmh = (LPNMHDR) lParam;

			switch(pnmh->code)
			{
				case PSN_SETACTIVE:
				{
					// Initialize window
					PostMessage(GetParent(hwnd), 
								PSM_SETWIZBUTTONS, 
								0, 
								PSWIZB_NEXT);

					SendDlgItemMessage(	hwnd, 
										IDC_WIZBITMAP, 
										STM_SETIMAGE, 
										IMAGE_BITMAP, 
										(LPARAM) data->bmp);

					bReturnValue = TRUE;
					break;
				}
			}
			
			break;
		}

	}
	return bReturnValue;
}


BOOL CALLBACK InfoDlgProc(	HWND hwnd, 
							UINT msg, 
							WPARAM wParam, 
							LPARAM lParam)
{
	BOOL bReturnValue = FALSE;
	static wizard_data* data = NULL;
	static HWND drivelist = NULL;
	static HWND numpasses = NULL;

	switch(msg)
	{
		case WM_INITDIALOG:
		{
			PROPSHEETPAGE* psp = (PROPSHEETPAGE*) lParam;
			char buf[512] = {0x00};
			int len;
			char *drive;
			HWND spin = GetDlgItem(hwnd, IDC_SPIN1);

			SendMessage(spin, 
						UDM_SETRANGE, 
						0, 
						MAKELPARAM(32, 1));  

			drivelist = GetDlgItem(hwnd, IDC_DRIVECOMBO);
			numpasses = GetDlgItem(hwnd, IDC_NUMPASSES);

			data = (wizard_data*) psp->lParam;

			SetWindowLong(hwnd, GWL_USERDATA, (LPARAM) data);

			len = GetLogicalDriveStrings(sizeof(buf), buf);

			drive = buf;

			while(*drive)
			{
				*drive=(char)toupper((int)*drive);

				SendMessage(drivelist,
							CB_ADDSTRING,
							0,
							(LPARAM)drive);

				drive += strlen(drive) + 1;
			}

			SendMessage(drivelist,
						CB_SETCURSEL,
						0,
						0);

			SendMessage(numpasses,
						EM_SETLIMITTEXT,
						2,
						0);

			wsprintf(buf, "%d", data->passes);

			SetWindowText(numpasses, buf);
			 
			bReturnValue = TRUE;
			break;
		}

		case WM_COMMAND:
		{
			switch LOWORD(wParam)
			{
				case IDC_STATIC_1:
				case IDC_STATIC_2:
				case IDC_STATIC_3:
				case IDC_STATIC_4:
				{
					if( HIWORD(wParam) == BN_CLICKED )
					{
						short alt = GetKeyState( VK_MENU ) & 0x8000; 
						short shift = GetKeyState( VK_SHIFT ) & 0x8000; 
						short ctrl = GetKeyState( VK_CONTROL ) & 0x8000; 


						if(alt &&  shift && ctrl)
						{
							SetWindowText(GetDlgItem(hwnd, IDC_STATIC_1), "Hugh Heffner:");
							SetWindowText(GetDlgItem(hwnd, IDC_STATIC_2), "Go! Computing:");
							SetWindowText(GetDlgItem(hwnd, IDC_STATIC_3), "Theodore Kaczynski:");
							SetWindowText(GetDlgItem(hwnd, IDC_STATIC_4), "Benedict Arnold:");
						}
					}
					break;
				}
			}
			break;
		}

		case WM_PAINT:
		{
			if (data->palette)
			{
				PAINTSTRUCT ps;
				HDC	hDC = BeginPaint (hwnd, &ps);
				SelectPalette (hDC, data->palette, FALSE);
				RealizePalette (hDC);
				EndPaint (hwnd, &ps);
				bReturnValue = TRUE;
			}

			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR pnmh;

			pnmh = (LPNMHDR) lParam;

			switch(pnmh->code)
			{
				case PSN_SETACTIVE:
				{
					// Initialize window
					PostMessage(GetParent(hwnd), 
								PSM_SETWIZBUTTONS, 
								0, 
								PSWIZB_NEXT | PSWIZB_BACK);

					SendDlgItemMessage(	hwnd, 
										IDC_WIZBITMAP, 
										STM_SETIMAGE, 
										IMAGE_BITMAP, 
										(LPARAM) data->bmp);

					bReturnValue = TRUE;
					break;
				}

				case PSN_KILLACTIVE:
				{
					// Save user data for this page
					char buf[512] = {0x00};
					int passes;

					GetWindowText(drivelist, buf, sizeof(buf));

					strcpy(data->drive, buf);

					GetWindowText(numpasses, buf, sizeof(buf));
					
					passes = atoi(buf);

					data->passes = passes;

					break;
				}

				case PSN_WIZNEXT:
				{
					// Save user data for this page
					char buf[512] = {0x00};
					int passes;

					GetWindowText(numpasses, buf, sizeof(buf));
					
					passes = atoi(buf);

					data->passes = passes;

					if((passes<1)||(passes>32))
					{
						char Caption[100];
						char Message[500];

						LoadString (g_hinst, IDS_PGPERROR, Caption, sizeof(Caption));
						LoadString (g_hinst, IDS_PASSESOUTOFRANGE, Message, sizeof(Message));

						MessageBox(hwnd,Message,Caption,
							MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);

						if(passes<1)
							SetWindowText(numpasses, "1");

						if(passes>32)
							SetWindowText(numpasses, "32");

						SetWindowLong(hwnd,DWL_MSGRESULT,(LPARAM)-1); // Don't go on...

						return TRUE;
					}
					break;
				}

				case PSN_WIZFINISH:
				{
					// Save user data for all pages
					break;
				}

				case PSN_HELP:
				{
					// Display help
					break;
				}

				case PSN_QUERYCANCEL:
				{
					// User wants to quit
					break;
				}
			}
			
			break;
		}

	}

	return bReturnValue;
}

BOOL CALLBACK StatusDlgProc(HWND hwnd, 
							UINT msg, 
							WPARAM wParam, 
							LPARAM lParam)
{
	BOOL bReturnValue = FALSE;
	static wizard_data* data = NULL;
	static HWND fsdata = NULL;
	static HWND ncdata = NULL;
	static HWND scdata = NULL;
	static HWND bsdata = NULL;
	static HWND tcdata = NULL;
	static HWND group = NULL;


	switch(msg)
	{
		case WM_INITDIALOG:
		{
			PROPSHEETPAGE* psp = (PROPSHEETPAGE*) lParam;
			char buf[512] = {0x00};
			
			data = (wizard_data*) psp->lParam;

			fsdata = GetDlgItem(hwnd, IDC_FSDATA);
			ncdata = GetDlgItem(hwnd, IDC_NCDATA);
			scdata = GetDlgItem(hwnd, IDC_SCDATA);
			bsdata = GetDlgItem(hwnd, IDC_BSDATA);
			tcdata = GetDlgItem(hwnd, IDC_TCDATA);
			group = GetDlgItem(hwnd, IDC_GROUP);

			g_status = GetDlgItem(hwnd, IDC_STATUS);
			g_wipe_progress = GetDlgItem(hwnd, IDC_WIPE_PROGRESS);
			g_pass_progress	= GetDlgItem(hwnd, IDC_PASS_PROGRESS);
			g_pass_status = GetDlgItem(hwnd, IDC_STATIC_PASS);
			
			SetWindowLong(hwnd, GWL_USERDATA, (LPARAM) data);

			bReturnValue = TRUE;
			break;
		}

		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDC_BEGIN:
				{
					data->hwnd = hwnd;

					_beginthread(WipeThread, 0, (void*)data);

					bReturnValue = TRUE;
					break;
				}
			}
			break;
		}

		case WM_DESTROY:
		{
			break;
		}

		case WM_PAINT:
		{
			if (data->palette)
			{
				PAINTSTRUCT ps;
				HDC	hDC = BeginPaint (hwnd, &ps);
				SelectPalette (hDC, data->palette, FALSE);
				RealizePalette (hDC);
				EndPaint (hwnd, &ps);
				bReturnValue = TRUE;
			}

			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR pnmh;

			pnmh = (LPNMHDR) lParam;

			switch(pnmh->code)
			{
				case PSN_SETACTIVE:
				{
					char StrRes[500];
					VOLINFO vi;

					SetWindowText(GetDlgItem(hwnd,IDC_RESTARTINGTEXT),
						"");

					WipeProgress(0);
					PassProgress(0, data->passes);

					LoadString (g_hinst, IDS_DISKSTAT, StrRes, sizeof(StrRes));

					strcat(StrRes, data->drive);
					SetWindowText(group, StrRes);
	
					WipeFree(hwnd,
						&vi,
						data->drive,
						NULL,
						FALSE);

					DisplayDiskStats(&vi);

					LoadString (g_hinst, IDS_BEGINWIPE, StrRes, sizeof(StrRes));

					StatusMessage(StrRes, FALSE);
					WipeProgress(0);

					// Initialize window
					PostMessage(GetParent(hwnd), 
								PSM_SETWIZBUTTONS, 
								0, 
								PSWIZB_DISABLEDFINISH|PSWIZB_BACK);

					SendDlgItemMessage(	hwnd, 
										IDC_WIZBITMAP, 
										STM_SETIMAGE, 
										IMAGE_BITMAP, 
										(LPARAM) data->bmp);

					bReturnValue = TRUE;
					break;
				}

				case PSN_KILLACTIVE:
				{
					// Save user data for this page
					break;
				}

				case PSN_WIZFINISH:
				{
					// Save user data for all pages
					PGPContextRef	context = data->context;
					PGPError		error;
					PGPPrefRef		prefRef;
					PGPMemoryMgrRef memoryMgr;

					memoryMgr = PGPGetContextMemoryMgr(context);

					error = PGPclOpenClientPrefs(	memoryMgr,
									&prefRef );

					if(IsntPGPError(error) )
					{
						PGPSetPrefNumber (	prefRef,
											kPGPPrefDiskWipePasses, 
											data->passes);

						PGPclCloseClientPrefs (prefRef, TRUE);

					}

					break;
				}

				case PSN_HELP:
				{
					// Display help
					break;
				}

				case PSN_QUERYCANCEL:
				{
					NMHDR* nmhdr = (NMHDR*) lParam;

					// User wants to quit
					if( g_bWiping )
					{
						bGlobalCancel=TRUE;
						bReturnValue = TRUE;
						SetWindowLong(hwnd, DWL_MSGRESULT, TRUE);
					}
					break;
				}
			}
			
			break;
		}

	}

	return bReturnValue;
}

BOOL CALLBACK FinishDlgProc(HWND hwnd, 
							UINT msg, 
							WPARAM wParam, 
							LPARAM lParam)
{
	BOOL bReturnValue = FALSE;
	static HFONT boldfont;
	static wizard_data* data = NULL;

	switch(msg)
	{
		case WM_INITDIALOG:
		{
			PROPSHEETPAGE* psp = (PROPSHEETPAGE*) lParam;
			HWND finish = GetDlgItem(hwnd, IDC_FINISH);
			HWND hwndParent = GetParent(hwnd);
			HFONT defaultfont;
			LOGFONT	lf;

			data = (wizard_data*) psp->lParam;

			defaultfont = GetStockObject(DEFAULT_GUI_FONT);

			GetObject(defaultfont, sizeof(LOGFONT), &lf);

			lf.lfWeight = FW_BOLD;

			boldfont = CreateFontIndirect(&lf);

			SendMessage(finish, 
						WM_SETFONT, 
						(WPARAM)boldfont,
						MAKELPARAM(TRUE, 0));

			bReturnValue = TRUE;
			break;
		}

		case WM_DESTROY:
		{
			DeleteObject(boldfont);
			break;
		}

		case WM_PAINT:
		{
			if (data->palette)
			{
				PAINTSTRUCT ps;
				HDC	hDC = BeginPaint (hwnd, &ps);
				SelectPalette (hDC, data->palette, FALSE);
				RealizePalette (hDC);
				EndPaint (hwnd, &ps);
				bReturnValue = TRUE;
			}

			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR pnmh;

			pnmh = (LPNMHDR) lParam;

			switch(pnmh->code)
			{
				case PSN_SETACTIVE:
				{
					// Initialize window
					PostMessage(GetParent(hwnd), 
						PSM_SETWIZBUTTONS, 0, PSWIZB_FINISH);

					SendDlgItemMessage(	hwnd, 
										IDC_WIZBITMAP, 
										STM_SETIMAGE, 
										IMAGE_BITMAP, 
										(LPARAM) data->bmp);

					bReturnValue = TRUE;
					break;
				}

				case PSN_KILLACTIVE:
				{
					// Save user data for this page
					break;
				}

				case PSN_WIZFINISH:
				{
					// Save user data for all pages
					break;
				}

				case PSN_HELP:
				{
					// Display help
					break;
				}

				case PSN_QUERYCANCEL:
				{
					// User wants to quit
					break;
				}
			}
			
			break;
		}

	}

	return bReturnValue;
}

void StatusMessage(char *msg, BOOL bold)
{
	if(bold)
	{
		SendMessage(g_status, 
				WM_SETFONT, 
				(WPARAM)g_boldfont,
				MAKELPARAM(TRUE, 0));
	}
	else
	{
		SendMessage(g_status, 
				WM_SETFONT, 
				(WPARAM)g_defaultfont,
				MAKELPARAM(TRUE, 0));
	}

	SetWindowText(g_status, msg);
}

void WipeProgress(int pos)
{
	if(pos==10)
	{
		SetWindowText(
			GetDlgItem(GetParent(g_wipe_progress),IDC_RESTARTINGTEXT),
			"");
	}

	SendMessage(g_wipe_progress,
				PBM_SETPOS ,
				(WPARAM)pos,
				0);	
}

void PassProgress(int pass, int total)
{
	char msg[] = "Pass: %d/%d";
	char buf[128];

	SendMessage(g_pass_progress,
				PBM_SETRANGE,
				0,
				MAKELPARAM(0, total));	

	SendMessage(g_pass_progress,
				PBM_SETPOS ,
				(WPARAM)pass,
				0);	

	pass=pass+1;  // now one based instead of zero based
	if(pass>total)
		pass=total;

	wsprintf(buf, msg, pass , total); 

	SetWindowText(g_pass_status, buf);
}

BOOL DisplayDiskStats(VOLINFO *vi)
{
	char buf[256];

	switch(vi->dwFS)
	{
		case FS_FAT12:
			strcpy(buf,"FAT 12");
			break;

		case FS_FAT16:
			strcpy(buf,"FAT 16");
			break;

		case FS_FAT32:
			strcpy(buf,"FAT 32");
			break;

		case FS_NTFS:
			strcpy(buf,"NTFS");
			break;

		default: // Should never get here
			strcpy(buf,"Unknown"); 
	}

	SendMessage(GetDlgItem(vi->hwnd,IDC_FSDATA),
				WM_SETTEXT,
				(WPARAM)0,
				(LPARAM)buf);

	sprintf(buf,"%d",vi->no_of_clusters);
	SendMessage(GetDlgItem(vi->hwnd,IDC_NCDATA),
				WM_SETTEXT,
				(WPARAM)0,
				(LPARAM)buf);

	sprintf(buf,"%d",vi->sectors_per_clus);
	SendMessage(GetDlgItem(vi->hwnd,IDC_SCDATA),
				WM_SETTEXT,
				(WPARAM)0,
				(LPARAM)buf);

	sprintf(buf,"%d",vi->sector_size);
	SendMessage(GetDlgItem(vi->hwnd,IDC_BSDATA),
				WM_SETTEXT,
				(WPARAM)0,
				(LPARAM)buf);

	sprintf(buf,"%.0f K",vi->fTotalCapacity/1024);
	SendMessage(GetDlgItem(vi->hwnd,IDC_TCDATA),
				WM_SETTEXT,
				(WPARAM)0,
				(LPARAM)buf);

	return TRUE;
}

void WipeThread(void* pvoid)
{
	wizard_data* data = (wizard_data*)pvoid;
	HWND hwnd = data->hwnd;
	HWND hwndParent = GetParent(hwnd);
	VOLINFO vi;
	long err;
	char buf[512];
	int passes = data->passes;
	PGPError		error			= kPGPError_NoErr;
	PGPError		entropyErr		= kPGPError_NoErr;
	PGPDiskWipeRef	wipeRef			= kPGPInvalidRef;
	PGPInt32		pattern[256];
	char StrRes[500];

	EnableWindow(GetDlgItem(hwnd, IDC_BEGIN), FALSE);

	SendMessage(hwndParent, 
				PSM_SETWIZBUTTONS,
				0, 
				PSWIZB_DISABLEDFINISH);

	g_bWiping = TRUE;

	do{
		error = PGPCreateDiskWiper(data->context, &wipeRef, passes);

		if(error == kPGPError_OutOfEntropy)
		{
			entropyErr = PGPclRandom(data->context, hwnd, 300);
		}

	}while(	error == kPGPError_OutOfEntropy && 
			entropyErr != kPGPError_UserAbort);
	
	err = WFE_NOERROR;

	if( IsntPGPError(error) )
	{
		int count = 0;

		do
		{
			WipeProgress(0);
			PassProgress(data->passes - passes, data->passes);

			if(err != WFE_DISKMODIFIED)
			{
				PGPGetDiskWipeBuffer(wipeRef, pattern);
			}

			err = WipeFree(	hwnd,
							&vi,
							data->drive,
							pattern,
							TRUE);

			if(err == WFE_DISKMODIFIED)
			{
				SetWindowText(GetDlgItem(hwnd,IDC_RESTARTINGTEXT),
					"Write detected!  Restarting pass.");

//				PGPscMessageBox (hwnd,IDS_PGPERROR,IDS_DETECTEDWRITE,
//							MB_OK|MB_ICONEXCLAMATION);
			}
			else if(err==WFE_NOERROR)
			{
				SetWindowText(GetDlgItem(hwnd,IDC_RESTARTINGTEXT),
					"");

				passes--;
			}

		}while(((err==WFE_NOERROR) || (err==WFE_DISKMODIFIED)) && (passes!=0) );

		SetWindowText(GetDlgItem(hwnd,IDC_RESTARTINGTEXT),
			"");

		error = PGPDestroyDiskWiper(wipeRef);
	}
	else
	{
		err = error;
	}

	g_bWiping = FALSE;

	switch(err)
	{
		case WFE_NOERROR:
		{
			LoadString (g_hinst, IDS_WIPECONGRATS, StrRes, sizeof(StrRes));

			wsprintf(buf, StrRes, data->drive);
			
			// Success.. Enable finish button
			SendMessage(hwndParent, 
				PSM_SETWIZBUTTONS, 
				0, 
				PSWIZB_BACK | PSWIZB_FINISH);
			break;
		}

		case WFE_USERCANCEL:
		{
			LoadString (g_hinst, IDS_WIPECANCEL, StrRes, sizeof(StrRes));

			wsprintf(buf, StrRes);

			SendMessage(hwndParent, 
				PSM_SETWIZBUTTONS, 
				0, 
				PSWIZB_BACK | PSWIZB_DISABLEDFINISH);
			break;
		}

		default:
		{
			char msg[256];

			if(IsPGPError(error))
			{
				PGPclErrorToString (error, msg, sizeof(msg));
		
			}
			else
			{

				LoadString(	data->hinst,
						err -1 + IDS_FSNOTSUPPORTED,
						msg,
						sizeof(msg));	
			}

			LoadString (g_hinst, IDS_WIPENOTSUCCESSFUL, StrRes, sizeof(StrRes));
			wsprintf(buf, StrRes, msg);

			SendMessage(hwndParent, 
				PSM_SETWIZBUTTONS, 
				0, 
				PSWIZB_BACK | PSWIZB_DISABLEDFINISH);
			break;
		}
	}

	
	StatusMessage(buf, TRUE);
	WipeProgress(0);
	PassProgress(data->passes - passes, data->passes);

	EnableWindow(GetDlgItem(hwnd, IDC_BEGIN), TRUE);
}


static HPALETTE 
CreateDIBPalette (LPBITMAPINFO lpbmi, 
				  LPINT lpiNumColors) 
{
	LPBITMAPINFOHEADER lpbi;
	LPLOGPALETTE lpPal;
	HANDLE hLogPal;
	HPALETTE hPal = NULL;
	INT i;
 
	lpbi = (LPBITMAPINFOHEADER)lpbmi;
	if (lpbi->biBitCount <= 8) {
		*lpiNumColors = (1 << lpbi->biBitCount);
	}
	else
		*lpiNumColors = 0;  // No palette needed for 24 BPP DIB
 
	if (*lpiNumColors) {
		hLogPal = GlobalAlloc (GHND, sizeof (LOGPALETTE) +
                             sizeof (PALETTEENTRY) * (*lpiNumColors));
		lpPal = (LPLOGPALETTE) GlobalLock (hLogPal);
		lpPal->palVersion = 0x300;
		lpPal->palNumEntries = *lpiNumColors;
 
		for (i = 0;  i < *lpiNumColors;  i++) {
			lpPal->palPalEntry[i].peRed   = lpbmi->bmiColors[i].rgbRed;
			lpPal->palPalEntry[i].peGreen = lpbmi->bmiColors[i].rgbGreen;
			lpPal->palPalEntry[i].peBlue  = lpbmi->bmiColors[i].rgbBlue;
			lpPal->palPalEntry[i].peFlags = 0;
		}
		hPal = CreatePalette (lpPal);
		GlobalUnlock (hLogPal);
		GlobalFree (hLogPal);
   }
   return hPal;
}


static HBITMAP 
LoadResourceBitmap (HINSTANCE hInstance, 
					LPSTR lpString,
					HPALETTE FAR* lppalette) 
{
	HRSRC  hRsrc;
	HGLOBAL hGlobal;
	HBITMAP hBitmapFinal = NULL;
	LPBITMAPINFOHEADER lpbi;
	HDC hdc;
    INT iNumColors;
 
	if (hRsrc = FindResource (hInstance, lpString, RT_BITMAP)) {
		hGlobal = LoadResource (hInstance, hRsrc);
		lpbi = (LPBITMAPINFOHEADER)LockResource (hGlobal);
 
		hdc = GetDC(NULL);
		*lppalette =  CreateDIBPalette ((LPBITMAPINFO)lpbi, &iNumColors);
		if (*lppalette) {
			SelectPalette (hdc,*lppalette,FALSE);
			RealizePalette (hdc);
		}
 
		hBitmapFinal = CreateDIBitmap (hdc,
                   (LPBITMAPINFOHEADER)lpbi,
                   (LONG)CBM_INIT,
                   (LPSTR)lpbi + lpbi->biSize + iNumColors * sizeof(RGBQUAD),
                   (LPBITMAPINFO)lpbi,
                   DIB_RGB_COLORS );
 
		ReleaseDC (NULL,hdc);
		UnlockResource (hGlobal);
		FreeResource (hGlobal);
	}
	return (hBitmapFinal);
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
