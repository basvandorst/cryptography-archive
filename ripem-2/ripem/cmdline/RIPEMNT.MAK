# Microsoft Visual C++ generated build script - Do not modify

PROJ = RIPEMNT
DEBUG = 1
PROGTYPE = 2
CALLER = 
ARGS = 
DLLS = 
ORIGIN = MSVCNT
ORIGIN_VER = 1.00
PROJPATH = C:\CIP\RIPEM\CMDLINE\ 
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = GETOPT.C
FIRSTCPP = 
RC = rc
CFLAGS_D_DEXE32 = /nologo /Za /W3 /Zi /YX /D "_X86_" /D "_DEBUG" /D "_CONSOLE" /FR /ML /Fd"RIPEMNT.PDB" /Fp"RIPEMNT.PCH" /I..\rsaref\source /I..\main
CFLAGS_R_DEXE32 = /nologo /W3 /YX /O2 /D "_X86_" /D "NDEBUG" /D "_CONSOLE" /FR /ML /Fp"RIPEMNT.PCH" /I..\rsaref\source /I..\main
LFLAGS_D_DEXE32 = /NOLOGO /DEBUG /DEBUGTYPE:cv /SUBSYSTEM:console netapi32.lib
LFLAGS_R_DEXE32 = /NOLOGO /SUBSYSTEM:console netapi32.lib
LFLAGS_D_LIB32 = /NOLOGO
LFLAGS_R_LIB32 = /NOLOGO
LIBS_D_DEXE32 = 
LIBS_R_DEXE32 = 
RCFLAGS32 = 
D_RCDEFINES32 = -d_DEBUG
R_RCDEFINES32 = -dNDEBUG
OBJS_EXT = 
LIBS_EXT = ..\MAIN\MAINNT.LIB ..\RSAREF\INSTALL\RSAREFNT.LIB 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_DEXE32)
LFLAGS = $(LFLAGS_D_DEXE32)
LIBS = $(LIBS_D_DEXE32)
LFLAGS_LIB=$(LFLAGS_D_LIB32)
MAPFILE_OPTION = 
RCDEFINES = $(D_RCDEFINES32)
!else
CFLAGS = $(CFLAGS_R_DEXE32)
LFLAGS = $(LFLAGS_R_DEXE32)
LIBS = $(LIBS_R_DEXE32)
MAPFILE_OPTION = 
LFLAGS_LIB=$(LFLAGS_R_LIB32)
RCDEFINES = $(R_RCDEFINES32)
!endif
SBRS = GETOPT.SBR \
		GETSYS.SBR \
		PARSIT.SBR \
		RCERTSMS.SBR \
		RIPEMCMD.SBR \
		USAGE.SBR \
		USAGEMSG.SBR


GETOPT_DEP =  \
	c:\cip\ripem\cmdline\getoptpr.h


GETSYS_DEP =  \
	c:\cip\ripem\rsaref\source\global.h \
	c:\cip\ripem\rsaref\source\rsaref.h \
	c:\cip\ripem\rsaref\source\md2.h \
	c:\cip\ripem\rsaref\source\md5.h \
	c:\cip\ripem\rsaref\source\des.h \
	c:\cip\ripem\cmdline\getsyspr.h \
	d:\msvcnt\include\unistd.h 


PARSIT_DEP =  \
	c:\cip\ripem\cmdline\parsitpr.h


RCERTSMS_DEP = 

RIPEMCMD_DEP =  \
	c:\cip\ripem\rsaref\source\global.h \
	c:\cip\ripem\rsaref\source\rsaref.h \
	c:\cip\ripem\rsaref\source\md2.h \
	c:\cip\ripem\rsaref\source\md5.h \
	c:\cip\ripem\rsaref\source\des.h \
	c:\cip\ripem\rsaref\source\r_random.h \
	c:\cip\ripem\cmdline\usagepro.h \
	c:\cip\ripem\cmdline\getsyspr.h \
	c:\cip\ripem\cmdline\parsitpr.h \
	c:\cip\ripem\cmdline\getoptpr.h


USAGE_DEP =  \
	c:\cip\ripem\cmdline\usagepro.h


USAGEMSG_DEP = 

MAINNT_DEP = 

RSAREFNT_DEP = 

all:	$(PROJ).EXE $(PROJ).BSC

GETOPT.OBJ:	GETOPT.C $(GETOPT_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c GETOPT.C

GETSYS.OBJ:	GETSYS.C $(GETSYS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GETSYS.C

PARSIT.OBJ:	PARSIT.C $(PARSIT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c PARSIT.C

RCERTSMS.OBJ:	RCERTSMS.C $(RCERTSMS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c RCERTSMS.C

RIPEMCMD.OBJ:	RIPEMCMD.C $(RIPEMCMD_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c RIPEMCMD.C

USAGE.OBJ:	USAGE.C $(USAGE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c USAGE.C

USAGEMSG.OBJ:	USAGEMSG.C $(USAGEMSG_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c USAGEMSG.C

$(PROJ).EXE:	GETOPT.OBJ GETSYS.OBJ PARSIT.OBJ RCERTSMS.OBJ RIPEMCMD.OBJ USAGE.OBJ \
	USAGEMSG.OBJ $(OBJS_EXT) $(LIBS_EXT)
	echo >NUL @<<$(PROJ).CRF
GETOPT.OBJ 
GETSYS.OBJ 
PARSIT.OBJ 
RCERTSMS.OBJ 
RIPEMCMD.OBJ 
USAGE.OBJ 
USAGEMSG.OBJ 
$(OBJS_EXT)
-OUT:$(PROJ).EXE
$(MAPFILE_OPTION)
..\MAIN\MAINNT.LIB
..\RSAREF\INSTALL\RSAREFNT.LIB
$(LIBS)
$(LIBS_EXT)
$(DEFFILE_OPTION) -implib:$(PROJ).lib
<<
	link $(LFLAGS) @$(PROJ).CRF

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
