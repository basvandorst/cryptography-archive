/*-- WVUTIL.C -- File containing utility routines.
 */

#include "windows.h"
#include "wvglob.h"
#include "winvn.h"
#ifndef MAC
#include "winundoc.h"
#include <ctype.h>
#endif

/*--- function GetNum --------------------------------------------
 *
 *  Cracks off a positive integer number from a string.
 *
 *  Entry    *ptr  is the character position to start scanning
 *                 for an integer
 *
 *  Exit     *ptr  is the character position at which we stopped
 *                 scanning (because of a non-digit).
 *           *num  is the cracked off number.
 *           Returns TRUE iff we got a number.
 */
BOOL
GetNum(ptr,num)
char **ptr;
long int *num;
{
   BOOL gotit = FALSE;

   /* Skip initial spaces                                            */

   while((**ptr) && **ptr == ' ') (*ptr)++;

   *num = 0;
   while(**ptr && isdigit(**ptr)) {
      *num = 10*(*num) + (**ptr - '0');
      gotit = TRUE;
      (*ptr)++;
   }
   return(gotit);
}

/*-- function StrToRGB -------------------------------------------------
 *
 *  Takes an ASCII string of the form "r,g,b" where r, g, and b are
 *  decimal ASCII numbers, and converts it to an RGB color number.
 */
DWORD
StrToRGB(cstring)
char *cstring;
{
   BYTE red, green, blue;
   long int lred, lgreen, lblue;

   GetNum(&cstring,&lred);
   cstring++;
   GetNum(&cstring,&lgreen);
   cstring++;
   GetNum(&cstring,&lblue);
   red = (BYTE) lred; green = (BYTE) lgreen; blue = (BYTE) lblue;
#ifndef MAC
   return(RGB(red,green,blue));
#else
   return((DWORD) red+green+blue);
#endif
}

/*-- function DoCommInput ---------------------------------------
 *
 *
 */
void
DoCommInput()
{
   int ch;

   while((ch = MRRReadComm()) >= 0) {
      /*   putchar(ch); */  /* debug */
      if(ch == IgnoreCommCh) {
      } else if(ch == EOLCommCh) {
         *CommLinePtr = '\0';
         DoCommState();
         CommLinePtr = CommLineIn;
      } else {
         *(CommLinePtr++) = (char) ch;
         if(CommLinePtr == CommLineLWAp1) CommLinePtr--;
      }
   }
}

/*-- function DoCommState ----------------------------------------------
 *
 *  Function to implement an FSA to process incoming lines from
 *  the server.
 *  This function is called once for each line from the server.
 *
 *    Entry    CommLineIn  is a zero-terminated line received from
 *                         the server.
 *             CommState   is the current state of the FSA.
 */
