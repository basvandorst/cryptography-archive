# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=bn - Win32 Release
!MESSAGE No configuration specified.  Defaulting to bn - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "bn - Win32 Release" && "$(CFG)" != "bn - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "bn.mak" CFG="bn - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bn - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "bn - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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
# PROP Target_Last_Scanned "bn - Win32 Debug"
RSC=rc.exe
MTL=mktyplib.exe
CPP=cl.exe

!IF  "$(CFG)" == "bn - Win32 Release"

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

ALL : "$(OUTDIR)\bn.dll"

CLEAN : 
	-@erase "$(INTDIR)\bn.obj"
	-@erase "$(INTDIR)\bn32.obj"
	-@erase "$(INTDIR)\bngermain.obj"
	-@erase "$(INTDIR)\bni32.obj"
	-@erase "$(INTDIR)\bni80386c.obj"
	-@erase "$(INTDIR)\bnimem.obj"
	-@erase "$(INTDIR)\bninit32.obj"
	-@erase "$(INTDIR)\bnjacobi.obj"
	-@erase "$(INTDIR)\bnlegal.obj"
	-@erase "$(INTDIR)\bnprime.obj"
	-@erase "$(INTDIR)\bnprint.obj"
	-@erase "$(INTDIR)\bnsieve.obj"
	-@erase "$(OUTDIR)\bn.dll"
	-@erase "$(OUTDIR)\bn.exp"
	-@erase "$(OUTDIR)\bn.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../.." /I "../../include" /D "NDEBUG" /D "BNLIB" /D "WIN32" /D "_WINDOWS" /D HAVE_CONFIG_H=1 /D BNINCLUDE=bni80386c.h /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "../.." /I "../../include" /D "NDEBUG" /D\
 "BNLIB" /D "WIN32" /D "_WINDOWS" /D HAVE_CONFIG_H=1 /D BNINCLUDE=bni80386c.h\
 /Fp"$(INTDIR)/bn.pch" /YX /Fo"$(INTDIR)/" /c 
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
BSC32_FLAGS=/nologo /o"$(OUTDIR)/bn.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/bn.pdb" /machine:I386 /out:"$(OUTDIR)/bn.dll"\
 /implib:"$(OUTDIR)/bn.lib" 
LINK32_OBJS= \
	"$(INTDIR)\bn.obj" \
	"$(INTDIR)\bn32.obj" \
	"$(INTDIR)\bngermain.obj" \
	"$(INTDIR)\bni32.obj" \
	"$(INTDIR)\bni80386c.obj" \
	"$(INTDIR)\bnimem.obj" \
	"$(INTDIR)\bninit32.obj" \
	"$(INTDIR)\bnjacobi.obj" \
	"$(INTDIR)\bnlegal.obj" \
	"$(INTDIR)\bnprime.obj" \
	"$(INTDIR)\bnprint.obj" \
	"$(INTDIR)\bnsieve.obj"

"$(OUTDIR)\bn.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "bn - Win32 Debug"

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

ALL : "$(OUTDIR)\bn.dll"

CLEAN : 
	-@erase "$(INTDIR)\bn.obj"
	-@erase "$(INTDIR)\bn32.obj"
	-@erase "$(INTDIR)\bngermain.obj"
	-@erase "$(INTDIR)\bni32.obj"
	-@erase "$(INTDIR)\bni80386c.obj"
	-@erase "$(INTDIR)\bnimem.obj"
	-@erase "$(INTDIR)\bninit32.obj"
	-@erase "$(INTDIR)\bnjacobi.obj"
	-@erase "$(INTDIR)\bnlegal.obj"
	-@erase "$(INTDIR)\bnprime.obj"
	-@erase "$(INTDIR)\bnprint.obj"
	-@erase "$(INTDIR)\bnsieve.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\bn.dll"
	-@erase "$(OUTDIR)\bn.exp"
	-@erase "$(OUTDIR)\bn.ilk"
	-@erase "$(OUTDIR)\bn.lib"
	-@erase "$(OUTDIR)\bn.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "../.." /I "../../include" /D "_DEBUG" /D "BNLIB" /D "WIN32" /D "_WINDOWS" /D HAVE_CONFIG_H=1 /D BNINCLUDE=bni80386c.h /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I "../.." /I "../../include" /D\
 "_DEBUG" /D "BNLIB" /D "WIN32" /D "_WINDOWS" /D HAVE_CONFIG_H=1 /D\
 BNINCLUDE=bni80386c.h /Fp"$(INTDIR)/bn.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/"\
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
BSC32_FLAGS=/nologo /o"$(OUTDIR)/bn.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/bn.pdb" /debug /machine:I386 /out:"$(OUTDIR)/bn.dll"\
 /implib:"$(OUTDIR)/bn.lib" 
