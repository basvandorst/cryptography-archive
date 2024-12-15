/* wvusenet.c */
/*  This file consists of the window procedure and support routines
 *  for the main window to the application WinVn.
 */

#include "windows.h"

#ifndef MAC
#include "winundoc.h"
#include <stdlib.h>
#endif

#include "WVglob.h"
#include "WinVn.h"
#ifdef MAC
#include "MRRM1.h"
#include <DialogMgr.h>
#endif

/*--- FUNCTION: WinVnConfWndProc ---------------------------------------
 *
 *    Window procedure for the "Net" window.
 *    This window displays the names of the newsgroups, one per line.
 *
 *    Entry    The usual parameters for any window function.
 *
 *    Note:    We don't do anything until "Initialized" is true.
 *             This way, the user won't try to perform functions
 *             while the communications are still being set up.
 */

long FAR PASCAL WinVnConfWndProc(hWnd, message, wParam, lParam)
HWND hWnd;
unsigned message;
WORD wParam;
LONG lParam;
{
#ifndef MAC
    FARPROC lpProcAbout;
    HMENU hMenu;
#endif
    PAINTSTRUCT ps;                              /* paint structure          */
   POINT ptCursor;

    HDC hDC;                                    /* handle to display context */
    HWND hWndView;
    HDC  hDCView;
    RECT myRect;                                  /* selection rectangle      */

    int j, idoc;
    int found;
    char far *lpsz;
    TypLine far *LinePtr, far *GroupLinePtr;
    TypBlock far *BlockPtr, far *GroupBlockPtr;
    int LinesGone;
    int SelLine;
   HANDLE hBlock;
   TypDoc *MyDoc;
   unsigned int Offset;

    int X, Y, nWidth, nHeight;
   int docnum;
   int newdoc;
    char mybuf[MAXINTERNALLINE];
    int mylen;
    int OldSel = FALSE;
    int CtrlState;
   TypLineID MyLineID;

    switch (message) {
#ifndef MAC
        case WM_SYSCOMMAND:
            if (wParam == ID_ABOUT) {
                lpProcAbout = MakeProcInstance(About, hInst);
                DialogBox(hInst, "AboutBox", hWnd, lpProcAbout);
                FreeProcInstance(lpProcAbout);
                break;
            }
            else
                return (DefWindowProc(hWnd, message, wParam, lParam));
#endif
        case WM_CREATE:
            /* Set up the timer which will wake us up every so often
             * so we can check to see if new characters have arrived from
             * the server.
             */

#ifndef MAC
            idTimer =  SetTimer(hWnd, NULL, 1000, (FARPROC) NULL);

            /* Get handle to the hourglass cursor */

            hHourGlass = LoadCursor(hInst, IDC_WAIT);

            /* Add the "About" option to the system menu.            */

            hMenu = GetSystemMenu(hWnd, FALSE);
            ChangeMenu(hMenu, NULL, NULL, NULL, MF_APPEND | MF_SEPARATOR);
            ChangeMenu(hMenu, NULL, "A&bout WinVn...", ID_ABOUT,
                MF_APPEND | MF_STRING);
#endif
            /* Fill in the Net document's window handle that we
             * were unable to fill in in the call to InitDoc.
             */
            NetDoc.hDocWnd = hWnd;
            break;

        case WM_CHAR:
            /* Hitting CR on a group name is the same as double-clicking
             * on the name.
             */
            if(!Initializing) {
               if (wParam == '\r') {
#ifndef MAC
                   GetCursorPos(&ptCursor);
                   ScreenToClient(hWnd, &ptCursor);
                   X = ptCursor.x;
                   Y = ptCursor.y;
#else
               GetMouse(&ptCursor);
               X = ptCursor.h;
               Y = ptCursor.v;
#endif
                   goto getgroup;
               }
            }
            break;

         case WM_KEYDOWN:
            /* See if this key should be mapped to a scrolling event
             * for which we have programmed the mouse.  If so,
             * construct the appropriate mouse call and call the mouse code.
             * Also, F6 means switch to next window.
             * (This latter should also be table-driven.)
             */
            if(!Initializing) {
               if(wParam == VK_F6) {
                  NextWindow(&NetDoc);
               } else {
                  /* Based on the state of the Control key and the value
                   * of the key just pressed, look up in the array
                   * key2scroll to see if we should process this keystroke
                   * as if it were a mouse event.  If so, simulate the
                   * mouse event by sending the appropriate message.
                   */
#ifndef MAC
                  CtrlState = GetKeyState(VK_CONTROL) < 0;
                  for(j=0; j<NUMKEYS; j++) {
                     if(wParam == key2scroll[j].wVirtKey &&
                      CtrlState == key2scroll[j].CtlState) {
                        SendMessage(hWnd,key2scroll[j].iMessage,
                         key2scroll[j].wRequest,0L);
                        break;
                     }
                  }
#endif
               }
            }
            break;

        case WM_LBUTTONDOWN:
          /*  Clicking the left button on a group name toggles the
           *  selected/not selected status of that group.
           *  Currently selected groups are displayed in reverse video.
           */
            if(!Initializing) {
               X = LOWORD(lParam);
               Y = HIWORD(lParam);

               if(CursorToTextLine(X,Y,&NetDoc,&BlockPtr,&LinePtr)) {
                  ( (TypGroup far *)( ((char far *)LinePtr) + sizeof(TypLine) ) )
                   ->Selected ^= TRUE;
                  GlobalUnlock(BlockPtr->hCurBlock);
               }

               InvalidateRect(hWnd, NULL, FALSE);
            }
            break;

        case WM_LBUTTONDBLCLK:
           /*  Double-clicking on a group name creates a "Group"
            *  window, whose purpose is to display the subjects
            *  of the articles in the group.
            */
            if(!Initializing) {
               X = LOWORD(lParam);
               Y = HIWORD(lParam);
            getgroup:;

               if(CursorToTextLine(X,Y,&NetDoc,&GroupBlockPtr,&GroupLinePtr)) {
                  if(MyDoc = (( (TypGroup far *)
                    ( ((char far *)GroupLinePtr)+sizeof(TypLine) ) )->SubjDoc)) {

                     /* We already have a document containing the subjects */
                     /* of this group, so just activate it.                */

                     SetActiveWindow(MyDoc->hDocWnd);
                     SetFocus(MyDoc->hDocWnd);
                     GlobalUnlock(GroupBlockPtr->hCurBlock);
                     break;
                  }
                  if(CommBusy) {
                     GlobalUnlock(GroupBlockPtr->hCurBlock);
                     MessageBox(hWnd,"Sorry, I am already busy retrieving information from the server.\n\
Try again in a little while.","Can't request info on group",MB_OK|MB_ICONASTERISK);
                     break;
                  }

                  /* At this point, we've determined that the subjects for
                   * this newsgroup are not in memory, and that it's OK
                   * to fetch them from the server.  (Comm line not busy.)
                   * Figure out whether a new window/document should be
                   * created for this group & set "newdoc" accordingly.
                   */
                  newdoc = FALSE;
                  if(ViewNew || !ActiveGroupDoc || !(ActiveGroupDoc->InUse)) {
                     found = FALSE;
                     for(docnum=0; docnum<MAXGROUPWNDS; docnum++) {
                       if(!GroupDocs[docnum].InUse) {
                           found = TRUE;
                           newdoc = TRUE;
                           CommDoc = &(GroupDocs[docnum]);
                           break;
                        }
                     }
                     if(!found) {
                        MessageBox(hWnd,"You have too many group windows \
active;\nClose one or select 'Reuse Old Window'.","Can't open new window",MB_OK|MB_ICONASTERISK);
                        UnlockLine(GroupBlockPtr,GroupLinePtr,&hBlock,&Offset,&MyLineID);
                        break;
                     }
                  } else {
                     /* Must reuse old window for this group.           */
                     CommDoc = ActiveGroupDoc;
                     UnlinkArtsInGroup(CommDoc);
                     LockLine(CommDoc->hParentBlock,CommDoc->ParentOffset,CommDoc->ParentLineID,&BlockPtr,&LinePtr);
                     ( (TypGroup far *)
                      ( ((char far *)LinePtr)+sizeof(TypLine) ) )->SubjDoc = (TypDoc *)NULL;
                     UnlockLine(BlockPtr,LinePtr,&hBlock,&Offset,&MyLineID);
                     /* clear out contents of this document for reuse */
                     FreeDoc(CommDoc);
                  }

                  /* Create window title indicating we are retrieving text. */

                  CommDoc->InUse = TRUE;
                  lpsz = (char far *) ( ((char far *)GroupLinePtr) +
                   sizeof(TypLine)+ sizeof(TypGroup) ) ;
                  strcpy(mybuf,"Retrieving ");
                  lstrcat(mybuf,lpsz);

                  /* If necessary, create a new window.              */

                  if(newdoc) {
                     hWndView = CreateWindow("WinVnView",
                     mybuf,
                     WS_OVERLAPPEDWINDOW | WS_VSCROLL  ,
                     (int) (xScreen*7 / 20)+docnum*CharWidth+1,      /* Initial X pos */
                     (int) docnum*LineHeight,
                     (int) (xScreen * 5 / 8),     /* Initial X Width */
                     (int) (yScreen * 3 / 8),  /* CW_USEDEFAULT, */
                     NULL,
                     NULL,
                     hInst,
                     NULL);

                     if (!hWndView) return (0);  /* ??? */
#ifndef MAC
                     ShowWindow(hWndView, SW_SHOWNORMAL);
#endif
                  } else {
                     hWndView = CommDoc->hDocWnd;
                     SetWindowText(hWndView,mybuf);
                  }
                  RcvLineCount = 0;
                  TimesWndUpdated = 0;

                  /* Do some housekeeping:  Set group as selected,
                   * initialize empty document to hold subject lines,
                   * set focus to this document, set pointers linking
                   * this document and the subject line.
                   */
                  ( (TypGroup far *)( ((char far *)GroupLinePtr) +
                    sizeof(TypLine) ) )->Selected = TRUE;
                  InitDoc(CommDoc,hWndView,&NetDoc,DOCTYPE_GROUP);
                  PtrToOffset(GroupBlockPtr,GroupLinePtr,&(CommDoc->hParentBlock),
                   &(CommDoc->ParentOffset),&(CommDoc->ParentLineID));
                  SetActiveWindow(hWndView);
                  SetFocus(hWndView);

                  ( (TypGroup far *)( ((char far *)GroupLinePtr) + sizeof(TypLine) ) )
                   ->SubjDoc = CommDoc;
                  GlobalUnlock(GroupBlockPtr->hCurBlock);
                  InvalidateRect(hWndView,NULL,FALSE);
#ifndef MAC
                  UpdateWindow(hWndView);
#endif
                  InvalidateRect(NetDoc.hDocWnd,NULL,FALSE);

                  /* Deal with Comm-related stuff:  set FSA variables,
                   * send the GROUP command to NNTP.
                   */

                  CommLinePtr = CommLineIn;
                  CommBusy = TRUE;
                  CommState = ST_GROUP_RESP;
                  strcpy(mybuf,"GROUP ");
                  lpsz = (char far *)GroupLinePtr + sizeof(TypLine) + sizeof(TypGroup);
                  lstrcat(mybuf,lpsz);
                  mylstrncpy(CurrentGroup,lpsz,MAXFINDSTRING);
                  /* lstrcat(mybuf,"\r"); */
                  PutCommLine(mybuf,lstrlen(mybuf));
               }
            }
            break;

        case WM_TIMER:
            DoCommInput();
            break;

        case WM_SIZE:
            /* Store the new size of the window.                     */
            GetClientRect(hWnd, &myRect);
            NetDoc.ScXWidth =  myRect.right;
            NetDoc.ScYHeight = myRect.bottom;
#ifdef MAC
            NetDoc.ScYLines = (myRect.bottom - myRect.top) / LineHeight;
#else
            NetDoc.ScYLines = (myRect.bottom - myRect.top - TopSpace) / LineHeight;
#endif
            NetDoc.ScXChars = (myRect.right - myRect.left - SideSpace) / CharWidth;
            break;

        case WM_VSCROLL:
            if(!Initializing) {
               ScrollIt(&NetDoc,wParam,lParam);
            }
            break;

            case WM_COMMAND:
            switch(wParam) {

              case IDM_QUIT:
                SaveNewsrc = FALSE;
              case IDM_EXIT:
                DestroyWindow(hWnd);
                break;

              case IDM_VIEW_SEL_GROUP:
                break;

              case IDM_SHOW_SUBSCR:
              case IDM_SHOW_ALL_GROUP:
              case IDM_SEL_SUBSCR:
              case IDM_SELECTALL:
                MessageBox(hWnd, "Command not implemented",
                  "Sorry", MB_OK);
                break;

              case IDM_UNSEL_ALL:
                ForAllLines(&NetDoc,SetLineFlag,0,FALSE);
                InvalidateRect(hWnd,NULL,FALSE);
                break;

              case IDM_SUBSCRIBE:
                InitGroupTable();
                ForAllLines(&NetDoc,GroupAction,GROUP_ACTION_SUBSCRIBE,TRUE);
                MergeGroups(ADD_SUBSCRIBED_END_OF_SUB);
                CleanUpGroupTable();
					 ForAllLines(&NetDoc,SetLineFlag,0,FALSE);
					 ScreenToTop(&NetDoc);
                InvalidateRect(hWnd,NULL,FALSE);
                break;

              case IDM_UNSUBSCRIBE:
                InitGroupTable();
                ForAllLines(&NetDoc,GroupAction,GROUP_ACTION_UNSUBSCRIBE,FALSE);
                ShellSort(NewGroupTable,nNewGroups,sizeof(void far *),GroupCompare);
                MergeGroups(ADD_SUBSCRIBED_END_OF_SUB);
                CleanUpGroupTable();
                ForAllLines(&NetDoc,SetLineFlag,0,FALSE);
					 ScreenToTop(&NetDoc);
					 InvalidateRect(hWnd,NULL,FALSE);
                break;

              case IDM_GROUP_TOP:
                InitGroupTable();
                ForAllLines(&NetDoc,GroupAction,GROUP_ACTION_SUBSCRIBE,TRUE);
                MergeGroups(ADD_SUBSCRIBED_TOP_OF_DOC);
                CleanUpGroupTable();
                ForAllLines(&NetDoc,SetLineFlag,0,FALSE);
					 ScreenToTop(&NetDoc);
					 InvalidateRect(hWnd,NULL,FALSE);
                break;

              case IDM_NEW_WIN_GROUP:
                ViewNew = !ViewNew;
                CheckView(hWnd);
                break;

              case IDM_NEW_WIN_ARTICLE:
                NewArticleWindow = !NewArticleWindow;
                CheckView(hWnd);
                break;

              case IDM_COMMOPTIONS:
#ifndef MAC
                if(DialogBox(hInst,"WinVnComm",hWnd,lpfnWinVnCommDlg)) {
                  InvalidateRect(hWnd,NULL,TRUE);
                }
#endif

                break;

               case IDM_CONFIG_PERSONAL:
#ifndef MAC
                  DialogBox(hInst,"WinVnPersonal",hWnd,lpfnWinVnPersonalInfoDlg);
#endif
                  break;

               case IDM_CONFIG_MISC:
#ifndef MAC
                  DialogBox(hInst,"WinVnMisc",hWnd,lpfnWinVnMiscDlg);
#endif
                  break;

               case IDM_RESET:
                  CommBusy = FALSE;
                  CommState = ST_NONE;
                  break;

               case IDM_FIND:
                  FindDoc = &NetDoc;
#ifndef MAC
                  if(DialogBox(hInst,"WinVnFind",hWnd,lpfnWinVnFindDlg)) {
                     found = DoFind(TRUE);
                     if(!found) {
                        strcpy(mybuf,"\"");
                        strcat(mybuf,NetDoc.SearchStr);
                        strcat(mybuf,"\" not found.");
                        MessageBox(hWnd,mybuf,"Not found",MB_OK);
                     }
                  }
#else
                  {
                  DialogPtr myDialog;
                  BOOL dialogDone = FALSE, oktofind = FALSE;
                  int itemhit;
                  int itemType;
                  Handle itemHandle;
                  Rect itemRect;
                  char myStr[255];

                  myDialog = GetNewDialog(DLOG_FIND,NULL,(WindowPtr)-1);
                  GetDItem(myDialog,3,&itemType,&itemHandle,&itemRect);
                  strcpy(myStr,FindDoc->SearchStr);
                  CtoPstr(myStr);
                  SetIText(itemHandle,myStr);
                  ShowWindow((WindowPtr)myDialog);
                  while(!dialogDone) {
                     ModalDialog(NULL,&itemhit);
                     switch(itemhit) {
                        case OK:
                           dialogDone = TRUE;
                           oktofind = TRUE;
                           break;
                        case Cancel:
                           dialogDone = TRUE;
                           break;
                     }
                  }
                  HideWindow(myDialog);
                  if(oktofind) {
                     GetIText(itemHandle,&myStr);
                     PtoCstr(myStr);
                     strcpy(FindDoc->SearchStr,(char *)myStr);
                     found = DoFind(TRUE);
                     if(!found) {
                        strcpy(mybuf,"\"");
                        strcat(mybuf,NetDoc.SearchStr);
                        strcat(mybuf,"\" not found.");
                        MessageBox(hWnd,mybuf,"Not found",MB_OK);
                     }
                  }
                  }
#endif
                  break;

               case IDM_FIND_NEXT_SAME:
                  FindDoc = &NetDoc;
                  if(strcmp(FindDoc->SearchStr,"")) {
                     found = DoFind(FALSE);
                     if(!found) {
                        strcpy(mybuf,"\"");
                        strcat(mybuf,NetDoc.SearchStr);
                        strcat(mybuf,"\" not found.");
                        MessageBox(hWnd,mybuf,"No more occurrences",MB_OK);
                     }
                  }
                  break;
#ifndef MAC
               case ID_ABOUT:
                  lpProcAbout = MakeProcInstance(About, hInst);
                  DialogBox(hInst, "AboutBox", hWnd, lpProcAbout);
                  FreeProcInstance(lpProcAbout);
                  break;

               case IDM_HELP:
                  MakeHelpPathName(mybuf,MAXINTERNALLINE);
                  WinHelp(hWndConf,mybuf,HELP_INDEX,0L);
                  break;
#endif

               case IDM_MAIL:
                  CreatePostingWnd((TypDoc *)NULL,DOCTYPE_MAIL);
                  break;
            }
            break;

        case WM_PAINT:
            {
            HANDLE hBlock;
            unsigned int Offset, MyLen, width, indicatorwidth;
            int VertLines, HorzChars;
            int EndofDoc = FALSE;
            int RangeHigh, CurPos;
            int RestX,Xtext;
            char far *textptr;
            char *cptr, *cptr2, *indcptr;
            char init_msg[60];
            TypGroup far *MyGroup;
            HANDLE hBlackBrush;
            DWORD MyColors[4], MyBack[4];
            DWORD Rop;
            RECT myRect;
            POINT myPoint;
            int MyColorMask = 1, PrevColorMask = MyColorMask;
#define SUBSCR_MASK 1
#define SELECT_MASK 2

            hDC = BeginPaint (hWnd, &ps);
            GetClientRect(hWnd, &myRect);
            SelectObject(hDC,hFont);

            /* If still initializing comm link, put out a message    */
            /* to that effect.                                       */

            switch(Initializing) {
             case INIT_ESTAB_CONN:
               cptr = "Establishing link";
               cptr2 = "to news server...";

               goto display_msg;

             case INIT_READING_NEWSRC:
               cptr = "Reading your log";
               cptr2 = "(\"newsrc\")...";
               goto display_msg;

             case INIT_SCANNING_NETDOC:
               cptr = "Creating hash table";
               cptr2 = "from current groups...";
               goto display_msg;

             case INIT_GETTING_LIST:
               sprintf(mybuf,"Receiving %uth newsgroup",RcvLineCount);
               cptr = mybuf;
               cptr2 = "name from server...";

              display_msg:;
               X = SideSpace + CharWidth;
               Y = StartPen + 2*LineHeight;
               strcpy(init_msg,cptr);
               strcat(init_msg,"    ");
               j = strlen(init_msg);
               TextOut(hDC,X,Y,init_msg,j);
               strcpy(init_msg,cptr2);
               strcat(init_msg,"    ");
               j = strlen(init_msg);
               TextOut(hDC,X,Y+LineHeight,init_msg,j);
               break;


             case INIT_DONE:

               VertLines = NetDoc.ScYLines;
               HorzChars = NetDoc.ScXChars;
#ifndef MAC
               MyColors[0] = NetUnSubscribedColor;
               MyColors[1] = GetTextColor(hDC);
               MyColors[2] = MyColors[0];
               MyColors[3] = GetBkColor(hDC);

               MyBack[0] = MyColors[3];
               MyBack[1] = MyBack[0];
               MyBack[2] = MyColors[1];
               MyBack[3] = MyBack[2];

               indicatorwidth = LOWORD(GetTextExtent(hDC,"*",2)) * 7 / 7;
#endif
               LockLine(NetDoc.hCurTopScBlock,NetDoc.TopScOffset,NetDoc.TopScLineID,
                &BlockPtr,&LinePtr);

               /* Update the scroll bar thumb position.                 */

               CurPos = NetDoc.TopLineOrd;
               if(CurPos<0) CurPos = 0;
               RangeHigh = NetDoc.TotalLines - VertLines;
               if(RangeHigh<0) RangeHigh = 0;
               SetScrollRange(hWnd,SB_VERT,0,RangeHigh,FALSE);
               SetScrollPos  (hWnd,SB_VERT,CurPos,TRUE);
#ifdef MAC
               myRect.right = NetDoc.DocClipRect.right;
               myRect.top = 0;
               myRect.bottom = LineHeight;
#endif

               /* Loop through the lines, painting them on the screen. */

               X = SideSpace;
               Xtext = X + indicatorwidth;
               Y = StartPen;
               if(LinePtr->length != END_OF_BLOCK)
               do {
                  MyGroup =  ((TypGroup far *)
                   ((char far *)LinePtr + sizeof(TypLine)));
                  MyLen = MyGroup->NameLen;
                  textptr = (char far *) LinePtr + sizeof(TypLine) +
                   sizeof(TypGroup);

                  /* Figure out the color of this line.                 */

                  if(MyGroup->Subscribed) {
                     MyColorMask |= SUBSCR_MASK;
                  } else {
                     MyColorMask &= (0xff - SUBSCR_MASK);
                  }
                  if(MyGroup->Selected) {
                     MyColorMask |= SELECT_MASK;
                     Rop = BLACKNESS;
                  } else {
                     MyColorMask &= 0xff - SELECT_MASK;
                     Rop = WHITENESS;
                  }
                  if(MyColorMask != PrevColorMask) {
                     SetTextColor(hDC,MyColors[MyColorMask]);
                     SetBkColor(hDC,MyBack[MyColorMask]);
                     PrevColorMask = MyColorMask;
                  }

                  /* Figure out what indicator character to use. */

                  indcptr = "    ";
                  if(NetDoc.FindLineID == LinePtr->LineID) {
                     indcptr = ">   ";
                  } else if(MyGroup->HighestPrevSeen) {
                     if(MyGroup->ServerLast > MyGroup->HighestPrevSeen) {
                        indcptr = "*   ";
                     }
                  }

                  /* Now write out the line.                            */

                  TextOut(hDC,X,Y,indcptr,4);
                  width = LOWORD(GetTextExtent(hDC,textptr,MyLen));
                  TextOut(hDC,Xtext,Y,textptr,MyLen);
#if 0
                  if(MyLen < HorzChars) {
#endif
#ifdef MAC
                     GetPen(&myPoint);
                     myRect.left = myPoint.h;
                     FillRect(&myRect,white);

                     myRect.top += LineHeight;
                     myRect.bottom += LineHeight;
#else
                     RestX = Xtext + width;
                     PatBlt(hDC,RestX,Y,myRect.right-RestX,LineHeight,Rop);
#endif
#if 0
                  }
#endif
                  Y += LineHeight;
               } while(--VertLines>0 && NextLine(&BlockPtr,&LinePtr) );

               SetTextColor(hDC,MyColors[1]);
               SetBkColor(hDC,MyBack[1]);

               /* Blank out bottom and top of screen */
               PatBlt(hDC,0,Y,myRect.right,myRect.bottom-Y,PATCOPY);
               PatBlt(hDC,0,0,myRect.right,TopSpace,PATCOPY);

               UnlockLine(BlockPtr,LinePtr,&hBlock,&Offset,&MyLineID);
            }
            EndPaint(hWnd, &ps);
            break;
            }

        case WM_ENDSESSION:
#ifndef MAC
            WinHelp(hWndConf,LFN_HELP,HELP_QUIT,0L);
            CloseComm(CommDevice);
#endif
            break;

        case WM_DESTROY:
            for(idoc=0; idoc<MAXGROUPWNDS; idoc++) {
               if(GroupDocs[idoc].InUse) {
                  UpdateSeenArts(&(GroupDocs[idoc]));
               }
            }
            if(SaveNewsrc) {
               WriteNewsrc();
            }
            MRRCloseComm();
#if 0
            SendMessage(hDosWnd,WM_KEYDOWN,0x5a,0x2c0001L);
            SendMessage(hDosWnd,WM_CHAR,   0x1a,0x2c0001L);
            SendMessage(hDosWnd,WM_KEYDOWN,0xd,0x1c0001L);
            SendMessage(hDosWnd,WM_CHAR,   0xd,0x1c0001L);
#endif
            PostQuitMessage(0);
            break;

        default:
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (0);
}

#ifndef MAC
/*---  FUNCTION: About ---------------------------------------------------
 *
 *  Process messages for "About" dialog box
 */

BOOL FAR PASCAL About(hDlg, message, wParam, lParam)
HWND hDlg;
unsigned message;
WORD wParam;
LONG lParam;
{
    switch (message) {
        case WM_INITDIALOG:
            return (TRUE);

        case WM_COMMAND:
            if (wParam == IDOK) {
                EndDialog(hDlg, TRUE);
                return (TRUE);
            }
            break;
    }
    return (FALSE);
}
#endif

/* Function CheckView -----------------------------
 *
 *  This function handles checking and unchecking menu items.
 *
 *    Entry    hWnd     is the window whose menu is to be altered.
 *                      Currently works only for the Net window.
 *
 *    Exit     The appropriate items on the menu have been checked or
 *             unchecked.
 */

VOID CheckView(hWnd)
HWND hWnd;
{
   WORD MenuItemToCheck, MenuItemToUncheck;
   HMENU hMenu;
   int CheckIt;

   if(ViewNew) {
      CheckIt = MF_CHECKED | MF_BYCOMMAND;
   } else {
      CheckIt = MF_UNCHECKED | MF_BYCOMMAND;
   }
#ifdef MAC
   hMenu = gOptionsMenu;
#else
   hMenu = GetMenu(hWnd);
#endif
   CheckMenuItem(hMenu,IDM_NEW_WIN_GROUP,CheckIt);

   if(NewArticleWindow) {
      CheckIt = MF_CHECKED | MF_BYCOMMAND;
   } else {
      CheckIt = MF_UNCHECKED | MF_BYCOMMAND;
   }
   CheckMenuItem(hMenu,IDM_NEW_WIN_ARTICLE,CheckIt);
}

/* --- function CrackGroupLine --------------------------------------
 *
 *  Given a line from a .newsrc file, create a text line containing
 *  a structure of type TypGroup containing the same information.
 *  The line is zero-terminated upon input.
 *
 *    Entry    buf      points to a zero-terminated line from .newsrc.
 *             lineptr  points to a place to put the converted textline.
 *
 *    Exit     The line pointed to by "lineptr" now is a TypLine textline
 *             containing a structure of type TypGroup, containing the
 *             information in the input line from .newsrc.
 *
 *  Returns TRUE iff group was subscribed to.
 */
BOOL
CrackGroupLine(buf,lineptr)
char *buf;
TypLine *lineptr;
{
   char *grname = (char *) lineptr + sizeof(TypLine) + sizeof(TypGroup);
   TypGroup *group = (TypGroup *) ((char *) lineptr + sizeof(TypLine));
   TypRange *RangePtr;
   BOOL MoreNums;
   unsigned int MyLength;

   group->Subscribed = FALSE;
   group->Selected = FALSE;
   group->NameLen = 0;
   group->SubjDoc = (TypDoc *)NULL;
   group->ServerEstNum = 0;
   group->ServerFirst = 0;
   group->ServerLast = 0;
   group->HighestPrevSeen = 0;
   group->nRanges = 0;

   /* Copy group name to output line.                               */

   while(*buf && *buf != ':' && *buf != '!') {
      *(grname++) = *(buf++);
      (group->NameLen)++;
   }
   *(grname++) = '\0';

   if(!(*buf)) {
      /* Ran off end of line without seeing ':' or '!'.  Error.      */
   } else {
      if(*buf == ':') {
         group->Subscribed = TRUE;
      }
      buf++;
   }

   /* Look for the highest article number previously seen, in an
    * entry of form "s" followed by a number.
    */

   while(*buf && *buf == ' ') buf++;
   if(*buf == 's') {
      buf++;
      GetNum(&buf,&(group->HighestPrevSeen));
   }

   /* Convert the article number ranges to the internal numeric
    * form we use in WinVN.
    */

   RangePtr = (TypRange *) ((char *) lineptr + sizeof(TypLine) +
    RangeOffset(group->NameLen));

   RangePtr->Last = RangePtr->First = 0;

   MoreNums = TRUE;
   while(MoreNums) {
      while(*buf && (*buf == ' ' || *buf == ',') ) buf++;
      if(GetNum(&buf,&(RangePtr->First))) {
         /* We have the first number in a range.                     */
         (group->nRanges)++;
         RangePtr->Last = RangePtr->First;
         if(*buf == '-') {
            buf++;
            if(GetNum(&buf,&(RangePtr->Last))) {
               RangePtr++;
            /* at this point, we are positioned just after a range */
            } else {
               RangePtr->Last = RangePtr->First;
               MoreNums = FALSE;
            }
         } else if(*buf == ',') {
            /* We have a single number "n"; interpret as the range "n-n".
             */
            RangePtr++;
         } else {
            /* That must have been the last number.                  */
            MoreNums = FALSE;
         }
      } else {
         MoreNums = FALSE;
      }
   }
   if(group->nRanges == 0) (group->nRanges)++;

   MyLength = sizeof(TypLine) + RangeOffset(group->NameLen) +
    sizeof(TypRange)*(group->nRanges) + sizeof(int);

   lineptr->length = MyLength;
   lineptr->LineID = NextLineID++;
   *(int *) ( (char *)lineptr + MyLength - sizeof(int) ) = MyLength;

   return(group->Subscribed);
}

/*-- function CursorToTextLine ----------------------------------------
 *
 *   Routine to locate a text line in a document, based on the
 *   cursor position.  Used to figure out which line is being selected
 *   when a user clicks a mouse button.
 *
 *   Entry    X, Y    are the position of the cursor.
 *            DocPtr  points to the current document.
 *
 *   Exit     *LinePtr points to the current line, if one was found.
 *            *BlockPtr points to the current block, if found.
 *            Function returns TRUE iff a line was found that corresponds
 *              to the cursor position.
 */
BOOL
CursorToTextLine(X,Y,DocPtr,BlockPtr,LinePtr)
int X;
int Y;
TypDoc *DocPtr;
TypBlock far **BlockPtr;
TypLine far **LinePtr;
{
   int found;
   int SelLine;

   if(Y < TopSpace || Y > TopSpace + DocPtr->ScYLines*LineHeight ||
    X < SideSpace) {
      /* Cursor is in no-man's-land at edge of window.               */
      found = FALSE;
   } else {
      found = TRUE;
      SelLine = (Y - TopSpace) / LineHeight;

      LockLine(DocPtr->hCurTopScBlock,DocPtr->TopScOffset,DocPtr->TopScLineID,
       BlockPtr,LinePtr);

      for(found=TRUE,il=0; il<SelLine; il++) {
         if(!NextLine(BlockPtr,LinePtr)) {
            found = FALSE;     /* ran off end of document */
            break;
         }
      }
   }
   return(found);
}

/*-- function ReadNewsrc ----------------------------------------------
 *
 *    Reads NEWSRC into the Net document.
 *    This routine opens NEWSRC, reads & parses the lines into the NetDoc
 *    document, and closes the file.  One call does it all.
 *
 *    Entry    The NetDoc document is assumed to be initialized.
 *
 *    Exit     Returns TRUE if all went well, else zero.
 */
int
ReadNewsrc()
{
   TypBlock far *BlockPtr;
   TypLine *LocalLinePtr, far *GroupPtr;
   HANDLE hLine;
   HANDLE hBlock;
   HANDLE hRetCode;
   unsigned int Offset;
   TypLineID MyLineID;
   char mes[60];
   char far *chptr;
   int mylen,j;
#define  TEMPBUFSIZE   1240
   char mybuf[TEMPBUFSIZE];
   char *grname;
   TypMRRFile *MRRFile;
   int returned;


   LockLine(NetDoc.hCurAddBlock,NetDoc.AddOffset,NetDoc.AddLineID,&BlockPtr,&GroupPtr);
   NetDoc.hDocWnd = hWndConf;

   hLine = LocalAlloc(LMEM_MOVEABLE,TEMPBUFSIZE );
   LocalLinePtr = (TypLine *) LocalLock(hLine);

   strcpy(mybuf,"newsrc");
   hRetCode = MRROpenFile(mybuf,0,OF_READ,&MRRFile);
   if((int)hRetCode <= 0) {
      return FALSE;
#if 0
      sprintf(mes,"Unable to open the NEWSRC file; code=%d",(int)hRetCode);
      MessageBox(hWndConf,mes,mybuf,MB_OK);
#endif
   } else {
      /* Loop to read lines, convert them to internal format, and
       * insert them into the NetDoc document.
       */

      while((returned=MRRReadLine(MRRFile,mybuf,TEMPBUFSIZE)) > (-1)) {
         mybuf[returned] = '\0';
         if(CrackGroupLine(mybuf,LocalLinePtr)) {
            NetDoc.ActiveLines++;
         }
         AddLine(LocalLinePtr,&BlockPtr,&GroupPtr);
      }
      MRRCloseFile(MRRFile);
   }

   /* Change the title of the Net document.  I suppose that,
    * strictly speaking, this oughtn't be done in this routine.
    */
   SetNetDocTitle();
   UnlockLine(BlockPtr,GroupPtr,&hBlock,&Offset,&MyLineID);

   NetDoc.hCurTopScBlock = NetDoc.hFirstBlock;
   NetDoc.TopScOffset = sizeof(TypBlock);
   NetDoc.TopScLineID = 0L;

   return(TRUE);
}

char *ltoa();

/*--- function WriteNewsrc ---------------------------------------------
 *
 *  Write out a NEWSRC file, based on the information in the
 *  NetDoc document.  Use the standard Unix "rn" format for .newsrc.
 *
 *    Entry    no parameters
 *             NetDoc   has the group information.
 *
 *    Exit     The NEWSRC file has been written.
 */
void
WriteNewsrc()
{
   TypBlock far *BlockPtr;
   TypLine  far *LinePtr;
   HANDLE hLine;
   HANDLE hBlock;
   unsigned int Offset;
   TypLineID MyLineID;
   char mes[60], mybuf[60];
   char far *fromptr;
   char *toptr;
   char *NewsLine;
   HANDLE hRetCode;
   TypMRRFile *MRRFile;
   int returned;
   int nranges, nchars;
   TypGroup far *Group;
   TypRange far *RangePtr;
   BOOL firstrange;

   LockLine(NetDoc.hFirstBlock,sizeof(TypBlock),0L,&BlockPtr,&LinePtr);

   hLine = LocalAlloc(LMEM_MOVEABLE,BLOCK_SIZE);
   NewsLine = (char *) LocalLock(hLine);

   strcpy(mybuf,"newsrc");
   hRetCode = MRROpenFile(mybuf,0,OF_CREATE,&MRRFile);
   if((int)hRetCode < 0) {
      sprintf(mes,"MRROpenFile returned %d",hRetCode);
      MessageBox(hWndConf,mes,mybuf,MB_OK);
   } else {
      do {
         toptr = NewsLine;
         Group = (TypGroup far *) ((char far *)LinePtr + sizeof(TypLine));

         /* Copy group name                                          */
         fromptr = (char far *)LinePtr + sizeof(TypLine) + sizeof(TypGroup);
         while(*(toptr++) = *(fromptr++));
         toptr--;

         /* Affix : or ! depending upon whether subscribed.          */
         *(toptr++) = (char) (Group->Subscribed ? ':' : '!');
         *(toptr++) = ' ';

         /* If we know the highest article number on the server,
          * output it preceded by an "s".
          */
         if(Group->ServerLast) {
            *(toptr++) = 's';
            ltoa((unsigned long)Group->ServerLast,toptr,10);
            while(*toptr) toptr++;
            *(toptr++) = ' ';
         }

         /* Affix ranges of articles read.                          */
         firstrange = TRUE;
         nranges = Group->nRanges;
         RangePtr = (TypRange far *) ((char far *)Group + RangeOffset(Group->NameLen));

         while(nranges--) {
            /* Write out ',' if not first range of articles.         */

            if(!firstrange) {
               *(toptr++) = ',';
            } else {
               firstrange = FALSE;
            }
            /* Write out first article in a range.                   */

            ltoa((unsigned long)RangePtr->First,toptr,10);
            while(*toptr) toptr++;

            /* If the range is of form "n-n", just put out "n"       */

            if(RangePtr->First != RangePtr->Last) {
               /* Put out the hyphen in middle of range.                */
               *(toptr++) = '-';
               /* Put out the last article in a range.                  */
               ltoa((unsigned long)RangePtr->Last,toptr,10);
               while(*toptr) toptr++;
            }
            RangePtr++;
         }
         MRRWriteLine(MRRFile,NewsLine,toptr-NewsLine);

      } while(NextLine(&BlockPtr,&LinePtr));
      UnlockLine(BlockPtr,LinePtr,&hBlock,&Offset,&MyLineID);
      MRRCloseFile(MRRFile);
   }
}

/*--- function SetNetDocTitle -------------------------------------------
 *
 */
void
SetNetDocTitle()
{
   char mybuf[120];

   sprintf(mybuf,"WinVN:  %d groups; %d subscribed",NetDoc.TotalLines,
    NetDoc.ActiveLines);
   SetWindowText(hWndConf,mybuf);

}

/*--- function SetLineFlag --------------------------------------------
 *
 *  Set some flag in a line in a document.
 *
 *  Entry   Doc      points to the document.
 *          LinePtr  points to th line.
 *
 *  Exit    lFlag    says what to do.
 */
void
SetLineFlag(TypDoc *Doc, TypBlock far **BlockPtr, TypLine far **LinePtr, int wFlag, int wValue)
{
   switch(wFlag) {
      case LINE_FLAG_SET:
         ( (TypGroup far *)( ((char far *)*LinePtr) + sizeof(TypLine) ) )
            ->Selected = wValue;
         break;
   }
}


/*--- function GroupAction --------------------------------------------
 *
 *  Perform some action on a group that is specified by a pointer
 *  to a line in the Net document.
 *  Typically called for each line in the Net document by
 *  ForAllLines.
 *
 *  Entry   Doc      points to NetDoc
 *          LinePtr  points to a line in that document.
 *          lFlag    indicates what to do with that line.
 */
void
GroupAction(TypDoc *Doc, TypBlock far **BlockPtr, TypLine far **LinePtr, int wFlag, int wValue)
{

   switch(wFlag) {
      case GROUP_ACTION_SUBSCRIBE:
      case GROUP_ACTION_UNSUBSCRIBE:
         if(((TypGroup far *)( ((char far *)*LinePtr) + sizeof(TypLine)) )
                   ->Selected) {
            ((TypGroup far *)( ((char far *)*LinePtr) + sizeof(TypLine)) )
                   ->Subscribed = wValue;
            AddGroupToTable((char far *)*LinePtr);
            DeleteLine(BlockPtr,LinePtr);
         }
         break;

   }
}

/****************************************************************************

   FUNCTION:   MakeHelpPathName

   PURPOSE:    HelpEx assumes that the .HLP help file is in the same
               directory as the HelpEx executable.This function derives
               the full path name of the help file from the path of the
               executable.

   Taken from HELPEX.C, from the MS Windows SDK.

****************************************************************************/

void MakeHelpPathName(szFileName,maxchars)
char * szFileName;
int maxchars;
{
   char *  pcFileName;
   int     nFileNameLen;

   nFileNameLen = GetModuleFileName(hInst,szFileName,maxchars);
   pcFileName = szFileName + nFileNameLen;

   while (pcFileName > szFileName) {
       if (*pcFileName == '\\' || *pcFileName == ':') {
           *(++pcFileName) = '\0';
           break;
       }
   nFileNameLen--;
   pcFileName--;
   }

   if ((nFileNameLen+13) < maxchars) {
       lstrcat(szFileName, LFN_HELP);
   }

   else {
       lstrcat(szFileName, "?");
   }

   return;
}


/* Last line of WVUSENET.C */
