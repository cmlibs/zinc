/*******************************************************************************
FILE : computed_field_private.h

LAST MODIFIED : 20 January 2005

DESCRIPTION :
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
#if !defined (COMPUTED_FIELD_PRIVATE_H)
#define COMPUTED_FIELD_PRIVATE_H

typedef int (*Computed_field_clear_type_specific_function)(
	struct Computed_field *field);
typedef void* (*Computed_field_copy_type_specific_function)(
	struct Computed_field *source_field,
	struct Computed_field *destination_field);
typedef int (*Computed_field_clear_cache_type_specific_function)(
	struct Computed_field *field);
typedef int (*Computed_field_type_specific_contents_match_function)(
	struct Computed_field *field, struct Computed_field *other_computed_field);
typedef int (*Computed_field_is_defined_in_element_function)(
	struct Computed_field *field,struct FE_element *element);
typedef int (*Computed_field_is_defined_at_node_function)(
	struct Computed_field *field,struct FE_node *node);
typedef int (*Computed_field_has_numerical_components_function)(
	struct Computed_field *field);
typedef int (*Computed_field_not_in_use_function)(
	struct Computed_field *field);
typedef int (*Computed_field_evaluate_cache_at_node_function)(
	struct Computed_field *field,struct FE_node *node,FE_value time);
typedef int (*Computed_field_evaluate_cache_in_element_function)(
	struct Computed_field *field,struct FE_element *element,FE_value *xi,
	FE_value time,struct FE_element *top_level_element,int calculate_derivatives);
typedef char* (*Computed_field_evaluate_as_string_at_node_function)(
	struct Computed_field *field,int component_number,struct FE_node *node,
	FE_value time);
typedef char* (*Computed_field_evaluate_as_string_in_element_function)(
	struct Computed_field *field,int component_number,
	struct FE_element *element,FE_value *xi,FE_value time,
	struct FE_element *top_level_element);
typedef int (*Computed_field_set_values_at_node_function)(
	struct Computed_field *field,struct FE_node *node,FE_value time,FE_value *values);
typedef int (*Computed_field_set_values_in_element_function)(
	struct Computed_field *field,struct FE_element *element,
	FE_value *xi,FE_value time,FE_value *values);
typedef int (*Computed_field_get_native_discretization_in_element_function)(
	struct Computed_field *field,struct FE_element *element,int *number_in_xi);
typedef int (*Computed_field_find_element_xi_function)(
	struct Computed_field *field, FE_value *values, int number_of_values, 
	struct FE_element **element, FE_value *xi,
	int element_dimension, struct Cmiss_region *search_region);
typedef int (*List_Computed_field_function)(struct Computed_field *field);
typedef char* (*Computed_field_get_command_string_function)(
	struct Computed_field *field);
typedef int (*Computed_field_has_multiple_times_function)(struct Computed_field *field);
typedef int (*Computed_field_get_native_resolution_function)(struct Computed_field *field,
        int *dimension, int **sizes,
	struct Computed_field **texture_coordinate_field);
	
/* Used by the register_type_function, Computed_field_type_data and 
	Computed_field_add_type_to_option_table*/
typedef int (*Define_Computed_field_type_function)(
	struct Parse_state *state,void *field_void,void *computed_field_package_void);


/*
Module types
------------
*/

