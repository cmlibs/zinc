package Cmiss::scene_viewer;

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
	get_lookat_parameters
	set_lookat_parameters_non_skew
   get_projection_mode
   set_projection_mode
   get_transparency_mode
   set_transparency_mode
   get_transparency_layers
   set_transparency_layers
	get_near_and_far_plane
	set_near_and_far_plane
	get_view_angle
	set_view_angle
	get_antialias_mode
   set_antialias_mode
	get_perturb_lines
   set_perturb_lines
   get_background_colour_rgb
   set_background_colour_rgb
   get_interactive_tool_name
   set_interactive_tool_by_name
   get_scene_name
   set_scene_by_name
   view_all
   redraw_now
   write_image_to_file
	CMISS_SCENE_VIEWER_PERSPECTIVE
	CMISS_SCENE_VIEWER_PARALLEL
	CMISS_SCENE_VIEWER_FAST_TRANSPARENCY
 	CMISS_SCENE_VIEWER_SLOW_TRANSPARENCY
 	CMISS_SCENE_VIEWER_LAYERED_TRANSPARENCY
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

bootstrap Cmiss::scene_viewer $VERSION;

# Preloaded methods go here.

sub CMISS_SCENE_VIEWER_PERSPECTIVE();
sub CMISS_SCENE_VIEWER_PARALLEL();
sub CMISS_SCENE_VIEWER_FAST_TRANSPARENCY();
sub CMISS_SCENE_VIEWER_SLOW_TRANSPARENCY();
sub CMISS_SCENE_VIEWER_LAYERED_TRANSPARENCY();

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__

=head1 NAME

Cmiss::scene_viewer - Perl extension for interacting with the Cmiss scene_viewer.

=head1 SYNOPSIS

  use Cmiss::scene_viewer;
  use Cmiss::graphics_window;

=head1 DESCRIPTION

Perl bindings to the public interface of Cmiss::scene_viewer.

=head2 EXPORT

None by default.

=head2 Exportable functions

  (double eyex, double eyey, double eyez, double lookatx, double lookaty, double lookatz,
        double upx, double upy, double upz) get_lookat_parameters(
	     Cmiss_scene_viewer_id scene_viewer)

  int set_lookat_parameters_non_skew(
        Cmiss_scene_viewer_id scene_viewer,double eyex,double eyey,double eyez,
        double lookatx,double lookaty,double lookatz,
        double upx,double upy,double upz)

  double get_view_angle(Cmiss_scene_viewer_id scene_viewer)
The <view_angle> is returned in radians.

  int set_view_angle(Cmiss_scene_viewer_id scene_viewer,
        double view_angle)
The <view_angle> should be specified in radians.

  int get_antialias_mode(Cmiss_scene_viewer_id scene_viewer)

  int set_antialias_mode(Cmiss_scene_viewer_id scene_viewer,
        int antialias_mode)

  int get_perturb_lines(Cmiss_scene_viewer_id scene_viewer)

  int set_perturb_lines(Cmiss_scene_viewer_id scene_viewer,
        int perturb_lines)

  (double red, double green, double blue) get_background_colour_rgb(
		  Cmiss_scene_viewer_id scene_viewer)

  int set_background_colour_rgb(Cmiss_scene_viewer_id scene_viewer,
        double red, double green, double blue)

  char *get_interactive_tool_name(Cmiss_scene_viewer_id scene_viewer)

  int set_interactive_tool_by_name(Cmiss_scene_viewer_id scene_viewer,
		  char *tool_name)

  char *get_scene_name(Cmiss_scene_viewer_id scene_viewer)

  int set_scene_by_name(Cmiss_scene_viewer_id scene_viewer,
		  char *scene_name)

  int view_all(Cmiss_scene_viewer_id scene_viewer)

  int redraw_now(Cmiss_scene_viewer_id scene_viewer)

  int write_image_to_file(Cmiss_scene_viewer_id scene_viewer,
        char *file_name, int force_onscreen, int preferred_width,
        int preferred_height)
=head1 AUTHOR

Shane Blackett <s.blackett@auckland.ac.nz>

=head1 SEE ALSO

L<perl> and L<Cmiss::graphics_window>.

=cut

