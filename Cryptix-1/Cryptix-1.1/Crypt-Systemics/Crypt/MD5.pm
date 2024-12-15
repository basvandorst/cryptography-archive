#! /usr/local/bin/perl -w

#
# Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
#

package Crypt::MD5;

use Exporter;
use DynaLoader;
use Crypt::MessageDigest;
@ISA = qw(Exporter DynaLoader Crypt::MessageDigest);

$VERSION = '1.08';
bootstrap Crypt::MD5 $VERSION;

use Crypt::HashMD5;

sub digestAsHash
{
	new Crypt::HashMD5 shift->digest();
}

1;
