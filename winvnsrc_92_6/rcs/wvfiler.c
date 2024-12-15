head	 1.1;
branch   ;
access   ;
symbols  ;
locks    ;
comment  @ * @;


1.1
date	 94.09.16.01.01.04;  author jcooper;  state Exp;
branches ;
next	 ;


desc
@Smart filer: extension mapping, name shortening, etc
@



1.1
log
@Initial revision
@
text
@/* ------------------------------------------------------------------------
 * File handlers
 *
 * author: john s. cooper
 * sept 10, 1994
 *
 */
#include <windows.h>
#include <windowsx.h>
#include "wvglob.h"
#include "winvn.h"
#pragma hdrstop
#include <io.h>			// for _access
#include <stdlib.h>		// itoa

#define MAXFILTERLEN 512

/* ------------------------------------------------------------------------
 * File/path open routines
 * jsc
 */
void	/* Set up filters */
GenerateFileFilters (HWND hParentWnd, char *filters)
{
  register int i;
  int len;
  char chReplace;

  if ((len = LoadString (hInst, IDS_FILTERSTRING, filters, MAXFILTERLEN - 1)) == 0) {
    MessageBox (hParentWnd, "No Filters Available", "String Load Failure", MB_OK);
    *filters = '\0';
  }
  else {
    chReplace = filters[len - 1];   /* grab separator */

    for (i = 0; filters[i] != '\0'; i++)
      if (filters[i] == chReplace)
   filters[i] = '\0';
  }
}

/* ------------------------------------------------------------------------
 *  Asks for a path
 *  jsc
 */
BOOL
AskForFilePath (HWND hParentWnd, char *pathName, char *title)
{
	if (!DialogBoxParam (hInst, (LPCSTR) "WinVnSelectPath", hParentWnd, (DLGPROC) lpfnWinVnSelectPathDlg, (LPARAM)pathName))
		return (FAIL);         
	strcpy (pathName, strlwr(DialogString));

	return (SUCCESS);
}

/* ------------------------------------------------------------------------
 * Asks for a filename which must exist
 */
BOOL
AskForExistingFileName (HWND hParentWnd, char *fileName, char *title)
{
  OPENFILENAME ofn;
  char szFile[MAXFILENAME];
  char filters[MAXFILTERLEN];

  GenerateFileFilters (hParentWnd, filters);

  memset (&ofn, 0, sizeof (OPENFILENAME));
  szFile[0] = '\0';
  ofn.lpstrTitle = title;
  ofn.lStructSize = sizeof (OPENFILENAME);
  ofn.hwndOwner = hParentWnd;
  ofn.lpstrFilter = filters;
  ofn.nFilterIndex = 1;
  ofn.lpstrFile = szFile;
  ofn.nMaxFile = sizeof (szFile);
  ofn.lpstrFileTitle = NULL;
  ofn.Flags = OFN_SHOWHELP | OFN_FILEMUSTEXIST;

  if (GetOpenFileName (&ofn) == 0)
    return (FAIL);
  strcpy (fileName, strlwr (szFile));
  return (SUCCESS);
}
/* ------------------------------------------------------------------------
 * Check if file name is OK based on SmartFiler settings.  If not, prompt user new name
 * Check if file already exists.  If so, prompt user OK to overwrite
 * If fileName contains a name on entry, check if it's OK, if yes-done
 */
BOOL
AskForNewFileName (HWND hParentWnd, char *fileName, char *startDir, BOOL appendOk)
{
  OPENFILENAME ofn;
  char mybuf[MAXINTERNALLINE];
  char szDirName[256];
  char szFile[256];
  BOOL retcode;
  char filters[MAXFILTERLEN];

  GenerateFileFilters (hParentWnd, filters);
  while (fileName[0] == '\0' ||
    (retcode = VerifyFileName (fileName)) == FAIL || 
	(!appendOk && _access(AttachFileName, 0) == 0)) {
    if (fileName[0] != '\0' && retcode == SUCCESS) {
      /* file exists - ask if user wants to overwrite it */
      sprintf (mybuf, "File %s exists.  OK to overwrite it?", fileName);
      retcode = MessageBox (hParentWnd, mybuf, "File exists",
          MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION);
      if (retcode == IDYES)
         break;
    }

/*      Open dialog to ask user for a new file name */
    memset (&ofn, 0, sizeof (OPENFILENAME));
    if (fileName[0] != '\0')
      sprintf (mybuf, "Invalid file name %s. Select new name", fileName);
    else {
       if (appendOk)
          strcpy (mybuf, "Select new file name to create/append");
       else
          strcpy (mybuf, "Select new file name to create/overwrite");
    }
    ofn.lpstrTitle = mybuf;
    strcpy (szDirName, startDir);
	if (szDirName[0])
    	ofn.lpstrInitialDir = szDirName;
    else
    	ofn.lpstrInitialDir = NULL;
    	
    ofn.lStructSize = sizeof (OPENFILENAME);
    ofn.hwndOwner = hParentWnd;
    ofn.lpstrFilter = filters;
    ofn.nFilterIndex = 1;
    szFile[0] = '\0';
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof (szFile);
    ofn.lpstrFileTitle = NULL;
    ofn.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST;

    if (GetOpenFileName (&ofn) == 0)
      return (FAIL);

    strcpy (fileName, strlwr (ofn.lpstrFile));
  }
  return (SUCCESS);
}
/* ------------------------------------------------------------------------
 * Test file name is OK using SmartFiler settings
 */
