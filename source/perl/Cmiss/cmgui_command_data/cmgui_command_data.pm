package Cmiss::cmgui_command_data;

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
	new
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
	new
);

our $VERSION = '0.01';

sub AUTOLOAD {
    # This AUTOLOAD is used to 'autoload' constants from the constant()
    # XS function.

    my $constname;
    our $AUTOLOAD;
    ($constname = $AUTOLOAD) =~ s/.*:://;
    croak "&Cmiss::cmgui_command_data::constant not defined" if $constname eq 'constant';
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
if (!defined $Cmiss::cmgui_command_data)
{
   require Cmiss::Perl_cmiss;
   Cmiss::require_library('cmgui');
}

package Cmiss::cmgui_command_data;

require XSLoader;
XSLoader::load('Cmiss::cmgui_command_data', $VERSION);

#Only destroy the actual command data if it was made here.
my $actually_destroy_command_data = 0;

if (!defined $Cmiss::cmgui_command_data)
{
  no warnings; #Suppress warnings from variables defined in initialisation.

  $actually_destroy_command_data = 1;

  my $tmp_command_data = create(["cmgui", "-console"]);
  bless $tmp_command_data, "SomethingThatWillNOTMatch";

  if (!defined $Cmiss::cmgui_command_data)
  {
	 die "Cmgui failed to initialise correctly";
  }
  sub cmiss
	 {
		Cmiss::cmgui_command_data::execute_command($Cmiss::cmgui_command_data, @_);
	 }
  *cmiss::cmiss = \&Cmiss::cmgui_command_data::cmiss;
}

# Preloaded methods go here.

sub new
{
	my ($class) = @_;
	my $objref;

	$objref = $Cmiss::cmgui_command_data;
	bless $objref, $class;

	return $objref;
}

sub DESTROY
{
  if ($actually_destroy_command_data)
  {
	 destroy(@_);
  }
}

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

Cmiss::cmgui_command_data - Perl extension for Cmiss values

=head1 SYNOPSIS

  use Cmiss::cmgui_command_data;

=head1 ABSTRACT

  This should be the abstract for Cmiss::cmgui_command_data.
  The abstract is used when making PPD (Perl Package Description) files.
  If you don't want an ABSTRACT you should also edit Makefile.PL to
  remove the ABSTRACT_FROM option.

=head1 DESCRIPTION

Stub documentation for Cmiss::cmgui_command_data, created by h2xs. It looks like the
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

