package Cmiss::Graphics_window;

use 5.006;
use strict;
use warnings;
use Carp;

require Exporter;
require DynaLoader;
use AutoLoader;

our @ISA = qw(Exporter DynaLoader);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use Cmiss::Graphics_window ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(
	get_scene_viewer_by_name
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
	
);
our $VERSION = '0.01';

sub AUTOLOAD {
    # This AUTOLOAD is used to 'autoload' constants from the constant()
    # XS function.  If a constant is not found then control is passed
    # to the AUTOLOAD in AutoLoader.

    my $constname;
    our $AUTOLOAD;
    ($constname = $AUTOLOAD) =~ s/.*:://;
    croak "& not defined" if $constname eq 'constant';
    my $val = constant($constname, @_ ? $_[0] : 0);
    if ($! != 0) {
	if ($! =~ /Invalid/ || $!{EINVAL}) {
	    $AutoLoader::AUTOLOAD = $AUTOLOAD;
	    goto &AutoLoader::AUTOLOAD;
	}
	else {
	    croak "Your vendor has not defined Cmiss::Graphics_window macro $constname";
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

#Graphics windows currently require the command data to be initialised.
use Cmiss::Cmgui_command_data;

require XSLoader;
XSLoader::load('Cmiss::Graphics_window', $VERSION);

# Preloaded methods go here.

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__

=head1 NAME

Cmiss::Graphics_window - Perl extension for interacting with Cmiss graphics_windows.

=head1 SYNOPSIS

gfx create window 1;

use Cmiss::Graphics_window;

$bob = Cmiss::Graphics_window::get_scene_viewer_by_name("1", 0);

=head1 DESCRIPTION

Perl bindings to the public interface of Cmiss::Graphics_window.

=head2 EXPORT

None by default.

=head2 Exportable functions

Cmiss::Scene_viewer Cmiss_scene_viewer_id get_scene_viewer_by_name(char *graphics_window_name,
	 int pane_number);
The first pane is <pane_number> 0.

=head1 AUTHOR

Shane Blackett <s.blackett@auckland.ac.nz>

=head1 SEE ALSO

L<perl> and L<Cmiss::scene_viewer>.

=cut
