BEGIN { push @INC, qw(. .. ../lib ../../lib ../../../lib) }

package Crypt::Blowfish;

require Exporter;
require DynaLoader;

@ISA = (Exporter, DynaLoader);

bootstrap Crypt::Blowfish;



use strict;
use Carp;

sub usage
{
    my ($package, $filename, $line, $subr) = caller(1);
	$Carp::CarpLevel = 2;
	croak "Usage: $subr(@_)"; 
}


sub blocksize { 8; }
sub keysize { 0; }

sub new
{
	usage("new Blowfish key") unless @_ == 2;

	my $type = shift; my $self = {}; bless $self, $type;

	$self->{'ks'} = Crypt::Blowfish::init(shift);

	$self;
}

sub encrypt
{
	usage("encrypt data[8 bytes]") unless @_ == 2;

	my $self = shift;
	my $data = shift;

	Crypt::Blowfish::crypt($data, $data, $self->{'ks'}, 0);

	$data;
}

sub decrypt
{
	usage("decrypt data[8 bytes]") unless @_ == 2;

	my $self = shift;
	my $data = shift;

	Crypt::Blowfish::crypt($data, $data, $self->{'ks'}, 1);

	$data;
}


package main;


#
# '6162636465666768696a6b6c6d6e6f707172737475767778797a',
#			'424c4f5746495348', '324ed0fef413a203',
#
# '57686f206973204a6f686e2047616c743f', 'fedcba9876543210', 'cc91732b8022f684')
#

print "1..2\n";

my $key = pack("H*", "6162636465666768696a6b6c6d6e6f707172737475767778797a");
my $in = pack("H*", "424c4f5746495348");
my $out = pack("H*", "324ed0fef413a203");

my $cipher = new Crypt::Blowfish $key;

print "not " unless ($cipher->encrypt($in) eq $out);
print "ok 1\n";

my $key = pack("H*", "57686f206973204a6f686e2047616c743f");
my $in = pack("H*", "fedcba9876543210");
my $out = pack("H*", "cc91732b8022f684");

my $cipher = new Crypt::Blowfish $key;

print "not " unless ($cipher->decrypt($out) eq $in);
print "ok 2\n";

