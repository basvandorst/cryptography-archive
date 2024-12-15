#!/usr/local/bin/perl

## the file the setup commands will be read from.
## MUST be an absolute path!!
$rcfilename = "/some/dir/in/your/machine";

### Shouldn't need to touch anything below here ###

$date = `date`;
chop $date;
$pid = $$;
$version = "Nate's Remailer Software, version 1.0";
$padbegin = "\n-----BEGIN REMAILER GARBAGE-----";
$padend = "-----END REMAILER GARBAGE-----\n";

## parse the RC file and set variables.
&parsercfile;

$rc = chdir $home;
if ($rc == 0) { exit 0; }
if ($debug) {
  open(DEBUG, ">$debugfile");
  select DEBUG;
  }
$ENV{'HOME'} = $home;
substr($ENV{'PATH'},0,0) = ".:";
$SIG{'PIPE'} = 'IGNORE';

@message = <STDIN>;

##makes the @abody and @acommands vars. (parses header also)
&parsemessage("first");

print "message:\n=============================\n";
foreach $i (@abody) {
  print "$i\n";
  }
print "===============================\n";

## if there aren't any remailer commands, let's get out as fast as we can.
if (!@acommands) {
  &dumptombox;
  }

## parse commands array (external envelope)
foreach $_ (@acommands) {
  if (/SendInformation/) { &sendinformation; }  ## send info
  if (/Encrypted: PGP/) { $decrypt = 1; }
  }

if (!$decrypt) {
  if ($debug > 0) {
    print DEBUG "Message was not encryted.  Exiting.\n";
    }
  exit 0;
  }

## decrypt the message
## decrypt takes @message and returns @message.
&decrypt;

## makes the @abody and @acommands vars. (skips the header stuff, though)
undef @abody;
undef @acommands;
&parsemessage("second");

## parse the @acommands array (again!)
foreach $_ (@acommands) {
  if (/^Request-Remailing-To: ([\w!@.,]+)/) { $addressee = $1; }
  elsif (/^Encrypted: PGP/) { $encrypted = 1; }
  elsif (/^Subject:\s(\S[\s\S]*)/) { $subject = $1; }
  elsif (/^PleaseDontPad/) { $pleasedontpad = 1; }
  elsif (/^Latency:/) {
    if (/^Latency:\s([\d]+) MINUTES/) { $latmin = $1; }
    elsif (/^Latency: RANDOM MINUTES/) { $latrand = 1; }
    }
  }
undef @acommands;

if ($debug > 0) {
  print DEBUG "Message flags on Internal Envelope:\n";
  &printmessageflags;
  print DEBUG "\n\n";
  }

&checkblocks;
undef @message;
$body = join("\n", @abody);
@message[0] = $body;
undef $body;

if ($encrypted) {
  &decrypt;
  }

## Now, reassemble the body of the message
undef $body;
$body = join("", @message);
undef @message;

$bodylength = length($body);
&do_padding;
&dologs;

if ($latmin) {}
elsif ($latrand) { $latmin = rand $maxlatmin; }
elsif ($latmin > $maxlatmin) { $latmin = $maxlatmin; }
else { $latmin = 0; }
$sleeptime = ($latmin * 60);
if (-e "$tempfile") {
  print DEBUG "ERROR: \"$tempfile\" exists!\n";
  close(DEBUG);
  exit 0;
  }
open(TEMP, ">$tempfile");
print TEMP "To: $addressee\n";
print TEMP "From: $fromwho\n";
print TEMP "Subject: $subject\n";
print TEMP "X-Remailer-Name: $remailername\n";
print TEMP "X-Remailer-Version: $version\n";
if (@comments[0]) {
  foreach $i (0..(@comments - 1)) {
    print TEMP "X-Comment: @comments[$i]\n";
    }
  }
print TEMP "\n$body\n";
close(TEMP);
## start the waitnmail.pl program "nicely" so it doesn't hog cpu time.
system "nice -10 ./waitnmail.pl $sendmail $addressee $sleeptime $tempfile &";

################# subroutine defs ##################

