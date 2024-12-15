/********************************************************************
 *                                                                  *
 *  MODULE    :  WVUSENET.C                                         *
 *                                                                  *
 *  PURPOSE   : This file contains the window procedure and support *
 *              routines for WinVn's main window                    *
 *                                                                  *
 ********************************************************************/



/*
 * $Id: wvusenet.c 1.22 1993/12/08 01:28:01 rushing Exp $
 * $Log: wvusenet.c $
 * Revision 1.22  1993/12/08  01:28:01  rushing
 * new version box and cr lf consistency
 *
 * Revision 1.21  1993/12/07  20:24:43  rushing
 * testing new ID.
 *
 * Revision 1.20  1993/10/12  17:47:26  rushing
 * make winvn grok servers that start article numbers at 0
 *
 * Revision 1.19  1993/09/12  20:26:52  rushing
 * handle can't connect condition
 *
 * Revision 1.18  1993/08/25  18:54:36  mbretherton
 * MRB merge, mail & post logging
 *
 *
 * MBretherton
 * remove vRef from MRROpenFile (Mac Stuff)
 *
 * Revision 1.17  1993/08/18  21:49:21  rushing
 * more 16-bit article number fixes.
 *
 * Revision 1.16  1993/08/17  21:46:49  DUMOULIN
 * Added support for Dragging mouse for selecting/deselecting Usenet groups
 *
 * Revision 1.15  1993/07/06  21:09:50  cnolan
 * win32 support
 *
 * Revision 1.14  1993/06/28  17:51:39  rushing
 * fixed compiler warnings
 *
 * Revision 1.13  1993/06/24  17:13:14  riordan
 * Save window positions between sessions.
 *
 * Revision 1.12  1993/06/14  18:52:00  rushing
 * 'New Posting' menu option added
 *
 * Revision 1.11  1993/06/11  00:10:35  rushing
 * second merge from Matt Bretherton sources
 *
 * Revision 1.10  1993/06/10  18:24:41  rushing
 * hopefully newsrc&dolist thing is fixed FOR GOOD.
 *
 * Revision 1.9  1993/05/24  23:55:25  rushing
 * Delete Fonts (cleanup before exiting) (MRB)
 *
 * Revision 1.8  1993/05/20  18:12:31  rushing
 * don't write out newsrc if Intializing is true
 *
 * Revision 1.7  1993/04/29  20:23:06  rushing
 * attempted support for WSAAsyncSelect
 *
 * Revision 1.6  1993/04/27  20:11:38  rushing
 * Fixed out-of-date code that killed the timer if
 * CommState was ST_NONE.
 *
 * Revision 1.5  1993/04/27  18:53:17  rushing
 * attempt to fix newsrc problems.  allow newsrc line with
 * no range data.
 *
 * Revision 1.4  1993/03/30  21:07:15  DUMOULIN
 * handle conflicting MAPI merge problem (with wvsck???.c)
 * use ST_NONE instead of ST_CLOSED_COMM
 *
 * Revision 1.3  1993/03/30  20:46:07  bretherton
 * MAPI
 *
 * Revision 1.2  1993/02/25  03:08:52  rushing
 * fixed newsrc truncation when DoList = 0.
 *
 * Revision 1.1  1993/02/16  20:54:22  rushing
 * Initial revision
 *
 *
 */

#include "windows.h"


#include "winundoc.h"
#include <stdlib.h>


#include "WVglob.h"
#include "WinVn.h"

#include <stdlib.h>   /* for getenv */


// This is for the special case where the user invoked the
// program with dolist=0, then turned it on.  Without checking
// this condition, the newsrc will be truncated to only those
// groups read.  SMR 930224

int started_with_no_dolist;            

void show_version_strings(void);
void SetGroupActiveLines(void);


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

