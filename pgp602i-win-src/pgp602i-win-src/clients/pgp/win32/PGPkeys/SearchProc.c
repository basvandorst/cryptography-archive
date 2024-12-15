/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: SearchProc.c,v 1.99.2.2 1998/10/28 23:11:51 pbj Exp $
____________________________________________________________________________*/

// System Headers 
#include <windows.h>
#include <windowsx.h>
#include <winsock.h>
#include <commctrl.h>

// PGPsdk Headers
#include "pgpKeys.h"
#include "pgpErrors.h"
#include "pgpKeyServer.h"
#include "pgpKeyServerPrefs.h"
#include "pgpBuildFlags.h"
#include "pgpUtilities.h"
#include "pgpClientPrefs.h"

// Shared Headers 
#include "PGPcl.h"
#include "PGPpkHlp.h" 

// Project Headers
#include "Search.h"
#include "Choice.h"
#include "SearchFilter.h"
#include "SearchCommon.h"
#include "resource.h"

typedef struct _SEARCHTHREADSTRUCT
{
	HWND				hwnd;
	HANDLE				threadHandle;
	DWORD				threadID;
	PGPFilterRef		filter;
	PGPKeyServerEntry	keyserverEntry;
	PGPKeySetRef		keySet;		
	PGPKeyServerRef		server;
	BOOL				bCancel;
	BOOL				bAlreadyAsked;

}SEARCHTHREADSTRUCT,*PSEARCHTHREADSTRUCT;


extern PGPContextRef	g_Context; 
extern PGPtlsContextRef g_TLSContext;
extern HINSTANCE		g_hInst;
extern CHAR				g_szHelpFile[MAX_PATH];

static DWORD aSearchIds[] = {			// Help IDs
    IDC_SEARCH,		IDH_PGPPKSEARCH_SEARCH, 
    IDC_STOP,		IDH_PGPPKSEARCH_STOP, 
    IDC_CLEAR,		IDH_PGPPKSEARCH_CLEAR, 
    IDC_LOCATION,	IDH_PGPPKSEARCH_LOCATION, 
    IDC_ATTRIBUTE,	IDH_PGPPKSEARCH_ATTRIBUTE, 
    IDC_VERB,		IDH_PGPPKSEARCH_VERB, 
    IDC_MORE,		IDH_PGPPKSEARCH_MORE, 
    IDC_FEWER,		IDH_PGPPKSEARCH_FEWER, 
    IDC_PENDING,	IDH_PGPPKSEARCH_PENDING,
    0,0 
}; 

PGPError PerformSearch(HWND hwndLocation, 
					   PSEARCHTHREADSTRUCT psts,
					   BOOL	bPending,
					   long* flags,
					   PGPKeySetRef* pKeySetOut);

PGPError SearchEventHandler(PGPContextRef context,
							PGPEvent *event, 
							PGPUserValue userValue);

DWORD WINAPI SearchThreadRoutine( LPVOID lpvoid);

void GrabUserIdStrings(HWND hwnd, 
					   char** string,
					   int* length);

