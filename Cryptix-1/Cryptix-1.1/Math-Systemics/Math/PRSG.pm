#! /usr/local/bin/perl -w

#
# Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
#

package Math::PRSG;

require DynaLoader;
@ISA = qw(DynaLoader);

$VERSION = '1.01';
bootstrap Math::PRSG $VERSION;

1;
