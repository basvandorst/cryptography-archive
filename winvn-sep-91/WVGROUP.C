/*---  wvgroup.c ------------------------------------------- */
/*  This file contains the window procedure for the Group Viewing window
 *  for WinVN.
 */

#include "windows.h"
#ifndef MAC
#include "winundoc.h"
#endif
#include "WVglob.h"
#include "WinVn.h"
#ifdef MAC
#include "MRRM1.h"
#include <DialogMgr.h>
#endif

/*--- FUNCTION: WinVnViewWndProc --------------------------------------------
 *
 *    Window procedure for a Group window, which contains the subjects
 *    of the various articles in a newsgroup.
 *    Note that there may be several different Group windows active;
 *    this routine gets called any time anything happens to any of them.
 */

long FAR PASCAL WinVnViewWndProc(hWnd, message, wParam, lParam)
HWND hWnd;
unsigned message;
WORD wParam;
LONG lParam;
{
   FARPROC lpProcAbout;
   HMENU hMenu;

   PAINTSTRUCT ps;                              /* paint structure          */

   HDC hDC;                                    /* handle to display context */
   RECT clientRect;                                  /* selection rectangle      */
   HDC  hDCView;
   TypDoc *ThisDoc;
   int ih,j;
   int iart;
   int found;
   int imemo;
   int CtrlState;
   TypBlock far *BlockPtr, far *ArtBlockPtr;
   TypLine far *LinePtr, far *ArtLinePtr;
   HANDLE hBlock;
   unsigned int Offset;
   char mybuf[MAXINTERNALLINE];
   TypDoc *MyDoc;
   TypLineID MyLineID;
   POINT ptCursor;

   int X, Y, nWidth, nHeight;
   int mylen;
   int OldSel = FALSE;

   /* We know what *window* is being acted on, but we must find
    * out which *document* is being acted on.  There's a one-to-one
    * relationship between the two, and we find out which document
    * corresponds to this window by scanning the GroupDocs array.
    */

   for(ih=0,found=FALSE; !found && ih<MAXGROUPWNDS; ih++) {
      if(GroupDocs[ih].hDocWnd == hWnd) {
         found = TRUE;
         ThisDoc = &(GroupDocs[ih]);
      }
   }

   if(!found) {
      ThisDoc = CommDoc;
   }

    switch (message) {
        case WM_ACTIVATE:
            if(wParam){
               ActiveGroupDoc = ThisDoc;               
#if 0
               SetMenuBar(groupMenuBar);
               DrawMenuBar();
#endif
            }
            /* fall through */
        case WM_SYSCOMMAND:
           return (DefWindowProc(hWnd, message, wParam, lParam));

        case WM_SIZE:
            GetClientRect(hWnd, &clientRect);
            ThisDoc->ScXWidth =  clientRect.right;
            ThisDoc->ScYHeight = clientRect.bottom;
            ThisDoc->ScYLines = (clientRect.bottom - clientRect.top - TopSpace) / LineHeight;
            ThisDoc->ScXChars = (clientRect.right - clientRect.left - SideSpace) / CharWidth;
            break;

         case WM_DESTROY:
            /* Unlink all the article windows that belong to this group */

            UpdateSeenArts(ThisDoc);
            UnlinkArtsInGroup(ThisDoc);
            ThisDoc->InUse = FALSE;
            if(ThisDoc == CommDoc) {
               CommBusy = FALSE;
               CommDoc = (TypDoc *) NULL;
            }
            /* Clear the pointer in the line for this group in the   */
            /* NetDoc document.  This pointer currently points       */
            /* to the current document, which we are wiping out      */
            /* with the destruction of this window.                  */

            LockLine(ThisDoc->hParentBlock,ThisDoc->ParentOffset,ThisDoc->ParentLineID,&BlockPtr,&LinePtr);
            ( (TypGroup far *)
              ( ((char far *)LinePtr)+sizeof(TypLine) ) )->SubjDoc = (TypDoc *)NULL;
            UnlockLine(BlockPtr,LinePtr,&hBlock,&Offset,&MyLineID);
            /* Clear document                                        */
            FreeDoc(ThisDoc);

            /* If there's another group window, make it the active   */
            /* group window so we don't create a new one if the      */
            /* New Group flag is FALSE.                              */

            for(j=MAXGROUPWNDS-1; j>=0; j--) {
               if(GroupDocs[j].InUse) {
                  ActiveGroupDoc = &(GroupDocs[j]);
                  break;
               }
            }
            break;

         case WM_KEYDOWN:
            /* See if this key should be mapped to a scrolling event
             * for which we have programmed the mouse.  If so,
             * construct the appropriate mouse call and call the mouse code.
             */
#ifndef MAC
            if(wParam == VK_F6) {
               NextWindow(ThisDoc);
            } else {
               CtrlState = GetKeyState(VK_CONTROL) < 0;
               for(j=0; j<NUMKEYS; j++) {
                  if(wParam == key2scroll[j].wVirtKey &&
                   CtrlState == key2scroll[j].CtlState) {
                     SendMessage(hWnd,key2scroll[j].iMessage,
                      key2scroll[j].wRequest,0L);
                     break;
                  }
               }
            }
#endif
            break;


        case WM_CHAR:
            /* Carriage Return means the same as double-clicking
             * on where the cursor is currently pointing.
             */
            if (wParam == '\r') {
                GetCursorPos(&ptCursor);
                ScreenToClient(hWnd, &ptCursor);
#ifdef MAC
            X = ptCursor.h;
            Y = ptCursor.v;
#else
                X = ptCursor.x;
                Y = ptCursor.y;
#endif
                goto getarticle;
            } else {
            }
            break;

         case WM_LBUTTONDOWN:
          /*  Clicking the left button on an article name toggles the
           *  selected/not selected status of that article.
           *  Currently selected articles are displayed in reverse video.
           */
#if 0
          /****  This code is currently disabled.  */
            X = LOWORD(lParam);
            Y = HIWORD(lParam);
            if(CursorToTextLine(X,Y,ThisDoc,&BlockPtr,&LinePtr)) {
               ( (TypArticle far *)( ((char far *)LinePtr) + sizeof(TypLine) ) )
                ->Selected ^= TRUE;
               GlobalUnlock(BlockPtr->hCurBlock);
            }

            InvalidateRect(hWnd, NULL, FALSE);
#endif
            break;

        case WM_LBUTTONDBLCLK:
           /*  Double-clicking on an article subject creates an "Article"
            *  window, whose purpose is to display the article.
            */
            X = LOWORD(lParam);
            Y = HIWORD(lParam);
           getarticle:;

            if(CursorToTextLine(X,Y,ThisDoc,&ArtBlockPtr,&ArtLinePtr)) {
               ViewArticle(ThisDoc,ArtBlockPtr,ArtLinePtr,FALSE);
            }

            break;

        case WM_VSCROLL:
            ScrollIt(ThisDoc,wParam,lParam);
            break;

         case WM_PAINT:
          {
            HANDLE hBlock;
            int MyLen, width;
            unsigned int Offset;
            int VertLines, HorzChars;
            int EndofDoc = FALSE;
            int RangeHigh, CurPos;
            int RestX,indicatorwidth,Xtext;
            char far *textptr;
            char *indcptr;
            TypArticle far *MyArt;
            TypBlock far *BlockPtr;
            TypLine far *LinePtr;
            TypBlock far *NetBlockPtr;
            TypLine far *NetLinePtr;
            TypGroup far *group;
            long int OldHighestSeen;
            HANDLE hBlackBrush;
            DWORD MyColors[4], MyBack[4];
#ifdef MAC
            RECT myRect;
            POINT myPoint;
#endif
            DWORD Rop;
            int MyColorMask = 0;
            int PrevColorMask = MyColorMask;

            /* MyColors and MyBack are arrays of colors used to display text
             * foreground and background.
             * The ColorMask variables are indices into these arrays.
             * We set and clear bits in these indices depending upon
             * whether the article has been selected or seen.
             */

#define SEEN_MASK 1
#define SELECT_MASK 2
            hDC = BeginPaint (hWnd, &ps);

            GetClientRect(hWnd, &clientRect);
            SelectObject(hDC,hFont);
            VertLines = ThisDoc->ScYLines;
            HorzChars = ThisDoc->ScXChars;

            MyColors[0] = GetTextColor(hDC);  /* black */
            MyColors[1] = GroupSeenColor;
            MyColors[2] = GetBkColor(hDC);    /* white */
            MyColors[3] = MyColors[1];        /* blue  */

            MyBack[0] = MyColors[2];          /* white */
            MyBack[1] = MyColors[2];
            MyBack[2] = MyColors[0];
            MyBack[3] = MyColors[0];

            LockLine(ThisDoc->hCurTopScBlock,ThisDoc->TopScOffset,ThisDoc->TopScLineID,
             &BlockPtr,&LinePtr);

            /* Update the scroll bar thumb position.                 */

            CurPos = ThisDoc->TopLineOrd;
            if(CurPos<0) CurPos = 0;
            RangeHigh = ThisDoc->TotalLines - VertLines;
            if(RangeHigh<0) RangeHigh = 0;
            SetScrollRange(hWnd,SB_VERT,0,RangeHigh,FALSE);
            SetScrollPos  (hWnd,SB_VERT,CurPos,TRUE);

            indicatorwidth = LOWORD(GetTextExtent(hDC,"s",2)) * 8 / 7;
            LockLine(ThisDoc->hParentBlock,ThisDoc->ParentOffset,ThisDoc->ParentLineID,
              &NetBlockPtr, &NetLinePtr);
            group = (TypGroup far *) ((char far *)NetLinePtr + sizeof(TypLine));
            OldHighestSeen = group->HighestPrevSeen;
            UnlockLine(NetBlockPtr,NetLinePtr,&hBlock,&Offset,&MyLineID);

#ifdef MAC
            myRect.right = ThisDoc->DocClipRect.right;
            myRect.top = 0;
            myRect.bottom = LineHeight;
#endif

            /* Now paint this stuff on the screen.               */

            X = SideSpace;
            Xtext = X + indicatorwidth;
            Y = StartPen;

            if(LinePtr->length != END_OF_BLOCK)
            do {
               MyArt =  ((TypArticle far *)
                ((char far *)LinePtr + sizeof(TypLine)));
               MyLen = LinePtr->length - sizeof(TypLine) -
                sizeof(TypArticle) - sizeof(int) - 1;
               MyLen = MyArt->NameLen;
               textptr = (char far *) LinePtr + sizeof(TypLine) +
                sizeof(TypArticle);

               /* Figure out the color of this line.                 */

               if(MyArt->Seen) {
                  MyColorMask |= SEEN_MASK;
               } else {
                  MyColorMask &= (0xff - SEEN_MASK);
               }
               if(MyArt->Selected) {
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
               if(ThisDoc->FindLineID == LinePtr->LineID) {
                  indcptr = ">   ";
               } else if(MyArt->Seen) {
                  indcptr = "s   ";
               } else if(OldHighestSeen) {
                  if(MyArt->Number > OldHighestSeen) {
                     indcptr = "n   ";
                  }
               }

               /* Now write out the line.                            */

               TextOut(hDC,X,Y,indcptr,4);
               width = LOWORD(GetTextExtent(hDC,textptr,MyLen));
               TextOut(hDC,Xtext,Y,textptr,MyLen);
#ifdef MAC
               GetPen(&myPoint);
               myRect.left = myPoint.h;
               FillRect(&myRect,white);

               myRect.top += LineHeight;
               myRect.bottom += LineHeight;
#else
               RestX = Xtext + width;
               PatBlt(hDC,RestX,Y,clientRect.right-RestX,LineHeight,Rop);
#endif
#if 0
               if(MyLen < HorzChars) {
                  RestX = X + width;
            /*    TextOut(hDC,RestX,Y,Blanks,MAXINTERNALLINE); */
                  PatBlt(hDC,RestX,Y,clientRect.right-RestX,LineHeight,Rop);
               }
#endif
               Y += LineHeight;
            } while(--VertLines>0 && NextLine(&BlockPtr,&LinePtr) );

            /* We've reached the end of the data to be displayed     */
            /* on this window.  If there's more screen real estate   */
            /* left, just blank it out.                              */

            SetTextColor(hDC,MyColors[0]);
            SetBkColor(hDC,MyBack[0]);
#ifndef MAC
            PatBlt(hDC,0,Y,clientRect.right,clientRect.bottom-Y,PATCOPY);
#else
            myRect.bottom = ThisDoc->DocClipRect.bottom;
            myRect.left = 0;
            EraseRect(& myRect);
#endif
            UnlockLine(BlockPtr,LinePtr,&hBlock,&Offset,&MyLineID);
            EndPaint(hWnd, &ps);
            break;
         }

        case WM_COMMAND:
            switch(wParam) {
                case IDV_EXIT:
                    DestroyWindow(hWnd);
                    break;

                case IDV_NEXT:
                     break;

               case IDM_FIND:
                  FindDoc = ThisDoc;
#ifndef MAC
                  if(DialogBox(hInst,"WinVnFind",hWnd,lpfnWinVnFindDlg)) {
                     found = DoFind(TRUE);
                     if(!found) {
                        strcpy(mybuf,"\"");
                        strcat(mybuf,ThisDoc->SearchStr);
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
                        strcat(mybuf,ThisDoc->SearchStr);
                        strcat(mybuf,"\" not found.");
                        MessageBox(hWnd,mybuf,"Not found",MB_OK);
                     }
                  }
                  }
#endif
                  break;

               case IDM_FIND_NEXT_SAME:
                  FindDoc = ThisDoc;
                  if(strcmp(FindDoc->SearchStr,"")) {
                     found = DoFind(FALSE);
                     if(!found) {
                        strcpy(mybuf,"\"");
                        strcat(mybuf,ThisDoc->SearchStr);
                        strcat(mybuf,"\" not found.");
                        MessageBox(hWnd,mybuf,"No more occurrences",MB_OK);
                     }
                  }
                  break;

               case IDV_CREATE:
                  /* We are creating the skeleton text of a new posting.
                   * Most of the work is done by CreatePostingWnd and
                   * CreatePostingText.  Here we have to identify
                   * the newsgroup for those routines.
                   * Get the newsgroup from the line in NetDoc that
                   * points to this document.
                   */
                  LockLine(ThisDoc->hParentBlock,ThisDoc->ParentOffset,
                   ThisDoc->ParentLineID,&BlockPtr,&LinePtr);
                  ExtractTextLine(ThisDoc->ParentDoc,LinePtr,
                   mybuf,MAXINTERNALLINE);
                  UnlockLine(BlockPtr,LinePtr,&hBlock,&Offset,&MyLineID);
                  NewsgroupsPtr = mybuf;
                  CreatePostingWnd((TypDoc *)NULL,DOCTYPE_POSTING);
                  break;
            }
            break;


        default:
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (0);
}


/* --- function CopyNonBlank ----------------------------------------
 *
 *    Copy a string, terminating at the first blank or zero byte.
 *
 *    Entry    strtarg     FWA of target area.
 *             strsource   FWA of source string.
 *             maxchars    the maximum number of characters to copy
 *                         if no blank or zero is encountered.
 *
 *    Exit     returns the number of characters copied.
 *
 *    I believe this routine is no longer used, but I'm leaving it here.
 */

int
CopyNonBlank(strtarg,strsource,maxchars)
char *strtarg,*strsource;
int maxchars;
{
    int j;

    for(j=0; j<maxchars && strsource[j] != ' '; j++) strtarg[j] = strsource[j];
    return(j);
}

/* --- function FileLength -------------------------------------------
 *
 *    Find the size, in bytes, of a file.
 *
 *    Entry    hFile    handle of the file in question.
 *
 *    Exit     returns the length of the file in bytes.
 *
 *    This routine is no longer used.
 */

long
FileLength(hFile)
HANDLE hFile;
{
    long lCurrentPos = _llseek(hFile, 0L, 1);
    long lFileLength = _llseek(hFile, 0L, 2);

    _llseek(hFile, lCurrentPos, 0);

    return lFileLength;

}

/*-- function ViewArticle -------------------------------------------------
 *
 *  View a given article.   Either create a new window for it or
 *  recycle an existing window.
 *  This function requests an article from the server, so there
 *  must not already be a transaction in progress.
 *
 *    Entry    Doc            points to the document for this group.
 *             ArtBlockPtr &
 *             ArtLinePtr     point to the line in the group document
 *                            that has the subject for this article.
 *             Reuse          is TRUE if we ought to reuse the
 *                            currently active article window (if any).
 */
void
ViewArticle(Doc,ArtBlockPtr,ArtLinePtr,Reuse)
TypDoc *Doc;
TypBlock far *ArtBlockPtr;
TypLine far  *ArtLinePtr;
BOOL Reuse;
{
   TypDoc *MyDoc;
   TypDoc *GroupDoc;
   BOOL newdoc;
   BOOL found;
   int docnum;
   HANDLE hBlock;
   unsigned int Offset;
   TypLineID MyLineID;
   TypBlock far *BlockPtr;
   TypLine far *LinePtr;
   HWND hWndArt;
   int width;
   char mybuf[MAXINTERNALLINE];
   long int artnum;
   char far *lpsz;
   HWND hWndGroup = Doc->hDocWnd;
   char far *lpszGroupName;

   if(MyDoc = (( (TypArticle far *)
     ( ((char far *)ArtLinePtr)+sizeof(TypLine) ) )->ArtDoc)) {

   /* We already have a document containing the article */
   /* so just activate it.                */

  /*  ShowWindow(MyDoc->hDocWnd,SW_SHOW); */
      SetActiveWindow(MyDoc->hDocWnd);
      SetFocus(MyDoc->hDocWnd);
      GlobalUnlock(ArtBlockPtr->hCurBlock);
      goto endit;
   }
   if(CommBusy) {
      GlobalUnlock(ArtBlockPtr->hCurBlock);
      MessageBox(hWndGroup,"Sorry, I am already busy retrieving information from the server.\n\
Try again in a little while.","Can't request text of article",MB_OK|MB_ICONASTERISK);
      goto endit;
   }

   newdoc = FALSE;
   if((NewArticleWindow && !Reuse) || !ActiveArticleDoc || !(ActiveArticleDoc->InUse)) {
      found = FALSE;
      for(docnum=0; docnum<MAXARTICLEWNDS; docnum++) {
        if(!ArticleDocs[docnum].InUse) {
            found = TRUE;
            newdoc = TRUE;
            CommDoc = &(ArticleDocs[docnum]);
            break;
         }
      }
      if(!found) {
         MessageBox(hWndGroup,"You have too many article windows \
active;\nClose one or uncheck the option \"New Window for each Article\".","Can't open new window",MB_OK|MB_ICONASTERISK);
         UnlockLine(ArtBlockPtr,ArtLinePtr,&hBlock,&Offset,&MyLineID);
         goto endit;
      }
   } else {
      /* Must reuse old window for this article.         */
      CommDoc = ActiveArticleDoc;
      if(CommDoc->hParentBlock) {
         LockLine(CommDoc->hParentBlock,CommDoc->ParentOffset,CommDoc->ParentLineID,&BlockPtr,&LinePtr);
         ( (TypArticle far *)
          ( ((char far *)LinePtr)+sizeof(TypLine) ) )->ArtDoc = (TypDoc *)NULL;
         UnlockLine(BlockPtr,LinePtr,&hBlock,&Offset,&MyLineID);
      }
      /* clear out old doc */
      FreeDoc(CommDoc);
   }

   ( (TypArticle far *)( ((char far *)ArtLinePtr) + sizeof(TypLine) ) )
    ->Seen = TRUE;
   InvalidateRect(hWndGroup,NULL,FALSE);

   lpsz = (char far *) ( ((char far *)ArtLinePtr) +
    sizeof(TypLine)+ sizeof(TypArticle) ) ;
   strcpy(mybuf,"Retrieving \"");
   lstrcat(mybuf,lpsz);
   lstrcat(mybuf,"\"");

   if(newdoc) {
      if(xScreen > 88*CharWidth) {
         width = 88*CharWidth;
      } else {
         width = xScreen - 1*CharWidth;
      }
      hWndArt = CreateWindow("WinVnArt",
      mybuf,
      WS_OVERLAPPEDWINDOW | WS_VSCROLL  ,
      (docnum)*CharWidth,     /* Initial X pos */
      (int) (yScreen*3/8) + (docnum)*LineHeight, /* Initial Y pos */
      (int) width,                     /* Initial X Width */
      (int) (yScreen*5/8) - (1*LineHeight),  /* Initial Y height */
      NULL,
      NULL,
      hInst,
      NULL);

      if (!hWndArt) return;  /* ??? */
#ifndef MAC
      ShowWindow(hWndArt, SW_SHOWNORMAL);
#else
      MyShowWindow(hWndArt, SW_SHOWNORMAL);
#endif
   } else {
      hWndArt = CommDoc->hDocWnd;
      SetWindowText(hWndArt,mybuf);
   }

   /*  Now that we have created the window, create the corresponding
    *  document, and make the new window active.
    */

   InitDoc(CommDoc,hWndArt,ArtBlockPtr->OwnerDoc,DOCTYPE_ARTICLE);
   CommDoc->InUse = TRUE;
   PtrToOffset(ArtBlockPtr,ArtLinePtr,&(CommDoc->hParentBlock),
    &(CommDoc->ParentOffset),&(CommDoc->ParentLineID));
   ArtBlockPtr->OwnerDoc->hLastSeenBlock = CommDoc->hParentBlock;
   ArtBlockPtr->OwnerDoc->LastSeenOffset = CommDoc->ParentOffset;
   SetActiveWindow(hWndArt);
   SetFocus(hWndArt);

   ( (TypArticle far *)( ((char far *)ArtLinePtr) + sizeof(TypLine) ) )
    ->ArtDoc = CommDoc;
   InvalidateRect(hWndArt,NULL,FALSE);
   UpdateWindow(hWndArt);

   CommLinePtr = CommLineIn;
   CommBusy = TRUE;
   CommState = ST_ARTICLE_RESP;
               
               
   /* If we're not already in this group on the server,
    * send out a GROUP command for this window so we get back
    * into the right Group.
    */
   LockLine(Doc->hParentBlock,Doc->ParentOffset,Doc->ParentLineID,
       &BlockPtr,&LinePtr);
   lpszGroupName = (char far *)LinePtr + sizeof(TypLine) + sizeof(TypGroup);
   if(lstrcmp(CurrentGroup,lpszGroupName)) {     
      CommState = ST_GROUP_REJOIN;
      strcpy(mybuf,"GROUP ");
      lstrcat(mybuf,lpszGroupName);
      /* lstrcat(mybuf,"\r"); */
      mylstrncpy(CurrentGroup,lpsz,MAXGROUPNAME);
      PutCommLine(mybuf,lstrlen(mybuf));
   }       
   UnlockLine(BlockPtr,LinePtr,&hBlock,&Offset,&MyLineID);

   artnum = ((TypArticle far *)((char far *)ArtLinePtr +
    sizeof(TypLine)))->Number;
   sprintf(mybuf,"ARTICLE %ld\r",artnum);
   PutCommLine(mybuf,lstrlen(mybuf));

   GlobalUnlock(ArtBlockPtr->hCurBlock);
endit:;
}

/*-- Function UnlinkArtsInGroup ---------------------------------------
 *
 *  Modify all the article documents and all the article windows currently
 *  associated with a group so that none of them points to that group.
 *  Used when the group window is going away or is being recycled.
 *
 *    Entry    GroupDoc    points to the document to which references
 *                         should be eliminated.
 */
void
UnlinkArtsInGroup(GroupDoc)
TypDoc *GroupDoc;
{
   int iart;

   for(iart=0; iart<MAXARTICLEWNDS; iart++ ) {
      if(ArticleDocs[iart].InUse && ArticleDocs[iart].ParentDoc == GroupDoc) {
         ArticleDocs[iart].ParentDoc = (TypDoc *)NULL;
         ArticleDocs[iart].hParentBlock = 0;
      }
   }
}

/*--- function UpdateSeenArts -------------------------------------------
 *
 *  Given a Group document, update the TypGroup line for
 *  that document in the Net document with respect to which
 *  articles have been seen.
 *  This routine would typically be called just before a Group document
 *  is going to be destroyed or erased.  That would be the time to
 *  take the information in the TypArticle structures of each line
 *  in the document and transfer it to the line in the NetDoc document
 *  corresponding to this group.
 *
 *  This routine has to take information of the form:
 *    123:Unseen;  124:Seen; 125:Unseen; 126:Unseen; 127:Seen; 128:Seen; 129:Seen
 *  found in the TypArticle structures in consecutive lines in the document
 *  and transform it to the general form used by .newsrc files:
 *    124,127-129
 *  (though we are using our internal representation & not ASCII characters).
 *
 *    Entry    Doc      points to the document for this group.
 *
 *    Exit     The line in the Net document corresponding to this
 *              group has been updated.
 */
void
UpdateSeenArts(Doc)
TypDoc *Doc;
{
   TypRange MyRange, *RangePtr;
   TypGroup *group;
   TypLine far *LinePtr, far *ParentLine;
   TypBlock far *BlockPtr, far *ParentBlock;
   HANDLE hLine;
   TypLine *LocalLinePtr;
   TypArticle far *Art;
   BOOL InSeen = TRUE;
   unsigned int MyLength;
   unsigned int maxRanges;

   /*  Get the line in the Net document that corresponds to this
    *  group.  Make a local copy of it and set RangePtr to point to
    *  the first range in that line.  We will ignore the old line's
    *  "seen" data and create the information afresh from what we
    *  have in this document.
    */
   LockLine(Doc->hParentBlock,Doc->ParentOffset,Doc->ParentLineID,
     &ParentBlock, &ParentLine);

   hLine = LocalAlloc(LMEM_MOVEABLE,BLOCK_SIZE);
   LocalLinePtr = (TypLine *) LocalLock(hLine);
   group = (TypGroup *) ((char *) LocalLinePtr + sizeof(TypLine));

   MoveBytes(ParentLine,LocalLinePtr,ParentLine->length);
   group->nRanges = 0;
   maxRanges = ((Doc->BlockSize - Doc->SplitSize) - ParentLine->length +
      group->nRanges * sizeof(TypRange)) / sizeof(TypRange) - 1;

   RangePtr = (TypRange *) ((char *) LocalLinePtr + sizeof(TypLine) +
    RangeOffset(group->NameLen));
   MyRange.First = 1;

   /* Get the first line in this document.
    * If it cannot be found, just set Last=First and skip the
    * proceeding processing.  Otherwise, assume we've seen everything
    * up to but not including the first article in the document.
    */

   LockLine(Doc->hFirstBlock,sizeof(TypBlock),0L,&BlockPtr,&LinePtr);
   if(LinePtr->length == END_OF_BLOCK) {
      MyRange.Last = 1;
   } else {
      MyRange.Last = ((TypArticle far *)((char far *)LinePtr+sizeof(TypLine)))->Number - 1;

      /* Loop to scan through the document, fabricating article ranges.
       */
      do {
         Art = (TypArticle far *) ((char far *)LinePtr + sizeof(TypLine));
         if(Art->Seen) {
            if(InSeen) {
               /* Continuing a sequence of seen articles.            */
               MyRange.Last = Art->Number;
            } else {
               /* Starting a new sequence of seen articles.          */
               MyRange.First = Art->Number;
               MyRange.Last  = Art->Number;
               InSeen = TRUE;
            }
         } else {
            if(InSeen) {
               /* Ending a sequence of seen articles.                   */
               InSeen = FALSE;
               *(RangePtr++) = MyRange;
               (group->nRanges)++;
            } else {
               /* Continuing a sequence of unseen articles.             */
            }
         }
      } while ((group->nRanges < maxRanges) &&
          NextLine(&BlockPtr,&LinePtr));

      if(InSeen) {
         *(RangePtr++) = MyRange;
         (group->nRanges)++;
      }
   }
   GlobalUnlock(BlockPtr->hCurBlock);

   MyLength = sizeof(TypLine) + RangeOffset(group->NameLen) +
    sizeof(TypRange)*(group->nRanges) + sizeof(int);
   LocalLinePtr->length = MyLength;
   *(int *) ( (char *)LocalLinePtr + MyLength - sizeof(int) ) = MyLength;

   ReplaceLine(LocalLinePtr,&ParentBlock,&ParentLine);
   GlobalUnlock(ParentBlock->hCurBlock);

   LocalUnlock(hLine);
   LocalFree(hLine);

}
