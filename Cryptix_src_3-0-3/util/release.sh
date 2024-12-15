#!/bin/sh
if [ "${CRYPTIX_VERSION}" = "" ]; then
    echo "Set CRYPTIX_VERSION to the suffix of the release files, in the form \"-x_y_z\"."
    exit
fi

sh util/zipdoc.sh
sh util/zipclasses.sh
sh util/zipsrc.sh
sh util/zipnative.sh
