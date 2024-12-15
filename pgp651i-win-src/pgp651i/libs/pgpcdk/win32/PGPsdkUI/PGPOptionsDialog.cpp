/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	$Id: PGPOptionsDialog.cpp,v 1.32 1999/03/31 23:22:46 wjb Exp $
____________________________________________________________________________*/

// Public includes:

#include "PGPui32.h"
#include "pgpDialogs.h"
#include "pgpSDKUILibPriv.h"

#define CHECKWIDTH 19
#define INDENT 19
#define LINESPERCONTROL 1.60
#define COMBOSPACER 5
#define COMBOYADJ 3

extern HHOOK hhookKeyboard;
extern HHOOK hhookMouse;

WNDPROC origCheckBoxProc;
WNDPROC origComboBoxProc;

#define OI_CHECKBOX 1
#define OI_COMBOBOX 2

#define MINDLGX 200
#define MINDLGY 200

typedef struct _OPTIONINFO
{
	DWORD dwType;
	HWND hwndOption;
	BOOL bDisableListBoxesIfChecked;
	BOOL bDisableChildrenIfUnchecked;
	struct _OPTIONINFO *poiParent;
	struct _OPTIONINFO *nextSibling;
	struct _OPTIONINFO *nextChild;
	DWORD dwTempValue;
	DWORD *pdwFinalValue;
} OPTIONINFO, *POPTIONINFO;

typedef struct _OPTDLG
{
	PGPOptionListRef options;
	POPTIONINFO headpoi;
	int cxChar;
	int cxCaps;
	int cyChar;
	int MaxWidth;
	int NumLines;
	int cxClient;
	int cyClient;
	int iVscrollPos;
	int iVscrollMax;
	int iHscrollPos;
	int iHscrollMax;
	int iVscrollInc;
	int iHscrollInc;
} OPTDLG, *POPTDLG;

typedef struct _DLGDATA
{
	CPGPOptionsDialogOptions * options;
	HWND hwndOptions;
} DLGDATA, *PDLGDATA;

void EnableOrDisableChildren(POPTIONINFO poi,BOOL bEnable)
{
	POPTIONINFO poiParent;
	POPTIONINFO poiChild;

	if(poi==NULL)
		return;

	// If we check the box, the parent must also be checked
	poiParent=poi->poiParent;

	while(poiParent!=NULL)
	{
		if(bEnable)
		{
			poiParent->dwTempValue=TRUE;
			Button_SetCheck(poiParent->hwndOption,poiParent->dwTempValue);
		}

		if(poiParent->bDisableListBoxesIfChecked)
		{
			HWND hDlg;

			hDlg=GetParent(GetParent(poiParent->hwndOption));

			EnableWindow(GetDlgItem(hDlg,IDC_USER_ID_LIST),!poiParent->dwTempValue);
						
			EnableWindow(GetDlgItem(hDlg,IDC_RECIPIENT_LIST),!poiParent->dwTempValue);
		}

		poiParent=poiParent->poiParent;
	}

	poiChild=poi->nextChild;

	// If we uncheck the box, the siblings must be unchecked
	while(poiChild!=NULL)
	{
		// Loop through the siblings

//		We no longer do the enable disable thing, but keep this
//		code here just in case...
//		EnableWindow(poiChild->hwndOption,bEnable);

		if(!bEnable)
		{
			poiChild->dwTempValue=*(poiChild->pdwFinalValue);
			Button_SetCheck(poiChild->hwndOption,poiChild->dwTempValue);
		}

		// Go through the children
		EnableOrDisableChildren(poiChild->nextChild,bEnable);

		poiChild=poiChild->nextSibling;
	}
}

// Clean up copy of Option Data for CheckBoxes
LRESULT WINAPI MyCheckBoxWndProc(HWND hwnd, UINT msg, 
                                 WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
		case WM_DESTROY:
		{
			POPTIONINFO poi;

			poi=(POPTIONINFO)GetWindowLong(hwnd,GWL_USERDATA);
			free(poi);
			break;
		}
	}

    //  Pass all non-custom messages to old window proc
    return CallWindowProc(origCheckBoxProc, hwnd, 
                           msg, wParam, lParam ) ;
}

