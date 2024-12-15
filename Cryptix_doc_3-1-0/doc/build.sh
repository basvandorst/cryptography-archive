#!/bin/sh
if [ "${javadoc}" = "" ]; then javadoc="java -ms2m sun.tools.javadoc.Main"; fi

if [ "${JDK_SOURCE}" = "" ]; then echo "You need to set the JDK_SOURCE variable."; fi
javadoc -sourcepath "../src:${JDK_SOURCE}" -d . -author $* java.security java.security.interfaces cryptix cryptix.util.core cryptix.provider cryptix.provider.cipher cryptix.provider.mode cryptix.provider.padding cryptix.provider.key cryptix.provider.md cryptix.provider.mac cryptix.provider.rsa cryptix.provider.elgamal cryptix.provider.rpk cryptix.tools cryptix.util.i18n cryptix.util.io cryptix.util.math cryptix.util.checksum cryptix.util.mime cryptix.util.test
