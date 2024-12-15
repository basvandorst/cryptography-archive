$! VMSBUILD.COM (V0203A) - Build PGP For VMS W/O MMS
$! (c) Copyright 1991-93 by Hugh Kennedy. All rights reserved.
$!
$! The author assumes no liability for damages resulting from the use 
$! of this software, even if the damage results from defects in this 
$! software.  No warranty is expressed or implied.  
$!
$! The above notwithstanding, a license to use this software is granted
$! to anyone either in the original or modified form on condition that 
$! this notice is not removed.
$!
$! Version:	2
$!                                                                 
$! Generated from DESCRIP.MMS with MMS/NOACT/OUT=VMSBUILD.COM
$!
$! Modification:02	Date:	11-Feb-1993	Author:	Hugh A.J. Kennedy
$!
$! Regenerate from new DESCRIP.MMS with logicals.
$!
$! Modification:03	Date:	17-May-1993	Author:	Hugh A.J. Kennedy
$!
$! Incorporate new modules.
$!----------------------------------------------------------------------
$ if f$trnlnm("src") .eqs. "" then define src 'f$environment("default")'
$ if f$trnlnm("obj") .eqs. "" then define obj 'f$environment("default")'
$ if f$trnlnm("C$INCLUDE") .eqs. "" then define C$INCLUDE SRC
$ CC /NOLIST/OBJECT=OBJ:PGP.OBJ/opt=noinline SRC:PGP.C
$ CC /NOLIST/OBJECT=OBJ:CONFIG.OBJ/opt=noinline SRC:CONFIG.C
$ CC /NOLIST/OBJECT=OBJ:CRYPTO.OBJ/opt=noinline SRC:CRYPTO.C
$ CC /NOLIST/OBJECT=OBJ:KEYMGMT.OBJ/opt=noinline SRC:KEYMGMT.C
$ CC /NOLIST/OBJECT=OBJ:KEYADD.OBJ/opt=noinline SRC:KEYADD.C
$ CC /NOLIST/OBJECT=OBJ:KEYMAINT.OBJ/opt=noinline SRC:KEYMAINT.C
$ CC /NOLIST/OBJECT=OBJ:FILEIO.OBJ/opt=noinline SRC:FILEIO.C
$ CC /NOLIST/OBJECT=OBJ:MDFILE.OBJ/opt=noinline SRC:MDFILE.C
$ CC /NOLIST/OBJECT=OBJ:MORE.OBJ/opt=noinline SRC:MORE.C
$ CC /NOLIST/OBJECT=OBJ:ARMOR.OBJ/opt=noinline SRC:ARMOR.C
$ CC /NOLIST/OBJECT=OBJ:MPILIB.OBJ/opt=noinline SRC:MPILIB.C
$ CC /NOLIST/OBJECT=OBJ:MPIIO.OBJ/opt=noinline SRC:MPIIO.C
$ CC /NOLIST/OBJECT=OBJ:GETOPT.OBJ/opt=noinline SRC:GETOPT.C
$ CC /NOLIST/OBJECT=OBJ:GENPRIME.OBJ/opt=noinline SRC:GENPRIME.C
$ CC /NOLIST/OBJECT=OBJ:RSAGEN.OBJ/opt=noinline SRC:RSAGEN.C
$ CC /NOLIST/OBJECT=OBJ:RANDOM.OBJ/opt=noinline SRC:RANDOM.C
$ CC /NOLIST/OBJECT=OBJ:IDEA.OBJ/opt=noinline SRC:IDEA.C
$ CC /NOLIST/OBJECT=OBJ:PASSWD.OBJ/opt=noinline SRC:PASSWD.C
$ CC /NOLIST/OBJECT=OBJ:MD5.OBJ/opt=noinline SRC:MD5.C
$ CC /NOLIST/OBJECT=OBJ:SYSTEM.OBJ/opt=noinline SRC:SYSTEM.C
$ CC /NOLIST/OBJECT=OBJ:LANGUAGE.OBJ/opt=noinline SRC:LANGUAGE.C
$ MACRO /NOLIST/OBJECT=OBJ:VAX.OBJ SRC:VAX.MAR
$ CC /NOLIST/OBJECT=OBJ:CHARSET.OBJ/opt=noinline SRC:CHARSET.C
$ CC /NOLIST/OBJECT=OBJ:RSAGLUE.OBJ/opt=noinline SRC:RSAGLUE.C
$ CC /opt=noinline /NOLIST/OBJECT=OBJ:ZBITS.OBJ /define=(EXPORT,NO_ASM,NOSTORE) SRC:ZBITS.C
$ CC  /opt=noinline /NOLIST/OBJECT=OBJ:ZDEFLATE.OBJ /define=(EXPORT,NO_ASM,NOSTORE) SRC:ZDEFLATE.C
$ CC  /opt=noinline /NOLIST/OBJECT=OBJ:ZGLOBALS.OBJ /define=(EXPORT,NO_ASM,NOSTORE) SRC:ZGLOBALS.C
$ CC  /opt=noinline /NOLIST/OBJECT=OBJ:ZINFLATE.OBJ /define=(EXPORT,NO_ASM,NOSTORE) SRC:ZINFLATE.C
$ CC  /opt=noinline /NOLIST/OBJECT=OBJ:ZIP.OBJ /define=(EXPORT,NO_ASM,NOSTORE) SRC:ZIP.C
$ CC  /opt=noinline /NOLIST/OBJECT=OBJ:ZIPUP.OBJ /define=(EXPORT,NO_ASM,NOSTORE) SRC:ZIPUP.C
$ CC  /opt=noinline /NOLIST/OBJECT=OBJ:ZFILE_IO.OBJ /define=(EXPORT,NO_ASM,NOSTORE) SRC:ZFILE_IO.C
$ CC  /opt=noinline /NOLIST/OBJECT=OBJ:ZTREES.OBJ /define=(EXPORT,NO_ASM,NOSTORE) SRC:ZTREES.C
$ CC  /opt=noinline /NOLIST/OBJECT=OBJ:ZUNZIP.OBJ /define=(EXPORT,NO_ASM,NOSTORE) SRC:ZUNZIP.C
$ LINK /exe=OBJ:PGP.EXE src:pgp/opt,   src:VAXCRTL/opt
$ deassign C$INCLUDE
$!Last Modified:  19-MAY-1993 07:08:47.39
