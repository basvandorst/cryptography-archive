BEGIN { push @INC, qw(. .. ../lib ../../lib ../../../lib) }

use strict;

use Crypt::MD5;
use Crypt::SHA;
use Crypt::Blowfish;
use Crypt::DES;
use Crypt::DES3EDE;
use Crypt::IDEA;
use Crypt::CBC;
use Crypt::CFB;


print "1..14\n";

my $md5_hash = Crypt::MD5->hash("ABC");
my $sha_hash = Crypt::SHA->hash("ABC");

print "1 ";
("$md5_hash" eq "MD5:902fbdd2b1df0c4f70b4a5d23525e932") || print "not ";
print "ok\n";

print "2 ";
("$sha_hash" eq "SHA:e6376bfcf102eab637c32689d6ae46e46437248d") || print "not ";
print "ok\n";

my $md = new Crypt::MD5;
$md->reset();
$md->add("ABC");

print "3 ";
($md5_hash == $md->digestAsHash()) || print "not ";
print "ok\n";


$md = new Crypt::SHA;
$md->reset();
$md->add("ABC");

print "4 ";
($sha_hash == $md->digestAsHash()) || print "not ";
print "ok\n";


my $init = pack("H*", "1234567812345678");
my $key = pack("H*", "1234567812345678");
my $msg = "the quick brown fox jumps over the lazy dog";

my $block_cipher = new Crypt::DES $key;
my $cipher = new Crypt::CBC $block_cipher;

my $ciphertext = $cipher->encrypt($init . $msg);

print "5 ";
($ciphertext eq pack("H*", "738189139e0b66e1f2f6a9f38b642c784b65460bbd028522e601c075b849d80c507b1579382b3b5a082cc2e34d3427f8f4283f3085270305")) || print "not ";
print "ok\n";

print "6 ";
my $plaintext = $cipher->decrypt($ciphertext);
$plaintext = substr($plaintext, length($init), length($msg));	# Remove IV and trailer
($plaintext eq "the quick brown fox jumps over the lazy dog") || print "not ";
print "ok\n";

$block_cipher = new Crypt::DES $key;
$cipher = new Crypt::CFB $block_cipher;

$ciphertext = $cipher->encrypt($init . $msg);

print "7 ";
($ciphertext eq pack("H*", "7d859ec74f2fecf395ccc524911df6996c7985ce844af22307a855158e654e9b5eafa70c6c6e47ffabc574482222f5bfe39964")) || print "not ";
print "ok\n";

print "8 ";
$plaintext = $cipher->decrypt($ciphertext);
substr($plaintext, 0, length($init)) = '';	# Remove IV
($plaintext eq "the quick brown fox jumps over the lazy dog") || print "not ";
print "ok\n";


my $key = pack("H*", "12345678123456781234567812345678");
my $block_cipher = new Crypt::IDEA $key;
my $cipher = new Crypt::CBC $block_cipher;

my $ciphertext = $cipher->encrypt($init . $msg);

print "9 ";
($ciphertext eq pack("H*", "1dd4f67ff04a9a2068b993fdd6781bee5d848160f905fdceb5cd67921a4457845417d4d0ff68ae6399770358887974b0467c834b5b2d8aaf")) || print "not ";
print "ok\n";

print "10 ";
my $plaintext = $cipher->decrypt($ciphertext);
$plaintext = substr($plaintext, length($init), length($msg));	# Remove IV and trailer
($plaintext eq "the quick brown fox jumps over the lazy dog") || print "not ";
print "ok\n";

$block_cipher = new Crypt::IDEA $key;
$cipher = new Crypt::CFB $block_cipher;

$ciphertext = $cipher->encrypt($init . $msg);

print "11 ";
($ciphertext eq pack("H*", "4fe7f788e9e98b22b3dc9a7ffa37e416192d9d166b3f1a6316c6e03a4fba781dda9b2146c7b804158b2c2197b3a78bc7a4f1dd")) || print "not ";
print "ok\n";

print "12 ";
$plaintext = $cipher->decrypt($ciphertext);
substr($plaintext, 0, length($init)) = '';	# Remove IV
($plaintext eq "the quick brown fox jumps over the lazy dog") || print "not ";
print "ok\n";


$block_cipher = new Crypt::Blowfish $key;
$cipher = new Crypt::CFB $block_cipher;

$ciphertext = $cipher->encrypt($init . $msg);

print "13 ";
($ciphertext eq pack("H*", "59c2044d75682701f5156c38a6b36d289bb864f60348a94a096b3f056bb1a170f6ea27168d4ffdfb81877f79f4a7172400a71c")) || print "not ";
print "ok\n";

print "14 ";
$plaintext = $cipher->decrypt($ciphertext);
substr($plaintext, 0, length($init)) = '';	# Remove IV
($plaintext eq "the quick brown fox jumps over the lazy dog") || print "not ";
print "ok\n";


my $key = pack("H*", "123456781234567812345678123456781234567812345678");
$block_cipher = new Crypt::DES3EDE $key;
$cipher = new Crypt::CFB $block_cipher;

$ciphertext = $cipher->encrypt($init . $msg);

print "15 ";
($ciphertext eq pack("H*", "7d859ec74f2fecf395ccc524911df6996c7985ce844af22307a855158e654e9b5eafa70c6c6e47ffabc574482222f5bfe39964")) || print "not ";
print "ok\n";

print "16 ";
$plaintext = $cipher->decrypt($ciphertext);
substr($plaintext, 0, length($init)) = '';	# Remove IV
($plaintext eq "the quick brown fox jumps over the lazy dog") || print "not ";
print "ok\n";

