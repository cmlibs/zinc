package Cmiss;

use 5.006;
use strict;
use warnings;

require Exporter;
require DynaLoader;

our @ISA = qw(Exporter DynaLoader);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use Cmiss ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(
	cmgui_command_data
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
);

our $VERSION = '0.01';

# Preloaded methods go here.

my %loaded_modules = ();

sub croak
{
  require Carp;
  Carp::croak(@_);
}

sub require_library_stub
{
  1;
}

sub require_library_actual
{
  my ($module_name) = @_;

  no strict 'vars';
  if (! defined $loaded_modules{$module_name})
  {
	 print ("Loading $module_name\n");
	 my @libraries = DynaLoader::dl_findfile($module_name);
	 if (defined $libraries[0])
	 {
		my $libref = DynaLoader::dl_load_file($libraries[0], 0x01) or
		  croak("Can't load '$module_name' for Cmiss: " . DynaLoader::dl_error());
		push (@DynaLoader::dl_librefs,$libref); # record loaded object
		$loaded_modules{$module_name} = 1;
	 }
	 else
	 {
		croak("Could not find module '$module_name'");
	 }
  }
}



{
  no strict 'vars';
  if (defined ($cmgui_command_data))
  {
	 #Then presumably we have all the cmgui libraries
	 *require_library = \&require_library_stub;
  }
  else
  {
	 #This pointer must be initialised before requiring any of the 
	 #libraries further down as they depend on it.
	 *require_library = \&require_library_actual;
	 require Cmiss::Perl_cmiss;
	 require_library("cmgui");
    require Cmiss::cmgui_command_data;
	 my $tmp_command_data = new Cmiss::cmgui_command_data("cmgui", "-console") or 
		croak("Unable to initialise cmgui");
	 #Bless this into some undefined class so that no destructor will be called.
	 #$Cmiss::cmgui_command_data is set as a side effect of creating the 
	 #cmgui_command_data at the moment.
	 bless($tmp_command_data, "SomethingThatWillNOTMatchAnything");
	 sub cmiss
		{
		  $Cmiss::cmgui_command_data->cmiss(@_);
		}
	 *cmiss::cmiss = \&Cmiss::cmiss;
  }
}

sub cmgui_command_data
{
  no strict 'vars';
  $cmgui_command_data;
}


# Autoload methods go after =cut, and are processed by the autosplit program.

1;

__END__

# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

Cmiss - Perl extension for interfacing to Cmiss

=head1 SYNOPSIS

  use Cmiss;

=head1 ABSTRACT

=head1 DESCRIPTION

=head2 EXPORT

=head1 SEE ALSO

=head1 AUTHOR

Shane Blackett, <s.blackett@auckland.ac.nz>

=head1 COPYRIGHT AND LICENSE

Copyright 2003 by Auckland UniServices

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself. 

=cut

