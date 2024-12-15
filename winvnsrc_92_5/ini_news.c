/*
 *                              
 * $Id: ini_news.c 1.5 1994/09/06 12:46:21 brydon Exp $
 * $Log: ini_news.c $
 * Revision 1.5  1994/09/06  12:46:21  brydon
 * win32 fixes, comments changed
 *
 * Revision 1.5  1994/08/09  15:30:00  brydon
 * More searching, OPENFILENAME logic fix
 *
 * Revision 1.4  1994/02/17  19:49:53  brydon
 * cleanup and fixes for win32
 *
 * Revision 1.3  1993/12/08  01:28:01  rushing
 * new version box and cr lf consistency
 *
 * Revision 1.2  1993/07/07  19:52:22  rushing
 * check value returned from getenv for NULL
 *
 * Revision 1.1  1993/02/16  20:53:50  rushing
 * Initial revision
 *
 *
 */

/*#*xx********************************************************************/
/* Filename: Ini-News.c */

#include <windows.h>
#include <stdlib.h>
#include <direct.h>
#include "WvGlob.h"
#include <time.h>
#include <commdlg.h>

#ifdef WIN32   /* Looks like SDK defines "WIN32", MSVCNT defines "_WIN32" */
#ifndef _WIN32
#define _WIN32
#endif
#endif

#ifndef TRACE
#ifdef _DEBUG
char    szDebugBuffer[80];
#define TRACE(parm1,parm2) {\
    wsprintf(szDebugBuffer,parm1,parm2);\
    OutputDebugString(szDebugBuffer);}
#else
#define TRACE(parm1,parm2)
#endif
#endif

#define PROFTYPE "Admin"

/*************************************************************************
  This function looks for the .Ini file and the NewsSrc file.  Each file
  is searched for separately and they need not be in the same directory.
  If neither file can be found, the user is asked to locate the files with
  an OPENFILENAME dialog box.  Once the location of the NewSrc file is
  known, it is saved in the WinVN.ini file.

  The search order is:
    Command Line (one or two entries)
    Environment variable "WINVN" or "winvn" (case sensitive in Win16)
    Current Working directory (set by "Properties" dialog)
    Application Directory
    Windows Directory (eg. C:\WINDOWS)
    Windows System Directory (eg. C:\WINDOWS\SYSTEM, \System32)
    the path
    result of an "Open File" dialog box (WinVN.ini only)
    the .INI file (NewSrc only)
    result of an "Open File" dialog box (NewSrc only)


  If the Function is sucessful, the szAppProFile variable contains the path and
  filename of the .Ini file and the szNewsSrc variable contains the path and
  filename of the NewSrc file.  If either file does not exist, the "Open
  File" dialog creates each file.  In all cases, the name of the NewSrc
  file is written to the .INI file for future accesses.

  RETURNS:  zero if sucessful
            -1 if one or both files could not be found

  NOTE: The command line allows for the use of unique names for the standard
        WinVn.ini and NewSrc files. The entry order is .Ini and then NewSrc.

 Command line examples are as follows:

  1.  c:\WinVn\My.Ini<sp>c:\Tom\NewsSrc.Tom  => both unique names

  2.  c:\Tom<sp>c:\Tom\NewsSrc.Tom        => standard .Ini, unique NewSrc
  2a. c:\Tom\<sp>c:\Tom\NewsSrc.Tom          (same, less ambiguous)

  3.  c:\Tom\TomVn.ini                    => unique .Ini, Standard NewSrc

  4.  c:\Tom                              => standard .Ini, Standard NewSrc
  4a. c:\Tom\                                (same, less ambiguous)

  In the first example, if both files exist, szAppProFile = "c:\WinVn\My.Ini",
  szNewsSrc = "c:\Tom\NewsSrc.Tom" and the function returns Zero.

 For all specification types other than the Command line:
 1. Only one entry may be made.
 2. The "Working directory" spec doesn't allow trailing "\".
 3. Syntax and algorithm are otherwise the same as for Command Line.

**************************************************************************/

#define CHECK_DIR 1
#define CHECK_FILE 2

