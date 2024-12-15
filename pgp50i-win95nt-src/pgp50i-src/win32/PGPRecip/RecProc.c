/*
 * RecProc.c  Main message loop for the recipient dialog
 *
 * This message loops handles all the operations of the recipient
 * dialog, except those that are done in the listview subclass.
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */

#include "precomp.h"

// Used for WinHelp
static DWORD aIds[] = {            // Help IDs
IDCANCEL,IDH_IDCANCEL,
IDC_CHECK_ASCII,IDH_IDC_CHECK_ASCII,
IDC_RECIPIENT_LIST,IDH_IDC_RECIPIENT_LIST,
IDC_USER_ID_LIST,IDH_IDC_USER_ID_LIST,
IDOK,IDH_IDOK,
    0,0
};

// Used for header controls of the listviews
#define NUMCOLUMNS 4

static int LVWidth[NUMCOLUMNS]={260,50,50,75};
static char *RecColText[NUMCOLUMNS]={"Recipients","Validity","Trust","Size"};
static char *UserColText[NUMCOLUMNS]=
                {"Drag users from this list to the Recipients list",
                 "Validity","Trust","Size"};

// Used to calculate the placement of GUIs due to a resizing
#define DLGMARGIN 4
#define BUTTONWIDTH 75
#define BUTTONHEIGHT 25
#define DIVIDEMARGIN 4
#define BUTTONSPACE 10
#define BUTTONXMARGIN 20
#define BUTTONYMARGIN 5
#define BUTTONSHIFT 5
#define OPTMARGIN 20
#define CHECKX 10
#define CHECKY 20
#define CHECKWIDTH 75
#define CHECKHEIGHT 15
#define MINDLGX 400
#define MINDLGY 275

int ResizeEm(HWND hdlg)
{
    RECT dlgRect,recipRect,userRect;
    RECT listareaRect,buttonareaRect,optRect;
    int divider;

    GetClientRect(hdlg, &dlgRect);

    if(dlgRect.right<MINDLGX)
        dlgRect.right=MINDLGX;
    if(dlgRect.bottom<MINDLGY)
        dlgRect.bottom=MINDLGY;

    dlgRect.top+=DLGMARGIN;
    dlgRect.bottom-=DLGMARGIN;
    dlgRect.left+=DLGMARGIN;
    dlgRect.right-=DLGMARGIN;

    CopyRect(&listareaRect,&dlgRect);
    CopyRect(&buttonareaRect,&dlgRect);

    listareaRect.bottom=dlgRect.bottom-
        BUTTONHEIGHT-BUTTONYMARGIN-OPTMARGIN;
    buttonareaRect.top=dlgRect.bottom-BUTTONHEIGHT;

    divider=(int)((float)listareaRect.bottom*(float)0.67);

    CopyRect(&recipRect,&listareaRect);
    CopyRect(&userRect,&listareaRect);

    userRect.bottom=divider-DIVIDEMARGIN;
    recipRect.top=divider+DIVIDEMARGIN;

    CopyRect(&optRect,&buttonareaRect);
    buttonareaRect.left=buttonareaRect.right-3*BUTTONWIDTH-
        2*BUTTONSPACE-BUTTONSHIFT;
    optRect.right=buttonareaRect.left-BUTTONXMARGIN;
        optRect.top-=OPTMARGIN;

    MoveWindow(GetDlgItem(hdlg, IDC_RECIPIENT_LIST),
        recipRect.left,recipRect.top,
        recipRect.right-recipRect.left,
        recipRect.bottom-recipRect.top,TRUE);

    MoveWindow(GetDlgItem(hdlg, IDC_USER_ID_LIST),
        userRect.left,userRect.top,
        userRect.right-userRect.left,
        userRect.bottom-userRect.top,TRUE);

    MoveWindow(GetDlgItem(hdlg, IDC_STATIC_OPTIONS_GROUP),
        optRect.left,optRect.top,
        optRect.right-optRect.left,
        optRect.bottom-optRect.top,TRUE);

    MoveWindow(GetDlgItem(hdlg, IDC_CHECK_ASCII),
        optRect.left+CHECKX,optRect.top+CHECKY,
        CHECKWIDTH,CHECKHEIGHT,TRUE);

    MoveWindow(GetDlgItem(hdlg, IDOK),
        buttonareaRect.left,buttonareaRect.top-BUTTONSHIFT,
        BUTTONWIDTH,BUTTONHEIGHT,TRUE);

    MoveWindow(GetDlgItem(hdlg, IDCANCEL),
        buttonareaRect.left+BUTTONWIDTH+BUTTONSPACE,
        buttonareaRect.top-BUTTONSHIFT,
        BUTTONWIDTH,BUTTONHEIGHT,TRUE);

    MoveWindow(GetDlgItem(hdlg, IDHELP),
        buttonareaRect.left+2*BUTTONWIDTH+2*BUTTONSPACE,
        buttonareaRect.top-BUTTONSHIFT,
        BUTTONWIDTH,BUTTONHEIGHT,TRUE);

    InvalidateRect(hdlg,NULL,TRUE);
    return TRUE;
}