BOOL 
CALLBACK 
SearchProc(	HWND hwnd, 
			UINT msg, 
			WPARAM wParam, 
			LPARAM lParam )
{ 
	static int offsetButtons	= 0;
	static int offsetBoxes		= 0;
	static int originalWidth	= 0;
	int oldWidth				= (int)GetProp(hwnd, "oldWidth");
	int oldHeight				= (int)GetProp(hwnd, "oldHeight");

	switch(msg)
	{
		case WM_INITDIALOG:
		{
			RECT rectWindow, rectButton, rectBox;
			HWND hwndBox = GetDlgItem(hwnd, IDC_SPECIFIER_COMBO);
			HWND hwndButton = GetDlgItem(hwnd, IDC_SEARCH);
			HWND hwndPending = GetDlgItem(hwnd, IDC_PENDING);
			HWND hwndTime = NULL;
			RECT rect;
			PSEARCHTHREADSTRUCT psts;
			CRITICAL_SECTION* keyserverLock;

			GetClientRect(hwndBox, &rect);
			MapWindowPoints(hwndBox, hwnd, (LPPOINT)&rect, 2);

			hwndTime = CreateWindowEx (0, DATETIMEPICK_CLASS,
							"DateTime",
							WS_BORDER|WS_CHILD|WS_VISIBLE|WS_TABSTOP,
							rect.left, rect.top-1, 
							rect.right-rect.left, rect.bottom-rect.top+2, 
							hwnd, NULL, g_hInst, NULL);

			SetWindowPos (hwndTime, GetDlgItem (hwnd, IDC_VERB),
							0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
			SendMessage (hwndTime, DTM_SETMCCOLOR, 
						MCSC_MONTHBK, (LPARAM)GetSysColor (COLOR_3DFACE));
			ShowWindow(hwndTime, SW_HIDE);
			SetProp( hwnd, "hwndTime", hwndTime); 

	
			GetWindowRect(hwnd, &rectWindow);
			GetWindowRect(hwndButton, &rectButton);
			GetWindowRect(hwndBox, &rectBox);

			offsetButtons = rectWindow.right - rectButton.right;
			offsetBoxes = rectButton.left - rectBox.right;
			oldWidth = rectWindow.right - rectWindow.left;
			SetProp( hwnd, "oldWidth",(HANDLE) oldWidth); 
			oldHeight = rectWindow.bottom - rectWindow.top;
			SetProp( hwnd, "oldHeight",(HANDLE) oldHeight);
			originalWidth  = oldWidth; 

			SetProp( hwnd, "ChoiceList",(HANDLE) 0);

			keyserverLock = 
				(CRITICAL_SECTION*)malloc(sizeof(CRITICAL_SECTION));
			
			if(keyserverLock)
			{
				InitializeCriticalSection(keyserverLock);

				SetProp(hwnd,"KeyserverLock", (HANDLE) keyserverLock ); 
			}

			EnterCriticalSection(keyserverLock);

			InitializeControlValues(hwnd, 0);

			LeaveCriticalSection(keyserverLock);

			psts = (PSEARCHTHREADSTRUCT) malloc( sizeof(SEARCHTHREADSTRUCT));

			if(psts)
			{
				memset(psts, 0x00, sizeof(SEARCHTHREADSTRUCT));
				SetProp(hwnd,"SearchInfo", (HANDLE) psts ); 
			}

			// subclass datetime control
			SetProp(hwndTime,"hwnd", hwnd ); 
			SetProp (hwndTime, "oldproc", 
						(HANDLE)GetWindowLong( hwndTime, GWL_WNDPROC ) ); 
			SetWindowLong(	hwndTime, 
							GWL_WNDPROC, 
							(DWORD)SearchSubclassWndProc );

			// subclass combo box
			hwndBox = GetDlgItem(hwnd, IDC_SPECIFIER_COMBO);
			SetProp(hwndBox,"hwnd", hwnd ); 
			SetProp (hwndBox, "oldproc", 
						(HANDLE)GetWindowLong( hwndBox, GWL_WNDPROC ) ); 
			SetWindowLong(	hwndBox, 
							GWL_WNDPROC, 
							(DWORD)SearchSubclassWndProc );

			// subclass combo box
			hwndBox = GetDlgItem(hwnd, IDC_SPECIFIER_COMBO_LIST);
			SetProp(hwndBox,"hwnd", hwnd ); 
			SetProp (hwndBox, "oldproc", 
						(HANDLE)GetWindowLong( hwndBox, GWL_WNDPROC ) ); 
			SetWindowLong(	hwndBox, 
							GWL_WNDPROC, 
							(DWORD)SearchSubclassWndProc );

			// subclass edit box
			hwndBox = GetDlgItem(hwnd, IDC_SPECIFIER_EDIT);
			SetProp(hwndBox,"hwnd", hwnd ); 
			SetProp (hwndBox, "oldproc", 
						(HANDLE)GetWindowLong( hwndBox, GWL_WNDPROC ) ); 
			SetWindowLong(	hwndBox, 
							GWL_WNDPROC, 
							(DWORD)SearchSubclassWndProc );

			SetFocus(hwndBox);


			#if PGP_BUSINESS_SECURITY
			#if PGP_ADMIN_BUILD
				ShowWindow(hwndPending, SW_SHOW);
			#endif	// PGP_ADMIN_BUILD
			#endif	// PGP_BUSINESS_SECURITY

			return FALSE;
		}

		case WM_DESTROY:
		{
			HINSTANCE			hinst			= GetModuleHandle(NULL);
			PCHOICE				choice			= NULL;
			HWND				hwndChoice		= NULL;
			HWND				hwndLocation	= NULL;
			LPARAM				itemData		= 0;
			int					searchPathIndex	= 0;
			int					locationSelection = 0;
			char				szString[256]	= {0x00};
			PSEARCHTHREADSTRUCT	psts			= NULL;
			CRITICAL_SECTION*	keyserverLock	= NULL;
			PSEARCHPATH			path			= NULL;
			
			psts = (PSEARCHTHREADSTRUCT) GetProp(hwnd, "SearchInfo");

			if(psts)
			{
				if(psts->threadHandle)
					TerminateThread(psts->threadHandle, 0); 
				free(psts);
			}

			keyserverLock = (CRITICAL_SECTION*) GetProp(hwnd, 
														"KeyserverLock");

			if(keyserverLock)
			{
				EnterCriticalSection(keyserverLock);
				DeleteCriticalSection( keyserverLock ); 
				free(keyserverLock);
			}

			// remove all the choice windows
			do
			{
				choice = RemoveLastChoice(hwnd, &hwndChoice);
				DestroyWindow( hwndChoice );

			}while( choice );

			// free the prefs struct I am using
			
			hwndLocation = GetDlgItem(hwnd, IDC_LOCATION);
			locationSelection = ComboBox_GetCurSel(hwndLocation);

			path = (PSEARCHPATH) GetProp(hwnd, "SearchPath");

			if(path)
			{
				PGPDisposePrefData(path->prefRef, path->keyserverList);

				PGPSetPrefNumber (	path->prefRef,
							kPGPPrefLastSearchWindowSelection, 
							(PGPUInt32)locationSelection);

				PGPclCloseClientPrefs (	path->prefRef, TRUE);
				free(path);
			}

			CloseSearch ();
			return TRUE;
		}

#define DividerItem 1

		case WM_MEASUREITEM:     
		{
		    LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT) lParam; 
		     		    
            if (lpmis->itemID != -1) 
            {
                lpmis->itemHeight += 2;
 			}
 			
  			return TRUE; 
		}
		
		//  Draw the listbox
	  	case WM_DRAWITEM:                                    
	  	{
			LPDRAWITEMSTRUCT lpDs	= (LPDRAWITEMSTRUCT)lParam;
			int iTextVOff			=	2;
			RECT Rect, EtchRect;
			HBRUSH hBrush;
			char szText[ 256 ];
			
			//  If nothing to draw, exit now
			if( lpDs->itemID == CB_ERR )     
			{     
				return( FALSE );
			}
			
			Rect.left = lpDs->rcItem.left;
			Rect.top = lpDs->rcItem.top;
			Rect.right = lpDs->rcItem.right;
			Rect.bottom = lpDs->rcItem.bottom;
			
			//  Create a brush using the item's background color
			hBrush = CreateSolidBrush( GetSysColor(COLOR_WINDOW ));
			
			//  Fill entire item area
			FillRect( lpDs->hDC, &Rect, hBrush );   
			
			//  Delete the brush when done
			DeleteObject( hBrush );       
			
			//  Highlight if selected
			if( lpDs->itemState & ODS_FOCUS )
			{
				if (!(lpDs->itemState & ODS_COMBOBOXEDIT)) {
					if (lpDs->itemID == DividerItem) {
						iTextVOff = 1;
	               		Rect.bottom -= 2;
					}
					else if (lpDs->itemID == DividerItem+1)
						Rect.top += 1;
				}

				//  Create a brush using the item's background color
				hBrush = CreateSolidBrush( GetSysColor(COLOR_HIGHLIGHT ));
			
				//  Fill entire item area
				FillRect( lpDs->hDC, &Rect, hBrush );   
			
				//  Delete the brush when done
				DeleteObject( hBrush );  
			}       
			
			//  Get the item string
			SendMessage(lpDs->hwndItem, 
						CB_GETLBTEXT , 
						lpDs->itemID,
						(LPARAM)szText );
						
			
			SetBkMode( lpDs->hDC, TRANSPARENT );
			
			SetTextColor( 	lpDs->hDC, 
							GetSysColor(( (lpDs->itemState & ODS_FOCUS) ?
									COLOR_HIGHLIGHTTEXT : COLOR_BTNTEXT) ));

			//  and display the text
			TextOut(lpDs->hDC, 
					4, 
					lpDs->rcItem.top + iTextVOff, 
					szText,
			      	strlen( szText ) );  
			 
 			
 			EtchRect = lpDs->rcItem;
 			
 			if (!(lpDs->itemState & ODS_COMBOBOXEDIT) &&
				 (lpDs->itemID == DividerItem)) 
			{
       			EtchRect.top = EtchRect.bottom-1;
        		
            	DrawEdge( 	lpDs->hDC,
 							&EtchRect, 
 							EDGE_ETCHED, 
 							BF_TOP ); 
			}
			      	                      	
			return TRUE;

		} //  End of WM_DRAWITEM

		case WM_GETDLGCODE:
		{
			return DLGC_WANTMESSAGE;	
		}

		case WM_CONTEXTMENU: 
		{
			WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU, 
				(DWORD) (LPVOID) aSearchIds); 
			break; 
		}

		case SEARCH_SET_LOCAL_KEYSET:
		{
			HINSTANCE hinst		= GetModuleHandle(NULL);
			HWND hwndBox		= NULL;
			char szString[256]	= {0x00};
			int i				= 0;

			hwndBox = GetDlgItem(hwnd, IDC_LOCATION);

			LoadString(hinst, IDS_LOCAL_KEYRING, szString, sizeof(szString));
			i = ComboBox_FindString(hwndBox, -1, szString);
			ComboBox_SetItemData(hwndBox, i, lParam);

			break;
		}

		case SEARCH_SET_CURRENT_SEARCH:
		{
			HINSTANCE hinst		= GetModuleHandle(NULL);
			HWND hwndBox		= NULL;
			char szString[256]	= {0x00};
			int i				= 0;

			hwndBox = GetDlgItem(hwnd, IDC_LOCATION);

			LoadString(hinst, IDS_CURRENT_SEARCH, szString, sizeof(szString));
			i = ComboBox_FindString(hwndBox, -1, szString);
			ComboBox_SetItemData(hwndBox, i, lParam);

			break;
		}

		case SEARCH_SET_FOCUS:
		{
			SetFocus (GetDlgItem (hwnd, IDC_SEARCH));
			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR pnmh = (LPNMHDR) lParam;
			
			switch(pnmh->code)
			{
				case REFRESH_KEYSERVER_LIST:
				{
					CRITICAL_SECTION* keyserverLock = 
						(CRITICAL_SECTION*) GetProp(hwnd, "KeyserverLock");

					EnterCriticalSection(keyserverLock);
					
					ReloadKeyserverList(hwnd);

					LeaveCriticalSection(keyserverLock);

					break;
				}
			}

			break;
		}

		case WM_SIZE:
		{
			BOOL fwSizeType = wParam;      // resizing flag 
			WORD newWidth = LOWORD(lParam);  // width of client area 
			WORD newHeight = HIWORD(lParam); // height of client area 
			int	 delta =  newWidth - oldWidth;
			RECT rectControl;
			HWND hwndControl;


			if( newWidth < originalWidth )
			{
				delta = originalWidth - oldWidth;
				newWidth = originalWidth;
				
			}

			if( newWidth >= originalWidth )
			{
				PCHOICE choice = NULL;

				oldWidth = newWidth;
				SetProp( hwnd, "oldWidth",(HANDLE) oldWidth);

				// move Search Button
				hwndControl = GetDlgItem(hwnd, IDC_SEARCH);
				GetWindowRect(hwndControl, &rectControl);
				MapWindowPoints(NULL, hwnd, (LPPOINT)&rectControl, 2);
				MoveWindow(	hwndControl, 
							rectControl.left + delta,
							rectControl.top,
							rectControl.right - rectControl.left,
							rectControl.bottom - rectControl.top,
							TRUE);

				// move Stop Button
				hwndControl = GetDlgItem(hwnd, IDC_STOP);
				GetWindowRect(hwndControl, &rectControl);
				MapWindowPoints(NULL, hwnd, (LPPOINT)&rectControl, 2);
				MoveWindow(	hwndControl, 
							rectControl.left + delta,
							rectControl.top,
							rectControl.right - rectControl.left,
							rectControl.bottom - rectControl.top,
							TRUE);

				// move Clear Button
				hwndControl = GetDlgItem(hwnd, IDC_CLEAR);
				GetWindowRect(hwndControl, &rectControl);
				MapWindowPoints(NULL, hwnd, (LPPOINT)&rectControl, 2);
				MoveWindow(	hwndControl, 
							rectControl.left + delta,
							rectControl.top,
							rectControl.right - rectControl.left,
							rectControl.bottom - rectControl.top,
							TRUE);


				// move Help Button
				hwndControl = GetDlgItem(hwnd, IDHELP);
				GetWindowRect(hwndControl, &rectControl);
				MapWindowPoints(NULL, hwnd, (LPPOINT)&rectControl, 2);
				MoveWindow(	hwndControl, 
							rectControl.left + delta,
							rectControl.top,
							rectControl.right - rectControl.left,
							rectControl.bottom - rectControl.top,
							TRUE);

				// move the Boxes
				hwndControl = GetDlgItem(hwnd, IDC_SPECIFIER_COMBO);
				GetWindowRect(hwndControl, &rectControl);
				MapWindowPoints(NULL, hwnd, (LPPOINT)&rectControl, 2);
				MoveWindow(	hwndControl, 
							rectControl.left,
							rectControl.top,
							rectControl.right - rectControl.left + delta,
							rectControl.bottom - rectControl.top,
							TRUE);

				hwndControl = GetDlgItem(hwnd, IDC_SPECIFIER_EDIT);
				GetWindowRect(hwndControl, &rectControl);
				MapWindowPoints(NULL, hwnd, (LPPOINT)&rectControl, 2);
				MoveWindow(	hwndControl, 
							rectControl.left,
							rectControl.top,
							rectControl.right - rectControl.left  + delta,
							rectControl.bottom - rectControl.top,
							TRUE);

				hwndControl = GetDlgItem(hwnd, IDC_SPECIFIER_COMBO_LIST);
				GetWindowRect(hwndControl, &rectControl);
				MapWindowPoints(NULL, hwnd, (LPPOINT)&rectControl, 2);
				MoveWindow(	hwndControl, 
							rectControl.left,
							rectControl.top,
							rectControl.right - rectControl.left + delta,
							rectControl.bottom - rectControl.top,
							TRUE);

				// move all the choice windows
				choice = FirstChoice(hwnd);

				while( choice )
				{
					hwndControl =  choice->hwnd;
					GetWindowRect(hwndControl, &rectControl);
					MapWindowPoints(NULL, hwnd, (LPPOINT)&rectControl, 2);
					MoveWindow(	hwndControl, 
								rectControl.left,
								rectControl.top ,
								rectControl.right - rectControl.left + delta,
								rectControl.bottom - rectControl.top,
								TRUE);

					choice = NextChoice(hwnd, choice);
				}

				// move the more and less buttons and the pending checkbox
				if(oldHeight != newHeight)
				{
					delta = newHeight - oldHeight;

					oldHeight = newHeight;
					SetProp( hwnd, "oldHeight",(HANDLE) oldHeight);

					hwndControl = GetDlgItem(hwnd, IDC_MORE);
					GetWindowRect(hwndControl, &rectControl);
					MapWindowPoints(NULL, hwnd, (LPPOINT)&rectControl, 2);
					MoveWindow(	hwndControl, 
								rectControl.left,
								rectControl.top + delta,
								rectControl.right - rectControl.left,
								rectControl.bottom - rectControl.top,
								TRUE);

					hwndControl = GetDlgItem(hwnd, IDC_FEWER);
					GetWindowRect(hwndControl, &rectControl);
					MapWindowPoints(NULL, hwnd, (LPPOINT)&rectControl, 2);
					MoveWindow(	hwndControl, 
								rectControl.left,
								rectControl.top + delta,
								rectControl.right - rectControl.left,
								rectControl.bottom - rectControl.top,
								TRUE);

					hwndControl = GetDlgItem(hwnd, IDC_PENDING);
					GetWindowRect(hwndControl, &rectControl);
					MapWindowPoints(NULL, hwnd, (LPPOINT)&rectControl, 2);
					MoveWindow(	hwndControl, 
								rectControl.left,
								rectControl.top + delta,
								rectControl.right - rectControl.left,
								rectControl.bottom - rectControl.top,
								TRUE);

				}
			}
			
			return TRUE;
		}

		case WM_PAINT:
		{
			RECT rect, rectParent;

			GetClientRect(hwnd, &rect);
			GetClientRect(GetParent(hwnd), &rectParent);

			if(rect.right != rectParent.right)
			{
				MapWindowPoints(hwnd, GetParent(hwnd), (LPPOINT)&rect, 2);

				rect.left	= rectParent.left;
				rect.top	= rectParent.top;
				rect.right	= rectParent.right;

				MoveWindow(	hwnd, 
							rect.left, 
							rect.top, 
							rect.right - rect.left,
							rect.bottom - rect.top,  
							TRUE);
			}
			break;
		}

		case WM_COMMAND:
		{
			WORD wNotifyCode = HIWORD(wParam); // notification code 
			WORD wID = LOWORD(wParam);         // item, control id
			HWND hwndCtl = (HWND) lParam;      // handle of control 
 
			switch( wID )
			{
				case IDC_MORE:
				{
					PCHOICE choice;
					RECT rectChoice;
					RECT rect;
					int height;

					if(wNotifyCode)
					{
						break;
					}

					choice = NewChoice(hwnd);

					if(choice)
					{
						SIZEREQUEST request;
						HWND hwndCtrl = NULL;


						GetClientRect(hwnd, &rect);
						GetClientRect(choice->hwnd, &rectChoice);

						height = rectChoice.bottom - rectChoice.top + 1;

						memset(&request, 0x00, sizeof(request));

						request.nmhdr.hwndFrom = hwnd;
						request.nmhdr.idFrom = IDD_SEARCH;
						request.nmhdr.code = SEARCH_REQUEST_SIZING;
						request.delta = height;

						if(SendMessage(	GetParent(hwnd), 
										WM_NOTIFY, 
										(WPARAM)hwnd, 
										(LPARAM)&request))
						{
							NMHDR nmhdr;

							height += rect.bottom - rect.top;

							MapWindowPoints(hwnd,
											GetParent(hwnd),
											(LPPOINT)&rect,
											2);

							rect.bottom	= rect.top + height;

							MoveWindow(	hwnd, 
										rect.left, 
										rect.top, 
										rect.right - rect.left,
										rect.bottom - rect.top, 
										TRUE);

							ShowWindow(choice->hwnd, SW_SHOW);

							EnableWindow(GetDlgItem(hwnd, IDC_FEWER), TRUE);
							
							memset(&nmhdr, 0x00, sizeof(nmhdr));

							nmhdr.hwndFrom = hwnd;
							nmhdr.idFrom = IDD_SEARCH;
							nmhdr.code = SEARCH_SIZING;

							SendMessage(GetParent(hwnd), 
										WM_NOTIFY, 
										(WPARAM)hwnd, 
										(LPARAM)&nmhdr);

							hwndCtrl = GetDlgItem(hwnd, IDC_MORE);
							InvalidateRect(hwndCtrl, NULL, FALSE);
							hwndCtrl = GetDlgItem(hwnd, IDC_FEWER);
							InvalidateRect(hwndCtrl, NULL, FALSE);

						}
						else
						{
							HWND hwndChoice;

							RemoveLastChoice(hwnd, &hwndChoice);
							DestroyWindow(hwndChoice);
						}
					}

					break;
				}

				case IDC_FEWER:
				{
					PCHOICE choice;
					HWND hwndChoice;
					RECT rectChoice;
					RECT rect;
					int height;
					NMHDR nmhdr;

					if(wNotifyCode)
					{
						break;
					}

					choice = RemoveLastChoice(hwnd, &hwndChoice);

					GetClientRect(hwnd, &rect);
					GetClientRect(hwndChoice, &rectChoice);

					height = rect.bottom - rect.top;
					height -= rectChoice.bottom - rectChoice.top + 1;

					MapWindowPoints(hwnd, GetParent(hwnd), (LPPOINT)&rect, 2);

					rect.bottom	= rect.top + height;

					MoveWindow(	hwnd, 
								rect.left, 
								rect.top, 
								rect.right - rect.left,
								rect.bottom - rect.top, 
								TRUE);

					if(!choice)
					{
						EnableWindow(GetDlgItem(hwnd, IDC_FEWER), FALSE);
					}

					DestroyWindow(hwndChoice);

					memset(&nmhdr, 0x00, sizeof(nmhdr));

					nmhdr.hwndFrom = hwnd;
					nmhdr.idFrom = IDD_SEARCH;
					nmhdr.code = SEARCH_SIZING;

					SendMessage(GetParent(hwnd), 
								WM_NOTIFY, 
								(WPARAM)hwnd, 
								(LPARAM)&nmhdr);

					break;
				}

				case IDC_CLEAR:
				{
					HWND	hwndItem	= NULL;
					int		count		= ChoiceCount(hwnd);
					
					if(wNotifyCode)
					{
						break;
					}

					// delete extra choices
					for(count; count > 0; count--)
					{
						SendMessage(hwnd, 
									WM_COMMAND, 
									MAKEWPARAM(IDC_FEWER, 0), 
									0);
					}
					
					// reset default choice
					hwndItem = GetDlgItem(hwnd, IDC_ATTRIBUTE);
					ComboBox_SetCurSel(hwndItem, 0);

					ChangeAttributeSelection(hwndItem);

					hwndItem = GetDlgItem(hwnd, IDC_SPECIFIER_EDIT);
					Edit_SetText(hwndItem, "");

					break;
				}

				case IDC_SEARCH:
				{
					PGPFilterRef	filter = NULL;
					PGPFilterRef	SearchFilter = NULL;
					PCHOICE			choice = NULL;
					PGPError		error;
					int				action = 0;
					HWND			hwndStop = NULL;
					
					if(wNotifyCode)
					{
						break;
					}

					hwndStop = GetDlgItem(hwnd, IDC_STOP);

					ShowWindow(hwndStop, SW_SHOW);
					EnableWindow(hwndCtl, FALSE);
					ShowWindow(hwndCtl, SW_HIDE);

					// Take Care of the Default Search Item in ourself
					error = CreateFilter(hwnd, &filter, &action);

					SearchFilter = filter;

					if( IsntPGPError (error ) )
					{
						// now iterate the children if there are any
						choice = FirstChoice(hwnd);

						while( choice && IsntPGPError (error ) )
						{							
							error = CreateFilter(	choice->hwnd, 
													&filter, 
													&action);

							if( IsntPGPError (error ) )
							{
								PGPFilterRef CombinedFilter = NULL;

								// | the filters
								if( action == ACTION_UNION )  
								{
									error = PGPUnionFilters(
													filter,
													SearchFilter, 
													&CombinedFilter);
								}
								// & the filters
								else if( action == ACTION_INTERSECT ) 
								{
									error = PGPIntersectFilters(
														filter,
														SearchFilter, 
														&CombinedFilter);
									
								}

								SearchFilter = CombinedFilter; 
							}

							choice = NextChoice(hwnd, choice);
						}
					}
					else 
					{
						PGPclErrorBox (NULL, error);
					}
					
					EnableWindow (hwndStop, TRUE);

					if( IsntPGPError(error) )
					{
						PSEARCHTHREADSTRUCT psts = NULL;

						psts = (PSEARCHTHREADSTRUCT)
									GetProp( hwnd, "SearchInfo");

						if( psts )
						{
							if (!psts->threadHandle)
							{
								memset(psts, 0x00, 
											sizeof(SEARCHTHREADSTRUCT));

								psts->hwnd = hwnd;
								psts->filter = SearchFilter;
								psts->bCancel = FALSE;
								psts->bAlreadyAsked = FALSE;

								SetProp( hwnd, "SearchInfo",(HANDLE) psts);

								psts->threadHandle = CreateThread(	NULL, 
													0, 
													SearchThreadRoutine, 
													(void*)psts, 
													0, 
													&psts->threadID);
							}
						}
					}

					if( IsPGPError(error) )
					{
						SEARCHABORT abort;

						if (filter) PGPFreeFilter (filter);

						abort.nmhdr.hwndFrom = hwnd;
						abort.nmhdr.idFrom = IDD_SEARCH;
						abort.nmhdr.code = SEARCH_ABORT;
						abort.error = kPGPError_UserAbort;

						SendMessage(GetParent(hwnd), 
								WM_NOTIFY, 
								(WPARAM)hwnd, 
								(LPARAM)&abort);

						ShowWindow(hwndCtl, SW_SHOW);
						ShowWindow(hwndStop, SW_HIDE);
					}

					break;
				}

				case IDC_STOP:
				{
					HWND hwndSearch, hwndStop;
					PSEARCHTHREADSTRUCT psts;

					if(wNotifyCode)
					{
						break;
					}

					hwndSearch = GetDlgItem(hwnd, IDC_SEARCH);
					hwndStop = GetDlgItem(hwnd, IDC_STOP);

					if(!IsWindowEnabled (hwndStop))
					{
						break;
					}

					psts = (PSEARCHTHREADSTRUCT) GetProp(hwnd, "SearchInfo");

					EnableWindow(hwndStop, FALSE);

					if(psts)
					{
						PGPEvent event;
						PGPEventErrorData data;
						PGPError err;

						if (PGPKeyServerRefIsValid (psts->server))
						{
							err = PGPCancelKeyServerCall (psts->server);
							PGPclErrorBox (hwnd, err);
						}

						data.error = kPGPError_UserAbort;

						memset(&event, 0x00, sizeof(PGPEvent));

						event.type = kPGPEvent_ErrorEvent;
						event.data.errorData = data;

						SearchEventHandler(	NULL,
											&event, 
											psts);
					}

					if(psts)
					{
						SEARCHABORT abort;

						memset(&abort, 0x00, sizeof(abort));

						abort.nmhdr.hwndFrom = psts->hwnd;
						abort.nmhdr.idFrom = IDD_SEARCH;
						abort.nmhdr.code = SEARCH_ABORT;
						abort.error = kPGPError_UserAbort;

						SendMessage(GetParent(psts->hwnd), 
								WM_NOTIFY, 
								(WPARAM)psts->hwnd, 
								(LPARAM)&abort);
					}

					break;
				}

				case IDHELP:
				{
					if(wNotifyCode)
					{
						break;
					}

					WinHelp (hwnd, g_szHelpFile, HELP_CONTEXT,               
                        IDH_PGPPK_SEARCH_DIALOG);

					break;
				}

				
				case IDC_ATTRIBUTE:
				{
					HANDLE_IDC_ATTRIBUTE(hwnd, msg, wParam, lParam);
					break;
				}

				case IDC_LOCATION:
				{
					INT i;
					if (wNotifyCode == CBN_DROPDOWN)
					{
						RECT rc;
						GetWindowRect (hwndCtl, &rc);
						i = rc.top;
					}
					if (wNotifyCode == CBN_SELCHANGE)
					{
						RECT rc;
						GetWindowRect (hwndCtl, &rc);
						i = rc.top;
					}
					break;
				}
			}

			return TRUE;
		}

	}

	return FALSE;
}


