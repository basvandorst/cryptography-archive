#!/bin/sh
if [ "${javac}" = "" ]; then javac=java -ms2m sun.tools.javac.Main; fi

echo "Deleting temporary directories..."
rm -rf tmp
echo "Deleting classfiles..."
find . -name "*.class" |xargs rm -f
echo "Compiling utility programs..."
${javac} util/*.java
