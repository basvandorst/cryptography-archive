# Microsoft Visual C++ generated build script - Do not modify

PROJ = MAINNT
DEBUG = 1
PROGTYPE = 3
CALLER = 
ARGS = 
DLLS = 
ORIGIN = MSVCNT
ORIGIN_VER = 1.00
PROJPATH = C:\CIP\RIPEM\MAIN\ 
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = BEMPARSE.C
FIRSTCPP = 
RC = rc
CFLAGS_D_LIB32 = /nologo /Za /W3 /Z7 /YX /D "_DEBUG" /D "_X86_" /D "_WINDOWS" /FR /ML /Fp"MAINNT.PCH" /I..\rsaref\source
CFLAGS_R_LIB32 = /nologo /Za /W3 /YX /O2 /Op- /D "NDEBUG" /D "_X86_" /D "_WINDOWS" /FR /ML /Fp"MAINNT.PCH" /I..\rsaref\source
LFLAGS_D_LIB32 = /NOLOGO
LFLAGS_R_LIB32 = /NOLOGO
RCFLAGS32 = 
D_RCDEFINES32 = -d_DEBUG
R_RCDEFINES32 = -dNDEBUG
OBJS_EXT = 
LIBS_EXT = 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_LIB32)
LFLAGS = 
LIBS = 
LFLAGS_LIB=$(LFLAGS_D_LIB32)
MAPFILE_OPTION = 
RCDEFINES = $(D_RCDEFINES32)
!else
CFLAGS = $(CFLAGS_R_LIB32)
LFLAGS = 
LIBS = 
MAPFILE_OPTION = 
LFLAGS_LIB=$(LFLAGS_R_LIB32)
RCDEFINES = $(R_RCDEFINES32)
!endif
SBRS = BEMPARSE.SBR \
		BFSTREAM.SBR \
		CERTUTIL.SBR \
		CRACKHED.SBR \
		DERKEY.SBR \
		HEXBIN.SBR \
		KEYDER.SBR \
		KEYMAN.SBR \
		LIST.SBR \
		PUBINFO.SBR \
		RDWRMSG.SBR \
		RIPEMMAI.SBR \
		RIPEMSOC.SBR \
		STRUTIL.SBR


BEMPARSE_DEP =  \
	c:\cip\ripem\rsaref\source\global.h \
	c:\cip\ripem\rsaref\source\rsaref.h \
	c:\cip\ripem\rsaref\source\md2.h \
	c:\cip\ripem\rsaref\source\md5.h \
	c:\cip\ripem\rsaref\source\des.h \
	c:\cip\ripem\main\ripem.h \
	c:\cip\ripem\main\p.h \
	c:\cip\ripem\main\bemparse.h


BFSTREAM_DEP =  \
	c:\cip\ripem\rsaref\source\global.h \
	c:\cip\ripem\rsaref\source\rsaref.h \
	c:\cip\ripem\rsaref\source\md2.h \
	c:\cip\ripem\rsaref\source\md5.h \
	c:\cip\ripem\rsaref\source\des.h \
	c:\cip\ripem\main\ripem.h \
	c:\cip\ripem\main\p.h \
	c:\cip\ripem\main\bfstream.h


CERTUTIL_DEP =  \
	c:\cip\ripem\rsaref\source\global.h \
	c:\cip\ripem\rsaref\source\rsaref.h \
	c:\cip\ripem\rsaref\source\md2.h \
	c:\cip\ripem\rsaref\source\md5.h \
	c:\cip\ripem\rsaref\source\des.h \
	c:\cip\ripem\main\ripem.h \
	c:\cip\ripem\main\p.h \
	c:\cip\ripem\main\bfstream.h \
	c:\cip\ripem\main\certder.h \
	c:\cip\ripem\main\keyderpr.h \
	c:\cip\ripem\main\derkeypr.h \
	c:\cip\ripem\main\keyfield.h \
	c:\cip\ripem\main\pubinfop.h \
	c:\cip\ripem\main\certutil.h \
	c:\cip\ripem\main\keymanpr.h \
	c:\cip\ripem\main\headers.h \
	c:\cip\ripem\main\rdwrmsgp.h


CRACKHED_DEP =  \
	c:\cip\ripem\rsaref\source\global.h \
	c:\cip\ripem\rsaref\source\rsaref.h \
	c:\cip\ripem\rsaref\source\md2.h \
	c:\cip\ripem\rsaref\source\md5.h \
	c:\cip\ripem\rsaref\source\des.h \
	c:\cip\ripem\main\ripem.h \
	c:\cip\ripem\main\p.h \
	c:\cip\ripem\main\headers.h \
	c:\cip\ripem\main\bfstream.h \
	c:\cip\ripem\main\crackhpr.h \
	c:\cip\ripem\main\strutilp.h \
	c:\cip\ripem\main\hexbinpr.h \
	c:\cip\ripem\main\derkeypr.h \
	c:\cip\ripem\main\certder.h \
	c:\cip\ripem\main\certutil.h


