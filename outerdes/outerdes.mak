# Microsoft Visual C++ generated build script - Do not modify

PROJ = OUTERDES
DEBUG = 0
PROGTYPE = 6
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\CIP\OUTERDES\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = D3DES.C     
FIRSTCPP =             
RC = rc
CFLAGS_D_DEXE = /nologo /G2 /W3 /Zi /AM /Od /D "_DEBUG" /D "_DOS" /FR /Fd"OUTERDES.PDB"
CFLAGS_R_DEXE = /nologo /Gs /W3 /AM /Ox /D "NDEBUG" /D "_DOS" /D "MSDOS" /FR 
LFLAGS_D_DEXE = /NOLOGO /ONERROR:NOEXE /NOI /CO /STACK:5120
LFLAGS_R_DEXE = /NOLOGO /ONERROR:NOEXE /NOI /STACK:5120
LIBS_D_DEXE = oldnames mlibce
LIBS_R_DEXE = oldnames mlibce
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
OBJS_EXT = 
LIBS_EXT = 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_DEXE)
LFLAGS = $(LFLAGS_D_DEXE)
LIBS = $(LIBS_D_DEXE)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_DEXE)
LFLAGS = $(LFLAGS_R_DEXE)
LIBS = $(LIBS_R_DEXE)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = D3DES.SBR \
		DESMAIN.SBR \
		MYGETOPT.SBR \
		USAGE.SBR


D3DES_DEP = c:\cip\outerdes\d3des.h


DESMAIN_DEP = c:\cip\outerdes\d3des.h \
	c:\cip\outerdes\usagepro.h \
	c:\cip\outerdes\getoptpr.h


MYGETOPT_DEP = 

USAGE_DEP = c:\cip\outerdes\usagepro.h


all:	$(PROJ).EXE $(PROJ).BSC

D3DES.OBJ:	D3DES.C $(D3DES_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c D3DES.C

DESMAIN.OBJ:	DESMAIN.C $(DESMAIN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c DESMAIN.C

MYGETOPT.OBJ:	MYGETOPT.C $(MYGETOPT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c MYGETOPT.C

USAGE.OBJ:	USAGE.C $(USAGE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c USAGE.C

$(PROJ).EXE::	D3DES.OBJ DESMAIN.OBJ MYGETOPT.OBJ USAGE.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
D3DES.OBJ +
DESMAIN.OBJ +
MYGETOPT.OBJ +
USAGE.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
d:\msvc\lib\+
d:\msvc\mfc\lib\+
d:\ftpdev22\netmsc6.0\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<