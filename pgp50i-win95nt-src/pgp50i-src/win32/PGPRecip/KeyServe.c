/*
 * KeyServe.c  Routines needed to support the keyserver context menu
 *
 * All of these routines are used within the listview subclass handler, but
 * since they deal with the keyserver, they are isolated in this file for
 * clarity.
 *
 * Copyright (C) 1996, 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
#include "precomp.h"
#include "../pgpkeyserver/winids.h"

/*
This context menu code replaces the normal help context menu code in the
main dialog loop.

case WM_CONTEXTMENU:
    if(wParam == (WPARAM) hwndRecipients)
    {
        SendMessage(hwndRecipients, WM_PGP_DO_CONTEXT_MENU, wParam, lParam);
    }
    else
    {
         WinHelp ((HWND) wParam, g_szHelpFile, HELP_CONTEXTMENU,
                  (DWORD) (LPVOID) aIds);
    }
    break;

case WM_PGP_GET_LEFT_HELP:
    WinHelp(hdlg, g_szHelpFile, HELP_CONTEXTPOPUP, IDH_IDC_RECIPIENT_LIST);
    break;
*/

void GetSelectedItems(HWND hwnd, char **pszBuffer)
{
    LV_ITEM lvi;
    int iCount, i;
    UINT uState;
    PUSERKEYINFO pui = NULL;

    assert(pszBuffer);
    *pszBuffer = NULL;

    if( !ListView_GetSelectedCount( hwnd ) )
    {
         return; //  Exit if not
    }

    //  How many items are there?
    iCount = ListView_GetItemCount( hwnd );

    for( i = 0; i < iCount; i++ )          //  Loop through each item
    {
         uState = ListView_GetItemState( hwnd, i, LVIS_SELECTED );

         if( uState ) //  If this item is selected we wanna move it
         {
             lvi.mask = LVIF_PARAM;
             lvi.iItem         = i;
             lvi.iSubItem    = 0;

             if(ListView_GetItem( hwnd, &lvi ))
             {
                  pui = (PUSERKEYINFO)lvi.lParam;
                  if(!*pszBuffer)
                  {
                      if((*pszBuffer =
                          (char *) pgpMemAlloc((strlen(pui->UserId) + 1) *
                          sizeof(char))))
                      {
                          strcpy(*pszBuffer, pui->UserId);
                      }
                  }
                  else
                  {
                      if((*pszBuffer = (char *)
                              pgpMemRealloc(*pszBuffer,
                                           (strlen(pui->UserId) +
                                            strlen(*pszBuffer) + 2) *
                                            sizeof(char))))
                      {
                          strcat(*pszBuffer, "\n");
                          strcat(*pszBuffer, pui->UserId);
                      }
                  }
              }
          }
     }
}

/* Add this routine to the listview Proc message handler */

void ListviewKeyserverRoutine(HWND hwnd, UINT msg,
                              WPARAM wParam, LPARAM lParam)
{
    HWND hdlg;

    hdlg=GetParent(hwnd);

    switch(msg)
    {
         case WM_PGP_DO_CONTEXT_MENU:
         {
             if(!(RecGbl(hdlg))->hPopup)
             {
                 if(((RecGbl(hdlg))->hPopup = CreatePopupMenu()))
                 {
                     if(BuildMenu(KS_NO_SEND_KEYS, (RecGbl(hdlg))->hPopup,
                        0xFFFFFFFF) == SUCCESS)
                     {
                         InsertMenu((RecGbl(hdlg))->hPopup,
                                    0xFFFFFFFF,
                                    MF_BYPOSITION | MF_SEPARATOR,
                                    0,
                                    "");
                }
                InsertMenu((RecGbl(hdlg))->hPopup,
                           0xFFFFFFFF,
                           MF_BYPOSITION | MF_STRING,
                           ID_GET_LEFT_HELP,
                           "What's This?");
                }
            }

            if((RecGbl(hdlg))->hPopup)
            {
                TrackPopupMenu((RecGbl(hdlg))->hPopup,
                               TPM_CENTERALIGN | TPM_LEFTBUTTON |
                               TPM_RIGHTBUTTON,
                               LOWORD(lParam),
                               HIWORD(lParam),
                               0,
                               hwnd,
                               NULL);
            }
            break;
        }

        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case ID_GET_LEFT_HELP:
                {
                    HWND hwndParent;

                    if((hwndParent = GetParent(hwnd)))
                    {
                        SendMessage(hwndParent, WM_PGP_GET_LEFT_HELP,
                                    0, (LPARAM)hwnd);
                    }
                    break;
                }

                default:
                {
                    HWND hwndRecpts = hwnd;
                    BOOL UpdatedKeys = FALSE;
                    DWORD dwNumRecipients = 0;
                    char *szRecipients = NULL;

                    GetSelectedItems(hwndRecpts, &szRecipients);

                    ProcessKeyServerMessage(hwnd,
                                            LOWORD(wParam),
                                            szRecipients,
                                            &UpdatedKeys);

                    if(szRecipients)
                        pgpFree(szRecipients);

                    if(UpdatedKeys)
                        PGPcomdlgNotifyKeyringChanges(GetCurrentProcessId());
                break;
                }
            }
            break;
        }
    }
}

void GetNotFoundItems(HWND hdlg,char **pszBuffer)
{
      PUSERKEYINFO pui;

    pui=(RecGbl(hdlg))->gUserLinkedList;

    while(pui!=0)
    {
        if(pui->Flags&PUIF_NOTFOUND)
        {
            if(!*pszBuffer)
            {
                    if((*pszBuffer =
                        (char *) pgpMemAlloc((strlen(pui->UserId) + 1) *
                                             sizeof(char))))
                    {
                        strcpy(*pszBuffer, pui->UserId);
                    }
            }
            else
            {
                    if((*pszBuffer = (char *)
                            pgpMemRealloc(*pszBuffer,
                                          (strlen(pui->UserId) +
                                          strlen(*pszBuffer) + 2) *
                                          sizeof(char))))
                    {
                         strcat(*pszBuffer, "\n");
                         strcat(*pszBuffer, pui->UserId);
                    }
            }
        }
        pui=pui->next;
    }
}

int LookUpNotFoundKeys(HWND hdlg)
{
    BOOL UpdatedKeys;
    char *szRecipients;

    UpdatedKeys=FALSE;
    szRecipients=0;

    GetNotFoundItems(hdlg, &szRecipients);

    if(szRecipients!=0)
    {
         ProcessKeyServerMessage((RecGbl(hdlg))->hwndRecipients,
             ID_GET_KEY,
        szRecipients,
        &UpdatedKeys);

        if(szRecipients)
            pgpFree(szRecipients);

        if(UpdatedKeys)
            PGPcomdlgNotifyKeyringChanges(GetCurrentProcessId());
    }

    return TRUE;
}
