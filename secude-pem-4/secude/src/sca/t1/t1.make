#
#  STAMOD Release 1.1 (GMD)
#
#########################################################################
#                                                                       #
#  Projectname          :       STAPAC                                  #
#                                                                       #
#  Date                 :       November 1991                           #
#                                                                       #
#  Author               :       L. Eckstein                             #
#                                                                       #
#  Filename             :       Makefile                                #
#                                                                       #
#  Description          :       this makefile generates the t1-         #
#                               object                                  #
#                                                                       #
#  DFLAGS		:       -DT1TRACE -D_DOS_   for DOS		#
#  DFLAGS		:       -DT1TRACE -D_BSD42_ for BSD		#
#  DFLAGS		:       -DT1TRACE -D_POSIX_   for  HP           #
#  DFLAGS		:       -DT1TRACE -D_MACC7_   for  MAC          #
#                                                                       #
#  DFLAG = MFCHECK if malloc/free macro in MF_check.h should be used    #	
#########################################################################

#SYSTEM is set in CONFIG.make
#SYSTEM   =      _BSD42_
T1TRACE  =      T1TRACE
#TIME	=	TIME
#MFCHECK  =      MFCHECK

DFLAGS    =  -D$(SYSTEM) $(DISODE80) $(DICR1)


DSCA     =
DD         = ../

CC = cc $(CCPIC)

OBJS       =  t1.o
 
all: $(LIBSECUDE)

#t1.o:
#	$(CC) -c t1.c $(CFLAGS) $(DFLAGS) $(IFLAGS) -D$(SYSTEM)


trace:	; make "OPT=$(OPT) -DTRACE"




























