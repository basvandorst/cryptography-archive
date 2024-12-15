/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: RDproc.c,v 1.21.2.2.2.1 1998/11/12 03:24:33 heller Exp $
____________________________________________________________________________*/

#include "RDprecmp.h"


/*
 * RecProc.c  Main message loop for the recipient dialog
 *
 * This message loops handles all the operations of the recipient
 * dialog, except those that are done in the listview subclass.
 *
 * Copyright (C) 1996 Network Associates, Inc. and its affiliates.
 * All rights reserved.
 */


// Used for WinHelp
static DWORD aIds[] = {            // Help IDs
IDC_RECIPIENT_LIST,IDH_IDC_RECIPIENT_LIST,
IDC_USER_ID_LIST,IDH_IDC_USER_ID_LIST,
801,IDH_TEXTOUTPUT, // Hardcoded in ClientLib
802,IDH_CONVENCRYPTION, // Hardcoded in ClientLib
803,IDH_WIPEORIGINAL, // Hardcoded in ClientLib
805,IDH_FYEO, // Hardcoded in ClientLib
0,0 
};

static float ColRatio[NUMCOLUMNS]={0.65F,0.11F,0.18F};
static char *RecColText[NUMCOLUMNS]={"Recipients","Validity","Size"};
static char *UserColText[NUMCOLUMNS]=
               {"Drag users from this list to the Recipients list",
                "Validity","Size"};

// Used for header controls of the listviews

// Used to calculate the placement of GUIs due to a resizing
#define DLGMARGIN 2
#define BUTTONWIDTH 75
#define BUTTONHEIGHT 25
#define DIVIDEMARGIN 3
#define BUTTONSPACE 10
#define MINDLGX 350
#define MINDLGY 350

int ResizeEm(HWND hdlg)
{
    RECT dlgRect,recipRect,userRect;
    RECT listareaRect,buttonareaRect,optRect;
    int divider;
	int topmsg,bottommsg;
	PRECGBL prg;
	int ButtOptArea;

	prg=(PRECGBL)(PRECGBL)GetWindowLong(hdlg,GWL_USERDATA);

	ButtOptArea=90; // The height for buttons and options

    GetClientRect(hdlg, &dlgRect);

    dlgRect.top+=DLGMARGIN;
    dlgRect.bottom-=DLGMARGIN;
    dlgRect.left+=DLGMARGIN;
    dlgRect.right-=DLGMARGIN;

    CopyRect(&listareaRect,&dlgRect);
    CopyRect(&buttonareaRect,&dlgRect);

    listareaRect.bottom=dlgRect.bottom-ButtOptArea-DIVIDEMARGIN;
    buttonareaRect.top=dlgRect.bottom-ButtOptArea+DIVIDEMARGIN;

    divider=(int)((float)listareaRect.bottom*(float)0.67);
	topmsg=divider;
	bottommsg=divider;

    CopyRect(&recipRect,&listareaRect);
    CopyRect(&userRect,&listareaRect);

	if(prg->AddUserRetVal>=ADDUSER_KEYSNOTVALID)
	{
		topmsg=divider-20;
		bottommsg=divider+20;

		MoveWindow(GetDlgItem(hdlg, IDC_MSGTXTBORDER),
		    userRect.left,topmsg+DIVIDEMARGIN,
			userRect.right-userRect.left,40-DIVIDEMARGIN*2,TRUE);

		ShowWindow(GetDlgItem(hdlg, IDC_MSGTXT),SW_SHOW);
	
		MoveWindow(GetDlgItem(hdlg, IDC_MSGTXT),
			userRect.left+4,topmsg+DIVIDEMARGIN+4,
			userRect.right-userRect.left-8,40-DIVIDEMARGIN*2-8,TRUE);

		ShowWindow(GetDlgItem(hdlg, IDC_MSGTXTBORDER),SW_SHOW);
	}
	else
	{
		ShowWindow(GetDlgItem(hdlg, IDC_MSGTXT),SW_HIDE);
		ShowWindow(GetDlgItem(hdlg, IDC_MSGTXTBORDER),SW_HIDE);
	}

	userRect.bottom=topmsg-DIVIDEMARGIN;
    recipRect.top=bottommsg+DIVIDEMARGIN;

    CopyRect(&optRect,&buttonareaRect);
    buttonareaRect.left=buttonareaRect.right-3*BUTTONWIDTH-
        2*BUTTONSPACE;
    optRect.right=buttonareaRect.left-BUTTONSPACE;

	MoveList(&(prg->lsRec),&recipRect);
	MoveList(&(prg->lsUser),&userRect);

    MoveWindow(GetDlgItem(hdlg, IDOK),
        buttonareaRect.left,buttonareaRect.top,
        BUTTONWIDTH,BUTTONHEIGHT,TRUE);
        
    MoveWindow(GetDlgItem(hdlg, IDCANCEL),
        buttonareaRect.left+BUTTONWIDTH+BUTTONSPACE,
        buttonareaRect.top,
        BUTTONWIDTH,BUTTONHEIGHT,TRUE);

    MoveWindow(GetDlgItem(hdlg, IDHELP),
        buttonareaRect.left+2*BUTTONWIDTH+2*BUTTONSPACE,
        buttonareaRect.top,
        BUTTONWIDTH,BUTTONHEIGHT,TRUE);

    InvalidateRect(hdlg,NULL,TRUE);

	if(prg->mDialogOptions!=NULL)
	{
		MoveOptionsControl(hdlg,&(prg->hwndOptions),
			prg->mDialogOptions,
			optRect.left,optRect.top,
			optRect.right-optRect.left,
			optRect.bottom-optRect.top);
	}

    return TRUE;
}

