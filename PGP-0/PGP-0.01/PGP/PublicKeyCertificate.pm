#!/usr/local/bin/perl -w

#
# Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
#

package PGP::PublicKeyCertificate;

use Stream::Streamable;
@ISA = qw( Stream::Streamable );

use strict;
use POSIX;

use PGP::PublicKey;

use overload
	'""' => \&asString;



#
#	TODO
#
#	Make a key factory so that this code
#	doesn't have to deal with algorithm bytes
#


sub publicKey { shift->{'pk'}; }


sub new
{
	my $type = shift; my $self = {}; bless $self, $type;

	my $pk = shift;
	ref($pk) || return "public-key parameter missing";
	(ref($pk) eq 'PGP::PublicKey') or $pk = new PGP::PublicKey $pk->n(), $pk->e();

	$self->{'pk'} = $pk;
	$self->{'validity'} = shift || 0;
	$self->{'version'} = 2;
	$self->{'timestamp'} = time();
	$self->{'alg'} = 1;

	$self;
}

sub restoreFromDataStream
{
	my $type = shift; my $self = {}; bless $self, $type;
	my $dis = shift;

	my $version = $dis->readByte();
	return "Could not version byte" unless defined $version;
	return "Bad version ($version)" unless ($version == 2 || $version == 3);

	my $timestamp = $dis->readTime();
	return "Could not read time" unless defined $timestamp;

	my $validity = $dis->readInt16();
	return "Could not read validity" unless defined $validity;

	my $alg = $dis->readByte();
	return "Could not read algorithm byte" unless defined $alg;

	my $pk = restoreFromDataStream PGP::PublicKey $dis;
	return "Could not read public key ($pk)" unless ref $pk;

	$self->{'version'} = $version;
	$self->{'timestamp'} = $timestamp;
	$self->{'validity'} = $validity;
	$self->{'alg'} = $alg;
	$self->{'pk'} = $pk;

	$self;
}

sub saveToDataStream
{
	my $self = shift;
	my $dos = shift;

	$dos->writeByte($self->{'version'});
	$dos->writeTime($self->{'timestamp'});
	$dos->writeInt16($self->{'validity'});
	$dos->writeByte($self->{'alg'});
	$self->{'pk'}->saveToDataStream($dos);
}


sub asString
{
	my $self = shift;

	my $retval = "version byte: \t" . $self->{'version'} . "\n";
	$retval .= "Timestamp: \t" . POSIX::ctime($self->{'timestamp'}) . "\n";
	$retval .= "validity (days): \t" . $self->{'validity'} . "\n";
	#
	# algorithm byte (is 1 for RSA)
	#
	$retval .= "alg: \t" . $self->{'alg'} . "\n";

	$retval .= "Public key:\n";
	$retval .= $self->{'pk'}->asString();
}




# This is going to go ...
sub display { print shift->asString(), "\n"; }

#
#	This doesn't belong in this file
#	It belongs in a ascii armouring factory
#
sub saveToDataOutputAsAscii
{
	usage("saveToDataOutputAsAscii DataOutputStream") unless @_ == 2;

	my $self = shift;
	my $dos = shift;

	$dos->write("-----BEGIN PGP PUBLIC KEY BLOCK-----\n");
	$dos->write("Version: 2.6.2\n");
	$dos->write("\n");
	$dos->write(Armour::messageWithChecksum($self->saveWithHdr()));
	$dos->write("-----END PGP PUBLIC KEY BLOCK-----\n");
}

1;
