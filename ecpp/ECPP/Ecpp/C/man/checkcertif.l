.TH CHECKCERTIF 1
.SH NAME
checkcertif \- is a program for checking certificates produced by the 
.I runecpp
program.
.SH SYNOPSIS
.B checkcertif
[
.B \-v
] [
.B \-q
] [
.B \-d\fP\^\fInumber\fP
] [
.B \-f\fP\^\fIfile\fP
]
.SH DESCRIPTION
.I Checkcertif
is a program that checks the certificates generated by the runecpp
program using the -Fc option.
.SH OPTIONS
.IP \fB\-q\fR 10
Quiet mode. This is the default.
.IP \fB\-v\fR 10
Verbose mode.
.PP
.IP \fB\-f\fP\^\fIfile\fP\fR 10
uses
.I file
as input file for the program. See the directory Ecpp/C/Ex for some
examples.
.PP
.SH DEBUG OPTIONS
.IP \fB\-d\fP\^\fInumber\fP\fR 10
sets debuglevel to
.I number.
Cannot be used if the programs are not compiled with the -DDEBUG option.
.SH EXAMPLES
.IP
.nf
% checkcertif -v -fcert
.fi
.LP
Checks the certificates contained in
.I file.

