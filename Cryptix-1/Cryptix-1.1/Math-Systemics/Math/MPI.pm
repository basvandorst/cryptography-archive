#!/usr/local/bin/perl -w

#
# Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
#

#
#	IMPORTANT!  This module is due to change
#	Please refrain from using it!
#
#	Thanks - Gary.Howland@sytstemics.com
#

package Math::MPI;

#
#	Module for reading/writing PGP style multi-precision integers (MPIs)
#

use Stream::Streamable;
@ISA = qw(Stream::Streamable );

use strict;
use integer;

use Math::BigInteger;


sub new
{
	my $type = shift; my $self = {}; bless $self, $type;
	my $bi = shift;

	ref($bi) || return "Usage: new MPI BigInteger ($bi)";

	$self->{'bits'} = $bi->bits();
	$self->{'data'} = $bi->save();

	$self;
}

sub asBigInteger
{
	restore Math::BigInteger shift->{'data'};
}

sub restoreFromDataStream
{
	usage("input-stream") unless @_ == 2;

	my $type = shift; my $self = {}; bless $self, $type;
	my $dis = shift;

	my $bits = $dis->readInt16();
	return "Failed to read MPI header" unless defined $bits;

	my $bytes = (($bits-1)/8)+1;

	my $data = $dis->read($bytes);
	return "Failed to read full MPI" if (length($data) != $bytes);

	$self->{'bits'} = $bits;
	$self->{'data'} = $data;

	$self;
}

sub saveToDataStream
{
	usage("output-stream") unless @_ == 2;

	my $self = shift;
	my $dos = shift;

	$dos->writeInt16($self->{'bits'});
	$dos->write($self->{'data'});
	undef;
}

1;
