/*******************************************************************************
FILE : element_point_ranges.h

LAST MODIFIED : 29 February 2000

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

/*
Global functions
----------------
*/

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
#endif /* !defined (ELEMENT_POINT_RANGES_H) */
