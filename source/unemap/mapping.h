/*******************************************************************************
FILE : mapping.h

LAST MODIFIED : 14 July 1998

DESCRIPTION :
==============================================================================*/
#if !defined (MAPPING_H)
#define MAPPING_H

#include <stddef.h>
#if defined (MOTIF)
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#endif /* defined (MOTIF) */
#include "general/geometry.h"
#include "graphics/spectrum.h"
#include "unemap/drawing_2d.h"
#include "unemap/rig.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/
enum Map_type
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
==============================================================================*/
{
	NO_MAP_FIELD,
	SINGLE_ACTIVATION,
	MULTIPLE_ACTIVATION,
	INTEGRAL,
	POTENTIAL
}; /* enum Map_type */

enum Interpolation_type
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
==============================================================================*/
{
	NO_INTERPOLATION,
	BICUBIC_INTERPOLATION,
	BILINEAR_INTERPOLATION
}; /* enum Interpolation_type */

#if defined (OLD_CODE)
/*???DB.  Replaced by Projection_type in general/geometry.h */
enum Projection
/*******************************************************************************
LAST MODIFIED : 24 June 1995

DESCRIPTION :
???DB.  There will be different types of CYLINDRICAL, depending on where the
cut is and if the projection should overlap (3/2 times round).
==============================================================================*/
{
	CYLINDRICAL,
	HAMMER,
	POLAR
}; /* enum Projection */
#endif /* defined (OLD_CODE) */

enum Colour_option
/*******************************************************************************
LAST MODIFIED : 3 February 1993

DESCRIPTION :
==============================================================================*/
{
	SHOW_COLOUR,
	HIDE_COLOUR
}; /* enum Colour_option */

enum Contours_option
/*******************************************************************************
LAST MODIFIED : 3 February 1993

DESCRIPTION :
==============================================================================*/
{
	SHOW_CONTOURS,
	HIDE_CONTOURS
}; /* enum Contours_option */

enum Electrodes_option
/*******************************************************************************
LAST MODIFIED : 18 June 1998

DESCRIPTION :
==============================================================================*/
{
	SHOW_ELECTRODE_NAMES,
	SHOW_ELECTRODE_VALUES,
	SHOW_CHANNEL_NUMBERS,
	HIDE_ELECTRODES
}; /* enum Electrodes_option */

enum Electrodes_marker_type
/*******************************************************************************
LAST MODIFIED : 24 July 1998

DESCRIPTION :
==============================================================================*/
{
	CIRCLE_ELECTRODE_MARKER,
	PLUS_ELECTRODE_MARKER,
	SQUARE_ELECTRODE_MARKER
}; /* enum Electrodes_marker_type */

enum Fibres_option
/*******************************************************************************
LAST MODIFIED : 4 February 1994

DESCRIPTION :
==============================================================================*/
{
	HIDE_FIBRES,
	SHOW_FIBRES_FINE,
	SHOW_FIBRES_MEDIUM,
	SHOW_FIBRES_COARSE
}; /* enum Fibres_option */

enum Landmarks_option
/*******************************************************************************
LAST MODIFIED : 4 February 1994

DESCRIPTION :
==============================================================================*/
{
	SHOW_LANDMARKS,
	HIDE_LANDMARKS
}; /* enum Landmarks_option */

enum Extrema_option
/*******************************************************************************
LAST MODIFIED : 22 May 1997

DESCRIPTION :
==============================================================================*/
{
	SHOW_EXTREMA,
	HIDE_EXTREMA
}; /* enum Extrema_option */

enum Contour_thickness
/*******************************************************************************
LAST MODIFIED : 7 August 1994

DESCRIPTION :
==============================================================================*/
{
	CONSTANT_THICKNESS,
	VARIABLE_THICKNESS
}; /* enum Contour_thickness */

struct Map_drawing_information
/*******************************************************************************
LAST MODIFIED : 27 September 1999

DESCRIPTION :
Information needed for drawing a map.  Windowing system dependent
==============================================================================*/
{
#if defined (MOTIF)
	Boolean maintain_aspect_ratio;
	Colormap colour_map;
	int read_only_colour_map;
#endif /* defined (MOTIF) */
	int number_of_spectrum_colours,pixels_between_contour_values;
#if defined (MOTIF)
	Pixel background_drawing_colour,boundary_colour,contour_colour,fibre_colour,
		highlighted_colour,landmark_colour,node_text_colour,*spectrum_colours,
		spectrum_text_colour,unhighlighted_colour;
	struct
	{
		GC background_drawing_colour,contour_colour,copy,fibre_colour,
			highlighted_colour,node_marker_colour,spectrum,spectrum_marker_colour,
			unhighlighted_colour;
		/*???DB.  When using an RS/6000 model with the Color Graphics Display
			Adapter (#2770), GCs stop writing text to the pixel map after they've
			been used for drawing lines to the window.  So I duplicate them */
		/*???DB.  How should background_drawing_colour,copy be shared ? */
		GC node_text_colour,spectrum_text_colour;
	} graphics_context;
#endif /* defined (MOTIF) */
	struct Spectrum *spectrum;
#if defined (MOTIF)
	XColor *spectrum_rgb;
	XFontStruct *font;
#endif /* defined (MOTIF) */
	struct User_interface *user_interface;
}; /* struct Map_drawing_information */

