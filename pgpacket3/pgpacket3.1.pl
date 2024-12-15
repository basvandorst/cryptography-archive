#!/usr/local/bin/perl -s
$dmy=<<"-----BEGIN PGP SIGNED MESSAGE-----"
-----BEGIN PGP SIGNED MESSAGE-----

#!/usr/local/bin/perl -s
    ;				# Deep in the heart of PGP, there is a reason
				# for this semicolon.
# Make sure you use perl 5.000 or higher!  This program won't even
# compile with a lower one.

# PGPACKET
# PGP Packet analyzer, by Mark E. Shoulson (shoulson@cs.columbia.edu)

# (c) Mark E. Shoulson 1995, 1996, 1998
# Version 3.1, August 1998

# Permission is granted to distribute this program freely and make
# modifications or improvements.  Please keep the name of the author on
# modified versions, AND note modifications as the products of those
# who made them.  This program is signed with pgp key 0x429C07E1 and should
# run both before and after being checked with pgp.

# Please let me know if you like this program.

# This program takes any PGP output and prints out a report of each packet
# it comes across (what type, what's in it, etc).  It's not going to crack
# anything or do any real work (like uncompressing or decrypting); it just
# analyzes the structure of the file.

# Version 2.0 (and above) can handle ascii-armored files as well.  It does
# its level best to detect when you have an ascii-armored file (it assumes
# that if the file "looks" like text (perl's -T test), it's ascii-armored),
# and will try to skip past any junk at the beginning of the file to get to
# the real stuff.  I'm not 100% sure of its ability to find the end of the
# pgp stuff, though; I'm pretty sure there's a possibility of failure.
# Just be prepared, until I fix that.  For signed cleartext, it will skip
# the cleartext and analyze only the signature.  Currently it will only
# handle the first chunk of PGP ascii-armor in a file.  You can also force
# ascii-armor mode with the -a option.

# Version 3.0: Handles "new" packets, as used in PGP 5.0 (and above)
# (i.e. the OpenPGP standard)!  It really does seem to work, but hasn't
# been tested extensively, partly because I haven't used PGP 5.0 very much
# and partly because PGP 5.0 itself doesn't use most of the new packets
# defined in OpenPGP.  The program *should* handle nearly anything PGP can
# throw at it, even stuff not yet ever emitted by any version; I was pretty
# thorough with the documentation.  The -t option more or less works, but
# hasn't had testing with all inputs (nothing else has either).  The -u
# option now also works with a 5.0 keyring (defaulting to it [pubring.pkr]
# if it's there, otherwise the pubring.pgp, or whatever's specified).
# Since the new keys use SHA to determine keyIDs, it's a lot slower, so now
# it quits after finding ONE keyID that matches, rather than finding all of
# them.  Bummer.  Maybe I'll put in a flag to disable doing the SHA at all,
# since it's so slow.  Yeah.  It's -nid (no ID).  RSA IDs (more precisely,
# RSA IDs on V3 packets; in current implementations that's the same thing)
# will still be computed, because they're quick, but not ones that need
# SHA.

# Actual data is generally not printed, except for N and E of public keys
# and user-ID information, unless the -v (verbose) option is given.  Other
# information may be parsed out and displayed, but I don't see much reason
# for printing out the encrypted D or encrypted data.  I read all the stuff
# in anyway, often wasting variables and space on it, in case you actually
# want to do anything with it.

# Use the -u option to enable looking up keyIDs in your pubring file (use
# -u=keyfile to specify an alternate file).  Makes it easy to find out who
# else someone has included on the readers-list...

# -h gives a usage note.

# Some symbolic names for CTB's...
# Manifest constants would be better, but who cares?
# CTB types:

$CTB_RES=00;			# Reserved
$CTB_PKE=01;			# Public-Key Encrypted
$CTB_SKE=02;			# Secret-Key Encypted (signature)
$CTB_SYM=3;			# Symmetric-Key Encrypted session key
$CTB_ONE=4;			# One-pass Signature
$CTB_SEC=05;			# Secret Key Certificate
$CTB_PUB=06;			# Public Key Certificate
$CTB_SSUB=7;			# Secret subkey
$CTB_COMP=010;			# Compressed Data
$CTB_CKE=011;			# Conventional-Key Encrypted
$CTB_MARK=10;			# Marker
$CTB_RAW=013;			# Raw literal data, filename and mode
$CTB_TRUST=014;			# Keyring Trust Packet
$CTB_UID=015;			# User ID Packet
$CTB_CMT=016;			# Comment Packet -- Subkey in 5.0
$CTB_PSUB=016;			# Comment Packet -- Subkey in 5.0

%PKALGS=( 
	 1 => "RSA",
	 2 => "RSA (Encrypt only)",
	 3 => "RSA (Sign only)",
	 16 => "Elgamal (Encrypt only)",
	 17 => "DSA",
	 18 => "Elliptic Curve",
	 19 => "ECDSA",
	 20 => "Elgamal",
	 21 => "Diffie-Hellman"
	 );

%SYMALGS=(
	  0 => "None",
	  1 => "IDEA",
	  2 => "Triple-DES",
	  3 => "CAST5",
	  4 => "Blowfish",
	  5 => "SAFER-SK128",
	  6 => "DES/SK"
	  );

%COMPALGS=(
	   0 => "None",
	   1 => "ZIP",
	   2 => "ZLIB"
	   );

%HASHALGS=(
	   1 => "MD5",
	   2 => "SHA-1",
	   3 => "RIPE-MD/160",
	   4 => "Double-width SHA",
	   5 => "MD2",
	   6 => "TIGER/192",
	   7 => "HAVAL-5-160"
	   );

sub ctb {
    my ($c,$d,$ct,$lf,$n,$len,$fh,$len1,$len2);
    # This is the only EOF check in the program, aside from packets
    # which read the rest of the file.  So it won't be terribly
    # clean with files that end in the wrong places.  Cope.
    if (@_) {
	($fh)=@_;
    }
    else {
	$fh="PGP";
    }
    return () if eof($fh);
    # Get the next character, which is the CTB.
    $d = getc($fh);
    $c=unpack('C',$d);
    # bit 7 must be set, or this is no CTB.
    return (-1,$c) unless $c&0200;
    if (($c&0300) == 0200) {	# Old-style packet!!
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
	    read($fh,$n,$lf);
	    $n="\0"x(4-$lf).$n;
	    $len=unpack('N',$n);
	} else {
	    # This isn't really checked for in most of the handlers.  Perhaps
	    # more of them should?
	    $len=0;
	}
	return ($ct,$len,$lf);
    }
    else {			# New-style packet
#	print "New Packet!!!";
#	print $c;
	$ct=$c&0x3f;		# The new type field is 6 bits long.
	read($fh,$n,1);
	$len1=unpack('C',$n);
	if ($len1<=191) {	# One-octet length
	    $len=$len1;
	}
	elsif ($len1<=223) {	# Two-octet length
	    read($fh,$n,1);
	    $len2=unpack('C',$n);
	    $len=(($len1-192)<<8)+$len2+192;
	}
	elsif ($len1<255) {	# partial length
	    $len=1<<($len1 & 0x1f);
	}
	else {			# five-octet length, $len1==255
	    read($fh,$n,4);
	    $len=unpack('N',$n);
	}
	return ($ct,$len,-1);	# Use -1 as the flag that this is New.
				# Not that anyone cares.
    }
}

