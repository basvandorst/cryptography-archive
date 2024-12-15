#!/bin/sh
if [ "${jar}" = "" ]; then jar=jar; fi
if [ "${java}" = "" ]; then java=java; fi

echo "Creating empty test directory..."
rm -rf test
mkdir test
cd test

echo "Extracting Cryptix-classes${CRYPTIX_VERSION}.zip...
${jar} xf ..\..\Cryptix-classes${CRYPTIX_VERSION}.zip

CLASSPATH=${PWD}\cryptix.zip;${PWD}\examples.zip
echo ""
echo "CLASSPATH = ${CLASSPATH}"

echo ""
echo "Checking installation..."
java cryptix.provider.Install

echo "Running tests..."
${java} cryptix.test.TestAll
cd ..
