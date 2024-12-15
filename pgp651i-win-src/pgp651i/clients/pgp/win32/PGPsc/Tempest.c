/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: Tempest.c,v 1.16 1999/03/30 17:24:50 wjb Exp $
____________________________________________________________________________*/

#include "precomp.h"
#include "pgpIO.h"
#include "pgpMemoryIO.h"
#include "pgpWordWrap.h"

#define DEACTIVATETIMERID 100
#define DEACTIVATETIME 50

#define MINDLGX 300
#define MINDLGY 200

#define MODX 16
#define XMARGIN 2
#define YMARGIN 1
#define TABWIDTH 4

// This is our global tempest viewer variable structure
// used in case we ever need to make this reentrant. Globals
// are Very Bad (tm).
typedef struct _GTV {
	HWND hdlg;
	HWND hwndLB;
	char *pInput;
	HBITMAP hAlphaBmp;
	HBITMAP hTempABmp;
	HBITMAP hTempBBmp;
	HBITMAP hTempCBmp;
	HBITMAP hTempDBmp;
	HFONT hFixedWidthFont;
	HBRUSH hBackBrush;
	int LtrX;
	int LtrY;
	COLORREF BackColor;
	COLORREF TextColor;
	BOOL bDoTempest;
} GTV;

PGPError
PGPtempestWrapBuffer (
		PGPContextRef context,
		LPSTR		szInText,
		PGPUInt16	wrapColumn,
		LPSTR*		pszOutText)
{
	PGPError		err				= kPGPError_NoErr;
	PGPIORef		inRef, outRef;
	PGPMemoryMgrRef	memMgr;
	PGPFileOffset	outSize;
	PGPSize			outRead;
	INT				InSize;
	PGPMemoryMgrRef	memmgr;

	memmgr = PGPGetContextMemoryMgr (context);

	outRead = 0;

	err = PGPNewMemoryMgr ( 0, &memMgr);

	if (IsntPGPError (err))
	{
		PGPNewMemoryIO (
			memMgr,
			(PGPMemoryIORef *)(&inRef));

		InSize = strlen (szInText);
		PGPIOWrite (inRef,
			InSize,
			szInText);
		PGPIOFlush (inRef);
		PGPIOSetPos (inRef,0);

		PGPNewMemoryIO (
			memMgr,
			(PGPMemoryIORef *)(&outRef));

		err = pgpWordWrapIO(
			  inRef,outRef,
			  wrapColumn,
			  "\r\n");

		if (IsntPGPError (err))
		{
			INT memamt;

			PGPIOGetPos (outRef, &outSize);

			memamt = (INT)outSize+1;
			*pszOutText = (CHAR *)PGPNewSecureData (memmgr, memamt, 0);

			if (*pszOutText)
			{
				memset (*pszOutText, 0x00, memamt);

				PGPIOSetPos (outRef,0);
				PGPIORead (outRef,
					(INT)outSize,
					*pszOutText, 
					&outRead);
				PGPIOFlush (outRef);
			}
			else err = kPGPError_OutOfMemory;
		}
		PGPFreeIO (inRef); 
		PGPFreeIO (outRef);
	}
	PGPFreeMemoryMgr (memMgr);

	return err;
}

void SizeTempest(HWND hdlg,int Width,int Height)
{
	InvalidateRect(GetDlgItem(hdlg, IDC_TEMPESTLIST),NULL,TRUE);

	MoveWindow(GetDlgItem(hdlg, IDC_TEMPESTLIST),
		10,10,
		Width-20,Height-45,
		TRUE);

	InvalidateRect(GetDlgItem(hdlg, IDC_TEMPESTFONT),NULL,TRUE);

	MoveWindow(GetDlgItem(hdlg, IDC_TEMPESTFONT),
		10,Height-25,
		300,15,
		TRUE);
}

//
//  DrawListViewItem
//
//  This routine, given a standard Windows LPDRAWITEMSTRUCT, draws the
//  elements of our custom listview.

