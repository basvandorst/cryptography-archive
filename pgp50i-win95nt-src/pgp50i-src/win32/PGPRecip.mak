# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=PGPrecip - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to PGPrecip - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "PGPrecip - Win32 Release" && "$(CFG)" !=\
 "PGPrecip - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPRecip.mak" CFG="PGPrecip - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPrecip - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PGPrecip - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "PGPrecip - Win32 Release"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "PGPrecip - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "pgprecip\Release"
# PROP Intermediate_Dir "pgprecip\Release"
# PROP Target_Dir ""
OUTDIR=.\pgprecip\Release
INTDIR=.\pgprecip\Release

ALL : "$(OUTDIR)\PGPRecip.dll"

CLEAN : 
	-@erase "$(INTDIR)\DragItem.obj"
	-@erase "$(INTDIR)\KeyServe.obj"
	-@erase "$(INTDIR)\ListSort.obj"
	-@erase "$(INTDIR)\ListSub.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\MoveItem.obj"
	-@erase "$(INTDIR)\OwnDraw.obj"
	-@erase "$(INTDIR)\PGPrecip.res"
	-@erase "$(INTDIR)\RecKeyDB.obj"
	-@erase "$(INTDIR)\RecProc.obj"
	-@erase "$(INTDIR)\strstri.obj"
	-@erase "$(INTDIR)\WarnProc.obj"
	-@erase "$(OUTDIR)\PGPRecip.dll"
	-@erase "$(OUTDIR)\PGPRecip.exp"
	-@erase "$(OUTDIR)\PGPRecip.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/PGPRecip.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\pgprecip\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/PGPrecip.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/PGPRecip.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib lib\bn.lib comctl32.lib lib\keydb.lib keyserversupport\release\keyserversupport.lib pgpcmdlg\release\pgpcmdlg.lib lib\pgp.lib /nologo /subsystem:windows /dll /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib lib\bn.lib comctl32.lib lib\keydb.lib\
 keyserversupport\release\keyserversupport.lib pgpcmdlg\release\pgpcmdlg.lib\
 lib\pgp.lib /nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/PGPRecip.pdb" /machine:I386 /out:"$(OUTDIR)/PGPRecip.dll"\
 /implib:"$(OUTDIR)/PGPRecip.lib" 
LINK32_OBJS= \
	"$(INTDIR)\DragItem.obj" \
	"$(INTDIR)\KeyServe.obj" \
	"$(INTDIR)\ListSort.obj" \
	"$(INTDIR)\ListSub.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\MoveItem.obj" \
	"$(INTDIR)\OwnDraw.obj" \
	"$(INTDIR)\PGPrecip.res" \
	"$(INTDIR)\RecKeyDB.obj" \
	"$(INTDIR)\RecProc.obj" \
	"$(INTDIR)\strstri.obj" \
	"$(INTDIR)\WarnProc.obj"

"$(OUTDIR)\PGPRecip.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PGPrecip - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "pgprecip\Debug"
# PROP Intermediate_Dir "pgprecip\Debug"
# PROP Target_Dir ""
OUTDIR=.\pgprecip\Debug
INTDIR=.\pgprecip\Debug

ALL : "$(OUTDIR)\PGPRecip.dll"

