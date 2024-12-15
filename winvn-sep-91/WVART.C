/*---  wvart.c ------------------------------------------- */
/*  This file contains the window procedure for the Article Viewing window
 *  for WinVn.
 */

#include "windows.h"
#ifndef MAC
#include "winundoc.h"
#endif
#include "WVglob.h"
#include "WinVn.h"

/****************************************************************************

    FUNCTION: WinVnArtWndProc(HWND, unsigned, WORD, LONG)

    PURPOSE:  Processes messages

****************************************************************************/

long FAR PASCAL WinVnArtWndProc(hWnd, message, wParam, lParam)
HWND hWnd;
unsigned message;
WORD wParam;
LONG lParam;
{
    FARPROC lpProcAbout;
    HMENU hMenu;

    PAINTSTRUCT ps;                              /* paint structure          */

    HDC hDC;                                    /* handle to display context */
    RECT myRect;                                  /* selection rectangle      */
    HWND hWndView;
    HDC  hDCView;
   TypDoc *ThisDoc;
   TypDoc *GroupDoc;
    int ih,j;
    int found;
    int imemo;
    int CtrlState;
   TypLine far *LinePtr, far *ArtLinePtr;
   TypBlock far *BlockPtr, far *ArtBlockPtr;
   HANDLE hBlock;
   unsigned int Offset;
   int reply;
   int result;
   char far *cptr;
   TypLineID MyLineID;
#ifdef MAC
   int vRef;
#endif

   for(ih=0,found=FALSE; !found && ih<MAXARTICLEWNDS; ih++) {
     if(ArticleDocs[ih].hDocWnd == hWnd) {
       found = TRUE;
       ThisDoc = &(ArticleDocs[ih]);
     }
   }

   if(!found) {
      ThisDoc = CommDoc;
   }

    switch (message) {

        case WM_ACTIVATE:
        case WM_SETFOCUS:
            if(wParam){
               ActiveArticleDoc = ThisDoc;
            }
            /* fall through */
        case WM_SYSCOMMAND:
           return (DefWindowProc(hWnd, message, wParam, lParam));

        case WM_SIZE:
            GetClientRect(hWnd, &myRect);
            ThisDoc->ScXWidth =  myRect.right;
            ThisDoc->ScYHeight = myRect.bottom;
            ThisDoc->ScYLines = (myRect.bottom - myRect.top - TopSpace) / LineHeight;
            ThisDoc->ScXChars = (myRect.right - myRect.left - SideSpace) / CharWidth;
            break;

         case WM_DESTROY:
            ThisDoc->InUse = FALSE;
            if(ThisDoc == CommDoc) {
               CommBusy = FALSE;
               CommDoc = (TypDoc *) NULL;
            }
            /* Clear the pointer in the line for this article in the   */
            /* group  document.  This pointer currently points       */
            /* to the current document, which we are wiping out      */
            /* with the destruction of this window.                  */

            if(ThisDoc->hParentBlock) {
               LockLine(ThisDoc->hParentBlock,ThisDoc->ParentOffset,ThisDoc->ParentLineID,&BlockPtr,&LinePtr);
               ( (TypArticle far *)
                ( ((char far *)LinePtr)+sizeof(TypLine) ) )->ArtDoc =
                 (TypDoc *)NULL;
               UnlockLine(BlockPtr,LinePtr,&hBlock,&Offset,&MyLineID);
            }
            /* Clear document                                        */
            FreeDoc(ThisDoc);

            /* If there's another article window, make it the active   */
            /* artcile window so we don't create a new one if the      */
            /* New Article flag is FALSE.                              */

            for(j=MAXARTICLEWNDS-1; j>=0; j--) {
               if(ArticleDocs[j].InUse) {
                  ActiveArticleDoc = &(ArticleDocs[j]);
                  break;
               }
            }
            break;
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

        case WM_VSCROLL:
            ScrollIt(ThisDoc,wParam,lParam);
            break;

         case WM_PAINT:
          {
            HANDLE hBlock;
            unsigned int Offset, MyLen, width;
            int VertLines, HorzChars;
            int EndofDoc = FALSE;
            int RangeHigh, CurPos;
            int RestX;
            char far *textptr;
            TypArticle far *MyArt;
            TypBlock far *BlockPtr;
            TypLine far *LinePtr;
            HANDLE hBlackBrush;
            DWORD MyColors[4], MyBack[4];
            int MyColorMask = 1, PrevColorMask = MyColorMask;
#ifdef MAC
            RECT myRect;
            POINT myPoint;
#endif

            hDC = BeginPaint (hWnd, &ps);

            GetClientRect(hWnd, &myRect);
            SelectObject(hDC,hFont);
            VertLines = ThisDoc->ScYLines;
            HorzChars = ThisDoc->ScXChars;

            LockLine(ThisDoc->hCurTopScBlock,ThisDoc->TopScOffset,ThisDoc->TopScLineID,
             &BlockPtr,&LinePtr);

            /* Update the scroll bar thumb position.                 */

            CurPos = ThisDoc->TopLineOrd;
            if(CurPos<0) CurPos = 0;
            RangeHigh = ThisDoc->TotalLines - VertLines;
            if(RangeHigh<0) RangeHigh = 0;
            SetScrollRange(hWnd,SB_VERT,0,RangeHigh,FALSE);
            SetScrollPos  (hWnd,SB_VERT,CurPos,TRUE);
#ifdef MAC
            myRect.right = ThisDoc->DocClipRect.right;
            myRect.top = 0;
            myRect.bottom = LineHeight;
#endif
            /* Now paint this stuff on the screen for debugging. */

            X = SideSpace;
            Y = StartPen;

            if(LinePtr->length != END_OF_BLOCK)
             do {
               MyLen = LinePtr->length - sizeof(TypLine) - sizeof(int) - 1;
               MyLen = ((TypText far *)((char far *) LinePtr +
                sizeof(TypLine)))->NameLen;
               textptr = (char far *) LinePtr + sizeof(TypLine)+ sizeof(TypText);

               /* Now write out the line.                            */

#if 1
               width = LOWORD(GetTextExtent(hDC,textptr,MyLen));
#else
      width = CharWidth * MyLen;
#endif
               TextOut(hDC,X,Y,textptr,MyLen);
#ifdef MAC
               GetPen(&myPoint);
               myRect.left = myPoint.h;
               FillRect(&myRect,white);

               myRect.top += LineHeight;
               myRect.bottom += LineHeight;
#else
               RestX = X + width;
               PatBlt(hDC,RestX,Y,myRect.right-RestX,LineHeight,PATCOPY);
#endif
#if 0
               if(MyLen < HorzChars) {
                  RestX = X + width;
#if 0
                  TextOut(hDC,RestX,Y,Blanks,MAXINTERNALLINE);
#endif
                  PatBlt(hDC,RestX,Y,myRect.right-RestX,LineHeight,PATCOPY);
               }
#endif
               Y += LineHeight;
            } while(--VertLines>0 && NextLine(&BlockPtr,&LinePtr) );

            /* We've reached the end of the data to be displayed     */
            /* on this window.  If there's more screen real estate   */
            /* left, just blank it out.                              */

#if !MAC
            PatBlt(hDC,0,Y,myRect.right,myRect.bottom-Y,PATCOPY);
#else
         /* myRect.top = myRect.top + LineHeight; */
            myRect.bottom = ThisDoc->DocClipRect.bottom;
            myRect.left = 0;
            EraseRect(& myRect);
#endif
#if 0
            while(VertLines-- > -1) {
               TextOut(hDC,X,Y,Blanks,HorzChars);
               Y += LineHeight;
            }
#endif

            UnlockLine(BlockPtr,LinePtr,&hBlock,&Offset,&MyLineID);
            EndPaint(hWnd, &ps);
            break;
         }

        case WM_COMMAND:
            switch(wParam) {
                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;

                case IDM_SAVE:
                  if(strcmp(SaveArtFileName,"")) {
                     SaveArtAppend = TRUE;
                     MRRWriteDocument(ActiveArticleDoc,sizeof(TypText),SaveArtFileName,SaveArtvRef,SaveArtAppend);
                       /* Should the 0 be sizeof(TypText) ? */
                     break;
                  } else {
                     goto saveas;
                  }

                case IDM_SAVEAS:
saveas:;
#ifndef MAC
                  if(DialogBox(hInst,"WinVnSaveArt",hWnd,lpfnWinVnSaveArtDlg)) {
                     InvalidateRect(hWnd,NULL,TRUE);
                  }
#else
                  result = MySaveAs(SaveArtFileName,&SaveArtvRef,&SaveArtAppend);
                  if(result) {
                     MRRWriteDocument(ActiveArticleDoc,sizeof(TypText),SaveArtFileName,SaveArtvRef,SaveArtAppend);
                  }
#endif
                   break;

                case IDM_NEXT_ARTICLE:
                  GroupDoc = ThisDoc->ParentDoc;
                  if(GroupDoc) {
                     LockLine(GroupDoc->hLastSeenBlock,GroupDoc->LastSeenOffset,GroupDoc->LastSeenLineID,
                      &BlockPtr,&LinePtr);
                     if(NextLine(&BlockPtr,&LinePtr)){
                        ViewArticle(GroupDoc,BlockPtr,LinePtr,TRUE);
                        /* ViewArticle unlocks the block itself. */
                     } else {
                        MessageBox(hWnd,"No more articles","That's all!",MB_OK);
                        GlobalUnlock(BlockPtr->hCurBlock);
                     }
                  } else {
                     MessageBox(hWnd,"Sorry--you must have the group window around\n\
for me to be able to find the next article","Can't find next article",MB_OK);
                  }
                  break;

               case IDM_FIND_NEXT_SAME:
                  GroupDoc = ThisDoc->ParentDoc;
                  if(GroupDoc) {
                     /* Locate the line in the parent Group document */
                     /* containing the subject of this article.      */
                     /* Make this subject our search pattern,        */
                     /* after stripping off a leading "Re: "         */
                     /* if necessary.                                */

                     LockLine(ThisDoc->hParentBlock,ThisDoc->ParentOffset,ThisDoc->ParentLineID,
                        &BlockPtr,&LinePtr);
                     cptr = (char far *)LinePtr + sizeof(TypLine) + sizeof(TypArticle);
                     if(mylstrncmp(cptr,"Re: ",4)==0) cptr += 4;
                     mylstrncpy(GroupDoc->SearchStr,cptr,MAXFINDSTRING);
                     GlobalUnlock(BlockPtr->hCurBlock);

                     /* Now try to find another line in the group    */
                     /* document that contains this subject.          */

                     GroupDoc->hFindBlock = ThisDoc->hParentBlock;
                     GroupDoc->FindOffset = ThisDoc->ParentOffset;
                     GroupDoc->FindLineID = ThisDoc->ParentLineID;
                     FindDoc = GroupDoc;
                     result = DoFind(FALSE);

                     /* If such an article was found, display it.    */
                     if(result) {
                        LockLine(FindDoc->hFindBlock,FindDoc->FindOffset,FindDoc->FindLineID,
                         &BlockPtr,&LinePtr);
                        ViewArticle(FindDoc,BlockPtr,LinePtr,TRUE);
                     } else {
                        MessageBox(hWnd,"No more articles with this subject.","None found",MB_OK);
                     }
                  }
                  break;

               case IDM_FOLLOWUP:
                  CreatePostingWnd(ThisDoc,DOCTYPE_POSTING);
                  break;

               case IDM_MAIL:
                  CreatePostingWnd(ThisDoc,DOCTYPE_MAIL);
                  break;
            }
            break;

        default:
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (0);
}