DERKEY_DEP =  \
	c:\cip\ripem\rsaref\source\global.h \
	c:\cip\ripem\rsaref\source\rsaref.h \
	c:\cip\ripem\rsaref\source\md2.h \
	c:\cip\ripem\rsaref\source\md5.h \
	c:\cip\ripem\rsaref\source\des.h \
	c:\cip\ripem\main\ripem.h \
	c:\cip\ripem\main\p.h \
	c:\cip\ripem\main\derkeypr.h \
	c:\cip\ripem\main\certder.h


HEXBIN_DEP =  \
	c:\cip\ripem\main\hexbinpr.h


KEYDER_DEP =  \
	c:\cip\ripem\rsaref\source\global.h \
	c:\cip\ripem\rsaref\source\rsaref.h \
	c:\cip\ripem\rsaref\source\md2.h \
	c:\cip\ripem\rsaref\source\md5.h \
	c:\cip\ripem\rsaref\source\des.h \
	c:\cip\ripem\main\ripem.h \
	c:\cip\ripem\main\p.h \
	c:\cip\ripem\main\keyderpr.h \
	c:\cip\ripem\main\certder.h \
	c:\cip\ripem\main\derkeypr.h


KEYMAN_DEP =  \
	c:\cip\ripem\rsaref\source\global.h \
	c:\cip\ripem\rsaref\source\rsaref.h \
	c:\cip\ripem\rsaref\source\md2.h \
	c:\cip\ripem\rsaref\source\md5.h \
	c:\cip\ripem\rsaref\source\des.h \
	c:\cip\ripem\main\ripem.h \
	c:\cip\ripem\main\p.h \
	c:\cip\ripem\main\keyfield.h \
	c:\cip\ripem\main\keymanpr.h \
	c:\cip\ripem\main\strutilp.h \
	c:\cip\ripem\main\derkeypr.h \
	c:\cip\ripem\main\hexbinpr.h \
	c:\cip\ripem\main\ripemsop.h \
	c:\cip\ripem\main\pubinfop.h \
	c:\cip\ripem\main\keyderpr.h \
	c:\cip\ripem\main\bfstream.h \
	c:\cip\ripem\main\rdwrmsgp.h \
	c:\cip\ripem\main\certder.h \
	c:\cip\ripem\main\certutil.h \
	c:\cip\ripem\main\bemparse.h


LIST_DEP =  \
	c:\cip\ripem\rsaref\source\global.h \
	c:\cip\ripem\rsaref\source\rsaref.h \
	c:\cip\ripem\rsaref\source\md2.h \
	c:\cip\ripem\rsaref\source\md5.h \
	c:\cip\ripem\rsaref\source\des.h \
	c:\cip\ripem\main\ripem.h \
	c:\cip\ripem\main\p.h \
	c:\cip\ripem\main\strutilp.h


PUBINFO_DEP =  \
	c:\cip\ripem\rsaref\source\global.h \
	c:\cip\ripem\rsaref\source\rsaref.h \
	c:\cip\ripem\rsaref\source\md2.h \
	c:\cip\ripem\rsaref\source\md5.h \
	c:\cip\ripem\rsaref\source\des.h \
	c:\cip\ripem\main\ripem.h \
	c:\cip\ripem\main\p.h \
	c:\cip\ripem\main\headers.h \
	c:\cip\ripem\main\keyfield.h \
	c:\cip\ripem\main\pubinfop.h \
	c:\cip\ripem\main\protserv.h \
	c:\cip\ripem\main\strutilp.h


RDWRMSG_DEP =  \
	c:\cip\ripem\rsaref\source\global.h \
	c:\cip\ripem\rsaref\source\rsaref.h \
	c:\cip\ripem\rsaref\source\md2.h \
	c:\cip\ripem\rsaref\source\md5.h \
	c:\cip\ripem\rsaref\source\des.h \
	c:\cip\ripem\main\ripem.h \
	c:\cip\ripem\main\p.h \
	c:\cip\ripem\main\headers.h \
	c:\cip\ripem\main\bfstream.h \
	c:\cip\ripem\main\rdwrmsgp.h \
	c:\cip\ripem\main\strutilp.h


