head	1.5;
access;
symbols
	V80:1.1
	V76d:1.1;
locks; strict;
comment	@ * @;


1.5
date	94.09.06.12.46.21;	author brydon;	state Exp;
branches;
next	1.4;

1.4
date	94.02.17.19.49.53;	author brydon;	state Exp;
branches;
next	1.3;

1.3
date	93.12.08.01.28.01;	author rushing;	state Exp;
branches;
next	1.2;

1.2
date	93.07.07.19.52.22;	author rushing;	state Exp;
branches;
next	1.1;

1.1
date	93.02.16.20.53.50;	author rushing;	state Exp;
branches;
next	;


desc
@winvn version 0.76 placed into RCS
@


1.5
log
@win32 fixes, comments changed
@
text
@/*
 *                              
 * $Id: ini_news.c 1.5 1994/08/09  15:30:00  brydon Exp $
 * $Log: ini_news.c $
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


@


1.4
log
@cleanup and fixes for win32
@
text
@a0 1

d2 5
d8 3
a10 2
 * $Id: ini_news.c 1.3 1993/12/08 01:28:01 rushing Exp $
 * $Log: ini_news.c $
d28 1
a31 1
#define defTempStringSize 128
d33 19
d53 17
a69 1
  This function looks for the .Ini file and the NewsSrc file.
a70 6
  The search order is: Command Line               Unique filenames
                       Environment                Standard filenames
                       Application Directory      Standard filenames
                       Windows Directory          Standard filenames
                       Windows System Directory   Standard filenames
                       the path                   Standard filenames
a71 1

d74 3
a76 1
  filename of the NewSrc file.
d99 4
a102 26
  If the files do not exist but the path's do and if they contain the
  standard file(s), szAppProFile is set to "c:\WinVn\WinVn.Ini",
  szNewsSrc is set to "c:\Tom\NewSrc" and the function returns Zero.

  If the NewSrc file is not found first by it's unique name and then by the
  standard name in the defined directory, the .ini path is searched for the
  standard NewSrc filename and if this file is found, the path and name
  are returned.

  If the command line entry is just a name, the search path is the following:
           the current drive/directory
           windows directory
           windows system directory
           the application directory
           the path

  The above behavior may lead to the use of the wrong information files if the
  standard names are used. It is recommended that unique names be used.

  If the .Ini file or the NewSrc file is/are not found, first by Name
  then by Path with standard names, a warning is displayed and the
  function continues to search for the file(s) in the order described
  above ( Environment, Appication directory, ...etc).

  The intent of this is to allow multiple and unique .Ini and NewsSrc
  location as well and an exhaustive attempt to locate the files.
a103 6
  [Additions to the function/program shold be to popup dialog boxes for the
  necessary information if not found or it is incorrect. Maybe later!]

  H. Brydon: I added the dialog boxes to find files not found by the above,
  and create new, empty files if required.  1994/01/25

d106 2
a107 1
/*-*xx********************************************************************/
a108 1