void GetHelpFile(char *Path)
{
    DWORD Size,Type;
    HKEY hkey;
    int PathLen;
    char *EndOfPath;

    if(ERROR_SUCCESS ==
                RegOpenKeyEx(HKEY_CURRENT_USER,
                "SOFTWARE\\PGP\\PGP\\PGPkeys",
                0,KEY_READ,&hkey))
    {
        Size = 500;
        if(RegQueryValueEx(hkey,"ExePath", 0,&Type,(BYTE*)Path,&Size) !=
            ERROR_SUCCESS)
        {
            strcpy(Path, "C:\\");
        }
        else
        {
            EndOfPath=strstri(Path,"PGPkeys.exe");
            if(EndOfPath!=0)
                 *EndOfPath=0;

//Add a trailing backslash to the path if there isn't one there already.
            PathLen = strlen(Path);
            if(PathLen && Path[PathLen - 1] != '\\')
                 strcat(Path, "\\");
        }

        RegCloseKey(hkey);
    }

    strcat(Path,"PGP50.hlp");

}

void ReloadRings(HWND hdlg)
{
        MakeReloadString((RecGbl(hdlg))->hwndRecipients,
            &((RecGbl(hdlg))->prds->dwNumRecipients),
            &((RecGbl(hdlg))->prds->szRecipients));
        ListView_DeleteAllItems((RecGbl(hdlg))->hwndRecipients);
        ListView_DeleteAllItems((RecGbl(hdlg))->hwndUserIDs);
        FreeLinkedLists((RecGbl(hdlg))->gUserLinkedList);
        AddUsersToLists(&((RecGbl(hdlg))->gUserLinkedList),
            (RecGbl(hdlg))->prds->dwNumRecipients,
            (RecGbl(hdlg))->prds->szRecipients,TRUE);
        AddLinkedListtoListView((RecGbl(hdlg))->hwndUserIDs,
            (RecGbl(hdlg))->gUserLinkedList);
        InitialMoveListViewItems((RecGbl(hdlg))->hwndRecipients,
            (RecGbl(hdlg))->hwndUserIDs,
            (RecGbl(hdlg))->gUserLinkedList);
}

