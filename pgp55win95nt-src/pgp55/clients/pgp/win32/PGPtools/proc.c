/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: proc.c,v 1.34.4.3 1997/11/18 18:13:46 wjb Exp $
____________________________________________________________________________*/
#include "precomp.h"
#include <commctrl.h>

#define ICONSIZE 32

HFONT hFont;

BOOL IsOnTop(void);
void RememberOnTop(BOOL bOnTop);
void RememberPosition(HWND hwnd);
void GetSavedPosition(POINT* pt, BOOL* bLarge);

typedef struct _buttonstruct 
{
	HWND hwndbutton;
	unsigned int idbutton;
	int showbutton;
	HWND hwndtext;
	unsigned int idtext;
	int showtext;
	char text[30];
	unsigned int idicon;
	int x;
	int y;
	int dx;
	int dy;
	unsigned int L8idicon;
	unsigned int L4idicon;
	int Lx;
	int Ly;
	int Ldx;
	int Ldy;
	unsigned int S8idicon;
	unsigned int S4idicon;
	int Sx;
	int Sy;
	int Sdx;
	int Sdy;
} BUTTONSTRUCT;

HWND hwndToolTip=NULL;

BOOL LargeMode;
BOOL HighColor;

static BUTTONSTRUCT bd[6]={
	{0,IDC_KEYS   ,0,0,2000,0,"PGPkeys",
				   0,0,0,0,0,
	   IDI_L8KEYS,IDI_L4KEYS    , 0 ,1,46,46,
	   IDI_S8KEYS,IDI_S4KEYS    , 0 ,1,25,25},
	{0,IDC_ENCRYPT,0,0,2001,0,"Encrypt",
				   0,0,0,0,0,
	   IDI_L8ENC,IDI_L4ENC      ,46 ,1,46,46,
	   IDI_S8ENC,IDI_S4ENC      ,25 ,1,25,25},
	{0,IDC_SIGN   ,0,0,2002,0,"Sign",
	               0,0,0,0,0,
	   IDI_L8SIG,IDI_L4SIG      ,92 ,1,46,46,
	   IDI_S8SIG,IDI_S4SIG      ,50 ,1,25,25},
	{0,IDC_ENCSIGN,0,0,2003,0,"Encrypt + Sign",
	               0,0,0,0,0,
	   IDI_L8ENCSIG,IDI_L4ENCSIG,138,1,46,46,
	   IDI_S8ENCSIG,IDI_S4ENCSIG,75,1,25,25},
	{0,IDC_DECRYPT,0,0,2004,0,"Decrypt/Verify",
	               0,0,0,0,0,
	   IDI_L8DECVER,IDI_L4DECVER,184,1,46,46,
	   IDI_S8DECVER,IDI_S4DECVER,100,1,25,25},
	{0,IDC_WIPE   ,0,0,2005,0,"Wipe",
	               0,0,0,0,0,
	   IDI_L8WIPE,IDI_L4WIPE    ,230,1,46,46,
	   IDI_S8WIPE,IDI_S4WIPE    ,125,1,25,25}};

LRESULT CALLBACK HiddenWndProc (HWND hwnd, UINT iMsg, 
								WPARAM wParam, LPARAM lParam)
{
	 RECT rc;
	 
     switch (iMsg)
     {
         case WM_CREATE :

  			GetWindowRect (hwnd, &rc);
			SetWindowPos (hwnd, NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);
			                
            return 0 ;
	 }
 
    return DefWindowProc (hwnd, iMsg, wParam, lParam) ;
}