@months=("Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep",
	 "Oct","Nov","Dec");
sub date_fmt {
    my($time);
    my($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst);
    # If I'm going to make this available, I might as well avoid
    # the month-first/day-first problem.  'Course, by rights I should
    # do something about the English month-names.  Maybe DD MM YYYY with
    # month in roman numerals (e.g. 15 VII 1993)?  If you feel like it.

    ($time)=@_;
    if ($time==0) {
	return "Zero";		# Jan 1 1970 never is right.
    }
    ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=gmtime($time);
    return(sprintf("%d %s %d  %02d:%02d:%02d",
		   $mday,$months[$mon],$year+1900,$hour,$min,$sec));
}

sub data_print {
    my($data,$pack);

    ($data,$pack)=@_;
    $data=unpack('H*',$data) unless $pack;
    printf "Data:\t%s\U$data\E\n", ($pack?"":"0x");
}

sub length_warn {
    my($len,$str);

    ($len)=@_;
    # Warn if length is non-zero.
    return unless $len;		# Check this first, since it's the usual case.
    if ($len>0) {
	$str="long";
    }
    else {
	$str="short";
	$len= -$len;
    }
    print "Warning! Length field is $len bytes too $str!\n" unless $t;
}

sub rmpi {
    # read a PGP format Multi-Precision Integer from a string.
    my($in,$ix,$len,$nlen,$n,$fh);

    ($in,$ix)=@_;
    $len=substr($in,$ix,2); $ix+=2; ;$len=unpack('n',$len);
    $nlen=int($len/8);
    $nlen++ if $len%8;
    $n=substr($in,$ix,$nlen);
    return (unpack('H*',$n),$nlen+2);
}

# *Sigh* I guess I actually DO have to do some crypto in here after all.
# The keyID's of keys stored in version 4 packets (i.e. everyone but RSA
# keys.  RSA keys too, come to think of it, but PGP 5.x doesn't treat them
# that way, and only works with RSA packets in V3) is the last 64 bits of
# the fingerprint, which is computed (for V4 keys) with SHA-1.  So I need
# an SHA subroutine.  I stole this one from Adam Back's web page
# (http://www.dcs.ex.ac.uk/~aba/); it was written by John Allen
# <allen@grumman.com> and perturbed (by me) to make a callable subroutine
# out of it (broke out the formatting too, etc).  Which is why it's so
# unreadable (Adam collects *SMALL* crypto perl scripts, so this was
# written with "small" in mind.  Also inefficient, since the original was
# written to read STDIN and write to stdout, and I modified it as little as
# possible to operate as a subroutine.  Cope.

sub sha {
    local ($sha,@A,@F,$x,$n,$a,$b,$c,$d,$t,@W,$r,$l,$S,$in,$ix,$v);
    ($in)=@_; $ix=0;
    @A=(
	1732584193,
	4023233417,
	2562383102,
	271733878,
	3285377520,
	1518500249,
	1859775393,
	2400959708,
	3395469782);
    @K=splice @A,5,4;
    sub M{
	($x=pop)-($m=1+~0)*int $x/$m;
    }
sub L{
    $n=pop;
    ($x=pop)<<$n|2**$n-1&$x>>32-$n;
}

@F=(
    sub{$b&($c^$d)^$d},
    $S=sub{$b^$c^$d},
    sub{($b|$c)&$d|$b&$c},
    $S);
do{
    $_=substr($in,$ix,64);
    $l+=$r=length($_);
    $ix+=64;
    $r++,$_.="\x80" if $r<64&&!$p++;
    @W=unpack N16,$_."\0"x7;
    $W[15]=$l*8 if$r<57;
    for(16..79) {
	push@W,L$W[$_-3]^$W[$_-8]^$W[$_-14]^$W[$_-16],1;
    }
    ($a,$b,$c,$d,$e)=@A;
    for(0..79) {
	$t=M&{$F[$_/20]}+$e+$W[$_]+$K[$_/20]+L$a,5;
	$e=$d;$d=$c;$c=L$b,30;$b=$a;$a=$t;
    }
    $v='a';
    @A=map{M$_+$ {$v++}}@A;
}while $r>56;
return sprintf'%.8x'x5 ,@A;
}


sub key_id {
    my ($in,$ix,$nn,$tmp);

    ($in)=@_; $ix=0;
    $vb=substr($in,$ix++,1); $vb=unpack('C',$vb);
    if ($vb<4) {		# V3 packet.  RSA.  low 64 bits of N.
	$ix+=4+2+1;		# Skip timestamp, validity, algorithm.
	($nn,$tmp)=&rmpi($in,$ix);
	return uc(substr($nn,-16,16));
    }
    else {
	return '' if $nid;	# No key-id computation requested.
	$nn=length($in); $nn=pack('n',$nn);
	$tmp="\x99$n".$nn.$in;
	return uc(substr(&sha($tmp),-16,16));
    }
}

sub findkey {
    # Find a keyID in the pubring file and print out its UserID.
    # For use with the -u function
    my($targ,$ct,$len,$lf,$dmy,$nn,$uid,$kid);

    unless (stat(PUBRING)) {
	# Silently return if the file handle isn't open for
	# some reason.
	return "";
    }
    ($targ)=@_;
    $targ=uc($targ);
    seek PUBRING,0,0;		# rewind the filehandle
    # OK, do this carefully, since in OpenPGP you can have the userID
    # PRECEDE the key *IF* the key is a subkey.  So.....
    while (($ct,$len,$lf)=&ctb(PUBRING)) {
#	print "Read something from pubkey: ctb=$ct, len=$len\n";
	if ($ct==$CTB_UID) {
#	    print "About to read UID: len=$len\n";
	    read(PUBRING,$uid,$len);
#	    print "UID: \"$uid\"\n";
	    next unless eof(PUBRING);
	}
	elsif ($ct!=$CTB_PUB && $ct!=$CTB_PSUB) {
	    # Eat the packet.
#	    print "About to read non-pubkey packet: ctb=$ct, len=$len\n";
	    read(PUBRING,$dmy,$len);
	    next unless eof(PUBRING);
	}
	# OK, it's a public key packet.  Is it the right one?
	read(PUBRING,$dmy,$len);
      outer:
	$kid=&key_id($dmy);
	next unless $targ eq $kid;
	# It's the right one.  Look for a UID packet.
	# If the packet was a subkey, though, the ALREADY SEEN uid is right.
	# 'Course, since by convention the *main* key is capable of signing,
	# it's unlikely to be a subkey; still, there's rule that it can't be,
	# so I'd better make sure.
	if ($ct==$CTB_PSUB) {
	    print "    User ID for above key: \"$uid\"\n" if $uid;
	    # Once you find it, return.  I had used 'next', to get all the
	    # other UIDs, and I guess you can too, but with the keyIDs
	    # computed by SHA, doing that for EVERY key and subkey in the
	    # keyring got too slow.
	    return; # next;
	}
	while (($ct,$len,$lf)=&ctb(PUBRING)) {
#	    print "Read in inner: ctb=$ct, len=$len\n";
	    if ($ct==$CTB_PUB) {
		# New public key.  No userID, I guess.
		# Start again.  Have to use a goto.
		read(PUBRING,$dmy,$len);
		goto outer unless eof(PUBRING);
	    }
	    unless ($ct==$CTB_UID) {
		# Skip non-uid packets.
		read(PUBRING,$dmy,$len);
		next;
	    }
	    # At long last, found the uid!
	    read(PUBRING,$uid,$len);
	    print "    User ID for above key: \"$uid\"\n";
	    # See above
	    return; # next;
	}
    }
}

