/*******************************************************************************
FILE : computed_field.c

LAST MODIFIED : 17 November 1999

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
#include "finite_element/computed_field.h"
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

	/* for types COMPUTED_FIELD_FINITE_ELEMENT, COMPUTED_FIELD_EMBEDDED,
		 COMPUTED_FIELD_NODE_VALUE,COMPUTED_FIELD_NODE_ARRAY_VALUE_AT_TIME only */
	/* the real FE_field being computed */
	struct FE_field *fe_field;

	/* for types COMPUTED_FIELD_FINITE_ELEMENT, COMPUTED_FIELD_EMBEDDED only */
	/* element field values of fe_field in last_element */
	struct FE_element_field_values *fe_element_field_values;

	/* for COMPUTED_FIELD_COMPONENT only */
	int component_no;

	/* for COMPUTED_FIELD_NODE_VALUE,COMPUTED_FIELD_NODE_ARRAY_VALUE_AT_TIME only */
	enum FE_nodal_value_type nodal_value_type;
	int version_number;

	/* for COMPUTED_FIELD_DEFAULT_COORDINATE only */
	struct MANAGER(Computed_field) *computed_field_manager;

	/* for COMPUTED_FIELD_XI_TEXTURE_COORDINATES only */
	struct FE_element *seed_element;
	struct LIST(Computed_field_element_texture_mapping) *texture_mapping;

	/* for COMPUTED_FIELD_SAMPLE_TEXTURE only */
	struct Texture *texture;

	/* for COMPUTED_FIELD_EXTERNAL only */
	char *child_filename;
	int timeout;
	struct Child_process *child_process;

	/* for COMPUTED_FIELD_CURVE_LOOKUP only */
	struct Control_curve *curve;

	/* for all Computed_field_types calculated from others */

	/* array of computed fields this field is calculated from */
	int number_of_source_fields;
	struct Computed_field **source_fields;
	/* array of constant values this field is calculated from */
	int number_of_source_values;
	FE_value *source_values;

	int access_count;
}; /* struct Computed_field */

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

struct Computed_field_package
/*******************************************************************************
LAST MODIFIED : 5 November 1999

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
	struct MANAGER(Texture) *texture_manager;
	struct MANAGER(Control_curve) *control_curve_manager;
	void *fe_field_manager_callback_id;
	void *control_curve_manager_callback_id;
}; /* struct Computed_field_package */

struct Computed_field_element_texture_mapping
/*******************************************************************************
LAST MODIFIED : 15 March 1999

DESCRIPTION :
==============================================================================*/
{
	/* Holds the pointer to the element, I don't access it so that the
		default object functions can be used.  The pointer is not
		referenced, just used as a label */
	struct FE_element *element;
	/* The three offsets for the xi1 = 0, xi2 = 0, xi3 = 0 corner. */
	float offset[MAXIMUM_ELEMENT_XI_DIMENSIONS];

	int access_count;
}; /* struct Computed_field_element_texture_mapping */

struct Computed_field_element_texture_mapping_fifo
/*******************************************************************************
LAST MODIFIED : 6 July 1999

DESCRIPTION :
Simple linked-list node structure for building a FIFO stack for the mapping
structure - needed for consistent growth of xi_texture_coordinates.
==============================================================================*/
{
	struct Computed_field_element_texture_mapping *mapping_item;
	struct Computed_field_element_texture_mapping_fifo *next;
}; /* struct Computed_field_element_texture_mapping_fifo */

/*
Module functions
----------------
*/

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Computed_field,name,char *,strcmp)

DECLARE_LOCAL_MANAGER_FUNCTIONS(Computed_field)

struct Computed_field_element_texture_mapping *CREATE(Computed_field_element_texture_mapping)
		 (void)
/*******************************************************************************
LAST MODIFIED : 16 March 1999

DESCRIPTION :
Creates a basic Computed_field with the given <name>. Its type is initially
COMPUTED_FIELD_CONSTANT with 1 component, returning a value of zero.
==============================================================================*/
{
	struct Computed_field_element_texture_mapping *mapping_item;

	ENTER(CREATE(Computed_field_element_texture_mapping));
	
	if (ALLOCATE(mapping_item,struct Computed_field_element_texture_mapping,1))
	{
		mapping_item->element = (struct FE_element *)NULL;
		mapping_item->offset[0] = 0.0;
		mapping_item->offset[1] = 0.0;
		mapping_item->offset[2] = 0.0;
		mapping_item->access_count = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Computed_field_element_texture_mapping).  Not enough memory");
		mapping_item = (struct Computed_field_element_texture_mapping *)NULL;
	}
	LEAVE;

	return (mapping_item);
} /* CREATE(Computed_field_element_texture_mapping) */

int DESTROY(Computed_field_element_texture_mapping)
	  (struct Computed_field_element_texture_mapping **mapping_address)
/*******************************************************************************
LAST MODIFIED : 26 May 1999

DESCRIPTION :
Frees memory/deaccess mapping at <*mapping_address>.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Computed_field_element_texture_mapping));
	if (mapping_address&&*mapping_address)
	{
		if (0 >= (*mapping_address)->access_count)
		{
			if ((*mapping_address)->element)
			{
				DEACCESS(FE_element)(&((*mapping_address)->element));
			}
			DEALLOCATE(*mapping_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Computed_field_element_texture_mapping).  Positive access_count");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Computed_field_element_texture_mapping).  Missing mapping");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Computed_field_element_texture_mapping) */

DECLARE_OBJECT_FUNCTIONS(Computed_field_element_texture_mapping)
DECLARE_LIST_TYPES(Computed_field_element_texture_mapping);
FULL_DECLARE_INDEXED_LIST_TYPE(Computed_field_element_texture_mapping);
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Computed_field_element_texture_mapping,
  element,struct FE_element *,compare_pointer)
DECLARE_INDEXED_LIST_FUNCTIONS(Computed_field_element_texture_mapping)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(
	Computed_field_element_texture_mapping,
	element,struct FE_element *,compare_pointer)

