package Cmiss::Element;

use 5.006;
use strict;
use warnings;
use Carp;

require Exporter;
use AutoLoader;

our @ISA = qw(Exporter);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use Cmiss::Element ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(
	new
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
	LINEAR_LAGRANGE
);

our $VERSION = '0.01';

sub AUTOLOAD {
    # This AUTOLOAD is used to 'autoload' constants from the constant()
    # XS function.

    my $constname;
    our $AUTOLOAD;
    ($constname = $AUTOLOAD) =~ s/.*:://;
    croak "&Cmiss::Element::constant not defined" if $constname eq 'constant';
    my ($val) = constant($constname, @_ ? $_[0] : 0);
    if ($! != 0) {
		if ($! =~ /Invalid/ || $!{EINVAL}) {
	    $AutoLoader::AUTOLOAD = $AUTOLOAD;
	    goto &AutoLoader::AUTOLOAD;
	}
	else {
	    croak "Your vendor has not defined Cmiss::Element macro $constname";
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
Cmiss::require_library('cmgui_finite_element');

sub create_with_line_shape
{
	my (%arg)=@_;
	my ($identifier,$region,$dimension,$objref);

	$identifier=$arg{identifier};
	if (defined($identifier)&&($identifier))
	{
	  $region=$arg{region};
	  if (defined($region)&&($region))
	  {
		 $dimension=$arg{dimension};
		 if (defined($dimension)&&($dimension))
		 {
			$objref = create_with_line_shape_xs(
				 $identifier,$region,$dimension);
			if (defined($objref)&&($objref))
		   {
			  bless $objref,"Cmiss::Element";
			}
			else
			{
			  croak "Could not create Cmiss::Element";
			}
		 }
		 else
		 {
			croak "Missing number of dimension";
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

	return($objref);
}

sub set_node
{
	my ($self, %args) = @_;
	my %defaults=();
	my %args_with_defaults=(%defaults,%args);
	my ($node_index,$node);

	$node_index=$args_with_defaults{node_index};
	if (defined ($node_index))
	{
	  $node=$args_with_defaults{node};
	  if (defined ($node) && $node)
	  {
		 set_node_xs($self, $node_index, $node);
	  }
	  else
	  {
		 croak "Missing node";
	  }
	}
	else
	{
		croak "Missing node index";
	}
}

use overload '""' => \&print_string;

sub print_string
{
  	my $self = shift;
	my $identifier = get_identifier_xs($self);
	return ("element=$identifier");
}

require XSLoader;
XSLoader::load('Cmiss::Element', $VERSION);

# Preloaded methods go here.

sub LINEAR_LAGRANGE();

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

Cmiss::Element - Perl extension for Cmiss regions

=head1 SYNOPSIS

  use Cmiss::Element;

=head1 ABSTRACT

  This should be the abstract for Cmiss::Element.
  The abstract is used when making PPD (Perl Package Description) files.
  If you don't want an ABSTRACT you should also edit Makefile.PL to
  remove the ABSTRACT_FROM option.

=head1 DESCRIPTION

Stub documentation for Cmiss::Element, created by h2xs. It looks like the
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

