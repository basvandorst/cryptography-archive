/*
 * ListSub.c  List View helper routines
 *
 * These routines handle the listview subclass operations and also
 * do some of the needed operations on the linked list of keys.
 *
 * Copyright (C) 1996, 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */

#include "precomp.h"

LRESULT WINAPI MyListviewWndProc(HWND hwnd, UINT msg,
                                 WPARAM wParam, LPARAM lParam)
{
    ListviewKeyserverRoutine(hwnd,msg,wParam,lParam);

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

int AddListViewItem(HWND hWndLV,int iImage,LPSTR szText,LPARAM lParam)
{
    LV_ITEM  lvItem;

    lvItem.mask = LVIF_PARAM | LVIF_IMAGE | LVIF_TEXT;

    lvItem.lParam        = lParam;
    lvItem.iImage        = iImage;
	lvItem.pszText       = szText;
    lvItem.iSubItem      = 0;
    lvItem.iItem         = 0;

    return ListView_InsertItem ( hWndLV, &lvItem );
}


BOOL AddListViewColumn(HWND hwnd,char **ColText,int *ColWidth,int num)
{
    LV_COLUMN lvc;
    RECT rect;
    BOOL ReturnValue = TRUE;
    int scrollwidth = GetSystemMetrics(SM_CXVSCROLL);
    int index;

    GetClientRect(hwnd, &rect);

    for(index=0;index<num;index++)
    {
    lvc.mask        = LVCF_WIDTH | LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT;
    lvc.pszText     = ColText[index];
    lvc.cx          = ColWidth[index];
    lvc.iSubItem    = index;
    if(index==0)
        lvc.fmt     = LVCFMT_LEFT;
    else
        lvc.fmt     = LVCFMT_CENTER;

    ListView_InsertColumn(hwnd, index, &lvc);
    }

    return ReturnValue;
}

BOOL AddImagesToImageList(HWND hdlg,HIMAGELIST *hImage)
{
    HBITMAP hBmp;

    *hImage=ImageList_Create(16,16,ILC_MASK,5,0);

    hBmp = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDB_VALRSA));

    (RecGbl(hdlg))->g_ValRSA=
         ImageList_AddMasked(*hImage, //handle to the imagelist
                             hBmp, //handle to the bitmap
                            RGB(255, 255, 255));

    DeleteObject(hBmp);

    hBmp = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDB_VALDSA));

    (RecGbl(hdlg))->g_ValDSA=
        ImageList_AddMasked(*hImage, //handle to the imagelist
                            hBmp, //handle to the bitmap
                            RGB(255, 255, 255));

    DeleteObject(hBmp);

    hBmp = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDB_UNKNOWN));

    (RecGbl(hdlg))->g_Unknown=
        ImageList_AddMasked(*hImage, //handle to the imagelist
                            hBmp, //handle to the bitmap
                            RGB(255, 255, 255));

    DeleteObject(hBmp);

    hBmp = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDB_MULTRSA));

    (RecGbl(hdlg))->g_MultRSA=
        ImageList_AddMasked(*hImage, //handle to the imagelist
                            hBmp, //handle to the bitmap
                            RGB(255, 255, 255));

    DeleteObject(hBmp);

    hBmp = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDB_MULTDSA));

    (RecGbl(hdlg))->g_MultDSA=
        ImageList_AddMasked(*hImage, //handle to the imagelist
                            hBmp, //handle to the bitmap
                            RGB(255, 255, 255));

    DeleteObject(hBmp);

    return TRUE;
}


void MakeReloadString(HWND hwnd,DWORD* pdwNumRecpts, char** pszBuffer)
{
   LV_ITEM lvi;
   int iCount, i;
   PUSERKEYINFO pui = NULL;
   char* cp = NULL;

   //  How many items are there?
   iCount = ListView_GetItemCount( hwnd );

   *pdwNumRecpts = 0;

   if(iCount)
   {
        *pszBuffer = (char*) pgpMemRealloc ( *pszBuffer,
                                              iCount * USERIDLENGTH + 2);

        if(*pszBuffer)
        {
            cp = *pszBuffer;

            *cp = 0x00;

            for( i = 0; i < iCount; i++ )  //  Loop through each item
            {
                lvi.mask = LVIF_PARAM;
                lvi.iItem         = i;
                lvi.iSubItem    = 0;

                if(ListView_GetItem( hwnd, &lvi ))
                {
                    pui = (PUSERKEYINFO)lvi.lParam;

					if((pui->Flags&PUIF_DEFAULTKEY)==0)
					{
                    strcat(cp, pui->UserId); // inefficient but easy
                    strcat(cp, "\n");

                    *pdwNumRecpts=(*pdwNumRecpts)+1;
					}
                }
            }

        }
    }
}

