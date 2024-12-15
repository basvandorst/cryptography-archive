package Crypt::MD5;

require Exporter;
require DynaLoader;
@ISA = qw(Exporter DynaLoader);
# Items to export into callers namespace by default
@EXPORT = qw();
# Other items we are prepared to export if requested
@EXPORT_OK = qw();


bootstrap Crypt::MD5;


package MD5;

sub new { Crypt::MD5::new(@_); }
sub reset { Crypt::MD5::reset(@_); }
sub add { Crypt::MD5::add(@_); }
sub digest { Crypt::MD5::digest(@_); }

sub addfile
{
    my ($self, $handle) = @_;
    my ($package, $file, $line) = caller;
    my ($data) = '';

    if (!ref($handle))
    {
	# Old-style passing of filehandle by name. We need to add
	# the calling package scope qualifier, if there is not one
	# supplied already.

	$handle = $package . '::' . $handle unless ($handle =~ /\:\:/);
    }

    while (read($handle, $data, 1024))
    {
	$self->add($data);
    }
}

sub hexdigest
{
    my ($self) = shift;

    unpack("H*", ($self->digest()));
}

sub hash
{
    my ($self, $data) = @_;

    if (ref($self))
    {
	# This is an instance method call so reset the current context

	$self->reset();
    }
    else
    {
	# This is a static method invocation, create a temporary MD5 context

	$self = new MD5;
    }

    # Now do the hash

    $self->add($data);
    $self->digest();
}

sub hexhash
{
    my ($self, $data) = @_;

    unpack("H*", ($self->hash($data)));
}

1;