static int Computed_field_extract_rc(struct Computed_field *field,
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

static int Computed_field_clear_type(struct Computed_field *field)
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

		/* for types COMPUTED_FIELD_FINITE_ELEMENT, COMPUTED_FIELD_EMBEDDED,
			 COMPUTED_FIELD_NODE_VALUE,COMPUTED_FIELD_NODE_ARRAY_VALUE_AT_TIME only */
		if (field->fe_field)
		{
			DEACCESS(FE_field)(&(field->fe_field));
		}

		/* for types COMPUTED_FIELD_FINITE_ELEMENT, COMPUTED_FIELD_EMBEDDED only */
		if (field->fe_element_field_values)
		{
			DEALLOCATE(field->fe_element_field_values);
		}

		/* for COMPUTED_FIELD_COMPONENT only */
		field->component_no=0;

		/* for COMPUTED_FIELD_DEFAULT_COORDINATE only */
		field->computed_field_manager=(struct MANAGER(Computed_field) *)NULL;

		/* for COMPUTED_FIELD_NODE_VALUE,COMPUTED_FIELD_NODE_ARRAY_VALUE_AT_TIME only */
		field->nodal_value_type=FE_NODAL_VALUE;
		field->version_number=0;
	
		/* for COMPUTED_FIELD_XI_TEXTURE_COORDINATES only */
		if (field->seed_element)
		{
			DEACCESS(FE_element)(&(field->seed_element));
		}
		if (field->texture_mapping)
		{
			DESTROY_LIST(Computed_field_element_texture_mapping)
				(&field->texture_mapping);
			field->texture_mapping =
				(struct LIST(Computed_field_element_texture_mapping) *)NULL;
		}
		
		/* for COMPUTED_FIELD_SAMPLE_TEXTURE only */
		if(field->texture)
		{
			DEACCESS(Texture)(&field->texture);
			field->texture = (struct Texture *)NULL;
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

		/* for COMPUTED_FIELD_CURVE_LOOKUP only */
		if (field->curve)
		{
			DEACCESS(Control_curve)(&(field->curve));
		}

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

int Computed_field_is_read_only_with_fe_field(
	struct Computed_field *field,void *fe_field_void)
/*******************************************************************************
LAST MODIFIED : 3 February 1999

DESCRIPTION :
Iterator/conditional function returning true if <field> is read only and a
wrapper for <fe_field>.
==============================================================================*/
{
	int return_code;
	struct FE_field *fe_field;

	ENTER(Computed_field_is_read_only_with_fe_field);
	if (field&&(fe_field=(struct FE_field *)fe_field_void))
	{
		return_code=field->read_only&&(field->fe_field==fe_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_read_only_with_fe_field.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_is_read_only_with_fe_field */

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

			/* values/derivatives cache and working_values */
			field->values=(FE_value *)NULL;
			field->derivatives=(FE_value *)NULL;
			field->derivatives_valid=0;
			field->element=(struct FE_element *)NULL;
			field->node=(struct FE_node *)NULL;

			field->find_element_xi_mapping=(struct Computed_field_element_texture_mapping *)NULL;

			/* for types COMPUTED_FIELD_FINITE_ELEMENT, COMPUTED_FIELD_EMBEDDED,
				 COMPUTED_FIELD_NODE_VALUE,COMPUTED_FIELD_NODE_ARRAY_VALUE_AT_TIME only */
			field->fe_field=(struct FE_field *)NULL;

			/* for COMPUTED_FIELD_FINITE_ELEMENT and COMPUTED_FIELD_EMBEDDED only */
			field->fe_element_field_values=(struct FE_element_field_values *)NULL;

			/* for COMPUTED_FIELD_COMPONENT only */
			field->component_no=-1;

			/* for COMPUTED_FIELD_DEFAULT_COORDINATE only */
			field->computed_field_manager=(struct MANAGER(Computed_field) *)NULL;

			/* for COMPUTED_FIELD_NODE_VALUE,COMPUTED_FIELD_NODE_ARRAY_VALUE_AT_TIME only */
			field->nodal_value_type=FE_NODAL_VALUE;
			field->version_number=0;	
		
			/* for COMPUTED_FIELD_XI_TEXTURE_COORDINATES only */
			field->seed_element = (struct FE_element *)NULL;
			field->texture_mapping =
				(struct LIST(Computed_field_element_texture_mapping) *)NULL;
			
			/* for COMPUTED_FIELD_SAMPLE_TEXTURE only */
			field->texture = (struct Texture *)NULL;

			/* for COMPUTED_FIELD_EXTERNAL only */
			field->child_filename = (char *)NULL;
			field->child_process = (struct Child_process *)NULL;
			field->timeout = 0;

			/* for COMPUTED_FIELD_CURVE_LOOKUP only */
			field->curve=(struct Control_curve *)NULL;

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
	FE_value *source_values;
	int i,return_code;
	struct Computed_field **source_fields;
	struct LIST(Computed_field_element_texture_mapping) *texture_mapping;

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
				/* 1. make dynamic allocations for any new type-specific data */
				if (((0==source->number_of_source_fields)||ALLOCATE(source_fields,
					struct Computed_field *,source->number_of_source_fields))&&
					((0==source->number_of_source_values)||ALLOCATE(source_values,
						FE_value,source->number_of_source_values))&&
					((!source->texture_mapping)||(texture_mapping=
						CREATE_LIST(Computed_field_element_texture_mapping)())&&
						(COPY_LIST(Computed_field_element_texture_mapping)
							(texture_mapping,source->texture_mapping))))
				{
					/* 2. free current type-specific data */
					Computed_field_clear_type(destination);
					/* 3. establish the new type */
					destination->number_of_components=source->number_of_components;
					destination->read_only=source->read_only;
					COPY(Coordinate_system)(&destination->coordinate_system,
						&source->coordinate_system);
					destination->type=source->type;
					/* for types COMPUTED_FIELD_FINITE_ELEMENT, COMPUTED_FIELD_EMBEDDED,
						 COMPUTED_FIELD_NODE_VALUE,COMPUTED_FIELD_NODE_ARRAY_VALUE_AT_TIME only */
					if (source->fe_field)
					{
						destination->fe_field=ACCESS(FE_field)(source->fe_field);
					}
				
					/* for COMPUTED_FIELD_COMPONENT only */
					destination->component_no=source->component_no;

					/* for COMPUTED_FIELD_NODE_VALUE,COMPUTED_FIELD_NODE_ARRAY_VALUE_AT_TIME only */
					destination->nodal_value_type=source->nodal_value_type;
					destination->version_number=source->version_number;

					/* for COMPUTED_FIELD_DEFAULT_COORDINATE only */
					destination->computed_field_manager=source->computed_field_manager;

					/* for COMPUTED_FIELD_XI_TEXTURE_COORDINATES only */
					REACCESS(FE_element)(&destination->seed_element,source->seed_element);
					if (source->texture_mapping)
					{
						destination->texture_mapping = texture_mapping;
					}

					/* for COMPUTED_FIELD_SAMPLE_TEXTURE only */
					if (source->texture)
					{
						destination->texture = ACCESS(Texture)(source->texture);
					}

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

					/* for COMPUTED_FIELD_CURVE_LOOKUP only */
					if (source->curve)
					{
						destination->curve=ACCESS(Control_curve)(source->curve);
					}

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
					display_message(ERROR_MESSAGE,
						"MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_field,name).  "
						"Not enough memory");
					return_code=0;
					if (source_fields)
					{
						DEALLOCATE(source_fields);
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

struct Computed_field *Computed_field_begin_wrap_coordinate_field(
	struct Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Returns a RECTANGULAR_CARTESIAN coordinate field that may be the original
<coordinate field> if it is already in this coordinate system, or a
COMPUTED_FIELD_RC_COORDINATE wrapper for it if it is not.
Notes:
Used to ensure RC coordinate fields are passed to graphics functions.
Must call Computed_field_end_wrap to clean up the returned field after use.
==============================================================================*/
{
	struct Computed_field *wrapper_field;

	ENTER(Computed_field_begin_wrap_coordinate_field);
	if (coordinate_field&&(3>=coordinate_field->number_of_components))
	{
		if (RECTANGULAR_CARTESIAN==
			get_coordinate_system_type(&(coordinate_field->coordinate_system)))
		{
			wrapper_field=ACCESS(Computed_field)(coordinate_field);
		}
		else
		{
			/* make RC wrapper for the coordinate_field */
			if ((wrapper_field=CREATE(Computed_field)("rc_wrapper"))&&
				Computed_field_set_type_rc_coordinate(wrapper_field,
					coordinate_field))
			{
				ACCESS(Computed_field)(wrapper_field);
			}
			else
			{
				DESTROY(Computed_field)(&wrapper_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_begin_wrap_coordinate_field.  Invalid argument(s)");
		wrapper_field=(struct Computed_field *)NULL;
	}
	LEAVE;

	return (wrapper_field);
} /* Computed_field_begin_wrap_coordinate_field */

struct Computed_field *Computed_field_begin_wrap_orientation_scale_field(
	struct Computed_field *orientation_scale_field,
	struct Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 28 June 1999

DESCRIPTION :
Takes the <orientation_scale_field> and returns a field ready for use in the
rest of the program. This involves making a COMPUTED_FIELD_FIBRE_AXES wrapper
if the field has 3 or fewer components and a FIBRE coordinate system (this
requires the coordinate_field too). If the field has 3 or fewer components and
a non-RECTANGULAR_CARTESIAN coordinate system, a wrapper of type
COMPUTED_FIELD_RC_ORIENTATION_SCALE will be made for it. If the field is deemed
already usable in in its orientation_scale role, it is simply returned. Note
that the function accesses any returned field.
Note:
Must call Computed_field_end_wrap to clean up the returned field after use.
==============================================================================*/
{
	struct Computed_field *wrapper_field;
	enum Coordinate_system_type coordinate_system_type;

	ENTER(Computed_field_begin_wrap_orientation_scale_field);
	if (orientation_scale_field&&coordinate_field&&
		Computed_field_is_orientation_scale_capable(orientation_scale_field,NULL)&&
		Computed_field_has_1_to_3_components(coordinate_field,NULL))
	{
		coordinate_system_type=get_coordinate_system_type(
			&(orientation_scale_field->coordinate_system));
		if ((3>=orientation_scale_field->number_of_components)&&
			(FIBRE==coordinate_system_type))
		{
			/* make FIBRE_AXES wrapper */
			if ((wrapper_field=CREATE(Computed_field)("fibre_axes_wrapper"))&&
				Computed_field_set_type_fibre_axes(wrapper_field,
					orientation_scale_field,coordinate_field))
			{
				ACCESS(Computed_field)(wrapper_field);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_begin_wrap_orientation_scale_field.  "
					"Unable to make fibre_axes wrapper for field");
				DESTROY(Computed_field)(&wrapper_field);
			}
		}
		else if ((1==orientation_scale_field->number_of_components)||
			(RECTANGULAR_CARTESIAN==coordinate_system_type))
		{
			/* scalar or RC fields are already OK */
			wrapper_field=ACCESS(Computed_field)(orientation_scale_field);
		}
		else
		{
			/* make RC_VECTOR wrapper for the orientation_scale_field */
			if ((wrapper_field=CREATE(Computed_field)("rc_wrapper"))&&
				Computed_field_set_type_rc_vector(wrapper_field,
					orientation_scale_field,coordinate_field))
			{
				ACCESS(Computed_field)(wrapper_field);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_begin_wrap_orientation_scale_field.  "
					"Unable to make rc_component wrapper for field");
				DESTROY(Computed_field)(&wrapper_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_begin_wrap_orientation_scale_field.  "
			"Invalid argument(s)");
		wrapper_field=(struct Computed_field *)NULL;
	}
	LEAVE;

	return (wrapper_field);
} /* Computed_field_begin_wrap_orientation_scale_field */

int Computed_field_end_wrap(struct Computed_field **wrapper_field_address)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Cleans up a field accessed/created by a Computed_field_begin_wrap*() function.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_end_wrap);
	if (wrapper_field_address)
	{
		return_code=DEACCESS(Computed_field)(wrapper_field_address);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_field_end_wrap.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_end_wrap */

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
		if (((COMPUTED_FIELD_FINITE_ELEMENT==field->type)||
			(COMPUTED_FIELD_EMBEDDED==field->type))&&
			field->fe_element_field_values&&field->fe_element_field_values->element)
		{
			clear_FE_element_field_values(field->fe_element_field_values);
			/* clear element to indicate that values are clear */
			field->fe_element_field_values->element=(struct FE_element *)NULL;
		}
		if (field->element)
		{
			DEACCESS(FE_element)(&field->element);
		}
		if (field->find_element_xi_mapping)
		{
			DEACCESS(Computed_field_element_texture_mapping)
				(&field->find_element_xi_mapping);
		}
		if (field->node)
		{
			DEACCESS(FE_node)(&field->node);
		}
		field->derivatives_valid=0;
		for (i=0;i<field->number_of_source_fields;i++)
		{
			Computed_field_clear_cache(field->source_fields[i]);
		}
		if ((COMPUTED_FIELD_DEFAULT_COORDINATE==field->type)&&field->source_fields)
		{
			/* must deaccess any source_fields, since these act as a cache for type
				 COMPUTED_FIELD_DEFAULT_COORDINATE */
			for (i=0;i< field->number_of_source_fields;i++)
			{
				DEACCESS(Computed_field)(&(field->source_fields[i]));
			}
			DEALLOCATE(field->source_fields);
			field->number_of_source_fields=0;
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
LAST MODIFIED :  29 October  1999

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
		if (COMPUTED_FIELD_DEFAULT_COORDINATE==field->type)
		{
			if (get_FE_element_default_coordinate_field(element))
			{
				return_code=1;
			}
			else
			{
				return_code=0;
			}
		}
		else
		{
			return_code=1;
			if (field->fe_field)
			{
				if (!FE_field_is_defined_in_element(field->fe_field,element))
				{
					return_code=0;
				}
			}
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
	int comp_no,i,return_code;
	struct FE_field_component fe_field_component;

	ENTER(Computed_field_is_defined_at_node);
	return_code=0;
	if (field&&node)
	{	
		switch (field->type)
		{
			case COMPUTED_FIELD_DEFAULT_COORDINATE:
			{
				if (get_FE_node_default_coordinate_field(node))
				{
					return_code=1;
				}
				else
				{
					return_code=0;
				}
			} break;
			case COMPUTED_FIELD_FINITE_ELEMENT:
			{
				return_code=FE_field_is_defined_at_node(field->fe_field,node);
			} break;
			case COMPUTED_FIELD_NODE_VALUE:
			case COMPUTED_FIELD_NODE_ARRAY_VALUE_AT_TIME:
			{
				if (FE_field_is_defined_at_node(field->fe_field,node))
				{
					/* must ensure at least one component of version_number,
						 nodal_value_type defined at node */
					return_code=0;
					fe_field_component.field=field->fe_field;
					for (comp_no=0;(comp_no<field->number_of_components)&&(!return_code);
							 comp_no++)
					{
						fe_field_component.number=comp_no;
						if (FE_nodal_value_version_exists(node,&fe_field_component,
							field->version_number,field->nodal_value_type))
						{
							return_code=1;
						}
					}
				}
			} break;
			case COMPUTED_FIELD_EMBEDDED:
			{
				FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
				int number_of_components;
				struct FE_element *element;

				if (FE_field_is_defined_at_node(field->fe_field,node))
				{
					return_code=1;
					fe_field_component.field=field->fe_field;
					number_of_components=
						get_FE_field_number_of_components(field->fe_field);
					for (comp_no=0;(comp_no<number_of_components)&&return_code;comp_no++)
					{
						fe_field_component.number=comp_no;
						if (FE_nodal_value_version_exists(node,&fe_field_component,
							field->version_number,field->nodal_value_type)&&
							get_FE_nodal_element_xi_value(node,field->fe_field,comp_no,
								field->version_number,field->nodal_value_type,&element,xi))
						{
							for (i=0;(i<field->number_of_source_fields)&&return_code;i++)
							{
								if (!Computed_field_is_defined_in_element(
									field->source_fields[i],element))
								{
									return_code=0;
								}
							}
						}
						else
						{
							return_code=0;
						}
					}
				}
			} break;
			case COMPUTED_FIELD_ADD:
			case COMPUTED_FIELD_CLAMP_MAXIMUM:
			case COMPUTED_FIELD_CLAMP_MINIMUM:
			case COMPUTED_FIELD_CMISS_NUMBER:
			case COMPUTED_FIELD_COMPONENT:
			case COMPUTED_FIELD_COMPOSITE:
			case COMPUTED_FIELD_CONSTANT:
			case COMPUTED_FIELD_CUBIC_TEXTURE_COORDINATES:
			case COMPUTED_FIELD_CURVE_LOOKUP:
			case COMPUTED_FIELD_DOT_PRODUCT:
			case COMPUTED_FIELD_EDIT_MASK:
			case COMPUTED_FIELD_EXTERNAL:
			case COMPUTED_FIELD_RC_COORDINATE:
			case COMPUTED_FIELD_RC_VECTOR:
			case COMPUTED_FIELD_MAGNITUDE:
			case COMPUTED_FIELD_OFFSET:
			case COMPUTED_FIELD_SCALE:
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
			case COMPUTED_FIELD_2D_STRAIN:
			case COMPUTED_FIELD_FIBRE_AXES:
			case COMPUTED_FIELD_FIBRE_SHEET_AXES:
			case COMPUTED_FIELD_GRADIENT:
			case COMPUTED_FIELD_SAMPLE_TEXTURE:
			case COMPUTED_FIELD_XI_COORDINATES:
			case COMPUTED_FIELD_XI_TEXTURE_COORDINATES:
			{
				/* can not be evaluated at nodes */
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_is_defined_at_node.  Unknown field type");
				return_code=0;
			} break;

		} /* switch */
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

int Computed_field_depends_on_embedded_field(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 28 April 1999

DESCRIPTION :
Returns true if the field is of an embedded type or depends on any computed
fields which are or an embedded type.
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_depends_on_embedded_field);
	if (field)
	{
		switch(field->type)
		{
			case COMPUTED_FIELD_EMBEDDED:
			{
				return_code=1;
			} break;
			default:
			{
				return_code=0;
				for (i=0;(i<field->number_of_source_fields)&&(!return_code);i++)
				{
					return_code=Computed_field_depends_on_embedded_field(
						field->source_fields[i]);
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_depends_on_embedded_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_depends_on_embedded_field */

static int Computed_field_evaluate_2D_strain(struct Computed_field *field,
	int element_dimension)
/*******************************************************************************
LAST MODIFIED : 25 June 1999

DESCRIPTION :
Function called by Computed_field_evaluate_in_element to compute the wrinkle strain.
NOTE: Assumes that values and derivatives arrays are already allocated in
<field>, and that its source_fields are already computed (incl. derivatives)
for the same element, with the given <element_dimension> = number of Xi coords.
Derivatives are not available.
Source fields are coordinates, undeformed_coordinates and fibre angle.
==============================================================================*/
{
	double A,A2,B,B2,cos_fibre_angle,C2,D,dxi_dnu[6],E[4],fibre_angle, 
		F_x[6],F_X[6],sin_fibre_angle;
	FE_value def_derivative_xi[9], undef_derivative_xi[9];
	int return_code;
	
	ENTER(Computed_field_evaluate_2D_strain);
	if (field&&(COMPUTED_FIELD_2D_STRAIN==field->type))
	{
		field->derivatives_valid = 0;
		return_code = 1;
		switch(element_dimension)
		{
			case 2:
			{
				def_derivative_xi[0] = field->source_fields[0]->derivatives[0];
				def_derivative_xi[1] = field->source_fields[0]->derivatives[1];
				def_derivative_xi[3] = field->source_fields[0]->derivatives[2];
				def_derivative_xi[4] = field->source_fields[0]->derivatives[3];
				def_derivative_xi[6] = field->source_fields[0]->derivatives[4];
				def_derivative_xi[7] = field->source_fields[0]->derivatives[5];
				undef_derivative_xi[0] = field->source_fields[1]->derivatives[0];
				undef_derivative_xi[1] = field->source_fields[1]->derivatives[1];
				undef_derivative_xi[3] = field->source_fields[1]->derivatives[2];
				undef_derivative_xi[4] = field->source_fields[1]->derivatives[3];
				undef_derivative_xi[6] = field->source_fields[1]->derivatives[4];
				undef_derivative_xi[7] = field->source_fields[1]->derivatives[5];
			} break;
			case 3:
			{
				/* Convert to 2D ignoring xi3, should be able to choose the
				 direction that is ignored */
				def_derivative_xi[0] = field->source_fields[0]->derivatives[0];
				def_derivative_xi[1] = field->source_fields[0]->derivatives[1];
				def_derivative_xi[3] = field->source_fields[0]->derivatives[3];
				def_derivative_xi[4] = field->source_fields[0]->derivatives[4];
				def_derivative_xi[6] = field->source_fields[0]->derivatives[6];
				def_derivative_xi[7] = field->source_fields[0]->derivatives[7];
				undef_derivative_xi[0] = field->source_fields[1]->derivatives[0];
				undef_derivative_xi[1] = field->source_fields[1]->derivatives[1];
				undef_derivative_xi[3] = field->source_fields[1]->derivatives[3];
				undef_derivative_xi[4] = field->source_fields[1]->derivatives[4];
				undef_derivative_xi[6] = field->source_fields[1]->derivatives[6];
				undef_derivative_xi[7] = field->source_fields[1]->derivatives[7];
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_evaluate_2D_strain.  Unknown element dimension");
				return_code=0;
			} break;
		}
		if (return_code)
		{
			/* do 2D_strain calculation */
			fibre_angle=field->source_fields[2]->values[0];

			/* X is the undeformed coordinates
				x is the deformed coordinates
				nu is the fibre coordinate system (2-D,
				within sheet) */
			/* calculate F_X=dX_dnu and dxi_dnu */
			cos_fibre_angle=cos(fibre_angle);
			sin_fibre_angle=sin(fibre_angle);
			A2=undef_derivative_xi[0]*
				undef_derivative_xi[0]+
				undef_derivative_xi[3]*
				undef_derivative_xi[3]+
				undef_derivative_xi[6]*
				undef_derivative_xi[6];
			A=sqrt(A2);
			B2=undef_derivative_xi[1]*
				undef_derivative_xi[1]+
				undef_derivative_xi[4]*
				undef_derivative_xi[4]+
				undef_derivative_xi[7]*
				undef_derivative_xi[7];
			B=sqrt(B2);
			C2=undef_derivative_xi[0]*
				undef_derivative_xi[1]+
				undef_derivative_xi[3]*
				undef_derivative_xi[4]+
				undef_derivative_xi[6]*
				undef_derivative_xi[7];
			D=sin_fibre_angle*B/(A2*B2-C2*C2);
			dxi_dnu[0]=
				cos_fibre_angle/A-sin_fibre_angle*C2*D;
			dxi_dnu[1]=D*A2;
			F_X[0]=dxi_dnu[0]*undef_derivative_xi[0]+
				dxi_dnu[1]*undef_derivative_xi[1];
			F_X[2]=dxi_dnu[0]*undef_derivative_xi[3]+
				dxi_dnu[1]*undef_derivative_xi[4];
			F_X[4]=dxi_dnu[0]*undef_derivative_xi[6]+
				dxi_dnu[1]*undef_derivative_xi[7];
			D=cos_fibre_angle*B/(A2*B2-C2*C2);
			dxi_dnu[2]=
				-(sin_fibre_angle/A+cos_fibre_angle*C2*D);
			dxi_dnu[3]=D*A2;
			F_X[1]=dxi_dnu[2]*undef_derivative_xi[0]+
				dxi_dnu[3]*undef_derivative_xi[1];
			F_X[3]=dxi_dnu[2]*undef_derivative_xi[3]+
				dxi_dnu[3]*undef_derivative_xi[4];
			F_X[5]=dxi_dnu[2]*undef_derivative_xi[6]+
				dxi_dnu[3]*undef_derivative_xi[7];
			/* calculate F_x=dx_dnu=dx_dxi*dxi_dnu */
			F_x[0]=dxi_dnu[0]*def_derivative_xi[0]+
				dxi_dnu[1]*def_derivative_xi[1];
			F_x[1]=dxi_dnu[2]*def_derivative_xi[0]+
				dxi_dnu[3]*def_derivative_xi[1];
			F_x[2]=dxi_dnu[0]*def_derivative_xi[3]+
				dxi_dnu[1]*def_derivative_xi[4];
			F_x[3]=dxi_dnu[2]*def_derivative_xi[3]+
				dxi_dnu[3]*def_derivative_xi[4];
			F_x[4]=dxi_dnu[0]*def_derivative_xi[6]+
				dxi_dnu[1]*def_derivative_xi[7];
			F_x[5]=dxi_dnu[2]*def_derivative_xi[6]+
				dxi_dnu[3]*def_derivative_xi[7];
			/* calculate the strain tensor
				E=0.5*(trans(F_x)*F_x-trans(F_X)*F_X) */
			E[0]=0.5*((F_x[0]*F_x[0]+F_x[2]*F_x[2]+
				F_x[4]*F_x[4])-
				(F_X[0]*F_X[0]+F_X[2]*F_X[2]+
					F_X[4]*F_X[4]));
			E[1]=0.5*((F_x[0]*F_x[1]+F_x[2]*F_x[3]+
				F_x[4]*F_x[5])-
				(F_X[0]*F_X[1]+F_X[2]*F_X[3]+
					F_X[4]*F_X[5]));
			E[2]=E[1];
			E[3]=0.5*((F_x[1]*F_x[1]+F_x[3]*F_x[3]+
				F_x[5]*F_x[5])-
				(F_X[1]*F_X[1]+F_X[3]*F_X[3]+
					F_X[5]*F_X[5]));

			field->values[0] = E[0];
			field->values[1] = E[1];
			field->values[2] = E[2];
			field->values[3] = E[3];
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_2D_strain.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_evaluate_2D_strain */

static int Computed_field_evaluate_fibre_axes(struct Computed_field *field,
	int element_dimension)
/*******************************************************************************
LAST MODIFIED : 2 July 1999

DESCRIPTION :
Function called by Computed_field_evaluate_cache_in_element to compute the
3, 3-component fibre axes (fibre, sheet, normal) from the source fibre and
coordinate fields. Function reads the coordinate field and derivatives in
rectangular cartesian. The 1 to 3 fibre angles in the fibre_field are used as
follows (2 values omit step 3, 1 only does step 1):
1 = fibre_angle in xi1-xi2 plane measured from xi1;
2 = sheet_angle, inclination of the fibres from xi1-xi2 plane after step 1.
3 = imbrication_angle, rotation of the sheet about the fibre vec
coordinates from the source_field values in an arbitrary
coordinate system.
Notes:
<element_dimension> may be 2 or 3 only.
Derivatives may not be computed for this type of Computed_field [yet].
Assumes that values and derivatives arrays are already allocated in <field>, and
that its source_fields are already computed (incl. derivatives of the coordinate
field) for the same element, with the given <element_dimension> = number of xi
coordinates.
If field->type is COMPUTED_FIELD_FIBRE_AXES, the 3 vectors are returned in the
order fibre,sheet,normal. Type COMPUTED_FIELD_FIBRE_SHEET_AXES returns them as
sheet,-fibre,normal.
==============================================================================*/
{
	FE_value a_x,a_y,a_z,axes[9],b_x,b_y,b_z,c_x,c_y,c_z,cos_alpha,cos_beta,
		cos_gamma,dx_dxi[9],f11,f12,f13,f21,f22,f23,f31,f32,f33,*fibre_angle,length,
		sin_alpha,sin_beta,sin_gamma,x[3];
	int i,return_code;

	ENTER(Computed_field_evaluate_fibre_axes);
	if (field&&((2==element_dimension)||(3==element_dimension))&&
		((COMPUTED_FIELD_FIBRE_AXES==field->type)||
			(COMPUTED_FIELD_FIBRE_SHEET_AXES==field->type)))
	{
		if (return_code=Computed_field_extract_rc(field->source_fields[1],
			element_dimension,x,dx_dxi))
		{
			/* get f1~ = vector in xi1 direction */
			f11=dx_dxi[0];
			f12=dx_dxi[3];
			f13=dx_dxi[6];
			/* get f2~ = vector in xi2 direction */
			f21=dx_dxi[1];
			f22=dx_dxi[4];
			f23=dx_dxi[7];
			/* get f3~ = vector normal to xi1-xi2 plane */
			f31=f12*f23-f13*f22;
			f32=f13*f21-f11*f23;
			f33=f11*f22-f12*f21;
			/* normalise vectors f1~ and f3~ */
			if (0.0<(length=sqrt(f11*f11+f12*f12+f13*f13)))
			{
				f11 /= length;
				f12 /= length;
				f13 /= length;
			}
			if (0.0<(length=sqrt(f31*f31+f32*f32+f33*f33)))
			{
				f31 /= length;
				f32 /= length;
				f33 /= length;
			}
			/* get vector f2~ = f3~ (x) f1~ = normal to xi1 in xi1-xi2 plane */
			f21=f32*f13-f33*f12;
			f22=f33*f11-f31*f13;
			f23=f31*f12-f32*f11;
			/* get sin/cos of fibre angles alpha, beta and gamma */
			fibre_angle=field->source_fields[0]->values;
			sin_alpha=sin(fibre_angle[0]);
			cos_alpha=cos(fibre_angle[0]);
			if (1<field->source_fields[0]->number_of_components)
			{
				sin_beta=sin(fibre_angle[1]);
				cos_beta=cos(fibre_angle[1]);
			}
			else
			{
				/* default beta is 0 */
				sin_beta=0;
				cos_beta=1;
			}
			/*???RC calculate_FE_field_anatomical had 1 instead of 2 in following: */
			if (2<field->source_fields[0]->number_of_components)
			{
				sin_gamma=sin(fibre_angle[2]);
				cos_gamma=cos(fibre_angle[2]);
			}
			else
			{
				/* default gamma is pi/2 */
				sin_gamma=1;
				cos_gamma=0;
			}
			/* calculate the fibre axes a=fibre, b=sheet, c=normal */
			a_x=cos_alpha*f11+sin_alpha*f21;
			a_y=cos_alpha*f12+sin_alpha*f22;
			a_z=cos_alpha*f13+sin_alpha*f23;
			b_x= -sin_alpha*f11+cos_alpha*f21;
			b_y= -sin_alpha*f12+cos_alpha*f22;
			b_z= -sin_alpha*f13+cos_alpha*f23;
			f11=a_x;
			f12=a_y;
			f13=a_z;
			f21=b_x;
			f22=b_y;
			f23=b_z;
			a_x=cos_beta*f11+sin_beta*f31;
			a_y=cos_beta*f12+sin_beta*f32;
			a_z=cos_beta*f13+sin_beta*f33;
			c_x= -sin_beta*f11+cos_beta*f31;
			c_y= -sin_beta*f12+cos_beta*f32;
			c_z= -sin_beta*f13+cos_beta*f33;
			f31=c_x;
			f32=c_y;
			f33=c_z;
			b_x=sin_gamma*f21-cos_gamma*f31;
			b_y=sin_gamma*f22-cos_gamma*f32;
			b_z=sin_gamma*f23-cos_gamma*f33;
			c_x=cos_gamma*f21+sin_gamma*f31;
			c_y=cos_gamma*f22+sin_gamma*f32;
			c_z=cos_gamma*f23+sin_gamma*f33;
			if (COMPUTED_FIELD_FIBRE_AXES==field->type)
			{
				/* fibre,sheet,normal */
				axes[0]=a_x;
				axes[1]=a_y;
				axes[2]=a_z;
				axes[3]=b_x;
				axes[4]=b_y;
				axes[5]=b_z;
				axes[6]=c_x;
				axes[7]=c_y;
				axes[8]=c_z;
			}
			else /* COMPUTED_FIELD_FIBRE_SHEET_AXES */
			{
				/* sheet,-fibre,normal */
				axes[0]=b_x;
				axes[1]=b_y;
				axes[2]=b_z;
				axes[3]=-a_x;
				axes[4]=-a_y;
				axes[5]=-a_z;
				axes[6]=c_x;
				axes[7]=c_y;
				axes[8]=c_z;
			}
			for (i=0;i<field->number_of_components;i++)
			{
				field->values[i]=axes[i];
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_evaluate_fibre_axes.  Could not convert to RC");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_fibre_axes.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_evaluate_fibre_axes */

static int Computed_field_evaluate_gradient(struct Computed_field *field,
	int element_dimension)
/*******************************************************************************
LAST MODIFIED : 2 July 1999

DESCRIPTION :
Function called by Computed_field_evaluate_in_element to compute the gradient.
NOTE: Assumes that values and derivatives arrays are already allocated in
<field>, and that its source_fields are already computed (incl. derivatives)
for the same element, with the given <element_dimension> = number of Xi coords.
Derivatives are always calculated and set since they are always zero.
If function fails to invert the coordinate derivatives then the gradient is
simply returned as the 0 vector.
==============================================================================*/
{
	FE_value *destination,dx_dxi[9],dxi_dx[9],x[3],*source;
	int coordinate_components,i,j,return_code;
	
	ENTER(Computed_field_evaluate_gradient);
	if (field&&(COMPUTED_FIELD_GRADIENT==field->type))
	{
		coordinate_components=field->source_fields[1]->number_of_components;
		/* Following asks: can dx_dxi be inverted? */
		if (((3==element_dimension)&&(3==coordinate_components))||
			((RECTANGULAR_CARTESIAN==field->source_fields[1]->coordinate_system.type)
				&&(coordinate_components==element_dimension))||
			((CYLINDRICAL_POLAR==field->source_fields[1]->coordinate_system.type)&&
				(2==element_dimension)&&(2==coordinate_components)))
		{
			if (return_code=Computed_field_extract_rc(field->source_fields[1],
				element_dimension,x,dx_dxi))
			{
				/* if the element_dimension is less than 3, put ones on the main
					 diagonal to allow inversion of dx_dxi */
				if (3>element_dimension)
				{
					dx_dxi[8]=1.0;
					if (2>element_dimension)
					{
						dx_dxi[4]=1.0;
					}
				}
				if (invert_FE_value_matrix3(dx_dxi,dxi_dx))
				{
					destination=field->values;
					for (i=0;i<field->number_of_components;i++)
					{
						*destination=0.0;
						if (i<element_dimension)
						{
							source=field->source_fields[0]->derivatives;
							for (j=0;j<element_dimension;j++)
							{
								*destination += (*source) * dxi_dx[3*j+i];
								source++;
							}
						}
						destination++;
					}
				}
				else
				{
					/* could not invert coordinate derivatives; set gradient to 0 */
					for (i=0;i<field->number_of_components;i++)
					{
						field->values[i]=0.0;
					}			
				}
				/* derivatives = div(grad()) are always zero */
				destination=field->derivatives;
				for (i=element_dimension*(field->number_of_components);0<i;i--)
				{
					*destination = 0.0;
					destination++;
				}
				field->derivatives_valid=1;
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_evaluate_gradient.  Failed");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_evaluate_gradient.  Elements of wrong dimension");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_gradient.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_evaluate_gradient */

static int Computed_field_evaluate_rc_coordinate(struct Computed_field *field,
	int element_dimension,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 25 June 1999

DESCRIPTION :
Function called by Computed_field_evaluate_cache_in_element/at_node to compute
rectangular cartesian coordinates from the source_field values in an arbitrary
coordinate system.
NOTE: Assumes that values and derivatives arrays are already allocated in
<field>, and that its source_fields are already computed (incl. derivatives if
calculate_derivatives set) for the same element, with the given
<element_dimension> = number of Xi coords.
Note: both COMPUTED_FIELD_DEFAULT_COORDINATE and COMPUTED_FIELD_RC_COORDINATE
are computed with this function.
==============================================================================*/
{
	FE_value *destination,*dx_dxi,temp[9],x[3];
	int i,j,return_code;
	
	ENTER(Computed_field_evaluate_rc_coordinate);
	if (field&&((COMPUTED_FIELD_DEFAULT_COORDINATE==field->type)||
		(COMPUTED_FIELD_RC_COORDINATE==field->type)))
	{
		if (calculate_derivatives)
		{
			dx_dxi=temp;
		}
		else
		{
			dx_dxi=(FE_value *)NULL;
		}
		if (return_code=Computed_field_extract_rc(field->source_fields[0],
			element_dimension,x,dx_dxi))
		{
			/*???RC works because number_of_components is always 3 */
			for (i=0;i<3;i++)
			{
				field->values[i]=x[i];
			}
			if (calculate_derivatives)
			{
				destination=field->derivatives;
				for (i=0;i<3;i++)
				{
					for (j=0;j<element_dimension;j++)
					{
						*destination=dx_dxi[3*i+j];
						destination++;
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_evaluate_rc_coordinate.  Could not convert to RC");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_rc_coordinate.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_evaluate_rc_coordinate */

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

static int Computed_field_get_default_coordinate_source_field_in_element(
	struct Computed_field *field,struct FE_element *element)
/*******************************************************************************
LAST MODIFIED : 27 October 1999

DESCRIPTION :
For fields of type COMPUTED_FIELD_DEFAULT_COORDINATE, makes sure the <field>'s
source_fields are allocated to contain 1 field, the finite_element wrapper for
the first coordinate field defined at <node>. For efficiency, checks that the
currently cached field/node are not already correct before finding a new one.
==============================================================================*/
{
	int return_code;
	struct FE_field *fe_field;

	ENTER(Computed_field_get_default_coordinate_source_field_in_element);
	if (field&&element&&(COMPUTED_FIELD_DEFAULT_COORDINATE==field->type))
	{
		return_code=1;
		/* get Computed_field wrapping first coordinate field of element */
		/* if the element is still pointed to by the cache, then already ok */
		if (element != field->element)
		{
			if (fe_field=get_FE_element_default_coordinate_field(element))
			{
				if (!field->source_fields||
					(field->source_fields[0]->fe_field!=fe_field))
				{
					/* finding a new field, so must clear cache of current one */
					if (field->source_fields)
					{
						Computed_field_clear_cache(field->source_fields[0]);
						DEACCESS(Computed_field)(&(field->source_fields[0]));
					}
					else
					{
						field->source_fields=
							ALLOCATE(field->source_fields,struct Computed_field *,1);
					}
					if (field->source_fields)
					{
						if (field->source_fields[0]=
							FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
								Computed_field_is_read_only_with_fe_field,
								(void *)fe_field,field->computed_field_manager))
						{
							ACCESS(Computed_field)(field->source_fields[0]);
							field->number_of_source_fields=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_get_default_coordinate_source_field_in_element."
								"  No computed field for default coordinate field");
							/* don't want empty source_fields array left around */
							DEALLOCATE(field->source_fields);
							field->number_of_source_fields=0;
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_get_default_coordinate_source_field_in_element.  "
							"Could not allocate default coordinate source fields");
						field->number_of_source_fields=0;
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_get_default_coordinate_source_field_in_element.  "
					"No default coordinate field");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_default_coordinate_source_field_in_element.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_default_coordinate_source_field_in_element */

static int Computed_field_evaluate_cache_in_element(
	struct Computed_field *field,struct FE_element *element,FE_value *xi,
	struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 4 November 1999

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
	double texture_values[3];
	FE_value element_to_top_level[9],sum,*temp,*temp1,*temp2,
		top_level_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS],
		compose_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int cache_is_valid,element_dimension,i,index,j,k,return_code,total_values,
		top_level_element_dimension;
	struct Computed_field_element_texture_mapping *mapping;
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
			/* 1. Get top_level_element for types that must be calculated on them */
			switch (field->type)
			{
				case COMPUTED_FIELD_FIBRE_AXES:
				case COMPUTED_FIELD_FIBRE_SHEET_AXES:
				case COMPUTED_FIELD_GRADIENT:
				case COMPUTED_FIELD_XI_TEXTURE_COORDINATES:
				{
					if (CM_ELEMENT == element->cm.type)
					{
						top_level_element=element;
						for (i=0;i<element_dimension;i++)
						{
							top_level_xi[i]=xi[i];
						}
						/* do not set element_to_top_level */
						top_level_element_dimension=element_dimension;
					}
					else
					{
						/* check or get top_level element and xi coordinates for it */
						if (top_level_element=FE_element_get_top_level_element_conversion(
							element,top_level_element,(struct GROUP(FE_element) *)NULL,
							-1,element_to_top_level))
						{
							/* convert xi to top_level_xi */
							top_level_element_dimension=top_level_element->shape->dimension;
							for (j=0;j<top_level_element_dimension;j++)
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
								"Computed_field_evaluate_cache_in_element.  "
								"No top-level element found to evaluate field %s on",
								field->name);
							return_code=0;
						}
					}
				} break;
			}

			/* 2. Precalculate any source fields that this field depends on. For type
				 COMPUTED_FIELD_FINITE_ELEMENT, this means getting
				 FE_element_field_values. */
			if (return_code)
			{
				switch (field->type)
				{
					case COMPUTED_FIELD_2D_STRAIN:
					{
						/* always calculate derivatives of undeformed and deformed coordinate fields */
						return_code=
							Computed_field_evaluate_cache_in_element(field->source_fields[0],
								element,xi,top_level_element,1)&&
							Computed_field_evaluate_cache_in_element(field->source_fields[1],
								element,xi,top_level_element,1)&&
							Computed_field_evaluate_cache_in_element(field->source_fields[2],
								element,xi,top_level_element,0);
					} break;
					case COMPUTED_FIELD_COMPOSE:
					{
						/* only calculate the first source_field at this location */
						return_code=
							Computed_field_evaluate_cache_in_element(
							field->source_fields[0],element,xi,top_level_element,0);
					} break;
					case COMPUTED_FIELD_DEFAULT_COORDINATE:
					{
						if (Computed_field_get_default_coordinate_source_field_in_element(
							field,element))
						{
							/* calculate values of source_field */
							return_code=Computed_field_evaluate_cache_in_element(
								field->source_fields[0],element,xi,top_level_element,
								calculate_derivatives);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_evaluate_cache_in_element.  "
								"Could not get default coordinate source_field");
							return_code=0;
						}
					} break;
					case COMPUTED_FIELD_EMBEDDED:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_evaluate_cache_at_element.  "
							"Cannot evaluate an embedded field in elements");
						return_code=0;
					} break;
					case COMPUTED_FIELD_FIBRE_AXES:
					case COMPUTED_FIELD_FIBRE_SHEET_AXES:
					{
						if (calculate_derivatives)
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_evaluate_cache_in_element.  "
								"Derivatives not available for fibre_axes/fibre_sheet_axes");
							return_code=0;
						}
						else
						{
							/* coordinate field must be evaluated on the top_level_element
								 and derivatives are always required for it */
							return_code=
								Computed_field_evaluate_cache_in_element(
									field->source_fields[0],element,xi,top_level_element,0)&&
								Computed_field_evaluate_cache_in_element(
									field->source_fields[1],top_level_element,top_level_xi,
									top_level_element,1);
						}
					} break;
					case COMPUTED_FIELD_FINITE_ELEMENT:
					{
						/* ensure we have FE_element_field_values for element, with
							 derivatives_calculated if requested */
						if ((!field->fe_element_field_values)||
							(!FE_element_field_values_are_for_element(
								field->fe_element_field_values,element,top_level_element))||
							(calculate_derivatives&&
								(!field->fe_element_field_values->derivatives_calculated)))
						{
							if (!field->fe_element_field_values)
							{
								if (ALLOCATE(field->fe_element_field_values,
									struct FE_element_field_values,1))
								{
									/* clear element to indicate that values are clear */
									field->fe_element_field_values->element=
										(struct FE_element *)NULL;
								}
								else
								{
									return_code=0;
								}
							}
							else
							{
								if (field->fe_element_field_values->element)
								{
									/* following clears fe_element_field_values->element */
									clear_FE_element_field_values(field->fe_element_field_values);
								}
							}
							if (return_code)
							{
								/* note that FE_element_field_values accesses the element */
								if (!calculate_FE_element_field_values(element,field->fe_field,
									calculate_derivatives,field->fe_element_field_values,
									top_level_element))
								{
									/* clear element to indicate that values are clear */
									field->fe_element_field_values->element=
										(struct FE_element *)NULL;
									return_code=0;
								}
							}
						}
					} break;
					case COMPUTED_FIELD_GRADIENT:
					{
						/* always calculate derivatives of scalar and coordinate fields
							 and evaluate on the top_level_element */
						return_code=
							Computed_field_evaluate_cache_in_element(field->source_fields[0],
								top_level_element,top_level_xi,top_level_element,1)&&
							Computed_field_evaluate_cache_in_element(field->source_fields[1],
								top_level_element,top_level_xi,top_level_element,1);
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
			}

			/* 4. Calculate the field */
			if (return_code)
			{
				/* set the flag indicating if derivatives are valid, assuming all values
					 will be successfully calculated. Note that some types always get
					 derivatives since they are so inexpensive to calculate. */
				field->derivatives_valid=calculate_derivatives;
				switch (field->type)
				{
					case COMPUTED_FIELD_2D_STRAIN:
					{
						return_code=
							Computed_field_evaluate_2D_strain(field,element_dimension);
					} break;
					case COMPUTED_FIELD_ADD:
					{
						for (i=0;i<field->number_of_components;i++)
						{
							field->values[i]=
								field->source_values[0]*field->source_fields[0]->values[i]+
								field->source_values[1]*field->source_fields[1]->values[i];
						}
						if (calculate_derivatives)
						{
							temp=field->derivatives;
							temp1=field->source_fields[0]->derivatives;
							temp2=field->source_fields[1]->derivatives;
							for (i=(field->number_of_components*element_dimension);
							  0<i;i--)
							{
								(*temp)=field->source_values[0]*(*temp1)+
									field->source_values[1]*(*temp2);
								temp++;
								temp1++;
								temp2++;
							}
						}
					} break;
					case COMPUTED_FIELD_CLAMP_MAXIMUM:
					{
						if (calculate_derivatives)
						{
							temp=field->derivatives;
							temp2=field->source_fields[0]->derivatives;
						}
						for (i=0;i<field->number_of_components;i++)
						{
							if (field->source_fields[0]->values[i] < field->source_values[i])
							{
								field->values[i]=field->source_fields[0]->values[i];
								if (calculate_derivatives)
								{
									for (j=0;j<element_dimension;j++)
									{
										(*temp)=(*temp2);
										temp++;
										temp2++;
									}
								}
							}
							else
							{
								field->values[i]=field->source_values[i];
								if (calculate_derivatives)
								{
									for (j=0;j<element_dimension;j++)
									{
										(*temp)=0.0;
										temp++;
										temp2++; /* To ensure that the following components match */
									}
								}
							}
						}
					} break;
					case COMPUTED_FIELD_CLAMP_MINIMUM:
					{
						if (calculate_derivatives)
						{
							temp=field->derivatives;
							temp2=field->source_fields[0]->derivatives;
						}
						for (i=0;i<field->number_of_components;i++)
						{
							if (field->source_fields[0]->values[i] > field->source_values[i])
							{
								field->values[i]=field->source_fields[0]->values[i];
								if (calculate_derivatives)
								{
									for (j=0;j<element_dimension;j++)
									{
										(*temp)=(*temp2);
										temp++;
										temp2++;
									}
								}
							}
							else
							{
								field->values[i]=field->source_values[i];
								if (calculate_derivatives)
								{
									for (j=0;j<element_dimension;j++)
									{
										(*temp)=0.0;
										temp++;
										temp2++; /* To ensure that the following components match */
									}
								}
							}
						}
					} break;
					case COMPUTED_FIELD_CMISS_NUMBER:
					{
						/* simply convert the element number into an FE_value */
						field->values[0]=(FE_value)element->cm.number;
						/* no derivatives for this type */
						field->derivatives_valid=0;
					} break;
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
							&compose_element, compose_xi))
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
					case COMPUTED_FIELD_COMPOSITE:
					{
						for (i=0;i<field->number_of_components;i++)
						{
							field->values[i]= *(field->source_fields[i]->values);
							if (calculate_derivatives)
							{
								temp=field->derivatives + i*element_dimension;
								temp2=field->source_fields[i]->derivatives;
								for (j=0;j<element_dimension;j++)
								{
									temp[j]=temp2[j];
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
						temp=field->source_fields[0]->values;
						field->values[3] = fabs(*temp);
						temp++;
						j = 0;
						for (i=1;i < field->source_fields[0]->number_of_components;i++)
						{
							if (fabs(*temp) > field->values[3])
							{
								field->values[3] = fabs(*temp);
								j = i;
							}
							temp++;
						}
						temp=field->source_fields[0]->values;
						for (i=0;i < field->source_fields[0]->number_of_components - 1;i++)
						{
							if ( i == j )
							{
								/* Skip over the maximum coordinate */
								temp++;
							}
							field->values[i] = *temp / field->values[3];
							temp++;
						}
						field->derivatives_valid = 0;
					} break;
					case COMPUTED_FIELD_CURVE_LOOKUP:
					{
						FE_value dx_dt,*jacobian;

						if (calculate_derivatives)
						{
							jacobian=field->derivatives;
						}
						else
						{
							jacobian=(FE_value *)NULL;
						}
						/* only slightly dodgy - stores derivatives of curve in start
							 of derivatives space - must be at least big enough */
						if (Control_curve_get_values_at_parameter(field->curve,
							field->source_fields[0]->values[0],field->values,jacobian))
						{
							if (jacobian)
							{
								/* use product rule to get derivatives */
								temp=field->source_fields[0]->derivatives;
								/* count down in following loop because of slightly dodgy bit */
								for (j=field->number_of_components-1;0<=j;j--)
								{
									dx_dt = jacobian[j];
									for (i=0;i<element_dimension;i++)
									{
										jacobian[j*element_dimension+i] = dx_dt*temp[i];
									}
								}
							}
						}
						else
						{
							return_code=0;
						}
					} break;
					case COMPUTED_FIELD_DEFAULT_COORDINATE:
					case COMPUTED_FIELD_RC_COORDINATE:
					{
						/* once the default_coordinate source field is found, its
							 calculation is the same as rc_coordinate */
						return_code=Computed_field_evaluate_rc_coordinate(field,
							element_dimension,calculate_derivatives);
					} break;
					case COMPUTED_FIELD_DOT_PRODUCT:
					{
						field->values[0] = 0.0;
						if (calculate_derivatives)
						{
							for (j=0;j<element_dimension;j++)
							{
								field->derivatives[j]=0.0;
							}
						}
						temp=field->source_fields[0]->values;
						temp2=field->source_fields[1]->values;
						for (i=0;i < field->source_fields[0]->number_of_components;i++)
						{
							field->values[0] += (*temp) * (*temp2);
							temp++;
							temp2++;
						}
						if (calculate_derivatives)
						{
							temp=field->source_fields[0]->values;
							temp2=field->source_fields[1]->derivatives;
							for (i=0;i < field->source_fields[0]->number_of_components;i++)
							{
								for (j=0;j<element_dimension;j++)
								{
									field->derivatives[j] += (*temp)*(*temp2);
									temp2++;
								}
								temp++;
							}
							temp=field->source_fields[1]->values;
							temp2=field->source_fields[0]->derivatives;
							for (i=0;i < field->source_fields[0]->number_of_components;i++)
							{
								for (j=0;j<element_dimension;j++)
								{
									field->derivatives[j] += (*temp)*(*temp2);
									temp2++;
								}
								temp++;
							}
						}
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
					case COMPUTED_FIELD_FIBRE_AXES:
					case COMPUTED_FIELD_FIBRE_SHEET_AXES:
					{
						return_code=Computed_field_evaluate_fibre_axes(field,
							top_level_element_dimension);
					} break;
					case COMPUTED_FIELD_FINITE_ELEMENT:
					{
						enum Value_type value_type;

						value_type=get_FE_field_value_type(field->fe_field);
						/* component number -1 = calculate all components */
						switch (value_type)
						{
							case FE_VALUE_VALUE:
							{
								if (calculate_derivatives)
								{
									return_code=calculate_FE_element_field(-1,
										field->fe_element_field_values,xi,field->values,
										field->derivatives);
								}
								else
								{
									return_code=calculate_FE_element_field(-1,
										field->fe_element_field_values,xi,field->values,
										(FE_value *)NULL);
								}
							} break;
							case INT_VALUE:
							{
								int *int_values;

								/* no derivatives for this value_type */
								field->derivatives_valid=0;
								if (calculate_derivatives)
								{
									display_message(ERROR_MESSAGE,
										"Computed_field_evaluate_cache_in_element.  "
										"Derivatives not defined for integer fields");
									return_code=0;
								}
								else
								{
									if (ALLOCATE(int_values,int,field->number_of_components))
									{
										return_code=calculate_FE_element_field_int_values(-1,
											field->fe_element_field_values,xi,int_values);
										for (i=0;i<field->number_of_components;i++)
										{
											field->values[i]=(FE_value)int_values[i];
										}
										DEALLOCATE(int_values);
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"Computed_field_evaluate_cache_in_element.  "
											"Not enough memory for int_values");
										return_code=0;
									}
								}
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"Computed_field_evaluate_cache_in_element.  "
									"Unsupported value type %s in finite_element field",
									Value_type_string(value_type));
								return_code=0;
							} break;
						}
					} break;
					case COMPUTED_FIELD_GRADIENT:
					{
						return_code=Computed_field_evaluate_gradient(field,
							top_level_element_dimension);
					} break;
					case COMPUTED_FIELD_MAGNITUDE:
					{
						field->values[0]=0.0;
						if (calculate_derivatives)
						{
							for (j=0;j<element_dimension;j++)
							{
								field->derivatives[j]=0.0;
							}
						}
						temp=field->source_fields[0]->values;
						temp2=field->source_fields[0]->derivatives;
						for (i=field->source_fields[0]->number_of_components;0<i;i--)
						{
							field->values[0] += ((*temp)*(*temp));
							if (calculate_derivatives)
							{
								for (j=0;j<element_dimension;j++)
								{
									field->derivatives[j] += 2.0*(*temp)*(*temp2);
									temp2++;
								}
							}
							temp++;
						}
						field->values[0]=sqrt(field->values[0]);
					} break;
					case COMPUTED_FIELD_NODE_VALUE:
					case COMPUTED_FIELD_NODE_ARRAY_VALUE_AT_TIME:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_evaluate_cache_in_element.  "
							"Cannot evaluate node_value in elements");
						return_code=0;
					} break;
					case COMPUTED_FIELD_OFFSET:
					{
						for (i=0;i<field->number_of_components;i++)
						{
							field->values[i]=
								field->source_values[i]+field->source_fields[0]->values[i];
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
					case COMPUTED_FIELD_RC_VECTOR:
					{
						return_code=Computed_field_evaluate_rc_vector(field);
						/* no derivatives for this type */
						field->derivatives_valid=0;
					} break;
					case COMPUTED_FIELD_SCALE:
					{
						for (i=0;i<field->number_of_components;i++)
						{
							field->values[i]=
								field->source_values[i]*field->source_fields[0]->values[i];
						}
						if (calculate_derivatives)
						{
							temp=field->derivatives;
							temp2=field->source_fields[0]->derivatives;
							for (i=0;i<field->number_of_components;i++)
							{
								for (j=0;j<element_dimension;j++)
								{
									(*temp)=field->source_values[i]*(*temp2);
									temp++;
									temp2++;
								}
							}
						}
					} break;
					case COMPUTED_FIELD_SAMPLE_TEXTURE:
					{
						/* Could generalise to 1D and 3D textures */
						Texture_get_pixel_values(field->texture,
							field->source_fields[0]->values[0],
							field->source_fields[0]->values[1], texture_values);
						field->values[0] =  texture_values[0];
						field->values[1] =  texture_values[1];
						field->values[2] =  texture_values[2];
						field->derivatives_valid = 0;
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
					case COMPUTED_FIELD_XI_COORDINATES:
					{
						/* returns the values in xi, up to the element_dimension and padded
							 with zeroes */
						temp=field->derivatives;
						for (i=0;i<field->number_of_components;i++)
						{
							if (i<element_dimension)
							{
								field->values[i]=xi[i];
							}
							else
							{
								field->values[i]=0.0;
							}
							for (j=0;j<element_dimension;j++)
							{
								if (i==j)
								{
									*temp = 1.0;
								}
								else
								{
									*temp = 0.0;
								}
								temp++;
							}
						}
						/* derivatives are always calculated since they are merely part of
							 the identity matrix */
						field->derivatives_valid=1;
					} break;
					case COMPUTED_FIELD_XI_TEXTURE_COORDINATES:
					{
						if (field->texture_mapping)
						{
							if (mapping = FIND_BY_IDENTIFIER_IN_LIST
								(Computed_field_element_texture_mapping,element)
								(top_level_element, field->texture_mapping))
							{
								if (element == top_level_element)
								{
									temp=field->derivatives;
									for (i = 0 ; i < element_dimension ; i++)
									{
										field->values[i] = mapping->offset[i] + xi[i];
										if (calculate_derivatives)
										{
											for (j=0;j<element_dimension;j++)
											{
												if (i==j)
												{
													*temp = 1.0;
												}
												else
												{
													*temp = 0.0;
												}
												temp++;
											}
										}
									}
								}
								else
								{
									temp = element_to_top_level;
									temp2 = field->derivatives;
									for (i = 0 ; i < top_level_element_dimension ; i++)
									{
										field->values[i] = mapping->offset[i] + (*temp);
										temp++;
										for (j = 0 ; j < element->shape->dimension ; j++)
										{
											field->values[i] += (*temp) * xi[j];
											if (calculate_derivatives)
											{
												*temp2 = *temp;
												*temp2++;
											}
											temp++;
										}
									}
								}
							}
							else
							{
								FE_element_to_element_string(element,&temp_string);
								display_message(ERROR_MESSAGE,
									"Computed_field_evaluate_cache_in_element."
									"  Element %s not found in Xi texture coordinate mapping",
									temp_string);
								DEALLOCATE(temp_string);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_evaluate_cache_in_element.  "
								"Xi texture coordinate mapping not calculated");
							return_code=0;
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

			/* 5. Store information about what is cached, or clear it if error */
			if (return_code&&calculate_derivatives&&!(field->derivatives_valid))
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_evaluate_cache_in_element.  "
					"Derivatives unavailable for field %s of type %s",field->name,
					Computed_field_type_to_string(field->type));
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
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_cache_in_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_evaluate_cache_in_element */

char *Computed_field_evaluate_as_string_in_element(struct Computed_field *field,
	struct FE_element *element,FE_value *xi,struct FE_element *top_level_element)
/*******************************************************************************
LAST MODIFIED : 17 October 1999

DESCRIPTION :
Returns a string describing the value/s of the <field> at <element>:<xi>. If the
field is based on an FE_field but not returning FE_values, it is asked to supply
the string. Otherwise, a string built up of comma separated values evaluated
for the field in Computed_field_evaluate_cache_in_element. The FE_value
exception is used since it is likely the values are already in the cache in
most cases, or can be used by other fields again if calculated now.

The <top_level_element> parameter has the same use as in
Computed_field_evaluate_cache_in_element.

Some basic field types such as CMISS_NUMBER have special uses in this function.
It is up to the calling function to DEALLOCATE the returned string.
???RC.  Allow derivatives to be evaluated as string too?
==============================================================================*/
{
	char *return_string,tmp_string[50];
	int error,i,return_code;

	ENTER(Computed_field_evaluate_as_string_in_element);
	return_string=(char *)NULL;
	if (field&&element&&xi)
	{
		if ((COMPUTED_FIELD_FINITE_ELEMENT==field->type)&&
			(FE_VALUE_VALUE != get_FE_field_value_type(field->fe_field)))
		{
			return_code=1;
			/*???RC this code from Computed_field_evaluate_cache_in_element */
			/* handle separately since can have types other than FE_value */
			/* clear the cache if values already cached for a node */
			if (field->node)
			{
				Computed_field_clear_cache(field);
			}
			/* ensure element_field_values are calculated for fe_field in element */
			if (field->element != element)
			{
				/* ensure we have FE_element_field_values for element, with
					 derivatives_calculated if requested */
				if ((!field->fe_element_field_values)||
					(!FE_element_field_values_are_for_element(
						field->fe_element_field_values,element,top_level_element)))
				{
					if (!field->fe_element_field_values)
					{
						if (ALLOCATE(field->fe_element_field_values,
							struct FE_element_field_values,1))
						{
							/* clear element to indicate that values are clear */
							field->fe_element_field_values->element=
								(struct FE_element *)NULL;
						}
						else
						{
							return_code=0;
						}
					}
					else
					{
						if (field->fe_element_field_values->element)
						{
							/* following clears fe_element_field_values->element */
							clear_FE_element_field_values(field->fe_element_field_values);
						}
					}
					if (return_code)
					{
						/* note that FE_element_field_values accesses the element */
						if (!calculate_FE_element_field_values(element,field->fe_field,
							/*calculate_derivatives*/0,field->fe_element_field_values,
							top_level_element))
						{
							/* clear element to indicate that values are clear */
							field->fe_element_field_values->element=(struct FE_element *)NULL;
							return_code=0;
						}
					}
				}
			}
			if (return_code)
			{
				/* component_number of -1 = all components */
				return_code=calculate_FE_element_field_as_string(/*component_number*/-1,
					field->fe_element_field_values,xi,&return_string);
			}
		}
		else
		{
			error=0;
			if (COMPUTED_FIELD_CMISS_NUMBER==field->type)
			{
				/* put out the cmiss number as a string */
				sprintf(tmp_string,"%d",element->cm.number);
				append_string(&return_string,tmp_string,&error);
			}
			else
			{
				/* write the component values in %g format, comma separated */
				if (Computed_field_evaluate_cache_in_element(field,element,xi,
					top_level_element,/*calculate_derivatives*/0))
				{
					for (i=0;i<field->number_of_components;i++)
					{
						if (0<i)
						{
							sprintf(tmp_string,",%g",field->values[i]);
						}
						else
						{
							sprintf(tmp_string,"%g",field->values[i]);
						}
						append_string(&return_string,tmp_string,&error);
					}
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
			"Computed_field_evaluate_as_string_in_element.  Invalid argument(s)");
	}
	LEAVE;

	return (return_string);
} /* Computed_field_evaluate_as_string_in_element */

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
				"Computed_field_evaluate_in_element.  Failed");
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

static int Computed_field_get_default_coordinate_source_field_at_node(
	struct Computed_field *field,struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 7 July 1999

DESCRIPTION :
For fields of type COMPUTED_FIELD_DEFAULT_COORDINATE, makes sure the <field>'s
source_fields are allocated to contain 1 field, the finite_element wrapper for
the first coordinate field defined at <node>. For efficiency, checks that the
currently cached field/node are not already correct before finding a new one.
==============================================================================*/
{
	int return_code;
	struct FE_field *fe_field;

	ENTER(Computed_field_get_default_coordinate_source_field_at_node);
	if (field&&node&&(COMPUTED_FIELD_DEFAULT_COORDINATE==field->type))
	{
		return_code=1;
		/* if node and field->node have equivalent fields then source field will
			 already be correct */
		if (!(field->node&&equivalent_FE_fields_at_nodes(node,field->node)))
		{
			if (fe_field=get_FE_node_default_coordinate_field(node))
			{
				if (!field->source_fields||
					(fe_field != field->source_fields[0]->fe_field))
				{
					/* finding a new field, so must clear cache of current one */
					if (field->source_fields)
					{
						Computed_field_clear_cache(field->source_fields[0]);
						DEACCESS(Computed_field)(&(field->source_fields[0]));
					}
					else
					{
						field->source_fields=
							ALLOCATE(field->source_fields,struct Computed_field *,1);
					}
					if (field->source_fields)
					{
						if (field->source_fields[0]=
							FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
								Computed_field_is_read_only_with_fe_field,
								(void *)fe_field,field->computed_field_manager))
						{
							ACCESS(Computed_field)(field->source_fields[0]);
							field->number_of_source_fields=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_get_default_coordinate_source_field_at_node.  "
								"No computed field for default coordinate field");
							/* don't want empty source_fields array left around */
							DEALLOCATE(field->source_fields);
							field->number_of_source_fields=0;
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_get_default_coordinate_source_field_at_node.  "
							"Could not allocate default coordinate source fields");
						field->number_of_source_fields=0;
						return_code=0;
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_get_default_coordinate_source_field_at_node.  "
					"No default coordinate field");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_default_coordinate_source_field_at_node.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_default_coordinate_source_field_at_node */

static int Computed_field_evaluate_cache_at_node(
	struct Computed_field *field,struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 4 November 1999

DESCRIPTION :
Calculates the values of <field> at <node>, if it is defined over the element.
Upon successful return the node values of the <field> are stored in its cache.
Note: Type COMPUTED_FIELD_FINITE_ELEMENT currently computes the FE_NODAL_VALUE
and first version of the field at the node. Later may want to add Computed_field
wrappers for extracting different versions and types. This has the advantage
that more than one can be cached simultaneously.

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
	double double_value;
	enum Value_type value_type;
	FE_value sum,*temp,*temp2;
	float float_value;
	int i,int_value,j,k,return_code,total_values;
	/* For COMPUTED_FIELD_EMBEDDED and COMPUTED_FIELD_COMPOSE only */
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
#if defined (OLD_CODE)
	short short_value;
#endif
	struct FE_element *element;	
	struct FE_field_component fe_field_component;

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
				case COMPUTED_FIELD_DEFAULT_COORDINATE:
				{
					if (Computed_field_get_default_coordinate_source_field_at_node(
						field,node))
					{
						/* calculate values of source_field */
						return_code=Computed_field_evaluate_cache_at_node(
							field->source_fields[0],node);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_evaluate_cache_at_node.  "
							"Could not get default coordinate source_field");
						return_code=0;
					}
				} break;
				case COMPUTED_FIELD_EMBEDDED:
				{
					/* Need to evaluate the source field in the element and xi described
						by the fe_field rather than at the node so I do it down below */
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

			/* 2. Allocate values and derivative cache */
			/*???RC Code the same as in Computed_field_evaluate_cache_at_node. Could
				change to separate cache for node values. See ???RC comments above */
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
			}

			/* 3. Calculate the field */
			if (return_code)
			{
				switch (field->type)
				{
					case COMPUTED_FIELD_2D_STRAIN:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_evaluate_cache_at_node.  "
							"Cannot evaluate gradient at nodes");
						return_code=0;
					} break;
					case COMPUTED_FIELD_ADD:
					{
						for (i=0;i<field->number_of_components;i++)
						{
							field->values[i]=
								field->source_values[0]*field->source_fields[0]->values[i]+
								field->source_values[1]*field->source_fields[1]->values[i];
						}
					} break;
					case COMPUTED_FIELD_CLAMP_MAXIMUM:
					{
						for (i=0;i<field->number_of_components;i++)
						{
							if (field->source_fields[0]->values[i] < field->source_values[i])
							{
								field->values[i]=field->source_fields[0]->values[i];
							}
							else
							{
								field->values[i]=field->source_values[i];
							}
						}
					} break;
					case COMPUTED_FIELD_CLAMP_MINIMUM:
					{
						for (i=0;i<field->number_of_components;i++)
						{
							if (field->source_fields[0]->values[i] > field->source_values[i])
							{
								field->values[i]=field->source_fields[0]->values[i];
							}
							else
							{
								field->values[i]=field->source_values[i];
							}
						}
					} break;
					case COMPUTED_FIELD_CMISS_NUMBER:
					{
						/* simply convert the node number into an FE_value */
						field->values[0]=(FE_value)get_FE_node_cm_node_identifier(node);
					} break;
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
							field->source_fields[0]->number_of_components, &element, xi))
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
					case COMPUTED_FIELD_COMPOSITE:
					{
						for (i=0;i<field->number_of_components;i++)
						{
							field->values[i]= *(field->source_fields[i]->values);
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
						temp=field->source_fields[0]->values;
						field->values[3] = fabs(*temp);
						temp++;
						j = 0;
						for (i=1;i < field->source_fields[0]->number_of_components;i++)
						{
							if (fabs(*temp) > field->values[3])
							{
								field->values[3] = fabs(*temp);
								j = i;
							}
							temp++;
						}
						temp=field->source_fields[0]->values;
						for (i=0;i < field->source_fields[0]->number_of_components - 1;i++)
						{
							if ( i == j )
							{
								/* Skip over the maximum coordinate */
								temp++;
							}
							field->values[i] = *temp / field->values[3];
							temp++;
						}
					} break;
					case COMPUTED_FIELD_CURVE_LOOKUP:
					{
						return_code=Control_curve_get_values_at_parameter(field->curve,
							field->source_fields[0]->values[0],field->values,
							/*derivatives*/(FE_value *)NULL);
					} break;
					case COMPUTED_FIELD_DEFAULT_COORDINATE:
					case COMPUTED_FIELD_RC_COORDINATE:
					{
						/* once the default_coordinate source field is found, its
							 calculation is the same as rc_coordinate */
						return_code=Computed_field_evaluate_rc_coordinate(field,
							/*element_dimension*/0,/*calculate_derivatives*/0);
					} break;
					case COMPUTED_FIELD_DOT_PRODUCT:
					{
						field->values[0] = 0.0;
						temp=field->source_fields[0]->values;
						temp2=field->source_fields[1]->values;
						for (i=0;i < field->source_fields[0]->number_of_components;i++)
						{
							field->values[0] += *temp * *temp2;
							temp++;
							temp2++;
						}
					} break;
					case COMPUTED_FIELD_EDIT_MASK:
					{
						for (i=0;i<field->number_of_components;i++)
						{
							field->values[i]=field->source_fields[0]->values[i];
						}
					} break;
					case COMPUTED_FIELD_EMBEDDED:
					{
						if (get_FE_nodal_element_xi_value(node,
							field->fe_field, /* component */ 0,
							/* version */ 0, FE_NODAL_VALUE, &element, xi))
						{
							/* now calculate source_fields[0] */
							if(Computed_field_evaluate_cache_in_element(
								field->source_fields[0],element,xi,(struct FE_element *)NULL,0))
							{
								for (i=0;i<field->number_of_components;i++)
								{
									field->values[i] = field->source_fields[0]->values[i];
								}
							}
							else
							{
								return_code = 0;
							}
						}
						else
						{
							return_code = 0;
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
					case COMPUTED_FIELD_FIBRE_AXES:
					case COMPUTED_FIELD_FIBRE_SHEET_AXES:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_evaluate_cache_at_node.  "
							"Cannot evaluate fibre_axes/fibre_sheet_axes at nodes");
						return_code=0;
					} break;
					case COMPUTED_FIELD_FINITE_ELEMENT:
					{
						double double_value;
						enum Value_type value_type;
						float float_value;
						int int_value;
						struct FE_field_component fe_field_component;
						
						/* not very efficient - should cache FE_node_field or similar */
						fe_field_component.field=field->fe_field;
						value_type=get_FE_field_value_type(field->fe_field);
						for (i=0;(i<field->number_of_components)&&return_code;i++)
						{
							fe_field_component.number=i;
							switch (value_type)
							{
								case DOUBLE_VALUE:
								{
									return_code=get_FE_nodal_double_value(node,
										&fe_field_component,field->version_number,
										field->nodal_value_type,&double_value);
									field->values[i] = (FE_value)double_value;
								} break;
								case FE_VALUE_VALUE:
								{
									return_code=get_FE_nodal_FE_value_value(node,
										&fe_field_component,field->version_number,
										field->nodal_value_type,&(field->values[i]));
								} break;
								case FLT_VALUE:
								{
									return_code=get_FE_nodal_float_value(node,&fe_field_component,
										field->version_number,field->nodal_value_type,&float_value);
									field->values[i] = (FE_value)float_value;
								} break;
								case INT_VALUE:
								{
									return_code=get_FE_nodal_int_value(node,&fe_field_component,
										field->version_number,field->nodal_value_type,&int_value);
									field->values[i] = (FE_value)int_value;
								} break;
								default:
								{
									display_message(ERROR_MESSAGE,
										"Computed_field_evaluate_cache_at_node.  "
										"Unsupported value type %s in finite_element field",
										Value_type_string(value_type));
									return_code=0;
								}
							}
						}
						if (!return_code)
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_evaluate_cache_at_node.  "
								"Error evaluating finite_element field at node");
						}
					} break;
					case COMPUTED_FIELD_GRADIENT:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_evaluate_cache_at_node.  "
							"Cannot evaluate gradient at nodes");
						return_code=0;
					} break;
					case COMPUTED_FIELD_MAGNITUDE:
					{
						field->values[0]=0.0;
						temp=field->source_fields[0]->values;
						for (i=field->source_fields[0]->number_of_components;0<i;i--)
						{
							field->values[0] += ((*temp)*(*temp));
							temp++;
						}
						field->values[0]=sqrt(field->values[0]);
					} break;
					case COMPUTED_FIELD_NODE_ARRAY_VALUE_AT_TIME: 			 
					{						
						fe_field_component.field=field->fe_field;
						value_type=get_FE_field_value_type(field->fe_field);
						for (i=0;(i<field->number_of_components)&&return_code;i++)
						{
							fe_field_component.number=i;
							if (FE_nodal_value_version_exists(node,&fe_field_component,
								field->version_number,field->nodal_value_type))
							{
								switch (value_type)
								{									
									case FE_VALUE_ARRAY_VALUE:
									{
#if defined (OLD_CODE)										
										return_code=get_FE_nodal_FE_value_array_value_at_FE_value_time(node,
											&fe_field_component,field->version_number,
											field->nodal_value_type,field->time,&(field->values[i]));
#endif /*#if defined (OLD_CODE)	 */
									} break;
								
									case SHORT_ARRAY_VALUE:
									{
#if defined (OLD_CODE)	
										return_code=get_FE_nodal_short_array_value_at_FE_value_time(
											node,&fe_field_component,field->version_number,
											field->nodal_value_type,field->time,&short_value);
										field->values[i] = (FE_value)short_value;
#endif /*#if defined (OLD_CODE)	 */
									} break;	
									case DOUBLE_ARRAY_VALUE: 				
									case FLT_ARRAY_VALUE:
									case INT_ARRAY_VALUE:			
									case UNSIGNED_ARRAY_VALUE:
									{
										display_message(ERROR_MESSAGE,
											"Computed_field_evaluate_cache_at_node. value type %s "
											"not yet supported. Write the code!",
											Value_type_string(value_type));
										return_code=0;
									}break;
									default:
									{
										display_message(ERROR_MESSAGE,
											"Computed_field_evaluate_cache_at_node.  "
											"Unsupported value type %s in node_value field",
											Value_type_string(value_type));
										return_code=0;
									}
								}
							}
							else
							{
								/* use 0 for all undefined components */
								field->values[i]=0.0;
							}
						}
						if (!return_code)
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_evaluate_cache_at_node.  "
								"Error evaluating node_value field at node");
						}
					} break;
					case COMPUTED_FIELD_NODE_VALUE:
					{					
						fe_field_component.field=field->fe_field;
						value_type=get_FE_field_value_type(field->fe_field);
						for (i=0;(i<field->number_of_components)&&return_code;i++)
						{
							fe_field_component.number=i;
							if (FE_nodal_value_version_exists(node,&fe_field_component,
								field->version_number,field->nodal_value_type))
							{
								switch (value_type)
								{
									case DOUBLE_VALUE:
									{
										return_code=get_FE_nodal_double_value(node,
											&fe_field_component,field->version_number,
											field->nodal_value_type,&double_value);
										field->values[i] = (FE_value)double_value;
									} break;
									case FE_VALUE_VALUE:
									{
										return_code=get_FE_nodal_FE_value_value(node,
											&fe_field_component,field->version_number,
											field->nodal_value_type,&(field->values[i]));
									} break;
									case FLT_VALUE:
									{
										return_code=get_FE_nodal_float_value(node,
											&fe_field_component,field->version_number,
											field->nodal_value_type,&float_value);
										field->values[i] = (FE_value)float_value;
									} break;
									case INT_VALUE:
									{
										return_code=get_FE_nodal_int_value(node,&fe_field_component,
											field->version_number,field->nodal_value_type,&int_value);
										field->values[i] = (FE_value)int_value;
									} break;
									default:
									{
										display_message(ERROR_MESSAGE,
											"Computed_field_evaluate_cache_at_node.  "
											"Unsupported value type %s in node_value field",
											Value_type_string(value_type));
										return_code=0;
									}
								}
							}
							else
							{
								/* use 0 for all undefined components */
								field->values[i]=0.0;
							}
						}
						if (!return_code)
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_evaluate_cache_at_node.  "
								"Error evaluating node_value field at node");
						}
					} break;				
					case COMPUTED_FIELD_OFFSET:
					{
						for (i=0;i<field->number_of_components;i++)
						{
							field->values[i]=
								field->source_values[i] + field->source_fields[0]->values[i];
						}
					} break;
					case COMPUTED_FIELD_RC_VECTOR:
					{
						/* once the default_coordinate source field is found, its
							 calculation is the same as rc_coordinate */
						return_code=Computed_field_evaluate_rc_vector(field);
					} break;
					case COMPUTED_FIELD_SAMPLE_TEXTURE:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_evaluate_cache_at_node.  "
							"Currently cannot evaluate sample texture at nodes");
						return_code=0;
					} break;
					case COMPUTED_FIELD_SCALE:
					{
						for (i=0;i<field->number_of_components;i++)
						{
							field->values[i]=
								field->source_values[i]*field->source_fields[0]->values[i];
						}
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
					case COMPUTED_FIELD_XI_COORDINATES:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_evaluate_cache_at_node.  "
							"Cannot evaluate xi at nodes");
						return_code=0;
					} break;
					case COMPUTED_FIELD_XI_TEXTURE_COORDINATES:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_evaluate_cache_at_node.  "
							"Currently cannot evaluate xi texture coordinates at nodes");
						return_code=0;
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_evaluate_cache_at_node.  Unknown field type");
						return_code=0;
					} break;
				}
			}

			/* 4. Store information about what is cached, or clear it if error */
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
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_cache_at_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_evaluate_cache_at_node */

char *Computed_field_evaluate_as_string_at_node(struct Computed_field *field,
	struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 17 October 1999

DESCRIPTION :
Returns a string describing the value/s of the <field> at the <node>. If the
field is based on an FE_field but not returning FE_values, it is asked to supply
the string. Otherwise, a string built up of comma separated values evaluated
for the field in Computed_field_evaluate_cache_at_node. The FE_value exception
is used since it is likely the values are already in the cache in most cases,
or can be used by other fields again if calculated now.
Some basic field types such as CMISS_NUMBER have special uses in this function.
It is up to the calling function to DEALLOCATE the returned string.
==============================================================================*/
{
	char *return_string,tmp_string[50],*temp_string;
	int error,i;

	ENTER(Computed_field_evaluate_as_string_at_node);
	return_string=(char *)NULL;
	if (field&&node)
	{
		if (((COMPUTED_FIELD_FINITE_ELEMENT==field->type)||
			(COMPUTED_FIELD_NODE_VALUE==field->type))&&
			(FE_VALUE_VALUE != get_FE_field_value_type(field->fe_field)))
		{
			if (get_FE_nodal_value_as_string(node,field->fe_field,
				/*component_number*/0,field->version_number,field->nodal_value_type,
				&return_string))
			{
				error=0;
				for (i=1;i<field->number_of_components;i++)
				{
					if (get_FE_nodal_value_as_string(node,field->fe_field,
						i,field->version_number,field->nodal_value_type,&temp_string))
					{
						append_string(&return_string,",",&error);
						append_string(&return_string,temp_string,&error);
						DEALLOCATE(temp_string);
					}
				}
			}
		}
		else
		{
			error=0;
			if (COMPUTED_FIELD_CMISS_NUMBER==field->type)
			{
				/* put out the cmiss number as a string */
				sprintf(tmp_string,"%d",get_FE_node_cm_node_identifier(node));
				append_string(&return_string,tmp_string,&error);
			}
			else
			{
				/* write the component values in %g format, comma separated */
				if (Computed_field_evaluate_cache_at_node(field,node))
				{
					for (i=0;i<field->number_of_components;i++)
					{
						if (0<i)
						{
							sprintf(tmp_string,",%g",field->values[i]);
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
					error=1;
				}
			}
		}
		if (!return_string)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_evaluate_as_string_at_node.  Failed");
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

static int Computed_field_set_values_at_node_private(
	struct Computed_field *field,struct FE_node *node,FE_value *values)
/*******************************************************************************

LAST MODIFIED : 29 October 1999

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
Note that the values array will not be modified by this function.
#### Must ensure implemented correctly for new Computed_field_type. ####
???RC Note that some functions are not reversible in this way.
==============================================================================*/
{
	FE_value *source_values;
	int i,j,k,return_code;

	ENTER(Computed_field_set_values_at_node_private);
	if (field&&node&&values)
	{
		return_code=1;
		switch (field->type)
		{
			case COMPUTED_FIELD_CLAMP_MAXIMUM:
			{
				/* clamps to limits of maximums when setting values too */
				if (ALLOCATE(source_values,FE_value,field->number_of_components))
				{
					for (i=0;i<field->number_of_components;i++)
					{
						if (values[i] > field->source_values[i])
						{
							source_values[i] = field->source_values[i];
						}
						else
						{
							source_values[i] = values[i];
						}
					}
					return_code=Computed_field_set_values_at_node_private(
						field->source_fields[0],node,source_values);
					DEALLOCATE(source_values);
				}
				else
				{
					return_code=0;
				}
			} break;
			case COMPUTED_FIELD_CLAMP_MINIMUM:
			{
				/* clamps to limits of minimums when setting values too */
				if (ALLOCATE(source_values,FE_value,field->number_of_components))
				{
					for (i=0;i<field->number_of_components;i++)
					{
						if (values[i] < field->source_values[i])
						{
							source_values[i] = field->source_values[i];
						}
						else
						{
							source_values[i] = values[i];
						}
					}
					return_code=Computed_field_set_values_at_node_private(
						field->source_fields[0],node,source_values);
					DEALLOCATE(source_values);
				}
				else
				{
					return_code=0;
				}
			} break;
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
						return_code=Computed_field_set_values_at_node_private(
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
			case COMPUTED_FIELD_COMPOSITE:
			{
				/* set values of individual scalar fields */
				for (i=0;(i<field->number_of_components)&&return_code;i++)
				{
					return_code=Computed_field_set_values_at_node_private(
						field->source_fields[i],node,&(values[i]));
				}
			} break;
			case COMPUTED_FIELD_DEFAULT_COORDINATE:
			{
				FE_value non_rc_coordinates[3];
				struct Coordinate_system rc_coordinate_system;

				if (Computed_field_get_default_coordinate_source_field_at_node(
					field,node))
				{
					/* convert RC values back into source coordinate system */
					rc_coordinate_system.type = RECTANGULAR_CARTESIAN;
					return_code=
						convert_Coordinate_system(&rc_coordinate_system,values,
							&(field->source_fields[0]->coordinate_system),non_rc_coordinates,
							/*jacobian*/(float *)NULL)&&
						Computed_field_set_values_at_node_private(field->source_fields[0],
							node,non_rc_coordinates);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_set_values_at_node_private.  "
						"Could not get default coordinate source_field");
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
						return_code=Computed_field_set_values_at_node_private(
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
			case COMPUTED_FIELD_FINITE_ELEMENT:
			{
				double double_value;
				enum Value_type value_type;
				float float_value;
				int int_value;
				struct FE_field_component fe_field_component;

				fe_field_component.field=field->fe_field;
				value_type=get_FE_field_value_type(field->fe_field);
				for (i=0;(i<field->number_of_components)&&return_code;i++)
				{
					fe_field_component.number=i;
					/* set values all versions; to set values for selected version only,
						 use COMPUTED_FIELD_NODE_VALUE instead */
					k=get_FE_node_field_component_number_of_versions(node,
						field->fe_field,i);
					for (j=0;(j<k)&&return_code;j++)
					{
						switch (value_type)
						{
							case DOUBLE_VALUE:
							{
								double_value=(double)values[i];
								return_code=set_FE_nodal_double_value(node,
									&fe_field_component,j,field->nodal_value_type,double_value);
							} break;
							case FE_VALUE_VALUE:
							{
								return_code=set_FE_nodal_FE_value_value(node,
									&fe_field_component,j,field->nodal_value_type,values[i]);
							} break;
							case FLT_VALUE:
							{
								float_value=(float)values[i];
								return_code=set_FE_nodal_float_value(node,
									&fe_field_component,j,field->nodal_value_type,float_value);
							} break;
							case INT_VALUE:
							{
								int_value=(int)floor(values[i]+0.5);
								return_code=set_FE_nodal_float_value(node,
									&fe_field_component,j,field->nodal_value_type,int_value);
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"Computed_field_set_values_at_node_private.  "
									"Could not set finite_element field %s at node",field->name);
								return_code=0;
							}
						}
					}
				}
				if (!return_code)
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_set_values_at_node_private.  "
						"Could not set finite_element field %s at node",field->name);
				}
			} break;
			case COMPUTED_FIELD_MAGNITUDE:
			{
				FE_value magnitude;

				/* need current vector field values "magnify" */
				if (ALLOCATE(source_values,FE_value,
					field->source_fields[0]->number_of_components))
				{
					if (Computed_field_evaluate_at_node(field->source_fields[0],node,
						source_values))
					{
						/* if the source field is not a zero vector, set its magnitude to
							 the given value */
						magnitude = 0.0;
						for (i=0;i<field->number_of_components;i++)
						{
							magnitude += source_values[i]*source_values[i];
						}
						if (0.0 < magnitude)
						{
							magnitude = values[0] / magnitude;
							for (i=0;i<field->number_of_components;i++)
							{
								source_values[i] *= magnitude;
							}
							return_code=Computed_field_set_values_at_node_private(
								field->source_fields[0],node,source_values);
						}
						else
						{
							/* not an error; just a warning */
							display_message(WARNING_MESSAGE,
								"Magnitude field %s cannot be inverted for zero vector",
								field->name);
						}
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
			case COMPUTED_FIELD_NODE_ARRAY_VALUE_AT_TIME:
			{
				display_message(ERROR_MESSAGE,"Computed_field_set_values_at_node_private. "
					" %s not implemented yet. Write the code!",field->name);
				/*If we ever need to write the code for this, make it so that we set the array */
				/*values based upon the field->time ONLY if the time correspondes EXACTLY to an*/
				/* array index. No "tking the nearest one" or anything nasty like that */
				return_code=0;
			}break;	
			case COMPUTED_FIELD_NODE_VALUE:
			{
				double double_value;
				enum Value_type value_type;
				float float_value;
				int int_value;
				struct FE_field_component fe_field_component;

				fe_field_component.field=field->fe_field;
				value_type=get_FE_field_value_type(field->fe_field);
				for (i=0;(i<field->number_of_components)&&return_code;i++)
				{
					fe_field_component.number=i;
					/* only set nodal value/versions that exist */
					if (FE_nodal_value_version_exists(node,&fe_field_component,
						field->version_number,field->nodal_value_type))
					{
						switch (value_type)
						{
							case DOUBLE_VALUE:
							{
								double_value=(double)values[i];
								return_code=set_FE_nodal_double_value(node,
									&fe_field_component,j,field->nodal_value_type,double_value);
							} break;
							case FE_VALUE_VALUE:
							{
								return_code=set_FE_nodal_FE_value_value(node,
									&fe_field_component,j,field->nodal_value_type,values[i]);
							} break;
							case FLT_VALUE:
							{
								float_value=(float)values[i];
								return_code=set_FE_nodal_float_value(node,
									&fe_field_component,j,field->nodal_value_type,float_value);
							} break;
							case INT_VALUE:
							{
								int_value=(int)floor(values[i]+0.5);
								return_code=set_FE_nodal_float_value(node,
									&fe_field_component,j,field->nodal_value_type,int_value);
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"Computed_field_set_values_at_node_private.  "
									"Could not set finite_element field %s at node",field->name);
								return_code=0;
							}
						}
					}
				}
				if (!return_code)
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_set_values_at_node_private.  "
						"Could not set node_value field at node",field->name);
					return_code=0;
				}
			} break;			
			case COMPUTED_FIELD_OFFSET:
			{
				/* reverse the offset */
				if (ALLOCATE(source_values,FE_value,field->number_of_components))
				{
					for (i=0;i<field->number_of_components;i++)
					{
						source_values[i] = values[i] - field->source_values[i];
					}
					return_code=Computed_field_set_values_at_node_private(
						field->source_fields[0],node,source_values);
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
					Computed_field_set_values_at_node_private(field->source_fields[0],
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
						return_code=Computed_field_set_values_at_node_private(
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
			case COMPUTED_FIELD_SCALE:
			{
				/* reverse the scaling - unless any scale_factors are zero */
				if (ALLOCATE(source_values,FE_value,field->number_of_components))
				{
					for (i=0;(i<field->number_of_components)&&return_code;i++)
					{
						if (0.0 != field->source_values[i])
						{
							source_values[i] = values[i] / field->source_values[i];
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_set_values_at_node_private.  "
								"Cannot invert scale field %s with zero scale factor",
								field->name);
							return_code=0;
						}
					}
					if (return_code)
					{
						return_code=Computed_field_set_values_at_node_private(
							field->source_fields[0],node,source_values);
					}
					DEALLOCATE(source_values);
				}
				else
				{
					return_code=0;
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_values_at_node_private.  "
					"Cannot set values for field %s of type %s",field->name,
					Computed_field_type_to_string(field->type));
				return_code=0;
			} break;
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_values_at_node_private.  "
				"Failed for field %s of type %s",field->name,
				Computed_field_type_to_string(field->type));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_values_at_node_private.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_values_at_node_private */

int Computed_field_set_values_at_node(struct Computed_field *field,
	struct FE_node *node,FE_value *values,struct MANAGER(FE_node) *node_manager)
/*******************************************************************************
LAST MODIFIED : 22 July 1999

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

	ENTER(Computed_field_set_values_at_node);
	return_code=0;
	if (field&&node&&values&&node_manager)
	{
		/* CREATE with template node is assumed to copy its values */
		if (copy_node=CREATE(FE_node)(0,node))
		{
			/* The node must be accessed as the use of cache on the nodes
				by get values etc. access and deaccessess the nodes */
			ACCESS(FE_node)(copy_node);
			if (Computed_field_set_values_at_node_private(field,copy_node,values))
			{
				return_code=MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)(
					node,copy_node,node_manager);
			}
			Computed_field_clear_cache(field);
			DEACCESS(FE_node)(&copy_node);
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_values_at_node.  Failed");
			Computed_field_clear_cache(field);
		}
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

static int Computed_field_set_values_in_element_private(
	struct Computed_field *field,struct FE_element *element,int *number_in_xi,
	FE_value *values)
/*******************************************************************************
LAST MODIFIED : 1 November 1999

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

Note that the values array will not be modified by this function.
#### Must ensure implemented correctly for new Computed_field_type. ####
???RC Note that some functions are not reversible in this way.
==============================================================================*/
{
	FE_value *source_values;
	int element_dimension,i,j,k,number_of_points,return_code;

	ENTER(Computed_field_set_values_in_element_private);
	if (field&&element&&element->shape&&number_in_xi&&values)
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
					"Computed_field_set_values_in_element_private.  "
					"number_in_xi must be positive");
				return_code=0;
			}
		}
		if (return_code)
		{
			switch (field->type)
			{
				case COMPUTED_FIELD_CLAMP_MAXIMUM:
				{
					FE_value max;

					/* clamps to limits of maximums when setting values too */
					if (ALLOCATE(source_values,FE_value,
						number_of_points*field->number_of_components))
					{
						i=0;
						for (k=0;k<field->number_of_components;k++)
						{
							max=field->source_values[k];
							for (j=0;j<number_of_points;j++)
							{
								if (values[i] > max)
								{
									source_values[i] = max;
								}
								else
								{
									source_values[i] = values[i];
								}
								i++;
							}
						}
						return_code=Computed_field_set_values_in_element_private(
							field->source_fields[0],element,number_in_xi,source_values);
						DEALLOCATE(source_values);
					}
					else
					{
						return_code=0;
					}
				} break;
				case COMPUTED_FIELD_CLAMP_MINIMUM:
				{
					FE_value min;

					/* clamps to limits of maximums when setting values too */
					if (ALLOCATE(source_values,FE_value,
						number_of_points*field->number_of_components))
					{
						i=0;
						for (k=0;k<field->number_of_components;k++)
						{
							min=field->source_values[k];
							for (j=0;j<number_of_points;j++)
							{
								if (values[i] < min)
								{
									source_values[i] = min;
								}
								else
								{
									source_values[i] = values[i];
								}
								i++;
							}
						}
						return_code=Computed_field_set_values_in_element_private(
							field->source_fields[0],element,number_in_xi,source_values);
						DEALLOCATE(source_values);
					}
					else
					{
						return_code=0;
					}
				} break;
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
						return_code=Computed_field_set_values_in_element_private(
							field->source_fields[0],element,number_in_xi,source_values);
						DEALLOCATE(source_values);
					}
					else
					{
						return_code=0;
					}
				} break;
				case COMPUTED_FIELD_COMPOSITE:
				{
					/* set all the scalars separately */
					for (k=0;(k<field->number_of_components)&&return_code;k++)
					{
						return_code=Computed_field_set_values_in_element_private(
							field->source_fields[0],element,number_in_xi,
							values+k*number_of_points);
					}
				} break;
				case COMPUTED_FIELD_DEFAULT_COORDINATE:
				{
					FE_value non_rc_coordinates[3],rc_coordinates[3];
					struct Coordinate_system rc_coordinate_system;

					if (Computed_field_get_default_coordinate_source_field_in_element(
						field,element))
					{
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
								return_code=Computed_field_set_values_in_element_private(
									field->source_fields[0],element,number_in_xi,source_values);
							}
							DEALLOCATE(source_values);
						}
						else
						{
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_set_values_in_element_private.  "
							"Could not get default coordinate source_field");
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
						return_code=Computed_field_set_values_in_element_private(
							field->source_fields[0],element,number_in_xi,source_values);
						DEALLOCATE(source_values);
					}
					else
					{
						return_code=0;
					}
				} break;
				case COMPUTED_FIELD_FINITE_ELEMENT:
				{
					enum Value_type value_type;
					int grid_map_number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS],*int_values,
						offset;

					if (FE_element_field_is_grid_based(element,field->fe_field)&&
						get_FE_element_field_grid_map_number_in_xi(element,
							field->fe_field,grid_map_number_in_xi))
					{
						for (i=0;(i<element_dimension)&&return_code;i++)
						{
							if (number_in_xi[i] != grid_map_number_in_xi[i])
							{
								display_message(ERROR_MESSAGE,
									"Computed_field_set_values_in_element_private.  "
									"Finite element has different grid number_in_xi");
								return_code=0;
							}
						}
						if (return_code)
						{
							value_type=get_FE_field_value_type(field->fe_field);
							switch (value_type)
							{
								case FE_VALUE_VALUE:
								{
									for (k=0;(k<field->number_of_components)&&return_code;k++)
									{
										if (!set_FE_element_field_component_grid_FE_value_values(
											element,field->fe_field,k,values+k*number_of_points))
										{
											display_message(ERROR_MESSAGE,
												"Computed_field_set_values_in_element_private.  "
												"Unable to set finite element grid FE_value values");
											return_code=0;
										}
									}
								} break;
								case INT_VALUE:
								{
									if (ALLOCATE(int_values,int,number_of_points))
									{
										for (k=0;(k<field->number_of_components)&&return_code;k++)
										{
											offset=k*number_of_points;
											for (j=0;j<number_of_points;j++)
											{
												/*???RC this conversion could be a little dodgy */
												int_values[j]=(int)(values[offset+j]);
											}
											if (!set_FE_element_field_component_grid_int_values(
												element,field->fe_field,k,int_values))
											{
												display_message(ERROR_MESSAGE,
													"Computed_field_set_values_in_element_private.  "
													"Unable to set finite element grid int values");
												return_code=0;
											}
										}
										DEALLOCATE(int_values);
									}
									else
									{
										return_code=0;
									}
								} break;
								default:
								{
									display_message(ERROR_MESSAGE,
										"Computed_field_set_values_in_element_private.  "
										"Unsupported value_type for finite_element field");
									return_code=0;
								} break;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_set_values_in_element_private.  "
							"Finite element field %s is not grid based in element",
							field->name);
						return_code=0;
					}
				} break;
				case COMPUTED_FIELD_MAGNITUDE:
				{
					FE_value magnitude;
					int zero_warning;

					/* need current field values to "magnify" */
					if (Computed_field_get_values_in_element(field->source_fields[0],
						element,number_in_xi,&source_values))
					{
						zero_warning=0;
						for (j=0;j<number_of_points;j++)
						{
							/* if the source field is not a zero vector, set its magnitude to
								 the given value */
							magnitude = 0.0;
							for (k=0;k<field->number_of_components;k++)
							{
								magnitude += source_values[k*number_of_points+j]*
									source_values[k*number_of_points+j];
							}
							if (0.0 < magnitude)
							{
								magnitude = values[j] / magnitude;
								for (k=0;k<field->number_of_components;k++)
								{
									source_values[k*number_of_points+j] *= magnitude;
								}
							}
							else
							{
								zero_warning=1;
							}
						}
						if (zero_warning)
						{
							/* not an error; just a warning */
							display_message(WARNING_MESSAGE,
								"Magnitude field %s cannot be inverted for zero vectors",
								field->name);
						}
						return_code=Computed_field_set_values_in_element_private(
							field->source_fields[0],element,number_in_xi,source_values);
						DEALLOCATE(source_values);
					}
					else
					{
						return_code=0;
					}
				} break;
				case COMPUTED_FIELD_OFFSET:
				{
					FE_value offset_value;
					int offset;

					/* reverse the offset */
					if (ALLOCATE(source_values,FE_value,
						number_of_points*field->number_of_components))
					{
						for (k=0;k<field->number_of_components;k++)
						{
							offset=k*number_of_points;
							offset_value=field->source_values[k];
							for (j=0;j<number_of_points;j++)
							{
								source_values[offset+j] = values[offset+j] - offset_value;
							}
						}
						return_code=Computed_field_set_values_in_element_private(
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
							return_code=Computed_field_set_values_in_element_private(
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
									return_code=Computed_field_set_values_in_element_private(
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
				case COMPUTED_FIELD_SCALE:
				{
					FE_value scale_value;
					int offset;

					/* reverse the scaling */
					if (ALLOCATE(source_values,FE_value,
						number_of_points*field->number_of_components))
					{
						for (k=0;(k<field->number_of_components)&&return_code;k++)
						{
							offset=k*number_of_points;
							scale_value=field->source_values[k];
							if (0.0 != scale_value)
							{
								for (j=0;j<number_of_points;j++)
								{
									source_values[offset+j] = values[offset+j] / scale_value;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Computed_field_set_values_in_element_private.  "
									"Cannot invert scale field %s with zero scale factor",
									field->name);
								return_code=0;
							}
						}
						if (return_code)
						{
							return_code=Computed_field_set_values_in_element_private(
								field->source_fields[0],element,number_in_xi,source_values);
						}
						DEALLOCATE(source_values);
					}
					else
					{
						return_code=0;
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_set_values_in_element_private.  "
						"Cannot set values for field %s of type %s",field->name,
						Computed_field_type_to_string(field->type));
					return_code=0;
				} break;
			}
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_values_in_element_private.  "
				"Failed for field %s of type %s",field->name,
				Computed_field_type_to_string(field->type));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_values_in_element_private.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_values_in_element_private */

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
	struct FE_element *element,int *number_in_xi,FE_value *values,
	struct MANAGER(FE_element) *element_manager)
/*******************************************************************************
LAST MODIFIED : 28 October 1999

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

	ENTER(Computed_field_set_values_in_element);
	return_code=0;
	if (field&&element&&number_in_xi&&values&&element_manager)
	{
		/* CREATE with template element is assumed to copy its values */
		if (copy_element=CREATE(FE_element)(element->identifier,element))
		{
			/* The element must be accessed as the use of cache on the elements
				by get values etc. access and deaccessess the elements */
			ACCESS(FE_element)(copy_element);
			if (Computed_field_set_values_in_element_private(field,copy_element,
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
				"Computed_field_set_values_in_element.  Failed");
			Computed_field_clear_cache(field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_values_in_element.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_values_in_element */

struct Computed_field_update_nodal_values_from_source_data
/*******************************************************************************
LAST MODIFIED : 5 October 1999

DESCRIPTION :
==============================================================================*/
{
	int count;
	FE_value *values;
	struct Computed_field *source_field;
	struct Computed_field *destination_field;
	struct MANAGER(FE_node) *node_manager;
};

int Computed_field_update_nodal_values_from_source_sub(
	struct FE_node *node, void *data_void)
/*******************************************************************************
LAST MODIFIED : 5 October 1999

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_update_nodal_values_from_source_data *data;

	ENTER(Computed_field_update_nodal_values_from_source_sub);
	if (node && 
		(data=(struct Computed_field_update_nodal_values_from_source_data *)data_void))
	{
		return_code = 1;
		if (Computed_field_is_defined_at_node(data->source_field, node))
		{
			if(Computed_field_evaluate_at_node(data->source_field, node, data->values))
			{
				if (Computed_field_set_values_at_node(data->destination_field,
					node, data->values, data->node_manager))
				{
					data->count++;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_update_nodal_values_from_source_sub."
			"  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_update_nodal_values_from_source_sub */

int Computed_field_update_nodal_values_from_source(
	struct Computed_field *destination_field,	struct Computed_field *source_field,
	struct GROUP(FE_node) *node_group, struct MANAGER(FE_node) *node_manager)
/*******************************************************************************
LAST MODIFIED : 1 November 1999

DESCRIPTION :
Set <destination_field> in all the nodes in <node_group> or <node_manager> if
not supplied to the values from <source_field>.
==============================================================================*/
{
	int return_code;
	struct Computed_field_update_nodal_values_from_source_data data;

	ENTER(Computed_field_update_nodal_values_from_source);
	return_code=0;
	if (destination_field&&source_field&&node_manager)
	{
		if (Computed_field_get_number_of_components(source_field) ==
			 Computed_field_get_number_of_components(destination_field))
		{
			if (ALLOCATE(data.values, FE_value, 
				Computed_field_get_number_of_components(source_field)))
			{
				data.source_field = source_field;
				data.destination_field = destination_field;
				data.node_manager = node_manager;
				data.count = 0;
				MANAGER_BEGIN_CACHE(FE_node)(node_manager);
				if (node_group)
				{
					FOR_EACH_OBJECT_IN_GROUP(FE_node)(
						Computed_field_update_nodal_values_from_source_sub,
						(void *)&data, node_group);
					if (data.count != NUMBER_IN_GROUP(FE_node)(node_group))
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_update_nodal_values_from_source."
							"  Only able to set values for %d nodes out of %d\n"
							"  Either source field isn't defined at node or destination field could not be set.",
							data.count, NUMBER_IN_GROUP(FE_node)(node_group));
					}
				}
				else
				{
					FOR_EACH_OBJECT_IN_MANAGER(FE_node)(
						Computed_field_update_nodal_values_from_source_sub,
						(void *)&data, node_manager);
					if (data.count != NUMBER_IN_MANAGER(FE_node)(node_manager))
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_update_nodal_values_from_source."
							"  Only able to set values for %d nodes out of %d\n"
							"  Either source field isn't defined at node or destination field could not be set.",
							data.count, NUMBER_IN_MANAGER(FE_node)(node_manager));
					}
				}
				MANAGER_END_CACHE(FE_node)(node_manager);
				DEALLOCATE(data.values);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_update_nodal_values_from_source."
					"  Unable to allocate value storage.");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_update_nodal_values_from_source."
				"  Number of components in source and destination fields must match.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_update_nodal_values_from_source."
			"  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_update_nodal_values_from_source */

struct Computed_field_update_element_values_from_source_data
/*******************************************************************************
LAST MODIFIED : 1 November 1999

DESCRIPTION :
==============================================================================*/
{
	int attempt_count,success_count;
	struct Computed_field *source_field;
	struct Computed_field *destination_field;
	struct MANAGER(FE_element) *element_manager;
};

int Computed_field_update_element_values_from_source_sub(
	struct FE_element *element, void *data_void)
/*******************************************************************************
LAST MODIFIED : 1 November 1999

DESCRIPTION :
==============================================================================*/
{
	FE_value *values;
	int number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS],return_code;
	struct Computed_field_update_element_values_from_source_data *data;

	ENTER(Computed_field_update_element_values_from_source_sub);
	if (element && (data=
		(struct Computed_field_update_element_values_from_source_data *)data_void))
	{
		return_code = 1;
		/* elements with informatio only - no faces or lines */
		if (element->information)
		{
			data->attempt_count++;
			if (Computed_field_is_defined_in_element(data->source_field, element))
			{
				if (Computed_field_get_native_discretization_in_element(
					data->destination_field,element,number_in_xi))
				{
					if (Computed_field_get_values_in_element(data->source_field,element,
						number_in_xi,&values))
					{
						if (Computed_field_set_values_in_element(data->destination_field,
							element,number_in_xi,values,data->element_manager))
						{
							data->success_count++;
						}
						DEALLOCATE(values);
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_update_element_values_from_source_sub."
			"  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_update_element_values_from_source_sub */

int Computed_field_update_element_values_from_source(
	struct Computed_field *destination_field,	struct Computed_field *source_field,
	struct GROUP(FE_element) *element_group,
	struct MANAGER(FE_element) *element_manager)
/*******************************************************************************
LAST MODIFIED : 1 November 1999

DESCRIPTION :
Set grid-based <destination_field> in all the elements in <element_group> or
<element_manager> if not supplied to the values from <source_field>.
==============================================================================*/
{
	int return_code;
	struct Computed_field_update_element_values_from_source_data data;

	ENTER(Computed_field_update_element_values_from_source);
	return_code=0;
	if (destination_field&&source_field&&element_manager)
	{
		if (Computed_field_get_number_of_components(source_field) ==
			 Computed_field_get_number_of_components(destination_field))
		{
			data.source_field = source_field;
			data.destination_field = destination_field;
			data.element_manager = element_manager;
			data.attempt_count = 0;
			data.success_count = 0;
			MANAGER_BEGIN_CACHE(FE_element)(element_manager);
			if (element_group)
			{
				FOR_EACH_OBJECT_IN_GROUP(FE_element)(
					Computed_field_update_element_values_from_source_sub,
					(void *)&data, element_group);
				if (data.success_count != data.attempt_count)
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_update_element_values_from_source."
						"  Only able to set values for %d elements out of %d\n"
						"  Either source field isn't defined at element or destination "
						"field could not be set.",
						data.success_count,data.attempt_count);
				}
			}
			else
			{
				FOR_EACH_OBJECT_IN_MANAGER(FE_element)(
					Computed_field_update_element_values_from_source_sub,
					(void *)&data, element_manager);
				if (data.success_count != data.attempt_count)
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_update_element_values_from_source."
						"  Only able to set values for %d elements out of %d\n"
						"  Either source field isn't defined at element or destination "
						"field could not be set.",
						data.success_count,data.attempt_count);
				}
			}
			MANAGER_END_CACHE(FE_element)(element_manager);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_update_element_values_from_source."
				"  Number of components in source and destination fields must match.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_update_element_values_from_source."
			"  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_update_element_values_from_source */

int Computed_field_get_native_discretization_in_element(
	struct Computed_field *field,struct FE_element *element,int *number_in_xi)
/*******************************************************************************
LAST MODIFIED : 28 October 1999 

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
Computed_field_set_values_in_element_private.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_native_discretization_in_element);
	if (field&&element&&number_in_xi&&element->shape&&
		(MAXIMUM_ELEMENT_XI_DIMENSIONS>=element->shape->dimension))
	{
		switch (field->type)
		{
			case COMPUTED_FIELD_DEFAULT_COORDINATE:
			{
				if (Computed_field_get_default_coordinate_source_field_in_element(
					field,element))
				{
					return_code=
						Computed_field_get_native_discretization_in_element(
							field->source_fields[0],element,number_in_xi);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_get_native_discretization_in_element.  "
						"Could not get default coordinate source_field");
					return_code=0;
				}
				/* must clear the cache to remove default coordinate source field */
				Computed_field_clear_cache(field);
			} break;
			case COMPUTED_FIELD_FINITE_ELEMENT:
			{
				if (FE_element_field_is_grid_based(element,field->fe_field))
				{
					return_code=get_FE_element_field_grid_map_number_in_xi(element,
						field->fe_field,number_in_xi);
				}
				else
				{
					return_code=0;
				}
			} break;
			case COMPUTED_FIELD_CLAMP_MAXIMUM:
			case COMPUTED_FIELD_CLAMP_MINIMUM:
			case COMPUTED_FIELD_COMPONENT:
			case COMPUTED_FIELD_COMPOSITE:
				/* note: only find out if first field of composite grid based! */
			case COMPUTED_FIELD_EDIT_MASK:
			case COMPUTED_FIELD_MAGNITUDE:
			case COMPUTED_FIELD_OFFSET:
			case COMPUTED_FIELD_RC_COORDINATE:
			case COMPUTED_FIELD_RC_VECTOR:
			case COMPUTED_FIELD_SCALE:
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
LAST MODIFIED : 29 October 1999

DESCRIPTION :
Returns an allocated string containing the name of <component_no> of <field>.
Name of the component depends on the type of the Computed_field; for example,
COMPUTED_FIELD_FINITE_ELEMENT gets component names from the FE_field it is
wrapping. Default names are made out of the character form of the
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
		switch (field->type)
		{		 
			case COMPUTED_FIELD_FINITE_ELEMENT:
			case COMPUTED_FIELD_NODE_ARRAY_VALUE_AT_TIME:
			case COMPUTED_FIELD_NODE_VALUE:
			{
				component_name=get_FE_field_component_name(field->fe_field,component_no);
			} break;
			default:
			{
				sprintf(temp_name,"%i",component_no+1);
				if (!(component_name=duplicate_string(temp_name)))
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_get_component_name.  Not enough memory");
				}
			} break;
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

char *Computed_field_type_to_string(enum Computed_field_type field_type)
/*******************************************************************************
LAST MODIFIED : 4 November 1999

DESCRIPTION :
Returns a pointer to a static string token for the given <field_type>.
The calling function must not deallocate the returned string.
#### Must ensure implemented correctly for new Computed_field_type. ####
==============================================================================*/
{
	char *field_type_string;

	ENTER(Computed_field_type_to_string);
	switch (field_type)
	{
		case COMPUTED_FIELD_2D_STRAIN:
		{
			field_type_string="2D_strain";
		} break;
		case COMPUTED_FIELD_ADD:
		{
			field_type_string="add";
		} break;
		case COMPUTED_FIELD_CLAMP_MAXIMUM:
		{
			field_type_string="clamp_maximum";
		} break;
		case COMPUTED_FIELD_CLAMP_MINIMUM:
		{
			field_type_string="clamp_minimum";
		} break;
		case COMPUTED_FIELD_CMISS_NUMBER:
		{
			field_type_string="cmiss_number";
		} break;
		case COMPUTED_FIELD_COMPONENT:
		{
			field_type_string="component";
		} break;
		case COMPUTED_FIELD_COMPOSE:
		{
			field_type_string="compose";
		} break;
		case COMPUTED_FIELD_COMPOSITE:
		{
			field_type_string="composite";
		} break;
		case COMPUTED_FIELD_CONSTANT:
		{
			field_type_string="constant";
		} break;
		case COMPUTED_FIELD_CUBIC_TEXTURE_COORDINATES:
		{
			field_type_string="cubic_texture_coordinates";
		} break;
		case COMPUTED_FIELD_CURVE_LOOKUP:
		{
			field_type_string="curve_lookup";
		} break;
		case COMPUTED_FIELD_DEFAULT_COORDINATE:
		{
			field_type_string="default_coordinate";
		} break;
		case COMPUTED_FIELD_DOT_PRODUCT:
		{
			field_type_string="dot_product";
		} break;
		case COMPUTED_FIELD_EDIT_MASK:
		{
			field_type_string="edit_mask";
		} break;
		case COMPUTED_FIELD_EMBEDDED:
		{
			field_type_string="embedded";
		} break;
		case COMPUTED_FIELD_EXTERNAL:
		{
			field_type_string="external";
		} break;
		case COMPUTED_FIELD_FIBRE_AXES:
		{
			field_type_string="fibre_axes";
		} break;
		case COMPUTED_FIELD_FIBRE_SHEET_AXES:
		{
			field_type_string="fibre_sheet_axes";
		} break;
		case COMPUTED_FIELD_FINITE_ELEMENT:
		{
			field_type_string="finite_element";
		} break;
		case COMPUTED_FIELD_GRADIENT:
		{
			field_type_string="gradient";
		} break;
		case COMPUTED_FIELD_MAGNITUDE:
		{
			field_type_string="magnitude";
		} break;
		case COMPUTED_FIELD_NODE_VALUE:
		{
			field_type_string="node_value";
		} break;
		case COMPUTED_FIELD_NODE_ARRAY_VALUE_AT_TIME:
		{
			field_type_string="node_array_value_at_time";
		} break;
		case COMPUTED_FIELD_OFFSET:
		{
			field_type_string="offset";
		} break;
		case COMPUTED_FIELD_RC_COORDINATE:
		{
			field_type_string="rc_coordinate";
		} break;
		case COMPUTED_FIELD_RC_VECTOR:
		{
			field_type_string="rc_vector";
		} break;
		case COMPUTED_FIELD_SAMPLE_TEXTURE:
		{
			field_type_string="sample_texture";
		} break;
		case COMPUTED_FIELD_SCALE:
		{
			field_type_string="scale";
		} break;
		case COMPUTED_FIELD_SUM_COMPONENTS:
		{
			field_type_string="sum_components";
		} break;
		case COMPUTED_FIELD_XI_COORDINATES:
		{
			field_type_string="xi_coordinates";
		} break;
		case COMPUTED_FIELD_XI_TEXTURE_COORDINATES:
		{
			field_type_string="xi_texture_coordinates";
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

int Computed_field_get_type_2D_strain(struct Computed_field *field,
	struct Computed_field **deformed_coordinate_field,
	struct Computed_field **undeformed_coordinate_field,
	struct Computed_field **fibre_angle_field)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
If the field is of type COMPUTED_FIELD_2D_STRAIN, the undeformed and deformed
coordinate fields and the fibre angle field used by it are returned 
- otherwise an error is reported.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_2D_strain);
	if (field&&(COMPUTED_FIELD_2D_STRAIN==field->type)&&
		undeformed_coordinate_field && deformed_coordinate_field
		&& fibre_angle_field)
	{
		/* source_fields: 0=deformed_coordinate_field,
			1=undeformed_coordinate_field,
			2=fibre_angle_field */
		*deformed_coordinate_field=field->source_fields[0];
		*undeformed_coordinate_field=field->source_fields[1];
		*fibre_angle_field=field->source_fields[2];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_2D_strain.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_2D_strain */

int Computed_field_set_type_2D_strain(struct Computed_field *field,
	struct Computed_field *deformed_coordinate_field,
	struct Computed_field *undeformed_coordinate_field,
	struct Computed_field *fibre_angle_field)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_2D_STRAIN, combining a 
deformed_coordinate_field, undeformed_coordinate_field and
fibre_angle_field.  Sets the number of components to 4.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
The <coordinate_field>s must have no more than 3 components.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_2D_strain);
	if (field&&deformed_coordinate_field&&
		(3>=deformed_coordinate_field->number_of_components)&&
		undeformed_coordinate_field&&
		(3>=undeformed_coordinate_field->number_of_components)
		&&fibre_angle_field)
	{
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=3;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_2D_STRAIN;
			field->number_of_components=4;
			/* source_fields: 0=scalar, 1=coordinate */
			source_fields[0]=ACCESS(Computed_field)(deformed_coordinate_field);
			source_fields[1]=ACCESS(Computed_field)(undeformed_coordinate_field);
			source_fields[2]=ACCESS(Computed_field)(fibre_angle_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_2D_strain.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_2D_strain.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_2D_strain */

int Computed_field_get_type_add(struct Computed_field *field,
	struct Computed_field **source_field1, FE_value *scale_factor1,
	struct Computed_field **source_field2, FE_value *scale_factor2)
/*******************************************************************************
LAST MODIFIED : 2 August 1999

DESCRIPTION :
If the field is of type ADD, the 2 fields and scale factors it adds are returned
- otherwise an error is reported.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_add);
	if (field&&(COMPUTED_FIELD_ADD==field->type)&&source_field1&&scale_factor1&&
		source_field2&&scale_factor2)
	{
		*source_field1 = field->source_fields[0];
		*scale_factor1 = field->source_values[0];
		*source_field2 = field->source_fields[1];
		*scale_factor2 = field->source_values[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_add.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_add */

int Computed_field_set_type_add(struct Computed_field *field,
	struct Computed_field *source_field1, FE_value scale_factor1,
	struct Computed_field *source_field2, FE_value scale_factor2)
/*******************************************************************************
LAST MODIFIED : 2 August 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_ADD, which adds together the two fields,
each multiplied by their respective scale factors, so that eg. subtract is also
handled by it. Both source fields must have the same number of components.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	FE_value *source_values;
	int number_of_source_fields,number_of_source_values,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_add);
	if (field&&source_field1&&source_field2)
	{
		if (source_field1->number_of_components ==
			source_field2->number_of_components)
		{
			/* 1. make dynamic allocations for any new type-specific data */
			number_of_source_fields=2;
			number_of_source_values=2;
			if (ALLOCATE(source_fields,struct Computed_field *,
				number_of_source_fields)&&
				ALLOCATE(source_values,FE_value,number_of_source_values))
			{
				/* 2. free current type-specific data */
				Computed_field_clear_type(field);
				/* 3. establish the new type */
				field->type=COMPUTED_FIELD_ADD;
				field->number_of_components=source_field1->number_of_components;
				source_fields[0]=ACCESS(Computed_field)(source_field1);
				source_fields[1]=ACCESS(Computed_field)(source_field2);
				field->source_fields=source_fields;
				field->number_of_source_fields=number_of_source_fields;
				source_values[0]=scale_factor1;
				source_values[1]=scale_factor2;
				field->source_values=source_values;
				field->number_of_source_values=number_of_source_values;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_add.  Not enough memory");
				DEALLOCATE(source_fields);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Computed_field_set_type_add."
				"  Source fields must have the same number of components");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_add.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_add */

int Computed_field_get_type_clamp_maximum(struct Computed_field *field,
	struct Computed_field **source_field,FE_value **maximums)
/*******************************************************************************
LAST MODIFIED : 22 June 1999

DESCRIPTION :
If the field is of type COMPUTED_FIELD_CLAMP_MAXIMUM, the source_field and
maximums used by it are returned. Since the number of maximums is
equal to the number of components in the source_field, 
this function returns in *maximums a pointer to an allocated array
containing the FE_values.
It is up to the calling function to DEALLOCATE the returned <*maximums>.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_get_type_clamp_maximum);
	if (field&&(COMPUTED_FIELD_CLAMP_MAXIMUM==field->type)&&source_field&&
		maximums)
	{
		if (ALLOCATE(*maximums,FE_value,field->source_fields[0]->number_of_components))
		{
			*source_field=field->source_fields[0];
			for (i=0;i<field->source_fields[0]->number_of_components;i++)
			{
				(*maximums)[i]=field->source_values[i];
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_clamp_maximum.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_clamp_maximum.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_clamp_maximum */

int Computed_field_set_type_clamp_maximum(struct Computed_field *field,
	struct Computed_field *source_field,FE_value *maximums)
/*******************************************************************************
LAST MODIFIED : 22 June 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_CLAMP_MAXIMUM, returning the <source_field>
with each component clamp_maximumd by its respective FE_value in <maximums>.
The <maximums> array must therefore contain as many FE_values as there are
components in <source_field>.
Sets the number of components to the same as <source_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	FE_value *source_values;
	int i,number_of_source_fields,number_of_source_values,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_clamp_maximum);
	if (field&&source_field&&maximums)
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
			field->type=COMPUTED_FIELD_CLAMP_MAXIMUM;
			field->number_of_components=source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			for (i=0;i<number_of_source_values;i++)
			{
				source_values[i]=maximums[i];
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
			"Computed_field_set_type_clamp_maximum.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_clamp_maximum */

int Computed_field_get_type_clamp_minimum(struct Computed_field *field,
	struct Computed_field **source_field,FE_value **minimums)
/*******************************************************************************
LAST MODIFIED : 24 June 1999

DESCRIPTION :
If the field is of type COMPUTED_FIELD_CLAMP_MINIMUM, the source_field and
minimums used by it are returned. Since the number of minimums is
equal to the number of components in the source_field, 
this function returns in *minimums a pointer to an allocated array
containing the FE_values.
It is up to the calling function to DEALLOCATE the returned <*minimums>.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_get_type_clamp_minimum);
	if (field&&(COMPUTED_FIELD_CLAMP_MINIMUM==field->type)&&source_field&&
		minimums)
	{
		if (ALLOCATE(*minimums,FE_value,field->source_fields[0]->number_of_components))
		{
			*source_field=field->source_fields[0];
			for (i=0;i<field->source_fields[0]->number_of_components;i++)
			{
				(*minimums)[i]=field->source_values[i];
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_clamp_minimum.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_clamp_minimum.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_clamp_minimum */

int Computed_field_set_type_clamp_minimum(struct Computed_field *field,
	struct Computed_field *source_field,FE_value *minimums)
/*******************************************************************************
LAST MODIFIED : 24 June 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_CLAMP_MINIMUM, returning the <source_field>
with each component clamp_minimumd by its respective FE_value in <minimums>.
The <minimums> array must therefore contain as many FE_values as there are
components in <source_field>.
Sets the number of components to the same as <source_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	FE_value *source_values;
	int i,number_of_source_fields,number_of_source_values,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_clamp_minimum);
	if (field&&source_field&&minimums)
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
			field->type=COMPUTED_FIELD_CLAMP_MINIMUM;
			field->number_of_components=source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			for (i=0;i<number_of_source_values;i++)
			{
				source_values[i]=minimums[i];
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
			"Computed_field_set_type_clamp_minimum.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_clamp_minimum */

int Computed_field_set_type_cmiss_number(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 24 June 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_CMISS_NUMBER, which can be evaluated
as a single component field - ie. the node/element/face/line number is converted
into an FE_value - or more commonly, is evaluated as a string name.
Sets the number of components to 1.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_set_type_cmiss_number);
	if (field)
	{
		/* 1. make dynamic allocations for any new type-specific data */
		/* none */
		/* 2. free current type-specific data */
		Computed_field_clear_type(field);
		/* 3. establish the new type */
		field->type=COMPUTED_FIELD_CMISS_NUMBER;
		field->number_of_components=1;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_cmiss_number.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_cmiss_number */

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
	struct Computed_field **calculate_values_field)
/*******************************************************************************
LAST MODIFIED : 15 October 1999

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
		find_element_xi_field&&calculate_values_field)
	{
		*texture_coordinate_field = field->source_fields[0];
		*find_element_xi_field = field->source_fields[1];
		*calculate_values_field = field->source_fields[2];
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
	struct Computed_field *calculate_values_field)
/*******************************************************************************
LAST MODIFIED : 15 October 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_COMPOSE, this field allows you to
evaluate one field to find "texture coordinates", use a find_element_xi field
to then calculate a corresponding element/xi and finally calculate values using
this element/xi and a third field.  You can then evaluate values on a "host"
mesh for any points "contained" inside.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields, return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_compose);
	if (field&&texture_coordinate_field&&find_element_xi_field&&
		calculate_values_field)
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

int Computed_field_get_type_composite(struct Computed_field *field,
	int *number_of_scalars,struct Computed_field ***scalar_fields)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
If the field is of type COMPUTED_FIELD_COMPOSITE, the function allocates and
returns in <**scalar_fields> an array containing the <number_of_scalars> scalar
fields making up the composite field - otherwise an error is reported.
It is up to the calling function to DEALLOCATE the returned array. Note that the
fields in the returned array are not ACCESSed.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_get_type_composite);
	if (field&&(COMPUTED_FIELD_COMPOSITE==field->type)&&number_of_scalars&&
		scalar_fields)
	{
		*number_of_scalars=field->number_of_source_fields;
		if (ALLOCATE(*scalar_fields,struct Computed_field *,*number_of_scalars))
		{
			for (i=0;i<(*number_of_scalars);i++)
			{
				(*scalar_fields)[i]=field->source_fields[i];
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_composite.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_composite.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_composite */

int Computed_field_set_type_composite(struct Computed_field *field,
	int number_of_scalars,struct Computed_field **scalar_fields)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_COMPOSITE, returning a collection of
scalar fields as one vector. Useful for constructing artificial coordinate
fields for 2-D and 3-D plotting (eg. pressure vs. temperature vs. depth).
<scalar_fields> must point to an array of <number_of_scalars> pointers to
single-component Computed_fields. The resulting field will have as many
components as <number_of_scalars>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int i,number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_composite);
	if (field&&(0<number_of_scalars)&&scalar_fields)
	{
		return_code=1;
		/* make sure scalar_fields are all non-NULL */
		for (i=0;return_code&&(i<number_of_scalars);i++)
		{
			if (!(scalar_fields[i]&&(1==scalar_fields[i]->number_of_components)))
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_composite.  Invalid scalar_fields");
				return_code=0;
			}
		}
		if (return_code)
		{
			/* 1. make dynamic allocations for any new type-specific data */
			number_of_source_fields=number_of_scalars;
			if (ALLOCATE(source_fields,struct Computed_field *,
				number_of_source_fields))
			{
				/* 2. free current type-specific data */
				Computed_field_clear_type(field);
				/* 3. establish the new type */
				field->type=COMPUTED_FIELD_COMPOSITE;
				field->number_of_components=number_of_scalars;
				for (i=0;i<number_of_scalars;i++)
				{
					source_fields[i]=ACCESS(Computed_field)(scalar_fields[i]);
				}
				field->source_fields=source_fields;
				field->number_of_source_fields=number_of_source_fields;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_composite.  Not enough memory");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_composite.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_composite */

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
Sets the number of components to 3.
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
			field->number_of_components=3;
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

int Computed_field_get_type_curve_lookup(struct Computed_field *field,
	struct Control_curve **curve,struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 4 November 1999

DESCRIPTION :
If the field is of type COMPUTED_FIELD_CURVE_LOOKUP, the curve and
source_field used by it are returned - otherwise an error is reported.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_curve_lookup);
	if (field&&(COMPUTED_FIELD_CURVE_LOOKUP==field->type)&&curve&&
		source_field)
	{
		*curve=field->curve;
		*source_field=field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_curve_lookup.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_curve_lookup */

int Computed_field_set_type_curve_lookup(struct Computed_field *field,
	struct Control_curve *curve,struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_CURVE_LOOKUP, returning the value of
<curve> at the time/parameter value given by scalar <source_field>.
Sets number of components to same number as <curve>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_curve_lookup);
	if (field&&curve&&source_field&&(1==source_field->number_of_components))
	{
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_CURVE_LOOKUP;
			field->number_of_components=Control_curve_get_number_of_components(curve);
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			field->curve=ACCESS(Control_curve)(curve);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_curve_lookup.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_curve_lookup.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_curve_lookup */

int Computed_field_get_type_default_coordinate(struct Computed_field *field,
	struct MANAGER(Computed_field) **computed_field_manager)
/*******************************************************************************
LAST MODIFIED : 25 January 1999

DESCRIPTION :
If the field is of type COMPUTED_FIELD_DEFAULT_COORDINATE, the computed field
manager referred to by it is returned - otherwise an error is reported.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_default_coordinate);
	if (field&&(COMPUTED_FIELD_DEFAULT_COORDINATE==field->type)&&
		computed_field_manager)
	{
		*computed_field_manager=field->computed_field_manager;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_default_coordinate.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_default_coordinate */

int Computed_field_set_type_default_coordinate(struct Computed_field *field,
	struct MANAGER(Computed_field) *computed_field_manager)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_DEFAULT_COORDINATE, which returns the
values/derivatives of the first [coordinate] field defined for the element/node
in rectangular cartesian coordinates. This type is intended to replace the
NULL coordinate_field option in the calculate_FE_element_field_values function.
When a field of this type is calculated at and element/node, the evaluate
function finds the first FE_field (coordinate type) defined over it, then gets
its Computed_field wrapper from the manager and proceeds from there.
Consequences of this behaviour are:
- the field allocates its source_fields to point to the computed_field for the
actual coordinate field in the evaluate phase.
- when the source field changes the current one's cache is cleared and it is
deaccessed.
- when the cache is cleared, so is any reference to the source_field.
- always performs the conversion to RC since cannot predict the coordinate
system used by the eventual source_field. Coordinate_system of this type of
field need not be RC, although it usually will be.
Sets number of components to 3.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_set_type_default_coordinate);
	if (field&&computed_field_manager)
	{
		/* 1. make dynamic allocations for any new type-specific data */
		/* none until the evaluate phase */
		/* 2. free current type-specific data */
		Computed_field_clear_type(field);
		/* 3. establish the new type */
		field->type=COMPUTED_FIELD_DEFAULT_COORDINATE;
		field->number_of_components=3;
		field->computed_field_manager=computed_field_manager;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_default_coordinate.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_default_coordinate */

int Computed_field_get_type_dot_product(struct Computed_field *field,
	struct Computed_field **source_field1, struct Computed_field **source_field2)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
If the field is of type DOT_PRODUCT, the source fields used
by it is returned - otherwise an error is reported.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_dot_product);
	if (field&&(COMPUTED_FIELD_DOT_PRODUCT==field->type)
		&&source_field1&&source_field2)
	{
		*source_field1=field->source_fields[0];
		*source_field2=field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_dot_product.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_dot_product */

int Computed_field_set_type_dot_product(struct Computed_field *field,
	struct Computed_field *source_field1, struct Computed_field *source_field2)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_DOT_PRODUCT, 
which returns the dot product of the components of the two source fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_dot_product);
	if (field&&source_field1&&source_field2)
	{
		if (source_field1->number_of_components == source_field2->number_of_components)
		{
			/* 1. make dynamic allocations for any new type-specific data */
			number_of_source_fields=2;
			if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
			{
				/* 2. free current type-specific data */
				Computed_field_clear_type(field);
				/* 3. establish the new type */
				field->type=COMPUTED_FIELD_DOT_PRODUCT;
				field->number_of_components=1;
				source_fields[0]=ACCESS(Computed_field)(source_field1);
				source_fields[1]=ACCESS(Computed_field)(source_field2);
				field->source_fields=source_fields;
				field->number_of_source_fields=number_of_source_fields;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_dot_product.  Not enough memory");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_dot_product."
				"  Source fields must have the same number of components");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_dot_product.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_dot_product */

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

int Computed_field_get_type_embedded(struct Computed_field *field,
	struct FE_field **fe_field, struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 23 April 1999

DESCRIPTION :
If the field is of type COMPUTED_FIELD_EMBEDDED, the FE_field being
"wrapped" by it is returned - otherwise an error is reported.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_embedded);
	if (field&&(COMPUTED_FIELD_EMBEDDED==field->type)&&fe_field&&source_field)
	{
		*fe_field=field->fe_field;
		*source_field=field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_embedded.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_embedded */

int Computed_field_set_type_embedded(struct Computed_field *field,
	struct FE_field *fe_field, struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 23 April 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_EMBEDDED, wrapping the given
<fe_field>. Makes the number of components the same as in the <fe_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields, return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_embedded);
	if (field&&fe_field&&source_field)
	{
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_EMBEDDED;
			field->number_of_components=Computed_field_get_number_of_components(source_field);
			field->fe_field=ACCESS(FE_field)(fe_field);
			/* source_fields: 0=field_which_nodes_are_embedded_in */
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_embedded.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_embedded.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_embedded */

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

int Computed_field_get_type_finite_element(struct Computed_field *field,
	struct FE_field **fe_field)
/*******************************************************************************
LAST MODIFIED : 8 February 1999

DESCRIPTION :
If the field is of type COMPUTED_FIELD_FINITE_ELEMENT, the FE_field being
"wrapped" by it is returned - otherwise an error is reported.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_finite_element);
	if (field&&(COMPUTED_FIELD_FINITE_ELEMENT==field->type)&&fe_field)
	{
		*fe_field=field->fe_field;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_finite_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_finite_element */

int Computed_field_set_type_finite_element(struct Computed_field *field,
	struct FE_field *fe_field)
/*******************************************************************************
LAST MODIFIED : 31 August 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_FINITE_ELEMENT, wrapping the given
<fe_field>. Makes the number of components the same as in the <fe_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	enum Value_type value_type;
	int return_code;

	ENTER(Computed_field_set_type_finite_element);
	if (field&&fe_field)
	{
		/* 1. make dynamic allocations for any new type-specific data */
		/* NOTE: no dynamic data for type COMPUTED_FIELD_FINITE_ELEMENT */
		/* 2. free current type-specific data */
		Computed_field_clear_type(field);
		/* 3. establish the new type */
		field->type=COMPUTED_FIELD_FINITE_ELEMENT;
		/* if the value_type of the fe_field is not numerical then the number of
			 components from this computed field is zero */
		value_type=get_FE_field_value_type(fe_field);
		switch (value_type)
		{
			case DOUBLE_VALUE:
			case FE_VALUE_VALUE:
			case FLT_VALUE:
			case INT_VALUE:
			{
				field->number_of_components =
					get_FE_field_number_of_components(fe_field);
			} break;
			default:
			{
				field->number_of_components = 0;
			} break;
		}
		field->fe_field=ACCESS(FE_field)(fe_field);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_finite_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_finite_element */

int Computed_field_get_type_fibre_axes(struct Computed_field *field,
	struct Computed_field **fibre_field,struct Computed_field **coordinate_field)
/*******************************************************************************
LAST MODIFIED : 8 February 1999

DESCRIPTION :
If the field is of type COMPUTED_FIELD_FIBRE_AXES, the fibre and coordinate
fields used by it are returned - otherwise an error is reported.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_fibre_axes);
	if (field&&(COMPUTED_FIELD_FIBRE_AXES==field->type)&&fibre_field&&
		coordinate_field)
	{
		/* source_fields: 0=fibre, 1=coordinate */
		*fibre_field=field->source_fields[0];
		*coordinate_field=field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_fibre_axes.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_fibre_axes */

int Computed_field_set_type_fibre_axes(struct Computed_field *field,
	struct Computed_field *fibre_field,struct Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_FIBRE_AXES, combining a fibre and
coordinate field to return the 3, 3-component fibre axis vectors:
fibre  = fibre direction,
sheet  = fibre normal in the plane of the sheet,
normal = normal to the fibre sheet.
Sets the number of components to 9.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.

Both the fibre and coordinate fields must have no more than 3 components. The
fibre field is expected to have a FIBRE coordinate_system, although this is not
enforced.
???RC To enforce the fibre field to have a FIBRE coordinate_system, must make
the MANAGER_COPY_NOT_IDENTIFIER fail if it would change the coordinate_system
while the field is in use. Not sure if we want that restriction.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_fibre_axes);
	if (field&&fibre_field&&(3>=fibre_field->number_of_components)&&
		coordinate_field&&(3>=coordinate_field->number_of_components))
	{
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=2;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_FIBRE_AXES;
			field->number_of_components=9;
			/* source_fields: 0=fibre, 1=coordinate */
			source_fields[0]=ACCESS(Computed_field)(fibre_field);
			source_fields[1]=ACCESS(Computed_field)(coordinate_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_fibre_axes.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_fibre_axes.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_fibre_axes */

int Computed_field_get_type_fibre_sheet_axes(struct Computed_field *field,
	struct Computed_field **fibre_field,struct Computed_field **coordinate_field)
/*******************************************************************************
LAST MODIFIED : 8 February 1999

DESCRIPTION :
If the field is of type COMPUTED_FIELD_FIBRE_SHEET_AXES, the fibre and
coordinate fields used by it are returned - otherwise an error is reported.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_fibre_sheet_axes);
	if (field&&(COMPUTED_FIELD_FIBRE_SHEET_AXES==field->type)&&fibre_field&&
		coordinate_field)
	{
		/* source_fields: 0=fibre, 1=coordinate */
		*fibre_field=field->source_fields[0];
		*coordinate_field=field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_fibre_sheet_axes.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_fibre_sheet_axes */

int Computed_field_set_type_fibre_sheet_axes(struct Computed_field *field,
	struct Computed_field *fibre_field,struct Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_FIBRE_SHEET_AXES. This works just like
COMPUTED_FIELD_FIBRE_AXES except that the 3 vectors are returned in the order
sheet,fibre,normal. Useful for streamline tracking along fibre normals in the
sheet, rather than the fibre itself.
Sets the number of components to 9.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.

Both the fibre and coordinate fields must have no more than 3 components. The
fibre field is expected to have a FIBRE coordinate_system, although this is not
enforced.
???RC To enforce the fibre field to have a FIBRE coordinate_system, must make
the MANAGER_COPY_NOT_IDENTIFIER fail if it would change the coordinate_system
while the field is in use. Not sure if we want that restriction.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_fibre_sheet_axes);
	if (field&&fibre_field&&(3>=fibre_field->number_of_components)&&
		coordinate_field&&(3>=coordinate_field->number_of_components))
	{
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=2;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_FIBRE_SHEET_AXES;
			field->number_of_components=9;
			/* source_fields: 0=fibre, 1=coordinate */
			source_fields[0]=ACCESS(Computed_field)(fibre_field);
			source_fields[1]=ACCESS(Computed_field)(coordinate_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_fibre_sheet_axes.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_fibre_sheet_axes.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_fibre_sheet_axes */

int Computed_field_get_type_gradient(struct Computed_field *field,
	struct Computed_field **scalar_field,struct Computed_field **coordinate_field)
/*******************************************************************************
LAST MODIFIED : 29 December 1998

DESCRIPTION :
If the field is of type COMPUTED_FIELD_GRADIENT, the scalar and coordinate
fields used by it are returned - otherwise an error is reported.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_gradient);
	if (field&&(COMPUTED_FIELD_GRADIENT==field->type)&&scalar_field&&
		coordinate_field)
	{
		/* source_fields: 0=scalar, 1=coordinate */
		*scalar_field=field->source_fields[0];
		*coordinate_field=field->source_fields[1];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_gradient.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_gradient */

int Computed_field_set_type_gradient(struct Computed_field *field,
	struct Computed_field *scalar_field,struct Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_GRADIENT, combining a scalar and
coordinate field to return a vector (eg. velocity from the potential scalar).
Sets the number of components to 3.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
The <coordinate_field> must have no more than 3 components.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_gradient);
	if (field&&scalar_field&&(1==scalar_field->number_of_components)&&
		coordinate_field&&(3>=coordinate_field->number_of_components))
	{
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=2;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_GRADIENT;
			field->number_of_components=3;
			/* source_fields: 0=scalar, 1=coordinate */
			source_fields[0]=ACCESS(Computed_field)(scalar_field);
			source_fields[1]=ACCESS(Computed_field)(coordinate_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_gradient.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_gradient.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_gradient */

int Computed_field_get_type_magnitude(struct Computed_field *field,
	struct Computed_field **source_field)
/*******************************************************************************
LAST MODIFIED : 5 February 1999

DESCRIPTION :
If the field is of type COMPUTED_FIELD_MAGNITUDE, the source field used
by it is returned - otherwise an error is reported.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_magnitude);
	if (field&&(COMPUTED_FIELD_MAGNITUDE==field->type)&&source_field)
	{
		*source_field=field->source_fields[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_magnitude.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_magnitude */

int Computed_field_set_type_magnitude(struct Computed_field *field,
	struct Computed_field *source_field)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_MAGNITUDE, which returns a scalar value
equal to the square root of the sum of the squares of all components in the
<source_field>. Sets the number of components to 1.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_magnitude);
	if (field&&source_field)
	{
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_MAGNITUDE;
			field->number_of_components=1;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_magnitude.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_magnitude.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_magnitude */

int Computed_field_get_type_node_value(struct Computed_field *field,
	struct FE_field **fe_field,enum FE_nodal_value_type *nodal_value_type,
	int *version_number)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
If the field is of type COMPUTED_FIELD_NODE_VALUE, the fe_field,
nodal_value_type and version_number it extracts are returned - otherwise an
error is reported.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_node_value);
	if (field&&(COMPUTED_FIELD_NODE_VALUE==field->type)&&fe_field&&
		nodal_value_type&&version_number)
	{
		*fe_field=field->fe_field;
		*nodal_value_type=field->nodal_value_type;
		*version_number=field->version_number;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_node_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_node_value */

int Computed_field_set_type_node_value(struct Computed_field *field,
	struct FE_field *fe_field,enum FE_nodal_value_type nodal_value_type,
	int version_number)
/*******************************************************************************
LAST MODIFIED : 27 June 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_NODE_VALUE, returning the values for the
given <nodal_value_type> and <version_number> of <fe_field> at a node.
Makes the number of components the same as in the <fe_field>.
Field automatically takes the coordinate system of the source fe_field. See note
at start of this file about changing use of coordinate systems.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_set_type_node_value);
	if (field&&fe_field&&(FE_NODAL_UNKNOWN!=nodal_value_type)&&
		(0<=version_number))
	{
		/* 1. make dynamic allocations for any new type-specific data */
		/* NOTE: no dynamic data for type COMPUTED_FIELD_NODE_VALUE */
		/* 2. free current type-specific data */
		Computed_field_clear_type(field);
		/* 3. establish the new type */
		field->type=COMPUTED_FIELD_NODE_VALUE;
		/* copy the coordinate system from the fe_field */
		COPY(Coordinate_system)(&(field->coordinate_system),
			get_FE_field_coordinate_system(fe_field));
		/* If the value_type of the fe_field is no FE_VALUE_VALUE then
			the number of components from this computed field is zero */
		if (FE_VALUE_VALUE==get_FE_field_value_type(fe_field))
		{
			field->number_of_components = get_FE_field_number_of_components(fe_field);
		}
		else
		{
			field->number_of_components = 0;
		}
		field->fe_field=ACCESS(FE_field)(fe_field);
		field->nodal_value_type=nodal_value_type;
		field->version_number=version_number;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_node_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_node_value */

int Computed_field_get_type_node_array_value_at_time(struct Computed_field *field,
	struct FE_field **fe_field,enum FE_nodal_value_type *nodal_value_type,
	int *version_number,struct Computed_field **time_field)
/*******************************************************************************
LAST MODIFIED : 28 October 1999

DESCRIPTION :
If the field is of type COMPUTED_FIELD_NODE_ARRAY_VALUE_AT_TIME, the fe_field,
nodal_value_type and version_number it extracts are returned - otherwise an
error is reported.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_node_array_value_at_time);
	if (field&&(COMPUTED_FIELD_NODE_ARRAY_VALUE_AT_TIME==field->type)&&fe_field&&
		nodal_value_type&&version_number)
	{
		*fe_field=field->fe_field;
		*nodal_value_type=field->nodal_value_type;
		*version_number=field->version_number;
		USE_PARAMETER(time_field);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_node_array_value_at_time.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_node_array_value_at_time */

int Computed_field_set_type_node_array_value_at_time(struct Computed_field *field,
	struct FE_field *fe_field,enum FE_nodal_value_type nodal_value_type,
	int version_number,struct Computed_field *time_field)
/*******************************************************************************
LAST MODIFIED : 28 October 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_NODE_ARRAY_VALUE_AT_TIME, 
returning the values for the
given <nodal_value_type> and <version_number> of <fe_field> at a node.
Makes the number of components the same as in the <fe_field>.
Field automatically takes the coordinate system of the source fe_field. See note
at start of this file about changing use of coordinate systems.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int return_code,number_of_source_fields;
	enum Value_type value_type;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_node_array_value_at_time);
	if (field&&fe_field&&(FE_NODAL_UNKNOWN!=nodal_value_type)&&
		(0<=version_number))
	{
		USE_PARAMETER(time_field);
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_NODE_ARRAY_VALUE_AT_TIME;
			/* copy the coordinate system from the fe_field */
			COPY(Coordinate_system)(&(field->coordinate_system),
				get_FE_field_coordinate_system(fe_field));
			/* If the value_type of the fe_field is not an array type then
				 the number of components from this computed field is zero */				
			value_type=get_FE_field_value_type(fe_field);
			if ((value_type==FE_VALUE_ARRAY_VALUE)||(value_type==FLT_ARRAY_VALUE)||
				(value_type==UNSIGNED_ARRAY_VALUE)||(value_type==SHORT_ARRAY_VALUE)||
				(value_type==INT_ARRAY_VALUE))
			{
				field->number_of_components = get_FE_field_number_of_components(fe_field);
			}
			else
			{
				field->number_of_components = 0;
			}
			field->fe_field=ACCESS(FE_field)(fe_field);
			field->nodal_value_type=nodal_value_type;
			field->version_number=version_number;
			source_fields[0]=ACCESS(Computed_field)(time_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_node_array_value_at_time.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_node_array_value_at_time */

int Computed_field_get_type_offset(struct Computed_field *field,
	struct Computed_field **source_field,FE_value **offsets)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
If the field is of type COMPUTED_FIELD_OFFSET, the source_field and
offsets used by it are returned. Since the number of offsets is
equal to the number of components in the source_field (and you don't know this
yet), this function returns in *offsets a pointer to an allocated array
containing the FE_values.
It is up to the calling function to DEALLOCATE the returned <*offsets>.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_get_type_offset);
	if (field&&(COMPUTED_FIELD_OFFSET==field->type)&&source_field&&
		offsets)
	{
		if (ALLOCATE(*offsets,FE_value,
			field->source_fields[0]->number_of_components))
		{
			*source_field=field->source_fields[0];
			for (i=0;i<field->source_fields[0]->number_of_components;i++)
			{
				(*offsets)[i]=field->source_values[i];
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_offset.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_offset.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_offset */

int Computed_field_set_type_offset(struct Computed_field *field,
	struct Computed_field *source_field,FE_value *offsets)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_OFFSET, returning the <source_field>
with each component offsetd by its respective FE_value in <offsets>.
The <offsets> array must therefore contain as many FE_values as there are
components in <source_field>.
Sets the number of components to the same as <source_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	FE_value *source_values;
	int i,number_of_source_fields,number_of_source_values,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_offset);
	if (field&&source_field&&offsets)
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
			field->type=COMPUTED_FIELD_OFFSET;
			field->number_of_components=source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			for (i=0;i<number_of_source_values;i++)
			{
				source_values[i]=offsets[i];
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
			"Computed_field_set_type_offset.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_offset */

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
		Computed_field_has_1_to_3_components(coordinate_field,(void *)NULL))
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

int Computed_field_get_type_sample_texture(struct Computed_field *field,
	struct Computed_field **texture_coordinate_field, struct Texture **texture)
/*******************************************************************************
LAST MODIFIED : 15 March 1999

DESCRIPTION :
If the field is of type COMPUTED_FIELD_SAMPLE_TEXTURE, the 
texture_coordinate_field and texure itself are returned - 
otherwise an error is reported.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_sample_texture);
	if (field&&(COMPUTED_FIELD_SAMPLE_TEXTURE==field->type)&&
		texture&&texture_coordinate_field)
	{
		*texture_coordinate_field=field->source_fields[0];
		*texture = field->texture;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_sample_texture.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_sample_texture */

int Computed_field_set_type_sample_texture(struct Computed_field *field,
	struct Computed_field *texture_coordinate_field, struct Texture *texture)
/*******************************************************************************
LAST MODIFIED : 16 March 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_SAMPLE_TEXTURE with the supplied
texture.  Sets the number of components to 3.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int number_of_source_fields,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_sample_texture);

	if (field&&texture&&(3>=texture_coordinate_field->number_of_components))
	{
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=1;
		if (ALLOCATE(source_fields,struct Computed_field *,number_of_source_fields))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_SAMPLE_TEXTURE;
			field->number_of_components=3;
			field->texture = ACCESS(Texture)(texture);
			source_fields[0]=ACCESS(Computed_field)(texture_coordinate_field);
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
			"Computed_field_set_type_sample_texture.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_sample_texture */

int Computed_field_get_type_scale(struct Computed_field *field,
	struct Computed_field **source_field,FE_value **scale_factors)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
If the field is of type COMPUTED_FIELD_SCALE, the source_field and
scale_factors used by it are returned. Since the number of scale_factors is
equal to the number of components in the source_field (and you don't know this
yet), this function returns in *scale_factors a pointer to an allocated array
containing the FE_values.
It is up to the calling function to DEALLOCATE the returned <*scale_factors>.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_get_type_scale);
	if (field&&(COMPUTED_FIELD_SCALE==field->type)&&source_field&&
		scale_factors)
	{
		if (ALLOCATE(*scale_factors,FE_value,
			field->source_fields[0]->number_of_components))
		{
			*source_field=field->source_fields[0];
			for (i=0;i<field->source_fields[0]->number_of_components;i++)
			{
				(*scale_factors)[i]=field->source_values[i];
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_scale.  Not enough memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_scale.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_scale */

int Computed_field_set_type_scale(struct Computed_field *field,
	struct Computed_field *source_field,FE_value *scale_factors)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_SCALE, returning the <source_field>
with each component scaled by its respective FE_value in <scale_factors>.
The <scale_factors> array must therefore contain as many FE_values as there are
components in <source_field>.
Sets the number of components to the same as <source_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	FE_value *source_values;
	int i,number_of_source_fields,number_of_source_values,return_code;
	struct Computed_field **source_fields;

	ENTER(Computed_field_set_type_scale);
	if (field&&source_field&&scale_factors)
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
			field->type=COMPUTED_FIELD_SCALE;
			field->number_of_components=source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			for (i=0;i<number_of_source_values;i++)
			{
				source_values[i]=scale_factors[i];
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
			"Computed_field_set_type_scale.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_scale */

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

int Computed_field_set_type_xi_coordinates(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 24 June 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_XI_COORDINATES, defined only over elements and
returning the xi that is passed to it.
Sets the number of components to 3.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
???RC May want the number of components to be user-selectable; for now, stick
with 3.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_set_type_xi_coordinates);
	if (field)
	{
		/* 1. make dynamic allocations for any new type-specific data */
		/* none */
		/* 2. free current type-specific data */
		Computed_field_clear_type(field);
		/* 3. establish the new type */
		field->type=COMPUTED_FIELD_XI_COORDINATES;
		field->number_of_components=3;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_xi_coordinates.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_xi_coordinates */

int Computed_field_get_type_xi_texture_coordinates(struct Computed_field *field,
	struct FE_element **seed_element)
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
If the field is of type COMPUTED_FIELD_XI_TEXTURE_COORDINATES, 
the seed element used for the mapping is returned - otherwise an error is reported.
Use function Computed_field_get_type to determine the field type.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_type_xi_texture_coordinates);
	if (field&&seed_element)
	{
		*seed_element=field->seed_element;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_xi_texture_coordinates.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_xi_texture_coordinates */

static int write_Computed_field_element_texture_mapping(
	struct Computed_field_element_texture_mapping *mapping,
	void *user_data)
{
	USE_PARAMETER(user_data);
	printf("Mapping %p Element %p (%f %f %f)\n",
		mapping, mapping->element,
		mapping->offset[0], mapping->offset[1], mapping->offset[2]);

	return( 1 );
}

static int Computed_field_xi_texture_coordinates_add_neighbours(
	struct Computed_field_element_texture_mapping *mapping_item,
	struct LIST(Computed_field_element_texture_mapping) *texture_mapping,
	struct Computed_field_element_texture_mapping_fifo **last_to_be_checked)
/*******************************************************************************
LAST MODIFIED : 6 July 1999

DESCRIPTION :
Add the neighbours that haven't already been put in the texture_mapping list and 
puts each new member in the texture_mapping list and the to_be_checked list.
==============================================================================*/
{
	FE_value xi[3];
	int face_number, i, return_code;
	struct Computed_field_element_texture_mapping *mapping_neighbour;
	struct Computed_field_element_texture_mapping_fifo *fifo_node;
	struct FE_element *neighbour_element;

	ENTER(Computed_field_xi_texture_coordinates_add_neighbours);
	if (mapping_item && texture_mapping && last_to_be_checked &&
		(*last_to_be_checked))
	{
		return_code=1;
		
		for (i = 0;return_code&&(i<(mapping_item->element->shape->dimension*2));i++)
		{
			xi[0] = 0.5;
			xi[1] = 0.5;
			xi[2] = 0.5;
			switch (i)
			{
				case 0:
				{
					xi[0] = 0.0;
				} break;
				case 1:
				{
					xi[0] = 1.0;
				} break;
				case 2:
				{
					xi[1] = 0.0;
				} break;
				case 3:
				{
					xi[1] = 1.0;
				} break;
				case 4:
				{
					xi[2] = 0.0;
				} break;
				case 5:
				{
					xi[2] = 1.0;
				} break;
			}
			if ((FE_element_shape_find_face_number_for_xi(mapping_item->element->shape, xi,
				&face_number)) && (neighbour_element = 
				adjacent_FE_element(mapping_item->element, face_number)))
			{
				if(!(mapping_neighbour = FIND_BY_IDENTIFIER_IN_LIST(
					Computed_field_element_texture_mapping, element)
					(neighbour_element, texture_mapping)))
				{
					if (ALLOCATE(fifo_node,
						struct Computed_field_element_texture_mapping_fifo,1)&&
						(mapping_neighbour = 
							CREATE(Computed_field_element_texture_mapping)()))
					{
						REACCESS(FE_element)(&mapping_neighbour->element, neighbour_element);
						mapping_neighbour->offset[0] = mapping_item->offset[0];
						mapping_neighbour->offset[1] = mapping_item->offset[1];
						mapping_neighbour->offset[2] = mapping_item->offset[2];
						switch (i)
						{
							case 0:
							{
								mapping_neighbour->offset[0] -= 1.0;
							} break;
							case 1:
							{
								mapping_neighbour->offset[0] += 1.0;
							} break;
							case 2:
							{
								mapping_neighbour->offset[1] -= 1.0;
							} break;
							case 3:
							{
								mapping_neighbour->offset[1] += 1.0;
							} break;
							case 4:
							{
								mapping_neighbour->offset[2] -= 1.0;
							} break;
							case 5:
							{
								mapping_neighbour->offset[2] += 1.0;
							} break;
						}
						if (ADD_OBJECT_TO_LIST(Computed_field_element_texture_mapping)(
							mapping_neighbour, texture_mapping))
						{
							/* fill the fifo_node for the mapping_item; put at end of list */
							fifo_node->mapping_item=mapping_neighbour;
							fifo_node->next=
								(struct Computed_field_element_texture_mapping_fifo *)NULL;
							(*last_to_be_checked)->next = fifo_node;
							(*last_to_be_checked) = fifo_node;
						}
						else
						{
							printf("Error adding neighbour\n");
							write_Computed_field_element_texture_mapping(mapping_neighbour, NULL);
							printf("Texture mapping list\n");
							FOR_EACH_OBJECT_IN_LIST(Computed_field_element_texture_mapping)(
								write_Computed_field_element_texture_mapping, NULL, texture_mapping);
							DEALLOCATE(fifo_node);
							DESTROY(Computed_field_element_texture_mapping)(
								&mapping_neighbour);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_set_type_xi_texture_coordinates.  "
							"Unable to allocate member");
						DEALLOCATE(fifo_node);
						return_code=0;
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_xi_texture_coordinates_add_neighbours.  Invalid argument(s)");
		return_code=0;
	}

	return(return_code);
	LEAVE;
} /* Computed_field_xi_texture_coordinates_add_neighbours */

int Computed_field_set_type_xi_texture_coordinates(struct Computed_field *field,
	struct FE_element *seed_element, struct MANAGER(FE_element) *fe_element_manager)
/*******************************************************************************
LAST MODIFIED : 6 July 1999

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_XI_TEXTURE_COORDINATES.
The seed element is set to the number given and the mapping calculated.
Sets the number of components to the dimension of the given element.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int return_code;
	struct CM_element_information cm;
	struct Computed_field_element_texture_mapping *mapping_item;
	struct FE_element *element;
	struct LIST(Computed_field_element_texture_mapping) *texture_mapping;
	struct Computed_field_element_texture_mapping_fifo *fifo_node,
		*first_to_be_checked,*last_to_be_checked;

	ENTER(Computed_field_set_type_xi_texture_coordinates);
	if (field&&seed_element)
	{
		first_to_be_checked=last_to_be_checked=
			(struct Computed_field_element_texture_mapping_fifo *)NULL;
		/* 1. make dynamic allocations for any new type-specific data */
		if (texture_mapping = CREATE_LIST(Computed_field_element_texture_mapping)())
		{
			/* Add seed element */
			cm.number=seed_element->cm.number;
			cm.type=seed_element->cm.type;
			if ((element=FIND_BY_IDENTIFIER_IN_MANAGER(FE_element,identifier)(
				&cm,fe_element_manager)) && (element==seed_element))
			{
				if (ALLOCATE(fifo_node,
					struct Computed_field_element_texture_mapping_fifo,1)&&
					(mapping_item=CREATE(Computed_field_element_texture_mapping)()))
				{
					REACCESS(FE_element)(&mapping_item->element, element);
					ADD_OBJECT_TO_LIST(Computed_field_element_texture_mapping)
						(mapping_item, texture_mapping);
					/* fill the fifo_node for the mapping_item; put at end of list */
					fifo_node->mapping_item=mapping_item;
					fifo_node->next=
						(struct Computed_field_element_texture_mapping_fifo *)NULL;
					first_to_be_checked=last_to_be_checked=fifo_node;
					return_code = 1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_set_type_xi_texture_coordinates.  "
						"Unable to allocate member");
					DEALLOCATE(fifo_node);
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_type_xi_texture_coordinates.  "
					"Unable to find seed element");
				return_code=0;
			}
			if (return_code)
			{
				while (return_code && first_to_be_checked)
				{
					return_code = Computed_field_xi_texture_coordinates_add_neighbours(
						first_to_be_checked->mapping_item, texture_mapping,
						&last_to_be_checked);

#if defined (DEBUG)
					printf("Item removed\n");
					write_Computed_field_element_texture_mapping(mapping_item, NULL);
#endif /* defined (DEBUG) */

					/* remove first_to_be_checked */
					fifo_node=first_to_be_checked;
					if (!(first_to_be_checked=first_to_be_checked->next))
					{
						last_to_be_checked=
							(struct Computed_field_element_texture_mapping_fifo *)NULL;
					}
					DEALLOCATE(fifo_node);

#if defined (DEBUG)
					printf("Texture mapping list\n");
					FOR_EACH_OBJECT_IN_LIST(Computed_field_element_texture_mapping)(
						write_Computed_field_element_texture_mapping, NULL, texture_mapping);
					printf("To be checked list\n");
					FOR_EACH_OBJECT_IN_LIST(Computed_field_element_texture_mapping)(
						write_Computed_field_element_texture_mapping, NULL, to_be_checked);
#endif /* defined (DEBUG) */
				}
			}
			/* clean up to_be_checked list */
			while (first_to_be_checked)
			{
				fifo_node = first_to_be_checked;
				first_to_be_checked = first_to_be_checked->next;
				DEALLOCATE(fifo_node);
			}
			if (!return_code)
			{
				DESTROY_LIST(Computed_field_element_texture_mapping)(&texture_mapping);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_type_xi_texture_coordinates.  Unable to create list");
			return_code=0;
		}
		if(return_code)
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type=COMPUTED_FIELD_XI_TEXTURE_COORDINATES;
			field->number_of_components=seed_element->shape->dimension;
			field->source_fields=(struct Computed_field **)NULL;
			field->number_of_source_fields=0;
			field->seed_element = ACCESS(FE_element)(seed_element);
			field->texture_mapping = texture_mapping;
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_xi_texture_coordinates.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_xi_texture_coordinates */

int Computed_field_has_1_component(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 22 January 1999

DESCRIPTION :
Iterator/conditional function returning true if <field> has only one component.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_has_1_component);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=(1 == field->number_of_components);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_1_component.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_has_1_component */


int Computed_field_has_at_least_1_component(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 15 June 1999

DESCRIPTION :
Conditional function returning true if <field> has at least one component.
Ignores fields that are listed for selection but cannot return FE_values
(i.e. FE_fields with values other than FE_values)
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_has_1_component);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=(0 < field->number_of_components);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_at_least_1_component.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_has_at_least_1_component */

int Computed_field_has_1_to_3_components(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 22 January 1999

DESCRIPTION :
Iterator/conditional function returning true if <field> has from one to three
components - useful for selecting vector/coordinate fields.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_has_1_to_3_components);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=((3 >= field->number_of_components)&&
			(1 <= field->number_of_components));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_1_to_3_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_has_1_to_3_components */

int Computed_field_has_1_to_4_components(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 12 April 1999

DESCRIPTION :
Iterator/conditional function returning true if <field> has from one to three
components - useful for selecting vector/coordinate fields.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_has_1_to_4_components);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=((4 >= field->number_of_components)&&
			(1 <= field->number_of_components));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_1_to_4_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_has_1_to_4_components */

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

static int Computed_field_element_texture_mapping_has_values(
	struct Computed_field_element_texture_mapping *mapping, void *user_data)
/*******************************************************************************
LAST MODIFIED : 26 May 1999

DESCRIPTION :
Compares the user_data values with the offsets in the <mapping>
==============================================================================*/
{
	FE_value *values;
	int return_code;

	ENTER(Computed_field_element_texture_mapping_has_values);
	if (mapping && (values = (FE_value *)user_data))
	{
		return_code = 0;
		if ((values[0] == mapping->offset[0])&&
			 (values[1] == mapping->offset[1])&&
			 (values[2] == mapping->offset[2]))
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_element_texture_mapping_has_values.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_element_texture_mapping_has_values */

int Computed_field_find_element_xi(struct Computed_field *field, FE_value *values,
	int number_of_values, struct FE_element **element, FE_value *xi)
/*******************************************************************************
LAST MODIFIED : 26 May 1999

DESCRIPTION :
This function implements the reverse of some certain computed_fields
(Computed_field_is_find_element_xi_capable) so that it tries to find an element
and xi which would evaluate to the given values.
This has been implemented so that the texture_coordinates can be used to extract
information from textures (sample_texture computed_field) and then modified and
then put back into another texture.
==============================================================================*/
{
	FE_value floor_values[3];
	int i, return_code;
	struct Computed_field_element_texture_mapping *mapping;

	ENTER(Computed_field_find_element_xi);
	if (field&&values&&(number_of_values==field->number_of_components)&&element&&xi)
	{
		switch (field->type)
		{
			case COMPUTED_FIELD_XI_TEXTURE_COORDINATES:
			{
				if (number_of_values<=3)
				{
					for (i = 0 ; i < number_of_values ; i++)
					{
						floor_values[i] = floor(values[i]);
					}
					for ( ; i < 3 ; i++)
					{
						floor_values[i] = 0.0;
					}
					if (field->texture_mapping)
					{
						return_code=0;
						/* Check the last successful mapping first */
						if (field->find_element_xi_mapping&&
							Computed_field_element_texture_mapping_has_values(
							field->find_element_xi_mapping, (void *)floor_values))
						{
							return_code = 1;
						}
						else
						{
							/* Find in the list */
							if (mapping = FIRST_OBJECT_IN_LIST_THAT(Computed_field_element_texture_mapping)
								(Computed_field_element_texture_mapping_has_values, (void *)floor_values,
								field->texture_mapping))
							{
								REACCESS(Computed_field_element_texture_mapping)(&(field->find_element_xi_mapping),
									mapping);
								return_code = 1;
							}
						}
						if (return_code)
						{
							*element = field->find_element_xi_mapping->element;
							for (i = 0 ; i < (*element)->shape->dimension ; i++)
							{
								xi[i] = values[i] - floor_values[i];	
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_find_element_xi.  Unable to find mapping for given values");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_find_element_xi.  Xi texture coordinate mapping not calculated");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_find_element_xi.  Only implemented for three or less values");
					return_code=0;
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_find_element_xi.  Field type not implemented");
				return_code=0;
			} break;
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
LAST MODIFIED : 15 October 1999

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
			case COMPUTED_FIELD_XI_TEXTURE_COORDINATES:
			{
				return_code=1;
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
} /* Computed_field_is_find_element_xi_capableC */

int set_Computed_field_conditional(struct Parse_state *state,
	void *field_address_void,void *set_field_data_void)
/*******************************************************************************
LAST MODIFIED : 10 March 1999

DESCRIPTION :
Modifier function to set the field from a command. <set_field_data_void> should
point to a struct Set_Computed_field_conditional_data containing the
computed_field_package and an optional conditional function for narrowing the
range of fields available for selection. If the conditional_function is NULL,
this function works just like set_Computed_field.
==============================================================================*/
{
	char *current_token,*field_component_name,*temp_name;
	int component_no,i,return_code;
	struct Computed_field **field_address,*selected_field,*temp_field;
	struct Computed_field_component field_component;
	struct Set_Computed_field_conditional_data *set_field_data;

	ENTER(set_Computed_field_conditional);
	if (state&&(field_address=(struct Computed_field **)field_address_void)&&
		(set_field_data=
			(struct Set_Computed_field_conditional_data *)set_field_data_void)&&
		set_field_data->computed_field_package)
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
						set_field_data->computed_field_package->computed_field_manager)))
					{
						if (field_component_name=strchr(current_token,'.'))
						{
							*field_component_name='\0';
							field_component_name++;
							if (selected_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,
								name)(current_token,set_field_data->computed_field_package->
									computed_field_manager))
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
										field_component.field=selected_field;
										field_component.component_no=component_no;
										/* try to find an existing wrapper for this component */
										if (temp_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
											Computed_field_wraps_field_component,&field_component,
											set_field_data->computed_field_package->
											computed_field_manager))
										{
											selected_field=temp_field;
										}
										else
										{
											/* make a wrapper for the field component */
											field_component_name--;
											*field_component_name='.';
											if ((temp_field=CREATE(Computed_field)(current_token))&&
												Computed_field_set_type_component(temp_field,
													selected_field,component_no)&&
												ADD_OBJECT_TO_MANAGER(Computed_field)(temp_field,
													set_field_data->computed_field_package->
													computed_field_manager))
											{
												selected_field=temp_field;
												component_no=0;
											}
											else
											{
												display_message(WARNING_MESSAGE,
													"set_Computed_field_component.  Not enough memory");
												DESTROY(Computed_field)(&temp_field);
												selected_field=(struct Computed_field *)NULL;
											}
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
				if (temp_field= *field_address)
				{
					display_message(INFORMATION_MESSAGE,"[%s]",temp_field->name);
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

static int define_Computed_field_type_2D_strain(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_2D_STRAIN (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"deformed_coordinate",NULL,NULL,set_Computed_field_conditional},
		{"undeformed_coordinate",NULL,NULL,set_Computed_field_conditional},
		{"fibre_angle",NULL,NULL,set_Computed_field_conditional},
		{NULL,NULL,NULL,NULL}
	};
	struct Computed_field *deformed_coordinate_field,*field,
		*fibre_angle_field, *undeformed_coordinate_field;
	struct Computed_field_package *computed_field_package;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_scalar_field_data;

	ENTER(define_Computed_field_type_2D_strain);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		deformed_coordinate_field=(struct Computed_field *)NULL;
		undeformed_coordinate_field=(struct Computed_field *)NULL;
		fibre_angle_field=(struct Computed_field *)NULL;
		if (COMPUTED_FIELD_2D_STRAIN==Computed_field_get_type(field))
		{
			return_code=Computed_field_get_type_2D_strain(field,
				&deformed_coordinate_field, &undeformed_coordinate_field,
				&fibre_angle_field);
		}
		else
		{
			if (!(deformed_coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",computed_field_package->computed_field_manager)))
			{
				deformed_coordinate_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_has_1_to_3_components,(void *)NULL,
					computed_field_package->computed_field_manager);
			}
			undeformed_coordinate_field = deformed_coordinate_field;
			if (!(fibre_angle_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"fibres",computed_field_package->computed_field_manager)))
			{
				fibre_angle_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_has_1_to_3_components,(void *)NULL,
					computed_field_package->computed_field_manager);
			}
		}
		if (return_code)
		{
			/* have ACCESS/DEACCESS because set_Computed_field does */
			if (deformed_coordinate_field)
			{
				ACCESS(Computed_field)(deformed_coordinate_field);
			}
			if (undeformed_coordinate_field)
			{
				ACCESS(Computed_field)(undeformed_coordinate_field);
			}
			if (fibre_angle_field)
			{
				ACCESS(Computed_field)(fibre_angle_field);
			}
			/* deformed coordinate */
			set_coordinate_field_data.computed_field_package=computed_field_package;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_1_to_3_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			(option_table[0]).to_be_modified= &deformed_coordinate_field;
			(option_table[0]).user_data= &set_coordinate_field_data;
			/* undeformed coordinate */
			set_coordinate_field_data.computed_field_package=computed_field_package;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_1_to_3_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			(option_table[1]).to_be_modified= &undeformed_coordinate_field;
			(option_table[1]).user_data= &set_coordinate_field_data;
			/* fibre_angle */
			set_scalar_field_data.computed_field_package=computed_field_package;
			set_scalar_field_data.conditional_function=
				Computed_field_has_1_to_3_components;
			set_scalar_field_data.conditional_function_user_data=(void *)NULL;
			(option_table[2]).to_be_modified= &fibre_angle_field;
			(option_table[2]).user_data= &set_scalar_field_data;
			return_code=process_multiple_options(state,option_table)&&
				Computed_field_set_type_2D_strain(field,deformed_coordinate_field,
				undeformed_coordinate_field,	fibre_angle_field);
			if (deformed_coordinate_field)
			{
				DEACCESS(Computed_field)(&deformed_coordinate_field);
			}
			if (undeformed_coordinate_field)
			{
				DEACCESS(Computed_field)(&undeformed_coordinate_field);
			}
			if (fibre_angle_field)
			{
				DEACCESS(Computed_field)(&fibre_angle_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_2D_strain.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_2D_strain */

static int define_Computed_field_type_add(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 2 August 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_ADD (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	FE_value *scale_factors;
	int number_of_scale_factors,return_code;
	static struct Modifier_entry option_table[]=
	{
		{"fields",NULL,NULL,set_Computed_field_array},
		{"scale_factors",NULL,NULL,set_FE_value_array},
		{NULL,NULL,NULL,NULL}
	};
	struct Computed_field *field, **source_fields;
	struct Computed_field_package *computed_field_package;
	struct Set_Computed_field_array_data set_field_array_data;
	struct Set_Computed_field_conditional_data set_field_data;

	ENTER(define_Computed_field_type_add);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		source_fields=(struct Computed_field **)NULL;
		scale_factors=(FE_value *)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 2)&&
			ALLOCATE(scale_factors, FE_value, 2))
		{
			if (COMPUTED_FIELD_ADD==Computed_field_get_type(field))
			{
				return_code=Computed_field_get_type_add(field,
					&(source_fields[0]), &(scale_factors[0]),
					&(source_fields[1]), &(scale_factors[1]));
			}
			else
			{
				if (source_fields[0]=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL,(void *)NULL,
					computed_field_package->computed_field_manager))
				{
					source_fields[1] = source_fields[0];
					/* default scale factors for simple add */
					scale_factors[0] = 1.0;
					scale_factors[1] = 1.0;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_add.  No fields defined");
					return_code=0;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_add.  Not enough memory");
			return_code=0;
		}
		if (return_code)
		{
			/* have ACCESS/DEACCESS because set_Computed_field does */
			ACCESS(Computed_field)(source_fields[0]);
			ACCESS(Computed_field)(source_fields[1]);
			set_field_data.conditional_function=
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
			set_field_data.conditional_function_user_data=(void *)NULL;
			set_field_data.computed_field_package=computed_field_package;
			set_field_array_data.number_of_fields=2;
			set_field_array_data.conditional_data= &set_field_data;
			(option_table[0]).to_be_modified= source_fields;
			(option_table[0]).user_data= &set_field_array_data;
			number_of_scale_factors=2;
			(option_table[1]).to_be_modified= scale_factors;
			(option_table[1]).user_data= &number_of_scale_factors;
			return_code=process_multiple_options(state,option_table);
			if (return_code)
			{
				return_code = Computed_field_set_type_add(field,
					source_fields[0], scale_factors[0],
					source_fields[1], scale_factors[1]);
			}
			DEACCESS(Computed_field)(&source_fields[0]);
			DEACCESS(Computed_field)(&source_fields[1]);
		}
		DEALLOCATE(source_fields);
		DEALLOCATE(scale_factors);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_add.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_add */

int define_Computed_field_type_clamp_maximum(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 22 June 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CLAMP_MAXIMUM (if it is not already)
and allows its contents to be modified.
Must input the field before the maximums since there will be as many
maximums as there are components in the field.
==============================================================================*/
{
	auto struct Modifier_entry
		field_option_table[]=
		{
			{"field",NULL,NULL,set_Computed_field_conditional},
			{NULL,NULL,NULL,NULL}
		},
		maximums_option_table[]=
		{
			{"maximums",NULL,NULL,set_FE_value_array},
			{NULL,NULL,NULL,NULL}
		},
		help_option_table[]=
		{
			{"field",NULL,NULL,set_Computed_field_conditional},
			{"maximums",NULL,NULL,set_FE_value_array},
			{NULL,NULL,NULL,NULL}
		};
	char *current_token;
	FE_value *temp_maximums,*maximums;
	int i,number_of_components,return_code;
	struct Computed_field *field,*source_field;
	struct Computed_field_package *computed_field_package;
	struct Set_Computed_field_conditional_data set_field_data;

	ENTER(define_Computed_field_type_rc_coordinate);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		set_field_data.conditional_function=
			(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
		set_field_data.conditional_function_user_data=(void *)NULL;
		set_field_data.computed_field_package=computed_field_package;
		/* get source_field and maximums - from field if of type clamp_maximum */
		source_field=(struct Computed_field *)NULL;
		maximums=(FE_value *)NULL;
		if (COMPUTED_FIELD_CLAMP_MAXIMUM==field->type)
		{
			return_code=Computed_field_get_type_clamp_maximum(field,
				&source_field,&maximums);
		}
		else
		{
			/* get first available field, and set maximums for it to 1.0 */
			if ((source_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL,(void *)NULL,
				computed_field_package->computed_field_manager))&&
				ALLOCATE(maximums,FE_value,source_field->number_of_components))
			{
				for (i=0;i<source_field->number_of_components;i++)
				{
					maximums[i]=0.0;
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
					(help_option_table[1]).to_be_modified= maximums;
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
					/* save the number of components to maintain current maximums */
					number_of_components=source_field->number_of_components;
					(field_option_table[0]).to_be_modified= &source_field;
					(field_option_table[0]).user_data= &set_field_data;
					if (return_code=process_option(state,field_option_table))
					{
						if (source_field)
						{
							if (REALLOCATE(temp_maximums,maximums,FE_value,
								source_field->number_of_components))
							{
								maximums=temp_maximums;
								/* make any new maximums equal to 0.0 */
								for (i=number_of_components;
									i<source_field->number_of_components;i++)
								{
									maximums[i]=0.0;
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
								"define_Computed_field_type_clamp_maximum.  Invalid field");
							return_code=0;
						}
					}
				}
			}
			/* parse the maximums */
			if (return_code&&state->current_token)
			{
				number_of_components=source_field->number_of_components;
				(maximums_option_table[0]).to_be_modified= maximums;
				(maximums_option_table[0]).user_data= &number_of_components;
				return_code=process_multiple_options(state,maximums_option_table);
			}
			if (return_code)
			{
				return_code=
					Computed_field_set_type_clamp_maximum(field,source_field,maximums);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_clamp_maximum.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			if (maximums)
			{
				DEALLOCATE(maximums);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_clamp_maximum.  "
				"Could not get source_field or maximums");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_clamp_maximum.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_clamp_maximum */

int define_Computed_field_type_clamp_minimum(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 22 June 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CLAMP_MINIMUM (if it is not already)
and allows its contents to be modified.
Must input the field before the minimums since there will be as many
minimums as there are components in the field.
==============================================================================*/
{
	auto struct Modifier_entry
		field_option_table[]=
		{
			{"field",NULL,NULL,set_Computed_field_conditional},
			{NULL,NULL,NULL,NULL}
		},
		minimums_option_table[]=
		{
			{"minimums",NULL,NULL,set_FE_value_array},
			{NULL,NULL,NULL,NULL}
		},
		help_option_table[]=
		{
			{"field",NULL,NULL,set_Computed_field_conditional},
			{"minimums",NULL,NULL,set_FE_value_array},
			{NULL,NULL,NULL,NULL}
		};
	char *current_token;
	FE_value *temp_minimums,*minimums;
	int i,number_of_components,return_code;
	struct Computed_field *field,*source_field;
	struct Computed_field_package *computed_field_package;
	struct Set_Computed_field_conditional_data set_field_data;

	ENTER(define_Computed_field_type_rc_coordinate);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		set_field_data.conditional_function=
			(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
		set_field_data.conditional_function_user_data=(void *)NULL;
		set_field_data.computed_field_package=computed_field_package;
		/* get source_field and minimums - from field if of type clamp_minimum */
		source_field=(struct Computed_field *)NULL;
		minimums=(FE_value *)NULL;
		if (COMPUTED_FIELD_CLAMP_MINIMUM==field->type)
		{
			return_code=Computed_field_get_type_clamp_minimum(field,
				&source_field,&minimums);
		}
		else
		{
			/* get first available field, and set minimums for it to 1.0 */
			if ((source_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL,(void *)NULL,
				computed_field_package->computed_field_manager))&&
				ALLOCATE(minimums,FE_value,source_field->number_of_components))
			{
				for (i=0;i<source_field->number_of_components;i++)
				{
					minimums[i]=0.0;
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
					(help_option_table[1]).to_be_modified= minimums;
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
					/* save the number of components to maintain current minimums */
					number_of_components=source_field->number_of_components;
					(field_option_table[0]).to_be_modified= &source_field;
					(field_option_table[0]).user_data= &set_field_data;
					if (return_code=process_option(state,field_option_table))
					{
						if (source_field)
						{
							if (REALLOCATE(temp_minimums,minimums,FE_value,
								source_field->number_of_components))
							{
								minimums=temp_minimums;
								/* make any new minimums equal to 0.0 */
								for (i=number_of_components;
									i<source_field->number_of_components;i++)
								{
									minimums[i]=0.0;
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
								"define_Computed_field_type_clamp_minimum.  Invalid field");
							return_code=0;
						}
					}
				}
			}
			/* parse the minimums */
			if (return_code&&state->current_token)
			{
				number_of_components=source_field->number_of_components;
				(minimums_option_table[0]).to_be_modified= minimums;
				(minimums_option_table[0]).user_data= &number_of_components;
				return_code=process_multiple_options(state,minimums_option_table);
			}
			if (return_code)
			{
				return_code=
					Computed_field_set_type_clamp_minimum(field,source_field,minimums);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_clamp_minimum.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			if (minimums)
			{
				DEALLOCATE(minimums);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_clamp_minimum.  "
				"Could not get source_field or minimums");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_clamp_minimum.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_clamp_minimum */

static int define_Computed_field_type_cmiss_number(struct Parse_state *state,
	void *field_void,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 24 June 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CMISS_NUMBER.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field;

	ENTER(define_Computed_field_type_cmiss_number);
	USE_PARAMETER(dummy_void);
	if (state&&(field=(struct Computed_field *)field_void))
	{
		if (!state->current_token)
		{
			return_code=Computed_field_set_type_cmiss_number(field);
		}
		else
		{
			if (strcmp(PARSER_HELP_STRING,state->current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
			{
				display_message(ERROR_MESSAGE,
					"Unknown option <%s>",state->current_token);
				display_parse_state_location(state);
			}
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_cmiss_number.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_cmiss_number */

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
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL,(void *)NULL,
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
			{"texture_coordinates_field",NULL,NULL,set_Computed_field_conditional},
			{NULL,NULL,NULL,NULL}
		};
	struct Computed_field *field,*calculate_values_field,*find_element_xi_field,
		*texture_coordinates_field;
	struct Computed_field_package *computed_field_package;
	struct Set_Computed_field_conditional_data set_calculate_values_field_data,
		set_find_element_xi_field_data, set_texture_coordinates_field_data;

	ENTER(define_Computed_field_type_compose);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		set_calculate_values_field_data.computed_field_package=computed_field_package;
		set_calculate_values_field_data.conditional_function=
			(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
		set_calculate_values_field_data.conditional_function_user_data=(void *)NULL;
		set_find_element_xi_field_data.computed_field_package=computed_field_package;
		set_find_element_xi_field_data.conditional_function=
			Computed_field_is_find_element_xi_capable;
		set_find_element_xi_field_data.conditional_function_user_data=(void *)NULL;
		set_texture_coordinates_field_data.computed_field_package=computed_field_package;
		set_texture_coordinates_field_data.conditional_function=
			(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
		set_texture_coordinates_field_data.conditional_function_user_data=(void *)NULL;
		/* get valid parameters for composite field */
		if (COMPUTED_FIELD_COMPOSE==Computed_field_get_type(field))
		{
			return_code=Computed_field_get_type_compose(field,
				&calculate_values_field, &find_element_xi_field,
				&texture_coordinates_field);
		}
		else
		{
			calculate_values_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL ,(void *)NULL,
				computed_field_package->computed_field_manager);
			find_element_xi_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_is_find_element_xi_capable,(void *)NULL,
				computed_field_package->computed_field_manager);
			texture_coordinates_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL ,(void *)NULL,
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
				(option_table[2]).to_be_modified= &texture_coordinates_field;
				(option_table[2]).user_data= &set_texture_coordinates_field_data;
				return_code=process_multiple_options(state,option_table);
			}
			if (return_code)
			{
				return_code=Computed_field_set_type_compose(field,
					texture_coordinates_field, find_element_xi_field,
					calculate_values_field);
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
				DEACCESS(Computed_field)(&(calculate_values_field));
			}
			if (find_element_xi_field)
			{
				DEACCESS(Computed_field)(&(find_element_xi_field));
			}
			if (texture_coordinates_field)
			{
				DEACCESS(Computed_field)(&(texture_coordinates_field));
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

static int define_Computed_field_type_composite(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_COMPOSITE (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	char *current_token;
	int i,number_of_scalars,return_code,temp_number_of_scalars;
	static struct Modifier_entry 
		number_of_scalars_option_table[]=
		{
			{"number_of_scalars",NULL,NULL,set_int_positive},
			{NULL,NULL,NULL,NULL}
		},
		scalars_option_table[]=
		{
			{"scalars",NULL,NULL,set_Computed_field_array},
			{NULL,NULL,NULL,NULL}
		},
		help_option_table[]=
		{
			{"number_of_scalars",NULL,NULL,set_int_positive},
			{"scalars",NULL,NULL,set_Computed_field_array},
			{NULL,NULL,NULL,NULL}
		};
	struct Computed_field *field,**scalar_fields,*temp_field,**temp_scalar_fields;
	struct Computed_field_package *computed_field_package;
	struct Set_Computed_field_array_data set_scalar_field_array_data;
	struct Set_Computed_field_conditional_data set_scalar_field_data;

	ENTER(define_Computed_field_type_composite);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		set_scalar_field_data.computed_field_package=computed_field_package;
		set_scalar_field_data.conditional_function=Computed_field_has_1_component;
		set_scalar_field_data.conditional_function_user_data=(void *)NULL;
		/* get valid parameters for composite field */
		scalar_fields=(struct Computed_field **)NULL;
		if (COMPUTED_FIELD_COMPOSITE==Computed_field_get_type(field))
		{
			return_code=Computed_field_get_type_composite(field,
				&number_of_scalars,&scalar_fields);
		}
		else
		{
			/* ALLOCATE and fill array of scalar fields */
			temp_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_has_1_component,(void *)NULL,
				computed_field_package->computed_field_manager);
			number_of_scalars=1;
			if (ALLOCATE(scalar_fields,struct Computed_field *,number_of_scalars))
			{
				for (i=0;i<number_of_scalars;i++)
				{
					scalar_fields[i]=temp_field;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_composite.  Not enough memory");
				return_code=0;
			}
		}
		if (return_code)
		{
			/* ACCESS the scalar fields for set_Computed_field_array */
			for (i=0;i<number_of_scalars;i++)
			{
				if (scalar_fields[i])
				{
					ACCESS(Computed_field)(scalar_fields[i]);
				}
			}
			/* try to handle help first */
			if (current_token=state->current_token)
			{
				if (!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
				{
					(help_option_table[0]).to_be_modified= &number_of_scalars;
					set_scalar_field_array_data.number_of_fields=number_of_scalars;
					set_scalar_field_array_data.conditional_data= &set_scalar_field_data;
					(help_option_table[1]).to_be_modified= scalar_fields;
					(help_option_table[1]).user_data= &set_scalar_field_array_data;
					return_code=process_multiple_options(state,help_option_table);
				}
			}
			/* parse the number_of_scalars... */
			if (return_code&&(current_token=state->current_token))
			{
				/* ... only if the "number_of_scalars" token is next */
				if (fuzzy_string_compare(current_token,"number_of_scalars"))
				{
					/* keep the number_of_scalars to maintain any current ones */
					temp_number_of_scalars=number_of_scalars;
					(number_of_scalars_option_table[0]).to_be_modified=
						&temp_number_of_scalars;
					if (return_code=process_option(state,number_of_scalars_option_table))
					{
						if (temp_number_of_scalars != number_of_scalars)
						{
							if (ALLOCATE(temp_scalar_fields,struct Computed_field *,
								temp_number_of_scalars))
							{
								for (i=0;i<temp_number_of_scalars;i++)
								{
									if (i<number_of_scalars)
									{
										temp_field=scalar_fields[i];
									}
									/* new array members access last original scalar field */
									temp_scalar_fields[i]=ACCESS(Computed_field)(temp_field);
								}
								/* clean up the previous scalar_fields array */
								for (i=0;i<number_of_scalars;i++)
								{
									DEACCESS(Computed_field)(&(scalar_fields[i]));
								}
								DEALLOCATE(scalar_fields);
								scalar_fields=temp_scalar_fields;
								number_of_scalars=temp_number_of_scalars;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"define_Computed_field_type_composite.  Not enough memory");
								return_code=0;
							}
						}
					}
				}
			}
			/* parse the scalars */
			if (return_code&&state->current_token)
			{
				set_scalar_field_array_data.number_of_fields=number_of_scalars;
				set_scalar_field_array_data.conditional_data= &set_scalar_field_data;
				(scalars_option_table[0]).to_be_modified= scalar_fields;
				(scalars_option_table[0]).user_data= &set_scalar_field_array_data;
				return_code=process_multiple_options(state,scalars_option_table);
			}
			if (return_code)
			{
				return_code=Computed_field_set_type_composite(field,
					number_of_scalars,scalar_fields);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_composite.  Failed");
				}
			}
			/* clean up the scalar fields array */
			for (i=0;i<number_of_scalars;i++)
			{
				if (scalar_fields[i])
				{
					DEACCESS(Computed_field)(&(scalar_fields[i]));
				}
			}
			DEALLOCATE(scalar_fields);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_composite.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_composite */

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
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL,(void *)NULL,
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
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
			set_field_data.conditional_function_user_data=(void *)NULL;
			set_field_data.computed_field_package=computed_field_package;
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

static int define_Computed_field_type_curve_lookup(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 4 November 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_CURVE_LOOKUP (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"source",NULL,NULL,set_Computed_field_conditional},
		{"curve",NULL,NULL,set_Control_curve},
		{NULL,NULL,NULL,NULL},
	};
	struct Computed_field *field,*source_field;
	struct Computed_field_package *computed_field_package;
	struct Control_curve *curve;
	struct Set_Computed_field_conditional_data set_source_field_data;

	ENTER(define_Computed_field_type_curve_lookup);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		if (COMPUTED_FIELD_CURVE_LOOKUP==Computed_field_get_type(field))
		{
			return_code=
				Computed_field_get_type_curve_lookup(field,&curve,&source_field);
		}
		else
		{
			if (!((curve=FIRST_OBJECT_IN_MANAGER_THAT(Control_curve)(
				(MANAGER_CONDITIONAL_FUNCTION(Control_curve) *)NULL,(void *)NULL,
				computed_field_package->control_curve_manager))&&
				(source_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_has_1_component,(void *)NULL,
					computed_field_package->computed_field_manager))))
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_curve_lookup.  "
					"Could not get valid source field or curve");
				return_code=0;
			}
		}
		if (return_code)
		{
			ACCESS(Computed_field)(source_field);
			ACCESS(Control_curve)(curve);
			set_source_field_data.computed_field_package=computed_field_package;
			set_source_field_data.conditional_function=Computed_field_has_1_component;
			set_source_field_data.conditional_function_user_data=(void *)NULL;
			(option_table[0]).to_be_modified= &source_field;
			(option_table[0]).user_data= &set_source_field_data;
			(option_table[1]).to_be_modified= &curve;
			(option_table[1]).user_data=
				computed_field_package->control_curve_manager;
			return_code=process_multiple_options(state,option_table)&&
				Computed_field_set_type_curve_lookup(field,curve,source_field);
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			if (curve)
			{
				DEACCESS(Control_curve)(&curve);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_curve_lookup.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_curve_lookup */

static int define_Computed_field_type_default_coordinate(
	struct Parse_state *state,void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 27 January 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_DEFAULT_COORDINATE.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field;
	struct Computed_field_package *computed_field_package;

	ENTER(define_Computed_field_type_default_coordinate);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		if (!state->current_token)
		{
			return_code=Computed_field_set_type_default_coordinate(field,
				computed_field_package->computed_field_manager);
		}
		else
		{
			if (strcmp(PARSER_HELP_STRING,state->current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
			{
				display_message(ERROR_MESSAGE,
					"Unknown option <%s>",state->current_token);
				display_parse_state_location(state);
			}
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_default_coordinate.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_default_coordinate */

static int define_Computed_field_type_dot_product(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 2 August 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_DOT_PRODUCT
(if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"fields",NULL,NULL,set_Computed_field_array},
		{NULL,NULL,NULL,NULL}
	};
	struct Computed_field *field, **source_fields;
	struct Computed_field_package *computed_field_package;
	struct Set_Computed_field_array_data set_field_array_data;
	struct Set_Computed_field_conditional_data set_field_data;

	ENTER(define_Computed_field_type_dot_product);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		source_fields=(struct Computed_field **)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, 2))
		{
			if (COMPUTED_FIELD_DOT_PRODUCT==Computed_field_get_type(field))
			{
				return_code=Computed_field_get_type_dot_product(field,
					&(source_fields[0]), &(source_fields[1]));
			}
			else
			{
				if (source_fields[0]=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL,(void *)NULL,
					computed_field_package->computed_field_manager))
				{
					source_fields[1] = source_fields[0];
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_dot_product.  No fields defined");
					return_code=0;
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_dot_product.  Not enough memory");
			return_code=0;
		}
		if (return_code)
		{
			/* have ACCESS/DEACCESS because set_Computed_field does */
			ACCESS(Computed_field)(source_fields[0]);
			ACCESS(Computed_field)(source_fields[1]);
			set_field_data.conditional_function=
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
			set_field_data.conditional_function_user_data=(void *)NULL;
			set_field_data.computed_field_package=computed_field_package;
			set_field_array_data.number_of_fields=2;
			set_field_array_data.conditional_data= &set_field_data;
			(option_table[0]).to_be_modified= source_fields;
			(option_table[0]).user_data= &set_field_array_data;
			return_code=process_multiple_options(state,option_table);
			if (return_code)
			{
				return_code = Computed_field_set_type_dot_product(field,
					source_fields[0], source_fields[1]);
			}
			DEACCESS(Computed_field)(&source_fields[0]);
			DEACCESS(Computed_field)(&source_fields[1]);
		}
		DEALLOCATE(source_fields);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_dot_product.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_dot_product */

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

	ENTER(define_Computed_field_type_rc_coordinate);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		set_field_data.conditional_function=
			(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
		set_field_data.conditional_function_user_data=(void *)NULL;
		set_field_data.computed_field_package=computed_field_package;
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
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL,(void *)NULL,
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

static int define_Computed_field_type_embedded(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 23 April 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_EMBEDDED (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"element_xi",NULL,NULL,set_FE_field},
		{"field",NULL,NULL,set_Computed_field_conditional},
		{NULL,NULL,NULL,NULL}
	};
	struct Computed_field *source_field,*field;
	struct Computed_field_package *computed_field_package;
	struct FE_field *fe_field;
	struct Set_Computed_field_conditional_data set_field_data;

	ENTER(define_Computed_field_type_embedded);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		fe_field=(struct FE_field *)NULL;
		source_field=(struct Computed_field *)NULL;
		if (COMPUTED_FIELD_EMBEDDED==Computed_field_get_type(field))
		{
			return_code=Computed_field_get_type_embedded(field,&fe_field,&source_field);
		}
		else
		{
			fe_field=FIRST_OBJECT_IN_MANAGER_THAT(FE_field)(
				FE_field_has_value_type,(void *)ELEMENT_XI_VALUE,
				computed_field_package->fe_field_manager);
			source_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL,(void *)NULL,
				computed_field_package->computed_field_manager);
		}
		if (return_code)
		{
			if (fe_field)
			{
				ACCESS(FE_field)(fe_field);
			}
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			(option_table[0]).to_be_modified= &fe_field;
			(option_table[0]).user_data= computed_field_package->fe_field_manager;
			set_field_data.conditional_function=
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
			set_field_data.conditional_function_user_data=(void *)NULL;
			set_field_data.computed_field_package=computed_field_package;
			(option_table[1]).to_be_modified= &source_field;
			(option_table[1]).user_data= &set_field_data;
			if (return_code=process_multiple_options(state,option_table))
			{
				if (fe_field&&source_field)
				{
					return_code=Computed_field_set_type_embedded(field,fe_field,source_field);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_embedded.  FE_field or source field not selected");
					return_code=0;
				}
			}
			if (fe_field)
			{
				DEACCESS(FE_field)(&fe_field);
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_embedded.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_embedded */

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
		set_source_field_data.computed_field_package=computed_field_package;
		set_source_field_data.conditional_function =
			(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
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
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL,(void *)NULL,
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

static int define_Computed_field_type_fibre_axes(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 8 February 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_FIBRE_AXES (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"coordinate",NULL,NULL,set_Computed_field_conditional},
		{"fibre",NULL,NULL,set_Computed_field_conditional},
		{NULL,NULL,NULL,NULL}
	};
	struct Computed_field *coordinate_field,*field,*fibre_field;
	struct Computed_field_package *computed_field_package;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_fibre_field_data;

	ENTER(define_Computed_field_type_fibre_axes);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		coordinate_field=(struct Computed_field *)NULL;
		fibre_field=(struct Computed_field *)NULL;
		if (COMPUTED_FIELD_FIBRE_AXES==Computed_field_get_type(field))
		{
			return_code=Computed_field_get_type_fibre_axes(field,
				&fibre_field,&coordinate_field);
		}
		else
		{
			if (!(((coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,
				name)("default_coordinate",
					computed_field_package->computed_field_manager))||
				(coordinate_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_has_1_to_3_components,(void *)NULL,
					computed_field_package->computed_field_manager)))&&
				((fibre_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,
					name)("fibres",computed_field_package->computed_field_manager))||
				(fibre_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_has_1_to_3_components,(void *)NULL,
					computed_field_package->computed_field_manager)))))
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_fibre_axes.  "
					"No valid coordinate and/or fibre field available");
				return_code=0;
			}
		}
		if (return_code)
		{
			/* have ACCESS/DEACCESS because set_Computed_field does */
			ACCESS(Computed_field)(coordinate_field);
			ACCESS(Computed_field)(fibre_field);
			/* coordinate */
			set_coordinate_field_data.computed_field_package=computed_field_package;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_1_to_3_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			(option_table[0]).to_be_modified= &coordinate_field;
			(option_table[0]).user_data= &set_coordinate_field_data;
			/* fibre */
			set_fibre_field_data.computed_field_package=computed_field_package;
			set_fibre_field_data.conditional_function=
				Computed_field_has_1_to_3_components;
			set_fibre_field_data.conditional_function_user_data=(void *)NULL;
			(option_table[1]).to_be_modified= &fibre_field;
			(option_table[1]).user_data= &set_fibre_field_data;
			return_code=process_multiple_options(state,option_table)&&
				Computed_field_set_type_fibre_axes(field,fibre_field,coordinate_field);
			DEACCESS(Computed_field)(&coordinate_field);
			DEACCESS(Computed_field)(&fibre_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_fibre_axes.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_fibre_axes */

static int define_Computed_field_type_fibre_sheet_axes(
	struct Parse_state *state,void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 8 February 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_FIBRE_SHEET_AXES (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"coordinate",NULL,NULL,set_Computed_field_conditional},
		{"fibre",NULL,NULL,set_Computed_field_conditional},
		{NULL,NULL,NULL,NULL}
	};
	struct Computed_field *coordinate_field,*field,*fibre_field;
	struct Computed_field_package *computed_field_package;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_fibre_field_data;

	ENTER(define_Computed_field_type_fibre_sheet_axes);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		coordinate_field=(struct Computed_field *)NULL;
		fibre_field=(struct Computed_field *)NULL;
		if (COMPUTED_FIELD_FIBRE_SHEET_AXES==Computed_field_get_type(field))
		{
			return_code=Computed_field_get_type_fibre_sheet_axes(field,
				&fibre_field,&coordinate_field);
		}
		else
		{
			if (!(((coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,
				name)("default_coordinate",
					computed_field_package->computed_field_manager))||
				(coordinate_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_has_1_to_3_components,(void *)NULL,
					computed_field_package->computed_field_manager)))&&
				((fibre_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,
					name)("fibres",computed_field_package->computed_field_manager))||
				(fibre_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_has_1_to_3_components,(void *)NULL,
					computed_field_package->computed_field_manager)))))
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_fibre_sheet_axes.  "
					"No valid coordinate and/or fibre field available");
				return_code=0;
			}
		}
		if (return_code)
		{
			/* have ACCESS/DEACCESS because set_Computed_field does */
			ACCESS(Computed_field)(coordinate_field);
			ACCESS(Computed_field)(fibre_field);
			/* coordinate */
			set_coordinate_field_data.computed_field_package=computed_field_package;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_1_to_3_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			(option_table[0]).to_be_modified= &coordinate_field;
			(option_table[0]).user_data= &set_coordinate_field_data;
			/* fibre */
			set_fibre_field_data.computed_field_package=computed_field_package;
			set_fibre_field_data.conditional_function=
				Computed_field_has_1_to_3_components;
			set_fibre_field_data.conditional_function_user_data=(void *)NULL;
			(option_table[1]).to_be_modified= &fibre_field;
			(option_table[1]).user_data= &set_fibre_field_data;
			return_code=process_multiple_options(state,option_table)&&
				Computed_field_set_type_fibre_sheet_axes(field,fibre_field,
					coordinate_field);
			DEACCESS(Computed_field)(&coordinate_field);
			DEACCESS(Computed_field)(&fibre_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_fibre_sheet_axes.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_fibre_sheet_axes */

static int define_Computed_field_type_finite_element(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 8 February 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_FINITE_ELEMENT (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"fe_field",NULL,NULL,set_FE_field},
		{NULL,NULL,NULL,NULL}
	};
	struct Computed_field *field;
	struct Computed_field_package *computed_field_package;
	struct FE_field *fe_field;

	ENTER(define_Computed_field_type_finite_element);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		fe_field=(struct FE_field *)NULL;
		if (COMPUTED_FIELD_FINITE_ELEMENT==Computed_field_get_type(field))
		{
			return_code=Computed_field_get_type_finite_element(field,&fe_field);
		}
		else
		{
			fe_field=FIRST_OBJECT_IN_MANAGER_THAT(FE_field)(
				(MANAGER_CONDITIONAL_FUNCTION(FE_field) *)NULL,(void *)NULL,
				computed_field_package->fe_field_manager);
		}
		if (return_code)
		{
			/* have ACCESS/DEACCESS because set_FE_field does */
			if (fe_field)
			{
				ACCESS(FE_field)(fe_field);
			}
			(option_table[0]).to_be_modified= &fe_field;
			(option_table[0]).user_data= computed_field_package->fe_field_manager;
			if (return_code=process_multiple_options(state,option_table))
			{
				if (fe_field)
				{
					return_code=Computed_field_set_type_finite_element(field,fe_field);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_finite_element.  No FE_field selected");
					return_code=0;
				}
			}
			if (fe_field)
			{
				DEACCESS(FE_field)(&fe_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_finite_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_finite_element */

static int define_Computed_field_type_gradient(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 27 January 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_GRADIENT (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"coordinate",NULL,NULL,set_Computed_field_conditional},
		{"scalar",NULL,NULL,set_Computed_field_conditional},
		{NULL,NULL,NULL,NULL}
	};
	struct Computed_field *coordinate_field,*field,*scalar_field;
	struct Computed_field_package *computed_field_package;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_scalar_field_data;

	ENTER(define_Computed_field_type_gradient);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		coordinate_field=(struct Computed_field *)NULL;
		scalar_field=(struct Computed_field *)NULL;
		if (COMPUTED_FIELD_GRADIENT==Computed_field_get_type(field))
		{
			return_code=Computed_field_get_type_gradient(field,
				&scalar_field,&coordinate_field);
		}
		else
		{
			if (!(coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
				"default_coordinate",computed_field_package->computed_field_manager)))
			{
				coordinate_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_has_1_to_3_components,(void *)NULL,
					computed_field_package->computed_field_manager);
			}
			scalar_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_has_1_component,(void *)NULL,
				computed_field_package->computed_field_manager);
		}
		if (return_code)
		{
			/* have ACCESS/DEACCESS because set_Computed_field does */
			if (coordinate_field)
			{
				ACCESS(Computed_field)(coordinate_field);
			}
			if (scalar_field)
			{
				ACCESS(Computed_field)(scalar_field);
			}
			/* coordinate */
			set_coordinate_field_data.computed_field_package=computed_field_package;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_1_to_3_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			(option_table[0]).to_be_modified= &coordinate_field;
			(option_table[0]).user_data= &set_coordinate_field_data;
			/* scalar */
			set_scalar_field_data.computed_field_package=computed_field_package;
			set_scalar_field_data.conditional_function=
				Computed_field_has_1_component;
			set_scalar_field_data.conditional_function_user_data=(void *)NULL;
			(option_table[1]).to_be_modified= &scalar_field;
			(option_table[1]).user_data= &set_scalar_field_data;
			return_code=process_multiple_options(state,option_table)&&
				Computed_field_set_type_gradient(field,scalar_field,coordinate_field);
			if (coordinate_field)
			{
				DEACCESS(Computed_field)(&coordinate_field);
			}
			if (scalar_field)
			{
				DEACCESS(Computed_field)(&scalar_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_gradient.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_gradient */

static int define_Computed_field_type_magnitude(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 10 February 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_MAGNITUDE (if it is not already)
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

	ENTER(define_Computed_field_type_magnitude);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		source_field=(struct Computed_field *)NULL;
		if (COMPUTED_FIELD_MAGNITUDE==Computed_field_get_type(field))
		{
			return_code=Computed_field_get_type_magnitude(field,
				&source_field);
		}
		else
		{
			if (!(source_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL,(void *)NULL,
				computed_field_package->computed_field_manager)))
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_magnitude.  No fields defined");
				return_code=0;
			}
		}
		if (return_code)
		{
			/* have ACCESS/DEACCESS because set_Computed_field does */
			ACCESS(Computed_field)(source_field);
			set_field_data.conditional_function=
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
			set_field_data.conditional_function_user_data=(void *)NULL;
			set_field_data.computed_field_package=computed_field_package;
			(option_table[0]).to_be_modified= &source_field;
			(option_table[0]).user_data= &set_field_data;
			return_code=process_multiple_options(state,option_table)&&
				Computed_field_set_type_magnitude(field,source_field);
			DEACCESS(Computed_field)(&source_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_magnitude.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_magnitude */

static int define_Computed_field_type_node_value(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 23 June 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_NODE_VALUE (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	char *current_token;
	enum FE_nodal_value_type fe_nodal_d_ds1,fe_nodal_d_ds2,fe_nodal_d_ds3,
		fe_nodal_d2_ds1ds2,fe_nodal_d2_ds1ds3,fe_nodal_d2_ds2ds3,
		fe_nodal_d3_ds1ds2ds3,fe_nodal_value,nodal_value_type;
	int return_code,version_number;
	static struct Modifier_entry
		fe_field_option_table[]=
		{
			{"fe_field",NULL,NULL,set_FE_field},
			{NULL,NULL,NULL,NULL}
		},
		nodal_value_type_option_table[]=
  	{
			{"value",NULL,NULL,set_enum},
			{"d/ds1",NULL,NULL,set_enum},
			{"d/ds2",NULL,NULL,set_enum},
			{"d/ds3",NULL,NULL,set_enum},
			{"d2/ds1ds2",NULL,NULL,set_enum},
			{"d2/ds1ds3",NULL,NULL,set_enum},
			{"d2/ds2ds3",NULL,NULL,set_enum},
			{"d3/ds1ds2ds3",NULL,NULL,set_enum},
			{NULL,NULL,NULL,NULL}
		},
		value_type_version_option_table[]=
		{
			{NULL,NULL,NULL,NULL},
			{"version",NULL,NULL,set_int_positive},
			{NULL,NULL,NULL,NULL}
		},
		help_option_table[]=
		{
			{"fe_field",NULL,NULL,set_FE_field},
			{NULL,NULL,NULL,NULL},
			{"version",NULL,NULL,set_int_positive},
			{NULL,NULL,NULL,NULL}
		};
	struct Computed_field *field;
	struct Computed_field_package *computed_field_package;
	struct FE_field *fe_field;

	ENTER(define_Computed_field_type_node_value);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		fe_nodal_value=FE_NODAL_VALUE;
		nodal_value_type_option_table[0].user_data= &fe_nodal_value;
		nodal_value_type_option_table[0].to_be_modified= &nodal_value_type;
		fe_nodal_d_ds1=FE_NODAL_D_DS1;
		nodal_value_type_option_table[1].user_data= &fe_nodal_d_ds1;
		nodal_value_type_option_table[1].to_be_modified= &nodal_value_type;
		fe_nodal_d_ds2=FE_NODAL_D_DS2;
		nodal_value_type_option_table[2].user_data= &fe_nodal_d_ds2;
		nodal_value_type_option_table[2].to_be_modified= &nodal_value_type;
		fe_nodal_d_ds3=FE_NODAL_D_DS3;
		nodal_value_type_option_table[3].user_data= &fe_nodal_d_ds3;
		nodal_value_type_option_table[3].to_be_modified= &nodal_value_type;
		fe_nodal_d2_ds1ds2=FE_NODAL_D2_DS1DS2;
		nodal_value_type_option_table[4].user_data= &fe_nodal_d2_ds1ds2;
		nodal_value_type_option_table[4].to_be_modified= &nodal_value_type;
		fe_nodal_d2_ds1ds3=FE_NODAL_D2_DS1DS3;
		nodal_value_type_option_table[5].user_data= &fe_nodal_d2_ds1ds3;
		nodal_value_type_option_table[5].to_be_modified= &nodal_value_type;
		fe_nodal_d2_ds2ds3=FE_NODAL_D2_DS2DS3;
		nodal_value_type_option_table[6].user_data= &fe_nodal_d2_ds2ds3;
		nodal_value_type_option_table[6].to_be_modified= &nodal_value_type;
		fe_nodal_d3_ds1ds2ds3=FE_NODAL_D3_DS1DS2DS3;
		nodal_value_type_option_table[7].user_data= &fe_nodal_d3_ds1ds2ds3;
		nodal_value_type_option_table[7].to_be_modified= &nodal_value_type;
		fe_field=(struct FE_field *)NULL;
		nodal_value_type=FE_NODAL_UNKNOWN;
		/* user enters version number starting at 1; field stores it as 0 */
		version_number=1;
		if (COMPUTED_FIELD_NODE_VALUE==Computed_field_get_type(field))
		{
			return_code=Computed_field_get_type_node_value(field,&fe_field,
				&nodal_value_type,&version_number);
			version_number++;
		}
		if (return_code)
		{
			/* have ACCESS/DEACCESS because set_FE_field does */
			if (fe_field)
			{
				ACCESS(FE_field)(fe_field);
			}
			/* try to handle help first */
			if (current_token=state->current_token)
			{
				if (!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
				{
					(help_option_table[0]).to_be_modified= &fe_field;
					(help_option_table[0]).user_data=
						computed_field_package->fe_field_manager;
					(help_option_table[1]).user_data= &nodal_value_type_option_table;
					(help_option_table[2]).to_be_modified= &version_number;
					return_code=process_multiple_options(state,help_option_table);
				}
			}
			/* parse the fe_field if the "fe_field" token is next */
			if (return_code)
			{
				if ((current_token=state->current_token)&&
					fuzzy_string_compare(current_token,"fe_field"))
				{
					(fe_field_option_table[0]).to_be_modified= &fe_field;
					(fe_field_option_table[0]).user_data=
						computed_field_package->fe_field_manager;
					if (return_code=process_option(state,fe_field_option_table))
					{
						if (!fe_field)
						{
							display_parse_state_location(state);
							display_message(ERROR_MESSAGE,"Missing or invalid fe_field");
							return_code=0;
						}
					}
				}
				else
				{
					display_parse_state_location(state);
					display_message(ERROR_MESSAGE,
						"Must specify fe_field before other options");
					return_code=0;
				}
			}
			/* parse the value_type/version number */
			if (return_code&&state->current_token)
			{
				(value_type_version_option_table[0]).user_data=
					&nodal_value_type_option_table;
				(value_type_version_option_table[1]).to_be_modified= &version_number;
				return_code=
					process_multiple_options(state,value_type_version_option_table);
			}
			if (return_code)
			{
				if (FE_NODAL_UNKNOWN != nodal_value_type)
				{
					/* user enters version number starting at 1; field stores it as 0 */
					return_code=Computed_field_set_type_node_value(field,fe_field,
						nodal_value_type,version_number-1);
				}
				else
				{
					display_parse_state_location(state);
					display_message(ERROR_MESSAGE,"Must specify a value type");
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
						"define_Computed_field_type_node_value.  Failed");
				}
			}
			if (fe_field)
			{
				DEACCESS(FE_field)(&fe_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_node_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_node_value */

static int define_Computed_field_type_node_array_value_at_time(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 28 October 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_NODE_ARRAY_VALUE_AT_TIME 
(if it is not already) and allows its contents to be modified.
==============================================================================*/
{
	char *current_token;
	enum FE_nodal_value_type fe_nodal_d_ds1,fe_nodal_d_ds2,fe_nodal_d_ds3,
		fe_nodal_d2_ds1ds2,fe_nodal_d2_ds1ds3,fe_nodal_d2_ds2ds3,
		fe_nodal_d3_ds1ds2ds3,fe_nodal_value,nodal_value_type;

	int return_code,version_number;
	static struct Modifier_entry
		fe_field_option_table[]=
		{
			{"fe_field",NULL,NULL,set_FE_field},
			{NULL,NULL,NULL,NULL}
		},
		nodal_value_type_option_table[]=
  	{
			{"value",NULL,NULL,set_enum},
			{"d/ds1",NULL,NULL,set_enum},
			{"d/ds2",NULL,NULL,set_enum},
			{"d/ds3",NULL,NULL,set_enum},
			{"d2/ds1ds2",NULL,NULL,set_enum},
			{"d2/ds1ds3",NULL,NULL,set_enum},
			{"d2/ds2ds3",NULL,NULL,set_enum},
			{"d3/ds1ds2ds3",NULL,NULL,set_enum},
			{NULL,NULL,NULL,NULL}
		},
		value_type_version_option_table[]=
		{
			{NULL,NULL,NULL,NULL},
			{"version",NULL,NULL,set_int_positive},
			{NULL,NULL,NULL,NULL}
		},
		help_option_table[]=
		{
			{"fe_field",NULL,NULL,set_FE_field},
			{NULL,NULL,NULL,NULL},
			{"version",NULL,NULL,set_int_positive},
			{NULL,NULL,NULL,NULL}
		};
	struct Computed_field *field;
	struct Computed_field_package *computed_field_package;
	struct FE_field *fe_field;

	ENTER(define_Computed_field_type_node_array_value_at_time);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		fe_nodal_value=FE_NODAL_VALUE;
		nodal_value_type_option_table[0].user_data= &fe_nodal_value;
		nodal_value_type_option_table[0].to_be_modified= &nodal_value_type;
		fe_nodal_d_ds1=FE_NODAL_D_DS1;
		nodal_value_type_option_table[1].user_data= &fe_nodal_d_ds1;
		nodal_value_type_option_table[1].to_be_modified= &nodal_value_type;
		fe_nodal_d_ds2=FE_NODAL_D_DS2;
		nodal_value_type_option_table[2].user_data= &fe_nodal_d_ds2;
		nodal_value_type_option_table[2].to_be_modified= &nodal_value_type;
		fe_nodal_d_ds3=FE_NODAL_D_DS3;
		nodal_value_type_option_table[3].user_data= &fe_nodal_d_ds3;
		nodal_value_type_option_table[3].to_be_modified= &nodal_value_type;
		fe_nodal_d2_ds1ds2=FE_NODAL_D2_DS1DS2;
		nodal_value_type_option_table[4].user_data= &fe_nodal_d2_ds1ds2;
		nodal_value_type_option_table[4].to_be_modified= &nodal_value_type;
		fe_nodal_d2_ds1ds3=FE_NODAL_D2_DS1DS3;
		nodal_value_type_option_table[5].user_data= &fe_nodal_d2_ds1ds3;
		nodal_value_type_option_table[5].to_be_modified= &nodal_value_type;
		fe_nodal_d2_ds2ds3=FE_NODAL_D2_DS2DS3;
		nodal_value_type_option_table[6].user_data= &fe_nodal_d2_ds2ds3;
		nodal_value_type_option_table[6].to_be_modified= &nodal_value_type;
		fe_nodal_d3_ds1ds2ds3=FE_NODAL_D3_DS1DS2DS3;
		nodal_value_type_option_table[7].user_data= &fe_nodal_d3_ds1ds2ds3;
		nodal_value_type_option_table[7].to_be_modified= &nodal_value_type;
		fe_field=(struct FE_field *)NULL;
		nodal_value_type=FE_NODAL_UNKNOWN;
		/* user enters version number starting at 1; field stores it as 0 */
		version_number=1;
		if (COMPUTED_FIELD_NODE_ARRAY_VALUE_AT_TIME==Computed_field_get_type(field))
		{
#if defined (OLD_CODE)
			return_code=Computed_field_get_type_node_array_value_at_time(field,&fe_field,
				&nodal_value_type,&version_number,&time);
#endif /* #if defined (OLD_CODE) */
			version_number++;
		}
		if (return_code)
		{
			/* have ACCESS/DEACCESS because set_FE_field does */
			if (fe_field)
			{
				ACCESS(FE_field)(fe_field);
			}
			/* try to handle help first */
			if (current_token=state->current_token)
			{
				if (!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
				{
					(help_option_table[0]).to_be_modified= &fe_field;
					(help_option_table[0]).user_data=
						computed_field_package->fe_field_manager;
					(help_option_table[1]).user_data= &nodal_value_type_option_table;
					(help_option_table[2]).to_be_modified= &version_number;
					return_code=process_multiple_options(state,help_option_table);
				}
			}
			/* parse the fe_field if the "fe_field" token is next */
			if (return_code)
			{
				if ((current_token=state->current_token)&&
					fuzzy_string_compare(current_token,"fe_field"))
				{
					(fe_field_option_table[0]).to_be_modified= &fe_field;
					(fe_field_option_table[0]).user_data=
						computed_field_package->fe_field_manager;
					if (return_code=process_option(state,fe_field_option_table))
					{
						if (!fe_field)
						{
							display_parse_state_location(state);
							display_message(ERROR_MESSAGE,"Missing or invalid fe_field");
							return_code=0;
						}
					}
				}
				else
				{
					display_parse_state_location(state);
					display_message(ERROR_MESSAGE,
						"Must specify fe_field before other options");
					return_code=0;
				}
			}
			/* parse the value_type/version number */
			if (return_code&&state->current_token)
			{
				(value_type_version_option_table[0]).user_data=
					&nodal_value_type_option_table;
				(value_type_version_option_table[1]).to_be_modified= &version_number;
				return_code=
					process_multiple_options(state,value_type_version_option_table);
			}
			if (return_code)
			{
				if (FE_NODAL_UNKNOWN != nodal_value_type)
				{
#if defined (OLD_CODE)
					/* user enters version number starting at 1; field stores it as 0 */
					return_code=Computed_field_set_type_node_array_value_at_time(field,fe_field,
						nodal_value_type,version_number-1,time);
#endif
				}
				else
				{
					display_parse_state_location(state);
					display_message(ERROR_MESSAGE,"Must specify a value type");
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
						"define_Computed_field_type_node_array_value_at_time.  Failed");
				}
			}
			if (fe_field)
			{
				DEACCESS(FE_field)(&fe_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_node_array_value_at_time.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_node_array_value_at_time */

int define_Computed_field_type_offset(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_OFFSET (if it is not already)
and allows its contents to be modified.
Must input the field before the offsets since there will be as many
offsets as there are components in the field.
==============================================================================*/
{
	auto struct Modifier_entry
		field_option_table[]=
		{
			{"field",NULL,NULL,set_Computed_field_conditional},
			{NULL,NULL,NULL,NULL}
		},
		offsets_option_table[]=
		{
			{"offsets",NULL,NULL,set_FE_value_array},
			{NULL,NULL,NULL,NULL}
		},
		help_option_table[]=
		{
			{"field",NULL,NULL,set_Computed_field_conditional},
			{"offsets",NULL,NULL,set_FE_value_array},
			{NULL,NULL,NULL,NULL}
		};
	char *current_token;
	FE_value *temp_offsets,*offsets;
	int i,number_of_components,return_code;
	struct Computed_field *field,*source_field;
	struct Computed_field_package *computed_field_package;
	struct Set_Computed_field_conditional_data set_field_data;

	ENTER(define_Computed_field_type_rc_coordinate);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		set_field_data.conditional_function=
			(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
		set_field_data.conditional_function_user_data=(void *)NULL;
		set_field_data.computed_field_package=computed_field_package;
		/* get source_field and offsets - from field if of type offset */
		source_field=(struct Computed_field *)NULL;
		offsets=(FE_value *)NULL;
		if (COMPUTED_FIELD_OFFSET==field->type)
		{
			return_code=Computed_field_get_type_offset(field,
				&source_field,&offsets);
		}
		else
		{
			/* get first available field, and set offsets for it to 1.0 */
			if ((source_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL,(void *)NULL,
				computed_field_package->computed_field_manager))&&
				ALLOCATE(offsets,FE_value,source_field->number_of_components))
			{
				for (i=0;i<source_field->number_of_components;i++)
				{
					offsets[i]=1.0;
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
					(help_option_table[1]).to_be_modified= offsets;
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
					/* save the number of components to maintain current offsets */
					number_of_components=source_field->number_of_components;
					(field_option_table[0]).to_be_modified= &source_field;
					(field_option_table[0]).user_data= &set_field_data;
					if (return_code=process_option(state,field_option_table))
					{
						if (source_field)
						{
							if (REALLOCATE(temp_offsets,offsets,FE_value,
								source_field->number_of_components))
							{
								offsets=temp_offsets;
								/* make any new offsets equal to 1.0 */
								for (i=number_of_components;
									i<source_field->number_of_components;i++)
								{
									offsets[i]=1.0;
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
								"define_Computed_field_type_offset.  Invalid field");
							return_code=0;
						}
					}
				}
			}
			/* parse the offsets */
			if (return_code&&state->current_token)
			{
				number_of_components=source_field->number_of_components;
				(offsets_option_table[0]).to_be_modified= offsets;
				(offsets_option_table[0]).user_data= &number_of_components;
				return_code=process_multiple_options(state,offsets_option_table);
			}
			if (return_code)
			{
				return_code=
					Computed_field_set_type_offset(field,source_field,offsets);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_offset.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			if (offsets)
			{
				DEALLOCATE(offsets);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_offset.  "
				"Could not get source_field or offsets");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_offset.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_offset */

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
					Computed_field_has_1_to_3_components,(void *)NULL,
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
			set_coordinate_field_data.computed_field_package=computed_field_package;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_1_to_3_components;
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
					Computed_field_has_1_to_3_components,(void *)NULL,
					computed_field_package->computed_field_manager)))&&
				((vector_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,
					name)("vectors",computed_field_package->computed_field_manager))||
				(vector_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_has_1_to_3_components,(void *)NULL,
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
			set_coordinate_field_data.computed_field_package=computed_field_package;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_1_to_3_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			(option_table[0]).to_be_modified= &coordinate_field;
			(option_table[0]).user_data= &set_coordinate_field_data;
			/* vector */
			set_vector_field_data.computed_field_package=computed_field_package;
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

static int define_Computed_field_type_sample_texture(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 16 March 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_TYPE_SAMPLE_TEXTURE (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"coordinates",NULL,NULL,set_Computed_field_conditional},
		{"texture",NULL,NULL,set_Texture},
		{NULL,NULL,NULL,NULL}
	};
	struct Computed_field *source_field,*field;
	struct Computed_field_package *computed_field_package;
	struct Set_Computed_field_conditional_data set_field_data;
	struct Texture *texture;

	ENTER(define_Computed_field_type_sample_texture);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		source_field=(struct Computed_field *)NULL;
		if (COMPUTED_FIELD_SAMPLE_TEXTURE==Computed_field_get_type(field))
		{
			return_code=Computed_field_get_type_sample_texture(field,
				&source_field, &texture);
		}
		else
		{
			if (!(source_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				Computed_field_has_1_to_3_components,(void *)NULL,
				computed_field_package->computed_field_manager)))
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_sample_texture.  No fields defined");
				return_code=0;
			}
		}
		if (return_code)
		{
			/* have ACCESS/DEACCESS because set_Computed_field does */
			ACCESS(Computed_field)(source_field);
			texture = (struct Texture *)NULL;
			set_field_data.conditional_function=
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
			set_field_data.conditional_function_user_data=
				(void *)Computed_field_has_1_to_3_components;
			set_field_data.computed_field_package=computed_field_package;
			(option_table[0]).to_be_modified= &source_field;
			(option_table[0]).user_data= &set_field_data;
			(option_table[1]).to_be_modified= &texture;
			(option_table[1]).user_data= computed_field_package->texture_manager;			
			return_code=process_multiple_options(state,option_table)&&
				Computed_field_set_type_sample_texture(field,source_field,texture);
			DEACCESS(Computed_field)(&source_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_sample_texture.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_sample_texture */

int define_Computed_field_type_scale(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_SCALE (if it is not already)
and allows its contents to be modified.
Must input the field before the scale_factors since there will be as many
scale_factors as there are components in the field.
==============================================================================*/
{
	auto struct Modifier_entry
		field_option_table[]=
		{
			{"field",NULL,NULL,set_Computed_field_conditional},
			{NULL,NULL,NULL,NULL}
		},
		scale_factors_option_table[]=
		{
			{"scale_factors",NULL,NULL,set_FE_value_array},
			{NULL,NULL,NULL,NULL}
		},
		help_option_table[]=
		{
			{"field",NULL,NULL,set_Computed_field_conditional},
			{"scale_factors",NULL,NULL,set_FE_value_array},
			{NULL,NULL,NULL,NULL}
		};
	char *current_token;
	FE_value *temp_scale_factors,*scale_factors;
	int i,number_of_components,return_code;
	struct Computed_field *field,*source_field;
	struct Computed_field_package *computed_field_package;
	struct Set_Computed_field_conditional_data set_field_data;

	ENTER(define_Computed_field_type_rc_coordinate);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		set_field_data.conditional_function=
			(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
		set_field_data.conditional_function_user_data=(void *)NULL;
		set_field_data.computed_field_package=computed_field_package;
		/* get source_field and scale_factors - from field if of type scale */
		source_field=(struct Computed_field *)NULL;
		scale_factors=(FE_value *)NULL;
		if (COMPUTED_FIELD_SCALE==field->type)
		{
			return_code=Computed_field_get_type_scale(field,
				&source_field,&scale_factors);
		}
		else
		{
			/* get first available field, and set scale_factors for it to 1.0 */
			if ((source_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL,(void *)NULL,
				computed_field_package->computed_field_manager))&&
				ALLOCATE(scale_factors,FE_value,source_field->number_of_components))
			{
				for (i=0;i<source_field->number_of_components;i++)
				{
					scale_factors[i]=1.0;
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
					(help_option_table[1]).to_be_modified= scale_factors;
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
					/* save the number of components to maintain current scale_factors */
					number_of_components=source_field->number_of_components;
					(field_option_table[0]).to_be_modified= &source_field;
					(field_option_table[0]).user_data= &set_field_data;
					if (return_code=process_option(state,field_option_table))
					{
						if (source_field)
						{
							if (REALLOCATE(temp_scale_factors,scale_factors,FE_value,
								source_field->number_of_components))
							{
								scale_factors=temp_scale_factors;
								/* make any new scale_factors equal to 1.0 */
								for (i=number_of_components;
									i<source_field->number_of_components;i++)
								{
									scale_factors[i]=1.0;
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
								"define_Computed_field_type_scale.  Invalid field");
							return_code=0;
						}
					}
				}
			}
			/* parse the scale_factors */
			if (return_code&&state->current_token)
			{
				number_of_components=source_field->number_of_components;
				(scale_factors_option_table[0]).to_be_modified= scale_factors;
				(scale_factors_option_table[0]).user_data= &number_of_components;
				return_code=process_multiple_options(state,scale_factors_option_table);
			}
			if (return_code)
			{
				return_code=
					Computed_field_set_type_scale(field,source_field,scale_factors);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_scale.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			if (scale_factors)
			{
				DEALLOCATE(scale_factors);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"define_Computed_field_type_scale.  "
				"Could not get source_field or scale_factors");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_scale.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_scale */

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

	ENTER(define_Computed_field_type_rc_coordinate);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		set_field_data.conditional_function=
			(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
		set_field_data.conditional_function_user_data=(void *)NULL;
		set_field_data.computed_field_package=computed_field_package;
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
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL,(void *)NULL,
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

static int define_Computed_field_type_xi_coordinates(struct Parse_state *state,
	void *field_void,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 26 January 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_XI_COORDINATES.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field;

	ENTER(define_Computed_field_type_xi_coordinates);
	USE_PARAMETER(dummy_void);
	if (state&&(field=(struct Computed_field *)field_void))
	{
		if (!state->current_token)
		{
			return_code=Computed_field_set_type_xi_coordinates(field);
		}
		else
		{
			if (strcmp(PARSER_HELP_STRING,state->current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))
			{
				display_message(ERROR_MESSAGE,
					"Unknown option <%s>",state->current_token);
				display_parse_state_location(state);
			}
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_xi_coordinates.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_xi_coordinates */

static int define_Computed_field_type_xi_texture_coordinates(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 16 March 1999

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_XI_TEXTURE_COORDINATES (if it is not already)
and allows its contents to be modified.
==============================================================================*/
{
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"seed_element",NULL,NULL,set_FE_element_top_level},
		{NULL,NULL,NULL,NULL}
	};
	struct Computed_field *field;
	struct Computed_field_package *computed_field_package;
	struct FE_element *seed_element;	

	ENTER(define_Computed_field_type_xi_texture_coordinates);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_package=
			(struct Computed_field_package *)computed_field_package_void))
	{
		return_code=1;
		if (return_code)
		{
			seed_element = (struct FE_element *)NULL;
			(option_table[0]).to_be_modified= &seed_element;
			(option_table[0]).user_data=computed_field_package->fe_element_manager;
			return_code=process_multiple_options(state,option_table)&&
				Computed_field_set_type_xi_texture_coordinates(field,
				seed_element, computed_field_package->fe_element_manager);
			if (seed_element)
			{
				DEACCESS(FE_element)(&seed_element);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_xi_texture_coordinates.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_xi_texture_coordinates */

static int define_Computed_field_type(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 9 November 1999

DESCRIPTION :
Part of the group of define_Computed_field functions. Here, we already have the
<field> to be modified and have determined the number of components and
coordinate system, and must now determine the type of computed field function
and its parameter fields and values.
#### Must ensure implemented correctly for new Computed_field_type. ####
==============================================================================*/
{
	int i,return_code;
	static struct Modifier_entry option_table[]=
	{
		{"2D_strain",NULL,NULL,define_Computed_field_type_2D_strain},
		{"add",NULL,NULL,define_Computed_field_type_add},
		{"clamp_maximum",NULL,NULL,define_Computed_field_type_clamp_maximum},
		{"clamp_minimum",NULL,NULL,define_Computed_field_type_clamp_minimum},
		{"cmiss_number",NULL,NULL,define_Computed_field_type_cmiss_number},
		{"component",NULL,NULL,define_Computed_field_type_component},
		{"compose",NULL,NULL,define_Computed_field_type_compose},
		{"composite",NULL,NULL,define_Computed_field_type_composite},
		{"constant",NULL,NULL,define_Computed_field_type_constant},
		{"cubic_texture_coordinates",NULL,NULL,
		  define_Computed_field_type_cubic_texture_coordinates},
		{"curve_lookup",NULL,NULL,define_Computed_field_type_curve_lookup},
		{"default_coordinate",NULL,NULL,
		  define_Computed_field_type_default_coordinate},
		{"dot_product",NULL,NULL,define_Computed_field_type_dot_product},
		{"edit_mask",NULL,NULL,define_Computed_field_type_edit_mask},
		{"embedded",NULL,NULL,define_Computed_field_type_embedded},
		{"external",NULL,NULL,define_Computed_field_type_external},
		{"fibre_axes",NULL,NULL,define_Computed_field_type_fibre_axes},
		{"fibre_sheet_axes",NULL,NULL,define_Computed_field_type_fibre_sheet_axes},
		{"finite_element",NULL,NULL,define_Computed_field_type_finite_element},
		{"gradient",NULL,NULL,define_Computed_field_type_gradient},
		{"magnitude",NULL,NULL,define_Computed_field_type_magnitude},
		{"node_array_value_at_time",NULL,NULL,
		 define_Computed_field_type_node_array_value_at_time},
		{"node_value",NULL,NULL,define_Computed_field_type_node_value},
		{"offset",NULL,NULL,define_Computed_field_type_offset},
		{"rc_coordinate",NULL,NULL,define_Computed_field_type_rc_coordinate},
		{"rc_vector",NULL,NULL,define_Computed_field_type_rc_vector},
		{"sample_texture",NULL,NULL,define_Computed_field_type_sample_texture},
		{"scale",NULL,NULL,define_Computed_field_type_scale},
		{"sum_components",NULL,NULL,define_Computed_field_type_sum_components},
		{"xi_coordinates",NULL,NULL,define_Computed_field_type_xi_coordinates},
		{"xi_texture_coordinates",NULL,NULL,
		  define_Computed_field_type_xi_texture_coordinates},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(define_Computed_field_type);
	if (state)
	{
		if (state->current_token)
		{
			i=0;
			/* 2D_strain */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* add */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* clamp_maximum */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* clamp_minimum */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* cmiss_number */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* component */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* composite */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* compose */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* constant */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* cubic_texture_coordinates */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* curve_lookup */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* default_coordinate */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* dot_product */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* edit_mask */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* embedded */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* external */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* fibre_axes */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* fibre_sheet_axes */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* finite_element */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* gradient */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* magnitude */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* node_array_value_at_time */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* node_value */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* offset */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* rc_coordinate */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* rc_vector */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* sample_texture */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* scale */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* sum_components */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			/* xi_coordinates */
			(option_table[i]).to_be_modified=field_void;
			i++;
			/* xi_texture_coordinates */
			(option_table[i]).to_be_modified=field_void;
			(option_table[i]).user_data=computed_field_package_void;
			i++;
			return_code=process_option(state,option_table);
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

int define_Computed_field_coordinate_system(struct Parse_state *state,
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
	struct Coordinate_system coordinate_system, *coordinate_system_ptr;
	int i, return_code;
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
					if(return_code=process_option(state,option_table)&&
						Computed_field_set_coordinate_system(field,&coordinate_system))
					{
						return_code=define_Computed_field_type(state,field_void,
							computed_field_package_void);
					}
				}
				else
				{
					/* not setting coordinate system so try and get it from the source fields */
					return_code=define_Computed_field_type(state,field_void,
						computed_field_package_void);
					switch (field->type)
					{
						case COMPUTED_FIELD_CMISS_NUMBER:
						case COMPUTED_FIELD_COMPONENT:
						case COMPUTED_FIELD_COMPOSITE:
						case COMPUTED_FIELD_CONSTANT:
						case COMPUTED_FIELD_CUBIC_TEXTURE_COORDINATES:
						case COMPUTED_FIELD_CURVE_LOOKUP:
						case COMPUTED_FIELD_DOT_PRODUCT:
						case COMPUTED_FIELD_DEFAULT_COORDINATE:
						case COMPUTED_FIELD_EXTERNAL:
						case COMPUTED_FIELD_FIBRE_AXES:
						case COMPUTED_FIELD_FIBRE_SHEET_AXES:
						case COMPUTED_FIELD_GRADIENT:
						case COMPUTED_FIELD_MAGNITUDE:
						case COMPUTED_FIELD_RC_COORDINATE:
						case COMPUTED_FIELD_RC_VECTOR:
						case COMPUTED_FIELD_SAMPLE_TEXTURE:
						case COMPUTED_FIELD_SUM_COMPONENTS:
						case COMPUTED_FIELD_XI_COORDINATES:
						case COMPUTED_FIELD_XI_TEXTURE_COORDINATES:
						case COMPUTED_FIELD_2D_STRAIN:
						{
							/* Always RC which is the default */
						} break;
						case COMPUTED_FIELD_ADD:
						case COMPUTED_FIELD_CLAMP_MAXIMUM:
						case COMPUTED_FIELD_CLAMP_MINIMUM:
						case COMPUTED_FIELD_EDIT_MASK:
						case COMPUTED_FIELD_EMBEDDED:
						case COMPUTED_FIELD_OFFSET:
						case COMPUTED_FIELD_SCALE:
						{
							/* Inherit from source fields */
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
										"define_Computed_field_coordinate_system."
										"  Source fields differ in coordinate system\n"
										"     Defaulting to coordinate system from first source field.");
								}
							}
						} break;
						case COMPUTED_FIELD_COMPOSE:
						{
							/* Inherit from third source field */
							coordinate_system_ptr = 
								Computed_field_get_coordinate_system(field->source_fields[2]);
							Computed_field_set_coordinate_system(field, coordinate_system_ptr);
						} break;
						case COMPUTED_FIELD_FINITE_ELEMENT:
						case COMPUTED_FIELD_NODE_ARRAY_VALUE_AT_TIME:
						case COMPUTED_FIELD_NODE_VALUE:
						{
							/* Inherit from FE field */
							Computed_field_set_coordinate_system(field,
								get_FE_field_coordinate_system(field->fe_field));
						} break;
						default:
						{
							display_message(WARNING_MESSAGE,
								"define_Computed_field_coordinate_system.  Unknown computed field type");
						}
					}
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
	struct Computed_field *field_to_be_defined,*field_to_be_defined_copy;
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
					if (field_to_be_defined=
						FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(current_token,
							computed_field_package->computed_field_manager))
					{
						if (field_to_be_defined_copy=CREATE(Computed_field)("copy"))
						{
							MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_field,name)(
								field_to_be_defined_copy,field_to_be_defined);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"define_Computed_field.  Could not create field copy");
							return_code=0;
						}
					}
					else
					{
						/* create a new field with the supplied name */
						if (!(field_to_be_defined_copy=
							CREATE(Computed_field)(current_token)))
						{
							display_message(ERROR_MESSAGE,
								"define_Computed_field.  Could not create copy of field");
							return_code=0;
						}
					}
					shift_Parse_state(state,1);
					ACCESS(Computed_field)(field_to_be_defined_copy);
					if (return_code&&define_Computed_field_coordinate_system(state,
						field_to_be_defined_copy,computed_field_package_void))
					{
						if (field_to_be_defined)
						{
							if (Computed_field_is_read_only(field_to_be_defined))
							{
								display_message(ERROR_MESSAGE,
									"Not allowed to modify read-only field");
								return_code=0;
							}
							else
							{
								/* copy modifications to field_to_be_defined */
								return_code=MANAGER_MODIFY_NOT_IDENTIFIER(Computed_field,name)(
									field_to_be_defined,field_to_be_defined_copy,
									computed_field_package->computed_field_manager);
							}
						}
						else
						{
							/* add the new field to the manager */
							if (!ADD_OBJECT_TO_MANAGER(Computed_field)(
								field_to_be_defined_copy,
								computed_field_package->computed_field_manager))
							{
								display_message(ERROR_MESSAGE,
									"define_Computed_field.  Unable to add field to manager");
							}
						}
					}
					DEACCESS(Computed_field)(&field_to_be_defined_copy);
				}
				else
				{
					if (field_to_be_defined_copy=CREATE(Computed_field)("dummy"))
					{
						(help_option_table[0]).to_be_modified=
							(void *)field_to_be_defined_copy;
						(help_option_table[0]).user_data=computed_field_package_void;
						return_code=process_option(state,help_option_table);
						DESTROY(Computed_field)(&field_to_be_defined_copy);
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

int equivalent_computed_fields_at_nodes(struct FE_node *node_1,
	struct FE_node *node_2)
/*******************************************************************************
LAST MODIFIED : 17 May 1999

DESCRIPTION :
Returns non-zero if the same fields are defined in the same ways at the two
nodes.
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
LAST MODIFIED : 4 November 1999

DESCRIPTION :
Writes the properties of the <field> to the command window.
#### Must ensure implemented correctly for new Computed_field_type. ####
==============================================================================*/
{
	char *component_name,*field_name,*temp_string, *texture_name, *curve_name;
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
			Computed_field_type_to_string(field->type));
		switch (field->type)
		{
			case COMPUTED_FIELD_2D_STRAIN:
			{
				display_message(INFORMATION_MESSAGE,
					"    deformed coordinate field : %s\n",
					field->source_fields[0]->name);
				display_message(INFORMATION_MESSAGE,
					"    undeformed coordinate field : %s\n",
					field->source_fields[1]->name);
				display_message(INFORMATION_MESSAGE,
					"    fibre angle field : %s\n",
					field->source_fields[2]->name);
			} break;
			case COMPUTED_FIELD_ADD:
			{
				display_message(INFORMATION_MESSAGE,
					"    field 1 : %s\n    scale factor 1 : %g\n",
					field->source_fields[0]->name,field->source_values[0]);
				display_message(INFORMATION_MESSAGE,
					"    field 2 : %s\n    scale factor 2 : %g\n",
					field->source_fields[1]->name,field->source_values[1]);
			} break;
			case COMPUTED_FIELD_CLAMP_MAXIMUM:
			{
				display_message(INFORMATION_MESSAGE,"    field : %s\n",
					field->source_fields[0]->name);
				display_message(INFORMATION_MESSAGE,"    maximums :");
				for (i=0;i<field->source_fields[0]->number_of_components;i++)
				{
					display_message(INFORMATION_MESSAGE," %g",field->source_values[i]);
				}
				display_message(INFORMATION_MESSAGE,"\n");
			} break;
			case COMPUTED_FIELD_CLAMP_MINIMUM:
			{
				display_message(INFORMATION_MESSAGE,"    field : %s\n",
					field->source_fields[0]->name);
				display_message(INFORMATION_MESSAGE,"    minimums :");
				for (i=0;i<field->source_fields[0]->number_of_components;i++)
				{
					display_message(INFORMATION_MESSAGE," %g",field->source_values[i]);
				}
				display_message(INFORMATION_MESSAGE,"\n");
			} break;
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
			case COMPUTED_FIELD_COMPOSITE:
			{
				display_message(INFORMATION_MESSAGE,"    number_of_scalars : %d\n",
					field->number_of_source_fields);
				display_message(INFORMATION_MESSAGE,"    scalars :");
				for (i=0;i<field->number_of_source_fields;i++)
				{
					display_message(INFORMATION_MESSAGE," %s",
						field->source_fields[i]->name);
				}
				display_message(INFORMATION_MESSAGE,"\n");
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
			case COMPUTED_FIELD_CURVE_LOOKUP:
			{
				if (return_code=GET_NAME(Control_curve)(field->curve,&curve_name))
				{
					display_message(INFORMATION_MESSAGE,"    curve : %s\n",
						curve_name);
					display_message(INFORMATION_MESSAGE,"    source field : %s\n",
						field->source_fields[0]->name);
					DEALLOCATE(curve_name);
				}
			} break;
			case COMPUTED_FIELD_DOT_PRODUCT:
			{
				display_message(INFORMATION_MESSAGE,
					"    field 1 : %s\n    field 2 : %s\n",
					field->source_fields[0]->name, field->source_fields[1]->name);
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
			case COMPUTED_FIELD_EMBEDDED:
			{
				if (return_code=GET_NAME(FE_field)(field->fe_field,&field_name))
				{
					display_message(INFORMATION_MESSAGE,"    element_xi fe_field: %s\n",field_name);
					DEALLOCATE(field_name);
					display_message(INFORMATION_MESSAGE,
						"    field : %s\n",field->source_fields[0]->name);
				}
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
			case COMPUTED_FIELD_FIBRE_AXES:
			case COMPUTED_FIELD_FIBRE_SHEET_AXES:
			{
				display_message(INFORMATION_MESSAGE,
					"    coordinate field : %s\n",field->source_fields[1]->name);
				display_message(INFORMATION_MESSAGE,
					"    fibre field : %s\n",field->source_fields[0]->name);
			} break;
			case COMPUTED_FIELD_FINITE_ELEMENT:
			{
				if (return_code=GET_NAME(FE_field)(field->fe_field,&field_name))
				{
					display_message(INFORMATION_MESSAGE,"    fe_field : %s\n",field_name);
					DEALLOCATE(field_name);
				}
			} break;
			case COMPUTED_FIELD_GRADIENT:
			{
				display_message(INFORMATION_MESSAGE,
					"    coordinate field : %s\n",field->source_fields[1]->name);
				display_message(INFORMATION_MESSAGE,
					"    scalar field : %s\n",field->source_fields[0]->name);
			} break;
			case COMPUTED_FIELD_MAGNITUDE:
			{
				display_message(INFORMATION_MESSAGE,
					"    field : %s\n",field->source_fields[0]->name);
			} break;
			case COMPUTED_FIELD_OFFSET:
			{
				display_message(INFORMATION_MESSAGE,"    field : %s\n",
					field->source_fields[0]->name);
				display_message(INFORMATION_MESSAGE,"    offsets :");
				for (i=0;i<field->source_fields[0]->number_of_components;i++)
				{
					display_message(INFORMATION_MESSAGE," %g",field->source_values[i]);
				}
				display_message(INFORMATION_MESSAGE,"\n");
			} break;		
			case COMPUTED_FIELD_NODE_ARRAY_VALUE_AT_TIME:
			case COMPUTED_FIELD_NODE_VALUE:
			{
				if (return_code=GET_NAME(FE_field)(field->fe_field,&field_name))
				{
					display_message(INFORMATION_MESSAGE,"    fe_field : %s\n",field_name);
					display_message(INFORMATION_MESSAGE,"    nodal value type : %s\n",
						get_FE_nodal_value_type_string(field->nodal_value_type));
					display_message(INFORMATION_MESSAGE,"    version : %d\n",
						field->version_number+1);
					DEALLOCATE(field_name);
				}
			} break;
			case COMPUTED_FIELD_RC_COORDINATE:
			{
				display_message(INFORMATION_MESSAGE,
					"    coordinate field : %s\n",field->source_fields[0]->name);
			} break;
			case COMPUTED_FIELD_RC_VECTOR:
			{
				display_message(INFORMATION_MESSAGE,
					"    coordinate field : %s\n",field->source_fields[1]->name);
				display_message(INFORMATION_MESSAGE,
					"    vector field : %s\n",field->source_fields[0]->name);
			} break;
			case COMPUTED_FIELD_SAMPLE_TEXTURE:
			{
				display_message(INFORMATION_MESSAGE,
					"    texture coordinate field : %s\n",field->source_fields[0]->name);
				if (return_code=GET_NAME(Texture)(field->texture,&texture_name))
				{
					display_message(INFORMATION_MESSAGE,
						"    texture : %s\n",texture_name);
					DEALLOCATE(texture_name);
				}
			} break;
			case COMPUTED_FIELD_SCALE:
			{
				display_message(INFORMATION_MESSAGE,"    field : %s\n",
					field->source_fields[0]->name);
				display_message(INFORMATION_MESSAGE,"    scale_factors :");
				for (i=0;i<field->source_fields[0]->number_of_components;i++)
				{
					display_message(INFORMATION_MESSAGE," %g",field->source_values[i]);
				}
				display_message(INFORMATION_MESSAGE,"\n");
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
			case COMPUTED_FIELD_CMISS_NUMBER:
			case COMPUTED_FIELD_DEFAULT_COORDINATE:
			case COMPUTED_FIELD_XI_COORDINATES:
			{
				/* no extra parameters */
			} break;
			case COMPUTED_FIELD_XI_TEXTURE_COORDINATES:
			{
				display_message(INFORMATION_MESSAGE,"    seed_element : %d\n",
					field->seed_element->cm.number);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"list_Computed_field.  Unknown field type");
				return_code=0;
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
LAST MODIFIED : 4 November 1999

DESCRIPTION :
Writes the commands needed to reproduce <field> to the command window. Note that
only fields without the read_only flag set are written out, since the others
are created automatically by the program.
#### Must ensure implemented correctly for new Computed_field_type. ####
==============================================================================*/
{
	char *command_prefix,*component_name,*field_name,*temp_string, *texture_name,
		*curve_name;
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
				Computed_field_type_to_string(field->type));
			switch (field->type)
			{
				case COMPUTED_FIELD_2D_STRAIN:
				{
					display_message(INFORMATION_MESSAGE,
						" deformed_coordinates %s undeformed_coordinates %s fibre_angle %s",
						field->source_fields[0]->name, field->source_fields[1]->name,
						field->source_fields[2]->name);
				} break;
				case COMPUTED_FIELD_ADD:
				{
					display_message(INFORMATION_MESSAGE,
						" fields %s %s scale_factors %g %g",
						field->source_fields[0]->name,field->source_fields[1]->name,
						field->source_values[0],field->source_values[1]);
				} break;
				case COMPUTED_FIELD_CLAMP_MAXIMUM:
				{
					display_message(INFORMATION_MESSAGE," field %s maximums",
						field->source_fields[0]->name);
					for (i=0;i<field->source_fields[0]->number_of_components;i++)
					{
						display_message(INFORMATION_MESSAGE," %g",field->source_values[i]);
					}
				} break;
				case COMPUTED_FIELD_CLAMP_MINIMUM:
				{
					display_message(INFORMATION_MESSAGE," field %s minimums",
						field->source_fields[0]->name);
					for (i=0;i<field->source_fields[0]->number_of_components;i++)
					{
						display_message(INFORMATION_MESSAGE," %g",field->source_values[i]);
					}
				} break;
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
				case COMPUTED_FIELD_COMPOSITE:
				{
					display_message(INFORMATION_MESSAGE," number_of_scalars %d scalars",
						field->number_of_source_fields);
					for (i=0;i<field->number_of_source_fields;i++)
					{
						display_message(INFORMATION_MESSAGE," %s",
							field->source_fields[i]->name);
					}
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
				case COMPUTED_FIELD_CURVE_LOOKUP:
				{
					if (return_code=
						GET_NAME(Control_curve)(field->curve,&curve_name))
					{
						display_message(INFORMATION_MESSAGE," curve %s source %s",
							curve_name,field->source_fields[0]->name);
						DEALLOCATE(curve_name);
					}
				} break;
				case COMPUTED_FIELD_DOT_PRODUCT:
				{
					display_message(INFORMATION_MESSAGE,
						" fields %s %s",field->source_fields[0]->name,
						field->source_fields[1]->name);
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
				case COMPUTED_FIELD_EMBEDDED:
				{
					if (return_code=GET_NAME(FE_field)(field->fe_field,&field_name))
					{
						display_message(INFORMATION_MESSAGE," element_xi %s field %s",field_name,
							field->source_fields[0]->name);
						DEALLOCATE(field_name);
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
				case COMPUTED_FIELD_FIBRE_AXES:
				case COMPUTED_FIELD_FIBRE_SHEET_AXES:
				{
					display_message(INFORMATION_MESSAGE,
						" coordinate %s fibre %s",field->source_fields[1]->name,
						field->source_fields[0]->name);
				} break;
				case COMPUTED_FIELD_FINITE_ELEMENT:
				{
					if (return_code=GET_NAME(FE_field)(field->fe_field,&field_name))
					{
						display_message(INFORMATION_MESSAGE," fe_field %s",field_name);
						DEALLOCATE(field_name);
					}
				} break;
				case COMPUTED_FIELD_GRADIENT:
				{
					display_message(INFORMATION_MESSAGE,
						" coordinate %s scalar %s",field->source_fields[1]->name,
						field->source_fields[0]->name);
				} break;
				case COMPUTED_FIELD_MAGNITUDE:
				{
					display_message(INFORMATION_MESSAGE,
						" field %s",field->source_fields[0]->name);
				} break;
				case COMPUTED_FIELD_OFFSET:
				{
					display_message(INFORMATION_MESSAGE," field %s offsets",
						field->source_fields[0]->name);
					for (i=0;i<field->source_fields[0]->number_of_components;i++)
					{
						display_message(INFORMATION_MESSAGE," %g",field->source_values[i]);
					}
				} break;
				case COMPUTED_FIELD_NODE_VALUE:
				{
					if (return_code=GET_NAME(FE_field)(field->fe_field,&field_name))
					{
						display_message(INFORMATION_MESSAGE," fe_field %s %s version %d",
							field_name,
							get_FE_nodal_value_type_string(field->nodal_value_type),
							field->version_number+1);
						DEALLOCATE(field_name);
					}
				} break;
				case COMPUTED_FIELD_RC_COORDINATE:
				{
					display_message(INFORMATION_MESSAGE,
						" coordinate %s",field->source_fields[0]->name);
				} break;
				case COMPUTED_FIELD_RC_VECTOR:
				{
					display_message(INFORMATION_MESSAGE,
						" coordinate %s vector %s",field->source_fields[1]->name,
						field->source_fields[0]->name);
				} break;
				case COMPUTED_FIELD_SAMPLE_TEXTURE:
				{
					display_message(INFORMATION_MESSAGE,
						" coordinates %s",field->source_fields[0]->name);
					if (return_code=GET_NAME(Texture)(field->texture,&texture_name))
					{
						display_message(INFORMATION_MESSAGE,
							" texture %s",texture_name);
						DEALLOCATE(texture_name);
					}
				} break;
				case COMPUTED_FIELD_SCALE:
				{
					display_message(INFORMATION_MESSAGE," field %s scale_factors",
						field->source_fields[0]->name);
					for (i=0;i<field->source_fields[0]->number_of_components;i++)
					{
						display_message(INFORMATION_MESSAGE," %g",field->source_values[i]);
					}
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
				case COMPUTED_FIELD_CMISS_NUMBER:
				case COMPUTED_FIELD_DEFAULT_COORDINATE:
				case COMPUTED_FIELD_XI_COORDINATES:
				{
					/* no extra parameters */
				} break;
				case COMPUTED_FIELD_XI_TEXTURE_COORDINATES:
				{
					display_message(INFORMATION_MESSAGE," seed_element %d",
						field->seed_element->cm.number);
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"list_Computed_field.  Unknown field type");
					return_code=0;
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
			Computed_field_type_to_string(field->type));
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

static int Computed_field_contents_match(struct Computed_field *field,
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
			&&(field->fe_field==other_computed_field->fe_field)
			&&(field->component_no==other_computed_field->component_no)
			&&(field->computed_field_manager==
				other_computed_field->computed_field_manager)
			&&(field->seed_element==other_computed_field->seed_element)
			&&(field->texture==other_computed_field->texture)
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

static int Computed_field_update_fe_field_in_manager(
	struct Computed_field *computed_field, struct FE_field *fe_field,
	struct MANAGER(Computed_field) *computed_field_manager)
/*******************************************************************************
LAST MODIFIED : 29 April 1999

DESCRIPTION :
Searches the Computed_field_manager for a Computed_field which has the exact
same contents.  If there is then the object is "MODIFIED" so that the name is
updated and MANAGER messages are passed to any clients. (i.e. the fe_field may
have changed).  If there isn't a matching field then the new one is added to
the manager.
==============================================================================*/
{
	int return_code;
	struct Computed_field *field;

	ENTER(Computed_field_update_fe_field_in_manager);
	if (computed_field&&fe_field&&computed_field_manager)
	{
		/* find any existing wrapper for fe_field */
		if (field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
			Computed_field_contents_match,(void *)computed_field,
			computed_field_manager))
		{
			/* has the field name changed? */
			if (strcmp(field->name,computed_field->name))
			{
				return_code=MANAGER_MODIFY(Computed_field)(
					field,computed_field,computed_field_manager);
			}
			else
			{
				return_code=MANAGER_MODIFY_NOT_IDENTIFIER(Computed_field,name)(
					field,computed_field,computed_field_manager);
			}
			DESTROY(Computed_field)(&computed_field);
		}
		else
		{
			if (!(return_code=ADD_OBJECT_TO_MANAGER(Computed_field)(
				computed_field,computed_field_manager)))
			{
				DESTROY(Computed_field)(&computed_field);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_update_fe_field_in_manager.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_update_fe_field_in_manager */

static int FE_field_update_wrapper_Computed_field(struct FE_field *fe_field,
	void *computed_field_manager_void)
/*******************************************************************************
LAST MODIFIED : 3 February 1999

DESCRIPTION :
FE_field iterator function that performs the following:
1. Tries to find a wrapper Computed_field for <fe_field> in the
<computed_field_manager>.
2. If one is found, the contents are modified to match the changed values in
the fe_field, and if the name of the fields are the same, only a
MANAGER_MODIFY_NOT_IDENTIFIER is performed.
3. If no wrapper is found, a new one is created and added to the manager.
==============================================================================*/
{
	char *extra_field_name, *field_name;
	enum Value_type value_type;
	int return_code;
	struct Computed_field *default_coordinate_field,*temp_field;
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(FE_field_update_wrapper_Computed_field);
	if (fe_field&&(computed_field_manager=
		(struct MANAGER(Computed_field) *)computed_field_manager_void))
	{
		if (GET_NAME(FE_field)(fe_field,&field_name))
		{
			/* establish up-to-date wrapper for the fe_field */
			if ((temp_field=CREATE(Computed_field)(field_name))&&
				Computed_field_set_coordinate_system(temp_field,
				get_FE_field_coordinate_system(fe_field))&&
				Computed_field_set_type_finite_element(temp_field,fe_field)&&
				Computed_field_set_read_only(temp_field))
			{
				return_code = Computed_field_update_fe_field_in_manager(temp_field,
					fe_field, computed_field_manager);
			}
			else
			{
				return_code=0;
			}

			/* For the ELEMENT_XI_VALUE also make a default embedded coordinate field */
			value_type = get_FE_field_value_type(fe_field);
			switch(value_type)
			{
				case ELEMENT_XI_VALUE:
				{
					if(ALLOCATE(extra_field_name, char, strlen(field_name) + 20)&&
						sprintf(extra_field_name, "%s_coordinate", field_name))
					{
						/* establish up-to-date wrapper for the fe_field */
						if (temp_field=CREATE(Computed_field)(extra_field_name))
						{
							if (!(default_coordinate_field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
								"default_coordinate",computed_field_manager)))
							{
								default_coordinate_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
									Computed_field_has_1_to_3_components,(void *)NULL,
									computed_field_manager);
							}
							if(default_coordinate_field)
							{
								if(Computed_field_set_coordinate_system(temp_field,
									Computed_field_get_coordinate_system(default_coordinate_field))&&
									Computed_field_set_type_embedded(temp_field,fe_field,
							  		default_coordinate_field)&&Computed_field_set_read_only(temp_field))
								{
									return_code = Computed_field_update_fe_field_in_manager(temp_field,
										fe_field, computed_field_manager);
								}
								else
								{
									DESTROY(Computed_field)(&temp_field);
									return_code=0;
								}
							}
							else
							{
								DESTROY(Computed_field)(&temp_field);
								return_code=0;
							}
						}
						else
						{
							return_code=0;
						}
						DEALLOCATE(extra_field_name);
					}
					else
					{
						return_code=0;
					}
				} break;
			}
			DEALLOCATE(field_name);
		}
		else
		{
			return_code=0;
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"FE_field_update_wrapper_Computed_field.  Unable to update wrapper");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_field_update_wrapper_Computed_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_field_update_wrapper_Computed_field */

static void Computed_field_FE_field_change(
	struct MANAGER_MESSAGE(FE_field) *message,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 3 February 1999

DESCRIPTION :
Something has changed globally in the FE_field manager. This function ensures
that there is an appropriate (same name/#components/coordinate system)
Computed_field wrapper for each FE_field in the manager.
==============================================================================*/
{
	char *field_name;
	int return_code;
	struct Computed_field *field;
	struct Computed_field_package *computed_field_package;
	struct FE_field *fe_field;

	ENTER(Computed_field_FE_field_change);
	if (message&&(computed_field_package=
		(struct Computed_field_package *)computed_field_package_void))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_ALL(FE_field):
			{
				/* establish/update all FE_field wrappers */
				/*???RC Note: does not handle deletion of fe_fields during manager
					change all. Should not happen really */
				MANAGER_BEGIN_CACHE(Computed_field)(
					computed_field_package->computed_field_manager);
				return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_field)(
					FE_field_update_wrapper_Computed_field,
					(void *)computed_field_package->computed_field_manager,
					computed_field_package->fe_field_manager);
				MANAGER_END_CACHE(Computed_field)(
					computed_field_package->computed_field_manager);
			} break;
			case MANAGER_CHANGE_ADD(FE_field):
			case MANAGER_CHANGE_OBJECT(FE_field):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(FE_field):
			{
				return_code=
					FE_field_update_wrapper_Computed_field(message->object_changed,
						(void *)computed_field_package->computed_field_manager);
			} break;
			case MANAGER_CHANGE_DELETE(FE_field):
			{
				/* do nothing; this can never happen as must destroy Computed field before FE_field */
#if defined (OLD_CODE)
				fe_field=message->object_changed;
				if ((field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_is_read_only_with_fe_field,(void *)fe_field,
					computed_field_package->computed_field_manager))&&
					GET_NAME(FE_field)(fe_field,&field_name))
				{
					if (Computed_field_is_in_use(field))
					{
						display_message(ERROR_MESSAGE,"Computed_field_FE_field_change."
							"  FE_field %s destroyed while Computed_field %s is in use",
							field_name,field->name);
						return_code=0;
					}
					else
					{
						return_code=REMOVE_OBJECT_FROM_MANAGER(Computed_field)(field,
							computed_field_package->computed_field_manager);
					}
					DEALLOCATE(field_name);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_FE_field_change.  DELETE: Invalid field(s)");
					return_code=0;
				}
#endif /* defined (OLD_CODE) */
			} break;
			case MANAGER_CHANGE_IDENTIFIER(FE_field):
			{
				fe_field=message->object_changed;
				if ((field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_is_read_only_with_fe_field,(void *)fe_field,
					computed_field_package->computed_field_manager))&&
					GET_NAME(FE_field)(fe_field,&field_name))
				{
					return_code=MANAGER_MODIFY_IDENTIFIER(Computed_field,name)(
						field,field_name,computed_field_package->computed_field_manager);
					DEALLOCATE(field_name);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_FE_field_change.  IDENTIFIER: Invalid field(s)");
					return_code=0;
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_FE_field_change.  Unknown manager message");
				return_code=0;
			} break;
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_FE_field_change.  Unable to process changes");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_FE_field_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Computed_field_FE_field_change */

static int Computed_field_uses_Control_curve(struct Computed_field *field,
	void *curve_void)
/*******************************************************************************
LAST MODIFIED : 5 November 1999

DESCRIPTION :
Returns true if the <field> is of type COMPUTED_FIELD_CURVE_LOOKUP. If
<curve> is not NULL further checks it has a reference to <curve>.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_uses_Control_curve);
	return_code=0;
	if (field)
	{
		if (COMPUTED_FIELD_CURVE_LOOKUP==field->type)
		{
			return_code=((!curve_void)||
				((struct Control_curve *)curve_void == field->curve));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_uses_Control_curve.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Computed_field_uses_Control_curve */

struct Other_Computed_field_uses_Control_curve_data
{
	struct Computed_field *field;
	struct Control_curve *curve;
};

static int other_Computed_field_uses_Control_curve(struct Computed_field *field,
	void *curve_data_void)
/*******************************************************************************
LAST MODIFIED : 5 November 1999

DESCRIPTION :
Returns true if the <field> is of type COMPUTED_FIELD_CURVE_LOOKUP and is not
the same as the field in the <curve_data>. If <curve> is not NULL further
checks it has a reference to <curve>.
==============================================================================*/
{
	int return_code;
	struct Other_Computed_field_uses_Control_curve_data *curve_data;

	ENTER(Computed_field_uses_Control_curve);
	return_code=0;
	if (field&&(curve_data=
		(struct Other_Computed_field_uses_Control_curve_data *)curve_data_void))
	{
		if ((COMPUTED_FIELD_CURVE_LOOKUP==field->type)&&
			(field != curve_data->field))
		{
			return_code=((!curve_data->curve)||
				(curve_data->curve == field->curve));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_uses_Control_curve.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Computed_field_uses_Control_curve */

static void Computed_field_Control_curve_change(
	struct MANAGER_MESSAGE(Control_curve) *message,
	void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 5 November 1999

DESCRIPTION :
Something has changed globally in the Control_curve manager. Passes on messages
about changes as stemming from computed_field_manager for fields of type
COMPUTED_FIELD_CURVE_LOOKUP.
==============================================================================*/
{
	struct Computed_field *field;
	struct Computed_field_package *computed_field_package;
	struct MANAGER_MESSAGE(Computed_field) *computed_field_message;
	struct Other_Computed_field_uses_Control_curve_data curve_data;

	ENTER(Computed_field_Control_curve_change);
	if (message&&(computed_field_package=
		(struct Computed_field_package *)computed_field_package_void))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_ALL(Control_curve):
			case MANAGER_CHANGE_OBJECT(Control_curve):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Control_curve):
			{
				/* get first field, if any that references changed curve/s */
				if (field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_uses_Control_curve,(void *)(message->object_changed),
					computed_field_package->computed_field_manager))
				{
					/* use internal MANAGER messaging calls to send messages */
					if (ALLOCATE(computed_field_message,
						struct MANAGER_MESSAGE(Computed_field),1))
					{
						curve_data.field=field;
						curve_data.curve=message->object_changed;
						if (FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
							other_Computed_field_uses_Control_curve,(void *)&curve_data,
							computed_field_package->computed_field_manager))
						{
							/* send change all message */
							computed_field_message->change=MANAGER_CHANGE_ALL(Computed_field);
							computed_field_message->object_changed=
								(struct Computed_field *)NULL;
						}
						else
						{
							/* send change object not identifier message */
							computed_field_message->change=
								MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Computed_field);
							computed_field_message->object_changed=field;
						}
						/* send the message */
						MANAGER_UPDATE(Computed_field)(computed_field_message,
							computed_field_package->computed_field_manager);
						DEALLOCATE(computed_field_message);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_Control_curve_change.  "
							"Could not allocate manager message");
					}
				}
			} break;
			case MANAGER_CHANGE_ADD(Control_curve):
			case MANAGER_CHANGE_DELETE(Control_curve):
			case MANAGER_CHANGE_IDENTIFIER(Control_curve):
			{
				/* do nothing */
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_Control_curve_change.  Unknown manager message");
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_Control_curve_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Computed_field_Control_curve_change */

struct Computed_field_package *CREATE(Computed_field_package)(
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_element) *fe_element_manager,
	struct MANAGER(Texture) *texture_manager,
	struct MANAGER(Control_curve) *control_curve_manager)
/*******************************************************************************
LAST MODIFIED : 5 November 1999

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
	if (fe_field_manager&&fe_element_manager&&texture_manager&&
		control_curve_manager)
	{
		if (ALLOCATE(computed_field_package,struct Computed_field_package,1)&&
			(computed_field_package->computed_field_manager=
				CREATE(MANAGER(Computed_field))()))
		{
			computed_field_package->fe_field_manager=fe_field_manager;
			computed_field_package->fe_element_manager=fe_element_manager;
			computed_field_package->texture_manager=texture_manager;
			computed_field_package->control_curve_manager=control_curve_manager;
			computed_field_package->control_curve_manager_callback_id=
				MANAGER_REGISTER(Control_curve)(Computed_field_Control_curve_change,
					(void *)computed_field_package,control_curve_manager);
			computed_field_package->fe_field_manager_callback_id=
				MANAGER_REGISTER(FE_field)(Computed_field_FE_field_change,
					(void *)computed_field_package,fe_field_manager);
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

int DESTROY(computed_field_package)(
	struct Computed_field_package **package_address)
/*******************************************************************************
LAST MODIFIED : 5 November 1999

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
		MANAGER_DEREGISTER(FE_field)(
			computed_field_package->fe_field_manager_callback_id,
			computed_field_package->fe_field_manager);
		MANAGER_DEREGISTER(Control_curve)(
			computed_field_package->control_curve_manager_callback_id,
			computed_field_package->control_curve_manager);
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

int Computed_field_can_be_destroyed(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 19 August 1999

DESCRIPTION :
Returns true if the <field> is only accessed once (assumed to be by the manager).
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_can_be_destroyed);
	if (field)
	{
		return_code=(1==field->access_count);
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

int remove_computed_field_from_manager_given_FE_field(
	struct MANAGER(Computed_field) *computed_field_manager,struct FE_field *field)
/*******************************************************************************
LAST MODIFIED : August 27 1999

DESCRIPTION :
Frees the computed fields from the computed field manager, given the FE_field
==============================================================================*/
{
	int return_code;
	struct Computed_field *computed_field;

	ENTER(remove_computed_field_from_manager_given_FE_field);
	if(field&&computed_field_manager)
	{
		return_code=1;
		computed_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
			Computed_field_is_read_only_with_fe_field,
			(void *)(field),
			computed_field_manager);
		if(computed_field)
		{
			if (Computed_field_can_be_destroyed(computed_field))
			{
				REMOVE_OBJECT_FROM_MANAGER(Computed_field)(computed_field,
					computed_field_manager);			
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"remove_computed_field_from_manager_given_FE_field."
			" invalid arguments");
		return_code=0;
	}
	LEAVE;
	return(return_code);
}/* remove_computed_field_from_manager_given_FE_field */