// Clean up copy of Option Data for ComboBoxes
LRESULT WINAPI MyComboBoxWndProc(HWND hwnd, UINT msg, 
                                 WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
		case WM_DESTROY:
		{
			POPTIONINFO poi;

			poi=(POPTIONINFO)GetWindowLong(hwnd,GWL_USERDATA);
			free(poi);
			break;
		}
	}

    //  Pass all non-custom messages to old window proc
    return CallWindowProc(origComboBoxProc, hwnd, 
                           msg, wParam, lParam ) ;
}

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
				 int x,int *y,
				 int MaxWidth)
{
	int NumberOfLines;
	HWND hwndDesc;
	SIZE size;

	if(*String==0)
		return NULL;

	GetTextExtentPoint32(hdc,String,strlen(String),&size);

	NumberOfLines=NumLines(hdc,(char *)String,MaxWidth-INDENT-x);

	hwndDesc = CreateWindow ("static", String,
                              WS_CHILDWINDOW | WS_VISIBLE | SS_LEFT,
                              size.cy+INDENT+x,*y,
                              MaxWidth-INDENT-x,size.cy*NumberOfLines,
                              hwndParent, (HMENU) ID,
                              gPGPsdkUILibInst, NULL) ;

	*y=(int)((double)(*y)+
		((double)LINESPERCONTROL + (double)NumberOfLines - (double)1) * (double)size.cy);

	SendMessage(hwndDesc, WM_SETFONT,(WPARAM)hFont, MAKELPARAM(TRUE, 0));

	return hwndDesc;
}

POPTIONINFO MakeCheckBox(HWND hwndParent,
				 POPTIONINFO poiParent,
				 HDC hdc,
				 HFONT hFont,
				 PGPOption *optionData,
				 int x,int *y,
				 int MaxWidth)
{
	HWND hwndCheckBox;
	SIZE size;
	PGPOUICheckboxDesc *Descriptor;
	POPTIONINFO poi;

	poi=(POPTIONINFO)malloc(sizeof(OPTIONINFO));
	memset(poi,0x00,sizeof(OPTIONINFO));
	poi->dwType=OI_CHECKBOX;
	poi->poiParent=poiParent;

	Descriptor = (PGPOUICheckboxDesc *) optionData->value.asPtr;
	poi->pdwFinalValue=(DWORD *)Descriptor->valuePtr;
	poi->dwTempValue=*(poi->pdwFinalValue);

	GetTextExtentPoint32(hdc,Descriptor->title,strlen(Descriptor->title),&size);

	hwndCheckBox = CreateWindow ("button", Descriptor->title,
		WS_CHILD | WS_VISIBLE | BS_CHECKBOX | WS_TABSTOP,
        size.cy+x,*y,
        size.cx+CHECKWIDTH,size.cy,
        hwndParent, (HMENU) Descriptor->itemID,
        gPGPsdkUILibInst, NULL) ;

	*y=(int)((double)*y + (double)size.cy * (double)LINESPERCONTROL);

	SendMessage(hwndCheckBox, WM_SETFONT,
		(WPARAM)hFont, MAKELPARAM(TRUE, 0));

	Button_SetCheck (hwndCheckBox,poi->dwTempValue);

	if(Descriptor->description!=NULL)
		MakeDescription(hwndParent,hdc,hFont,
			5000+Descriptor->itemID,Descriptor->description,x,y,MaxWidth);

	SetWindowLong(hwndCheckBox, GWL_USERDATA, (DWORD)poi);    
	origCheckBoxProc=SubclassWindow(hwndCheckBox,MyCheckBoxWndProc);  

	// Hardcode in the conventional encryption disabling
	// (Ugly I know, but there is no API for this)
	if(!strcmp(Descriptor->title,"Conventional &Encryption"))
	{
		HWND hDlg;

		hDlg=GetParent(hwndParent);

		EnableWindow(GetDlgItem(hDlg,IDC_USER_ID_LIST),!poi->dwTempValue);
						
		EnableWindow(GetDlgItem(hDlg,IDC_RECIPIENT_LIST),!poi->dwTempValue);

		poi->bDisableListBoxesIfChecked=TRUE;
	}
	
	poi->bDisableChildrenIfUnchecked=TRUE;

	poi->hwndOption=hwndCheckBox;

	return poi;
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

POPTIONINFO MakeComboBox(HWND hwndParent,
				 POPTIONINFO poiParent,
				 HDC hdc,
				 HFONT hFont,
				 PGPOption *optionData,				 
				 int x,int *y,
				 int MaxWidth)
{
	HWND hwndText,hwndComboBox;
	SIZE size;
	unsigned int itemIndex;
	PGPOUIPopupListDesc *Descriptor;
	POPTIONINFO poi;

	poi=(POPTIONINFO)malloc(sizeof(OPTIONINFO));
	memset(poi,0x00,sizeof(OPTIONINFO));
	poi->dwType=OI_COMBOBOX;
	poi->poiParent=poiParent;

	Descriptor = (PGPOUIPopupListDesc *) optionData->value.asPtr;
	poi->pdwFinalValue=(DWORD *)Descriptor->valuePtr;
	poi->dwTempValue=*(poi->pdwFinalValue);

	GetTextExtentPoint32(hdc,Descriptor->title,strlen(Descriptor->title),&size);

	hwndText = CreateWindow ("static", Descriptor->title,
		WS_CHILDWINDOW | WS_VISIBLE | SS_LEFT ,
		size.cy+x,*y,
		size.cx,size.cy,
		hwndParent, (HMENU) (1000+Descriptor->itemID),
		gPGPsdkUILibInst, NULL) ;

	SendMessage(hwndText, WM_SETFONT,
		(WPARAM)hFont, MAKELPARAM(TRUE, 0));

	hwndComboBox = CreateWindow ("combobox",NULL,
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | WS_TABSTOP,
		size.cy+size.cx+COMBOSPACER+x,*y-COMBOYADJ,
		MeasureComboBox(hdc,Descriptor),size.cy*6,
        hwndParent, (HMENU)Descriptor->itemID,
		gPGPsdkUILibInst, NULL) ;

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
		(WPARAM)(poi->dwTempValue-1),0);

	SendMessage(hwndComboBox, WM_SETFONT,
		(WPARAM)hFont, MAKELPARAM(TRUE, 0));

	if(Descriptor->description!=NULL)
		MakeDescription(hwndParent,hdc,hFont,
			5000+Descriptor->itemID,Descriptor->description,x,y,MaxWidth);

	SetWindowLong(hwndComboBox, GWL_USERDATA, (DWORD)poi);    
	origComboBoxProc=SubclassWindow(hwndComboBox,MyComboBoxWndProc);  

	poi->hwndOption=hwndComboBox;

	return poi;
}