BOOL
VerifyFileName (char *fileName)
{
  char *beg, *ptr;

  if ((beg = strrchr (fileName, '\\')) == NULL)
    beg = fileName;
  else
    beg++;        /* skip to after path slash */

  if ((ptr = strchr (beg, '.')) == NULL)
    if (strlen (beg) <= MaxFileNameLen) /* no extension */
      return (SUCCESS);

  if ((unsigned int)(ptr - beg) > MaxFileNameLen)
    return (FAIL);

#ifndef WIN32
  beg = ptr + 1;     /* multiple extensions (ok in NT)z */
  if ((ptr = strchr (beg, '.')) != NULL)
    return (FAIL);
#endif

  if (strlen (beg) > MaxFileExtLen)    /* extension too long */
    return (FAIL);

  return (SUCCESS);
}

/* ------------------------------------------------------------------------
 * SplitFileName into path, name, ext, fullName
 * i.e. c:\windows\system\xyz.txt returns xyz.txt
 * path     --> c:\windows\system  (note no final slash)
 * name     --> xyz
 * ext      --> txt
 * fullName --> xyz.txt
 *
 * args must be pointers to buffers large enough to hold strings
 * any of args can be NULL, in which case that fileName portion is ignored
 */
void
SplitFileName(char *path, char *name, char *ext, char *fullName, char *fileName)
{
  char *start, *slash, *dot, *end;

  start = fileName;
  end = start + strlen(fileName);

  if ((slash = strrchr (start, '\\')) == NULL) {
     slash = start;				/* there is no path */
  } else {
     *slash = '\0';				/* replace last slash with null */
     slash++;               
  }
  if (path) {
     if (slash == start)
        *path = '\0';		
     else
        strcpy (path, start); 	/* from start to slash is the path */
  }
  
  if (fullName)
     strcpy (fullName, slash);
  
  if ((dot = strrchr (slash, '.')) == NULL) {
     dot = end;
  } else {
     *dot = '\0';			/* replace last dot with null */
     dot++;
  }
  if (name)
     strcpy (name, slash);	/* from after the slash to dot is the name */

  if (ext) {
     strcpy (ext, dot); 	/* from after the dot to the end is the extension */
  }
  if (slash != start)
     *(slash-1) = '\\';		/* restore the slash */
  if (dot != end)
     *(dot-1) = '.';		/* restore the dot */
}

/* useful front ends to SplitFileName */
char *GetFileExtension (char *ext, char *fileName)
{
  SplitFileName(NULL, NULL, ext, NULL, fileName);
  return ext;
}

char *NameWithoutPath (char *fullName, char *fileName)
{
  SplitFileName(NULL, NULL, NULL, fullName, fileName);
  return fullName;
}

char *PathWithoutName (char *path, char *fileName)
{
  SplitFileName(path, NULL, NULL, NULL, fileName);
  return path;
}


/* ------------------------------------------------------------------------
 * Smartfiler: HandleDupeName 
 * check if dupe file name, and if there is, deal with it 
 * return SUCCESS if dupe and handled (fixed name in newName),or if no dupe
 * return FAIL on serious failure
 */
