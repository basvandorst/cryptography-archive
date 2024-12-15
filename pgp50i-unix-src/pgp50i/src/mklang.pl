#!/usr/bin/perl
#
#This could probably be sed, but I'm feeling lazy.
#

$LANGUAGE_FILE = "language50.txt";
$HEADER_FILE = "lib/ttyui/language.h";

#In the future, this should call the routine to add the pgpErr.h strings to
#the bottom of the language50.txt file, here.

#Now, we want to turn the language50.txt file into lib/ttyui/language.h file.

if(open(INF, $LANGUAGE_FILE))
{
    if(open(OUF, ">$HEADER_FILE"))
    {
	print OUF "/*Automatic translation of $LANGUAGE_FILE.\n";
	print OUF " *This file is created by mklang.pl.  DO NOT EDIT!\n";
	print OUF " */\n\n";
	print OUF "static const char *lang = \"\\\n";

	while(<INF>)
	{
	    chop;

	    #Escape any single backslashes:
	    s/\\/\\\\/g;

	    #Escape any quotes:
	    s/"/\\"/g;

	    print OUF "$_\\n\\" . "\n";
	}
	print OUF "\";\n";
	close(OUF);
    }
    else
    {
	print STDERR "Unable to open $HEADER_FILE for reading!\n";
    }
    close(INF);
}
else
{
    print STDERR "Unable to open $LANGUAGE_FILE for reading!\n";
}
