package Cmiss::Function::Finite_element;

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

# This allows declaration	use Cmiss::Function::Finite_element ':all';
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
    croak "&Cmiss::Function::Finite_element::constant not defined" if $constname eq 'constant';
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
Cmiss::require_library('cmgui_computed_variable_finite_element');

# Named argument
sub new
{
	my ($class, %arg) = @_;
	my ($name,$objref,$path,$region);

	$name=$arg{name};
	if (defined($name)&&($name))
	{
		$name =~ s/^.*\///;
		$path=$arg{name};
		$path =~ s/[^\/]*$//;
		$region=$arg{region};
		if (defined($region)&&($region))
		{
			$objref=new_xs($region,$path,$name);
		}
		else
		{
			croak "Missing region";
		}
	}
	else
	{
		croak "Missing name";
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

sub element_xi
{
	my ($self,@args)=@_;
	my ($objref);

	$objref=element_xi_xs($self);
}

sub element
{
	my ($self,@args)=@_;
	my ($objref);

	$objref=element_xs($self);
}

sub xi
{
	my ($self,%arg)=@_;
	my ($index,$objref);

	$index=$arg{index};
	if (defined($index))
	{
		$objref=xi_entry_xs($self,$index);
	}
	else
	{
		$objref=xi_xs($self);
	}
}

sub nodal_values
{
	my ($self,%args)=@_;
	my %defaults=(type=>"all",version=>0);
	my %args_with_defaults=(%defaults,%args);
	my ($node,$node_null,$objref,$type,$version);
	my ($component_name,$component_name_null,$component_number);

	$component_name=$args_with_defaults{component_name};
	if (!(defined($component_name)&&($component_name)))
	{
		# make a NULL pointer
		$component_name_null=0;
		$component_name=\$component_name_null;
	}
	$component_number=$args_with_defaults{component_number};
	if (!(defined($component_number)&&($component_number)))
	{
		$component_number=0;
	}
	$node=$args_with_defaults{node};
	if (!(defined($node)&&($node)))
	{
		# make a NULL pointer
		$node_null=0;
		$node=\$node_null;
		bless $node,"Cmiss::node";
	}
	$type=$args_with_defaults{type};
	$version=$args_with_defaults{version};
	$objref=nodal_values_xs($self,$component_name,$component_number,$node,$type,
		$version);
}

# Inherit string conversion
## Overload string and numerical conversion
#use overload '""' => \&string_convert, '0+' => \&numerical_convert, fallback => 1;

require XSLoader;
XSLoader::load('Cmiss::Function::Finite_element', $VERSION);

# Preloaded methods go here.

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

Cmiss::Function::Finite_element - Perl extension for Cmiss finite element functions

=head1 SYNOPSIS

  use Cmiss::Function::Finite_element;

=head1 ABSTRACT

  This should be the abstract for Cmiss::Function::Finite_element.
  The abstract is used when making PPD (Perl Package Description) files.
  If you don't want an ABSTRACT you should also edit Makefile.PL to
  remove the ABSTRACT_FROM option.

=head1 DESCRIPTION

Stub documentation for Cmiss::Function::Finite_element, created by h2xs. It looks like
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