void DrawListBoxItem(GTV *gtv,LPDRAWITEMSTRUCT lpDrawItem)
{
	HDC mDC;
	UINT i;
	unsigned char *szString;
	unsigned char StringIndex;
	int xCoord,yCoord;
	int pos;

	srand( (unsigned)time( NULL ) );

	//  Create a memory DC for bitmap work
	mDC = CreateCompatibleDC( lpDrawItem->hDC );
    
	if(!gtv->bDoTempest)
	{
		SelectObject( mDC, gtv->hAlphaBmp);  
	}

	FillRect(lpDrawItem->hDC,&(lpDrawItem->rcItem),gtv->hBackBrush);

	szString=(char *)lpDrawItem->itemData;

	if(szString==NULL)
		return;

	pos=0;
	i=0;

	while((szString[i]!=0)&&(szString[i]!='\n'))
	{
		StringIndex=(unsigned char)szString[i];

		if(StringIndex=='\t')
		{
			pos=((pos/TABWIDTH)+1)*TABWIDTH;
		}
		else
		{
			// Avoid the characters outside the range of our font
			if(((StringIndex>=32)&&(StringIndex<=127))||
				((StringIndex>=160)&&(StringIndex<=255)))
			{
				if(StringIndex<=127)
				{
					StringIndex=StringIndex-32;
				}
				else
				{
					StringIndex=StringIndex-160+6*MODX;
				}

				// Randomly select through jittered fonts to
				// confuse snoopers further
				if(gtv->bDoTempest)
				{
					switch((4*rand())/RAND_MAX)
					{
						case 0:
							SelectObject( mDC, gtv->hTempABmp);  
							break;
						case 1:
							SelectObject( mDC, gtv->hTempBBmp);  
							break;
						case 2:
							SelectObject( mDC, gtv->hTempCBmp);  
							break;
						default:
							SelectObject( mDC, gtv->hTempDBmp);  
							break;
					}
				}

				xCoord=(StringIndex)%MODX;
				yCoord=(StringIndex)/MODX;

				BitBlt(lpDrawItem->hDC,// handle to destination device context 
					lpDrawItem->rcItem.left + XMARGIN + pos * gtv->LtrX,
					// x-coordinate of destination rectangle's upper-left  
					lpDrawItem->rcItem.top + YMARGIN,
					// x-coordinate of destination rectangle's upper-left  
					gtv->LtrX,   // width of destination rectangle 
					gtv->LtrY,   // height of destination rectangle 
					mDC,    // handle to source device context 
					xCoord*gtv->LtrX,
					// x-coordinate of source rectangle's upper-left   
					yCoord*gtv->LtrY,
					// y-coordinate of source rectangle's upper-left 
					SRCCOPY);
			}
			pos++;
		}
		i++;
	}
	//  Kill off the memory DC, unlocking passed BMP
	DeleteDC( mDC );  

    return;
}

// Main_OnDrawItem
//
// Entry function for the message handler. Basically, we want to draw
// the whole thing no matter what.

BOOL Main_OnDrawItem(GTV *gtv, const DRAWITEMSTRUCT * lpDrawItem)
{
    // Make sure the control is the listview control
    if (lpDrawItem->CtlType != ODT_LISTBOX)
        return FALSE;

    switch (lpDrawItem->itemAction)
    {
        case ODA_DRAWENTIRE:
        case ODA_FOCUS:
        case ODA_SELECT:
            DrawListBoxItem(gtv,(LPDRAWITEMSTRUCT)lpDrawItem);
            break;
    }

    return TRUE;
}

// Main_OnMeasureItem
//
// Entry function for the message handler. We need to get the width and
// height of the font we're using.

void Main_OnMeasureItem(GTV *gtv, MEASUREITEMSTRUCT * lpMeasureItem)
{
     // Make sure the control is the listview control
    if (lpMeasureItem->CtlType != ODT_LISTBOX)
        return;

    // Add a little extra space between items
    lpMeasureItem->itemHeight = gtv->LtrY + YMARGIN;
}

UINT TempestStringWidth(char *szString)
{
	UINT pos,i;

	pos=0;

	i=0;

	while((szString[i]!=0)&&(szString[i]!='\n'))
	{
		// Handle Tabs
		if(szString[i]=='\t')
		{
			pos=((pos/TABWIDTH)+1)*TABWIDTH;
		}
		else
		{
			pos++;
		}

		i++;
	}

	return pos;
}

void AddListBoxLines(GTV *gtv)
{ 
	int width,maxwidth;
	char *szNew,*szOld;

	maxwidth=0;
	szNew=gtv->pInput;

	while(szNew!=0)
	{
		szOld=szNew;

		szNew=strstr(szOld,"\n");

		if(szNew!=0)
		{
			szNew++;
		}

		width=TempestStringWidth(szOld)*gtv->LtrX+2*XMARGIN;

		if(width>maxwidth)
		{
			maxwidth=width;
		}
				
		SendMessage(gtv->hwndLB,
			LB_ADDSTRING,0,(LPARAM)szOld);
	}

	SendMessage(gtv->hwndLB,
		LB_SETHORIZONTALEXTENT,(WPARAM)maxwidth,0);

}

