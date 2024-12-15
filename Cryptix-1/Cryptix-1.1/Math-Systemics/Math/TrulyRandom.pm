#! /usr/local/bin/perl -w -I/usr/local/lib/perl5

#
# Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
#

package Math::TrulyRandom;

require Exporter;
require DynaLoader;

@ISA = qw(Exporter DynaLoader);

bootstrap Math::TrulyRandom;

1;
