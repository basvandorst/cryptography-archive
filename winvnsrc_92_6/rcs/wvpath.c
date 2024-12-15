head	 1.1;
branch   ;
access   ;
symbols  ;
locks    ;
comment  @ * @;


1.1
date	 94.09.18.22.36.45;  author jcooper;  state Exp;
branches ;
next	 ;


desc
@Dialog handler for WinVNSelectPath with folder/disk bitmaps, etc
@



1.1
log
@Initial revision
@
text
@/*
 * wvpath.c
 *
 * based on ddlist example is MSDN (Copyright (c)1992 Kraig Brockschmidt)
 *
 * John S. Cooper
 * 9/10/94
 */

#include <windows.h>
#include <windowsx.h>
#include "wvglob.h"
#include "winvn.h"
#pragma hdrstop
#include <dos.h>
#include <io.h>			// for _access
#include <stdlib.h>
#include <stdio.h>
#include <direct.h>
#include <string.h>

/* forward declarations */

void DriveListInitialize(HWND, HWND);
UINT DriveType(UINT);
void DirectoryListInitialize(HWND, HWND, LPSTR);
void DriveDirDrawItem(LPDRAWITEMSTRUCT, BOOL);
void TransparentBlt(HDC, UINT, UINT, HBITMAP, COLORREF);

//Special ROP code for TransparentBlt.
#define ROP_DSPDxax  0x00E20746

/*
 * DriveListInitialize
 *
 * Purpose:
 *  Resets and fills the given combobox with a list of current
 *  drives.  The type of drive is stored with the item data.
 *
 * Parameters:
 *  hList           HWND of the combobox to fill.
 *  hTempList       HWND to use for finding drives.
 *
 * Return Value:
 *  None
 */

void DriveListInitialize(HWND hList, HWND hTempList)
    {
    struct find_t   fi;
    char            ch;
    UINT            i, iItem;
    UINT            cch, cItems;
    UINT            iDrive, iType;
    UINT            iCurDrive;
    char            szDrive[10];
    char            szNet[64];
    char            szItem[26];

    if (NULL==hList)
        return;

    //Clear out all the lists.
    SendMessage(hList,     CB_RESETCONTENT, 0, 0L);
    SendMessage(hTempList, LB_RESETCONTENT, 0, 0L);

    //Get available drive letters in the temp list
    SendMessage(hTempList, LB_DIR, DDL_DRIVES | DDL_EXCLUSIVE, (LONG)(LPSTR)"*");

    iCurDrive=_getdrive()-1;       //Fix for zero-based drive indexing

    /*
     * Walk through the list of drives, parsing off the "[-" and "-]"
     * For each drive letter parsed, add a string to the combobox
     * composed of the drive letter and the volume label.  We then
     * determine the drive type and add that information as the item data.
     */
    cItems=(int)SendMessage(hTempList, LB_GETCOUNT, 0, 0L);

    for (i=0; i<cItems;  i++)
        {
        SendMessage(hTempList, LB_GETTEXT, i, (LONG)(LPSTR)szDrive);

        //Insure lowercase drive letters
        AnsiLower(szDrive);
        iDrive=szDrive[2]-'a';
        iType=DriveType(iDrive);        //See Below

        if (iType < 2)                  //Skip non-existent drive B's
            continue;

        //Start the item string with the drive letter, color, and two spaces
        wsprintf(szItem, "%c%s", szDrive[2], (LPSTR)":  ");

        /*
         * For fixed or ram disks, append the volume label which we find
         * using _dos_findfirst and attribute _A_VOLID.
         */
        if (DRIVE_FIXED==iType || DRIVE_RAM==iType)
            {
            wsprintf(szDrive, "%c:\\*.*", szDrive[2]);

            if (0==_dos_findfirst(szDrive, _A_VOLID, &fi))
                {
                //Convert volume to lowercase and strip any . in the name.
                AnsiLower(fi.name);

                //If a period exists, it has to be in position 8, so clear it.
                ch=fi.name[8];
                fi.name[8]=0;
                lstrcat(szItem, fi.name);

                //If we did have a broken volume name, append the last 3 chars
                if ('.'==ch)
                    lstrcat(szItem, &fi.name[9]);
                }
            }

        //For network drives, go grab the \\server\share for it.
        if (DRIVE_REMOTE==iType)
            {
            szNet[0]=0;
            cch=sizeof(szNet);

            wsprintf(szDrive, "%c:", szDrive[2]);
            AnsiUpper(szDrive);

            WNetGetConnection((LPSTR)szDrive, (LPSTR)szNet, &cch);
            AnsiLower(szNet);

            lstrcat(szItem, szNet);
            }

        iItem=(int)SendMessage(hList, CB_ADDSTRING, 0, (LONG)(LPSTR)szItem);
        SendMessage(hList, CB_SETITEMDATA, iItem, MAKELONG(iDrive, iType));

        if (iDrive==iCurDrive)
            SendMessage(hList, CB_SETCURSEL, iItem, 0L);
        }

    return;
    }

