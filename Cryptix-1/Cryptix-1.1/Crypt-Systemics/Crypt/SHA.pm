#! /usr/local/bin/perl -w

#
# Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
#

package Crypt::SHA;

use Exporter;
use DynaLoader;
use Crypt::MessageDigest;
@ISA = qw(Exporter DynaLoader Crypt::MessageDigest);

$VERSION = '1.08';
bootstrap Crypt::SHA $VERSION;

use Crypt::HashSHA;

sub digestAsHash
{
	new Crypt::HashSHA shift->digest();
}

1;
