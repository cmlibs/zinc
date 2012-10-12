package Cmiss::Variable_new::Finite_element;

use 5.006;
use strict;
use warnings;
use Carp;

require Cmiss::Variable_new;
require Exporter;
use AutoLoader;

our @ISA = qw(Cmiss::Variable_new Exporter);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use Cmiss::Variable_new::Finite_element ':all';
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
    croak "&Cmiss::Variable_new::Finite_element::constant not defined" if $constname eq 'constant';
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
	my ($component_name,$name,$objref,$path,$region);

	$name=$arg{name};
	if (defined($name)&&($name))
	{
		$name =~ s/^.*\///;
		$path=$arg{name};
		$path =~ s/[^\/]*$//;
		$region=$arg{region};
		if (defined($region)&&($region))
		{
			$component_name=$arg{component};
			if ($component_name)
			{
				$objref=new_xs($region,$path,$name,$component_name);
			}
			else
			{
				$objref=new_xs($region,$path,$name);
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
			croak "Missing region";
		}
	}
	else
	{
		croak "Missing name";
	}
}

sub input_element_xi
{
	my ($self,@args)=@_;
	my ($objref);

	$objref=input_element_xi_xs($self);
	if ($objref)
	{
		bless $objref,'Cmiss::Variable_new_input'
	}
}

sub input_xi
{
	my ($self,@indices)=@_;
	my ($objref);

	$objref=input_xi_xs($self,\@indices);
	if ($objref)
	{
		bless $objref,'Cmiss::Variable_new_input'
	}
}

sub input_nodal_values
{
	my ($self,%args)=@_;
	my %defaults=(type=>"all",version=> -1);
	my %args_with_defaults=(%defaults,%args);
	my ($node,$node_null,$objref,$type,$version);

	$node=$args_with_defaults{node};
	if (!$node)
	{
		# make a NULL pointer
		$node_null=0;
		$node=\$node_null;
		bless $node,"Cmiss::Node";
	}
	$type=$args_with_defaults{type};
	$version=$args_with_defaults{version};
	$objref=input_nodal_values_xs($self,$node,$type,$version);
	if ($objref)
	{
		bless $objref,'Cmiss::Variable_new_input'
	}
}

# Inherit string conversion
## Overload string and numerical conversion
#use overload '""' => \&string_convert, '0+' => \&numerical_convert, fallback => 1;

require XSLoader;
XSLoader::load('Cmiss::Variable_new::Finite_element', $VERSION);

# Preloaded methods go here.

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

Cmiss::Variable_new::Finite_element - Perl extension for Cmiss finite element variables

=head1 SYNOPSIS

  use Cmiss::Variable_new::Finite_element;

=head1 ABSTRACT

  This should be the abstract for Cmiss::Variable_new::Finite_element.
  The abstract is used when making PPD (Perl Package Description) files.
  If you don't want an ABSTRACT you should also edit Makefile.PL to
  remove the ABSTRACT_FROM option.

=head1 DESCRIPTION

Stub documentation for Cmiss::Variable_new::Finite_element, created by h2xs. It looks like
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

