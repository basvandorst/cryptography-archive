/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: RDproc.c,v 1.30.4.1 1997/11/20 21:17:47 wjb Exp $
____________________________________________________________________________*/

#include "RDprecmp.h"


/*
 * RecProc.c  Main message loop for the recipient dialog
 *
 * This message loops handles all the operations of the recipient
 * dialog, except those that are done in the listview subclass.
 *
 * Copyright (C) 1996 Pretty Good Privacy, Inc.
 * All rights reserved.
 */


// Used for WinHelp
static DWORD aIds[] = {            // Help IDs
IDCANCEL,IDH_IDCANCEL,
IDC_CHECK_ASCII,IDH_IDC_CHECK_ASCII,
IDC_RECIPIENT_LIST,IDH_IDC_RECIPIENT_LIST,
IDC_USER_ID_LIST,IDH_IDC_USER_ID_LIST,
IDOK,IDH_IDOK,
IDC_PASSONLY,IDH_IDC_PASSONLY,
IDC_WIPEORIG,IDH_IDC_WIPEORIG,
0,0 
};

static float ColRatio[NUMCOLUMNS]={0.60,0.11,0.11,0.18};
static char *RecColText[NUMCOLUMNS]={"Recipients","Validity","Trust","Size"};
static char *UserColText[NUMCOLUMNS]=
               {"Drag users from this list to the Recipients list",
                "Validity","Trust","Size"};

// Used for header controls of the listviews

// Used to calculate the placement of GUIs due to a resizing
#define DLGMARGIN 2
#define BUTTONWIDTH 75
#define BUTTONHEIGHT 25
#define DIVIDEMARGIN 3
#define BUTTONSPACE 10
#define BUTTONXMARGIN 20
#define BUTTONYMARGIN 5
#define BUTTONSHIFT 5
#define OPTMARGIN 24
#define CHECKX 10
#define CHECKY 15
#define CHECKWIDTH 100
#define CHECKHEIGHT 15
#define MINDLGX 500
#define MINDLGY 275

int ResizeEm(HWND hdlg)
{
    RECT dlgRect,recipRect,userRect;
    RECT listareaRect,buttonareaRect,optRect;
    int divider;
	int topmsg,bottommsg;

    GetClientRect(hdlg, &dlgRect);
/*
    if(dlgRect.right<MINDLGX) 
        dlgRect.right=MINDLGX;
    if(dlgRect.bottom<MINDLGY)
        dlgRect.bottom=MINDLGY;
*/
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
	topmsg=divider;
	bottommsg=divider;

    CopyRect(&recipRect,&listareaRect);
    CopyRect(&userRect,&listareaRect);

	if((RecGbl(hdlg))->AddUserRetVal>=ADDUSER_KEYSNOTVALID)
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
        2*BUTTONSPACE-BUTTONSHIFT;
    optRect.right=buttonareaRect.left-BUTTONXMARGIN;
        optRect.top-=OPTMARGIN;

	MoveList(&(RecGbl(hdlg)->lsRec),&recipRect);
	MoveList(&(RecGbl(hdlg)->lsUser),&userRect);

    MoveWindow(GetDlgItem(hdlg, IDC_STATIC_OPTIONS_GROUP),
        optRect.left,optRect.top,
        optRect.right-optRect.left,
        optRect.bottom-optRect.top,TRUE);

    MoveWindow(GetDlgItem(hdlg, IDC_CHECK_ASCII),
        optRect.left+CHECKX,optRect.top+CHECKY,
        CHECKWIDTH,CHECKHEIGHT,TRUE);

    MoveWindow(GetDlgItem(hdlg, IDC_PASSONLY),
        optRect.left+CHECKX,optRect.top+CHECKY+15,
        CHECKWIDTH*2,CHECKHEIGHT,TRUE);

    MoveWindow(GetDlgItem(hdlg, IDC_WIPEORIG),
        optRect.left+CHECKX+105,optRect.top+CHECKY,
        90,CHECKHEIGHT,TRUE);

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
 	PGPcomdlgGetPGPPath (Path,MAX_PATH);
    strcat(Path,"PGP55.hlp");
}

