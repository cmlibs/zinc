package Cmiss::Variable::Element_xi;

use 5.006;
use strict;
use warnings;
use Carp;

require Cmiss::Variable;
require Exporter;
use AutoLoader;

our @ISA = qw(Cmiss::Variable Exporter);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use Cmiss::Variable::Element_xi ':all';
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
    croak "&Cmiss::Variable::Element_xi::constant not defined" if $constname eq 'constant';
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
	my ($dimension,$name,$objref);

	$dimension=$arg{dimension};
	if (!$dimension)
	{
		$dimension=0;
	}
	$name=$arg{name};
	if ($name)
	{
		$objref=create($dimension,$name);
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
		croak "Missing name";
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
XSLoader::load('Cmiss::Variable::Element_xi', $VERSION);

# Preloaded methods go here.

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

Cmiss::Variable::Element_xi - Perl extension for Cmiss element/xi variables

=head1 SYNOPSIS

  use Cmiss::Variable::Element_xi;

=head1 ABSTRACT

  This should be the abstract for Cmiss::Variable::Element_xi.
  The abstract is used when making PPD (Perl Package Description) files.
  If you don't want an ABSTRACT you should also edit Makefile.PL to
  remove the ABSTRACT_FROM option.

=head1 DESCRIPTION

Stub documentation for Cmiss::Variable::Element_xi, created by h2xs. It looks like
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

