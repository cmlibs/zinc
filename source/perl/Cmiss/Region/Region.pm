package Cmiss::Region;

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

# This allows declaration	use Cmiss::Region ':all';
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
    croak "&Cmiss::Region::constant not defined" if $constname eq 'constant';
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
Cmiss::require_library('cmgui_finite_element');

sub new
{
	my ($class) = @_;
	my $objref;

	$objref=create();
	if ($objref)
	{
		bless $objref,$class;
	}
	else
	{
		croak "Could not create $class";
	}
}

#???DB.  Can't yet get the Cmiss_region name
## Overload string conversion
#use overload '""' => \&string_convert, fallback => 1;
#
#sub string_convert
#{
#	get_type(shift);
#}

sub read_file
{
	my ($self, %args) = @_;
	my %defaults=();
	my %args_with_defaults=(%defaults,%args);
	my $name=$args_with_defaults{name};

	if ($name)
	{
		region_read_file($self, $name);
	}
	else
	{
		croak "Missing name";
	}
}

sub get_element
{
	my ($self, %args) = @_;
	my %defaults=(type=>"element");
	my %args_with_defaults=(%defaults,%args);
	my $path=$args_with_defaults{name};

	if ($path)
	{
		$path =~ s/[^\/]*$//;
		my $identifier=$args_with_defaults{name};
		$identifier =~ s/^.*\///;
		region_get_element($self, $path, $identifier, $args_with_defaults{type});
	}
	else
	{
		croak "Missing name";
	}
}

sub get_field
{
	my ($self, %args) = @_;
	my %defaults=();
	my %args_with_defaults=(%defaults,%args);
	my $path=$args_with_defaults{name};

	if ($path)
	{
		$path =~ s/[^\/]*$//;
		my $identifier=$args_with_defaults{name};
		$identifier =~ s/^.*\///;
		region_get_field($self, $path, $identifier);
	}
	else
	{
		croak "Missing name";
	}
}

sub get_node
{
	my ($self, %args) = @_;
	my %defaults=();
	my %args_with_defaults=(%defaults,%args);
	my $path=$args_with_defaults{name};
	
	if ($path)
	{
		$path =~ s/[^\/]*$//;
		my $identifier=$args_with_defaults{name};
		$identifier =~ s/^.*\///;
		region_get_node($self, $path, $identifier);
	}
	else
	{
		croak "Missing name";
	}
}

sub get_sub_region
{
	my ($self, %args) = @_;
	my %defaults=();
	my %args_with_defaults=(%defaults,%args);
	my $name=$args_with_defaults{name};

	if ($name)
	{
		region_get_sub_region($self, $name);
	}
	else
	{
		croak "Missing name";
	}
}

require XSLoader;
XSLoader::load('Cmiss::Region', $VERSION);

# Preloaded methods go here.

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

Cmiss::Region - Perl extension for Cmiss regions

=head1 SYNOPSIS

  use Cmiss::Region;

=head1 ABSTRACT

  This should be the abstract for Cmiss::Region.
  The abstract is used when making PPD (Perl Package Description) files.
  If you don't want an ABSTRACT you should also edit Makefile.PL to
  remove the ABSTRACT_FROM option.

=head1 DESCRIPTION

Stub documentation for Cmiss::Region, created by h2xs. It looks like the
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

