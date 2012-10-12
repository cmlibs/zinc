package Cmiss::Function::Matrix;

use 5.006;
use strict;
use warnings;
use Carp;

require Cmiss::Function;
require Exporter;
use AutoLoader;

our @ISA = qw(Cmiss::Function Exporter);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use Cmiss::Function::Matrix ':all';
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
    croak "&Cmiss::Function::Matrix::constant not defined" if $constname eq 'constant';
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

sub new
{
	my ($class, %arg) = @_;
	my ($n_columns,$objref,$values);

	$values=$arg{values};
	if (defined($values))
	{
		$n_columns=$arg{n_columns};
		if (defined($n_columns))
		{
			$objref=new_xs($n_columns,$values);
		}
		else
		{
			$objref=new_xs(1,$values);
		}
		if (defined($objref)&&($objref))
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

sub row
{
	my ($self, $row_index) = @_;
	my ($objref, $column_index);

	if (defined($row_index))
	{
		$column_index=0;
		$objref=entry_xs($self,$row_index,$column_index);
	}
	else
	{
		croak "Missing row index";
	}
}

sub column
{
	my ($self, $column_index) = @_;
	my ($objref, $row_index);

	if (defined($column_index))
	{
		$row_index=0;
		$objref=entry_xs($self,$row_index,$column_index);
	}
	else
	{
		croak "Missing column index";
	}
}

sub entry
{
	my ($self, $row_index, $column_index) = @_;
	my ($objref);

	if (defined($row_index)&&defined($column_index))
	{
		$objref=entry_xs($self,$row_index,$column_index);
	}
	else
	{
		croak "Missing row or column index";
	}
}

sub sub_matrix
{
	my ($self,%args)=@_;
	my ($objref);

	if (%args)
	{
		$objref=sub_matrix_xs($self,\%args);
	}
	else
	{
		$objref=sub_matrix_xs($self);
	}
}

sub solve
{
	my ($self,$rhs,@args) = @_;
	my ($objref);

	$objref=solve_xs($self,$rhs);
}

require XSLoader;
XSLoader::load('Cmiss::Function::Matrix', $VERSION);

# Preloaded methods go here.

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

Cmiss::Function::Matrix - Perl extension for Cmiss matrix functions

=head1 SYNOPSIS

  use Cmiss::Function::Matrix;

=head1 ABSTRACT

  This should be the abstract for Cmiss::Function::Matrix.
  The abstract is used when making PPD (Perl Package Description) files.
  If you don't want an ABSTRACT you should also edit Makefile.PL to
  remove the ABSTRACT_FROM option.

=head1 DESCRIPTION

Stub documentation for Cmiss::Function::Matrix, created by h2xs. It looks like the
author of the extension was negligent enough to leave the stub
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

