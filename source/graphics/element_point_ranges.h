/*******************************************************************************
FILE : element_point_ranges.h

LAST MODIFIED : 30 May 2000

DESCRIPTION :
Structure for storing ranges of points in elements according to the various
Xi_discretization_modes.
==============================================================================*/
#if !defined (ELEMENT_POINT_RANGES_H)
#define ELEMENT_POINT_RANGES_H

#include "finite_element/finite_element.h"
#include "general/list.h"
#include "general/multi_range.h"
#include "general/object.h"
#include "graphics/auxiliary_graphics_types.h"

struct Element_point_ranges_identifier
/*******************************************************************************
LAST MODIFIED : 25 February 2000

DESCRIPTION :
Identifier created to allow LIST(Element_point_ranges) to be indexed for quick
access to objects. Objects are indexed by element first, then the
Xi_discretization_mode and any other identifying values relevant to the
Xi_discretization_mode.
See also compare_Element_point_ranges_identifier function.
==============================================================================*/
{
	struct FE_element *element;
	enum Xi_discretization_mode xi_discretization_mode;
	/* following could/should be in a union */
	int number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
}; /* Element_point_ranges_identifier */

struct Element_point_ranges;
/*******************************************************************************
LAST MODIFIED : 25 February 2000

DESCRIPTION :
Stores ranges of element/grid points in an element, used for selection.
The contents of this object are private.
==============================================================================*/

DECLARE_LIST_TYPES(Element_point_ranges);

struct FE_element_grid_to_Element_point_ranges_list_data
/*******************************************************************************
LAST MODIFIED : 26 May 2000

DESCRIPTION :
Data for passing to FE_element_grid_to_Element_point_ranges_list.
<grid_fe_field> must be a single component integer, <grid_value_ranges>
contains the range of values of <grid_fe_field> for which points are added to
<element_point_ranges_list>.
==============================================================================*/
{
	struct LIST(Element_point_ranges) *element_point_ranges_list;
	struct FE_field *grid_fe_field;
	struct Multi_range *grid_value_ranges;
};

/*
Global functions
----------------
*/

char **Xi_discretization_mode_get_valid_strings_for_Element_point_ranges(
	int *number_of_valid_strings);
/*******************************************************************************
LAST MODIFIED : 30 May 2000

DESCRIPTION :
Returns an allocated array of pointers to all static strings for valid
Xi_discretization_modes that can be used for Element_point_ranges, obtained
from function Xi_discretization_mode_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/

int Element_point_ranges_identifier_is_valid(
	struct Element_point_ranges_identifier *identifier);
/*******************************************************************************
LAST MODIFIED : 24 May 2000

DESCRIPTION :
Returns true if <identifier> has a valid element, Xi_discretization_mode and
number_in_xi for being used in an Element_point_ranges structure.
==============================================================================*/

int Element_point_ranges_identifier_element_point_number_is_valid(
	struct Element_point_ranges_identifier *identifier,int element_point_number);
/*******************************************************************************
LAST MODIFIED : 24 May 2000

DESCRIPTION :
Returns true if <element_point_number> is in the number_in_xi range for
<identifier>. Assumes <identifier> is already validated by
Element_point_ranges_identifier_is_valid.
==============================================================================*/

struct Element_point_ranges *CREATE(Element_point_ranges)(
	struct Element_point_ranges_identifier *identifier);
/*******************************************************************************
LAST MODIFIED : 28 February 2000

DESCRIPTION :
Creates an Element_point_ranges object that can store ranges of points in the
element:Xi_discretization_mode of the <identifier>.
==============================================================================*/

int DESTROY(Element_point_ranges)(
	struct Element_point_ranges **element_point_ranges_address);
/*******************************************************************************
LAST MODIFIED : 28 February 2000

DESCRIPTION :
Destroys the Element_point_ranges.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Element_point_ranges);
PROTOTYPE_LIST_FUNCTIONS(Element_point_ranges);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Element_point_ranges,identifier, \
	struct Element_point_ranges_identifier *);

int Element_point_ranges_get_identifier(
	struct Element_point_ranges *element_point_ranges,
	struct Element_point_ranges_identifier *identifier);
/*******************************************************************************
LAST MODIFIED : 28 February 2000

DESCRIPTION :
Puts the contents of the identifier for <element_point_ranges> in the
caller-supplied <identifier>.
==============================================================================*/

