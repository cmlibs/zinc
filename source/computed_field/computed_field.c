/*******************************************************************************
FILE : computed_field.c

LAST MODIFIED : 12 October 2001

DESCRIPTION :
A Computed_field is an abstraction of an FE_field. For each FE_field there is
a wrapper Computed_field automatically generated that can be called on to
evaluate the field in an element or node. The interface for evaluating
Computed_fields is much simpler than for FE_field, since they hide details of
caching of FE_element_field_values, for example. Their main benefit is in
allowing new types of fields to be defined as functions of other fields and
source information, such as scale, offset, magnitude, gradient,
coordinate transformations etc., thus providing cmgui with the ability to
provide customised features without cluttering dialogs such as the graphical
element editor. Also, finite_element_to_graphics_object is greatly simplified
because it can assume all coordinate and other fields are in rectangular
cartesian coordinates - if they are not already, the rest of the program can
make a simple wrapper computed_field effecting this change. A positive
consequence of this change is that this file should in time contain the only
code for invoking coordinate transformations in the entire program.

In addition to these functional benefits, computed fields cache the last
position they were evaluated at and the values and derivatives of the field at
that point so that if values at that point are requested again they can be
immediately returned. This allows functions using a number of fields, some
possibly depending on each other to get the maximum benefit out of values that
have already been calculated, without requiring complex logic for determining
if a value is already known.


NOTES ABOUT COMPUTED FIELDS:
----------------------------

- Each Computed_field has a coordinate system that tells the application how to
interpret the field/components. The important thing to note is that it does not
have to be the truth - you could for instance create a RC computed field that is
simply a copy of the prolate field; the graphics functions will then plot the
(lambda,mu,theta) as if they are (x,y,z) - to open up the heart, etc. If a
coordinate system is not relevant to the field in question it should be left
as rectangular cartesian so no automatic conversions are applied to it.

- The COMPUTED_FIELD_RC_COORDINATE type provides the code for conversion to
rectangular cartesian coordinates;

- The COMPUTED_FIELD_DEFAULT_COORDINATE type is the new substitute for the
NULL coordinate FE_field designation, meaning it returns values for the first
coordinate field defined at the node/element. Eventually it is hoped that each
Regional will have a default coordinate field that can be chosen directly.

- The number of components of a computed field depends on its type, parameters
and/or source fields. In many cases it is possible to modify a field to give
it different number of components. This has been prevented by the manager copy
functions because a field may have been chosen on the basis of its number of
components, and changing this number could have dire consequences.

- The manager copy function also prevents you from creating a field that depends
on itself in any way, to prevent infinite loops.

- Each computed field has names for its components. If the field is a wrapper
for an FE_field, the component names will match those for the FE_field,
otherwise they will be called "1", "2", etc.

- COMPUTED_FIELD_COMPONENT type allows individual components of fields to be
treated as if they are separate scalar fields, allowing old behaviour for
scalar plots to be reproduced.

- Handling messages from the MANAGER(Computed_field) is made tricky by the
possibility of fields depending on each other. If you are informed that field
"bob" has changed, and you are using field "fred", you must call function
Computed_field_depends_on_Computed_field to determine the dependency.

- After using a computed field in a block of code (ie. for a period until the
application main loop is returned to), you must call Computed_field_clear_cache
to clear its cache so that it is not accessing elements or nodes or promising
values at certain positions that may have changed. Alternative of getting
messages from element/node managers seems a little expensive. Forgetting to
clear the cache could cause some unexpected behaviour.

- COMPUTED_FIELD_XI is a simple field that simply returns the xi position that
the user requested, enabling such functionality as iso-surfaces of xi to be
performed with the same intercace.

- Computed fields that are not created by the user (eg. default coordinate, xi
and those automatically created to wrap FE_fields) have a read_only flag set,
so that the user can not modify them be text commands.


FUTURE PLANS AND ISSUES:
------------------------

- Handling problems of evaluating fields on faces and lines that must be
calculated on top level elements, eg. gradient, fibres, for which derivatives on
the top level element are needed. Since in many cases it is important for the
rest of the program to be able to specify which top-level element, if any - eg.
one in the same group with the face on the correct side, the element evaluation
functions will now allow a top_level_element to be supplied in addition to the
main element that the field is evaluated on. Any field that requires calculation
on a top-level element will use the given one or any top level element it can
find if none supplied.

eg. fibre_axes:
-coordinates - must be evaluated on top-level element with derivatives
-fibres - may be evaluated on face or line

When evaluating fibre_axes source fields, ensure we have a top_level_element if
one not supplied, then convert xi to top_level_xi. Then ask coordinates to be
evaluated on the top_level_element:top_level_xi, fibres on the main_element,
and in both cases pass on the top_level_element in case they have source fields
that wish to use it.

Random thoughts.

If evaluating fibres on surfaces and subsequently asking for coordinates and
derivatives on the face, how do we know whether we can convert the coordinates
to the face and return them? Or can we assume this at all?

Fields such as cmiss_number and xi depend on whether the element is the face or
is top_level. Never want cmiss_number/xi to be evaluated at anything but the
main element:xi; hence, need to pass these to source fields as well as
top_level_element:top_level_xi. No. If you make a field that must be evaluated
on top_level_elements, then you must expect the element for its source fields
to be top level too.

As a result, the field changes to the top_level_element only when necessary,
and the values returned for the top_level_element are in no way expected to
match those for the same position on a face for eg. coordinate field.

- Have a separate values cache for node computations. I am thinking of
cases where we have wrappers for calculating a coordinate field at element:xi
taken from a field or fields at a node - for showing the projection of a data
point during mesh fitting. At present the coordinate field of data pt. position
may be the same as that of the element, but the position is quite different.
Ideally, they should have distinct coordinate fields, but 3-component coordinate
fields with the name 'coordinates' are quite pervasive.

- Running into problems with clutter in the Computed_field structure. Should
re-write in C++. The best model for the object in C++ is to have a simple main
object CONTAINING a replacable function object that is pure C++. The contents
of the function objects could vary greatly depending on the source information
and cache required by that type of field. Advantage could be made out of
inheritance for similar function objects, while the interface to the rest of
the computed field object will be identical.

- Ability to evaluate a field as a string. For fields returning floats, this
would be a comma delimited list of values. Perhaps the number format could be
supplied as well. For FE_fields that do not evaluate to FE_value types (eg.
char strings, integers) they will have zero components, but could return a
valid name. This is the case for unemap electrodes.

- Should handle coordinate system differently. For the majority of field types
it can simply be assumed, eg. from source fields, or RC/irrelevant if scalar.
For those that do allow it to be specified, should make it a leaf option with
the gfx define field commands. Also should not allow coordinate system to be
changed if field is in use, since field may be chosen on the basis of this,
like the number of components.

- Allowing computed field values to be set as well as evaluated. Certain fields
may have easily invertible values that in many cases will allow us to, say,
tell a node its new RC position/derivatives, which causes the source fields to
be set leading back to the original calculation of the FE_field at a node. The
coordinates at the node may be in a prolate coordinate system, but one of the
fields between the node and its representation that is being changed will be
able to undo the coordinate transformation. We intend to use this idea to
support derivative editing for all coordinate systems using new type
COMPUTED_FIELD_NODE_VALUE.


HOW TO ADD NEW COMPUTED FIELDS:
-------------------------------

- add new type name enum Computed_field_type in computed_field.h;
- add any new source data storage to struct Computed_field, and make sure it
is initialised in CREATE(Computed_field) and cleaned up by function
Computed_field_clear_type;
- add functions Computed_field_get/set_type_NEW_TYPE_NAME;
- add function define_Computed_field_NEW_TYPE_NAME;
- add code to some or all functions containing the following text:
#### Must ensure implemented correctly for new Computed_field_type. ####
==============================================================================*/
#include <math.h>
#include "computed_field/computed_field.h"
/* temporary until rc_coordinate is shifted there too */
#include "computed_field/computed_field_coordinate.h"
#include "computed_field/computed_field_find_xi.h"
#include "computed_field/computed_field_private.h"
/* remove following once rc_vector is new_type */
#include "computed_field/computed_field_wrappers.h"
#include "finite_element/finite_element.h"
#include "general/child_process.h"
#include "general/compare.h"
#include "general/debug.h"
#include "general/geometry.h"
#include "general/indexed_list_private.h"
#include "general/list_private.h"
#include "general/manager_private.h"
#include "general/matrix_vector.h"
#include "general/mystring.h"
#include "general/value.h"
#include "user_interface/message.h"

FULL_DECLARE_INDEXED_LIST_TYPE(Computed_field);

FULL_DECLARE_MANAGER_TYPE(Computed_field);

struct Computed_field_component
/*******************************************************************************
LAST MODIFIED : 22 January 1999

DESCRIPTION :
Used to specify a component of a Computed_field with function
set_Computed_field_component.
???RC Note that in its current use the field is NOT assumed to be accessed by
this structure in set_Computed_field_component.
==============================================================================*/
{
	struct Computed_field *field;
	int component_no;
}; /* struct Computed_field_component */

struct Computed_field_type_data
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Stores information defining a type of computed field so that it can be 
accessed by the rest of the program.
==============================================================================*/
{
	char *name;
	Define_Computed_field_type_function define_Computed_field_type_function;
	void *define_type_user_data;
	int access_count;
};

PROTOTYPE_OBJECT_FUNCTIONS(Computed_field_type_data);

DECLARE_LIST_TYPES(Computed_field_type_data);
PROTOTYPE_LIST_FUNCTIONS(Computed_field_type_data);

struct Computed_field_package
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
Contains all information for editing and maintaining Computed_fields, including
the MANAGER(Computed_field). Holds pointer to MANAGER(FE_field) since computed
fields are based on them, and callback ID for FE_fields, since Computed_field
wrappers need to be automatically created for each FE_field.
???RC Make macro PACKAGE(Computed_field) etc.?
==============================================================================*/
{
	struct MANAGER(Computed_field) *computed_field_manager;
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(FE_element) *fe_element_manager;
	void *fe_field_manager_callback_id;
	struct LIST(Computed_field_type_data) *computed_field_type_list;
}; /* struct Computed_field_package */

/*
Module functions
----------------
*/

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Computed_field,name,char *,strcmp)

DECLARE_LOCAL_MANAGER_FUNCTIONS(Computed_field)

struct Computed_field_type_data *CREATE(Computed_field_type_data)
   (char *name, Define_Computed_field_type_function 
		define_Computed_field_type_function, void *define_type_user_data)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Creates a structure representing a type of computed field.  The <name> should
point to a static string which is used as the identifier of that type
throughout the program.  The <define_Computed_field_type_function> is added to
the define_computed_field option table when needed.
==============================================================================*/
{
	struct Computed_field_type_data *type_data;

	ENTER(CREATE(Computed_field_type_data));
	
	if (name && define_Computed_field_type_function)
	{
		if (ALLOCATE(type_data,struct Computed_field_type_data,1))
		{
			type_data->name = name;
			type_data->define_Computed_field_type_function = 
				define_Computed_field_type_function;
			type_data->define_type_user_data = define_type_user_data;
			type_data->access_count = 0;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Computed_field_type_data).  Not enough memory");
			type_data = (struct Computed_field_type_data *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Computed_field_type_data).  Invalid arguments");
		type_data = (struct Computed_field_type_data *)NULL;
	}
	LEAVE;

	return (type_data);
} /* CREATE(Computed_field_type_data) */

int DESTROY(Computed_field_type_data)
	(struct Computed_field_type_data **data_address)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Frees memory/deaccess data at <*data_address>.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Computed_field_type_data));
	if (data_address&&*data_address)
	{
		if (0 >= (*data_address)->access_count)
		{
			DEALLOCATE(*data_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Computed_field_type_data).  Positive access_count");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Computed_field_type_data).  Missing mapping");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Computed_field_type_data) */

DECLARE_OBJECT_FUNCTIONS(Computed_field_type_data)
FULL_DECLARE_INDEXED_LIST_TYPE(Computed_field_type_data);
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Computed_field_type_data,
  name, char *, strcmp)
DECLARE_INDEXED_LIST_FUNCTIONS(Computed_field_type_data)

