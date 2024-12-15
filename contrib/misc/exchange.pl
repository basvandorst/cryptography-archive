
>Do you know of a script or program which will select only those
>keys which form a transitive closure (or have I missed something
>in the PGP Docs?)?  I can run sh/ksh&awk scripts or 'C' programs
>on the PC, and I'm prepared to port it if it's not too outrageous.

I have a Perl script which does this for me.  Unfortunately it is
something of a hack, having numerous bugs, misfeatures and bad design
decisions - many of which are direct consequences of bugs and
misfeatures in PGP itself.  It's only about 200 lines or so.

With respect to performance in general, having a real operating system
which does disk buffering and read-ahead and can use all the memory in
the machine really helps.  Being able to do something else while it
grinds away is nice, as well :-).  You say you have a 386; perhaps you
should try Linux, the free Unix for 386/486, if you don't already know
about it.

I enclose the script below.  I understand there is a version of Perl
for DOS, though whether it would work given the amount of memory (over
1.5Mb) and the features of Perl and Unix used by my script I don't
know.

It is quite slow, mainly because it adds new keys from public-keys.pgp
to pubring.pgp one at a time, leading to quadratic behavour on the
first run when you start with a near-empty pubring.pgp.  On my 486/33
it takes perhaps 5 or 10 seconds per key added, so you can calculate
approximately how long it would take for the whole closure of 500-odd
keys.  This would probably be completely impossible without a proper
disk cache, as each run of pgp copies the whole pubring back and
forth.  If this were to result in actual disk activity rather than
just memory copies the runtime would be dreadful.  There is however no
real reason why you couldn't write a more effective keyring sort/merge
program in C that would do it in a fraction of the time.

The fastest part is calculating the transitive closure - 5 seconds or
so, using a really naive algorithm, after it has created and scanned
the pgp -kvv output from each of the keyrings.

