/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	$Id: PGPOptionsDialog.cpp,v 1.16.2.2.2.1 1998/11/12 03:24:17 heller Exp $
____________________________________________________________________________*/

// Public includes:

#include "PGPui32.h"
#include "pgpDialogs.h"

#define CHECKWIDTH 19
#define INDENT 19
#define LINESPERCONTROL 1.60
#define COMBOSPACER 5
#define COMBOYADJ 3

extern HINSTANCE g_hInst;
extern HHOOK hhookKeyboard;
extern HHOOK hhookMouse;

int NumLines (HDC hdc, char *pText, int Width)
{
	int  iBreakCount ;
	PSTR pBegin, pEnd ;
	SIZE size ;
	int numrows;

	numrows = 0;
	do                               // for each text line
	{
		iBreakCount = 0 ;
	
		pBegin = pText ;

		do                           // until the line is known
		{
			pEnd = pText ;

			while (*pText != '\0' && *pText++ != ' ') ;
			if (*pText == '\0')
				break ;
                                     // for each space, calculate extents
			iBreakCount++ ;
			SetTextJustification (hdc, 0, 0) ;
			GetTextExtentPoint32 (hdc, pBegin, pText - pBegin - 1, &size) ;
		}
		while ((int) size.cx < Width) ;

		iBreakCount-- ;
		while (*(pEnd - 1) == ' ')   // eliminate trailing blanks
		{
			pEnd-- ;
			iBreakCount-- ;
		}

		if (*pText == '\0' || iBreakCount <= 0)
			pEnd = pText;

		SetTextJustification (hdc, 0, 0) ;
		GetTextExtentPoint32 (hdc, pBegin, pEnd - pBegin, &size) ;

		numrows++;
		pText = pEnd ;
	} while (*pText);

	return numrows;
}

int GetClientWidth(HWND hwnd)
{
	RECT rc;

	GetClientRect(hwnd,&rc);
	return(rc.right-rc.left-GetSystemMetrics(SM_CXVSCROLL));
}

HWND MakeDescription(HWND hwndParent,
				 HDC hdc,
				 HFONT hFont,
				 int ID,
				 const char *String,
				 int *y,
				 int MaxWidth)
{
	int NumberOfLines;
	HWND hwndDesc;
	SIZE size;

	if(*String==0)
		return NULL;

	GetTextExtentPoint32(hdc,String,strlen(String),&size);

	NumberOfLines=NumLines(hdc,(char *)String,MaxWidth-INDENT);

	hwndDesc = CreateWindow ("static", String,
                              WS_CHILDWINDOW | WS_VISIBLE | SS_LEFT,
                              size.cy+INDENT,*y,
                              MaxWidth-INDENT,size.cy*NumberOfLines,
                              hwndParent, (HMENU) ID,
                              g_hInst, NULL) ;

	*y=(int)((double)(*y)+
		((double)LINESPERCONTROL + (double)NumberOfLines - (double)1) * (double)size.cy);

	SendMessage(hwndDesc, WM_SETFONT,(WPARAM)hFont, MAKELPARAM(TRUE, 0));

	return hwndDesc;
}

HWND MakeCheckBox(HWND hwndParent,
				 HDC hdc,
				 HFONT hFont,
				 PGPOUICheckboxDesc *Descriptor,
				 int *y,
				 int MaxWidth)
{
	HWND hwndCheckBox;
	SIZE size;

	GetTextExtentPoint32(hdc,Descriptor->title,strlen(Descriptor->title),&size);

	hwndCheckBox = CreateWindow ("button", Descriptor->title,
		WS_CHILD | WS_VISIBLE | BS_CHECKBOX | WS_TABSTOP,
        size.cy,*y,
        size.cx+CHECKWIDTH,size.cy,
        hwndParent, (HMENU) Descriptor->itemID,
        g_hInst, NULL) ;

	*y=(int)((double)*y + (double)size.cy * (double)LINESPERCONTROL);

	SendMessage(hwndCheckBox, WM_SETFONT,
		(WPARAM)hFont, MAKELPARAM(TRUE, 0));

	Button_SetCheck (hwndCheckBox,*(Descriptor->valuePtr));

	if(Descriptor->description!=NULL)
		MakeDescription(hwndParent,hdc,hFont,
			5000+Descriptor->itemID,Descriptor->description,y,MaxWidth);

	SetWindowLong(hwndCheckBox, GWL_USERDATA, (DWORD)Descriptor->valuePtr);    

	return hwndCheckBox;
}