PGPError PerformSearch(HWND hwndLocation, 
					   PSEARCHTHREADSTRUCT psts,
					   BOOL	bPending,
					   long* flags,
					   PGPKeySetRef* pKeySetOut)
{
	HINSTANCE		hinst				= GetModuleHandle(NULL);
	PGPError		error				= kPGPError_NoErr;
	PGPKeySetRef	localKeySet			= kPGPInvalidRef; 
	int				locationSelection	= 0;
	int				lastSearchIndex		= 0;
	int				localSearchIndex	= 0;
	int				searchPathIndex		= 0;
	char*			locationBuffer		= NULL;
	int				locationLength		= 0;
	LPARAM			lParam				= 0;
	int				lastSearchType		= 0;
	char			szString[256]		= {0x00};
	PGPKeyServerAccessType	accessType	= kPGPKeyServerAccessType_Normal;
	CRITICAL_SECTION*	keyserverLock	= 
						(CRITICAL_SECTION*) GetProp(GetParent(hwndLocation), 
													"KeyserverLock");
	PGPKeyServerThreadStorageRef	previousStorage;


	#if PGP_BUSINESS_SECURITY
	#if PGP_ADMIN_BUILD
		accessType	= kPGPKeyServerAccessType_Administrator;
	#endif	// PGP_ADMIN_BUILD
	#endif	// PGP_BUSINESS_SECURITY
	
	PGPKeyServerInit();
	PGPKeyServerCreateThreadStorage(&previousStorage);

	EnterCriticalSection(keyserverLock);

	lastSearchType = (int)GetProp(hwndLocation, "LastSearchType");

	LoadString(hinst, IDS_DEFAULT_PATH, szString, sizeof(szString));
	searchPathIndex = ComboBox_FindString(hwndLocation, -1, szString);

	LoadString(hinst, IDS_LOCAL_KEYRING, szString, sizeof(szString));
	localSearchIndex = ComboBox_FindString(hwndLocation, -1, szString);

	LoadString(hinst, IDS_CURRENT_SEARCH, szString, sizeof(szString));
	lastSearchIndex = ComboBox_FindString(hwndLocation, -1, szString);

	locationSelection = ComboBox_GetCurSel(hwndLocation);

	lParam = ComboBox_GetItemData(hwndLocation, locationSelection);

	psts->keySet = 
		(PGPKeySetRef) ComboBox_GetItemData(hwndLocation, localSearchIndex);

	LeaveCriticalSection(keyserverLock);

	if (lParam == 0L ) // NULL: Invalid Selection
	{
		*pKeySetOut = kPGPInvalidRef;
	}

	// Default Path 
	else if( locationSelection == searchPathIndex )
	{
		PGPPrefRef prefs;
		PGPtlsSessionRef tls = kInvalidPGPtlsSessionRef;
		PGPKeyServerEntry* keyserverList = NULL;
		PGPUInt32 keyserverCount = 0;
		char* searchString = NULL;
		int searchStringLength = 0;
		char emailDomain[512] = {0x00};

		*flags = FLAG_SEARCH_MEMORY;
		SetProp( hwndLocation, "LastSearchType",(HANDLE) *flags);

		// retrieve search string from control
		GrabUserIdStrings(	GetParent(hwndLocation),
							&searchString,
							&searchStringLength);

		if(searchString)
		{
			PGPFindEmailDomain(searchString, emailDomain);
		}

		error = PGPclOpenClientPrefs(
					PGPGetContextMemoryMgr (g_Context),
					&prefs);

		if( IsntPGPError( error ) )
		{
			DWORD index = 0;

			error = PGPCreateKeyServerPath(	prefs,
											emailDomain,
											&keyserverList,
											&keyserverCount);
			if( IsntPGPError( error ) )
			{
				for(index = 0; index < keyserverCount; index++)
				{
					SEARCHPROGRESS progress;
					char templateString[256];

					memset(&progress, 0x00, sizeof(progress));

					progress.nmhdr.hwndFrom = GetParent(hwndLocation);
					progress.nmhdr.idFrom = IDD_SEARCH;
					progress.nmhdr.code = SEARCH_PROGRESS;
					progress.step = SEARCH_PROGRESS_INFINITE;
					progress.total = SEARCH_PROGRESS_INFINITE;

					LoadString(	g_hInst,
								IDS_SEARCH_MESSAGE_SERVER_NAME,
								templateString,
								sizeof(templateString));

					wsprintf(	progress.message,
								templateString, 
								keyserverList[index].serverDNS);

					SendMessage(GetParent(GetParent(hwndLocation)), 
								WM_NOTIFY, 
								(WPARAM)GetParent(hwndLocation), 
								(LPARAM)&progress);

					error = PGPNewKeyServerFromHostName( 
								g_Context,
								keyserverList[index].serverDNS,
								keyserverList[index].serverPort,
								keyserverList[index].protocol,
								accessType, 
								(bPending ? kPGPKeyServerKeySpace_Pending : 
									kPGPKeyServerKeySpace_Default), 
								&psts->server );

				
					if( IsntPGPError( error ) )
					{
						memcpy(&(psts->keyserverEntry), 
							&(keyserverList[index]), 
							sizeof(PGPKeyServerEntry));

						error = PGPSetKeyServerEventHandler( psts->server, 
										SearchEventHandler, psts );

						if ( IsntPGPError( error ) ) 
						{
							
							if(	(kPGPKeyServerType_LDAPS == 
										keyserverList[index].protocol) ||
								(kPGPKeyServerType_HTTPS == 
										keyserverList[index].protocol))
							{
								error = PGPNewTLSSession(	g_TLSContext,
															&tls );
								if( IsPGPError(error) )
								{
									// warn beyond the icon?
									tls = kInvalidPGPtlsSessionRef;
								}
							}

							error = PGPKeyServerOpen( psts->server, tls );

							if ( IsntPGPError(error) ) 
							{
								error = PGPQueryKeyServer(	psts->server, 
													psts->filter, 
													pKeySetOut);
								
								PGPKeyServerClose ( psts->server );
							}
						}

						PGPFreeKeyServer( psts->server );
						psts->server = kInvalidPGPKeyServerRef;

						if( PGPtlsSessionRefIsValid(tls) )
						{
							PGPFreeTLSSession( tls );
							tls = kInvalidPGPtlsSessionRef;
						}

						if( IsntPGPError( error ) )
						{
							EnterCriticalSection(keyserverLock);

							ComboBox_SetItemData(	hwndLocation, 
													lastSearchIndex, 
													*pKeySetOut);
							LeaveCriticalSection(keyserverLock);

							break;
						}
					}
				}

				PGPDisposeKeyServerPath(keyserverList);
			}

			PGPclCloseClientPrefs (prefs, FALSE);
		}

		if(searchString)
		{  
			free(searchString);
		}
	}

	// peform search on a local KeySet (keyring or mem)
	else if(locationSelection == localSearchIndex ||
			locationSelection == lastSearchIndex)
	{
		localKeySet = (PGPKeySetRef) lParam;

		if(locationSelection == lastSearchIndex)
		{
			*flags = lastSearchType;
		}
		else
		{
			*flags = FLAG_SEARCH_LOCAL_KEYSET;
			SetProp( hwndLocation, "LastSearchType",(HANDLE) *flags);
		}

		if(localKeySet != kPGPInvalidRef)
		{
			error = PGPFilterKeySet( localKeySet, psts->filter, pKeySetOut);

			if( IsPGPError(error) )
			{
				*pKeySetOut = kPGPInvalidRef;
			}
		}
		else 
		{
			*pKeySetOut = kPGPInvalidRef;
		}
		
		EnterCriticalSection(keyserverLock);

		ComboBox_SetItemData(hwndLocation, lastSearchIndex, *pKeySetOut);

		LeaveCriticalSection(keyserverLock);
	}

	// perform search on specified keyserver
	else
	{
		PGPtlsSessionRef tls = kInvalidPGPtlsSessionRef;
		PGPKeyServerEntry* entry = NULL;
		
		*flags = FLAG_SEARCH_MEMORY;
		SetProp( hwndLocation, "LastSearchType",(HANDLE) *flags);

		// retrieve keyserver entry from control
		entry = (PGPKeyServerEntry*)lParam;

		if( entry )
		{
			error = PGPNewKeyServerFromHostName( 
								g_Context,
								entry->serverDNS,
								entry->serverPort,
								entry->protocol,
								accessType, 
								(bPending ? kPGPKeyServerKeySpace_Pending : 
									kPGPKeyServerKeySpace_Default), 
								&psts->server );

			memcpy(&(psts->keyserverEntry), 
						entry, sizeof(PGPKeyServerEntry));
		}
		else
		{
			error = kPGPError_OutOfMemory;
		}

		if( IsntPGPError( error ) )
		{
			error = PGPSetKeyServerEventHandler( psts->server, 
							SearchEventHandler, psts );
			if ( IsntPGPError( error ) )
			{
				if(	kPGPKeyServerType_LDAPS == entry->protocol ||
					kPGPKeyServerType_HTTPS	== entry->protocol)
				{
					error = PGPNewTLSSession(	g_TLSContext,
												&tls );
					if( IsPGPError(error) )
					{
						// warn beyond the icon?
						tls = kInvalidPGPtlsSessionRef;
					}
				}

				error = PGPKeyServerOpen( psts->server, tls );

				if ( IsntPGPError(error) ) 
				{
					error = PGPQueryKeyServer(	psts->server, 
												psts->filter, 
												pKeySetOut);

					PGPKeyServerClose ( psts->server );
				}
			}

			PGPFreeKeyServer( psts->server );
			psts->server = kInvalidPGPKeyServerRef;

			if( PGPtlsSessionRefIsValid(tls) )
			{
				PGPFreeTLSSession( tls );
				tls = kInvalidPGPtlsSessionRef;
			}

			if( (IsntPGPError( error )) || 
				(error == kPGPError_ServerPartialSearchResults ))
			{
				EnterCriticalSection(keyserverLock);

				ComboBox_SetItemData(	hwndLocation, 
										lastSearchIndex, 
										*pKeySetOut);

				LeaveCriticalSection(keyserverLock);
			}

		}
	}
	PGPKeyServerDisposeThreadStorage(previousStorage);

	PGPKeyServerCleanup();

	return error;
}

