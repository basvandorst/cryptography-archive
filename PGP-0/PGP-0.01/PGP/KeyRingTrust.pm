#!/usr/local/bin/perl -w

#
# Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
#

package PGP::KeyRingTrust;

use Stream::Streamable;
@ISA = qw( Stream::Streamable );

use strict;

use overload
	'""' => \&asString;


sub flags { shift->{'flags'}; }

sub new
{
	my $type = shift; my $self = {}; bless $self, $type;

	$self->{'flags'} = shift;

	$self;
}

sub restoreFromDataStream
{
	my $type = shift;
	my $dis = shift;

	$type->new($dis->readByte());
}

sub saveToDataStream
{
	my $self = shift;
	my $dos = shift;

	$dos->writeByte($self->{'flags'});
}

sub asString
{
	my $self = shift;

	$self->flags();
}

1;
