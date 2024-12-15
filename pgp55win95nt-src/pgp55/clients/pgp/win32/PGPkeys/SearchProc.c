/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: SearchProc.c,v 1.61 1997/11/04 17:21:53 pbj Exp $
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

// Shared Headers 
#include "PGPcmdlg.h"
#include "PGPphras.h"
#include "PGPpkHlp.h" 

// Project Headers
#include "Search.h"
#include "Choice.h"
#include "SearchFilter.h"
#include "SearchCommon.h"
#include "resource.h"

typedef struct _SEARCHTHREADSTRUCT
{
	HWND			hwnd;
	HANDLE			threadHandle;
	DWORD			threadID;
	PGPFilterRef	filter;
	char			url[kMaxServerNameLength + 1];
	BOOL			bCancel;
	BOOL			bAlreadyAsked;

}SEARCHTHREADSTRUCT,*PSEARCHTHREADSTRUCT;

extern PGPContextRef	g_ContextRef; 
extern HINSTANCE		g_hInst;
extern CHAR				g_szHelpFile[MAX_PATH];


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

			GetClientRect(hwndBox, &rect);
			MapWindowPoints(hwndBox, hwnd, (LPPOINT)&rect, 2);

			if(PGPcomdlgInitTimeDateControl(g_hInst))
			{
				hwndTime = PGPcomdlgCreateTimeDateControl(	
								hwnd,	
								g_hInst,
								rect.left,
								rect.top,
								PGPCOMDLG_DISPLAY_DATE);

				ShowWindow(hwndTime, SW_HIDE);
				SetProp( hwnd, "hwndTime", hwndTime); 
			}

			
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
			InitializeControlValues(hwnd, 0);

			psts = (PSEARCHTHREADSTRUCT) malloc( sizeof(SEARCHTHREADSTRUCT) );

			if(psts)
			{
				memset(psts, 0x00, sizeof(SEARCHTHREADSTRUCT));
				SetProp(hwnd,"SearchInfo", (HANDLE) psts ); 
			}


			hwndBox = GetDlgItem(hwnd, IDC_SPECIFIER_EDIT);

			SetProp(hwndBox, 
					"oldproc",
					(HANDLE)GetWindowLong( hwndBox, GWL_WNDPROC ) ); 

			SetWindowLong(	hwndBox, 
							GWL_WNDPROC, 
							(DWORD)EditSubclassWndProc );

			SetFocus(hwndBox);

			SetProp(hwndBox,"hwnd", hwnd ); 

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
			char				szString[256]	= {0x00};
			PSEARCHTHREADSTRUCT	psts			= NULL;
			
			psts = (PSEARCHTHREADSTRUCT) GetProp(hwnd, "SearchInfo");

			if(psts)
			{
				//PGPEvent event;
				//PGPEventErrorData data;

				TerminateThread(psts->threadHandle, 0); 

				/*data.error = kPGPError_UserAbort;

				memset(&event, 0x00, sizeof(PGPEvent));

				event.type = kPGPEvent_ErrorEvent;
				event.data.errorData = data;

				SearchEventHandler(	NULL,
									&event, 
									psts);

				WaitForSingleObject(psts->threadHandle, 1000); */

				free(psts);
			}

			// remove all the choice windows
			do
			{
				choice = RemoveLastChoice(hwnd, &hwndChoice);
				DestroyWindow( hwndChoice );

			}while( choice );

			// free the prefs struct I am using
			hwndLocation = GetDlgItem(hwnd, IDC_LOCATION);
			LoadString(hinst, IDS_DEFAULT_PATH, szString, sizeof(szString));
			searchPathIndex = ComboBox_FindString(hwndLocation, -1, szString);

			if(CB_ERR != searchPathIndex)
			{
				PSEARCHPATH path;

				itemData = ComboBox_GetItemData(hwndLocation, 
												searchPathIndex);
				path = (PSEARCHPATH) itemData;

				if( path )
				{
					PGPcomdlgCloseClientPrefs (	path->prefRef, 
												FALSE);
					free(path);
				}
			}

			CloseSearch ();
			return TRUE;
		}

		case WM_GETDLGCODE:
		{
			//MessageBox(NULL, "here", "wha?", MB_OK);
			return DLGC_WANTMESSAGE;	
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

		case WM_SIZE:
		{
			BOOL fwSizeType = wParam;      // resizing flag 
			WORD newWidth = LOWORD(lParam);  // width of client area 
			WORD newHeight = HIWORD(lParam); // height of client area 
			int	 delta =  newWidth - oldWidth;
			RECT rectControl;
			HWND hwndControl;
			//char szDebug[256];


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

				//wsprintf(szDebug, "delta = %d\r\n", delta);
				//OutputDebugString(szDebug);

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
						PGPcomdlgErrorMessage (error);
					}
					
					if( IsntPGPError(error) )
					{
						PSEARCHTHREADSTRUCT psts = NULL;

						psts = (PSEARCHTHREADSTRUCT)
									GetProp( hwnd, "SearchInfo");

						if( psts )
						{
							/*while(psts->threadHandle)
							{
								char szMsg[] = "Waiting on previous search to"
												" finish...";
								SEARCHPROGRESS progress;

								memset(&progress, 0x00, sizeof(progress));

								progress.nmhdr.hwndFrom = psts->hwnd;
								progress.nmhdr.idFrom = IDD_SEARCH;
								progress.nmhdr.code = SEARCH_PROGRESS;
								progress.step = SEARCH_PROGRESS_INFINITE; 
								progress.total = SEARCH_PROGRESS_INFINITE;
								
								strcpy(progress.message, szMsg);


								SendMessage(GetParent(hwnd), 
											WM_NOTIFY, 
											(WPARAM)hwnd, 
											(LPARAM)&progress);
							}*/

							memset(psts, 0x00, sizeof(SEARCHTHREADSTRUCT));

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

					psts = (PSEARCHTHREADSTRUCT) GetProp(hwnd, "SearchInfo");

					if(wNotifyCode)
					{
						break;
					}

					hwndSearch = GetDlgItem(hwnd, IDC_SEARCH);
					hwndStop = GetDlgItem(hwnd, IDC_STOP);

					EnableWindow(hwndSearch, FALSE);

					if(psts)
					{
						PGPEvent event;
						PGPEventErrorData data;

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

						//TerminateThread(psts->threadHandle, 0); 

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

					ShowWindow(hwndSearch, SW_SHOW);
					ShowWindow(hwndStop, SW_HIDE);

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
	PGPKeyServerAccessType	accessType	= kPGPKSAccess_Normal;
	PGPFlags		queryResultInfo		= 0;


	#if PGP_BUSINESS_SECURITY
	#if PGP_ADMIN_BUILD
		accessType	= kPGPKSAccess_Administrator;
	#endif	// PGP_ADMIN_BUILD
	#endif	// PGP_BUSINESS_SECURITY

	lastSearchType = (int)GetProp(hwndLocation, "LastSearchType");

	LoadString(hinst, IDS_DEFAULT_PATH, szString, sizeof(szString));
	searchPathIndex = ComboBox_FindString(hwndLocation, -1, szString);

	LoadString(hinst, IDS_LOCAL_KEYRING, szString, sizeof(szString));
	localSearchIndex = ComboBox_FindString(hwndLocation, -1, szString);

	LoadString(hinst, IDS_CURRENT_SEARCH, szString, sizeof(szString));
	lastSearchIndex = ComboBox_FindString(hwndLocation, -1, szString);

	locationSelection = ComboBox_GetCurSel(hwndLocation);

	lParam = ComboBox_GetItemData(hwndLocation, locationSelection);

	// Default Path 
	if( locationSelection == searchPathIndex )
	{
		PGPPrefRef prefs;
		PGPKeyServerRef server;
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

		error = PGPcomdlgOpenClientPrefs(&prefs);

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
								keyserverList[index].serverURL);

					SendMessage(GetParent(GetParent(hwndLocation)), 
								WM_NOTIFY, 
								(WPARAM)GetParent(hwndLocation), 
								(LPARAM)&progress);

					error = PGPNewKeyServerFromURL( g_ContextRef, 
								keyserverList[index].serverURL, 
								accessType,
								(bPending ? kPGPKSKeySpacePending : 
											kPGPKSKeySpaceDefault),
								&server );

					if( IsntPGPError( error ) )
					{
						error = PGPQueryKeyServer(	server, 
													psts->filter, 
													SearchEventHandler, 
													psts,
													pKeySetOut,
													&queryResultInfo);

						PGPFreeKeyServer( server );

						if( IsntPGPError( error ) )
						{
							ComboBox_SetItemData(	hwndLocation, 
													lastSearchIndex, 
													*pKeySetOut);

							strcpy(psts->url, keyserverList[index].serverURL);

							break;
						}
					}
				}

				PGPDisposeKeyServerPath(keyserverList);
			}

			PGPcomdlgCloseClientPrefs (prefs, FALSE);
		}

		if(searchString)
		{  
			free(searchString);
		}
	}
	// explicit keyserver
	else if( (lParam == (LPARAM) -1) )
	{
		PGPKeyServerRef server;
		
		*flags = FLAG_SEARCH_MEMORY;
		SetProp( hwndLocation, "LastSearchType",(HANDLE) *flags);

		// retrieve keyserver address from control
		locationLength = ComboBox_GetTextLength(hwndLocation);
		locationLength++; // Make room for trailing NULL
		locationBuffer = (char*) malloc(locationLength);

		if( locationBuffer )
		{
			ComboBox_GetText(	hwndLocation, 
								locationBuffer, 
								locationLength);

			error = PGPNewKeyServerFromURL( g_ContextRef, 
											locationBuffer, 
											accessType,
											(bPending ? kPGPKSKeySpacePending : 
														kPGPKSKeySpaceDefault),
											&server );

			strcpy(psts->url, locationBuffer);

			free(locationBuffer);
		}
		else
		{
			error = kPGPError_OutOfMemory;
		}

		if( IsntPGPError( error ) )
		{
			error = PGPQueryKeyServer(	server, 
										psts->filter, 
										SearchEventHandler, 
										psts,
										pKeySetOut,
										&queryResultInfo);

			PGPFreeKeyServer( server );

			if( IsntPGPError( error ) )
			{
				ComboBox_SetItemData(	hwndLocation, 
										lastSearchIndex, 
										*pKeySetOut);
			}

		}
	}
	else if (lParam == 0L ) // NULL: Invalid Selection
	{
		*pKeySetOut = kPGPInvalidRef;
	}
	else // peform on a local KeySet (keyring or mem)
	{
		localKeySet = (PGPKeySetRef) lParam;

		psts->url[0] = '\0';

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

		ComboBox_SetItemData(hwndLocation, lastSearchIndex, *pKeySetOut);
	}

	if(kPGPKeyServerQuery_PartialResults & queryResultInfo)
	{
		error = kPGPError_ServerTooManyResults;
	}

	return error;
}