LINK32_OBJS= \
	"$(INTDIR)\bn.obj" \
	"$(INTDIR)\bn32.obj" \
	"$(INTDIR)\bngermain.obj" \
	"$(INTDIR)\bni32.obj" \
	"$(INTDIR)\bni80386c.obj" \
	"$(INTDIR)\bnimem.obj" \
	"$(INTDIR)\bninit32.obj" \
	"$(INTDIR)\bnjacobi.obj" \
	"$(INTDIR)\bnlegal.obj" \
	"$(INTDIR)\bnprime.obj" \
	"$(INTDIR)\bnprint.obj" \
	"$(INTDIR)\bnsieve.obj"

"$(OUTDIR)\bn.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "bn - Win32 Release"
# Name "bn - Win32 Debug"

!IF  "$(CFG)" == "bn - Win32 Release"

!ELSEIF  "$(CFG)" == "bn - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\bnsieve.c
DEP_CPP_BNSIE=\
	"..\..\include\pgpTypes.h"\
	".\../../include\pgpDebug.h"\
	".\../..\config.h"\
	".\bn.h"\
	".\bnimem.h"\
	".\bnkludge.h"\
	".\bnsieve.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\bnsieve.obj" : $(SOURCE) $(DEP_CPP_BNSIE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\bnprint.c
DEP_CPP_BNPRI=\
	".\../..\config.h"\
	".\bn.h"\
	".\bnkludge.h"\
	".\bnprint.h"\
	

"$(INTDIR)\bnprint.obj" : $(SOURCE) $(DEP_CPP_BNPRI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\bnprime.c
DEP_CPP_BNPRIM=\
	"..\..\include\pgpTypes.h"\
	".\../../include\pgpDebug.h"\
	".\../..\config.h"\
	".\bn.h"\
	".\bnimem.h"\
	".\bnkludge.h"\
	".\bnprime.h"\
	".\bnsieve.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\bnprime.obj" : $(SOURCE) $(DEP_CPP_BNPRIM) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\bnlegal.c
DEP_CPP_BNLEG=\
	".\../..\config.h"\
	".\bnlegal.h"\
	

"$(INTDIR)\bnlegal.obj" : $(SOURCE) $(DEP_CPP_BNLEG) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\bnjacobi.c
DEP_CPP_BNJAC=\
	".\bn.h"\
	".\bnjacobi.h"\
	

"$(INTDIR)\bnjacobi.obj" : $(SOURCE) $(DEP_CPP_BNJAC) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\bnimem.c
DEP_CPP_BNIME=\
	".\../..\config.h"\
	".\bni.h"\
	".\bnimem.h"\
	".\bnkludge.h"\
	
NODEP_CPP_BNIME=\
	".\..\dbmalloc\malloc.h"\
	

"$(INTDIR)\bnimem.obj" : $(SOURCE) $(DEP_CPP_BNIME) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\bngermain.c
DEP_CPP_BNGER=\
	"..\..\include\pgpTypes.h"\
	".\../../include\pgpDebug.h"\
	".\../..\config.h"\
	".\bn.h"\
	".\bngermain.h"\
	".\bnimem.h"\
	".\bnjacobi.h"\
	".\bnkludge.h"\
	".\bnsieve.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\bngermain.obj" : $(SOURCE) $(DEP_CPP_BNGER) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\bn.c
DEP_CPP_BN_Ce=\
	".\bn.h"\
	

"$(INTDIR)\bn.obj" : $(SOURCE) $(DEP_CPP_BN_Ce) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\bni80386c.c
DEP_CPP_BNI80=\
	"..\..\include\pgpTypes.h"\
	".\../../include\pgpDebug.h"\
	".\../..\config.h"\
	".\bni.h"\
	".\bni32.h"\
	".\bnimem.h"\
	".\bnkludge.h"\
	".\bnlegal.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\bni80386c.obj" : $(SOURCE) $(DEP_CPP_BNI80) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\bninit32.c
DEP_CPP_BNINI=\
	".\bn.h"\
	".\bn32.h"\
	

"$(INTDIR)\bninit32.obj" : $(SOURCE) $(DEP_CPP_BNINI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\bni32.c
DEP_CPP_BNI32=\
	"..\..\include\pgpTypes.h"\
	".\../../include\pgpDebug.h"\
	".\../..\config.h"\
	".\bni.h"\
	".\bni32.h"\
	".\bnimem.h"\
	".\bnkludge.h"\
	".\bnlegal.h"\
	{$(INCLUDE)}"\sys\types.h"\
	

"$(INTDIR)\bni32.obj" : $(SOURCE) $(DEP_CPP_BNI32) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\bn32.c
DEP_CPP_BN32_=\
	"..\..\include\pgpTypes.h"\
	".\../../include\pgpDebug.h"\
	".\../..\config.h"\
	".\bn.h"\
	".\bn32.h"\
	".\bni.h"\
	".\bni32.h"\
	".\bnimem.h"\
	".\bnkludge.h"\
	{$(INCLUDE)}"\sys\types.h"\
	
NODEP_CPP_BN32_=\
	".\..\dbmalloc\malloc.h"\
	

"$(INTDIR)\bn32.obj" : $(SOURCE) $(DEP_CPP_BN32_) "$(INTDIR)"


# End Source File
# End Target
# End Project
################################################################################