BOOL WINAPI RecipientDlgProc(HWND hdlg, UINT uMsg,
                             WPARAM wParam, LPARAM lParam)
{

     if(uMsg==PGPM_RELOADKEYRING)
         ReloadRings(hdlg);

     switch(uMsg)
     {
         case WM_INITDIALOG:
         {
             RECT rc;
             LOGFONT lf;
             BOOL AutoKeyLookUp;

             InitRecGbl(hdlg,(PRECGBL)lParam);

             InitCommonControls();
             // need to save away the pointer to our structure...

             CreateDrawElements(hdlg);
             // save some hwnds
             (RecGbl(hdlg))->hwndRecipients =
                 GetDlgItem(hdlg, IDC_RECIPIENT_LIST);
             (RecGbl(hdlg))->hwndUserIDs =
                 GetDlgItem(hdlg, IDC_USER_ID_LIST);
             (RecGbl(hdlg))->hwndOptionASCII =
                 GetDlgItem(hdlg, IDC_CHECK_ASCII);

             origListBoxProc=
                 SubclassWindow((RecGbl(hdlg))->hwndRecipients,
                                  MyListviewWndProc);
             origListBoxProc=
                 SubclassWindow((RecGbl(hdlg))->hwndUserIDs,
                                  MyListviewWndProc);

             SystemParametersInfo (SPI_GETICONTITLELOGFONT,
                                     sizeof(LOGFONT), &lf, 0);

             (RecGbl(hdlg))->g_hTLFont = CreateFontIndirect (&lf);
             lf.lfItalic = !lf.lfItalic;
             (RecGbl(hdlg))->g_hTLFontItalic = CreateFontIndirect (&lf);
             lf.lfItalic = !lf.lfItalic;
             lf.lfStrikeOut=TRUE;
             (RecGbl(hdlg))->g_hTLFontStrike = CreateFontIndirect (&lf);


             if((RecGbl(hdlg))->prds->szTitle)
             {
                 SetWindowText(hdlg, (RecGbl(hdlg))->prds->szTitle);
             }
             // else they did not provide a title so keep default

            Button_SetCheck((RecGbl(hdlg))->hwndOptionASCII,
                (BOOL)((RecGbl(hdlg))->prds->dwPGPFlags & PGP_ASCII_ARMOR) );

            if( (BOOL)((RecGbl(hdlg))->prds->dwDisableFlags &
                       DISABLE_ASCII_ARMOR) )
            {
                EnableWindow((RecGbl(hdlg))->hwndOptionASCII, FALSE);
            }

            if((BOOL)((RecGbl(hdlg))->prds->dwDisableFlags &
                       DISABLE_ASCII_ARMOR))
            {
                EnableWindow(GetDlgItem(hdlg, IDC_STATIC_OPTIONS_GROUP),
                             FALSE);
            }

            if(!AddImagesToImageList(hdlg,&((RecGbl(hdlg))->hImageList)))
            {
                EndDialog(hdlg, FALSE);
            }
            else
            {
                ListView_SetImageList((RecGbl(hdlg))->hwndRecipients,
                    (RecGbl(hdlg))->hImageList, LVSIL_SMALL);
                ListView_SetImageList((RecGbl(hdlg))->hwndUserIDs,
                    (RecGbl(hdlg))->hImageList, LVSIL_SMALL);
            }

            if(!AddListViewColumn((RecGbl(hdlg))->hwndRecipients,
                                  RecColText,LVWidth,NUMCOLUMNS) ||
               !AddListViewColumn((RecGbl(hdlg))->hwndUserIDs,
                                  UserColText,LVWidth,NUMCOLUMNS) )
            {
                EndDialog(hdlg, FALSE);
            }

            (RecGbl(hdlg))->RSortAscending=TRUE;
            (RecGbl(hdlg))->RSortSub=0;
            (RecGbl(hdlg))->USortAscending=TRUE;
            (RecGbl(hdlg))->USortSub=0;

            AddLinkedListtoListView((RecGbl(hdlg))->hwndUserIDs,
                (RecGbl(hdlg))->gUserLinkedList);

            InitialMoveListViewItems((RecGbl(hdlg))->hwndRecipients,
                (RecGbl(hdlg))->hwndUserIDs,(RecGbl(hdlg))->gUserLinkedList);

            ResizeEm(hdlg);

              GetWindowRect (hdlg, &rc);
            SetWindowPos (hdlg, NULL,
                (GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
                (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2,
                0, 0, SWP_NOSIZE | SWP_NOZORDER);

            GetKeyserverValues(NULL,NULL,&AutoKeyLookUp);

            if(AutoKeyLookUp)    // Go to keyserver for not founds
            {
                ShowWindow(hdlg,SW_SHOW);
                SetForegroundWindow(hdlg);
                LookUpNotFoundKeys(hdlg);
            }
            else
            {
                SetForegroundWindow(hdlg);
            }

            return TRUE;
       }

       case WM_SIZE:
       {
            ResizeEm(hdlg);
            break;
       }

       #define ptrNMHDR       ((LPNMHDR)lParam)
       #define ptrNM_LISTVIEW ((NM_LISTVIEW *)lParam)
       #define ptrTV_DISPINFO ((TV_DISPINFO *)lParam)


       case WM_NOTIFY:
       {
            switch (ptrNMHDR->code)
            {
                case LVN_BEGINDRAG: // Sent by ListView when user
                {                    // wants to drag an item.
                    int dx,dy,left,bottom,result;
                    POINT pnt;

                    (RecGbl(hdlg))->hwndDragFrom = ptrNMHDR->hwndFrom;

                    if(!ListView_GetSelectedCount(
                         (RecGbl(hdlg))->hwndDragFrom))
                    {
                         break; //  Exit if not
                    }

                    result=ImageList_DragShowNolock(FALSE);

                    (RecGbl(hdlg))->hDragImage=
                         MakeDragImage((RecGbl(hdlg))->hwndDragFrom,
                                        &left,&bottom);

                    SetCapture(hdlg);

                    pnt.x=((NM_LISTVIEW *)lParam)->ptAction.x;
                    pnt.y=((NM_LISTVIEW *)lParam)->ptAction.y;

                    dx=pnt.x-left;
                    dy=pnt.y-bottom;

                    result=ImageList_BeginDrag((RecGbl(hdlg))->hDragImage,
                                                0,dx,dy);

                    MapWindowPoints((RecGbl(hdlg))->hwndDragFrom,hdlg,
                        (LPPOINT)&pnt,(UINT)1);

                    ImageList_DragEnter(hdlg,pnt.x,pnt.y);

                    result=ImageList_DragShowNolock(TRUE);

                    (RecGbl(hdlg))->bDragging = TRUE;
                    break;
                }

                case LVN_COLUMNCLICK: // Sent by ListView when user
                {                     // clicks header control
                    int *SortSub;
                    BOOL *SortAscending;

                    if(ptrNMHDR->hwndFrom==(RecGbl(hdlg))->hwndRecipients)
                    {
                        SortSub=&((RecGbl(hdlg))->RSortSub);
                        SortAscending=&((RecGbl(hdlg))->RSortAscending);
                    }
                    else
                    {
                        SortSub=&((RecGbl(hdlg))->USortSub);
                        SortAscending=&((RecGbl(hdlg))->USortAscending);
                    }

                    if(*SortSub==ptrNM_LISTVIEW->iSubItem)
                    {
                         *SortAscending=!(*SortAscending);
                    }
                    else
                    {
                        *SortAscending=TRUE;
                        *SortSub=ptrNM_LISTVIEW->iSubItem;
                    }

                    SortEm(ptrNMHDR->hwndFrom);

                    break;
                }

            }// switch

            break;
        } // case

        case WM_SYSCOLORCHANGE:
        {
            DeleteDrawElements(hdlg);
            CreateDrawElements(hdlg);
            SendMessage((RecGbl(hdlg))->hwndRecipients,
                WM_SYSCOLORCHANGE,0,0);
            SendMessage((RecGbl(hdlg))->hwndUserIDs,
                WM_SYSCOLORCHANGE,0,0);
            break;
        }

        case WM_DRAWITEM:
        {
            Main_OnDrawItem(hdlg,(LPDRAWITEMSTRUCT) lParam);
            return TRUE;
        }

        case WM_MEASUREITEM:
            Main_OnMeasureItem(hdlg,(LPMEASUREITEMSTRUCT) lParam);
            return TRUE;

        case WM_HELP:
        {
            char szHelpFile[500];

            GetHelpFile(szHelpFile);

            WinHelp (((LPHELPINFO) lParam)->hItemHandle, szHelpFile,
                  HELP_WM_HELP, (DWORD) (LPSTR) aIds);
            break;
        }
/*
        case WM_CONTEXTMENU:
            WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU,
               (DWORD) (LPVOID) aIds);
            break;

              Replaced with this for keyserver
*/
        case WM_CONTEXTMENU:
        {
            if((wParam == (WPARAM) ((RecGbl(hdlg))->hwndRecipients))||
               (wParam == (WPARAM) ((RecGbl(hdlg))->hwndUserIDs)))
            {

                SendMessage((HWND)wParam,
                    WM_PGP_DO_CONTEXT_MENU, wParam, lParam);
            }
            else
            {
                char szHelpFile[500];

                GetHelpFile(szHelpFile);

                WinHelp ((HWND) wParam, szHelpFile, HELP_CONTEXTMENU,
                  (DWORD) (LPVOID) aIds);
            }
            break;
        }

        case WM_PGP_GET_LEFT_HELP:
        {
            char szHelpFile[500];

            GetHelpFile(szHelpFile);

            if(lParam == (LPARAM) ((RecGbl(hdlg))->hwndRecipients))
                 WinHelp(hdlg, szHelpFile,
                    HELP_CONTEXTPOPUP, IDH_IDC_RECIPIENT_LIST);
            if(lParam == (LPARAM) ((RecGbl(hdlg))->hwndUserIDs))
                WinHelp(hdlg, szHelpFile,
                    HELP_CONTEXTPOPUP, IDH_IDC_USER_ID_LIST);

            break;
        }
/* End Replacement */

        case WM_DESTROY:
            {
            ImageList_Destroy((RecGbl(hdlg))->hImageList);
            DeleteDrawElements(hdlg);
            DeleteObject((RecGbl(hdlg))->g_hTLFont);
            DeleteObject((RecGbl(hdlg))->g_hTLFontItalic);
            DeleteObject((RecGbl(hdlg))->g_hTLFontStrike);
            if((RecGbl(hdlg))->hPopup)
            {
                DestroyMenu((RecGbl(hdlg))->hPopup);
            }

            SubclassWindow((RecGbl(hdlg))->hwndRecipients,origListBoxProc);
            SubclassWindow((RecGbl(hdlg))->hwndUserIDs,origListBoxProc);

            FreeRecGbl(hdlg);

            EndDialog(hdlg, FALSE);
            break;
            }


        case WM_MOUSEMOVE:
        {
            if ((RecGbl(hdlg))->bDragging)
            {
                 int result;

                 // drag the item to the current mouse position
                 result=ImageList_DragMove(LOWORD(lParam),
                     HIWORD(lParam));
            }

            break;
        }

        case WM_LBUTTONUP:
        {
            if ((RecGbl(hdlg))->bDragging)
            {
                 HWND hwndTarget;            // window under mouse
                 POINT pt;
                 int result;

                 // Release the mouse capture
                 ReleaseCapture();
                 // Clear the drag flag
                 (RecGbl(hdlg))->bDragging = FALSE;
                 result=ImageList_DragShowNolock(FALSE);
                ImageList_DragLeave(hdlg);
                ImageList_EndDrag();

                ImageList_Destroy((RecGbl(hdlg))->hDragImage);

                pt.x = LOWORD(lParam);  // horizontal position of cursor
                pt.y = HIWORD(lParam);  // vertical position of cursor

                ClientToScreen (hdlg, &pt);

                // First, check to see if there is a valid drop point.
                hwndTarget = WindowFromPoint( pt );

                // make sure everything is going in the right direction
                if(((hwndTarget == (RecGbl(hdlg))->hwndRecipients) ||
                    (hwndTarget == (RecGbl(hdlg))->hwndUserIDs))
                    && (hwndTarget != (RecGbl(hdlg))->hwndDragFrom))
                {
                    MoveListViewItems((RecGbl(hdlg))->hwndRecipients,
                        (RecGbl(hdlg))->hwndUserIDs,
                        (RecGbl(hdlg))->hwndDragFrom==
                             (RecGbl(hdlg))->hwndRecipients);
                }

            }

            break;
        }

        case WM_COMMAND:
        {
            switch(wParam)
            {
                case IDC_CHECK_ASCII:
                {
                    (RecGbl(hdlg))->prds->dwPGPFlags^= PGP_ASCII_ARMOR;
                    Button_SetCheck((RecGbl(hdlg))->hwndOptionASCII,
                        (BOOL)((RecGbl(hdlg))->prds->dwPGPFlags &
                             PGP_ASCII_ARMOR));
                    break;
                }

                case IDOK:
                {
                    int RSAandDSA;

                    RSAandDSA=WriteOutKeyIds((RecGbl(hdlg))->prds,
						(RecGbl(hdlg))->hwndRecipients);

                    if(RSAandDSA)
                        if(RSADSARecWarning(hdlg)==FALSE)
                            break;

                    EndDialog(hdlg, TRUE);
                    break;
                }

                case IDCANCEL:
                {
                    EndDialog(hdlg, FALSE);
                    break;
                }

                case IDHELP:
                {
                    char szHelpFile[500];

                    GetHelpFile(szHelpFile);

                    WinHelp (hdlg, szHelpFile, HELP_CONTEXT,
                        IDH_IDD_RECIPIENTDLG);
                    break;
                }

            }
            return TRUE;
       }

    }
    return FALSE;
}