/*  Routine to test existence of filename on path choice */
BOOL ProbeFilePath(char *szFileSrc, char *szPath, char *szFname, int iChk)
{ 
  DWORD stat;
  OFSTRUCT ofb;

  /* Check for path in the form of "x:\dir\...\dir[\]" */
  if(iChk & CHECK_DIR)
  {
    int i;
    lstrcpy(szFileSrc, szPath);
    i = lstrlen(szFileSrc);
    if(i)
    {
      if (szFileSrc[i-1] == '\\')  /* Trailing '\', it must be a directory */
      {
        lstrcat(szFileSrc, szFname);
        if (OpenFile (szFileSrc, &ofb, OF_EXIST) != HFILE_ERROR)
        {
          lstrcpy(szFileSrc, ofb.szPathName);
          return (TRUE);
        }
        else
          return(FALSE);
      }
    }
#ifdef GetFileAttributes
    stat = GetFileAttributes(szFileSrc);
    if((stat != 0xFFFFFFFF) && (stat & FILE_ATTRIBUTE_DIRECTORY))
#else
    if(TRUE)
#endif
    {
      lstrcat(szFileSrc, "\\");
      lstrcat(szFileSrc, szFname);
      if (OpenFile (szFileSrc, &ofb, OF_EXIST) != HFILE_ERROR)
      {
        lstrcpy(szFileSrc, ofb.szPathName);
        return TRUE;
      }
    }
  }
  
  /* Check for non-directory file in the form of "x:\dir\...\dir\file.ext" */
  if (iChk & CHECK_FILE)
  {
    lstrcpy(szFileSrc, szPath);
#ifdef GetFileAttributes
    stat = GetFileAttributes(szFileSrc);
    if((stat == 0xFFFFFFFF) || !(stat & FILE_ATTRIBUTE_NORMAL))
      return(FALSE);
#endif

    if (OpenFile (szFileSrc, &ofb, OF_EXIST) != HFILE_ERROR)
    {
      lstrcpy(szFileSrc, ofb.szPathName);
      return TRUE;
    }
  }
  return FALSE;
}