CLEAN : 
	-@erase "$(INTDIR)\DragItem.obj"
	-@erase "$(INTDIR)\KeyServe.obj"
	-@erase "$(INTDIR)\ListSort.obj"
	-@erase "$(INTDIR)\ListSub.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\MoveItem.obj"
	-@erase "$(INTDIR)\OwnDraw.obj"
	-@erase "$(INTDIR)\PGPrecip.res"
	-@erase "$(INTDIR)\RecKeyDB.obj"
	-@erase "$(INTDIR)\RecProc.obj"
	-@erase "$(INTDIR)\strstri.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\WarnProc.obj"
	-@erase "$(OUTDIR)\PGPRecip.dll"
	-@erase "$(OUTDIR)\PGPRecip.exp"
	-@erase "$(OUTDIR)\PGPRecip.ilk"
	-@erase "$(OUTDIR)\PGPRecip.lib"
	-@erase "$(OUTDIR)\PGPRecip.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/PGPRecip.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\pgprecip\Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/PGPrecip.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/PGPRecip.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib lib\bn.lib comctl32.lib lib\keydb.lib keyserversupport\debug\keyserversupport.lib pgpcmdlg\debug\pgpcmdlg.lib lib\pgp.lib /nologo /subsystem:windows /dll /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib lib\bn.lib comctl32.lib lib\keydb.lib\
 keyserversupport\debug\keyserversupport.lib pgpcmdlg\debug\pgpcmdlg.lib\
 lib\pgp.lib /nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/PGPRecip.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/PGPRecip.dll" /implib:"$(OUTDIR)/PGPRecip.lib" 
LINK32_OBJS= \
	"$(INTDIR)\DragItem.obj" \
	"$(INTDIR)\KeyServe.obj" \
	"$(INTDIR)\ListSort.obj" \
	"$(INTDIR)\ListSub.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\MoveItem.obj" \
	"$(INTDIR)\OwnDraw.obj" \
	"$(INTDIR)\PGPrecip.res" \
	"$(INTDIR)\RecKeyDB.obj" \
	"$(INTDIR)\RecProc.obj" \
	"$(INTDIR)\strstri.obj" \
	"$(INTDIR)\WarnProc.obj"

"$(OUTDIR)\PGPRecip.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "PGPrecip - Win32 Release"
# Name "PGPrecip - Win32 Debug"

!IF  "$(CFG)" == "PGPrecip - Win32 Release"

!ELSEIF  "$(CFG)" == "PGPrecip - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\PGPRecip\WarnProc.c
DEP_CPP_WARNP=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPRecip\..\include\config.h"\
	".\PGPRecip\..\include\pgpcomdlg.h"\
	".\PGPRecip\..\include\pgpKeyDB.h"\
	".\PGPRecip\..\include\PGPkeyserversupport.h"\
	".\PGPRecip\..\include\pgpMem.h"\
	".\PGPRecip\..\include\PGPRecip.h"\
	".\PGPRecip\..\include\pgpTypes.h"\
	".\PGPRecip\conerror.h"\
	".\PGPRecip\DragItem.h"\
	".\PGPRecip\KeyServe.h"\
	".\PGPRecip\ListSort.h"\
	".\PGPRecip\ListSub.h"\
	".\PGPRecip\MoveItem.h"\
	".\PGPRecip\OwnDraw.h"\
	".\PGPRecip\precomp.h"\
	".\PGPRecip\RecHelp.h"\
	".\PGPRecip\RecKeyDB.h"\
	".\PGPRecip\RecProc.h"\
	".\PGPRecip\strstri.h"\
	".\PGPRecip\structs.h"\
	".\PGPRecip\WarnProc.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\WarnProc.obj" : $(SOURCE) $(DEP_CPP_WARNP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPRecip\KeyServe.c
DEP_CPP_KEYSE=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\pgpkeyserver\winids.h"\
	".\PGPRecip\..\include\config.h"\
	".\PGPRecip\..\include\pgpcomdlg.h"\
	".\PGPRecip\..\include\pgpKeyDB.h"\
	".\PGPRecip\..\include\PGPkeyserversupport.h"\
	".\PGPRecip\..\include\pgpMem.h"\
	".\PGPRecip\..\include\PGPRecip.h"\
	".\PGPRecip\..\include\pgpTypes.h"\
	".\PGPRecip\conerror.h"\
	".\PGPRecip\DragItem.h"\
	".\PGPRecip\KeyServe.h"\
	".\PGPRecip\ListSort.h"\
	".\PGPRecip\ListSub.h"\
	".\PGPRecip\MoveItem.h"\
	".\PGPRecip\OwnDraw.h"\
	".\PGPRecip\precomp.h"\
	".\PGPRecip\RecHelp.h"\
	".\PGPRecip\RecKeyDB.h"\
	".\PGPRecip\RecProc.h"\
	".\PGPRecip\strstri.h"\
	".\PGPRecip\structs.h"\
	".\PGPRecip\WarnProc.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\KeyServe.obj" : $(SOURCE) $(DEP_CPP_KEYSE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPRecip\ListSort.c
