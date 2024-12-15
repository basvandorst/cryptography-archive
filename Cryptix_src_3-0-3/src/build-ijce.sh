#!/bin/sh
if [ "${javac}" = "" ]; then javac="javac -J-ms2m"; fi

${javac} $* java/lang/*.java netscape/security/*.java java/security/*.java java/security/interfaces/*.java