/*  See if we can find the WinVN.ini and NewSrc files... */
int LocIniSrc(HINSTANCE hInstance, LPSTR lpCmdLine)
{
  char szWPath[256];
  
  char szIniDef[] = "WinVN.ini";   // ' .ini' default filename
  char szNewsDefault[] = "NewSrc"; // 'newsrc' default filename

  BOOL fProfileFound = FALSE;
  BOOL fNewsFound = FALSE;

  int iRvalue;

  LPSTR lpEnvStr;
  OFSTRUCT ofOpenBuffer;

  ofOpenBuffer.cBytes = sizeof (OFSTRUCT);

     
  if (*lpCmdLine != '\0')  /* Look at the Command Line */
  {           /* path | path and file */
    lstrcpy (szWPath, lpCmdLine);
    TRACE("Checking Command Line: %s\n", szWPath);
    iRvalue = lstrlen (szWPath);

    while (iRvalue--)
    {           /* check for two arguments */
      if (szWPath[iRvalue] == ' ')
      { szWPath[iRvalue] = 0;
        break;
      }
    }

    if (iRvalue > 0)
    { /* two entries on command line - check for path and/or path\file */
      fProfileFound = ProbeFilePath(szAppProFile, szWPath,
        szIniDef, CHECK_DIR | CHECK_FILE);
      fNewsFound = ProbeFilePath(szNewsSrc, &szWPath[iRvalue + 1], 
        szNewsDefault, CHECK_FILE);
    }

    else
    { /* single command line entry - path and/or path\file */
      fProfileFound = ProbeFilePath(szAppProFile, szWPath,
        szIniDef, CHECK_DIR | CHECK_FILE);
      fNewsFound = ProbeFilePath(szNewsSrc, szWPath,
        szNewsDefault, CHECK_DIR | CHECK_FILE);
    }
  if(fProfileFound && fNewsFound) return 0;
  }

    

      /* Look in the Environment for a path */
  if ((lpEnvStr = getenv ("winvn")) == NULL)
  {
    lpEnvStr = getenv ("WINVN");
  }

  if (lpEnvStr && (iRvalue = lstrlen(lpEnvStr)))
  {           /* found environment varible, check it out */
    lstrcpy (szWPath, lpEnvStr);
    
    TRACE("Checking environment var: %s\n", szWPath);
    if (!fProfileFound)
      fProfileFound = ProbeFilePath(szAppProFile, szWPath,
        szIniDef, CHECK_DIR | CHECK_FILE);

    if (!fNewsFound)
      fNewsFound = ProbeFilePath(szNewsSrc, &szWPath[iRvalue + 1], 
        szNewsDefault, CHECK_DIR | CHECK_FILE);

    if (fProfileFound && fNewsFound)
      return 0;
  }



  /* Look in the current working Directory */
  if (_getcwd(szWPath, sizeof(szWPath)))
  {
    TRACE("Checking current working directory: %s\n", szWPath);
    if (!fProfileFound)
      fProfileFound = ProbeFilePath(szAppProFile, szWPath,
        szIniDef, CHECK_DIR);

    if (!fNewsFound)
      fNewsFound = ProbeFilePath(szNewsSrc, szWPath,
        szNewsDefault, CHECK_DIR);

    if (fProfileFound && fNewsFound) return 0;
  }



  /* Look in WinVn's Directory */
  if (iRvalue = GetModuleFileName(hInstance, szWPath, sizeof(szWPath)))
  {
    while (iRvalue--)
    {           /* remove the module name */
      if (szWPath[iRvalue] == '\\')
      { szWPath[++iRvalue] = 0;
        break;
      }
    }

    TRACE("Checking WinVN directory: %s\n", szWPath);
    if (!fProfileFound)
      fProfileFound = ProbeFilePath(szAppProFile, szWPath,
        szIniDef, CHECK_DIR);

    if (!fNewsFound)
      fNewsFound = ProbeFilePath(szNewsSrc, szWPath,
        szNewsDefault, CHECK_DIR);

    if (fProfileFound && fNewsFound) return 0;
  }



  /* Look in the Windows Directory */
  if (GetWindowsDirectory(szWPath, sizeof(szWPath)))
  {
    TRACE("Checking Windows directory: %s\n", szWPath);
    if (!fProfileFound)
      fProfileFound = ProbeFilePath(szAppProFile, szWPath,
        szIniDef, CHECK_DIR);

    if (!fNewsFound)
      fNewsFound = ProbeFilePath(szNewsSrc, szWPath,
        szNewsDefault, CHECK_DIR);

    if (fProfileFound && fNewsFound) return 0;
  }



  /* Look in the Windows System Directory */
  if (GetSystemDirectory (szWPath, sizeof(szWPath)))
  {
    TRACE("Checking Windows System directory: %s\n", szWPath);
    if (!fProfileFound)
      fProfileFound = ProbeFilePath(szAppProFile, szWPath,
        szIniDef, CHECK_DIR);

    if (!fNewsFound)
      fNewsFound = ProbeFilePath(szNewsSrc, szWPath,
        szNewsDefault, CHECK_DIR);

    if (fProfileFound && fNewsFound) return 0;
  }



  /* Try the current path */
  szWPath[0] = '\0';
  TRACE("Checking path\n", "");
  if (!fProfileFound)
    fProfileFound = ProbeFilePath(szAppProFile, szWPath,
      szIniDef, CHECK_DIR);

  if (!fNewsFound)
    fNewsFound = ProbeFilePath(szNewsSrc, szWPath,
      szNewsDefault, CHECK_DIR);

  if (fProfileFound && fNewsFound) return 0;
  else
  { /* Ask the user if he wants to create the files */
    static OPENFILENAME ofn;

    ofn.lStructSize       = sizeof(OPENFILENAME);
    ofn.hwndOwner         = NULL;
    ofn.hInstance         = NULL;
 // ofn.lpstrFilter       = (Filled in below)
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter    = 0;
    ofn.nFilterIndex      = 0;
 // ofn.lpstrFile         = (Filled in below)
 // ofn.nMaxFile          = (Filled in below)
    ofn.lpstrFileTitle    = NULL;
    ofn.nMaxFileTitle     = 0;
    ofn.lpstrInitialDir   = szWPath;
    ofn.lpstrTitle        = "Please identify existing or new filename";
    ofn.Flags             =
      OFN_CREATEPROMPT
#ifdef OFN_NONETWORKBUTTON
      | OFN_NONETWORKBUTTON
#endif    
      | OFN_NOREADONLYRETURN;
    ofn.nFileOffset       = 0;
    ofn.nFileExtension    = 0;
 // ofn.lpstrDefExt       = (Filled in below)
    ofn.lCustData         = 0L;
    ofn.lpfnHook          = NULL;
    ofn.lpTemplateName    = NULL;

    if (!fProfileFound)
    {
      char szCreated[32];

      ofn.lpstrFilter       =
        "Init Files (*.INI)\0*.ini\0All Files (*.*)\0*.*\0";
      lstrcpy (szAppProFile, szIniDef);
      ofn.lpstrFile         = szAppProFile;
      ofn.nMaxFile          = sizeof(szAppProFile);
      ofn.lpstrDefExt       = "INI";
   
      TRACE("Checking user-specified filename\n", "");
      fProfileFound = GetOpenFileName(&ofn);
      if (fProfileFound)
      {
        char dbuffer[9];
        char tbuffer[9];
        OFSTRUCT ofs;

     // ofn.lpstrInitialDir = ...
     
        if (OpenFile(szAppProFile, &ofs, OF_EXIST) == HFILE_ERROR)
        {
          _strdate(dbuffer);
          _strtime(tbuffer);
   
          TRACE("Writing new WinVN.INI file to %s\n", szAppProFile);
          lstrcpy(szCreated, dbuffer);
          lstrcat(szCreated, " ");
          lstrcat(szCreated, tbuffer);
   
          WritePrivateProfileString(PROFTYPE, "Created",
            szCreated, szAppProFile);
        }
      }
    }


    /* Look for NewSrc spec in WinVN.INI */
    if (fProfileFound && !fNewsFound)
    {
      if(GetPrivateProfileString(PROFTYPE, "Newsrc", "",
         szNewsSrc, sizeof(szNewsSrc), szAppProFile))
      {
        OFSTRUCT ofs;
        fNewsFound = (OpenFile(szNewsSrc, &ofs, OF_EXIST) != HFILE_ERROR);
      }
    }


    /* Give up?  Ask the user where the file is... */
    if (fProfileFound && !fNewsFound)
    {
      ofn.lpstrFilter       =
        "No-ext Files (*.)\0*.\0All Files (*.*)\0*.*\0";
      ofn.nFilterIndex      = 0;
      lstrcpy (szNewsSrc, szNewsDefault);
      ofn.lpstrFile         = szNewsSrc;
      ofn.nMaxFile          = sizeof(szNewsSrc);
      ofn.lpstrDefExt       = "";
    
      fNewsFound = GetOpenFileName(&ofn);
      if (fNewsFound)
      {
        HFILE hf;
        OFSTRUCT ofs;
        if (OpenFile(szNewsSrc, &ofs, OF_EXIST) == HFILE_ERROR)
        {
          hf = OpenFile(szNewsSrc, &ofs, OF_CREATE);
          /* (Optionally write some default newsgroups here... */
          _lclose(hf);
        }
      }
    }
  }

  if (fProfileFound && fNewsFound) return 0;
  return (-1);
}


/*  Locate the WinVN.ini and NewSrc files... */
int fnLocateFiles (HINSTANCE hInstance, LPSTR lpCmdLine)
{
  int result;
  result = LocIniSrc(hInstance, lpCmdLine);
  if (!result)
  {
    TRACE("WinVN.INI file located: %s\n", szAppProFile);
    TRACE("NewSrc file located:    %s\n", szNewsSrc);
    WritePrivateProfileString(PROFTYPE, "Newsrc",
            szNewsSrc, szAppProFile);
  }
  return(result);
} 