void
DoCommState()
{
   TypLine far *LinePtr;
   TypBlock far *BlockPtr;
   TypArticle *artptr;
   TypArticle far *MyArtPtr;
   HANDLE hBlock;
   HWND hWndPostEdit;
   unsigned int Offset;
   TypLineID MyLineID;
   int retcode;
   int ih, found;
   unsigned int estnum;
   long int first,last;
   long int artnum;
   int lineord;
   int mylen;
   int col;
   int mbcode;
   BOOL done=FALSE;
   BOOL dolist;
   char group[MAXINTERNALLINE];
   char mybuf[MAXINTERNALLINE];
   char artline[MAXINTERNALLINE];
   char *cptr, *cdest;
   char far *lpsz;
   HDC hDC;
   long int PrevHighArt;

   if(CommDoc) {

      switch(CommState) {
         case ST_NONE:
            break;

         case ST_ESTABLISH_COMM:
            retcode = 0;
            sscanf(CommLineIn,"%u",&retcode);
            if(retcode == 500) {
               dolist = DoList;
               if(dolist == ID_DOLIST_ASK-ID_DOLIST_BASE) {
                  dolist = DialogBox(hInst,"WinVnDoList",hWndConf,lpfnWinVnDoListDlg);
               }
               if(dolist) {
                  StartList();
               } else {
                  CommState = ST_NONE;
                  CommBusy = FALSE;
                  Initializing = INIT_DONE;
               }

               InvalidateRect(hWndConf,NULL,FALSE);
            }
            break;

         case ST_LIST_RESP:
            retcode = 0;
            sscanf(CommLineIn,"%d",&retcode);
            if(retcode != 215) break;
            CommState = ST_LIST_GROUPLINE;
            RcvLineCount = 0;
            break;

         case ST_LIST_GROUPLINE:
            if(strcmp(CommLineIn,".") == 0) {
               CommState = ST_NONE;
               CommBusy = FALSE;
               Initializing = INIT_DONE;
               InvalidateRect(hWndConf,NULL,FALSE);

               ProcEndList();
            } else {
               ProcListLine((unsigned char *)CommLineIn);
            }
            break;

         case ST_GROUP_RESP:
            retcode = 0;
            sscanf(CommLineIn,"%u %u %ld %ld %s",&retcode,&estnum,&first,&last,group);
            if(retcode < 100) break;
            LockLine(CommDoc->hParentBlock,CommDoc->ParentOffset,CommDoc->ParentLineID,&BlockPtr,&LinePtr);
            (((TypGroup far *) ((char far *) LinePtr + sizeof(TypLine)) )->ServerEstNum) = estnum;
            (((TypGroup far *) ((char far *) LinePtr + sizeof(TypLine)) )->ServerFirst ) = first;
            (((TypGroup far *) ((char far *) LinePtr + sizeof(TypLine)) )->ServerLast  ) = last;
            GlobalUnlock(BlockPtr->hCurBlock);
            /* Note:  the output from the GROUP command evidently
             * can't always be relied upon; there may be more articles
             * beyond the last article it reports.  So, I will just
             * use a very large last article number here.
             */
            mylen = sprintf(mybuf,"XHDR subject %ld-%ld\r",first,999999L);
            PutCommLine(mybuf,mylen);
            CommState = ST_XHDR_RESP;
            break;

         case ST_XHDR_RESP:
            retcode = 0;
            sscanf(CommLineIn,"%d",&retcode);
            if(retcode < 100) break;
            CommState = ST_XHDR_SUBJ;
            break;

         case ST_XHDR_SUBJ:
            if(strcmp(CommLineIn,".") == 0) {
               CommState = ST_IN_GROUP;
               CommBusy = FALSE;
               /* Fetch this group's line in NetDoc so we can get the
                * group's name for the window's title bar.
                */
               LockLine(CommDoc->hParentBlock,CommDoc->ParentOffset,CommDoc->ParentLineID,&BlockPtr,&LinePtr);
               lpsz = (char far *) ( ((char far *)LinePtr) +
                sizeof(TypLine)+ sizeof(TypGroup) ) ;
               mylstrncpy(group,lpsz,MAXGROUPNAME);
               sprintf(mybuf,"%s (%u articles)",group,CommDoc->TotalLines);
               SetWindowText(CommDoc->hDocWnd,mybuf);

               /* If we have information from NEWSRC on the highest-
                * numbered article previously seen, position the window
                * so the new articles can be seen without scrolling.
                */

               PrevHighArt = ((TypGroup far *)(lpsz - sizeof(TypGroup)))->HighestPrevSeen;
               GlobalUnlock(BlockPtr->hCurBlock);

               if(PrevHighArt && CommDoc->TotalLines > CommDoc->ScYLines
                 && !CommDoc->TopScLineID) {
                  TopOfDoc(CommDoc,&BlockPtr,&LinePtr);
                  found = FALSE;
                  lineord = 0;
                  do {
                     MyArtPtr = (TypArticle far *)((char far *)LinePtr + sizeof(TypLine));
                     if(MyArtPtr->Number > PrevHighArt) {
                        found = TRUE;
                        break;
                     }
                     if(!NextLine(&BlockPtr,&LinePtr)) break;
                     lineord++;
                  } while(!found);

                  /* If the line is in the last screen's worth of lines, back
                   * up the pointer so it points to the first line of the last
                   * screen.
                   */
                  if(found) {
                     AdjustTopSc(BlockPtr,LinePtr);
#if 0
                        while(lineord > CommDoc->TotalLines - CommDoc->ScYLines) {
                        PrevLine(&BlockPtr,&LinePtr);
                        lineord--;
                     }
                     UnlockLine(BlockPtr,LinePtr,&(CommDoc->hCurTopScBlock),
                       &(CommDoc->TopScOffset),&(CommDoc->TopScLineID));
                     CommDoc->TopLineOrd = lineord;
#endif
                  } else {
                     UnlockLine(BlockPtr,LinePtr,&hBlock,&Offset,&MyLineID);
                  }
               }
               InvalidateRect(CommDoc->hDocWnd,NULL,FALSE);
            } else {
               artnum = 0;
               sscanf(CommLineIn,"%ld",&artnum);
               artptr = (TypArticle *) (artline + sizeof(TypLine));
               if(artnum) {
                  LockLine(CommDoc->hParentBlock,CommDoc->ParentOffset,CommDoc->ParentLineID,&BlockPtr,&LinePtr);
                  artptr->Number  = artnum;
                  artptr->Seen    = (char)
                    WasArtSeen(artnum,(TypGroup far *)( ((char far *)LinePtr) + sizeof(TypLine) ) );
                  artptr->Selected= FALSE ;
                  artptr->ArtDoc  = (TypDoc *) NULL;
                  UnlockLine(BlockPtr,LinePtr,&(CommDoc->hParentBlock),&(CommDoc->ParentOffset),&(CommDoc->ParentLineID));

                  /* Skip past the leading article number and space. */

                  for(cptr=CommLineIn; isdigit(*cptr); cptr++);
                  for(;*cptr == ' ';cptr++);

                  /* Copy the rest of the received into the line
                   * to be placed in the textblock.
                   * Locate the place in the document to which we
                   * are adding lines, and add this new line.
                   */
                  cdest = artline+sizeof(TypLine)+sizeof(TypArticle);
                  for(artptr->NameLen=0;*(cdest++) = *(cptr++);
                    (artptr->NameLen)++);
                  *cdest = ' ';
                  mylen = (cdest-artline) + sizeof(int);
                  mylen += mylen%2;
                  ((TypLine *)artline)->length = mylen;
                  ((TypLine *)artline)->LineID = NextLineID++;
                  *( (int *) (artline+mylen-sizeof(int)) ) = mylen;
                  LockLine(CommDoc->hCurAddBlock,CommDoc->AddOffset,CommDoc->AddLineID,&BlockPtr,&LinePtr);
                  AddLine((TypLine *)artline,&BlockPtr,&LinePtr);
                  UnlockLine(BlockPtr,LinePtr,&(CommDoc->hCurAddBlock),
                   &(CommDoc->AddOffset),&(CommDoc->AddLineID));

                  /* Cause the window to be repainted.
                   * Also, every UPDATE_TITLE_FREQ lines, update the
                   * window title to let the user know how far we
                   * have gotten.
                   */
                  InvalidateRect(CommDoc->hDocWnd,NULL,FALSE);
                  if((++RcvLineCount)%UPDATE_TITLE_FREQ == 0) {
                     sprintf(mybuf,"Retrieving %uth article of ",RcvLineCount);
                     LockLine(CommDoc->hParentBlock,CommDoc->ParentOffset,CommDoc->ParentLineID,&BlockPtr,&LinePtr);
                     lpsz = (char far *) ( ((char far *)LinePtr) +
                      sizeof(TypLine)+ sizeof(TypGroup) ) ;
                     lstrcat(mybuf,lpsz);
                     SetWindowText(CommDoc->hDocWnd,mybuf);
                     GlobalUnlock(BlockPtr->hCurBlock);
                 /*
                  *  if(++TimesWndUpdated <= MAX_IMMEDIATE_UPDATE) {
                  *     InvalidateRect(CommDoc->hDocWnd,NULL,FALSE);
                  *  }
                  */
                     UpdateWindow(CommDoc->hDocWnd);
                  }
               }
            }

            break;

         case ST_IN_GROUP:
            break;

         case ST_ARTICLE_RESP:
            retcode = 0;
            sscanf(CommLineIn,"%d",&retcode);
            if(retcode < 100) break;
            CommState = ST_REC_ARTICLE;
            break;

         case ST_REC_ARTICLE:
            if(strcmp(CommLineIn,".") == 0) {
               CommState = ST_IN_GROUP;
               CommBusy = FALSE;
               LockLine(CommDoc->hParentBlock,CommDoc->ParentOffset,CommDoc->ParentLineID,&BlockPtr,&LinePtr);
               lpsz = (char far *) ( ((char far *)LinePtr) +
                sizeof(TypLine)+ sizeof(TypArticle) ) ;
               mylstrncpy(group,lpsz,MAXGROUPNAME);
               sprintf(mybuf,"%s (%u lines)",group,CommDoc->TotalLines);
               SetWindowText(CommDoc->hDocWnd,mybuf);
               InvalidateRect(CommDoc->hDocWnd,NULL,FALSE);
               GlobalUnlock(BlockPtr->hCurBlock);

               /* Skip to the first line of the text of the article
                * and make sure it's visible on the screen.  This is
                * so that the user doesn't have to have the first
                * screen filled with a lengthy, worthless header.
                */
               if(CommDoc->TotalLines > CommDoc->ScYLines
                 && !CommDoc->TopScLineID) {
                  TopOfDoc(CommDoc,&BlockPtr,&LinePtr);
                  found = FALSE;
                  do {
                     lpsz = ((char far *)LinePtr + sizeof(TypLine) + sizeof(TypText));
                     if(IsLineBlank(lpsz)) {
                        found = TRUE;
                        break;
                     }
                     if(!NextLine(&BlockPtr,&LinePtr)) break;
                  } while(!found);
                  NextLine(&BlockPtr,&LinePtr);

                  /* If the line is in the last screen's worth of lines, back
                   * up the pointer so it points to the first line of the last
                   * screen.
                   */
                  if(found) {
                     AdjustTopSc(BlockPtr,LinePtr);
                  } else {
                     UnlockLine(BlockPtr,LinePtr,&hBlock,&Offset,&MyLineID);
                  }
               }

            } else {
               /* Copy this line into an image of a textblock line,
                * expanding tabs.
                */
               cdest = artline+sizeof(TypLine)+sizeof(TypText);
               for(col=0,cptr=CommLineIn; *cptr &&
                col<(MAXINTERNALLINE-3*sizeof(TypLine)-sizeof(TypText)); cptr++) {
                  if(*cptr == '\t') {
                     do {
                        *(cdest++) = ' ';
                     } while (++col & 7);
                  } else {
                     *(cdest++) = *cptr;
                     col++;
                  }
               }
               *(cdest++) = '\0';

               mylen = (cdest-artline) + sizeof(int);
               mylen += mylen%2;
               ((TypText *)(artline+sizeof(TypLine)))->NameLen =
                (cdest-1) - (artline+sizeof(TypLine)+sizeof(TypText));
               ((TypLine *)artline)->length = mylen;
               ((TypLine *)artline)->LineID = NextLineID++;
               *( (int *) (artline+mylen-sizeof(int)) ) = mylen;
               LockLine(CommDoc->hCurAddBlock,CommDoc->AddOffset,CommDoc->AddLineID,&BlockPtr,&LinePtr);
               AddLine((TypLine *)artline,&BlockPtr,&LinePtr);
               UnlockLine(BlockPtr,LinePtr,&(CommDoc->hCurAddBlock),
                &(CommDoc->AddOffset),&(CommDoc->AddLineID));
               if((CommDoc->TotalLines % UPDATE_ART_FREQ) == 0) {
            InvalidateRect(CommDoc->hDocWnd,NULL,FALSE);  
               }
            }

            break;

         case ST_POST_WAIT_PERMISSION:
            for(ih=0,found=FALSE; !found && ih<MAXPOSTWNDS; ih++) {
              if(&(PostingDocs[ih]) == CommDoc) {
#ifndef MAC
                 hWndPostEdit = hWndPostEdits[ih];
#else
  /* mrr add here */
#endif
                 found = TRUE;
                 break;
              }
            }
            retcode = 0;
            sscanf(CommLineIn,"%d",&retcode);
            if(retcode <= 0) {
               break;
            } else if(retcode == 340) {
               PostText(ih,DOCTYPE_POSTING);
            } else {
               MessageBox(hWndPostEdit,CommLineIn+4,"Cannot Post Article",
                  MB_OK|MB_ICONEXCLAMATION);
               CommBusy = FALSE;
               CommState = ST_NONE;
            }

            break;

         case ST_POST_WAIT_END:
            for(ih=0,found=FALSE; !found && ih<MAXPOSTWNDS; ih++) {
              if(&(PostingDocs[ih]) == CommDoc) {
#ifndef MAC
                 hWndPostEdit = hWndPostEdits[ih];
#else
  /* mrr add here */
#endif
                 found = TRUE;
                 break;
              }
            }
            retcode = 0;
            sscanf(CommLineIn,"%d",&retcode);
            if(retcode == 441 || retcode == 440) {
               cptr = "Posting Failed";
               mbcode = MB_OK|MB_ICONEXCLAMATION;
               done = TRUE;
            } else if(retcode == 240) {
               cptr = "Article Posted OK";
               mbcode = MB_OK;
               done = TRUE;
            }
            if(done) {
               CommBusy = FALSE;
               CommState = ST_NONE;
               MessageBox(hWndPostEdit,CommLineIn+4,cptr,mbcode);
            }
            break;

         case ST_MAIL_WAIT_PERMISSION:
            for(ih=0,found=FALSE; !found && ih<MAXMAILWNDS; ih++) {
              if(&(MailDocs[ih]) == CommDoc) {
#ifndef MAC
                 hWndPostEdit = hWndMailEdits[ih];
#else
  /* mrr add here */
#endif
                 found = TRUE;
                 break;
              }
            }
            retcode = 0;
            sscanf(CommLineIn,"%d",&retcode);
            if(retcode <= 0) {
               break;
            } else if(retcode == 350) {
               PostText(ih,DOCTYPE_MAIL);
            } else {
               MessageBox(hWndPostEdit,CommLineIn+4,"Cannot Mail Message",
                  MB_OK|MB_ICONEXCLAMATION);
               CommBusy = FALSE;
               CommState = ST_NONE;
            }

            break;

         case ST_MAIL_WAIT_END:
            for(ih=0,found=FALSE; !found && ih<MAXMAILWNDS; ih++) {
              if(&(MailDocs[ih]) == CommDoc) {
#ifndef MAC
                 hWndPostEdit = hWndMailEdits[ih];
#else
  /* mrr add here */
#endif
                 found = TRUE;
                 break;
              }
            }
            retcode = 0;
            sscanf(CommLineIn,"%d",&retcode);
            if(retcode == 451 || retcode == 450) {
               cptr = "Mailing Failed";
               mbcode = MB_OK|MB_ICONEXCLAMATION;
               done = TRUE;
            } else if(retcode == 250) {
               cptr = "Message sent OK";
               mbcode = MB_OK;
               done = TRUE;
            }
            if(done) {
               CommBusy = FALSE;
               CommState = ST_NONE;
               MessageBox(hWndPostEdit,CommLineIn+4,cptr,mbcode);
            }
            break;

      case ST_GROUP_REJOIN:
         CommState = ST_ARTICLE_RESP;
         break;
      }
   }
}

