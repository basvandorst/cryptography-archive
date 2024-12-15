#!/bin/sh
if [ "${jar}" = "" ]; then jar=jar; fi

CLASSPATH=.

echo "Creating empty directories for classfiles..."
rm -rf build tmp
mkdir -p build tmp/classes tmp/examples

echo "Building classes..."
cd src
sh build.sh -d ../tmp/classes
cd ..

if [ ! "${cryptix11}" = "" ]; then
    echo "Creating cryptix11.zip file..."
    cd tmp/classes
    ${jar} cfM ../../build/cryptix11.zip *
    cd ../..
fi

echo "Building extra classes for Java 1.0.2..."
cd ../src10
sh build.sh

echo "Creating cryptix.zip file..."
cd ../tmp/classes
cp -R ../classes10/* .
# Hack to avoid confusing jar (it would otherwise use the wrong
# version of java.util.zip.*):
CLASSPATH=dummy
${jar} cf0M ../../build/cryptix.zip *
CLASSPATH=.
cd ../..

echo "Building examples..."
cd src
sh build-examples.sh -d ../tmp/examples

echo "Creating examples.zip file..."
cd ../tmp/examples
# No classes in java.* should be included.
rm -rf ../examples/java
${jar} cf0M ../../build/examples.zip *
cd ../..

echo "Copying properties files..."
mkdir build/cryptix-lib
cp src/cryptix-lib/*.properties build/cryptix-lib
mkdir build/ijce-lib
cp src/ijce-lib/*.properties build/ijce-lib
