# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=keydb - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to keydb - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "keydb - Win32 Release" && "$(CFG)" != "keydb - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "keydb.mak" CFG="keydb - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "keydb - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "keydb - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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
# PROP Target_Last_Scanned "keydb - Win32 Debug"
RSC=rc.exe
MTL=mktyplib.exe
CPP=cl.exe

!IF  "$(CFG)" == "keydb - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\keydb.dll"

CLEAN : 
	-@erase "$(INTDIR)\pgpDearmor.obj"
	-@erase "$(INTDIR)\pgpFileDB.obj"
	-@erase "$(INTDIR)\pgpFilteredSet.obj"
	-@erase "$(INTDIR)\pgpKeyIter.obj"
	-@erase "$(INTDIR)\pgpKeyLib.obj"
	-@erase "$(INTDIR)\pgpKeyMan.obj"
	-@erase "$(INTDIR)\pgpKeySet.obj"
	-@erase "$(INTDIR)\pgpKeyUpd.obj"
	-@erase "$(INTDIR)\pgpMemDB.obj"
	-@erase "$(INTDIR)\pgpPrefs.obj"
	-@erase "$(INTDIR)\pgpPrefWin32.obj"
	-@erase "$(INTDIR)\pgpUnionDB.obj"
	-@erase "$(OUTDIR)\keydb.dll"
	-@erase "$(OUTDIR)\keydb.exp"
	-@erase "$(OUTDIR)\keydb.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../../.." /I "../../../include" /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D DEBUG=0 /D UNFINISHED_CODE_ALLOWED=1 /D HAVE_CONFIG_H=1 /D PGPTRUSTMODEL=0 /D "PGPKDBLIB" /D USE_REGISTRY=1 /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "../../.." /I "../../../include" /I\
 "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D DEBUG=0 /D\
 UNFINISHED_CODE_ALLOWED=1 /D HAVE_CONFIG_H=1 /D PGPTRUSTMODEL=0 /D "PGPKDBLIB"\
 /D USE_REGISTRY=1 /Fp"$(INTDIR)/keydb.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/keydb.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\..\bn\release\bn.lib ..\..\pgp\release\pgp.lib /nologo /subsystem:windows /dll /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib ..\..\bn\release\bn.lib ..\..\pgp\release\pgp.lib /nologo\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)/keydb.pdb"\
 /machine:I386 /out:"$(OUTDIR)/keydb.dll" /implib:"$(OUTDIR)/keydb.lib" 
LINK32_OBJS= \
	"$(INTDIR)\pgpDearmor.obj" \
	"$(INTDIR)\pgpFileDB.obj" \
	"$(INTDIR)\pgpFilteredSet.obj" \
	"$(INTDIR)\pgpKeyIter.obj" \
	"$(INTDIR)\pgpKeyLib.obj" \
	"$(INTDIR)\pgpKeyMan.obj" \
	"$(INTDIR)\pgpKeySet.obj" \
	"$(INTDIR)\pgpKeyUpd.obj" \
	"$(INTDIR)\pgpMemDB.obj" \
	"$(INTDIR)\pgpPrefs.obj" \
	"$(INTDIR)\pgpPrefWin32.obj" \
	"$(INTDIR)\pgpUnionDB.obj"

"$(OUTDIR)\keydb.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "keydb - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\keydb.dll"

CLEAN : 
	-@erase "$(INTDIR)\pgpDearmor.obj"
	-@erase "$(INTDIR)\pgpFileDB.obj"
	-@erase "$(INTDIR)\pgpFilteredSet.obj"
	-@erase "$(INTDIR)\pgpKeyIter.obj"
	-@erase "$(INTDIR)\pgpKeyLib.obj"
	-@erase "$(INTDIR)\pgpKeyMan.obj"
	-@erase "$(INTDIR)\pgpKeySet.obj"
	-@erase "$(INTDIR)\pgpKeyUpd.obj"
	-@erase "$(INTDIR)\pgpMemDB.obj"
	-@erase "$(INTDIR)\pgpPrefs.obj"
	-@erase "$(INTDIR)\pgpPrefWin32.obj"
	-@erase "$(INTDIR)\pgpUnionDB.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\keydb.dll"
	-@erase "$(OUTDIR)\keydb.exp"
	-@erase "$(OUTDIR)\keydb.ilk"
	-@erase "$(OUTDIR)\keydb.lib"
	-@erase "$(OUTDIR)\keydb.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "../../.." /I "../../../include" /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D DEBUG=0 /D UNFINISHED_CODE_ALLOWED=1 /D HAVE_CONFIG_H=1 /D PGPTRUSTMODEL=0 /D "PGPKDBLIB" /D USE_REGISTRY=1 /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I "../../.." /I "../../../include"\
 /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D DEBUG=0 /D\
 UNFINISHED_CODE_ALLOWED=1 /D HAVE_CONFIG_H=1 /D PGPTRUSTMODEL=0 /D "PGPKDBLIB"\
 /D USE_REGISTRY=1 /Fp"$(INTDIR)/keydb.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/"\
 /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/keydb.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\..\bn\debug\bn.lib ..\..\pgp\debug\pgp.lib /nologo /subsystem:windows /dll /debug /machine:I386
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib ..\..\bn\debug\bn.lib ..\..\pgp\debug\pgp.lib /nologo\
 /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)/keydb.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/keydb.dll" /implib:"$(OUTDIR)/keydb.lib" 
