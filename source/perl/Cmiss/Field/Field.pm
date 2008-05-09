package Cmiss::Field;

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

# This allows declaration	use Cmiss::Field ':all';
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
    croak "&Cmiss::Field::constant not defined" if $constname eq 'constant';
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
use Cmiss::Cmgui_command_data;
Cmiss::require_library('cmgui_finite_element');

package Cmiss::Field;

sub new
{
	my ($class, $region, $type) = @_;
	my $objref;

	$objref=create($region, $type);
	if ($objref)
	{
		bless $objref,$class;
	}
	else
	{
		croak "Could not create $class";
	}
}

sub add
{
  my ($self, $addand) = @_;

  print("Cmiss::Field::add " . (ref $self) . " + " . (ref $addand) . "\n");

  if (ref $addand)
  {
	 if (UNIVERSAL::isa($addand, ref $self))
	 {
		print("Command data " . $Cmiss::Cmgui_command_data . "\n");

		#Should get the region from the $self field instead I think.
		return (new Cmiss::Field(
			$Cmiss::Cmgui_command_data->get_root_region(),
			Cmiss::Field::type_create_add($self, $addand)));
	 }
	 else
	 {
      croak "Can't add a ", ref $addand, " to a ", ref $self;
    }
  }
  else
  {
	 croak "Can't add $addand to ", ref $self;
  }
}

use overload
  '+' => 'add',
  fallback => 1;

require XSLoader;
XSLoader::load('Cmiss::Field', $VERSION);

# Preloaded methods go here.

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

Cmiss::Field - Perl extension for Cmiss regions

=head1 SYNOPSIS

  use Cmiss::Field;

=head1 ABSTRACT

  This should be the abstract for Cmiss::Field.
  The abstract is used when making PPD (Perl Package Description) files.
  If you don't want an ABSTRACT you should also edit Makefile.PL to
  remove the ABSTRACT_FROM option.

=head1 DESCRIPTION

Stub documentation for Cmiss::Field, created by h2xs. It looks like the
author of the extension was negligent enough to leave the stub
unedited.


=head2 EXPORT

None by default.



=head1 SEE ALSO

=head1 AUTHOR

Shane Blackett, <shane@blackett.co.nz>

=head1 COPYRIGHT AND LICENSE

Copyright 2003 by Auckland UniServices

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself. 

=cut