int MeasureComboBox(HDC hdc,PGPOUIPopupListDesc *Descriptor)
{
	SIZE size;
	unsigned int itemIndex;
	int ComboWidth;

	ComboWidth=0;

	// Find the longest text element
	for(itemIndex = 0; itemIndex < Descriptor->numListItems; itemIndex++)
	{
		if( IsntNull( Descriptor->listItems[itemIndex] ) )
		{
			GetTextExtentPoint32(hdc,Descriptor->listItems[itemIndex],
				strlen(Descriptor->listItems[itemIndex]),&size);
			if(size.cx>ComboWidth)ComboWidth=size.cx;
		}
	}

	ComboWidth=ComboWidth+2*GetSystemMetrics(SM_CXVSCROLL);

	return ComboWidth;
}

HWND MakeComboBox(HWND hwndParent,
				 HDC hdc,
				 HFONT hFont,
				 PGPOUIPopupListDesc *Descriptor,
				 int *y,
				 int MaxWidth)
{
	HWND hwndText,hwndComboBox;
	SIZE size;
	unsigned int itemIndex;

	GetTextExtentPoint32(hdc,Descriptor->title,strlen(Descriptor->title),&size);

	hwndText = CreateWindow ("static", Descriptor->title,
		WS_CHILDWINDOW | WS_VISIBLE | SS_LEFT ,
		size.cy,*y,
		size.cx,size.cy,
		hwndParent, (HMENU) (1000+Descriptor->itemID),
		g_hInst, NULL) ;

	SendMessage(hwndText, WM_SETFONT,
		(WPARAM)hFont, MAKELPARAM(TRUE, 0));

	hwndComboBox = CreateWindow ("combobox",NULL,
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | WS_TABSTOP,
		size.cy+size.cx+COMBOSPACER,*y-COMBOYADJ,
		MeasureComboBox(hdc,Descriptor),size.cy*6,
        hwndParent, (HMENU)Descriptor->itemID,
		g_hInst, NULL) ;

		*y=(int)((double)*y + (double)size.cy * (double)(LINESPERCONTROL));

	for(itemIndex = 0; itemIndex < Descriptor->numListItems; itemIndex++)
	{
		if( IsntNull( Descriptor->listItems[itemIndex] ) )
		{
			SendMessage (hwndComboBox, CB_ADDSTRING, 0, 
				(LPARAM)Descriptor->listItems[itemIndex]);
		}
	}

	SendMessage(hwndComboBox,CB_SETCURSEL,
		(WPARAM)(*Descriptor->valuePtr-1),0);

	SendMessage(hwndComboBox, WM_SETFONT,
		(WPARAM)hFont, MAKELPARAM(TRUE, 0));

	if(Descriptor->description!=NULL)
		MakeDescription(hwndParent,hdc,hFont,
			5000+Descriptor->itemID,Descriptor->description,y,MaxWidth);

	SetWindowLong(hwndComboBox, GWL_USERDATA, (DWORD)Descriptor->valuePtr);    

	return hwndComboBox;
}

int MeasureOptionsControls(HDC hdc,PGPOptionListRef options)
{
	PGPUInt32	optionIndex	= 0;
	PGPOption	optionData;
	int MaxWidth;
	SIZE size;

	if(options==NULL)
		return(0);

	MaxWidth=0;

	/* Build list of checkboxes and/or popups */
	
	while( IsntPGPError( pgpGetIndexedOption( options,
					optionIndex, FALSE, &optionData ) ) )
	{
		if( optionData.type == kPGPOptionType_Checkbox )
		{
			PGPOUICheckboxDesc		*descriptor;
			
			descriptor = (PGPOUICheckboxDesc *) optionData.value.asPtr;

			GetTextExtentPoint32(hdc,descriptor->title,strlen(descriptor->title),&size);

			if(size.cx>MaxWidth)MaxWidth=size.cx;
		}
		else if( optionData.type == kPGPOptionType_PopupList )
		{
			PGPOUIPopupListDesc		*descriptor;
			
			descriptor = (PGPOUIPopupListDesc *) optionData.value.asPtr;

			GetTextExtentPoint32(hdc,descriptor->title,strlen(descriptor->title),&size);
			size.cx=size.cx+COMBOSPACER;
			size.cx=size.cx+MeasureComboBox(hdc,descriptor);

			if(size.cx>MaxWidth)MaxWidth=size.cx;
		}		
		++optionIndex;
	}

	return MaxWidth;
}

	PGPError
