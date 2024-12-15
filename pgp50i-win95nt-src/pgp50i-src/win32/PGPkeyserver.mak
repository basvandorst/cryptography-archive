# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=PGPkeyserver - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to PGPkeyserver - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "PGPkeyserver - Win32 Release" && "$(CFG)" !=\
 "PGPkeyserver - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPkeyserver.mak" CFG="PGPkeyserver - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPkeyserver - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PGPkeyserver - Win32 Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
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
# PROP Target_Last_Scanned "PGPkeyserver - Win32 Debug"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "PGPkeyserver - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "pgpkeyserver\Release"
# PROP Intermediate_Dir "pgpkeyserver\Release"
# PROP Target_Dir ""
OUTDIR=.\pgpkeyserver\Release
INTDIR=.\pgpkeyserver\Release

ALL : "$(OUTDIR)\PGPks.dll"

CLEAN : 
	-@erase "$(INTDIR)\config.obj"
	-@erase "$(INTDIR)\getkeys.obj"
	-@erase "$(INTDIR)\keyserver.obj"
	-@erase "$(INTDIR)\menus.obj"
	-@erase "$(INTDIR)\pgpkeyserver.res"
	-@erase "$(INTDIR)\settings.obj"
	-@erase "$(INTDIR)\utils.obj"
	-@erase "$(OUTDIR)\PGPks.dll"
	-@erase "$(OUTDIR)\PGPks.exp"
	-@erase "$(OUTDIR)\PGPks.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/PGPkeyserver.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\pgpkeyserver\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/pgpkeyserver.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/PGPkeyserver.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib lib\bn.lib lib\pgp.lib lib\keydb.lib wsock32.lib lib\simple.lib /nologo /subsystem:windows /dll /machine:I386 /out:"pgpkeyserver\Release/PGPks.dll"
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib lib\bn.lib lib\pgp.lib lib\keydb.lib wsock32.lib lib\simple.lib\
 /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)/PGPks.pdb"\
 /machine:I386 /out:"$(OUTDIR)/PGPks.dll" /implib:"$(OUTDIR)/PGPks.lib" 
LINK32_OBJS= \
	"$(INTDIR)\config.obj" \
	"$(INTDIR)\getkeys.obj" \
	"$(INTDIR)\keyserver.obj" \
	"$(INTDIR)\menus.obj" \
	"$(INTDIR)\pgpkeyserver.res" \
	"$(INTDIR)\settings.obj" \
	"$(INTDIR)\utils.obj"

"$(OUTDIR)\PGPks.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PGPkeyserver - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "pgpkeyserver\Debug"
# PROP Intermediate_Dir "pgpkeyserver\Debug"
# PROP Target_Dir ""
OUTDIR=.\pgpkeyserver\Debug
INTDIR=.\pgpkeyserver\Debug

ALL : "$(OUTDIR)\PGPks.dll"

CLEAN : 
	-@erase "$(INTDIR)\config.obj"
	-@erase "$(INTDIR)\getkeys.obj"
	-@erase "$(INTDIR)\keyserver.obj"
	-@erase "$(INTDIR)\menus.obj"
	-@erase "$(INTDIR)\pgpkeyserver.res"
	-@erase "$(INTDIR)\settings.obj"
	-@erase "$(INTDIR)\utils.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\PGPks.dll"
	-@erase "$(OUTDIR)\PGPks.exp"
	-@erase "$(OUTDIR)\PGPks.ilk"
	-@erase "$(OUTDIR)\PGPks.lib"
	-@erase "$(OUTDIR)\PGPks.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/PGPkeyserver.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\pgpkeyserver\Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/pgpkeyserver.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/PGPkeyserver.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib lib\bn.lib lib\pgp.lib lib\keydb.lib wsock32.lib lib\simple.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"pgpkeyserver\Debug/PGPks.dll"
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib lib\bn.lib lib\pgp.lib lib\keydb.lib wsock32.lib lib\simple.lib\
 /nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)/PGPks.pdb"\
 /debug /machine:I386 /out:"$(OUTDIR)/PGPks.dll" /implib:"$(OUTDIR)/PGPks.lib" 
LINK32_OBJS= \
	"$(INTDIR)\config.obj" \
	"$(INTDIR)\getkeys.obj" \
	"$(INTDIR)\keyserver.obj" \
	"$(INTDIR)\menus.obj" \
	"$(INTDIR)\pgpkeyserver.res" \
	"$(INTDIR)\settings.obj" \
	"$(INTDIR)\utils.obj"

"$(OUTDIR)\PGPks.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "PGPkeyserver - Win32 Release"
# Name "PGPkeyserver - Win32 Debug"

!IF  "$(CFG)" == "PGPkeyserver - Win32 Release"