int MeasureOptionsControls(HDC hdc,int xindent,PGPOptionListRef options)
{
	PGPUInt32	optionIndex	= 0;
	PGPOption	optionData;
	int MaxWidth;
	int ChildWidth;
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
			size.cx=size.cx+xindent;

			if(size.cx>MaxWidth)MaxWidth=size.cx;
		}
		else if( optionData.type == kPGPOptionType_PopupList )
		{
			PGPOUIPopupListDesc		*descriptor;
			
			descriptor = (PGPOUIPopupListDesc *) optionData.value.asPtr;

			GetTextExtentPoint32(hdc,descriptor->title,strlen(descriptor->title),&size);
			size.cx=size.cx+xindent;
			size.cx=size.cx+COMBOSPACER;
			size.cx=size.cx+MeasureComboBox(hdc,descriptor);

			if(size.cx>MaxWidth)MaxWidth=size.cx;
		}		

		// Get the max width of the indented children
		ChildWidth=MeasureOptionsControls(hdc,xindent+INDENT,optionData.subOptions);

		if(ChildWidth>MaxWidth)
			MaxWidth=ChildWidth;

		++optionIndex;
	}

	return MaxWidth;
}

	POPTIONINFO
CreateOptionsControls(HWND hwndOptionsControl,
					  POPTIONINFO poiParent,
					  HDC hdc,HFONT hFont,
					  int xpos,int *ypos,int MaxWidth,
					  PGPOptionListRef options)
{
	PGPError	err 		= kPGPError_NoErr;
	PGPUInt32	optionIndex	= 0;
	PGPOption	optionData;
	POPTIONINFO FirstOption;
	POPTIONINFO OldOption;
	POPTIONINFO poi;

	if(options==NULL)
		return(NULL);

	FirstOption=OldOption=NULL;

	/* Build list of checkboxes and/or popups */
	
	while( IsntPGPError( pgpGetIndexedOption( options,
					optionIndex, FALSE, &optionData ) ) )
	{
		if( optionData.type == kPGPOptionType_Checkbox )
		{
			poi = MakeCheckBox(
				hwndOptionsControl,
				poiParent,
				hdc,hFont,
				&optionData,
				xpos,ypos,MaxWidth);
		}
		else if( optionData.type == kPGPOptionType_PopupList )
		{
			poi = MakeComboBox(
				hwndOptionsControl,
				poiParent,
				hdc,hFont,
				&optionData,
				xpos,ypos,MaxWidth);
		}	
		
		poi->nextChild=
			CreateOptionsControls(hwndOptionsControl,poi,hdc,hFont,
				xpos+INDENT,ypos,MaxWidth,
				optionData.subOptions);

		if(poi->bDisableChildrenIfUnchecked)
		{
			EnableOrDisableChildren(poi,poi->dwTempValue);
		}

		if(OldOption!=NULL)
			OldOption->nextSibling=poi;

		OldOption=poi;

		if(FirstOption==NULL)
			FirstOption=poi;

		++optionIndex;
	}

	return( FirstOption );
}

