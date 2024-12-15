#!/usr/local/bin/perl -w

#
# Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
#

package PGP::PublicKeyRingEntry;

use strict;

use PGP::KeyRingTrust;


# Private method to access members in %$self
sub _elem
{
	my($self, $elem, $val) = @_;
	my $old = $self->{$elem};
	$self->{$elem} = $val if defined $val;
	return $old;
}


sub key			{ shift->_elem('_key', @_); }
sub key_trust	{ shift->_elem('_key_trust', @_); }
sub id			{ shift->_elem('_id', @_); }
sub id_trust	{ shift->_elem('_id_trust', @_); }

sub new
{
	my $type = shift; my $self = {}; bless $self, $type;

	my $key = shift;
	my $id = shift;

	my $key_trust = new PGP::KeyRingTrust(2); # What is the default?
	my $id_trust = new PGP::KeyRingTrust(3); # What is the default?

	$self->key($key);
	$self->key_trust($key_trust);
	$self->id($id);
	$self->id_trust($id_trust);

	$self;
}

sub display
{
	my $self = shift;

	print "Key id: ", $self->id()->id(), "\n";
	$self->key()->display();
}

1;
