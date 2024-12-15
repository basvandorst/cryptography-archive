#!/usr/local/bin/perl -w

#
# Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
#

package Crypt::MessageDigest;

#
#	An abstract base class from which message digest
#	functions (eg. SHA, MD5) should inherit
#
#	Derived classes must define the following:
#
#		new
#		add
#		digest
#		digestAsHash
#


sub hash
{
    my ($self, @data) = @_;

    if (ref($self)) # An instance method call
    {
		$self->reset();
    }
    else	# A static method invocation
    {
		$self = new $self;
    }

	$self->add(@data);
	$self->digestAsHash();
}

1;