BOOL WINAPI RecipientDlgProc(HWND hdlg, UINT uMsg, 
                             WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_INITDIALOG:
        { 
            RECT rc;
			char MsgTxt[256];
			DRAWSTRUCT *ds;
			PRECGBL prg;

			InitCommonControls();

			// need to save away the pointer to our structure...
			SetWindowLong(hdlg,GWL_USERDATA,(long)lParam);

			prg=(PRECGBL)GetWindowLong(hdlg,GWL_USERDATA);

			prg->hwndRecDlg=hdlg;

			ds=&(prg->ds);
            
			CreateDrawElements(prg->context,ds);
			ds->DisplayMarginal=prg->mDisplayMarginalValidity;
			ds->MarginalInvalid=prg->mIgnoreMarginalValidity;

			InitList(hdlg,IDC_USER_ID_LIST,
				&(prg->lsUser),
				UserColText,
				ColRatio);
			InitList(hdlg,IDC_RECIPIENT_LIST,
				&(prg->lsRec),
				RecColText,
				ColRatio);

			prg->hwndUserIDs = prg->lsUser.hwndlist;
            prg->hwndRecipients = prg->lsRec.hwndlist;
      
            origListBoxProc=
                SubclassWindow(prg->hwndUserIDs,
                               MyListviewWndProc);  
            origListBoxProc=
                SubclassWindow(prg->hwndRecipients,
                               MyListviewWndProc);  
        
            if(prg->mWindowTitle)
            {
                SetWindowText(hdlg, prg->mWindowTitle);
            }
            // else they did not provide a title so keep default

			// Put necessary warning message up
			strcpy(MsgTxt,"");

			if((prg->AddUserRetVal)&ADDUSER_KEYSNOTCORPSIGNED)
				LoadString(g_hinst, IDS_KEYSNOTCORPSIGNED, 
						MsgTxt, sizeof(MsgTxt));
			if((prg->AddUserRetVal)&ADDUSER_ADKMISSING)
				LoadString(g_hinst, IDS_ADKMISSING, 
						MsgTxt, sizeof(MsgTxt));
			else if((prg->AddUserRetVal)&ADDUSER_KEYSNOTVALID)
				LoadString(g_hinst, IDS_KEYSNOTVALID, 
						MsgTxt, sizeof(MsgTxt));
			else if((prg->AddUserRetVal)&ADDUSER_KEYSNOTFOUND)
				LoadString(g_hinst, IDS_KEYSNOTFOUND, 
						MsgTxt, sizeof(MsgTxt));
			else if((prg->AddUserRetVal)&ADDUSER_MULTIPLEMATCH)
				LoadString(g_hinst, IDS_MULTIPLEMATCH, 
						MsgTxt, sizeof(MsgTxt));
	
			SetWindowText(GetDlgItem(hdlg, IDC_MSGTXT),MsgTxt);

            prg->RSortAscending=TRUE;
            prg->RSortSub=0;
            prg->USortAscending=TRUE;
            prg->USortSub=0;
 
			BuildTables(prg);

            ResizeEm(hdlg);

			GetWindowRect (hdlg, &rc);
            SetWindowPos (hdlg, NULL,
                (GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
                (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2,
                0, 0, SWP_NOSIZE | SWP_NOZORDER);

			 // Go to keyserver for not founds
            if(prg->mSearchBeforeDisplay)   
            {
				PGPError err;

                ShowWindow(hdlg,SW_SHOW);
                SetForegroundWindow(hdlg);
                err=LookUpUnknownKeys(hdlg,prg);
			}
			else
				SetForegroundWindow(hdlg);

			SetFocus(prg->hwndUserIDs);
            return FALSE;
        }

		case WM_CLOSE:
        case WM_QUIT:
        case WM_DESTROY:
        {
			PRECGBL prg;

			prg=(PRECGBL)GetWindowLong(hdlg,GWL_USERDATA);

            DeleteDrawElements(&(prg->ds));
    
            SubclassWindow(prg->hwndRecipients,origListBoxProc);
            SubclassWindow(prg->hwndUserIDs,origListBoxProc);
			EndDialog(hdlg, FALSE);
            break;
        }

		case WM_GETMINMAXINFO:
		{
			MINMAXINFO* lpmmi;

		    lpmmi = (MINMAXINFO*) lParam;
    		lpmmi->ptMinTrackSize.x = MINDLGX;
    		lpmmi->ptMinTrackSize.y = MINDLGY;
            break;
		}

#if LISTBOX
		case WM_COMPAREITEM:
		{     
			COMPAREITEMSTRUCT *comp;
			PUSERKEYINFO pui1,pui2;   
			HWND hwndFrom;

			comp=(COMPAREITEMSTRUCT *)lParam;
			pui1=(PUSERKEYINFO)comp->itemData1;
			pui2=(PUSERKEYINFO)comp->itemData2; 
			hwndFrom=comp->hwndItem;

			return ListViewCompareProc((LPARAM)pui1,
				(LPARAM)pui2,
				(LPARAM)hwndFrom);  
		}
#endif

        case WM_SIZE:
        {
            ResizeEm(hdlg);
            break;
        }

#if !LISTBOX
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
					PRECGBL prg;

					prg=(PRECGBL)GetWindowLong(hdlg,GWL_USERDATA);

                    prg->hwndDragFrom = ptrNMHDR->hwndFrom;

                    if(!ListView_GetSelectedCount(prg->hwndDragFrom))
                    {
                        break; //  Exit if not
                    }

                    result=ImageList_DragShowNolock(FALSE);

                    prg->hDragImage=
                        MakeDragImage(prg->hwndDragFrom,
                                      &left,&bottom);

                    SetCapture(hdlg);

                    pnt.x=((NM_LISTVIEW *)lParam)->ptAction.x;
                    pnt.y=((NM_LISTVIEW *)lParam)->ptAction.y;

                    dx=pnt.x-left;
                    dy=pnt.y-bottom;

                    result=ImageList_BeginDrag(prg->hDragImage,
                                               0,dx,dy);
    
                    MapWindowPoints(prg->hwndDragFrom,hdlg,
                        (LPPOINT)&pnt,(UINT)1);

                    ImageList_DragEnter(hdlg,pnt.x,pnt.y);

                    result=ImageList_DragShowNolock(TRUE);
    
                    prg->bDragging = TRUE;
                    break;
                }

                case LVN_COLUMNCLICK: // Sent by ListView when user 
                {                     // clicks header control
                    int *SortSub;
                    BOOL *SortAscending;
					PRECGBL prg;

					prg=(PRECGBL)GetWindowLong(hdlg,GWL_USERDATA);

                    if(ptrNMHDR->hwndFrom==prg->hwndRecipients)
                    {
                        SortSub=&(prg->RSortSub);
                        SortAscending=&(prg->RSortAscending);
                    }
                    else
                    {
                        SortSub=&(prg->USortSub);
                        SortAscending=&(prg->USortAscending);
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

#endif // !LISTBOX

        case WM_SYSCOLORCHANGE:
        {
			PRECGBL prg;

			prg=(PRECGBL)GetWindowLong(hdlg,GWL_USERDATA);

            DeleteDrawElements(&(prg->ds));
            CreateDrawElements(prg->context,&(prg->ds));

            SendMessage(prg->hwndRecipients,
                WM_SYSCOLORCHANGE,0,0);
            SendMessage(prg->hwndUserIDs,
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
			char szHelpFile[MAX_PATH+1];

            GetHelpDir(szHelpFile);

            WinHelp (((LPHELPINFO) lParam)->hItemHandle, szHelpFile, 
                  HELP_WM_HELP, (DWORD) (LPSTR) aIds); 
            break;
        }

		// Note, ListView's context are done through subclass
        case WM_CONTEXTMENU: 
		{
			char szHelpFile[MAX_PATH+1];

			GetHelpDir(szHelpFile);

			WinHelp ((HWND) wParam, szHelpFile, HELP_CONTEXTMENU, 
				(DWORD) (LPVOID) aIds); 
			break;
		}

                
#if !LISTBOX
          
        case WM_MOUSEMOVE:
        {
			PRECGBL prg;

			prg=(PRECGBL)GetWindowLong(hdlg,GWL_USERDATA);

            if (prg->bDragging)
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
			PRECGBL prg;

			prg=(PRECGBL)GetWindowLong(hdlg,GWL_USERDATA);

            if (prg->bDragging)
            {
                HWND hwndTarget;            // window under mouse
                POINT pt;
                int result;

                // Release the mouse capture
                ReleaseCapture();
                // Clear the drag flag
                prg->bDragging = FALSE;
                result=ImageList_DragShowNolock(FALSE);
                ImageList_DragLeave(hdlg);
                ImageList_EndDrag();

                ImageList_Destroy(prg->hDragImage);

                pt.x = LOWORD(lParam);  // horizontal position of cursor 
                pt.y = HIWORD(lParam);  // vertical position of cursor

                ClientToScreen (hdlg, &pt);

                // First, check to see if there is a valid drop point.
                hwndTarget = WindowFromPoint( pt );

                // make sure everything is going in the right direction
                if(((hwndTarget == prg->hwndRecipients) ||
                    (hwndTarget == prg->hwndUserIDs)) 
                    && (hwndTarget != prg->hwndDragFrom))
                {
                    MoveListViewItems(prg->hwndRecipients,prg->hwndUserIDs,
                        prg->hwndDragFrom==prg->hwndRecipients);
                }

            }

            break;
        }

#endif // !LISTBOX

        case WM_COMMAND:
        {
            switch(wParam)
            {
                case IDOK:
                {
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
                    char szHelpFile[MAX_PATH+1];

                    GetHelpDir(szHelpFile);

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

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
