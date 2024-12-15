#!/usr/local/bin/perl -w

#
# Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
#

package Crypt::HashSHA;

# Tempting ...
# @HashSHA::ISA = qw(Crypt::HashSHA);

use Crypt::MessageHash;
@ISA = qw(Crypt::MessageHash);

sub size { 20; }
sub name { "SHA"; }

#
#	These should really be inherited
#
use overload
	'cmp' => "cmp",
	'<=>' => "cmp",
	'""' => "asString";

1;
