package Cmiss::Cmiss_context;

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

# This allows declaration	use Cmiss::Value ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(
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
    croak "&Cmiss::Cmiss_context::constant not defined" if $constname eq 'constant';
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
if (!defined $Cmiss::Cmiss_context)
{
   require Cmiss::Perl_cmiss;
   Cmiss::require_library('cmgui');
}

package Cmiss::Cmiss_context;

require XSLoader;
XSLoader::load('Cmiss::Cmiss_context', $VERSION);

#Only destroy the actual command data if it was made here.
my $actually_destroy_context = 0;

if (!defined $Cmiss::Cmiss_context)
{
  no warnings; #Suppress warnings from variables defined in initialisation.

  $actually_destroy_context = 1;

  my $tmp_context = create(["cmgui", "-console"]);
  bless $tmp_context, "SomethingThatWillNOTMatch";

  if (!defined $Cmiss::Cmiss_context)
  {
	 die "Cmgui failed to initialise correctly";
  }
  sub cmiss
	 {
		Cmiss::Cmiss_context::execute_command($Cmiss::Cmiss_context, @_);
	 }
  *cmiss::cmiss = \&Cmiss::Cmiss_context::cmiss;
}

# Preloaded methods go here.

sub new
{
	my ($class) = @_;
	my $objref;

	$objref = $Cmiss::Cmiss_context;
	bless $objref, $class;

	return $objref;
}

sub get_cmiss_root_region
{
  my ($self, %args) = @_;
  my $objref;

  if (defined $self)
  {
	 $objref=context_get_default_region($self);
	 if ($objref)
	 {
		$objref;
	 }
	 else
	 {
		croak "Could not get root region.";
	 }
  }
  else
  {
	 croak "Missing cmiss_context.";
  }
}

sub get_element_selection
{
  my ($self, %args) = @_;
  my @objref;

  if (defined $self)
  {
	 return(context_get_element_selection($self));
  }
  else
  {
	 croak "Missing cmiss_context.";
  }
}

sub get_node_selection
{
  my ($self, %args) = @_;
  my @objref;

  if (defined $self)
  {
	 return(context_data_get_node_selection($self));
  }
  else
  {
	 croak "Missing cmiss_context.";
  }
}

sub DESTROY
{
  if ($actually_destroy_context)
  {
	 destroy(@_);
  }
}

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

Cmiss::Cmiss_context - Perl extension for Cmiss values

=head1 SYNOPSIS

  use Cmiss::Cmiss_context;

=head1 ABSTRACT

  This should be the abstract for Cmiss::Cmiss_context.
  The abstract is used when making PPD (Perl Package Description) files.
  If you don't want an ABSTRACT you should also edit Makefile.PL to
  remove the ABSTRACT_FROM option.

=head1 DESCRIPTION

Stub documentation for Cmiss::Cmiss_context, created by h2xs. It looks like the
author of the extension was negligent enough to leave the stub
unedited.


=head2 EXPORT

None by default.



=head1 SEE ALSO

=head1 AUTHOR

Shane Blackett, <s.blackett@auckland.ac.nz>

=head1 COPYRIGHT AND LICENSE

Copyright 2003 by Auckland UniServices

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself. 

=cut