sub null_handler {
    # Dummy handler; just skip $len bytes.
    my($in);

    ($in)=@_;
    printf "(No handler known.  Skipping %d bytes)\n",length($in) unless $t;
    &data_print($in) if $v&&!$t;
}

sub comment_handler {
    # Just skip $len bytes.
    # Same as above, minus the message.  Both are included for modularity.
    my($in);

    ($in)=@_;
    printf "%d bytes of comment.\n",length($in);
    print "Comment:\t$in\n" if $v&&!$t;
}

sub uid_handler {
    my($in);

    ($in)=@_;
    printf "User ID:%s\"$in\"\n",($t?" ":"\t");
}

sub sec_head {
    my($in,$ix,$origix,$s2kusage,$ciph,$s2k,$iv);

    ($in,$ix)=@_;
    $origix=$ix;
    $s2kusage=substr($in,$ix++,1); $s2kusage=unpack('C',$s2kusage);
    if ($s2kusage==0) {
	$ciph=0;
    }
    elsif ($s2kusage<255) {
	$ciph=$s2kusage;
    }
    else {			# S2K here...
	# Protection cipher first!
	$ciph=substr($in,$ix++,1); $ciph=unpack('C',$ciph);
	$s2k=&read_s2k($in,$ix); $ix+=length($s2k);
	&print_s2k($s2k) unless $t;
    }
    if ($ciph) {
	$iv=substr($in,$ix,8); $ix+=8;
	print "8 octets of initialization vector" unless $t;
	print " (0x".uc(unpack('H*',$iv)).")" if $v && !$t;
	print ".\n";
    }
    return ($ciph,$ix-$origix);
}
	    

# Read/print an RSA public key.  Broken out for modularity.
# also called for reading secret keys, with nonzero $sec.
sub pub_rsa_handler {
    my($in,$ix,$tmp,$nn,$ee,$sec,$pp,$qq,$uu,$ciph,$chksum,$headlen,$vb);

    ($in,$ix,$sec,$vb)=@_;
    ($nn,$tmp)=&rmpi($in,$ix); $ix+=$tmp;
    ($ee,$tmp)=&rmpi($in,$ix); $ix+=$tmp;
    unless ($t) {
	print "N:\t0x\U$nn\E\n";
	print "E:\t0x\U$ee\E\n";
    }
    if ($sec) {
	($ciph,$headlen)=&sec_head($in,$ix);
	$ix+=$headlen;
	if ($vb<4 || !$ciph) {
	    ($dd,$tmp)=&rmpi($in,$ix); $ix+=$tmp;
	    ($pp,$tmp)=&rmpi($in,$ix); $ix+=$tmp;
	    ($qq,$tmp)=&rmpi($in,$ix); $ix+=$tmp;
	    ($uu,$tmp)=&rmpi($in,$ix); $ix+=$tmp;
	    $chksum=substr($in,$ix,2); $ix+=2;
	    $chksum=unpack('H*',$chksum);
	    unless ($t) {
		print "Protection Algorithm: $ciph ($SYMALGS{$ciph}).\n";
		if ($ciph && !$v) {
		    printf "D: (encrypted: %d bytes)\n",length($dd)/2;
		    printf "P: (encrypted: %d bytes)\n",length($pp)/2;
		    printf "Q: (encrypted: %d bytes)\n",length($qq)/2;
		    printf "U: (encrypted: %d bytes)\n",length($uu)/2;
		}
		else {
		    # If there's no cipher encryption, print out P, Q, D, and U.
		    print "D:\t0x\U$dd\E\n";
		    print "P:\t0x\U$pp\E\n";
		    print "Q:\t0x\U$qq\E\n";
		    print "U:\t0x\U$uu\E\n";
		}
		print "Checksum:\t0x\U$chksum\E\n";
	    }
	}
	else {
	    # How do I know how long the MPI's are if the bitcounts are
	    # encrypted???
	    $dd=substr($dd,$ix,-2);
	    $dd=unpack('H*',$dd);
	    $chksum=substr($in,-2,2); $chksum=unpack('H*',$chksum);
	    if (!$t) {
		if (!$v) {
		    print "D, P, Q, U: (encrypted: $len bytes)\n";
		}
		else {
		    print "D, P, Q, U: \U$dd\E\n";
		}
	    }
	    $ix=length($in);
	}
    }
    &length_warn(length($in)-$ix) if !$t && length($in)-$ix;
    printf "Key ID: 0x%s\n",&key_id($in);
}

# DSA Public key
sub pub_dsa_handler {
    my ($in,$ix,$tmp,$pp,$qq,$gg,$yy,$sec,$xx,$ciph,$headlen,$chksum);

    ($in,$ix,$sec,$vb)=@_;
    ($pp,$tmp)=&rmpi($in,$ix); $ix+=$tmp;
    ($qq,$tmp)=&rmpi($in,$ix); $ix+=$tmp;
    ($gg,$tmp)=&rmpi($in,$ix); $ix+=$tmp;
    ($yy,$tmp)=&rmpi($in,$ix); $ix+=$tmp;
    unless ($t) {
	print "P:\t0x\U$pp\E\n";
	print "Q:\t0x\U$qq\E\n";
	print "G:\t0x\U$gg\E\n";
	print "Y:\t0x\U$yy\E\n";
    }
    if ($sec) {
	($ciph,$headlen)=&sec_head($in,$ix);
	$ix+=$headlen;
	if ($vb<4) {
	    ($xx,$tmp)=&rmpi($in,$ix); $ix+=$tmp;
	}
	else {
	    $xx=substr($in,$ix,-2); $ix=length($in)-2;
	    $xx=unpack('H*',$xx);
	}
	$chksum=substr($in,$ix,2); $ix+=2; $chksum=unpack('H*',$chksum);
	unless ($t) {
	    print "Protection Algorithm: $ciph ($SYMALGS{$ciph}).\n";
	    if ($ciph && !$v) {
		printf "X: (encrypted: %d bytes)\n",length($xx);
	    }
	    else {
		print "X:\t0x\U$xx\E\n";
	    }
	    print "Checksum:\t0x\U$chksum\E\n";
	}
    }
    &length_warn(length($in)-$ix) if !$t && length($in)-$ix;
    printf "Key ID: 0x%s\n",&key_id($in);
}