struct Computed_field
/*******************************************************************************
LAST MODIFIED : 8 October 2002

DESCRIPTION :
==============================================================================*/
{
	/* the name/identifier of the Computed_field */
	char *name;
	int number_of_components;
	/* This is set for fields where the components have names other than
		the defaults of 1,2...number_of_components */
	char **component_names;

	/* if the following flag is set, the field may not be modified or destroyed
		 by the user. See Computed_field_set_read_only function */
	int read_only;
	struct Coordinate_system coordinate_system;

	/* Value cache: */
	/* For all Computed_field_types: computed values/derivatives.
		 When the field is computed its values and derivatives are first placed
		 in these arrays. If the field is then recomputed at element:xi, the values
		 are returned immediately. The <values> array is allocated when the field is
		 evaluated and deallocated when the number_of_components is [re]established
		 or the field is copied over. The values array is made large enough to store
		 the values of the field while the derivatives fit those of the field in an
		 element of dimension MAXIMUM_ELEMENT_XI_DIMENSIONS. */
	/* ???RC note: separation of cache and field probably necessary if computed
		 fields are to be efficient under multiprocessing */
	FE_value *values,*derivatives,xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	/* flag saying whether derivatives were calculated */
	int derivatives_valid;
	/* Only one of element/node should be accessed at a time - if there is an
		 element accessed, then values are guaranteed to be valid and derivatives
		 are valid if the derivatives_valid flag is set - both at the above xi only.
		 If node is accessed, then the values are valid at that node. Either modes
		 of caching must be cleared by a call to Computed_field_clear_cache once
		 the field is no longer of immediate use. */
	/* last element in which values/derivatives calculated - ACCESSed by field */
	struct FE_element *element;
	/* last node at which values calculated - ACCESSed by field */
	struct FE_node *node;
	/* last time at which values were calculated */
	FE_value time;

	/* cache used when doing find_xi calculations */
	struct Computed_field_find_element_xi_cache *find_element_xi_cache;

	Computed_field_clear_type_specific_function
	   computed_field_clear_type_specific_function;
	Computed_field_copy_type_specific_function 
	   computed_field_copy_type_specific_function;
	Computed_field_clear_cache_type_specific_function
	   computed_field_clear_cache_type_specific_function;
	Computed_field_type_specific_contents_match_function
	   computed_field_type_specific_contents_match_function;
	Computed_field_is_defined_in_element_function
	   computed_field_is_defined_in_element_function;
	Computed_field_is_defined_at_node_function
	   computed_field_is_defined_at_node_function;
	Computed_field_has_numerical_components_function
	   computed_field_has_numerical_components_function;
	Computed_field_not_in_use_function
	   computed_field_not_in_use_function;
	Computed_field_evaluate_cache_at_node_function 
	   computed_field_evaluate_cache_at_node_function;
	Computed_field_evaluate_cache_in_element_function 
	   computed_field_evaluate_cache_in_element_function;
	Computed_field_evaluate_as_string_at_node_function
	   computed_field_evaluate_as_string_at_node_function;
	Computed_field_evaluate_as_string_in_element_function 
	   computed_field_evaluate_as_string_in_element_function;
	Computed_field_set_values_at_node_function
	   computed_field_set_values_at_node_function;
	Computed_field_set_values_in_element_function
	   computed_field_set_values_in_element_function;
	Computed_field_get_native_discretization_in_element_function
	   computed_field_get_native_discretization_in_element_function;	
	Computed_field_find_element_xi_function
	   computed_field_find_element_xi_function;	
	List_Computed_field_function list_Computed_field_function;
	Computed_field_get_command_string_function
		computed_field_get_command_string_function;
	Computed_field_has_multiple_times_function 
	   computed_field_has_multiple_times_function;
	Computed_field_get_native_resolution_function
	   computed_field_get_native_resolution_function;

	void *type_specific_data;
	/* The type string identifies the type, it should not be copied but
		point to the common type string for that type */
	char *type_string;

	/* for all Computed_field_types calculated from others */

	/* array of computed fields this field is calculated from */
	int number_of_source_fields;
	struct Computed_field **source_fields;
	/* array of constant values this field is calculated from */
	int number_of_source_values;
	FE_value *source_values;

	int access_count;
	
}; /* struct Computed_field */

#define COMPUTED_FIELD_SET_METHODS( \
	field_variable, \
	Computed_field_field_type_clear_type_specific, \
	Computed_field_field_type_copy_type_specific, \
	Computed_field_field_type_clear_cache_type_specific, \
	Computed_field_field_type_type_specific_contents_match, \
	Computed_field_field_type_is_defined_in_element, \
	Computed_field_field_type_is_defined_at_node, \
	Computed_field_field_type_has_numerical_components, \
	Computed_field_field_type_not_in_use, \
	Computed_field_field_type_evaluate_cache_at_node, \
	Computed_field_field_type_evaluate_cache_in_element, \
	Computed_field_field_type_evaluate_as_string_at_node, \
	Computed_field_field_type_evaluate_as_string_in_element, \
	Computed_field_field_type_set_values_at_node, \
	Computed_field_field_type_set_values_in_element, \
	Computed_field_field_type_get_native_discretization_in_element, \
	Computed_field_field_type_find_element_xi, \
	list_Computed_field_field_type, \
	Computed_field_field_type_get_command_string, \
	Computed_field_field_type_has_multiple_times, \
	Computed_field_field_type_get_native_resolution \
	) \
