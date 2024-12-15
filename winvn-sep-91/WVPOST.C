/*--- WVPOST.C ------------------------------------------------------------
 *
 *    This file contains the window procedure for the "posting" windows,
 *    windows in which the user edits the text of an article to be posted.
 *
 *    Mark Riordan   27 Jan 1990
 */

#include "windows.h"
#include "wvglob.h"
#include "winvn.h"
#ifndef MAC
#include "winundoc.h"
#endif

long FAR PASCAL WinVnPostWndProc(hWnd, message, wParam, lParam)
HWND hWnd;
unsigned message;
WORD wParam;
LONG lParam;
{
    FARPROC lpProcAbout;
    HMENU hMenu;

    PAINTSTRUCT ps;                              /* paint structure          */

    HDC hDC;                                    /* handle to display context */
    HWND hWndPost, hWndPostEdit;
#ifdef MAC
   TEHandle TEH;
   Rect myClipRect;
#endif
    HDC  hDCView;
    int ih,j;
    int found;
    int imemo;
   TypDoc *MyDoc;
    char mesbuf[348];  /*** debug */

    for(ih=0,found=FALSE; !found && ih<MAXPOSTWNDS; ih++) {
      if(hWndPosts[ih] == hWnd) {
#ifndef MAC
         hWndPostEdit = hWndPostEdits[ih];
#else 
         TEH = TEHPosts[ih];
#endif
         found = TRUE;
         break;
      }
    }
   if(!found && message == WM_CREATE) {
      for(ih=0; !found && ih<MAXVIEWS; ih++) {
         if(!hWndPosts[ih]) {
            found = TRUE;
            hWndPosts[ih] = hWnd;
            break;
          }
      }
   }
#if 0
   if(!found) {
      MessageBox(hWnd,"Can't find Post hWnd","Error!",MB_OK|MB_ICONHAND);
      DestroyWindow(hWnd);
      return(NULL);  /*** ? */
   }
#endif

   switch (message) {
      case WM_SYSCOMMAND:
         return (DefWindowProc(hWnd, message, wParam, lParam));

      case WM_CREATE:
#ifndef MAC
            hWndPostEdit = hWndPostEdits[ih] = CreateWindow("edit", (char *)NULL,
                WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_BORDER
                    | ES_LEFT | ES_MULTILINE |
                    ES_AUTOHSCROLL | ES_AUTOVSCROLL
                    ,
                0, 0, 0, 0,
                hWnd, EDITID, hInst, NULL);
            sprintf(mesbuf,"hWndPostEdits[%d] = %5d\r\n       ",ih,hWndPostEdits[ih]);
       /*   MessageBox(hWnd,mesbuf,"Edit Window Results",MB_OK|MB_ICONASTERISK); */
            SetWindowText(hWndPostEdits[ih],mesbuf);
#endif
#if 0
            { long int freeHeap = FreeMem();
               long int biggestBlock = MaxBlock();
               long int freeStack = StackSpace();
               long int el;

               el = 0;
               sprintf(mesbuf,"FreeMem=%ld  MaxBlock=%ld  StackSpace=%ld",
                  freeHeap,biggestBlock,freeStack);
               MessageBox(hWnd,mesbuf,"Available Memory",MB_OK);
            }
#endif
            CreatePostingText(PostDoc,hWnd,hWndPostEdit,DOCTYPE_POSTING);
       /*   SetFocus(hWndEdits[ih]);  */
            break;

      case WM_SIZE:
#ifndef MAC
            MoveWindow(hWndPostEdit, 0, 0, LOWORD(lParam),
                HIWORD(lParam), TRUE);
#endif
            break;

      case WM_SETFOCUS:
            SetFocus(hWndPostEdit);
            break;

      case WM_KEYDOWN:
         if(wParam == VK_F6) {
            NextWindow(&(PostingDocs[ih]));
         }
         break;

#ifdef MAC
      case WM_PAINT:
         if(TEH) {
            myClipRect = hWnd->portRect;
            myClipRect.right -= SBarWidth;
            myClipRect.bottom -= SBarWidth;
            (*TEH)->viewRect = myClipRect;
            TEUpdate(&myClipRect,TEH);
         }
         break;

#endif

      case WM_COMMAND:
         switch(wParam) {
            case IDM_CANCEL:
            case IDV_EXIT:
               DestroyWindow(hWnd);
               break;

            case IDM_POST:
               StartPost(ih);
               break;

            case IDM_UNDO:
               SendMessage(hWndPostEdit, WM_UNDO, 0, 0L);
               break;

            case IDM_CUT:
               SendMessage(hWndPostEdit, WM_CUT, 0, 0L);
               break;

            case IDM_COPY:
               SendMessage(hWndPostEdit, WM_COPY, 0, 0L);
               break;

            case IDM_PASTE:
               SendMessage(hWndPostEdit, WM_PASTE, 0, 0L);
               break;

            case IDM_CLEAR:
               SendMessage(hWndPostEdit, WM_CLEAR, 0, 0L);
               break;

            case IDM_SELALL:
               SendMessage(hWndPostEdit, EM_SETSEL, 0,
                MAKELONG(0,32767));
               break;

         }
            break;


        case WM_DESTROY:
            hWndPosts[ih] = 0;
#ifndef MAC
            hWndPostEdits[ih] = 0;
#else
            TEDispose(TEHPosts[ih]);
            TEHPosts[ih] = (TEHandle) 0;
            TEHCurrent = 0;
#endif
            break;


        default:
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (0L);
}

/*--- function StartPost ----------------------------------------------
 *
 *    Start the posting procedure for an article that has been created
 *    by the user.
 *
 *    Entry    ih    is the index into hWndPostEdits of the child
 *                   edit window containing the article.
 */
BOOL
StartPost(ih)
int ih;
{
   BOOL found;
   BOOL PostOK = FALSE;
   char outline[MAXOUTLINE];

   if(CommBusy) {
      MessageBox(hWndPosts[ih],"Sorry, I am already busy retrieving information from the server.\n\
Try again in a little while.","Can't post article right now",MB_OK|MB_ICONASTERISK);
   } else {
      CommLinePtr = CommLineIn;
      CommBusy = TRUE;
      CommState = ST_POST_WAIT_PERMISSION;
      CommDoc =  &(PostingDocs[ih]);

      strcpy(outline,"POST");
      PutCommLine(outline,strlen(outline));
      PostOK = TRUE;
   }
   return(PostOK);
}

/*--- function PostText -------------------------------------------------
 *
 *    Post the text of an article.  (Second phase of posting process.)
 *
 *    Entry    ih    is the index into hWndPostEdits of the child
 *                   edit window containing the article.
 */
BOOL
PostText(ih,DocType)
int ih;
int DocType;
{
#ifndef MAC
   HWND hWndPostEdit;
#else
   TEHandle TEH = TEHPosts[ih];
#endif
   char outline[MAXOUTLINE];
   char *outptr;
   char *editptr;
   HANDLE hEditBuffer;
   int linelen;
   BOOL EndBuf=FALSE;

#ifndef MAC
   if(DocType == DOCTYPE_POSTING) {
      hWndPostEdit = hWndPostEdits[ih];
      CommState = ST_POST_WAIT_END;
   } else {
      hWndPostEdit = hWndMailEdits[ih];
      CommState = ST_MAIL_WAIT_END;
   }
#endif

   /* Retrieve the text of the edit window, and run through it
    * line by line, sending the lines to the server.
    */
#ifndef MAC
   hEditBuffer = (HANDLE) SendMessage(hWndPostEdit,EM_GETHANDLE,0,0L);
   editptr = LocalLock(hEditBuffer);
#else
   hEditBuffer = TEGetText(TEH);
   editptr = LocalLock(hEditBuffer);
   editptr[(*TEH)->teLength] = '\0';
   
#ifdef MRRBUG
   sprintf(outline,"PostText after TEGetText, editptr=%.30s",editptr);
   MessageBox((HWND) 0,outline,"",MB_OK);
#endif

#endif
   while(!EndBuf) {
      for(linelen = 0, outptr = outline; *editptr && *editptr!='\r' &&
        linelen < MAXOUTLINE; linelen++) *(outptr++) = *(editptr++);
      PutCommLine(outline,linelen);
#ifdef MRRBUG
      outline[linelen] = '\0';
      MessageBox((HWND) 0,outline,"Posted line",MB_OK);
#endif
      if(*editptr == '\r') editptr++;
      if(*editptr == '\n') editptr++;
      if(!*editptr) {
         EndBuf = TRUE;
      }
   }
#if 1
   LocalUnlock(hEditBuffer);
#endif
   PutCommLine(".",1);
   return(TRUE);
}