void ProcessMessages(void)
{
	MSG msg;

	// Clean up our screen for the moment.
	while(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

// We should really have a lock of some kind here..
void ReloadRings(HWND hdlg)
{
	DWORD	tempNumRec;
	char	**tempRecArray;
	PRECGBL prg;
	PRECIPIENTDIALOGSTRUCT prds;
	int dummy;
	PGPError err;
	char WinText[100];
	PPREFS Prefs;

	prg=(RecGbl(hdlg));
	Prefs=prg->Prefs;

	// Don't want to process two at the same time!
	if(prg->Reloading==TRUE)
	{
		prg->Pending=TRUE;
		return;
	}

	do
	{
		prg->Reloading=TRUE;
		prg->Pending=FALSE;

		GetWindowText(hdlg,WinText,99);
		SetWindowText(hdlg,"Reloading keyrings. Please wait...");

		prds=prg->prds;

		tempNumRec=prds->dwNumRecipients;
		tempRecArray=prds->szRecipientArray;

		MakeReloadArray(prg->hwndRecipients,prds);

		ListView_DeleteAllItems(prg->hwndRecipients);
		ListView_DeleteAllItems(prg->hwndUserIDs);
		FreeLinkedLists((RecGbl(hdlg))->gUserLinkedList);
		(RecGbl(hdlg))->gUserLinkedList=0;
		PGPFreeKeySet(prds->OriginalKeySetRef);
		prds->OriginalKeySetRef=0;
		PGPcomdlgCloseGroupFile(Prefs->Group);
		Prefs->Group=0;

		ProcessMessages();

// Start all over again.. making sure we can open the 
// rings
		do
		{
			err=PGPOpenDefaultKeyRings( prds->Context,
				OPENKEYRINGFLAGS,&prds->OriginalKeySetRef);

			if(IsPGPError(err))
			{
				int MsgRet;

				MsgRet=MessageBox(hdlg,
					"Can't reopen the keyrings. Try again?",
					"SDK Error",
					MB_YESNO|MB_SETFOREGROUND|MB_ICONSTOP);

				if(MsgRet==IDNO)
				{
					EndDialog(hdlg, FALSE);
					return;
				}
			}
		} while(IsPGPError(err));

		ProcessMessages();

/*		// PGP Freeware does not support groups in recipient dialog
#if PGP_FREEWARE
		Prefs->Group=0;
#else
*/
		PGPcomdlgOpenGroupFile(&(Prefs->Group));
//#endif

		if(!AddUsersToLists(hdlg,&(prg->gUserLinkedList),
			prds,Prefs,&dummy))
		{
			EndDialog(hdlg, FALSE);
			return;
		}

		ProcessMessages();

		AddLinkedListtoListView(prg->hwndUserIDs,prg->gUserLinkedList);

		ProcessMessages();

		MoveMarkedItems(prg->hwndUserIDs,
			prg->hwndRecipients);
		FreeReloadArray(prds);
		prds->dwNumRecipients=tempNumRec;
		prds->szRecipientArray=tempRecArray;

		SetWindowText(hdlg,WinText);
		prg->Reloading=FALSE;
	}
	while(prg->Pending);
}

#define RECRELAY "PGPrecipHiddenRelayWindow"

LRESULT CALLBACK RecRelayWndProc (HWND hwnd, UINT iMsg, 
								WPARAM wParam, LPARAM lParam)
{
	if(iMsg==PGPM_RELOADKEYRING)
	{
		HWND hwndRec;

		hwndRec =(HWND)GetWindowLong(hwnd,GWL_USERDATA);

		SendMessage(hwndRec,PGPM_RELOADKEYRING,0,0);
		return 0;
	}

    switch (iMsg)
    {
		case WM_CREATE :
		{
			HWND hwndRec;
			LPCREATESTRUCT lpcs;
				
			lpcs = (LPCREATESTRUCT) lParam;

			hwndRec=(HWND)lpcs->lpCreateParams;

			SetWindowLong(hwnd,GWL_USERDATA,(long)hwndRec);
            return 0 ;
		}
	}
 
    return DefWindowProc (hwnd, iMsg, wParam, lParam) ;
}

HWND RecRelayWindow(HWND hwndRec)
{
	HWND hwnd;
	WNDCLASS wndclass; 

	wndclass.style			= 0;
	wndclass.lpfnWndProc	= (WNDPROC)RecRelayWndProc;
	wndclass.cbClsExtra		= 0;
	wndclass.cbWndExtra		= 0;
	wndclass.hInstance		= g_hinst;
	wndclass.hIcon			= 0;
	wndclass.hCursor		= 0;
	wndclass.hbrBackground	= 0;
	wndclass.lpszMenuName	= 0;
	wndclass.lpszClassName	= RECRELAY;

	RegisterClass(&wndclass);

	hwnd = CreateWindow(RECRELAY, 
		RECRELAY, WS_OVERLAPPEDWINDOW, 
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, NULL, NULL, g_hinst, hwndRec );

	ShowWindow(hwnd, SW_HIDE);
	UpdateWindow(hwnd);

	return hwnd;
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
			char MsgTxt[256];
			PRECGBL prg;
			BOOL AllowConv,DisableConv;
			BOOL bConv,bWipe,bText;

            InitRecGbl(hdlg,(PRECGBL)lParam);

            InitCommonControls();
            // need to save away the pointer to our structure...

            CreateDrawElements(&(RecGbl(hdlg)->ds));
            // save some hwnds

			prg=RecGbl(hdlg);

			prg->hwndRelay=RecRelayWindow(hdlg);

			prg->hwndOptionASCII=GetDlgItem(hdlg, IDC_CHECK_ASCII);

			InitList(hdlg,IDC_USER_ID_LIST,
				&(RecGbl(hdlg)->lsUser),
				UserColText,
				ColRatio);
			InitList(hdlg,IDC_RECIPIENT_LIST,
				&(RecGbl(hdlg)->lsRec),
				RecColText,
				ColRatio);

			prg->hwndUserIDs = RecGbl(hdlg)->lsUser.hwndlist;
            prg->hwndRecipients = RecGbl(hdlg)->lsRec.hwndlist;
      
            origListBoxProc=
                SubclassWindow(prg->hwndUserIDs,
                               MyListviewWndProc);  
            origListBoxProc=
                SubclassWindow(prg->hwndRecipients,
                               MyListviewWndProc);  
        
            if(prg->prds->szTitle)
            {
                SetWindowText(hdlg, prg->prds->szTitle);
            }
            // else they did not provide a title so keep default

            Button_SetCheck(GetDlgItem(hdlg, IDC_CHECK_ASCII),
                (BOOL)(prg->prds->dwOptions & 
				PGPCOMDLG_ASCIIARMOR));
            Button_SetCheck(GetDlgItem(hdlg, IDC_PASSONLY),
                (BOOL)(prg->prds->dwOptions & 
				PGPCOMDLG_PASSONLY));
            Button_SetCheck(GetDlgItem(hdlg, IDC_WIPEORIG),
                (BOOL)(prg->prds->dwOptions & 
				PGPCOMDLG_WIPEORIG));

			DisableConv=(BOOL)
				(prg->prds->dwDisableFlags & PGPCOMDLG_DISABLE_PASSONLY);
			AllowConv=prg->Prefs->bAllowConventionalEncryption;

			bConv=!(DisableConv||!AllowConv);
			bWipe=!(BOOL)(prg->prds->dwDisableFlags & 
				PGPCOMDLG_DISABLE_WIPEORIG);
			bText=!(BOOL)(prg->prds->dwDisableFlags & 
				PGPCOMDLG_DISABLE_ASCIIARMOR);
/*
#if PGP_FREEWARE
			bWipe=FALSE;
#endif
*/
            EnableWindow(GetDlgItem(hdlg, IDC_CHECK_ASCII),
				bText);
            EnableWindow(GetDlgItem(hdlg, IDC_PASSONLY),
				bConv);
            EnableWindow(GetDlgItem(hdlg, IDC_WIPEORIG),
                bWipe);
            EnableWindow(GetDlgItem(hdlg, IDC_STATIC_OPTIONS_GROUP), 
				(bText || bConv || bWipe));

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
 
            AddLinkedListtoListView(prg->hwndUserIDs,
                prg->gUserLinkedList);

			MoveMarkedItems(prg->hwndUserIDs,
				prg->hwndRecipients);
 
            ResizeEm(hdlg);

              GetWindowRect (hdlg, &rc);
            SetWindowPos (hdlg, NULL,
                (GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
                (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2,
                0, 0, SWP_NOSIZE | SWP_NOZORDER);

			 // Go to keyserver for not founds
            if(prg->Prefs->bSynchNotFound)   
            {
                ShowWindow(hdlg,SW_SHOW);
                SetForegroundWindow(hdlg);
                LookUpNotFoundKeys(hdlg,
					prg->prds->Context,
					prg->prds->OriginalKeySetRef);
            }
			else
				SetForegroundWindow(hdlg);
       

            return TRUE;
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

#endif // !LISTBOX

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
            char szHelpFile[MAX_PATH+1];

            GetHelpFile(szHelpFile);

            WinHelp (((LPHELPINFO) lParam)->hItemHandle, szHelpFile, 
                  HELP_WM_HELP, (DWORD) (LPSTR) aIds); 
            break;
        }

		// Note, ListView's context are done through subclass
        case WM_CONTEXTMENU: 
		{
			char szHelpFile[MAX_PATH+1];

			GetHelpFile(szHelpFile);

			WinHelp ((HWND) wParam, szHelpFile, HELP_CONTEXTMENU, 
				(DWORD) (LPVOID) aIds); 
			break;
		}

        case WM_DESTROY:
        {
			PRECGBL prg;

			prg=RecGbl(hdlg);

			DestroyWindow(prg->hwndRelay);

            DeleteDrawElements(&(prg->ds));
    
            SubclassWindow(prg->hwndRecipients,origListBoxProc);
            SubclassWindow(prg->hwndUserIDs,origListBoxProc);
            break;
        }
                
#if !LISTBOX
          
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

#endif // !LISTBOX

        case WM_COMMAND:
        {
            switch(wParam)
            {
	            case IDC_CHECK_ASCII:
                {
                    (RecGbl(hdlg))->prds->dwOptions ^= PGPCOMDLG_ASCIIARMOR;
					Button_SetCheck(GetDlgItem(hdlg, IDC_CHECK_ASCII),
						(BOOL)((RecGbl(hdlg))->prds->dwOptions & 
						PGPCOMDLG_ASCIIARMOR));
                    break;
                }

                case IDC_WIPEORIG:
                {
                    (RecGbl(hdlg))->prds->dwOptions ^= PGPCOMDLG_WIPEORIG;
					Button_SetCheck(GetDlgItem(hdlg, IDC_WIPEORIG),
						(BOOL)((RecGbl(hdlg))->prds->dwOptions & 
						PGPCOMDLG_WIPEORIG));
                    break;
                }

                case IDC_PASSONLY:
                {
                    (RecGbl(hdlg))->prds->dwOptions ^= PGPCOMDLG_PASSONLY;
					Button_SetCheck(GetDlgItem(hdlg, IDC_PASSONLY),
						(BOOL)((RecGbl(hdlg))->prds->dwOptions & 
						PGPCOMDLG_PASSONLY));
					EnableWindow(GetDlgItem(hdlg, IDC_USER_ID_LIST),
						!(BOOL)((RecGbl(hdlg))->prds->dwOptions & 
						PGPCOMDLG_PASSONLY));
					EnableWindow(GetDlgItem(hdlg, IDC_RECIPIENT_LIST),
						!(BOOL)((RecGbl(hdlg))->prds->dwOptions & 
						PGPCOMDLG_PASSONLY));
                    break;
                }
 
                case IDOK:
                {
					PRECIPIENTDIALOGSTRUCT prds;
					PGPUInt32 NumKeys;

					prds=(RecGbl(hdlg))->prds;

					if((prds->dwOptions&PGPCOMDLG_PASSONLY)==0)
					{
						int RSAandDSA;

						RSAandDSA=RecipientSetFromGUI(
							(RecGbl(hdlg))->hwndRecipients,
							prds,
							(RecGbl(hdlg))->gUserLinkedList,
							&NumKeys);

						if(NumKeys==0)
						{
							MessageBox(hdlg,
				"You must either choose at least one recipient or\n"
				"use the Conventional Encryption option.",
				"No Recipients Selected",MB_OK|MB_ICONSTOP);
							break;
						}

						if(RSAandDSA)
							if(RSADSARecWarning(hdlg)==FALSE)
							{
								PGPFreeKeySet(prds->SelectedKeySetRef);
								prds->SelectedKeySetRef=0;
								break;
							}
					}
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

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