int WriteOutKeyIds(PRECIPIENTDIALOGSTRUCT prds,HWND hwnd)
{
    LV_ITEM lvi;
    int iCount, i;
    PUSERKEYINFO pui = NULL;
    char* cp = NULL;
    int RSA,DSA;
	DWORD* pdwNumRecpts;
    char** pszBuffer;

	pdwNumRecpts=&(prds->dwNumRecipients);
	pszBuffer=&(prds->szRecipients);

    RSA=DSA=FALSE;

    //  How many items are there?
    iCount = ListView_GetItemCount( hwnd );

    *pdwNumRecpts = 0;

    if(iCount)
    {
         *pszBuffer = (char*) pgpMemRealloc ( *pszBuffer,
                                              iCount * KEYIDLENGTH + 2);

             if(*pszBuffer)
             {
                 cp = *pszBuffer;

                 *cp = 0x02; // simple PGP -> INCLUDE_ONLYKEYIDS
                 cp++;
                 *cp = 0x00;


                 for( i = 0; i < iCount; i++ )  //  Loop through each item
                 {
                     lvi.mask = LVIF_PARAM;
                     lvi.iItem         = i;
                     lvi.iSubItem    = 0;

                     if(ListView_GetItem( hwnd, &lvi ))
                     {
                         pui = (PUSERKEYINFO)lvi.lParam;

                         if((pui->Flags&PUIF_NOTFOUND)==0)
                         {
                             if(pui->Algorithm==PGP_PKALG_RSA)
                                 RSA=TRUE;

                             if(pui->Algorithm==PGP_PKALG_DSA)
                                 DSA=TRUE;

                             if(strstr(cp,pui->KeyId)==0)
                             {
                                 strcat(cp, pui->KeyId); // inefficient but easy
                                 strcat(cp, "\n");

                                 *pdwNumRecpts=(*pdwNumRecpts)+1;
                             }
                        }
                   }
              }

         }
    }

	if(RSA)
		prds->dwPGPFlags|=PGP_ATLEASTONERSAKEY;
	else
		prds->dwPGPFlags&=~PGP_ATLEASTONERSAKEY;

    if(RSA && DSA)
        return TRUE;

    return FALSE;
}

int WriteOutLinkedListKeyIds(PRECIPIENTDIALOGSTRUCT prds,
							 PUSERKEYINFO RecipientLL)
{
     PUSERKEYINFO pui;
     int iCount;
     int RSA,DSA;
	DWORD* pdwNumRecpts;
    char** pszBuffer;

	pdwNumRecpts=&(prds->dwNumRecipients);
	pszBuffer=&(prds->szRecipients);

    RSA=DSA=FALSE;

    iCount=LengthOfList(RecipientLL);

    *pdwNumRecpts = 0;

    if(iCount)
    {
        *pszBuffer = (char*) pgpMemRealloc ( *pszBuffer,
                                             iCount * KEYIDLENGTH + 2);

        (*pszBuffer)[0]=2;
        (*pszBuffer)[1]=0;

        pui=RecipientLL;

        while(pui!=0)
        {
            if(pui->Flags&PUIF_MATCHEDWITHRECIPIENT)
            {
            // PGP_PKALG_RSA or PGP_PKALG_DSA

            if((pui->Flags&PUIF_NOTFOUND)==0)
            {
	            if(pui->Algorithm==PGP_PKALG_RSA)
		            RSA=TRUE;

			    if(pui->Algorithm==PGP_PKALG_DSA)
				    DSA=TRUE;

				if(strstr(*pszBuffer,pui->KeyId)==0)
                {
                    strcat(*pszBuffer, pui->KeyId); // inefficient but easy
                    strcat(*pszBuffer, "\n");

                    *pdwNumRecpts=(*pdwNumRecpts)+1;
                }
            }
            }
            pui=pui->next;
        }
    }

	if(RSA)
		prds->dwPGPFlags|=PGP_ATLEASTONERSAKEY;
	else
		prds->dwPGPFlags&=~PGP_ATLEASTONERSAKEY;

    if(RSA && DSA)
        return TRUE;

    return FALSE;
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
        FreeMRKLists(oldpui->mrk); // Remove the MRK links
        free(oldpui);
    }
}
