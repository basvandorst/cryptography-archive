/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
/*4514 is "unreferenced inline func"*/
#pragma warning (disable : 4214 4201 4115 4514)
#include <windows.h>
#pragma warning (default : 4214 4201 4115)
#include <stdarg.h>
#include <direct.h>
#include <stdio.h>

BOOL SizeSet = FALSE;

DWORD ErrorOut(char* text,WORD attrib);

DWORD Log(char* text, ...);

DWORD ErrorOut(char* text,WORD attrib)
{
	DWORD NumBytesWritten;
	CONSOLE_SCREEN_BUFFER_INFO  csbi;
	COORD CursorPos;
	COORD ScreenSize;
	HANDLE console;

	AllocConsole();

	SetConsoleTitle("Debug Console");

	console = GetStdHandle(STD_OUTPUT_HANDLE);

   if(console == INVALID_HANDLE_VALUE)
		MessageBox(NULL,"Invalid Handle Returned by GetStdHandle()",0,MB_OK);

	if(!SizeSet)
	{
 	 	 ScreenSize.X = 80;
 	 	 ScreenSize.Y = 50;

 	 	 SetConsoleScreenBufferSize(console,ScreenSize);
 	 	 SizeSet = TRUE;
 	}

 	 if(attrib)
 	 	 SetConsoleTextAttribute(console,attrib);

 	 WriteFile(console, text, strlen(text), &NumBytesWritten, NULL);

 	 if(*(text+strlen(text)) == '\n')
 	 {
 	 	 GetConsoleScreenBufferInfo(console, &csbi);
 	 	 CursorPos = csbi.dwCursorPosition;
 	 	 CursorPos.X  = 0;
 	 	 CursorPos.Y += 1;
 	 	 SetConsoleCursorPosition(console,CursorPos);
 	 }

  	  return NumBytesWritten;
  }

  DWORD vErrorOut(WORD attrib,char* text, ...)
  {
  	  va_list argptr;
  	  DWORD NumBytesWritten;
  	  CONSOLE_SCREEN_BUFFER_INFO  csbi;
  	  COORD CursorPos;
       COORD ScreenSize;
  	  HANDLE console;
  	  char Output[32*1024+2];
  	  int size;

  	  //initialize argptr

  	  va_start(argptr,text);

  	  size = wvsprintf(Output,text,argptr);

  	  va_end(argptr);

  	  AllocConsole();

  	  SetConsoleTitle("Debug Console");

  	  console = GetStdHandle(STD_OUTPUT_HANDLE);

  	  if(console == INVALID_HANDLE_VALUE)
  	  	  MessageBox(NULL,"Invalid Handle Returned by GetStdHandle()",0,MB_OK);

  	  if(!SizeSet)
  	  {
  	  	  ScreenSize.X = 80;
  	  	  ScreenSize.Y = 500;

  	  	  SetConsoleScreenBufferSize(console,ScreenSize);
  	  	  SizeSet = TRUE;
  	  }

  	  if(attrib)
  	  	  SetConsoleTextAttribute(console,attrib);

  	  WriteFile(console, Output, size, &NumBytesWritten, NULL);
  	  Log (Output);
  	  if(*(text+strlen(text)) == '\n')
  	  {
      	  GetConsoleScreenBufferInfo(console, &csbi);
  	  	  CursorPos = csbi.dwCursorPosition;
  	  	  CursorPos.X  = 0;
  	  	  CursorPos.Y += 1;
  	  	  SetConsoleCursorPosition(console,CursorPos);
  	  }

  	  return NumBytesWritten;
  }

  DWORD Log(char* text, ...)
  {
  	  va_list argptr;
  	  FILE* file;
  	  char Output[32*1024+2];
  	  DWORD NumBytesWritten;
  	  DWORD size;
  	  char filename[MAX_PATH];
  	  char *Path = filename;
  	  static int opened = 0;

  	  //initialize argptr

  	  va_start(argptr,text);

  	  size = wvsprintf(Output,text,argptr);

  	  va_end(argptr);

  	  _getcwd(filename, MAX_PATH);
  	  strcat (filename, "\\iff.log");
  	  //	MessageBox(NULL,filename,"Name of the log file",MB_OK);

  	  if(!opened)
  	  {
  	  	  opened = 1;
  	  	  file = fopen (filename, "w");
  	  }
  	  else
  	  {
  	  	  file = fopen (filename, "a");
  	  }

  	  NumBytesWritten = fprintf (file, Output);
  	  fclose (file);

	return NumBytesWritten;
}
