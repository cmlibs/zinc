/*******************************************************************************
FILE : projection.h

LAST MODIFIED : 31 May 1997

DESCRIPTION :
???DB.  Started as mapping.h in emap
???DB.  Create own colour map
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#if !defined (PROJECTION_H)
#define PROJECTION_H

#include <stddef.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include "finite_element/finite_element.h"
#include "general/geometry.h"
#include "unemap/drawing_2d.h"
#include "unemap/mapping.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/
#if defined (OLD_CODE)
/*???DB.  Replaced by unemap/drawing_2d.h */
struct Projection_drawing
/*******************************************************************************
LAST MODIFIED : 23 June 1996

DESCRIPTION :
???DB.  Came from Drawing in emap
==============================================================================*/
{
	Widget widget;
	int depth,height,width;
	Pixmap pixel_map;
		/* stored by the X server */
	XImage *image;
		/* stored by the X client (application) machine */
	struct User_interface *user_interface;
}; /* struct Projection_drawing */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
/*???DB.  Replaced by unemap/mapping.h */
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
#endif /* defined (OLD_CODE) */

enum Nodes_option
/*******************************************************************************
LAST MODIFIED : 7 October 1995

DESCRIPTION :
==============================================================================*/
{
	SHOW_NODE_NAMES,
	SHOW_NODE_VALUES,
	HIDE_NODES
}; /* enum Electrodes_option */

#if defined (OLD_CODE)
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
#endif /* defined (OLD_CODE) */

enum Elements_option
/*******************************************************************************
LAST MODIFIED : 11 October 1996

DESCRIPTION :
==============================================================================*/
{
	SHOW_ELEMENT_BOUNDARIES_ONLY,
	SHOW_ELEMENT_NAMES_AND_BOUNDARIES,
	HIDE_ELEMENTS
}; /* enum elements_option */

#if defined (OLD_CODE)
enum Contour_thickness
/*******************************************************************************
LAST MODIFIED : 7 August 1994

DESCRIPTION :
==============================================================================*/
{
	CONSTANT_THICKNESS,
	VARIABLE_THICKNESS
}; /* enum Contour_thickness */
#endif /* defined (OLD_CODE) */

struct Projection
/*******************************************************************************
LAST MODIFIED : 31 May 1997

DESCRIPTION :
==============================================================================*/
{
	/* specify what is being displayed */
	struct FE_field *coordinate_field,*fibre_field,*field;
	int field_component;
	struct GROUP(FE_element) *element_group;
	/* parameter to control where the "slice" is */
		/*???DB.  To start with assume that it is always a fixed xi_3 */
	FE_value xi_3;
	/* specify how it is being displayed */
	enum Colour_option colour_option;
	enum Contours_option contours_option;
	enum Nodes_option nodes_option;
	enum Elements_option elements_option;
	enum Extrema_option extrema_option;
	enum Fibres_option fibres_option;
	enum Landmarks_option landmarks_option;
	enum Projection_type type;
	int maintain_aspect_ratio;
	int print_spectrum;
	int number_of_nodes;
	struct FE_node **nodes;
	char *node_drawn;
	float *node_depth;
	int *node_x,*node_y;
	float contour_maximum,contour_minimum;
		/*???DB.  How do contour values fit in with spectrum_range ? */
	int number_of_contours;
	int colour_bar_bottom,colour_bar_left,colour_bar_right,colour_bar_top;
	/* for drawing values on contours.  The drawing area is divided into equal
		sized area (number depends on a "density" specified in the user settings).
		For each area and each possible contour value there is a point (possibly
		(-1,-1)) for putting the contour value at */
	int number_of_contour_areas,number_of_contour_areas_in_x;
	short int *contour_x,*contour_y;
	enum Contour_thickness contour_thickness;
	/* save the interpolated value at each pixel to be used for contouring */
		/*???DB.  Is this the best place for this ?  Drawing size dependent */
	float *pixel_values;
	/* save the depth value at each pixel.  Used for determining what is in
		front */
		/*???DB.  Is this the best place for this ?  Drawing size dependent */
	float *pixel_depths;
	/* save the fibre angle at each pixel */
		/*???DB.  Is this the best place for this ?  Drawing size dependent */
	float *pixel_fibre_angles;
	/* set if the projection is being printed */
	char print;
	/* extrema */
	float maximum,minimum;
	int maximum_x,maximum_y,minimum_x,minimum_y;
	Colormap colour_map;
	struct
	{
		Pixel background_colour;
		Pixel boundary_colour;
		Pixel contour_colour;
		Pixel fibre_colour;
		Pixel landmark_colour;
		Pixel node_marker_colour;
		Pixel node_text_colour;
		Pixel *spectrum;
		Pixel spectrum_marker_colour;
		Pixel spectrum_text_colour;
	} pixel;
	struct
	{
		GC background_colour;
		GC contour_colour;
		GC copy;
		GC fibre_colour;
		GC node_marker_colour;
		GC node_text_colour;
		GC spectrum;
		GC spectrum_marker_colour;
		GC spectrum_text_colour;
	} graphics_context;
	XFontStruct *font;
	int number_of_spectrum_colours;
	struct Spectrum *spectrum;
	int expand_spectrum;
	int border_thickness_pixels,element_line_discretization,
		pixels_between_contour_values,seed_point_discretization;
	struct MANAGER(FE_element) *element_manager;
	struct User_interface *user_interface;
}; /* struct Projection */

