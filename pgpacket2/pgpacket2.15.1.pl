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

# (c) Mark E. Shoulson 1995, 1996
# Version 2.15.1, April 1996

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

$CTB_PKE=01;			# Public-Key Encrypted
$CTB_SKE=02;			# Secret-Key Encypted (signature)
$CTB_SEC=05;			# Secret Key Certificate
$CTB_PUB=06;			# Public Key Certificate
$CTB_COMP=010;			# Compressed Data
$CTB_CKE=011;			# Conventional-Key Encrypted
$CTB_RAW=013;			# Raw literal data, filename and mode
$CTB_TRUST=014;			# Keyring Trust Packet
$CTB_UID=015;			# User ID Packet
$CTB_CMT=016;			# Comment Packet

sub ctb {
    # Stolen from Adam Back's extract-pub, slightly modified for my
    # own stylistic preferences.

    my ($c,$d,$ct,$lf,$n,$len,$fh);
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
    ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=gmtime($time);
    return(sprintf("%d %s %d  %02d:%02d:%02d",
		   $mday,$months[$mon],$year+1900,$hour,$min,$sec));
}

sub data_print {
    my($data,$pack);

    ($data,$pack)=@_;
    $data=unpack('H*',$data) unless $pack;
    print "Data:\t0x\U$data\E\n";
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
    # read a PGP format Multi-Precision Integer
    my($len,$nlen,$n,$fh);

    if (@_) {
	($fh)=@_;
    }
    else {
	$fh="PGP";
    }
    read($fh,$len,2);$len=unpack('n',$len);
    $nlen=int($len/8);
    $nlen++ if $len%8;
    read($fh,$n,$nlen);
    return unpack('H*',$n);
}

sub findkey {
    # Find a keyID in the pubring file and print out its UserID.
    # For use with the -u function
    my($targ,$ct,$len,$lf,$dmy,$nn,$uid);

    unless (stat(PUBRING)) {
	# Silently return if the file handle isn't open for
	# some reason.
	return "";
    }
    ($targ)=@_;
    seek PUBRING,0,0;		# rewind the filehandle
    while (($ct,$len,$lf)=&ctb(PUBRING)) {
	unless ($ct==$CTB_PUB) {
	    # Eat the packet.
	    read(PUBRING,$dmy,$len);
	    next;
	}
      outer:
	# OK, it's a public key packet.  Is it the right one?
	read(PUBRING,$dmy,1+4+2+1); # Skip version, timestamp, validity, alg.
	$nn=&rmpi(PUBRING);	# Get the N.
	# It's the one we're looking for if the last 16 chars match.
	# Eat the rest of the packet.
	read(PUBRING,$dmy,$len-length($nn)/2-2-1-4-2-1);
	next unless $nn=~/$targ$/i; # Carry on if no match.
	# It's the right one.  Look for a UID packet.
      inner:
	while (($ct,$len,$lf)=&ctb(PUBRING)) {
	    if ($ct==$CTB_PUB) {
		# New public key.  No userID, I guess.
		# Start again.  Have to use a goto.
		goto outer;
	    }
	    unless ($ct==$CTB_UID) {
		# Skip non-uid packets.
		read(PUBRING,$dmy,$len);
		next;
	    }
	    # At long last, found the uid!
	    read(PUBRING,$uid,$len);
	    print "    User ID for above key: \"$uid\"\n";
	    next;
	}
    }
}

sub null_handler {
    # Dummy handler; just skip $len bytes.
    my($len,$dmy);

    ($len)=@_;
    print "(No handler known.  Skipping $len bytes)\n" unless $t;
    read(PGP,$dmy,$len);
    &data_print($dmy) if $v&&!$t;
}

sub comment_handler {
    # Just skip $len bytes.
    # Same as above, minus the message.  Both are included for modularity.
    my($len,$dmy);

    ($len)=@_;
    print "$len bytes of comment.\n";
    read(PGP,$dmy,$len);
    print "Comment:\t$dmy\n" if $v&&!$t;
}

