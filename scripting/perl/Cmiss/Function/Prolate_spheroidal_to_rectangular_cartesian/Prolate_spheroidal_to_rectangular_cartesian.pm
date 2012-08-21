package Cmiss::Function::Prolate_spheroidal_to_rectangular_cartesian;

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

# This allows declaration	use Cmiss::Function::Prolate_spheroidal_to_rectangular_cartesian ':all';
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
    croak "&Cmiss::Function::Prolate_spheroidal_to_rectangular_cartesian::constant not defined" if $constname eq 'constant';
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

# Named argument
sub new
{
	my ($class, @values) = @_;
	my ($objref);

	$objref=new_xs(\@values);
	if (defined($objref)&&($objref))
	{
		bless $objref,$class;
	}
	else
	{
		croak "Could not create $class";
	}
}

sub component
{
	my ($self,%arg)=@_;
	my ($name,$number,$objref);

	$name=$arg{name};
	if (defined($name)&&($name))
	{
		$objref=component_name_xs($self,$name);
	}
	else
	{
		$number=$arg{number};
		if (defined($number)&&($number))
		{
			$objref=component_number_xs($self,$number);
		}
		else
		{
			croak "Missing name and number";
		}
	}
}

sub focus
{
	my ($self,@args)=@_;
	my ($objref);

	$objref=focus_xs($self);
}

sub lambda
{
	my ($self,@args)=@_;
	my ($objref);

	$objref=lambda_xs($self);
}

sub mu
{
	my ($self,@args)=@_;
	my ($objref);

	$objref=mu_xs($self);
}

sub prolate
{
	my ($self,@args)=@_;
	my ($objref);

	$objref=prolate_xs($self);
}

sub theta
{
	my ($self,@args)=@_;
	my ($objref);

	$objref=theta_xs($self);
}

# Inherit string conversion
## Overload string and numerical conversion
#use overload '""' => \&string_convert, '0+' => \&numerical_convert, fallback => 1;

require XSLoader;
XSLoader::load('Cmiss::Function::Prolate_spheroidal_to_rectangular_cartesian', $VERSION);

# Preloaded methods go here.

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

Cmiss::Function::Prolate_spheroidal_to_rectangular_cartesian - Perl extension for Cmiss prolate_spheroidal_to_rectangular_cartesian functions

=head1 SYNOPSIS

  use Cmiss::Function::Prolate_spheroidal_to_rectangular_cartesian;

=head1 ABSTRACT

  This should be the abstract for Cmiss::Function::Prolate_spheroidal_to_rectangular_cartesian.
  The abstract is used when making PPD (Perl Package Description) files.
  If you don't want an ABSTRACT you should also edit Makefile.PL to
  remove the ABSTRACT_FROM option.

=head1 DESCRIPTION

Stub documentation for Cmiss::Function::Prolate_spheroidal_to_rectangular_cartesian, created by h2xs. It looks like
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