BOOL	
HandleDupeName (HWND hParentWnd, char *fileName)
{
	char path[MAXFILENAME], name[MAXFILENAME], ext[MAXFILENAME], numStr[4];
	char newName[MAXFILENAME], namecat[MAXFILENAME];
	int i, len;
	
	if (_access(fileName, 0) < 0)
		return SUCCESS;

	/* we have a dupe */
	strcpy (newName, fileName);
	SplitFileName(path, name, ext, NULL, fileName);
	switch (OnDupeName)
	{
	case DUPE_AVOID_NONE:
		return AskForNewFileName(hParentWnd, newName, path, FALSE);
		break;
		
	case DUPE_AVOID_NUMBER_EXT:
		for (i = 0; i < 999; i++) {
			_snprintf(newName, MAXFILENAME, "%s\\%s.%03d", path, name, i);
			if (_access(newName, 0) < 0)
				break;
		}
		break;
	
	case DUPE_AVOID_PREPEND_NUM:
		for (i = 0; i < 999; i++) {
			_snprintf(namecat, MAXFILENAME, "%d%s", i, name);
			namecat[MaxFileNameLen] = '\0'; 
			_snprintf(newName, MAXFILENAME, "%s\\%s.%s", path, namecat, ext);
			if (_access(newName, 0) < 0)
				break;
		}
		break;
								
	case DUPE_AVOID_APPEND_NUM:
		for (i = 0; i < 999; i++) {
			itoa(i, numStr, 10);
			len = MaxFileNameLen - strlen(numStr);
			if (len <= 0) {
				i = 1000; break;
			}
			name[len] = '\0';
			strcat(name, numStr);
			name[MaxFileNameLen] = '\0'; 
			_snprintf(newName, MAXFILENAME, "%s\\%s.%s", path, name, ext);
			if (_access(newName, 0) < 0)
				break;
		}
		break;
	}
	if (i == 1000)	/* we failed to get an alternative, so ask user for help */
		return AskForNewFileName(hParentWnd, fileName, path, FALSE);
	else
		strcpy (fileName, newName);

	return TRUE;	
}

/* ------------------------------------------------------------------------
 * SmartFiler: HandleLongName
 * check if given name is longer than given maxLen, and if it is, deal with it 
 * return SUCCESS if too long and handled (fixed name in newName), 
 * or if name is not too long
 * return FAIL if not handled
 */
BOOL
HandleLongName(char *name, unsigned int maxLen)
{
	char newName[MAXFILENAME];
	unsigned int skipped, len, src, dest;
	BOOL result; 
	
	len = strlen(name);  
	if (len <= maxLen)
		return SUCCESS;
	
	/* name too long */
	switch (OnNameTooLong)
	{
	case SHORTEN_NONE:
		result = FAIL;
		break;
	
	case SHORTEN_TRUNCATE:
		name[maxLen] = '\0';
		result = SUCCESS;
        break;
        
	case SHORTEN_SKIP_VOWELS:
		src = dest = skipped = 0;
		while (dest < maxLen && src < len) {
			if (len - skipped > maxLen && strchr("aeiou-_*!@@#$%^&*()+", name[src])) {
				skipped++;	
			} else {
				newName[dest] = name[src];
				dest++;
			}
			src++;
		}
		newName[dest] = '\0';
		strcpy (name, newName);
		result = SUCCESS;
		break;
	}
	return result;
}

/* ------------------------------------------------------------------------
 * SmartFiler: HandleExtensionCovnersion
 * check if ext is in the extension mapping list, and if so,
 * replace it with the mapped ext
 */
void
HandleExtensionCovnersion(char *ext)
{
	register unsigned long i;
	for (i = 0L; i < ExtMapSourceList->numLines; i++)
	{
		if (!_stricmp(TextBlockLine(ExtMapSourceList, i), ext)) {
			strcpy(ext, TextBlockLine(ExtMapDosList, i));
			break;
		}
	}
}	

/* ------------------------------------------------------------------------
 * SmartFiler
 * processes fileName for dupe, too long, extension conversion
 * returns SUCCESS if everything works, else FAIL
 */
BOOL
SmartFile (HWND hParentWnd, char *fileName)
{
	char newName[MAXFILENAME];
	char path[MAXFILENAME], name[MAXFILENAME], ext[MAXFILENAME];
	BOOL result, result2;

	strcpy (newName, fileName);
	SplitFileName(path, name, ext, NULL, fileName);

	if (VerifyFileName(fileName) == FAIL)
	{
		if (EnableExtensionConversion)
			HandleExtensionCovnersion(ext);	

		result  = HandleLongName(name, MaxFileNameLen);
		result2 = HandleLongName(ext, MaxFileExtLen);

		_snprintf(newName, MAXFILENAME, "%s\\%s.%s", path, name, ext);

		if (result == FAIL || result2 == FAIL) {
			AskForNewFileName(hParentWnd, newName, path, FALSE);
		}
		
	}
	result = HandleDupeName(hParentWnd, newName);
	strcpy(fileName, newName);
	return result;
}

@