sub uid_handler {
    my($len,$uid);

    ($len)=@_;
    read (PGP,$uid,$len);
    printf "User ID:%s\"$uid\"\n",($t?" ":"\t");
}

sub pub_handler {
    my($len,$vb,$ts,$valid,$alg,$ee,$nn);

    ($len)=@_;
    read(PGP,$vb,1); $len--; $vb=unpack('C',$vb);
    read(PGP,$ts,4); $len-=4; $ts=unpack('N',$ts);
    read(PGP,$valid,2); $len-=2; $valid=unpack('n',$valid);
    read(PGP,$alg,1); $len--; $alg=unpack('c',$alg);
    $nn=&rmpi(); $len-=length($nn)/2+2;
    $ee=&rmpi(); $len-=length($ee)/2+2;
    unless ($t) {
	printf ("Version Byte:\t%d\n",$vb);
	print "Key Created:\t".&date_fmt($ts)."\n";
	print "Valid for",($valid > 0? ":\t $valid days" : "ever"),"\n";
	printf("Algorithm:\t%d (%s)\n",$alg,($alg==1?"RSA":"??"));
	print "N:\t0x\U$nn\E\n";
	print "E:\t0x\U$ee\E\n";
	&length_warn($len) if $len;
    }
    else {
	printf "Key ID: 0x%s\n",substr("\U$nn",-16);
    }
}

sub conv_handler {
    my ($dmy,$len);

    # Yet another "just suck 'em in" handler.
    ($len)=@_;
    print "$len bytes of data...\n";
    read(PGP,$dmy,$len);
    &data_print($dmy) if $v&&!$t;
}

sub comp_handler {
    my ($dmy,$len,$c);

    ($len)=@_;
    read(PGP,$dmy,1); $len--; $dmy=unpack('C',$dmy);
    printf ("Algorithm:\t%d (%s)\n",$dmy,($dmy==1?"ZIP":"??")) unless $t;
    if ($len>0) {
	print "$len bytes of data...\n";
	read(PGP,$dmy,$len);
	&data_print($dmy) if $v&&!$t;
    }
    else {
	# Presumably runs to the end of the file.
	print "And then a mess of data...\n";
	print "Data:\t0x" if $v&&!$t;
	while (!eof(PGP)) {
	    $c=getc(PGP);
	    if ($v&&!$t) {
		$c=unpack('H2',$c);
		print "\U$c\E";
	    }
	}
	print "\n" if $v&&!$t;
    }
}

sub trust_handler {
    my($len,$byte);
    my($work,$ind);

    ($len)=@_;
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
	$lastctb!=$CTB_SKE) {
	# If it's not after a permitted type, then show *all* the
	# possible meanings, in a slightly different format.
	print
	    "Corrupt file! Trust packet should follow key, userID, or sig.\n";
	$ind="\t";
    }
    printf("Trust byte: %s\n",unpack("B8",$byte));
    $byte=unpack('C',$byte);
    if ($ind || $lastctb==$CTB_PUB || $lastctb==$CTB_SEC) {
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
    my ($len,$version,$kid,$alg,$dmy);

    ($len)=@_;
    read(PGP,$version,1); $len--;
    $version=unpack('C',$version);
    read(PGP,$kid,8); $len-=8;
    $kid=unpack('H*',$kid);
    read(PGP,$alg,1); $len--;
    $dmy=&rmpi();
    printf ("Version:\t%d\n",$version) unless $t;
    print "Key ID:\t0x\U$kid\E\n";
    unless ($t) {
	&findkey($kid) if $u;
	$alg=unpack('C',$alg);
	printf ("Algorithm:\t%d (%s)\n",$alg,(1==$alg ? "RSA" : "??"));
	print length($dmy)/2, " bytes of data.\n";
	$len-=length($dmy)/2+2;	# add two bytes for length field
	&data_print($dmy,1) if $v;
	&length_warn($len) if $len;
    }
}