DEP_CPP_LISTS=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPRecip\..\include\config.h"\
	".\PGPRecip\..\include\pgpcomdlg.h"\
	".\PGPRecip\..\include\pgpKeyDB.h"\
	".\PGPRecip\..\include\PGPkeyserversupport.h"\
	".\PGPRecip\..\include\pgpMem.h"\
	".\PGPRecip\..\include\PGPRecip.h"\
	".\PGPRecip\..\include\pgpTypes.h"\
	".\PGPRecip\conerror.h"\
	".\PGPRecip\DragItem.h"\
	".\PGPRecip\KeyServe.h"\
	".\PGPRecip\ListSort.h"\
	".\PGPRecip\ListSub.h"\
	".\PGPRecip\MoveItem.h"\
	".\PGPRecip\OwnDraw.h"\
	".\PGPRecip\precomp.h"\
	".\PGPRecip\RecHelp.h"\
	".\PGPRecip\RecKeyDB.h"\
	".\PGPRecip\RecProc.h"\
	".\PGPRecip\strstri.h"\
	".\PGPRecip\structs.h"\
	".\PGPRecip\WarnProc.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\ListSort.obj" : $(SOURCE) $(DEP_CPP_LISTS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPRecip\ListSub.c
DEP_CPP_LISTSU=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPRecip\..\include\config.h"\
	".\PGPRecip\..\include\pgpcomdlg.h"\
	".\PGPRecip\..\include\pgpKeyDB.h"\
	".\PGPRecip\..\include\PGPkeyserversupport.h"\
	".\PGPRecip\..\include\pgpMem.h"\
	".\PGPRecip\..\include\PGPRecip.h"\
	".\PGPRecip\..\include\pgpTypes.h"\
	".\PGPRecip\conerror.h"\
	".\PGPRecip\DragItem.h"\
	".\PGPRecip\KeyServe.h"\
	".\PGPRecip\ListSort.h"\
	".\PGPRecip\ListSub.h"\
	".\PGPRecip\MoveItem.h"\
	".\PGPRecip\OwnDraw.h"\
	".\PGPRecip\precomp.h"\
	".\PGPRecip\RecHelp.h"\
	".\PGPRecip\RecKeyDB.h"\
	".\PGPRecip\RecProc.h"\
	".\PGPRecip\strstri.h"\
	".\PGPRecip\structs.h"\
	".\PGPRecip\WarnProc.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\ListSub.obj" : $(SOURCE) $(DEP_CPP_LISTSU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPRecip\main.c
DEP_CPP_MAIN_=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPRecip\..\include\config.h"\
	".\PGPRecip\..\include\pgpcomdlg.h"\
	".\PGPRecip\..\include\pgpKeyDB.h"\
	".\PGPRecip\..\include\PGPkeyserversupport.h"\
	".\PGPRecip\..\include\pgpMem.h"\
	".\PGPRecip\..\include\PGPRecip.h"\
	".\PGPRecip\..\include\pgpTypes.h"\
	".\PGPRecip\conerror.h"\
	".\PGPRecip\DragItem.h"\
	".\PGPRecip\KeyServe.h"\
	".\PGPRecip\ListSort.h"\
	".\PGPRecip\ListSub.h"\
	".\PGPRecip\MoveItem.h"\
	".\PGPRecip\OwnDraw.h"\
	".\PGPRecip\precomp.h"\
	".\PGPRecip\RecHelp.h"\
	".\PGPRecip\RecKeyDB.h"\
	".\PGPRecip\RecProc.h"\
	".\PGPRecip\strstri.h"\
	".\PGPRecip\structs.h"\
	".\PGPRecip\WarnProc.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPRecip\MoveItem.c
