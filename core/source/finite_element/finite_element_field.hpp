/**
 * FILE : finite_element_field.hpp
 *
 * Internal class for field interpolated over finite elements.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (FINITE_ELEMENT_FIELD_HPP)
#define FINITE_ELEMENT_FIELD_HPP

#include "opencmiss/zinc/types/elementid.h"
#include "general/change_log.h"
#include "general/enumerator.h"
#include "general/list.h"

/*
Global types
------------
*/

/**
 * FE_field maintains pointer to owning FE_region.
 */
struct FE_region;

class FE_mesh;
class FE_mesh_field_data;

/** Information about what the field represents physically.
 * It is derived from how fields are used in cm, but does not correspond to a
 * field type in cm or identify fields in cm.
 * Note: the first value will be 0 by the ANSI standard, with each subsequent entry
 * incremented by 1. This pattern is expected by the ENUMERATOR macros.  Must
 * ensure the ENUMERATOR_STRING function returns a string for each value here. */
enum CM_field_type
{
	CM_ANATOMICAL_FIELD,
	CM_COORDINATE_FIELD,
	CM_GENERAL_FIELD
};

/** Legacy internal storage/function type for FE_field */
enum FE_field_type
{
	CONSTANT_FE_FIELD, /* fixed values */
	INDEXED_FE_FIELD,  /* indexed set of fixed values */
	GENERAL_FE_FIELD,  /* values held in nodes, elements */
	UNKNOWN_FE_FIELD
};

/** Internal class for field interpolated over finite elements. */
struct FE_field;

DECLARE_LIST_TYPES(FE_field);

DECLARE_CHANGE_LOG_TYPES(FE_field);

struct Set_FE_field_conditional_data
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
User data structure passed to set_FE_field_conditional, containing the
fe_field_list and the optional conditional_function (and
conditional_function_user_data) for selecting a field out of a subset of the
fields in the list.
==============================================================================*/
{
	LIST_CONDITIONAL_FUNCTION(FE_field) *conditional_function;
	void *conditional_function_user_data;
	struct LIST(FE_field) *fe_field_list;
}; /* struct Set_FE_field_conditional_data */

/*
Global functions
----------------
*/

PROTOTYPE_ENUMERATOR_FUNCTIONS(CM_field_type);