long FAR PASCAL
WinVnConfWndProc (hWnd, message, wParam, lParam)
     HWND hWnd;
     unsigned message;
     WPARAM wParam;
     LPARAM lParam;
{

  FARPROC lpProcAbout;
  HMENU hMenu;

  PAINTSTRUCT ps;               /* paint structure          */
  POINT ptCursor;

  HDC hDC;                      /* handle to display context */
  HWND hWndView;
  RECT myRect;                                    /* selection rectangle                */

  int j, idoc;
  int found;
  char far *lpsz;
  TypLine far *LinePtr, far * GroupLinePtr;
  TypBlock far *BlockPtr, far * GroupBlockPtr;
  HANDLE hBlock;
  TypDoc *MyDoc;
  unsigned int Offset;

  int X, Y;
  int docnum;
  int newdoc;
  char mybuf[MAXINTERNALLINE];
  int OldSel = FALSE;
  int CtrlState;
  TypLineID MyLineID;

  switch (message)
    {

    case WM_SYSCOMMAND:
      if (wParam == ID_ABOUT)
   {
     lpProcAbout = MakeProcInstance (About, hInst);
     DialogBox (hInst, "AboutBox", hWnd, lpProcAbout);
     FreeProcInstance (lpProcAbout);
     break;
   }
      else
   return (DefWindowProc (hWnd, message, wParam, lParam));

    case WM_CREATE:
      /* Set up the timer which will wake us up every so often
       * so we can check to see if new characters have arrived from
       * the server.
       */

      MailInit (hWnd) ;

      idTimer = SetTimer (hWnd, ID_TIMER ,1000, (FARPROC) NULL);

      if (!idTimer) {
   MessageBox (hWnd, "Couldn't create timer!", "WinVN", MB_OK | MB_ICONASTERISK);
      }

      /* Get handle to the hourglass cursor */

      hHourGlass = LoadCursor (hInst, IDC_WAIT);

      /* Add the "About" option to the system menu.            */

      hMenu = GetSystemMenu (hWnd, FALSE);
      ChangeMenu (hMenu, NULL, NULL, NULL, MF_APPEND | MF_SEPARATOR);
      ChangeMenu (hMenu, NULL, "A&bout WinVn...", ID_ABOUT,
        MF_APPEND | MF_STRING);

      /* Fill in the Net document's window handle that we
       * were unable to fill in in the call to InitDoc.
       */
      NetDoc.hDocWnd = hWnd;
      break;

    case WM_INITMENUPOPUP :
      EnableMenuItem(GetMenu(hWnd),IDM_LOGOUT,MailCtrl.enableLogout) ;
      EnableMenuItem(GetMenu(hWnd),IDM_MAIL,MailCtrl.enableMail) ;
      break ;

    case WM_CHAR:
      /* Hitting CR on a group name is the same as double-clicking
       * on the name.
       */
      if (!Initializing)
   {
     if (wParam == '\r')
       {

         GetCursorPos (&ptCursor);
         ScreenToClient (hWnd, &ptCursor);
         X = ptCursor.x;
         Y = ptCursor.y;
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
      if (!Initializing)
   {
     if (wParam == VK_F6)
       {
         NextWindow (&NetDoc);
       }
     else
       {
         /* Based on the state of the Control key and the value
          * of the key just pressed, look up in the array
          * key2scroll to see if we should process this keystroke
          * as if it were a mouse event.  If so, simulate the
          * mouse event by sending the appropriate message.
          */

         CtrlState = GetKeyState (VK_CONTROL) < 0;
         for (j = 0; j < NUMKEYS; j++)
      {
        if (wParam == key2scroll[j].wVirtKey &&
            CtrlState == key2scroll[j].CtlState)
          {
            SendMessage (hWnd, key2scroll[j].iMessage,
               key2scroll[j].wRequest, 0L);
            break;
          }
      }

       }
   }
      break;

    case WM_LBUTTONDOWN:
      /*  Clicking the left button on a group name toggles the
       *  selected/not selected status of that group.
       *  Currently selected groups are displayed in reverse video.
       */
   
      DragMouseAction = DRAG_NONE;
      if (!Initializing)
   {
          BOOL status;
     X = LOWORD (lParam);
     Y = HIWORD (lParam);

     if (CursorToTextLine (X, Y, &NetDoc, &BlockPtr, &LinePtr))
       {
         status = ((TypGroup far *) (((char far *) LinePtr) + sizeof (TypLine)))
      ->Selected ^= TRUE;
              DragMouseAction = status ? DRAG_SELECT : DRAG_DESELECT;
         GlobalUnlock (BlockPtr->hCurBlock);
       }

     InvalidateRect (hWnd, NULL, FALSE);
   }
      break;

    case WM_LBUTTONUP:
      /*  Letting up on the left button on a group name 
       *  gets us out of Dragging mode   */

      DragMouseAction = DRAG_NONE;
      break;

    case WM_MOUSEMOVE:
      /*  Code to drag the mouse and change the select/not selected
       *  status of that group.
       */

      if ((!Initializing) && (DragMouseAction != DRAG_NONE))
   {
     X = LOWORD (lParam);
     Y = HIWORD (lParam);

     if (CursorToTextLine (X, Y, &NetDoc, &BlockPtr, &LinePtr))
       {
              switch (DragMouseAction)
                {
       case DRAG_SELECT:
                       ((TypGroup far *) (((char far *) LinePtr) + sizeof (TypLine)))
               ->Selected = TRUE;
                      break;
       case DRAG_DESELECT:
                      ((TypGroup far *) (((char far *) LinePtr) + sizeof (TypLine)))
              ->Selected = FALSE;
                      break;
       }
         GlobalUnlock (BlockPtr->hCurBlock);
       }

     InvalidateRect (hWnd, NULL, FALSE);
        }

      break;

    case WM_LBUTTONDBLCLK:
      /*  Double-clicking on a group name creates a "Group"
       *  window, whose purpose is to display the subjects
       *  of the articles in the group.
       */
      if (!Initializing)
   {
     X = LOWORD (lParam);
     Y = HIWORD (lParam);
   getgroup:;

     if (CursorToTextLine (X, Y, &NetDoc, &GroupBlockPtr, &GroupLinePtr))
       {
         if (MyDoc = (((TypGroup far *)
      (((char far *) GroupLinePtr) + sizeof (TypLine)))->SubjDoc))
      {

        /* We already have a document containing the subjects */
        /* of this group, so just activate it.                */

        SetActiveWindow (MyDoc->hDocWnd);
        SetFocus (MyDoc->hDocWnd);
        GlobalUnlock (GroupBlockPtr->hCurBlock);
        break;
      }
         if (CommBusy)
      {
        GlobalUnlock (GroupBlockPtr->hCurBlock);
        MessageBox (hWnd, "Sorry, I am already busy retrieving information from the server.\n\
Try again in a little while.", "Can't request info on group", MB_OK | MB_ICONASTERISK);
        break;
      }

         /* At this point, we've determined that the subjects for
          * this newsgroup are not in memory, and that it's OK
          * to fetch them from the server.  (Comm line not busy.)
          * Figure out whether a new window/document should be
          * created for this group & set "newdoc" accordingly.
          */
         newdoc = FALSE;
         if (ViewNew || !ActiveGroupDoc || !(ActiveGroupDoc->InUse))
      {
        found = FALSE;
        for (docnum = 0; docnum < MAXGROUPWNDS; docnum++)
          {
            if (!GroupDocs[docnum].InUse)
         {
           found = TRUE;
           newdoc = TRUE;
           CommDoc = &(GroupDocs[docnum]);
           break;
         }
          }
        if (!found)
          {
            MessageBox (hWnd, "You have too many group windows \
active;\nClose one or select 'Reuse Old Window'.", "Can't open new window", MB_OK | MB_ICONASTERISK);
            UnlockLine (GroupBlockPtr, GroupLinePtr, &hBlock, &Offset, &MyLineID);
            break;
          }
      }
         else
      {
        /* Must reuse old window for this group.           */
        CommDoc = ActiveGroupDoc;
        UnlinkArtsInGroup (CommDoc);
        LockLine (CommDoc->hParentBlock, CommDoc->ParentOffset, CommDoc->ParentLineID, &BlockPtr, &LinePtr);
        ((TypGroup far *)
         (((char far *) LinePtr) + sizeof (TypLine)))->SubjDoc = (TypDoc *) NULL;
        UnlockLine (BlockPtr, LinePtr, &hBlock, &Offset, &MyLineID);
        /* clear out contents of this document for reuse */
        FreeDoc (CommDoc);
      }

         /* Create window title indicating we are retrieving text. */

         CommDoc->InUse = TRUE;
         lpsz = (char far *) (((char far *) GroupLinePtr) +
               sizeof (TypLine) + sizeof (TypGroup));
         strcpy (mybuf, "Retrieving ");
         lstrcat (mybuf, lpsz);

         /* If necessary, create a new window.              */

         if (newdoc)
      {
        int x,y,width,height;
        char poschars[MAXINTERNALLINE];

        /* Compute default screen position. */
        x = 1;
        y = 0;
        width = (int) xScreen;
        height = (int) (yScreen * 1 / 2);

        /* If the screen position has been saved, use that instead. */
        GetPrivateProfileString (szAppName, "GroupWindowPos", "!",
          poschars,MAXINTERNALLINE,szAppProFile);
        if(poschars[0] != '!') {
          sscanf(poschars,"%d,%d,%d,%d",&x,&y,&width,&height);
        }
        hWndView = CreateWindow ("WinVnView",
                  mybuf,
                  WS_OVERLAPPEDWINDOW | WS_VSCROLL,
                  x + (int) docnum * CharWidth,/* Initial X pos */
                  y + (int) docnum * LineHeight,
                  width,           /* Initial X Width */
                  height,
                  NULL,
                  NULL,
                  hInst,
                  NULL);

        if (!hWndView)
          return (0); /* ??? */

        ShowWindow (hWndView, SW_SHOWNORMAL);

      }
         else
      {
        hWndView = CommDoc->hDocWnd;
        SetWindowText (hWndView, mybuf);
      }
         RcvLineCount = 0;
         TimesWndUpdated = 0;

         /* Do some housekeeping:  Set group as selected,
          * initialize empty document to hold subject lines,
          * set focus to this document, set pointers linking
          * this document and the subject line.
          */
         ((TypGroup far *) (((char far *) GroupLinePtr) +
             sizeof (TypLine)))->Selected = TRUE;
         InitDoc (CommDoc, hWndView, &NetDoc, DOCTYPE_GROUP);
         PtrToOffset (GroupBlockPtr, GroupLinePtr, &(CommDoc->hParentBlock),
         &(CommDoc->ParentOffset), &(CommDoc->ParentLineID));
         SetActiveWindow (hWndView);
         SetFocus (hWndView);

         ((TypGroup far *) (((char far *) GroupLinePtr) + sizeof (TypLine)))
      ->SubjDoc = CommDoc;
         GlobalUnlock (GroupBlockPtr->hCurBlock);
         InvalidateRect (hWndView, NULL, FALSE);

         UpdateWindow (hWndView);

         InvalidateRect (NetDoc.hDocWnd, NULL, FALSE);

         /* Deal with Comm-related stuff:  set FSA variables,
          * send the GROUP command to NNTP.
          */

         CommLinePtr = CommLineIn;
         CommBusy = TRUE;
         CommState = ST_GROUP_RESP;
         hSaveCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));
         ShowCursor (TRUE);

/* capture the mouse to the usenet window, so that we keep the hourglass */

         SetCapture (hWnd);
         strcpy (mybuf, "GROUP ");
         lpsz = (char far *) GroupLinePtr + sizeof (TypLine) + sizeof (TypGroup);
         lstrcat (mybuf, lpsz);
         mylstrncpy (CurrentGroup, lpsz, MAXFINDSTRING);
         lstrcat(mybuf,"\r");
         PutCommLine (mybuf, lstrlen (mybuf));
       }
   }
      break;

// If we've lost the connection for some reason, kill the timer
// that's banging on the socket.
    case WM_TIMER:
     if ((CommState == ST_CLOSED_COMM) && (!Initializing))
       {
     KillTimer(hWnd,ID_TIMER) ;
       }
     else 
       {
    if (CommState != ST_CLOSED_COMM)
      DoCommInput ();
       };
      break;

    case WM_SIZE:
      /* Store the new size of the window.                     */
      GetClientRect (hWnd, &myRect);
      NetDoc.ScXWidth = myRect.right;
      NetDoc.ScYHeight = myRect.bottom;
      NetDoc.ScYLines = (myRect.bottom - myRect.top - TopSpace) / LineHeight;
      NetDoc.ScXChars = (myRect.right - myRect.left - SideSpace) / CharWidth;
      break;

    case WM_VSCROLL:
      if (!Initializing)
   {
     ScrollIt (&NetDoc, wParam, lParam);
   }
      break;

    case WM_COMMAND:
      switch (LOWORD(wParam))
   {

   case IDM_QUIT:
     SaveNewsrc = FALSE;
   case IDM_EXIT:
     DestroyWindow (hWnd);
     break;

   case IDM_VIEW_SEL_GROUP:
     break;

   case IDM_SHOW_SUBSCR:
   case IDM_SHOW_ALL_GROUP:
   case IDM_SEL_SUBSCR:
   case IDM_SELECTALL:
     MessageBox (hWnd, "Command not implemented",
            "Sorry", MB_OK);
     break;

   case IDM_UNSEL_ALL:
     ForAllLines (&NetDoc, SetLineFlag, 0, FALSE);
     InvalidateRect (hWnd, NULL, FALSE);
     break;

   case IDM_SUBSCRIBE:
     InitGroupTable ();
     ForAllLines (&NetDoc, GroupAction, GROUP_ACTION_SUBSCRIBE, TRUE);
     MergeGroups (ADD_SUBSCRIBED_END_OF_SUB);
     CleanUpGroupTable ();
     ForAllLines (&NetDoc, SetLineFlag, 0, FALSE);
     SetGroupActiveLines();
     ScreenToTop (&NetDoc);
     InvalidateRect (hWnd, NULL, FALSE);
     break;

   case IDM_UNSUBSCRIBE:
     InitGroupTable ();
     ForAllLines (&NetDoc, GroupAction, GROUP_ACTION_UNSUBSCRIBE, FALSE);
     ShellSort ((void far *) NewGroupTable,
           (size_t) nNewGroups,
           (size_t) sizeof (void far *),
           GroupCompare);
     MergeGroups (ADD_SUBSCRIBED_END_OF_SUB);
     CleanUpGroupTable ();
     ForAllLines (&NetDoc, SetLineFlag, 0, FALSE);
     SetGroupActiveLines();
     ScreenToTop (&NetDoc);
     InvalidateRect (hWnd, NULL, FALSE);
     break;

   case IDM_GROUP_TOP:
     InitGroupTable ();
     ForAllLines (&NetDoc, GroupAction, GROUP_ACTION_SUBSCRIBE, TRUE);
     MergeGroups (ADD_SUBSCRIBED_TOP_OF_DOC);
     CleanUpGroupTable ();
     ForAllLines (&NetDoc, SetLineFlag, 0, FALSE);
     ScreenToTop (&NetDoc);
     InvalidateRect (hWnd, NULL, FALSE);
     break;

   case IDM_NEW_WIN_GROUP:
     ViewNew = !ViewNew;
     CheckView (hWnd);
     break;

   case IDM_NEW_WIN_ARTICLE:
     NewArticleWindow = !NewArticleWindow;
     CheckView (hWnd);
     break;

   case IDM_COMMOPTIONS:

     if (DialogBox (hInst, "WinVnComm", hWnd, lpfnWinVnCommDlg))
       {
         InvalidateRect (hWnd, NULL, TRUE);
       }

     break;

   case IDM_CONFIG_PERSONAL:

     DialogBox (hInst, "WinVnPersonal", hWnd, lpfnWinVnPersonalInfoDlg);
     break;

   case IDM_CONFIG_MISC:

     DialogBox (hInst, "WinVnMisc", hWnd, lpfnWinVnMiscDlg);

     break;

   case IDM_CONFIG_LOG:

     DialogBox (hInst, "WinVnLogOpts", hWnd, lpfnWinVnLogOptDlg);

     break;


   case IDM_RESET:
     CommBusy = FALSE;
     CommState = ST_NONE;
     break;

   case IDM_SAVE_WINDOW:
     {
     RECT rect;

     /* Save position and size of Usenet window. */
     GetWindowRect(hWndConf,&rect);
     sprintf(mybuf,"%d,%d,%d,%d",rect.left,rect.top,
       rect.right-rect.left,rect.bottom-rect.top);
     WritePrivateProfileString
      (szAppName, "UsenetWindowPos", mybuf, szAppProFile);


     /* Save position and size of first Group window */

     for(found=FALSE,idoc=0; !found && idoc<MAXGROUPWNDS; idoc++) {
       if(GroupDocs[idoc].InUse) {
         GetWindowRect(GroupDocs[idoc].hDocWnd,&rect);
         found = TRUE;
         sprintf(mybuf,"%d,%d,%d,%d",rect.left,rect.top,
            rect.right-rect.left,rect.bottom-rect.top);
         WritePrivateProfileString
           (szAppName, "GroupWindowPos", mybuf, szAppProFile);
       }
     }

     /* Save position and size of first Article window */

     for(found=FALSE,idoc=0; !found && idoc<MAXARTICLEWNDS; idoc++) {
       if(ArticleDocs[idoc].InUse) {
         GetWindowRect(ArticleDocs[idoc].hDocWnd,&rect);
         found = TRUE;
         sprintf(mybuf,"%d,%d,%d,%d",rect.left,rect.top,
            rect.right-rect.left,rect.bottom-rect.top);
         WritePrivateProfileString
           (szAppName, "ArticleWindowPos", mybuf, szAppProFile);
       }
     }

     break;
     }

   case IDM_FIND:
     FindDoc = &NetDoc;

     if (DialogBox (hInst, "WinVnFind", hWnd, lpfnWinVnFindDlg))
       {
         found = DoFind (TRUE);
         if (!found)
      {
        strcpy (mybuf, "\"");
        strcat (mybuf, NetDoc.SearchStr);
        strcat (mybuf, "\" not found.");
        MessageBox (hWnd, mybuf, "Not found", MB_OK);
      }
       }
     break;

   case IDM_FIND_NEXT_SAME:
     FindDoc = &NetDoc;
     if (strcmp (FindDoc->SearchStr, ""))
       {
         found = DoFind (FALSE);
         if (!found)
      {
        strcpy (mybuf, "\"");
        strcat (mybuf, NetDoc.SearchStr);
        strcat (mybuf, "\" not found.");
        MessageBox (hWnd, mybuf, "No more occurrences", MB_OK);
      }
       }
     break;

   case ID_ABOUT:
     lpProcAbout = MakeProcInstance (About, hInst);
     DialogBox (hInst, "AboutBox", hWnd, lpProcAbout);
     FreeProcInstance (lpProcAbout);
     break;

   case IDM_HELP:
     MakeHelpPathName (mybuf, MAXINTERNALLINE);
     WinHelp (hWndConf, mybuf, HELP_INDEX, 0L);
     break;

   case IDM_MAIL:
     (MailCtrl.fnMlWinCreate)(hWnd, (TypDoc *) NULL, DOCTYPE_MAIL);
     break;
     
   case IDM_LOGOUT:
     (MailCtrl.fnMlLogout)(hWnd);
     break;

   case IDM_SHOW_VERSION:
     show_version_strings();
     break;

   case IDM_POST:
     DialogBoxParam (hInst, "WinVnGeneric", hWnd,
           lpfnWinVnGenericDlg, (LPARAM) (char far *) "Newsgroup(s):");
     NewsgroupsPtr = DialogString;
     CreatePostingWnd (hWnd, (TypDoc *) NULL, DOCTYPE_POSTING);
     break;

   }
      break;

    case WM_PAINT:
      {
   HANDLE hBlock;
   SIZE sz;
   unsigned int Offset, MyLen, width, indicatorwidth;
   int VertLines, HorzChars;
   int EndofDoc = FALSE;
   int RangeHigh, CurPos;
   int RestX, Xtext;
   char far *textptr;
   char *cptr, *cptr2;
   char indicator;
   char indcptr[128];
   char group_name[MAXINTERNALLINE];
   char init_msg[60];
   TypGroup far *MyGroup;
   DWORD MyColors[4], MyBack[4];
   DWORD Rop;
   RECT myRect;
   int MyColorMask = 1, PrevColorMask = MyColorMask;
#define SUBSCR_MASK 1
#define SELECT_MASK 2

   hDC = BeginPaint (hWnd, &ps);
   GetClientRect (hWnd, &myRect);
   SelectObject (hDC, hFont);

   /* If still initializing comm link, put out a message    */
   /* to that effect.                                       */

   switch (Initializing)
     {
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
       sprintf (mybuf, "Receiving %uth newsgroup", RcvLineCount);
       cptr = mybuf;
       cptr2 = "name from server...";

     display_msg:;
       X = SideSpace + CharWidth;
       Y = StartPen + 2 * LineHeight;
       strcpy (init_msg, cptr);
       strcat (init_msg, "    ");
       j = strlen (init_msg);
       TextOut (hDC, X, Y, init_msg, j);
       strcpy (init_msg, cptr2);
       strcat (init_msg, "    ");
       j = strlen (init_msg);
       TextOut (hDC, X, Y + LineHeight, init_msg, j);
       break;


     case INIT_DONE:

       VertLines = NetDoc.ScYLines;
       HorzChars = NetDoc.ScXChars;
       MyColors[0] = NetUnSubscribedColor;
       MyColors[1] = GetTextColor (hDC);
       MyColors[2] = MyColors[0];
       MyColors[3] = GetBkColor (hDC);

       MyBack[0] = MyColors[3];
       MyBack[1] = MyBack[0];
       MyBack[2] = MyColors[1];
       MyBack[3] = MyBack[2];

       GetTextExtentPoint (hDC, "*", 2, &sz);             
       indicatorwidth = sz.cx*7*7;                        

       LockLine (NetDoc.hCurTopScBlock, NetDoc.TopScOffset, NetDoc.TopScLineID,
            &BlockPtr, &LinePtr);

       /* Update the scroll bar thumb position.                 */

       CurPos = NetDoc.TopLineOrd;
       if (CurPos < 0)
         CurPos = 0;   
       RangeHigh = NetDoc.ActiveLines - VertLines;;
       if (RangeHigh < 0)
         RangeHigh = 0;
       SetScrollRange (hWnd, SB_VERT, 0, RangeHigh, FALSE);
       SetScrollPos (hWnd, SB_VERT, CurPos, TRUE);

       /* Loop through the lines, painting them on the screen. */

       X = SideSpace;
       Xtext = X + indicatorwidth;
       Y = StartPen;
       if (LinePtr->length != END_OF_BLOCK)
         do
       {                               
        MyGroup = ((TypGroup far *)
              ((char far *) LinePtr + sizeof (TypLine)));
        MyLen = MyGroup->NameLen;
        textptr = (char far *) LinePtr + sizeof (TypLine) +
          sizeof (TypGroup); 
          
        /* Display this line only if it is active. */  
        if(LinePtr->active) {

        /* Figure out the color of this line.                 */

        if (MyGroup->Subscribed)
          {
            MyColorMask |= SUBSCR_MASK;
          }
        else
          {
            MyColorMask &= (0xff - SUBSCR_MASK);
          }
        if (MyGroup->Selected)
          {
            MyColorMask |= SELECT_MASK;
            Rop = BLACKNESS;
          }
        else
          {
            MyColorMask &= 0xff - SELECT_MASK;
            Rop = WHITENESS;
          }
        if (MyColorMask != PrevColorMask)
          {
            SetTextColor (hDC, MyColors[MyColorMask]);
            SetBkColor (hDC, MyBack[MyColorMask]);
            PrevColorMask = MyColorMask;
          }

        /* Figure out what indicator character to use. */

        indicator = ' ';
        if (NetDoc.FindLineID == LinePtr->LineID)
          {
            indicator = '>';
          }
        else if (MyGroup->HighestPrevSeen)
          {
            if (MyGroup->ServerLast > MyGroup->HighestPrevSeen)
         {
           indicator = '*';
         }
          }

        mylstrncpy (group_name, textptr, MyGroup->NameLen + 1);

        sprintf (indcptr, "%c %5lu %s ",
                  indicator,
                  MyGroup->ServerEstNum,
                  group_name);

        /* Now write out the line.                            */

        MyLen = strlen (indcptr);
     GetTextExtentPoint (hDC, indcptr, MyLen, &sz);    
     width = sz.cx;                                    
        TextOut (hDC, X, Y, indcptr, MyLen);

      // This section was strange.  Maybe guilty.

            RestX = X + width;
            PatBlt (hDC, RestX, Y, myRect.right - RestX, LineHeight, Rop);

        Y += LineHeight;
        --VertLines;
      }  /* end if LinePtr->active */  
      }
         while (VertLines > 0 && NextLine (&BlockPtr, &LinePtr));

       SetTextColor (hDC, MyColors[1]);
       SetBkColor (hDC, MyBack[1]);

       /* Blank out bottom and top of screen */
       PatBlt (hDC, 0, Y, myRect.right, myRect.bottom - Y, PATCOPY);
       PatBlt (hDC, 0, 0, myRect.right, TopSpace, PATCOPY);

       UnlockLine (BlockPtr, LinePtr, &hBlock, &Offset, &MyLineID);
     }
   EndPaint (hWnd, &ps);
   break;
      }

    case WM_ENDSESSION:

      WinHelp (hWndConf, LFN_HELP, HELP_QUIT, 0L);
      //CloseComm (CommDevice);

      break;

    case WM_DESTROY:
      for (idoc = 0; idoc < MAXGROUPWNDS; idoc++)
   {
     if (GroupDocs[idoc].InUse)
       {
         UpdateSeenArts (&(GroupDocs[idoc]));
       }
   }
      if (SaveNewsrc && (!Initializing))
   {
     WriteNewsrc ();
   }
      MRRCloseComm ();

      MailClose(hWnd) ;

      /* Remove resources before exiting program */
      DeleteObject(hFont);
      DeleteObject(hFontArtNormal);
      DeleteObject(hFontArtQuote);
      DeleteObject(hFontPrint);
      DeleteObject(hFontPrintB);
      DeleteObject(hFontPrintS);
      DeleteObject(hFontPrintI);   

      PostQuitMessage (0);
      break;

    default:
      return (DefWindowProc (hWnd, message, wParam, lParam));
    }
  return (0);
}


