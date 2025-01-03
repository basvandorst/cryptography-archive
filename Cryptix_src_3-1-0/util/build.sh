#!/bin/sh

if [ "${javac}" = "" ]; then javac="javac"; fi

mkdir build
mkdir build/classes

cd src

${javac} -d ../build/classes java/lang/*.java netscape/security/*.java java/security/*.java java/security/interfaces/*.java cryptix/*.java cryptix/math/*.java cryptix/mime/*.java cryptix/util/*.java cryptix/util/checksum/*.java cryptix/util/core/*.java
${javac} -d ../build/classes cryptix/util/gui/*.java cryptix/util/io/*.java cryptix/util/math/*.java cryptix/util/mime/*.java cryptix/util/test/*.java cryptix/provider/*.java cryptix/provider/cipher/*.java cryptix/provider/mode/*.java
${javac} -d ../build/classes cryptix/provider/padding/*.java cryptix/provider/key/*.java cryptix/provider/md/*.java cryptix/provider/mac/*.java cryptix/provider/rsa/*.java cryptix/provider/elgamal/*.java cryptix/tools/*.java cryptix/security/*.java
${javac} -d ../build/classes cryptix/security/rsa/*.java cryptix/pgp/*.java cryptix/pgp/charset/*.java cryptix/test/*.java

cd ..

mkdir build/classes/cryptix-lib
cp src/cryptix-lib/* build/classes/cryptix-lib

mkdir build/classes/ijce-lib
cp src/ijce-lib/* build/classes/ijce-lib
