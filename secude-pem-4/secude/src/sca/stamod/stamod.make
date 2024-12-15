#
#  STAMOD Release 1.1 (GMD)
#
##########################################################################
###
### ueberabeiten   !!!!!!!
###
#########################################################################
#                                                                       #
#  Projectname          :       STAPAC                                  #
#                                                                       #
#  Date                 :       November 1990                           #
#                                                                       #
#  Modification		:	Version 1.1; January 1993		#
#                                                                       #
#  Author               :       L. Eckstein                             #
#                                                                       #
#  Programmname         :       Makefile                                #
#				The source files are compiled with the  #
#				options TEST and STREAM			#
#                                                                       #
#  Description          :       this makefile generates the             #
#                               - sct_if.o                              #
#                               - sctde.o                              #
#                               - sctmem.o                              #
#                               - iccde.o                               #
#				- sca_init.o				#
#				- stadev.o				#
#				- stasc.o				#
#				- sta_r_w.o				#
#				- staauth.o				#
#				- stacrypt.o				#
#				- staprint.o				#
#				- sta_free.o				#
#				- sta_resp.o				#
#				- sta_xdmp.o				#
#									#
#				with TEST and STREAM			#
#                                                                       #
#  Important            :                                               #
#  Set Environment Varibale STAMOD:                                     #
#  under DOS     : set STAMOD=filename      (../install/ustamod.gen)    #
#  under C-SHELL : setenv STAMOD filename                               #
#  Length of Filename : max 40 characters.                              #
#                                                                       #
#  DFLAGS = MFCHECK  : if set     => malloc/free stack is created       #
#				     should only be used for test       #
#                      if not set => malloc/free only called            #
#                                                                       #
#  DFLAGS = SCTTRACE SYSTEM  MEMTRACE (for sct_if.c)                      #
#									#
#      SCTTRACE =    : if not set => no TRACE of SCT-APDU               #
#                      if set     => Trace of SCT-APDU's in file        #
#                                    SCTINT.TRC                         #
#      SYSTEM =								#
#           DOS      : if set     => compute baudrate for DOS           #
#           BSD      : if set     => compute baudrate for Berkley       #
#           SYSTEMV  : if set     => compute baudrate for System V      #
#									#
#      ELEMTRACE =   : if not set => no TRACE of element                #
#                      if set     => Trace of element    in file        #
#                                    SCTINT.TRC                         #
#				     (only allowed, if SCTTRACE set )     #
#  DFLAGS = SCTTRACE SYSTEM      (for sctde.c)                           #
#									#
#      SCTTRACE =    : if not set => no TRACE of SCT-APDU               #
#                      if set     => Trace of SCT-APDU's in file        #
#                                    SCTINT.TRC                         #
#      SYSTEM =								#
#           DOS      : if set     => Systemcall TIME will be used       #
#                      if not set => Systemcall SLEEP will be used      #
#                                                                       #
#                                                                       #
#  DFLAGS = MEMTRACE  SYSTEM     (for sctmem.c)                            #
#									#
#      MEMTRACE  =   : if not set => no trace of memory-element         #
#                      if set     => Trace of memory    in file        #
#                                    SCTINT.TRC                         #
#				     (only allowed, if SCTTRACE set )     #
#      SYSTEM =								#
#           DOS      : if set     => compute baudrate for DOS           #
#           BSD      : if set     => compute baudrate for Berkley       #
#           SYSTEMV  : if set     => compute baudrate for System V      #
#                                                                       #
#                                                                       #
#  DFLAGS = ICCTRACE               (for iccde.c )                       #
#									#
#      ICCTRACE  =     : if not set => Trace of SC-APDU                 #
#                      if set     => Trace of SC-APDU's in file         #
#                                    SCCOM.TRC                          #
#                                                                       #
#                                                                       #
#                                                                       #
#                                                                       #
#  DFLAGS =   RSA         (for stacrypt.c)                     #
#									#
#      RSA    =      							#
#          ASSEMBLER : if set, then sca_sign, sca_verify  and           #
#                      sca_hash are compiled => assembler routines are  #
#                      used for hash function sqmodn                    #	
#                                                                       #
#  DFLAGS = TEST               (for stadev.o stasc.o sta_r_w.o          #
#                                   staauth.o stacrypt.o staprint.o)    #
#      TEST =								#
#           TEST      : if set => test output                           #
#                                                                       #
#                                                                       #
#########################################################################

#SYSTEM is set in CONFIG.make
#SYSTEM    =  _BSD42_        # !!!!!!!!
#SYSTEM    =  _SYSVxx_        # !!!!!!!!
#COMPILER  =  NOTANSIC
#TEST      =  TEST            
#SCTTRACE  =  SCTTRACE        # for test purpose
#ICCTRACE  =  ICCTRACE
#T1TRACE   =  T1TRACE
#MEMTRACE  =  MEMTRACE
#ELEMTRACE =  ELEMTRACE     
DSCA = 
  
#DFLAGS    = $(DISODE80) $(DICR1) -D$(MEM)  -D$(SYSTEM) -D$(SYSTYPE) -D$(RSA) -D$(VERSION10) $(PROCDAT) $(STREAM) $(TEST) $(MEMTRACE) $(TRACE)


DFLAGS   =  $(DISODE80) $(DICR1) -D$(SYSTEM)

OBJS       =  sca_init.o sct_if.o sctde.o sctmem.o \
              iccde.o  sca_dev.o sca_xdmp.o sca_prnt.o  \
	      sca_file.o sca_cryp.o sca_auth.o sca_data.o \
       	      sca_dkey.o

DD	   = ../


all:       $(LIBSECUDE)

trace:	   ; make "OPT=$(OPT) -DTRACE" 