LINK32_OBJS= \
	"$(INTDIR)\pgpDearmor.obj" \
	"$(INTDIR)\pgpFileDB.obj" \
	"$(INTDIR)\pgpFilteredSet.obj" \
	"$(INTDIR)\pgpKeyIter.obj" \
	"$(INTDIR)\pgpKeyLib.obj" \
	"$(INTDIR)\pgpKeyMan.obj" \
	"$(INTDIR)\pgpKeySet.obj" \
	"$(INTDIR)\pgpKeyUpd.obj" \
	"$(INTDIR)\pgpMemDB.obj" \
	"$(INTDIR)\pgpPrefs.obj" \
	"$(INTDIR)\pgpPrefWin32.obj" \
	"$(INTDIR)\pgpUnionDB.obj"

"$(OUTDIR)\keydb.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "keydb - Win32 Release"
# Name "keydb - Win32 Debug"

!IF  "$(CFG)" == "keydb - Win32 Release"

!ELSEIF  "$(CFG)" == "keydb - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\pgpUnionDB.c
DEP_CPP_PGPUN=\
	".\../../../include\pgpDebug.h"\
	".\../../../include\pgpErr.h"\
	".\../../../include\pgpFile.h"\
	".\../../../include\pgpFileMod.h"\
	".\../../../include\pgpFileRef.h"\
	".\../../../include\pgpHash.h"\
	".\../../../include\pgpLeaks.h"\
	".\../../../include\pgpMem.h"\
	".\../../../include\pgpPubKey.h"\
	".\../../../include\pgpRndom.h"\
	".\../../../include\pgpRndPool.h"\
	".\../../../include\pgpRngPub.h"\
	".\../../../include\pgpTimeDate.h"\
	".\../../../include\pgpTrstPkt.h"\
	".\../../../include\pgpTrust.h"\
	".\../../../include\pgpTypes.h"\
	".\../../../include\pgpUsuals.h"\
	".\../../..\config.h"\
	".\../../include\pgpMemPool.h"\
	".\pgpKDBint.h"\
	".\pgpKeyDB.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

"$(INTDIR)\pgpUnionDB.obj" : $(SOURCE) $(DEP_CPP_PGPUN) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpPrefWin32.c
DEP_CPP_PGPPR=\
	".\../../../include\pgpDebug.h"\
	".\../../../include\pgpEnv.h"\
	".\../../../include\pgpErr.h"\
	".\../../../include\pgpFile.h"\
	".\../../../include\pgpFileMod.h"\
	".\../../../include\pgpFileRef.h"\
	".\../../../include\pgpHash.h"\
	".\../../../include\pgpLeaks.h"\
	".\../../../include\pgpMem.h"\
	".\../../../include\pgpPubKey.h"\
	".\../../../include\pgpRndom.h"\
	".\../../../include\pgpRndPool.h"\
	".\../../../include\pgpRngPub.h"\
	".\../../../include\pgpTimeDate.h"\
	".\../../../include\pgpTrstPkt.h"\
	".\../../../include\pgpTrust.h"\
	".\../../../include\pgpTypes.h"\
	".\../../../include\pgpUsuals.h"\
	".\../../..\config.h"\
	".\../../include\pgpMemPool.h"\
	".\pgpKDBint.h"\
	".\pgpKeyDB.h"\
	".\pgpPrefsInt.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

