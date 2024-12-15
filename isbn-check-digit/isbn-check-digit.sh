#!/bin/sh
# From msuinfo!agate!howland.reston.ans.net!torn!watserv2.uwaterloo.ca!undergrad.math.uwaterloo.ca!calum!r-ridge Thu Jul 21 12:19:51 1994
# Newsgroups: alt.sources
# Path: msuinfo!agate!howland.reston.ans.net!torn!watserv2.uwaterloo.ca!undergrad.math.uwaterloo.ca!calum!r-ridge
# From: r-ridge@calum.csclub.uwaterloo.ca (Ross Ridge)
# Subject: Re: ISBN Check Digit, !Perl version
# Message-ID: <Ct9M6n.EKt@undergrad.math.uwaterloo.ca>
# Sender: news@undergrad.math.uwaterloo.ca (news spool owner)
# Organization: University of Waterloo
# References: <1994Jul14.142057.2642@aaf.alcatel.at>
# Date: Thu, 21 Jul 1994 00:47:11 GMT
# Lines: 1
# 
echo "1sa`echo $1|tr -cd '0-9'`[d10%la+sa10/d0<b]dsbxla11%P"|dc|tr '\001-\014' 0-9X; echo

