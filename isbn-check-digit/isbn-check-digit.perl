From msuinfo!agate!ihnp4.ucsd.edu!usc!math.ohio-state.edu!jussieu.fr!univ-lyon1.fr!swidir.switch.ch!newsfeed.ACO.net!Austria.EU.net!hp4at!rcvie!aaf.aclcatel.at!wnp Thu Jul 14 09:04:07 1994
Newsgroups: alt.sources
Path: msuinfo!agate!ihnp4.ucsd.edu!usc!math.ohio-state.edu!jussieu.fr!univ-lyon1.fr!swidir.switch.ch!newsfeed.ACO.net!Austria.EU.net!hp4at!rcvie!aaf.aclcatel.at!wnp
From: wnp@rcvie.co.at (Wolf Paul)
Subject: ISBN Check Digit, Perl version
Message-ID: <1994Jul14.142057.2642@aaf.alcatel.at>
Keywords:  isbn
Sender: news@aaf.alcatel.at
Nntp-Posting-Host: rcsw55
Reply-To: Wolf.Paul@AAF.Alcatel.AT
Organization: Alcatel Austria Research Center, Vienna, Austria
Submitted-By: wnp@doulos.or.at (Wolf N. Paul)
Date: Thu, 14 Jul 1994 14:20:57 GMT
Archive-Name: isbn.pl
Lines: 88

I found Russell Schulz's isbn.pas program interesting.
Since I do not have a Pascal compiler or interpreter, I decided to
reimplement it in Perl, with a few minor variations in the input and
output formats.

Maybe someone finds it useful.

--- cut here, start ---
#!/usr/bin/perl
# ^^^^^^^^^^^^^ you may have to adjust this!

#
# Given an ISBN number with or without the final check digit
# this program will calculate the correct check digit and 
# either print the completed ISBN, or report the error.
#
# Inspired by a PASCAL program by Russell Schulz posted to alt.sources.
#
# ISBN Check Digit algorithm:
#
# Add up the first nine digits and divide by 11.
# The Remainder is the check digit, written as 'X' if equal 10.
#
# Wolf Paul, wnp@doulos.or.at, 1994-07-14
#

$ISBN=$ARGV[0] ;

$ISBN =~ s/-//g ; # get rid of dashes
$ISBN =~ s/\s//g ; # get rid of whitespace

$Length = length($ISBN) ;

if ( $Length eq 10 ) { $check = 1 ; }

if ( $Length < 9 ) { die "You must give at least the first 9 digits!\n" ; }


# Convert the ISBN to an array
$ISBN =~ s/(.)/$1 /g ;
@digits = split(/ /,$ISBN,$Length) ;
$ISBN =~ s/ //g ;

# Check for digits only in first nine digits
foreach $d ( 0 .. 8 )
{
	die "Only digits are allowed in first nine positions of an ISBN\n")
		unless $d =~ /[0-9] ;
}

# Store the Check Digit for later comparison
if ( $check ) { $Checkdigit = $digits[9] ;}

# Now calculate the Check Digit
$x = 0 ;
foreach $i ( 1 .. 9 )
{
	$x = $x + ( $i * $digits[$i-1]) ;
}
$x = $x%11 ;
if ( $x eq 10 ) { $x = 'X' ; }

# Now print our results:
# Completed number if check digit was not given
print "The complete ISBN number should be $ISBN$x.\n" unless $check ;

# Or a diagnostic report if the check digit was given
if ( $check )
{
	# the given check digit is correct, say so
	if ( $Checkdigit == $x )
	{
		print "The ISBN number $ISBN appears to be valid.\n" ;
	}
	# the given check digit is not correct, say so
	else
	{
		print "The ISBN number $ISBN appears to be INVALID.\n";
		print "The final digit should be $x, not $Checkdigit.\n" ;
		print "But of course the error could be earlier in the ISBN.\n" ;
	}
}
--- cut here, end ---
-- 
         V           Wolf N. Paul, UNIX Support/KSF     wnp@aaf.alcatel.at
+-----------------+  Alcatel Austria AG, Site "F"     +43-1-291-21-122 (w)
|  A L C A T E L  |  Ruthnergasse 1-7                 +43-1-292-1452 (fax)
+-----------------+  A-1210 Vienna-Austria/Europe       +43-1-220-6481 (h)

