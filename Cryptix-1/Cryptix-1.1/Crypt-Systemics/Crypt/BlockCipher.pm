#!/usr/local/bin/perl -w

#
# Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
#

package Crypt::BlockCipher;

#
#	Crypt::BlockCipher - an abstract base class
#

use strict;
use Carp;


sub new { carp("Cannot call undefined function in abstract base class"); }
sub encrypt { carp("Cannot call undefined function in abstract base class"); }
sub decrypt { carp("Cannot call undefined function in abstract base class"); }
sub blocksize { carp("Cannot call undefined function in abstract base class"); }
sub keysize { carp("Cannot call undefined function in abstract base class"); }

1;