/*-- function WasArtSeen ---------------------------------------------
 *
 *  Determines whether (according to the information in a TypGroup entry)
 *  a given article number was seen.
 *
 *  Returns TRUE iff the article has been seen.
 */
BOOL
WasArtSeen(ArtNum,GroupPtr)
long int ArtNum;
TypGroup far *GroupPtr;
{
   TypRange far *RangePtr = (TypRange far *) ((char far *)
      GroupPtr + RangeOffset(GroupPtr->NameLen));
   int nr;

   for(nr=0; nr < GroupPtr->nRanges; nr++) {
      if(ArtNum >= RangePtr->First && ArtNum <= RangePtr->Last) {
         return(TRUE);
      } else {
         RangePtr++;
      }
   }
   return(FALSE);
}

/*--- function mylstrncmp -----------------------------------------------
 *
 *   Just like strncmp, except takes long pointers.
 */
int
mylstrncmp(ptr1,ptr2,len)
char far *ptr1;
char far *ptr2;
int len;
{
   for(;len--;ptr1++,ptr2++) {
      if(*ptr1 > *ptr2) {
         return(1);
      } else if(*ptr1 < *ptr2) {
         return(-1);
      }
   }
   return(0);
}

/*--- function mylstrncpy -----------------------------------------------
 *
 *   Just like strncpy, except takes long pointers.
 */
char far *
mylstrncpy(ptr1,ptr2,len)
char far *ptr1;
char far *ptr2;
int len;
{
   char far *targ = ptr1;

   for(; --len && *ptr2; ptr1++,ptr2++) {
      *ptr1 = *ptr2;
   }
   *ptr1 = '\0';
   return(targ);
}

#if 0
/*--- function lstrcmpnoblank ------------------------------------------
 *
 *   Like strcmp, except takes long pointers and also stops at
 *   the first blank.
 */
int
lstrcmpnoblank(str1,str2)
char far **str1;
char far **str2;
{
   register char far *s1=*str1, far *s2=*str2;

	for(;*s1 && *s2 && *s1!=' ' && *s2!=' '; s1++,s2++) {
		if(*s1 > *s2) {
			return (1);
		} else if(*s1 < *s2) {
			return (-1);
		}
	}
	if(*s1 == *s2) {
		return(0);
	} else if(*s1) {
		return(1);
	} else {
		return(-1);
	}
}
#endif
