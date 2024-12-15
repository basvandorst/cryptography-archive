#!/usr/contrib/bin/perl
## Title: pks-extract-key.pl
## Copyright 1994-1995 by Brian A. LaMacchia -- All Rights Reserved
## E-mail: bal@zurich.ai.mit.edu, bal@mit.edu
##
## $Header: /zu/bal/src/wwwpks/scripts/RCS/pks-extract-key.pl,v 1.1 1996/02/12 15:35:48 bal Exp $
##

require 'flush.pl';
do 'cgi-hacks.pl';

## First we define some variables we're going to need.
## PGP_BINARY is the absolute path of the PGP binary we will run
## PKS_EXTRACT_SCRIPT_URL is a URL that points to this script.  We need this so
##    particular, key-sepcific URL can be generated as arguments to this script.
## TEMP_FILE_PREFIX is the prefix of an absolute pathname to use as a temp file.
##    we append a period (.) and the PID to this prefix to make a unique filename.
## DISABLED_KEY_URL is a URL that points to the error message we should return
##    if a user tries to extract a disabled PGP key.

$PGP_BINARY = "/users/ftp/archive/users/bal/private/KeyServer/bin/pgp";
$PKS_EXTRACT_SCRIPT_URL = "http://www-swiss.ai.mit.edu/htbin/pks-extract-key.pl";
$TEMP_FILE_PREFIX = "/users/ftp/archive/users/bal/private/KeyServer/tmp/.pks-www";
$DISABLED_KEY_URL = "http://www-swiss.ai.mit.edu/~bal/disabled-key.html";

## Set the PGPPATH environment variable appropriately

$ENV{'PGPPATH'} = "/users/ftp/archive/users/bal/private/KeyServer/keyring";

open(S,">-");

## This script is called with GET, so the QUERY_STRING environment
## variable contains the goodies.

$query_source = $ENV{'QUERY_STRING'};
$string1 = "";

## encode_html encodes "+" characters as per the URL spec.

sub encode_html {
	local ($raw_text) = @_;
	for ($num = 0; $num < length($raw_text); $num++) {
		$nextchar = substr($raw_text,$num,1);
		if ($nextchar eq "+") {
			substr($raw_text,$num,1) = "%" . sprintf("%lx",ord($nextchar));
		}
	}
	$raw_text;
}

## This procedure decodes encoded HTML.  We have to filter out
## backquotes to prevent running commands on the host processor.

for ($num = 0; $num < length($query_source); $num++) {
	$nextchar = substr($query_source,$num,1);
	if ($nextchar eq "%") {
	    $nc1 = substr($query_source,++$num,1);
	    $nc2 = substr($query_source,++$num,1);
	    $tempchar = pack("c",hex($nc1.$nc2));
	    if ($tempchar ne "`") { $string1 .= $tempchar }
        } elsif ($nextchar eq "+") { $string1 .= " " }
	elsif ($nextchar eq "`") { }
	else { $string1 .= $nextchar; }
}

## Accept arguments in any order.

&find_form_args("op|search|fingerprint");
$command = $op;
$command =~ s/`//g;
$arg = $search;
$arg =~ s/`//g;