# Elgamal public key
sub pub_elgamal_handler {
    my ($in,$ix,$tmp,$pp,$gg,$yy, $sec, $xx,$headlen,$ciph,$chksum);

    ($in,$ix,$sec,$vb)=@_;
    ($pp,$tmp)=&rmpi($in,$ix); $ix+=$tmp;
    ($gg,$tmp)=&rmpi($in,$ix); $ix+=$tmp;
    ($yy,$tmp)=&rmpi($in,$ix); $ix+=$tmp;
    unless ($t) {
	print "P:\t0x\U$pp\E\n";
	print "G:\t0x\U$gg\E\n";
	print "Y:\t0x\U$yy\E\n";
    }
    if ($sec) {
	($ciph,$headlen)=&sec_head($in,$ix);
	$ix+=$headlen;
	if ($vb<4) {
	    ($xx,$tmp)=&rmpi($in,$ix); $ix+=$tmp;
	}
	else {
	    $xx=substr($in,$ix,-2); $ix=length($in)-2;
	    $xx=unpack('H*',$xx);
	}
	$chksum=substr($in,$ix,2); $ix+=2; $chksum=unpack('H*',$chksum);
	unless ($t) {
	    print "Protection Algorithm: $ciph ($SYMALGS{$ciph}).\n";
	    if ($ciph && !$v) {
		printf "X: (encrypted: %d bytes)\n",length($xx);
	    }
	    else {
		print "X:\t0x\U$xx\E\n";
	    }
	    print "Checksum:\t0x\U$chksum\E\n";
	}
    }
    &length_warn(length($in)-$ix) if !$t && length($in)-$ix;
    printf "Key ID: 0x%s\n", &key_id($in);
}

# called from sec_handler also, with nonzero $sec
sub pub_handler {
    my($in,$ix,$vb,$ts,$valid,$alg,$ee,$nn, $sec);

    ($in,$ix,$sec)=@_;
    $vb=substr($in,$ix++,1); $vb=unpack('C',$vb);
    $ts=substr($in,$ix,4); $ix+=4; $ts=unpack('N',$ts);
    if ($vb<4) {		# Version 3 packet (old style)
	$valid=substr($in,$ix,2); $ix+=2; $valid=unpack('n',$valid);
	$alg=substr($in,$ix++,1); $alg=unpack('c',$alg);
    }
    elsif ($vb==4) {		# New style packet
	$alg=substr($in,$ix++,1); $alg=unpack('c',$alg);
    }
    else {			# Who knows?
	print "Unknown version!($vb)\n";
	&data_print(substr($in,$ix)) if $v&&!$t;
	return;
    }
    unless ($t) {
	printf ("Version Byte:\t%d\n",$vb);
	print "Key Created:\t".&date_fmt($ts)."\n";
	if ($vb<4) {
	    print "Valid for",($valid > 0? ":\t $valid days" : "ever"),"\n";
	}
	printf("Algorithm:\t%d (%s)\n",$alg,$PKALGS{$alg});
    }
    # RSA.  Nothing else really comes in V3 packets
    if ($alg==1 || $alg==2 || $alg==3) {
	&pub_rsa_handler($in,$ix,$sec,$vb);
    }
    elsif ($alg==17 || $alg==19) { # DSA
	&pub_dsa_handler($in,$ix,$sec,$vb);
    }
    elsif ($alg==20 || $alg==16) { # Elgamal.  Any other options?
	&pub_elgamal_handler($in,$ix,$sec,$vb);
    }
    else {			# Who knows?
	print "Unknown algorithm ($alg).\n";
	&data_print(substr($in,$ix)) if $v&&!$t;
    }
}

sub conv_handler {
    my ($in,$dmy,$len);

    # Yet another "just suck 'em in" handler.
    ($in)=@_;
    printf "%d bytes of data...\n", length($in);
    &data_print($in) if $v&&!$t;
}

sub mark_handler {
    my ($in,$dmy);
    
    $in=pop;
    &data_print(substr($in,0,3),1) if !$t;
    if (!$t && ($in ne 'PGP')) {
	print "Error: Data should be 'PGP'.\n";
    }
    &length_warn(length($in)-3) if length($in)-3;
}

sub comp_handler {
    my ($in,$ix,$dmy,$c);

    ($in)=@_;
    $ix=0;
    $dmy=substr($in,$ix++,1); $dmy=unpack('C',$dmy);
    printf ("Algorithm:\t%d (%s)\n",$dmy,$COMPALGS{$dmy}) unless $t;
    printf "%d bytes of data...\n", length($in)-1;
    &data_print(substr($in,$ix)) if $v&&!$t;
}

sub trust_handler {
    my($in,$byte);
    my($work,$ind);

    ($in)=@_; $byte=$in;
    read(PGP,$byte,$len);
    $ind='';			# Indent set to null.
    # Three different meanings, depending on previous packet.
    # If the previous packet is neither key nor userID nor signature,
    # set $ind (so we can tell later) and remark.
    # If TERSE, just print a newline and get on with life.
    if ($t) {
	print "\n";
	return;
    }
    if ($lastctb!=$CTB_PUB && $lastctb!=$CTB_SEC && $lastctb!=$CTB_UID &&
	$lastctb!=$CTB_SKE && $lastctb!=$CTB_PSUB) {
	# If it's not after a permitted type, then show *all* the
	# possible meanings, in a slightly different format.
	print
	    "Corrupt file! Trust packet should follow key, userID, or sig.\n";
	$ind="\t";
    }
    printf("Trust byte: %s\n",unpack("B8",$byte));
    $byte=unpack('C',$byte);
    if ($ind || $lastctb==$CTB_PUB || $lastctb==$CTB_SEC || 
	$lastctb==$CTB_PSUB) {
	print "Trust information if previous packet is key:\n" if $ind;
	# If preceding packet is a KEY packet:
	print "${ind}Trust of this key owner:\t";
	$work=$byte&07;
	if ($work==00) {
	    print "Undefined\n";
	}
	elsif ($work==01) {
	    print "Unknown\n";
	}
	elsif ($work==02) {
	    print "Usually do not trust to sign\n";
	}
	elsif ($work==03) {
	    print "(reserved 011)\n";
	}
	elsif ($work==04) {
	    print "(reserved 100)\n";
	}
	elsif ($work==05) {
	    print "Usually trust to sign\n";
	}
	elsif ($work==06) {
	    print "Always trust to sign\n";
	}
	else {
	    print "Present on secret ring.\n";
	}
	$work=$byte&040;
	if ($work) {
	    print "${ind}DISABLED\n";
	}
	else {
	    print "${ind}Enabled\n";
	}
	# Other bits reserved
	$work=$byte&0200;
	if ($work) {
	    print "${ind}Ultimate Trust: Present on secret ring.\n";
	}
    }

    # If preceding packet is UID:
    if ($ind || $lastctb==$CTB_UID) {
	print "Trust information if previous packet is user ID:\n" if $ind;
	$work=$byte&03;
	if ($work==00) {
	    print "${ind}Unknown if this ID belongs to associated key\n";
	}
	elsif ($work==01) {
	    print "${ind}ID not trusted to belong to associated key\n";
	}
	elsif ($work==02) {
	    print
	       "${ind}Marginal confidence that ID belongs to associated key\n";
	}
	else {
	    print "${ind}Complete trust that key belongs to associated ID\n";
	}
	# Other bits reserved
	$work=$byte&0200;
	if ($work) {
	    print "${ind}Will only warn if this key is used\n";
	}
	else {
	    print "${ind}Will warn and verify before using this key\n";
	}
    }
    if ($ind || $lastctb==$CTB_SKE) {
	# If previous packet is SIG:
	print "Trust information if previous packet is signature:\n" if $ind;
	print "${ind}Trust of the owner of the signing key:\t";
	$work=$byte&07;
	if ($work==00) {
	    print "Undefined\n";
	}
	elsif ($work==01) {
	    print "Unknown\n";
	}
	elsif ($work==02) {
	    print "Usually do not trust to sign\n";
	}
	elsif ($work==03) {
	    print "(reserved 011)\n";
	}
	elsif ($work==04) {
	    print "(reserved 100)\n";
	}
	elsif ($work==05) {
	    print "Usually trust to sign\n";
	}
	elsif ($work==06) {
	    print "Always trust to sign\n";
	}
	else {
	    print "Present on secret ring.\n";
	}
	# Other bits reserved
	$work=$byte&0100;
	if ($work) {
	    print "${ind}Key checking pass has checked this signature\n";
	}
	else {
	    print "${ind}Key checking pass does not trust this signature\n";
	}
	$work=$byte&0200;
	if ($work) {
	    print
	     "${ind}Contiguous chain of trusted sigs to ultimate one exists\n";
	}
	else {
	    print
	 "${ind}No contiguous chain of trusted sigs to ultimate one exists\n";
	}
    }
}