/*
 * DriveType
 *
 * Purpose:
 *  Augments the Windows API GetDriveType with a call to the CD-ROM
 *  extensions to determine if a drive is a floppy, hard disk, CD-ROM,
 *  RAM-drive, or networked  drive.
 *
 * Parameters:
 *  iDrive          UINT containing the zero-based drive index
 *
 * Return Value:
 *  UINT            One of the following values describing the drive:
 *                  DRIVE_FLOPPY, DRIVE_HARD, DRIVE_CDROM, DRIVE_RAM,
 *                  DRIVE_NETWORK.
 */

UINT DriveType(UINT iDrive)
    {
    int     iType;
    BOOL    fCDROM=FALSE;
    BOOL    fRAM=FALSE;


    //Validate possible drive indices
    if (0 > iDrive  || 25 < iDrive)
        return 0xFFFF;

    iType=GetDriveType(iDrive);

    /*
     * Under Windows NT, GetDriveType returns complete information
     * not provided under Windows 3.x which we now get through other
     * means.
     */
   #ifdef WIN32
    return iType;
   #else
    //Check for CDROM on FIXED and REMOTE drives only
    if (DRIVE_FIXED==iType || DRIVE_REMOTE==iType)
        {
        _asm
            {
            mov     ax,1500h        //Check if MSCDEX exists
            xor     bx,bx
            int     2fh

            or      bx,bx           //BX unchanged if MSCDEX is not around
            jz      CheckRAMDrive   //No?  Go check for RAM drive.

            mov     ax,150Bh        //Check if drive is using CD driver
            mov     cx,iDrive
            int     2fh

            mov     fCDROM,ax       //AX if the CD-ROM flag
            or      ax,ax
            jnz     Exit            //Leave if we found a CD-ROM drive.

            CheckRAMDrive:
            }
        }

    //Check for RAM drives on FIXED disks only.
    if (DRIVE_FIXED==iType)
        {
        /*
         * Check for RAM drive is done by reading the boot sector and
         * looking at the number of FATs.  Ramdisks only have 1 while
         * all others have 2.
         */
        _asm
            {
            push    ds

            mov     bx,ss
            mov     ds,bx

            sub     sp,0200h            //Reserve 512 bytes to read a sector
            mov     bx,sp               //and point BX there.

            mov     ax,iDrive           //Read the boot sector of the drive
            mov     cx,1
            xor     dx,dx

            int     25h
            add     sp,2                //Int 25h requires our stack cleanup.
            jc      DriveNotRAM

            mov     bx,sp
            cmp     ss:[bx+15h],0f8h    //Reverify fixed disk
            jne     DriveNotRAM
            cmp     ss:[bx+10h],1       //Check if there's only one FATs
            jne     DriveNotRAM
            mov     fRAM,1

            DriveNotRAM:
            add     sp,0200h
            pop     ds

            Exit:
            //Leave fRAM untouched  it's FALSE by default.
            }
        }

    /*
     * If either CD-ROM or RAM drive flags are set, return privately
     * defined flags for them (outside of Win32).  Otherwise return
     * the type given from GetDriveType.
     */

    if (fCDROM)
        return DRIVE_CDROM;

    if (fRAM)
        return DRIVE_RAM;

    //Drive B on a one drive system returns < 2 from GetDriveType.
    return iType;
   #endif
    }