DWORD WINAPI SearchThreadRoutine (LPVOID lpvoid)
{
	PSEARCHTHREADSTRUCT psts = (PSEARCHTHREADSTRUCT) lpvoid;
	SEARCHRESULT result;
	PGPKeySetRef keySet = kPGPInvalidRef;
	HWND hwnd = psts->hwnd;
	HWND hwndLocation = NULL;
	HWND hwndSearch = NULL;
	HWND hwndStop = NULL;
	HWND hwndPending = NULL;
	long flags = 0;
	BOOL bPending = FALSE;
	PGPError error = kPGPError_NoErr;
	
	hwndLocation = GetDlgItem(hwnd, IDC_LOCATION);
	hwndSearch = GetDlgItem(hwnd, IDC_SEARCH);
	hwndStop = GetDlgItem(hwnd, IDC_STOP);
	hwndPending = GetDlgItem(hwnd, IDC_PENDING);

	bPending = Button_GetCheck(hwndPending);

	error = PerformSearch(	hwndLocation,
							psts,
							bPending,
							&flags,
							&keySet);

	memset(&result, 0x00, sizeof(result));

	if( IsntPGPError(error) ||
		(error == kPGPError_ServerPartialSearchResults))
	{
		result.pData = keySet;
		result.flags = flags;
		result.error = error;

		memcpy(&(result.keyserver), 
			&(psts->keyserverEntry), 
			sizeof(PGPKeyServerEntry));

		if(bPending)
		{
			result.flags |= FLAG_AREA_PENDING;
		}
	}
	else 
	{
		result.pData = NULL;
		result.flags = 0;
		result.error = error;
	}

	result.nmhdr.hwndFrom = hwnd;
	result.nmhdr.idFrom = IDD_SEARCH;
	result.nmhdr.code = SEARCH_DISPLAY_KEYSET;

	SendMessage(GetParent(hwnd), 
				WM_NOTIFY, 
				(WPARAM)hwnd, 
				(LPARAM)&result);

	if ( psts->filter )
		PGPFreeFilter ( psts->filter );

	memset(psts, 0x00, sizeof(SEARCHTHREADSTRUCT));

	EnableWindow(hwndStop, FALSE);
	ShowWindow(hwndStop, SW_HIDE);
	ShowWindow(hwndSearch, SW_SHOW);
	EnableWindow(hwndSearch, TRUE);

	return 0;
}

