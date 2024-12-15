#!/usr/bin/perl
#
# make-mkhdrs - Creates a batch file from the Makefile.in files which
# sets up the header file directories for Windows 95.
#
# Copyright (C) 1997 Pretty Good Privacy, Inc. All rights reserved.
#
# $Id: make-mkhdrs.pl,v 1.1.2.2 1997/06/12 01:22:37 mhw Exp $
#

open(OUT, ">mkhdrs.bat") || die;

print OUT <<'END';
copy config_win32.h config.h
if not exist include mkdir include
if not exist lib\include mkdir lib\include
END

open (FILELIST, "find . -name Makefile.in -print |") || die;
while ($file = <FILELIST>)
{
    chop $file;
    $dir = $file;
    $dir =~ s|/[^/]+$||;
    $dir =~ s|^\./||;
    $dir =~ s|/|\\|g;
    $files = "";
    open (FILE, "<$file") || die;
    while (<FILE>)
    {
	chop;
	if (/^(SHARED|PRIV)HDRS\s*=\s*(.*)$/)
	{
	    $dest = ($1 eq "SHARED") ? "include" : "lib\\include";
	    $files = $2;
	    while ($files =~ s/\\$// && ($_ = <FILE>))
	    {
		chop;
		$files .= " " . $_;
	    }
	    $files =~ s|/|\\|g;
	    foreach (split(/\s+/, $files))
	    {
		next if /^$/;
		print OUT "copy $dir\\$_ $dest\n";
	    }
	}
    }
    close (FILE);
}
close (FILELIST) || die;
close (OUT) || die;
