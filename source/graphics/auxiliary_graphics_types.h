/*******************************************************************************
FILE : auxiliary_graphics_types.h

LAST MODIFIED : 12 March 2002

DESCRIPTION :
Structures and enumerated types needed to produce graphics primitives but not
specific to any of them. Examples are:
- struct Element_discretization: stores the number of segments used to
represent curves in three xi-directions;
- Triple;
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
#if !defined (AUXILIARY_GRAPHICS_TYPES_H)
#define AUXILIARY_GRAPHICS_TYPES_H

#include "api/types/cmiss_graphics_render_type.h"
#include "general/enumerator.h"

struct User_interface;
struct Parse_state;

/*
Global types
------------
*/

struct Element_discretization
/*******************************************************************************
LAST MODIFIED : 23 November 2001

DESCRIPTION :
Stores the number of segment used in drawing each side of an element.
Controls quality of curve rendition.
==============================================================================*/
{
	int number_in_xi1,number_in_xi2,number_in_xi3;
}; /* struct Element_discretization */

typedef float Triple[3];

enum Graphics_compile_status
/*******************************************************************************
LAST MODIFIED : 12 March 2002

DESCRIPTION :
Graphical objects use this enumeration to flag their compile status, which
can indicate either no compilation is required, child graphics objects need
compiling, or the object and its children need compilation.
Currently applied only to OpenGL display lists, but relevant to other
intermediate storage systems.
==============================================================================*/
{
	GRAPHICS_COMPILED,
	CHILD_GRAPHICS_NOT_COMPILED,
	GRAPHICS_NOT_COMPILED
}; /* enum Graphics_compile_status */

enum Graphics_select_mode
/*******************************************************************************
LAST MODIFIED : 19 March 2001

DESCRIPTION :
Enumerator for specifying which parts of a graphic are named for selecting, if
any, and how they should be rendered depending on their selection status.
Note: the first value will be 0 by the ANSI standard, with each subsequent entry
incremented by 1. This pattern is expected by the ENUMERATOR macros.
Must ensure the ENUMERATOR_STRING function returns a string for each value here.
==============================================================================*/
{
	GRAPHICS_SELECT_MODE_INVALID = 0,
	GRAPHICS_SELECT_ON = 1,
	GRAPHICS_NO_SELECT = 2,
	GRAPHICS_DRAW_SELECTED = 3,
	GRAPHICS_DRAW_UNSELECTED = 4
}; /* enum Graphics_select_mode */

enum Streamline_type
/*******************************************************************************
LAST MODIFIED : 19 March 2001

DESCRIPTION :
Note: the first value will be 0 by the ANSI standard, with each subsequent entry
incremented by 1. This pattern is expected by the ENUMERATOR macros.
Must ensure the ENUMERATOR_STRING function returns a string for each value here.
for automatic creation of choose_enumerator widgets.
==============================================================================*/
{
	STREAM_INVALID = 0,
	STREAM_EXTRUDED_ELLIPSE = 1,
	STREAM_LINE = 2,
	STREAM_EXTRUDED_RECTANGLE = 3,
	STREAM_RIBBON = 4,
	STREAM_EXTRUDED_CIRCLE = 5
};

enum Streamline_data_type
/*******************************************************************************
LAST MODIFIED : 19 March 2001

DESCRIPTION :
Note: the first value will be 0 by the ANSI standard, with each subsequent entry
incremented by 1. This pattern is expected by the ENUMERATOR macros.
Must ensure the ENUMERATOR_STRING function returns a string for each value here.
for automatic creation of choose_enumerator widgets.
==============================================================================*/
{
	STREAM_DATA_INVALID = 0,
	STREAM_NO_DATA = 1,          /* The code relies on NODATA being zero as the
										          field creation types test if(data_type) */
	STREAM_FIELD_SCALAR = 2,     /* Generalised scalar as in other graphics objects */
	STREAM_MAGNITUDE_SCALAR = 3, /* The vector is necessarily calculated already
										          so can save computation by no using the scalar */
	STREAM_TRAVEL_SCALAR = 4     /* Integrate time along the curve so that you can
										          see how long each bit is */
};

enum Xi_discretization_mode
/*******************************************************************************
LAST MODIFIED : 1 May 2001

DESCRIPTION :
Enumerator for controlling where discrete objects are placed in the Xi space of
each element, eg. glyphs for element_points.
"CELL" types divide the element into cells of equal size in Xi space.
CELL_CENTRES puts one object at the centes of each cell.
CELL_CORNERS puts an object at the corners of each cell, but not repeating any
on neighbouring cells.
CELL_DENSITY puts a number of randomly-located points in each cell proportional
to the value of a density_field calculated at the centre of the cell and the
volume of the cell determined from the coordinate_field.
Note: the first value will be 0 by the ANSI standard, with each subsequent entry
incremented by 1. This pattern is expected by the ENUMERATOR macros.
Must ensure the ENUMERATOR_STRING function returns a string for each value here.
==============================================================================*/
{
	XI_DISCRETIZATION_INVALID_MODE = 0,
	XI_DISCRETIZATION_CELL_CENTRES = 1,
	XI_DISCRETIZATION_CELL_CORNERS = 2,
	/* number of points in each cell is rounded from the density*volume. Best
		 choice for showing density when cells are near-uniform sizes and there are
		 a reasonable number of points in most cells */
	XI_DISCRETIZATION_CELL_DENSITY = 3,
	/* same as XI_DISCRETIZATION_CELL_DENSITY but actual number of points per
		 cell is sampled from a Poisson distribution with mean density*volume.
		 May be better than CELL_DENSITY when cells are quite different in size,
		 but adds noise to the density field being viewed. */
	XI_DISCRETIZATION_CELL_POISSON = 4,
	/* exactly one point per cell at all times */
	XI_DISCRETIZATION_CELL_RANDOM = 5,
	XI_DISCRETIZATION_EXACT_XI = 6
}; /* enum Xi_discretization_mode */

/*
Global functions
----------------
*/

int set_exterior(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 29 June 1996

DESCRIPTION :
A modifier function for setting exterior flag and face number.
==============================================================================*/

int set_Circle_discretization(struct Parse_state *state,
	void *circle_discretization_void,void *user_interface_void);
/*******************************************************************************
LAST MODIFIED : 2 June 1998

DESCRIPTION :
A modifier function for setting number of segments used to draw circles.
==============================================================================*/

int set_Element_discretization(struct Parse_state *state,
	void *element_discretization_void,void *user_interface_void);
/*******************************************************************************
LAST MODIFIED : 30 October 1996

DESCRIPTION :
A modifier function for setting discretization in each element direction.
==============================================================================*/

PROTOTYPE_ENUMERATOR_FUNCTIONS(Graphics_select_mode);

PROTOTYPE_ENUMERATOR_FUNCTIONS(Cmiss_graphics_render_type);

PROTOTYPE_ENUMERATOR_FUNCTIONS(Streamline_type);

PROTOTYPE_ENUMERATOR_FUNCTIONS(Streamline_data_type);

PROTOTYPE_ENUMERATOR_FUNCTIONS(Xi_discretization_mode);

#endif /* AUXILIARY_GRAPHICS_TYPES_H */