RIPEMMAI_DEP =  \
	c:\cip\ripem\rsaref\source\global.h \
	c:\cip\ripem\rsaref\source\rsaref.h \
	c:\cip\ripem\rsaref\source\md2.h \
	c:\cip\ripem\rsaref\source\md5.h \
	c:\cip\ripem\rsaref\source\des.h \
	c:\cip\ripem\main\ripem.h \
	c:\cip\ripem\main\p.h \
	c:\cip\ripem\main\headers.h \
	c:\cip\ripem\main\keyfield.h \
	c:\cip\ripem\main\strutilp.h \
	c:\cip\ripem\main\keyderpr.h \
	c:\cip\ripem\main\derkeypr.h \
	c:\cip\ripem\main\keymanpr.h \
	c:\cip\ripem\rsaref\source\r_random.h \
	c:\cip\ripem\main\bemparse.h \
	c:\cip\ripem\main\hexbinpr.h \
	c:\cip\ripem\main\bfstream.h \
	c:\cip\ripem\main\crackhpr.h \
	c:\cip\ripem\main\rdwrmsgp.h \
	c:\cip\ripem\main\certder.h \
	c:\cip\ripem\main\certutil.h \
	c:\cip\ripem\main\version.h


RIPEMSOC_DEP =  \
	c:\cip\ripem\rsaref\source\global.h \
	c:\cip\ripem\rsaref\source\rsaref.h \
	c:\cip\ripem\rsaref\source\md2.h \
	c:\cip\ripem\rsaref\source\md5.h \
	c:\cip\ripem\rsaref\source\des.h \
	c:\cip\ripem\main\ripem.h \
	c:\cip\ripem\main\p.h \
	c:\cip\ripem\main\ripemsop.h \
	c:\cip\ripem\main\keyfield.h \
	c:\cip\ripem\main\protserv.h \
	c:\cip\ripem\main\strutilp.h \
	c:\cip\ripem\main\pubinfop.h


STRUTIL_DEP =  \
	c:\cip\ripem\rsaref\source\global.h \
	c:\cip\ripem\rsaref\source\rsaref.h \
	c:\cip\ripem\rsaref\source\md2.h \
	c:\cip\ripem\rsaref\source\md5.h \
	c:\cip\ripem\rsaref\source\des.h \
	c:\cip\ripem\main\ripem.h \
	c:\cip\ripem\main\p.h \
	c:\cip\ripem\main\strutilp.h \
	c:\cip\ripem\main\hexbinpr.h


all:	$(PROJ).LIB $(PROJ).BSC

BEMPARSE.OBJ:	BEMPARSE.C $(BEMPARSE_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c BEMPARSE.C

BFSTREAM.OBJ:	BFSTREAM.C $(BFSTREAM_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c BFSTREAM.C

CERTUTIL.OBJ:	CERTUTIL.C $(CERTUTIL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c CERTUTIL.C

CRACKHED.OBJ:	CRACKHED.C $(CRACKHED_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c CRACKHED.C

DERKEY.OBJ:	DERKEY.C $(DERKEY_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c DERKEY.C

HEXBIN.OBJ:	HEXBIN.C $(HEXBIN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c HEXBIN.C

KEYDER.OBJ:	KEYDER.C $(KEYDER_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c KEYDER.C

KEYMAN.OBJ:	KEYMAN.C $(KEYMAN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c KEYMAN.C

LIST.OBJ:	LIST.C $(LIST_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c LIST.C

PUBINFO.OBJ:	PUBINFO.C $(PUBINFO_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c PUBINFO.C

RDWRMSG.OBJ:	RDWRMSG.C $(RDWRMSG_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c RDWRMSG.C

RIPEMMAI.OBJ:	RIPEMMAI.C $(RIPEMMAI_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c RIPEMMAI.C

RIPEMSOC.OBJ:	RIPEMSOC.C $(RIPEMSOC_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c RIPEMSOC.C

STRUTIL.OBJ:	STRUTIL.C $(STRUTIL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c STRUTIL.C

$(PROJ).LIB:	BEMPARSE.OBJ BFSTREAM.OBJ CERTUTIL.OBJ CRACKHED.OBJ DERKEY.OBJ HEXBIN.OBJ \
	KEYDER.OBJ KEYMAN.OBJ LIST.OBJ PUBINFO.OBJ RDWRMSG.OBJ RIPEMMAI.OBJ RIPEMSOC.OBJ \
	STRUTIL.OBJ $(OBJS_EXT) $(LIBS_EXT)
	echo >NUL @<<$(PROJ).CRF
BEMPARSE.OBJ 
BFSTREAM.OBJ 
CERTUTIL.OBJ 
CRACKHED.OBJ 
DERKEY.OBJ 
HEXBIN.OBJ 
KEYDER.OBJ 
KEYMAN.OBJ 
LIST.OBJ 
PUBINFO.OBJ 
RDWRMSG.OBJ 
RIPEMMAI.OBJ 
RIPEMSOC.OBJ 
STRUTIL.OBJ


<<
	if exist $@ del $@
	link -LIB @$(PROJ).CRF
	ren bemparse.lib $@

$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
