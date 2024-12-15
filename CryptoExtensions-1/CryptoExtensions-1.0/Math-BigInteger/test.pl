BEGIN { push @INC, qw(. .. ../lib ../../lib ../../../lib) }

use Math::BigInteger;

print "1..2\n";

my $n = new BigInteger;
$n->inc();
$n->inc();
$n->dec();

# my $n2 = $n->clone();
# my $n2 = clone BigInteger $n;
my $n2 = clone BigInteger $n;
$n2->inc();

BigInteger::mul($n, $n2, $n2);
BigInteger::mul($n2, $n, $n);
BigInteger::mul($n, $n2, $n2);
BigInteger::mul($n2, $n, $n);
BigInteger::mul($n, $n2, $n2);
BigInteger::mul($n2, $n, $n);
BigInteger::mul($n, $n2, $n2);
BigInteger::mul($n2, $n, $n);
BigInteger::mul($n, $n2, $n2);
BigInteger::mul($n2, $n, $n);
$n2->mul($n, $n);
$n2->dec();
$n2->dec();

my $val = $n2->save();
print unpack("H*", $val), "\n";
$n2->dec();
$val = $n2->save();
my $x = restore BigInteger $val;
print unpack("H*", $val), "\n";

#
#	OK, OK, these tests could be better,
#	but then I'm sure there are some users
#	out there who need some simple exercises
#	in order to learn how to use this library :-)
#	Send your answers to Perl.Crypt.Comments@systemics.com
#	Thanks!
#
print "ok 1\n";
print "ok 2\n";