DWORD WINAPI SearchThreadRoutine( LPVOID lpvoid)
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
		error == kPGPError_ServerTooManyResults)
	{
		result.pData = keySet;
		result.flags = flags;
		result.error = error;

		strcpy(result.url, psts->url);

		if(bPending)
		{
			result.flags |= FLAG_AREA_PENDING;
		}
	}
	else 
	{
		/*char szString[256];

		wsprintf(szString, "%d", error);

		MessageBox(NULL, szString, "returning error...", MB_OK);*/

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

	PGPFreeFilter ( psts->filter );

	EnableWindow(hwndSearch, TRUE);
	ShowWindow(hwndSearch, SW_SHOW);
	ShowWindow(hwndStop, SW_HIDE);

	//SetProp( psts->hwnd, "SearchInfo",(HANDLE) NULL);

	//free( psts );

	// clear searchthreadstruct
	memset(psts, 0x00, sizeof(SEARCHTHREADSTRUCT));

	return 0;
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
//				SEARCHABORT abort;

				psts->bCancel = TRUE;

				/*memset(&abort, 0x00, sizeof(abort));

				abort.nmhdr.hwndFrom = (HWND)userValue;
				abort.nmhdr.idFrom = IDD_SEARCH;
				abort.nmhdr.code = SEARCH_ABORT;
				abort.error = kPGPError_UserAbort;

				SendMessage(GetParent(psts->hwnd), 
						WM_NOTIFY, 
						(WPARAM)psts->hwnd, 
						(LPARAM)&abort);*/

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
				index = IDS_SEARCH_MESSAGE_BASE + data.state;
			}
			else
			{
				index = IDS_SEARCH_MESSAGE_CANCEL;
				progress.step = 0;
				progress.total = 0;
			}

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
		
		case kPGPEvent_KeyServerSignEvent:
		{
			char*			szPrompt		= "Please enter your passphrase:";
			PGPKeySetRef	signKeySet		= kPGPInvalidRef;
			PGPKeyRef		signKey			= kPGPInvalidRef;
			char*			szPassPhrase	= NULL;

			if (psts->bAlreadyAsked)
			{
				szPrompt = "Passphrase did not match. Please try again:";
			}
			
			error = PGPGetSignCachedPhrase(	context, 
											psts->hwnd, 
											szPrompt, 
											psts->bAlreadyAsked, 
											&szPassPhrase, 
											signKeySet,
											&signKey,
											NULL,
											NULL,
											0);

			if( IsntPGPError (error) )
			{
				psts->bAlreadyAsked = TRUE;

				error = PGPAddJobOptions(event->job,
										PGPOSignWithKey (
											context, 
											signKey, 
											PGPOPassphraseBuffer (
												context, 
												szPassPhrase, 
												strlen(szPassPhrase)),
											PGPOLastOption (context)),
										PGPOClearSign (context, TRUE),
										PGPOLastOption(context));
			}

			if (szPassPhrase)
			{
				PGPFreePhrase(szPassPhrase);
				szPassPhrase = NULL;
			}

			break;
			
		}
