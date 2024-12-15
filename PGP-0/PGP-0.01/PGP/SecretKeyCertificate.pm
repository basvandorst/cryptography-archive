#!/usr/local/bin/perl -w

#
# Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
#

package PGP::SecretKeyCertificate;

#
#	Cipher_init should be an object
#

use Stream::Streamable;
@ISA = qw( Stream::Streamable );

use strict;
use POSIX;
use Carp;

use Math::MPI;
use PGP::SecretKey;


#
#	This is a hack!
#	We need a block cipher factory,
#	not hard coded IDEA
#
use Crypt::IDEA;
use Crypt::MD5;
use Crypt::CFB;


#
#	TODO
#
#	Make a key factory so that this code
#	doesn't have to deal with algorithm bytes
#


sub secretKey { shift->{'sk'}; }
sub encrypted { shift->{'cc_alg'}; }

sub publicKey
{
	my $self = shift;
	new PGP::PublicKey $self->{'n'}, $self->{'e'};
}


sub new
{
	my $type = shift; my $self = {}; bless $self, $type;
	my $sk = shift;
	my $passphrase = shift;
	my $ris = shift;


	$self->{'version'} = '2';
	$self->{'timestamp'} = time();
	$self->{'validity'} = 0;
	$self->{'pkc_alg'} = 1;
	$self->{'sk'} = $sk;

	$self->{'n'} = $sk->n();
	$self->{'e'} = $sk->e();


	#
	#	Need to store the encrypted part as hdr/body
	#

	$self->{'dh'} = pack("n", $sk->d()->bits());
	$self->{'ph'} = pack("n", $sk->p()->bits());
	$self->{'qh'} = pack("n", $sk->q()->bits());
	$self->{'uh'} = pack("n", $sk->u()->bits());

	$self->{'db'} = $sk->d()->save();
	$self->{'pb'} = $sk->p()->save();
	$self->{'qb'} = $sk->q()->save();
	$self->{'ub'} = $sk->u()->save();

	$self->{'csum'} = $self->csum();

	if (!defined($passphrase))
	{
		$self->{'cc_alg'} = 0;
		$self->{'cipher_init'} = "\0" x 8;
	}
	else
	{
		ref($ris) || return "random-input-stream undefined";
		$self->encrypt($passphrase, $ris)
	}

	$self;
}

sub readCryptedMPI
{
	my $dis = shift;

	my $hdr = $dis->read(2);
	return unless (defined($hdr) && length($hdr) == 2);
	my $len = unpack("n", $hdr);
	my $body = $dis->read(int(($len-1)/8)+1);
	return unless (defined($body));
	($hdr, $body);
}

sub csum
{
	my $self = shift;

	my $sum = 0;
	$sum += unpack("%16C*", $self->{'dh'});
	$sum += unpack("%16C*", $self->{'db'});
	$sum += unpack("%16C*", $self->{'ph'});
	$sum += unpack("%16C*", $self->{'pb'});
	$sum += unpack("%16C*", $self->{'qh'});
	$sum += unpack("%16C*", $self->{'qb'});
	$sum += unpack("%16C*", $self->{'uh'});
	$sum += unpack("%16C*", $self->{'ub'});
	($sum & 0xFFFF);
}

sub restoreFromDataStream
{
	my $type = shift; my $self = {}; bless $self, $type;
	my $dis = shift;


	$self->{'version'} = $dis->readByte();
	$self->{'timestamp'} = $dis->readTime();
	$self->{'validity'} = $dis->readInt16();
	$self->{'pkc_alg'} = $dis->readByte();


	my $n = restoreFromDataStream Math::MPI $dis;
	my $e = restoreFromDataStream Math::MPI $dis;

	$self->{'cc_alg'} = $dis->readByte();

	$self->{'cipher_init'} = $dis->read(8);

	($self->{'dh'}, $self->{'db'}) = readCryptedMPI($dis);
	($self->{'ph'}, $self->{'pb'}) = readCryptedMPI($dis);
	($self->{'qh'}, $self->{'qb'}) = readCryptedMPI($dis);
	($self->{'uh'}, $self->{'ub'}) = readCryptedMPI($dis);

	# Just in case they were not read in correctly ...
	return "problems reading D" unless (defined $self->{'db'});
	return "problems reading P" unless (defined $self->{'pb'});
	return "problems reading Q" unless (defined $self->{'qb'});
	return "problems reading U" unless (defined $self->{'ub'});

	$self->{'csum'} = $dis->readInt16();

	$self->{'n'} = $n->asBigInteger();
	$self->{'e'} = $e->asBigInteger();

	#
	#	If unencrypted
	#
	if ($self->{'cc_alg'} == 0)
	{
		my $d = restore Math::BigInteger $self->{'db'};
		my $p = restore Math::BigInteger $self->{'dp'};
		my $q = restore Math::BigInteger $self->{'dq'};
		my $u = restore Math::BigInteger $self->{'du'};

		$self->{'sk'} = new PGP::SecretKey $self->{'n'}, $self->{'e'}, $d, $p, $q, $u;
	}

	$self;
}