sub pke_handler {
    my ($in,$ix,$version,$kid,$alg,$dmy,$tmp,$wild);

    ($in)=@_; $ix=0;
    $version=substr($in,$ix++,1);
    $version=unpack('C',$version);
    $kid=substr($in,$ix,8); $ix+=8;
    $kid=unpack('H*',$kid);
    $wild=($kid eq '0000000000000000');	# "wildcard" key.
    $alg=substr($in,$ix++,1);
    printf ("Version:\t%d\n",$version) unless $t;
    if ($wild) {
	print "Key ID:\t*ANY*\n";
    }
    else {
	print "Key ID:\t0x\U$kid\E\n";
    }
    unless ($t) {
	&findkey($kid) if $u && !$wild;
	$alg=unpack('C',$alg);
	printf ("Algorithm:\t%d (%s)\n",$alg,$PKALGS{$alg});
	print length($in)-$ix, " bytes of data.\n";
	# Hrm; no guarantee the remainder of the fields will be
	# MPI's; best not assume it.
	if ($alg==1 || $alg==2) { # RSA
	    # Only one field.
	    ($dmy,$tmp)=&rmpi($in,$ix); $ix+=$tmp;
	    print "Data:\t0x\U$dmy\E\n" if $v;
	}
	elsif ($alg==16 || $alg==20) { # ElGamal
	    # TWO fields!
	    ($dmy,$tmp)=&rmpi($in,$ix); $ix+=$tmp;
	    print "Data(1):\t0x\U$dmy\E\n" if $v;
	    ($dmy,$tmp)=&rmpi($in,$ix); $ix+=$tmp;
	    print "Data(2):\t0x\U$dmy\E\n" if $v;
	}
	else {
	    $dmy=substr($in,$ix);
	    $ix=length($in);
	    print "Unknown algorithm!\n";
	    &data_print($dmy) if $v;
	}
	&length_warn(length($in)-$ix) if length($in)-$ix;
    }
}

%SIGTYPES=(
	   0x0  => "Signature of binary document",
	   0x1  => "Signature of canonical text document",
	   0x2  => "Standalone signature", # New
	   0x10 => "Generic Key certification",
	   # Most of these are unsupported
	   0x11 => "Persona Key certification",
	   0x12 => "Casual ID Key certification",
	   0x13 => "Positive ID Key certification",
	   0x18	=> "Subkey binding signature", # New
	   0x1f	=> "Direct key signature", # New
	   0x20 => "Key Compromise certificate, revoking public key",
	   0x28 => "Subkey revocation certificate", # New
	   0x30 => "Key/UserID Revocation certificate",
	   0x40 => "Timestamp of signature"
	   );

sub ske_handler {
    my ($in,$ix,$vb,$md5len,$class,$ts,$kid,$pkalg,$digalg,$chk,$dmy,$tmp);
    my ($data1, $data2, $subs1, $subs2, $subslen1, $subslen2, $thisone);

    ($in)=@_; $ix=0;
    $vb=substr($in,$ix++,1); $vb=unpack('C',$vb);
    printf("Version:\t%d\n",$vb) unless $t;
    if ($vb<4) {		# old-style packet
	$md5len=substr($in,$ix++,1); $md5len=unpack('C',$md5len);
	printf("Adding %d bytes of header to digest\n",$md5len) unless $t;
	$class=substr($in,$ix++,1); $class=unpack('C',$class);
	$ts=substr($in,$ix,4); $ix+=4; $ts=unpack('N',$ts);
	$kid=substr($in,$ix,8); $ix+=8; $kid=unpack("H*",$kid);
	$pkalg=substr($in,$ix++,1); $pkalg=unpack('C',$pkalg);
	$digalg=substr($in,$ix++,1); $digalg=unpack('C',$digalg);
    }
    elsif ($vb==4) {		# New packet
	$class=substr($in,$ix++,1); $class=unpack('C',$class);
	$pkalg=substr($in,$ix++,1); $pkalg=unpack('C',$pkalg);
	$digalg=substr($in,$ix++,1); $digalg=unpack('C',$digalg);
	$subslen1=substr($in,$ix,2); $ix+=2; $subslen1=unpack('n',$subslen1);
	$subs1=substr($in,$ix,$subslen1); $ix+=$subslen1;
	$subslen2=substr($in,$ix,2); $ix+=2; $subslen2=unpack('n',$subslen2);
	$subs2=substr($in,$ix,$subslen2); $ix+=$subslen2;
#	print "class=$class; pkalg=$pkalg; digalg=$digalg; subslen1=$subslen1; subslen2=$subslen2\n";
    }
    $chk=substr($in,$ix,2); $ix+=2; $chk=unpack('H*',$chk);
    if ($pkalg==1 || $pkalg==3) {	# RSA
	($data1,$tmp)=&rmpi($in,$ix);
	$ix+=$tmp;
    }
    elsif ($pkalg==17) {	# DSA
	($data1,$tmp)=&rmpi($in,$ix);
	$ix+=$tmp;
	($data2,$tmp)=&rmpi($in,$ix);
	$ix+=$tmp;
    }
    if ($SIGTYPES{$class}) {
	print $SIGTYPES{$class};
    }
    else {
	printf("Unknown signature classification (%x)",$class);
    }
    unless ($t) {
	print "\n";
	print "Signature Created:\t".&date_fmt($ts)."\n" if $ts;
	print "Signing Key ID:\t0x\U$kid\E\n" if $kid;
	&findkey($kid) if $u && $kid;
	printf("Public Key Algorithm:\t%d (%s)\n",
	       $pkalg,$PKALGS{$pkalg});
	printf("Message Digest Algorithm:\t%d (%s)\n",
	       $digalg,$HASHALGS{$digalg});
	if ($subslen1) {
	    print "Hashed subpackets: $subslen1 bytes.\n";
	    while ($subslen1>0) {
#		print "subslen1=$subslen1\n";
		$thisone=&sig_subpack($subs1);
		$subslen1-=$thisone;
		$subs1=substr($subs1,$thisone);
	    }
	}
	if ($subslen2) {
	    print "Unhashed subpackets: $subslen2 bytes.\n";
	    while ($subslen2>0) {
		$thisone=&sig_subpack($subs2);
		$subslen2-=$thisone;
		$subs2=substr($subs2,$thisone);
	    }
	}
	print "Check bytes:\t0x\U$chk\E\n";
	print length($data1)/2, " bytes of data (1)\n" if $data1;
	&data_print($data1,1) if $v && $data1;
	print length($data2)/2, " bytes of data (2)\n" if $data2;
	&data_print($data2,1) if $v && $data2;
	&length_warn(length($in)-$ix) if length($in)-$ix;
    }
    else {
	if ($subslen1 || $subslen2) {
	    print "\n";
	    if ($subslen1) {
		while ($subslen1>0) {
		    $thisone=&sig_subpack($subs1);
		    $subslen1-=$thisone;
		    $subs1=substr($subs1,$thisone);
		}
	    }
	    if ($subslen2) {
		while ($subslen2>0) {
		    $thisone=&sig_subpack($subs2);
		    $subslen2-=$thisone;
		    $subs2=substr($subs2,$thisone);
		}
	    }
	}
	elsif ($kid) {	# a keyID can't happen when there are subpacks, right?
	    print " (0x\U$kid\E)\n";
	}
	else {
	    print "\n";
	}
    }
}

