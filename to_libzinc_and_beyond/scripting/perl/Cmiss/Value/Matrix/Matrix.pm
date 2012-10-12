package Cmiss::Value::Matrix;

use 5.006;
use strict;
use warnings;
use Carp;

require Cmiss::Value;
require Exporter;
use AutoLoader;

our @ISA = qw(Cmiss::Value Exporter);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use Cmiss::Value::Matrix ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(
	new
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
	
);

our $VERSION = '0.01';

sub AUTOLOAD {
    # This AUTOLOAD is used to 'autoload' constants from the constant()
    # XS function.

    my $constname;
    our $AUTOLOAD;
    ($constname = $AUTOLOAD) =~ s/.*:://;
    croak "&Cmiss::Value::Matrix::constant not defined" if $constname eq 'constant';
    my ($error, $val) = constant($constname);
    if ($error) { croak $error; }
    {
	no strict 'refs';
	# Fixed between 5.005_53 and 5.005_61
#XXX	if ($] >= 5.00561) {
#XXX	    *$AUTOLOAD = sub () { $val };
#XXX	}
#XXX	else {
	    *$AUTOLOAD = sub { $val };
#XXX	}
    }
    goto &$AUTOLOAD;
}

use Cmiss;
Cmiss::require_library('cmgui_computed_variable');

# Class attributes and methods
{
	my $string_convert_max_rows=8;
	sub get_string_convert_max_rows { $string_convert_max_rows }
	sub set_string_convert_max_rows
	{
		$string_convert_max_rows=$_[1];
		if ($string_convert_max_rows<1)
		{
			$string_convert_max_rows=1;
		}
	}

	my $string_convert_max_columns=8;
	sub get_string_convert_max_columns { $string_convert_max_columns }
	sub set_string_convert_max_columns
	{
		$string_convert_max_columns=$_[1];
		if ($string_convert_max_columns<1)
		{
			$string_convert_max_columns=1;
		}
	}
}

# Constructor
sub new
{
	my ($class, %arg) = @_;
	my ($n_columns,$objref,$values);

	$values=$arg{values};
	if ($values)
	{
		$n_columns=$arg{n_columns};
		if ($n_columns)
		{
			$objref=create($n_columns, $values);
		}
		else
		{
			$objref=create(1, $values);
		}
		if ($objref)
		{
			bless $objref,$class;
		}
		else
		{
			croak "Could not create $class";
		}
	}
	else
	{
		croak "Missing values";
	}
}

sub sub_matrix
{
	my ($self,%args)=@_;
	my ($objref);

	if (%args)
	{
		$objref=get_sub_matrix($self,\%args);
	}
	else
	{
		$objref=get_sub_matrix($self);
	}
}

use Cmiss;
Cmiss::require_library('cmgui_computed_variable');

require XSLoader;
XSLoader::load('Cmiss::Value::Matrix', $VERSION);

# Preloaded methods go here.

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

Cmiss::Value::Matrix - Perl extension for Cmiss matrices

=head1 SYNOPSIS

  use Cmiss::Value::Matrix;

=head1 ABSTRACT

  This should be the abstract for Cmiss::Value::Matrix.
  The abstract is used when making PPD (Perl Package Description) files.
  If you don't want an ABSTRACT you should also edit Makefile.PL to
  remove the ABSTRACT_FROM option.

=head1 DESCRIPTION

Stub documentation for Cmiss::Value::Matrix, created by h2xs. It looks like
the author of the extension was negligent enough to leave the stub
unedited.

=head2 EXPORT

None by default.



=head1 SEE ALSO

=head1 AUTHOR

David Bullivant, <d.bullivant@auckland.ac.nz>

=head1 COPYRIGHT AND LICENSE

Copyright 2003 by Auckland UniServices

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself. 

=cut

