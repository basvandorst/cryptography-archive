! DESCRIP.MMS - MMS file for PGP/VMS
! (c) Copyright 1991-92 by Hugh Kennedy. All rights reserved.
!
! The author assumes no liability for damages resulting from the use
! of this software, even if the damage results from defects in this
! software.  No warranty is expressed or implied.
!
! The above notwithstanding, a license to use this software is granted
! to anyone either in the original form or modified on condition that 
! this notice is not removed.
!
! Options Flags:
!
! PGP_DEBUG - Define if you want the debug version
! PGP_GCC - Define to use GNU C instead of VAX C
!
! Modified:	03	Date:	21-Nov-1991	Author: Hugh A.J. Kennedy.
!
! Adapted to use new modules from release 1.3
!
! Modified:	04	Date:	11-Mar-1991	Author:	Hugh A.J. Kennedy
!
! Add test harness program, RSATST as a target.
!
! Modified:	07	Date:	8-Apr-1992	Author:	Hugh A.J. Kennedy
!
! Adapted for new modules from release 1.7.
! Change method of support for GCC (use one options file)
!
! Modified:	08	Date:	25-Jun-1992	Author:	Hugh A.J. Kennedy.
!
! All change (again) for 1.8. Alphabetise module list for ease of comparison.
!
! Modified:	09	Date:	18-Sep-1992	Author: Hugh A.J. Kennedy
!
! Remove references to private string routine versions - no longer reqd.
!
! Modified:	10	Date:	7-Nov-1992	Author:	Hugh A.J. Kennedy
!
! Misc fixes for V2.01
!

.ifdef PGP_COMPAT

VFLAGS = ,COMPATIBLE
MD = MD4

.else

MD = MD5

.endif

.ifdef WFLAGS
XFLAGS = /define=($(WFLAGS))
ZFLAGS = /define=($(WFLAGS),EXPORT,NO_ASM,NOSTORE)
.else
ZFLAGS = /define=(EXPORT,NO_ASM,NOSTORE)
.endif

.ifdef PGP_GCC		! Use GNU CC Compiler

CC = GCC
CCLIB = GNU_CC:[000000]GCCLIB/lib,

.endif
!
! Debugging Support
!
.ifdef PGP_DEBUG

MFLAGS = $(MFLAGS) /debug
LINKFLAGS = $(LINKFLAGS) /debug

.ifdef PGP_GCC		! Are we debugging AND using GCC?

DFLAGS = /DEBUG

.else			! No, Debugging with VAX C
                      
DFLAGS = /debug/noopt
     
.endif

YFLAGS = $(XFLAGS)

.else			! Not debugging

.ifdef PGP_GCC		! Use GCC w/o debug

YFLAGS = $(XFLAGS)

.else			! Use VAX C w/o debug

DFLAGS = /opt=noinline
YFLAGS = $(XFLAGS)

.endif
.endif

CFLAGS = $(DFLAGS)$(YFLAGS)

default : pgp.exe
	@	! do nothing...
!
! ZIP Stuff
!
ZIPOBJS = zbits.obj zdeflate.obj zglobals.obj zinflate.obj zip.obj -
	zipup.obj zfile_io.obj ztrees.obj zunzip.obj
ZIPH=	zrevisio.h ztailor.h zunzip.h zip.h ziperr.h      
zbits.obj : zbits.c $(ZIPH)
	$(CC) $(DFLAGS) $(ZFLAGS) $(mms$source)
zdeflate.obj : zdeflate.c $(ZIPH)
	$(CC) $(DFLAGS) $(ZFLAGS) $(mms$source)
zfile_io.obj : zfile_io.c $(ZIPH)
	$(CC) $(DFLAGS) $(ZFLAGS) $(mms$source)
zglobals.obj : zglobals.c $(ZIPH)
	$(CC) $(DFLAGS) $(ZFLAGS) $(mms$source)
zinflate.obj : zinflate.c $(ZIPH)
	$(CC) $(DFLAGS) $(ZFLAGS) $(mms$source)
zip.obj : zip.c $(ZIPH)
	$(CC) $(DFLAGS) $(ZFLAGS) $(mms$source)
zipup.obj : zipup.c $(ZIPH)
	$(CC) $(DFLAGS) $(ZFLAGS) $(mms$source)
ztrees.obj : ztrees.c $(ZIPH)
	$(CC) $(DFLAGS) $(ZFLAGS) $(mms$source)
zunzip.obj : zunzip.c $(ZIPH)
	$(CC) $(DFLAGS) $(ZFLAGS) $(mms$source)
!
! PGP Stuff
!
armor.obj : armor.c
config.obj : config.c usuals.h pgp.h
CRYPTO.obj : mpilib.h mpiio.h random.h crypto.h -
keymgmt.h mdfile.h $(MD).h fileio.h pgp.h CRYPTO.C
idea.obj : idea.h pgp.h idea.c
FILEIO.obj : FILEIO.C random.h mpilib.h mpiio.h fileio.h pgp.h 
getopt.obj : getopt.c
genprime.obj : genprime.c genprime.h mpilib.h random.h
keyadd.obj :	mpilib.h random.h crypto.h fileio.h -
	keymgmt.h genprime.h rsagen.h mpiio.h pgp.h language.h keyadd.c
keymaint.obj :	mpilib.h random.h crypto.h fileio.h -
	keymgmt.h genprime.h mpiio.h pgp.h language.h keymaint.c
KEYMGMT.obj : mpilib.h usuals.h random.h crypto.h -
	fileio.h mpiio.h pgp.h KEYMGMT.C
$(MD).obj : $(MD).h $(MD).C                          
MDFILE.obj : mpilib.h mdfile.h $(MD).h pgp.h MDFILE.C
MORE.obj : MORE.C mpilib.h pgp.h 
MPILIB.obj : MPILIB.C mpilib.h 
passwd.obj : passwd.c random.h $(MD).h pgp.h 
PGP.obj : mpilib.h random.h crypto.h fileio.h keymgmt.h pgp.h PGP.C
RANDOM.obj : random.h pgp.h RANDOM.C
rsagen.obj : rsagen.c mpilib.h genprime.h rsagen.h random.h
rsatst.obj : rsatst.c mpilib.h mpiio.h genprime.h rsagen.h random.h
SYSTEM.obj : SYSTEM.C
vax.obj : vax.mar
!	$(CC) $(DFLAGS) /define=("index=strchr",VMS)/opt=noinline system
! LZH.obj : mpilib.h mpiio.h LZH.C
rsatst.exe : rsatst.opt rsatst.obj mpilib.obj genprime.obj rsagen.obj -
mpiio.obj random.obj vax.obj system.obj language.obj fileio.obj
	$(LINK) $(LINKFLAGS) rsatst/opt
OBJ1 =	pgp.obj config.obj crypto.obj keymgmt.obj keyadd.obj, keymaint.obj, fileio.obj -
	mdfile.obj more.obj armor.obj mpilib.obj mpiio.obj -
	getopt.obj genprime.obj rsagen.obj random.obj idea.obj passwd.obj -
	$(MD).obj system.obj language.obj vax.obj charset.obj

pgp.exe : pgp.opt $(OBJ1) $(ZIPOBJS)
	assign/user $(MD).obj MD
	$(LINK) $(LINKFLAGS) pgp/opt, $(CCLIB) 'f$environment("default")'VAXCRTL/opt
