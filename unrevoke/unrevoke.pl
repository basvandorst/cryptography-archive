#!/usr/local/bin/perl
# Make sure you use perl 5.000 or higher!  This program won't even
# compile with a lower one.

# PGP key unrevoker, by Mark E. Shoulson (shoulson@cs.columbia.edu).
# Little or no error-checking at this point.  Feed it a public
# keyring as stdin, and it will spit out onto stdout the same keyring
# without all the key revocation (and subkey revocation) certificates.

# NOTE: I suppose it goes without saying that this program is not intended
# to misrepresent someone's key (fraudulently) as valid when in fact it
# was revoked.  Rather, it is to be used responsibly, presumably by 
# key-owners, to regain use of keys that were revoked for whatever reason.
# Besides, revocation certificates are stronger than valid keys, so unless
# you can stop the propogation of ALL the revocation certificates out there,
# the key will still wind up being revoked in most people's keyrings.

# Some symbolic names for CTB's...
# Manifest constants would be better, but who cares?
# CTB types:

$CTB_PKE=01;                    # Public-Key Encrypted
$CTB_SKE=02;                    # Secret-Key Encypted (signature)
$CTB_SEC=05;                    # Secret Key Certificate
$CTB_PUB=06;                    # Public Key Certificate
$CTB_COMP=010;                  # Compressed Data
$CTB_CKE=011;                   # Conventional-Key Encrypted
$CTB_RAW=013;                   # Raw literal data, filename and mode
$CTB_TRUST=014;                 # Keyring Trust Packet
$CTB_UID=015;                   # User ID Packet
$CTB_CMT=016;                   # Comment Packet

sub ctb {
    # Stolen from Adam Back's extract-pub, slightly modified for my 
    # own stylistic preferences.

    my ($c,$d,$ct,$lf,$n,$len,$whole,$rest);
    return () if eof(STDIN);
    $d = getc(STDIN);
    $whole=$d;
    $c=unpack('C',$d);
    # bit 7 must be set, or this is no CTB.
    return (-1,$c) unless $c&0300;
    $ct=$c&074;
    $ct>>=2;
    $lf=$c&03;
    if ($lf==0){
        $lf=1;
    } elsif ($lf==1){
        $lf=2;
    } elsif ($lf==2){
        $lf=4;
    } elsif ($lf==3){
        $lf=0;
    }
    if ($lf){
        read(STDIN,$n,$lf);
	$whole.=$n;
        $n="\0"x(4-$lf).$n;
        $len=unpack('N',$n);
        read(STDIN,$rest,$len);
        $whole.=$rest;
    } else {
        $len=0;
    }
    return ($ct,$len,$rest,$whole,$lf);
}

$revlast=0;
while (($ct,$len,$pack,$whole,$lf)=&ctb) {
    # Skip if it's a revocation, or if it's a trust packet and the previous
    # packet was a revocation.
    # Probably shouldn't have trust packets after revocations, but it's
    # cheap to check.
    if ($ct==$CTB_SKE) {	# Don't bother unless this is sig packet
	$vb=unpack('C',substr($pack,0,1));
	if ($vb<4) {		# Old packet
	    $type=unpack('C',substr($pack,2,1));
	}
	else {
	    $type=unpack('C',substr($pack,1,1));
	}
	if ($type==0x20 || $type==0x28) {
	    $revlast=1;
	    next;
	}
    }
    if ($ct==$CTB_TRUST && $revlast) {
        $revlast=0;
        next;
    }
    $revlast=0;
    print $whole;
}
