/*******************************************************************************
FILE : auxiliary_graphics_types.h

LAST MODIFIED : 7 June 2000

DESCRIPTION :
Structures and enumerated types needed to produce graphics primitives but not
specific to any of them. Examples are:
- struct Element_discretization: stores the number of segments used to
represent curves in three xi-directions;
- Triple;
==============================================================================*/
#if !defined (AUXILIARY_GRAPHICS_TYPES_H)
#define AUXILIARY_GRAPHICS_TYPES_H

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

enum Graphics_select_mode
/*******************************************************************************
LAST MODIFIED : 23 February 2000

DESCRIPTION :
Enumerator for specifying which parts of a graphic are named for selecting, if
any, and how they should be rendered depending on their selection status.
Have members BEFORE_FIRST and AFTER_LAST to enable iterating through the
list for automatic creation of choose_enumerator widgets.
==============================================================================*/
{
	GRAPHICS_SELECT_MODE_INVALID,
	GRAPHICS_SELECT_MODE_BEFORE_FIRST,
	GRAPHICS_SELECT_ON,
	GRAPHICS_NO_SELECT,
	GRAPHICS_DRAW_SELECTED,
	GRAPHICS_DRAW_UNSELECTED,
	GRAPHICS_SELECT_MODE_AFTER_LAST
}; /* enum Graphics_select_mode */

enum Streamline_type
/*******************************************************************************
LAST MODIFIED : 22 December 1999

DESCRIPTION :
Must keep function Streamline_type_string up-to-date with entries here.
Have members BEFORE_FIRST and AFTER_LAST to enable iterating through the list
for automatic creation of choose_enumerator widgets.
==============================================================================*/
{
	STREAMLINE_TYPE_INVALID,
	STREAMLINE_TYPE_BEFORE_FIRST,
	STREAM_EXTRUDED_ELLIPSE,
	STREAM_LINE,
	STREAM_EXTRUDED_RECTANGLE,
	STREAM_RIBBON,
	STREAMLINE_TYPE_AFTER_LAST
};

enum Streamline_data_type
/*******************************************************************************
LAST MODIFIED : 24 March 1999

DESCRIPTION :
Must keep function Streamline_data_type_string up-to-date with entries here.
Have members BEFORE_FIRST and AFTER_LAST to enable iterating through the list
for automatic creation of choose_enumerator widgets.
==============================================================================*/
{
	STREAMLINE_DATA_TYPE_INVALID,
	STREAMLINE_DATA_TYPE_BEFORE_FIRST,
	STREAM_NO_DATA,          /* The code relies on NODATA being zero as the
										          field creation types test if(data_type) */
	STREAM_FIELD_SCALAR,     /* Generalised scalar as in other graphics objects */
	STREAM_MAGNITUDE_SCALAR, /* The vector is necessarily calculated already
										          so can save computation by no using the scalar */
	STREAM_TRAVEL_SCALAR,    /* Integrate time along the curve so that you can
										          see how long each bit is */
	STREAMLINE_DATA_TYPE_AFTER_LAST
};

typedef float Triple[3];

enum Xi_discretization_mode
/*******************************************************************************
LAST MODIFIED : 7 June 2000

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
Have members BEFORE_FIRST and AFTER_LAST to allow iteration through the modes
in functions (used for parsing/making chooser widgets for enumeration):
Xi_discretization_mode_from_string;
Xi_discretization_mode_get_valid_strings;

Make sure Xi_discretization_mode_string handles any enumerated values you add.
==============================================================================*/
{
	XI_DISCRETIZATION_INVALID,
	XI_DISCRETIZATION_MODE_BEFORE_FIRST,
	XI_DISCRETIZATION_CELL_CENTRES,
	XI_DISCRETIZATION_CELL_CORNERS,
	XI_DISCRETIZATION_CELL_RANDOM,
	XI_DISCRETIZATION_EXACT_XI,
	XI_DISCRETIZATION_MODE_AFTER_LAST
}; /* enum Xi_discretization_mode */

enum Render_type
/*******************************************************************************
LAST MODIFIED : 2 May 2000

DESCRIPTION :
Must keep function Render_type_string up-to-date with entries here.
Have members BEFORE_FIRST and AFTER_LAST to enable iterating through the list
for automatic creation of choose_enumerator widgets.
==============================================================================*/
{
	RENDER_TYPE_INVALID,
	RENDER_TYPE_BEFORE_FIRST,
	RENDER_TYPE_SHADED,
	RENDER_TYPE_WIREFRAME,
	RENDER_TYPE_AFTER_LAST
};