void RedoTempestList(GTV *gtv)
{
	RECT rc;

	if(gtv->hwndLB!=NULL)
		DestroyWindow(gtv->hwndLB);

	gtv->hwndLB=CreateWindowEx (WS_EX_CLIENTEDGE,"listbox", NULL,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL |  WS_HSCROLL |
		LBS_OWNERDRAWFIXED | LBS_NOINTEGRALHEIGHT | WS_TABSTOP,
		0,0,0,0,
		gtv->hdlg, (HMENU)IDC_TEMPESTLIST,
		(HINSTANCE) g_hinst,
		NULL) ;

	// Size the viewer
	GetClientRect(gtv->hdlg,&rc);
	SizeTempest(gtv->hdlg,rc.right-rc.left,rc.bottom-rc.top);

	AddListBoxLines(gtv);
}

// Get rid of all drawing elements to clean up
void UninitFontBitmap(GTV *gtv)
{
	if(gtv->hFixedWidthFont!=NULL)
		DeleteObject(gtv->hFixedWidthFont);
	gtv->hFixedWidthFont=NULL;

	if(gtv->hBackBrush!=NULL)
		DeleteObject(gtv->hBackBrush);
	gtv->hBackBrush=NULL;

	if(gtv->hAlphaBmp!=NULL)
		DeleteObject(gtv->hAlphaBmp);
	gtv->hAlphaBmp=NULL;

	if(gtv->hTempABmp!=NULL)
		DeleteObject(gtv->hTempABmp);
	if(gtv->hTempBBmp!=NULL)
		DeleteObject(gtv->hTempBBmp);
	if(gtv->hTempCBmp!=NULL)
		DeleteObject(gtv->hTempCBmp);
	if(gtv->hTempDBmp!=NULL)
		DeleteObject(gtv->hTempDBmp);

	gtv->hTempABmp=gtv->hTempBBmp=gtv->hTempCBmp=gtv->hTempDBmp=NULL;
}