CreateOptionsControls(HWND hwndOptionsControl,
					  HDC hdc,HFONT hFont,
					  int *ypos,int MaxWidth,
					  PGPOptionListRef options)
{
	PGPError	err 		= kPGPError_NoErr;
	PGPUInt32	optionIndex	= 0;
	PGPOption	optionData;
	
	if(options==NULL)
		return( err);

	/* Build list of checkboxes and/or popups */
	
	while( IsntPGPError( pgpGetIndexedOption( options,
					optionIndex, FALSE, &optionData ) ) )
	{
		if( optionData.type == kPGPOptionType_Checkbox )
		{
			PGPOUICheckboxDesc		*descriptor;
			HWND checkboxView;
			
			descriptor = (PGPOUICheckboxDesc *) optionData.value.asPtr;

			checkboxView = MakeCheckBox(
				hwndOptionsControl,
				hdc,hFont,
				descriptor,
				ypos,MaxWidth);
		}
		else if( optionData.type == kPGPOptionType_PopupList )
		{
			PGPOUIPopupListDesc		*descriptor;
			HWND popupView;
			
			descriptor = (PGPOUIPopupListDesc *) optionData.value.asPtr;

			popupView = MakeComboBox(
				hwndOptionsControl,
				hdc,hFont,
				descriptor,
				ypos,MaxWidth);
		}		
		++optionIndex;
	}

	return( err );
}

