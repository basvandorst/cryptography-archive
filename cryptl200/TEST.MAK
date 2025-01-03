# Microsoft Visual C++ generated build script - Do not modify

PROJ = TEST
DEBUG = 1
PROGTYPE = 3
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\WORK\CRYPT\WINDOWS\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = TEST.C      
FIRSTCPP =             
RC = rc
CFLAGS_D_WTTY = /nologo /G2 /Mq /W3 /Zi /AL /Od /D "_DEBUG" /Fd"TEST.PDB"
CFLAGS_R_WTTY = /nologo /Gs /G2 /Mq /W3 /AL /Ox /D "NDEBUG"
LFLAGS_D_WTTY = /NOLOGO /NOD /PACKC:57344 /ALIGN:16 /ONERROR:NOEXE /CO
LFLAGS_R_WTTY = /NOLOGO /NOD /PACKC:57344 /ALIGN:16 /ONERROR:NOEXE
LIBS_D_WTTY = oldnames libw llibcewq
LIBS_R_WTTY = oldnames libw llibcewq
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
DEFFILE = TEST.DEF
OBJS_EXT = 
LIBS_EXT = CRYPT.LIB 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_WTTY)
LFLAGS = $(LFLAGS_D_WTTY)
LIBS = $(LIBS_D_WTTY)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
DEFFILE=C:\WORK\CRYPT\WINDOWS\TEST.DEF
!else
CFLAGS = $(CFLAGS_R_WTTY)
LFLAGS = $(LFLAGS_R_WTTY)
LIBS = $(LIBS_R_WTTY)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
DEFFILE=C:\WORK\CRYPT\WINDOWS\TEST.DEF
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = TESTHL.SBR \
		TESTLIB.SBR


CRYPT_DEP = 

TESTHL_DEP = c:\work\crypt\capi.h


TESTLIB_DEP = c:\work\crypt\capi.h \
	c:\work\crypt\crypt.h \
	c:\work\crypt\bnlib/bn.h


all:	$(PROJ).EXE

TESTHL.OBJ:	..\TESTHL.C $(TESTHL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\TESTHL.C

TESTLIB.OBJ:	..\TESTLIB.C $(TESTLIB_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\TESTLIB.C


$(PROJ).EXE::	TESTHL.OBJ TESTLIB.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
TESTHL.OBJ +
TESTLIB.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
G:\MSVC15\LIB\+
G:\MSVC15\MFC\LIB\+
CRYPT.LIB+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) $@


run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
