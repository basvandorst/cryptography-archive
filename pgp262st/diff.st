diff -cwB pgp262.old/src/makefile pgp262/src/makefile
*** pgp262.old/src/makefile	Thu Oct 13 17:33:34 1994
--- pgp262/src/makefile	Sun Dec 18 02:53:16 1994
***************
*** 151,156 ****
--- 151,161 ----
  $(PROJ):	$(OBJS)
  		$(LD) -o $(PROJ) $(OBJS) $(LDFLAGS) $(LIBS_EXT) $(RSALIBS)
  
+ # Atari-version with gcc
+ atari:
+ 	$(MAKE) all CC=gcc LD=gcc LIBS_EXT=-liio RSALIBS=-lrsaref PROJ=pgp.ttp \
+ 	CFLAGS="$(RSAINCDIR) $(USEMPILIB) -O -DATARI -DHIGHFIRST -DPORTABLE -DUNIT32 -DIDEA32"
+ 
  #  Commodore Amiga Running SVR4 2.1p2a with GCC 2.6.0 LER
  amix-68k-gcc:
  	$(MAKE) all CC=gcc LD=gcc \
diff -cwB pgp262.old/src/pgp.c pgp262/src/pgp.c
*** pgp262.old/src/pgp.c	Tue Oct 11 17:26:10 1994
--- pgp262/src/pgp.c	Sat Dec 17 21:52:28 1994
***************
*** 1170,1181 ****
  #endif
  #endif				/* MSDOS */
  #ifdef SIGINT
- #ifdef ATARI
-     signal(SIGINT, (sigfunc_t) breakHandler);
- #else
      if (signal(SIGINT, SIG_IGN) != SIG_IGN)
  	signal(SIGINT, breakHandler);
! #if defined(UNIX) || defined(VMS)
      if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
  	signal(SIGHUP, breakHandler);
      if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
--- 1174,1182 ----
  #endif
  #endif				/* MSDOS */
  #ifdef SIGINT
      if (signal(SIGINT, SIG_IGN) != SIG_IGN)
  	signal(SIGINT, breakHandler);
! #if defined(UNIX) || defined(VMS) || defined(ATARI)
      if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
  	signal(SIGHUP, breakHandler);
      if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
***************
*** 1193,1199 ****
  #endif
  #endif				/* DEBUG */
  #endif				/* UNIX */
- #endif				/* not Atari */
  #endif				/* SIGINT */
  }				/* initsigs */
  
--- 1194,1199 ----
diff -cwB pgp262.old/src/pgp.h pgp262/src/pgp.h
*** pgp262.old/src/pgp.h	Tue Oct 11 17:26:10 1994
--- pgp262/src/pgp.h	Sat Dec 17 21:53:36 1994
***************
*** 30,39 ****
  #endif
  #endif
  
- #ifdef ATARI
- #define sizeof(x) (int)sizeof(x)
- #define fread(a,b,c,d)	((int)fread(a,b,c,d))
- #endif
  
  /*
  **********************************************************************
--- 30,35 ----
diff -cwB pgp262.old/src/rsaglue2.c pgp262/src/rsaglue2.c
*** pgp262.old/src/rsaglue2.c	Sun Oct  2 23:23:32 1994
--- pgp262/src/rsaglue2.c	Sun Dec 18 02:19:52 1994
***************
*** 52,60 ****
  Uses the RSAREF(tm) Toolkit, which is copyright RSA Data Security, Inc.\n\
  Distributed by the Massachusetts Institute of Technology.\n");
  
! #include <global.h>
! #include <rsaref.h>
! #include <rsa.h>
  /*
   * The functions we call in rsa.h are:
   *
--- 52,60 ----
  Uses the RSAREF(tm) Toolkit, which is copyright RSA Data Security, Inc.\n\
  Distributed by the Massachusetts Institute of Technology.\n");
  
! #include "global.h"
! #include "rsaref.h"
! #include "rsa.h"
  /*
   * The functions we call in rsa.h are:
   *
diff -cwB pgp262.old/src/system.c pgp262/src/system.c
*** pgp262.old/src/system.c	Tue Oct 11 17:26:12 1994
--- pgp262/src/system.c	Sat Dec 17 21:59:54 1994
***************
*** 1707,1714 ****
   */
  
  #ifdef ATARI
! #include <string.h>
! #endif
  
  #if !defined(MSDOS) && !defined(ATARI)
  #include <ctype.h>
--- 1707,1724 ----
   */
  
  #ifdef ATARI
! #include <osbind.h>		/* use GEMDOS functions for I/O */
! 
! int kbhit(void)
! {
! 	return Cconis();	/* ret == 0 : no char available */
! }
! 
! int getch(void)
! {
! 	return (Cnecin() & 0x000000FF);	/* ASCII-Code in Bits 0..7   */
! }					/* Scan-Codes in Bits 16..23 */
! #endif /* ATARI */
  
  #if !defined(MSDOS) && !defined(ATARI)
  #include <ctype.h>
diff -cwB pgp262.old/src/zdeflate.c pgp262/src/zdeflate.c
*** pgp262.old/src/zdeflate.c	Fri Jul 15 13:59:40 1994
--- pgp262/src/zdeflate.c	Sat Dec 17 22:01:14 1994
***************
*** 220,228 ****
   *  MSDOS but not Unix. However the asm version version is recommended
   *  for 386 Unix.
   */
- #ifdef ATARI_ST
- #  undef MSDOS /* avoid the processor specific parts */
- #endif
  #if defined(MSDOS) && !defined(NO_ASM) && !defined(ASM)
  #  define ASM
  #endif
--- 220,225 ----