/*
 * DirectoryListInitialize
 *
 * Purpose:
 *  Initializes strings in a listbox given a directory path.  The first
 *  string in the listbox is the drive letter.  The remaining items are
 *  each directory in the path completed with a listing of all sub-
 *  directories under the last directory in the path.
 *
 * Parameters:
 *  hList           HWND of the listbox to fill.
 *  hTempList       HWND of a listbox to use for  directory enumerations.
 *  pszDir          LPSTR of the path to use in initialization assumed
 *                  to be at least _MAX_DIR long.
 *
 * Return Value:
 *  None
 */

void DirectoryListInitialize(HWND hList, HWND hTempList, LPSTR pszDir)
    {
    LPSTR       psz, pszLast;
    char        ch;
    char        szDir[_MAX_DIR];
    UINT        cch, cItems;
    UINT        i, iItem, iIndent;
    BOOL        fFirst=TRUE;

    if (NULL==hList || NULL==pszDir)
        return;

    //If the path ends in a \, strip the '\'
    cch=lstrlen(pszDir);

    if ('\\'==pszDir[cch-1])
        pszDir[cch-1]=0;

    //Clear out all the lists.
    SendMessage(hList,     WM_SETREDRAW,    FALSE, 0L);
    SendMessage(hList,     LB_RESETCONTENT, 0,     0L);
    SendMessage(hTempList, LB_RESETCONTENT, 0,     0L);

    /*
     * Walk through the path one \ at a time.  At each one found,
     * we add the string composed of the characters between it and
     * the last \ found.  We also make sure everything is lower case.
     */

    pszLast=AnsiLower(pszDir);

    //Save this for changing directories
    SetWindowText(hList, pszDir);

    //Save the directory appended with \*.*
    wsprintf(szDir, "%s\\*.*", pszDir);

    while (TRUE)
        {
        psz=_fstrchr(pszLast, '\\');

        if (NULL!=psz)
            {
            /*
             * Save the character here so we can NULL terminate.  If this
             * if the first entry, it's a drive root, so keep the \
             */

            if (fFirst)
                ch=*(++psz);
            else
                ch=*psz;

            *psz=0;
            }
        else
            {
            //If we're looking at a drive only, then append a backslash
            if (pszLast==pszDir && fFirst)
                lstrcat(pszLast, "\\");
            }

        //Add the drive string--includes the last one where psz==NULL
        iItem=(UINT)SendMessage(hList, LB_ADDSTRING, 0, (LONG)pszLast);

        /*
         * The item data here has in the HIWORD the bitmap to use for
         * the item with the LOWORD containing the indentation.  The
         * bitmap is IDB_FOLDEROPEN for anything above the current
         * directory (that is, c:\foo is above than c:\foo\bar),
         * IDB_FOLDERCLOSED for anything below the current, and
         * IDB_FOLDEROPENSELECT for the current directory.
         */

        i=(NULL!=psz) ? IDB_FOLDEROPEN : IDB_FOLDEROPENSELECT;
        SendMessage(hList, LB_SETITEMDATA, iItem, MAKELONG(iItem, i));

        if (NULL==psz)
            break;

        //Restore last character.
        *psz=ch;
        psz+=(fFirst) ? 0 : 1;

        fFirst=FALSE;
        pszLast=psz;
        }


    /*
     * Now that we have the path in, enumerate the subdirectories here
     * and place them in the list at the indentation iItem+1 since iItem
     * was the index of the last item in the path added to the list.
     *
     * To enumerate the directories, we send LB_DIR to an alternate
     * listbox.  On return, we have to parse off the brackets around
     * those directory names before bringing them into this listbox.
     */

    iIndent=iItem+1;

    //Get available directories; szDir is pszDir\*.*
    SendMessage(hTempList, LB_DIR, DDL_DIRECTORY | DDL_EXCLUSIVE
                , (LONG)(LPSTR)szDir);

    cItems=(int)SendMessage(hTempList, LB_GETCOUNT, 0, 0L);

    for (i=0; i<cItems; i++)
        {
        cch=(UINT)SendMessage(hTempList, LB_GETTEXT, i, (LONG)(LPSTR)szDir);

        //Skip directories beginning with . (skipping . and ..)
        if ('.'==szDir[1])
            continue;

        //Remove the ending ']'
        szDir[cch-1]=0;

        //Add the string to the real directory list.
        iItem=(UINT)SendMessage(hList, LB_ADDSTRING, 0
                                , (LONG)(LPSTR)(szDir+1));
        SendMessage(hList, LB_SETITEMDATA, iItem
                    , MAKELONG(iIndent, IDB_FOLDERCLOSED));
        }

    //Force a listbox repaint.
    SendMessage(hList, WM_SETREDRAW, TRUE, 0L);
    InvalidateRect(hList, NULL, TRUE);

    /*
     * If there's a vertical scrollbar, then we've added more items than
     * are visible at once.  To meet the UI specifications, we're supposed
     * to make the next directory up the top visible one.
     */
    GetScrollRange(hList, SB_VERT, (LPINT)&i, (LPINT)&iItem);

    if (!(0==i && 0==iItem))
        SendMessage(hList, LB_SETTOPINDEX, max((int)(iIndent-2), 0), 0L);

    //Last thing is to set the current directory as the selection
    SendMessage(hList, LB_SETCURSEL, iIndent-1, 0L);
    return;
    }