Note that I also had to apply the following patch to keymgmt.c to make
it possible to remove a key with several userids in batchmode.  For
some reason +force didn't have any effect on this one.  Grrr.  This
also has the side-effect of making deleting the whole key the default
option for removing such a key interactively.

 --- keymgmt.c~  Wed Jun 23 21:53:53 1993
 +++ keymgmt.c   Sun Sep 12 15:37:36 1993
 @@ -1803,7 +1803,7 @@
                 if (userids > 1)
                 {       fprintf(pgpout, PSTR("\nKey has more than one user ID.\n\
  Do you want to remove the whole key (y/N)? "));
 -                       if (!getyesno('n'))
 +                       if (!getyesno('y'))
                         {       /* find out which userid should be removed */
                                 rmuserid = TRUE;
                                 fseek (f, fp+packetlength, SEEK_SET);

I'm not really interested in bug reports for the script, however
patches to fix bugs you discover are welcome.  There is no
documentation and no guarantee of success.

#!/usr/bin/perl --
#
# Usage: .../exchange.pl

chdir($ENV{'PGPPATH'}) || die "chdir to PGPPATH: $ENV{'PGPPATH'}: $!\n"
    if defined($ENV{'PGPPATH'});

select((select(STDERR),$|=1)[0]);

$serverring= 'public-keys.pgp';
$mailcmd= 'mail -s ADD pgp-public-keys@toxicwaste.mit.edu `whoami`';

# Key/signature status codes:
#   1  001 = key transfer inhibited
#   2  002 = pubring.pgp
#   4  004 = public-keys.pgp
#   8  010 = revoked
#  16  020 = in transitive closure
#  32  040 = have the secret key in secring.pgp

&mstart("reading kconfig");
open(INH,"kconfig.txt") || die "excludelist unreadable: $!\n";
while(<INH>) {
    next if m/^#/;
    s/\s*$//;
    if (m/^serverring\s+/) {
        $serverring= $';
    } elsif (m/^explictroot$/) {
        $explictroot= 1;
    } elsif (m/^root\s+(\w{6})$/) {
        $key_status{$1} |= 040;
    } elsif (m/^mailcmd\s+/) {
        $mailcmd= $';
    } elsif (m/^inhibit\s+(\w{6})$/) {
        $key_status{$1} |= 001;
    } else {
        die "unknown thing in excludelist: `$_' at line $.\n";
    }
    $anychange++;
}
&mend(sprintf("%d commands processed",$anychange));

&scan("pubring.pgp",002);
&scan($serverring,004);
unless ($explictroot) {
    &scan("secring.pgp",0);

    &mstart("locating secret keys");
    $anychange=0;
    for $id (keys %key_status) {
        next unless ($key_status{$id} & 40) == 040;
        $key_status{$id} |= 020;
        $anychange++;
    }
    &mend(sprintf("%d secret keys found",$anychange));
}

$pass=0;
for (;;) {
    $anychange=0; $pass++;
    &mstart("finding closure (pass $pass)");
    for $idid (keys %sig_status) {
        $idon= substr($idid,0,6); $idby= substr($idid,6);
        next if ($key_status{$idby} & 030) != 020; # need inclosure,!revoked
        next if ($key_status{$idon} & 030) != 000; # need !inclosure,!revoked
        $key_status{$idon} |= 020;  $anychange++;
    }
    last unless $anychange;
    $total+= $anychange;
    &mend(sprintf("pass %2d: %3d keys -> %4d",$pass,$anychange,$total));
}

&mend(sprintf("%d keys in %d passes",$total,$pass-1));

&mstart("looking for new signatures");
for $idid (keys %sig_status) {
    $status= $sig_status{$idid};
    $idon= substr($idid,0,6); $onstatus= $key_status{$idon};
    $idby= substr($idid,6); $bystatus= $key_status{$idby};
    if (($status & 006) == 004 && # need !pubring,publickeys
        ($onstatus & 030) == 020 && # need !revoked,inclosure
        ($bystatus & 030) == 020) { # need !revoked,inclosure
        $key_public2pub{$idon}.= "  sig $key_name{$idby}\n";  $insig++;
    } elsif (($status & 006) == 002 && # need pubring,!publickeys
             ($onstatus & 031) == 020 && # need !inhibit,!revoked,inclosure
             ($bystatus & 030) == 020) { # need !revoked,inclosure
        $key_pub2public{$idon}.= "  sig $key_name{$idby}\n";  $outsig++;
    }
}
&mend(sprintf("sigs: %d in, %d out",$insig,$outsig));

&mstart("looking for key changes");
for $id (keys %key_status) {
    $status= $key_status{$id};
    if (($status & 037) == 024) { # need inclosure,!revoked,publickeys,!pubring,!inhibit
        $key_public2pub{$id}= '';  $inkey++;
    } elsif (($status & 017) == 002) { # need !revoked,!publickeys,pubring,!inhibit
#printf STDERR "status %03o %s\n",$status,$key_name{$id};
        $key_pub2public{$id}= '';  $outkey++;
    } elsif (($status & 023) == 002) { # need !closure,pubring,!inhibit
        $key_delete{$id}= '';  $oldkey++;
    }
}
&mclear;

@pub2public= keys %key_pub2public;
if (@pub2public) {
    open(T,">/tmp/ke$$") || die "create /tmp/ke$$: $!\n";
    print STDERR "Exporting:\n";
    while ($id = pop(@pub2public)) {
        &extractkey($id,'pubring.pgp',$key_pub2public{$id});
    }
    close(T) || die "close /tmp/ke$$: $!\n";
    system("$mailcmd </tmp/ke$$");
    $? && die "mail command gave $?\n";
    unlink("/tmp/ke$$") || die "unlink /tmp/ke$$: $!\n";
}

@public2pub= keys %key_public2pub;
if (@public2pub) {
    open(T,">/tmp/ke$$") || die "create /tmp/ke$$: $!\n";
    print STDERR "Importing:\n";
    while ($id = pop(@public2pub)) {
        &extractkey($id,$serverring,$key_public2pub{$id});
    }
    close(T) || die "close /tmp/ke$$: $!\n";
    open(KAF,"pgp +batchmode -kaf </tmp/ke$$ 2>&1 |")
        || die "pipe/fork for pgp -kaf: $!\n";
    $msgs=''; $state=0;
    while(<KAF>) {
        $msgs.= $_; print STDERR "  ",$_ if m/^sig/ || m/^pub/;
    }
    close(KAF);
    $? && die "pgp add command gave $?\nHere is what PGP said:\n$msgs\n";
    unlink("/tmp/ke$$") || die "unlink /tmp/ke$$: $!\n";
}

@delete= keys %key_delete;
if (@delete) {
    print STDERR "Deleting:\n";
    while ($id = pop(@delete)) {
        print STDERR " DEL ",$key_name{$id},"\n";
        open(KR,"pgp +force +batchmode -kr 0x$id 2>&1 |")
            || die "pipe/fork for pgp -kr: $!\n";
        $msgs='';
        while(<KR>) { $msgs.= $_; }
        close(KR);
        $? && warn "pgp remove command gave $?\nHere is what PGP said:\n$msgs\n";
    }
}

exit 0;

sub extractkey {
    local ($id,$ring,$level) = @_;
    print STDERR ' ',length($level)?'on ':'pub','  ',$key_name{$id},"\n",$level;
    open(KXA,"pgp +batchmode -kxaf 0x$id 2>&1 $ring |")
        || die "pipe/fork for pgp -kxa: $!\n";
    $msgs=''; $state=0; $key='';
    while(<KXA>) {
        $state=1 if m/^-----BEGIN PGP PUBLIC KEY BLOCK-----/;
        if ($state) { $key.= $_; } else { $msgs.= $_; }
        $state=0 if m/^-----END PGP PUBLIC KEY BLOCK-----/;
    }
    close(KXA);
    $? && die "pgp extract command gave $?\nHere is what PGP said:\n$msgs\n";
    $key ne '' && $state==0 ||
        die "pgp extract command produced no key\nHere is what PGP said:\n$msgs\n";
    print(T $key) || die "write to /tmp/ke$$: $!\n";
}

sub mstart { print STDERR "@_ ..."; }
sub mclear { print STDERR "\r",' 'x79,"\r"; }
sub mend { &mclear; print STDERR "@_;  "; }

sub scan {
    local ($keyring,$locationcode) = @_;
    &mstart("scanning $keyring");
    local ($nkeys, $nsigs);
    open(KVV,"pgp +verbose=0 -kvv $keyring|") || die "pipe/fork for pgp -kvv: $!\n";
    while(<KVV>) {
        s|\s*$||;
        if (m|^pub\s+\d+/(\w{6})\s+\d{4}/\d\d/\d\d\s+|) {
            $currentkey= $1;
            $key_status{$currentkey} |= $locationcode;
            $key_status{$currentkey} |= 010 if $' eq '*** KEY REVOKED ***';
            s/^pub\s//; $key_name{$currentkey}= substr($_,0,70); $nkeys++;
#print STDERR "recording $locationcode $key_status{$currentkey} $currentkey $key_name{$currentkey}\n" if $locationcode != 2;
        } elsif (m|^sig\s+(\w{6})\s+|) {
            defined($currentkey) || die "signature (by $1) before key in $keyring\n";
            $sig_status{$currentkey.$1} |= $locationcode;
            $nsigs++;
        } elsif (m|^sec\s+\d+/(\w{6})\s+\d{4}/\d\d/\d\d\s+|) {
            next if $locationcode;
            $key_status{$1} |= 040;
            $currentkey= undef;
            $nkeys++;
        } elsif (!(m|^\s+| ||
                   m|^$| ||
                   m|^Type bits/keyID| ||
                   m|^\d+ key\(s\) examined|)) {
            die "unknown thing in $keyring: `$_' at line $.\n";
        }
    }
    close(KVV);
    $? && die "pgp -kvv $keyring gave code $?\n";
    &mend(sprintf("%s has %d keys, %d sigs",$keyring,$nkeys,$nsigs));
}

