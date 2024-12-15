# Usage:
#  pgp-decrypt
#   decrypt into a clearsig message or check signature
# Always reads from stdin, writes to stdout

# This file is COPYRIGHT - see notice above or file COPYRIGHT
# in this directory.

&parseargs;
&setup;
&dearmour;
&evaluate;
&erase;

#
#
# Setting up activities
#
#

sub parseargs {
    if ($ARGV[0] eq 'debug') {
        open(DEBUG,">&STDERR");
        shift @ARGV;
    }
    if (@ARGV) { &usage; }
}

sub signal { print STDERR "Error: caught SIG$_[0]\n"; &erase; exit(1); }
sub quit { print STDERR "Error: $_[0]\n"; &erase; exit(1); }

sub setup {
    umask(077);

    chop($p=`hostname`);
    $p .= "-$$";
    
    chdir($config_wrapperdir)
        || die "$0: changing to $config_wrapperdir: $!\n";
    
    @s= stat('.');
    @s || die "stat $config_wrapperdir: $!\n";
    ($s[2] & 0777) == 0700
        || die "$0: $config_wrapperdir must be mode 700 (rwx------)";

    select(STDERR); $|=1;
    select(STDOUT); $|=1;

    for $s (( 'INT','TERM','PIPE','HUP','ALRM' )) {
        $SIG{$s} = 'signal';
    }
    &inittables;
}

sub erase {
    for $letter (( 's','m','r' )) {
        for $extn (( '','.pgp','.asc','.sig','.txt','.z','.gz','.$00','.$01','.$02' )) {
            unlink($p.$letter.$extn);
        }
    }
}

sub usage { &quit("usage: $0 [debug]"); }

sub inittables {
    %ctbtypes = (
                  1, 'pke',	# public-key-encrypted
                  2, 'ske',	# secret-key-encrypted
                  5, 'skc',	# secret-key-certificate
                  6, 'pkc',	# public-key-certificate
                  8, 'cd',	# compressed-data
                  9, 'cke',	# conventional-key-encrypted
                 11, 'rlp',	# raw-literal-plaintext
                 12, 't',	# trust
                 13, 'ui',	# user-id
                 14, 'c',	# comment
                 );

#                  St packet St+1 action
    %ctbstates = (
                  'S pke',   'E1 recipient',
                  'E1 pke',  'E2 recipient',
                  'E2 pke',  'E2 recipient',
                  'E1 cke',  'E3',
                  'E2 cke',  'E3',
                  'E3 eof',  'Y decrypt',
                  'S ske',   'V1',
                  'V1 ske',  'V1',
                  'V1 rlp',  'V2',
                  'V2 eof',  'Y checksig',
                  'S pkc',   'K1',
                  'K1 pkc',  'K1',
                  'S skc',   'K1',
                  'K1 skc',  'K1',
                  'K1 ui' ,  'K1',
                  'K1 t',    'K1',
                  'K1 eof',  'Y keyring',
                  );
}

#
#
# Actual processing steps
#
#

#
# De-armour
#

sub dearmour {
    open(TM,"> ${p}m.asc") || &quit("create ${p}m.asc: $!");
    $_= <STDIN>;
    $warnspaces= 0;
    do {
        $warnspaces= 1 if m/^-----BEGIN PGP SIGNED MESSAGE-----/;
        $warnspaces= 2 if $warnspaces==1 && m/\s\n/;
        print TM || &quit("write ${p}m: $!");
    } while (<STDIN>);
    close(TM);
    &runpgp('el_dearmour',0,'+batchmode','+verbose=1','-da',"${p}m.asc");
    $dearmourlevel == 1 || &quit('Failed to strip radix-64 encapsulation.');
    if ($warnspaces==2) {
        print STDERR <<END
Warning: trailing spaces in clearsig message.
Not all versions of PGP will be able to verify this signature unassisted.
END
            || &quit("writing stderr: $!");
    }
}

