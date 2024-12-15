#!/usr/local/bin/perl -w

#
# Copyright (C) 1995, 1996 Systemics Ltd (http://www.systemics.com/)
# All rights reserved.
#

package PGP::Armoury;

use strict;
use integer;
use PGP::CRC;

use Stream::IO;

# Is this needed?
use FileHandle;


sub encode_base64
{
	my $res = "";
	my $eol = $_[1];
	$eol = "\n" unless defined $eol;
	while ($_[0] =~ /(.{1,45})/gs)
	{
		$res .= substr(pack('u', $1), 1);
		chop($res);
	}
	$res =~ tr|` -_|AA-Za-z0-9+/|;
	# fix padding at the end
	my $padding = (3 - length($_[0]) % 3) % 3;
	$res =~ s/.{$padding}$/'=' x $padding/e if $padding;
	# break encoded string into lines of no more than 64 characters each
	if (length $eol)
	{
		$res =~ s/(.{1,64})/$1$eol/g;
	}
	$res;
}

sub decode_base64
{
	local($^W) = 0; # unpack("u",...) gives bogus warning in 5.001m

	my $str = shift;
	my $res = "";

	$str =~ tr|A-Za-z0-9+/||cd;             # remove non-base64 chars (padding)
	$str =~ tr|A-Za-z0-9+/| -_|;            # convert to uuencoded format
	while ($str =~ /(.{1,60})/gs)
	{
		my $len = chr(32 + length($1)*3/4); # compute length byte
		$res .= unpack("u", $len . $1 );    # uudecode
	}
	$res;
}

sub checksum
{
	usage("checksum msg") unless @_ == 1;

	my $data = shift;

	"=" . PGP::Armoury::encode_base64(substr(pack("N", PGP::CRC::checksum($data)), 1));
}

sub messageWithChecksum
{
	my $data = shift;

	PGP::Armoury::encode_base64($data) . PGP::Armoury::checksum($data);
}


sub create
{
	my $version = shift;
	my $data = shift;

	print("-----BEGIN PGP MESSAGE-----\n");
	print("Version: $version\n\n");
	print encode_base64($data);
	print("-----END PGP MESSAGE-----\n");
}





sub readUntil
{
	usage("pattern data-input-stream") unless @_ == 2;

	my $pattern = shift;
	my $dis = shift;

	my @retval;

	while (defined($_ = $dis->readLine()))
	{
		return @retval if (/$pattern/);
		push @retval, $_;
	}
	@retval;
}

#
#	Does this work?
#
sub readUntilBegin
{
	usage("readUntilBegin DataInputStream") unless @_ == 1;

	my $dis = shift;

	my @begin = PGP::Armoury::readUntil("^-----BEGIN PGP ", $dis);
	return unless defined(@begin);
	unless (defined(PGP::Armoury::readUntil("\^\$", $dis)))
	{
		warn("No blank line found after begin line\n");
		return;
	}
	@begin;
}



sub readPacket
{
	usage("data-input-stream") unless @_ == 1;

	my $dis = shift;

	# Skip the header
	my @begin = PGP::Armoury::readUntil("^-----BEGIN PGP ", $dis);
	return (undef, "begin line not found") unless defined(@begin);

	# Skip the blank line
	defined(PGP::Armoury::readUntil("\^\$", $dis))
		|| return (undef, "No blank line found after begin line");

	my @body = PGP::Armoury::readUntil("^-----END PGP ", $dis);
	my $csum = pop @body;

	my $body = ''; foreach(@body) { $body .= $_; }
	my $body = PGP::Armoury::decode_base64($body);
	my $sum = PGP::Armoury::checksum($body);
	chop $sum;

	($csum eq $sum) || return (undef, "Incorrect checksum");

	($body, undef);
}

sub readPacketFromString
{
	my $sis = new Stream::StringInput shift;
	my $dis = new Stream::DataInput $sis;

	PGP::Armoury::readPacket($dis);
}




#
#	All old ...
#	will soon be gone ...
#

# sub readPacket
# {
# 	my $dis = shift;
# 
# 	# Skip the header
# 	PGP::Armoury::readUntilBegin($dis);
# 
# 	# Read pkt
# # 	my @pkt = PGP::Armoury::readUntil("^-----END PGP ", $dis);
# 	pop @pkt;
# 	my $csum = pop @pkt;
# 
# 	my $pkt = ''; foreach(@pkt) { $pkt .= $_; }
# 	my $pkt = PGP::Armoury::decode_base64($pkt);
# 	my $sum = PGP::Armoury::checksum($pkt);
# 	chop $sum;
# 
# 	if ($csum ne $sum)
# 	{
# 		return(undef, "Incorrect checksum");
# 	}
# 
# 	return ($pkt, undef);
# }
# 
# sub readPacketFromString
# {
# 	usage("readPacketFromString string") unless @_ == 1;
# 
# 	my $str = shift;
# 	my $sis = new Stream::StringInput $str;
# 	my $dis = new Stream::DataInput $sis;
# 	PGP::Armoury::readPacket $dis;
# }
# 
sub writePublicKeyToStream
{
	usage("writePublicKeyToStream DataOutputStream keydata") unless @_ == 1;

	my $dos = shift;
	my $keydata = shift;

#
#	Of course the following isn't quite right yet ...
#
	$dos->write("Type bits/keyID    Date        User ID\n");
	$dos->write("pub  1024/CE471D69 1995/08/27  Gary Howland\n");
	$dos->write("-----BEGIN PGP PUBLIC KEY BLOCK-----\n");
	$dos->write("Version: 2.6.2\n");

	$dos->write("\n");

	my $asc = PGP::Armoury::messageWithChecksum($keydata);

	$dos->write("-----END PGP PUBLIC KEY BLOCK-----\n");
}

sub readPublicKeyFromStream
{
	usage("readPublicKeyFromStream DataInputStream") unless @_ == 1;

	my $dis = shift;

	# Skip the header
	PGP::Armoury::readUntilBegin($dis);

	# Read sig
	my @sig = PGP::Armoury::readUntil("^-----END PGP ", $dis);
	my $csum = pop @sig;

	my $sig = ''; foreach(@sig) { $sig .= $_; }
	my $sig = PGP::Armoury::decode_base64($sig);
	my $sum = PGP::Armoury::checksum($sig);
	chop $sum;

	if ($csum ne $sum)
	{
		warn("Incorrect checksum");
		return;
	}

	$sig;
}


sub readSignatureFromString
{
	usage("readSignatureFromString string") unless @_ == 1;

	my $str = shift;
	my $sis = new StringInputStream $str;
	my $dis = new DataInputStream $sis;
	PGP::Armoury::readSignatureFromStream $dis;
}

sub readSignatureFromStream
{
	usage("readSignatureFromStream DataInputStream") unless @_ == 1;
	
	my $dis = shift;
	my @msg = readSigFileMsg($dis);
	my $sig = readSigFileSignature($dis);
	($sig, @msg);
}


#
#	The following two function should always be used as a pair,
#	eg:
#		my @msg = readSigFileMsg($dis);
#		my $sig = readSigFileSignature($dis);
#


sub readSigFileMsg
{
	usage("readSigFileMsg DataInputStream") unless @_ == 1;

	my $dis = shift;

	# Skip the header
	PGP::Armoury::readUntilBegin($dis);

	# Read the message
	my @msg = PGP::Armoury::readUntilBegin($dis);
	pop @msg;	# Remove the trailing blank line

	@msg;
}

sub readSigFileSignature
{
	usage("readSigFileSignature DataInputStream") unless @_ == 1;

	my $dis = shift;

	# Read sig
	my @sig = PGP::Armoury::readUntil("^-----END PGP ", $dis);
	my $csum = pop @sig;

	my $sig = ''; foreach(@sig) { $sig .= $_; }
	my $sig = PGP::Armoury::decode_base64($sig);
	my $sum = PGP::Armoury::checksum($sig);
	chop $sum;

	if ($csum ne $sum)
	{
		warn("Incorrect checksum");
		return;
	}

	$sig;
}

1;
