/*******************************************************************************
FILE : glyph.h

LAST MODIFIED : 20 November 2000

DESCRIPTION :
Glyphs are GT_objects which contain simple geometric shapes such as
cylinders, arrows and axes which are (or should) fit into a unit (1x1x1) cube,
with the major axes of the glyph aligned with the x, y and z axes. 
The logical centre of each glyph should be at (0,0,0). This should be
interpreted as follows:
- if the glyph is symmetrical along any axis, its coordinates in that
direction should vary from -0.5 to +0.5;
- if the glyph involves any sort of arrow that is unsymmetric in its direction
(ie. is single-headed), (0,0,0) should be at the base of the arrow.
- axes should therefore be centred at (0,0,0) and extend to 1 in each axis
direction. Axis titles "x", "y" and "z" may be outside the unit cube.

Glyphs are referenced by GT_glyph_set objects. Glyphs themselves should not
reference graphical materials or spectrums.
==============================================================================*/
#if !defined (GLYPH_H)
#define GLYPH_H
#include "graphics/graphics_object.h"

struct GT_object *make_glyph_arrow_line(char *name,float head_length,
	float half_head_width);
/*******************************************************************************
LAST MODIFIED : 3 August 1999

DESCRIPTION :
Creates a graphics object named <name> consisting of a line from <0,0,0> to
<1,0,0> with 4 arrow head ticks <head_length> long and <half_head_width> out
from the shaft.
==============================================================================*/

struct GT_object *make_glyph_arrow_solid(char *name,
	int number_of_segments_around,float shaft_length,float shaft_radius);
/*******************************************************************************
LAST MODIFIED : 17 July 1998

DESCRIPTION :
Creates a graphics object named <name> resembling an arrow made from a cone on
a cylinder. The base of the arrow is at (0,0,0) while its head lies at (1,0,0).
The radius of the cone is 0.5 at its base. The cylinder is <shaft_length> long
with its radius given by <shaft_radius>. The ends of the arrow and the cone
are both closed.
==============================================================================*/

struct GT_object *make_glyph_axes(char *name,float head_length,
	float half_head_width,float label_offset);
/*******************************************************************************
LAST MODIFIED : 20 July 1998

DESCRIPTION :
Creates a graphics object named <name> consisting of three axis arrows heading
from (0,0,0) to 1 in each of their directions. The arrows are made up of lines,
with a 4-way arrow head so it looks normal from the other two axes. A further
graphics object containing the axis labels 'x', 'y' and 'z' is attached to it so
that the two objects are displayed and destroyed together. The labels are
located on the respective axes <label_offset> past 1.0.
The length and width of the arrow heads are specified by the final parameters.
==============================================================================*/

struct GT_object *make_glyph_cone(char *name,int number_of_segments_around);
/*******************************************************************************
LAST MODIFIED : 17 July 1998

DESCRIPTION :
Creates a graphics object named <name> resembling a cone with the given
<number_of_segments_around>. The base of the cone is at (0,0,0) while its head
lies at (1,0,0). The radius of the cone is 0.5 at its base.
==============================================================================*/

struct GT_object *make_glyph_cross(char *name);
/*******************************************************************************
LAST MODIFIED : 16 July 1999

DESCRIPTION :
Creates a graphics object named <name> consisting of a 3 lines:
from <-0.5,0,0> to <+0.5,0,0>
from <0,-0.5,0> to <0,+0.5,0>
from <0,0,-0.5> to <0,0,+0.5>
==============================================================================*/

struct GT_object *make_glyph_cube_solid(char *name);
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Creates a graphics object named <name> consisting of a unit-sized GT_surface
cube centred at <0,0,0>.
==============================================================================*/

struct GT_object *make_glyph_cube_wireframe(char *name);
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Creates a graphics object named <name> consisting of lines marking a unit-sized
wireframe cube centred at <0,0,0>.
==============================================================================*/

struct GT_object *make_glyph_cylinder(char *name,int number_of_segments_around);
/*******************************************************************************
LAST MODIFIED : 16 July 1998

DESCRIPTION :
Creates a graphics object named <name> resembling a cylinder with the given
<number_of_segments_around>. The cylinder is centred at (0,0,0) and its axis
lies in the direction (1,0,0). It fits into the unit cube spanning from -0.5 to
+0.5 across all axes.
==============================================================================*/

struct GT_object *make_glyph_line(char *name);
/*******************************************************************************
LAST MODIFIED : 16 September 1998

DESCRIPTION :
Creates a graphics object named <name> consisting of a line from <-0.5,0,0> to
<0.5,0,0>.
==============================================================================*/

struct GT_object *make_glyph_mirror(char *name, struct GT_object *mirror_glyph);
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Makes a glyph with the given <name> that automatically mirrors the given
<mirror_glyph>.
==============================================================================*/

struct GT_object *make_glyph_point(char *name,gtMarkerType marker_type,
	float marker_size);
/*******************************************************************************
LAST MODIFIED : 1 December 1998

DESCRIPTION :
Creates a graphics object named <name> consisting of a single point at <0,0,0>.
The point will be drawn with the given <marker_type> and <marker_size>.
==============================================================================*/

struct GT_object *make_glyph_sheet(char *name);
/*******************************************************************************
LAST MODIFIED : 16 July 1998

DESCRIPTION :
Creates a graphics object named <name> resembling a square sheet spanning from
coordinate (-0.5,-0.5,0) to (0.5,0.5,0).
==============================================================================*/

struct GT_object *make_glyph_sphere(char *name,int number_of_segments_around,
	int number_of_segments_down);
/*******************************************************************************
LAST MODIFIED : 23 July 1998

DESCRIPTION :
Creates a graphics object named <name> resembling a sphere with the given
<number_of_segments_around> and <number_of_segments_down> from pole to pole.
The sphere is centred at (0,0,0) and its poles are on the (1,0,0) line. It fits
into the unit cube spanning from -0.5 to +0.5 across all axes. Parameter
<number_of_segments_around> should normally be an even number at least 6 and
twice <number_of_segments_down> look remotely spherical.
==============================================================================*/

#endif /* !defined (GLYPH_H) */