sub specialdearmour {
    open(TM,"< ${p}m.asc") || &quit("reopen ${p}m.asc: $!");
    open(TS,"> ${p}s.asc") || &quit("create ${p}s.asc: $!");
    open(TR,"> ${p}r.asc") || &quit("create ${p}r.asc: $!");
    do { $_= <TM>; } while (!m/^-----BEGIN PGP SIGNED MESSAGE-----/);
    length($_) || &quit("badly formatted clearsig message");
    $_= <TM>; m/\S/ && &quit("line after BEGIN SIGNED is non-blank");
    while (<TM>) {
        last if m/^-----BEGIN PGP SIGNATURE-----/;
        s/\n$/\r\n/ || &quit("clearsig message line has no newline");
        s/^- //;
        s/^From /^- From /;
        $plainmsg .= $_;
    }
    length($_) || &quit("missing signature at end of clearsig message");
    $plainmsg =~ s/\r\n$// || &quit("line before BEGIN SIGNATURE is non-blank");
    print(TS "-----BEGIN PGP MESSAGE-----\n") || &quit("write ${p}s: $!");
    do { print TS || &quit("write ${p}s: $!"); } while (<TM>);
    close(TR); close(TM); close(TS);
    $dearmourlevel= 0;
    &runpgp('el_dearmour',0,'+batchmode','+verbose=1','-da',"${p}s.asc");
    $dearmourlevel == 1 || &quit('Failed to strip radix-64 encapsulation of signature.');
    open(TS,">> ${p}s.pgp") || &quit("Failed to open ${p}s.pgp for append: $!");
    print(TS
          "\xae",
          pack("N",length($plainmsg)+6),
          "t\0\0\0\0\0",
          $plainmsg) || &quit("Failed to append to ${p}s.pgp: $!");
    close(TS);
}

sub el_dearmour {
    &elany_default && return 1;
    if (m|^Stripped transport armou?r from '.*\.asc', producing '.*\.pgp'\.|) {
        $dearmourlevel= 1; return 1;
    }
    print(STDERR "??? $_") || &quit("writing stderr: $!");
    0;
}

#
# Evaluate contents and act accordingly
#

sub evaluate_actupon {
    ($ns,$act) = split(/ /,$ctbstates{"$state $tn"});
print DEBUG "($state, $tn -> $ns, $act)\n";
    $ns || &quit("format not interpretable by $0 ($state+$tn->?)".
                 " - process manually");
    if ($act) {
        $act= "ev_$act";
print DEBUG "{{{ $act\n";
        &$act;
print DEBUG "}}} $act\n";
    }
}

sub evaluate {
    open(TP,"< ${p}m.pgp") || &quit("open to read ${p}m.pgp: $!");
    $state= 'S';
    for (;;) {
        ($ctb,$pkt)= &readpkt(TP,"${p}m.pgp");
        last if $ctb < 0;
        $t= ($ctb >> 2) & 0x0f;
        $tn= $ctbtypes{$t};
        $tn || &quit("unknown packet type $t");
        &evaluate_actupon;
        $state= $ns;
    }
    $tn= 'eof';
    $ctb= 0;
    &evaluate_actupon;
}

#
#
# Functions called during evaluation by state machine
#
#

#
# Messages that are encrypted (states En).
#

sub ev_recipient {
    unpack("C",$pkt) == 2 || &quit("public-key-encrypted packet version != 2");
    $keyid= substr($pkt,1,8);
    $keyids= sprintf("%02X%02X%02X%02X%02X%02X%02X%02X",
                     unpack("CCCCCCCC",$keyid));
    $pubring= "$config_pgppath/$config_allkeys";
    $pubring= "$config_pgppath/pubring.pgp" unless -r $pubring;
    open(RC,"$config_x_ringsearch $pubring $keyids |") ||
        &quit("trying to run $config_x_ringsearch: $!");
    push(@recipients, <RC>);
    close(RC);
    $? && &quit("$config_aringsearch: gave error exit status $?");
}