sub saveToDataStream
{
	my $self = shift;
	my $dos = shift;

	my $mpi_n = new Math::MPI $self->{'n'};
	my $mpi_e = new Math::MPI $self->{'e'};

	$dos->writeByte($self->{'version'});
	$dos->writeTime($self->{'timestamp'});
	$dos->writeInt16($self->{'validity'});
	$dos->writeByte($self->{'pkc_alg'});
	$mpi_n->saveToDataStream($dos);
	$mpi_e->saveToDataStream($dos);
	$dos->writeByte($self->{'cc_alg'});
	$dos->write($self->{'cipher_init'});
	$dos->write($self->{'dh'});
	$dos->write($self->{'db'});
	$dos->write($self->{'ph'});
	$dos->write($self->{'pb'});
	$dos->write($self->{'qh'});
	$dos->write($self->{'qb'});
	$dos->write($self->{'uh'});
	$dos->write($self->{'ub'});
	$dos->writeInt16($self->{'csum'});
}


sub encrypt
{
	my $self = shift;
	my $passphrase = shift;
	my $ris = shift;

	croak("random-input undefined") unless ref($ris);

	my $sk = $self->{'sk'};
	croak("Secret key missing") unless (defined($sk));

	my $key = Crypt::MD5->hash($passphrase);
	my $block_cipher = new Crypt::IDEA $key->data();
	my $cipher = new Crypt::CFB $block_cipher;
	$self->{'cipher_init'} = $cipher->encrypt($ris->read(8));

	$self->{'cc_alg'} = 1;
	$self->{'db'} = $cipher->encrypt($self->{'db'});
	$self->{'pb'} = $cipher->encrypt($self->{'pb'});
	$self->{'qb'} = $cipher->encrypt($self->{'qb'});
	$self->{'ub'} = $cipher->encrypt($self->{'ub'});

# Note - PGP documentation is wrong
#	$self->{'csum'} = unpack("n", $cipher->encrypt(pack("n", $self->{'csum'})));
}

sub decrypt
{
	my $self = shift;
	my $passphrase = shift;

	return "Strange - sk not defined" if (defined($self->{'sk'}));

	my $key = Crypt::MD5->hash($passphrase);
	my $block_cipher = new Crypt::IDEA $key->data();
	my $cipher = new Crypt::CFB $block_cipher;
	my $r = $cipher->decrypt($self->{'cipher_init'});

	$self->{'cc_alg'} = 0;
	$self->{'db'} = $cipher->decrypt($self->{'db'});
	$self->{'pb'} = $cipher->decrypt($self->{'pb'});
	$self->{'qb'} = $cipher->decrypt($self->{'qb'});
	$self->{'ub'} = $cipher->decrypt($self->{'ub'});

# Note - PGP documentation is wrong
#	$self->{'csum'} = unpack("n", $cipher->decrypt(pack("n", $self->{'csum'})));

	return "Bad checksum" unless ($self->{'csum'} == $self->csum());

	my $d = restore Math::BigInteger $self->{'db'};
	my $p = restore Math::BigInteger $self->{'pb'};
	my $q = restore Math::BigInteger $self->{'qb'};
	my $u = restore Math::BigInteger $self->{'ub'};

	$self->{'sk'} = new PGP::SecretKey $self->{'n'}, $self->{'e'}, $d, $p, $q, $u;
}


sub asString
{
	my $self = shift;

	my $retval = "version byte: \t" . $self->{'version'} . "\n";
	$retval .= "Timestamp: \t" . POSIX::ctime($self->{'timestamp'});
	$retval .= "validity (days): \t" . $self->{'validity'} . "\n";
	#
	# algorithm byte (is 1 for RSA)
	#
	$retval .= "Public key alg: \t" . $self->{'pkc_alg'} . "\n";
	$retval .= "Conventional alg: \t" . $self->{'cc_alg'} . "\n";

	if (ref($self->{'sk'}))
	{
		$retval .= "Secret key:\n";
		$retval .= $self->{'sk'}->asString();
	}
	else
	{
		$retval .= "Secret key is encrypted\n";
	}
	$retval;
}




#
#	This is all due to go ...
#



sub display { print shift->asString(), "\n"; }

#
#	This should be in an ascii armour factory
#
sub saveToDataOutputAsAscii
{
	usage("saveToDataOutputAsAscii DataOutputStream") unless @_ == 2;

	my $self = shift;
	my $dos = shift;

#
#	Of course the following isn't quite right yet ...
#
	$dos->write("Type bits/keyID    Date        User ID\n");
	$dos->write("pub  1024/CE471D69 1995/08/27  Gary Howland\n");

	#
	#	All of the following should really be in Armour.pm
	#

	$dos->write("-----BEGIN PGP PUBLIC KEY BLOCK-----\n");
	$dos->write("Version: 2.6.2\n");

	$dos->write("\n");

	my $asc = Armour::encode_base64($self->save());
	print "$asc\n";

	# This should of course be in Armour.pm
	$dos->write("=H839\n");		# Hmm, Need to write checksum code
#	my $sum = Armour::checksum($sig);

	$dos->write("-----END PGP PUBLIC KEY BLOCK-----\n");
}

1;
