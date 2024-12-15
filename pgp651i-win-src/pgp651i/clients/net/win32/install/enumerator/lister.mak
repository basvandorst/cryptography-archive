# Microsoft Developer Studio Generated NMAKE File, Based on lister.dsp
!IF "$(CFG)" == ""
CFG=lister - Win32 Debug
!MESSAGE No configuration specified. Defaulting to lister - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "lister - Win32 Release" && "$(CFG)" != "lister - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "lister.mak" CFG="lister - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "lister - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "lister - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "lister - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\SetAdapter.exe"

!ELSE 

ALL : "$(OUTDIR)\SetAdapter.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\config95.obj"
	-@erase "$(INTDIR)\CopyKey.obj"
	-@erase "$(INTDIR)\lister.obj"
	-@erase "$(INTDIR)\Lister.res"
	-@erase "$(INTDIR)\regsearch.obj"
	-@erase "$(INTDIR)\remove95.obj"
	-@erase "$(INTDIR)\removeNT.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\SetAdapter.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /I ".\include" /D "WIN32" /D "NDEBUG" /D\
 "_WINDOWS" /Fp"$(INTDIR)\lister.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD\
 /c 
CPP_OBJS=.\Release/
CPP_SBRS=.
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o NUL /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Lister.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\lister.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib comctl32.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)\SetAdapter.pdb" /machine:I386 /out:"$(OUTDIR)\SetAdapter.exe" 
LINK32_OBJS= \
	"$(INTDIR)\config95.obj" \
	"$(INTDIR)\CopyKey.obj" \
	"$(INTDIR)\lister.obj" \
	"$(INTDIR)\Lister.res" \
	"$(INTDIR)\regsearch.obj" \
	"$(INTDIR)\remove95.obj" \
	"$(INTDIR)\removeNT.obj"

"$(OUTDIR)\SetAdapter.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "lister - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\SetAdapter.exe" "$(OUTDIR)\lister.bsc"

!ELSE 

ALL : "$(OUTDIR)\SetAdapter.exe" "$(OUTDIR)\lister.bsc"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\config95.obj"
	-@erase "$(INTDIR)\config95.sbr"
	-@erase "$(INTDIR)\CopyKey.obj"
	-@erase "$(INTDIR)\CopyKey.sbr"
	-@erase "$(INTDIR)\lister.obj"
	-@erase "$(INTDIR)\Lister.res"
	-@erase "$(INTDIR)\lister.sbr"
	-@erase "$(INTDIR)\regsearch.obj"
	-@erase "$(INTDIR)\regsearch.sbr"
	-@erase "$(INTDIR)\remove95.obj"
	-@erase "$(INTDIR)\remove95.sbr"
	-@erase "$(INTDIR)\removeNT.obj"
	-@erase "$(INTDIR)\removeNT.sbr"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\lister.bsc"
	-@erase "$(OUTDIR)\SetAdapter.exe"
	-@erase "$(OUTDIR)\SetAdapter.ilk"
	-@erase "$(OUTDIR)\SetAdapter.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I ".\include" /D "WIN32" /D "_DEBUG"\
 /D "_WINDOWS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\lister.pch" /YX /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o NUL /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Lister.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\lister.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\config95.sbr" \
	"$(INTDIR)\CopyKey.sbr" \
	"$(INTDIR)\lister.sbr" \
	"$(INTDIR)\regsearch.sbr" \
	"$(INTDIR)\remove95.sbr" \
	"$(INTDIR)\removeNT.sbr"

"$(OUTDIR)\lister.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib comctl32.lib /nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)\SetAdapter.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)\SetAdapter.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\config95.obj" \
	"$(INTDIR)\CopyKey.obj" \
	"$(INTDIR)\lister.obj" \
	"$(INTDIR)\Lister.res" \
	"$(INTDIR)\regsearch.obj" \
	"$(INTDIR)\remove95.obj" \
	"$(INTDIR)\removeNT.obj"

"$(OUTDIR)\SetAdapter.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(CFG)" == "lister - Win32 Release" || "$(CFG)" == "lister - Win32 Debug"
SOURCE=.\config95.c
DEP_CPP_CONFI=\
	".\Define.h"\
	".\Lister.h"\
	

!IF  "$(CFG)" == "lister - Win32 Release"


"$(INTDIR)\config95.obj" : $(SOURCE) $(DEP_CPP_CONFI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "lister - Win32 Debug"


"$(INTDIR)\config95.obj"	"$(INTDIR)\config95.sbr" : $(SOURCE) $(DEP_CPP_CONFI)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\CopyKey.c

!IF  "$(CFG)" == "lister - Win32 Release"


"$(INTDIR)\CopyKey.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "lister - Win32 Debug"


"$(INTDIR)\CopyKey.obj"	"$(INTDIR)\CopyKey.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\lister.c

!IF  "$(CFG)" == "lister - Win32 Release"

DEP_CPP_LISTE=\
	".\Define.h"\
	".\globals.h"\
	".\Lister.h"\
	

"$(INTDIR)\lister.obj" : $(SOURCE) $(DEP_CPP_LISTE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "lister - Win32 Debug"

DEP_CPP_LISTE=\
	".\Define.h"\
	".\globals.h"\
	".\Lister.h"\
	

"$(INTDIR)\lister.obj"	"$(INTDIR)\lister.sbr" : $(SOURCE) $(DEP_CPP_LISTE)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\Lister.rc
DEP_RSC_LISTER=\
	".\res\images24.bmp"\
	".\res\images4.bmp"\
	".\res\Info.ico"\
	".\res\Lister.ico"\
	".\Res\Lister2.ico"\
	".\res\qmark.ico"\
	".\res\qmark2.ico"\
	

"$(INTDIR)\Lister.res" : $(SOURCE) $(DEP_RSC_LISTER) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\regsearch.c
DEP_CPP_REGSE=\
	".\Lister.h"\
	

!IF  "$(CFG)" == "lister - Win32 Release"


"$(INTDIR)\regsearch.obj" : $(SOURCE) $(DEP_CPP_REGSE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "lister - Win32 Debug"


"$(INTDIR)\regsearch.obj"	"$(INTDIR)\regsearch.sbr" : $(SOURCE)\
 $(DEP_CPP_REGSE) "$(INTDIR)"


!ENDIF 

SOURCE=.\remove95.c
DEP_CPP_REMOV=\
	".\Define.h"\
	".\globals.h"\
	".\Lister.h"\
	

!IF  "$(CFG)" == "lister - Win32 Release"


"$(INTDIR)\remove95.obj" : $(SOURCE) $(DEP_CPP_REMOV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "lister - Win32 Debug"


"$(INTDIR)\remove95.obj"	"$(INTDIR)\remove95.sbr" : $(SOURCE) $(DEP_CPP_REMOV)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\removeNT.c
DEP_CPP_REMOVE=\
	".\Define.h"\
	".\globals.h"\
	".\Lister.h"\
	

!IF  "$(CFG)" == "lister - Win32 Release"


"$(INTDIR)\removeNT.obj" : $(SOURCE) $(DEP_CPP_REMOVE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "lister - Win32 Debug"


"$(INTDIR)\removeNT.obj"	"$(INTDIR)\removeNT.sbr" : $(SOURCE) $(DEP_CPP_REMOVE)\
 "$(INTDIR)"


!ENDIF 


!ENDIF 