LRESULT WINAPI MyDropFilesProc(HWND hwnd, UINT msg, 
							   WPARAM wParam, LPARAM lParam)
{
 
	switch(msg)
	{
		case WM_DROPFILES:
		{		
			HWND hwndDlg;
			HDROP hDrop;
			int NumFiles,ReturnCode;
			BOOL bEncrypt,bSign;
			int i;
			char szFile[MAX_PATH+1];
			FILELIST *ListHead;

			ReturnCode=TRUE;
			hDrop=(HDROP)wParam;

			if(hDrop==0)
				return 0;

			NumFiles=DragQueryFile(hDrop,0xFFFFFFFF,szFile,MAX_PATH);
			
			if(NumFiles==0)
				return 0;

			hwndDlg=GetParent(hwnd);

			bEncrypt=bSign=FALSE;

			if(hwnd==GetDlgItem(hwndDlg,IDC_ENCRYPT))
				bEncrypt = TRUE;

			if(hwnd==GetDlgItem(hwndDlg,IDC_SIGN))
				bSign = TRUE;

			if(hwnd==GetDlgItem(hwndDlg,IDC_ENCSIGN))
				bEncrypt = bSign = TRUE;

			if ((hwnd==GetDlgItem(hwndDlg,IDC_ENCRYPT))||
				(hwnd==GetDlgItem(hwndDlg,IDC_SIGN))   ||
				(hwnd==GetDlgItem(hwndDlg,IDC_ENCSIGN)))
			{
				ListHead=0;

				for(i=0;i<NumFiles && ReturnCode == TRUE;i++)
				{
					DragQueryFile(hDrop,i,szFile,MAX_PATH);
					AddToFileList(&ListHead,szFile);
				}

				ReturnCode = EncryptFileList(hwndDlg,PGPsc,
					ListHead,
					bEncrypt, 
					bSign);
			}

			if(hwnd==GetDlgItem(hwndDlg,IDC_KEYS))
			{
				ListHead=0;

				for(i=0;i<NumFiles && ReturnCode == TRUE;i++)
				{
					DragQueryFile(hDrop,i,szFile,MAX_PATH);
					AddToFileList(&ListHead,szFile);
				}

				ReturnCode = AddKeyFileList(hwndDlg,PGPsc,
					ListHead);
			}

			if(hwnd==GetDlgItem(hwndDlg,IDC_DECRYPT))
			{
				ListHead=0;

				for(i=0;i<NumFiles && ReturnCode == TRUE;i++)
				{
					DragQueryFile(hDrop,i,szFile,MAX_PATH);
					AddToFileList(&ListHead,szFile);
				}

				ReturnCode = DecryptFileList(hwndDlg,PGPsc,
					ListHead);
			}

			if(hwnd==GetDlgItem(hwndDlg,IDC_WIPE))
			{
				if(FreewareWipeCheck(hwnd))
				{
					ListHead=0;

					for(i=0;i<NumFiles && ReturnCode == TRUE;i++)
					{
						DragQueryFile(hDrop,i,szFile,MAX_PATH);
						AddToFileList(&ListHead,szFile);
					}

					ReturnCode = WipeFileList(hwndDlg,
						ListHead);
				}
			}

			DragFinish(hDrop);

			return 0;
		}
    }
     
	//  Pass all non-custom messages to old window proc
	return CallWindowProc((FARPROC)origButtonProc, hwnd, 
							msg, wParam, lParam ) ;
}

BOOL CreateTooltips(HWND hWnd)
{
	TOOLINFO ti;
	int i;
	
	if(hwndToolTip!=NULL)
		DestroyWindow(hwndToolTip);

      /* create the tooltip window - we will use the default delays, but 
      if you want to change that, you can use the TTM_SETDELAYTIME 
      message */
	hwndToolTip = CreateWindowEx( 0, 
		TOOLTIPS_CLASS, 
        NULL, 
        WS_POPUP | TTS_ALWAYSTIP, 
        CW_USEDEFAULT, 
        CW_USEDEFAULT, 
        10, 
        10, 
        hWnd, 
        NULL, 
        g_hinst, 
        NULL);
   
	for(i=0;i<6;i++)
	{
		//add the button to the tooltip
		ZeroMemory(&ti, sizeof(ti));
		ti.cbSize = sizeof(ti);
		ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;  
		//TTF_SUBCLASS causes the tooltip to automatically 
		//subclass the window and look for the messages it 
		//is interested in.
		ti.hwnd = hWnd;
		ti.uId = (UINT)bd[i].hwndbutton;
		ti.lpszText = bd[i].text;
		SendMessage(hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&ti);
	}

	return TRUE;
}