int Computed_field_extract_rc(struct Computed_field *field,
	int element_dimension,FE_value *rc_coordinates,FE_value *rc_derivatives)
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
{
	FE_value *source;
	float coordinates[3],derivatives[9],*destination,x,y,z,*jacobian,temp[9];
	int field_components,i,j,return_code;
	
	ENTER(Computed_field_extract_rc);
	if (field&&rc_coordinates&&(((FE_value *)NULL==rc_derivatives)||
		((0<element_dimension)&&field->derivatives_valid)))
	{
		field_components=field->number_of_components;
		/* copy coordinates, padding to 3 components */
		for (i=0;i<3;i++)
		{
			if (i<field_components)
			{
				coordinates[i] = (float)field->values[i];
			}
			else
			{
				coordinates[i]=0.0;
			}
		}
		if (rc_derivatives)
		{
			/* copy derivatives, padding to 3 components x 3 dimensions */
			destination=derivatives;
			source=field->derivatives;
			for (i=0;i<3;i++)
			{
				for (j=0;j<3;j++)
				{
					if ((i<field_components)&&(j<element_dimension))
					{
						*destination = (float)(*source);
						source++;
					}
					else
					{
						*destination = 0.0;
					}
					destination++;
				}
			}
			/* make sure jacobian only calculated if rc_derivatives requested */
			jacobian=temp;
		}
		else
		{
			jacobian=(float *)NULL;
		}

		switch (field->coordinate_system.type)
		{
			case CYLINDRICAL_POLAR:
			{
				cylindrical_polar_to_cartesian(
					coordinates[0],coordinates[1],coordinates[2],&x,&y,&z,jacobian);
			} break;
			case SPHERICAL_POLAR:
			{
				spherical_polar_to_cartesian(
					coordinates[0],coordinates[1],coordinates[2],&x,&y,&z,jacobian);
			} break;
			case PROLATE_SPHEROIDAL:
			{
				prolate_spheroidal_to_cartesian(
					coordinates[0],coordinates[1],coordinates[2],
					field->coordinate_system.parameters.focus,&x,&y,&z,jacobian);
			} break;
			case OBLATE_SPHEROIDAL:
			{
				oblate_spheroidal_to_cartesian(
					coordinates[0],coordinates[1],coordinates[2],
					field->coordinate_system.parameters.focus,&x,&y,&z,jacobian);
			} break;
			default:
			{
				/* treat all others as RECTANGULAR_CARTESIAN; copy coordinates */
				x=coordinates[0];
				y=coordinates[1];
				z=coordinates[2];
				if (rc_derivatives)
				{
					for (i=0;i<9;i++)
					{
						rc_derivatives[i]=(FE_value)(derivatives[i]);
					}
					/* clear jacobian to avoid derivative conversion below */
					jacobian=(FE_value *)NULL;
				}
			} break;
		}
		rc_coordinates[0]=(FE_value)x;
		rc_coordinates[1]=(FE_value)y;
		rc_coordinates[2]=(FE_value)z;
		if (jacobian)
		{
			for (i=0;i<3;i++)
			{
				/* derivative of x with respect to xi[i] */
				rc_derivatives[i]=(FE_value)(
					jacobian[0]*derivatives[0+i]+
					jacobian[1]*derivatives[3+i]+
					jacobian[2]*derivatives[6+i]);
				/* derivative of y with respect to xi[i] */
				rc_derivatives[3+i]=(FE_value)(
					jacobian[3]*derivatives[0+i]+
					jacobian[4]*derivatives[3+i]+
					jacobian[5]*derivatives[6+i]);
				/* derivative of z with respect to xi[i] */
				rc_derivatives[6+i]=(FE_value)(
					jacobian[6]*derivatives[0+i]+
					jacobian[7]*derivatives[3+i]+
					jacobian[8]*derivatives[6+i]);
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_extract_rc.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_extract_rc */

int Computed_field_clear_type(struct Computed_field *field)
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
{
	int i,return_code;

	ENTER(Computed_field_clear_type);
	if (field)
	{
		/* must clear the cache first */
		Computed_field_clear_cache(field);

		/* clear values and derivatives cache as size may be changing */
		if (field->values)
		{
			DEALLOCATE(field->values);
			DEALLOCATE(field->derivatives);
		}

		if (field->component_names)
		{
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				DEALLOCATE(field->component_names[i]);
			}
			DEALLOCATE(field->component_names);
		}

		/* for COMPUTED_FIELD_COMPONENT only */
		field->component_no=0;

		/* for COMPUTED_FIELD_COMPOSE only */
		if (field->compose_element_group)
		{
			DEACCESS(GROUP(FE_element))(&field->compose_element_group);
		}
	
		/* for COMPUTED_FIELD_EXTERNAL only */
		if (field->child_filename)
		{
			DEALLOCATE(field->child_filename);
		}
		if (field->child_process)
		{
			DEACCESS(Child_process)(&(field->child_process));
			field->child_process = (struct Child_process *)NULL;
		}
		field->timeout = 0;

		/* for COMPUTED_FIELD_NEW_TYPES */
		if (field->type_specific_data)
		{
			if(field->computed_field_clear_type_specific_function)
			{
				field->computed_field_clear_type_specific_function(
					field);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_clear_type.  "
					"Type specific data but no function to clear it.");
				return_code=0;
			}
			field->type_specific_data = (void *)NULL;
		}
		field->type_string = (char *)NULL;

		/* for all Computed_field_types calculated from others */
		if (field->source_fields)
		{
			for (i=0;i< field->number_of_source_fields;i++)
			{
				DEACCESS(Computed_field)(&(field->source_fields[i]));
			}
			DEALLOCATE(field->source_fields);
		}
		field->number_of_source_fields=0;
		if (field->source_values)
		{
			DEALLOCATE(field->source_values);
		}
		field->number_of_source_values=0;

		/* Clear all methods */
		field->computed_field_clear_type_specific_function = 
			(Computed_field_clear_type_specific_function)NULL;
		field->computed_field_copy_type_specific_function =
			(Computed_field_copy_type_specific_function)NULL;
		field->computed_field_clear_cache_type_specific_function =
			(Computed_field_clear_cache_type_specific_function)NULL;
		field->computed_field_type_specific_contents_match_function = 
			(Computed_field_type_specific_contents_match_function)NULL;
		field->computed_field_is_defined_in_element_function =
			(Computed_field_is_defined_in_element_function)NULL;
		field->computed_field_is_defined_at_node_function =
			(Computed_field_is_defined_at_node_function)NULL;
		field->computed_field_has_numerical_components_function =
			(Computed_field_has_numerical_components_function)NULL;
		field->computed_field_can_be_destroyed_function =
			(Computed_field_can_be_destroyed_function)NULL;
		field->computed_field_evaluate_cache_at_node_function =
			(Computed_field_evaluate_cache_at_node_function)NULL; 
		field->computed_field_evaluate_cache_in_element_function =
			(Computed_field_evaluate_cache_in_element_function)NULL; 
		field->computed_field_evaluate_as_string_in_element_function =
			(Computed_field_evaluate_as_string_in_element_function)NULL; 
		field->computed_field_evaluate_as_string_at_node_function =
			(Computed_field_evaluate_as_string_at_node_function)NULL;
		field->computed_field_set_values_at_node_function =
			(Computed_field_set_values_at_node_function)NULL;
		field->computed_field_set_values_in_element_function =
			(Computed_field_set_values_in_element_function)NULL;
		field->computed_field_get_native_discretization_in_element_function =	
			(Computed_field_get_native_discretization_in_element_function)NULL;
		field->computed_field_find_element_xi_function =	
			(Computed_field_find_element_xi_function)NULL;
		field->list_Computed_field_function =
			(List_Computed_field_function)NULL;
		field->list_Computed_field_commands_function =
			(List_Computed_field_commands_function)NULL;

		field->type=COMPUTED_FIELD_INVALID;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_clear_type.  Missing field");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_clear_type */

static int Computed_field_wraps_field_component(struct Computed_field *field,
	void *field_component_void)
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Returns true if <field> is of type COMPUTED_FIELD_COMPONENT returning the
field.component referred to by <field_component>.
==============================================================================*/
{
	int return_code;
	struct Computed_field_component *field_component;

	ENTER(Computed_field_wraps_field_component);
	if (field&&(field_component=
		(struct Computed_field_component *)field_component_void))
	{
		return_code=(COMPUTED_FIELD_COMPONENT==field->type)&&
			(field->source_fields[0]==field_component->field)&&
			(field->component_no==field_component->component_no);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_wraps_field_component.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_wraps_field_component */

int Computed_field_set_coordinate_system_from_sources(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 3 July 2000

DESCRIPTION :
Sets the coordinate system of the <field> to match that of it's sources.
==============================================================================*/
{
	int i, return_code;
	struct Coordinate_system *coordinate_system_ptr;

	ENTER(Computed_field_set_coordinate_system_from_sources);
	if (field)
	{
		return_code = 1;
		if (field->number_of_source_fields > 0)
		{
			coordinate_system_ptr = 
				Computed_field_get_coordinate_system(field->source_fields[0]);
			Computed_field_set_coordinate_system(field, coordinate_system_ptr);
			i = 1;
			while (i < field->number_of_source_fields && 
				Coordinate_systems_match(coordinate_system_ptr,
					Computed_field_get_coordinate_system(field->source_fields[1])))
			{
				i++;
			}
			if (i < field->number_of_source_fields)
			{
				display_message(WARNING_MESSAGE,
					"Computed_field_set_coordinate_system_from_sources."
					"  Source fields differ in coordinate system\n"
					"     Defaulting to coordinate system from first source field.");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_coordinate_system_from_sources.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_coordinate_system_from_sources */

/*
Global functions
----------------
*/

struct Computed_field *CREATE(Computed_field)(char *name)
/*******************************************************************************
LAST MODIFIED : 4 November 1999

DESCRIPTION :
Creates a basic Computed_field with the given <name>. Its type is initially
COMPUTED_FIELD_CONSTANT with 1 component, returning a value of zero.
==============================================================================*/
{
	struct Computed_field *field;

	ENTER(CREATE(Computed_field));
	if (name)
	{
		if (ALLOCATE(field,struct Computed_field,1)&&
			(field->name=duplicate_string(name))&&
			ALLOCATE(field->source_values,FE_value,1))
		{
			/* initialise all members of computed_field */
			field->number_of_components=1;
			/* allowed to modify/remove from manager until disabled with
				 Computed_field_set_read_only */
			field->read_only=0;
			field->coordinate_system.type=RECTANGULAR_CARTESIAN;

			field->type=COMPUTED_FIELD_CONSTANT;

			field->component_names = (char **)NULL;

			/* values/derivatives cache and working_values */
			field->values=(FE_value *)NULL;
			field->derivatives=(FE_value *)NULL;
			field->derivatives_valid=0;
			field->element=(struct FE_element *)NULL;
			field->node=(struct FE_node *)NULL;

			field->find_element_xi_cache=(struct Computed_field_find_element_xi_special_cache *)NULL;

			/* for COMPUTED_FIELD_COMPONENT only */
			field->component_no=-1;

			/* for COMPUTED_FIELD_COMPOSE only */
			field->compose_element_group = (struct GROUP(FE_element) *)NULL;

			/* for COMPUTED_FIELD_EXTERNAL only */
			field->child_filename = (char *)NULL;
			field->child_process = (struct Child_process *)NULL;
			field->timeout = 0;

			/* for COMPUTED_FIELD_NEW_TYPES */
			/* Soon this will be the only way it is done. */
			field->computed_field_clear_type_specific_function = 
				(Computed_field_clear_type_specific_function)NULL;
			field->computed_field_copy_type_specific_function =
				(Computed_field_copy_type_specific_function)NULL;
	   	field->computed_field_clear_cache_type_specific_function =
				(Computed_field_clear_cache_type_specific_function)NULL;
			field->computed_field_type_specific_contents_match_function = 
				(Computed_field_type_specific_contents_match_function)NULL;
			field->computed_field_is_defined_in_element_function =
				(Computed_field_is_defined_in_element_function)NULL;
	   	field->computed_field_is_defined_at_node_function =
				(Computed_field_is_defined_at_node_function)NULL;
			field->computed_field_has_numerical_components_function =
				(Computed_field_has_numerical_components_function)NULL;
			field->computed_field_can_be_destroyed_function =
				(Computed_field_can_be_destroyed_function)NULL;
	   	field->computed_field_evaluate_cache_at_node_function =
				(Computed_field_evaluate_cache_at_node_function)NULL; 
	   	field->computed_field_evaluate_cache_in_element_function =
				(Computed_field_evaluate_cache_in_element_function)NULL; 
	   	field->computed_field_evaluate_as_string_in_element_function =
				(Computed_field_evaluate_as_string_in_element_function)NULL; 
	   	field->computed_field_evaluate_as_string_at_node_function =
				(Computed_field_evaluate_as_string_at_node_function)NULL;
	   	field->computed_field_set_values_at_node_function =
				(Computed_field_set_values_at_node_function)NULL;
	   	field->computed_field_set_values_in_element_function =
				(Computed_field_set_values_in_element_function)NULL;
	   	field->computed_field_get_native_discretization_in_element_function =	
				(Computed_field_get_native_discretization_in_element_function)NULL;
			field->computed_field_find_element_xi_function =	
				(Computed_field_find_element_xi_function)NULL;
			field->list_Computed_field_function =
				(List_Computed_field_function)NULL;
			field->list_Computed_field_commands_function =
				(List_Computed_field_commands_function)NULL;
			
			field->type_specific_data = NULL;
			field->type_string = (char *)NULL;

			/* for all types of Computed_field calculated from others */
			field->source_fields=(struct Computed_field **)NULL;
			field->number_of_source_fields=0;
			/* set up the initial, constant [zero] field */
			field->source_values[0]=0.0;
			field->number_of_source_values=1;

			field->access_count=0;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Computed_field).  Not enough memory");
			DEALLOCATE(field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Computed_field).  Missing name");
		field=(struct Computed_field *)NULL;
	}
	LEAVE;

	return (field);
} /* CREATE(Computed_field) */

int DESTROY(Computed_field)(struct Computed_field **field_address)
/*******************************************************************************
LAST MODIFIED : 29 December 1998

DESCRIPTION :
Frees memory/deaccess objects in computed_field at <*field_address>.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field;

	ENTER(DESTROY(Computed_field));
	if (field_address&&(field= *field_address))
	{
		if (0 >= field->access_count)
		{
			DEALLOCATE(field->name);
			Computed_field_clear_type(field);
			DEALLOCATE(*field_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Computed_field).  Positive access_count");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Computed_field).  Missing field");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Computed_field) */

DECLARE_OBJECT_FUNCTIONS(Computed_field)

DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Computed_field)

DECLARE_INDEXED_LIST_FUNCTIONS(Computed_field)

DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Computed_field,name,
	char *,strcmp)

DECLARE_INDEXED_LIST_IDENTIFIER_CHANGE_FUNCTIONS(Computed_field,name)

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(Computed_field,name)
{
	char *name;
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(Computed_field,name));
	if (source&&destination)
	{
		if (source->name)
		{
			if (ALLOCATE(name,char,strlen(source->name)+1))
			{
				strcpy(name,source->name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"MANAGER_COPY_WITH_IDENTIFIER(Computed_field,name).  "
					"Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			if (return_code = MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_field,name)(
				destination, source))
			{
				/* copy values */
				DEALLOCATE(destination->name);
				destination->name=name;
			}
			else
			{
				DEALLOCATE(name);
				display_message(ERROR_MESSAGE,
					"MANAGER_COPY_WITH_IDENTIFIER(Computed_field,name).  "
					"Could not copy without identifier");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITH_IDENTIFIER(Computed_field,name).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(Computed_field,name) */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(Computed_field,name)
/*******************************************************************************
LAST MODIFIED : 4 November 1999

DESCRIPTION :
Do not allow copy if:
- it creates a self-referencing field (one that depends on itself) which will
  result in an infinite loop;
- it changes the number of components of a field in use;
???RC.  Previously denied copy if read_only flag was set in the destination
field. However, this makes it impossible to modify computed fields that wrap
fe_fields when the latter changes. Hence, now leave it up to define/destroy
functions to check if read_only flag is set.
#### Must ensure implemented correctly for new Computed_field_type. ####
==============================================================================*/
{
	char **component_names;
	FE_value *source_values;
	int i,return_code;
	struct Computed_field **source_fields;
	void *type_specific_data;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_field,name));
	if (source&&destination)
	{
		/* check <source> does not depend on <destination> else infinite loop */
		if (Computed_field_depends_on_Computed_field(source,destination))
		{
			display_message(ERROR_MESSAGE,
				"MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_field,name).  "
				"Cannot make field depend on itself");
			return_code=0;		
		}
		else
		{
			/* check not changing number of components while field is in use */
			if ((source->number_of_components!=destination->number_of_components)&&
				Computed_field_is_in_use(destination))
			{
				display_message(ERROR_MESSAGE,
					"MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_field,name).  "
					"Cannot change number of components while field is in use");
				return_code=0;		
			}
			else
			{
				source_fields=(struct Computed_field **)NULL;
				source_values=(FE_value *)NULL;
				type_specific_data = NULL;
				component_names = (char **)NULL;
				/* 1. make dynamic allocations for any new type-specific data */
				return_code = 1;
				if (source->component_names)
				{
					if (ALLOCATE(component_names, char *, 
						source->number_of_components))
					{
						for (i = 0 ; i < source->number_of_components; i++)
						{
							if (!(component_names[i]=duplicate_string(
								source->component_names[i])))
							{
								return_code = 0;
							}
						}
					}
					else
					{
						return_code = 0;
					}
				}
				if (return_code&&
					((0==source->number_of_source_fields)||ALLOCATE(source_fields,
					struct Computed_field *,source->number_of_source_fields))&&
					((0==source->number_of_source_values)||ALLOCATE(source_values,
						FE_value,source->number_of_source_values)))
				{
					if ((!source->type_specific_data)||
						(!source->computed_field_copy_type_specific_function)||
						(type_specific_data = 
							source->computed_field_copy_type_specific_function(source)))
					{
						/* 2. free current type-specific data */
						Computed_field_clear_type(destination);
						/* 3. establish the new type */
						destination->number_of_components=source->number_of_components;
						destination->read_only=source->read_only;
						COPY(Coordinate_system)(&destination->coordinate_system,
							&source->coordinate_system);
						destination->type=source->type;

						destination->component_names = component_names;
				
						/* for COMPUTED_FIELD_COMPONENT only */
						destination->component_no=source->component_no;

						/* for COMPUTED_FIELD_COMPOSE only */
						REACCESS(GROUP(FE_element))(&destination->compose_element_group,
							source->compose_element_group);

						/* for COMPUTED_FIELD_EXTERNAL only */
						if (source->child_filename)
						{
							ALLOCATE(destination->child_filename, char,
								strlen(source->child_filename) + 1);
							strcpy(destination->child_filename, source->child_filename);
						}
						destination->timeout = source->timeout;
						if (source->child_process)
						{
							destination->child_process = 
								ACCESS(Child_process)(source->child_process);
						}

						/* for COMPUTED_FIELD_NEW_TYPES */
						destination->computed_field_clear_type_specific_function = 
							source->computed_field_clear_type_specific_function;
						destination->computed_field_copy_type_specific_function =
							source->computed_field_copy_type_specific_function;
						destination->computed_field_type_specific_contents_match_function = 
							source->computed_field_type_specific_contents_match_function;
						destination->computed_field_is_defined_in_element_function =
							source->computed_field_is_defined_in_element_function;
						destination->computed_field_is_defined_at_node_function =
							source->computed_field_is_defined_at_node_function;
						destination->computed_field_has_numerical_components_function =
							source->computed_field_has_numerical_components_function;
						destination->computed_field_evaluate_cache_at_node_function =
							source->computed_field_evaluate_cache_at_node_function;
						destination->computed_field_evaluate_cache_in_element_function =
							source->computed_field_evaluate_cache_in_element_function;
						destination->computed_field_evaluate_as_string_in_element_function =
							source->computed_field_evaluate_as_string_in_element_function;
						destination->computed_field_evaluate_as_string_at_node_function =
							source->computed_field_evaluate_as_string_at_node_function;
						destination->computed_field_set_values_at_node_function =
							source->computed_field_set_values_at_node_function;
						destination->computed_field_set_values_in_element_function =
							source->computed_field_set_values_in_element_function;
						destination->computed_field_get_native_discretization_in_element_function =	
							source->computed_field_get_native_discretization_in_element_function;
						destination->computed_field_find_element_xi_function =	
							source->computed_field_find_element_xi_function;
						destination->list_Computed_field_function =
							source->list_Computed_field_function;
						destination->list_Computed_field_commands_function =
							source->list_Computed_field_commands_function;
						if (source->type_specific_data)
						{
							destination->type_specific_data = type_specific_data;
						}
						destination->type_string = source->type_string;
						

						/* for all Computed_field_types calculated from others */
						destination->number_of_source_fields=
							source->number_of_source_fields;
						for (i=0;i<source->number_of_source_fields;i++)
						{
							source_fields[i]=ACCESS(Computed_field)(source->source_fields[i]);
						}
						destination->source_fields=source_fields;

						destination->number_of_source_values=
							source->number_of_source_values;
						for (i=0;i<source->number_of_source_values;i++)
						{
							source_values[i]=source->source_values[i];
						}
						destination->source_values=source_values;
						return_code=1;
					}
					else
					{
						if (source->computed_field_copy_type_specific_function)
						{
							display_message(ERROR_MESSAGE,
								"MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_field,name).  "
								"Type specific copy function failed.");
							return_code=0;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_field,name).  "
								"Type specific data but no copy function.");
							return_code=0;
						}
						if (source_fields)
						{
							DEALLOCATE(source_fields);
						}
						if (source_values)
						{
							DEALLOCATE(source_values);
						}
						if (component_names)
						{
							for (i = 0 ; i < source->number_of_components ; i++)
							{
								if (component_names[i])
								{
									DEALLOCATE(component_names[i]);
								}
							}
							DEALLOCATE(component_names);
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_field,name).  "
						"Not enough memory");
					return_code=0;
					if (source_fields)
					{
						DEALLOCATE(source_fields);
					}
					if (source_values)
					{
						DEALLOCATE(source_values);
					}
					if (component_names)
					{
						for (i = 0 ; i < source->number_of_components ; i++)
						{
							if (component_names[i])
							{
								DEALLOCATE(component_names[i]);
							}
						}
						DEALLOCATE(component_names);
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_field,name).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_field,name) */

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Computed_field,name,char *)
{
	char *destination_name;
	int return_code;

	ENTER(MANAGER_COPY_IDENTIFIER(Computed_field,name));
	if (name&&destination)
	{
		if (name)
		{
			if (ALLOCATE(destination_name,char,strlen(name)+1))
			{
				strcpy(destination_name,name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"MANAGER_COPY_IDENTIFIER(Computed_field,name).  Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			/* copy name */
			DEALLOCATE(destination->name);
			destination->name=destination_name;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(Computed_field,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(Computed_field,name) */

DECLARE_MANAGER_FUNCTIONS(Computed_field)

DECLARE_MANAGER_IDENTIFIER_FUNCTIONS(Computed_field,name,char *)

int Computed_field_changed(struct Computed_field *field,
	struct MANAGER(Computed_field) *computed_field_manager)
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
Notifies the <computed_field_manager> that the <field> has changed.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_changed);
	if (field && computed_field_manager)
	{
		/*???RC Review Manager Messages Here */
		MANAGER_BEGIN_CHANGE(Computed_field)(computed_field_manager,
			MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Computed_field), field);
		MANAGER_END_CHANGE(Computed_field)(computed_field_manager);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_changed.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_changed */

int Computed_field_clear_cache(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 4 November 1999

DESCRIPTION :
Clears any caching of elements/values of <field> and any fields it refers to.
Must call this function for each field after you have used functions
Computed_field_evaluate_in_element or Computed_field_evaluate_at_node and they
are possibly not going to be called again for some time.
#### Must ensure implemented correctly for new Computed_field_type. ####
Only certain field types require a special implementation of this function.
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_clear_cache);
	if (field)
	{
		if (field->element)
		{
			DEACCESS(FE_element)(&field->element);
		}
		if (field->find_element_xi_cache)
		{
			DESTROY(Computed_field_find_element_xi_special_cache)
				(&field->find_element_xi_cache);
		}
		if (field->node)
		{
			DEACCESS(FE_node)(&field->node);
		}
		if (field->computed_field_clear_cache_type_specific_function)
		{
			field->computed_field_clear_cache_type_specific_function(
				field);
		}
		field->derivatives_valid=0;
		for (i=0;i<field->number_of_source_fields;i++)
		{
			Computed_field_clear_cache(field->source_fields[i]);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_clear_cache.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_clear_cache */

int Computed_field_is_defined_in_element(struct Computed_field *field,
	struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 29 October 1999

DESCRIPTION :
Returns true if <field> can be calculated in <element>. If the field depends on
any other fields, this function is recursively called for them.
???RC.  Should also ask if derivatives defined for it.
#### Must ensure implemented correctly for new Computed_field_type. ####
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_is_defined_in_element);
	return_code=0;
	if (field&&element)
	{
		if (COMPUTED_FIELD_NEW_TYPES == field->type)
		{
			if (field->computed_field_evaluate_cache_in_element_function)
			{
				if (field->computed_field_is_defined_in_element_function)
				{
					return_code = 
						field->computed_field_is_defined_in_element_function(
							field, element);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_is_defined_in_element.  "
						"Is_defined_in_element_function for field %s is not defined.",
						field->name);
					return_code=0;
				}
			}
			else
			{
				return_code=0;
			}
		}
		else
		{
			return_code=1;
			for (i=0;(i<field->number_of_source_fields)&&return_code;i++)
			{
				if (!Computed_field_is_defined_in_element(field->source_fields[i],
					element))
				{
					return_code=0;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_defined_in_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_defined_in_element */

int Computed_field_default_is_defined_in_element(struct Computed_field *field,
	struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Returns 1 if the all the source fields are defined in the supplied <element>.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_default_is_defined_in_element);
	if (field && element)
	{
		return_code=1;
		for (i=0;(i<field->number_of_source_fields)&&return_code;i++)
		{
			if (!Computed_field_is_defined_in_element(field->source_fields[i],
				element))
			{
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_default_is_defined_in_element.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_default_is_defined_in_element */

int Computed_field_is_defined_in_element_conditional(
	struct Computed_field *field,void *element_void)
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Manager conditional function version of Computed_field_is_defined_in_element.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_defined_in_element_conditional);
	return_code=Computed_field_is_defined_in_element(field,
		(struct FE_element *)element_void);
	LEAVE;

	return (return_code);
} /* Computed_field_is_defined_in_element_conditional */

int Computed_field_is_defined_at_node(struct Computed_field *field,
	struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 29 October 1999

DESCRIPTION :
Returns true if <field> can be calculated at <node>. If the field depends on
any other fields, this function is recursively called for them.
#### Must ensure implemented correctly for new Computed_field_type. ####
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_is_defined_at_node);
	return_code=0;
	if (field&&node)
	{	
		if (COMPUTED_FIELD_NEW_TYPES == field->type)
		{
			if (field->computed_field_evaluate_cache_at_node_function)
			{
				if (field->computed_field_is_defined_at_node_function)
				{
					return_code = field->computed_field_is_defined_at_node_function(
						field, node);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_is_defined_at_node.  "
						"Is_defined_in_node_function for field %s is not defined.",
						field->name);
					return_code=0;
				}
			}
			else
			{
				return_code=0;
			}
		}
		else
		{
			switch (field->type)
			{
				case COMPUTED_FIELD_COMPONENT:
				case COMPUTED_FIELD_CONSTANT:
				case COMPUTED_FIELD_CUBIC_TEXTURE_COORDINATES:
				case COMPUTED_FIELD_EDIT_MASK:
				case COMPUTED_FIELD_EXTERNAL:
				case COMPUTED_FIELD_RC_COORDINATE:
				case COMPUTED_FIELD_RC_VECTOR:
				case COMPUTED_FIELD_SUM_COMPONENTS:
				{
					return_code=1;
					for (i=0;(i<field->number_of_source_fields)&&return_code;i++)
					{
						if (!Computed_field_is_defined_at_node(field->source_fields[i],node))
						{
							return_code=0;
						}
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_is_defined_at_node.  Unknown field type");
					return_code=0;
				} break;

			} /* switch */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_defined_at_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_defined_at_node */

int Computed_field_is_defined_at_node_conditional(struct Computed_field *field,
	void *node_void)
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
Computed_field conditional function version of
Computed_field_is_defined_at_node.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_defined_at_node_conditional);
	return_code=
		Computed_field_is_defined_at_node(field,(struct FE_node *)node_void);
	LEAVE;

	return (return_code);
} /* Computed_field_is_defined_at_node_conditional */

int Computed_field_default_is_defined_at_node(struct Computed_field *field,
	struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Returns 1 if all the source fields are defined at the supplied <node>.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_default_is_defined_at_node);
	if (field && node)
	{
		return_code=1;
		for (i=0;(i<field->number_of_source_fields)&&return_code;i++)
		{
			if (!Computed_field_is_defined_at_node(field->source_fields[i],
				node))
			{
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_default_is_defined_at_node.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_default_is_defined_at_node */

int FE_node_has_Computed_field_defined(struct FE_node *node,void *field_void)
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
FE_node conditional function version of Computed_field_is_defined_at_node.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_defined_at_node_conditional);
	return_code=
		Computed_field_is_defined_at_node((struct Computed_field *)field_void,node);
	LEAVE;

	return (return_code);
} /* Computed_field_is_defined_at_node_conditional */

int Computed_field_depends_on_Computed_field(struct Computed_field *field,
	struct Computed_field *other_field)
/*******************************************************************************
LAST MODIFIED : 26 January 1999

DESCRIPTION :
Returns true if the two fields are identical or one of the source_fields of
<field> is <other_field> or depends on it.

This function is used by MANAGER_COPY_WITHOUT_IDENTIFIER to ensure that the
destination field will not depend on itself, thus leading to an infinite loop,
and also to prevent copying a field over itself.

Parts of the program receiving manager messages for Computed_fields should call
this function with the field=field in use and other_field=modified field to
determine if the field in use needs updating.
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_depends_on_Computed_field);
	if (field&&other_field)
	{
		if (field==other_field)
		{
			return_code=1;
		}
		else
		{
			return_code=0;
			for (i=0;(i<field->number_of_source_fields)&&(!return_code);i++)
			{
				return_code=Computed_field_depends_on_Computed_field(
					field->source_fields[i],other_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_depends_on_Computed_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_depends_on_Computed_field */

static int Computed_field_is_source_field_conditional(
	struct Computed_field *field, void *other_field_void)
/*******************************************************************************
LAST MODIFIED : 23 May 2001

DESCRIPTION :
List conditional function returning true if <field> is a source field of
<other_field>, ie. <other_field> depends on <field>.
???RC Review Manager Messages Here
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_source_field_conditional);
	return_code = Computed_field_depends_on_Computed_field(
		(struct Computed_field *)other_field_void, field);
	LEAVE;

	return (return_code);
} /* Computed_field_is_source_field_conditional */

int Computed_field_depends_on_Computed_field_in_list(
	struct Computed_field *field, struct LIST(Computed_field) *field_list)
/*******************************************************************************
LAST MODIFIED : 1 June 2001

DESCRIPTION :
Returns true if <field> depends on any field in <field_list>.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_depends_on_Computed_field_in_list);
	if (field && field_list)
	{
		if (FIRST_OBJECT_IN_LIST_THAT(Computed_field)(
			Computed_field_is_source_field_conditional, (void *)field, field_list))
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_depends_on_Computed_field_in_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_depends_on_Computed_field_in_list */

static int Computed_field_evaluate_rc_vector(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 27 June 1999

DESCRIPTION :
Function called by Computed_field_evaluate_cache_in_element/at_node to compute
a vector in rectangular cartesian coordinates from one that may not be already.
The <field> must be of type COMPUTED_FIELD_RC_VECTOR, which gives it a source
vector field - describing 1, 2 or 3 vectors - and a coordinate field which says
where in space the vector is being converted. The function uses the jacobian
between the old coordinate system and the new RC one at the coordinate position
to get the new vector. Hence, derivatives of the converted vectors are not
available.
==============================================================================*/
{
	FE_value cx[3],jacobian[9],*source,sum,x[3];
	int coordinates_per_vector,i,j,k,number_of_vectors,return_code;
	struct Coordinate_system rc_coordinate_system;

	ENTER(Computed_field_evaluate_rc_vector);
	if (field&&(COMPUTED_FIELD_RC_VECTOR==field->type))
	{
		rc_coordinate_system.type = RECTANGULAR_CARTESIAN;
		/* extract coordinate position in RC coordinates, convert to coordinate
			 system of vector field, and then back to RC, getting the jacobian for
			 the final conversion */
		if (return_code=(Computed_field_extract_rc(field->source_fields[1],
			/*element_dimension*/0,x,/*dx_dxi*/(FE_value *)NULL)&&
			convert_Coordinate_system(&rc_coordinate_system,x,
				&(field->source_fields[0]->coordinate_system),cx,
				/*jacobian*/(float *)NULL)&&
			convert_Coordinate_system(&(field->source_fields[0]->coordinate_system),
				cx,&rc_coordinate_system,x,jacobian)))
		{
			number_of_vectors=field->number_of_components/3;
			coordinates_per_vector=
				field->source_fields[0]->number_of_components/number_of_vectors;
			source=field->source_fields[0]->values;
			for (i=0;i<number_of_vectors;i++)
			{
				for (j=0;j<3;j++)
				{
					sum=0.0;
					for (k=0;k<coordinates_per_vector;k++)
					{
						sum += jacobian[j*3+k]*source[i*coordinates_per_vector+k];
					}
					field->values[i*3+j]=sum;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_evaluate_rc_vector.  Could not convert to RC");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_rc_vector.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_evaluate_rc_vector */

int Computed_field_evaluate_cache_in_element(
	struct Computed_field *field,struct FE_element *element,FE_value *xi,
	struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 23 May 2000

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

???RC  May want to make this function non-static since there will be occasions
when the coordinate field is calculated without derivatives, then straight away
with derivatives for computing fibre axes/gradient etc. By first calling this
function with <calculate_derivatives> set, a recalculation of the field values
is avoided.
#### Must ensure implemented correctly for new Computed_field_type. ####
==============================================================================*/
{
	char buffer[100], *temp_string;
	FE_value sum,*temp,*temp2,compose_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int cache_is_valid,element_dimension,i,index,j,k,number_of_components,
		return_code,total_values;
	struct FE_element *compose_element;

	ENTER(Computed_field_evaluate_cache_in_element);
	if (field&&element)
	{
		element_dimension=get_FE_element_dimension(element);
		return_code=1;
		/* clear the cache if values already cached for a node */
		if (field->node)
		{
			Computed_field_clear_cache(field);
		}
		/* Are the values and derivatives in the cache not already calculated? */
		if ((field->element != element)||
			(calculate_derivatives && (!field->derivatives_valid)))
		{
			cache_is_valid=0;
		}
		else
		{
			cache_is_valid=1;
			for (i=0;cache_is_valid&&(i<element_dimension);i++)
			{
				if (field->xi[i] != xi[i])
				{
					cache_is_valid=0;
				}
			}
		}
		if (!cache_is_valid)
		{
			/* 3. Allocate values and derivative cache */
			if (return_code)
			{
				/* make sure we have allocated values AND derivatives, or nothing */
				if (!field->values)
				{
					/* get enough space for derivatives in highest dimension element */
					if (!(ALLOCATE(field->values,FE_value,field->number_of_components)&&
						ALLOCATE(field->derivatives,FE_value,
							MAXIMUM_ELEMENT_XI_DIMENSIONS*field->number_of_components)))
					{
						if (field->values)
						{
							DEALLOCATE(field->values);
						}
						return_code=0;
					}
				}
				field->derivatives_valid=0;
			}
			if (return_code)
			{
				if (field->type == COMPUTED_FIELD_NEW_TYPES)
				{
					if (field->computed_field_evaluate_cache_in_element_function)
					{
						return_code = 
							field->computed_field_evaluate_cache_in_element_function(
								field, element, xi, top_level_element,calculate_derivatives);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_evaluate_cache_in_element.  "
							"Function for calculating field %s in element not defined.",
							field->name);
						return_code=0;
					}
				}
				else
				{
					/* 2. Precalculate any source fields that this field depends on.  */
					if (return_code)
					{
						switch (field->type)
						{
							case COMPUTED_FIELD_COMPOSE:
							{
								/* only calculate the first source_field at this location */
								return_code=
									Computed_field_evaluate_cache_in_element(
										field->source_fields[0],element,xi,top_level_element,0);
							} break;
							default:
							{
								/* calculate values of source_fields, derivatives only if requested */
								for (i=0;(i<field->number_of_source_fields)&&return_code;i++)
								{
									return_code=
										Computed_field_evaluate_cache_in_element(
											field->source_fields[i],element,xi,top_level_element,
											calculate_derivatives);
								}
							} break;
						}
					}

					/* 3. Calculate the field */
					if (return_code)
					{
						/* set the flag indicating if derivatives are valid, assuming all values
							will be successfully calculated. Note that some types always get
							derivatives since they are so inexpensive to calculate. */
						field->derivatives_valid=calculate_derivatives;
						switch (field->type)
						{
							case COMPUTED_FIELD_COMPONENT:
							{
								field->values[0]=
									field->source_fields[0]->values[field->component_no];
								if (calculate_derivatives)
								{
									temp=field->source_fields[0]->derivatives+
										field->component_no*element_dimension;
									for (j=0;j<element_dimension;j++)
									{
										field->derivatives[j]=temp[j];
									}
								}
							} break;
							case COMPUTED_FIELD_COMPOSE:
							{
								/* The values from the first source field are inverted in the
									second source field to get element_xi which is evaluated with
									the third source field */
								if (return_code = Computed_field_find_element_xi(field->source_fields[1],
									field->source_fields[0]->values,
									field->source_fields[0]->number_of_components,
									&compose_element, compose_xi, field->compose_element_group))
								{
									/* calculate the third source_field at this new location */
									return_code=
										Computed_field_evaluate_cache_in_element(
											field->source_fields[2],compose_element,compose_xi,
											/*top_level*/(struct FE_element *)NULL,calculate_derivatives);
									for (i=0;i<field->number_of_components;i++)
									{
										field->values[i]=field->source_fields[2]->values[i];
									}
									if (calculate_derivatives)
									{
										temp=field->derivatives;
										temp2=field->source_fields[2]->derivatives;
										for (i=0;i<field->number_of_components;i++)
										{
											for (j=0;j<element_dimension;j++)
											{
												(*temp)=(*temp2);
												temp++;
												temp2++;
											}
										}
									}
								}
							} break;
							case COMPUTED_FIELD_CONSTANT:
							{
								/* returns constant vector values, zero derivatives (always) */
								temp=field->derivatives;
								for (i=0;i<field->number_of_components;i++)
								{
									field->values[i]=field->source_values[i];
									for (j=0;j<element_dimension;j++)
									{
										*temp=0.0;
										temp++;
									}
								}
								field->derivatives_valid=1;
							} break;
							case COMPUTED_FIELD_CUBIC_TEXTURE_COORDINATES:
							{
								number_of_components = field->number_of_components;
								temp=field->source_fields[0]->values;
								field->values[number_of_components - 1] = fabs(*temp);
								temp++;
								j = 0;
								for (i=1;i<number_of_components;i++)
								{
									if (fabs(*temp) > field->values[number_of_components - 1])
									{
										field->values[number_of_components - 1] = fabs(*temp);
										j = i;
									}
									temp++;
								}
								temp=field->source_fields[0]->values;
								for (i=0;i < number_of_components - 1;i++)
								{
									if ( i == j )
									{
										/* Skip over the maximum coordinate */
										temp++;
									}
									field->values[i] = *temp / field->values[number_of_components - 1];
									temp++;
								}
								field->derivatives_valid = 0;
							} break;
							case COMPUTED_FIELD_RC_COORDINATE:
							{
								return_code=Computed_field_evaluate_rc_coordinate(field,
									element_dimension,calculate_derivatives);
							} break;
							case COMPUTED_FIELD_EDIT_MASK:
							{
								for (i=0;i<field->number_of_components;i++)
								{
									field->values[i]=field->source_fields[0]->values[i];
								}
								if (calculate_derivatives)
								{
									temp=field->derivatives;
									temp2=field->source_fields[0]->derivatives;
									for (i=0;i<field->number_of_components;i++)
									{
										for (j=0;j<element_dimension;j++)
										{
											(*temp)=(*temp2);
											temp++;
											temp2++;
										}
									}
								}
							} break;
							case COMPUTED_FIELD_EXTERNAL:
							{
								total_values = field->number_of_source_values * 12 + 2;
								for (i = 0 ; i < field->number_of_source_fields ; i++)
								{
									total_values+=field->source_fields[i]->number_of_components * 12;
									if (calculate_derivatives /* and not never || always give derivatives */)
									{
										total_values+=field->source_fields[i]->number_of_components *
											(element_dimension * 12 + 20);
									}
								}
								if (ALLOCATE(temp_string, char, total_values))
								{
									index = 0;
									for (i = 0 ; i < field->number_of_source_values ; i++)
									{
										sprintf(temp_string + index, "%10.5e ", field->source_values[i]);
										index += 12;
									}
									for (i = 0 ; i < field->number_of_source_fields ; i++)
									{
										for (j = 0 ; j < field->source_fields[i]->number_of_components ; j++)
										{
											sprintf(temp_string + index, "%10.5e ", field->source_fields[i]->values[j]);
											index += 12;
											if (calculate_derivatives /* and not never || always give derivatives */)
											{
												sprintf(temp_string + index, "#deriv=%d %n", element_dimension,
													&k);
												index += k;
												for (k = 0 ; k < element_dimension ; k++)
												{
													sprintf(temp_string + index, "%10.5e ",
														field->source_fields[i]->derivatives[j * element_dimension + k]);
													index += 12;
												}
											}
										}
									}
									sprintf(temp_string + index, "\n");
									Child_process_send_string_to_stdin(field->child_process,temp_string);
									DEALLOCATE(temp_string);
									if (temp_string = Child_process_get_line_from_stdout(field->child_process,
										field->timeout))
									{
										index = 0;
										for (i = 0 ; i < field->number_of_components ; i++)
										{
											sscanf(temp_string + index, "%f%n", &(field->values[i]), &j);
											index += j;
											sscanf(temp_string + index, "%s%n", buffer, &j);
											if (!strncmp(buffer, "#deriv=", 7))
											{
												index += j;
												/* Should check to see derivatives are supplied for
													all or none of the components and set the derivatives
													valid flag appropriately */
												if (sscanf(buffer + 7, "%d", &k) && k == element_dimension)
												{											
													for (k = 0 ; k < element_dimension ; k++)
													{
														sscanf(temp_string + index, "%f%n",
															&(field->derivatives[i * element_dimension + k]), &j);
														index += j;
													}
												}
											}
										}
										DEALLOCATE(temp_string);
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"Computed_field_evaluate_cache_in_element."
											"  Invalid response from child process");
										return_code = 0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Computed_field_evaluate_cache_in_element."
										"  Unable to allocate temporary string");
									return_code = 0;
								}
							} break;
							case COMPUTED_FIELD_RC_VECTOR:
							{
								return_code=Computed_field_evaluate_rc_vector(field);
								/* no derivatives for this type */
								field->derivatives_valid=0;
							} break;
							case COMPUTED_FIELD_SUM_COMPONENTS:
							{
								/* weighted sum of components */
								temp=field->source_fields[0]->values;
								sum=0.0;
								for (i=0;i<field->source_fields[0]->number_of_components;i++)
								{
									sum += temp[i]*field->source_values[i];
								}
								field->values[0]=sum;
								if (calculate_derivatives)
								{
									for (j=0;j<element_dimension;j++)
									{
										temp=field->source_fields[0]->derivatives + j;
										sum=0.0;
										for (i=0;i<field->source_fields[0]->number_of_components;i++)
										{
											sum += (*temp)*field->source_values[i];
											temp += element_dimension;
										}
										field->derivatives[j]=sum;
									}
								}
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"Computed_field_evaluate_cache_in_element.  Unknown field type");
								return_code=0;
							} break;
						}
					}
				}
				if (return_code&&calculate_derivatives&&!(field->derivatives_valid))
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_evaluate_cache_in_element.  "
						"Derivatives unavailable for field %s of type %s",field->name,
						Computed_field_type_to_string(field));
					return_code=0;
				}
				if (return_code)
				{
					if (field->element)
					{
						DEACCESS(FE_element)(&field->element);
					}
					field->element=ACCESS(FE_element)(element);
					for (i=0;i<element_dimension;i++)
					{
						field->xi[i]=xi[i];
					}
					for (;i<MAXIMUM_ELEMENT_XI_DIMENSIONS;i++)
					{
						field->xi[i]=0.0;
					}
				}
				else
				{
					/* make sure value cache is marked as invalid */
					if (field->element)
					{
						DEACCESS(FE_element)(&field->element);
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_cache_in_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_evaluate_cache_in_element */

int Computed_field_get_top_level_element_and_xi(struct FE_element *element, 
	FE_value *xi, int element_dimension, struct FE_element **top_level_element,
	FE_value *top_level_xi, int *top_level_element_dimension)
/*******************************************************************************
LAST MODIFIED : 30 October 2000

DESCRIPTION :
Finds the <top_level_element>, <top_level_xi> and <top_level_element_dimension>
for the given <element> and <xi>.  If <top_level_element> is already set it 
is checked and the <top_level_xi> calculated.
==============================================================================*/
{
	FE_value element_to_top_level[9];
	int i,j,k,return_code;

	ENTER(Computed_field_get_top_level_element_and_xi);
	if (element&&xi&&top_level_element&&top_level_xi&&top_level_element_dimension)
	{
		return_code = 1;
		if (CM_ELEMENT == element->cm.type)
		{
			*top_level_element = element;
			for (i=0;i<element_dimension;i++)
			{
				top_level_xi[i]=xi[i];
			}
			/* do not set element_to_top_level */
			*top_level_element_dimension=element_dimension;
		}
		else
		{
			/* check or get top_level element and xi coordinates for it */
			if (*top_level_element = FE_element_get_top_level_element_conversion(
				element,*top_level_element,(struct GROUP(FE_element) *)NULL,
				-1,element_to_top_level))
			{
				/* convert xi to top_level_xi */
				*top_level_element_dimension = (*top_level_element)->shape->dimension;
				for (j=0;j<*top_level_element_dimension;j++)
				{
					top_level_xi[j] = element_to_top_level[j*(element_dimension+1)];
					for (k=0;k<element_dimension;k++)
					{
						top_level_xi[j] +=
							element_to_top_level[j*(element_dimension+1)+k+1]*xi[k];
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_get_top_level_element_and_xi.  "
					"No top-level element found to evaluate on");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_top_level_element_and_xi.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_top_level_element_and_xi */

int Computed_field_evaluate_source_fields_cache_in_element(
	struct Computed_field *field,struct FE_element *element,FE_value *xi,
	struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Calculates the cache values of each source field in <field> in <element>, if it 
is defined over the element.
Upon successful return the element values of the source fields are stored in their
cache.
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_evaluate_source_fields_cache_in_element);
	if (field&&element&&xi)
	{
		return_code = 1;
		/* calculate values of source_fields, derivatives only if requested */
		for (i=0;(i<field->number_of_source_fields)&&return_code;i++)
		{
			return_code=
				Computed_field_evaluate_cache_in_element(
					field->source_fields[i],element,xi,top_level_element,
					calculate_derivatives);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_source_fields_cache_in_element.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_evaluate_source_fields_cache_in_element */

char *Computed_field_evaluate_as_string_in_element(
	struct Computed_field *field,int component_number,
	struct FE_element *element,FE_value *xi,struct FE_element *top_level_element)
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
Returns a string representing the value of <field>.<component_number> at
<element>:<xi>. Calls Computed_field_evaluate_cache_in_element and
converts the value for <component_number> to a string (since result 
may already be in cache).

Use -1 as the <component_number> if you want all the components.

The <top_level_element> parameter has the same use as in
Computed_field_evaluate_cache_in_element.

Some basic field types such as CMISS_NUMBER have special uses in this function.
It is up to the calling function to DEALLOCATE the returned string.
???RC.  Allow derivatives to be evaluated as string too?
==============================================================================*/
{
	char *return_string,tmp_string[50];
	int error,i;

	ENTER(Computed_field_evaluate_as_string_in_element);
	return_string=(char *)NULL;
	if (field&&element&&xi&&(-1<=component_number)&&
		(component_number < field->number_of_components))
	{
		if (COMPUTED_FIELD_NEW_TYPES==field->type)
		{
			if (field->computed_field_evaluate_as_string_in_element_function)
			{
				return_string = 
					field->computed_field_evaluate_as_string_in_element_function(
					field, component_number, element, xi, top_level_element);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_evaluate_as_string_in_element.  "
					"No function defined.");
			}
		}
		else
		{
			/* write the component value in %g format */
			if (Computed_field_evaluate_cache_in_element(field,element,xi,
				top_level_element,/*calculate_derivatives*/0))
			{
				if (-1 == component_number)
				{
					error=0;
					for (i=0;i<field->number_of_components;i++)
					{
						if (0<i)
						{
							sprintf(tmp_string,", %g",field->values[i]);
						}
						else
						{
							sprintf(tmp_string,"%g",field->values[i]);
						}
						append_string(&return_string,tmp_string,&error);
					}
				}
				else
				{
					sprintf(tmp_string,"%g",field->values[component_number]);
					return_string=duplicate_string(tmp_string);
				}
			}
		}
		if (!return_string)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_evaluate_as_string_in_element.  Failed");
			/* clear the cache since values may be half set */
			Computed_field_clear_cache(field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_as_string_in_element.  "
			"Invalid argument(s)");
	}
	LEAVE;

	return (return_string);
} /* Computed_field_evaluate_as_string_in_element */

char *Computed_field_default_evaluate_as_string_in_element(
	struct Computed_field *field,int component_number,
	struct FE_element *element,FE_value *xi,struct FE_element *top_level_element)
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
Returns a string representing the value of <field>.<component_number> at
<element>:<xi>. Calls Computed_field_evaluate_cache_in_element and converts 
the value for <component_number> to a string (since result may already be in 
cache).

Use -1 as the <component_number> if you want all the components.

The <top_level_element> parameter has the same use as in
Computed_field_evaluate_cache_in_element.
==============================================================================*/
{
	char *return_string,tmp_string[50];
	int error,i;

	ENTER(Computed_field_default_evaluate_as_string_in_element);
	return_string=(char *)NULL;
	if (field&&element&&xi&&(-1<=component_number)&&
		(component_number < field->number_of_components))
	{
		error = 0;
		/* write the component value in %g format */
		if (Computed_field_evaluate_cache_in_element(field,element,xi,
			top_level_element,/*calculate_derivatives*/0))
		{
			if (-1 == component_number)
			{
				for (i=0;i<field->number_of_components;i++)
				{
					if (0<i)
					{
						sprintf(tmp_string,", %g",field->values[i]);
					}
					else
					{
						sprintf(tmp_string,"%g",field->values[i]);
					}
					append_string(&return_string,tmp_string,&error);
				}
			}
			else
			{
				sprintf(tmp_string,"%g",field->values[component_number]);
				return_string=duplicate_string(tmp_string);
			}
		}
		if (!return_string)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_default_evaluate_as_string_in_element.  Failed");
			/* clear the cache since values may be half set */
			Computed_field_clear_cache(field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_default_evaluate_as_string_in_element.  "
			"Invalid argument(s)");
	}
	LEAVE;

	return (return_string);
} /* Computed_field_default_evaluate_as_string_in_element */

int Computed_field_evaluate_in_element(struct Computed_field *field,
	struct FE_element *element,FE_value *xi,struct FE_element *top_level_element,
	FE_value *values,FE_value *derivatives)
/*******************************************************************************
LAST MODIFIED : 22 July 1999

DESCRIPTION :
Returns the values and derivatives (if <derivatives> != NULL) of <field> at
<element>:<xi>, if it is defined over the element. Can verify this in advance
by calling function Computed_field_defined_in_element. Each <field> has a cache
for storing its values and derivatives, which is allocated and filled by a call
to Computed_field_evaluate_cache_in_element, then copied to the <values> and
<derivatives> arrays.

The optional <top_level_element> may be supplied for the benefit of this or
any source fields that may require calculation on it instead of a face or line.
FIBRE_AXES and GRADIENT are examples of such fields, since they require
top-level coordinate derivatives. The term "top_level" refers to an ultimate
parent element for the face or line, eg. the 3-D element parent to 2-D faces.
If no such top level element is supplied and one is required, then the first
available parent element will be chosen - if the user requires a top-level
element in the same group as the face or with the face on the correct side,
then they should supply the top_level_element here.

The <values> and <derivatives> arrays must be large enough to store all the
values and derivatives for this field in the given element, ie. values is
number_of_components in size, derivatives has the element dimension times the
number_of_components
==============================================================================*/
{
	FE_value *destination,*source;
	int i,return_code;

	ENTER(Computed_field_evaluate_in_element);
	if (field&&element&&xi&&values)
	{
		if (return_code=Computed_field_evaluate_cache_in_element(field,element,xi,
			top_level_element,((FE_value *)NULL != derivatives)))
		{
			/* copy values from cache to <values> and <derivatives> */
			source=field->values;
			destination=values;
			for (i=field->number_of_components;0<i;i--)
			{
				*destination = *source;
				source++;
				destination++;
			}
			if (derivatives)
			{
				source=field->derivatives;
				destination=derivatives;
				for (i=field->number_of_components*element->shape->dimension;0<i;i--)
				{
					*destination = *source;
					source++;
					destination++;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_evaluate_in_element.  Failed for %s",field->name);
			/* clear the cache since values may be half set */
			Computed_field_clear_cache(field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_in_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_evaluate_in_element */

static int Computed_field_evaluate_cache_at_node(
	struct Computed_field *field,struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 21 May 2001

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
#### Must ensure implemented correctly for new Computed_field_type. ####
==============================================================================*/
{
	char *temp_string;
	FE_value sum,*temp;
	int i,j,k,number_of_components,return_code,total_values;
	/* For COMPUTED_FIELD_EMBEDDED and COMPUTED_FIELD_COMPOSE only */
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct FE_element *element;	

	ENTER(Computed_field_evaluate_cache_at_node);
	if (field&&node)
	{
		return_code=1;
		/* clear the cache if values already cached for an element */
		if (field->element)
		{
			Computed_field_clear_cache(field);
		}
		/* does cache need recomputing? */
		if (node != field->node)
		{
			/* make sure we have allocated values AND derivatives, or nothing */
			if (!field->values)
			{
				/* get enough space for derivatives in highest dimension element */
				if (!(ALLOCATE(field->values,FE_value,field->number_of_components)&&
					ALLOCATE(field->derivatives,FE_value,
						MAXIMUM_ELEMENT_XI_DIMENSIONS*field->number_of_components)))
				{
					if (field->values)
					{
						DEALLOCATE(field->values);
					}
					return_code=0;
				}
			}
			if (return_code)
			{
				if (COMPUTED_FIELD_NEW_TYPES == field->type)
				{
					if (field->computed_field_evaluate_cache_at_node_function)
					{
						return_code = 
							field->computed_field_evaluate_cache_at_node_function(
								field, node);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_evaluate_cache_at_node.  "
							"No function defined.");
						return_code = 0;
					}
				}
				else
				{
					/* 1. Precalculate any source fields that this field depends on */
					switch(field->type)
					{
						case COMPUTED_FIELD_COMPOSE:
						{
							/* only calculate the first source_field at this location */
							return_code=
								Computed_field_evaluate_cache_at_node(
									field->source_fields[0],node);
						} break;
						default:
						{
							/* calculate values of source_fields */
							for (i=0;(i<field->number_of_source_fields)&&return_code;i++)
							{
								return_code=Computed_field_evaluate_cache_at_node(
									field->source_fields[i],node);
							}
						} break;
					}

					/* 2. Calculate the field */
					if (return_code)
					{
						switch (field->type)
						{
							case COMPUTED_FIELD_COMPONENT:
							{
								field->values[0]=
									field->source_fields[0]->values[field->component_no];
							} break;
							case COMPUTED_FIELD_COMPOSE:
							{
								/* The values from the first source field are inverted in the
									second source field to get element_xi which is evaluated with
									the third source field */
								if (return_code = Computed_field_find_element_xi(field->source_fields[1],
									field->source_fields[0]->values,
									field->source_fields[0]->number_of_components, &element, xi,
									field->compose_element_group))
								{
									/* calculate the third source_field at this new location */
									return_code=
										Computed_field_evaluate_cache_in_element(
											field->source_fields[2],element,xi,
											/*top_level*/(struct FE_element *)NULL,0);
									for (i=0;i<field->number_of_components;i++)
									{
										field->values[i]=field->source_fields[2]->values[i];
									}
								}
							} break;
							case COMPUTED_FIELD_CONSTANT:
							{
								/* returns constant vector values, zero derivatives (always) */
								for (i=0;i<field->number_of_components;i++)
								{
									field->values[i]=field->source_values[i];
								}
							} break;
							case COMPUTED_FIELD_CUBIC_TEXTURE_COORDINATES:
							{
								number_of_components = field->number_of_components;
								temp=field->source_fields[0]->values;
								field->values[number_of_components - 1] = fabs(*temp);
								temp++;
								j = 0;
								for (i=1;i <number_of_components;i++)
								{
									if (fabs(*temp) > field->values[number_of_components - 1])
									{
										field->values[number_of_components - 1] = fabs(*temp);
										j = i;
									}
									temp++;
								}
								temp=field->source_fields[0]->values;
								for (i=0;i < number_of_components - 1;i++)
								{
									if ( i == j )
									{
										/* Skip over the maximum coordinate */
										temp++;
									}
									field->values[i] = *temp / field->values[number_of_components - 1];
									temp++;
								}
							} break;
							case COMPUTED_FIELD_RC_COORDINATE:
							{
								return_code=Computed_field_evaluate_rc_coordinate(field,
									/*element_dimension*/0,/*calculate_derivatives*/0);
							} break;
							case COMPUTED_FIELD_EDIT_MASK:
							{
								for (i=0;i<field->number_of_components;i++)
								{
									field->values[i]=field->source_fields[0]->values[i];
								}
							} break;
							case COMPUTED_FIELD_EXTERNAL:
							{
								total_values = field->number_of_source_values;
								for (i = 0 ; i < field->number_of_source_fields ; i++)
								{
									total_values+=field->source_fields[i]->number_of_components;
								}
								if (ALLOCATE(temp_string, char, total_values * 12 + 2))
								{
									k = 0;
									for (i = 0 ; i < field->number_of_source_values ; i++)
									{
										sprintf(temp_string + k, "%10.5e ", field->source_values[i]);
										k += 12;
									}
									for (i = 0 ; i < field->number_of_source_fields ; i++)
									{
										for (j = 0 ; j < field->source_fields[i]->number_of_components ; j++)
										{
											sprintf(temp_string + k, "%10.5e ", field->source_fields[i]->values[j]);
											k += 12;
										}
									}
									sprintf(temp_string + k, "\n");
									Child_process_send_string_to_stdin(field->child_process,temp_string);
									DEALLOCATE(temp_string);
									if (temp_string = Child_process_get_line_from_stdout(field->child_process,
										field->timeout))
									{
										k = 0;
										for (i = 0 ; i < field->number_of_components ; i++)
										{
											sscanf(temp_string + k, "%f%n", &(field->values[i]), &j);
											k += j;
										}
										DEALLOCATE(temp_string);
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"Computed_field_evaluate_cache_at_node."
											"  Invalid response from child process");
										return_code = 0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Computed_field_evaluate_cache_at_node."
										"  Unable to allocate temporary string");
									return_code = 0;
								}
							} break;
							case COMPUTED_FIELD_RC_VECTOR:
							{
								return_code=Computed_field_evaluate_rc_vector(field);
							} break;
							case COMPUTED_FIELD_SUM_COMPONENTS:
							{
								/* weighted sum of components */
								temp=field->source_fields[0]->values;
								sum=0.0;
								for (i=0;i<field->source_fields[0]->number_of_components;i++)
								{
									sum += temp[i]*field->source_values[i];
								}
								field->values[0]=sum;
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"Computed_field_evaluate_cache_at_node.  Unknown field type");
								return_code=0;
							} break;
						}
					}
				}

				/* Store information about what is cached, or clear it if error */
				if (return_code)
				{
					if (field->node)
					{
						DEACCESS(FE_node)(&field->node);
					}
					field->node=ACCESS(FE_node)(node);
				}
				else
				{
					/* make sure value cache is marked as invalid */
					if (field->node)
					{
						DEACCESS(FE_node)(&field->node);
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_cache_at_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_evaluate_cache_at_node */

int Computed_field_evaluate_source_fields_cache_at_node(
	struct Computed_field *field,struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Calculates the cache values of each source field in <field> at <node>, if it 
is defined.
Upon successful return the node values of the source fields are stored in their
cache.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_evaluate_source_fields_cache_at_node);
	if (field&&node)
	{
		return_code = 1;
		/* calculate values of source_fields */
		for (i=0;(i<field->number_of_source_fields)&&return_code;i++)
		{
			return_code=Computed_field_evaluate_cache_at_node(
				field->source_fields[i],node);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_source_fields_cache_at_node.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_evaluate_source_fields_cache_at_node */

char *Computed_field_evaluate_as_string_at_node(struct Computed_field *field,
	int component_number, struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
Returns a string describing the value/s of the <field> at the <node>. If the
field is based on an FE_field but not returning FE_values, it is asked to supply
the string. Otherwise, a string built up of comma separated values evaluated
for the field in Computed_field_evaluate_cache_at_node. The FE_value exception
is used since it is likely the values are already in the cache in most cases,
or can be used by other fields again if calculated now.
The <component_number> indicates which component to calculate.  Use -1 to 
create a string which represents all the components.
Some basic field types such as CMISS_NUMBER have special uses in this function.
It is up to the calling function to DEALLOCATE the returned string.
==============================================================================*/
{
	char *return_string,tmp_string[50];
	int error,i;

	ENTER(Computed_field_evaluate_as_string_at_node);
	return_string=(char *)NULL;
	if (field&&node&&(component_number >= -1)&&
		(component_number<field->number_of_components))
	{
		if (COMPUTED_FIELD_NEW_TYPES==field->type)
		{
			if (field->computed_field_evaluate_as_string_at_node_function)
			{
				return_string = 
					field->computed_field_evaluate_as_string_at_node_function(
					field, component_number, node);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_evaluate_as_string_at_node.  "
					"No function defined.");
			}
		}
		else
		{
			error=0;
			/* write the component values in %g format, comma separated */
			if (Computed_field_evaluate_cache_at_node(field,node))
			{
				if (-1 == component_number)
				{
					for (i=0;i<field->number_of_components;i++)
					{
						if (0<i)
						{
							sprintf(tmp_string,", %g",field->values[i]);
						}
						else
						{
							sprintf(tmp_string,"%g",field->values[i]);
						}
						append_string(&return_string,tmp_string,&error);
					}
				}
				else
				{
					sprintf(tmp_string,"%g",field->values[component_number]);
					append_string(&return_string,tmp_string,&error);
				}
			}
			else
			{
				error=1;
			}
			if (!return_string)
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_evaluate_as_string_at_node.  Failed");
			}
		}
		if (!return_string)
		{
			/* clear the cache since values may be half set */
			Computed_field_clear_cache(field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_as_string_at_node.  Invalid argument(s)");
	}
	LEAVE;

	return (return_string);
} /* Computed_field_evaluate_as_string_at_node */

char *Computed_field_default_evaluate_as_string_at_node(
	struct Computed_field *field, int component_number, struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 7 June 2001

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
{
	char *return_string,tmp_string[50];
	int error,i;

	ENTER(Computed_field_evaluate_as_string_at_node);
	return_string=(char *)NULL;
	if (field&&node&&(component_number >= -1)&&
		(component_number<field->number_of_components))
	{
		error = 0;
		/* write the component values in %g format, comma separated */
		if (Computed_field_evaluate_cache_at_node(field,node))
		{
			if (-1 == component_number)
			{
				for (i=0;i<field->number_of_components;i++)
				{
					if (0<i)
					{
						sprintf(tmp_string,", %g",field->values[i]);
					}
					else
					{
						sprintf(tmp_string,"%g",field->values[i]);
					}
					append_string(&return_string,tmp_string,&error);
				}
			}
			else
			{
				sprintf(tmp_string,"%g",field->values[component_number]);
				return_string=duplicate_string(tmp_string);
			}
		}
		if (!return_string)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_default_evaluate_as_string_at_node.  Failed");
			/* clear the cache since values may be half set */
			Computed_field_clear_cache(field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_default_evaluate_as_string_at_node.  "
			"Invalid argument(s)");
	}
	LEAVE;

	return (return_string);
} /* Computed_field_default_evaluate_as_string_at_node */

int Computed_field_evaluate_at_node(struct Computed_field *field,
	struct FE_node *node,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 22 July 1999

DESCRIPTION :
Returns the <values> of <field> at <node> if it is defined there. Can verify
this in advance by calling function Computed_field_defined_at_node. Each <field>
has a cache for storing its values and derivatives, which is allocated and the
field->values array filled by a call to Computed_field_evaluate_cache_at_node,
which is then copied to <values> by this function. Derivatives may only be
calculated in elements, however, the field->derivatives array is allocated here
with field->values since Computed_field_evaluate_cache_in_element expects both
to be allocated together.

The <values> array must be large enough to store as many FE_values as there are
number_of_components.
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_evaluate_at_node);
	if (field&&node&&values)
	{
		if (return_code=Computed_field_evaluate_cache_at_node(field,node))
		{
			/* copy values from cache to <values> */
			for (i=0;i<field->number_of_components;i++)
			{
				values[i]=field->values[i];
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_evaluate_at_node.  Failed");
			/* clear the cache since values may be half set */
			Computed_field_clear_cache(field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_at_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_evaluate_at_node */

int Computed_field_set_values_at_node(struct Computed_field *field,
	struct FE_node *node,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 5 October 2000

DESCRIPTION :
Sets the <values> of the computed <field> at <node>. Only certain computed field
types allow their values to be set. Fields that deal directly with FE_fields eg.
FINITE_ELEMENT and NODE_VALUE fall into this category, as do the various
transformations, RC_COORDINATE, RC_VECTOR, OFFSET, SCALE, etc. which convert
the values into what they expect from their source field, and then call the same
function for it. If a field has more than one source field, eg. RC_VECTOR, it
can in many cases still choose which one is actually being changed, for example,
the 'vector' field in this case - coordinates should not change. This process
continues until the actual FE_field values at the node are changed or a field
is reached for which its calculation is not reversible, or is not supported yet.
Note that you must only call this function for nodes that are not managed as it
will change values inside them. Also, this function does not clear the cache at
any time, so up to the calling function to do so.
Note that the values array will not be modified by this function. Also, <node>
should not be managed at the time it is modified by this function.
#### Must ensure implemented correctly for new Computed_field_type. ####
???RC Note that some functions are not reversible in this way.
==============================================================================*/
{
	FE_value *source_values;
	int i,j,k,return_code;

	ENTER(Computed_field_set_values_at_node);
	if (field&&node&&values)
	{
		if (COMPUTED_FIELD_NEW_TYPES == field->type)
		{
			if (field->computed_field_set_values_at_node_function)
			{
				return_code = 
					field->computed_field_set_values_at_node_function(
					field, node, values);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_values_at_node.  "
					"No function defined.");
				return_code = 0;
			}
		}
		else
		{
			return_code=1;
			switch (field->type)
			{
				case COMPUTED_FIELD_COMPONENT:
				{
					/* need current field values to partially set */
					if (ALLOCATE(source_values,FE_value,
						field->source_fields[0]->number_of_components))
					{
						if (Computed_field_evaluate_at_node(field->source_fields[0],node,
							source_values))
						{
							/* set value of the component leaving other components intact */
							source_values[field->component_no] = values[0];
							return_code=Computed_field_set_values_at_node(
								field->source_fields[0],node,source_values);
						}
						else
						{
							return_code=0;
						}
						DEALLOCATE(source_values);
					}
					else
					{
						return_code=0;
					}
				} break;
				case COMPUTED_FIELD_EDIT_MASK:
				{
					/* need current field values to partially set */
					if (ALLOCATE(source_values,FE_value,
						field->source_fields[0]->number_of_components))
					{
						if (Computed_field_evaluate_at_node(field->source_fields[0],node,
							source_values))
						{
							/* set value of the component leaving other components intact */
							for (i=0;i<field->number_of_components;i++)
							{
								if (field->source_values[i])
								{
									source_values[i] = values[i];
								}
							}
							return_code=Computed_field_set_values_at_node(
								field->source_fields[0],node,source_values);
						}
						else
						{
							return_code=0;
						}
						DEALLOCATE(source_values);
					}
					else
					{
						return_code=0;
					}
				} break;
				case COMPUTED_FIELD_RC_COORDINATE:
				{
					FE_value non_rc_coordinates[3];
					struct Coordinate_system rc_coordinate_system;

					/* convert RC values back into source coordinate system */
					rc_coordinate_system.type = RECTANGULAR_CARTESIAN;
					return_code=
						convert_Coordinate_system(&rc_coordinate_system,values,
							&(field->source_fields[0]->coordinate_system),non_rc_coordinates,
							/*jacobian*/(float *)NULL)&&
						Computed_field_set_values_at_node(field->source_fields[0],
							node,non_rc_coordinates);
				} break;
				case COMPUTED_FIELD_RC_VECTOR:
				{
					FE_value jacobian[9],non_rc_coordinates[3],rc_coordinates[3],sum;
					int coordinates_per_vector,number_of_vectors;
					struct Coordinate_system rc_coordinate_system;

					rc_coordinate_system.type = RECTANGULAR_CARTESIAN;
					/* need jacobian at current coordinate position for converting to
						coordinate system of source vector field (=source_fields[0]) */
					if (Computed_field_evaluate_cache_at_node(field->source_fields[1],node)
						&&Computed_field_extract_rc(field->source_fields[1],
							/*element_dimension*/0,rc_coordinates,/*dx_dxi*/(FE_value *)NULL)&&
						convert_Coordinate_system(&rc_coordinate_system,rc_coordinates,
							&(field->source_fields[0]->coordinate_system),non_rc_coordinates,
							jacobian))
					{
						if (ALLOCATE(source_values,FE_value,field->number_of_components))
						{
							number_of_vectors=field->number_of_components/3;
							coordinates_per_vector=
								field->source_fields[0]->number_of_components/number_of_vectors;
							for (i=0;i<number_of_vectors;i++)
							{
								for (j=0;j<coordinates_per_vector;j++)
								{
									sum=0.0;
									for (k=0;k<3;k++)
									{
										sum += jacobian[j*3+k]*values[i*3+k];
									}
									source_values[i*coordinates_per_vector+j]=sum;
								}
							}
							return_code=Computed_field_set_values_at_node(
								field->source_fields[0],node,source_values);
							DEALLOCATE(source_values);
						}
						else
						{
							return_code=0;
						}
					}
					else
					{
						return_code=0;
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_set_values_at_node.  "
						"Cannot set values for field %s of type %s",field->name,
						Computed_field_type_to_string(field));
					return_code=0;
				} break;
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_values_at_node.  "
					"Failed for field %s of type %s",field->name,
					Computed_field_type_to_string(field));
			}
		}
		Computed_field_clear_cache(field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_values_at_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_values_at_node */

int Computed_field_set_values_at_managed_node(struct Computed_field *field,
	struct FE_node *node,FE_value *values,struct MANAGER(FE_node) *node_manager)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Sets the <values> of the computed <field> at <node>. Only certain computed field
types allow their values to be set. Fields that deal directly with FE_fields eg.
FINITE_ELEMENT and NODE_VALUE fall into this category, as do the various
transformations, RC_COORDINATE, RC_VECTOR, OFFSET, SCALE, etc. which convert
the values into what they expect from their source field, and then call the same
function for it. If a field has more than one source field, eg. RC_VECTOR, it
can in many cases still choose which one is actually being changed, for example,
the 'vector' field in this case - coordinates should not change. This process
continues until the actual FE_field values at the node are changed or a field
is reached for which its calculation is not reversible, or is not supported yet.

This function works by making a copy of the node, then performing all
modifications to it. If these are successful then MANAGER_MODIFY_NOT_IDENTIFIER
is called to effect the change on the real <node>, thus sending manager messages
to the rest of the program. Because all changes are made on a temporary copy of
the node, all current cache values will be ignored. For safety, however, the
cache is always cleared after calling.
It is up to calling function to begin/end node manager cache if more than one
node is being modified.
Note that the values array will not be modified by this function.
==============================================================================*/
{
	int return_code;
	struct FE_node *copy_node;

	ENTER(Computed_field_set_values_at_managed_node);
	return_code=0;
	if (field&&node&&values&&node_manager)
	{
		/* CREATE with template node is assumed to copy its values */
		if (copy_node=CREATE(FE_node)(0,node))
		{
			/* The node must be accessed as the use of cache on the nodes
				by get values etc. access and deaccessess the nodes */
			ACCESS(FE_node)(copy_node);
			return_code=
				Computed_field_set_values_at_node(field,copy_node,values);
			/* must clear the cache before MANAGER_MODIFY as previously cached
				 values may be invalid */
			Computed_field_clear_cache(field);
			if (return_code)
			{
				return_code=MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)(
					node,copy_node,node_manager);
			}
			DEACCESS(FE_node)(&copy_node);
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_values_at_managed_node.  Failed");
			Computed_field_clear_cache(field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_values_at_managed_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_values_at_managed_node */

int Computed_field_get_values_in_element(struct Computed_field *field,
	struct FE_element *element,int *number_in_xi,FE_value **values)
/*******************************************************************************
LAST MODIFIED : 28 October 1999

DESCRIPTION :
Companion function to Computed_field_set_values_in_element.
Returns the <field> calculated at the corners of the <number_in_xi> cells,
evenly spaced in xi, over the element. <values> should be allocated with enough
space for number_of_components * product of number_in_xi+1 in each element
direction, the returned values cycling fastest through number of grid points in
xi1, number of grid points in xi2 etc. and lastly components.
It is up to the calling function to deallocate the returned values.
==============================================================================*/
{
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int element_dimension,i,j,k,number_of_points,return_code;

	ENTER(Computed_field_get_values_in_element);
	if (field&&element&&number_in_xi&&values)
	{
		return_code=1;
		element_dimension=element->shape->dimension;
		number_of_points=1;
		for (i=0;(i<element_dimension)&&return_code;i++)
		{
			if (0<number_in_xi[i])
			{
				number_of_points *= (number_in_xi[i]+1);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_get_values_in_element.  "
					"number_in_xi must be positive");
				return_code=0;
			}
		}
		if (return_code)
		{
			if (ALLOCATE(*values,FE_value,
				number_of_points*field->number_of_components))
			{
				for (j=0;(j<number_of_points)&&return_code;j++)
				{
					/* calculate xi at this point */
					k=j;
					for (i=0;i<element_dimension;i++)
					{
						xi[i]=(FE_value)(k % (number_in_xi[i]+1)) /
							(FE_value)(number_in_xi[i]);
						k /= (number_in_xi[i]+1);
					}
					if (Computed_field_evaluate_cache_in_element(field,element,xi,
						/*top_level_element*/(struct FE_element *)NULL,
						/*calculate_derivatives*/0))
					{
						for (k=0;k<field->number_of_components;k++)
						{
							(*values)[k*number_of_points+j] = field->values[k];
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_get_values_in_element.  Could not evaluate");
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_get_values_in_element.  Not enough memory");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_values_in_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_values_in_element */

int Computed_field_set_values_in_element(struct Computed_field *field,
	struct FE_element *element,int *number_in_xi,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 5 October 2000

DESCRIPTION :
Sets the <values> of the computed <field> over the <element>. Only certain
computed field types allow their values to be set. Fields that deal directly
with FE_fields eg. FINITE_ELEMENT fall into this category, as do the various
transformations, RC_COORDINATE, RC_VECTOR, OFFSET, SCALE, etc. which convert
the values into what they expect from their source field, and then call the
same function for it. If a field has more than one source field, eg. RC_VECTOR,
it can in many cases still choose which one is actually being changed, for
example, the 'vector' field in this case - coordinates should not change. This
process continues until the actual FE_field values in the element are changed or
a field is reached for which its calculation is not reversible, or is not
supported yet.

<number_in_xi> has the number of grid cells in each xi direction of <element>,
such that there is one more grid point in each direction than this number. Grid
points are evenly spaced in xi. There are as many <values> as there are grid
points X number_of_components, cycling fastest through number of grid points in
xi1, number of grid points in xi2 etc. and lastly components.

Note that the values array will not be modified by this function. Also,
<element> should not be managed at the time it is modified by this function.
#### Must ensure implemented correctly for new Computed_field_type. ####
???RC Note that some functions are not reversible in this way.
==============================================================================*/
{
	FE_value *source_values;
	int element_dimension,i,j,k,number_of_points,return_code;

	ENTER(Computed_field_set_values_in_element);
	if (field&&element&&element->shape&&number_in_xi&&values)
	{
		if (COMPUTED_FIELD_NEW_TYPES == field->type)
		{
			if (field->computed_field_set_values_in_element_function)
			{
				return_code = 
					field->computed_field_set_values_in_element_function(
					field, element, number_in_xi, values);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_values_in_element.  "
					"No function defined.");
				return_code = 0;
			}
		}
		else
		{
			return_code=1;
			element_dimension=element->shape->dimension;
			number_of_points=1;
			for (i=0;(i<element_dimension)&&return_code;i++)
			{
				if (0<number_in_xi[i])
				{
					number_of_points *= (number_in_xi[i]+1);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_set_values_in_element.  "
						"number_in_xi must be positive");
					return_code=0;
				}
			}
			if (return_code)
			{
				switch (field->type)
				{
					case COMPUTED_FIELD_COMPONENT:
					{
						int offset;

						/* need current field values to partially set */
						if (Computed_field_get_values_in_element(field->source_fields[0],
							element,number_in_xi,&source_values))
						{
							/* insert the component values into this array */
							offset=number_of_points*field->component_no;
							for (j=0;j<number_of_points;j++)
							{
								source_values[offset+j]=values[j];
							}
							return_code=Computed_field_set_values_in_element(
								field->source_fields[0],element,number_in_xi,source_values);
							DEALLOCATE(source_values);
						}
						else
						{
							return_code=0;
						}
					} break;
					case COMPUTED_FIELD_EDIT_MASK:
					{
						int offset;

						/* need current field values to partially set */
						if (Computed_field_get_values_in_element(field->source_fields[0],
							element,number_in_xi,&source_values))
						{
							/* insert the components with mask on into this array */
							for (k=0;k<field->number_of_components;k++)
							{
								if (field->source_values[k])
								{
									offset=k*number_of_points;
									for (j=0;j<number_of_points;j++)
									{
										source_values[offset+j] = values[offset+j];
									}
								}
							}
							return_code=Computed_field_set_values_in_element(
								field->source_fields[0],element,number_in_xi,source_values);
							DEALLOCATE(source_values);
						}
						else
						{
							return_code=0;
						}
					} break;
					case COMPUTED_FIELD_RC_COORDINATE:
					{
						FE_value non_rc_coordinates[3],rc_coordinates[3];
						struct Coordinate_system rc_coordinate_system;

						rc_coordinate_system.type = RECTANGULAR_CARTESIAN;
						/* 3 values for non-rc coordinate system */
						if (ALLOCATE(source_values,FE_value,number_of_points*3))
						{
							for (j=0;(j<number_of_points)&&return_code;j++)
							{
								for (k=0;k<3;k++)
								{
									rc_coordinates[k]=values[k*number_of_points+j];
								}
								/* convert RC values back into source coordinate system */
								if (convert_Coordinate_system(&rc_coordinate_system,
									rc_coordinates,&(field->source_fields[0]->coordinate_system),
									non_rc_coordinates,/*jacobian*/(float *)NULL))
								{
									for (k=0;k<3;k++)
									{
										source_values[k*number_of_points+j]=non_rc_coordinates[k];
									}
								}
								else
								{
									return_code=0;
								}
							}
							if (return_code)
							{
								return_code=Computed_field_set_values_in_element(
									field->source_fields[0],element,number_in_xi,source_values);
							}
							DEALLOCATE(source_values);
						}
						else
						{
							return_code=0;
						}
					} break;
					case COMPUTED_FIELD_RC_VECTOR:
					{
						FE_value jacobian[9],non_rc_coordinates[3],
							rc_coordinates[3],*rc_coordinate_values,sum;
						int coordinates_per_vector,m,number_of_vectors;
						struct Computed_field *rc_coordinate_field;
						struct Coordinate_system rc_coordinate_system;

						/* need current rc_coordinate positions for converting to
							coordinate system of source vector field */
						if (rc_coordinate_field=Computed_field_begin_wrap_coordinate_field(
							field->source_fields[1]))
						{
							if (Computed_field_get_values_in_element(rc_coordinate_field,
								element,number_in_xi,&rc_coordinate_values))
							{
								rc_coordinate_system.type = RECTANGULAR_CARTESIAN;
								/* 3 values for non-rc coordinate system */
								if (ALLOCATE(source_values,FE_value,
									number_of_points*field->source_fields[0]->number_of_components))
								{
									for (j=0;(j<number_of_points)&&return_code;j++)
									{
										for (k=0;k<3;k++)
										{
											rc_coordinates[k]=
												rc_coordinate_values[k*number_of_points+j];
										}
										/* need jacobian at current coordinate position for
											converting to coordinate system of source vector field
											(=source_fields[0]) */
										if (convert_Coordinate_system(&rc_coordinate_system,
											rc_coordinates,&field->source_fields[0]->coordinate_system,
											non_rc_coordinates,jacobian))
										{
											number_of_vectors=field->number_of_components/3;
											coordinates_per_vector=
												field->source_fields[0]->number_of_components/
												number_of_vectors;
											for (i=0;i<number_of_vectors;i++)
											{
												for (m=0;m<coordinates_per_vector;m++)
												{
													sum=0.0;
													for (k=0;k<3;k++)
													{
														sum +=
															jacobian[m*3+k]*values[(i*3+k)*number_of_points+j];
													}
													source_values[(i*coordinates_per_vector+m)*
														number_of_points+j]=sum;
												}
											}
										}
										else
										{
											return_code=0;
										}
									}
									if (return_code)
									{
										return_code=Computed_field_set_values_in_element(
											field->source_fields[0],element,number_in_xi,source_values);
									}
									DEALLOCATE(source_values);
								}
								DEALLOCATE(rc_coordinate_values);
							}
							else
							{
								return_code=0;
							}
							Computed_field_end_wrap(&rc_coordinate_field);
						}
						else
						{
							return_code=0;
						}
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_set_values_in_element.  "
							"Cannot set values for field %s of type %s",field->name,
							Computed_field_type_to_string(field));
						return_code=0;
					} break;
				}
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_values_in_element.  "
					"Failed for field %s of type %s",field->name,
					Computed_field_type_to_string(field));
			}
		}
		Computed_field_clear_cache(field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_values_in_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_values_in_element */

int Computed_field_set_values_in_managed_element(struct Computed_field *field,
	struct FE_element *element,int *number_in_xi,FE_value *values,
	struct MANAGER(FE_element) *element_manager)
/*******************************************************************************
LAST MODIFIED : 20 April 2000

DESCRIPTION :
Sets the <values> of the computed <field> over the <element>. Only certain
computed field types allow their values to be set. Fields that deal directly
with FE_fields eg. FINITE_ELEMENT fall into this category, as do the various
transformations, RC_COORDINATE, RC_VECTOR, OFFSET, SCALE, etc. which convert
the values into what they expect from their source field, and then call the
same function for it. If a field has more than one source field, eg. RC_VECTOR,
it can in many cases still choose which one is actually being changed, for
example, the 'vector' field in this case - coordinates should not change. This
process continues until the actual FE_field values in the element are changed or
a field is reached for which its calculation is not reversible, or is not
supported yet.

<number_in_xi> has the number of grid cells in each xi direction of <element>,
such that there is one more grid point in each direction than this number. Grid
points are evenly spaced in xi. There are as many <values> as there are grid
points X number_of_components, cycling fastest through number of grid points in
xi1, number of grid points in xi2 etc. and lastly components.

This function works by making a copy of the element, then performing all
modifications to it. If these are successful then MANAGER_MODIFY_NOT_IDENTIFIER
is called to effect the change on the real <element>, thus sending manager
messages to the rest of the program. Because all changes are made on a temporary
copy of the element, all current cache values will be ignored. For safety,
however, the cache is always cleared after calling.
It is up to calling function to begin/end node manager cache if more than one
node is being modified.
Note that the values array will not be modified by this function.
==============================================================================*/
{
	int return_code;
	struct FE_element *copy_element;

	ENTER(Computed_field_set_values_in_managed_element);
	return_code=0;
	if (field&&element&&number_in_xi&&values&&element_manager)
	{
		/* CREATE with template element is assumed to copy its values */
		if (copy_element=CREATE(FE_element)(element->identifier,element))
		{
			/* The element must be accessed as the use of cache on the elements
				by get values etc. access and deaccessess the elements */
			ACCESS(FE_element)(copy_element);
			if (Computed_field_set_values_in_element(field,copy_element,
				number_in_xi,values))
			{
				if (MANAGER_MODIFY_NOT_IDENTIFIER(FE_element,identifier)(
					element,copy_element,element_manager))
				{
					return_code=1;
				}
			}
			Computed_field_clear_cache(field);
			DEACCESS(FE_element)(&copy_element);
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_values_in_managed_element.  Failed");
			Computed_field_clear_cache(field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_values_in_managed_element.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_values_in_managed_element */

int Computed_field_get_native_discretization_in_element(
	struct Computed_field *field,struct FE_element *element,int *number_in_xi)
/*******************************************************************************
LAST MODIFIED : 20 April 2000

DESCRIPTION :
If the <field> or its source field is grid-based in <element>, returns in
<number_in_xi> the numbers of finite difference cells in each xi-direction of
<element>. Note that this number is one less than the number of grid points in
each direction. <number_in_xi> should be allocated with at least as much space
as the number of dimensions in <element>, but is assumed to have no more than
MAXIMUM_ELEMENT_XI_DIMENSIONS so that
int number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS] can be passed to this function.
Returns 0 with no errors if the field is not grid-based.
#### Must ensure implemented correctly for new Computed_field_type. ####
In particular, make sure all the same field types are supported here and in
Computed_field_set_values_in_[managed_]element.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_native_discretization_in_element);
	if (field&&element&&number_in_xi&&element->shape&&
		(MAXIMUM_ELEMENT_XI_DIMENSIONS>=element->shape->dimension))
	{
		if (COMPUTED_FIELD_NEW_TYPES == field->type)
		{
			if (field->computed_field_get_native_discretization_in_element_function)
			{
				return_code = 
					field->computed_field_get_native_discretization_in_element_function(
					field, element, number_in_xi);
			}
			else
			{
				return_code = 0;
			}
		}
		else
		{
			switch (field->type)
			{
				case COMPUTED_FIELD_COMPONENT:
				case COMPUTED_FIELD_EDIT_MASK:
				case COMPUTED_FIELD_RC_COORDINATE:
				case COMPUTED_FIELD_RC_VECTOR:
				{
					return_code=Computed_field_get_native_discretization_in_element(
						field->source_fields[0],element,number_in_xi);
				} break;
				default:
				{
					return_code=0;
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_native_discretization_in_element.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_native_discretization_in_element */

int Computed_field_default_get_native_discretization_in_element(
	struct Computed_field *field,struct FE_element *element,int *number_in_xi)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Inherits its result from the first source field.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_default_get_native_discretization_in_element);
	if (field&&element&&number_in_xi&&element->shape&&
		(MAXIMUM_ELEMENT_XI_DIMENSIONS>=element->shape->dimension))
	{
		return_code=Computed_field_get_native_discretization_in_element(
			field->source_fields[0],element,number_in_xi);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_default_get_native_discretization_in_element.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_default_get_native_discretization_in_element */

int Computed_field_get_number_of_components(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 23 December 1998

DESCRIPTION :
==============================================================================*/
{
	int number_of_components;

	ENTER(Computed_field_get_number_of_components);
	if (field)
	{
		number_of_components=field->number_of_components;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_number_of_components.  Missing field");
		number_of_components=0;
	}
	LEAVE;

	return (number_of_components);
} /* Computed_field_get_number_of_components */

char *Computed_field_get_component_name(struct Computed_field *field,
	int component_no)
/*******************************************************************************
LAST MODIFIED : 20 July 2000

DESCRIPTION :
Returns an allocated string containing the name of <component_no> of <field>.
If the <field> has special component names then these are returned,
otherwise default names are made out of the character form of the
component_no+1, eg, 1 -> "2".
It is up to the calling function to deallocate the returned string.
#### Must ensure implemented correctly for new Computed_field_type. ####
??? Get names from coordinate system?
==============================================================================*/
{
	char *component_name,temp_name[20];

	ENTER(Computed_field_get_component_name);
	component_name=(char *)NULL;
	if (field&&(0<=component_no)&&(component_no<field->number_of_components))
	{
		if (field->component_names)
		{
			if (!(component_name=duplicate_string(
				field->component_names[component_no])))
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_get_component_name.  Not enough memory");
			}					
		}
		else
		{
			sprintf(temp_name,"%i",component_no+1);
			if (!(component_name=duplicate_string(temp_name)))
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_get_component_name.  Not enough memory");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_component_name.  Invalid argument(s)");
	}
	LEAVE;

	return (component_name);
} /* Computed_field_get_component_name */

struct Coordinate_system *Computed_field_get_coordinate_system(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 26 January 1999

DESCRIPTION :
Returns the coordinate system <field> is to be interpreted under. See function
Computed_field_set_coordinate_system for further details.
==============================================================================*/
{
	struct Coordinate_system *coordinate_system;

	ENTER(Computed_field_get_coordinate_system);
	if (field)
	{
		coordinate_system=&(field->coordinate_system);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_coordinate_system.  Invalid argument(s)");
		coordinate_system=(struct Coordinate_system *)NULL;
	}
	LEAVE;

	return (coordinate_system);
} /* Computed_field_get_coordinate_system */

int Computed_field_set_coordinate_system(struct Computed_field *field,
	struct Coordinate_system *coordinate_system)
/*******************************************************************************
LAST MODIFIED : 26 January 1999

DESCRIPTION :
Sets the coordinate system <field> is to be interpreted under. Note the careful
choice of words here: the coordinate system merely tells the rest of the program
what needs to be done to transform the field values into any other coordinate
system. It does not have to be "correct" for the values in the field, eg. you
can describe prolate spheroidal values as RC to "open out" the heart model.
???RC How to check the coordinate system is valid?
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_set_coordinate_system);
	if (field&&coordinate_system)
	{
		return_code=
			COPY(Coordinate_system)(&(field->coordinate_system),coordinate_system);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_coordinate_system.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_coordinate_system */

char *Computed_field_get_type_string(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Returns the string which identifies the type.
==============================================================================*/
{
	char *return_string;

	ENTER(Computed_field_get_type_string);
	if (field)
	{
		return_string = field->type_string;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_field_get_type.  Missing field");
		return_string = (char *)NULL;
	}
	LEAVE;

	return (return_string);
} /* Computed_field_get_type_string */

enum Computed_field_type Computed_field_get_type(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 24 December 1998

DESCRIPTION :
Returns the type of the computed <field> eg. COMPUTED_FIELD_FINITE_ELEMENT etc.
==============================================================================*/
{
	enum Computed_field_type type;

	ENTER(Computed_field_get_type);
	if (field)
	{
		type=field->type;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_field_get_type.  Missing field");
		type=COMPUTED_FIELD_INVALID;
	}
	LEAVE;

	return (type);
} /* Computed_field_get_type */

char *Computed_field_type_to_string(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 23 March 2000

DESCRIPTION :
Returns a pointer to a static string token for the given <field_type>.
The calling function must not deallocate the returned string.
#### Must ensure implemented correctly for new Computed_field_type. ####
==============================================================================*/
{
	char *field_type_string;

	ENTER(Computed_field_type_to_string);
	switch (field->type)
	{
		case COMPUTED_FIELD_COMPONENT:
		{
			field_type_string="component";
		} break;
		case COMPUTED_FIELD_COMPOSE:
		{
			field_type_string="compose";
		} break;
		case COMPUTED_FIELD_CONSTANT:
		{
			field_type_string="constant";
		} break;
		case COMPUTED_FIELD_CUBIC_TEXTURE_COORDINATES:
		{
			field_type_string="cubic_texture_coordinates";
		} break;
		case COMPUTED_FIELD_EDIT_MASK:
		{
			field_type_string="edit_mask";
		} break;
		case COMPUTED_FIELD_EXTERNAL:
		{
			field_type_string="external";
		} break;
		case COMPUTED_FIELD_NEW_TYPES:
		{
			field_type_string=field->type_string;
		} break;
		case COMPUTED_FIELD_RC_COORDINATE:
		{
			field_type_string="rc_coordinate";
		} break;
		case COMPUTED_FIELD_RC_VECTOR:
		{
			field_type_string="rc_vector";
		} break;
		case COMPUTED_FIELD_SUM_COMPONENTS:
		{
			field_type_string="sum_components";
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_type_to_string.  Invalid field type");
			field_type_string=(char *)NULL;
		} break;
	}
	LEAVE;

	return (field_type_string);
} /* Computed_field_type_to_string */

int Computed_field_has_numerical_components(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 24 May 2000

DESCRIPTION :
Conditional function returning true if <field> returns numerical components.
Note that whether the numbers were integer, FE_value or double, they may be
returned as FE_value when evaluated.
#### Must ensure implemented correctly for new Computed_field_type. ####
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_has_numerical_components);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		if (COMPUTED_FIELD_NEW_TYPES == field->type)
		{
			if (field->computed_field_has_numerical_components_function)
			{
				return_code = field->computed_field_has_numerical_components_function(
					field);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_has_numerical_components.  "
					"No function defined.");
				return_code = 0;
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_numerical_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_has_numerical_components */

int Computed_field_default_has_numerical_components(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Most computed fields have numerical components so this function returns 1.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_default_has_numerical_components);
	if (field)
	{
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_numerical_components.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_default_has_numerical_components */

int Computed_field_is_scalar(struct Computed_field *field,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Conditional function returning true if <field> has 1 component and it is
numerical.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_scalar);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=(1 == field->number_of_components)&&
			Computed_field_has_numerical_components(field,(void *)NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_field_is_scalar.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_is_scalar */

int Computed_field_has_up_to_3_numerical_components(
	struct Computed_field *field,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Conditional function returning true if <field> has up to 3 components and they
are numerical - useful for selecting vector/coordinate fields.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_has_up_to_3_numerical_components);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=(3 >= field->number_of_components)&&
			Computed_field_has_numerical_components(field,(void *)NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_up_to_3_numerical_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_has_up_to_3_numerical_components */

int Computed_field_has_up_to_4_numerical_components(
	struct Computed_field *field,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Conditional function returning true if <field> has up to 4 components and they
are numerical - useful for selecting vector/coordinate fields.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_has_up_to_4_numerical_components);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=(4 >= field->number_of_components)&&
			Computed_field_has_numerical_components(field,(void *)NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_up_to_4_numerical_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_has_up_to_4_numerical_components */

int Computed_field_has_at_least_2_components(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 16 March 1999

DESCRIPTION :
Iterator/conditional function returning true if <field> has at least 2 components.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_has_at_least_2_components);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=(2 <= field->number_of_components);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_at_least_2_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_has_at_least_2_components */

int Computed_field_has_3_components(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 8 May 2000

DESCRIPTION :
Iterator/conditional function returning true if <field> has exactly three
components - useful for selecting vector/coordinate fields.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_has_3_components);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=(3 == field->number_of_components);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_3_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_has_3_components */

int Computed_field_has_4_components(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Iterator/conditional function returning true if <field> has exactly four
components - useful for selecting vector/coordinate fields.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_has_4_components);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=(4 == field->number_of_components);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_4_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_has_4_components */

int Computed_field_has_n_components(struct Computed_field *field,
	void *components_ptr_void)
/*******************************************************************************
LAST MODIFIED : 11 July 2000

DESCRIPTION :
Iterator/conditional function returning true if <field> has the same number of
components as that specified by <components_ptr_void>.
==============================================================================*/
{
	int *components, return_code;

	ENTER(Computed_field_has_n_components);
	if (field && (components = (int *)components_ptr_void))
	{
		return_code=(*components == field->number_of_components);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_n_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_has_n_components */

int Computed_field_is_orientation_scale_capable(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 12 February 1999

DESCRIPTION :
Conditional function returning true if the field can be used to orient or scale
glyphs. Generally, this means it has 1,2,3,4,6 or 9 components, where:
1 = scalar (no vector, isotropic scaling).
2 = 1 2-D vector (2nd axis is normal in plane, 3rd is out of 2-D plane);
3 = 1 3-D vector (orthogonal 2nd and 3rd axes are arbitrarily chosen);
4 = 2 2-D vectors (3rd axis taken as out of 2-D plane);
6 = 2 3-D vectors (3rd axis found from cross product);
9 = 3 3-D vectors = complete definition of 3 axes.
???RC.  Include coordinate system in test?
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_orientation_scale_capable);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=(
			(1==field->number_of_components)||(2==field->number_of_components)||
			(3==field->number_of_components)||(4==field->number_of_components)||
			(6==field->number_of_components)||(9==field->number_of_components));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_orientation_scale_capable.  Missing field");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_orientation_scale_capable */

int Computed_field_is_stream_vector_capable(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 15 March 1999

DESCRIPTION :
Conditional function returning true if the field is suitable for 3-D streamline
tracking. This means it has either 3, 6 or 9 components (with 3 components per
vector), or has a FIBRE coordinate_system, meaning it can be wrapped to produce
9-component fibre_axes.
The number of components controls how the field is interpreted:
3 = 1 3-D vector (lateral direction and normal worked out from curl of field);
6 = 2 3-D vectors (2nd vector is lateral direction. Stream ribbon normal found
    from cross product);
9 = 3 3-D vectors (2nd vector is lateral direction; 3rd vector is stream ribbon
    normal).
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_stream_vector_capable);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=((3==field->number_of_components)||
			(6==field->number_of_components)||(9==field->number_of_components)||
			((3>=field->number_of_components)&&
				(FIBRE==get_coordinate_system_type(&(field->coordinate_system)))));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_stream_vector_capable.  Missing field");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_stream_vector_capable */

int Computed_field_get_type_component(struct Computed_field *field,
	struct Computed_field **source_field,int *component_no)
/*******************************************************************************
LAST MODIFIED : 29 December 1998

DESCRIPTION :
If the field is of type COMPUTED_FIELD_COMPONENT, the source_field/component_no
used by it are returned - otherwise an error is reported.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_component);
	if (field&&(COMPUTED_FIELD_COMPONENT==field->type)&&source_field&&
		component_no)
	{
		*source_field=field->source_fields[0];
		*component_no=field->component_no;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_component.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_component */

int Computed_field_set_type_component(struct Computed_field *field,
	struct Computed_field *source_field,int component_no)
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_COMPONENT, returning the value of
component <component_no> of <source_field>. Sets number of components to 1.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_component);
	if (field&&source_field&&(0<=component_no)&&
		(component_no<source_field->number_of_components))
	{
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_COMPONENT;
			field->number_of_components=1;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			field->component_no=component_no;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_component.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_component.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_component */

int Computed_field_get_type_compose(struct Computed_field *field,
	struct Computed_field **texture_coordinate_field,
	struct Computed_field **find_element_xi_field,
	struct Computed_field **calculate_values_field,
	struct GROUP(FE_element) **search_element_group)
/*******************************************************************************
LAST MODIFIED : 16 June 2000

DESCRIPTION :
If the field is of type COMPUTED_FIELD_COMPOSE, the function returns the three
fields which define the field.
Note that the fields are not ACCESSed.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_compose);
	if (field&&(COMPUTED_FIELD_COMPOSE==field->type)&&texture_coordinate_field&&
		find_element_xi_field&&calculate_values_field&&search_element_group)
	{
		*texture_coordinate_field = field->source_fields[0];
		*find_element_xi_field = field->source_fields[1];
		*calculate_values_field = field->source_fields[2];
		*search_element_group = field->compose_element_group;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_compose.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_compose */

int Computed_field_set_type_compose(struct Computed_field *field,
	struct Computed_field *texture_coordinate_field,
	struct Computed_field *find_element_xi_field,
	struct Computed_field *calculate_values_field,
	struct GROUP(FE_element) *search_element_group)
/*******************************************************************************
LAST MODIFIED : 16 June 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_COMPOSE, this field allows you to
evaluate one field to find "texture coordinates", use a find_element_xi field
to then calculate a corresponding element/xi and finally calculate values using
this element/xi and a third field.  You can then evaluate values on a "host"
mesh for any points "contained" inside.  The <search_element_group> is the group
from which any returned element_xi will belong.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields, return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_compose);
	if (field&&texture_coordinate_field&&find_element_xi_field&&
		calculate_values_field&&search_element_group)
	{
		return_code=1;
		if (return_code)
		{
			/* 1. make dynamic allocations for any new type-specific data and
			 check fields are valid */
			number_of_source_fields=3;
			if (ALLOCATE(source_fields,struct Computed_field *,
				number_of_source_fields))
			{
				if (texture_coordinate_field->number_of_components ==
					find_element_xi_field->number_of_components)
				{
					if (texture_coordinate_field->number_of_components ==
						find_element_xi_field->number_of_components)
					{
						/* 2. free current type-specific data */
						Computed_field_clear_type(field);
						/* 3. establish the new type */
						field->type=COMPUTED_FIELD_COMPOSE;
						field->number_of_components=
							calculate_values_field->number_of_components;
						source_fields[0]=ACCESS(Computed_field)(texture_coordinate_field);
						source_fields[1]=ACCESS(Computed_field)(find_element_xi_field);
						source_fields[2]=ACCESS(Computed_field)(calculate_values_field);
						field->source_fields=source_fields;
						field->number_of_source_fields=number_of_source_fields;
						field->compose_element_group=search_element_group;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_set_type_compose."
							"  The find_element_xi_field must be find_element_xi capable");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_set_type_compose."
						"  The texuture_coordinate_field and find_element_xi_field"
						" must have the same number of components");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_compose.  Not enough memory");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_compose.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_compose */

int Computed_field_get_type_constant(struct Computed_field *field,
	int *number_of_values,FE_value **values)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
If the field is of type COMPUTED_FIELD_CONSTANT, the number_of_values and
values it contains are returned. The <*values> array is allocated and returned
by this function - it is up to the calling function to DEALLOCATE it.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_get_type_constant);
	if (field&&(COMPUTED_FIELD_CONSTANT==field->type)&&number_of_values&&values)
	{
		*number_of_values=field->number_of_source_values;
		if (ALLOCATE(*values,FE_value,*number_of_values))
		{
			for (i=0;i<(*number_of_values);i++)
			{
				(*values)[i]=field->source_values[i];
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_constant.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_constant.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_constant */

int Computed_field_set_type_constant(struct Computed_field *field,
	int number_of_values,FE_value *values)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_CONSTANT, returning the given array of
<values> everywhere.
Gives the field as many components as the specified <number_of_values>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	FE_value *source_values;
	int i,number_of_source_values,return_code;

	ENTER(Computed_field_set_type_constant);
	if (field&&(0<number_of_values)&&values)
	{
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_values=number_of_values;
		if (ALLOCATE(source_values,FE_value,number_of_source_values))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_CONSTANT;
			field->number_of_components=number_of_values;
			for (i=0;i<number_of_values;i++)
			{
				source_values[i]=values[i];
			}
			field->source_values=source_values;
			field->number_of_source_values=number_of_source_values;
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_constant.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_constant */

int Computed_field_get_type_cubic_texture_coordinates(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 17 June 1999

DESCRIPTION :
If the field is of type CUBIC_TEXTURE_COORDINATES, the source field used
by it is returned - otherwise an error is reported.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_cubic_texture_coordinates);
	if (field&&(COMPUTED_FIELD_CUBIC_TEXTURE_COORDINATES==field->type)
		&&source_field)
	{
		*source_field=field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_cubic_texture_coordinates.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_cubic_texture_coordinates */

int Computed_field_set_type_cubic_texture_coordinates(struct Computed_field *field,
	struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_CUBIC_TEXTURE_COORDINATES, 
which returns texture coordinates based on a cubic projection from the origin.
Sets the number of components to the same as the source field.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_cubic_texture_coordinates);
	if (field&&source_field)
	{
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_CUBIC_TEXTURE_COORDINATES;
			field->number_of_components=source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_cubic_texture_coordinates.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_cubic_texture_coordinates.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_cubic_texture_coordinates */

int Computed_field_get_type_edit_mask(struct Computed_field *field,
	struct Computed_field **source_field,FE_value **edit_mask)
/*******************************************************************************
LAST MODIFIED : 20 August 1999

DESCRIPTION :
If the field is of type COMPUTED_FIELD_EDIT_MASK, the source_field and
edit_mask used by it are returned. Since the number of edit_mask is
equal to the number of components in the source_field (and you don't know this
yet), this function returns in *edit_mask a pointer to an allocated array
containing the FE_values.
It is up to the calling function to DEALLOCATE the returned <*edit_mask>.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_get_type_edit_mask);
	if (field&&(COMPUTED_FIELD_EDIT_MASK==field->type)&&source_field&&
		edit_mask)
	{
		if (ALLOCATE(*edit_mask,FE_value,
			field->source_fields[0]->number_of_components))
		{
			*source_field=field->source_fields[0];
			for (i=0;i<field->source_fields[0]->number_of_components;i++)
			{
				(*edit_mask)[i]=field->source_values[i];
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_edit_mask.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_edit_mask.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_edit_mask */

int Computed_field_set_type_edit_mask(struct Computed_field *field,
	struct Computed_field *source_field,FE_value *edit_mask)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_EDIT_MASK, returning the <source_field>
with each component edit_maskd by its respective FE_value in <edit_mask>.
The <edit_mask> array must therefore contain as many FE_values as there are
components in <source_field>.
Sets the number of components to the same as <source_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	FE_value *source_values;
	int i,number_of_source_fields,number_of_source_values,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_edit_mask);
	if (field&&source_field&&edit_mask)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_values=source_field->number_of_components;
		number_of_source_fields=1;
		if (ALLOCATE(source_values,FE_value,number_of_source_values)&&
			ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_EDIT_MASK;
			field->number_of_components=source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			for (i=0;i<number_of_source_values;i++)
			{
				source_values[i]=edit_mask[i];
			}
			field->source_values=source_values;
			field->number_of_source_values=number_of_source_values;
		}
		else
		{
			DEALLOCATE(source_values);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_edit_mask.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_edit_mask */

static int Computed_field_get_type_external(struct Computed_field *field,
	char **filename, int *timeout,
	int *number_of_source_values, FE_value **source_values,
	int *number_of_source_fields,struct Computed_field ***source_fields)
/*******************************************************************************
LAST MODIFIED : 31 August 1999

DESCRIPTION :
If the field is of type COMPUTED_FIELD_EXTERNAL, the function allocates and
returns in <**source_fields> an array containing the <number_of_sources> source
fields making up the composite field - otherwise an error is reported.
It is up to the calling function to DEALLOCATE the returned array. Note that the
fields in the returned array are not ACCESSed.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_get_type_external);
	if (field&&(COMPUTED_FIELD_EXTERNAL==field->type)&&number_of_source_fields&&
		source_fields)
	{
		return_code = 1;
		*number_of_source_fields=field->number_of_source_fields;
		*number_of_source_values=field->number_of_source_values;
		*timeout = field->timeout;
		if (*number_of_source_fields)
		{
			if (ALLOCATE(*source_fields,struct Computed_field *,*number_of_source_fields))
			{
				for (i=0;i<(*number_of_source_fields);i++)
				{
					(*source_fields)[i]=field->source_fields[i];
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_get_type_external.  Could not allocate source_field array");
				return_code = 0;
			}
		}
		else
		{
			*source_fields = (struct Computed_field **)NULL;
		}
		if (return_code && *number_of_source_values)
		{
			if (ALLOCATE(*source_values, FE_value, *number_of_source_values))
			{
				for (i=0;i<(*number_of_source_values);i++)
				{
					(*source_values)[i]=field->source_values[i];
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_get_type_external.  Could not allocate source_values array");
				return_code = 0;
			}
		}
		else
		{
			*source_values = (FE_value *)NULL;
		}
		if (return_code &&
			ALLOCATE(*filename, char, strlen(field->child_filename)+1))
		{
			strcpy(*filename, field->child_filename);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_external.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_external */

static int Computed_field_set_type_external(struct Computed_field *field,
	char *filename, int timeout,
	int number_of_source_values, FE_value *source_values,
	int number_of_source_fields, struct Computed_field **source_fields)
/*******************************************************************************
LAST MODIFIED : 30 August 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_EXTERNAL.
<source_fields> must point to an array of <number_of_sources> pointers to
Computed_fields. The resulting field will have as many
components as <number_of_sources> * components + number_of_source_values.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	char *filename_space, outstring[500], *result_string;
	FE_value *source_values_copy;
	int components, i, index, j, number_of_fields, number_of_values,
		return_code, total_values;
	struct Child_process *child_process;
	struct Computed_field **source_fields_copy;

	ENTER(Computed_field_set_type_external);
	if (field&&filename&&source_fields)
	{
		return_code = 1;
		total_values = number_of_source_values;
		/* make sure source_fields are all non-NULL */
		for (i=0;return_code&&(i<number_of_source_fields);i++)
		{
			if (source_fields[i])
			{
				total_values += source_fields[i]->number_of_components;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_external.  Invalid source_fields");
				return_code=0;
			}
		}
		/* Try executable and see how many components it will return */
		if (return_code && (child_process = CREATE(Child_process)(filename)))
		{
			sprintf(outstring, "%d #values %d #fields %d%n", total_values, number_of_source_values, 
				number_of_source_fields, &index);
			for (i = 0 ; i < number_of_source_fields ; i++)
			{
				sprintf(outstring + index, " %s %d AS_FOR_FIELD%n", source_fields[i]->name,
					source_fields[i]->number_of_components, &j);
				index += j;
			}
			sprintf(outstring + index, "\n");
			Child_process_send_string_to_stdin(child_process, outstring);
			if (result_string = Child_process_get_line_from_stdout(
				child_process, timeout))
			{
				sscanf(result_string, "%d%n", &components, &index);
				if (components < 1)
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_set_type_external.  "
						"External field incompatible with source fields");
					display_message(ERROR_MESSAGE,
						"%s", result_string);
					DESTROY(Child_process)(&child_process);
					return_code=0;
				}
				else
				{
					/* Check all the fields of the return string and give
					 sensible errors if they don't match */
					sscanf(result_string + index, "%s%d%n", outstring, &number_of_values,
						&j);
					index += j;
					if (fuzzy_string_compare(outstring, "#values"))
					{
						if (number_of_values == number_of_source_values)
						{
							sscanf(result_string + index, "%s%d%n", outstring, &number_of_fields,
								&j);
							index += j;
							if (fuzzy_string_compare(outstring, "#fields"))
							{
								if (number_of_fields == number_of_source_fields)
								{
									for (i = 0 ; (i < number_of_fields) ; i++)
									{
										sscanf(result_string + index, "%*s%d%s%n", 
											&number_of_values, outstring, &j);
										index += j;
										if (number_of_values != source_fields[i]->number_of_components)
										{
											display_message(ERROR_MESSAGE,
												"Computed_field_set_type_external."
												"  Number of components in field (%s %d) does not match the requirements of the external program (%d)",
												source_fields[i]->name, source_fields[i]->number_of_components,
												number_of_values);
											DESTROY(Child_process)(&child_process);
											return_code = 0;
										}
										if (fuzzy_string_compare(outstring, "AS_FOR_FIELD"))
										{
											/* Set a flag saying this and also check for
												other flags (i.e. NEVER and ALWAYS) */
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"Computed_field_set_type_external."
												"  Unknown derivative specifier (%s)",
												outstring);
											DESTROY(Child_process)(&child_process);
											return_code = 0;
										}
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Computed_field_set_type_external."
										"  Number of source fields given (%d) does not match the requirements of the external program (%d)",
										number_of_source_fields, number_of_fields);
									DESTROY(Child_process)(&child_process);
									return_code = 0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Computed_field_set_type_external.  #values keyword not found");
								DESTROY(Child_process)(&child_process);
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_set_type_external."
								"  Number of source values given (%d) does not match the requirements of the external program (%d)",
								number_of_source_values, number_of_values);
							DESTROY(Child_process)(&child_process);
							return_code = 0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_set_type_external.  #values keyword not found");
						DESTROY(Child_process)(&child_process);
						return_code = 0;
					}
				}
				DEALLOCATE(result_string);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_external.  Invalid response from child process");
				DESTROY(Child_process)(&child_process);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_external.  Could not create child process");
			return_code=0;
		}
		if (return_code)
		{
			/* 1. make dynamic allocations for any new type-specific data */
			if (number_of_source_fields)
			{
				if (!ALLOCATE(source_fields_copy, struct Computed_field *,
					number_of_source_fields))
				{
					return_code = 0;
				}
			}
			else
			{
				source_fields_copy = (struct Computed_field **)NULL;
			}
			if (number_of_source_values)
			{
				if (!ALLOCATE(source_values_copy, FE_value, number_of_source_values))
				{
					return_code = 0;
				}
			}
			else
			{
				source_values_copy = (FE_value *)NULL;
			}
			if (return_code &&
				ALLOCATE(filename_space,char,strlen(filename) + 1))
			{
				/* 2. free current type-specific data */
				Computed_field_clear_type(field);
				/* 3. establish the new type */
				field->type=COMPUTED_FIELD_EXTERNAL;
				for (i=0;i<number_of_source_fields;i++)
				{
					source_fields_copy[i]=ACCESS(Computed_field)(source_fields[i]);
				}
				field->source_fields=source_fields_copy;
				field->number_of_source_fields=number_of_source_fields;
				field->number_of_source_values = number_of_source_values;
				for (i=0;i<number_of_source_values;i++)
				{
					source_values_copy[i]=source_values[i];
				}
				field->source_values = source_values_copy;
				field->number_of_components=components;
				field->child_filename = filename_space;
				strcpy(filename_space, filename);
				field->timeout = timeout;
				field->child_process = ACCESS(Child_process)(child_process);
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_external.  Unable to establish external field");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_external.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_external */

int Computed_field_get_type_rc_coordinate(struct Computed_field *field,
	struct Computed_field **coordinate_field)
/*******************************************************************************
LAST MODIFIED : 19 January 1999

DESCRIPTION :
If the field is of type COMPUTED_FIELD_RC_COORDINATE, the coordinate field used
by it is returned - otherwise an error is reported.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_rc_coordinate);
	if (field&&(COMPUTED_FIELD_RC_COORDINATE==field->type)&&coordinate_field)
	{
		*coordinate_field=field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_rc_coordinate.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_rc_coordinate */

int Computed_field_set_type_rc_coordinate(struct Computed_field *field,
	struct Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_RC_COORDINATE, which returns the values
and derivatives of the coordinate field in rectangular cartesian coordinates.
Coordinate_system of this type of field need not be RC, although it usually will
be. The <coordinate_field> must have no more than 3 components.
Sets the number of components to 3.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_rc_coordinate);
	if (field&&coordinate_field&&(3>=coordinate_field->number_of_components))
	{
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_RC_COORDINATE;
			field->number_of_components=3;
			source_fields[0]=ACCESS(Computed_field)(coordinate_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_rc_coordinate.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_rc_coordinate.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_rc_coordinate */

int Computed_field_get_type_rc_vector(struct Computed_field *field,
	struct Computed_field **vector_field,struct Computed_field **coordinate_field)
/*******************************************************************************
LAST MODIFIED : 24 June 1999

DESCRIPTION :
If the field is of type COMPUTED_FIELD_RC_VECTOR, the vector and coordinate
fields used by it are returned - otherwise an error is reported.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_rc_vector);
	if (field&&(COMPUTED_FIELD_RC_VECTOR==field->type)&&vector_field&&
		coordinate_field)
	{
		/* source_fields: 0=vector, 1=coordinate */
		*vector_field=field->source_fields[0];
		*coordinate_field=field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_rc_vector.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_rc_vector */

int Computed_field_set_type_rc_vector(struct Computed_field *field,
	struct Computed_field *vector_field,struct Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 24 June 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_RC_VECTOR, combining a vector field
supplying a single vector (1,2 or 3 components), two vectors (4 or 6 components)
or three vectors (9 components) with a coordinate field. This field type ensures
that each source vector is converted to RC coordinates at the position given by
the coordinate field - as opposed to RC_COORDINATE which assumes the
transformation is always based at the origin.
Sets the number of components to 3 times the number of vectors expected from
the source vector_field.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_rc_vector);
	if (field&&vector_field&&coordinate_field&&
		Computed_field_is_orientation_scale_capable(vector_field,(void *)NULL)&&
		Computed_field_has_up_to_3_numerical_components(coordinate_field,
			(void *)NULL))
	{
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=2;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_RC_VECTOR;
			if (3 >= vector_field->number_of_components)
			{
				field->number_of_components=3;
			}
			else if (6 >= vector_field->number_of_components)
			{
				field->number_of_components=6;
			}
			else
			{
				field->number_of_components=9;
			}
			/* source_fields: 0=vector, 1=coordinate */
			source_fields[0]=ACCESS(Computed_field)(vector_field);
			source_fields[1]=ACCESS(Computed_field)(coordinate_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_rc_vector.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_rc_vector.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_rc_vector */

int Computed_field_get_type_sum_components(struct Computed_field *field,
	struct Computed_field **source_field,FE_value **weights)
/*******************************************************************************
LAST MODIFIED : 8 March 1999

DESCRIPTION :
If the field is of type COMPUTED_FIELD_SUM_COMPONENTS, the source_field and
weights used by it are returned. Since the number of weights is equal to the
number of components in the source_field (and you don't know this yet), this
function returns in *weights a pointer to an allocated array containing the
FE_values.
It is up to the calling function to DEALLOCATE the returned <*weights>.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_get_type_sum_components);
	if (field&&(COMPUTED_FIELD_SUM_COMPONENTS==field->type)&&source_field&&
		weights)
	{
		if (ALLOCATE(*weights,FE_value,
			field->source_fields[0]->number_of_components))
		{
			*source_field=field->source_fields[0];
			for (i=0;i<field->source_fields[0]->number_of_components;i++)
			{
				(*weights)[i]=field->source_values[i];
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_sum_components.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_sum_components.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_sum_components */

int Computed_field_set_type_sum_components(struct Computed_field *field,
	struct Computed_field *source_field,FE_value *weights)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_SUM_COMPONENTS, returning a scalar
weighted sum of the components of <source_field>. The <weights> array must
contain as many FE_value weights as there are components in <source_field>.
Sets the number of components to 1.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	FE_value *source_values;
	int i,number_of_source_fields,number_of_source_values,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_sum_components);
	if (field&&source_field&&weights)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_values=source_field->number_of_components;
		number_of_source_fields=1;
		if (ALLOCATE(source_values,FE_value,number_of_source_values)&&
			ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_SUM_COMPONENTS;
			field->number_of_components=1;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			for (i=0;i<number_of_source_values;i++)
			{
				source_values[i]=weights[i];
			}
			field->source_values=source_values;
			field->number_of_source_values=number_of_source_values;
		}
		else
		{
			DEALLOCATE(source_values);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_sum_components.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_sum_components */

int Computed_field_is_in_use(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 26 January 1999

DESCRIPTION :
Returns true if the field is accessed more than once; ie. it is in use somewhere
else in the program - apart from being accessed by its manager.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_in_use);
	if (field)
	{
		return_code=(1 < field->access_count);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_field_is_in_use.  Missing field");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_in_use */

int Computed_field_is_read_only(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 26 January 1999

DESCRIPTION :
Returns true if the field is read_only - use this to check if you are allowed
to modify or remove it from the manager.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_read_only);
	if (field)
	{
		return_code=field->read_only;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_read_only.  Missing field");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_read_only */

int Computed_field_set_read_only(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 5 February 1999

DESCRIPTION :
Marks <field> as read-only, telling the program that the user is not permitted
to modify or destroy it.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_set_read_only);
	if (field)
	{
		field->read_only=1;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_read_only.  Missing field");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_read_only */

int Computed_field_set_read_write(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 5 February 1999

DESCRIPTION :
Clears read-only status of <field>, telling the program that the user is allowed
to modify and destroy it.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_set_read_write);
	if (field)
	{
		field->read_only=0;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_read_write.  Missing field");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_read_write */

int Computed_field_find_element_xi(struct Computed_field *field,
	FE_value *values, int number_of_values, struct FE_element **element, 
	FE_value *xi, struct GROUP(FE_element) *search_element_group)
/*******************************************************************************
LAST MODIFIED : 17 July 2000

DESCRIPTION :
This function implements the reverse of some certain computed_fields
(Computed_field_is_find_element_xi_capable) so that it tries to find an element
and xi which would evaluate to the given values.
This has been implemented so that the texture_coordinates can be used to extract
information from textures (sample_texture computed_field) and then modified and
then put back into another texture.
The <search_element_group> is the set of elements from which the chosen element
will belong.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_find_element_xi);
	if (field&&values&&(number_of_values==field->number_of_components)
		&&element&&xi&&search_element_group)
	{
		if (COMPUTED_FIELD_NEW_TYPES == field->type)
		{
			if (field->computed_field_find_element_xi_function)
			{
				return_code = 
					field->computed_field_find_element_xi_function(
					field, values, number_of_values, element, xi,
					search_element_group);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_find_element_xi.  No function defined.");
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_find_element_xi.  Field type not implemented");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_find_element_xi.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_find_element_xi */

int Computed_field_is_find_element_xi_capable(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 16 June 2000

DESCRIPTION :
This function returns true if the <field> can find element and xi given
a set of values.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_find_element_xi_capable);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		switch (field->type)
		{
			case COMPUTED_FIELD_NEW_TYPES:
			{
				if (field->computed_field_find_element_xi_function)
				{
					return_code=1;
				}
				else
				{
					return_code=0;
				}
			} break;
			default:
			{
				return_code=0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_find_element_xi_capable.  Missing field");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_find_element_xi_capable */

struct Computed_field *Computed_field_manager_get_component_wrapper(
	struct MANAGER(Computed_field) *computed_field_manager,
	struct Computed_field *field,int component_no)
/*******************************************************************************
LAST MODIFIED : 3 December 1999

DESCRIPTION :
If a COMPONENT wrapper for <field> <component_no> exists in the
<computed_field_manager>, it is returned, otherwise a new one is made in the
manager and returned.
==============================================================================*/
{
	char *component_field_name,*component_name;
	struct Computed_field *component_field;
	struct Computed_field_component field_component;

	ENTER(Computed_field_manager_get_component_wrapper);
	component_field=(struct Computed_field *)NULL;
	if (computed_field_manager&&field&&(0<=component_no)&&
		(component_no<field->number_of_components))
	{
		field_component.field=field;
		field_component.component_no=component_no;
		/* try to find an existing wrapper for this component */
		if (!(component_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
			Computed_field_wraps_field_component,&field_component,
			computed_field_manager)))
		{
			if (component_name=Computed_field_get_component_name(field,component_no))
			{
				if (ALLOCATE(component_field_name,char,strlen(field->name)+
					strlen(component_name)+2))
				{
					sprintf(component_field_name,"%s.%s",field->name,component_name);
					if (component_field=CREATE(Computed_field)(component_field_name))
					{
						if (!(Computed_field_set_type_component(component_field,field,
							component_no)&&
							ADD_OBJECT_TO_MANAGER(Computed_field)(component_field,
								computed_field_manager)))
						{
							DESTROY(Computed_field)(&component_field);
						}
					}
					DEALLOCATE(component_field_name);
				}
				DEALLOCATE(component_name);
			}
			if (!component_field)
			{
				display_message(WARNING_MESSAGE,
					"Computed_field_manager_get_component_wrapper.  Failed");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_manager_get_component_wrapper.  Invalid argument(s)");
	}
	LEAVE;

	return (component_field);
} /* Computed_field_manager_get_component_wrapper */

int set_Computed_field_conditional(struct Parse_state *state,
	void *field_address_void,void *set_field_data_void)
/*******************************************************************************
LAST MODIFIED : 3 December 1999

DESCRIPTION :
Modifier function to set the field from a command. <set_field_data_void> should
point to a struct Set_Computed_field_conditional_data containing the
computed_field_manager and an optional conditional function for narrowing the
range of fields available for selection. If the conditional_function is NULL,
this function works just like set_Computed_field.
==============================================================================*/
{
	char *current_token,*field_component_name,*temp_name;
	int component_no,i,return_code;
	struct Computed_field **field_address,*selected_field;
	struct Set_Computed_field_conditional_data *set_field_data;

	ENTER(set_Computed_field_conditional);
	if (state&&(field_address=(struct Computed_field **)field_address_void)&&
		(set_field_data=
			(struct Set_Computed_field_conditional_data *)set_field_data_void)&&
		set_field_data->computed_field_manager)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (fuzzy_string_compare(current_token,"NONE"))
				{
					if (*field_address)
					{
						DEACCESS(Computed_field)(field_address);
						*field_address=(struct Computed_field *)NULL;
					}
					return_code=1;
				}
				else
				{
					/* component_no = -1 denotes the whole field may be used */
					component_no=-1;
					if (!(selected_field=
						FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(current_token,
						set_field_data->computed_field_manager)))
					{
						if (field_component_name=strchr(current_token,'.'))
						{
							*field_component_name='\0';
							field_component_name++;
							if (selected_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,
								name)(current_token,set_field_data->computed_field_manager))
							{
								/* get the component number */
								for (i=0;(i<selected_field->number_of_components)&&
									(0>component_no)&&selected_field;i++)
								{
									if (temp_name=
										Computed_field_get_component_name(selected_field,i))
									{
										if (0==strcmp(field_component_name,temp_name))
										{
											component_no=i;
										}
										DEALLOCATE(temp_name);
									}
									else
									{
										display_message(WARNING_MESSAGE,
											"set_Computed_field_component.  Not enough memory");
										selected_field=(struct Computed_field *)NULL;
									}
								}
								if (0<=component_no)
								{
									if (1==selected_field->number_of_components)
									{
										/* already a single component field */
										component_no=-1;
									}
									else
									{
										/* get or make wrapper for field component */
										if (!(selected_field=
											Computed_field_manager_get_component_wrapper(
												set_field_data->computed_field_manager,
												selected_field,component_no)))
										{
											display_message(WARNING_MESSAGE,
												"set_Computed_field_component.  "
												"Could not make component wrapper");
										}
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"Unknown field component: %s.%s",current_token,
										field_component_name);
									selected_field=(struct Computed_field *)NULL;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"Unknown field : %s",
									current_token);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown field : %s",
								current_token);
						}
					}
					if (selected_field)
					{
						if ((NULL==set_field_data->conditional_function)||
							((set_field_data->conditional_function)(selected_field,
								set_field_data->conditional_function_user_data)))
						{
							if (*field_address != selected_field)
							{
								DEACCESS(Computed_field)(field_address);
								*field_address=ACCESS(Computed_field)(selected_field);
							}
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,"Field of incorrect type : %s",
								current_token);
							return_code=0;
						}
					}
					else
					{
						return_code=0;
					}
				}
				shift_Parse_state(state,1);
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					" FIELD_NAME[.COMPONENT_NAME]|none");
				/* if possible, then write the name */
				if (selected_field= *field_address)
				{
					display_message(INFORMATION_MESSAGE,"[%s]",selected_field->name);
				}
				else
				{
					display_message(INFORMATION_MESSAGE,"[none]");
				}
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing field name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Computed_field_conditional.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Computed_field_conditional */

int set_Computed_field_array(struct Parse_state *state,
	void *field_array_void,void *set_field_array_data_void)
/*******************************************************************************
LAST MODIFIED : 9 March 1999

DESCRIPTION :
Modifier function to set an array of field from a command.
<set_field_array_data_void> should point to a struct
Set_Computed_field_array_conditional_data containing the number_of_fields in the
array, the computed_field_package and an optional conditional function for
narrowing the range of fields available for selection.
Works by repeatedly calling set_Computed_field_conditional.
???RC Make this globally available for calling any modifier function?
==============================================================================*/
{
	int i,return_code;
	struct Computed_field **field_array;
	struct Set_Computed_field_array_data *set_field_array_data;

	ENTER(set_Computed_field_array);
	if (state&&(field_array=(struct Computed_field **)field_array_void)&&
		(set_field_array_data=(struct Set_Computed_field_array_data *)
			set_field_array_data_void)&&(0<set_field_array_data->number_of_fields)&&
			set_field_array_data->conditional_data)
	{
		return_code=1;
		for (i=0;i<set_field_array_data->number_of_fields;i++)
		{
			if (!set_Computed_field_conditional(state,
				&(field_array[i]),(void *)set_field_array_data->conditional_data))
			{
				return_code=0;
			}
		}
		if (!return_code)
		{
			if ((!state->current_token)||
				(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
			{
				display_message(ERROR_MESSAGE,
					"set_Computed_field_array.  Error parsing field array");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Computed_field_array.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Computed_field_array */

static int set_Computed_field_component(struct Parse_state *state,
	void *field_component_void,void *computed_field_manager_void)
/*******************************************************************************
LAST MODIFIED : 25 January 1999

DESCRIPTION :
Used in command parsing to translate a FIELD_NAME.COMPONENT_NAME into a struct
Computed_field_component.
???RC.  Does not ACCESS the field (unlike set_Computed_field).
==============================================================================*/
{
	char *current_token,*field_component_name,*field_name,*temp_name;
	int component_no,i,return_code;
	struct Computed_field *field;
	struct Computed_field_component *field_component;
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(set_Computed_field_component);
	if (state&&
		(field_component=(struct Computed_field_component *)field_component_void)&&
		(computed_field_manager=
			(struct MANAGER(Computed_field) *)computed_field_manager_void))
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (field_component_name=strchr(current_token,'.'))
				{
					*field_component_name='\0';
					field_component_name++;
				}
				if (field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
					current_token,computed_field_manager))
				{
					if (field_component_name)
					{
						return_code=1;
						component_no=-1;
						for (i=0;(0>component_no)&&(i<field->number_of_components)&&
							return_code;i++)
						{
							if (temp_name=Computed_field_get_component_name(field,i))
							{
								if (0==strcmp(field_component_name,temp_name))
								{
									component_no=i;
								}
								DEALLOCATE(temp_name);
							}
							else
							{
								display_message(WARNING_MESSAGE,
									"set_Computed_field_component.  Not enough memory");
								return_code=0;
							}
						}
						if (return_code)
						{
							if (0 <= component_no)
							{
								field_component->field=field;
								field_component->component_no=component_no;
							}
							else
							{
								display_message(WARNING_MESSAGE,
									"Unknown field component %s.%s",current_token,
									field_component_name);
								return_code=0;
							}
						}
					}
					else
					{
						field_component->field=field;
						field_component->component_no=0;
						return_code=1;
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,"Unknown field %s",current_token);
					return_code=1;
				}
				shift_Parse_state(state,1);
			}
			else
			{
				display_message(INFORMATION_MESSAGE," FIELD_NAME.COMPONENT_NAME");
				if (field_component->field&&
					GET_NAME(Computed_field)(field_component->field,&field_name))
				{
					if (1<field_component->field->number_of_components)
					{
						if (field_component_name=Computed_field_get_component_name(
							field_component->field,field_component->component_no))
						{
							display_message(INFORMATION_MESSAGE,"[%s.%s]",field_name,
								field_component_name);
							DEALLOCATE(field_component_name);
						}
					}
					else
					{
						display_message(INFORMATION_MESSAGE,"[%s]",field_name);
					}
					DEALLOCATE(field_name);
				}
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing field component name");
			display_parse_state_location(state);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Computed_field_component.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Computed_field_component */

static int define_Computed_field_type_component(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 26 January 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_COMPONENT (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{NULL,NULL,NULL,set_Computed_field_component}
	};
	struct Computed_field *field;
	struct Computed_field_component field_component;
	struct Computed_field_package *computed_field_package;

	ENTER(define_Computed_field_type_component);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		field_component.field=(struct Computed_field *)NULL;
		if (COMPUTED_FIELD_COMPONENT==Computed_field_get_type(field))
		{
			return_code=Computed_field_get_type_component(field,
				&field_component.field,&field_component.component_no);
		}
		else
		{
			if (field_component.field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_has_numerical_components,(void *)NULL,
				computed_field_package->computed_field_manager))
			{
				field_component.component_no=0;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_component.  No fields defined");
				return_code=0;
			}
		}
		if (return_code)
		{
			(option_table[0]).to_be_modified= &field_component;
			(option_table[0]).user_data=
				computed_field_package->computed_field_manager;
			return_code=process_multiple_options(state,option_table)&&
				Computed_field_set_type_component(field,field_component.field,
					field_component.component_no);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_component.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_component */

static int define_Computed_field_type_compose(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 15 October 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_COMPOSE (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry 
		option_table[]=
		{
			{"calculate_values_field",NULL,NULL,set_Computed_field_conditional},
			{"find_element_xi_field",NULL,NULL,set_Computed_field_conditional},
			{"group",NULL,NULL,set_FE_element_group},
			{"texture_coordinates_field",NULL,NULL,set_Computed_field_conditional},
			{NULL,NULL,NULL,NULL}
		};
	struct Computed_field *field,*calculate_values_field,*find_element_xi_field,
		*texture_coordinates_field;
	struct Computed_field_package *computed_field_package;
	struct Coordinate_system *coordinate_system_ptr;
	struct GROUP(FE_element) *search_element_group;
	struct Set_Computed_field_conditional_data set_calculate_values_field_data,
		set_find_element_xi_field_data, set_texture_coordinates_field_data;

	ENTER(define_Computed_field_type_compose);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		set_calculate_values_field_data.computed_field_manager=
			computed_field_package->computed_field_manager;
		set_calculate_values_field_data.conditional_function=
			Computed_field_has_numerical_components;
		set_calculate_values_field_data.conditional_function_user_data=(void *)NULL;
		set_find_element_xi_field_data.computed_field_manager=
			computed_field_package->computed_field_manager;
		set_find_element_xi_field_data.conditional_function=
			Computed_field_is_find_element_xi_capable;
		set_find_element_xi_field_data.conditional_function_user_data=(void *)NULL;
		set_texture_coordinates_field_data.computed_field_manager=
			computed_field_package->computed_field_manager;
		set_texture_coordinates_field_data.conditional_function=
			Computed_field_has_numerical_components;
		set_texture_coordinates_field_data.conditional_function_user_data=(void *)NULL;
		search_element_group = (struct GROUP(FE_element) *)NULL;
		/* get valid parameters for composite field */
		if (COMPUTED_FIELD_COMPOSE==Computed_field_get_type(field))
		{
			return_code=Computed_field_get_type_compose(field,
				&calculate_values_field, &find_element_xi_field,
				&texture_coordinates_field, &search_element_group);
		}
		else
		{
			calculate_values_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_has_numerical_components ,(void *)NULL,
				computed_field_package->computed_field_manager);
			find_element_xi_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_is_find_element_xi_capable,(void *)NULL,
				computed_field_package->computed_field_manager);
			texture_coordinates_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_has_numerical_components ,(void *)NULL,
				computed_field_package->computed_field_manager);
		}
		if (return_code)
		{
			/* ACCESS the fields for set_Computed_field */
			if (calculate_values_field)
			{
				ACCESS(Computed_field)(calculate_values_field);
			}
			if (find_element_xi_field)
			{
				ACCESS(Computed_field)(find_element_xi_field);
			}
			if (search_element_group)
			{
				ACCESS(GROUP(FE_element))(search_element_group);
			}
			if (texture_coordinates_field)
			{
				ACCESS(Computed_field)(texture_coordinates_field);
			}
			/* parse the scalars */
			if (return_code&&state->current_token)
			{
				(option_table[0]).to_be_modified= &calculate_values_field;
				(option_table[0]).user_data= &set_calculate_values_field_data;
				(option_table[1]).to_be_modified= &find_element_xi_field;
				(option_table[1]).user_data= &set_find_element_xi_field_data;
				(option_table[2]).to_be_modified= &search_element_group;
				(option_table[2]).user_data= computed_field_package->fe_element_manager;
				(option_table[3]).to_be_modified= &texture_coordinates_field;
				(option_table[3]).user_data= &set_texture_coordinates_field_data;
				return_code=process_multiple_options(state,option_table);
			}
			if (return_code)
			{
				if (return_code=Computed_field_set_type_compose(field,
					texture_coordinates_field, find_element_xi_field,
					calculate_values_field, search_element_group))
				{
					/* Set default coordinate system */
					/* Inherit from third source field */
					coordinate_system_ptr = 
						Computed_field_get_coordinate_system(calculate_values_field);
					Computed_field_set_coordinate_system(field, coordinate_system_ptr);
				}
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_compose.  Failed");
				}
			}
			if (calculate_values_field)
			{
				DEACCESS(Computed_field)(&calculate_values_field);
			}
			if (find_element_xi_field)
			{
				DEACCESS(Computed_field)(&find_element_xi_field);
			}
			if (search_element_group)
			{
				DEACCESS(GROUP(FE_element))(&search_element_group);
			}
			if (texture_coordinates_field)
			{
				DEACCESS(Computed_field)(&texture_coordinates_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_compose.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_compose */

static int define_Computed_field_type_constant(struct Parse_state *state,
	void *field_void,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 22 June 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CONSTANT (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	char *current_token;
	FE_value *values,*temp_values;
	int i,number_of_values,return_code,temp_number_of_values;
	static struct Modifier_entry 
		number_of_values_option_table[]=
		{
			{"number_of_values",NULL,NULL,set_int_positive},
			{NULL,NULL,NULL,NULL}
		},
		values_option_table[]=
		{
			{"values",NULL,NULL,set_FE_value_array},
			{NULL,NULL,NULL,NULL}
		},
		help_option_table[]=
		{
			{"number_of_values",NULL,NULL,set_int_positive},
			{"values",NULL,NULL,set_FE_value_array},
			{NULL,NULL,NULL,NULL}
		};
	struct Computed_field *field;

	ENTER(define_Computed_field_type_constant);
	USE_PARAMETER(dummy_void);
	if (state&&(field=(struct Computed_field *)field_void))
	{
		return_code=1;
		/* get valid parameters for constant field */
		values=(FE_value *)NULL;
		if (COMPUTED_FIELD_CONSTANT==Computed_field_get_type(field))
		{
			return_code=Computed_field_get_type_constant(field,
				&number_of_values,&values);
		}
		else
		{
			/* ALLOCATE and fill array of values - with zeroes */
			number_of_values=1;
			if (ALLOCATE(values,FE_value,number_of_values))
			{
				for (i=0;i<number_of_values;i++)
				{
					values[i]=0.0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_constant.  Not enough memory");
				return_code=0;
			}
		}
		if (return_code)
		{
			/* try to handle help first */
			if (current_token=state->current_token)
			{
				if (!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
				{
					(help_option_table[0]).to_be_modified= &number_of_values;
					(help_option_table[1]).to_be_modified= values;
					(help_option_table[1]).user_data= &number_of_values;
					return_code=process_multiple_options(state,help_option_table);
				}
			}
			/* parse the number_of_values... */
			if (return_code&&(current_token=state->current_token))
			{
				/* ... only if the "number_of_values" token is next */
				if (fuzzy_string_compare(current_token,"number_of_values"))
				{
					/* keep the number_of_values to maintain any current ones */
					temp_number_of_values=number_of_values;
					(number_of_values_option_table[0]).to_be_modified=
						&temp_number_of_values;
					if (return_code=process_option(state,number_of_values_option_table))
					{
						if (temp_number_of_values != number_of_values)
						{
							if (REALLOCATE(temp_values,values,FE_value,temp_number_of_values))
							{
								values=temp_values;
								/* clear any new values to zero */
								for (i=number_of_values;i<temp_number_of_values;i++)
								{
									values[i]=0.0;
								}
								number_of_values=temp_number_of_values;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"define_Computed_field_type_constant.  Not enough memory");
								return_code=0;
							}
						}
					}
				}
			}
			/* parse the values */
			if (return_code&&state->current_token)
			{
				(values_option_table[0]).to_be_modified= values;
				(values_option_table[0]).user_data= &number_of_values;
				return_code=process_multiple_options(state,values_option_table);
			}
			if (return_code)
			{
				return_code=
					Computed_field_set_type_constant(field,number_of_values,values);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_constant.  Failed");
				}
			}
			/* clean up the values array */
			DEALLOCATE(values);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_constant.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_constant */

static int define_Computed_field_type_cubic_texture_coordinates(
	struct Parse_state *state,void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 10 February 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CUBIC_TEXTURE_COORDINATES
(if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"field",NULL,NULL,set_Computed_field_conditional},
		{NULL,NULL,NULL,NULL}
	};
	struct Computed_field *source_field,*field;
	struct Computed_field_package *computed_field_package;
	struct Set_Computed_field_conditional_data set_field_data;

	ENTER(define_Computed_field_type_cubic_texture_coordinates);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		source_field=(struct Computed_field *)NULL;
		if (COMPUTED_FIELD_CUBIC_TEXTURE_COORDINATES==Computed_field_get_type(field))
		{
			return_code=Computed_field_get_type_cubic_texture_coordinates(field,
				&source_field);
		}
		else
		{
			if (!(source_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_has_numerical_components,(void *)NULL,
				computed_field_package->computed_field_manager)))
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_cubic_texture_coordinates.  No fields defined");
				return_code=0;
			}
		}
		if (return_code)
		{
			/* have ACCESS/DEACCESS because set_Computed_field does */
			ACCESS(Computed_field)(source_field);
			set_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_field_data.conditional_function_user_data=(void *)NULL;
			set_field_data.computed_field_manager=
            computed_field_package->computed_field_manager;
			(option_table[0]).to_be_modified= &source_field;
			(option_table[0]).user_data= &set_field_data;
			return_code=process_multiple_options(state,option_table)&&
				Computed_field_set_type_cubic_texture_coordinates(field,source_field);
			DEACCESS(Computed_field)(&source_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_cubic_texture_coordinates.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_cubic_texture_coordinates */

int define_Computed_field_type_edit_mask(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 20 August 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_EDIT_MASK (if it is not already)
and allows its contents to be modified.
Must input the field before the edit_mask since there will be as many
edit_mask flags as there are components in the field.
==============================================================================*/
{
	auto struct Modifier_entry
		field_option_table[]=
		{
			{"field",NULL,NULL,set_Computed_field_conditional},
			{NULL,NULL,NULL,NULL}
		},
		edit_mask_option_table[]=
		{
			{"edit_mask",NULL,NULL,set_FE_value_array},
			{NULL,NULL,NULL,NULL}
		},
		help_option_table[]=
		{
			{"field",NULL,NULL,set_Computed_field_conditional},
			{"edit_mask",NULL,NULL,set_FE_value_array},
			{NULL,NULL,NULL,NULL}
		};
	char *current_token;
	FE_value *temp_edit_mask,*edit_mask;
	int i,number_of_components,return_code;
	struct Computed_field *field,*source_field;
	struct Computed_field_package *computed_field_package;
	struct Set_Computed_field_conditional_data set_field_data;

	ENTER(define_Computed_field_type_edit_mask);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		set_field_data.conditional_function=
			Computed_field_has_numerical_components;
		set_field_data.conditional_function_user_data=(void *)NULL;
		set_field_data.computed_field_manager=
			computed_field_package->computed_field_manager;
		/* get source_field and edit_mask - from field if of type edit_mask */
		source_field=(struct Computed_field *)NULL;
		edit_mask=(FE_value *)NULL;
		if (COMPUTED_FIELD_EDIT_MASK==field->type)
		{
			return_code=Computed_field_get_type_edit_mask(field,
				&source_field,&edit_mask);
		}
		else
		{
			/* get first available field, and set edit_mask for it to 1.0 */
			if ((source_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_has_numerical_components,(void *)NULL,
				computed_field_package->computed_field_manager))&&
				ALLOCATE(edit_mask,FE_value,source_field->number_of_components))
			{
				for (i=0;i<source_field->number_of_components;i++)
				{
					edit_mask[i]=1.0;
				}
			}
			else
			{
				return_code=0;
			}
		}
		if (return_code)
		{
			/* must access source_field for set_Computed_field */
			ACCESS(Computed_field)(source_field);
			/* try to handle help first */
			if (current_token=state->current_token)
			{
				if (!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
				{
					number_of_components=source_field->number_of_components;
					(help_option_table[0]).to_be_modified= &source_field;
					(help_option_table[0]).user_data= &set_field_data;
					(help_option_table[1]).to_be_modified= edit_mask;
					(help_option_table[1]).user_data= &number_of_components;
					return_code=process_multiple_options(state,help_option_table);
				}
			}
			/* parse the field ... */
			if (return_code&&(current_token=state->current_token))
			{
				/* ... only if the "field" token is next */
				if (fuzzy_string_compare(current_token,"field"))
				{
					/* save the number of components to maintain current edit_mask */
					number_of_components=source_field->number_of_components;
					(field_option_table[0]).to_be_modified= &source_field;
					(field_option_table[0]).user_data= &set_field_data;
					if (return_code=process_option(state,field_option_table))
					{
						if (source_field)
						{
							if (REALLOCATE(temp_edit_mask,edit_mask,FE_value,
								source_field->number_of_components))
							{
								edit_mask=temp_edit_mask;
								/* make any new edit_mask equal to 1.0 */
								for (i=number_of_components;
									i<source_field->number_of_components;i++)
								{
									edit_mask[i]=1.0;
								}
							}
							else
							{
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"define_Computed_field_type_edit_mask.  Invalid field");
							return_code=0;
						}
					}
				}
			}
			/* parse the edit_mask */
			if (return_code&&state->current_token)
			{
				number_of_components=source_field->number_of_components;
				(edit_mask_option_table[0]).to_be_modified= edit_mask;
				(edit_mask_option_table[0]).user_data= &number_of_components;
				return_code=process_multiple_options(state,edit_mask_option_table);
			}
			if (return_code)
			{
				return_code=
					Computed_field_set_type_edit_mask(field,source_field,edit_mask);
				Computed_field_set_coordinate_system_from_sources(field);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_edit_mask.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			if (edit_mask)
			{
				DEALLOCATE(edit_mask);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_edit_mask.  "
				"Could not get source_field or edit_mask");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_edit_mask.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_edit_mask */

static int define_Computed_field_type_external(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 25 August 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_EXTERNAL (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	char *current_token, *filename;
	int i,number_of_fields,number_of_source_values,return_code,
		temp_number_of_fields, temp_number_of_source_values,
		timeout;
	FE_value *source_values, *temp_source_values;
	static struct Modifier_entry 
		number_of_fields_option_table[]=
		{
			{"number_of_fields",NULL,NULL,set_int_non_negative},
			{"number_of_values",NULL,NULL,set_int_non_negative},
			{NULL,NULL,NULL,NULL}
		},
		fields_option_table[]=
		{
			{"fields",NULL,NULL,set_Computed_field_array},
			{"filename",NULL,(void *)1,set_name},
			{"timeout",NULL,NULL,set_int_positive},
			{"values",NULL,NULL,set_FE_value_array},
			{NULL,NULL,NULL,NULL}
		},
		help_option_table[]=
		{
			{"number_of_fields",NULL,NULL,set_int_non_negative},
			{"number_of_values",NULL,NULL,set_int_non_negative},
			{"fields",NULL,NULL,set_Computed_field_array},
			{"filename",NULL,(void *)1,set_name},
			{"timeout",NULL,NULL,set_int_positive},
			{"values",NULL,NULL,set_FE_value_array},
			{NULL,NULL,NULL,NULL}
		};
	struct Computed_field *field,**source_fields,*temp_field,**temp_source_fields;
	struct Computed_field_package *computed_field_package;
	struct Set_Computed_field_array_data set_source_field_array_data;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_external);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		filename = (char *)NULL;
		return_code=1;
		set_source_field_data.computed_field_manager=
			computed_field_package->computed_field_manager;
		set_source_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data=(void *)NULL;
		/* get valid parameters for external field */
		source_fields=(struct Computed_field **)NULL;
		number_of_source_values = 0;
		source_values = (FE_value *)NULL;
		timeout = 5;
		if (COMPUTED_FIELD_EXTERNAL==Computed_field_get_type(field))
		{
			return_code=Computed_field_get_type_external(field,
				&filename, &timeout,
				&number_of_source_values,&source_values,
				&number_of_fields,&source_fields);
		}
		else
		{
			/* ALLOCATE and fill array of source fields */
			temp_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_has_numerical_components,(void *)NULL,
				computed_field_package->computed_field_manager);
			number_of_fields=1;
			if (ALLOCATE(source_fields,struct Computed_field *,number_of_fields))
			{
				for (i=0;i<number_of_fields;i++)
				{
					source_fields[i]=temp_field;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_external.  Not enough memory");
				return_code=0;
			}
		}
		if (return_code)
		{
			/* ACCESS the source fields for set_Computed_field_array */
			for (i=0;i<number_of_fields;i++)
			{
				if (source_fields[i])
				{
					ACCESS(Computed_field)(source_fields[i]);
				}
			}
			/* try to handle help first */
			if (current_token=state->current_token)
			{
				if (!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
				{
					(help_option_table[0]).to_be_modified= &number_of_fields;
					(help_option_table[1]).to_be_modified= &number_of_source_values;
					set_source_field_array_data.number_of_fields=number_of_fields;
					set_source_field_array_data.conditional_data= &set_source_field_data;
					(help_option_table[2]).to_be_modified= source_fields;
					(help_option_table[2]).user_data= &set_source_field_array_data;
					(help_option_table[3]).to_be_modified= &filename;
					(help_option_table[4]).to_be_modified= &timeout;
					(help_option_table[5]).to_be_modified= source_values;
					(help_option_table[5]).user_data= &number_of_source_values;
					return_code=process_multiple_options(state,help_option_table);
				}
			}
			/* parse the number_of_fields... */
			if (return_code&&(current_token=state->current_token))
			{
				/* ... only if the "number_of_fields" token or 
					"number_of_values" is next */
				while (fuzzy_string_compare("number_of_",state->current_token))
				{
					/* keep the number_of_fields to maintain any current ones */
					temp_number_of_fields=number_of_fields;
					temp_number_of_source_values=number_of_source_values;
					(number_of_fields_option_table[0]).to_be_modified=
						&temp_number_of_fields;
					(number_of_fields_option_table[1]).to_be_modified=
						&temp_number_of_source_values;
					if (return_code=process_option(state,number_of_fields_option_table))
					{
						if (temp_number_of_fields != number_of_fields)
						{
							if (ALLOCATE(temp_source_fields,struct Computed_field *,
								temp_number_of_fields))
							{
								for (i=0;i<temp_number_of_fields;i++)
								{
									if (i<number_of_fields)
									{
										temp_field=source_fields[i];
									}
									/* new array members access last original source field */
									temp_source_fields[i]=ACCESS(Computed_field)(temp_field);
								}
								/* clean up the previous source_fields array */
								for (i=0;i<number_of_fields;i++)
								{
									DEACCESS(Computed_field)(&(source_fields[i]));
								}
								DEALLOCATE(source_fields);
								source_fields=temp_source_fields;
								number_of_fields=temp_number_of_fields;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"define_Computed_field_type_external.  Not enough memory");
								return_code=0;
							}
						}
						if (number_of_source_values != temp_number_of_source_values)
						{
							if (REALLOCATE(temp_source_values,source_values,FE_value,
								temp_number_of_source_values))
							{
								source_values=temp_source_values;
								/* make any new source values 0.0 */
								for (i=temp_number_of_source_values;
									i<number_of_source_values;i++)
								{
									source_values[i]=0.0;
								}
								number_of_source_values = temp_number_of_source_values;
							}
							else
							{
								return_code=0;
							}
						}
					}
				}
			}
			/* parse the fields */
			if (return_code&&state->current_token)
			{
				set_source_field_array_data.number_of_fields=number_of_fields;
				set_source_field_array_data.conditional_data= &set_source_field_data;
				(fields_option_table[0]).to_be_modified= source_fields;
				(fields_option_table[0]).user_data= &set_source_field_array_data;
				(fields_option_table[1]).to_be_modified= &filename;
				(fields_option_table[2]).to_be_modified= &timeout;
				(fields_option_table[3]).to_be_modified= source_values;
				(fields_option_table[3]).user_data= &number_of_source_values;
				return_code=process_multiple_options(state,fields_option_table);
			}
			if (return_code)
			{
				return_code=Computed_field_set_type_external(field,
					filename, timeout, number_of_source_values, source_values,
					number_of_fields,source_fields);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_external.  Failed");
				}
			}
			/* clean up the source fields array */
			for (i=0;i<number_of_fields;i++)
			{
				if (source_fields[i])
				{
					DEACCESS(Computed_field)(&(source_fields[i]));
				}
			}
			DEALLOCATE(source_fields);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_external.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_external */

static int define_Computed_field_type_rc_coordinate(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 26 January 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_RC_COORDINATE (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"coordinate",NULL,NULL,set_Computed_field_conditional},
		{NULL,NULL,NULL,NULL}
	};
	struct Computed_field *coordinate_field,*field;
	struct Computed_field_package *computed_field_package;
	struct Set_Computed_field_conditional_data set_coordinate_field_data;

	ENTER(define_Computed_field_type_rc_coordinate);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		coordinate_field=(struct Computed_field *)NULL;
		if (COMPUTED_FIELD_RC_COORDINATE==Computed_field_get_type(field))
		{
			return_code=Computed_field_get_type_rc_coordinate(field,
				&coordinate_field);
		}
		else
		{
			if (!((coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,
				name)("default_coordinate",
					computed_field_package->computed_field_manager))||
				(coordinate_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_has_up_to_3_numerical_components,(void *)NULL,
					computed_field_package->computed_field_manager))))
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_rc_coordinate.  "
					"No valid coordinate field available");
				return_code=0;
			}
		}
		if (return_code)
		{
			/* have ACCESS/DEACCESS because set_Computed_field does */
			ACCESS(Computed_field)(coordinate_field);
			set_coordinate_field_data.computed_field_manager=
				computed_field_package->computed_field_manager;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			(option_table[0]).to_be_modified= &coordinate_field;
			(option_table[0]).user_data= &set_coordinate_field_data;
			return_code=process_multiple_options(state,option_table)&&
				Computed_field_set_type_rc_coordinate(field,coordinate_field);
			DEACCESS(Computed_field)(&coordinate_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_rc_coordinate.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_rc_coordinate */

static int define_Computed_field_type_rc_vector(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 24 June 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_RC_VECTOR (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"coordinate",NULL,NULL,set_Computed_field_conditional},
		{"vector",NULL,NULL,set_Computed_field_conditional},
		{NULL,NULL,NULL,NULL}
	};
	struct Computed_field *coordinate_field,*field,*vector_field;
	struct Computed_field_package *computed_field_package;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_vector_field_data;

	ENTER(define_Computed_field_type_rc_vector);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		coordinate_field=(struct Computed_field *)NULL;
		vector_field=(struct Computed_field *)NULL;
		if (COMPUTED_FIELD_RC_VECTOR==Computed_field_get_type(field))
		{
			return_code=Computed_field_get_type_rc_vector(field,
				&vector_field,&coordinate_field);
		}
		else
		{
			if (!(((coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,
				name)("default_coordinate",
					computed_field_package->computed_field_manager))||
				(coordinate_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_has_up_to_3_numerical_components,(void *)NULL,
					computed_field_package->computed_field_manager)))&&
				((vector_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,
					name)("vectors",computed_field_package->computed_field_manager))||
				(vector_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_has_up_to_3_numerical_components,(void *)NULL,
					computed_field_package->computed_field_manager)))))
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_rc_vector.  "
					"No valid coordinate and/or vector field available");
				return_code=0;
			}
		}
		if (return_code)
		{
			/* have ACCESS/DEACCESS because set_Computed_field does */
			ACCESS(Computed_field)(coordinate_field);
			ACCESS(Computed_field)(vector_field);
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				computed_field_package->computed_field_manager;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			(option_table[0]).to_be_modified= &coordinate_field;
			(option_table[0]).user_data= &set_coordinate_field_data;
			/* vector */
			set_vector_field_data.computed_field_manager=
				computed_field_package->computed_field_manager;
			set_vector_field_data.conditional_function=
				Computed_field_is_orientation_scale_capable;
			set_vector_field_data.conditional_function_user_data=(void *)NULL;
			(option_table[1]).to_be_modified= &vector_field;
			(option_table[1]).user_data= &set_vector_field_data;
			return_code=process_multiple_options(state,option_table)&&
				Computed_field_set_type_rc_vector(field,vector_field,coordinate_field);
			DEACCESS(Computed_field)(&coordinate_field);
			DEACCESS(Computed_field)(&vector_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_rc_vector.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_rc_vector */

int define_Computed_field_type_sum_components(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_SUM_COMPONENTS (if it is not already)
and allows its contents to be modified.
Must input the field before the weights since there will be as many weights as
there are components in field.
==============================================================================*/
{
	auto struct Modifier_entry
		field_option_table[]=
		{
			{"field",NULL,NULL,set_Computed_field_conditional},
			{NULL,NULL,NULL,NULL}
		},
		weights_option_table[]=
		{
			{"weights",NULL,NULL,set_FE_value_array},
			{NULL,NULL,NULL,NULL}
		},
		help_option_table[]=
		{
			{"field",NULL,NULL,set_Computed_field_conditional},
			{"weights",NULL,NULL,set_FE_value_array},
			{NULL,NULL,NULL,NULL}
		};
	char *current_token;
	FE_value *temp_weights,*weights;
	int i,number_of_components,return_code;
	struct Computed_field *field,*source_field;
	struct Computed_field_package *computed_field_package;
	struct Set_Computed_field_conditional_data set_field_data;

	ENTER(define_Computed_field_type_sum_components);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		set_field_data.conditional_function=
			Computed_field_has_numerical_components;
		set_field_data.conditional_function_user_data=(void *)NULL;
		set_field_data.computed_field_manager=
			computed_field_package->computed_field_manager;
		/* get source_field and weights - from field if of type sum_components */
		source_field=(struct Computed_field *)NULL;
		weights=(FE_value *)NULL;
		if (COMPUTED_FIELD_SUM_COMPONENTS==field->type)
		{
			return_code=Computed_field_get_type_sum_components(field,
				&source_field,&weights);
		}
		else
		{
			/* get first available field, and set weights for it to 1.0 */
			if ((source_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_has_numerical_components,(void *)NULL,
				computed_field_package->computed_field_manager))&&
				ALLOCATE(weights,FE_value,source_field->number_of_components))
			{
				for (i=0;i<source_field->number_of_components;i++)
				{
					weights[i]=1.0;
				}
			}
			else
			{
				return_code=0;
			}
		}
		if (return_code)
		{
			/* must access source_field for set_Computed_field */
			ACCESS(Computed_field)(source_field);
			/* try to handle help first */
			if (current_token=state->current_token)
			{
				if (!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
				{
					number_of_components=source_field->number_of_components;
					(help_option_table[0]).to_be_modified= &source_field;
					(help_option_table[0]).user_data= &set_field_data;
					(help_option_table[1]).to_be_modified= weights;
					(help_option_table[1]).user_data= &number_of_components;
					return_code=process_multiple_options(state,help_option_table);
				}
			}
			/* parse the field ... */
			if (return_code&&(current_token=state->current_token))
			{
				/* ... only if the "field" token is next */
				if (fuzzy_string_compare(current_token,"field"))
				{
					/* save the number of components to maintain any current weights */
					number_of_components=source_field->number_of_components;
					(field_option_table[0]).to_be_modified= &source_field;
					(field_option_table[0]).user_data= &set_field_data;
					if (return_code=process_option(state,field_option_table))
					{
						if (source_field)
						{
							if (REALLOCATE(temp_weights,weights,FE_value,
								source_field->number_of_components))
							{
								weights=temp_weights;
								/* make any new weights equal to 1.0 */
								for (i=number_of_components;
									i<source_field->number_of_components;i++)
								{
									weights[i]=1.0;
								}
							}
							else
							{
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"define_Computed_field_type_sum_components.  Invalid field");
							return_code=0;
						}
					}
				}
			}
			/* parse the weights */
			if (return_code&&state->current_token)
			{
				number_of_components=source_field->number_of_components;
				(weights_option_table[0]).to_be_modified= weights;
				(weights_option_table[0]).user_data= &number_of_components;
				return_code=process_multiple_options(state,weights_option_table);
			}
			if (return_code)
			{
				return_code=
					Computed_field_set_type_sum_components(field,source_field,weights);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_sum_components.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			if (weights)
			{
				DEALLOCATE(weights);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_sum_components.  "
				"Could not get source_field or weights");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_sum_components.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_sum_components */

struct Add_type_to_option_table_data
{
	struct Option_table *option_table;
	void *field_void;
	void *computed_field_package_void;
};

static int Computed_field_add_type_to_option_table(struct Computed_field_type_data *type,
	void *add_type_to_option_table_data_void)
/*******************************************************************************
LAST MODIFIED : 3 July 2000

DESCRIPTION :
Adds <type> to the <option_table> so it is available to the commands.
==============================================================================*/
{
	int return_code;
	struct Add_type_to_option_table_data *data;

	ENTER(Computed_field_add_type_to_option_table);
	if (type&&(data=(struct Add_type_to_option_table_data *)
		add_type_to_option_table_data_void))
	{
		Option_table_add_entry(data->option_table,type->name,data->field_void,
			type->define_type_user_data,
			type->define_Computed_field_type_function);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_add_type_to_option_table.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_add_type_to_option_table */

static int define_Computed_field_type(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 23 March 2000

DESCRIPTION :
Part of the group of define_Computed_field functions. Here, we already have the
<field> to be modified and have determined the number of components and
coordinate system, and must now determine the type of computed field function
and its parameter fields and values.
#### Must ensure implemented correctly for new Computed_field_type. ####
==============================================================================*/
{
	int return_code;
	struct Add_type_to_option_table_data data;
	struct Computed_field_package *computed_field_package;
	struct Option_table *option_table;

	ENTER(define_Computed_field_type);
	if (state && (computed_field_package = (struct Computed_field_package *)
		computed_field_package_void))
	{
		if (state->current_token)
		{
			option_table=CREATE(Option_table)();
			/* component */
			Option_table_add_entry(option_table,"component",field_void,
				computed_field_package_void,define_Computed_field_type_component);
			/* compose */
			Option_table_add_entry(option_table,"compose",field_void,
				computed_field_package_void,define_Computed_field_type_compose);
			/* constant */
			Option_table_add_entry(option_table,"constant",field_void,
				computed_field_package_void,define_Computed_field_type_constant);
			/* cubic_texture_coordinates */
			Option_table_add_entry(option_table,"cubic_texture_coordinates",
				field_void,computed_field_package_void,
				define_Computed_field_type_cubic_texture_coordinates);
			/* edit_mask */
			Option_table_add_entry(option_table,"edit_mask",field_void,
				computed_field_package_void,define_Computed_field_type_edit_mask);
			/* external */
			Option_table_add_entry(option_table,"external",field_void,
				computed_field_package_void,define_Computed_field_type_external);
			/* rc_coordinate */
			Option_table_add_entry(option_table,"rc_coordinate",field_void,
				computed_field_package_void,define_Computed_field_type_rc_coordinate);
			/* rc_vector */
			Option_table_add_entry(option_table,"rc_vector",field_void,
				computed_field_package_void,define_Computed_field_type_rc_vector);
			/* sum_components */
			Option_table_add_entry(option_table,"sum_components",field_void,
				computed_field_package_void,define_Computed_field_type_sum_components);
			/* new_types */
			data.option_table = option_table;
			data.field_void = field_void;
			data.computed_field_package_void = computed_field_package_void;
			FOR_EACH_OBJECT_IN_LIST(Computed_field_type_data)(
				Computed_field_add_type_to_option_table, (void *)&data,
				computed_field_package->computed_field_type_list);

			return_code=Option_table_parse(option_table,state);
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			/* OK if no more modifications */
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"define_Computed_field_type.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type */

int
define_Computed_field_coordinate_system(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 4 November 1999

DESCRIPTION :
Modifier entry function acting as an optional prerequisite for settings the
coordinate_system. That means that if the token "coordinate_system" or part
thereof is found in the current token, then the coordinate system is read. In
each case, assuming no error has occurred, control passes to the next parsing
level, defining the type of computed field function.  Then, if the
coordinate_system was not explictly stated, it is set in accordance with the type.
Function assumes that <field> is not currently managed, as it would be illegal
to modify it if it was.
#### Must ensure implemented correctly for new Computed_field_type. ####
==============================================================================*/
{
	auto struct Modifier_entry
		option_table[]=
		{
			{"coordinate_system",NULL,NULL,set_Coordinate_system},
			{NULL,NULL,NULL,NULL}
		},
		help_option_table[]=
		{
			{"[coordinate_system NAME]",NULL,NULL,define_Computed_field_type},
			{NULL,NULL,NULL,NULL}
		};
	char *current_token;
	struct Coordinate_system coordinate_system;
	int return_code;
	struct Computed_field *field;

	ENTER(define_Computed_field_coordinate_system);
	if (state&&(field=(struct Computed_field *)field_void))
	{
		if (current_token=state->current_token)
		{
			COPY(Coordinate_system)(&coordinate_system,
				Computed_field_get_coordinate_system(field));
			/* read the optional cooordinate_system NAME [focus #] parameter */
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (fuzzy_string_compare(current_token,"coordinate_system"))
				{
					/* parse coordinate_system */
					(option_table[0]).to_be_modified= &coordinate_system;
					if(return_code=process_option(state,option_table))
					{
						if(return_code=define_Computed_field_type(state,field_void,
							computed_field_package_void))
						{
							/* Override the type set by define_Computed_field_type */
							return_code = Computed_field_set_coordinate_system(
								field,&coordinate_system);
						}
					}
				}
				else
				{
					/* Default coordinate system should be set when type is defined */
					return_code=define_Computed_field_type(state,field_void,
						computed_field_package_void);
				}
			}
			else
			{
				/* write help */
				(help_option_table[0]).to_be_modified=field_void;
				(help_option_table[0]).user_data=computed_field_package_void;
				return_code=process_option(state,help_option_table);
				return_code=1;
			}
		}
		else
		{
			/* OK if no more modifications */
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_coordinate_system.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_coordinate_system */

int define_Computed_field(struct Parse_state *state,void *dummy_to_be_modified,
	void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 18 June 1999

DESCRIPTION :
Modifier entry function for creating and modifying Computed_fields. Format for
parameters from the parse state are:
  FIELD_NAME|NEW_FIELD_NAME
    rectangular_cartesian/cylindrical_polar/spherical_polar/prolate_sph...
      component
        FIELD_NAME.COMPONENT_NAME
      composite
        number_of_scalars #
		    scalars FIELD_NAME FIELD_NAME... FIELD_NAME{number_of_scalars}
      gradient
			  scalar FIELD_NAME
				coordinate FIELD_NAME
      rc_coordinate
				coordinate FIELD_NAME
      scale
		    field FIELD_NAME
				values # # ... #{number of components in field}
      ... (more as more types added)
Note that the above layout is used because:
1. The number_of_components is often prerequisite information for setting up
the modifier functions for certain types of computed field, eg. "composite"
requires as many scalar fields as there are components, while scale has as many
FE_values.
2. The number_of_components and coordinate system are options for all types of
computed field so it makes sense that they are set before splitting up into the
options for the various types.
The <field_copy_void> parameter, if set, points to the field we are to modify
and should not itself be managed.
==============================================================================*/
{
	auto struct Modifier_entry
		help_option_table[]=
		{
			{"FIELD_NAME",NULL,NULL,define_Computed_field_coordinate_system},
			{NULL,NULL,NULL,NULL}
		};
	char *current_token;
	int return_code;
	struct Computed_field *existing_field,*temp_field;
	struct Computed_field_package *computed_field_package;

	ENTER(define_Computed_field);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (computed_field_package=
			(struct Computed_field_package *)computed_field_package_void)
		{
			return_code=1;
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					existing_field=
						FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(current_token,
							computed_field_package->computed_field_manager);
					/* create temp_field with the supplied name for working on */
					if (temp_field=CREATE(Computed_field)(current_token))
					{
						ACCESS(Computed_field)(temp_field);
						if (existing_field)
						{
							if (!MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_field,name)(
								temp_field,existing_field))
							{
								display_message(ERROR_MESSAGE,
									"define_Computed_field.  Could not copy existing field");
								return_code=0;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"define_Computed_field.  Could not build temporary field");
						return_code=0;
					}
					shift_Parse_state(state,1);
					if (return_code&&define_Computed_field_coordinate_system(state,
						temp_field,computed_field_package_void))
					{
						/* FINITE_ELEMENT computed field wrappers are not defined here -
							 done automatically in response to FE_field manager messages,
							 so they always return false */
						if (existing_field)
						{
							if (Computed_field_is_read_only(existing_field))
							{
								display_message(ERROR_MESSAGE,
									"Not allowed to modify read-only field");
								return_code=0;
							}
							else
							{
								/* copy modifications to existing_field */
								return_code=
									MANAGER_MODIFY_NOT_IDENTIFIER(Computed_field,name)(
										existing_field,temp_field,
										computed_field_package->computed_field_manager);
							}
						}
						else
						{
							/* add the new field to the manager */
							if (!ADD_OBJECT_TO_MANAGER(Computed_field)(
								temp_field,
								computed_field_package->computed_field_manager))
							{
								display_message(ERROR_MESSAGE,
									"define_Computed_field.  Unable to add field to manager");
							}
						}
					}
					if (temp_field)
					{
						DEACCESS(Computed_field)(&temp_field);
					}
				}
				else
				{
					if (temp_field=CREATE(Computed_field)("dummy"))
					{
						(help_option_table[0]).to_be_modified=(void *)temp_field;
						(help_option_table[0]).user_data=computed_field_package_void;
						return_code=process_option(state,help_option_table);
						DESTROY(Computed_field)(&temp_field);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"define_Computed_field.  Could not create dummy field");
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Missing field name");
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field.  Missing computed_field_package_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field */

int equivalent_computed_fields_at_elements(struct FE_element *element_1,
	struct FE_element *element_2)
/*******************************************************************************
LAST MODIFIED : 10 September 2001

DESCRIPTION :
Returns true if all fields are defined in the same way at the two elements.
==============================================================================*/
{
	int return_code;

	ENTER(equivalent_computed_fields_at_elements);
	return_code=0;
	if (element_1&&element_2)
	{
		return_code=equivalent_FE_fields_in_elements(element_1,element_2);
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* equivalent_computed_fields_at_elements */

int equivalent_computed_fields_at_nodes(struct FE_node *node_1,
	struct FE_node *node_2)
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
Returns true if all fields are defined in the same way at the two nodes.
==============================================================================*/
{
	int return_code;

	ENTER(equivalent_computed_fields_at_nodes);
	return_code=0;
	if (node_1&&node_2)
	{
		return_code=equivalent_FE_fields_at_nodes(node_1,node_2);
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* equivalent_computed_fields_at_nodes */

int list_Computed_field(struct Computed_field *field,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
Writes the properties of the <field> to the command window.
#### Must ensure implemented correctly for new Computed_field_type. ####
==============================================================================*/
{
	char *component_name,*temp_string;
	int i,return_code;

	ENTER(list_Computed_field);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=1;
		display_message(INFORMATION_MESSAGE,"field : %s\n",field->name);
		display_message(INFORMATION_MESSAGE,"  number_of_components = %d\n",
			field->number_of_components);
		if (temp_string=Coordinate_system_string(&field->coordinate_system))
		{
			display_message(INFORMATION_MESSAGE,"  coordinate_system = %s\n",
				temp_string);
			DEALLOCATE(temp_string);
		}
		display_message(INFORMATION_MESSAGE,"  field type = %s\n",
			Computed_field_type_to_string(field));
		if (COMPUTED_FIELD_NEW_TYPES == field->type)
		{
			if (field->list_Computed_field_function)
			{
				field->list_Computed_field_function(field);
			}
			else
			{
				display_message(ERROR_MESSAGE,"list_Computed_field.  "
					"Function not defined.");
				return_code=0;
			}
		}
		else
		{
			switch (field->type)
			{
				case COMPUTED_FIELD_COMPONENT:
				{
					if (component_name=Computed_field_get_component_name(
						field->source_fields[0],field->component_no))
					{
						display_message(INFORMATION_MESSAGE,"    component : %s.%s\n",
							field->source_fields[0]->name,component_name);
						DEALLOCATE(component_name);
					}
					else
					{
						return_code=0;
					}
				} break;
				case COMPUTED_FIELD_COMPOSE:
				{
					display_message(INFORMATION_MESSAGE,"    texture coordinates field :");
					display_message(INFORMATION_MESSAGE," %s\n",
						field->source_fields[0]->name);
					display_message(INFORMATION_MESSAGE,"    find element xi field :");
					display_message(INFORMATION_MESSAGE," %s\n",
						field->source_fields[1]->name);
					display_message(INFORMATION_MESSAGE,"    calculate values field :");
					display_message(INFORMATION_MESSAGE," %s\n",
						field->source_fields[2]->name);
				} break;
				case COMPUTED_FIELD_CONSTANT:
				{
					display_message(INFORMATION_MESSAGE,"    number_of_values : %d\n",
						field->number_of_source_values);
					display_message(INFORMATION_MESSAGE,"    values :");
					for (i=0;i<field->number_of_source_values;i++)
					{
						display_message(INFORMATION_MESSAGE," %g",field->source_values[i]);
					}
					display_message(INFORMATION_MESSAGE,"\n");
				} break;
				case COMPUTED_FIELD_CUBIC_TEXTURE_COORDINATES:
				{
					display_message(INFORMATION_MESSAGE,
						"    field : %s\n",field->source_fields[0]->name);
				} break;
				case COMPUTED_FIELD_RC_VECTOR:
				{
					display_message(INFORMATION_MESSAGE,
						"    coordinate field : %s\n",field->source_fields[1]->name);
					display_message(INFORMATION_MESSAGE,
						"    vector field : %s\n",field->source_fields[0]->name);
				} break;
				case COMPUTED_FIELD_EDIT_MASK:
				{
					display_message(INFORMATION_MESSAGE,"    field : %s\n",
						field->source_fields[0]->name);
					display_message(INFORMATION_MESSAGE,"    edit mask :");
					for (i=0;i<field->source_fields[0]->number_of_components;i++)
					{
						display_message(INFORMATION_MESSAGE," %g",field->source_values[i]);
					}
					display_message(INFORMATION_MESSAGE,"\n");
				} break;
				case COMPUTED_FIELD_EXTERNAL:
				{
					display_message(INFORMATION_MESSAGE,"    external_filename : %s\n",
						field->child_filename);
					display_message(INFORMATION_MESSAGE,"    number_of_source_values : %d\n",
						field->number_of_source_values);
					if (field->number_of_source_values)
					{
						display_message(INFORMATION_MESSAGE,"    source_values :");
						for (i=0;i<field->number_of_source_values;i++)
						{
							display_message(INFORMATION_MESSAGE," %f",
								field->source_values[i]);
						}
						display_message(INFORMATION_MESSAGE,"\n");
					}
					display_message(INFORMATION_MESSAGE,"    number_of_source_fields : %d\n",
						field->number_of_source_fields);
					if (field->number_of_source_fields)
					{				
						display_message(INFORMATION_MESSAGE,"    source_fields :");
						for (i=0;i<field->number_of_source_fields;i++)
						{
							display_message(INFORMATION_MESSAGE," %s",
								field->source_fields[i]->name);
						}
						display_message(INFORMATION_MESSAGE,"\n");
					}
					display_message(INFORMATION_MESSAGE,"    timeout %d\n",
						field->timeout);
				} break;
				case COMPUTED_FIELD_RC_COORDINATE:
				{
					display_message(INFORMATION_MESSAGE,
						"    coordinate field : %s\n",field->source_fields[0]->name);
				} break;
				case COMPUTED_FIELD_SUM_COMPONENTS:
				{
					display_message(INFORMATION_MESSAGE,"    field : %s\n",
						field->source_fields[0]->name);
					display_message(INFORMATION_MESSAGE,"    weights :");
					for (i=0;i<field->source_fields[0]->number_of_components;i++)
					{
						display_message(INFORMATION_MESSAGE," %g",field->source_values[i]);
					}
					display_message(INFORMATION_MESSAGE,"\n");
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"list_Computed_field.  Unknown field type");
					return_code=0;
				}
			}
		}
		/* write the names of the components */
		if (1<field->number_of_components)
		{
			display_message(INFORMATION_MESSAGE,"  component names:");
			for (i=0;i<field->number_of_components;i++)
			{
				if (component_name=Computed_field_get_component_name(field,i))
				{
					if (0<i)
					{
						display_message(INFORMATION_MESSAGE,",");
					}
					display_message(INFORMATION_MESSAGE," %s",component_name);
					DEALLOCATE(component_name);
				}
			}
			display_message(INFORMATION_MESSAGE,"\n");
		}
		if (field->read_only)
		{
			display_message(INFORMATION_MESSAGE,"  field is read only\n");
		}
		else
		{
			display_message(INFORMATION_MESSAGE,"  field is read/write\n");
		}
		display_message(INFORMATION_MESSAGE,"  (access count = %d)\n",
			field->access_count);
	}
	else
	{
		display_message(ERROR_MESSAGE,"list_Computed_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field */

int list_Computed_field_commands(struct Computed_field *field,
	void *command_prefix_void)
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
Writes the commands needed to reproduce <field> to the command window. Note that
only fields without the read_only flag set are written out, since the others
are created automatically by the program.
#### Must ensure implemented correctly for new Computed_field_type. ####
==============================================================================*/
{
	char *command_prefix,*component_name,*temp_string;
	int i,return_code;

	ENTER(list_Computed_field_commands);
	if (field&&(command_prefix=(char *)command_prefix_void))
	{
		/* don't list fields if read-only = automatically created by cmgui */
		if (!field->read_only)
		{
			display_message(INFORMATION_MESSAGE,"%s%s",command_prefix,field->name);
			if (temp_string=Coordinate_system_string(&field->coordinate_system))
			{
				display_message(INFORMATION_MESSAGE," coordinate_system %s",
					temp_string);
				DEALLOCATE(temp_string);
			}
			display_message(INFORMATION_MESSAGE," %s",
				Computed_field_type_to_string(field));
			if (COMPUTED_FIELD_NEW_TYPES == field->type)
			{
				if (field->list_Computed_field_commands_function)
				{
					field->list_Computed_field_commands_function(field);
				}
				else
				{
					display_message(ERROR_MESSAGE,"list_Computed_field_commands.  "
						"Function not defined.");
					return_code=0;
				}
			}
			else
			{
				switch (field->type)
				{
					case COMPUTED_FIELD_COMPONENT:
					{
						if (component_name=Computed_field_get_component_name(
							field->source_fields[0],field->component_no))
						{
							display_message(INFORMATION_MESSAGE," %s.%s",
								field->source_fields[0]->name,component_name);
							DEALLOCATE(component_name);
						}
						else
						{
							return_code=0;
						}
					} break;
					case COMPUTED_FIELD_COMPOSE:
					{
						display_message(INFORMATION_MESSAGE," texture_coordinates_field %s"
							" find_element_xi_field %s calculate_values_field %s",
							field->source_fields[0]->name, field->source_fields[1]->name,
							field->source_fields[2]->name);
					} break;
					case COMPUTED_FIELD_CONSTANT:
					{
						display_message(INFORMATION_MESSAGE," number_of_values %d values",
							field->number_of_source_values);
						for (i=0;i<field->number_of_source_values;i++)
						{
							display_message(INFORMATION_MESSAGE," %g",field->source_values[i]);
						}
					} break;
					case COMPUTED_FIELD_CUBIC_TEXTURE_COORDINATES:
					{
						display_message(INFORMATION_MESSAGE,
							" field %s",field->source_fields[0]->name);
					} break;
					case COMPUTED_FIELD_RC_VECTOR:
					{
						display_message(INFORMATION_MESSAGE,
							" coordinate %s vector %s",field->source_fields[1]->name,
							field->source_fields[0]->name);
					} break;
					case COMPUTED_FIELD_EDIT_MASK:
					{
						display_message(INFORMATION_MESSAGE," field %s edit_mask",
							field->source_fields[0]->name);
						for (i=0;i<field->source_fields[0]->number_of_components;i++)
						{
							display_message(INFORMATION_MESSAGE," %g",field->source_values[i]);
						}
					} break;
					case COMPUTED_FIELD_EXTERNAL:
					{
						display_message(INFORMATION_MESSAGE," number_of_values %d",
							field->number_of_source_values);
						display_message(INFORMATION_MESSAGE," number_of_fields %d",
							field->number_of_source_fields);
						display_message(INFORMATION_MESSAGE," filename %s",
							field->child_filename);
						if (field->number_of_source_values)
						{
							display_message(INFORMATION_MESSAGE," values");
							for (i=0;i<field->number_of_source_values;i++)
							{
								display_message(INFORMATION_MESSAGE," %f",
									field->source_values[i]);
							}
						}
						if (field->number_of_source_fields)
						{				
							display_message(INFORMATION_MESSAGE," fields");
							for (i=0;i<field->number_of_source_fields;i++)
							{
								display_message(INFORMATION_MESSAGE," %s",
									field->source_fields[i]->name);
							}
						}
						display_message(INFORMATION_MESSAGE," timeout %d",
							field->timeout);
					} break;
					case COMPUTED_FIELD_RC_COORDINATE:
					{
						display_message(INFORMATION_MESSAGE,
							" coordinate %s",field->source_fields[0]->name);
					} break;
					case COMPUTED_FIELD_SUM_COMPONENTS:
					{
						display_message(INFORMATION_MESSAGE," field %s weights",
							field->source_fields[0]->name);
						for (i=0;i<field->source_fields[0]->number_of_components;i++)
						{
							display_message(INFORMATION_MESSAGE," %g",field->source_values[i]);
						}
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"list_Computed_field.  Unknown field type");
						return_code=0;
					}
				}
			}
			display_message(INFORMATION_MESSAGE,"\n");
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_commands.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_commands */

int Computed_field_commands_ready_to_list(struct Computed_field *field,
	void *list_of_fields_void)
/*******************************************************************************
LAST MODIFIED : 12 March 1999

DESCRIPTION :
Returns true if <field> is not in <list_of_fields>, but it has no source fields
or all are in the list. Used to ensure field commands are listed in the order
they must be created - so not referring to a field that is not created yet.
==============================================================================*/
{
	int i,return_code;
	struct LIST(Computed_field) *list_of_fields;

	ENTER(Computed_field_commands_ready_to_list);
	if (field&&(list_of_fields=
		(struct LIST(Computed_field) *)list_of_fields_void))
	{
		return_code = !IS_OBJECT_IN_LIST(Computed_field)(field,list_of_fields);
		for (i=0;return_code&&(i<field->number_of_source_fields);i++)
		{
			return_code=IS_OBJECT_IN_LIST(Computed_field)(field->source_fields[i],
				list_of_fields);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_commands_ready_to_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_commands_ready_to_list */

int list_Computed_field_name(struct Computed_field *field,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 4 February 1999

DESCRIPTION :
Lists a single line about a computed field including just name, type, number of
components and coordinate system.
==============================================================================*/
{
	char *temp_string;
	int return_code;

	ENTER(list_Computed_field_name);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"%s",field->name);
		display_message(INFORMATION_MESSAGE," : %s",
			Computed_field_type_to_string(field));
		display_message(INFORMATION_MESSAGE,", %d component(s)",
			field->number_of_components);
		if (temp_string=Coordinate_system_string(&field->coordinate_system))
		{
			display_message(INFORMATION_MESSAGE,", %s",temp_string);
			DEALLOCATE(temp_string);
		}
		display_message(INFORMATION_MESSAGE,"\n");
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_name.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_name */

int Computed_field_contents_match(struct Computed_field *field,
	void *other_computed_field_void)
/*******************************************************************************
LAST MODIFIED : 22 January 1999

DESCRIPTION :
Iterator/conditional function returning true if contents of <field> other than
its name matches the contents of the <other_computed_field_void>.
==============================================================================*/
{
	int i, return_code;
	struct Computed_field *other_computed_field;

	ENTER(Computed_field_contents_match);
	if (field && (other_computed_field=(struct Computed_field *)other_computed_field_void))
	{
		if((field->number_of_components==other_computed_field->number_of_components)
			&&(field->read_only==other_computed_field->read_only)
			&&(field->coordinate_system.type==other_computed_field->coordinate_system.type)
			/* Ignoring other coordinate_system parameters */
			&&(field->type==other_computed_field->type)
			&&(field->component_no==other_computed_field->component_no)
			&&(field->number_of_source_fields==
				other_computed_field->number_of_source_fields)
			&&(field->number_of_source_values==
				other_computed_field->number_of_source_values))
		{
			return_code = 1;
			for(i = 0 ; return_code && (i < field->number_of_source_fields) ; i++)
			{
				return_code = (field->source_fields[i]==
					other_computed_field->source_fields[i]);
			}
			if(return_code)
			{
				for(i = 0 ; return_code && (i < field->number_of_source_values) ; i++)
				{
					return_code = (field->source_values[i]==
						other_computed_field->source_values[i]);				
				}
			}
			if (return_code && (field->type == COMPUTED_FIELD_NEW_TYPES))
			{
				if (field->type_string == other_computed_field->type_string)
				{
					
					if (field->computed_field_type_specific_contents_match_function)
					{
						return_code = 
							field->computed_field_type_specific_contents_match_function(
							field, other_computed_field);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_contents_match.  "
							"Type specific contents match function undefined.");
						return_code = 0;					
					}
				}
				else
				{
					return_code = 0;					
				}
			}
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_contents_match.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_contents_match */

struct Computed_field_package *CREATE(Computed_field_package)(
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_element) *fe_element_manager)
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
Creates a Computed_field_package which is used by the rest of the program to
access everything to do with computed fields. The computed_field_manager is
created here, and callbacks for changes in the fe_field_manager are established
so that wrapper Computed_fields are automatically created and updated to match
FE_fields.
???RC Create default computed_fields (xi, default_coordinate etc.) here?
==============================================================================*/
{
	struct Computed_field_package *computed_field_package;

	ENTER(CREATE(Computed_field_package));
	if (fe_field_manager && fe_element_manager)
	{
		if (ALLOCATE(computed_field_package,struct Computed_field_package,1)&&
			(computed_field_package->computed_field_manager=
				CREATE(MANAGER(Computed_field))()))
		{
			computed_field_package->fe_field_manager=fe_field_manager;
			computed_field_package->fe_element_manager=fe_element_manager;
			computed_field_package->computed_field_type_list =
			  CREATE(LIST(Computed_field_type_data))();
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Computed_field_package).  Not enough memory");
			DEALLOCATE(computed_field_package);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Computed_field_package).  Invalid argument(s)");
		computed_field_package=(struct Computed_field_package *)NULL;
	}
	LEAVE;

	return (computed_field_package);
} /* CREATE(Computed_field_package) */

int DESTROY(Computed_field_package)(
	struct Computed_field_package **package_address)
/*******************************************************************************
LAST MODIFIED : 21 May 2001

DESCRIPTION :
Frees memory/deaccess objects in computed_field_package at <*package_address>.
Cancels any further messages from managers.
==============================================================================*/
{
	int return_code;
	struct Computed_field_package *computed_field_package;

	ENTER(DESTROY(Computed_field_package));
	if (package_address&&(computed_field_package= *package_address))
	{
		DESTROY(MANAGER(Computed_field))(&computed_field_package->computed_field_manager);
		DESTROY(LIST(Computed_field_type_data))(
			&computed_field_package->computed_field_type_list);
		DEALLOCATE(*package_address);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Computed_field_package).  Missing field");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Computed_field_package) */

struct MANAGER(Computed_field)
	*Computed_field_package_get_computed_field_manager(
		struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 3 February 1999

DESCRIPTION :
Extracts the computed_field_manager from the computed_field_package. Note that
the rest of the program should use this sparingly - it is really only here to
allow interfacing to the choose_object widgets.
==============================================================================*/
{
	struct MANAGER(Computed_field) *computed_field_manager

	ENTER(Computed_field_package_get_computed_field_manager);
	if (computed_field_package)
	{
		computed_field_manager=computed_field_package->computed_field_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_package_get_computed_field_manager.  "
			"Missing computed_field_package");
		computed_field_manager=(struct MANAGER(Computed_field) *)NULL;
	}
	LEAVE;

	return (computed_field_manager);
} /* Computed_field_package_get_computed_field_manager */

int Computed_field_package_add_type(
	struct Computed_field_package *computed_field_package, char *name,
	Define_Computed_field_type_function define_Computed_field_type_function,
	void *define_type_user_data)
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
Adds the type of Computed_field described by <name> and 
<define_Computed_field_type_function> to those in the LIST held by the 
<computed_field_package>.  This type is then added to the 
define_Computed_field_type option table when parsing commands.
==============================================================================*/
{
	int return_code;
	struct Computed_field_type_data *data;

	ENTER(Computed_field_package_add_type);
	if (computed_field_package && name && define_Computed_field_type_function)
	{
		if(data = CREATE(Computed_field_type_data)(name,
			define_Computed_field_type_function, define_type_user_data))
		{
			return_code = ADD_OBJECT_TO_LIST(Computed_field_type_data)(data,
				computed_field_package->computed_field_type_list);
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_package_add_type.  Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_package_add_type */

int Computed_field_can_be_destroyed(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 May 2000

DESCRIPTION :
Returns true if the <field> is only accessed once - assumed by its manager. If
it is of type COMPUTED_FIELD_FINITE_ELEMENT further tests that its fe_field can
be destroyed, assuming it is only accessed by this field and its manager.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_can_be_destroyed);
	if (field)
	{
		if (return_code=!Computed_field_is_in_use(field))
		{
			if ((field->type == COMPUTED_FIELD_NEW_TYPES) &&
				(field->computed_field_can_be_destroyed_function))
			{
				return_code = field->computed_field_can_be_destroyed_function(field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_can_be_destroyed.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_can_be_destroyed */