struct Map_frame
/*******************************************************************************
LAST MODIFIED : 7 October 1997

DESCRIPTION :
==============================================================================*/
{
	/* save the interpolated value at each pixel to be used for contouring */
		/*???DB.  Is this the best place for this ?  Drawing size dependent */
	float *pixel_values;
	/* extrema */
	float maximum,minimum;
	int maximum_x,maximum_y,minimum_x,minimum_y;
	struct Region *maximum_region,*minimum_region;
	/* for writing contour values */
	short int *contour_x,*contour_y;
#if defined (MOTIF)
	XImage *image;
		/* stored by the X client (application) machine */
#endif /* defined (MOTIF) */
}; /* struct Map_frame */

struct Map
/*******************************************************************************
LAST MODIFIED : 22 June 1998

DESCRIPTION :
==============================================================================*/
{
	enum Map_type *type;
	int *event_number;
	int *potential_time;
	int *datum;
	/* for integral maps */
	int *end_search_interval,*start_search_interval;
	enum Colour_option colour_option;
	enum Contours_option contours_option;
	enum Electrodes_option electrodes_option;
	enum Electrodes_marker_type electrodes_marker_type;
	int electrodes_marker_size;
	enum Interpolation_type interpolation_type;
	enum Fibres_option fibres_option;
	enum Landmarks_option landmarks_option;
	enum Extrema_option extrema_option;
	enum Projection_type projection_type;
	int maintain_aspect_ratio;
	int print_spectrum;
	char undecided_accepted;
	struct Rig **rig_pointer;
	int number_of_electrodes;
	struct Device **electrodes;
	int *electrode_x,*electrode_y;
	float *electrode_value;
	int number_of_auxiliary;
	char *electrode_drawn;
	struct Device **auxiliary;
	int *auxiliary_x,*auxiliary_y;
	int fixed_range;
		/*???DB.  Should be combined with spectrum_range */
	float contour_maximum,contour_minimum,maximum_value,minimum_value;
	int activation_front,number_of_contours;
		/*???DB.  Replace activation_front by frame_number ? */
	int colour_bar_bottom,colour_bar_left,colour_bar_right,colour_bar_top;
	/* for drawing values on contours.  The drawing area is divided into equal
		sized area (number depends on a "density" specified in the user settings).
		For each area and each possible contour value there is a point (possibly
		(-1,-1)) for putting the contour value at */
	int number_of_contour_areas,number_of_contour_areas_in_x;
	enum Contour_thickness contour_thickness;
	/* for potential movies */
	float frame_end_time,frame_start_time;
	int frame_number,number_of_frames;
	struct Map_frame *frames;
	/* set if the map is being printed */
	char print;
	/*???DB.  Used to be in user_settings */
	float membrane_smoothing,plate_bending_smoothing;
	int finite_element_mesh_columns,finite_element_mesh_rows;
	struct Map_drawing_information *drawing_information;
#if defined (OLD_CODE)
	struct User_interface *user_interface;
#endif /* defined (OLD_CODE) */
	struct Unemap_package *unemap_package;
}; /* struct Map */

/*
Global functions
----------------
*/
struct Map *create_Map(enum Map_type *map_type,enum Colour_option colour_option,
	enum Contours_option contours_option,enum Electrodes_option electrodes_option,
	enum Fibres_option fibres_option,enum Landmarks_option landmarks_option,
	enum Extrema_option extrema_option,int maintain_aspect_ratio,
	int print_spectrum,enum Projection_type projection_type,
	enum Contour_thickness contour_thickness,struct Rig **rig_pointer,
	int *event_number_address,int *potential_time_address,int *datum_address,
	int *start_search_interval_address,int *end_search_interval_address,
	struct Map_drawing_information *map_drawing_information,
	struct User_interface *user_interface,struct Unemap_package *package);
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION :
This function allocates memory for a map and initializes the fields to the
specified values.  It returns a pointer to the created map if successful and
NULL if not successful.
==============================================================================*/

int destroy_Map(struct Map **map);
/*******************************************************************************
LAST MODIFIED : 19 May 1992

DESCRIPTION :
This function deallocates the memory associated with the fields of <**map>
(except the <rig_pointer> field), deallocates the memory for <**map> and sets
<*map> to NULL.
==============================================================================*/

int update_colour_map_unemap(struct Map *map,struct Drawing_2d *drawing);
/*******************************************************************************
LAST MODIFIED : 6 July 1997

DESCRIPTION :
Updates the colour map being used for map.
???DB.  <drawing> added because of read only colour maps.
==============================================================================*/

int draw_map(struct Map *map,int recalculate,struct Drawing_2d *drawing);
/*******************************************************************************
LAST MODIFIED : 7 October 1997

DESCRIPTION :
This function draws the <map> in the <drawing>.  If <recalculate> is >0 then the
colours for the pixels are recalculated.  If <recalculate> is >1 then the
interpolation functions are also recalculated.  If <recalculate> is >2 then the
<map> is resized to match the <drawing>.
==============================================================================*/

int draw_colour_or_auxiliary_area(struct Map *map,struct Drawing_2d *drawing);
/*******************************************************************************
LAST MODIFIED : 20 December 1996

DESCRIPTION :
This function draws the colour bar or the auxiliary inputs in the <drawing>.
==============================================================================*/

struct Map_drawing_information *create_Map_drawing_information(
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 19 June 1997

DESCRIPTION :
==============================================================================*/

int destroy_Map_drawing_information(
	struct Map_drawing_information **map_drawing_information_address);
/*******************************************************************************
LAST MODIFIED : 21 June 1997

DESCRIPTION :
==============================================================================*/
#endif /* !defined (MAPPING_H) */