LRESULT CALLBACK CustomOptionsWndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
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
			POPTDLG pod;

			pod=(POPTDLG)malloc(sizeof(OPTDLG));
			memset(pod,0x00,sizeof(OPTDLG));
			
			SetWindowLong(hwnd, GWL_USERDATA, (DWORD)pod);    

			lpcs=(LPCREATESTRUCT)lParam;
			options=(PGPOptionListRef)lpcs->lpCreateParams;

			pod->options=options;

			SystemParametersInfo (SPI_GETICONTITLELOGFONT, 
				sizeof(LOGFONT), &lf, 0);

			hFont=CreateFontIndirect (&lf);
				   
			hdc=GetDC(hwnd);
			SelectObject(hdc,hFont);

			GetTextMetrics (hdc, &tm) ;
			pod->cxChar = tm.tmAveCharWidth ;
			pod->cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * (pod->cxChar) / 2 ;
			pod->cyChar = tm.tmHeight + tm.tmExternalLeading ;

			pod->MaxWidth = GetClientWidth(hwnd)-2*(pod->cyChar);

			HeaderWidths=MeasureOptionsControls(hdc,0,options);

			if(HeaderWidths>pod->MaxWidth)
				pod->MaxWidth=HeaderWidths;

			ypos=pod->cyChar;

			pod->headpoi=CreateOptionsControls(hwnd,NULL,hdc,hFont,0,&ypos,pod->MaxWidth,options);

			ypos+=pod->cyChar; // margin at bottom?

			ReleaseDC(hwnd,hdc);

			pod->NumLines=ypos/pod->cyChar;

			return 0 ;
		}

		case WM_DESTROY:
		{
			POPTDLG pod;

			pod=(POPTDLG)GetWindowLong(hwnd,GWL_USERDATA);
			free(pod);
			break;
		}

		case WM_COMMAND:
		{
			HWND hwndItem;

			hwndItem=(HWND)lParam;

			if(hwndItem!=NULL)
			{			
				POPTIONINFO poi;
				char WinClass[255];

				poi=(POPTIONINFO)GetWindowLong(hwndItem,GWL_USERDATA);

				GetClassName(hwndItem,WinClass,255);

				if(!strcmp(WinClass,"Button"))
				{
					poi->dwTempValue=!poi->dwTempValue;
					Button_SetCheck(hwndItem,poi->dwTempValue);

					if(poi->bDisableListBoxesIfChecked)
					{
						// Hardcode in the conventional encryption disabling
						// (Ugly I know, but there is no API for this)
						HWND hDlg;

						hDlg=GetParent(hwnd);

						EnableWindow(GetDlgItem(hDlg,IDC_USER_ID_LIST),!poi->dwTempValue);
						
						EnableWindow(GetDlgItem(hDlg,IDC_RECIPIENT_LIST),!poi->dwTempValue);
					}

					if(poi->bDisableChildrenIfUnchecked)
					{
						EnableOrDisableChildren(poi,poi->dwTempValue);
					}
				}

				if(!strcmp(WinClass,"ComboBox"))
				{
					if(HIWORD(wParam)==CBN_SELCHANGE)
					{
						poi->dwTempValue=SendMessage(hwndItem,CB_GETCURSEL,0,0)+1;
					}
				}
			}
			return 0;
		}

		case WM_SIZE :
		{
			int x,y;
			POPTDLG pod;

			pod=(POPTDLG)GetWindowLong(hwnd,GWL_USERDATA);

			pod->cxClient = LOWORD (lParam) ;
			pod->cyClient = HIWORD (lParam) ;

//			pod->iVscrollPos=pod->iHscrollPos=0;

			pod->iVscrollMax = max (0, pod->NumLines - pod->cyClient/ pod->cyChar) ;
			   
			x = min (pod->iVscrollPos, pod->iVscrollMax) ;
			y = x - pod->iVscrollPos;

			ScrollWindow (hwnd, 0, -(pod->cyChar) * y, NULL, NULL) ;

			pod->iVscrollPos=x;
				
			SetScrollRange (hwnd, SB_VERT, 0, pod->iVscrollMax, FALSE) ;
			SetScrollPos   (hwnd, SB_VERT, pod->iVscrollPos, TRUE) ;       

			pod->iHscrollMax = max (0, 
				2 + (pod->MaxWidth + 2*(pod->cyChar) - pod->cxClient) / pod->cxChar) ;
			x = min (pod->iHscrollPos, pod->iHscrollMax) ;
			y = x - pod->iHscrollPos;

			ScrollWindow (hwnd, -(pod->cxChar) * y, 0, NULL, NULL) ;

			pod->iHscrollPos=x;

			SetScrollRange (hwnd, SB_HORZ, 0, pod->iHscrollMax, FALSE) ;
			SetScrollPos   (hwnd, SB_HORZ, pod->iHscrollPos, TRUE) ;
			return 0 ;
		}

		case WM_VSCROLL :
		{
			POPTDLG pod;

			pod=(POPTDLG)GetWindowLong(hwnd,GWL_USERDATA);

			switch (LOWORD (wParam))
			{
				case SB_TOP :
					pod->iVscrollInc = -(pod->iVscrollPos) ;
					break ;

				case SB_BOTTOM :
					pod->iVscrollInc = pod->iVscrollMax - (pod->iVscrollPos) ;
					break ;

				case SB_LINEUP :
					pod->iVscrollInc = -1 ;
					break ;

				case SB_LINEDOWN :
					pod->iVscrollInc = 1 ;
					break ;

				case SB_PAGEUP :
					pod->iVscrollInc = min (-1, -(pod->cyClient) / pod->cyChar) ;
					break ;

				case SB_PAGEDOWN :
					pod->iVscrollInc = max (1, pod->cyClient / pod->cyChar) ;
					break ;

				case SB_THUMBTRACK :
					pod->iVscrollInc = HIWORD (wParam) - pod->iVscrollPos ;
					break ;

				default :
					pod->iVscrollInc = 0 ;
			}

			pod->iVscrollInc = max (-(pod->iVscrollPos),
				min (pod->iVscrollInc, pod->iVscrollMax - (pod->iVscrollPos))) ;

			if (pod->iVscrollInc != 0)
			{
				pod->iVscrollPos += pod->iVscrollInc ;
				ScrollWindow (hwnd, 0, -(pod->cyChar) * pod->iVscrollInc, NULL, NULL) ;
				SetScrollPos (hwnd, SB_VERT, pod->iVscrollPos, TRUE) ;
				UpdateWindow (hwnd) ;
			}

			return 0 ;
		}

		case WM_HSCROLL :
		{
			POPTDLG pod;

			pod=(POPTDLG)GetWindowLong(hwnd,GWL_USERDATA);

			switch (LOWORD (wParam))
			{
				case SB_LINEUP :
					pod->iHscrollInc = -1 ;
					break ;

				case SB_LINEDOWN :
					pod->iHscrollInc = 1 ;
					break ;

				case SB_PAGEUP :
					pod->iHscrollInc = -8 ;
					break ;

				case SB_PAGEDOWN :
					pod->iHscrollInc = 8 ;
					break ;

				case SB_THUMBPOSITION :
					pod->iHscrollInc = HIWORD (wParam) - pod->iHscrollPos ;
					break ;

				default :
					pod->iHscrollInc = 0 ;
			}

			pod->iHscrollInc = max (-(pod->iHscrollPos),
				min (pod->iHscrollInc, pod->iHscrollMax - (pod->iHscrollPos))) ;

			if (pod->iHscrollInc != 0)
			{
				pod->iHscrollPos += pod->iHscrollInc ;
				ScrollWindow (hwnd, -(pod->cxChar) * pod->iHscrollInc, 0, NULL, NULL) ;
				SetScrollPos (hwnd, SB_HORZ, pod->iHscrollPos, TRUE) ;
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
	wndclass.hInstance     = gPGPsdkUILibInst ;
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
		hwndParent,NULL,gPGPsdkUILibInst,(LPVOID)options) ;

	return hwndOptionsWindow;
}