int Element_point_ranges_add_range(
	struct Element_point_ranges *element_point_ranges,int start,int stop);
/*******************************************************************************
LAST MODIFIED : 28 February 2000

DESCRIPTION :
Adds the range from <start> to <stop> to the ranges in <element_point_ranges>.
==============================================================================*/

struct Multi_range *Element_point_ranges_get_ranges(
	struct Element_point_ranges *element_point_ranges);
/*******************************************************************************
LAST MODIFIED : 29 February 2000

DESCRIPTION :
Returns a pointer to the ranges in <element_point_ranges>. This should not be
modified in any way.
==============================================================================*/

int Element_point_ranges_has_ranges(
	struct Element_point_ranges *element_point_ranges);
/*******************************************************************************
LAST MODIFIED : 25 May 2000

DESCRIPTION :
Returns true if <element_point_ranges> has ranges, ie. is not empty.
==============================================================================*/

int Element_point_ranges_add_to_list(
	struct Element_point_ranges *element_point_ranges,
	void *element_point_ranges_list_void);
/*******************************************************************************
LAST MODIFIED : 28 February 2000

DESCRIPTION :
Ensures the <element_point_ranges> are in <element_point_ranges_list>.
==============================================================================*/

int Element_point_ranges_remove_from_list(
	struct Element_point_ranges *element_point_ranges,
	void *element_point_ranges_list_void);
/*******************************************************************************
LAST MODIFIED : 28 February 2000

DESCRIPTION :
Ensures the <element_point_ranges> is not in <element_point_ranges_list>.
==============================================================================*/

int Element_point_ranges_toggle_in_list(
	struct Element_point_ranges *element_point_ranges,
	void *element_point_ranges_list_void);
/*******************************************************************************
LAST MODIFIED : 28 February 2000

DESCRIPTION :
Toggles the <element_point_ranges> in <element_point_ranges_list>.
==============================================================================*/

int set_Element_point_ranges(struct Parse_state *state,
	void *element_point_ranges_address_void,void *element_manager_void);
/*******************************************************************************
LAST MODIFIED : 27 March 2000

DESCRIPTION :
Modifier function to set an element_point_ranges. <element_point_ranges_address>
should point to a currently-NULL pointer to a struct Element_point_ranges. Upon
successful return an Element_point_ranges will be created and the pointer to it
returned in this location, for the calling function to use or destroy.
==============================================================================*/

int Element_point_ranges_element_is_in_group(
	struct Element_point_ranges *element_point_ranges,void *element_group_void);
/*******************************************************************************
LAST MODIFIED : 28 March 2000

DESCRIPTION :
Returns true if the element for <element_point_ranges> is in <element_group>.
==============================================================================*/

struct Element_point_ranges *Element_point_ranges_from_grid_field_ranges(
	struct FE_element *element,struct FE_field *grid_field,
	struct Multi_range *ranges);
/*******************************************************************************
LAST MODIFIED : 18 May 2000

DESCRIPTION :
If <grid_field> is a single component grid-based field in <element>, creates and
returns an Element_point_ranges containing all the grid points at which the
value of <grid_field> is in the <ranges>.
No Element_point_ranges object is returned without error if:
- <grid_field> is not grid-based in <element>.
- No grid points in <element> have <grid_field> value in the given <ranges>.
==============================================================================*/

int FE_element_grid_to_Element_point_ranges_list(struct FE_element *element,
	void *grid_to_list_data_void);
/*******************************************************************************
LAST MODIFIED : 26 May 2000

DESCRIPTION :
Iterator function that gets an Element_point_ranges structure representing all
the grid_points in <element> with discretization of the single component
integer <grid_field>, for which the field value is in the given <ranges>.
Note that there may legitimately be none if <grid_field> is not grid-based in
<element> or the ranges do not intersect with the values in the field.
The structure is then added to the <element_point_ranges_list>.
select_data_void should point to a
struct FE_element_grid_to_Element_point_ranges_list_data.
Uses only top level elements, type CM_ELEMENT.
==============================================================================*/

#endif /* !defined (ELEMENT_POINT_RANGES_H) */
