#!/usr/local/bin/perl -w

#
# Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
#

package PGP::UserId;

use Stream::Streamable;
@ISA = qw( Stream::Streamable );

use strict;

use overload
	'""' => \&asString;

sub id { shift->{'id'}; }

sub new
{
	my $type = shift; my $self = {}; bless $self, $type;

	$self->{'id'} = shift;

	$self;
}

sub restoreFromDataStream
{
	my $type = shift;
	my $dis = shift;

	$type->new($dis->readAll());
}

sub saveToDataStream
{
	my $self = shift;
	my $dos = shift;

	$dos->write($self->{'id'});
}

sub asString { shift->{'id'}; }

1;
