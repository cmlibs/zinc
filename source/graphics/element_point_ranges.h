/*******************************************************************************
FILE : element_point_ranges.h

LAST MODIFIED : 6 March 2003

DESCRIPTION :
Structure for storing ranges of points in elements according to the various
Xi_discretization_modes.
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
#if !defined (ELEMENT_POINT_RANGES_H)
#define ELEMENT_POINT_RANGES_H

#include "computed_field/computed_field_value_index_ranges.h"
#include "finite_element/finite_element.h"
#include "general/list.h"
#include "general/multi_range.h"
#include "general/object.h"
#include "graphics/auxiliary_graphics_types.h"

struct Element_point_ranges_identifier
/*******************************************************************************
LAST MODIFIED : 8 June 2000

DESCRIPTION :
Identifier created to allow LIST(Element_point_ranges) to be indexed for quick
access to objects. Objects are indexed by element first, then the
Xi_discretization_mode and any other identifying values relevant to the
Xi_discretization_mode.
See also compare_Element_point_ranges_identifier function.
==============================================================================*/
{
	struct FE_element *element,*top_level_element;
	enum Xi_discretization_mode xi_discretization_mode;
	/* following could/should be in a union */
	int number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	FE_value exact_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
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

struct Element_point_ranges_grid_to_multi_range_data
/*******************************************************************************
LAST MODIFIED : 16 June 2000

DESCRIPTION :
Data for passing to Element_point_ranges_grid_to_multi_range.
==============================================================================*/
{
	/* following field must be single component integer field */
	struct FE_field *grid_fe_field;
	struct Multi_range *multi_range;
	/* set following to 1 before calling - after calling, if set can report that
		 some element points were not at native/grid discretizations */
	int all_points_native;
};

struct FE_element_grid_to_multi_range_data
/*******************************************************************************
LAST MODIFIED : 21 September 2000

DESCRIPTION :
Data for passing to FE_element_grid_to_multi_range.
==============================================================================*/
{
	/* following field must be single component integer field */
	struct FE_field *grid_fe_field;
	struct Multi_range *multi_range;
};

struct Element_point_ranges_set_grid_values_data
/*******************************************************************************
LAST MODIFIED : 19 March 2003

DESCRIPTION :
Data for passing to Element_point_ranges_set_grid_values and for it to pass to
Field_value_index_ranges_set_grid_values.
==============================================================================*/
{
	/* the source element points for setting grid values from */
	struct Element_point_ranges_identifier *source_identifier;
	int source_element_point_number;
	/* the list of field components ranges being set - not needed in
		 Field_value_index_ranges_set_grid_values */
	struct LIST(Field_value_index_ranges) *field_component_ranges_list;
	/* the element points being set. Note values are modified in element by
		 Field_value_index_ranges_set_grid_values */
	struct Element_point_ranges_identifier *destination_identifier;
	struct Multi_range *destination_element_point_numbers;
	struct FE_element *element;
	/* the FE_region the elements belong to */
	struct FE_region *fe_region;
	/* set following to 0 before calling - after calling, can compare to
	 see how many eligible points there were and how many were successfully set. */
	int number_of_points;
	int number_of_points_set;
};

/*
Global functions
----------------
*/

const char **Xi_discretization_mode_get_valid_strings_for_Element_point_ranges(
	int *number_of_valid_strings);
/*******************************************************************************
LAST MODIFIED : 30 May 2000

DESCRIPTION :
Returns an allocated array of pointers to all static strings for valid
Xi_discretization_modes that can be used for Element_point_ranges, obtained
from function Xi_discretization_mode_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/

int compare_Element_point_ranges_identifier(
	struct Element_point_ranges_identifier *identifier1,
	struct Element_point_ranges_identifier *identifier2);
/*******************************************************************************
LAST MODIFIED : 7 June 2000

DESCRIPTION :
Returns -1 (identifier1 less), 0 (equal) or +1 (identifier1 greater) for
indexing lists of Element_point_ranges.
First the element is compared, then the Xi_discretization_mode, then the
identifying values depending on this mode.
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

PROTOTYPE_COPY_OBJECT_FUNCTION(Element_point_ranges_identifier);

int Element_point_make_top_level(
	struct Element_point_ranges_identifier *identifier,int *element_point_number);
/*******************************************************************************
LAST MODIFIED : 8 June 2000

DESCRIPTION :
If <identifier> does not already refer to a top_level_element - ie. element
and top_level_element are ont the same, converts it to an EXACT_XI point that is
top_level. Assumes <identifier> has been validated.
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

int Element_point_ranges_list_add_element_point(
	struct LIST(Element_point_ranges) *element_point_ranges_list,
	struct Element_point_ranges_identifier *element_point_ranges_identifier,
	int element_point_number);
/*******************************************************************************
LAST MODIFIED : 16 June 2000

DESCRIPTION :
Shortcut for ensuring the element point indicated by
<element_point_ranges_identifier> <element_point_number> is in the
<element_point_ranges_list>.
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

int Element_point_ranges_is_wholly_within_element_list_tree(
	struct Element_point_ranges *element_point_ranges, void *element_list_void);
/*******************************************************************************
LAST MODIFIED : 2 March 2001

DESCRIPTION :
Returns true if either the top_level_element in the <element_point_ranges>
identifier is in <element_list>, or if the element is wholly within the list
tree with FE_element_is_wholly_within_element_list_tree function. Used to check
if element or top_level_element used by element_point_ranges will be destroyed,
since faces and lines are destroyed with their parents if they are not also
faces or lines of other elements not being destroyed.
==============================================================================*/

int set_Element_point_ranges(struct Parse_state *state,
	void *element_point_ranges_address_void, void *fe_region_void);
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
Modifier function to set an element_point_ranges. <element_point_ranges_address>
should point to a currently-NULL pointer to a struct Element_point_ranges. Upon
successful return an Element_point_ranges will be created and the pointer to it
returned in this location, for the calling function to use or destroy.
==============================================================================*/

int Element_point_ranges_element_is_in_FE_region(
	struct Element_point_ranges *element_point_ranges, void *fe_region_void);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Returns true if the element for <element_point_ranges> is in <fe_region>.
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

int Element_point_ranges_grid_to_multi_range(
	struct Element_point_ranges *element_point_ranges,
	void *grid_to_multi_range_data_void);
/*******************************************************************************
LAST MODIFIED : 16 June 2000

DESCRIPTION :
Last parameter is a struct Element_point_ranges_grid_to_multi_range_data.
If <grid_fe_field> is grid-based as in <element_point_ranges>, adds the values
for this field for points in the ranges to the <multi_range>.
If field and element_point_ranges not identically grid-based, clear
<all_points_native> flag.
==============================================================================*/

int FE_element_grid_to_multi_range(struct FE_element *element,
	void *grid_to_multi_range_data_void);
/*******************************************************************************
LAST MODIFIED : 21 September 2000

DESCRIPTION :
Last parameter is a struct FE_element_grid_to_multi_range_data.
If <grid_fe_field> is grid-based as in <element>, adds all values for this field
in <element> to the <multi_range>.
==============================================================================*/

int Element_point_ranges_set_grid_values(
	struct Element_point_ranges *element_point_ranges,
	void *set_grid_values_data_void);
/*******************************************************************************
LAST MODIFIED : 19 June 2000

DESCRIPTION :
Last parameter is a struct Element_point_ranges_set_grid_values_data. Sets the
listed field components in <element_point_ranges> to the values taken from
<source_identifier><element_point_number>. Works on a local element_copy, then
uses a manager_modify to make changes global.
==============================================================================*/

#endif /* !defined (ELEMENT_POINT_RANGES_H) */
