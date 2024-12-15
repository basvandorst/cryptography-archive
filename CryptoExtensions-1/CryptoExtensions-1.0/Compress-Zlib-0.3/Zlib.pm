# File	  : Zlib.pm
# Author  : Paul Marquess
# Created : 2nd October 1995
# Version : 0.3
#
#     Copyright (c) 1995 Paul Marquess. All rights reserved.
#     This program is free software; you can redistribute it and/or
#     modify it under the same terms as Perl itself.
#

package Compress::Zlib;

use Carp ;

require 5.002 ;
require Exporter;
require DynaLoader;
require AutoLoader;

@ISA = qw(Exporter DynaLoader);
# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.
@EXPORT = qw(
	deflateInit inflateInit

	compress uncompress

	gzip gunzip

	gzopen $gzerrno

	adler32 crc32

	ZLIB_VERSION

        DEFLATED
        DEF_MEM_LEVEL
        DEF_WBITS
	MAX_WBITS
        OS_CODE

	Z_ASCII
	Z_BEST_COMPRESSION
	Z_BEST_SPEED
	Z_BINARY
	Z_BUF_ERROR
	Z_DATA_ERROR
	Z_DEFAULT_COMPRESSION
	Z_DEFAULT_STRATEGY
	Z_ERRNO
	Z_FILTERED
	Z_FINISH
	Z_FULL_FLUSH
	Z_HUFFMAN_ONLY
	Z_MEM_ERROR
	Z_NO_FLUSH
	Z_NULL
	Z_OK
	Z_PARTIAL_FLUSH
	Z_STREAM_END
	Z_STREAM_ERROR
	Z_SYNC_FLUSH
	Z_UNKNOWN
);


$VERSION = 0.3 ;


sub AUTOLOAD {
    # This AUTOLOAD is used to 'autoload' constants from the constant()
    # XS function.  If a constant is not found then control is passed
    # to the AUTOLOAD in AutoLoader.

    local($constname);
    ($constname = $AUTOLOAD) =~ s/.*:://;
    $val = constant($constname, @_ ? $_[0] : 0);
    if ($! != 0) {
	if ($! =~ /Invalid/) {
	    $AutoLoader::AUTOLOAD = $AUTOLOAD;
	    goto &AutoLoader::AUTOLOAD;
	}
	else {
	    ($pack,$file,$line) = caller;
	    die "Your vendor has not defined Compress::Zlib macro $constname, used at $file line $line.
";
	}
    }
    eval "sub $AUTOLOAD { $val }";
    goto &$AUTOLOAD;
}

bootstrap Compress::Zlib $VERSION ;

# Preloaded methods go here.

sub ParseParameters
{
    my($ref, $default) = @_ ;
    my(%got) = %$default ;
    my (@Bad) ;

    croak "2nd parameter is not a reference to a hash"
        unless ref $ref eq "HASH" ;

    while (($key, $value) = each %$ref)
    {
        if (defined $default->{$key})
          { $got{$key} = $value }
        else
	  { push (@Bad, $key) }
    }
    
    if (@Bad) {
        my ($bad) = join(", ", @Bad) ;
        croak "$subname: unknown key value(s) @Bad" ;
    }

    \%got ;
}

$deflateDefault = {
	Level	   =>	Z_DEFAULT_COMPRESSION(),
	Method	   =>	DEFLATED(),
	WindowBits =>	MAX_WBITS(),
	MemLevel   =>	DEF_MEM_LEVEL(),
	Strategy   =>	Z_DEFAULT_STRATEGY(),
	Bufsize    =>	4096,
	} ;

$inflateDefault = {
	WindowBits =>	DEF_WBITS(),
	Bufsize    =>	4096,
	} ;


sub deflateInit
{
    croak "Usage: deflateInit([,{ } ])"
	unless @_ == 1 or @_ == 0 ;

    my($ref) = @_ ;
    $ref = {} unless $ref ;

    my ($got) = ParseParameters($ref, $deflateDefault) ;
    _deflateInit($got->{Level}, $got->{Method}, $got->{WindowBits}, 
		$got->{MemLevel}, $got->{Strategy}, $got->{Bufsize}) ;
		
}

sub inflateInit
{
    croak "Usage: inflateInit([,{ } ])"
	unless @_ == 1 or @_ == 0 ;

    my($output, $ref) = @_ ;
    $ref = {} unless $ref ;
 
    my ($got) = ParseParameters($ref, $inflateDefault) ;
    _inflateInit($got->{WindowBits}, $got->{Bufsize}) ;
 
}