/***************************************************************************** \
LAST MODIFIED : 9 June 2006 \
\
DESCRIPTION : \
Set these with a macro to ensure we always set them all. \
============================================================================*/ \
field_variable->computed_field_clear_type_specific_function = \
	Computed_field_field_type_clear_type_specific; \
field_variable->computed_field_copy_type_specific_function = \
	Computed_field_field_type_copy_type_specific; \
field_variable->computed_field_clear_cache_type_specific_function = \
	Computed_field_field_type_clear_cache_type_specific; \
field_variable->computed_field_type_specific_contents_match_function = \
	Computed_field_field_type_type_specific_contents_match; \
field_variable->computed_field_is_defined_in_element_function = \
	Computed_field_field_type_is_defined_in_element; \
field_variable->computed_field_is_defined_at_node_function = \
	Computed_field_field_type_is_defined_at_node; \
field_variable->computed_field_has_numerical_components_function = \
	Computed_field_field_type_has_numerical_components; \
field_variable->computed_field_not_in_use_function = \
	Computed_field_field_type_not_in_use; \
field_variable->computed_field_evaluate_cache_at_node_function = \
	Computed_field_field_type_evaluate_cache_at_node; \
field_variable->computed_field_evaluate_cache_in_element_function = \
	Computed_field_field_type_evaluate_cache_in_element; \
field_variable->computed_field_evaluate_as_string_at_node_function = \
	Computed_field_field_type_evaluate_as_string_at_node; \
field_variable->computed_field_evaluate_as_string_in_element_function = \
	Computed_field_field_type_evaluate_as_string_in_element; \
field_variable->computed_field_set_values_at_node_function = \
	Computed_field_field_type_set_values_at_node; \
field_variable->computed_field_set_values_in_element_function = \
	Computed_field_field_type_set_values_in_element; \
field_variable->computed_field_get_native_discretization_in_element_function = \
	Computed_field_field_type_get_native_discretization_in_element; \
field_variable->computed_field_find_element_xi_function = \
	Computed_field_field_type_find_element_xi; \
field_variable->list_Computed_field_function = \
	list_Computed_field_field_type; \
field_variable->computed_field_get_command_string_function =  \
	Computed_field_field_type_get_command_string; \
field_variable->computed_field_has_multiple_times_function =  \
	Computed_field_field_type_has_multiple_times; \
field_variable->computed_field_get_native_resolution_function = \
        Computed_field_field_type_get_native_resolution

