/*******************************************************************************
FILE : computed_field_private.h

LAST MODIFIED : 4 July 2000

DESCRIPTION :
==============================================================================*/
#if !defined (COMPUTED_FIELD_PRIVATE_H)
#define COMPUTED_FIELD_PRIVATE_H

typedef int (*Computed_field_clear_type_specific_function)(
	struct Computed_field *field);
typedef void* (*Computed_field_copy_type_specific_function)(
	struct Computed_field *field);
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
typedef int (*Computed_field_can_be_destroyed_function)(
	struct Computed_field *field);
typedef int (*Computed_field_evaluate_cache_at_node_function)(
	struct Computed_field *field,struct FE_node *node);
typedef int (*Computed_field_evaluate_cache_in_element_function)(
	struct Computed_field *field,struct FE_element *element,FE_value *xi,
	struct FE_element *top_level_element,int calculate_derivatives);
typedef char* (*Computed_field_evaluate_as_string_at_node_function)(
	struct Computed_field *field,int component_number,struct FE_node *node);
typedef char* (*Computed_field_evaluate_as_string_in_element_function)(
	struct Computed_field *field,int component_number,
	struct FE_element *element,FE_value *xi,struct FE_element *top_level_element);
typedef int (*Computed_field_set_values_at_node_function)(
	struct Computed_field *field,struct FE_node *node,FE_value *values);
typedef int (*Computed_field_set_values_in_element_function)(
	struct Computed_field *field,struct FE_element *element,
	int *number_in_xi,FE_value *values);
typedef int (*Computed_field_get_native_discretization_in_element_function)(
	struct Computed_field *field,struct FE_element *element,int *number_in_xi);
typedef int (*Computed_field_find_element_xi_function)(
	struct Computed_field *field, FE_value *values, int number_of_values, 
	struct FE_element **element, FE_value *xi,
	struct GROUP(FE_element) *search_element_group);
typedef int (*List_Computed_field_function)(struct Computed_field *field);
typedef int (*List_Computed_field_commands_function)(
	struct Computed_field *field);

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
LAST MODIFIED : 17 June 1999

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
	enum Computed_field_type type;

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

	/* last mapping successfully used by Computed_field_find_element_xi so 
		that it can first try this element again */
	struct Computed_field_element_texture_mapping *find_element_xi_mapping;

	/* cache used by external routine Computed_field_find_element_xi_special,
	 contains all sorts of things we don't want to include in computed_field */
	struct Computed_field_find_element_xi_special_cache *find_element_xi_cache;

	/* for types COMPUTED_FIELD_FINITE_ELEMENT, COMPUTED_FIELD_EMBEDDED,
		 COMPUTED_FIELD_NODE_VALUE,COMPUTED_FIELD_NODE_ARRAY_VALUE_AT_TIME only */
	/* the real FE_field being computed */
	struct FE_field *fe_field;

	/* for types COMPUTED_FIELD_FINITE_ELEMENT, COMPUTED_FIELD_EMBEDDED only */
	/* element field values of fe_field in last_element */
	struct FE_element_field_values *fe_element_field_values;

	/* for COMPUTED_FIELD_COMPONENT only */
	int component_no;

	/* for COMPUTED_FIELD_COMPOSE only */
	struct GROUP(FE_element) *compose_element_group;

	/* for COMPUTED_FIELD_NODE_VALUE,COMPUTED_FIELD_NODE_ARRAY_VALUE_AT_TIME only */
	enum FE_nodal_value_type nodal_value_type;
	int version_number;

	/* for COMPUTED_FIELD_XI_TEXTURE_COORDINATES only */
	struct FE_element *seed_element;
	struct LIST(Computed_field_element_texture_mapping) *texture_mapping;

	/* for COMPUTED_FIELD_EXTERNAL only */
	char *child_filename;
	int timeout;
	struct Child_process *child_process;

	/* for COMPUTED_FIELD_CURVE_LOOKUP only */
	struct Control_curve *curve;

	/* for COMPUTED_FIELD_PROJECTION only */
	double *projection_matrix;

	/* for COMPUTED_FIELD_NEW_TYPES */
	/* Soon this will be the only way it is done. */
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
	Computed_field_can_be_destroyed_function
	   computed_field_can_be_destroyed_function;
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
	List_Computed_field_commands_function list_Computed_field_commands_function;

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

int Computed_field_default_has_numerical_components(struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Most computed fields have numerical components so this function returns 1.
==============================================================================*/

int Computed_field_evaluate_source_fields_cache_at_node(
	struct Computed_field *field,struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Calculates the cache values of each source field in <field> at <node>, if it 
is defined over the element.
Upon successful return the node values of the source fields are stored in their
cache.
==============================================================================*/

int Computed_field_evaluate_source_fields_cache_in_element(
	struct Computed_field *field,struct FE_element *element,FE_value *xi,
	struct FE_element *top_level_element,int calculate_derivatives);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Calculates the cache values of each source field in <field> in <element>, if it 
is defined over the element.
Upon successful return the element values of the source fields are stored in their
cache.
==============================================================================*/

char *Computed_field_default_evaluate_as_string_at_node(
	struct Computed_field *field, int component_number, struct FE_node *node);
/*******************************************************************************
LAST MODIFIED : 4 July 2000

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
	struct FE_element *element,FE_value *xi,struct FE_element *top_level_element);
/*******************************************************************************
LAST MODIFIED : 30 June 2000

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

int Computed_field_get_top_level_element_and_xi(struct FE_element *element, 
	FE_value *xi, int element_dimension, struct FE_element *top_level_element,
	FE_value *top_level_xi, int *top_level_element_dimension);
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
Finds the <top_level_element>, <top_level_xi> and <top_level_element_dimension>
for the given <element> and <xi>.  If <top_level_element> is already set it 
is checked and the <top_level_xi> calculated.
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_PRIVATE_H) */
