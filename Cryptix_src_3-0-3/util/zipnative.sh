#!/bin/sh
if [ "${jar}" = "" ]; then jar=jar; fi

if [ "${ARCH}" = "" ]; then
    echo "You need to set the ARCH variable to specify your hardware and OS."
    exit
fi

echo "Creating empty nzip/cryptix-lib/bin directory..."
rm -rf nzip
mkdir -p nzip/cryptix-lib/bin

echo "Copying native libraries..."
cp native/build/%ARCH%/* nzip/cryptix-lib/bin

echo "Creating Cryptix-${ARCH}${CRYPTIX_VERSION}.zip file..."
cd nzip
rm -f ../../Cryptix-${ARCH}${CRYPTIX_VERSION}.zip
${jar} cfM ../../Cryptix-${ARCH}${CRYPTIX_VERSION}.zip *
cd ..

echo "Deleting nzip directory..."
rm -rf nzip