// Render the font bitmaps (either normal or tempest)
// if they aren't done already. Create the background 
// brush, and set the text and background colors.
void InitFontBitmap(HWND hwnd,GTV *gtv)
{
	LOGFONT lf;
	TEXTMETRIC tm;
	HDC hdc,hdcMem;

	// Get rid of all drawing elements to clean up
	UninitFontBitmap(gtv);

	// **** Create either normal or load tempest fonts ****
	SystemParametersInfo (SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0);
	strcpy(lf.lfFaceName,"Courier");  // as per prz
	lf.lfPitchAndFamily=FF_MODERN|FIXED_PITCH;

	gtv->hFixedWidthFont = CreateFontIndirect (&lf);

    hdc=GetDC(gtv->hdlg); // Get DC
	hdcMem=CreateCompatibleDC(hdc);

	SelectObject (hdcMem, gtv->hFixedWidthFont); // Font for non Tempest
	GetTextMetrics (hdcMem, &tm);

	if(gtv->bDoTempest)
	{
		if(tm.tmHeight>=19)
		{
			gtv->hTempABmp = LoadBitmap (g_hinst, 
				MAKEINTRESOURCE (IDB_MA13X24));; // Tempest Font
			gtv->hTempBBmp = LoadBitmap (g_hinst, 
				MAKEINTRESOURCE (IDB_MB13X24));; // Tempest Font
			gtv->hTempCBmp = LoadBitmap (g_hinst, 
				MAKEINTRESOURCE (IDB_MC13X24));; // Tempest Font
			gtv->hTempDBmp = LoadBitmap (g_hinst, 
				MAKEINTRESOURCE (IDB_MD13X24));; // Tempest Font

			gtv->LtrX=13;
			gtv->LtrY=24;
		}
		else
		{
			gtv->hTempABmp = LoadBitmap (g_hinst, 
				MAKEINTRESOURCE (IDB_MA8X14));; // Tempest Font
			gtv->hTempBBmp = LoadBitmap (g_hinst, 
				MAKEINTRESOURCE (IDB_MB8X14));; // Tempest Font
			gtv->hTempCBmp = LoadBitmap (g_hinst, 
				MAKEINTRESOURCE (IDB_MC8X14));; // Tempest Font
			gtv->hTempDBmp = LoadBitmap (g_hinst, 
				MAKEINTRESOURCE (IDB_MD8X14));; // Tempest Font

			
			gtv->LtrX=8;
			gtv->LtrY=14;
		}

		gtv->TextColor=RGB(0,0,0); // White

		SelectObject(hdcMem,gtv->hTempABmp);  // Bitmap for hdcMem

		gtv->BackColor=GetPixel(hdcMem,0,0);
		gtv->hBackBrush=CreateSolidBrush(gtv->BackColor); // Create the brush needed
	}
	else
	{
		unsigned char myChar;
		RECT rc;
		int i,pos;

		gtv->TextColor=GetSysColor (COLOR_WINDOWTEXT); // Text Color
		gtv->BackColor=GetSysColor (COLOR_WINDOW); // Window Color

		gtv->hBackBrush=CreateSolidBrush(gtv->BackColor); // Create the brush needed

		gtv->LtrX=tm.tmMaxCharWidth;
		gtv->LtrY=tm.tmHeight;

		rc.top=rc.left=0;
		rc.right=MODX*gtv->LtrX; // 16 x 16 for 256 characters
		rc.bottom=MODX*gtv->LtrY;

		gtv->hAlphaBmp=CreateCompatibleBitmap(hdc,rc.right,rc.bottom); 
	    SelectObject(hdcMem,gtv->hAlphaBmp);  // Bitmap for hdcMem

		SetBkColor (hdcMem, gtv->BackColor);
		SetTextColor (hdcMem, gtv->TextColor);

		FillRect(hdcMem,&rc,gtv->hBackBrush);

		pos=0;

		for(i=32;i<=127;i++) // Draw standard charset in first 6 lines
		{
			myChar=(unsigned char)i;

			TextOut(hdcMem,
				(pos%MODX)*gtv->LtrX,
				(pos/MODX)*gtv->LtrY,
				&myChar,
				1);

			pos++;
		}

		for(i=160;i<=255;i++) // Draw high order charset in next 6 lines
		{
			myChar=(char)i;

			TextOut(hdcMem,
				(pos%MODX)*gtv->LtrX,
				(pos/MODX)*gtv->LtrY,
				&myChar,
				1);

			pos++;
		}
	}

	ReleaseDC(hwnd,hdc); // Release DC
	DeleteDC(hdcMem);

	RedoTempestList(gtv);
}

BOOL WINAPI TempestDlgProc(HWND hdlg, UINT uMsg, 
                             WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_INITDIALOG:
        { 
			GTV *gtv;

			gtv=(GTV *)lParam;
			SetWindowLong(hdlg,GWL_USERDATA,(long)lParam);

			// Save some of our window handles
			gtv->hdlg=hdlg;
			gtv->hwndLB=GetDlgItem(hdlg,IDC_TEMPESTLIST);

			InitFontBitmap(gtv->hdlg,gtv);
			RedoTempestList(gtv);

			// Stick text into owner drawn list box and setup checkbox
			Button_SetCheck(GetDlgItem(hdlg,IDC_TEMPESTFONT),gtv->bDoTempest);

			// Force window to the foreground
			SetForegroundWindow(hdlg);
			SetFocus(gtv->hwndLB);
            return FALSE;
        }

		case WM_GETMINMAXINFO:
		{
			MINMAXINFO* lpmmi;

		    lpmmi = (MINMAXINFO*) lParam;
    		lpmmi->ptMinTrackSize.x = MINDLGX;
    		lpmmi->ptMinTrackSize.y = MINDLGY;
            break;
		}

        case WM_SIZE:
        {
			unsigned short Width, Height;
			Width = LOWORD(lParam);  // width of client area 
			Height = HIWORD(lParam); // height of client area 

			SizeTempest(hdlg,Width,Height);
			break;
        }

		// Make sure we set color to white/black for Tempest
		case WM_CTLCOLORLISTBOX:
		{          
			HDC hdcLB;
			HWND hwndLB;
			GTV *gtv;

			gtv=(GTV *)GetWindowLong(hdlg,GWL_USERDATA);

			hdcLB = (HDC) wParam;
			hwndLB = (HWND) lParam;

			SetTextColor(hdcLB,gtv->TextColor);
			SetBkColor(hdcLB, gtv->BackColor);
	
		  	return (BOOL)gtv->hBackBrush;
		}

        case WM_SYSCOLORCHANGE:
		{
			GTV *gtv;

			gtv=(GTV *)GetWindowLong(hdlg,GWL_USERDATA);
			InitFontBitmap(hdlg,gtv);
            break;
        }

        case WM_DRAWITEM: 
        {
			GTV *gtv;

			gtv=(GTV *)GetWindowLong(hdlg,GWL_USERDATA);

            Main_OnDrawItem(gtv,(LPDRAWITEMSTRUCT) lParam);
            return TRUE;
        }

        case WM_MEASUREITEM:
		{
			GTV *gtv;

			gtv=(GTV *)GetWindowLong(hdlg,GWL_USERDATA);

            Main_OnMeasureItem(gtv,(LPMEASUREITEMSTRUCT) lParam);
            return TRUE;
		}

		case WM_ACTIVATE:
		{
			if(LOWORD(wParam)==WA_INACTIVE)
			{
				// Killing the window right away _really_ irks
				// Win95, so we delay 50 ms (basically till everything
				// gets cleaned up in the OS)
				SetTimer(hdlg,DEACTIVATETIMERID,DEACTIVATETIME,NULL);
			}
			break;
		}

		// Here we kill the window from the timer...
		case WM_TIMER:
		case WM_CLOSE:
		case WM_DESTROY:
		case WM_QUIT:
		{
			EndDialog(hdlg, FALSE);
			break;
		}

        case WM_COMMAND:
        {
            switch(wParam)
            {
				case IDC_TEMPESTFONT:
				{
					GTV *gtv;

					gtv=(GTV *)GetWindowLong(hdlg,GWL_USERDATA);

					gtv->bDoTempest=!gtv->bDoTempest;

					InitFontBitmap(hdlg,gtv);

					InvalidateRect(gtv->hwndLB,NULL,TRUE);
					UpdateWindow(gtv->hwndLB);
				    break;
				}
						
				case IDCANCEL:
				{
					EndDialog(hdlg, FALSE);
					break;
				}
            }
            return TRUE;
        }

    }
    return FALSE;
}