/*
Global functions
----------------
*/
char *Graphics_select_mode_string(enum Graphics_select_mode select_mode);
/*******************************************************************************
LAST MODIFIED : 23 February 2000

DESCRIPTION :
Returns a pointer to a static string describing the select_mode, eg.
GRAPHICS_NO_SELECT="no_select".This string should match the command
used to enact the mode. The returned string must not be DEALLOCATEd!
==============================================================================*/

char **Graphics_select_mode_get_valid_strings(int *number_of_valid_strings);
/*******************************************************************************
LAST MODIFIED : 23 February 2000

DESCRIPTION :
Returns an allocated array of pointers to all static strings for valid
Graphics_select_modes - obtained from function Graphics_select_mode_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/

enum Graphics_select_mode Graphics_select_mode_from_string(
	char *select_mode_string);
/*******************************************************************************
LAST MODIFIED : 23 February 2000

DESCRIPTION :
Returns the <Graphics_select_mode> described by <select_mode_string>,
or GRAPHICS_SELECT_MODE_INVALID if not recognized.
==============================================================================*/

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

char *Xi_discretization_mode_string(
	enum Xi_discretization_mode xi_discretization_mode);
/*******************************************************************************
LAST MODIFIED : 2 March 1999

DESCRIPTION :
Returns a pointer to a static string describing the <xi_discretization_mode>,
eg. XI_DISCRETIZATION_MODE_CELL_CENTRES == "cell_centres". This string should
match the command used to create that type of settings.
The returned string must not be DEALLOCATEd!
==============================================================================*/

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

char **Xi_discretization_mode_get_valid_strings(int *number_of_valid_strings);
/*******************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Returns and allocated array of pointers to all static strings for valid
Xi_discretization_modes - obtained from function Xi_discretization_mode_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/

enum Xi_discretization_mode Xi_discretization_mode_from_string(
	char *xi_discretization_mode_string);
/*******************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Returns the <Xi_discretization_mode> described by
<xi_discretization_mode_string>.
==============================================================================*/

char *Streamline_type_string(enum Streamline_type streamline_type);
/*******************************************************************************
LAST MODIFIED : 19 March 1999

DESCRIPTION :
Returns a pointer to a static string describing the streamline_type, eg.
STREAM_LINE == "line". This string should match the command used
to create that type of streamline. The returned string must not be DEALLOCATEd!
==============================================================================*/

char **Streamline_type_get_valid_strings(int *number_of_valid_strings);
/*******************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Returns and allocated array of pointers to all static strings for valid
Streamline_types - obtained from function Streamline_type_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/

enum Streamline_type Streamline_type_from_string(char *streamline_type_string);
/*******************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Returns the <Streamline_type> described by <streamline_type_string>.
==============================================================================*/

char *Streamline_data_type_string(
	enum Streamline_data_type streamline_data_type);
/*******************************************************************************
LAST MODIFIED : 19 March 1999

DESCRIPTION :
Returns a pointer to a static string describing the streamline_data_type, eg.
STREAM_FIELD_SCALAR == "field_scalar". This string should match the command used
to create that type of streamline. The returned string must not be DEALLOCATEd!
==============================================================================*/

char **Streamline_data_type_get_valid_strings(int *number_of_valid_strings);
/*******************************************************************************
LAST MODIFIED : 22 March 1999

DESCRIPTION :
Returns and allocated array of pointers to all static strings for valid
Streamline_data_types - obtained from function Streamline_data_type_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/

enum Streamline_data_type Streamline_data_type_from_string(
	char *streamline_data_type_string);
/*******************************************************************************
LAST MODIFIED : 13 July 1999

DESCRIPTION :
Returns the <Streamline_data_type> described by <streamline_data_type_string>,
or NULL if not recognized.
==============================================================================*/

char *Render_type_string(enum Render_type render_type);
/*******************************************************************************
LAST MODIFIED : 2 May 2000

DESCRIPTION :
Returns a pointer to a static string describing the render_type, eg.
STREAM_LINE == "line". This string should match the command used
to create that type of render. The returned string must not be DEALLOCATEd!
==============================================================================*/

char **Render_type_get_valid_strings(int *number_of_valid_strings);
/*******************************************************************************
LAST MODIFIED : 2 May 2000

DESCRIPTION :
Returns and allocated array of pointers to all static strings for valid
Render_types - obtained from function Render_type_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/

enum Render_type Render_type_from_string(char *render_type_string);
/*******************************************************************************
LAST MODIFIED : 2 May 2000

DESCRIPTION :
Returns the <Render_type> described by <render_type_string>.
==============================================================================*/

#endif /* AUXILIARY_GRAPHICS_TYPES_H */