/*
 * DriveDirDrawItem
 *
 * Purpose:
 *  Handles WM_DRAWITEM for both drive and directory listboxes.
 *
 * Parameters:
 *  pDI             LPDRAWITEMSTRUCT passed with the WM_DRAWITEM message.
 *  fDrive          BOOL TRUE to draw a drive, FALSE to draw directory.
 *
 * Return Value:
 *  None
 */

void DriveDirDrawItem(LPDRAWITEMSTRUCT pDI, BOOL fDrive)
    {
    char        szItem[40];
    int         iType=0;
    int         iIndent=0;
    UINT        uMsg;
    DWORD       dw;
    BITMAP      bm;
    COLORREF    crText, crBack;
    HBITMAP     hBmp;

    if ((int)pDI->itemID < 0)
        return;

    if (fDrive)
        dw=SendMessage(pDI->hwndItem, CB_GETITEMDATA, pDI->itemID, 0L);

    //Get the text string for this item (controls have different messages)
    uMsg=(fDrive) ? CB_GETLBTEXT : LB_GETTEXT;
    SendMessage(pDI->hwndItem, uMsg, pDI->itemID, (LONG)(LPSTR)szItem);

    if ((ODA_DRAWENTIRE | ODA_SELECT) & pDI->itemAction)
        {
        if (ODS_SELECTED & pDI->itemState)
            {
            //Select the appropriate text colors
            crText=SetTextColor(pDI->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
            crBack=SetBkColor(pDI->hDC, GetSysColor(COLOR_HIGHLIGHT));
            }

        /*
         * References to the two bitmap arrays here are the only external
         * dependencies of this code.  To keep it simple, we didn't use
         * a more complex scheme like putting all the images into one bitmap.
         */
        if (fDrive)
            {
            //For drives, get the type, which determines the bitmap.
            iType=(int)HIWORD(dw);
            hBmp=DiskBitmaps[iType-IDB_DRIVEMIN];
            }
        else
            {
            //For directories, indentation level is 4 pixels per indent.
            iIndent=4*(1+LOWORD(pDI->itemData));
            hBmp=FolderBitmaps[HIWORD(pDI->itemData)-IDB_FOLDERMIN];
            }

        GetObject(hBmp, sizeof(bm), &bm);

        /*
         * Paint the text and the rectangle in whatever colors.  If
         * we're drawing drives, iIndent is zero so it's ineffective.
         */
        ExtTextOut(pDI->hDC, pDI->rcItem.left+bm.bmWidth+4+iIndent
                   , pDI->rcItem.top, ETO_OPAQUE, &pDI->rcItem, szItem
                   , lstrlen(szItem), (LPINT)NULL);


        //Go draw the bitmap we want.
        TransparentBlt(pDI->hDC, pDI->rcItem.left+iIndent
                       , pDI->rcItem.top, hBmp, RGB(0,0,255));

        //Restore original colors if we changed them above.
        if (ODS_SELECTED & pDI->itemState)
            {
            SetTextColor(pDI->hDC, crText);
            SetBkColor(pDI->hDC,   crBack);
            }
        }

    if ((ODA_FOCUS & pDI->itemAction) || (ODS_FOCUS & pDI->itemState))
        DrawFocusRect(pDI->hDC, &pDI->rcItem);

    return;
    }

/*
 * TransparentBlt
 *
 *  Given a DC, a bitmap, and a color to assume as transparent in that
 *  bitmap, BitBlts the bitmap to the DC letting the existing background
 *  show in place of the transparent color.
 *
 * Parameters:
 *  hDC             HDC on which to draw.
 *  x, y            UINT location at which to draw the bitmap
 *  hBmp            HBITMIP to draw
 *  cr              COLORREF to consider as transparent.
 */

void TransparentBlt(HDC hDC, UINT x, UINT y, HBITMAP hBmp, COLORREF cr)
    {
    HDC         hDCSrc, hDCMid, hMemDC;
    HBITMAP     hBmpMono, hBmpT;
    HBRUSH      hBr, hBrT;
    COLORREF    crBack, crText;
    BITMAP      bm;

    if (NULL==hBmp)
        return;

    GetObject(hBmp, sizeof(bm), &bm);

    //Get three intermediate DC's
    hDCSrc=CreateCompatibleDC(hDC);
    hDCMid=CreateCompatibleDC(hDC);
    hMemDC=CreateCompatibleDC(hDC);

    SelectObject(hDCSrc, hBmp);

    //Create a monochrome bitmap for masking
    hBmpMono=CreateCompatibleBitmap(hDCMid, bm.bmWidth, bm.bmHeight);
    SelectObject(hDCMid, hBmpMono);

    //Create a middle bitmap
    hBmpT=CreateCompatibleBitmap(hDC, bm.bmWidth, bm.bmHeight);
    SelectObject(hMemDC, hBmpT);


    //Create a monochrome mask where we have 0's in the image, 1's elsewhere.
    crBack=SetBkColor(hDCSrc, cr);
    BitBlt(hDCMid, 0, 0, bm.bmWidth, bm.bmHeight, hDCSrc, 0, 0, SRCCOPY);
    SetBkColor(hDCSrc, crBack);

    //Put the unmodified image in the temporary bitmap
    BitBlt(hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, hDCSrc, 0, 0, SRCCOPY);

    //Create an select a brush of the background color
    hBr=CreateSolidBrush(GetBkColor(hDC));
    hBrT=SelectObject(hMemDC, hBr);

    //Force conversion of the monochrome to stay black and white.
    crText=SetTextColor(hMemDC, 0L);
    crBack=SetBkColor(hMemDC, RGB(255, 255, 255));

    /*
     * Where the monochrome mask is 1, Blt the brush; where the mono mask
     * is 0, leave the destination untouches.  This results in painting
     * around the image with the background brush.  We do this first
     * in the temporary bitmap, then put the whole thing to the screen.
     */
    BitBlt(hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, hDCMid, 0, 0, ROP_DSPDxax);
    BitBlt(hDC,    x, y, bm.bmWidth, bm.bmHeight, hMemDC, 0, 0, SRCCOPY);


    SetTextColor(hMemDC, crText);
    SetBkColor(hMemDC, crBack);

    SelectObject(hMemDC, hBrT);
    DeleteObject(hBr);

    DeleteDC(hMemDC);
    DeleteDC(hDCSrc);
    DeleteDC(hDCMid);
    DeleteObject(hBmpT);
    DeleteObject(hBmpMono);

    return;
    }

/*
 * WinVnSelectPathDialogDlg
 *
 *  Handles the dialog that displays a drive and directory owner-draw
 *  combobox and listbox, respectiviely.  This procedure handles only
 *  the WM_INITDIALOG and WM_DRAWITEM messages,dispatching them to
 *  alternate functions that handle each message for each list.
 *
 */

BOOL FAR PASCAL WinVnSelectPathDlg(HWND hDlg,unsigned iMessage,WPARAM wParam,LPARAM lParam)
    {

    char cwd[_MAX_PATH];

    WORD            wID;
    WORD            wCode;
    HWND            hWndMsg;

    hWndMsg=(HWND)(UINT)lParam;
    wID=LOWORD(wParam);
    #ifdef WIN32
        wCode=HIWORD(wParam);
    #else
        wCode=HIWORD(lParam);
    #endif

    switch (iMessage)
        {
        case WM_INITDIALOG:
            ShowWindow(hDlg, SW_SHOW);
            UpdateWindow(hDlg);

            DriveListInitialize(GetDlgItem(hDlg, ID_DRIVELIST), GetDlgItem(hDlg, ID_TEMPLIST));

			if (lParam	&& _access((char *)lParam, 0) >= 0) {
				strcpy (cwd, (char *)lParam);
				_chdir(cwd);
			} else
	           	_getcwd(cwd, _MAX_PATH);
            
            DirectoryListInitialize(GetDlgItem(hDlg, ID_DIRECTORYLIST), GetDlgItem(hDlg, ID_TEMPLIST), cwd);
            return TRUE;

        case WM_MEASUREITEM:
            {
            static int              cyItem=-1;      //Height of a listbox item
            LPMEASUREITEMSTRUCT     pMI;

            pMI=(LPMEASUREITEMSTRUCT)lParam;

            if (-1==cyItem)
                {
                HFONT       hFont;
                HDC         hDC;
                TEXTMETRIC  tm;
                BITMAP      bm;

                /*
                 * Attempt to get the font of the dialog. However,
                 * on the first attempt in the life of the dialog,
                 * this could fail; in that case use the system font.
                 */
                hFont=(HANDLE)(UINT)SendMessage(hDlg, WM_GETFONT, 0, 0L);

                if (NULL==hFont)
                    hFont=GetStockObject(SYSTEM_FONT);

                hDC=GetDC(hDlg);
                hFont=SelectObject(hDC, hFont);

                /*
                 * Item height is the maximum of the font height and the
                 * bitmap height.  We know that all bitmaps are the same
                 * size, so we just get information for one of them.
                 */
                GetTextMetrics(hDC, &tm);
                GetObject(FolderBitmaps[0], sizeof(bm), &bm);
                cyItem=max(bm.bmHeight, tm.tmHeight);

                ReleaseDC(hDlg, hDC);
                }

            pMI->itemHeight=cyItem;
            }
            break;


        case WM_DRAWITEM:
            //Go draw the appropriate item and bitmap.
            DriveDirDrawItem((LPDRAWITEMSTRUCT)lParam, (ID_DRIVELIST==wID));

            //Prevent default actions in listboxes (drawing focus rect)
            return TRUE;


        case WM_COMMAND:
            switch (wID)
                {
                case ID_DIRECTORYLIST:
                    if (LBN_DBLCLK==wCode)
                        {
                        UINT        i;
                        DWORD       dw;
                        char        szDir[_MAX_PATH];
                        LPSTR       psz;

                        /*
                         * On double-click, change directory and reinit the
                         * listbox.  But all we stored in the string was
                         * the directory's single name, so we use the bitmap
                         * type to tell if we're below the current directory
                         * (in which case we just chdir to our name) or above
                         * (in which case we prepend "..\"'s as necessary.
                         */
                        i=(UINT)SendMessage(hWndMsg, LB_GETCURSEL, 0, 0L);
                        dw=SendMessage(hWndMsg, LB_GETITEMDATA, i, 0L);

                        /*
                         * If out bitmap is IDB_FOLDERCLOSED or the root,
                         * then just .  If we're IDB_FOLDEROPENSELECT,
                         * don't do anything.  If we're IDB_FOLDEROPEN then
                         * we get the full current path and truncate it
                         * after the directory to which we're switching.
                         */
                        if (IDB_FOLDEROPENSELECT==HIWORD(dw))
                            break;

                        //Get get the directory for sub-directory changes.
                        SendMessage(hWndMsg, LB_GETTEXT, i, (LONG)(LPSTR)cwd);

                        if (IDB_FOLDEROPEN==HIWORD(dw) && 0!=i)
                            {
                            //Get the current path and find us in this path
                            GetWindowText(hWndMsg, szDir, sizeof(szDir));
                            psz=_fstrstr(szDir, cwd);

                            //Null terminate right after us.
                            *(psz+lstrlen(cwd))=0;

                            //Get this new directory in the right place
                            lstrcpy(cwd, szDir);
                            }

                        //chdir has a nice way of validating for us.
                        if (0==_chdir(cwd))
                            {
                            //Get the new full path.
                            _getcwd(cwd, _MAX_PATH);

                            DirectoryListInitialize(hWndMsg
                                , GetDlgItem(hDlg, ID_TEMPLIST), cwd);
                            }
                        }
                    break;


                case ID_DRIVELIST:
                    if (CBN_SELCHANGE==wCode)
                        {
                        UINT        i, iCurDrive;
                        char        szDrive[18]; //Enough for drive:volume

                        //Get the first letter in the current selection
                        i=(UINT)SendMessage(hWndMsg, CB_GETCURSEL, 0, 0L);
                        SendMessage(hWndMsg, CB_GETLBTEXT
                                    , i, (LONG)(LPSTR)szDrive);

                        iCurDrive=_getdrive();  //Save in case of restore

                        /*
                         * Attempt to set the drive and get the current
                         * directory on it.  Both must work for the change
                         * to be certain.  If we are certain, reinitialize
                         * the directories.  Note that we depend on drives
                         * stored as lower case in the combobox.
                         */

                        if (0==_chdrive((int)(szDrive[0]-'a'+1))
                            && NULL!=_getcwd(cwd, _MAX_PATH))
                            {
                            DirectoryListInitialize(
                                GetDlgItem(hDlg, ID_DIRECTORYLIST),
                                GetDlgItem(hDlg, ID_TEMPLIST), cwd);

                            //Insure that the root is visible (UI guideline)
                            SendDlgItemMessage(hDlg, ID_DIRECTORYLIST
                                               , LB_SETTOPINDEX, 0, 0L);

                            break;
                            }

                        //Changing drives failed so restore drive and selection
                        _chdrive((int)iCurDrive);

                        wsprintf(szDrive, "%c:", (char)(iCurDrive+'a'-1));
                        i=(UINT)SendMessage(hWndMsg, CB_SELECTSTRING
                                            , (WPARAM)-1, (LONG)(LPSTR)szDrive);
                        }

                    break;

                case IDOK:
                    _getcwd(DialogString, MAXINTERNALLINE);
                    EndDialog(hDlg, TRUE);
                    break;
      			case IDCANCEL:
			        EndDialog (hDlg, FALSE);
        			break;
                }
            break;

        default:
            break;
        }

    return FALSE;
    }

@