UINT TempestViewer(void *PGPsc,HWND hwnd,char *pInput,DWORD dwInSize,BOOL bWarn)
{
	UINT ReturnValue = FALSE;
	BOOL DialogReturn = FALSE;
	WNDCLASS wndclass;
	PGPContextRef context;
	PGPMemoryMgrRef	memmgr;
	GTV *gtv;
	int Response;
	char szCaption[100];
	char szMessage[256];
	char *pWrappedInput;
	
	if(bWarn)
	{
		LoadString (g_hinst, IDS_FYEOCAPTION, szCaption, sizeof(szCaption));
		LoadString (g_hinst, IDS_FYEOMSG, szMessage, sizeof(szMessage));

		Response=MessageBox(hwnd,szMessage,szCaption,
			MB_OKCANCEL|MB_SETFOREGROUND|MB_ICONEXCLAMATION);
	
		if(Response==IDCANCEL)
			return FALSE;
	}

	assert(pInput);

	context=(PGPContextRef)PGPsc;
	memmgr = PGPGetContextMemoryMgr (context);

 	memset(&wndclass,0x00,sizeof(WNDCLASS));

    wndclass.style = CS_HREDRAW | CS_VREDRAW;                              
    wndclass.lpfnWndProc = (WNDPROC)DefDlgProc; 
    wndclass.cbClsExtra = 0;              
    wndclass.cbWndExtra = DLGWINDOWEXTRA;              
    wndclass.hInstance = g_hinst;       
    wndclass.hIcon = LoadIcon (g_hinst, MAKEINTRESOURCE(IDI_FYEO));
    wndclass.hCursor = LoadCursor (NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = "PGPtempestClass";

    RegisterClass (&wndclass);

	// Globals structure for re-entrancy
	gtv=(GTV *)malloc(sizeof(GTV));
	memset(gtv,0x00,sizeof(GTV));

	// We always wrap tempest viewer at 70
	PGPtempestWrapBuffer (context,
		pInput,70,&pWrappedInput);

	gtv->pInput=pWrappedInput;
	gtv->bDoTempest=TRUE; // Always use Tempest font by default

	DialogReturn = DialogBoxParam(g_hinst, 
		MAKEINTRESOURCE(IDD_TEMPEST), hwnd, 
		TempestDlgProc, (LPARAM) gtv);

	UninitFontBitmap(gtv);

	free(gtv);

	// Free the word wrapped data
	PGPFreeData(pWrappedInput);

	return DialogReturn;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