sub compress
{
    croak 'Usage: compress($buffer)'
        unless @_ == 1 ;

    my ($x, $output, $out, $err) ;

    if ( (($x, $err) = deflateInit())[1] == &Z_OK) {

        ($output, $err) = $x->deflate($_[0]) ;
        return undef unless $err == &Z_OK ;

        ($out, $err) = $x->flush() ;
        return undef unless $err == &Z_OK ;
    
        return ($output . $out) ;

    }

    return undef ;
}


sub uncompress
{
    croak 'Usage: uncompress($buffer)'
        unless @_ == 1 ;

    my ($x, $output, $err) ;

    if ( (($x, $err) = inflateInit())[1] == &Z_OK)  {
 
        ($output, $err) = $x->inflate($_[0]) ;
        return undef unless $err == &Z_STREAM_END ;
 
	return $output ;
    }
 
    return undef ;
}

1 ;
# Autoload methods go after __END__, and are processed by the autosplit program.

1;
__END__


=cut

=head1 NAME

Compress::Zlib - Interface to zlib compression library

=head1 SYNOPSIS

    use Compress::Zlib ;

    ($d, $status) = deflateInit() ;
    ($out, $status) = $d->deflate($buffer) ;
    ($out, $status) = $d->flush() ;

    ($i, $status) = inflateInit() ;
    ($out, $status) = $i->inflate($buffer) ;

    $dest = compress($source) ;
    $dest = uncompress($source) ;

    $gz = gzopen($filename, $mode) ;
    $status = $gz->gzread($buffer [,$size]) ;
    $status = $gz->gzreadline($line) ;
    $status = $gz->gzwrite($buffer) ;
    $status = $gz->gzflush($flush) ;
    $status = $gz->gzclose() ;
    $errstring = $gz->gzerror() ; 
    $gzerrno

    $crc = adler32($buffer [,$crc]) ;
    $crc = crc32($buffer [,$crc]) ;

    ZLIB_VERSION

=head1 DESCRIPTION

The I<Compress::Zlib> module provides a Perl interface to the I<zlib>
compression library. Most of the functionality provided by I<zlib> is
available for use in I<Compress::Zlib>.

The module actually can be split into two general areas of
functionality, namely in-memory compression/decompression and
read/write access to I<gzip> files. Each of these areas will be
discussed separately below.


=head1 DEFLATE 

The interface I<Compress::Zlib> provides to the in-memory I<deflate>
(and I<inflate>) functions has been modified to fit into a Perl model.

For both inflation and deflation, the Perl interface will I<always>
consume the complete input buffer before returning. Also the output
buffer returned will be automatically grown to fit the amount of output
available.

Here is a definition of the interface available:


=head2 ($d, $status) = deflateInit()

Initialises a deflation stream. 

It combines the features of both I<zlib> functions B<deflateInit> and
B<deflateInit2>.

If successful, it will return the initialised deflation stream, B<$d>
and B<$status> of C<Z_OK> in a list context. In scalar context it
returns the deflation stream, B<$d>, only.

If not successful, the returned deflation stream (B<$d>) will be
I<undef> and B<$status> will hold the exact I<zlib> error code.

The function takes one optional parameter, a reference to a hash.  The
contents of the hash allow the deflation interface to be tailored.

Below is a list of the valid keys that the hash can take.


=over 5

=item B<Level>

Defines the compression level. Valid values are 1 through 9,
C<Z_BEST_SPEED>, C<Z_BEST_COMPRESSION>, and C<Z_DEFAULT_COMPRESSION>.

The default is C<Z_DEFAULT_COMPRESSION>.

=item B<Method>

Defines the compression method. The only valid value at present (and
the default) is C<DEFLATED>.

=item B<WindowBits>

For a definition of the meaning and valid values for B<WindowBits>
refer to the I<zlib> documentation for I<deflateInit2>.

Defaults to C<MAX_WBITS>.

=item B<MemLevel>

For a definition of the meaning and valid values for B<MemLevel>
refer to the I<zlib> documentation for I<deflateInit2>.

Defaults to C<DEF_MEM_LEVEL>.

=item B<Strategy>

Defines the strategy used to tune the compression. The valid values are
C<Z_DEFAULT_STRATEGY>, C<Z_FILTERED> and C<Z_HUFFMAN_ONLY>. 

The default is C<Z_DEFAULT_STRATEGY>.

=item B<Bufsize>

Sets the initial size for the deflation buffer. If the buffer has to be
reallocated to increase the size, it will grow in increments of
B<Bufsize>.

The default is 4096.

=back

Here is an example of using the B<deflateInit> optional parameter to
override the default buffer size and compression level.

    deflateInit( {Bufsize => 300, Level => Z_BEST_SPEED } ) ;


