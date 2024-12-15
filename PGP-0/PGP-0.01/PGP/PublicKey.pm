#!/usr/local/bin/perl -w

#
# Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
#

package PGP::PublicKey;

use Crypt::RSAPublicKey;      
use Stream::Streamable;      
@ISA = qw( Stream::Streamable Crypt::RSAPublicKey );


use strict;

use Math::MPI;
use Crypt::MD5;


sub new
{
	my $type = shift; my $self = {}; bless $self, $type;

	my $n = shift;
	my $e = shift;
	return "$n - not a reference" unless ref($n);
	return "$e - not a reference" unless ref($e);

	$self->{'n'} = $n;
	$self->{'e'} = $e;

	$self;
}

sub restoreFromDataStream
{
	my $type = shift;
	my $dis = shift;

	my $n = restoreFromDataStream Math::MPI $dis; return $n unless ref($n);
	my $e = restoreFromDataStream Math::MPI $dis; return $e unless ref($e);

	$n = $n->asBigInteger(); return $n unless ref($n);
	$e = $e->asBigInteger(); return $e unless ref($e);

	new PGP::PublicKey $n, $e;
}

sub saveToDataStream
{
	my $self = shift;
	my $dos = shift;

	my $n = new Math::MPI $self->{'n'};
	my $e = new Math::MPI $self->{'e'};

	$n->saveToDataStream($dos);
	$e->saveToDataStream($dos);
}

sub id
{
    my $self = shift;

	substr($self->{'n'}->save(), -8, 8);
}

sub fingerprint
{
	shift->fingerprintFromMessageDigest(new Crypt::MD5);
}

1;