sub ske_handler {
    my ($len,$vb,$md5len,$class,$ts,$kid,$pkalg,$digalg,$chk,$dmy);

    ($len)=@_;
    read(PGP,$vb,1); $len--; $vb=unpack('C',$vb);
    read(PGP,$md5len,1); $len--; $md5len=unpack('C',$md5len);
    read(PGP,$class,1); $len--; $class=unpack('C',$class);
    printf("Version:\t%d\n",$vb) unless $t;
    printf("Adding %d bytes of header to digest\n",$md5len) unless $t;
    if ($class==0x0) {
	print "Signature of binary document";
    }
    elsif ($class==0x1) {
	print "Signature of canonical text document";
    }
    elsif ($class==0x10) {
	print "Generic Key certification";
    }
    # Most of these are unsupported
    elsif ($class==0x11) {
	print "Persona Key certification (unsupported)";
    }
    elsif ($class==0x12) {
	print "Casual ID Key certification (unsupported)";
    }
    elsif ($class==0x13) {
	print "Positive ID Key certification (unsupported)";
    }
    elsif ($class==0x20) {
	print "Key Compromise certificate, revoking public key";
    }
    elsif ($class==0x30) {
	print "Key/UserID Revocation certificate (unsupported)";
    }
    elsif ($class==0x40) {
	print "Timestamp of signature (unsupported)";
    }
    else {
	printf("Unknown signature classification (%x)",$class);
    }
    read(PGP,$ts,4); $len-=4; $ts=unpack('N',$ts);
    read(PGP,$kid,8); $len-=8; $kid=unpack("H*",$kid);
    read(PGP,$pkalg,1); $len--; $pkalg=unpack('C',$pkalg);
    read(PGP,$digalg,1); $len--; $digalg=unpack('C',$digalg);
    read(PGP,$chk,2); $len-=2; $chk=unpack('H*',$chk);
    $dmy=&rmpi();
    unless ($t) {
	print "\nSignature Created:\t".&date_fmt($ts)."\n";
	print "Signing Key ID:\t0x\U$kid\E\n";
	&findkey($kid) if $u;
	printf("Public Key Algorithm:\t%d (%s)\n",
	       $pkalg,($pkalg==1?"RSA":"??"));
	printf("Message Digest Algorithm:\t%d (%s)\n",
	       $digalg,($digalg==1?"MD5":"??"));
	print "Check bytes:\t0x\U$chk\E\n";
	print length($dmy)/2, " bytes of data\n";
	$len-=length($dmy)/2+2;	# Add two for bitcount field
	&data_print($dmy,1) if $v;
	&length_warn($len) if $len;
    }
    else {
	print " (0x\U$kid\E)\n";
    }
}

sub sec_handler {
    my($len,$vb,$ts,$valid,$alg,$nn,$ee,$ciph,$iv,$dd,$pp,$qq,$uu,
       $chksum);

    ($len)=@_;
    read(PGP,$vb,1); $len--; $vb=unpack('C',$vb);
    read(PGP,$ts,4); $len-=4; $ts=unpack('N',$ts);
    read(PGP,$valid,2); $len-=2; $valid=unpack('n',$valid);
    read(PGP,$alg,1); $len--; $alg=unpack('C',$alg);
    $nn=&rmpi(); $len-=length($nn)/2+2;
    $ee=&rmpi(); $len-=length($ee)/2+2;
    read(PGP,$ciph,1); $len--; $ciph=unpack('C',$ciph);
    unless ($t) {
	print "Version Byte:\t$vb\n";
	print "Key Created:\t".&date_fmt($ts)."\n";
	printf("Valid for%s\n",($valid?"\t$valid days":"ever"));
	printf("Algorithm:\t%d (%s)\n",$alg,(1==$alg?"RSA":"??"));
	print "N:\t0x\U$nn\E\n";
	print "E:\t0x\U$ee\E\n";
	printf("Protection Algorithm:\t%d (%s)\n",$ciph,
	       ($ciph ? (1==$ciph ? "IDEA" : "??") : "None"));
    }
    else {
	printf "0x%s\n",substr("\U$nn",-16,16);
    }
    if ($ciph) {
	read(PGP,$iv,8); $len-=8;
	print "8 bytes of cipher feedback\n" unless $t;
    }
    $dd=&rmpi(); $len-=length($dd)/2+2;
    $pp=&rmpi(); $len-=length($pp)/2+2;
    $qq=&rmpi(); $len-=length($qq)/2+2;
    $uu=&rmpi(); $len-=length($uu)/2+2;
    read(PGP,$chksum,2); $len-=2; $chksum=unpack('H*',$chksum);
    unless ($t) {
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
	&length_warn($len) if $len;
    }
}