"$(INTDIR)\pgpPrefWin32.obj" : $(SOURCE) $(DEP_CPP_PGPPR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpMemDB.c
DEP_CPP_PGPME=\
	".\../../../include\pgpDebug.h"\
	".\../../../include\pgpEnv.h"\
	".\../../../include\pgpErr.h"\
	".\../../../include\pgpFile.h"\
	".\../../../include\pgpFileMod.h"\
	".\../../../include\pgpFileRef.h"\
	".\../../../include\pgpHash.h"\
	".\../../../include\pgpLeaks.h"\
	".\../../../include\pgpMem.h"\
	".\../../../include\pgpPubKey.h"\
	".\../../../include\pgpRndom.h"\
	".\../../../include\pgpRndPool.h"\
	".\../../../include\pgpRngPub.h"\
	".\../../../include\pgpTimeDate.h"\
	".\../../../include\pgpTrstPkt.h"\
	".\../../../include\pgpTrust.h"\
	".\../../../include\pgpTypes.h"\
	".\../../../include\pgpUsuals.h"\
	".\../../..\config.h"\
	".\../../include\pgpMemPool.h"\
	".\pgpKDBint.h"\
	".\pgpKeyDB.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

"$(INTDIR)\pgpMemDB.obj" : $(SOURCE) $(DEP_CPP_PGPME) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpKeySet.c
DEP_CPP_PGPKE=\
	".\../../../include\pgpDebug.h"\
	".\../../../include\pgpErr.h"\
	".\../../../include\pgpFile.h"\
	".\../../../include\pgpFileMod.h"\
	".\../../../include\pgpFileRef.h"\
	".\../../../include\pgpHash.h"\
	".\../../../include\pgpLeaks.h"\
	".\../../../include\pgpMem.h"\
	".\../../../include\pgpPubKey.h"\
	".\../../../include\pgpRndom.h"\
	".\../../../include\pgpRndPool.h"\
	".\../../../include\pgpRngMnt.h"\
	".\../../../include\pgpRngPub.h"\
	".\../../../include\pgpTimeDate.h"\
	".\../../../include\pgpTrstPkt.h"\
	".\../../../include\pgpTrust.h"\
	".\../../../include\pgpTypes.h"\
	".\../../../include\pgpUsuals.h"\
	".\../../..\config.h"\
	".\../../include\pgpMemPool.h"\
	".\pgpKDBint.h"\
	".\pgpKeyDB.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

"$(INTDIR)\pgpKeySet.obj" : $(SOURCE) $(DEP_CPP_PGPKE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpKeyMan.c
DEP_CPP_PGPKEY=\
	".\../../../include\bn.h"\
	".\../../../include\pgpDebug.h"\
	".\../../../include\pgpEnv.h"\
	".\../../../include\pgpErr.h"\
	".\../../../include\pgpFile.h"\
	".\../../../include\pgpFileMod.h"\
	".\../../../include\pgpFileRef.h"\
	".\../../../include\pgpHash.h"\
	".\../../../include\pgpKeySpec.h"\
	".\../../../include\pgpLeaks.h"\
	".\../../../include\pgpMem.h"\
	".\../../../include\pgpPubKey.h"\
	".\../../../include\pgpRndom.h"\
	".\../../../include\pgpRndPool.h"\
	".\../../../include\pgpRngPub.h"\
	".\../../../include\pgpRngRead.h"\
	".\../../../include\pgpSigSpec.h"\
	".\../../../include\pgpTimeDate.h"\
	".\../../../include\pgpTrstPkt.h"\
	".\../../../include\pgpTrust.h"\
	".\../../../include\pgpTypes.h"\
	".\../../../include\pgpUsuals.h"\
	".\../../..\config.h"\
	".\../../include\pgpMemPool.h"\
	".\pgpKDBint.h"\
	".\pgpKeyDB.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

"$(INTDIR)\pgpKeyMan.obj" : $(SOURCE) $(DEP_CPP_PGPKEY) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpKeyLib.c
DEP_CPP_PGPKEYL=\
	".\../../../include\pgpDebug.h"\
	".\../../../include\pgpEnv.h"\
	".\../../../include\pgpErr.h"\
	".\../../../include\pgpFile.h"\
	".\../../../include\pgpFileMod.h"\
	".\../../../include\pgpFileNames.h"\
	".\../../../include\pgpFileRef.h"\
	".\../../../include\pgpHash.h"\
	".\../../../include\pgpLeaks.h"\
	".\../../../include\pgpMem.h"\
	".\../../../include\pgpPubKey.h"\
	".\../../../include\pgpRndom.h"\
	".\../../../include\pgpRndPool.h"\
	".\../../../include\pgpRndSeed.h"\
	".\../../../include\pgpRngPub.h"\
	".\../../../include\pgpTimeDate.h"\
	".\../../../include\pgpTrstPkt.h"\
	".\../../../include\pgpTrust.h"\
	".\../../../include\pgpTypes.h"\
	".\../../../include\pgpUsuals.h"\
	".\../../..\config.h"\
	".\../../include\pgpMemPool.h"\
	".\pgpKDBint.h"\
	".\pgpKeyDB.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	
NODEP_CPP_PGPKEYL=\
	".\MacFiles.h"\
	".\MacStrings.h"\
	

"$(INTDIR)\pgpKeyLib.obj" : $(SOURCE) $(DEP_CPP_PGPKEYL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpKeyIter.c
DEP_CPP_PGPKEYI=\
	".\../../../include\pgpDebug.h"\
	".\../../../include\pgpErr.h"\
	".\../../../include\pgpFile.h"\
	".\../../../include\pgpFileMod.h"\
	".\../../../include\pgpFileRef.h"\
	".\../../../include\pgpHash.h"\
	".\../../../include\pgpLeaks.h"\
	".\../../../include\pgpMem.h"\
	".\../../../include\pgpPubKey.h"\
	".\../../../include\pgpRndom.h"\
	".\../../../include\pgpRndPool.h"\
	".\../../../include\pgpRngPub.h"\
	".\../../../include\pgpTimeDate.h"\
	".\../../../include\pgpTrstPkt.h"\
	".\../../../include\pgpTrust.h"\
	".\../../../include\pgpTypes.h"\
	".\../../../include\pgpUsuals.h"\
	".\../../..\config.h"\
	".\../../include\pgpMemPool.h"\
	".\pgpKDBint.h"\
	".\pgpKeyDB.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

"$(INTDIR)\pgpKeyIter.obj" : $(SOURCE) $(DEP_CPP_PGPKEYI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpFileDB.c
DEP_CPP_PGPFI=\
	".\../../../include\pgpDebug.h"\
	".\../../../include\pgpErr.h"\
	".\../../../include\pgpFile.h"\
	".\../../../include\pgpFileMod.h"\
	".\../../../include\pgpFileNames.h"\
	".\../../../include\pgpFileRef.h"\
	".\../../../include\pgpHash.h"\
	".\../../../include\pgpLeaks.h"\
	".\../../../include\pgpMem.h"\
	".\../../../include\pgpPubKey.h"\
	".\../../../include\pgpRndom.h"\
	".\../../../include\pgpRndPool.h"\
	".\../../../include\pgpRngPub.h"\
	".\../../../include\pgpRngRead.h"\
	".\../../../include\pgpTimeDate.h"\
	".\../../../include\pgpTrstPkt.h"\
	".\../../../include\pgpTrust.h"\
	".\../../../include\pgpTypes.h"\
	".\../../../include\pgpUsuals.h"\
	".\../../..\config.h"\
	".\../../include\pgpMemPool.h"\
	".\pgpKDBint.h"\
	".\pgpKeyDB.h"\
	{$(INCLUDE)}"\sys\Stat.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

"$(INTDIR)\pgpFileDB.obj" : $(SOURCE) $(DEP_CPP_PGPFI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpKeyUpd.c
DEP_CPP_PGPKEYU=\
	".\../../../include\pgpDebug.h"\
	".\../../../include\pgpErr.h"\
	".\../../../include\pgpFile.h"\
	".\../../../include\pgpFileMod.h"\
	".\../../../include\pgpFileRef.h"\
	".\../../../include\pgpHash.h"\
	".\../../../include\pgpLeaks.h"\
	".\../../../include\pgpMem.h"\
	".\../../../include\pgpPubKey.h"\
	".\../../../include\pgpRndom.h"\
	".\../../../include\pgpRndPool.h"\
	".\../../../include\pgpRngPub.h"\
	".\../../../include\pgpSigSpec.h"\
	".\../../../include\pgpTimeDate.h"\
	".\../../../include\pgpTrstPkt.h"\
	".\../../../include\pgpTrust.h"\
	".\../../../include\pgpTypes.h"\
	".\../../../include\pgpUsuals.h"\
	".\../../..\config.h"\
	".\../../include\pgpMemPool.h"\
	".\pgpKDBint.h"\
	".\pgpKeyDB.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

"$(INTDIR)\pgpKeyUpd.obj" : $(SOURCE) $(DEP_CPP_PGPKEYU) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpDearmor.c
DEP_CPP_PGPDE=\
	".\../../../include\pgpAnnotate.h"\
	".\../../../include\pgpArmor.h"\
	".\../../../include\pgpDebug.h"\
	".\../../../include\pgpDecPipe.h"\
	".\../../../include\pgpDevNull.h"\
	".\../../../include\pgpEnv.h"\
	".\../../../include\pgpErr.h"\
	".\../../../include\pgpESK.h"\
	".\../../../include\pgpFIFO.h"\
	".\../../../include\pgpFile.h"\
	".\../../../include\pgpFileMod.h"\
	".\../../../include\pgpFileRef.h"\
	".\../../../include\pgpHash.h"\
	".\../../../include\pgpLeaks.h"\
	".\../../../include\pgpMem.h"\
	".\../../../include\pgpMemMod.h"\
	".\../../../include\pgpPipeline.h"\
	".\../../../include\pgpPubKey.h"\
	".\../../../include\pgpRndom.h"\
	".\../../../include\pgpRndPool.h"\
	".\../../../include\pgpRngPub.h"\
	".\../../../include\pgpRngRead.h"\
	".\../../../include\pgpSig.h"\
	".\../../../include\pgpTextFilt.h"\
	".\../../../include\pgpTimeDate.h"\
	".\../../../include\pgpTrstPkt.h"\
	".\../../../include\pgpTrust.h"\
	".\../../../include\pgpTypes.h"\
	".\../../../include\pgpUI.h"\
	".\../../../include\pgpUsuals.h"\
	".\../../..\config.h"\
	".\../../include\pgpMemPool.h"\
	".\pgpKDBint.h"\
	".\pgpKeyDB.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

"$(INTDIR)\pgpDearmor.obj" : $(SOURCE) $(DEP_CPP_PGPDE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpFilteredSet.c
DEP_CPP_PGPFIL=\
	".\../../../include\pgpDebug.h"\
	".\../../../include\pgpErr.h"\
	".\../../../include\pgpFile.h"\
	".\../../../include\pgpFileMod.h"\
	".\../../../include\pgpFileRef.h"\
	".\../../../include\pgpHash.h"\
	".\../../../include\pgpLeaks.h"\
	".\../../../include\pgpMem.h"\
	".\../../../include\pgpPubKey.h"\
	".\../../../include\pgpRndom.h"\
	".\../../../include\pgpRndPool.h"\
	".\../../../include\pgpRngPub.h"\
	".\../../../include\pgpTimeDate.h"\
	".\../../../include\pgpTrstPkt.h"\
	".\../../../include\pgpTrust.h"\
	".\../../../include\pgpTypes.h"\
	".\../../../include\pgpUsuals.h"\
	".\../../..\config.h"\
	".\../../include\pgpMemPool.h"\
	".\pgpKDBint.h"\
	".\pgpKeyDB.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

"$(INTDIR)\pgpFilteredSet.obj" : $(SOURCE) $(DEP_CPP_PGPFIL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\pgpPrefs.c
DEP_CPP_PGPPRE=\
	".\../../../include\pgpDebug.h"\
	".\../../../include\pgpEnv.h"\
	".\../../../include\pgpErr.h"\
	".\../../../include\pgpFile.h"\
	".\../../../include\pgpFileMod.h"\
	".\../../../include\pgpFileRef.h"\
	".\../../../include\pgpHash.h"\
	".\../../../include\pgpLeaks.h"\
	".\../../../include\pgpMem.h"\
	".\../../../include\pgpPubKey.h"\
	".\../../../include\pgpRndom.h"\
	".\../../../include\pgpRndPool.h"\
	".\../../../include\pgpRngPub.h"\
	".\../../../include\pgpTimeDate.h"\
	".\../../../include\pgpTrstPkt.h"\
	".\../../../include\pgpTrust.h"\
	".\../../../include\pgpTypes.h"\
	".\../../../include\pgpUsuals.h"\
	".\../../..\config.h"\
	".\../../include\pgpMemPool.h"\
	".\pgpKDBint.h"\
	".\pgpKeyDB.h"\
	".\pgpPrefsInt.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

"$(INTDIR)\pgpPrefs.obj" : $(SOURCE) $(DEP_CPP_PGPPRE) "$(INTDIR)"


# End Source File
# End Target
# End Project
################################################################################