/*
		case kPGPEvent_PassphraseEvent:
		{
			char*			szPrompt		= "Please enter your passphrase:";
			PGPKeySetRef	signKeySet		= NULL;
			char*			szPassPhrase	= NULL;

			if (psts->bAlreadyAsked)
			{
				szPrompt = "Passphrase did not match. Please try again:";
			}

			if(!event->data.passphraseData.fConventional)
			{
				signKeySet = event->data.passphraseData.keyset;
			}

			error = PGPGetSignCachedPhrase(	context, 
											psts->hwnd, 
											szPrompt, 
											psts->bAlreadyAsked, 
											&szPassPhrase, 
											signKeySet,
											NULL,
											NULL,
											NULL,
											0);

			if( IsntPGPError (error) )
			{
				psts->bAlreadyAsked = TRUE;

				error = PGPAddJobOptions(event->job,
					PGPOPassphrase(context, szPassPhrase),
					PGPOLastOption(context));
			}

			if (szPassPhrase)
			{
				PGPFreePhrase(szPassPhrase);
				szPassPhrase = NULL;
			}

			break;
		}
*/
		case kPGPEvent_FinalEvent:
		{
			psts->bAlreadyAsked = FALSE;
			break;
		}
	}


	if( psts->bCancel )
	{
		error = kPGPError_UserAbort;
//		psts->bCancel = FALSE;
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

/*

  {
			LPVOID lpMessageBuffer;

			FormatMessage(
			  FORMAT_MESSAGE_ALLOCATE_BUFFER |
			  FORMAT_MESSAGE_FROM_SYSTEM,
			  NULL,
			  GetLastError(),
			  //The user default language
			  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
			  (LPTSTR) &lpMessageBuffer,
			  0,
			  NULL );

			// now display this string
 			MessageBox(NULL, (char*)lpMessageBuffer, 0, MB_OK);

			// Free the buffer allocated by the system
			LocalFree( lpMessageBuffer );
		}

  */