#define COMPUTED_FIELD_ESTABLISH_METHODS( field_type ) \
/***************************************************************************** \
LAST MODIFIED : 9 June 2006 \
\
DESCRIPTION : \
Each Computed_field_set_type function should call this macro to establish the \
virtual functions that give the field its particular behaviour; Each function \
must therefore be defined for each field type, even if it is set to NULL or \
some default function. The field_variable is hard coded to 'field' for this \
macro. \
============================================================================*/ \
	COMPUTED_FIELD_SET_METHODS( \
	field, \
	Computed_field_ ## field_type ## _clear_type_specific, \
	Computed_field_ ## field_type ## _copy_type_specific, \
	Computed_field_ ## field_type ## _clear_cache_type_specific, \
	Computed_field_ ## field_type ## _type_specific_contents_match, \
	Computed_field_ ## field_type ## _is_defined_in_element, \
	Computed_field_ ## field_type ## _is_defined_at_node, \
	Computed_field_ ## field_type ## _has_numerical_components, \
	Computed_field_ ## field_type ## _not_in_use, \
	Computed_field_ ## field_type ## _evaluate_cache_at_node, \
	Computed_field_ ## field_type ## _evaluate_cache_in_element, \
	Computed_field_ ## field_type ## _evaluate_as_string_at_node, \
	Computed_field_ ## field_type ## _evaluate_as_string_in_element, \
	Computed_field_ ## field_type ## _set_values_at_node, \
	Computed_field_ ## field_type ## _set_values_in_element, \
	Computed_field_ ## field_type ## _get_native_discretization_in_element, \
	Computed_field_ ## field_type ## _find_element_xi, \
	list_Computed_field_ ## field_type, \
	Computed_field_ ## field_type ## _get_command_string, \
	Computed_field_ ## field_type ## _has_multiple_times, \
   Computed_field_ ## field_type ## _get_native_resolution )

int Computed_field_changed(struct Computed_field *field,
	struct MANAGER(Computed_field) *computed_field_manager);
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Notifies the <computed_field_manager> that the <field> has changed.
==============================================================================*/

struct Computed_field_type_data;

struct Computed_field_type_data *CREATE(Computed_field_type_data)
   (char *name, Define_Computed_field_type_function 
	define_Computed_field_type_function, void *define_type_user_data);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Creates a structure representing a type of computed field.  The <name> should
point to a static string which is used as the identifier of that type
throughout the program.  The <define_Computed_field_type_function> is added to
the define_computed_field option table when needed.
==============================================================================*/

int DESTROY(Computed_field_type_data)
   (struct Computed_field_type_data **data_address);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Frees memory/deaccess data at <*data_address>.
==============================================================================*/

int Computed_field_package_add_type(
	struct Computed_field_package *computed_field_package, char *name,
	Define_Computed_field_type_function define_Computed_field_type_function,
	void *define_type_user_data);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Adds the type of Computed_field described by <name> and 
<define_Computed_field_type_function> to those in the LIST held by the 
<computed_field_package>.  This type is then added to the 
define_Computed_field_type option table when parsing commands.
==============================================================================*/

int Computed_field_clear_type(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 18 June 1999

DESCRIPTION :
Used internally by DESTROY and Computed_field_set_type_*() functions to
deallocate or deaccess data specific to any Computed_field_type. Functions
changing the type of the Computed_field should allocate any dynamic data needed
for the type, call this function to clear what is currently in the field and
then set values - that way the field will never be left in an invalid state.
Calls Computed_field_clear_cache before clearing the type.
==============================================================================*/

int Computed_field_contents_match(struct Computed_field *field,
	void *other_computed_field_void);
/*******************************************************************************
LAST MODIFIED : 22 January 1999

DESCRIPTION :
Iterator/conditional function returning true if contents of <field> other than
its name matches the contents of the <other_computed_field_void>.
==============================================================================*/

int Computed_field_default_clear_type_specific(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
A default implementation of this function to use when there is no type
specific data.
==============================================================================*/

void *Computed_field_default_copy_type_specific(
	struct Computed_field *source, struct Computed_field *destination);
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
A default implementation of this function to use when there is no type
specific data.
==============================================================================*/

int Computed_field_default_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field);
/*******************************************************************************
LAST MODIFIED : 25 February 2002

DESCRIPTION :
A default implementation of this function to use when there is no type
specific data.
==============================================================================*/

int Computed_field_default_is_defined_in_element(struct Computed_field *field,
	struct FE_element *element);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Returns 1 if the all the source fields are defined in the supplied <element>.
==============================================================================*/

int Computed_field_default_is_defined_at_node(struct Computed_field *field,
	struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Returns 1 if all the source fields are defined at the supplied <node>.
==============================================================================*/

int Computed_field_default_has_multiple_times(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 22 November 2001

DESCRIPTION :
Returns 1 if any of the source fields have multiple times.
==============================================================================*/

int Computed_field_set_coordinate_system_from_sources(
	struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 3 July 2000

DESCRIPTION :
Sets the coordinate system of the <field> to match that of it's sources.
==============================================================================*/

int Computed_field_default_has_numerical_components(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Most computed fields have numerical components so this function returns 1.
==============================================================================*/

int Computed_field_evaluate_cache_at_node(
	struct Computed_field *field,struct FE_node *node, FE_value time);
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
Calculates the values of <field> at <node>, if it is defined over the element.
Upon successful return the node values of the <field> are stored in its cache.

???RC Could have a separate values cache for node computations. I am thinking of
cases where we have wrappers for calculating a coordinate field at element:xi
taken from a field or fields at a node - for showing the projection of a data
point during mesh fitting. At present the coordinate field of data pt. position
may be the same as that of the element, but the position is quite different.
Ideally, they should have distinct coordinate fields, but 3-component coordinate
fields with the name 'coordinates' are quite pervasive.
==============================================================================*/

int Computed_field_evaluate_cache_in_element(
	struct Computed_field *field,struct FE_element *element,FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives);
/*******************************************************************************
LAST MODIFIED : 30 November 2001

DESCRIPTION :
Calculates the values and derivatives (if <calculate_derivatives> set) of
<field> at <element>:<xi>, if it is defined over the element. Upon successful
return values and derivatives of the field are stored in the internal cache for
the <field>. <xi> is assumed to contain the same number of values as the
dimension of the element.

The optional <top_level_element> may be supplied for the benefit of this or
any source fields that may require calculation on it instead of a face or line.
FIBRE_AXES and GRADIENT are examples of such fields, since they require
top-level coordinate derivatives. The term "top_level" refers to an ultimate
parent element for the face or line, eg. the 3-D element parent to 2-D faces.
If no such top level element is supplied and one is required, then the first
available parent element will be chosen - if the user requires a top-level
element in the same group as the face or with the face on the correct side,
then they should supply the top_level_element here. Once a field has switched
to being calculated on the top_level_element, all its source fields will be
too - this should be understood when supplying source fields to such functions.
==============================================================================*/

int Computed_field_evaluate_source_fields_cache_at_node(
	struct Computed_field *field,struct FE_node *node, FE_value time);
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
Calculates the cache values of each source field in <field> at <node>, if it 
is defined over the element.
Upon successful return the node values of the source fields are stored in their
cache.
==============================================================================*/

int Computed_field_evaluate_source_fields_cache_in_element(
	struct Computed_field *field,struct FE_element *element,FE_value *xi,
	FE_value time,struct FE_element *top_level_element,int calculate_derivatives);
/*******************************************************************************
LAST MODIFIED : 30 November 2001

DESCRIPTION :
Calculates the cache values of each source field in <field> in <element>, if it 
is defined over the element.
Upon successful return the element values of the source fields are stored in their
cache.
==============================================================================*/

char *Computed_field_default_evaluate_as_string_at_node(
	struct Computed_field *field, int component_number, struct FE_node *node,
	FE_value time);
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
Returns a string describing the value/s of the <field> at the <node>. A string 
built up of comma separated values evaluated
for the field in Computed_field_evaluate_cache_at_node. The FE_value exception
is used since it is likely the values are already in the cache in most cases,
or can be used by other fields again if calculated now.
The <component_number> indicates which component to calculate.  Use -1 to 
create a string which represents all the components.
It is up to the calling function to DEALLOCATE the returned string.
==============================================================================*/

char *Computed_field_default_evaluate_as_string_in_element(
	struct Computed_field *field,int component_number,
	struct FE_element *element,FE_value *xi,FE_value time,
	struct FE_element *top_level_element);
/*******************************************************************************
LAST MODIFIED : 30 November 2001

DESCRIPTION :
Returns a string representing the value of <field>.<component_number> at
<element>:<xi>. Calls Computed_field_evaluate_cache_in_element and converts 
the value for <component_number> to a string (since result may already be in 
cache).

Use -1 as the <component_number> if you want all the components.

The <top_level_element> parameter has the same use as in
Computed_field_evaluate_cache_in_element.
==============================================================================*/

int Computed_field_default_get_native_discretization_in_element(
	struct Computed_field *field,struct FE_element *element,int *number_in_xi);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Inherits its result from the first source field.
==============================================================================*/

int Computed_field_default_get_native_resolution(struct Computed_field *field,
        int *dimension, int **sizes, 
	struct Computed_field **texture_coordinate_field);
/*******************************************************************************
LAST MODIFIED : 15 September 2005

DESCRIPTION :
Inherits its result from the first source field -- if any.
==============================================================================*/

int Computed_field_extract_rc(struct Computed_field *field,
	int element_dimension,FE_value *rc_coordinates,FE_value *rc_derivatives);
/*******************************************************************************
LAST MODIFIED : 9 February 1999

DESCRIPTION :
Takes the values in <field> and converts them from their current coordinate
system into rectangular cartesian, returning them in the 3 component
<rc_coordinates> array. If <rc_derivatives> is not NULL, the derivatives are
also converted to rc and returned in that 9-component FE_value array.
Note that odd coordinate systems, such as FIBRE are treated as if they are
RECTANGULAR_CARTESIAN, which just causes a copy of values.
If <element_dimension> or the number of components in <field> are less than 3,
the missing places in the <rc_coordinates> and <rc_derivatives> arrays are
cleared to zero.
???RC Uses type float for in-between values x,y,z and jacobian for future
compatibility with coordinate system transformation functions in geometry.c.
This causes a slight drop in performance.

Note the order of derivatives:
1. All the <element_dimension> derivatives of component 1.
2. All the <element_dimension> derivatives of component 2.
3. All the <element_dimension> derivatives of component 3.
==============================================================================*/

#define Computed_field_is_type(filter) Computed_field_is_type_ ## filter

#define DEFINE_DEFAULT_COMPUTED_FIELD_IS_TYPE_FUNCTION(filter) \
int Computed_field_is_type(filter)(struct Computed_field *field) \
/******************************************************************************* \
LAST MODIFIED : 7 November 2005 \
\
DESCRIPTION : \
==============================================================================*/ \
{ \
	int return_code; \
\
	ENTER(Computed_field_is_type_ ## filter); \
	if (field) \
	{ \
		return_code = \
		  (field->type_string == computed_field_ ## filter ## _type_string); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"Computed_field_is_type_" #filter ".  Missing field"); \
		return_code = 0; \
	} \
\
	return (return_code); \
} /* Computed_field_is_type_ ## filter */

#define DEFINE_DEFAULT_LIST_COMPUTED_FIELD_FUNCTION(filter,field_string,value_string) \
int list_Computed_field_ ## filter(struct Computed_field *field) \
/******************************************************************************* \
LAST MODIFIED : 3 May 2006 \
\
DESCRIPTION : \
==============================================================================*/ \
{ \
	int i, return_code;									\
\
	ENTER(list_Computed_field_ ## filter); \
	if (field) \
	{ \
		if (0 < field->number_of_source_fields) \
		{ \
			display_message(INFORMATION_MESSAGE,	\
				"    " #field_string " :");									 \
			for (i = 0 ; i < field->number_of_source_fields ; i++)	 \
			{																			 \
				display_message(INFORMATION_MESSAGE,						 \
					" %s", field->source_fields[i]->name);					 \
			}																			 \
			display_message(INFORMATION_MESSAGE, "\n");					 \
		} \
		if (0 < field->number_of_source_values) \
		{ \
			display_message(INFORMATION_MESSAGE,	\
				"    " #value_string " :");									 \
			for (i = 0 ; i < field->number_of_source_values ; i++)	 \
			{																			 \
				display_message(INFORMATION_MESSAGE,						 \
					" %g", field->source_values[i]);							 \
			}																			 \
			display_message(INFORMATION_MESSAGE, "\n");					 \
		} \
		return_code = 1; \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"list_Computed_field_" #filter ".  Missing field"); \
		return_code = 0; \
	} \
\
	return (return_code); \
} /* list_Computed_field_ ## filter */

#define Computed_field_get_command_string(filter) \
	Computed_field_ ## filter ## _get_command_string

#define DEFINE_DEFAULT_COMPUTED_FIELD_GET_COMMAND_STRING_FUNCTION( \
	filter,field_string,value_string) \
static char *Computed_field_get_command_string(filter)(struct Computed_field *field) \
/******************************************************************************* \
LAST MODIFIED : 3 May 2006 \
\
DESCRIPTION : \
==============================================================================*/ \
{ \
	char *command_string, *field_name, temp_string[40]; \
	int error, i;								  \
\
	ENTER(Computed_field_get_command_string(filter));	\
	command_string = (char *)NULL; \
	if (field) \
	{ \
		error = 0; \
		append_string(&command_string, \
			computed_field_ ## filter ## _type_string, &error); \
		if (0 < field->number_of_source_fields) \
		{ \
			append_string(&command_string, " " #field_string " ", &error); \
			for (i = 0 ; i < field->number_of_source_fields ; i++) \
			{ \
				if (GET_NAME(Computed_field)(field->source_fields[i], &field_name)) \
				{ \
					make_valid_token(&field_name); \
					append_string(&command_string, field_name, &error); \
					DEALLOCATE(field_name); \
				} \
			} \
		} \
		if (0 < field->number_of_source_values) \
		{ \
			append_string(&command_string, " " #value_string, &error); \
			for (i = 0 ; i < field->number_of_source_values ; i++) \
			{ \
				sprintf(temp_string, " %g", field->source_values[i]); \
				append_string(&command_string, temp_string, &error); \
			} \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"Computed_field_" #filter "_get_command_string.  Missing field"); \
	} \
\
	return (command_string); \
} /* Computed_field_ ## filter ## get_command_string */

#endif /* !defined (COMPUTED_FIELD_PRIVATE_H) */