struct FE_field *CREATE(FE_field)(const char *name, struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 27 February 2003

DESCRIPTION :
Creates and returns a struct FE_field of <name> belonging to the ultimate
master FE_region of <fe_region>. The new field has no name/identifier, zero
components, field_type FIELD, NOT_APPLICABLE coordinate system, no field values.
???RC Used to pass <fe_time> in here and store in FE_field; can now get it from
FE_region.
==============================================================================*/

int DESTROY(FE_field)(struct FE_field **field_address);
/*******************************************************************************
LAST MODIFIED : 23 September 1995

DESCRIPTION :
Frees the memory for <**field_address> and sets <*field_address> to NULL.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(FE_field);

PROTOTYPE_GET_OBJECT_NAME_FUNCTION(FE_field);

PROTOTYPE_LIST_FUNCTIONS(FE_field);

PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(FE_field,name,const char *);

PROTOTYPE_CHANGE_LOG_FUNCTIONS(FE_field);

/**
 * Copies the field definition from source to destination, except for name,
 * mesh field data and node definition. Assumes source field has been
 * proven compatible with destination including that source indexer field has
 * already been merged.
 * If source and destination fields belong to different FE_regions, substitute
 * local indexer_field and element_xi_host_mesh if appropriate.
 */
int FE_field_copy_without_identifier(struct FE_field *destination,
	struct FE_field *source);

/**
 * Returns true if <field1> and <field2> have the same fundamental definition,
 * namely they:
 * 1. Have the same value type
 * 2. Have the same fe field type (general, indexed etc.)
 * 3. Have the same number of components
 * 4. Have the same coordinate system
 * If so, they can be merged without affecting the rest of the model.
 * Other attributes such as cm field type (field, coordinate, anatomical) are
 * not considered fundamental. The name is also not compared.
 * Must ensure this function fits with FE_fields_match_exact.
 */
bool FE_fields_match_fundamental(struct FE_field *field1,
	struct FE_field *field2);

/**
 * Returns true if <field1> and <field2> have exactly the same definition,
 * comparing all attributes.
 * @see FE_fields_match_fundamental
 */
bool FE_fields_match_exact(struct FE_field *field1, struct FE_field *field2);

/**
 * List iterator function which fetches a field with the same name as <field>
 * from <field_list>. Returns 1 (true) if there is either no such field in the
 * list or the two fields return true for FE_fields_match_fundamental(),
 * otherwise returns 0 (false).
 */
int FE_field_can_be_merged_into_list(struct FE_field *field, void *field_list_void);

int FE_field_has_multiple_times(struct FE_field *fe_field);
/*******************************************************************************
LAST MODIFIED : 26 February 2003

DESCRIPTION :
Returns true if any node_fields corresponding to <field> have time_seqences.
This will be improved when regionalised, so that hopefully the node field
list we will be looking at will not be global but will belong to the region.
==============================================================================*/

/**
 * Return true if any basis functions used by the field is non-linear i.e.
 * quadratic, cubic, Fourier etc.
 */
bool FE_field_uses_non_linear_basis(struct FE_field *fe_field);

struct FE_field *find_first_time_field_at_FE_node(struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 9 June 1999

Find the first time based field at a node
==============================================================================*/

int ensure_FE_field_is_in_list(struct FE_field *field, void *field_list_void);
/*******************************************************************************
LAST MODIFIED : 29 March 2006

DESCRIPTION :
Iterator function for adding <field> to <field_list> if not currently in it.
==============================================================================*/

struct FE_region *FE_field_get_FE_region(struct FE_field *fe_field);
/*******************************************************************************
LAST MODIFIED : 26 February 2003

DESCRIPTION :
Returns the FE_region that <fe_field> belongs to.
==============================================================================*/

/*******************************************************************************
 * Records that a Computed_field_finite_element is wrapping this FE_field.
 * @return  The number of wrappers existing.
 */
int FE_field_add_wrapper(struct FE_field *field);

/*******************************************************************************
 * Records that a Computed_field_finite_element is no longer wrapping this
 * FE_field.
 * @return  The number of wrappers remaining.
 */
int FE_field_remove_wrapper(struct FE_field *field);

/***************************************************************************//**
 * @return  number of objects using fe_field.
 */
int FE_field_get_access_count(struct FE_field *fe_field);

char *get_FE_field_component_name(struct FE_field *field,int component_no);
/*******************************************************************************
LAST MODIFIED : 10 May 2000

DESCRIPTION :
Returns the name of component <component_no> of <field>. If no name is stored
for the component, a string comprising the value component_no+1 is returned.
Up to calling function to DEALLOCATE the returned string.
==============================================================================*/

int set_FE_field_component_name(struct FE_field *field,int component_no,
	const char *component_name);
/*******************************************************************************
LAST MODIFIED : 10 May 2000

DESCRIPTION :
Sets the name of component <component_no> of <field>. Only sets name if it is
different from that already returned for field to preserve default names if can.
==============================================================================*/

struct Coordinate_system *get_FE_field_coordinate_system(
	struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 22 January 1999

DESCRIPTION :
Returns the coordinate system for the <field>.
==============================================================================*/

int set_FE_field_coordinate_system(struct FE_field *field,
	struct Coordinate_system *coordinate_system);
/*******************************************************************************
LAST MODIFIED : 28 January 1999

DESCRIPTION :
Sets the coordinate system of the <field>.
==============================================================================*/

/** Ensure field is not defined on mesh
  * Assumes called with FE_region change caching on; records change but doesn't notify */
void FE_field_clearMeshFieldData(struct FE_field *field, FE_mesh *mesh);

/**
 * Note: must get no field data from FE_field_getMeshFieldData first!
 * @return  New mesh field data defining field over the given mesh, or 0 if failed.
 * @param field  The field to create data for.
 * @param mesh  The mesh to create data for. Must be from same region as field.
 */
FE_mesh_field_data *FE_field_createMeshFieldData(struct FE_field *field,
	FE_mesh *mesh);

/**
 * @return  Mesh field data defining field over the given mesh, or 0 if none.
 * @param field  The field to create data for.
 * @param mesh  The mesh to create data for. Must be from same region as field.
 */
FE_mesh_field_data *FE_field_getMeshFieldData(struct FE_field *field,
	const FE_mesh *mesh);

int get_FE_field_number_of_components(struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 16 November 1998

DESCRIPTION :
Returns the number of components for the <field>.
==============================================================================*/

int set_FE_field_number_of_components(struct FE_field *field,
	int number_of_components);
/*******************************************************************************
LAST MODIFIED : 1 September 1999

DESCRIPTION :
Sets the number of components in the <field>. Automatically assumes names for
any new components. Clears/reallocates the values_storage for FE_field_types
that use them, eg. CONSTANT_FE_FIELD and INDEXED_FE_FIELD - but only if number
of components changes. If function fails the field is left exactly as it was.
Should only call this function for unmanaged fields.
ELEMENT_XI_VALUE, STRING_VALUE and URL_VALUE fields may only have 1 component.
==============================================================================*/

int get_FE_field_number_of_values(struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 16 November 1998

DESCRIPTION :
Returns the number of global values for the <field>.
==============================================================================*/

int get_FE_field_number_of_times(struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 9 June 1999

DESCRIPTION :
Returns the number of global times for the <field>.
==============================================================================*/

int set_FE_field_number_of_times(struct FE_field *field,
	int number_of_times);
/*******************************************************************************
LAST MODIFIED : 9 June 1999

DESCRIPTION :
Sets the number of times stored with the <field>
REALLOCATES the requires memory in field->value_storage, based upon the
field->time_value_type.

For non-array types, the contents of field->times_storage is:
   | data type (eg FE_value) | x number_of_times

For array types, the contents of field->times is:
   ( | int (number of array values) | pointer to array (eg double *) |
	 x number_of_times )

Sets data in this memory to 0, pointers to NULL.

MUST have called set_FE_field_time_value_type() before calling this function.
Should only call this function for unmanaged fields.
==============================================================================*/

enum CM_field_type get_FE_field_CM_field_type(struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 31 August 1999

DESCRIPTION :
Returns the CM_field_type of the <field>.
==============================================================================*/

int set_FE_field_CM_field_type(struct FE_field *field,
	enum CM_field_type cm_field_type);
/*******************************************************************************
LAST MODIFIED : 31 August 1999

DESCRIPTION :
Sets the CM_field_type of the <field>.
Should only call this function for unmanaged fields.
==============================================================================*/

enum FE_field_type get_FE_field_FE_field_type(struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 31 August 1999

DESCRIPTION :
Returns the FE_field_type for the <field>.
==============================================================================*/

int set_FE_field_type_constant(struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 1 September 1999

DESCRIPTION :
Converts the <field> to type CONSTANT_FE_FIELD.
Allocates and clears the values_storage of the field to fit
field->number_of_components of the current value_type.
If function fails the field is left exactly as it was.
Should only call this function for unmanaged fields.
==============================================================================*/

int set_FE_field_type_general(struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 1 September 1999

DESCRIPTION :
Converts the <field> to type GENERAL_FE_FIELD.
Frees any values_storage currently in use by the field.
If function fails the field is left exactly as it was.
Should only call this function for unmanaged fields.
==============================================================================*/

int get_FE_field_type_indexed(struct FE_field *field,
	struct FE_field **indexer_field,int *number_of_indexed_values);
/*******************************************************************************
LAST MODIFIED : 1 September 1999

DESCRIPTION :
If the field is of type INDEXED_FE_FIELD, the indexer_field and
number_of_indexed_values it uses are returned - otherwise an error is reported.
Use function get_FE_field_FE_field_type to determine the field type.
==============================================================================*/

int set_FE_field_type_indexed(struct FE_field *field,
	struct FE_field *indexer_field,int number_of_indexed_values);
/*******************************************************************************
LAST MODIFIED : 1 September 1999

DESCRIPTION :
Converts the <field> to type INDEXED_FE_FIELD, indexed by the given
<indexer_field> and with the given <number_of_indexed_values>. The indexer_field
must return a single integer value to be valid.
Allocates and clears the values_storage of the field to fit
field->number_of_components x number_of_indexed_values of the current
value_type. If function fails the field is left exactly as it was.
Should only call this function for unmanaged fields.
==============================================================================*/

int get_FE_field_time_FE_value(struct FE_field *field,int number,FE_value *value);
/*******************************************************************************
LAST MODIFIED : 10 June 1999

DESCRIPTION :
Gets the specified global time value for the <field>.
==============================================================================*/

enum Value_type get_FE_field_time_value_type(struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 9 June 1999

DESCRIPTION :
Returns the time_value_type of the <field>.
==============================================================================*/

int set_FE_field_time_value_type(struct FE_field *field,
	enum Value_type time_value_type);
/*******************************************************************************
LAST MODIFIED : 9 June 1999

DESCRIPTION :
Sets the time_value_type of the <field>.
Should only call this function for unmanaged fields.
=========================================================================*/

enum Value_type get_FE_field_value_type(struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 19 February 1999

DESCRIPTION :
Returns the value_type of the <field>.
==============================================================================*/

int set_FE_field_value_type(struct FE_field *field,enum Value_type value_type);
/*******************************************************************************
LAST MODIFIED : 1 September 1999

DESCRIPTION :
Sets the value_type of the <field>. Clears/reallocates the values_storage for
FE_field_types that use them, eg. CONSTANT_FE_FIELD and INDEXED_FE_FIELD - but
only if the value_type changes. If function fails the field is left exactly as
it was. Should only call this function for unmanaged fields.
ELEMENT_XI_VALUE, STRING_VALUE and URL_VALUE fields may only have 1 component.
=========================================================================*/

/**
 * If the FE_field has value_type ELEMENT_XI_VALUE, this returns the
 * host mesh the embedded locations are within, or 0 if not yet set (legacy
 * input only).
 *
 * @param field  The field to query.
 * @return  Host mesh (not accessed) or 0 if none.
 */
const FE_mesh *FE_field_get_element_xi_host_mesh(struct FE_field *field);

/**
 * If the FE_field has value_type ELEMENT_XI_VALUE, sets the host mesh the
 * embedded locations are within.
 *
 * @param field  The field to modify.
 * @param hostMesh  The host mesh to set.
 * @return  Standard result code.
 */
int FE_field_set_element_xi_host_mesh(struct FE_field *field,
	const FE_mesh *hostMesh);

int get_FE_field_max_array_size(struct FE_field *field,
	int *max_number_of_array_values, enum Value_type *value_type);
/*******************************************************************************
LAST MODIFIED : 4 March 1999

DESCRIPTION :
Given the field, search vaules_storage  for the largest array, and return it in
max_number_of_array_values. Return the field value_type.
==============================================================================*/

int get_FE_field_array_attributes(struct FE_field *field, int value_number,
	int *number_of_array_values, enum Value_type *value_type);
/*******************************************************************************
LAST MODIFIED : 4 March 1999

DESCRIPTION :
Get the value_type and the number of array values for the array in
field->values_storage specified by value_number.
Give an error if field->values_storage isn't storing array types.
==============================================================================*/

int get_FE_field_string_value(struct FE_field *field, int value_number,
	char **string);
/*******************************************************************************
LAST MODIFIED : 22 September 1999

DESCRIPTION :
Returns a copy of the string stored at <value_number> in the <field>.
Up to the calling function to DEALLOCATE the returned string.
Returned <*string> may be a valid NULL if that is what is in the field.
==============================================================================*/

int set_FE_field_string_value(struct FE_field *field, int value_number,
	char *string);
/*******************************************************************************
LAST MODIFIED : 22 September 1999

DESCRIPTION :
Copies and sets the <string> stored at <value_number> in the <field>.
<string> may be NULL.
==============================================================================*/

int get_FE_field_element_xi_value(struct FE_field *field, int number,
	cmzn_element **element, FE_value *xi);
/*******************************************************************************
LAST MODIFIED : 20 April 1999

DESCRIPTION :
Gets the specified global value for the <field>.
==============================================================================*/

int set_FE_field_element_xi_value(struct FE_field *field, int number,
	cmzn_element *element, FE_value *xi);
/*******************************************************************************
LAST MODIFIED : 20 April 1999

DESCRIPTION :
Sets the specified global value for the <field>, to the passed Element and xi
The field value MUST have been previously allocated with
set_FE_field_number_of_values
==============================================================================*/

int get_FE_field_FE_value_value(struct FE_field *field, int number,
	FE_value *value);
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
Gets the specified global FE_value <value> from <field>.
==============================================================================*/

int set_FE_field_FE_value_value(struct FE_field *field, int number,
	FE_value value);
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
Sets the specified global FE_value <value> in <field>.
The <field> must be of type FE_VALUE_VALUE to have such values and
<number> must be within the range from get_FE_field_number_of_values.
==============================================================================*/

int get_FE_field_int_value(struct FE_field *field, int number, int *value);
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
Gets the specified global int <value> from <field>.
==============================================================================*/

int set_FE_field_int_value(struct FE_field *field, int number, int value);
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
Sets the specified global int <value> in <field>.
The <field> must be of type INT_VALUE to have such values and
<number> must be within the range from get_FE_field_number_of_values.
==============================================================================*/

int set_FE_field_time_FE_value(struct FE_field *field, int number,
	FE_value value);
/*******************************************************************************
LAST MODIFIED : l0 June 1999

DESCRIPTION :
Sets the specified global time value for the <field>, to the passed FE_value
The field value MUST have been previously allocated with
set_FE_field_number_of_times
==============================================================================*/

const char *get_FE_field_name(struct FE_field *field);
/*******************************************************************************
LAST MODIFIED : 19 February 1999

DESCRIPTION :
Returns a pointer to the name for the <field>.
Should only call this function for unmanaged fields.
==============================================================================*/

/***************************************************************************//**
 * Sets the name of the <field>.
 * Should only call this function for unmanaged fields.
 * All others should use FE_region_set_FE_field_name.
 */
int set_FE_field_name(struct FE_field *field, const char *name);

int FE_field_is_1_component_integer(struct FE_field *field,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 18 May 2000

DESCRIPTION :
Conditional function returning true if <field> has exactly 1 component and a
value type of integer.
This type of field is used for storing eg. grid_point_number.
==============================================================================*/

int FE_field_is_coordinate_field(struct FE_field *field,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 18 May 2000

DESCRIPTION :
Conditional function returning true if the <field> is a coodinate field
(defined by having a CM_field_type of coordinate) has a Value_type of
FE_VALUE_VALUE and has from 1 to 3 components.
==============================================================================*/

int FE_field_is_anatomical_fibre_field(struct FE_field *field,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 16 July 1998

DESCRIPTION :
Conditional function returning true if the <field> is a anatomical field
(defined by having a CM_field_type of anatomical), has a Value_type of
FE_VALUE_VALUE, has from 1 to 3 components, and has a FIBRE coordinate system.
==============================================================================*/

int FE_field_is_embedded(struct FE_field *field, void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 5 June 2003

DESCRIPTION :
Returns true if the values returned by <field> are a location in an FE_region,
either an element_xi value, or eventually a node.
==============================================================================*/

#endif /* !defined (FINITE_ELEMENT_FIELD_HPP) */
