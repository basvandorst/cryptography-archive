#!/bin/sh
if [ "${javac}" = "" ]; then javac="javac -J-ms2m"; fi

${javac} $* cryptix/examples/*.java cryptix/examples/math/*.java
# ${javac} $* cryptix/examples/ssl/*.java
# ${javac} $* cryptix/examples/pgp/*.java cryptix/examples/pgp/engine/*.java