%SIGSUBTYPES=(
	      2 => "signature creation time",
	      3 => "signature expiration time",
	      4 => "exportable certification",
	      5 => "trust level",
	      6 => "regular expression",
	      7 => "revocable",
	      9 => "key expiration time",
	      10 => "(unsupported placeholder)",
	      11 => "preferred symmetric algorithms",
	      12 => "revocation key",
	      16 => "issuer key ID",
	      20 => "notation data",
	      21 => "preferred hash algorithms",
	      22 => "preferred compression algorithms",
	      23 => "key server preferences",
	      24 => "preferred key server",
	      25 => "primary user id",
	      26 => "policy URL",
	      27 => "key flags",
	      28 => "signer's user id",
	      29 => "reason for revocation"
	      );

sub sig_subpack {
    my ($len,$len2,$type,$data,$index,$input,$crit);

    ($input)=@_;
    $index=0;
    $len=substr($input,$index++,1);
    $len=unpack('C',$len);
    if ($len>=192 && $len<255) {
	$len2=substr($input,$index++,1);
	$len2=unpack('C',$len2);
	$len=(($len-192)<<8)+len2+192;
    }
    elsif ($len==255) {
	$len=substr($input,$index,4);
	$index+=4;
	$len=unpack('N',$len);
    }
    $type=substr($input,$index++,1);
    $type=unpack('C',$type);
    $crit=($type&0x80);		# Critical?
    $type=($type&0x7f);
    print "  Subpacket: $SIGSUBTYPES{$type},  $len bytes";
    if ($crit) {
	print "  CRITICAL!";
    }
    print "\n" unless $t;
    if ($type==2) { # Time...
	$data=substr($input,$index,4); $index+=4;
	$data=unpack('N',$data);
	print "   Date: ".&date_fmt($data)."\n";;
    }
    elsif ($type==4) {		# Exportable?
	$data=substr($input,$index++,1);
	$data=unpack('C',$data);
	print "   ";
	if ($data) {
	    print "Exportable\n";
	}
	else {
	    print "Unexportable\n";
	}
    }
    elsif ($type==7) {		# Exportable?
	$data=substr($input,$index++,1);
	$data=unpack('C',$data);
	print "   ";
	if ($data) {
	    print "Revocable\n";
	}
	else {
	    print "Unrevocable\n";
	}
    }
    elsif ($type==5) {		# Trust level
	$data=substr($input,$index++,1);
	$data=unpack('C',$data);
	print "   Trusted to introduce to level $data\n";
    }
    elsif ($type==16) {		# Issuing ID
	$data=substr($input,$index,8); $index+=8;
	$data=unpack('H*',$data);
	print "   ID: 0x\U$data\E\n";
    }
    elsif ($type==3 || $type==9) { # Expiration times
	$data=substr($input,$index,4); $index+=4;
	$data=unpack('N',$data);
	print "   Expires: $data seconds after creation.\n";
    }
    elsif ($type==6) {		# Regexp
	$data=substr($input,$index,$len-1); $index+=$len-1;
	print "   Regular expression: $data\n";
    }
    elsif ($type==11) {		# Preferred symmetric algorithms
	$data=substr($input,$index,$len-1); $index+=$len-1;
	if ($t) {
	    print " ";
	    print join " ",map {$_=unpack('C',$_);
				"$_ ($SYMALGS{$_})"} (split //,$data);
	    print "\n";
	}
	else {
	    for (split //,$data) {
		$_=unpack('C',$_);
		print "   Symmetric Algorithm: $_ ($SYMALGS{$_})\n";
	    }
	}
    }
    elsif ($type==21) {		# Preferred hash algorithms
	$data=substr($input,$index,$len-1); $index+=$len-1;
	if ($t) {
	    print " ";
	    print join " ",map {$_=unpack('C',$_);
				"$_ ($HASHALGS{$_})"} (split //,$data);
	    print "\n";
	}
	else {
	    for (split //,$data) {
		$_=unpack('C',$_);
		print "   Hash Algorithm: $_ ($HASHALGS{$_})\n";
	    }
	}
    }
    elsif ($type==22) {		# Preferred compression algorithms
	$data=substr($input,$index,$len-1); $index+=$len-1;
	if ($t) {
	    print " ";
	    print join " ",map {$_=unpack('C',$_);
				"$_ ($COMPALGS{$_})"} (split //,$data);
	    print "\n";
	}
	else {
	    for (split //,$data) {
		$_=unpack('C',$_);
		print "   Compression Algorithm: $_ ($COMPALGS{$_})\n";
	    }
	}
    }
    elsif ($type==12) {		# Revocation key
	my ($class, $algid);
	$class=substr($input,$index++,1);
	$algid=substr($input,$index++,1);
	$data=substr($input,$index,20); $index+=20;
	if (!$class&0x80) {
	    print "   Error: class octet does not have high bit set.\n";
	}
	if ($class&0x40) {
	    print "   Sensitive relationship.";
	}
	print "\n" unless $t;
	$data=unpack('H*',$data);
	print "   Fingerprint of key: 0x\U$data\E\n";
    }
    elsif ($type==20) {		# Notation data
	my($flags,$namelen,$valuelen,$name,$value);
	$flags=substr($input,$index,4); $index+=4;
	$flags=unpack('N',$flags);
	$namelen=substr($input,$index,2); $index+=2;
	$valuelen=substr($input,$index,2); $index+=2;
	$namelen=unpack('n',$namelen);
	$valuelen=unpack('n',$valuelen);
	$name=substr($input,$index,$namelen); $index+=$namelen;
	$name=substr($input,$index,$valuelen); $index+=$valuelen;
	if ($flags&0x80000000) {
	    print "   Human-readable data.\n";
	}
	unless ($t) {
	    print "   Name:\t$name\n";
	    print "   Value:\t'value'\n";
	}
    }
    elsif ($type==23) {		# Key server prefs
	for ($_=0;$_<$len-1;$_++) { # This right?
	    $data=substr($input,$index++,1);
	    if ($_==0) {
		if ($data&0x80) {
		    print "   Do not modify.\n";
		}
		# Nothing else is defined for this octet.
	    }
	    # No other octets have anything defined.
	}
    }
    elsif ($type==24) {		# Preferred key server
	$data=substr($input,$index,$len-1);
	$index+=$len-1;
	print "   Server: $data\n";
    }
    elsif ($type==25) {		# Primary user ID
	$data=substr($input,$index++,1);
	print "   This is primary User ID\n";
    }
    elsif ($type==26) {		# Policy URL
	$data=substr($input,$index,$len-1);
	$index+=$len-1;
	print "   Policy URL: $data\n";
    }
    elsif ($type==27) {		# Key flags
	for ($_=0;$_<$len-1;$_++) {
	    $data=substr($input,$index++,1);
	    if ($_==0) {
		# First octet...
		if ($data&0x01) {
		    print "   Key may be used to certify keys.\n";
		}
		if ($data&0x02) {
		    print "   Key may be used to sign.\n";
		}
		if ($data&0x04) {
		    print "   Key may be used to encrypt communications.\n";
		}
		if ($data&0x08) {
		    print "   Key may be used to encrypt storage.\n";
		}
		if ($data&0x10) {
		    print "   Private key may have been split.\n";
		}
		if ($data&0x80) {
		    print "   Private key may be used by many people.\n";
		}
		# No more flags defined for this octet
	    }
	    # No more flags defined in any other octets.
	}
    }
    elsif ($type==28) {		# Signer's user ID
	$data=substr($input,$index,$len-1);
	$index+=$len-1;
	# Note: Spec doesn't specify the content/format of this subpacket!
	# I assume it's just a string.
	print "   Signer's UID: $data\n";
    }
    elsif ($type==29) {		# Reason for revocation
	my ($reason);
	$reason=substr($input,$index++,1);
	$data=substr($input,$index,$len-1);
	$index+=$len-1;
	if ($reason==0x00) {
	    print "   No reason given.\n";
	}
	elsif ($reason==0x01) {
	    print "   Key is superceded.\n";
	}
	elsif ($reason==0x02) {
	    print "   Key has been compromised.\n";
	}
	elsif ($reason==0x03) {
	    print "   Key no longer used.\n";
	}
	elsif ($reason==0x20) {
	    print "   User ID no longer valid.\n";
	}
	else {
	    print "   Unknown reason ($reason).\n";
	}
	print "   Reason string: $data\n" unless $t;
    }
    # Other types...
    else {
	# Just print for now, since we don't know.
	print "  Unhandled signature subpacket ($type).\n";
	$data=substr($input,$index,$len-1);	# Subtract one for type octet.
	$index+=$len-1;
	data_print($data) if $v;
    }
    return $index;
}

sub read_s2k {
    my ($in,$ix,$rv,$first);

    ($in,$ix)=@_;
    $first=unpack('C',substr($in,$ix++,1));
    if ($first==0x00) {
	$rv=substr($in,$ix++,1);
    }
    elsif ($first==0x01) {
	$rv=substr($in,$ix,9); $ix+=9;
    }
    elsif ($first==0x03) {
	$rv=substr($in,$ix,10); $ix+=10;
    }
    return pack('C',$first).$rv;
}

sub print_s2k {
    my ($s2k,$flag,$hash,$salt,$count,$rest);

    ($s2k)=@_;
    ($flag,$hash,$rest)=unpack('C C a*',$s2k);
    print "String-to-Key: ";
    if ($flag==0x00) {
	print "Simple. hash algorithm: $hash ($HASHALGS{$hash})\n";
    }
    elsif ($flag==0x01) {
	$salt=unpack('H*',$rest);
	print "Salted. hash algorithm: $hash ($HASHALGS{$hash})";
	print "  Salt: 0x\U$salt\E" if $v;
	print "\n";
    }
    elsif ($flag==0x03) {
	($salt,$count)=unpack('H8 C',$rest);
	print "Iterated and Salted.  hash algorithm: $hash ($HASHALGS{$hash})";
	$count=(16+($count&15))<<(($count>>4)+6);
	print "  Salt: 0x$salt, count: $count" if $v;
	print "\n";
    }
    else {
	$rest=unpack('H*',($hash,$rest));
	print "Unknown S2K ($flag):\n  Data: \U$rest\E\n";
    }
}

sub skesk_handler {
    my ($in,$ix,$vb,$alg,$s2k);

    ($in)=@_; $ix=0;
    $vb=substr($in,$ix++,1); $vb=unpack('C',$vb);
    $alg=substr($in,$ix++,1); $alg=unpack('C',$alg);
    $s2k=&read_s2k($in,$ix); $ix+=length($s2k);
    print "Version: $vb\n";
    print "Symmetric Algorithm: $SYMALGS{$alg}\n";
    &print_s2k($s2k);
    if (length($in)-$ix) {
	data_print(substr($in,$ix)) if $v&&!$t;
    }
}

sub one_pass_handler {
    my ($in, $ix, $vb, $sigtype, $hashalg, $pkalg, $kid, $nested);

    ($in)=@_; $ix=0;
    $vb=substr($in,$ix++,1); $vb=unpack('C',$vb);
    $sigtype=substr($in,$ix++,1); $sigtype=unpack('C',$sigtype);
    $hashalg=substr($in,$ix++,1); $hashalg=unpack('C',$hashalg);
    $pkalg=substr($in,$ix++,1); $pkalg=unpack('C',$pkalg);
    $kid=substr($in,$ix,8); $ix+=8; $kid=unpack('H*',$kid);
    $nested=substr($in,$ix++,1);
    print "Version: $vb\n";
    print "Signature type: $sigtype ($SIGTYPES{$sigtype})\n";
    print "Hash algorithm: $hashalg ($HASHALGS{$hashalg})\n";
    print "Public Key algorithm: $pkalg ($PKALGS{$pkalg})\n";
    if ($nested) {
	print "Nested; next packet also applies to same data.\n";
    }
    else {
	print "Not nested.\n";
    }
    &length_warn(length($in)-$ix) if length($in)-$ix;
}

sub sec_handler {
    &pub_handler(@_,0,1);
}

sub raw_handler {
    my($in,$ix,$mode,$namelen,$name,$ts,$data);

    ($in)=@_; $ix=0;
    $mode=substr($in,$ix++,1);
    $namelen=substr($in,$ix++,1); $namelen=unpack('C',$namelen);
    $name=substr($in,$ix,$namelen); $ix+=$namelen;
    $ts=substr($in,$ix,4); $ix+=4; $ts=unpack('N',$ts);
    # There's also 'l'ocal mode, now documented
    printf("Mode:\t%s (%s)\n",$mode,
	   ('b' eq $mode ? "binary" : 't' eq $mode ? "canonical text" :
				       'l' eq $mode ? "local" : "unknown"))
	unless $t;
    print "Length of filename:\t$namelen\n" unless $t;
    if ($name eq '_CONSOLE') {
	print "For eyes only!\n";
    }
    else {
	print "Filename:\t$name\n";
    }
    unless ($t) {
	if ($ts) {
	    print "Timestamp:\t".&date_fmt($ts)."\n";
	}
	else {
	    # Timestamp can be (and often is) zero, which may mean
	    # whatever, but certainly doesn't mean it was created
	    # 1 Jan 1970.  Better just to say it's zero.
	    print "Timestamp:\tZero\n";
	}
    }
    printf "%d bytes of data.\n", length($in)-$ix unless $t;
    &data_print(substr($in,$ix)) if $v&&!$t;
}

sub process_packet {
    my($ct,$lf,$len,$typestr,$handler,$pack);

    return 0 unless (($ct,$len,$lf)=&ctb());
    if ($ct<0) {
	# Error!  CTB with bit 7 not set found.
	print "\nINVALID CTB ($len)!  Bit 7 not set.\n";
	exit(10);
    }
    if ($ct==$CTB_PKE) {
	$typestr="Public-Key Encrypted Packet";
	$handler=\&pke_handler;
    }
    elsif ($ct==$CTB_SKE) {
	$typestr="Secret-Key Encrypted Packet (signature)";
	$handler=\&ske_handler;
    }
    elsif ($ct==$CTB_SYM) {
	$typestr="Symmetric-Key Encrypted Session Key Packet";
	$handler=\&skesk_handler;
    }
    elsif ($ct==$CTB_ONE) {
	$typestr="One-pass Signature Packet";
	$handler=\&one_pass_handler;
    }
    elsif ($ct==$CTB_SEC) {
	$typestr="Secret Key Packet";
	$handler=\&sec_handler;
    }
    elsif ($ct==$CTB_SSUB) {
	$typestr="Secret SubKey Packet";
	$handler=\&sec_handler;
    }
    elsif ($ct==$CTB_PUB) {
	$typestr="Public Key Packet";
	$handler=\&pub_handler;
    }
    elsif ($ct==$CTB_PSUB) {
	$typestr="Public SubKey Packet"; # Same format as public key packets
	$handler=\&pub_handler;
    }
    elsif ($ct==$CTB_COMP) {
	$typestr="Compressed Data Packet";
	$handler=\&comp_handler;
    }
    elsif ($ct==$CTB_CKE) {
	$typestr="Conventionally Encrypted Packet";
	$handler=\&conv_handler;
    }
    elsif ($ct==$CTB_MARK) {
	$typestr="Marker Packet";
	$handler=\&mark_handler;
    }
    elsif ($ct==$CTB_RAW) {
	$typestr="Raw Data Packet";
	$handler=\&raw_handler;
    }
    elsif ($ct==$CTB_TRUST) {
	$typestr="Keyring Trust Packet";
	$handler=\&trust_handler;
    }
    elsif ($ct==$CTB_UID) {
	$typestr="User ID Packet";
	$handler=\&uid_handler;
    }
    # Comment packet CTB now used for subkeys.
#     elsif ($ct==$CTB_CMT) {
# 	$typestr="Comment Packet";
# 	$handler=\&comment_handler;
#     }
    else {
	$typestr=sprintf("UNKNOWN PACKET!! (%o)",$ct);
	$handler=\&null_handler;
    }
    print "\n---------------------------\nPacket Type:\t" unless $t;
    print "$typestr",($t?": ":"\n");
    print "Length:\t$len\n" unless $t;
    if ($len) {
	read(PGP,$pack,$len);
    }
    else {			# Zero length => read to EOF
	$pack="";
	while (!eof(PGP)) {
	    $pack.=getc(PGP);
	}
    }
    &$handler($pack);
    $lastctb=$ct;
    return(1);
}

# MAIN PROGRAM

die "Usage: $0 [-v] [-a] [-v] [-u[=pubfile]] [pgpfile], stopped"
    if $#ARGV>1;		# -v for verbose, -a to force ascii mode.
# Check an option or two...
if ($h) {
    # print help message and exit
    print<<"EEOOFF"
PGPACKET: analyze the output of PGP.

Usage: $0 [options] [pgpfile]

If no file is specified, reads stdin.

Options:
\t-v: Verbose.  Print out more information.
\t-t: Terse.  Print out very little information.  Overrides -v and -u.
\t-a: ASCII mode.  Presume ASCII mode, even if first char is 8-bit.
\t-nid: No IDs.  Don't compute key-IDs that require hashing.
\t-u[=pubring]: Look up keyIDs in pubring file.
\t\tIf pubring is not specified, defaults to ~/.pgp/pubring.\{pgp\|pkr\}.

EEOOFF
    ;
    exit(1);
}
if ($u) {
    # User-ID lookup enabled.  Open the pubring.
    if ($u==1) {
	# No value specified.  Default to $HOME/.pgp/pubring.pgp
	# I realize that this makes it tough to specify a keyring file
	# named "1".
	if (-r $ENV{'HOME'}.'/.pgp/pubring.pkr') {
	    $u=$ENV{'HOME'}.'/.pgp/pubring.pkr';
	}
	else {
	    $u=$ENV{'HOME'}.'/.pgp/pubring.pgp';
	}
    }
    unless (open(PUBRING,$u)) {
	print "Warning: $u could not be opened.  User-ID lookup disabled.\n";
	$u=0;
    }
}
if (@ARGV) {
    die "Couldn't open $ARGV[0]" unless open(PGP,$ARGV[0]);
}
else {
    open(PGP,"<&STDIN");
}
# Test for binaryness.
if (-T PGP || $a) {
    # If it's a text file, assume this is ascii-armored and dearmor it.
    # To dearmor, try forking and running a dearmor script in the child,
    # piping the output back to the parent.
    # Eerie.  Apparently once I've read from a file-descriptor, reads
    # from its duplicates return nulls all the time.  I'm not sure if
    # that's a bug or a feature; I'm not really adept at the mysteries
    # of file descriptors.  I'll code around it, I guess.  Assuming I
    # can figure out HOW.  Ah, got it.
    # Have to read in the first line here.  Otherwise, the -T test eats
    # the first line in the child, so if you're starting right at the
    # -----BEGIN line it won't find it.
    $_=<PGP>;
    $^F=fileno(PGP);		# Need in the child.
    if (!open(PGP2,"-|")) {
	# In the child...
	# Search for and eat frame.
	# Skip past the header.  Note special-case, to ignore the top of a
	# signed plain-text document; skip down to the real sig.
	until (!$_ ||
	       (/^-----BEGIN PGP / &&! /^-----BEGIN PGP SIGNED MESSAGE/)) {
	    $_=<PGP>;
	}
	$_=<PGP> until !$_ || /^\s*\n?$/;
	while (1) {
	    $last=0;
	    $_=<PGP>;
	    # equals signs are used to pad out the end.
	    s/^([^-=]*)[-=].*/\1/;
	    # Only last chunk may be less than 64chars.  This may not always
	    # be able to exclude the checksum.  Oh well.
	    $last=1 if length($_)<64;
	    tr@A-Za-z0-9+/@`!-_@d; # matching `
	    $_=pack('C',length($_)*3/4+040).$_;
	    $_=unpack("u",$_);
	    $_=~s/\0$// if $last;	# Strip *one* trailing null
	    print $_;
	    last if $last;
	}
	# And finish!  The child needs to exit here.
	exit(0);
    }
    close PGP;
    open (PGP,"<&PGP2");
}
# Otherwise, just continue.
$lastctb=0;			# Last seen ctb;
while (1) {
    last unless &process_packet;
}
exit(0);

$dmy=<<"-END"
- -END
    ;

-----BEGIN PGP SIGNATURE-----
Version: 2.6.2

iQCVAwUBNeTW+djId3FCnAfhAQHYpQP9H/YkHBKGqc7LEqXnuT5/MDJ+d9LBKfaz
U48501Qfd5Hcl2NBuS5Fc1s+pmGWu+ZezMEyjkzWzfxeA1ddg11eAoM01NIonRDq
9GiJudJ2QAyehCmVPRJPllU8ACb2UnPaZ/5QvuzMdZxlzhVGNnrt6+hpwD0eYAj1
dbbv4NSJkPM=
=1+Ng
-----END PGP SIGNATURE-----
-END
;
