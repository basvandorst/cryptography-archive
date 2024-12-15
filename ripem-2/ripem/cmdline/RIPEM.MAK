# Microsoft Visual C++ generated build script - Do not modify

PROJ = RIPEM
DEBUG = 0
PROGTYPE = 6
CALLER = 
ARGS = -d -p test1.pub -s test1.prv -k test1pw -i ../test/message.enc -y rpub.cl.msu.edu -u test
DLLS = 
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = D:\CIP\RIPEM\CMDLINE\
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
CFLAGS_D_DEXE = /nologo /W3 /Zi /AL /Od /D "_DEBUG" /D "_DOS" /D "MSDOS" /I "..\rsaref\source" /I "..\main" /FR /Fd"RIPEM.PDB"
CFLAGS_R_DEXE = /nologo /Gs /Za /W3 /AL /Op- /Ox /D "NDEBUG" /D "_DOS" /I "..\rsaref\source" /I "..\main" /FR 
LFLAGS_D_DEXE = /NOLOGO /NOI /STACK:26000 /ONERROR:NOEXE /CO /MAP /LINE 
LFLAGS_R_DEXE = /NOLOGO /NOI /STACK:26000 /ONERROR:NOEXE 
LIBS_D_DEXE = oldnames llibce 
LIBS_R_DEXE = oldnames llibce 
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
OBJS_EXT = 
LIBS_EXT = ..\MAIN\RIPEM.LIB ..\RSAREF\INSTALL\RSAREF.LIB 
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
SBRS = GETOPT.SBR \
		GETSYS.SBR \
		PARSIT.SBR \
		RCERTSMS.SBR \
		RIPEMCMD.SBR \
		USAGE.SBR \
		USAGEMSG.SBR


GETOPT_DEP = ..\cmdline\getoptpr.h


GETSYS_DEP = ..\rsaref\source\global.h \
	..\rsaref\source\rsaref.h \
	..\rsaref\source\md2.h \
	..\rsaref\source\md5.h \
	..\rsaref\source\des.h \
	..\cmdline\getsyspr.h


PARSIT_DEP = ..\cmdline\parsitpr.h


RCERTSMS_DEP = 

RIPEMCMD_DEP = ..\rsaref\source\global.h \
	..\rsaref\source\rsaref.h \
	..\rsaref\source\md2.h \
	..\rsaref\source\md5.h \
	..\rsaref\source\des.h \
	..\rsaref\source\r_random.h \
	..\cmdline\usagepro.h \
	..\cmdline\getsyspr.h \
	..\cmdline\parsitpr.h \
	..\cmdline\getoptpr.h


USAGE_DEP = ..\cmdline\usagepro.h


USAGEMSG_DEP = 

RIPEM_DEP = 

RSAREF_DEP = 

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

$(PROJ).EXE::	GETOPT.OBJ GETSYS.OBJ PARSIT.OBJ RCERTSMS.OBJ RIPEMCMD.OBJ USAGE.OBJ \
	USAGEMSG.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
GETOPT.OBJ +
GETSYS.OBJ +
PARSIT.OBJ +
RCERTSMS.OBJ +
RIPEMCMD.OBJ +
USAGE.OBJ +
USAGEMSG.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
c:\msvc\lib\+
c:\msvc\mfc\lib\+
..\MAIN\RIPEM.LIB+
..\RSAREF\INSTALL\RSAREF.LIB+
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