void MoveButtons(HWND hwnd)
{
	int i;
	int windowSizeX,windowSizeY;
	RECT rc;
	int menuheight,captionheight;
	int edgex,edgey;

	for(i=0;i<6;i++)
	{
		if(LargeMode)
		{
			if(HighColor)
				bd[i].idicon=bd[i].L8idicon;
			else
				bd[i].idicon=bd[i].L4idicon;

			bd[i].x=bd[i].Lx;
			bd[i].y=bd[i].Ly;
			bd[i].dx=bd[i].Ldx;
			bd[i].dy=bd[i].Ldy;
			bd[i].showbutton=SW_SHOW;
			bd[i].showtext=SW_HIDE;
		}
		else
		{
			if(HighColor)
				bd[i].idicon=bd[i].S8idicon;
			else
				bd[i].idicon=bd[i].S4idicon;

			bd[i].x=bd[i].Sx;
			bd[i].y=bd[i].Sy;
			bd[i].dx=bd[i].Sdx;
			bd[i].dy=bd[i].Sdy;
			bd[i].showbutton=SW_SHOW;
			bd[i].showtext=SW_HIDE;
		}

		SendMessage(bd[i].hwndbutton,BM_SETIMAGE,(WPARAM)IMAGE_ICON,
				(LPARAM)LoadIcon(g_hinst,MAKEINTRESOURCE(bd[i].idicon)));
	}

	edgex=GetSystemMetrics(SM_CXFIXEDFRAME);
	edgey=GetSystemMetrics(SM_CYFIXEDFRAME);

	if(LargeMode)
	{
		windowSizeX  = 278 + edgex*2;
		windowSizeY = 49 + edgey*2;
	}
	else
	{
		windowSizeX= 152 + edgex*2;
		windowSizeY = 28 + edgey*2;
	}

	rc.left=0;
	rc.top=0;
	rc.right=windowSizeX;
	rc.bottom=windowSizeY;

	menuheight = GetSystemMetrics(SM_CYMENU);
	captionheight= GetSystemMetrics(SM_CYCAPTION);

	rc.bottom=rc.bottom+captionheight; 

	SetWindowPos (hwnd, NULL, 0, 0,rc.right,rc.bottom, SWP_NOMOVE|SWP_NOZORDER);

	for(i=0;i<6;i++)
    {
		MoveWindow(bd[i].hwndbutton,
			bd[i].x,bd[i].y,
			bd[i].dx,bd[i].dy,TRUE);

		MoveWindow(bd[i].hwndtext,
			bd[i].x-5,bd[i].y+bd[i].dy+5,
			bd[i].dx+10,20,TRUE);

		ShowWindow(bd[i].hwndbutton,bd[i].showbutton);
		ShowWindow(bd[i].hwndtext,bd[i].showtext);
	}

	CreateTooltips(hwnd);
}

void LargePGPtools(void)
{
	LargeMode=TRUE;
}

void SmallPGPtools(void)
{
	LargeMode=FALSE;
}