DEP_CPP_MOVEI=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPRecip\..\include\config.h"\
	".\PGPRecip\..\include\pgpcomdlg.h"\
	".\PGPRecip\..\include\pgpKeyDB.h"\
	".\PGPRecip\..\include\PGPkeyserversupport.h"\
	".\PGPRecip\..\include\pgpMem.h"\
	".\PGPRecip\..\include\PGPRecip.h"\
	".\PGPRecip\..\include\pgpTypes.h"\
	".\PGPRecip\conerror.h"\
	".\PGPRecip\DragItem.h"\
	".\PGPRecip\KeyServe.h"\
	".\PGPRecip\ListSort.h"\
	".\PGPRecip\ListSub.h"\
	".\PGPRecip\MoveItem.h"\
	".\PGPRecip\OwnDraw.h"\
	".\PGPRecip\precomp.h"\
	".\PGPRecip\RecHelp.h"\
	".\PGPRecip\RecKeyDB.h"\
	".\PGPRecip\RecProc.h"\
	".\PGPRecip\strstri.h"\
	".\PGPRecip\structs.h"\
	".\PGPRecip\WarnProc.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\MoveItem.obj" : $(SOURCE) $(DEP_CPP_MOVEI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPRecip\OwnDraw.c
DEP_CPP_OWNDR=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPRecip\..\include\config.h"\
	".\PGPRecip\..\include\pgpcomdlg.h"\
	".\PGPRecip\..\include\pgpKeyDB.h"\
	".\PGPRecip\..\include\PGPkeyserversupport.h"\
	".\PGPRecip\..\include\pgpMem.h"\
	".\PGPRecip\..\include\PGPRecip.h"\
	".\PGPRecip\..\include\pgpTypes.h"\
	".\PGPRecip\conerror.h"\
	".\PGPRecip\DragItem.h"\
	".\PGPRecip\KeyServe.h"\
	".\PGPRecip\ListSort.h"\
	".\PGPRecip\ListSub.h"\
	".\PGPRecip\MoveItem.h"\
	".\PGPRecip\OwnDraw.h"\
	".\PGPRecip\precomp.h"\
	".\PGPRecip\RecHelp.h"\
	".\PGPRecip\RecKeyDB.h"\
	".\PGPRecip\RecProc.h"\
	".\PGPRecip\strstri.h"\
	".\PGPRecip\structs.h"\
	".\PGPRecip\WarnProc.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\OwnDraw.obj" : $(SOURCE) $(DEP_CPP_OWNDR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPRecip\RecKeyDB.c
DEP_CPP_RECKE=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPRecip\..\include\config.h"\
	".\PGPRecip\..\include\pgpcomdlg.h"\
	".\PGPRecip\..\include\pgpKeyDB.h"\
	".\PGPRecip\..\include\PGPkeyserversupport.h"\
	".\PGPRecip\..\include\pgpMem.h"\
	".\PGPRecip\..\include\PGPRecip.h"\
	".\PGPRecip\..\include\pgpTypes.h"\
	".\PGPRecip\conerror.h"\
	".\PGPRecip\DragItem.h"\
	".\PGPRecip\KeyServe.h"\
	".\PGPRecip\ListSort.h"\
	".\PGPRecip\ListSub.h"\
	".\PGPRecip\MoveItem.h"\
	".\PGPRecip\OwnDraw.h"\
	".\PGPRecip\precomp.h"\
	".\PGPRecip\RecHelp.h"\
	".\PGPRecip\RecKeyDB.h"\
	".\PGPRecip\RecProc.h"\
	".\PGPRecip\strstri.h"\
	".\PGPRecip\structs.h"\
	".\PGPRecip\WarnProc.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\RecKeyDB.obj" : $(SOURCE) $(DEP_CPP_RECKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPRecip\RecProc.c