sub decrypt {
  if (-e $tempfile) { die "ERROR: temp file \"$tempfile\" exists!\n"; }
  if (-e $pgptempfile) { die "ERROR: temp file \"$pgptempfile\" exists!\n"; }
  open(PGPTEMP, ">$pgptempfile");
  print PGPTEMP "\n" . join("", @message) . "\n";
  close(PGPTEMP);
  $ENV{'PGPPATH'} = "$pgppath";
  $ENV{'PGPPASS'} = "$passphrase";
  $rc = (system "$pgpcmd $pgptempfile -o $tempfile")/256;
  if (($rc == 0) && ($debug >0)) {
    print DEBUG "PGP Call was successful.\n";
    }
  elsif ($rc != 0) {
    print DEBUG "PGP Call was unsuccessful.\n" if $debug >0;
    exit 0;
    }
  unlink("$pgptempfile");
  $ENV{'PGPPASS'} = 0;
  open(TEMPF, "$tempfile");
  @message = <TEMPF>;
  close(TEMPF);
  unlink("$tempfile");
  }

sub dumptombox {
  print DEBUG "No commands... putting mail into $mbox.\n" if ($debug > 0);
  if (!(-e "$mbox")) {
    system "touch $mbox";
    }
  open(MBOX, ">>$mbox");
  flock(MBOX, 2);
  seek(MBOX, 0, 2);
  print MBOX join("", @message) . "\n\n";
  close(MBOX);
  close(DEBUG);
  exit 0;
  }

sub parsemessage {
  print DEBUG "Parsing the text of the message: @_[0] pass.\n" if ($debug > 0);
  undef $headerdone;
  $commandsdone = 0;
  undef @abody;
  undef @acommands;
  if (@_[0] eq "first") {   ## check if we should look for a header
    $headerdone = 0;        ## or go straight for the commands
    }
  elsif (@_[0] eq "second") {
    $headerdone = 1;
    }

  ## body is read from the global @message variable
  $i = 0;
  chop($_ = @message[$i]);
  if (!$headerdone) {
    while (/^\s*$/) {
      chop($_ = @message[$i + 1]);
      ++$i;
      }
    do {
      if (/From: (\S+)/) {
	$sender = $1;
	}
      chop($_ = @message[$i + 1]);
      ++$i;
      } until /^\s*$/;
    }
  while (/^\s*$/) {
    chop($_ = @message[$i + 1]);
    ++$i;
    }
  while (/^::$/) { ## command marker
    chop($_ = @message[$i + 1]);
    ++$i;
    push(@acommands, $_);
    chop($_ = @message[$i + 1]);
    ++$i;
    $starthere = $i;
    }
  do {
    push(@abody, $_);
    chop($_ = @message[$i + 1]);
    ++$i;
    } until $i == @message;
  }

sub dologs {
  if ($loglev == 1) {
    if (!(-e "$logsynopsis")) {
      open(SYNOPSISLOG, ">$logsynopsis");
      close(SYNOPSISLOG);
      }
    open(SYNOPSISLOG, ">>$logsynopsis");
    flock(SYNOPSISLOG, 2);
    seek(SYNOPSISLOG, 0, 2);
    select SYNOPSISLOG;
    print "\nMessage:\n";
    print "Date:\t\t\t$date\n";
    print "From:\t\t\t$sender\n";
    print "To:\t\t\t$addressee\n";
    print "Subject:\t\t$subject\n";
    print "Request-Remailing-To:\t$addressee\n";
    print "Message size:\t\t$bodylength characters\n";
    if ($decrypt) { print "Encrypted:\t\tYES\n"; }
      else { print "Encrypted:\t\tNO\n"; }
    if ($encrypted) { print "Super Encrypted:\tYES\n"; }
      else { print "Super Encrypted:\tNO\n"; }
    if ($pleasedontpad) { print "PleaseDontPad:\t\tYES\n"; }
      else { print "PleaseDontPad:\t\tNO\n"; }
    if ($latmin) { print "Latency:\t\t$latmin minutes\n"; }
      elsif ($latrand) { print "Latency:\t\trandom number of minutes\n"; }
      else { print "Latency:\t\tNONE\n"; }
    print "\n";
    close(SYNOPSISLOG);
    }
  select DEBUG;
  }

sub printbody {
  print DEBUG "\nBody of message is as follows:\n===================\n";
  print DEBUG "$body";
  print DEBUG "\n=============================\n\n";
  }

