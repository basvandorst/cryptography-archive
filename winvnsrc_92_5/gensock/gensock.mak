# Microsoft Visual C++ generated build script - Do not modify

PROJ = GENSOCK
DEBUG = 1
PROGTYPE = 1
CALLER = c:\winvn\winvngen.exe c:\winvn\niven\winvn.ini c:\winvn\niven\newsrc
ARGS = 
DLLS = 
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\WINVN\GENSOCK\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC =             
FIRSTCPP = GENSOCK.CPP 
RC = rc
CFLAGS_D_WDLL = /nologo /G2 /W3 /Zi /ALw /Gx- /Od /D "_DEBUG" /D "BROKEN_IOCTLSOCKET" /FR /GD /Fd"GENSOCK.PDB"
CFLAGS_R_WDLL = /nologo /G2 /W3 /ALw /Gx- /O2 /D "NDEBUG" /D "BROKEN_IOCTLSOCKET" /FR /GD 
LFLAGS_D_WDLL = /NOLOGO /NOD /NOE /PACKC:61440 /ALIGN:16 /ONERROR:NOEXE /CO /MAP:FULL
LFLAGS_R_WDLL = /NOLOGO /NOD /NOE /PACKC:61440 /ALIGN:16 /ONERROR:NOEXE /MAP:FULL
LIBS_D_WDLL = oldnames libw ldllcew commdlg.lib shell.lib 
LIBS_R_WDLL = oldnames libw ldllcew commdlg.lib shell.lib 
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
DEFFILE = GENSOCK.DEF
OBJS_EXT = 
LIBS_EXT = 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_WDLL)
LFLAGS = $(LFLAGS_D_WDLL)
LIBS = $(LIBS_D_WDLL)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_WDLL)
LFLAGS = $(LFLAGS_R_WDLL)
LIBS = $(LIBS_R_WDLL)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = GENSOCK.SBR


GENSOCK_DEP = c:\msvc\include\winsock.h \
	c:\winvn\gensock\gensock.h


GENSOCK_RCDEP = c:\winvn\gensock\gensock.ico


all:	$(PROJ).DLL $(PROJ).BSC

GENSOCK.OBJ:	GENSOCK.CPP $(GENSOCK_DEP)
	$(CPP) $(CFLAGS) $(CPPCREATEPCHFLAG) /c GENSOCK.CPP

GENSOCK.RES:	GENSOCK.RC $(GENSOCK_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r GENSOCK.RC


$(PROJ).DLL::	GENSOCK.RES

$(PROJ).DLL::	GENSOCK.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
GENSOCK.OBJ +
$(OBJS_EXT)
$(PROJ).DLL
$(MAPFILE)
c:\msvc\lib\+
c:\msvc\mfc\lib\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) GENSOCK.RES $@
	@copy $(PROJ).CRF MSVC.BND
	implib /nowep $(PROJ).LIB $(PROJ).DLL

$(PROJ).DLL::	GENSOCK.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) GENSOCK.RES $@

run: $(PROJ).DLL
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