LRESULT CALLBACK PGPtoolsWndProc (HWND hwnd, UINT iMsg, 
								  WPARAM wParam, LPARAM lParam)
{
	LONG Ret = NOERROR;
	UINT cbFiles = 0;
	UINT i = 0;
	UINT ReturnCode = TRUE;
	static BOOL bOnTop = FALSE;

	switch (iMsg)
    {
		case WM_CREATE :
		{
			LOGFONT lf;
			int i;
			int iNumBits;
			HDC hDC;
			HMENU hmenu = NULL;
			POINT pt;
			BOOL bIsLarge = FALSE;
			
			//don't forget this
			InitCommonControls();

			FreewareSetParent(hwnd);

			hDC = GetDC (NULL);		// DC for desktop
			iNumBits = 
				GetDeviceCaps (hDC, BITSPIXEL) * GetDeviceCaps (hDC, PLANES);
			ReleaseDC (NULL, hDC);

			if (iNumBits <= 8)
				HighColor=FALSE;
			else
				HighColor=TRUE;

            SystemParametersInfo(SPI_GETICONTITLELOGFONT,
				sizeof(LOGFONT), &lf, 0);
			hFont = CreateFontIndirect (&lf);

         	for(i=0;i<6;i++)
         	{
				bd[i].hwndbutton=CreateWindow ("BUTTON", "",
                    WS_CHILD | WS_VISIBLE | BS_ICON, //BS_OWNERDRAW,
                    0,0,0,0,
                    hwnd, (HMENU) bd[i].idbutton, g_hinst, NULL) ; 
 
				DragAcceptFiles(bd[i].hwndbutton, TRUE);
				origButtonProc=SubclassWindow(bd[i].hwndbutton,
					MyDropFilesProc);

				bd[i].hwndtext=CreateWindow ("STATIC",bd[i].text,
                    WS_CHILD | WS_VISIBLE | SS_CENTER,
                    0,0,0,0,
                    hwnd, (HMENU) bd[i].idtext, 
					g_hinst, NULL) ; 

				SendMessage(bd[i].hwndtext,WM_SETFONT,
				   (WPARAM)hFont,(LPARAM)MAKELPARAM(TRUE,0));
			}

			hmenu = GetSystemMenu(hwnd, FALSE);
		
			AppendMenu(hmenu, MF_SEPARATOR, 0, NULL);
			AppendMenu(hmenu, MF_STRING, IDM_STAYONTOP, "Stay On &Top");

			if(IsOnTop())
			{
				SetWindowPos(	hwnd, 
								HWND_TOPMOST, 
								0,
								0,
								0,
								0,
								SWP_NOMOVE|SWP_NOSIZE);

				bOnTop = TRUE;

				if(hmenu)
				{
					CheckMenuItem(hmenu, 
						IDM_STAYONTOP, MF_BYCOMMAND|MF_CHECKED);
				}
			}

			GetSavedPosition(&pt, &bIsLarge);

			if(bIsLarge)
			{
				LargePGPtools();
			}
			else
			{
				SmallPGPtools();
			}

			MoveButtons(hwnd);

			// wjb we want to make sure it will be on the screen! Otherwise
			// let the system decide....
			if((pt.x>=0)&&(pt.y>=0)&&
				(pt.x<GetSystemMetrics(SM_CXSCREEN))&&
				(pt.y<GetSystemMetrics(SM_CYSCREEN)))
			{
				SetWindowPos(	hwnd, 
							NULL, 
							pt.x,
							pt.y,
							0,
							0,
							SWP_NOSIZE|SWP_NOZORDER);
            }
			
            return 0 ;
		}

  		case WM_CTLCOLOR:
		{
			if(HIWORD(lParam)!=CTLCOLOR_BTN)
			{      
				DWORD color;
            
				color=GetSysColor(COLOR_BTNFACE);
				
				SetBkColor((HDC)wParam, color);
				SetTextColor((HDC)wParam, 
					GetSysColor (COLOR_WINDOWTEXT));      
				return (BOOL)GreyBrush;
			}
			break;
		}

		case WM_SYSCOMMAND:
		{
			switch(wParam)
			{
				case IDM_STAYONTOP:
				{
					HMENU hmenu = NULL;
					UINT flags = 0;

					SetWindowPos(	hwnd, 
									(bOnTop ? HWND_NOTOPMOST: HWND_TOPMOST), 
									0,
									0,
									0,
									0,
									SWP_NOMOVE|SWP_NOSIZE);

					bOnTop = !bOnTop;

					RememberOnTop(bOnTop);

					hmenu = GetSystemMenu(hwnd, FALSE);

					if(hmenu)
					{
						flags = (bOnTop ?	MF_BYCOMMAND|MF_CHECKED : 
											MF_BYCOMMAND|MF_UNCHECKED);

						CheckMenuItem(hmenu, IDM_STAYONTOP, flags);
					}
					break;
				}
			}
			break;
		}

		case WM_SIZE:
		{
			MoveButtons(hwnd);
			break;
		}

		case WM_SIZING:
		{
			LPRECT DragRect = (LPRECT) lParam;
			int edgeX=GetSystemMetrics(SM_CXFIXEDFRAME);
			int edgeY=GetSystemMetrics(SM_CYFIXEDFRAME);
			int captionY = GetSystemMetrics(SM_CYCAPTION);
			int smallSizeX, smallSizeY, largeSizeX, largeSizeY;

			largeSizeX  = 278 + edgeX * 2;
			largeSizeY = 49 + edgeY * 2 + captionY;
		
			smallSizeX= 152 + edgeX * 2;
			smallSizeY = 28 + edgeY*2 + captionY;

			if(DragRect)
			{
				switch(wParam)		//side being resized;
				{
					case WMSZ_BOTTOM:
						
						if((DragRect->bottom - DragRect->top) < largeSizeY - 5)
						{
							DragRect->bottom = DragRect->top + smallSizeY;
							DragRect->right = DragRect->left + smallSizeX;

							if(LargeMode)
							{
								SmallPGPtools();
								//MoveButtons(hwnd);
							}
						}

						if((DragRect->bottom - DragRect->top) > smallSizeY + 5)
						{
							DragRect->bottom = DragRect->top + largeSizeY;
							DragRect->right = DragRect->left + largeSizeX;

							if(!LargeMode)
							{
								LargePGPtools();
								//MoveButtons(hwnd);
							}
						}

						break;

					case WMSZ_BOTTOMLEFT:

						if((DragRect->bottom - DragRect->top) < largeSizeY - 5)
						{
							DragRect->bottom = DragRect->top + smallSizeY;
							DragRect->left = DragRect->right - smallSizeX;

							if(LargeMode)
							{
								SmallPGPtools();
								//MoveButtons(hwnd);
							}
						}

						if((DragRect->bottom - DragRect->top) > smallSizeY + 5)
						{
							DragRect->bottom = DragRect->top + largeSizeY;
							DragRect->left = DragRect->right - largeSizeX;

							if(!LargeMode)
							{
								LargePGPtools();
								//MoveButtons(hwnd);
							}
						}

						if((DragRect->right - DragRect->left) < largeSizeX - 5)
						{
							DragRect->left = DragRect->right - smallSizeX;
							DragRect->bottom = DragRect->top + smallSizeY;

							if(LargeMode)
							{
								SmallPGPtools();
								//MoveButtons(hwnd);
							}
						}

						if((DragRect->right - DragRect->left) > smallSizeX + 5)
						{
							DragRect->left = DragRect->right - largeSizeX;
							DragRect->bottom = DragRect->top + largeSizeY;

							if(!LargeMode)
							{
								LargePGPtools();
								//MoveButtons(hwnd);
							}
						}

						break;

					case WMSZ_BOTTOMRIGHT:
						if((DragRect->bottom - DragRect->top) < largeSizeY - 5)
						{
							DragRect->bottom = DragRect->top + smallSizeY;
							DragRect->right = DragRect->left + smallSizeX;

							if(LargeMode)
							{
								SmallPGPtools();
								//MoveButtons(hwnd);
							}
						}

						if((DragRect->bottom - DragRect->top) > smallSizeY + 5)
						{
							DragRect->bottom = DragRect->top + largeSizeY;
							DragRect->right = DragRect->left + largeSizeX;

							if(!LargeMode)
							{
								LargePGPtools();
								//MoveButtons(hwnd);
							}
						}

						if((DragRect->right - DragRect->left) < largeSizeX - 5)
						{
							DragRect->right = DragRect->left + smallSizeX;
							DragRect->bottom = DragRect->top + smallSizeY;

							if(LargeMode)
							{
								SmallPGPtools();
								//MoveButtons(hwnd);
							}
						}

						if((DragRect->right - DragRect->left) > smallSizeX + 5)
						{
							DragRect->right = DragRect->left + largeSizeX;
							DragRect->bottom = DragRect->top + largeSizeY;

							if(!LargeMode)
							{
								LargePGPtools();
								//MoveButtons(hwnd);
							}
						}
						break;

					case WMSZ_LEFT:
						if((DragRect->right - DragRect->left) < largeSizeX - 5)
						{
							DragRect->left = DragRect->right - smallSizeX;
							DragRect->bottom = DragRect->top + smallSizeY;

							if(LargeMode)
							{
								SmallPGPtools();
								//MoveButtons(hwnd);
							}
						}

						if((DragRect->right - DragRect->left) > smallSizeX + 5)
						{
							DragRect->left = DragRect->right - largeSizeX;
							DragRect->bottom = DragRect->top + largeSizeY;

							if(!LargeMode)
							{
								LargePGPtools();
								//MoveButtons(hwnd);
							}
						}
						break;

					case WMSZ_RIGHT:
						if((DragRect->right - DragRect->left) < largeSizeX - 5)
						{
							DragRect->right = DragRect->left + smallSizeX;
							DragRect->bottom = DragRect->top + smallSizeY;

							if(LargeMode)
							{
								SmallPGPtools();
								//MoveButtons(hwnd);
							}
						}

						if((DragRect->right - DragRect->left) > smallSizeX + 5)
						{
							DragRect->right = DragRect->left + largeSizeX;
							DragRect->bottom = DragRect->top + largeSizeY;

							if(!LargeMode)
							{
								LargePGPtools();
								//MoveButtons(hwnd);
							}
						}
						break;

					case WMSZ_TOP:
						if((DragRect->bottom - DragRect->top) < largeSizeY - 5)
						{
							DragRect->top = DragRect->bottom - smallSizeY;
							DragRect->right = DragRect->left + smallSizeX;

							if(LargeMode)
							{
								SmallPGPtools();
								//MoveButtons(hwnd);
							}
						}

						if((DragRect->bottom - DragRect->top) > smallSizeY + 5)
						{
							DragRect->top = DragRect->bottom - largeSizeY;
							DragRect->right = DragRect->left + largeSizeX;

							if(!LargeMode)
							{
								LargePGPtools();
								//MoveButtons(hwnd);
							}
						}
						break;

					case WMSZ_TOPLEFT:
						if((DragRect->bottom - DragRect->top) < largeSizeY - 5)
						{
							DragRect->top = DragRect->bottom - smallSizeY;
							DragRect->left = DragRect->right - smallSizeX;

							if(LargeMode)
							{
								SmallPGPtools();
								//MoveButtons(hwnd);
							}
						}

						if((DragRect->bottom - DragRect->top) > smallSizeY + 5)
						{
							DragRect->top = DragRect->bottom - largeSizeY;
							DragRect->left = DragRect->right - largeSizeX;

							if(!LargeMode)
							{
								LargePGPtools();
								//MoveButtons(hwnd);
							}
						}

						if((DragRect->right - DragRect->left) < largeSizeX - 5)
						{
							DragRect->left = DragRect->right - smallSizeX;
							DragRect->bottom = DragRect->top + smallSizeY;

							if(LargeMode)
							{
								SmallPGPtools();
								//MoveButtons(hwnd);
							}
						}

						if((DragRect->right - DragRect->left) > smallSizeX + 5)
						{
							DragRect->left = DragRect->right - largeSizeX;
							DragRect->bottom = DragRect->top + largeSizeY;

							if(!LargeMode)
							{
								LargePGPtools();
								//MoveButtons(hwnd);
							}
						}
						break;

					case WMSZ_TOPRIGHT:
						if((DragRect->bottom - DragRect->top) < largeSizeY - 5)
						{
							DragRect->top = DragRect->bottom - smallSizeY;
							DragRect->right = DragRect->left + smallSizeX;

							if(LargeMode)
							{
								SmallPGPtools();
								//MoveButtons(hwnd);
							}
						}

						if((DragRect->bottom - DragRect->top) > smallSizeY + 5)
						{
							DragRect->top = DragRect->bottom - largeSizeY;
							DragRect->right = DragRect->left + largeSizeX;

							if(!LargeMode)
							{
								LargePGPtools();
								//MoveButtons(hwnd);
							}
						}

						if((DragRect->right - DragRect->left) < largeSizeX - 5)
						{
							DragRect->right = DragRect->left + smallSizeX;
							DragRect->bottom = DragRect->top + smallSizeY;

							if(LargeMode)
							{
								SmallPGPtools();
								//MoveButtons(hwnd);
							}
						}

						if((DragRect->right - DragRect->left) > smallSizeX + 5)
						{
							DragRect->right = DragRect->left + largeSizeX;
							DragRect->bottom = DragRect->top + largeSizeY;

							if(!LargeMode)
							{
								LargePGPtools();
								//MoveButtons(hwnd);
							}
						}
						break;
				}
			}
			return 0;
		}

		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDC_RESIZE:
				{
					if(LargeMode)
						SmallPGPtools();
					else
						LargePGPtools();

					MoveButtons(hwnd);
					break;
				}

				case IDC_EDITCLIP:
				{
					LaunchInternalViewer(hwnd);
					break;
				}
			
				case IDC_HELPTOPICS:
				{
					char HelpFile[MAX_PATH];

					PGPpath(HelpFile);
					strcat(HelpFile,"PGP55.hlp");

					WinHelp(hwnd, HelpFile, HELP_FINDER, 0);
					break;
				}

				case IDC_PGPPREFS:
				{
					PGPscPreferences(hwnd,PGPsc);
					break;
				}

				case IDC_ABOUTPGP:
				{
					PGPscAbout(hwnd);
					break;
				}

				case IDC_KEYS:
				{
					DoLaunchKeys(hwnd);
					break;
				}

				case IDC_ENCRYPT:
				case IDC_SIGN:
				case IDC_ENCSIGN:
				{
					BOOL bEncrypt,bSign;
					FILELIST *ListHead;
					char Prompt[100];

					ListHead=0;
					bEncrypt=bSign=FALSE;

					if(LOWORD(wParam)==IDC_ENCRYPT)
					{
						bEncrypt = TRUE;
						strcpy(Prompt,"Select File(s) to Encrypt");
					}

					if(LOWORD(wParam)==IDC_SIGN)
					{
						bSign = TRUE;
						strcpy(Prompt,"Select File(s) to Sign");
					}

					if(LOWORD(wParam)==IDC_ENCSIGN)
					{
						bEncrypt = bSign = TRUE;
						strcpy(Prompt,"Select File(s) to Encrypt + Sign");
					}

					if(GetFileName(hwnd,
						Prompt,
						&ListHead,TRUE,
						"All Files (*.*)\0*.*\0\0"))
					{
						if(ListHead==0)
						{
							DoEncrypt(hwnd,PGPsc, bEncrypt, bSign);
						}
						else
						{
							ReturnCode = EncryptFileList(hwnd,PGPsc,
								ListHead,
								bEncrypt, 
								bSign);
						}
					}					
					break;
				}  

				case IDC_DECRYPT:
				{
					FILELIST *ListHead;

					ListHead=0;

					if(GetFileName(hwnd,
						"Select File(s) to Decrypt/Verify",
						&ListHead,TRUE,
						"PGP Files (*.pgp, *.asc, *.sig)\0*.pgp;*.asc;*.sig\0"
						"All Files (*.*)\0*.*\0\0"))
					{
						if(ListHead==0)
						{
							DoDecrypt(hwnd,PGPsc);
						}
						else
						{
							ReturnCode = DecryptFileList(hwnd,PGPsc,
								ListHead);	
						}
					}					
					break;
				}   

				case IDC_WIPE:
				{
					if(FreewareWipeCheck(hwnd))
					{
						FILELIST *ListHead;

						ListHead=0;

						if(GetFileName(hwnd,
							"Select File(s) to Wipe",
							&ListHead,FALSE,
							"All Files (*.*)\0*.*\0\0"))
						{
							if(ListHead!=0)
							{
								ReturnCode = WipeFileList(hwnd,
									ListHead);	
							}
						}
					}
					break;
				}

				case IDC_ADDKEY:
				{
					FILELIST *ListHead;

					ListHead=0;

					if(GetFileName(hwnd,
						"Select File(s) to Add Keys",
						&ListHead,TRUE,
						"PGP Key Files (*.pgp, *.asc)\0*.pgp;*.asc\0\0"))
					{
						if(ListHead==0)
						{
							ReturnCode=DoAddKey(hwnd,PGPsc);
						}
						else
						{
							ReturnCode = AddKeyFileList(hwnd,PGPsc,
								ListHead);	
						}
					}					
					break;
				}

				case IDC_QUIT:
				{
					PostQuitMessage (0) ;
					break;
				}
			}   
			break;
		}

