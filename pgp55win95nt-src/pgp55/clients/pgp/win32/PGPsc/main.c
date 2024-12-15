/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: main.c,v 1.5 1997/09/03 17:15:07 wjb Exp $
____________________________________________________________________________*/
#include "precomp.h"

#ifdef WIN32

BOOL WINAPI DllMain (HINSTANCE hinstDll, DWORD fdwReason,LPVOID lpvReserved)
{
   switch (fdwReason) {
      case DLL_PROCESS_ATTACH:
         g_hinst = hinstDll;
		 //vErrorOut(fg_white,"DLL_PROCESS_ATTACH\n");
         break;
      case DLL_THREAD_ATTACH:
		  //vErrorOut(fg_white,"DLL_THREAD_ATTACH\n");
         break;
      case DLL_THREAD_DETACH:
		  //vErrorOut(fg_red,"DLL_THREAD_DETACH\n");
		break;
      case DLL_PROCESS_DETACH:
		  //vErrorOut(fg_red,"DLL_PROCESS_DETACH\n");
         break;
   }
   return(TRUE);
}

#else  

int FAR PASCAL LibMain (HANDLE hInstance,
                        WORD wDataSeg,
                        WORD wHeapSize,
                        LPSTR lpszCmdLine)
{
   g_hinst = hInstance;

   if (wHeapSize != 0)  // If DLL data seg is MOVEABLE
      UnlockData (0);

   return (1);
}

#endif

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