sub do_padding {
  if (!$pleasedontpad && $havepads) {
    $msglen = length($body);
    for ($count = 0 ; (($count < (@padlength -1)) && !$usepad) ; $count++) {
      if ($msglen < @padlength[$count]) {
        $usepad = $count;
        }
      }
    if (!$usepad) { $padlen = 0; }
    else { $padlen = @padlength[$usepad] - $msglen; }
    if ($debug > 0) {
      if ($usepad) {
        print "Selected pad size = @padlength[$usepad] characters\n";
        }
      else {
        print "Message is larger than maximum pad size.  ";
	print "Message will not be padded.\n";
        }
      print "Message will need $padlen characters of padding.\n";
      }
    @padchars = (a .. z, '+', '=', A .. Z, '/');
    $padlinelength = 65;
    $newpadlen = $padlen - length($padbegin) - length($padend);
    $numlines = int( $newpadlen / $padlinelength );
    $numextra = $padlen - ($numlines * $padlinelength);
    while ($numextra > $padlinelength) {                ## because of a bug??
      $numextra = $numextra - $padlinelength;
      ++$numlines;
      }
    
    push(@padding, $padbegin);
    foreach $i (0 .. $numlines) {
      foreach $j (0 .. ($padlinelength-1)) {
        @newpadchars[$j] = $padchars[rand @padchars];
        }
      push(@padding, join("", @newpadchars));
      }
    foreach $i (0..$numextra) {
      push(@extrachars, $padchars[rand @padchars]);
      }
    push(@padding, join("", @extrachars));
    push(@padding, $padend);
    $body = $body . join("\n", @padding);
    undef @padding;
    undef @extrachars;
    undef @padchars;
    }
  }

sub checkblocks {  ## checks to see if the message should be blocked
  $_ = $sender;
  /([^@\s]+)@(\S+)/; $senderlogin = $1; $sendermachine = $2;
  $sendermachine =~ tr/A-Z/a-z/;

  $_ = $addressee;
  /([^@\s]+)@(\S+)/; $addresseelogin = $1; $addresseemachine = $2;
  $addresseemachine =~ tr/A-Z/a-z/;

  foreach $_ (@sourceblock) {
    /([^@\s]+)@(\S+)/; $login = $1; $machine = $2;
    $machine =~ tr/A-Z/a-z/;
    if (($senderlogin eq $login) && ($sendermachine eq $machine)) {
      if ($debug > 0) {
	print "\nMessage was sent from $sender. This address\n";
	print "has been source blocked in the remailer's rc file.\n\n";
	}
      close(DEBUG);
      exit 0;
      }
    }

  foreach $_ (@destblock) {
    /([^@\s]+)@(\S+)/; $login = $1; $machine = $2;
    $machine =~ tr/A-Z/a-z/;
    if (($login eq $addresseelogin) && ($machine eq $addresseemachine)) {
      if ($debug > 0) {
	print "\nMessage was addressed to $addressee.  This address\n";
	print "has been destination blocked in the remailer's rc file.\n\n";
	}
      close(DEBUG);
      exit 0;
      }
    }

  foreach $_ (@netblock) {
    tr/A-Z/a-z/;
    $blocknet = $_;
    $_ = $addresseemachine;
    if (/\S*$blocknet/) {
      if ($debug > 0) {
	print "\nMessage was addressed to a user within $blocknet,\n";
	print "which has been blocked in the remailer's rc file.\n\n";
	}
      close(DEBUG);
      exit 0;
      }
    }
  }

