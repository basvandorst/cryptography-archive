# Arithmetic in Global Fields
# Copyright (C) 1990 Kevin R. Coombes and David R. Grant
# For full copyright notice, see the file COPYRIGHT in this directory.

ARCH = ${TARGET_ARCH:-%=%}
LIBDIR = ./Library/${ARCH}

CFLAGS = -O

LIBRARIES = ${LIBDIR}/libU.a \
            ${LIBDIR}/libP.a \
            ${LIBDIR}/libZ.a \
            ${LIBDIR}/libQ.a \
            ${LIBDIR}/libD.a

TESTS = ./Utility/trial ./Tests/all

all : ${LIBRARIES} ${TESTS}

./Utility/trial : ./Utility/libU.a
	(cd Utility; make trial "CFLAGS=${CFLAGS}")

./Tests/all : ${LIBRARIES}
	(cd ./Tests; make all "CFLAGS=${CFLAGS}")

${LIBDIR}/libU.a : ./Utility/libU.a
	(cd Utility; make install)

./Utility/libU.a :
	(cd Utility; make libU.a "CFLAGS=${CFLAGS}")

${LIBDIR}/libP.a : ./Polynomial/libP.a
	(cd Polynomial; make install)

./Polynomial/libP.a :
	(cd Polynomial; make libP.a "CFLAGS=${CFLAGS}")

${LIBDIR}/libZ.a : ./Integer/libZ.a
	(cd Integer; make install)

./Integer/libZ.a :
	(cd Integer; make libZ.a "CFLAGS=${CFLAGS}")

${LIBDIR}/libQ.a : ./Joint/libQ.a
	(cd Joint; make install)

./Joint/libQ.a :
	(cd Joint; make libQ.a "CFLAGS=${CFLAGS}")

${LIBDIR}/libD.a : ./Quadratic/libD.a
	(cd Quadratic; make install)

./Quadratic/libD.a :
	(cd Quadratic; make libD.a "CFLAGS=${CFLAGS}")

clean :
	(cd Utility; make clean)
	(cd Polynomial; make clean)
	(cd Integer; make clean)
	(cd Joint; make clean)
	(cd Quadratic; make clean)

veryclean :
	(cd Utility; make veryclean)
	(cd Polynomial; make veryclean)
	(cd Integer; make veryclean)
	(cd Joint; make veryclean)
	(cd Quadratic; make veryclean)
	(cd Tests; make veryclean)

scrub : veryclean
	-${RM} ${LIBRARIES}
	-${RM} *~ core