=head2 ($out, $status) = $d->deflate($buffer)


Deflates the contents of B<$buffer>. When finished, B<$buffer> will be
completely processed (assuming there were no errors). If the deflation
was successful it returns the deflated output, B<$out>, and a status
value, B<$status>, of C<Z_OK>.

On error, B<$out> will be I<undef> and B<$status> will contain the
I<zlib> error code.

In a scalar context B<deflate> will return B<$out> only.

As with the I<deflate> function in I<zlib>, it is not necessarily the
case that any output will be produced by this method. So don't rely on
the fact that B<$out> is empty for an error test.


=head2 ($out, $status) = $d->flush()

Finishes the deflation. Any pending output will be returned via B<$out>.
B<$status> will have a value C<Z_OK> if successful.

In a scalar context B<flush> will return B<$out> only.

Note that flushing can degrade the compression ratio, so it should only
be used to terminate a decompression.



=head2 Example


Here is a trivial example of using B<deflate>. It simply reads standard
input, deflates it and writes it to standard output.

    use Compress::Zlib ;

    $x = deflateInit()
       or die "Cannot create a deflation stream\n" ;

    while (<>)
    {
        ($output, $status) = $x->deflate($_) ;
    
        $status == Z_OK
            or die "deflation failed\n" ;

        print $output ;
    }

    ($output, $status) = $x->flush() ;

    $status == Z_OK
        or die "deflation failed\n" ;

    print $output ;


=head1 INFLATE

Here is a definition of the interface:


=head2 ($i, $status) = inflateInit()

Initialises an inflation stream. 

In a list context it returns the inflation stream, B<$i>, and the
I<zlib> status code (B<$status>). In a scalar context it returns the
inflation stream only.

If successful, B<$i> will hold the inflation stream and B<$status> will
be C<Z_OK>.

If not successful, B<$i> will be I<undef> and B<$status> will hold the
I<zlib> error code.

The function takes one optional parameter, a reference to a hash.  The
contents of the hash allow the inflation interface to be tailored.

Below is a list of the valid keys that the hash can take.

=over 5

=item B<WindowBits>

For a definition of the meaning and valid values for B<WindowBits>
refer to the I<zlib> documentation for I<inflateInit2>.

Defaults to C<MAX_WBITS>.

=item B<Bufsize>

Sets the initial size for the inflation buffer. If the buffer has to be
reallocated to increase the size, it will grow in increments of
B<Bufsize>. 

Default is 4096.

=back

Here is an example of using the B<inflateInit> optional parameter to
override the default buffer size.

    deflateInit( {Bufsize => 300 } ) ;

=head2 ($out, $status) = $i->inflate($buffer)

Inflates the complete contents of B<$buffer> 

Returns C<Z_OK> if successful and C<Z_STREAM_END> if the end of the
compressed data has been reached.


=head2 Example

Here is an example of using B<inflate>.

    use Compress::Zlib ;

    $x = inflateInit()
       or die "Cannot create a inflation stream\n" ;

    while ($size = read(STDIN, $input, 4096))
    {
        ($output, $status) = $x->inflate($input) ;

        print $output 
            if $status == Z_OK or $status == Z_STREAM_END ;

        last if $status != Z_OK ;
    }

    die "inflation failed\n"
        unless $status == Z_STREAM_END ;

=head1 COMPRESS/UNCOMPRESS

Two high-level functions are provided by I<zlib> to perform in-memory
compression. They are B<compress> and B<uncompress>. Two Perl subs are
provided which provide similar functionality.

=over 5

=item B<$dest = compress($source) ;>

Compresses B<$source>. If successful it returns the
compressed data. Otherwise it returns I<undef>.

=item B<$dest = uncompress($source) ;>

Uncompresses B<$source>. If successful it returns the uncompressed
data. Otherwise it returns I<undef>.

=back

=head1 GZIP INTERFACE

A number of functions are supplied in I<zlib> for reading and writing
I<gzip> files. This module provides an interface to most of them. In
general the interface provided by this module operates identically to
the functions provided by I<zlib>. Any differences are explained
below.

=over 5

=item B<$gz = gzopen(filename, mode)>

This function operates identically to the I<zlib> equivalent except
that it returns an object which is used to access the other I<gzip>
methods.

As with the I<zlib> equivalent, the B<mode> parameter is used to
specify both whether the file is opened for reading or writing and to
optionally specify a a compression level. Refer to the I<zlib>
documentation for the exact format of the B<mode> parameter.

=item B<$status = $gz-E<gt>gzread($buffer [, $size]) ;>