/*
Global functions
----------------
*/
#if defined (OLD_CODE)
/*???DB.  Replaced by unemap/drawing_2d.h */
struct Projection_drawing *create_Projection_drawing(Widget widget,int width,
	int height,char create_image,struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 23 June 1996

DESCRIPTION :
This function allocates memory for a drawing and initializes the fields to the
specified values.  It returns a pointer to the created drawing if successful
and NULL if unsuccessful.
???DB.  Came from Drawing in emap
==============================================================================*/

int destroy_Projection_drawing(struct Projection_drawing **drawing);
/*******************************************************************************
LAST MODIFIED : 7 October 1995

DESCRIPTION :
This function frees the memory associated with the fields of <**drawing>, frees
the memory for <**drawing> and changes <*drawing> to NULL.
???DB.  Came from Drawing in emap
==============================================================================*/
#endif /* defined (OLD_CODE) */

struct Projection *CREATE(Projection)(Widget window,
	struct FE_field *coordinate_field,struct FE_field *fibre_field,
	struct FE_field *field,int field_component,
	struct GROUP(FE_element) *element_group,FE_value xi_3,
	enum Colour_option colour_option,enum Contours_option contours_option,
	enum Nodes_option nodes_option,enum Elements_option elements_option,
	enum Fibres_option fibres_option,enum Landmarks_option landmarks_option,
	enum Extrema_option extrema_option,int maintain_aspect_ratio,
	int print_spectrum,enum Projection_type projection_type,
	enum Contour_thickness contour_thickness,struct Spectrum *spectrum,
	int seed_point_discretization,
	int element_line_discretization,struct MANAGER(FE_element) *element_manager,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 22 May 1997

DESCRIPTION :
This function allocates memory for a projection and initializes the fields to
the specified values.  It returns a pointer to the created projection if
successful and NULL if not successful.
==============================================================================*/

int DESTROY(Projection)(struct Projection **projection);
/*******************************************************************************
LAST MODIFIED : 7 October 1995

DESCRIPTION :
This function deaccesses the fields of <**projection>, deallocates the memory
for <**projection> and sets <*projection> to NULL.
==============================================================================*/

int update_colour_map(struct Projection *projection);
/*******************************************************************************
LAST MODIFIED : 7 October 1995

DESCRIPTION :
Updates the colour map being used for <projection>.
==============================================================================*/

int draw_projection(struct Projection *projection,int recalculate,
	struct Drawing_2d *drawing);
#if defined (OLD_CODE)
/*???DB.  Replaced by unemap/drawing_2d.h */
	struct Projection_drawing *drawing);
#endif /* defined (OLD_CODE) */
/*******************************************************************************
LAST MODIFIED : 22 May 1997

DESCRIPTION :
This function draws the <projection> in the <drawing>.  If <recalculate> is >0
then the colours for the pixels are recalculated.  If <recalculate> is >1 then
the projection is also recalculated.
==============================================================================*/

int draw_spectrum_area(struct Projection *projection,
	struct Drawing_2d *drawing);
#if defined (OLD_CODE)
/*???DB.  Replaced by unemap/drawing_2d.h */
	struct Projection_drawing *drawing);
#endif /* defined (OLD_CODE) */
/*******************************************************************************
LAST MODIFIED : 3 January 1997

DESCRIPTION :
This function draws the colour bar in the <drawing>.
==============================================================================*/
#endif
