#!/usr/local/bin/perl -w

#
# Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
#

package Crypt::RSAPublicKey;

use Crypt::RSAKey;
@ISA = qw(Crypt::RSAKey);

use strict;

use Math::BigInteger;


sub new
{
	my $type = shift; my $self = {}; bless $self, $type;

	$self->{'n'} = shift->clone();
	$self->{'e'} = shift->clone();

	$self;
}

sub crypt
{
	my $self = shift;
	my $msg = shift;

	my $r = $msg->clone();	# Cloning to create an object, not to copy $msg
	Math::BigInteger::mod_exp($r, $msg, $self->{'e'}, $self->{'n'});
	$r;
}

sub asString
{
	my $self = shift;

	my $n = $self->{'n'};
	my $e = $self->{'e'};

	"n: $n\ne: $e";
}

sub fingerprintFromMessageDigest
{
	my $self = shift;
	my $md = shift;

	$md->add($self->{'n'}->save());
	$md->add($self->{'e'}->save());
	$md->digestAsHash();
}

1;