Reads B<$size> bytes from the compressed file into B<$buffer>. If
B<$size> is not specified, it will default to 4096. If the scalar
B<$buffer> is not large enough, it will be extended automatically.

=item B<$status = $gz-E<gt>gzreadline($line) ;>

Reads the next line from the compressed file into B<$line>.

It is legal to intermix calls to B<gzread> and B<gzreadline>.

At this time B<gzreadline> ignores the variable C<$/>
(C<$INPUT_RECORD_SEPARATOR> or C<$RS> when C<English> is in use). The
end of a line is denoted by the C character C<'\n'>.

=item B<$status = $gz-E<gt>gzwrite($buffer) ;>

Writes the contents of B<$buffer> to the compressed file.

=item B<$status = $gz-E<gt>gzflush($flush) ;>

Flushes all pending output into the compressed file.
Works identically to the I<zlib> function it interfaces to. Note that
the use of B<gzflush> can degrade compression.

Refer to the I<zlib> documentation for the valid values of B<$flush>.

=item B<$gz-E<gt>gzclose>

Closes the compressed file. Any pending data is flushed to the file
before it is closed.

=item B<$gz-E<gt>gzerror>

Returns the I<zlib> error message or number for the last operation
associated with B<$gz>. The return value will be the I<zlib> error
number when used in a numeric context and the I<zlib> error message
when used in a string context. The I<zlib> error number constants,
shown below, are available for use.

    Z_OK
    Z_STREAM_END
    Z_ERRNO
    Z_STREAM_ERROR
    Z_DATA_ERROR
    Z_MEM_ERROR
    Z_BUF_ERROR

=item B<$gzerrno>

The B<$gzerrno> scalar holds the error code associated with the most
recent I<gzip> routine. Note that unlike B<gzerror()>, the error is
I<not> associated with a particular file.

As with B<gzerror()> it returns an error number in numeric context and
an error message in string context. Unlike B<gzerror()> though, the
error message will correspond to the I<zlib> message when the error is
associated with I<zlib> itself, or the UNIX error message when it is
not (i.e. I<zlib> returned C<Z_ERRORNO>).

As there is an overlap between the error numbers used by I<zlib> and
UNIX, B<$gzerrno> should only be used to check for the presence of
I<an> error in numeric context. Use B<gzerror()> to check for specific
I<zlib> errors. The I<gzcat> example below shows how the variable can
be used safely.

=back


=head2 Examples

Here is an example script which uses the interface. It implements a
I<gzcat> function.

    use Compress::Zlib ;

    die "Usage: gzcat file...\n"
	unless @ARGV ;

    foreach $file (@ARGV) {
        $gz = gzopen($file, "rb") 
	    or die "Cannot open $file: $gzerrno\n" ;

        print $buffer 
            while $gz->gzread($buffer) > 0 ;
        die "Error reading from $file: $gzerrno\n" if $gzerrno ;
    
        $gz->gzclose() ;
    }

Below is a script which makes use of B<gzreadline>. It implements a
simple very I<grep> like script.

    use Compress::Zlib ;

    die "Usage: gzgrep pattern file...\n"
        unless @ARGV >= 2;

    $pattern = shift ;

    foreach $file (@ARGV) {
        $gz = gzopen($file, "rb") 
             or die "Cannot open $file: $gzerrno\n" ;
    
        while ($gz->gzreadline($_) > 0) {
            print if /$pattern/ ;
        }
    
        die "Error reading from $file: $gzerrno\n" if $gzerrno ;
    
        $gz->gzclose() ;
    }


=head1 CHECKSUM FUNCTIONS

Two functions are provided by I<zlib> to calculate a checksum. For the
Perl interface, the order of the two parameters in both functions has
been reversed. This allows both running checksums and one off
calculations to be done.

    $crc = adler32($buffer [,$crc]) ;
    $crc = crc32($buffer [,$crc]) ;

=head1 CONSTANTS

All the I<zlib> constants are automatically imported when you make use
of I<Compress::Zlib>.

=head1 AUTHOR

The I<Compress::Zlib> module was written by Paul Marquess,
F<pmarquess@bfsec.bt.co.uk>.

The I<zlib> compression library was written by Jean-loup Gailly
F<gzip@prep.ai.mit.edu> and Mark Adler F<madler@alumni.caltech.edu>.

=head1 MODIFICATION HISTORY

=head2 0.1 

First public release of I<Compress::Zlib>.

Paul Marquess, 2nd October 1995.

=head2 0.2 

Fixed a minor allocation problem in Zlib.xs

Paul Marquess, 5th October 1995.

=head2 0.3 

Paul Marquess, 12th October 1995.
