#!/usr/contrib/bin/perl
## Title: pks-add-key.pl
## Copyright 1994 by Brian A. LaMacchia -- All Rights Reserved
## E-mail: bal@zurich.ai.mit.edu, bal@mit.edu
##
## $Header: /zu/bal/src/wwwpks/scripts/RCS/pks-add-key.pl,v 1.1 1996/02/12 15:35:14 bal Exp $
##

require 'flush.pl';

## First we define some variables we're going to need.
## PGP_MAIL_PROCESSOR is the script that incoming mail to the
##   keyserver goes through first
## PGP_SUBMIT_ADDRESS is the e-mail address of the keyserver

$PGP_MAIL_PROCESSOR = "/users/ftp/archive/users/bal/private/KeyServer/bin/pgp_mrec /users/ftp/archive/users/bal/private/KeyServer/bin/pgp_mrec.cf";
$PGP_SUBMIT_ADDRESS = "pgp-public-keys\@pgp.ai.mit.edu";

## unencode_html decodes encoded HTML (%xx stuff).

sub unencode_html {
	local ($raw_text) = @_;
	$return_text = "";
	for ($num = 0; $num < length($raw_text); $num++) {
		$nextchar = substr($raw_text,$num,1);
		if ($nextchar eq "%") {
		    $nc1 = substr($raw_text,++$num,1);
		    $nc2 = substr($raw_text,++$num,1);
	            $return_text .= pack("c",hex($nc1.$nc2));
        	} elsif ($nextchar eq "+") { $return_text .= " " }
		else { $return_text .= $nextchar; }
	}
	$return_text;
}

## This script is called with POST method.

## We used to be able to say:
##   $query_source = <STDIN>;
## but this isn't correct, since CONTENT_LENGTH determines how many
## bytes to read, not the presence of CRLF.  In fact, Netscape doesn't put
## a CRLF on the end of POST requests...

$query_source = "";
for ($i = 0; $i < $ENV{'CONTENT_LENGTH'}; $i++) {
     $query_source .= getc;
}

## make sure to accept arguments in either order.  Order is not specified in the forms spec.

if ((($return_address, $arg) = ($query_source =~ m/search=(.*)&keytext=(.*)/)) ||
    (($arg, $return_address) = ($query_source =~ m/keytext=(.*)&search=(.*)/))) {
	$keytext = &unencode_html($arg);
	$return_address = &unencode_html($return_address);
## 
## CR/LF/CR-LF problems: In e-mail, mail that ends up on Unix systems
## always uses a bare linefeed (LF) to terminate lines.  With the WWW,
## though, some when sending the contents of a <TEXTAREA> field in forms
## end-of-line is denoted by CR-LF or just a CR.  We need to trap these
## here and fix them.  The first line, s/\015\012/\012/g, globally
## converts CRLF->LF.  The second line, tr/\015/\012/, changes any
## remaining CRs to LFs.
##
	$keytext =~ s/\015\012/\012/g;
	$keytext =~ tr/\015/\012/;
	if ($keytext =~ m/BEGIN PGP PUBLIC KEY BLOCK/) {
		open (BP,"|$PGP_MAIL_PROCESSOR");
		print BP "From: $return_address\n";
		print BP "To: $PGP_SUBMIT_ADDRESS\n";
		print BP "Subject: Add\n\n";
		print BP $keytext;
		print BP "\n";
		close(BP);

		print "Content-type: text/html\n\n";
		print "<title>Public Key Server -- Add</title>\n";
		print "<h1>Public Key Server -- Add</h1>\n";
		print "Your <b>ADD</b> request was successful.<p>\n";
		print "Confirmation will be mailed to: $return_address\n<p>";
		print "<a href=\"/~bal/pks-commands.html\">Click here to return to the command page.</a><p>\n";
		print "<hr><address><a href=\"/~bal/bal-home.html\">Brian A. LaMacchia</a> <br>\n";
		print "bal\@martigny.ai.mit.edu <br>\n";
		print "NE43-431 <br>\n";
		print "545 Technology Square <br>\n";
		print "Cambridge, MA 02139 <br>\n";
		print "(617) 253-0290 \n";
		print "</address>\n";
	} else {
		print "Content-type: text/html\n\n";
		print "<title>Public Key Server -- Add</title>\n";
		print "<h1>Public Key Server -- Add</h1>\n";
		print "Your <b>ADD</b> request <b>failed</b>.<p>\n";
		print "I couldn't find the start of an ASCII-armored PGP Public Key Block.\n<p>";
		print "<a href=\"/~bal/pks-commands.html\">Click here to return to the command page.</a><p>\n";
		print "<hr><address><a href=\"/~bal/bal-home.html\">Brian A. LaMacchia</a> <br>\n";
		print "bal\@martigny.ai.mit.edu <br>\n";
		print "NE43-431 <br>\n";
		print "545 Technology Square <br>\n";
		print "Cambridge, MA 02139 <br>\n";
		print "(617) 253-0290 \n";
		print "</address>\n";
	}	
} else {
	print "Content-type: text/html\n\n";
	print "<title>Public Key Server -- Add</title>\n";
	print "<h1>Public Key Server -- Add</h1>\n";
	print "Your <b>ADD</b> request <b>failed</b>.<p>\n";
	print "I couldn't parse QUERY_SOURCE: $query_source.<p>\n";
	print "Please report this failure to the address below.\n<p>";
	print "<a href=\"/~bal/pks-commands.html\">Click here to return to the command page.</a><p>\n";
	print "<hr><address><a href=\"/~bal/bal-home.html\">Brian A. LaMacchia</a> <br>\n";
	print "bal\@martigny.ai.mit.edu <br>\n";
	print "NE43-431 <br>\n";
	print "545 Technology Square <br>\n";
	print "Cambridge, MA 02139 <br>\n";
	print "(617) 253-0290 \n";
	print "</address>\n";
}


