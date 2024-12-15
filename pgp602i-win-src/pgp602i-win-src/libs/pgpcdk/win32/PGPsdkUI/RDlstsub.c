/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: RDlstsub.c,v 1.7.10.1 1998/11/12 03:24:29 heller Exp $
____________________________________________________________________________*/

#include "RDprecmp.h"


/*
 * ListSub.c  List View helper routines
 *
 * These routines handle the listview subclass operations and also
 * do some of the needed operations on the linked list of keys.
 *
 * Copyright (C) 1996 Network Associates, Inc. and its affiliates.
 * All rights reserved.
 */


LRESULT WINAPI MyListviewWndProc(HWND hwnd, UINT msg, 
                                 WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_LBUTTONDBLCLK:
        {
            HWND hwndUserIds = 
                GetDlgItem(GetParent(hwnd), IDC_USER_ID_LIST);
            HWND hwndRecpts = 
                GetDlgItem(GetParent(hwnd), IDC_RECIPIENT_LIST);
                                                                        
            MoveListViewItems(hwndRecpts, hwndUserIds,
                              hwnd == hwndRecpts); 
            break;
        }

        case WM_CONTEXTMENU: 
        {
			POINT pnt;
			HMENU hMenu=NULL;
			HMENU hSubMenu=NULL;

			pnt.x=LOWORD(lParam);
			pnt.y=HIWORD(lParam);
	
			hMenu = LoadMenu(g_hinst, MAKEINTRESOURCE(IDR_MENU1));
			hSubMenu = GetSubMenu(hMenu,0);

			TrackPopupMenu(hSubMenu,TPM_RIGHTBUTTON,pnt.x,pnt.y,0,hwnd,NULL);
			DestroyMenu(hMenu);
			return 0;
        }

		case WM_SETFOCUS:
		case WM_KILLFOCUS:
		{
			InvalidateRect(hwnd,NULL,TRUE);
			break;
		}

        case WM_COMMAND:
        {
            switch(wParam)
            {
				case IDM_WHATSTHIS:
				{
					PRECGBL prg;
                    char szHelpFile[MAX_PATH];

                    GetHelpDir(szHelpFile);

					prg=(PRECGBL)GetWindowLong(GetParent(hwnd),GWL_USERDATA);

					if(hwnd == prg->hwndRecipients)
						WinHelp(hwnd, szHelpFile,
						HELP_CONTEXTPOPUP, IDH_IDC_RECIPIENT_LIST);
					if(hwnd == prg->hwndUserIDs)
						WinHelp(hwnd, szHelpFile,
						HELP_CONTEXTPOPUP, IDH_IDC_USER_ID_LIST);
					return 0;
				}

				case IDM_LOOKUP:
				{
					PRECGBL prg;
					HWND hdlg;
					PGPError err;

					hdlg=GetParent(hwnd);
					prg=(PRECGBL)GetWindowLong(GetParent(hwnd),GWL_USERDATA);

					err=LookUpUnknownKeys(hdlg,prg);
					break;
				}
			}
			break;
		}

		case WM_KEYDOWN:
        {
            int nVirtKey = (int) wParam;    // virtual-key code 
            HWND hwndUserIds = 
                GetDlgItem(GetParent(hwnd), IDC_USER_ID_LIST);
            HWND hwndRecpts = 
                GetDlgItem(GetParent(hwnd), IDC_RECIPIENT_LIST);

            if( ((nVirtKey == VK_RIGHT)||(nVirtKey == VK_LEFT)||
                (nVirtKey == VK_DELETE)) && (hwnd == hwndRecpts))
            {
                MoveListViewItems(hwndRecpts, hwndUserIds,TRUE);
                break;
            }
            else if( ((nVirtKey == VK_RIGHT)||(nVirtKey == VK_LEFT)||
                (nVirtKey == VK_DELETE)) && (hwnd == hwndUserIds))
            {
                MoveListViewItems(hwndRecpts, hwndUserIds,FALSE);
                break;
            }
            break;
        }

    }

    //  Pass all non-custom messages to old window proc
    return CallWindowProc((FARPROC)origListBoxProc, hwnd, 
                           msg, wParam, lParam ) ;
}

int LengthOfList(PUSERKEYINFO ListHead)
{
    PUSERKEYINFO pui;
    int length;

    length=0;
    pui=ListHead;

    while(pui!=0)
    {
        pui=pui->next;
        length++;
    }
    return length;
}


void FreeLinkedLists(PUSERKEYINFO ListHead)
{
    PUSERKEYINFO pui,oldpui;

    pui=ListHead;

    while(pui!=0)
    {
        oldpui=pui;
        pui=pui->next;

        free(oldpui);
    }
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
