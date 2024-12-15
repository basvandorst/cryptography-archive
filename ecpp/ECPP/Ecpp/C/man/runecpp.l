.TH RUNECPP 1
.SH NAME
runecpp \- is a general program for certifying prime numbers.
.SH SYNOPSIS
.B runecpp
[
.B \-h
] [
.B \-l\fP\^\fIpath\fP
] [
.B \-f\fP\^\fIfile\fP
] [
.B \-A\fP\^\fIfile\fP
] [
.B \-C\fP\^\fIfile\fP
] [
.B \-F\fP\^\fIformat\fP
] [
.B \-d\fP\^\fInumber\fP
] [
.B \-r\fP\^\fInumber\fP
] [
.B \-j
] [
.B \-q
] [
.B \-v
] [
.B \-N
] [
.B \-nm
] [
.B \-np
] [
.B \-nr
] [
.B \-cyc
] [
.B \-t\fP\^\fIfile\fP
] [
.B \-
] [
.B file
]
.SH DESCRIPTION
.I Runecpp
is a program using the theory of elliptic curves over finite fields to
prove the primality of positive integers. It can also produce
certificates that can be checked either by
.I checkcertif
or (if the -Fmaple option is used) by Maple.
.SH OPTIONS
.IP \fB\-h\fR 10
displays a short help.
.PP
.IP \fB\-l\fP\^\fIpath\fP\fR 10
indicates where the directory Data is to be found.
.PP
.IP \fB\-f\fP\^\fIfile\fP\fR 10
uses
.I file
as input file for the program. See the directory Ecpp/C/Ex for some examples. 
One can use the name - for input file, meaning standard input.
.PP
.IP \fB\-v\fR 10
verbose mode: prints a lot of messages to entertain the user. This is the
default mode.
.PP
.IP \fB\-q\fR 10
quiet option: nothing is printed. This is the opposite of -v.
.PP
.IP \fB\-Q\fR 10
hyperquiet: prints only a 1 or a 0 corresponding to the primality or
compositeness.
.PP
.IP \fB\-cyc\fR 10
cyclic: use only cyclic elliptic curves.
.PP
.IP \fB\-N\fR 10
no proof: do not perform the proving part of the algorithm.
.PP
.IP \fB\-nm\fR 10
no N-1 test: do not use the N-1 test.
.PP
.IP \fB\-np\fR 10
no N+1 test: do not use the N-1 test.
.PP
.IP \fB\-nr\fR 10
no Rabin test: do not perform a pseudoprimality test on the number.
.PP
.IP \fB\-j\fR 10
jump option: as soon as the first good discriminant is found, jump to the
computation of j mod p.
.PP
.SH CERTIFICATE OPTIONS
.IP \fB\-F\fP\^\fIformat\fP\fR 10
selects the format of the certificate among c, maple, mathematica. The
default format is c, to be used with the 
.I checkcertif
program.
.PP
.IP \fB\-A\fP\^\fIfile\fP\fR 10
appends the certificate to the file
.I file
.PP
.IP \fB\-C\fP\^\fIfile\fP\fR 10
puts the certificate in the file
.I file
.PP
.SH DEBUG OPTIONS
.IP \fB\-d\fP\^\fInumber\fP\fR 10
sets debuglevel to
.I number.
Cannot be used if the programs are not compiled with the -DDEBUG option.
.PP
.IP \fB\-r\fP\^\fInumber\fP\fR 10
sets the rank number of the first discriminant used for the first search at
.I number.
.PP
.IP \fB\-t\fP\^\fIfile\fP\fR 10
Using the file
.I file
, one can test that all the polynomials contained in 
Data/Weber are valid. See the file Ecpp/C/Test/pford.data for an example.
.SH EXAMPLES
.IP
.nf
% runecpp -l../ -f -
.fi
.LP
Use stdin as input file, the directory Data is ../Data.
.IP
.nf
% runecpp -l../ -Fc -Acert -q -f myfile
.fi
.LP
Tests the primality of all numbers contained in myfile and puts their 
primality certificate in cert. Moreover, do not print messages.
.SH FEATURES and BUGS
None yet :-)
.PP
When using the option -Fmathematica, the options -nm and -np must be given.
.SH AUTHOR
Francois Morain (27/08/91)
.SH "SEE ALSO"
A. O. L. Atkin and F. Morain
.I Elliptic curves and primality proving,
Math. Comp., 1991, to appear.
.PP
man checkcertif