sub raw_handler {
    my($len,$mode,$namelen,$name,$ts,$data);

    ($len)=@_;
    read(PGP,$mode,1); $len--;
    read(PGP,$namelen,1); $len--; $namelen=unpack('C',$namelen);
    read(PGP,$name,$namelen); $len-=$namelen;
    read(PGP,$ts,4); $len-=4; $ts=unpack('N',$ts);
    # There's also 'l'ocal mode, undocumented.
    printf("Mode:\t%s (%s)\n",$mode,
	   ('b' eq $mode ? "binary" : ('t' eq $mode ? "canonical text" :
				       "unknown"))) unless $t;
    print "Length of filename:\t$namelen\n" unless $t;
    print "Filename:\t$name\n";
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
    if ($len>0) {
	read(PGP,$data,$len);
	print "$len bytes of data.\n" unless $t;
	&data_print($data) if $v&&!$t;
    }
    else {
	print "And then a mess of data...\n" unless $t;
	while (!eof(PGP)) {
	    $c=getc(PGP);
	    if ($v&&!$t) {
		$c=unpack('H2',$c);
		print "\U$c\E";
	    }
	}
	print "\n" if $v&&!$t;
    }
}

sub process_packet {
    my($ct,$lf,$len,$typestr,$handler);

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
    elsif ($ct==$CTB_SEC) {
	$typestr="Secret Key Packet";
	$handler=\&sec_handler;
    }
    elsif ($ct==$CTB_PUB) {
	$typestr="Public Key Packet";
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
    elsif ($ct==$CTB_CMT) {
	$typestr="Comment Packet";
	$handler=\&comment_handler;
    }
    else {
	$typestr=sprintf("UNKNOWN PACKET!! (%o)",$ct);
	$handler=\&null_handler;
    }
    print "\n---------------------------\nPacket Type:\t" unless $t;
    print "$typestr",($t?": ":"\n");
    print "Length:\t$len\n" unless $t;
    &$handler($len);
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
\t-a: ASCII mode.  Presume ASCII mode, even if first char is 8-bit
\t-u[=pubring]: Look up keyIDs in pubring file.
\t\tIf pubring is not specified, defaults to ~/.pgp/pubring.pgp.

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
	$u=$ENV{'HOME'}.'/.pgp/pubring.pgp';
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

iQCVAwUBMWFS4djId3FCnAfhAQFLdwQAqaI/IyPImQcDmTbXXslAmxvdqZHlgqRa
kR9TJ4TbmPze/TpRI/gmjkemZQ/K8gx/Sdwax8zM5Xxuitdx6+NtnRPh5sxytBRp
rk3AZK8sI391rBhoa7YkMQ6a/1F4c06GngN9AmP2mZEBeC5G11HDy2Jb68ZawBu7
ZbW58VweelE=
=whIy
-----END PGP SIGNATURE-----
-END
    ;
