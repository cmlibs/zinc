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
    my ($val) = constant($constname, @_ ? $_[0] : 0);
    if ($! != 0) {
		if ($! =~ /Invalid/ || $!{EINVAL}) {
	    $AutoLoader::AUTOLOAD = $AUTOLOAD;
	    goto &AutoLoader::AUTOLOAD;
	}
	else {
	    croak "Your vendor has not defined Cmiss::Function::Finite_element macro $constname";
	}
    }
     {
	no strict 'refs';
	# Fixed between 5.005_53 and 5.005_61
	if ($] >= 5.00561) {
	    *$AUTOLOAD = sub () { $val };
	}
	else {
	    *$AUTOLOAD = sub { $val };
	}
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

sub create_standard_interpolation_rc_constant_time
{
	my (%arg)=@_;
	my ($region,$name,$number_of_components,$component_names,$objref);

	$region=$arg{region};
	if (defined($region)&&($region))
	{
	  $name=$arg{name};
	  if (defined($name)&&($name))
	  {
		 $number_of_components=$arg{number_of_components};
		 if (defined($number_of_components)&&($number_of_components))
		 {
			$component_names=$arg{component_names};
			if (defined($component_names)&&($component_names))
			{
			  $objref = create_standard_interpolation_rc_constant_time_xs(
				  $region,$name,$number_of_components,$component_names);
			  if (defined($objref)&&($objref))
			  {
				 bless $objref,"Cmiss::Function::Finite_element";
			  }
			  else
			  {
				 croak "Could not create Cmiss::Function::Finite_element";
			  }
			}
			else
			{
			  croak "Missing component names";
			}
		 }
		 else
		 {
			croak "Missing number of components";
		 }
	  }
	  else
	  {
		 croak "Missing name";
	  }
	}
	else
	{
	  croak "Missing region";
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

sub time
{
	my ($self,@args)=@_;
	my ($objref);

	$objref=time_xs($self);
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
	my ($node,$node_null,$objref,$type,$version,$time_sequence,$time_sequence_null);
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
		bless $node,"Cmiss::Node";
	}
	$type=$args_with_defaults{type};
	$version=$args_with_defaults{version};
	$time_sequence=$args_with_defaults{time_sequence};
	if (!(defined($time_sequence)&&($time_sequence)))
	{
		# make a NULL pointer
		$time_sequence_null=0;
		$time_sequence=\$time_sequence_null;
		bless $time_sequence,"Cmiss::Time_sequence";
	}
	$objref=nodal_values_xs($self,$component_name,$component_number,$node,$type,
		$version,$time_sequence);
}

sub define_on_Cmiss_node
{
	my ($self,%arg)=@_;
	my ($node,$time_sequence,$time_sequence_null,$node_field_creator);

	$node=$arg{node};
	if (defined($node)&&($node))
	{
		$node_field_creator=$arg{node_field_creator};
		if (defined($node_field_creator)&&($node_field_creator))
		{
		  #Time sequence is not required
		  $time_sequence=$arg{time_sequence};
		  if (!defined($time_sequence))
		  {
			 $time_sequence_null = 0;
			 $time_sequence = \$time_sequence_null;
			 bless $time_sequence, "Cmiss::Time_sequence";
		  }
		  define_on_Cmiss_node_xs($self, $node, $time_sequence,
			  $node_field_creator);
		}
		else
		{
			croak "Missing node field creator";
		}
	}
	else
	{
	  croak "Missing node";
	}
}

sub define_tensor_product_basis_on_element
{
	my ($self,%arg)=@_;
	my ($element,$dimension,$basis_type);

	$element=$arg{element};
	if (defined($element)&&($element))
	{
		$dimension=$arg{dimension};
		if (defined($dimension)&&($dimension))
		{
		  $basis_type=$arg{basis_type};
		  if (defined($basis_type)&&($basis_type))
		  {
			 define_tensor_product_basis_on_element_xs($self,
				 $element, $dimension, $basis_type);
		  }
		  else
		  {
			 croak "Missing basis type";
		  }
		}
		else
		{
			croak "Missing dimension";
		}
	}
	else
	{
	  croak "Missing element";
	}
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