sub ev_decrypt {
    print(STDERR
          "Message is encrypted to ",
          ((@recipients>1) ? "multiple recipients" : "single recipient"),
          ":\n",@recipients,"Decrypting ...\n") ||
              &quit("write stderr: $!");
    open(OP,"> ${p}m.txt") || die "create ${p}m.txt";
    print(OP "Message was encrypted; recipients:\n", @recipients)
        || &quit("write ${p}m.txt: $!");
    &runpgp('el_decrypt',1,'+batchmode','+verbose=1','-b',"${p}m.pgp");
    $plainfile || &quit('no plaintext filename!');
    open(PF,"< $plainfile") || &quit("open $plainfile");
    if ($sigfile) {
        print(OP "-----BEGIN PGP SIGNED MESSAGE-----\n\n")
            || &quit("write ${p}m.txt: $!");
        while (<PF>) {
            print(OP "- ") || &quit("write ${p}m.txt")
                if m/^-/ || m/^From /;
            print OP || &quit("write ${p}m.txt");
        }
        print(OP "\n") || &quit("write ${p}m.txt: $!");
        rename($sigfile,"${p}s.pgp");
        &runpgp('el_rearmoursig',0,'+batchmode','+verbose=1','+force','-a',"${p}s.pgp");
        open(SA,"< $sigarmourfile") || &quit("open $sigarmourfile");
        while (<SA>) {
            s/^-----(BEGIN|END) PGP MESSAGE-----/-----$1 PGP SIGNATURE-----/;
            print(OP $_) || &quit("write ${p}m.txt: $!");
        }
        close(SA);
    } else {
        print(OP "----- decrypted message text follows:\n\n")
            || &quit("write ${p}m.txt: $!");
        while (<PF>) {
            s/^From />From /;
            print OP || &quit("write ${p}m.txt: $!");
        }
    }
    close(PF);
    close(OP);
    &writetext;
}   

sub el_decrypt {
    (
     &elany_default ||
     m|^Key for user ID: | ||
     m|^\d+-bit key, Key ID \w{6}, created | ||
     m|^You need a pass phrase to unlock your RSA secret key\.| ||
     m|^File is encrypted\.  Secret key is required to read it\.| ||
     m|^File has signature\.  Public key is required to check signature\.|
     ) && return 1;
    if (
        m|^Good signature from user \"(.*)\"\.$| ||
        m|^Signature made (.*)\s*$|
        ) {
        # do nothing - ie just print the message
    } elsif (m|^Writing signature certificate to '(\S+\.sig)'$|) {
        $sigfile= $1; return 1;
    } elsif (m|^Plaintext filename: (\S+)\s*$|) {
        $plainfile= $1; return 1;
    } elsif (m|^You do not have the secret key needed to decrypt this file\.|) {
        &quit($&);
    } else {
        $_= "??? $_";
    }
    print(STDERR $_) || &quit("writing stderr: $!");
    0;
}

sub el_rearmoursig {
    &elany_default && return 1;
    if (m|Transport armou?r file: (\S+\.asc)|) {
        $sigarmourfile= $1; return 1;
    }
    print(STDERR "??? $_") || &quit("writing stderr: $!");
    0;
}

#
# Messages that are signed but not encrypted (states Vn).
#

sub ev_checksig {
    &runpgp('el_checksig',0,'+batchmode','+verbose=1',"${p}m.pgp");
    if ($warnspaces == 3) {
        print STDERR <<END
Warning: Signature doesn't match message if trailing spaces ignored;
Trying again counting trailing spaces ...
END
            || &quit("writing stderr: $!");
        &specialdearmour;
        &runpgp('el_checksig',0,'+batchmode','+verbose=1',"${p}s.pgp");
    }
}

sub el_ignore { }

sub el_checksig {
    (
     &elany_default ||
     m|^File has signature\.  Public key is required to check signature\.| ||
     m|^Plaintext filename: (\S+)\s*$|
     ) && return 1;
    if (
        m|^Good signature from user \"(.*)\"\.$| ||
        m|^Signature made (.*)\s*$|
        ) {
        # do nothing - ie just print the message
    } elsif (m|^WARNING: Bad signature, doesn't match file contents!|) {
        if ($warnspaces == 3) {
            print(STDERR "Nope, counting the trailing spaces doesn't work either.\n")
                || &quit("writing stderr: $!");
        } elsif ($warnspaces == 2) {
            $warnspaces= 3; $wnline= 'el_ignore';
            return 1;
        }
    } else {
        $_= "??? $_";
    }
    print(STDERR "$_") || &quit("writing stderr: $!");
    0;
}