!ELSEIF  "$(CFG)" == "PGPkeyserver - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\pgpkeyserver\keyserver.c
DEP_CPP_KEYSE=\
	".\pgpkeyserver\keyserver.h"\
	

"$(INTDIR)\keyserver.obj" : $(SOURCE) $(DEP_CPP_KEYSE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpkeyserver\pgpkeyserver.rc
DEP_RSC_PGPKE=\
	".\pgpkeyserver\..\include\pgpversion.h"\
	".\pgpkeyserver\PGPkeyserverVer.rc"\
	

!IF  "$(CFG)" == "PGPkeyserver - Win32 Release"


"$(INTDIR)\pgpkeyserver.res" : $(SOURCE) $(DEP_RSC_PGPKE) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/pgpkeyserver.res" /i "pgpkeyserver" /d\
 "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "PGPkeyserver - Win32 Debug"


"$(INTDIR)\pgpkeyserver.res" : $(SOURCE) $(DEP_RSC_PGPKE) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/pgpkeyserver.res" /i "pgpkeyserver" /d\
 "_DEBUG" $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpkeyserver\settings.c
DEP_CPP_SETTI=\
	".\pgpkeyserver\..\include\config.h"\
	".\pgpkeyserver\..\include\pgpkeyserversupport.h"\
	".\pgpkeyserver\keyserver.h"\
	".\pgpkeyserver\ksconfig.h"\
	".\pgpkeyserver\PGPkeyserverHelp.h"\
	".\pgpkeyserver\settings.h"\
	

"$(INTDIR)\settings.obj" : $(SOURCE) $(DEP_CPP_SETTI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpkeyserver\menus.c
DEP_CPP_MENUS=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\pgpkeyserver\..\include\config.h"\
	".\pgpkeyserver\..\include\pgpkeydb.h"\
	".\pgpkeyserver\..\include\pgpkeyserversupport.h"\
	".\pgpkeyserver\getkeys.h"\
	".\pgpkeyserver\keyserver.h"\
	".\pgpkeyserver\menus.h"\
	".\pgpkeyserver\winids.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\menus.obj" : $(SOURCE) $(DEP_CPP_MENUS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpkeyserver\getkeys.c
DEP_CPP_GETKE=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpEncPipe.h"\
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
	".\INCLUDE\pgpTextFilt.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\pgpkeyserver\..\include\config.h"\
	".\pgpkeyserver\..\include\pgpcomdlg.h"\
	".\pgpkeyserver\..\include\pgpkeydb.h"\
	".\pgpkeyserver\..\include\pgpkeyserversupport.h"\
	".\pgpkeyserver\..\include\pgpmem.h"\
	".\pgpkeyserver\..\include\spgp.h"\
	".\pgpkeyserver\getkeys.h"\
	".\pgpkeyserver\keyserver.h"\
	".\pgpkeyserver\ksconfig.h"\
	".\pgpkeyserver\utils.h"\
	".\pgpkeyserver\winids.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\getkeys.obj" : $(SOURCE) $(DEP_CPP_GETKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpkeyserver\config.c
DEP_CPP_CONFI=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\pgpkeyserver\..\include\config.h"\
	".\pgpkeyserver\..\include\pgpkeydb.h"\
	".\pgpkeyserver\..\include\pgpkeyserversupport.h"\
	".\pgpkeyserver\ksconfig.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\config.obj" : $(SOURCE) $(DEP_CPP_CONFI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpkeyserver\utils.c
DEP_CPP_UTILS=\
	".\INCLUDE\pgpDebug.h"\
	".\INCLUDE\pgpErr.h"\
	".\INCLUDE\pgpFile.h"\
	".\INCLUDE\pgpFileMod.h"\
	".\INCLUDE\pgpFileRef.h"\
	".\INCLUDE\pgpHash.h"\
	".\INCLUDE\pgpPubKey.h"\
	".\INCLUDE\pgpRndom.h"\
	".\INCLUDE\pgpRndPool.h"\
	".\INCLUDE\pgpRngPub.h"\
	".\INCLUDE\pgpTimeDate.h"\
	".\INCLUDE\pgpTrstPkt.h"\
	".\INCLUDE\pgpTrust.h"\
	".\INCLUDE\pgpTypes.h"\
	".\INCLUDE\pgpUsuals.h"\
	".\pgpkeyserver\..\include\config.h"\
	".\pgpkeyserver\..\include\pgpcomdlg.h"\
	".\pgpkeyserver\..\include\pgpkeydb.h"\
	".\pgpkeyserver\..\include\pgpkeyserversupport.h"\
	".\pgpkeyserver\keyserver.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\utils.obj" : $(SOURCE) $(DEP_CPP_UTILS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