#ifndef _WIN32
		case WM_DRAWITEM :
		{
			int              cx, cy ;
			LPDRAWITEMSTRUCT pdis ;

			HICON hIco;                
			int index;                 


		   pdis = (LPDRAWITEMSTRUCT) lParam ;

		   cx = pdis->rcItem.right  - pdis->rcItem.left ;
		   cy = pdis->rcItem.bottom - pdis->rcItem.top  ;

		   FillRect (pdis->hDC, &pdis->rcItem,GreyBrush);

			FrameRect (pdis->hDC, &pdis->rcItem,
			  (HBRUSH) GetStockObject (BLACK_BRUSH)) ;
		
				for(index=0;index<6;index++)
				{
					if(pdis->CtlID==bd[index].idbutton)
					{
						hIco=LoadIcon(g_hinst,
							MAKEINTRESOURCE(bd[index].idicon));
						break;
					}
				}

       
                         
		   DrawIcon(pdis->hDC, 
			   pdis->rcItem.left+((bd[index].dx-ICONSIZE)/2),
			   pdis->rcItem.top+((bd[index].dy-ICONSIZE)/2), hIco);

		   DeleteObject(hIco);
   
		   // Invert the rectangle if the button is selected

		   if (pdis->itemState & ODS_SELECTED)
				InvertRect (pdis->hDC, &pdis->rcItem) ;

		   // Draw a focus rectangle if the button has the focus

		   if (pdis->itemState & ODS_FOCUS)
				{
				pdis->rcItem.left   += 3 ;
				pdis->rcItem.top    += 3 ;
				pdis->rcItem.right  -= 3 ;
				pdis->rcItem.bottom -= 3 ;

				DrawFocusRect (pdis->hDC, &pdis->rcItem) ;
				}

		   return 0 ;   
		}