if ($command eq "index") { 
  print S "Content-type: text/html\n\n";
  print S "<title>Public Key Server -- Index $arg</title>\n";
  print S "<h1>Public Key Server -- Index ``$arg''</h1>\n";
  print S "<pre>\n";
  &flush(S);
  $temp_file = join(".",$TEMP_FILE_PREFIX,$$);
  if ($fingerprint eq "on") { $pgpargs = "-kvc"; } else { $pgpargs = "-kv" }
  system("$PGP_BINARY $pgpargs \"$arg\" 1>$temp_file 2>&1"); 
  open(foo,$temp_file);
  while (<foo>) {
    if (m/^pub@/) { 
      s#<(.*)>#&lt;<a href="$DISABLED_KEY_URL">$1</a>>#g;
      $keyid = substr($_,10,8);
      substr($_,10,8) = "<a href=\"$DISABLED_KEY_URL\">$keyid</a>";
    } else {
      if (m/<([^>]*)>/) {
        $href_string = $PKS_EXTRACT_SCRIPT_URL . "?" . &encode_html("op=get&search=$1");
        s#<([^>]*)>#&lt;<a href="$href_string">$1</a>>#g;
      } 
      if (m/^(pub|sig)/) { 
        $keyid = substr($_,10,8);
        substr($_,10,8) = "<a href=\"".$PKS_EXTRACT_SCRIPT_URL."?op=get&search=0x".$keyid."\">$keyid</a>";
      } 
    }
    if (m/^Key ring/) {
      $_ = "\n";
    }
    print;
  }
  print S "</pre>\n";
  system("rm $temp_file");
} elsif ($command eq "vindex") { 
  print S "Content-type: text/html\n\n";
  print S "<title>Public Key Server -- Verbose Index $arg</title>\n";
  print S "<h1>Public Key Server -- Verbose Index ``$arg''</h1>\n";
  print S "<pre>\n";
  &flush(S);
  $temp_file = join(".",$TEMP_FILE_PREFIX,$$);
  if ($fingerprint eq "on") { $pgpargs = "-kvvc"; } else { $pgpargs = "-kvv" }
  system("$PGP_BINARY $pgpargs \"$arg\" 1>$temp_file 2>&1"); 
  open(foo,$temp_file);
  while (<foo>) {
    if (m/^pub@/) { 
      s#<(.*)>#&lt;<a href="$DISABLED_KEY_URL">$1</a>>#g;
      $keyid = substr($_,10,8);
      substr($_,10,8) = "<a href=\"$DISABLED_KEY_URL\">$keyid</a>";
    } else {
      if (m/<([^>]*)>/) {
        $href_string = $PKS_EXTRACT_SCRIPT_URL . "?" . &encode_html("op=get&search=$1");
        s#<([^>]*)>#&lt;<a href="$href_string">$1</a>>#g;
      }
      if (m/^(pub|sig)/) { 
        $keyid = substr($_,10,8);
        substr($_,10,8) = "<a href=\"".$PKS_EXTRACT_SCRIPT_URL."?op=get&search=0x".$keyid."\">$keyid</a>";
      }
    }
    if (m/^Key ring/) {
      $_ = "\n";
    }
    print;
  }
  print S "</pre>\n";
  system("rm $temp_file");
} elsif ($command eq "get") { 
  print S "Content-type: text/html\n\n";
  print S "<title>Public Key Server -- GET $arg</title>\n";
  print S "<h1>Public Key Server -- GET ``$arg''</h1>\n";
  print S "<pre>\n";
  
  &flush(S);
  $temp_file = join(".",$TEMP_FILE_PREFIX,$$,"asc");
  system("$PGP_BINARY -kxa \"$arg\" $temp_file 1>/dev/null 2>&1"); 
  open(foo,"$temp_file");
  while (<foo>) {
     s/</&lt;/g;
     print;
  }
  print S "</pre>\n";
  system("rm $temp_file");
} else {
  print "Content-type: text/html\n\n";
  print "<title>Public Key Server</title>\n";
  print "<h1>Public Key Server</h1>\n";
  print "Your request <b>failed</b>.<p>\n";
  print "The server could not parse the QUERY_STRING: $query_source\n<p>";
  print "Please report this failure to the address below.  Include the QUERY_STRING\n";
  print "that failed, along with which WWW client you are using.\n<p>";
  print "<a href=\"/~bal/pks-commands.html\">Click here to return to the command page.</a><p>\n";
  print "<hr><address><a href=\"/~bal/bal-home.html\">Brian A. LaMacchia</a> <br>\n";
  print "bal\@martigny.ai.mit.edu <br>\n";
  print "NE43-431 <br>\n";
  print "545 Technology Square <br>\n";
  print "Cambridge, MA 02139 <br>\n";
  print "(617) 253-0290 \n";
  print "</address>\n";
}

close(S);
