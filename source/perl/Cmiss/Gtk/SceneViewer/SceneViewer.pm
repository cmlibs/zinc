package Cmiss::Gtk::SceneViewer;

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

# This allows declaration	use Cmiss::scene_viewer ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
	
);
our $VERSION = '0.03';

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
	    croak "Your vendor has not defined Cmiss::scene_viewer macro $constname";
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

bootstrap Cmiss::Gtk::SceneViewer $VERSION;

require Cmiss::Gtk::Types;

# Preloaded methods go here.

#sub CMISS_SCENE_VIEWER_PERSPECTIVE();
#sub CMISS_SCENE_VIEWER_PARALLEL();
#sub CMISS_SCENE_VIEWER_FAST_TRANSPARENCY();
#sub CMISS_SCENE_VIEWER_SLOW_TRANSPARENCY();
#sub CMISS_SCENE_VIEWER_LAYERED_TRANSPARENCY();

Gtk->mod_init_add('Gtk', sub {
	init Cmiss::Gtk::SceneViewer;
});

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__

=head1 NAME

Cmiss::Gtk::SceneViewer - Perl extension for using the Cmiss Gtk scene_viewer.

=head1 SYNOPSIS

  use Cmiss::scene_viewer;
  use Cmiss::Gtk::SceneViewer;

=head1 DESCRIPTION

Perl bindings to the public interface of Cmiss::SceneViewer.

=head2 EXPORT

None by default.

=head2 Exportable functions

=head1 AUTHOR

Shane Blackett <s.blackett@auckland.ac.nz>

=head1 SEE ALSO

L<perl> and L<Cmiss::graphics_window>.

=cut

