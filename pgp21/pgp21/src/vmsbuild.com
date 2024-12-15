$! VMSBUILD.COM (X0201A) - Build PGP For VMS W/O MMS
$! (c) Copyright 1991-92 by Hugh Kennedy. All rights reserved.
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
$!----------------------------------------------------------------------
$ CC /opt=noinline PGP.C
$ CC /opt=noinline CONFIG.C
$ CC /opt=noinline CRYPTO.C
$ CC /opt=noinline KEYMGMT.C
$ CC /opt=noinline KEYADD.C
$ CC /opt=noinline KEYMAINT.C
$ CC /opt=noinline FILEIO.C
$ CC /opt=noinline MDFILE.C
$ CC /opt=noinline MORE.C
$ CC /opt=noinline ARMOR.C
$ CC /opt=noinline MPILIB.C
$ CC /opt=noinline MPIIO.C
$ CC /opt=noinline GETOPT.C
$ CC /opt=noinline GENPRIME.C
$ CC /opt=noinline RSAGEN.C
$ CC /opt=noinline RANDOM.C
$ CC /opt=noinline IDEA.C
$ CC /opt=noinline PASSWD.C
$ CC /opt=noinline MD5.C
$ CC /opt=noinline SYSTEM.C
$ CC /opt=noinline LANGUAGE.C
$ MACRO /NOLIST/OBJECT=VAX.OBJ VAX.MAR
$ CC /opt=noinline CHARSET.C
$ CC /opt=noinline /define=(EXPORT,NO_ASM,NOSTORE) ZBITS.C
$ CC /opt=noinline /define=(EXPORT,NO_ASM,NOSTORE) ZDEFLATE.C
$ CC /opt=noinline /define=(EXPORT,NO_ASM,NOSTORE) ZGLOBALS.C
$ CC /opt=noinline /define=(EXPORT,NO_ASM,NOSTORE) ZINFLATE.C
$ CC /opt=noinline /define=(EXPORT,NO_ASM,NOSTORE) ZIP.C
$ CC /opt=noinline /define=(EXPORT,NO_ASM,NOSTORE) ZIPUP.C
$ CC /opt=noinline /define=(EXPORT,NO_ASM,NOSTORE) ZFILE_IO.C
$ CC /opt=noinline /define=(EXPORT,NO_ASM,NOSTORE) ZTREES.C
$ CC /opt=noinline /define=(EXPORT,NO_ASM,NOSTORE) ZUNZIP.C
$ assign/user MD5.obj MD
$ LINK /TRACE/NOMAP/EXEC=PGP.EXE pgp/opt, VAXCRTL/opt
$ ! do nothing...
$!Last Modified:   8-NOV-1992 00:26:38.56
