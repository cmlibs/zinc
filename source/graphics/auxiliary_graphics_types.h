/*******************************************************************************
FILE : auxiliary_graphics_types.h

LAST MODIFIED : 19 March 2001

DESCRIPTION :
Structures and enumerated types needed to produce graphics primitives but not
specific to any of them. Examples are:
- struct Element_discretization: stores the number of segments used to
represent curves in three xi-directions;
- Triple;
==============================================================================*/
#if !defined (AUXILIARY_GRAPHICS_TYPES_H)
#define AUXILIARY_GRAPHICS_TYPES_H

#include "general/enumerator.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/

struct Element_discretization
/*******************************************************************************
LAST MODIFIED : 22 September 1997

DESCRIPTION :
==============================================================================*/
{
	int number_in_xi1,number_in_xi2,number_in_xi3;
}; /* struct Element_discretization */

typedef float Triple[3];

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
	GRAPHICS_SELECT_ON,
	GRAPHICS_NO_SELECT,
	GRAPHICS_DRAW_SELECTED,
	GRAPHICS_DRAW_UNSELECTED
}; /* enum Graphics_select_mode */

enum Render_type
/*******************************************************************************
LAST MODIFIED : 19 March 2001

DESCRIPTION :
Note: the first value will be 0 by the ANSI standard, with each subsequent entry
incremented by 1. This pattern is expected by the ENUMERATOR macros.
Must ensure the ENUMERATOR_STRING function returns a string for each value here.
==============================================================================*/
{
	RENDER_TYPE_SHADED,
	RENDER_TYPE_WIREFRAME
};

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
	STREAM_EXTRUDED_ELLIPSE,
	STREAM_LINE,
	STREAM_EXTRUDED_RECTANGLE,
	STREAM_RIBBON
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
	STREAM_NO_DATA,          /* The code relies on NODATA being zero as the
										          field creation types test if(data_type) */
	STREAM_FIELD_SCALAR,     /* Generalised scalar as in other graphics objects */
	STREAM_MAGNITUDE_SCALAR, /* The vector is necessarily calculated already
										          so can save computation by no using the scalar */
	STREAM_TRAVEL_SCALAR     /* Integrate time along the curve so that you can
										          see how long each bit is */
};

enum Xi_discretization_mode
/*******************************************************************************
LAST MODIFIED : 19 March 2001

DESCRIPTION :
Enumerator for controlling where discrete objects are placed in the Xi space of
each element, eg. glyphs for element_points.
"CELL" types divide the element into cells of equal size in Xi space.
CELL_CENTRES puts one object at the centes of each cell.
CELL_CORNERS puts an object at the corners of each cell, but not repeating any
on neighbouring cells.
CELL_RANDOM puts a single object at a random location in each cell, to remove
the regularity of the above types.
In future, may wish to add further modes for specifying random locations at
specified densities in real x,y,z space - for more realistic point clouds, esp.
in cases where elements are not of uniform size.
Note: the first value will be 0 by the ANSI standard, with each subsequent entry
incremented by 1. This pattern is expected by the ENUMERATOR macros.
Must ensure the ENUMERATOR_STRING function returns a string for each value here.
==============================================================================*/
{
	XI_DISCRETIZATION_CELL_CENTRES,
	XI_DISCRETIZATION_CELL_CORNERS,
	XI_DISCRETIZATION_CELL_RANDOM,
	XI_DISCRETIZATION_EXACT_XI
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

int read_circle_discretization_defaults(int *default_value,
	int *maximum_value,struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 2 June 1998

DESCRIPTION :
Reads that maximum and default number of line segments used to approximate
a circle. Minimum is always 2, but this does not look much like a circle!
==============================================================================*/

int check_Circle_discretization(int *circle_discretization,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 2 June 1998

DESCRIPTION :
Ensures the <circle_discretization> is within the bounds of the minimum of 2
and the maximum read in from the defaults.
==============================================================================*/

int set_Circle_discretization(struct Parse_state *state,
	void *circle_discretization_void,void *user_interface_void);
/*******************************************************************************
LAST MODIFIED : 2 June 1998

DESCRIPTION :
A modifier function for setting number of segments used to draw circles.
==============================================================================*/

int read_element_discretization_defaults(int *default_value,
	int *maximum_value,struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 2 June 1998

DESCRIPTION :
Reads that maximum and default number of line segments used to approximate
element curves. Minimum is always 1.
???RC. Actually uses points = 1 greater than number of segments so minimum is
2 - this is due to change.
==============================================================================*/

int check_Element_discretization(struct Element_discretization
	*element_discretization,struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 2 June 1998

DESCRIPTION :
Ensures the <element_discretization> is within the bounds of the minimum of 1
and the maximum read in from the defaults.
???DB.  Changed the lower bound to 1 because also used for elements.
???DB.  Need to make consistent.
==============================================================================*/

int set_Element_discretization(struct Parse_state *state,
	void *element_discretization_void,void *user_interface_void);
/*******************************************************************************
LAST MODIFIED : 30 October 1996

DESCRIPTION :
A modifier function for setting discretization in each element direction.
==============================================================================*/

PROTOTYPE_ENUMERATOR_FUNCTIONS(Graphics_select_mode);

PROTOTYPE_ENUMERATOR_FUNCTIONS(Render_type);

PROTOTYPE_ENUMERATOR_FUNCTIONS(Streamline_type);

PROTOTYPE_ENUMERATOR_FUNCTIONS(Streamline_data_type);

PROTOTYPE_ENUMERATOR_FUNCTIONS(Xi_discretization_mode);

int Xi_discretization_mode_get_number_of_xi_points(
	enum Xi_discretization_mode xi_discretization_mode,int dimension,
	int *number_in_xi);
/*******************************************************************************
LAST MODIFIED : 28 March 2000

DESCRIPTION :
Returns the number of points that should be created for <xi_discretization_mode>
in an element of the given <dimension> with <number_in_xi> cells in each
xi direction. Returns zero if the number_in_xi are invalid (less than 1) in any
direction.
==============================================================================*/

Triple *Xi_discretization_mode_get_xi_points(
	enum Xi_discretization_mode xi_discretization_mode,int dimension,
	int *number_in_xi,Triple exact_xi,int *number_of_xi_points);
/*******************************************************************************
LAST MODIFIED : 7 June 2000

DESCRIPTION :
Allocates and returns the set of points for <xi_discretization_mode>
in an element of the given <dimension> with <number_in_xi> cells in each
xi direction. Layout of points is controlled by the <xi_discretization_mode>.
Function also returns <number_of_xi_points> calculated. Xi positions are always
returned as triples with remaining xi coordinates 0 for 1-D and 2-D cases.
Note: xi changes from 0 to 1 over each element direction.
<exact_xi> should be supplied for mode XI_DISCRETIZATION_EXACT_XI - passed and
allocated here for a consistent interface.
==============================================================================*/

int Xi_discretization_mode_get_element_point_xi(
	enum Xi_discretization_mode xi_discretization_mode,int dimension,
	int *number_in_xi,Triple exact_xi,int element_point_number,FE_value *xi);
/*******************************************************************************
LAST MODIFIED : 7 June 2000

DESCRIPTION :
Returns in <xi> the single xi location for <element_point_number> from those
that would be returned by Xi_discretization_mode_get_xi_points.
Fails for truly random discretization modes.
<exact_xi> should be supplied for mode XI_DISCRETIZATION_EXACT_XI - passed here
for a consistent interface.
==============================================================================*/

#endif /* AUXILIARY_GRAPHICS_TYPES_H */