#endif // !_WIN32
               			  
		case WM_CLOSE:
		case WM_DESTROY :
		{
			RememberPosition(hwnd);
			DeleteObject(hFont);
			PostQuitMessage (0) ;
			return 0 ;     
		}
	}
 
    return DefWindowProc (hwnd, iMsg, wParam, lParam) ;
}

void RememberOnTop(BOOL bOnTop)
{
	HKEY	hKey;
	LONG	lResult;
	DWORD	dw = 0;
	char	path[] = "Software\\PGP\\PGP55\\PGPtools";
	
	dw = bOnTop;

	lResult = RegOpenKeyEx(	HKEY_CURRENT_USER,
							path, 
							0, 
							KEY_ALL_ACCESS, 
							&hKey);

	if (lResult == ERROR_SUCCESS) 
	{
		RegSetValueEx (	hKey, 
						"StayOnTop", 
						0, 
						REG_DWORD, 
						(LPBYTE)&dw, 
						sizeof(dw));
	}
}

BOOL IsOnTop()
{
	HKEY	hKey;
	LONG	lResult;
	DWORD	dw;
	char	path[] = "Software\\PGP\\PGP55\\PGPtools";

	lResult = RegOpenKeyEx(	HKEY_CURRENT_USER,
							path, 
							0, 
							KEY_ALL_ACCESS, 
							&hKey);

	if (lResult == ERROR_SUCCESS) 
	{
		DWORD  size = sizeof(dw);
		DWORD  type = 0;

		RegQueryValueEx(hKey, 
						"StayOnTop", 
						0, 
						&type, 
						(LPBYTE)&dw, 
						&size);
	}
	else // Init Values
	{
		lResult = RegCreateKeyEx (	HKEY_CURRENT_USER, 
									path, 
									0, 
									NULL,
									REG_OPTION_NON_VOLATILE, 
									KEY_ALL_ACCESS, 
									NULL, 
									&hKey, 
									&dw);

		if (lResult == ERROR_SUCCESS) 
		{
			dw = 0;

			RegSetValueEx (	hKey, 
							"StayOnTop", 
							0, 
							REG_DWORD, 
							(LPBYTE)&dw, 
							sizeof(dw));
			dw = 110;

			RegSetValueEx (	hKey, 
						"xPos", 
						0, 
						REG_DWORD, 
						(LPBYTE)&dw, 
						sizeof(dw));

			dw = 1;

			RegSetValueEx (	hKey, 
						"yPos", 
						0, 
						REG_DWORD, 
						(LPBYTE)&dw, 
						sizeof(dw));

			dw = 1;

			RegSetValueEx (	hKey, 
							"size", 
							0, 
							REG_DWORD, 
							(LPBYTE)&dw, 
							sizeof(dw));

			RegCloseKey (hKey);

		}
	}

	return (BOOL)dw;
}