DEP_CPP_RECPR=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPRecip\..\include\config.h"\
	".\PGPRecip\..\include\pgpcomdlg.h"\
	".\PGPRecip\..\include\pgpKeyDB.h"\
	".\PGPRecip\..\include\PGPkeyserversupport.h"\
	".\PGPRecip\..\include\pgpMem.h"\
	".\PGPRecip\..\include\PGPRecip.h"\
	".\PGPRecip\..\include\pgpTypes.h"\
	".\PGPRecip\conerror.h"\
	".\PGPRecip\DragItem.h"\
	".\PGPRecip\KeyServe.h"\
	".\PGPRecip\ListSort.h"\
	".\PGPRecip\ListSub.h"\
	".\PGPRecip\MoveItem.h"\
	".\PGPRecip\OwnDraw.h"\
	".\PGPRecip\precomp.h"\
	".\PGPRecip\RecHelp.h"\
	".\PGPRecip\RecKeyDB.h"\
	".\PGPRecip\RecProc.h"\
	".\PGPRecip\strstri.h"\
	".\PGPRecip\structs.h"\
	".\PGPRecip\WarnProc.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\RecProc.obj" : $(SOURCE) $(DEP_CPP_RECPR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPRecip\strstri.c

"$(INTDIR)\strstri.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPRecip\DragItem.c
DEP_CPP_DRAGI=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpLeaks.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\PGPRecip\..\include\config.h"\
	".\PGPRecip\..\include\pgpcomdlg.h"\
	".\PGPRecip\..\include\pgpKeyDB.h"\
	".\PGPRecip\..\include\PGPkeyserversupport.h"\
	".\PGPRecip\..\include\pgpMem.h"\
	".\PGPRecip\..\include\PGPRecip.h"\
	".\PGPRecip\..\include\pgpTypes.h"\
	".\PGPRecip\conerror.h"\
	".\PGPRecip\DragItem.h"\
	".\PGPRecip\KeyServe.h"\
	".\PGPRecip\ListSort.h"\
	".\PGPRecip\ListSub.h"\
	".\PGPRecip\MoveItem.h"\
	".\PGPRecip\OwnDraw.h"\
	".\PGPRecip\precomp.h"\
	".\PGPRecip\RecHelp.h"\
	".\PGPRecip\RecKeyDB.h"\
	".\PGPRecip\RecProc.h"\
	".\PGPRecip\strstri.h"\
	".\PGPRecip\structs.h"\
	".\PGPRecip\WarnProc.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	

"$(INTDIR)\DragItem.obj" : $(SOURCE) $(DEP_CPP_DRAGI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\PGPRecip\PGPrecip.rc
DEP_RSC_PGPRE=\
	".\PGPRecip\..\include\pgpversion.h"\
	".\PGPRecip\PGPrecV.rc"\
	".\PGPRecip\Res\dlgicon.ico"\
	".\PGPRecip\Res\multdsa.bmp"\
	".\PGPRecip\Res\multrsa.bmp"\
	".\PGPRecip\Res\unknown.bmp"\
	".\PGPRecip\Res\valdsa.bmp"\
	".\PGPRecip\Res\valrsa.bmp"\
	

!IF  "$(CFG)" == "PGPrecip - Win32 Release"


"$(INTDIR)\PGPrecip.res" : $(SOURCE) $(DEP_RSC_PGPRE) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/PGPrecip.res" /i "PGPRecip" /d "NDEBUG"\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPrecip - Win32 Debug"


"$(INTDIR)\PGPrecip.res" : $(SOURCE) $(DEP_RSC_PGPRE) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/PGPrecip.res" /i "PGPRecip" /d "_DEBUG"\
 $(SOURCE)


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
