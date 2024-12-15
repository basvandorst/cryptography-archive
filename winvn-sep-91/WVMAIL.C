/*--- WVMAIL.C ------------------------------------------------------------
 *
 *    This file contains the window procedure for the windows
 *    used to edit outgoing mail messages.
 *
 *    Mark Riordan   11 June 1991
 */

#include "windows.h"
#include "wvglob.h"
#include "winvn.h"
#ifndef MAC
#include "winundoc.h"
#endif

long FAR PASCAL WinVnMailWndProc(hWnd, message, wParam, lParam)
HWND hWnd;
unsigned message;
WORD wParam;
LONG lParam;
{
    HMENU hMenu;

    PAINTSTRUCT ps;                              /* paint structure          */

    HDC hDC;                                    /* handle to display context */
    HWND hWndMail, hWndMailEdit;
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

    for(ih=0,found=FALSE; !found && ih<MAXMAILWNDS; ih++) {
      if(hWndMails[ih] == hWnd) {
#ifndef MAC
         hWndMailEdit = hWndMailEdits[ih];
#else 
         TEH = TEHMails[ih];
#endif
         found = TRUE;
         break;
      }
    }
   if(!found && message == WM_CREATE) {
      for(ih=0; !found && ih<MAXVIEWS; ih++) {
         if(!hWndMails[ih]) {
            found = TRUE;
            hWndMails[ih] = hWnd;
            break;
          }
      }
   }
#if 0
   if(!found) {
      MessageBox(hWnd,"Can't find Mail hWnd","Error!",MB_OK|MB_ICONHAND);
      DestroyWindow(hWnd);
      return(NULL);  /*** ? */
   }
#endif

   switch (message) {
      case WM_SYSCOMMAND:
         return (DefWindowProc(hWnd, message, wParam, lParam));

      case WM_CREATE:
#ifndef MAC
            hWndMailEdit = hWndMailEdits[ih] = CreateWindow("edit", (char *)NULL,
                WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_BORDER
                    | ES_LEFT | ES_MULTILINE |
                    ES_AUTOHSCROLL | ES_AUTOVSCROLL
                    ,
                0, 0, 0, 0,
                hWnd, EDITID, hInst, NULL);
            sprintf(mesbuf,"hWndMailEdits[%d] = %5d\r\n       ",ih,hWndMailEdits[ih]);
       /*   MessageBox(hWnd,mesbuf,"Edit Window Results",MB_OK|MB_ICONASTERISK); */
            SetWindowText(hWndMailEdits[ih],mesbuf);
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
            CreatePostingText(MailDoc,hWnd,hWndMailEdit,DOCTYPE_MAIL);
       /*   SetFocus(hWndEdits[ih]);  */
            break;

      case WM_SIZE:
#ifndef MAC
            MoveWindow(hWndMailEdit, 0, 0, LOWORD(lParam),
                HIWORD(lParam), TRUE);
#endif
            break;

      case WM_SETFOCUS:
            SetFocus(hWndMailEdit);
            break;

      case WM_KEYDOWN:
         if(wParam == VK_F6) {
            NextWindow(&(MailDocs[ih]));
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
               StartMail(ih);
               break;

            case IDM_UNDO:
               SendMessage(hWndMailEdit, WM_UNDO, 0, 0L);
               break;

            case IDM_CUT:
               SendMessage(hWndMailEdit, WM_CUT, 0, 0L);
               break;

            case IDM_COPY:
               SendMessage(hWndMailEdit, WM_COPY, 0, 0L);
               break;

            case IDM_PASTE:
               SendMessage(hWndMailEdit, WM_PASTE, 0, 0L);
               break;

            case IDM_CLEAR:
               SendMessage(hWndMailEdit, WM_CLEAR, 0, 0L);
               break;

            case IDM_SELALL:
               SendMessage(hWndMailEdit, EM_SETSEL, 0,
                MAKELONG(0,32767));
               break;
         }
         break;


        case WM_DESTROY:
            hWndMails[ih] = 0;
#ifndef MAC
            hWndMailEdits[ih] = 0;
#else
            TEDispose(TEHMails[ih]);
            TEHMails[ih] = (TEHandle) 0;
            TEHCurrent = 0;
#endif
            break;


        default:
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (0L);
}

/*--- function StartMail ----------------------------------------------
 *
 *    Start the Mailing procedure for an article that has been created
 *    by the user.
 *
 *    Entry    ih    is the index into hWndMailEdits of the child
 *                   edit window containing the article.
 */
BOOL
StartMail(ih)
int ih;
{
   BOOL found;
   BOOL MailOK = FALSE, got_To = FALSE;
   char outline[MAXOUTLINE], cmdline[MAXOUTLINE];
   char *editptr, *outptr, *cptr;
   HANDLE hEditBuffer;
   HWND hWndMailEdit = hWndMailEdits[ih];
   int linelen;

   if(CommBusy) {
      MessageBox(hWndMails[ih],"Sorry, I am already busy retrieving information from the server.\n\
Try again in a little while.","Can't mail message right now",MB_OK|MB_ICONASTERISK);
   } else {
      CommLinePtr = CommLineIn;
      CommBusy = TRUE;
      CommState = ST_MAIL_WAIT_PERMISSION;
      CommDoc =  &(MailDocs[ih]);

      /* Create XMAIL command.  We need to extract the "To:" line
       * in order to find the recipient for the command.
       */
#ifndef MAC
      hEditBuffer = (HANDLE) SendMessage(hWndMailEdit,EM_GETHANDLE,0,0L);
      editptr = LocalLock(hEditBuffer);
#else
      hEditBuffer = TEGetText(TEH);
      editptr = LocalLock(hEditBuffer);
      editptr[(*TEH)->teLength] = '\0';
   
#ifdef MRRBUG
      sprintf(outline,"MailText after TEGetText, editptr=%.30s",editptr);
      MessageBox((HWND) 0,outline,"",MB_OK);
#endif

#endif
      while(!got_To && *editptr) {
         for(linelen = 0, outptr = outline; *editptr && *editptr!='\r' &&
           linelen < MAXOUTLINE; linelen++) *(outptr++) = *(editptr++);
         *outptr = '\0';
         if(CompareStringNoCase(outline,"To: ",4)==0) {
            got_To = TRUE;
         }
      }
      LocalUnlock(hEditBuffer);

      if(!got_To) {
         MessageBox(hWndMails[ih],"Fill in the \"To:\" line","Need a recipient",
           MB_OK|MB_ICONHAND);
      } else {
         outptr = outline;
         NextToken(&outptr);
         strcpy(cmdline,"xMail ");
         for(cptr=cmdline+strlen(cmdline);
           *outptr>' ' && cptr-cmdline<MAXOUTLINE;
           *(cptr++) = *(outptr++));
         *cptr = '\0';
         PutCommLine(cmdline,strlen(cmdline));
         MailOK = TRUE;
      }
   }
   return(MailOK);
}