d110 3
a112 2
BOOL ProbeFilePath(char *szFileSrc, char *szPath, char *szFname, BOOL bChk)
{ int i;
d115 35
a149 13
  /* (Enhancement possibility: we can do better checking for directory
      files and can get rid of the bChk variable.  We can use API call
      GetFileAttributes that can check whether or not a file is a 
      directory (ask for FILE_ATTRIBUTE_DIRECTORY)...  - H. Brydon )  */
  lstrcpy(szFileSrc, szPath);
  i = lstrlen(szPath);
  if(i)
  { if (szFileSrc[i-1] != '\\') lstrcat(szFileSrc, "\\");
  }
  lstrcat(szFileSrc, szFname);
  if (OpenFile (szFileSrc, &ofb, OF_EXIST) != HFILE_ERROR)
  { lstrcpy(szFileSrc, ofb.szPathName);
    return TRUE;
d152 10
a161 2
  if (bChk)
  { lstrcpy(szFileSrc, szPath);
d163 2
a164 1
    { lstrcpy(szFileSrc, ofb.szPathName);
a167 3
  
  // other tests can go here...
  
a172 2


d174 3
a176 3
int fnLocateFiles (HINSTANCE hInstance, LPSTR lpCmdLine)
{ char szPath[defTempStringSize];
  char szWPath[defTempStringSize];
a179 1
  char szEnv[] = "WINVN";          // environment variable
d194 3
a196 2
    lstrcpy (szPath, lpCmdLine);
    iRvalue = lstrlen (szPath);
d200 2
a201 2
      if (szPath[iRvalue] == ' ')
      { szPath[iRvalue] = 0;
d207 5
a211 4
    { /* two entries on command line */
      fProfileFound = ProbeFilePath(szAppProFile, szPath, szIniDef,TRUE);
      fNewsFound = ProbeFilePath(szNewsSrc, &szPath[iRvalue + 1], 
szNewsDefault,TRUE);
d215 5
a219 3
    { /* single command line entry - assume it is a path and a file name */
      fProfileFound = ProbeFilePath(szAppProFile, szPath, szIniDef,FALSE);
      fNewsFound = ProbeFilePath(szNewsSrc, szPath, szNewsDefault, FALSE);
d227 1
a227 1
  if ((lpEnvStr = getenv (szEnv)) == NULL)
d229 1
a229 2
    AnsiLowerBuff (szEnv, lstrlen (szEnv));
    lpEnvStr = getenv (szEnv);
d234 1
a234 1
    lstrcpy (szPath, lpEnvStr);
d236 1
d238 2
a239 1
      fProfileFound = ProbeFilePath(szAppProFile, szPath, szIniDef,FALSE);
d242 2
a243 2
      fNewsFound = ProbeFilePath(szNewsSrc, &szPath[iRvalue + 1], 
szNewsDefault, FALSE);
d250 18
d269 1
a269 1
  if ((iRvalue = GetModuleFileName(hInstance, szWPath, defTempStringSize)))
d279 1
d281 2
a282 1
      fProfileFound = ProbeFilePath(szAppProFile, szWPath, szIniDef, FALSE);
d285 2
a286 1
      fNewsFound = ProbeFilePath(szNewsSrc, szWPath, szNewsDefault, FALSE);
d292 16
a307 17
/*  Note:  The following two code segments are written and tested but
*    commented out because they are believed to be superfluous.  Just
*    searching for the files on the current path (below) looks in these
*    directories.  If I am wrong, then please uncomment this code.  It has
*    been tested properly and works...  HAB  */


      /* Look in the Windows Directory */
//  if ((iRvalue = (int) GetWindowsDirectory (szPath, defTempStringSize)))
//  { if (!fProfileFound)
//      fProfileFound = ProbeFilePath(szAppProFile, szPath, szIniDef, FALSE);
//  
//    if (!fNewsFound)
//      fNewsFound = ProbeFilePath(szNewsSrc, szPath, szNewsDefault, FALSE);
//  
//    if (fProfileFound && fNewsFound) return 0;
//   }
d310 7
d318 3
d322 2
a323 10
      /* Try the Windows System Directory */
//  if ((iRvalue = (int) GetSystemDirectory (szPath, defTempStringSize)))
//  { if (!fProfileFound)
//      fProfileFound = ProbeFilePath(szAppProFile, szPath, szIniDef, FALSE);
//
//    if (!fNewsFound)
//      fNewsFound = ProbeFilePath(szNewsSrc, szPath, szNewsDefault, FALSE);
//
//    if (fProfileFound && fNewsFound) return 0;
//  }
d328 2
a329 1
  szPath[0] = '\0';
d331 2
a332 1
    fProfileFound = ProbeFilePath(szAppProFile, szPath, szIniDef, FALSE);
d335 2
a336 1
    fNewsFound = ProbeFilePath(szNewsSrc, szPath, szNewsDefault, FALSE);
d358 4
a361 4
#ifdef WIN32
	| OFN_NONETWORKBUTTON
#endif	  
	  | OFN_NOREADONLYRETURN;
d370 2
a371 9
    { char szCreated[32];

   // The following code supposedly used to work with older MS and Borland
   // compilers, but doesn't now (definitely not with MSVC/NT 1.0)
   // static char *szFilter[] = { "Init Files (*.INI)", "*.ini",
   //                             "All Files (*.*)",    "*.*",
   //                             "",""};
   
   // ofn.lpstrFilter       = szFilter[0];
d375 1
d380 1
d383 2
a384 1
      { char dbuffer[9];
d386 1
d390 4
a393 2
        _strdate(dbuffer);
        _strtime(tbuffer);
d395 4
a398 3
        lstrcpy(szCreated, dbuffer);
        lstrcat(szCreated, " ");
        lstrcat(szCreated, tbuffer);
d400 3
a402 2
        WritePrivateProfileString(szAppName, "Created",
          szCreated, szAppProFile);
d405 18
a422 3
   
    if (!fNewsFound)
    { ofn.lpstrFilter       =
d425 1
d432 2
a433 1
      { HFILE hf;
d450 2
a451 6



int 
old_fnLocateFiles (HINSTANCE hInstance, LPSTR lpCmdLine)

d453 11
a463 202

  char szPath[defTempStringSize];
  char szNews[defTempStringSize];
  char szTemp[defTempStringSize];

  char szEnv[] = "WINVN";
  char szIni[] = ".Ini";
  char szNewsDefault[] = "NewSrc";

  BOOL fNewsFound = FALSE;
  BOOL fProfileFound = FALSE;

  int iRvalue;

  LPSTR lpEnvStr;
  OFSTRUCT ofOpenBuffer;

  ofOpenBuffer.cBytes = sizeof (OFSTRUCT);

  while (1)
    {
      /* Look at the Command Line */
      if (*lpCmdLine != '\0')
    {           /* path | path and file */
      lstrcpy (szPath, lpCmdLine);
      iRvalue = lstrlen (szPath);

      while (iRvalue--)
        {           /* check for two arguments */
          if (szPath[iRvalue] == ' ')
        {
          szPath[iRvalue] = 0;
          break;
        }
        }

      if (iRvalue > 0)
        {           /* two entries on command line */
          lstrcpy (szNews, &szPath[iRvalue + 1]);
          lstrcpy (szTemp, szNews);

          /* assume it is a path and a filename */
          if (OpenFile (szNews, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
        {
          lstrcpy (szNewsSrc, szNews);
          fNewsFound = TRUE;
        }
          else
        {
          /* assume it is just a path name and look for NEWSSRC */
          lstrcpy (szNews, szTemp);
          iRvalue = lstrlen (szNews);
          if (szNews[iRvalue - 1] != '\\')
            {
              lstrcat (szNews, "\\");
            }
          lstrcat (szNews, szNewsDefault);

          if (OpenFile (szNews, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
            {
              fNewsFound = TRUE;
              lstrcpy (szNewsSrc, szNews);
            }
          else
            {       /* assume it has a bad filename */
              while (iRvalue--)
            {
              if (szTemp[iRvalue] == '\\')
                {
                  szTemp[iRvalue + 1] = 0;
                  break;
                }
            }

              lstrcat (szTemp, szNewsDefault);
              if (OpenFile (szTemp, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
            {
              fNewsFound = TRUE;
              lstrcpy (szNewsSrc, szTemp);
            }
            }

          if (!fNewsFound)
            {
              /* need to add a dialog that the user may enter the 
               * information! */
              MessageBox (NULL,
            "Can not use the Command Line NewsSrc Information!",
                  "Soft Error",
                  MB_OK | MB_ICONQUESTION);
            }
        }

          /* looking for Ini, assume it is a path and a file name */
          if (OpenFile (szPath, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
        {
          fProfileFound = TRUE;
          lstrcpy (szAppProFile, szPath);
        }
          else
        {

          /* assume it is just a path name and look for winvn.ini */
          lstrcpy (szTemp, szPath);
          iRvalue = lstrlen (szTemp);
          if (szTemp[iRvalue - 1] != '\\')
            {
              lstrcat (szTemp, "\\");
            }
          lstrcat (szTemp, szEnv);
          lstrcat (szTemp, szIni);

          if (OpenFile (szTemp, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
            {
              fProfileFound = TRUE;
              lstrcpy (szAppProFile, szTemp);
            }
          else
            {       /* assume a bad filename */
              lstrcpy (szTemp, szPath);
              while (iRvalue--)
            {
              if (szTemp[iRvalue] == '\\')
                {
                  szTemp[iRvalue + 1] = 0;
                  break;
                }
            }

              lstrcat (szTemp, szEnv);
              lstrcat (szTemp, szIni);

              if (OpenFile (szTemp, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
            {
              fProfileFound = TRUE;
              lstrcpy (szAppProFile, szTemp);
            }
            }

          if (!fProfileFound)
            {
              /* need to add a dialog so that the user may */
              /* enter/correct the information! */
              MessageBox (NULL,
                "Can not use the Command Line Ini Information!",
                  "Soft Error",
                  MB_OK | MB_ICONQUESTION);

            }

        }

          if (fProfileFound && !fNewsFound)
        { /* try the .Ini path for the standard NewsSrc */
          iRvalue = lstrlen (szPath);
          while (iRvalue--)
            {
              if (szPath[iRvalue] == '\\')
            {
              szPath[++iRvalue] = 0;
              break;
            }
            }

          lstrcpy (szNews, szPath);
          lstrcat (szNews, szNewsDefault);
          if (OpenFile (szNews, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
            {
              fNewsFound = TRUE;
              lstrcpy (szNewsSrc, szNews);
              break;
            }

        }

        }
      else
        {           /* single command line entry */
          /* assume it is a path and a file name */
          lstrcpy (szTemp, szPath);

          if (OpenFile (szTemp, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
        {
          lstrcpy (szAppProFile, szTemp);
          fProfileFound = TRUE;

          /* Use the ini path and look for NewsSrc */
          iRvalue = lstrlen (szPath);
          lstrcpy (szTemp, szPath);
          if (szTemp[iRvalue - 1] != '\\')
            {
              lstrcat (szTemp, "\\");
            }
          lstrcat (szTemp, szNewsDefault);

          if (OpenFile (szTemp, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
            {
              lstrcpy (szNewsSrc, szTemp);
              fNewsFound = TRUE;
              break;
            }
        }
a464 197
          /* could not find .Ini or NewsSrc assume it is just a */
          /* path name and look for winvn.ini */
          iRvalue = lstrlen (szPath);
          if (szPath[iRvalue - 1] != '\\')
        {
          lstrcat (szPath, "\\");
        }
          lstrcpy (szTemp, szPath);

          if (!fProfileFound)
        {
          lstrcat (szTemp, szEnv);
          lstrcat (szTemp, szIni);

          if (OpenFile (szTemp, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
            {
              lstrcpy (szAppProFile, szTemp);
              fProfileFound = TRUE;
            }

          lstrcpy (szTemp, szPath);
          lstrcat (szTemp, szNewsDefault);
          if (OpenFile (szTemp, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
            {
              lstrcpy (szNewsSrc, szTemp);
              fNewsFound = TRUE;
            }

        }

          if (fProfileFound && fNewsFound)
        {
          break;
        }

          /* need to add a dialog that the user may enter the */
          /* information! */
          lstrcpy (szTemp, "Could not use the Command Line Information for ");

          if (!fProfileFound && !fNewsFound)
        {
          lstrcat (szTemp, ".Ini and NewsSrc");
        }
          else if (!fProfileFound)
        {
          lstrcat (szTemp, szIni);
        }
          else
        {
          lstrcat (szTemp, szNewsDefault);
        }

          MessageBox (NULL, szTemp, "Soft Error", MB_OK | MB_ICONQUESTION);

        }

    }


      /* Look in the Environment for a path */
      if ((lpEnvStr = getenv (szEnv)) == NULL)
    {
      AnsiLowerBuff (szEnv, lstrlen (szEnv));
      lpEnvStr = getenv (szEnv);
    }

      if (lpEnvStr && (iRvalue = lstrlen (lpEnvStr)))
    {           /* found environment varible, check it out */
      lstrcpy (szPath, lpEnvStr);
      if (szPath[iRvalue - 1] != '\\')
        {
          lstrcat (szPath, "\\");
        }

      if (!fProfileFound)
        {           /* look for the .Ini file */
          lstrcpy (szAppProFile, szPath);
          lstrcat (szAppProFile, szEnv);
          lstrcat (szAppProFile, szIni);

          if (OpenFile (szAppProFile, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
        {
          fProfileFound = TRUE;
        }
        }

      if (!fNewsFound)
        {           /* look for the .NewsSrc file */
          lstrcpy (szNewsSrc, szPath);
          lstrcat (szNewsSrc, szNewsDefault);

          if (OpenFile (szNewsSrc, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
        {
          fNewsFound = TRUE;
        }
        }

      if (fProfileFound && fNewsFound)
        {
          break;
        }

    }


      /* Look in WinVn's Directory */
      if ((iRvalue = GetModuleFileName (hInstance, szPath, defTempStringSize)))
    {
      while (iRvalue--)
        {           /* remove the module name */
          if (szPath[iRvalue] == '\\')
        {
          szPath[++iRvalue] = 0;
          break;
        }
        }

      if (!fProfileFound)
        {           /* look for the .Ini file */
          lstrcpy (szAppProFile, szPath);
          lstrcat (szAppProFile, szEnv);
          lstrcat (szAppProFile, szIni);
          if (OpenFile (szAppProFile, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
        {
          fProfileFound = TRUE;
        }
        }

      if (!fNewsFound)
        {           /* look for the NewsSrc file */
          lstrcpy (szNewsSrc, szPath);
          lstrcat (szNewsSrc, szNewsDefault);

          if (OpenFile (szNewsSrc, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
        {
          fNewsFound = TRUE;
        }
        }

      if (fProfileFound && fNewsFound)
        {
          break;
        }

    }


      /* Look in the Windows's Directory */
      if ((iRvalue = (int) GetWindowsDirectory (szPath, defTempStringSize)))
    {
      lstrcat (szPath, "\\");

      if (!fProfileFound)
        {           /* look for the .Ini file */
          lstrcpy (szAppProFile, szPath);
          lstrcat (szAppProFile, szEnv);
          lstrcat (szAppProFile, szIni);
          if (OpenFile (szAppProFile, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
        {
          fProfileFound = TRUE;
        }
        }

      if (!fNewsFound)
        {           /* look for the NewsSrc file */
          lstrcpy (szNewsSrc, szPath);
          lstrcat (szNewsSrc, szNewsDefault);

          if (OpenFile (szNewsSrc, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
        {
          fNewsFound = TRUE;
        }
        }

      if (fProfileFound && fNewsFound)
        {
          break;
        }

    }


      /* Try the Windows System's Directory */
      if ((iRvalue = (int) GetSystemDirectory (szPath, defTempStringSize)))
    {
      lstrcat (szPath, "\\");

      if (!fProfileFound)
        {           /* look for the .Ini file */
          lstrcpy (szAppProFile, szPath);
          lstrcat (szAppProFile, szEnv);
          lstrcat (szAppProFile, szIni);
          if (OpenFile (szAppProFile, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
        {
          fProfileFound = TRUE;
        }
        }
a465 59
      if (!fNewsFound)
        {           /* look for the NewsSrc file */
          lstrcpy (szNewsSrc, szPath);
          lstrcat (szNewsSrc, szNewsDefault);

          if (OpenFile (szNewsSrc, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
        {
          fNewsFound = TRUE;
        }
        }

      if (fProfileFound && fNewsFound)
        {
          break;
        }

    }

      /* Last chance, Try the current path */
      if (!fProfileFound)
    {           /* look for the .Ini file */
      lstrcpy (szAppProFile, szEnv);
      lstrcat (szAppProFile, szIni);
      if (OpenFile (szAppProFile, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
        {
          fProfileFound = TRUE;
        }
    }

      if (!fNewsFound)
    {           /* look for the NewsSrc file */
      lstrcpy (szNewsSrc, szNewsDefault);

      if (OpenFile (szNewsSrc, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
        {
          fNewsFound = TRUE;
        }
    }

      if (fProfileFound && fNewsFound)
    {
      break;
    }

      /* No potatoes, RTFM
       * need to add a dialog that the user may enter the information!
       */
      MessageBox (NULL,
          "Can Not Find Your .Ini File or NewsSrc File",
          "Fatal Error",
          MB_OK | MB_ICONEXCLAMATION);

      return (-1);

    }

  return (0);

}
@


1.3
log
@new version box and cr lf consistency
@
text
@d4 5
a8 2
 * $Id: ini-news.c 1.2 1993/07/07 19:52:22 rushing Exp rushing $
 * $Log: ini-news.c $
d24 2
a25 1

d51 1
a51 1
  1. c:\WinVn\My.Ini<sp>c:\Tom\NewsSrc.Tom  => both unique names
d53 2
a54 1
  2. c:\Tom<sp>c:\Tom\NewsSrc.Tom        => standard .Ini, unique NewSrc
d56 1
a56 1
  3. c:\Tom\TomVn.ini                    => unique .Ini, Standard NewSrc
d58 2
a59 1
  4. c:\Tom                              => standard .Ini, Standard NewSrc
d91 2
a92 2
  Additions to the function/program shold be to popup dialog boxes for the
  necessary information if not found or it is incorrect. Maybe later!
d94 3
d101 271
d373 1
a373 1
fnLocateFiles (HINSTANCE hInstance, LPSTR lpCmdLine)
d398 232
a629 232
      if (*lpCmdLine != NULL)
	{			/* path | path and file */
	  lstrcpy (szPath, lpCmdLine);
	  iRvalue = lstrlen (szPath);

	  while (iRvalue--)
	    {			/* check for two arguments */
	      if (szPath[iRvalue] == ' ')
		{
		  szPath[iRvalue] = 0;
		  break;
		}
	    }

	  if (iRvalue > 0)
	    {			/* two entries on command line */
	      lstrcpy (szNews, &szPath[iRvalue + 1]);
	      lstrcpy (szTemp, szNews);

	      /* assume it is a path and a filename */
	      if (OpenFile (szNews, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
		{
		  lstrcpy (szNewsSrc, szNews);
		  fNewsFound = TRUE;
		}
	      else
		{
		  /* assume it is just a path name and look for NEWSSRC */
		  lstrcpy (szNews, szTemp);
		  iRvalue = lstrlen (szNews);
		  if (szNews[iRvalue - 1] != '\\')
		    {
		      lstrcat (szNews, "\\");
		    }
		  lstrcat (szNews, szNewsDefault);

		  if (OpenFile (szNews, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
		    {
		      fNewsFound = TRUE;
		      lstrcpy (szNewsSrc, szNews);
		    }
		  else
		    {		/* assume it has a bad filename */
		      while (iRvalue--)
			{
			  if (szTemp[iRvalue] == '\\')
			    {
			      szTemp[iRvalue + 1] = 0;
			      break;
			    }
			}

		      lstrcat (szTemp, szNewsDefault);
		      if (OpenFile (szTemp, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
			{
			  fNewsFound = TRUE;
			  lstrcpy (szNewsSrc, szTemp);
			}
		    }

		  if (!fNewsFound)
		    {
		      /* need to add a dialog that the user may enter the 
		       * information! */
		      MessageBox (NULL,
			"Can not use the Command Line NewsSrc Information!",
				  "Soft Error",
				  MB_OK | MB_ICONQUESTION);
		    }
		}

	      /* looking for Ini, assume it is a path and a file name */
	      if (OpenFile (szPath, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
		{
		  fProfileFound = TRUE;
		  lstrcpy (szAppProFile, szPath);
		}
	      else
		{

		  /* assume it is just a path name and look for winvn.ini */
		  lstrcpy (szTemp, szPath);
		  iRvalue = lstrlen (szTemp);
		  if (szTemp[iRvalue - 1] != '\\')
		    {
		      lstrcat (szTemp, "\\");
		    }
		  lstrcat (szTemp, szEnv);
		  lstrcat (szTemp, szIni);

		  if (OpenFile (szTemp, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
		    {
		      fProfileFound = TRUE;
		      lstrcpy (szAppProFile, szTemp);
		    }
		  else
		    {		/* assume a bad filename */
		      lstrcpy (szTemp, szPath);
		      while (iRvalue--)
			{
			  if (szTemp[iRvalue] == '\\')
			    {
			      szTemp[iRvalue + 1] = 0;
			      break;
			    }
			}

		      lstrcat (szTemp, szEnv);
		      lstrcat (szTemp, szIni);

		      if (OpenFile (szTemp, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
			{
			  fProfileFound = TRUE;
			  lstrcpy (szAppProFile, szTemp);
			}
		    }

		  if (!fProfileFound)
		    {
		      /* need to add a dialog so that the user may */
		      /* enter/correct the information! */
		      MessageBox (NULL,
			    "Can not use the Command Line Ini Information!",
				  "Soft Error",
				  MB_OK | MB_ICONQUESTION);

		    }

		}

	      if (fProfileFound && !fNewsFound)
		{ /* try the .Ini path for the standard NewsSrc */
		  iRvalue = lstrlen (szPath);
		  while (iRvalue--)
		    {
		      if (szPath[iRvalue] == '\\')
			{
			  szPath[++iRvalue] = 0;
			  break;
			}
		    }

		  lstrcpy (szNews, szPath);
		  lstrcat (szNews, szNewsDefault);
		  if (OpenFile (szNews, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
		    {
		      fNewsFound = TRUE;
		      lstrcpy (szNewsSrc, szNews);
		      break;
		    }

		}

	    }
	  else
	    {			/* single command line entry */
	      /* assume it is a path and a file name */
	      lstrcpy (szTemp, szPath);

	      if (OpenFile (szTemp, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
		{
		  lstrcpy (szAppProFile, szTemp);
		  fProfileFound = TRUE;

		  /* Use the ini path and look for NewsSrc */
		  iRvalue = lstrlen (szPath);
		  lstrcpy (szTemp, szPath);
		  if (szTemp[iRvalue - 1] != '\\')
		    {
		      lstrcat (szTemp, "\\");
		    }
		  lstrcat (szTemp, szNewsDefault);

		  if (OpenFile (szTemp, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
		    {
		      lstrcpy (szNewsSrc, szTemp);
		      fNewsFound = TRUE;
		      break;
		    }
		}

	      /* could not find .Ini or NewsSrc assume it is just a */
	      /* path name and look for winvn.ini */
	      iRvalue = lstrlen (szPath);
	      if (szPath[iRvalue - 1] != '\\')
		{
		  lstrcat (szPath, "\\");
		}
	      lstrcpy (szTemp, szPath);

	      if (!fProfileFound)
		{
		  lstrcat (szTemp, szEnv);
		  lstrcat (szTemp, szIni);

		  if (OpenFile (szTemp, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
		    {
		      lstrcpy (szAppProFile, szTemp);
		      fProfileFound = TRUE;
		    }

		  lstrcpy (szTemp, szPath);
		  lstrcat (szTemp, szNewsDefault);
		  if (OpenFile (szTemp, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
		    {
		      lstrcpy (szNewsSrc, szTemp);
		      fNewsFound = TRUE;
		    }

		}

	      if (fProfileFound && fNewsFound)
		{
		  break;
		}

	      /* need to add a dialog that the user may enter the */
	      /* information! */
	      lstrcpy (szTemp, "Could not use the Command Line Information for ");

	      if (!fProfileFound && !fNewsFound)
		{
		  lstrcat (szTemp, ".Ini and NewsSrc");
		}
	      else if (!fProfileFound)
		{
		  lstrcat (szTemp, szIni);
		}
	      else
		{
		  lstrcat (szTemp, szNewsDefault);
		}
d631 1
a631 1
	      MessageBox (NULL, szTemp, "Soft Error", MB_OK | MB_ICONQUESTION);
d633 1
a633 1
	    }
d635 1
a635 1
	}
d640 4
a643 4
	{
	  AnsiLowerBuff (szEnv, lstrlen (szEnv));
	  lpEnvStr = getenv (szEnv);
	}
d646 34
a679 34
	{			/* found environment varible, check it out */
	  lstrcpy (szPath, lpEnvStr);
	  if (szPath[iRvalue - 1] != '\\')
	    {
	      lstrcat (szPath, "\\");
	    }

	  if (!fProfileFound)
	    {			/* look for the .Ini file */
	      lstrcpy (szAppProFile, szPath);
	      lstrcat (szAppProFile, szEnv);
	      lstrcat (szAppProFile, szIni);

	      if (OpenFile (szAppProFile, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
		{
		  fProfileFound = TRUE;
		}
	    }

	  if (!fNewsFound)
	    {			/* look for the .NewsSrc file */
	      lstrcpy (szNewsSrc, szPath);
	      lstrcat (szNewsSrc, szNewsDefault);

	      if (OpenFile (szNewsSrc, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
		{
		  fNewsFound = TRUE;
		}
	    }

	  if (fProfileFound && fNewsFound)
	    {
	      break;
	    }
d681 1
a681 1
	}
d686 36
a721 36
	{
	  while (iRvalue--)
	    {			/* remove the module name */
	      if (szPath[iRvalue] == '\\')
		{
		  szPath[++iRvalue] = 0;
		  break;
		}
	    }

	  if (!fProfileFound)
	    {			/* look for the .Ini file */
	      lstrcpy (szAppProFile, szPath);
	      lstrcat (szAppProFile, szEnv);
	      lstrcat (szAppProFile, szIni);
	      if (OpenFile (szAppProFile, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
		{
		  fProfileFound = TRUE;
		}
	    }

	  if (!fNewsFound)
	    {			/* look for the NewsSrc file */
	      lstrcpy (szNewsSrc, szPath);
	      lstrcat (szNewsSrc, szNewsDefault);

	      if (OpenFile (szNewsSrc, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
		{
		  fNewsFound = TRUE;
		}
	    }

	  if (fProfileFound && fNewsFound)
	    {
	      break;
	    }
d723 1
a723 1
	}
d728 24
a751 2
	{
	  lstrcat (szPath, "\\");
d753 4
a756 26
	  if (!fProfileFound)
	    {			/* look for the .Ini file */
	      lstrcpy (szAppProFile, szPath);
	      lstrcat (szAppProFile, szEnv);
	      lstrcat (szAppProFile, szIni);
	      if (OpenFile (szAppProFile, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
		{
		  fProfileFound = TRUE;
		}
	    }

	  if (!fNewsFound)
	    {			/* look for the NewsSrc file */
	      lstrcpy (szNewsSrc, szPath);
	      lstrcat (szNewsSrc, szNewsDefault);

	      if (OpenFile (szNewsSrc, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
		{
		  fNewsFound = TRUE;
		}
	    }

	  if (fProfileFound && fNewsFound)
	    {
	      break;
	    }
d758 1
a758 1
	}
d763 2
a764 2
	{
	  lstrcat (szPath, "\\");
d766 21
a786 26
	  if (!fProfileFound)
	    {			/* look for the .Ini file */
	      lstrcpy (szAppProFile, szPath);
	      lstrcat (szAppProFile, szEnv);
	      lstrcat (szAppProFile, szIni);
	      if (OpenFile (szAppProFile, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
		{
		  fProfileFound = TRUE;
		}
	    }

	  if (!fNewsFound)
	    {			/* look for the NewsSrc file */
	      lstrcpy (szNewsSrc, szPath);
	      lstrcat (szNewsSrc, szNewsDefault);

	      if (OpenFile (szNewsSrc, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
		{
		  fNewsFound = TRUE;
		}
	    }

	  if (fProfileFound && fNewsFound)
	    {
	      break;
	    }
d788 6
a793 1
	}
d797 8
a804 8
	{			/* look for the .Ini file */
	  lstrcpy (szAppProFile, szEnv);
	  lstrcat (szAppProFile, szIni);
	  if (OpenFile (szAppProFile, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
	    {
	      fProfileFound = TRUE;
	    }
	}
d807 2
a808 2
	{			/* look for the NewsSrc file */
	  lstrcpy (szNewsSrc, szNewsDefault);
d810 5
a814 5
	  if (OpenFile (szNewsSrc, &ofOpenBuffer, OF_EXIST) != HFILE_ERROR)
	    {
	      fNewsFound = TRUE;
	    }
	}
d817 3
a819 3
	{
	  break;
	}
d825 3
a827 3
		  "Can Not Find Your .Ini File or NewsSrc File",
		  "Fatal Error",
		  MB_OK | MB_ICONEXCLAMATION);
@


1.2
log
@check value returned from getenv for NULL
@
text
@d1 1
d4 1
a4 1
 * $Id: ini-news.c 1.1 1993/02/16 20:53:50 rushing Exp rushing $
d6 3
@


1.1
log
@Initial revision
@
text
@d3 4
a6 2
 * $Id$
 * $Log$
d8 1
d361 1
a361 1
      if ((iRvalue = lstrlen (lpEnvStr)))
@