LRESULT CALLBACK CustomOptionsWndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
	{
	static int  cxChar, cxCaps, cyChar,MaxWidth,NUMLINES,cxClient,cyClient,
		iVscrollPos, iVscrollMax, iHscrollPos, iHscrollMax,
		iVscrollInc,iHscrollInc;

	switch (iMsg)
	{
		case WM_CREATE :
		{
			PGPOptionListRef options;
			LPCREATESTRUCT lpcs;
			HFONT hFont;
			LOGFONT lf;
			int ypos;
			HDC hdc;
			TEXTMETRIC tm;
			int HeaderWidths;
			
			lpcs=(LPCREATESTRUCT)lParam;
			options=(PGPOptionListRef)lpcs->lpCreateParams;
			SetWindowLong(hwnd, GWL_USERDATA, (DWORD)options);    

			SystemParametersInfo (SPI_GETICONTITLELOGFONT, 
				sizeof(LOGFONT), &lf, 0);

			hFont=CreateFontIndirect (&lf);
				   
			hdc=GetDC(hwnd);
			SelectObject(hdc,hFont);

			GetTextMetrics (hdc, &tm) ;
			cxChar = tm.tmAveCharWidth ;
			cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * cxChar / 2 ;
			cyChar = tm.tmHeight + tm.tmExternalLeading ;

			MaxWidth = GetClientWidth(hwnd)-2*cyChar;

			HeaderWidths=MeasureOptionsControls(hdc,options);

			if(HeaderWidths>MaxWidth)MaxWidth=HeaderWidths;

			ypos=cyChar;

			CreateOptionsControls(hwnd,hdc,hFont,&ypos,MaxWidth,options);

			ypos+=cyChar; // margin at bottom?

			ReleaseDC(hwnd,hdc);

			NUMLINES=ypos/cyChar;

			return 0 ;
		}

		case WM_COMMAND:
		{
			HWND hwndItem;

			hwndItem=(HWND)lParam;

			if(hwndItem!=NULL)
			{			
				DWORD *Data;
				char WinClass[255];

				Data=(DWORD *)GetWindowLong(hwndItem,GWL_USERDATA);
				GetClassName(hwndItem,WinClass,255);

				if(!strcmp(WinClass,"Button"))
				{
					*Data=!*Data;
					Button_SetCheck(hwndItem,*Data);
				}

				if(!strcmp(WinClass,"ComboBox"))
				{
					if(HIWORD(wParam)==CBN_SELCHANGE)
					{
						*Data=SendMessage(hwndItem,CB_GETCURSEL,0,0)+1;
					}
				}
			}
			return 0;
		}

			case WM_SIZE :
			{
				int x,y;

               cxClient = LOWORD (lParam) ;
               cyClient = HIWORD (lParam) ;

			   iVscrollPos=iHscrollPos=0;

               iVscrollMax = max (0, NUMLINES - cyClient/ cyChar) ;
			   
               x = min (iVscrollPos, iVscrollMax) ;
			   y = x - iVscrollPos;

               ScrollWindow (hwnd, 0, -cyChar * y, NULL, NULL) ;

			   iVscrollPos=x;
				
               SetScrollRange (hwnd, SB_VERT, 0, iVscrollMax, FALSE) ;
   			   SetScrollPos   (hwnd, SB_VERT, iVscrollPos, TRUE) ;       

               iHscrollMax = max (0, 2 + (MaxWidth + 2*cyChar - cxClient) / cxChar) ;
               x = min (iHscrollPos, iHscrollMax) ;
			   y = x - iHscrollPos;

			   ScrollWindow (hwnd, -cxChar * y, 0, NULL, NULL) ;

			   iHscrollPos=x;

               SetScrollRange (hwnd, SB_HORZ, 0, iHscrollMax, FALSE) ;
               SetScrollPos   (hwnd, SB_HORZ, iHscrollPos, TRUE) ;
               return 0 ;
			}

          case WM_VSCROLL :
			  {
               switch (LOWORD (wParam))
                    {
                    case SB_TOP :
                         iVscrollInc = -iVscrollPos ;
                         break ;

                    case SB_BOTTOM :
                         iVscrollInc = iVscrollMax - iVscrollPos ;
                         break ;

                    case SB_LINEUP :
                         iVscrollInc = -1 ;
                         break ;

                    case SB_LINEDOWN :
                         iVscrollInc = 1 ;
                         break ;

                    case SB_PAGEUP :
                         iVscrollInc = min (-1, -cyClient / cyChar) ;
                         break ;

                    case SB_PAGEDOWN :
                         iVscrollInc = max (1, cyClient / cyChar) ;
                         break ;

                    case SB_THUMBTRACK :
                         iVscrollInc = HIWORD (wParam) - iVscrollPos ;
                         break ;

                    default :
                         iVscrollInc = 0 ;
                    }
               iVscrollInc = max (-iVscrollPos,
                             min (iVscrollInc, iVscrollMax - iVscrollPos)) ;

               if (iVscrollInc != 0)
                    {
                    iVscrollPos += iVscrollInc ;
                    ScrollWindow (hwnd, 0, -cyChar * iVscrollInc, NULL, NULL) ;
                    SetScrollPos (hwnd, SB_VERT, iVscrollPos, TRUE) ;
                    UpdateWindow (hwnd) ;
                    }
               return 0 ;
			  }

          case WM_HSCROLL :
		{
               switch (LOWORD (wParam))
                    {
                    case SB_LINEUP :
                         iHscrollInc = -1 ;
                         break ;

                    case SB_LINEDOWN :
                         iHscrollInc = 1 ;
                         break ;

                    case SB_PAGEUP :
                         iHscrollInc = -8 ;
                         break ;

                    case SB_PAGEDOWN :
                         iHscrollInc = 8 ;
                         break ;

                    case SB_THUMBPOSITION :
                         iHscrollInc = HIWORD (wParam) - iHscrollPos ;
                         break ;

                    default :
                         iHscrollInc = 0 ;
			}

			iHscrollInc = max (-iHscrollPos,
				min (iHscrollInc, iHscrollMax - iHscrollPos)) ;

			if (iHscrollInc != 0)
			{
				iHscrollPos += iHscrollInc ;
				ScrollWindow (hwnd, -cxChar * iHscrollInc, 0, NULL, NULL) ;
				SetScrollPos (hwnd, SB_HORZ, iHscrollPos, TRUE) ;
			}
			return 0 ;
		}

	}

	return DefWindowProc (hwnd, iMsg, wParam, lParam) ;
}