//	____________________________________
//
//  translate keyserver state to string index

static INT
sTranslateKSMessage (INT iKSmessage) 
{
	switch (iKSmessage) {
	case kPGPKeyServerState_Opening :
								return IDS_SEARCH_MESSAGE_CONNECTING;
	case kPGPKeyServerState_Querying:			
								return IDS_SEARCH_MESSAGE_QUERYING;
	case kPGPKeyServerState_ProcessingResults :
								return IDS_SEARCH_MESSAGE_PROCESSING;
	case kPGPKeyServerState_Uploading :			
								return IDS_SEARCH_MESSAGE_SENDING;
	case kPGPKeyServerState_Deleting :			
								return IDS_SEARCH_MESSAGE_DELETING;
	case kPGPKeyServerState_Disabling :			
								return IDS_SEARCH_MESSAGE_DISABLING;
	case kPGPKeyServerState_Closing	:			
								return IDS_SEARCH_MESSAGE_CLOSING;

	default :
//	case kPGPKeyServerState_TLSUnableToSecureConnection :
//	case kPGPKeyServerState_TLSConnectionSecured :
		return IDS_SEARCH_MESSAGE_BASE;
	}
}


PGPError SearchEventHandler(PGPContextRef context,
							PGPEvent *event, 
							PGPUserValue userValue)
{
	PGPError error = kPGPError_NoErr;
	PSEARCHTHREADSTRUCT psts;

	psts = (PSEARCHTHREADSTRUCT) userValue;

	switch(event->type)
	{
		case kPGPEvent_ErrorEvent:
		{
			PGPEventErrorData data = event->data.errorData;

			if(data.error == kPGPError_UserAbort)
			{
				psts->bCancel = TRUE;
				return kPGPError_NoErr;
			}
			
			break;
		}

		case kPGPEvent_KeyServerEvent:
		{
			PGPEventKeyServerData data = event->data.keyServerData;
			SEARCHPROGRESS progress;
			int index;

			memset(&progress, 0x00, sizeof(progress));

			progress.nmhdr.hwndFrom = psts->hwnd;
			progress.nmhdr.idFrom = IDD_SEARCH;
			progress.nmhdr.code = SEARCH_PROGRESS;
			progress.step = SEARCH_PROGRESS_INFINITE; // data.soFar;//
			progress.total = SEARCH_PROGRESS_INFINITE;// data.total;//

			if(!psts->bCancel)
			{
				index = data.state;
			}
			else
			{
				index = IDS_SEARCH_MESSAGE_CANCEL;
				progress.step = 0;
				progress.total = 0;
			}

			index = sTranslateKSMessage (index);
			LoadString(	g_hInst,
						index,
						progress.message,
						sizeof(progress.message));


			SendMessage(GetParent(psts->hwnd), 
						WM_NOTIFY, 
						(WPARAM)psts->hwnd, 
						(LPARAM)&progress);
			
			break;
		}
		
		case kPGPEvent_KeyServerTLSEvent:
		{
			PGPEventKeyServerTLSData data = event->data.keyServerTLSData;

			SEARCHSECURE secure;
			
			memset(&secure, 0x00, sizeof(secure));

			secure.nmhdr.hwndFrom = psts->hwnd;
			secure.nmhdr.idFrom = IDD_SEARCH;
			secure.nmhdr.code = SEARCH_SECURE_STATUS;
			secure.secure = FALSE;

			if(data.state == kPGPKeyServerState_TLSConnectionSecured)
			{
				
				PGPtlsSessionRef	tls = 
									event->data.keyServerTLSData.tlsSession;

				PGPInt32			remoteKeyAlg;
				PGPKeyID			keyID,
									expectedID;
				PGPError			error;
	
				error = 
					PGPtlsGetRemoteAuthenticatedKey( tls, &secure.keyAuth );
				if( IsPGPError( error ) )
					break;

				PGPGetKeyServerURL(&(psts->keyserverEntry), 
												secure.szServerName);
				
				error = PGPGetKeyIDFromKey( secure.keyAuth, &keyID );
				if( IsPGPError( error ) )
					break;

				error = PGPGetKeyNumber( secure.keyAuth, kPGPKeyPropAlgID,
										&remoteKeyAlg );
				if( IsPGPError( error ) )
					break;

				// check if we already have an auth key from this server
				if( psts->keyserverEntry.authAlg !=
					kPGPPublicKeyAlgorithm_Invalid )
				{
					error = PGPGetKeyIDFromString(
								psts->keyserverEntry.authKeyIDString,
								&expectedID );

					if( IsPGPError( error ) )
						break;

					if( PGPCompareKeyIDs( &expectedID, &keyID ) )
					{
						error = PGPclConfirmRemoteAuthentication (
												g_Context,
												psts->hwnd,
												secure.szServerName,
												secure.keyAuth,
												psts->keySet,
												PGPCL_AUTHUNEXPECTEDKEY);
						// if user approve
						if(IsntPGPError(error))
						{
							NMHDR nmhdr;

							error = PGPGetKeyIDString( &keyID, 
									kPGPKeyIDString_Full,
									psts->keyserverEntry.authKeyIDString );

							if( IsPGPError( error ) )
								break;

							psts->keyserverEntry.authAlg =
								(PGPPublicKeyAlgorithm) remoteKeyAlg;

							PGPclSyncKeyserverPrefs(g_Context,
												&(psts->keyserverEntry));

							secure.secure = TRUE;

							nmhdr.hwndFrom = psts->hwnd;
							nmhdr.idFrom = IDD_SEARCH;
							nmhdr.code = REFRESH_KEYSERVER_LIST;

							SendMessage(psts->hwnd, 
								WM_NOTIFY, 
								(WPARAM)psts->hwnd, 
								(LPARAM)&nmhdr);
						}
					}
					else
					{
						error = PGPclConfirmRemoteAuthentication (
												g_Context,
												psts->hwnd,
												secure.szServerName,
												secure.keyAuth,
												psts->keySet,
												PGPCL_AUTHEXPECTEDKEY);
						// if user approve
						if(IsntPGPError(error))
						{
							secure.secure = TRUE;
						}
					}
				}
				// this is the first time we're talking to this server
				else
				{
					
					error = PGPclConfirmRemoteAuthentication (
											g_Context,
											psts->hwnd,
											secure.szServerName,
											secure.keyAuth,
											psts->keySet,
											PGPCL_AUTHNEWKEY);
					// if user approve
					if(IsntPGPError(error))
					{
						NMHDR nmhdr;

						error = PGPGetKeyIDString( &keyID, 
									kPGPKeyIDString_Full,
									psts->keyserverEntry.authKeyIDString );

						if( IsPGPError( error ) )
							break;

						psts->keyserverEntry.authAlg =
							(PGPPublicKeyAlgorithm) remoteKeyAlg;


						PGPclSyncKeyserverPrefs(g_Context,
												&(psts->keyserverEntry));

						secure.secure = TRUE;

						nmhdr.hwndFrom = psts->hwnd;
						nmhdr.idFrom = IDD_SEARCH;
						nmhdr.code = REFRESH_KEYSERVER_LIST;

						SendMessage(psts->hwnd, 
							WM_NOTIFY, 
							(WPARAM)psts->hwnd, 
							(LPARAM)&nmhdr);
					}
				}

				if (secure.secure != TRUE) 
				{
					SEARCHABORT abort;

					psts->bCancel = TRUE;

					memset(&abort, 0x00, sizeof(abort));

					abort.nmhdr.hwndFrom = psts->hwnd;
					abort.nmhdr.idFrom = IDD_SEARCH;
					abort.nmhdr.code = SEARCH_ABORT;
					abort.error = kPGPError_UserAbort;

					SendMessage(GetParent(psts->hwnd), 
							WM_NOTIFY, 
							(WPARAM)psts->hwnd, 
							(LPARAM)&abort);
				}

			}
			else if(data.state == 
						kPGPKeyServerState_TLSUnableToSecureConnection)
			{
				secure.secure = FALSE;
			}

			SendMessage(GetParent(psts->hwnd), 
						WM_NOTIFY, 
						(WPARAM)psts->hwnd, 
						(LPARAM)&secure);
				
			break;
		}

		case kPGPEvent_FinalEvent:
		{
			psts->bAlreadyAsked = FALSE;
			break;
		}
	}


	if( psts->bCancel )
	{
		error = kPGPError_UserAbort;
	}

	return error;
}

