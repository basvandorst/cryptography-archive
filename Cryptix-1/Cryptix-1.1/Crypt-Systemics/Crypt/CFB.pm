#!/usr/local/bin/perl -w

#
# Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
#

package Crypt::CFB;

use strict;
use Carp;



sub usage
{   
	my ($package, $filename, $line, $subr) = caller(1);
	$Carp::CarpLevel = 2;
	croak "Usage: $subr(@_)";
}



sub new
{
	usage("new Crypt::CFB block-cipher") unless @_ == 2;

	my $type = shift; my $self = {}; bless $self, $type;

	my $cipher = shift;
	$self->{'cipher'} = $cipher;
	$self->{'iv'} = "\0" x $cipher->blocksize();

	$self;
}

sub keysize { shift->{'cipher'}->keysize(); }

sub encrypt
{
	usage("encrypt data") unless @_ == 2;

	my $self = shift;
	my $data = shift;

	my $retval = "";
	my $iv = $self->{'iv'};
	my $size = $self->{'cipher'}->blocksize();

	while (length($data) > 0)
	{
		my $in = substr($data, 0, $size);
		substr($data, 0, $size) = '';
		my $out;

		# NB - The following is probably more efficient
		# but we don't check for the second arg yet etc.
		# $self->{'cipher'}->encrypt($iv, $out);
		$out = $self->{'cipher'}->encrypt($iv);

		my $len = length($in);
		substr($iv, 0, $len) = '';

		$in ^= substr($out, 0, $len);

		$iv .= $in;

		$retval .= $in;
	}

	$self->{'iv'} = $iv;

	$retval;
}

sub decrypt
{
	usage("decrypt data") unless @_ == 2;

	my $self = shift;
	my $data = shift;

	my $retval = "";
	my $iv = $self->{'iv'};
	my $size = $self->{'cipher'}->blocksize();

	while (length($data) > 0)
	{
		my $in = substr($data, 0, $size);
		substr($data, 0, $size) = '';
		my $out;

		# NB - The following is probably more efficient
		# but we don't check for the second arg yet etc.
		# $self->{'cipher'}->encrypt($iv, $out);
		$out = $self->{'cipher'}->encrypt($iv);

		my $len = length($in);
		substr($iv, 0, $len) = '';

		$iv .= $in;
		$in ^= substr($out, 0, $len);

		$retval .= $in;
	}

	$self->{'iv'} = $iv;

	$retval;
}

1;