#
# Messages that are keyrings, not messages (states Kn).
#

sub ev_keyring {
    &quit('Message is a keyring - not processed');
}

#
#
# Utility functions
#
#

sub writetext {
    open(TX,"< ${p}m.txt") || &quit("reopen ${p}m.txt");
    @statv= stat(TX);
    @statv || &quit("fstat ${p}m.txt");
    printf(STDERR "Enclosed message text (%d bytes):\n",$statv[7])
        || &quit("write stderr");
    while (<TX>) {
        print || (&erase,exit(1));
    }
    close(TX);
}

sub readpkt { # returns ($ctb,$packet) or (-1)
    local ($fh,$fn) = @_;
    local ($lenrd,$ctb,$pkt,$lenlen,$lenbin,$len,$ctbb);
    $lenrd= read($fh,$ctbb,1);
    defined($lenrd) || &quit("read CTB ($lenrd) from $fn: $!");
#printf DEBUG "(lenrd=0x%x)\n",$lenrd;
    $lenrd || return (-1);
    $ctb= unpack("C",$ctbb);
#printf DEBUG "(ctb=0x%x)\n",$ctb;
    $ctb & 0x80 || &quit("CTB has bit 7 clear");
    $lenlen= $ctb & 0x03;
    $lenlen != 3 || &quit("CTB has length code `unknown' - cannot cope");
    $lenrd= read($fh,$lenbin,1<<$lenlen);
    defined($lenrd) || &quit("read length from $fn: $!");
    $lenrd == (1<<$lenlen) || &quit("EOF in length code");
    $len= unpack( ('C','n','N')[$lenlen], $lenbin );
    $len &= 0x0ffff if $lenlen==1;
#printf DEBUG "(read %d lenlen=%d len=0x%lx)\n",$lenrd,$lenlen,$len;
    $lenrd= read($fh,$pkt,$len);
    defined($lenrd) || &quit("read packet body from $fn: $!");
#printf DEBUG "(packet read 0x%lx)\n",$lenrd;
    $lenrd == $len || &quit("EOF in packet body");
    ($ctb,$pkt);
}

sub warn {
    return if $warndone{$_[0]};
    print STDERR "$0: Warning: $_[0]\n";
    $warndone{$_[0]}= 1;
}

sub runpgp {
    local ($wnline,$needpass,@cmd) = @_;
    $c= open(PGPOUTPUT,"-|");
    defined($c) || &quit("fork for pgp: $!");
    unshift(@cmd, $needpass ? $config_x_pgpauto : $config_x_normalpgp);
    if (!$c) {
        if (!$needpass) {
            $ENV{'PGPPASS'}= '';
            # This is to give an error if it thinks it needs one,
            # rather than having it prompt and deadlock
        }
        close(STDIN);
        open(STDIN,"</dev/null");
        close(STDERR);
        open(STDERR,">&STDOUT");
        die "STDERR != 2 !" unless fileno(STDERR) == 2; # sanity
        exec @cmd;
        die "failed to execute $cmd[0]: $!\n";
    }
    
print DEBUG "running `@cmd' ($needpass)\n";
    for(;;) {
print DEBUG "<"; $line="\n";
        $_= <PGPOUTPUT>;
        s/\007//g;
print DEBUG "($wnline)> $_";
        last unless $_;
        if (&$wnline) {
            print(STDERR " ",$_) || &quit("error writing stderr");
        }
    }
print DEBUG "EOF\n";
    close(PGPOUTPUT);
}

sub elany_default {
    m/^Error:  Bad pass phrase\./ && &quit("Bad pass phrase.");
    m/^For a usage summary, type:/ && &quit("Failure running PGP.");
    (
     m|^Pretty Good Privacy 2\.\d.? - Public\-key encryption for the masses\.| ||
     m|^\(c\) 1990-199\d Philip Zimmermann, Phil's Pretty Good Software\.| ||
     m|^Date: \d{4}/\d\d/\d\d \d\d:\d\d| ||
     m|^\s*$| ||
     m|^Pass phrase is good\.| ||
     m|^Just a moment\.+| ||
     m|^\.+$|
     );
}
