#!/usr/local/bin/perl
#
# arcfour - By Steve Reid <steve@edmweb.com>
#
# File encryption program. Uses the RC4 stream cipher with a 160-bit key.
# Keys are generated from a SHA-1 hash of a passphrase and salt.
#

# Parse command line options
do {
    $options .= shift;
} while $ARGV[0] =~ /^-/;
if ($options !~ /^[\-edsrkp]*$/ || $options !~ /[eds]/) {
    &help;
    exit;
}
if ($options !~ /p/) {
    system("/bin/stty -echo");  # Turn keypress echo OFF
}
print STDERR "Enter passphrase: ";  # STDOUT may be redirected; use STDERR
chop($pass = <STDIN>);
print STDERR "\n";
$passhash = sha1($pass);
if ($options =~ /e/) {  # Encrypting, gotta verify passphrase
    print STDERR "Enter same passphrase again: ";
    chop($i = <STDIN>);
    print STDERR "\n";
}
if ($options !~ /p/) {
    system("/bin/stty echo");  # Turn keypress echo back ON
}
if ($options =~ /e/ && $pass ne $i) {
    print STDERR "Passphrases were different. Try again.\007\n";
    exit;
}
foreach $filename (@ARGV) {  # Main loop. Go through all the files.
    # Open input file
    if (!open(INFILE, $filename)) {
        print STDERR "Unable to open input file: $filename\n";
        next;
    }
    # Open output file
    if ($options !~ /s/ && !open(OUTFILE, '>'.$filename.'.TMP')) {
        print STDERR "Unable to open output file.\n";
        close(INFILE);
        next;
    }
    # Now get key
    # When encrypting, generate "random" key from hash of salt
    # (filename, time, pid) and passphrase. The passphrase is the ONLY
    # source of entropy that we have. The salt ensures that the key
    # will be different every time.
    # The key is stored as the first 20 bytes of the encrypted file,
    # xor'ed with the hash of the passphrase. If the key is sufficiently
    # random (from an attacker's point of view; he doesn't know $pass)
    # then simple xor shouldn't leak any information.
    if ($options =~ /e/) {  # Encrypting
        $key = sha1($filename.pack('N2', time, $$).$pass);  # Random key
        for (0..19) {
            $keyarray[$_] = ((unpack('C20', $key))[$_]) ^ 
             ((unpack('C20', $passhash))[$_]);
        }
        print OUTFILE pack('C20', @keyarray);  # Put the xor'ed key in the file
        &keyinit($key);
    }
    else {  # Decrypting
        read(INFILE, $key, 20);  # Read xor'ed key from file
        for (0..19) {
            $keyarray[$_] = ((unpack('C20', $key))[$_]) ^
             ((unpack('C20', $passhash))[$_]);
        }
        $key = pack('C20', @keyarray);
        &keyinit($key);
    }
    # Now we discard the first 1024 bytes of the cipher's stream. This
    # will scramble the sbox some more and should help to defend against
    # the weak key attack. It also makes brute-force attacks against
    # the cipher more difficult. It certainly doesn't hurt.
    for (0..1023) {
        $var1 = ($var1 + 1) & 255;
        $var2 = ($var2 + $sbox[$var1]) & 255;
        ($sbox[$var1], $sbox[$var2]) = ($sbox[$var2], $sbox[$var1]);
    }
    # If we aren't sending to STDOUT, select OUTFILE
    if ($options !~ /s/) {
        select OUTFILE;
    }
    # Now we actually (en|de)crypt the file.
    while (!eof(INFILE)) {
        $var1 = ($var1 + 1) & 255;
        $var2 = ($var2 + $sbox[$var1]) & 255;
        ($sbox[$var1], $sbox[$var2]) = ($sbox[$var2], $sbox[$var1]); 
        print pack('C', unpack('C', getc(INFILE)) ^ 
         $sbox[($sbox[$var1] + $sbox[$var2]) & 255]);
    }
    select STDOUT;
    close(OUTFILE);
    close(INFILE);
    if ($options =~ /[er]/ && $options !~ /k/) {
        system('/bin/rm -P '.$filename);  # Remove old file
        rename($filename.'.TMP', $filename);  # All done
    }
    elsif ($options !~ /s/) {  # If output to STDOUT, don't touch files
        rename($filename, $filename.'.BAK');  # Save file as .BAK
        rename($filename.'.TMP', $filename);  # All done
    }
}
exit;