sub printmessageflags {  ## prints command flags from message
  print "\nCommand flags in message:\n";
  if ($sender) { print "Sender:\t\t\t$sender\n"; }
    else { print "Sender:\t\tNOT FOUND!\n"; }
  if ($subject) { print "Subject:\t\t$subject\n"; }
    else { print "Subject:\t\tNO SUBJECT GIVEN\n"; }
  if ($addressee) { print "Request-Remailing-To:\t$addressee\n"; }
    else { print "Request-Remailing-To:\tNO ADDRESS GIVEN\n" }
  if ($encrypted) { print "Encrypted:\t\tYES\n"; }
    else { print "Encrypted:\t\tNO\n"; }
  if ($superencrypted) { print "SuperEncrypted:\tYES\n"; }
    else { print "SuperEncrypted:\tNO\n"; }
  if ($subjectenclosed) { print "SubjectEnclosed:\tYES\n"; }
    else { print "SubjectEnclosed:\tNO\n"; }
  if ($pleasedontpad) { print "PleaseDontPad:\t\tYES\n"; }
    else { print "PleaseDontPad:\t\tNO\n"; }
  if ($latmin) { print "Latency:\t\t$latmin minutes\n"; }
    elsif ($latrand) { print "Latency:\t\trandom number of minutes\n"; }
    else { print "Latency:\t\tNONE\n"; }
  print "\n";
  }