/*---  FUNCTION: About ---------------------------------------------------
 *
 *  Process messages for "About" dialog box
 */

BOOL FAR PASCAL 
About (hDlg, message, wParam, lParam)
     HWND hDlg;
     unsigned message;
     WORD wParam;
     LONG lParam;
{
  switch (message)
    {
    case WM_INITDIALOG:
      return (TRUE);

    case WM_COMMAND:
      if (wParam == IDOK)
   {
     EndDialog (hDlg, TRUE);
     return (TRUE);
   }
      break;
    }
  return (FALSE);
}

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

VOID 
CheckView (hWnd)
     HWND hWnd;
{
  HMENU hMenu;
  int CheckIt;

  if (ViewNew)
    {
      CheckIt = MF_CHECKED | MF_BYCOMMAND;
    }
  else
    {
      CheckIt = MF_UNCHECKED | MF_BYCOMMAND;
    }
  hMenu = GetMenu (hWnd);

  CheckMenuItem (hMenu, IDM_NEW_WIN_GROUP, CheckIt);

  if (NewArticleWindow)
    {
      CheckIt = MF_CHECKED | MF_BYCOMMAND;
    }
  else
    {
      CheckIt = MF_UNCHECKED | MF_BYCOMMAND;
    }
  CheckMenuItem (hMenu, IDM_NEW_WIN_ARTICLE, CheckIt);
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
CrackGroupLine (buf, lineptr)
     char *buf;
     TypLine *lineptr;
{
  char *grname = (char *) lineptr + sizeof (TypLine) + sizeof (TypGroup);
  TypGroup *group = (TypGroup *) ((char *) lineptr + sizeof (TypLine));
  TypRange *RangePtr;
  BOOL MoreNums;
  unsigned int MyLength;

  group->Subscribed = FALSE;
  group->Selected = FALSE;
  group->NameLen = 0;
  group->SubjDoc = (TypDoc *) NULL;
  group->ServerEstNum = 0;
  group->ServerFirst = 0;
  group->ServerLast = 0;
  group->HighestPrevSeen = 0;
  group->nRanges = 0;
  group->header_handle = (HANDLE) NULL;

  /* Copy group name to output line.                               */

  while (*buf && *buf != ':' && *buf != '!')
    {
      *(grname++) = *(buf++);
      (group->NameLen)++;
    }
  *(grname++) = '\0';

  if (!(*buf))
    {
      /* Ran off end of line without seeing ':' or '!'.  Error.      */
    }
  else
    {
      if (*buf == ':')
   {
     group->Subscribed = TRUE;
   }
      buf++;
    }

  /* Look for the highest article number previously seen, in an
   * entry of form "s" followed by a number.
   */

  while (*buf && *buf == ' ' && *buf != '\n')
    buf++;
  if (*buf == 's')
    {
      buf++;
      GetNum (&buf, &(group->HighestPrevSeen));
    }

  /* Convert the article number ranges to the internal numeric
   * form we use in WinVN.
   */

  RangePtr = (TypRange *) ((char *) lineptr + sizeof (TypLine) +
            RangeOffset (group->NameLen));

  RangePtr->Last = RangePtr->First = 0;

  // allow lines like this, with no range data:
  // news.group: 
  //
  MoreNums = ( (*buf)=='\n' ? FALSE : TRUE);
  while (MoreNums)
    {
      while (*buf && (*buf == ' ' || *buf == ','))
   buf++;
      if (GetNum (&buf, &(RangePtr->First)))
   {
     /* We have the first number in a range.                     */
     (group->nRanges)++;
     RangePtr->Last = RangePtr->First;
     if (*buf == '-')
       {
         buf++;
         if (GetNum (&buf, &(RangePtr->Last)))
      {
        RangePtr++;
        /* at this point, we are positioned just after a range */
      }
         else
      {
        RangePtr->Last = RangePtr->First;
        MoreNums = FALSE;
      }
       }
     else if (*buf == ',')
       {
         /* We have a single number "n"; interpret as the range "n-n".
          */
         RangePtr++;
       }
     else
       {
         /* That must have been the last number.                  */
         MoreNums = FALSE;
       }
   }
      else
   {
     MoreNums = FALSE;
   }
    }
  if (group->nRanges == 0)
    (group->nRanges)++;

  MyLength = sizeof (TypLine) + RangeOffset (group->NameLen) +
    sizeof (TypRange) * (group->nRanges) + sizeof (int);

  lineptr->length = MyLength;
  lineptr->LineID = NextLineID++;
  *(int *) ((char *) lineptr + MyLength - sizeof (int)) = MyLength;

  return (group->Subscribed);
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
CursorToTextLine (X, Y, DocPtr, BlockPtr, LinePtr)
     int X;
     int Y;
     TypDoc *DocPtr;
     TypBlock far **BlockPtr;
     TypLine far **LinePtr;
{
  int found;
  int SelLine;

  if (Y < TopSpace || (unsigned) Y > TopSpace + DocPtr->ScYLines * LineHeight ||
      X < SideSpace)
    {
      /* Cursor is in no-man's-land at edge of window.               */
      found = FALSE;
    }
  else
    {
      found = TRUE;
      SelLine = (Y - TopSpace) / LineHeight;

      LockLine (DocPtr->hCurTopScBlock, DocPtr->TopScOffset, DocPtr->TopScLineID,
      BlockPtr, LinePtr);
      AdvanceToActive(BlockPtr,LinePtr);

      for (found = TRUE, il = 0; il < SelLine; )
   {
     if (!NextLine (BlockPtr, LinePtr))
       {
         found = FALSE;    /* ran off end of document */
         break;
       } else if((*LinePtr)->active){
         il++;
       }
   }
    }
  return (found);
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
ReadNewsrc ()
{
  TypBlock far *BlockPtr;
  TypLine *LocalLinePtr, far * GroupPtr;
  HANDLE hLine;
  HANDLE hBlock;
  HANDLE hRetCode;
  unsigned int Offset;
  TypLineID MyLineID;
#define  TEMPBUFSIZE   1240
  char mybuf[TEMPBUFSIZE];
  TypMRRFile *MRRFile;
  int returned;

  LockLine (NetDoc.hCurAddBlock, NetDoc.AddOffset, NetDoc.AddLineID, &BlockPtr, &GroupPtr);
  NetDoc.hDocWnd = hWndConf;

  hLine = LocalAlloc (LMEM_MOVEABLE, TEMPBUFSIZE);
  LocalLinePtr = (TypLine *) LocalLock (hLine);

#if 0
  env_var = getenv ("WINVN");           /* get path to winvn.ini */
  if (lstrlen (env_var))
   {
     lstrcpy (newsrc_filename, env_var);
     if (*(newsrc_filename + lstrlen (newsrc_filename) - 1) == '\\')
       lstrcat (newsrc_filename, "newsrc");
     else
       lstrcat (newsrc_filename, "\\newsrc");
   }
  else
   {
     MessageBox (hWndConf,"Environment variable WINVN not set.","Fatal Error", MB_OK | MB_ICONEXCLAMATION);
     return (0);
   }

#endif


  hRetCode = MRROpenFile (szNewsSrc, OF_READ, &MRRFile);
  if ((int) hRetCode <= 0)
    {
      return FALSE;
    }
  else
    {
      /* Loop to read lines, convert them to internal format, and
       * insert them into the NetDoc document.
       */

      while ((returned = MRRReadLine (MRRFile, mybuf, TEMPBUFSIZE)) > (-1))
   {
     mybuf[returned] = '\0';
     if (CrackGroupLine (mybuf, LocalLinePtr))
       {
         NetDoc.ActiveLines++;
       }
     AddLine (LocalLinePtr, &BlockPtr, &GroupPtr);
   }
      MRRCloseFile (MRRFile);
    }

  /* Change the title of the Net document.  I suppose that,
   * strictly speaking, this oughtn't be done in this routine.
   */
  SetNetDocTitle ();
  UnlockLine (BlockPtr, GroupPtr, &hBlock, &Offset, &MyLineID);

  NetDoc.hCurTopScBlock = NetDoc.hFirstBlock;
  NetDoc.TopScOffset = sizeof (TypBlock);
  NetDoc.TopScLineID = 0L;
  
  /* Mark lines active or inactive according to ShowUnsubscribed. */                                        
  SetGroupActiveLines();                                          
  // we'll check this during WriteNewsrc(). SMR 930224
  started_with_no_dolist = !DoList;

  return (TRUE);
}

char *ltoa ();

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
WriteNewsrc ()
{
  TypBlock far *BlockPtr;
  TypLine far *LinePtr;
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
  int nranges;
  TypGroup far *Group;
  TypRange far *RangePtr;
  BOOL firstrange;

  LockLine (NetDoc.hFirstBlock, sizeof (TypBlock), 0L, &BlockPtr, &LinePtr);

  hLine = LocalAlloc (LMEM_MOVEABLE, BLOCK_SIZE);
  NewsLine = (char *) LocalLock (hLine);

  hRetCode = MRROpenFile (szNewsSrc, OF_CREATE, &MRRFile);
  if ((int) hRetCode < 0)
    {
      sprintf (mes, "MRROpenFile returned %d", hRetCode);
      MessageBox (hWndConf, mes, mybuf, MB_OK);
    }
  else
    {
      do
   {
     toptr = NewsLine;
     Group = (TypGroup far *) ((char far *) LinePtr + sizeof (TypLine));

     /* Remove groups only if we performed a LIST command and */
     /* the server did not have them.  Otherwise if !did_list, the */
     /* newsrc will be practically empty! */

     /* Jeeeeez!!  I'm fixing this AGAIN! */
     /* DoList is not a boolean. SMR 930610 */

     if ((Group->ServerFirst || Group->ServerEstNum) || !did_list ) {
     /* Copy group name                                          */
     fromptr = (char far *) LinePtr + sizeof (TypLine) + sizeof (TypGroup);
     while (*(toptr++) = *(fromptr++));
     toptr--;

     /* Affix : or ! depending upon whether subscribed.          */
     *(toptr++) = (char) (Group->Subscribed ? ':' : '!');
     *(toptr++) = ' ';

     /* If we know the highest article number on the server,
      * output it preceded by an "s".
      */
     if (Group->ServerLast)
       {
         *(toptr++) = 's';
         ltoa ((unsigned long) Group->ServerLast, toptr, 10);
         while (*toptr)
      toptr++;
         *(toptr++) = ' ';
       }

     /* Affix ranges of articles read.                          */
     firstrange = TRUE;
     nranges = Group->nRanges;
     RangePtr = (TypRange far *) ((char far *) Group + RangeOffset (Group->NameLen));

     while ((nranges--) > 0)
       {
         /* Write out ',' if not first range of articles.         */

         if (!firstrange)
      {
        *(toptr++) = ',';
      }
         else
      {
        firstrange = FALSE;
      }
         /* Write out first article in a range.                   */

         ltoa ((unsigned long) RangePtr->First, toptr, 10);
         while (*toptr)
      toptr++;

         /* If the range is of form "n-n", just put out "n"       */

         if (RangePtr->First != RangePtr->Last)
      {
        /* Put out the hyphen in middle of range.                */
        *(toptr++) = '-';
        /* Put out the last article in a range.                  */
        ltoa ((unsigned long) RangePtr->Last, toptr, 10);
        while (*toptr)
          toptr++;
      }
         RangePtr++;
       }
     MRRWriteLine (MRRFile, NewsLine, toptr - NewsLine);
     }
   }
      while (NextLine (&BlockPtr, &LinePtr));
      UnlockLine (BlockPtr, LinePtr, &hBlock, &Offset, &MyLineID);
      MRRCloseFile (MRRFile);
    }
}

/*--- function SetNetDocTitle -------------------------------------------
 *
 */
void
SetNetDocTitle ()
{
  char mybuf[120];

  sprintf (mybuf, "WinVN:  %d groups; %d subscribed", NetDoc.TotalLines,
      NetDoc.ActiveLines);
  SetWindowText (hWndConf, mybuf);

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
SetLineFlag (TypDoc * Doc, TypBlock far ** BlockPtr, TypLine far ** LinePtr, int wFlag, int wValue)
{
  switch (wFlag)
    {
      case LINE_FLAG_SET:
      ((TypGroup far *) (((char far *) *LinePtr) + sizeof (TypLine)))
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
GroupAction (TypDoc * Doc, TypBlock far ** BlockPtr, TypLine far ** LinePtr, int wFlag, int wValue)
{

  switch (wFlag)
    {
      case GROUP_ACTION_SUBSCRIBE:
      case GROUP_ACTION_UNSUBSCRIBE:
      if (((TypGroup far *) (((char far *) *LinePtr) + sizeof (TypLine)))
     ->Selected)
   {
     ((TypGroup far *) (((char far *) *LinePtr) + sizeof (TypLine)))
     ->Subscribed = wValue;
     AddGroupToTable ((char far *) *LinePtr);
     DeleteLine (BlockPtr, LinePtr);
   }
      break;
      
      case GROUP_ACTION_CHECK_ACTIVE:
         if(((TypGroup far *) (((char far *) *LinePtr) + sizeof (TypLine)))
            ->Subscribed || ShowUnsubscribed ) {
            (*LinePtr)->active = TRUE;
            (*BlockPtr)->NumActiveLines++;
            NetDoc.ActiveLines++;
         } else {
            (*LinePtr)->active = FALSE;
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

void 
MakeHelpPathName (szFileName, maxchars)
     char *szFileName;
     int maxchars;
{
  char *pcFileName;
  int nFileNameLen;

  nFileNameLen = GetModuleFileName (hInst, szFileName, maxchars);
  pcFileName = szFileName + nFileNameLen;

  while (pcFileName > szFileName)
    {
      if (*pcFileName == '\\' || *pcFileName == ':')
   {
     *(++pcFileName) = '\0';
     break;
   }
      nFileNameLen--;
      pcFileName--;
    }

  if ((nFileNameLen + 13) < maxchars)
    {
      lstrcat (szFileName, LFN_HELP);
    }

  else
    {
      lstrcat (szFileName, "?");
    }

  return;
}


int
cursor_to_char_number (X, Y, DocPtr, BlockPtr, LinePtr)
     int X;
     int Y;
     TypDoc *DocPtr;
     TypBlock far **BlockPtr;
     TypLine far **LinePtr;
{
  int SelLine;
  int charnum = -1;
  SIZE extent;

  char far * textptr;
  int textlen;
  HDC display_context;

  if (Y < TopSpace || (unsigned) Y > TopSpace + DocPtr->ScYLines * LineHeight ||
      X < SideSpace)
    {
   return (-1);
    }
  else
    {
      SelLine = (Y - TopSpace) / ArtLineHeight;

      LockLine (DocPtr->hCurTopScBlock, DocPtr->TopScOffset, DocPtr->TopScLineID,
      BlockPtr, LinePtr);

      for (il = 0; il < SelLine; il++)
   {
     if (!NextLine (BlockPtr, LinePtr))
       {
         return (-1);
         break;
       }
   }
    }

  /* find the right character on the text line */
  textlen = ((TypText far *) ((char far *) (*LinePtr) +
               sizeof (TypLine)))->NameLen;


  if (textlen) {
    textptr = (char far *) ((char far *) (*LinePtr) + sizeof (TypLine) + sizeof (TypText) );
    display_context = GetDC (DocPtr->hDocWnd);

    for (charnum = 1; charnum < textlen; charnum++)
      {
    GetTextExtentPoint(display_context, (LPSTR) textptr, charnum, &extent); 
    if (extent.cx > (X - SideSpace)) break;                                 
      }
    ReleaseDC (DocPtr->hDocWnd, display_context);
    return (charnum - 1);
  }

}

/*--- function SetGroupActiveLines --------------------------------------
 *
 *  Go through all the lines in the Net document, marking each
 *  as active or inactive according to whether the corresponding
 *  group is subscribed and whether we are displaying unsubscribed
 *  groups.
 *
 *  Entry:  NetDoc and ShowUnsubscribed are set properly.
 *
 *  Exit:   Each of the lines in NetDoc has had its "active" field
 *          set properly.
 */
void
SetGroupActiveLines() {
   NetDoc.ActiveLines = 0;
   ForAllBlocks (&NetDoc, SetForBlock, BLOCK_ACTION_SET_ACTIVE, 0);
   ForAllLines (&NetDoc, GroupAction, GROUP_ACTION_CHECK_ACTIVE, 0);
}                 


/* stuff for showing version numbers of the files */

#include "version.h"
#include "version.c"

void show_version_strings(void)
{
  
  MessageBox (hWndConf,
         version_string,
         "WinVN Version Information",
         MB_OK);
}

/* Last line of WVUSENET.C */