void GrabUserIdStrings(HWND hwnd, 
					   char** string,
					   int* length)
{
	PCHOICE	choice				= NULL;
	HWND	hwndEdit			= NULL;
	HWND	hwndAttribute		= NULL;
	int		AttributeSelection	= 0;
	int		userIdSelection		= 0;

	*string = NULL;
	*length = 0;

	hwndAttribute	= GetDlgItem(hwnd, IDC_ATTRIBUTE);
	hwndEdit		= GetDlgItem(hwnd, IDC_SPECIFIER_EDIT);

	AttributeSelection	= ComboBox_GetCurSel(hwndAttribute);

	if(AttributeSelection == 0) // User Id
	{
		*length = Edit_GetTextLength(hwndEdit) + 1;

		*string = (char*)malloc(*length);

		if( *string )
		{
			Edit_GetText(	hwndEdit, 
							*string, 
							*length);
		}
	}

	// now iterate the children if there are any
	choice = FirstChoice(hwnd);

	while( choice )
	{							
		hwndAttribute	= GetDlgItem(choice->hwnd, IDC_ATTRIBUTE);
		hwndEdit		= GetDlgItem(choice->hwnd, IDC_SPECIFIER_EDIT);

		AttributeSelection	= ComboBox_GetCurSel(hwndAttribute);

		if(AttributeSelection == 0) // User Id
		{
			int tempLength = 0;

			tempLength = *length + Edit_GetTextLength(hwndEdit) + 1;

			if(!(*string))
			{
				*string = (char*)malloc(tempLength);
			}
			else
			{
				*string = (char*)realloc(*string, tempLength);
			}

			if( *string )
			{
				if(*length)
				{
					*(*string + *length - 1) = 0x20; // add space
				}

				Edit_GetText(	hwndEdit, 
								*string + *length, 
								tempLength);

				*length = tempLength;
			}
		}

		choice = NextChoice(hwnd, choice);
	}
}