## parses the RC file and gets the options
sub parsercfile {
  if (!(-e "$rcfilename")) {
    print DEBUG "ERROR: \"$rcfilename\" does not exist!\n";
    close(DEBUG);
    exit 0;
    }
  if (-z "$rcfilename") {
    print DEBUG "ERROR: \"$rcfilename\" is empty!\n";
    close(DEBUG);
    exit 0;
    }
  open(RCFILE, "$rcfilename");
  while (<RCFILE>) {
    chop $_;
    $1 = 0;
    if (/^\s*#/ || /^\s*$/) {;}  ## skip comments
    elsif (/^sourceblock\s+([\w!@,.]+)/) {
      push(@sourceblock, $1);
      }
    elsif (/^destblock\s+(\S+)/) {
      push(@destblock, $1);
      }
    elsif (/^netblock\s+(\S+)/) {
      push(@netblock, $1);
      }
    elsif (/^maxlatmin\s+(\d+)/) {
      $maxlatmin = $1;
      }
    elsif (/^home\s+(\S+)/) {
      $home = $1;
      }
    elsif (/^pgpcmd\s+(\S+)/) {
      $pgpcmd = $1;
      }
    elsif (/^pgppath\s+(\S+)/) {
      $pgppath = $1;
      }
    elsif (/^pgptempfile\s+(\S+)/) {
      $pgptempfile = "$1.$pid";
      }
    elsif (/^tempfile\s+(\S+)/) {
      $tempfile = "$1.$pid";
      }
    elsif (/^pubkey\s+(\S+)/) {
      $publickeyfilename = $1;
      }
    elsif (/^mbox\s+(\S+)/) {
      $mbox = $1;
      }
    elsif (/^loglevel\s+(\d)/) {
      $loglev = $1;
      }
    elsif (/^debugfile\s+(\S+)/) {
      $debugfile = "$1.$pid";
      }
    elsif (/^commandsfile\s+(\S+)/) {
      $commandsfile = $1;
      }
    elsif (/^logsynopsis\s+(\S+)/) {
      $logsynopsis = $1;
      }
    elsif (/^debuglevel\s+(\d)/) {
      $debug = $1;
      }
    elsif (/^padlen\s+(\d+)/) {
      $havepads = 1;
      @padlength[0] = 0;
      push(@padlength, $1);
      }
    elsif (/^sendmail\s+(\S+)/) {
      $sendmail = $1;
      }
    elsif (/^passphrase\s+(\S+[\S\s]*)/) {
      $passphrase = $1;
      }
    elsif (/^ownername\s+(\S+[\S\s]*)/) {
      $ownername = $1;
      }
    elsif (/^remailername\s+(\S+[\S\s]*)/) {
      $remailername = $1;
      }
    elsif (/^fromwho\s+(\S+[\S\s]*)/) {
      $fromwho = $1;
      }
    elsif (/^defaultsubject\s+(\S+[\S\s]*)/) {
      $defaultsubject = $1;
      $subject = $1;
      }
    elsif (/^comment\s+(\S+[\S\s]*)/) {
      push(@comments, $1);
      }
    elsif (/^infofile\s+(\S+)/) {
      $infofile = $1;
      }
    else {
      die "Error parsing \"$rcfilename\"\n  Line = \"$_\"";
      }
    }
  close(RCFILE);
  if ($debug > 0) {
    print "\nRC file options:\n";
    print "Debug level\t\t = $debug\n";
    print "Logging level\t\t = $loglev\n";
    print "Home Directory\t\t = $homedir\n";
    print "Logging mailbox\t\t = $logmbox\n";
    print "Logging stats file\t = $logstats\n";
    print "Logging numbers file\t = $logstats\n";
    print "Mailbox\t\t\t = $mbox\n";
    print "Tempfile\t\t = $tempfile\n";
    print "Information File\t\t = $infofile\n";
    print "Passphrase\t\t = $passphrase\n";
    print "Maximum Latency\t\t = $maxlatmin minutes.\n";
    if (!$havepads) {
      print "Message Padding\t\t = OFF\n";
      }
    else {
      print "Message Padding\t\t = ON\n";
      print "Pad lengths\t\t = @padlength[1] bytes\n";
      if (@padlength[2]) {
        foreach $i (2..(@padlength -1)) {
	  print "\t\t\t = @padlength[$i] bytes\n";
          }
        }
      }
    if (@comments[0]) {
      foreach $i (0..(@comments-1)) {
	print "X-Comment: @comments[$i]\n";
	}
      }
    print "\nSource blocked addresses:\n";
    foreach $_ (@sourceblock) {
      print "\t$_\n";
      }
    print "\nDestination blocked addresses:\n";
    foreach $_ (@destblock) {
      print "\t$_\n";
      }
    print "\nBlocked networks:\n";
    foreach $_ (@netblock) {
      print "\t$_\n";
      }
    print "\n";
    }
  }

sub sendinformation {
  ## This will send information about the remailer to the sender
  if (!$sender) { close(DEBUG); exit 0; }
  open(SENDMAIL, "|$sendmail $sender");
  select SENDMAIL;
  print "To: $sender\n";
  print "From: $fromwho\n";
  print "Subject: Remailer Information\n\n";
  print "\n\n";
&center("--------------------------------------------------------------------------");
  &center("Anonymous Remailer Information");
  &center("for\n");
  &center("$remailername\n");
  &center("$version");
&center("--------------------------------------------------------------------------");
  print "\n\n";
  if ($infofile) {
    open(INFO, "$infofile");
    @info = <INFO>;
    close(INFO);
    $info = join("", @info);
    undef @info;
    &center("The Remailer Operator has written this information message:");
    print "\n$info\n\n";
    }
  print "This remailer keeps level $loglev logs.  This means:\n";
  if ($loglev == 0) {
    print "\tIt keeps no logs whatsoever.\n\n";
    }
  elsif ($loglev == 1) {
    print "\tIt keeps a message synopsis, which includes the following:\n";
    print "\tDate (including time), What options the message requested,\n";
    print "\tWho the message was from and who it was sent to, the subject,\n";
    print "\tand the message size before padding.  It does NOT keep the\n";
    print "\tmessage body.\n\n";
    }
  print "Default subject:\t\"$defaultsubject\"\n";
  print "Maximum latency:\t$maxlatmin minutes.\n";
  print "Remailer's Public key:\n\n";
  open(KEY, "$publickeyfilename");
  @key = <KEY>;
  close(KEY);
  print join("", @key) . "\n\n";
  
&center("--------------------------------------------------------------------------");
&center("Remailer Commands");
&center("--------------------------------------------------------------------------");

  print "\n\n";
  open(COMMANDS, "$commandsfile");
  @commanddefs = <COMMANDS>;
  close(COMMANDS);
  print join("", @commanddefs);
  print "\n\n";

&center("--------------------------------------------------------------------------");
&center("Addresses and subnets with blocks attached to them");
&center("--------------------------------------------------------------------------");

  print "\nSource blockes:\n";
  foreach $i (@sourceblock) {
    print "\t$i\n";
    }
  print "\nDestination blockes:\n";
  foreach $i (@destblock) {
    print "\t$i\n";
    }
  print "\nSubnet blockes:\n";
  foreach $i (@netblock) {
    print "\t$i\n";
    }
  print "\n\n\t\t\t\t\tIn Liberty,";
  print "\n\n\n\n\t\t\t\t\t$ownername\n\n";
  close(DEBUG);
  close(SENDMAIL);
  exit 0;
  }

sub center {
  $line = @_[0];
  foreach $i (0..(int((70-length($line))/2))) {
    print " ";
    }
  print "$line\n";
  }

######################################################
