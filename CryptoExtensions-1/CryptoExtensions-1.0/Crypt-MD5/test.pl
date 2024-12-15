BEGIN { push @INC, qw(. .. ../lib ../../lib ../../../lib) }

use Crypt::MD5;

sub DoTest
{
    my ($str, $expect) = @_;
    my ($digest, $hex);
    my $md5 = new MD5;

    $md5->add($str);
    $digest = $md5->digest();
    $hex = unpack("H*", $digest);

    ($expect eq $hex) || print "not ";
	print "ok";
}


print "1..7\n";

my $md5 = new MD5;

DoTest("", "d41d8cd98f00b204e9800998ecf8427e");
print " 1\n";
DoTest("a", "0cc175b9c0f1b6a831c399e269772661");
print " 2\n";
DoTest("abc", "900150983cd24fb0d6963f7d28e17f72");
print " 3\n";
DoTest("message digest", "f96b697d7cb7938d525a2f31aaf161d0");
print " 4\n";
DoTest("abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b");
print " 5\n";
DoTest("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
   "d174ab98d277d9f5a5611c2c9f419d9f");
print " 6\n";
DoTest("12345678901234567890123456789012345678901234567890123456789012345678901234567890",
   "57edf4a22be3c955ac49da2e2107b67a");
print " 7\n";

