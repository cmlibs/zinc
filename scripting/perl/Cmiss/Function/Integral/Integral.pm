package Cmiss::Function::Integral;

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

# This allows declaration	use Cmiss::Function::Integral ':all';
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
    croak "&Cmiss::Function::Integral::constant not defined" if $constname eq 'constant';
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

# Named argument
sub new
{
	my ($class, %arg) = @_;
	my ($name,$independent_input,$independent_input_null,$independent_output,
		$independent_output_null,$integrand_input,$integrand_output,$objref,
		$quadrature_scheme,$region);

	$integrand_input=$arg{integrand_input};
	if (defined($integrand_input)&&($integrand_input))
	{
		$integrand_output=$arg{integrand_output};
		if (defined($integrand_output)&&($integrand_output))
		{
			$region=$arg{region};
			if (defined($region)&&($region))
			{
				$quadrature_scheme=$arg{quadrature_scheme};
				if (defined($quadrature_scheme)&&($quadrature_scheme))
				{
					$independent_input=$arg{independent_input};
					$independent_output=$arg{independent_output};
					if (!(defined($independent_input)&&($independent_input)&&
						defined($independent_output)&&($independent_output)))
					{
						$independent_input_null=0;
						$independent_input=\$independent_input_null;
						bless $independent_input,"Cmiss::Function_variable";
						$independent_output_null=0;
						$independent_output=\$independent_output_null;
						bless $independent_output,"Cmiss::Function_variable";
					}
					$objref=new_xs($integrand_output,$integrand_input,
						$independent_output,$independent_input,$region,$quadrature_scheme);
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
					croak "Missing quadrature_scheme";
				}
			}
			else
			{
				croak "Missing region";
			}
		}
		else
		{
			croak "Missing integrand_output";
		}
	}
	else
	{
		croak "Missing integrand_input";
	}
}

# Inherit string conversion
## Overload string and numerical conversion
#use overload '""' => \&string_convert, '0+' => \&numerical_convert, fallback => 1;
#
#sub numerical_convert
#{
#	get_type(shift);
#}
#
#sub string_convert
#{
#	get_type(shift);
#}

require XSLoader;
XSLoader::load('Cmiss::Function::Integral', $VERSION);

# Preloaded methods go here.

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

Cmiss::Function::Integral - Perl extension for Cmiss integral functions

=head1 SYNOPSIS

  use Cmiss::Function::Integral;

=head1 ABSTRACT

  This should be the abstract for Cmiss::Function::Integral.
  The abstract is used when making PPD (Perl Package Description) files.
  If you don't want an ABSTRACT you should also edit Makefile.PL to
  remove the ABSTRACT_FROM option.

=head1 DESCRIPTION

Stub documentation for Cmiss::Function::Integral, created by h2xs. It looks like
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