void RememberPosition(HWND hwnd)
{
	HKEY	hKey;
	LONG	lResult;
	DWORD	dw = 0;
	char	path[] = "Software\\PGP\\PGP55\\PGPtools";
	RECT rect;

	GetWindowRect(hwnd, &rect);
	

	lResult = RegOpenKeyEx(	HKEY_CURRENT_USER,
							path, 
							0, 
							KEY_ALL_ACCESS, 
							&hKey);

	if (lResult == ERROR_SUCCESS) 
	{
		dw = rect.left;

		RegSetValueEx (	hKey, 
						"xPos", 
						0, 
						REG_DWORD, 
						(LPBYTE)&dw, 
						sizeof(dw));

		dw = rect.top;

		RegSetValueEx (	hKey, 
						"yPos", 
						0, 
						REG_DWORD, 
						(LPBYTE)&dw, 
						sizeof(dw));

		dw = LargeMode;

		RegSetValueEx (	hKey, 
						"size", 
						0, 
						REG_DWORD, 
						(LPBYTE)&dw, 
						sizeof(dw));
	}

}

void GetSavedPosition(POINT* pt, BOOL* bLarge)
{
	HKEY	hKey;
	LONG	lResult;
	DWORD	dw;
	char	path[] = "Software\\PGP\\PGP55\\PGPtools";

	lResult = RegOpenKeyEx(	HKEY_CURRENT_USER,
							path, 
							0, 
							KEY_ALL_ACCESS, 
							&hKey);

	if (lResult == ERROR_SUCCESS) 
	{
		DWORD  size = sizeof(dw);
		DWORD  type = 0;

		RegQueryValueEx(hKey, 
						"xPos", 
						0, 
						&type, 
						(LPBYTE)&dw, 
						&size);
		pt->x = dw;
		size = sizeof(dw);

		RegQueryValueEx(hKey, 
						"yPos", 
						0, 
						&type, 
						(LPBYTE)&dw, 
						&size);
		pt->y = dw;
		size = sizeof(dw);

		RegQueryValueEx(hKey, 
						"size", 
						0, 
						&type, 
						(LPBYTE)&dw, 
						&size);

		*bLarge = dw;

		RegCloseKey (hKey);
	}
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