// Helper function for SaveOptionSettings
void RecurseOptionSettings(POPTIONINFO poi)
{
	while(poi!=NULL)
	{
		*(poi->pdwFinalValue)=poi->dwTempValue;

		// Go through the children
		RecurseOptionSettings(poi->nextChild);

		poi=poi->nextSibling;
	}
}

// Save all temp values to output (Use for OK cases...
// For cancel everything is thrown away)
void SaveOptionSettings(HWND hwndOptionsWindow)
{
	POPTDLG pod;

	if(hwndOptionsWindow!=NULL)
	{
		pod=(POPTDLG)GetWindowLong(hwndOptionsWindow,GWL_USERDATA);
		RecurseOptionSettings(pod->headpoi);
	}
}

void ResizeOptionsControl(HWND hwndOptions,
						  int x,int y,
						  int dx,int dy)
{
	if(hwndOptions!=NULL)
	{
		InvalidateRect(hwndOptions,NULL,TRUE);

		MoveWindow(hwndOptions,
			x,y,
			dx,dy,TRUE);
	}
}

void MoveOptionsDialog(HWND hDlg,HWND hwndChild,
					   int cxClient,int cyClient)
{
	ResizeOptionsControl(hwndChild,
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
	switch (uMsg) 
	{
		case WM_INITDIALOG:
		{    
			RECT rc;
			CPGPOptionsDialogOptions *options;
			PDLGDATA pdd;

			pdd=(PDLGDATA)lParam;
			SetWindowLong(hDlg, GWL_USERDATA, (DWORD)pdd);    

			options=pdd->options;

			if(options->mPrompt!=NULL)
				SetWindowText(hDlg,options->mPrompt);

			GetClientRect(hDlg,&rc);
	
			pdd->hwndOptions=CreateOptionsControl(hDlg,
				options->mClientOptions,
				10,10,rc.right-rc.left-10,
				rc.bottom-rc.top-20);

			MoveOptionsDialog(hDlg,pdd->hwndOptions,
				rc.right-rc.left,
				rc.bottom-rc.top);

			return TRUE;
		}

		case WM_SIZE :
		{
			int cxClient, cyClient;
			CPGPOptionsDialogOptions *options;
			PDLGDATA pdd;

			pdd=(PDLGDATA)GetWindowLong(hDlg,GWL_USERDATA);

			options=pdd->options;

			cxClient = LOWORD (lParam) ;
			cyClient = HIWORD (lParam) ;

			MoveOptionsDialog(hDlg,pdd->hwndOptions,
				cxClient,
				cyClient);
			break ;
		}

		case WM_GETMINMAXINFO:
		{
			MINMAXINFO* lpmmi;

		    lpmmi = (MINMAXINFO*) lParam;
    		lpmmi->ptMinTrackSize.x = MINDLGX;
    		lpmmi->ptMinTrackSize.y = MINDLGY;
            break;
		}

		case WM_COMMAND:
		{
			switch(wParam)
			{
				case IDOK:
				{
					PDLGDATA pdd;

					pdd=(PDLGDATA)GetWindowLong(hDlg,GWL_USERDATA);

					SaveOptionSettings(pdd->hwndOptions);
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
		case WM_DESTROY:
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
	PDLGDATA pdd;

	pdd=(PDLGDATA)malloc(sizeof(DLGDATA));
	memset(pdd,0x00,sizeof(DLGDATA));

	pdd->options=options;

	InitRandomKeyHook(&hhookKeyboard,&hhookMouse);

	DialogBoxParam(gPGPsdkUILibInst,
		MAKEINTRESOURCE(IDD_OPTIONSDIALOG),
		options->mHwndParent,
		(DLGPROC)OptionsDialogProc,
		(LPARAM)pdd);

	UninitRandomKeyHook(hhookKeyboard,hhookMouse);

	free(pdd);

	return( err );
}
