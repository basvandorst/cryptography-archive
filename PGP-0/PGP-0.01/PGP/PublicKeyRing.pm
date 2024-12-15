#!/usr/local/bin/perl -w

#
# Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
#

package PGP::PublicKeyRing;

use Stream::Streamable;
@ISA = qw( Stream::Streamable );


use strict;

use PGP::PacketFactory;
use PGP::PublicKeyRingEntry;


sub new
{
	my $type = shift; my $self = {}; bless $self, $type;

	$self;
}

sub add
{
	my $self = shift;
	my $entry = shift;

	$self->{$entry->id()} = $entry;
}

sub restoreFromDataStream
{
	my $type = shift; my $self = {}; bless $self, $type;
	my $dis = shift;

	my $packet = PGP::PacketFactory::restore($dis);
	while (defined($packet))
	{
		return $packet unless ref $packet;
		(ref($packet) eq "PGP::PublicKeyCertificate")
								|| return "Bad format public key ring";

		my $pkp = $packet;
		$packet = PGP::PacketFactory::restore($dis);

		my $key_trust;
		if (ref($packet) eq "PGP::KeyRingTrust")
		{
			$key_trust = $packet;
			$packet = PGP::PacketFactory::restore($dis);
		}

		my $idp;
		my $id_trust;
		if (ref($packet) eq "PGP::UserId")
		{
			$idp = $packet;
			$packet = PGP::PacketFactory::restore($dis);
			return $packet unless ref $packet;

			if (ref($packet) eq "PGP::KeyRingTrust")
			{
				$id_trust = $packet;
				$packet = PGP::PacketFactory::restore($dis);
			}
		}

		my $id = $idp->id();
		$self->{"$id"} = new PGP::PublicKeyRingEntry;
		$self->{"$id"}->key($pkp);
		$self->{"$id"}->key_trust($key_trust);
		$self->{"$id"}->id($idp);
		$self->{"$id"}->id_trust($id_trust);


		# Now we should look for some comment/signature packets
		# Do it later ...
		while (defined ($packet))
		{
			last if (ref($packet) eq "PGP::PublicKeyCertificate");
			$packet = PGP::PacketFactory::restore($dis);
			last unless defined $packet;
			return $packet unless ref $packet;
		}
	}

	$self;
}

sub saveToDataStream
{
	my $self = shift;
	my $dos = shift;

	my ($id, $key);
	while (($id, $key) = each %$self)
	{
		PGP::PacketFactory::save($dos, $key->key());
		PGP::PacketFactory::save($dos, $key->key_trust()) if defined $key->key_trust();
		if (defined $key->id())
		{
			PGP::PacketFactory::save($dos, $key->id());
			PGP::PacketFactory::save($dos, $key->id_trust()) if defined $key->id_trust();
		}
	}
}

#
#	Get public key details
#
sub getKeyDetails
{
	my $self = shift;
	my $id = shift;

	$self->{"$id"};
}

#
#	Get whole entry
#
sub get
{
	my $self = shift;
	my $id = shift;

	$self->{"$id"};
}

#
#	Get a public key certificate
#
sub getKey
{
	my $self = shift;
	my $id = shift;

	return unless defined $self->{"$id"};
	$self->{"$id"}->key();
}

sub display
{
	my $self = shift;

	my ($id, $key);
	while (($id, $key) = each %$self)
	{
		$key->display();
		print "\n";
	}
}

1;