# Sub keyinit takes a key and initializes @sbox, resets $var1 & $var2
sub keyinit
{
local (@key, $i);
    @key = unpack('C20', $_[0]);  # Turn string into an array of chars
    $var1 = $var2 = $i = 0;
    for (0..255) {
        $sbox[$_] = $_;
    }
    for (0..255) {
        $i = ($i + $sbox[$_] + $key[$_ % length($_[0])]) & 255;
        ($sbox[$_], $sbox[$i]) = ($sbox[$i], $sbox[$_]);
    }
}


# Sub sha1 returns the SHA-1 hash of the arguments. See FIPS PUB 180-1.
# This is based on the eight line version by John Allen <allen@grumman.com>
# I've re-optimized it more for speed than size (obviously).
sub sha1
{
local ($a, $b, $c, $d, $e, $i, $input, $l, $p, $r, $t, @A, @W);
    $input = join ('', @_); # All arguments are concatenated together
    # Initial accumulator values
    ($a,$b,$c,$d,$e)=@A=(0x67452301,0xEFCDAB89,0x98BADCFE,0x10325476,0xC3D2E1F0);
    do {
        $_ = substr($input, 0, 64);  # Grab the first 64 bytes (512 bits)
        $input = substr($input, 64);  # $input is whatever is left
        $l+=($r=length);  # Keep track of volume of data
        # To use this code on a file, replace previous 3 lines with this:
        # $l+=($r=read FILEHANDLE, $_, 64);
        $r++, $_.="\x80" if ($r<64 && !$p++);  # handle padding, but once only
        @W = unpack("N16", $_."\0"x7);  # unpack block into array of 16 ints
        $W[15] = $l*8 if ($r<57);  # bit length of file in final block
        # extend @W to 80 ints by munging prior values together
        for(16..79) {
            $i = $W[$_-3]^$W[$_-8]^$W[$_-14]^$W[$_-16];
            push(@W, ($i<<1)|($i>=0x80000000));
        }
        # the meat of SHA is 80 iterations applied to the working vars
        for(0..19) {
            $t = (($b&($c^$d))^$d)+$e+$W[$_]+0x5A827999+
             (($a<<5)|(($a>>27)&31))&0xFFFFFFFF;
            $e = $d; $d = $c; 
            $c = (($b<<30)|(($b>>2)&0x3FFFFFFF)); 
            $b = $a; $a = $t;
        }
        for(20..39) {
            $t = ($b^$c^$d)+$e+$W[$_]+0x6ED9EBA1+
             (($a<<5)|(($a>>27)&31))&0xFFFFFFFF;
            $e = $d; $d = $c; 
            $c = (($b<<30)|(($b>>2)&0x3FFFFFFF)); 
            $b = $a; $a = $t;
        }
        for(40..59) {
            $t = ((($b|$c)&$d)|($b&$c))+$e+$W[$_]+0x8F1BBCDC+
             (($a<<5)|(($a>>27)&31))&0xFFFFFFFF;
            $e = $d; $d = $c; 
            $c = (($b<<30)|(($b>>2)&0x3FFFFFFF)); 
            $b = $a; $a = $t;
        }
        for(60..79) {
            $t = ($b^$c^$d)+$e+$W[$_]+0xCA62C1D6+
             (($a<<5)|(($a>>27)&31))&0xFFFFFFFF;
            $e = $d; $d = $c; 
            $c = (($b<<30)|(($b>>2)&0x3FFFFFFF)); 
            $b = $a; $a = $t;
        }
        # add in the working vars to the accumulators, modulo 2**32
        $a=$A[0] = ($A[0]+$a)&0xFFFFFFFF;
        $b=$A[1] = ($A[1]+$b)&0xFFFFFFFF;
        $c=$A[2] = ($A[2]+$c)&0xFFFFFFFF;
        $d=$A[3] = ($A[3]+$d)&0xFFFFFFFF;
        $e=$A[4] = ($A[4]+$e)&0xFFFFFFFF;
    } while $r>56;
    return pack('N5', @A);
}


sub help
{
print <<'-END-';
arcfour - File encryption program by Steve Reid <steve@edmweb.com>
Implements RSA DSI's RC4 stream cipher and the SHA-1 one-way hash.

Usage:
 arcfour [options] filename(s)

You must specify exactly _one_ of the following options:
 -e    Encrypt file(s). Passphrase is used to encrypt a random key, and
        the random key is used to encrypt the file.
 -d    Decrypt file(s). Passphrase is used to decrypt the random key.
 -s    Decrypt file(s) to standard output. Useful when decrypting text.

Optional options:
 -k    Keep input file when done. This is the default when decrypting.
 -r    Remove input file when done. Uses '/bin/rm -P file' to delete and 
        overwrite. This is the default when encrypting.
 -p    Enable echoing of the passphrase. Doesn't run "/bin/stty -echo".

WARNING: Memory is not properly wiped after use.

-END-
}