HWND CreateOptionsControl(HWND hwndParent,
	PGPOptionListRef options,
	int x,int y,int dx,int dy)
{
	static char szClassName[] = "CustomOptionsControl" ;
	WNDCLASSEX  wndclass ;
	HWND hwndOptionsWindow;

	wndclass.cbSize        = sizeof (wndclass) ;
	wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
	wndclass.lpfnWndProc   = CustomOptionsWndProc ;
	wndclass.cbClsExtra    = 0 ;
	wndclass.cbWndExtra    = 0 ;
	wndclass.hInstance     = g_hInst ;
	wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION) ;
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
	wndclass.hbrBackground = (HBRUSH)COLOR_WINDOW; 
	wndclass.lpszMenuName  = NULL ;
	wndclass.lpszClassName = szClassName ;
	wndclass.hIconSm       = LoadIcon (NULL, IDI_APPLICATION) ;

	RegisterClassEx (&wndclass) ;

	hwndOptionsWindow = CreateWindowEx (WS_EX_CONTROLPARENT,szClassName, "CustomOptionsWindow",
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | WS_DLGFRAME | WS_TABSTOP,
		x,y,
		dx,dy,
		hwndParent,NULL,g_hInst,(LPVOID)options) ;

	return hwndOptionsWindow;
}

void MoveOptionsControl(HWND hDlg,HWND *hwndChild,
					   PGPOptionListRef options,
					   int x,int y,
					   int dx,int dy)
{
	if(*hwndChild!=NULL)
	{
		InvalidateRect(*hwndChild,NULL,TRUE);
		DestroyWindow(*hwndChild);
	}
	
	*hwndChild=CreateOptionsControl(hDlg,
		options,
		x,y,
		dx,dy);
}

void MoveOptionsDialog(HWND hDlg,HWND *hwndChild,
					   CPGPOptionsDialogOptions *options,
					   int cxClient,int cyClient)
{
	MoveOptionsControl(hDlg,hwndChild,options->mClientOptions,
		10,10,cxClient-20,cyClient-55);

	InvalidateRect(GetDlgItem(hDlg, IDOK),NULL,TRUE);

	MoveWindow(GetDlgItem(hDlg, IDOK),
		cxClient-175,cyClient-35,
		75,25,TRUE);

	InvalidateRect(GetDlgItem(hDlg, IDCANCEL),NULL,TRUE);

	MoveWindow(GetDlgItem(hDlg, IDCANCEL),
		cxClient-85,cyClient-35,
		75,25,TRUE);
}


BOOL CALLBACK 
OptionsDialogProc (HWND hDlg, 
				  UINT uMsg, 
				  WPARAM wParam, 
				  LPARAM lParam) 
{
	static HWND hwndChild;

	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{    
			RECT rc;
			CPGPOptionsDialogOptions *options;

			options=(CPGPOptionsDialogOptions *)lParam;
		    SetWindowLong(hDlg,GWL_USERDATA,(long)options);

			if(options->mPrompt!=NULL)
				SetWindowText(hDlg,options->mPrompt);

			GetClientRect(hDlg,&rc);
	
			hwndChild=NULL;

			MoveOptionsDialog(hDlg,&hwndChild,options,
				rc.right-rc.left,
				rc.bottom-rc.top);
			return TRUE;
		}

		case WM_SIZE :
		{
			int cxClient, cyClient;
			CPGPOptionsDialogOptions *options;

			options=(CPGPOptionsDialogOptions *)
				GetWindowLong(hDlg,GWL_USERDATA);

			cxClient = LOWORD (lParam) ;
			cyClient = HIWORD (lParam) ;

			MoveOptionsDialog(hDlg,&hwndChild,options,
				cxClient,
				cyClient);
			break ;
		}

		case WM_COMMAND:
		{
			switch(wParam)
			{
				case IDOK:
				{
					EndDialog(hDlg, TRUE);
					break;
				}

				case IDCANCEL:
				{
					EndDialog(hDlg, FALSE);
					break;
				}
			}
			break;
		}

		case WM_QUIT:
		case WM_CLOSE:
		case WM_DESTROY :
		{
			EndDialog(hDlg, FALSE);
			break ;
		}
	}

	return FALSE;
}

	PGPError
pgpOptionsDialogPlatform(
	PGPContextRef				context,
	CPGPOptionsDialogOptions 	*options)
{
	PGPError err = kPGPError_NoErr;

	InitRandomKeyHook(&hhookKeyboard,&hhookMouse);

	DialogBoxParam(g_hInst,
		MAKEINTRESOURCE(IDD_OPTIONSDIALOG),
		options->mHwndParent,
		(DLGPROC)OptionsDialogProc,
		(LPARAM)options);

	UninitRandomKeyHook(hhookKeyboard,hhookMouse);

	return( err );
}
