/*******************************************************************************
FILE : computed_field.cpp

LAST MODIFIED : 16 April 2008

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

- Should handle coordinate system differently. For the majority of field types
it can simply be assumed, eg. from source fields, or RC/irrelevant if scalar.
For those that do allow it to be specified, should make it a leaf option with
the gfx define field commands. Also should not allow coordinate system to be
changed if field is in use, since field may be chosen on the basis of this,
like the number of components.

==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "zinc/status.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_find_xi.h"
#include "computed_field/computed_field_composite.h"
#include "computed_field/computed_field_private.hpp"
#include "general/indexed_list_stl_private.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/differential_operator.hpp"
#include "computed_field/field_cache.hpp"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_discretization.h"
#include "general/any_object_definition.h"
#include "general/compare.h"
#include "general/debug.h"
#include "general/geometry.h"
#include "general/indexed_list_private.h"
#include "general/list_private.h"
#include "general/manager.h"
#include "general/manager_private.h"
#include "general/matrix_vector.h"
#include "general/mystring.h"
#include "general/value.h"
#include "region/cmiss_region_private.h"
#include "general/message.h"
#include "general/enumerator_conversion.hpp"
#include <typeinfo>

FULL_DECLARE_MANAGER_TYPE_WITH_OWNER(Computed_field, struct cmzn_region, struct cmzn_field_change_detail *);

/*
Module functions
----------------
*/

/** override to set change status of fields which depend on changed fields */
static inline void MANAGER_UPDATE_DEPENDENCIES(Computed_field)(
	struct MANAGER(Computed_field) *manager)
{
	cmzn_set_cmzn_field *all_fields = reinterpret_cast<cmzn_set_cmzn_field *>(manager->object_list);
	for (cmzn_set_cmzn_field::iterator iter = all_fields->begin(); iter != all_fields->end(); iter++)
	{
		cmzn_field_id field = *iter;
		field->core->check_dependency();
	}
}

static inline struct cmzn_field_change_detail *MANAGER_EXTRACT_CHANGE_DETAIL(Computed_field)(
	struct Computed_field *field)
{
	return field->core->extract_change_detail();
}

static inline void MANAGER_CLEANUP_CHANGE_DETAIL(Computed_field)(
	cmzn_field_change_detail **change_detail_address)
{
	delete *change_detail_address;
}

DECLARE_MANAGER_UPDATE_FUNCTION(Computed_field)

DECLARE_MANAGER_FIND_CLIENT_FUNCTION(Computed_field)

DECLARE_MANAGED_OBJECT_NOT_IN_USE_CONDITIONAL_FUNCTION(Computed_field)

static int Computed_field_clear_type(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Used internally by DESTROY and Computed_field_set_type_*() functions to
deallocate or deaccess data specific to any Computed_field_type. Functions
changing the type of the Computed_field should allocate any dynamic data needed
for the type, call this function to clear what is currently in the field and
then set values - that way the field will never be left in an invalid state.
==============================================================================*/
{
	int i,return_code = 0;

	ENTER(Computed_field_clear_type);
	if (field)
	{
		if (field->component_names)
		{
			for (i = 0 ; i < field->number_of_components ; i++)
			{
				DEALLOCATE(field->component_names[i]);
			}
			DEALLOCATE(field->component_names);
		}

		delete field->core;

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
		return_code = 1;
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

int Computed_field_set_coordinate_system_from_sources(
	struct Computed_field *field)
{
	int return_code;
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

static struct Computed_field *CREATE(Computed_field)(const char *name)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Creates a basic Computed_field with the given <name>. Its type is initially
COMPUTED_FIELD_INVALID with no components.
==============================================================================*/
{
	struct Computed_field *field;

	ENTER(CREATE(Computed_field));
	if (name)
	{
		if (ALLOCATE(field, struct Computed_field, 1) &&
			(field->name = duplicate_string(name)))
		{
			field->cache_index = 0;
			/* By default the name and the command_string are the same */
			field->command_string = (char *)field->name;
			/* initialise all members of computed_field */
			field->number_of_components = 0;
			field->coordinate_system.type = RECTANGULAR_CARTESIAN;
			field->coordinate_system.parameters.focus = 1.0;
			field->component_names = (char **)NULL;

			field->core = (Computed_field_core*)NULL;

			/* for all types of Computed_field calculated from others */
			field->source_fields = (struct Computed_field **)NULL;
			field->number_of_source_fields = 0;
			/* for all Computed_fields which use real source values */
			field->source_values = (FE_value *)NULL;
			field->number_of_source_values = 0;

			field->access_count = 0;

			field->manager = (struct MANAGER(Computed_field) *)NULL;
			field->manager_change_status = MANAGER_CHANGE_NONE(Computed_field);

			field->attribute_flags = 0;
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
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Frees memory/deaccess objects in computed_field at <*field_address>.
==============================================================================*/
{
	 char **component_name;
	 int  i, return_code;
	struct Computed_field *field;
	ENTER(DESTROY(Computed_field));
	if (field_address&&(field= *field_address))
	{
		if (0 >= field->access_count)
		{
			/* Only DEALLOCATE the command_string if it is different to the name */
			if (field->command_string && (field->command_string != field->name))
			{
				DEALLOCATE(field->command_string);
			}
			DEALLOCATE(field->name);
			if (NULL != (component_name=field->component_names))
			{
				for (i=field->number_of_components;i>0;i--)
				{
					DEALLOCATE(*component_name);
					component_name++;
				}
				DEALLOCATE(field->component_names);
			}
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

DECLARE_ACCESS_OBJECT_FUNCTION(Computed_field)

PROTOTYPE_DEACCESS_OBJECT_FUNCTION(Computed_field)
{
	int return_code;
	struct Computed_field *object;

	ENTER(DEACCESS(Computed_field));
	if (object_address && (object = *object_address))
	{
		(object->access_count)--;
		if (object->access_count <= 0)
		{
			return_code = DESTROY(Computed_field)(object_address);
		}
		else if ((0 == (object->attribute_flags & COMPUTED_FIELD_ATTRIBUTE_IS_MANAGED_BIT)) &&
			(object->manager) && ((1 == object->access_count) ||
				((2 == object->access_count) &&
					(MANAGER_CHANGE_NONE(Computed_field) != object->manager_change_status))) &&
			object->core->not_in_use())
		{
			return_code =
				REMOVE_OBJECT_FROM_MANAGER(Computed_field)(object, object->manager);
		}
		else
		{
			return_code = 1;
		}
		*object_address = (struct Computed_field *)NULL;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DEACCESS(Computed_field) */

PROTOTYPE_REACCESS_OBJECT_FUNCTION(Computed_field)
{
	int return_code;

	ENTER(REACCESS(Computed_field));
	if (object_address)
	{
		return_code = 1;
		if (new_object)
		{
			/* access the new object */
			(new_object->access_count)++;
		}
		if (*object_address)
		{
			/* deaccess the current object */
			DEACCESS(Computed_field)(object_address);
		}
		/* point to the new object */
		*object_address = new_object;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"REACCESS(Computed_field).  Invalid argument");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* REACCESS(Computed_field) */

cmzn_field_id cmzn_field_access(cmzn_field_id field)
{
	cmzn_field_id accessed_field = 0;
	if (field)
	{
		accessed_field = (ACCESS(Computed_field)(field));
	}

	return accessed_field;
}

int cmzn_field_destroy(cmzn_field_id *field_address)
{
	return (DEACCESS(Computed_field)(field_address));
}

cmzn_field_module_id cmzn_field_get_field_module(cmzn_field_id field)
{
	struct cmzn_region *region = Computed_field_get_region(field);
	return cmzn_region_get_field_module(region);
}

enum cmzn_field_value_type cmzn_field_get_value_type(cmzn_field_id field)
{
	cmzn_field_value_type value_type = CMZN_FIELD_VALUE_TYPE_INVALID;
	if (field && field->core)
	{
		value_type = field->core->get_value_type();
	}
	return value_type;
}

#if defined (DEBUG_CODE)
int cmzn_field_get_access_count(cmzn_field_id field)
{
	return field->access_count;
}
#endif /* defined (DEBUG_CODE) */

PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Computed_field)
/*****************************************************************************
LAST MODIFIED : 6 September 2007

DESCRIPTION :
Forms a string out of the objects identifier.
If the name of the computed field is "constants" then this is a special field
and the values are listed in an array.  See set_Computed_field_conditional
in computed_field_set.cpp.
============================================================================*/
{
	int return_code;

	ENTER(GET_NAME(Computed_field));
	if (object&&name_ptr)
	{
		if (ALLOCATE(*name_ptr,char,strlen(object->command_string)+1))
		{
			strcpy(*name_ptr,object->command_string);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"GET_NAME(Computed_field).  Could not allocate space for name");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GET_NAME(Computed_field).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GET_NAME(Computed_field) */

DECLARE_INDEXED_LIST_STL_FUNCTIONS(Computed_field)

DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_STL_FUNCTION(Computed_field,name,const char *)

int cmzn_field_iterator_destroy(cmzn_field_iterator_id *field_iterator_address)
{
	if (!field_iterator_address)
		return 0;
	delete *field_iterator_address;
	*field_iterator_address = 0;
	return 1;
}

cmzn_field_id cmzn_field_iterator_next(cmzn_field_iterator_id field_iterator)
{
	if (field_iterator)
		return field_iterator->next();
	return 0;
}

cmzn_field_id cmzn_field_iterator_next_non_access(cmzn_field_iterator_id field_iterator)
{
	if (field_iterator)
		return field_iterator->next_non_access();
	return 0;
}

/***************************************************************************//**
 * Copy the type specific parts of <source> field to <destination>, namely the
 * number_of_components, the source_fields, the soure_values and the core.
 * For safety, <destination> must be unmanaged or its contents must have been
 * copied to a temporary field while copying, otherwise clearing the type of
 * <destination> can cause objects to be cleaned up such as volatile source
 * fields.
 *
 * @destination  Field being modified to have a copy of type-specific data.
 * @source  Field providing the type-specific data.
 * @return  1 on success, 0 on failure.
 */
int Computed_field_copy_type_specific(
	struct Computed_field *destination, struct Computed_field *source)
{
	int return_code;

	ENTER(Computed_field_copy_type_specific);
	if (source && destination)
	{
		return_code = 1;
		char **component_names = (char **)NULL;
		Computed_field **source_fields = (struct Computed_field **)NULL;
		FE_value *source_values = (FE_value *)NULL;
		if (source->component_names)
		{
			if (ALLOCATE(component_names, char *, source->number_of_components))
			{
				for (int i = 0 ; i < source->number_of_components; i++)
				{
					if (!(component_names[i] = duplicate_string(source->component_names[i])))
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
		if (source->number_of_source_fields > 0)
		{
			if (!ALLOCATE(source_fields, struct Computed_field *, source->number_of_source_fields))
			{
				return_code = 0;
			}
		}
		if (source->number_of_source_values > 0)
		{
			if (!ALLOCATE(source_values, FE_value, source->number_of_source_values))
			{
				return_code = 0;
			}
		}
		if (return_code)
		{
			Computed_field_clear_type(destination);

			destination->number_of_components=source->number_of_components;
			destination->component_names = component_names;
			destination->number_of_source_fields = source->number_of_source_fields;
			for (int i = 0; i < source->number_of_source_fields; i++)
			{
				source_fields[i] = ACCESS(Computed_field)(source->source_fields[i]);
			}
			destination->source_fields = source_fields;
			destination->number_of_source_values = source->number_of_source_values;
			for (int i = 0; i < source->number_of_source_values; i++)
			{
				source_values[i] = source->source_values[i];
			}
			destination->source_values = source_values;

			if (source->core)
			{
				destination->core = source->core->copy();
				if ((NULL == destination->core) ||
					(!destination->core->attach_to_field(destination)))
				{
					return_code = 0;
				}
			}

			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_copy_type_specific.  Unable to copy Computed_field_core.");
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Computed_field_copy_type_specific.  Not enough memory");
			if (component_names)
			{
				for (int i = 0 ; i < source->number_of_components ; i++)
				{
					if (component_names[i])
					{
						DEALLOCATE(component_names[i]);
					}
				}
				DEALLOCATE(component_names);
			}
			if (source_fields)
			{
				DEALLOCATE(source_fields);
			}
			if (source_values)
			{
				DEALLOCATE(source_values);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "Computed_field_copy_type_specific.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_copy_type_specific */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(Computed_field,name)
/*******************************************************************************
LAST MODIFIED : 31 March 2009

DESCRIPTION :
Do not allow copy if:
- it creates a self-referencing field (one that depends on itself) which will
  result in an infinite loop;
- it changes the number of components of a field in use;
- it would make field depend on fields from another region
==============================================================================*/
{
	int return_code;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_field,name));
	if (source && destination && (source != destination))
	{
		return_code = 1;
		/* check <source> does not depend on <destination> else infinite loop */
		if (source->dependsOnField(destination))
		{
			display_message(ERROR_MESSAGE,
				"MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_field,name).  "
				"Cannot make field depend on itself");
			return_code=0;
		}
		if ((destination->manager) && (source->manager) &&
			(destination->manager != source->manager))
		{
			display_message(ERROR_MESSAGE,
				"MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_field,name).  "
				"Cannot modify definition to depend on field from another region");
			return_code = 0;
		}
		if (return_code)
		{
			COPY(Coordinate_system)(&destination->coordinate_system,
				&source->coordinate_system);
			if (!Computed_field_copy_type_specific(destination, source))
			{
				display_message(ERROR_MESSAGE,
					"MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_field,name).  Not enough memory");
				return_code=0;
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

DECLARE_MANAGER_FUNCTIONS(Computed_field, manager)

PROTOTYPE_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Computed_field)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Computed_field requires a special version of this function mainly due to the
finite_element type which automatically wraps FE_fields. If the computed field
is not itself in use, it calls the field's optional computed_field_not_in_use
function and bases its result on that.
Note: assumes caller is accessing field once!
==============================================================================*/
{
	int return_code;

	ENTER(MANAGED_OBJECT_NOT_IN_USE(Computed_field));
	return_code = 0;
	if (manager && object)
	{
		if (manager == object->manager)
		{
			if ((2 >= object->access_count) ||
				((MANAGER_CHANGE_NONE(Computed_field) != object->manager_change_status) &&
				 (3 == object->access_count)))
			{
				return_code = object->core ? object->core->not_in_use() : 1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"MANAGED_OBJECT_NOT_IN_USE(Computed_field).  Object is not in this manager");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGED_OBJECT_NOT_IN_USE(Computed_field).  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* MANAGED_OBJECT_NOT_IN_USE(Computed_field) */

DECLARE_ADD_OBJECT_TO_MANAGER_FUNCTION(Computed_field,name,manager)

PROTOTYPE_MANAGER_MODIFY_NOT_IDENTIFIER_FUNCTION(Computed_field, name)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Computed_field type needs a special versions of MANAGER_MODIFY_NOT_IDENTIFIER
since changes to number_of_components are not permitted unless it is NOT_IN_USE.
==============================================================================*/
{
	int return_code;

	ENTER(MANAGER_MODIFY_NOT_IDENTIFIER(Computed_field,name));
	if (manager && object && new_data)
	{
		if (!(manager->locked))
		{
			if (IS_OBJECT_IN_LIST(Computed_field)(object,manager->object_list))
			{
				/* can only change number_of_components and value type if field NOT_IN_USE */
				if (((new_data->number_of_components == object->number_of_components) &&
					(cmzn_field_get_value_type(new_data) == cmzn_field_get_value_type(object))) ||
					MANAGED_OBJECT_NOT_IN_USE(Computed_field)(object, manager))
				{
					return_code = 1;
					if ((new_data->manager) && (new_data->manager != manager))
					{
						display_message(ERROR_MESSAGE,
							"MANAGER_MODIFY_NOT_IDENTIFIER(Computed_field,name).  "
							"Cannot modify definition to depend on field from another region");
						return_code = 0;
					}
					if (return_code)
					{
						/* cache changes because there could be new source fields added to manager and
						 * old, volatile source fields removed by this modification */
						MANAGER_BEGIN_CACHE(Computed_field)(manager);
						if (!MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_field,
							name)(object, new_data))
						{
							display_message(ERROR_MESSAGE,
								"MANAGER_MODIFY_NOT_IDENTIFIER(Computed_field,name).  "
								"Could not copy object");
							return_code = 0;
						}
						MANAGED_OBJECT_CHANGE(Computed_field)(object,
							MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Computed_field));
						MANAGER_END_CACHE(Computed_field)(manager);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"MANAGER_MODIFY_NOT_IDENTIFIER(Computed_field,name).  "
						"Cannot change number of components or value type while field is in use");
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"MANAGER_MODIFY_NOT_IDENTIFIER(Computed_field,name).  "
					"Object is not managed");
				return_code = 0;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"MANAGER_MODIFY_NOT_IDENTIFIER(Computed_field,name).  "
				"Manager is locked");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_MODIFY_NOT_IDENTIFIER(Computed_field,name).  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_MODIFY_NOT_IDENTIFIER(Computed_field,name) */

DECLARE_FIND_BY_IDENTIFIER_IN_MANAGER_FUNCTION(Computed_field, name, const char *)
DECLARE_CREATE_INDEXED_LIST_STL_ITERATOR_FUNCTION(Computed_field,cmzn_field_iterator)

DECLARE_MANAGER_OWNER_FUNCTIONS(Computed_field, struct cmzn_region)

char *Computed_field_manager_get_unique_field_name(
	struct MANAGER(Computed_field) *manager, const char *stem_name,
	const char *separator, int first_number)
{
	char *field_name = NULL;
	ALLOCATE(field_name, char, strlen(stem_name) + strlen(separator) + 20);
	int number = first_number;
	if (number < 0)
	{
		number = NUMBER_IN_MANAGER(Computed_field)(manager) + 1;
	}
	do
	{
		sprintf(field_name, "%s%s%d", stem_name, separator, number);
		number++;
	}
	while (FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(field_name, manager));
	return field_name;
}

cmzn_field_iterator_id Computed_field_manager_create_iterator(
	struct MANAGER(Computed_field) *manager)
{
	if (manager)
		return CREATE_LIST_ITERATOR(Computed_field)(manager->object_list);
	return 0;
}

int cmzn_field_get_cache_index_private(cmzn_field_id field)
{
	return field ? field->cache_index : 0;
}

int cmzn_field_set_cache_index_private(cmzn_field_id field, int cache_index)
{
	if (field && (cache_index >= 0))
	{
		field->cache_index = cache_index;
		return 1;
	}
	return 0;
}

int Computed_field_add_to_manager_private(struct Computed_field *field,
	struct MANAGER(Computed_field) *manager)
{
	int return_code;

	ENTER(Computed_field_add_to_manager_private);
	if (field && manager && (!field->manager))
	{
		if (field->name[0] != 0)
		{
			if (FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(field->name, manager))
			{
				char *unique_name = Computed_field_manager_get_unique_field_name(manager, field->name, "_", 1);
				cmzn_field_set_name(field, unique_name);
				DEALLOCATE(unique_name);
			}
		}
		else
		{
			char *unique_name = Computed_field_manager_get_unique_field_name(manager);
			cmzn_field_set_name(field, unique_name);
			DEALLOCATE(unique_name);
		}
		return_code = ADD_OBJECT_TO_MANAGER(Computed_field)(field,manager);
		if (return_code)
		{
			/* notify field types which need to know when they are managed */
			field->core->field_is_managed();
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_add_to_manager_private.  Invalid argument(s).");
		return_code = 0;
	}
	LEAVE;

	return return_code;
}

Computed_field *Computed_field_create_generic(
	cmzn_field_module *field_module, bool check_source_field_regions,
	int number_of_components,
	int number_of_source_fields, Computed_field **source_fields,
	int number_of_source_values, const double *source_values,
	Computed_field_core *field_core)
{
	Computed_field *field = NULL;

	ENTER(Computed_field_create_generic);
	if ((NULL != field_module) && (0 < number_of_components) &&
		((0 == number_of_source_fields) ||
			((0 < number_of_source_fields) && (NULL != source_fields))) &&
		((0 == number_of_source_values) ||
			((0 < number_of_source_values) && (NULL != source_values))) &&
		(NULL != field_core))
	{
		int return_code = 1;
		cmzn_region *region = cmzn_field_module_get_region_internal(field_module);
		for (int i = 0; i < number_of_source_fields; i++)
		{
			if (NULL != source_fields[i])
			{
				if (check_source_field_regions &&
					(Computed_field_get_region(source_fields[i]) != region))
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_create_generic.  Source field is from a different region");
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_create_generic.  Missing source field");
				return_code = 0;
			}
		}
		if (return_code)
		{
			char *field_name = cmzn_field_module_get_field_name(field_module);
			field = CREATE(Computed_field)(field_name ? field_name : "");
			if (field_name)
			{
				DEALLOCATE(field_name);
			}
			if (NULL != field)
			{
				ACCESS(Computed_field)(field);
				field->number_of_components = number_of_components;
				if (0 < number_of_source_fields)
				{
					ALLOCATE(field->source_fields, struct Computed_field *, number_of_source_fields);
					if (NULL != field->source_fields)
					{
						field->number_of_source_fields = number_of_source_fields;
						for (int i = 0; i < number_of_source_fields; i++)
						{
							field->source_fields[i] = ACCESS(Computed_field)(source_fields[i]);
						}
					}
					else
					{
						return_code = 0;
					}
				}
				if (0 < number_of_source_values)
				{
					ALLOCATE(field->source_values, FE_value, number_of_source_values);
					if (NULL != field->source_values)
					{
						field->number_of_source_values = number_of_source_values;
						for (int i = 0; i < number_of_source_values; i++)
						{
							field->source_values[i] = static_cast<FE_value>(source_values[i]);
						}
					}
					else
					{
						return_code = 0;
					}
				}
				if (return_code)
				{
					// 2nd stage of construction - can fail
					if (!field_core->attach_to_field(field))
					{
						return_code = 0;
					}
				}
				field->core = field_core;
				if (return_code)
				{
					// only some field types implement the following, e.g. set default
					// coordinate system of new field to that of a source field:
					field_core->inherit_source_field_attributes();
					// default coordinate system can also be overridden:
					if (cmzn_field_module_coordinate_system_is_set(field_module))
					{
						struct Coordinate_system coordinate_system =
							cmzn_field_module_get_coordinate_system(field_module);
						Computed_field_set_coordinate_system(field, &coordinate_system);
					}

					Computed_field *replace_field =
						cmzn_field_module_get_replace_field(field_module);
					if (replace_field)
					{
						if (replace_field->core->not_in_use() ||
							(replace_field->core->get_type_string() == field_core->get_type_string()))
						{
							/* copy modifications to existing field. Can fail if new definition is incompatible */
							return_code = MANAGER_MODIFY_NOT_IDENTIFIER(Computed_field,name)(
								replace_field, field,
								cmzn_region_get_Computed_field_manager(region));
							REACCESS(Computed_field)(&field, replace_field);
						}
						else
						{
							display_message(ERROR_MESSAGE, "Computed_field_create_generic.  "
								"Cannot change type of field '%s' while its objects are in use",
								replace_field->name);
							return_code = 0;
						}
					}
					else
					{
						if (!cmzn_region_add_field_private(region, field))
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_create_generic.  Unable to add field to region");
							return_code = 0;
						}
					}
				}
				if (!return_code)
				{
					DEACCESS(Computed_field)(&field);
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_create_generic.  Invalid argument(s)");
	}
	if (field_module)
	{
		// replace_field must not be used for further field creates, so clear
		cmzn_field_module_set_replace_field(field_module, NULL);
	}
	LEAVE;

	return (field);
}

int Computed_field_changed(struct Computed_field *field)
{
	int return_code;

	ENTER(Computed_field_changed);
	if (field)
	{
		return_code = MANAGED_OBJECT_CHANGE(Computed_field)(field,
			MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Computed_field));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_changed.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int Computed_field_dependency_changed(struct Computed_field *field)
{
	int return_code;

	ENTER(Computed_field_dependency_changed);
	if (field)
	{
		return_code = MANAGED_OBJECT_CHANGE(Computed_field)(field,
			MANAGER_CHANGE_DEPENDENCY(Computed_field));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_dependency_changed.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int Computed_field_dependency_change_private(struct Computed_field *field)
{
	int return_code;

	ENTER(Computed_field_dependency_change_private);
	if (field)
	{
		if (field->manager_change_status == MANAGER_CHANGE_NONE(Computed_field))
		{
			ADD_OBJECT_TO_LIST(Computed_field)(field, field->manager->changed_object_list);
		}
		field->manager_change_status |= MANAGER_CHANGE_DEPENDENCY(Computed_field);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_dependency_change_private.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

/** @return  True if field can be evaluated with supplied cache.
 * Must be called to check field evaluate/assign functions from external API.
 */
inline bool cmzn_field_cache_check(cmzn_field_id field, cmzn_field_cache_id cache)
{
	return (field && cache && (field->manager->owner == cache->getRegion()));
}

void cmzn_field::clearCaches()
{
	// some fields (integration, histogram) still have caches in field itself to clear:
	core->clear_cache();
	cmzn_region_id region = this->manager->owner;
	cmzn_set_cmzn_field *all_fields = reinterpret_cast<cmzn_set_cmzn_field *>(this->manager->object_list);
	for (cmzn_set_cmzn_field::iterator iter = all_fields->begin(); iter != all_fields->end(); iter++)
	{
		cmzn_field_id field = *iter;
		if (field->dependsOnField(this))
		{
			cmzn_region_clear_field_value_caches(region, field);
		}
	}
}

int Computed_field_is_defined_in_element(struct Computed_field *field,
	struct FE_element *element)
{
	int return_code = 0;
	if (field && element)
	{
		cmzn_field_module_id field_module = cmzn_field_get_field_module(field);
		cmzn_field_cache_id field_cache = cmzn_field_module_create_cache(field_module);
		cmzn_field_cache_set_element(field_cache, element);
		return_code = cmzn_field_is_defined_at_location(field, field_cache);
		cmzn_field_cache_destroy(&field_cache);
		cmzn_field_module_destroy(&field_module);
	}
	return return_code;
}

int Computed_field_is_defined_in_element_conditional(
	struct Computed_field *field, void *element_void)
{
	return Computed_field_is_defined_in_element(field, static_cast<FE_element*>(element_void));
}

int Computed_field_is_defined_at_node(struct Computed_field *field,
	struct FE_node *node)
{
	int return_code = 0;
	if (field && node)
	{
		cmzn_field_module_id field_module = cmzn_field_get_field_module(field);
		cmzn_field_cache_id field_cache = cmzn_field_module_create_cache(field_module);
		cmzn_field_cache_set_node(field_cache, node);
		return_code = cmzn_field_is_defined_at_location(field, field_cache);
		cmzn_field_cache_destroy(&field_cache);
		cmzn_field_module_destroy(&field_module);
	}
	return return_code;
}

int Computed_field_is_defined_at_node_conditional(struct Computed_field *field,
	void *node_void)
{
	return Computed_field_is_defined_at_node(field, static_cast<FE_node*>(node_void));
}

FieldValueCache *Computed_field_core::createValueCache(cmzn_field_cache& /*parentCache*/)
{
	return new RealFieldValueCache(field->number_of_components);
}

/** @return  true if all source fields are defined at cache location */
bool Computed_field_core::is_defined_at_location(cmzn_field_cache& cache)
{
	for (int i = 0; i < field->number_of_source_fields; ++i)
	{
		if (!cmzn_field_is_defined_at_location(getSourceField(i), &cache))
			return false;
	}
	return true;
}

int Computed_field_is_in_list(struct Computed_field *field,
	void *field_list_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Computed_field conditional/iterator function returning true if <field> is in the
computed <field_list>.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_in_list);
	return_code = IS_OBJECT_IN_LIST(Computed_field)(field,
		(struct LIST(Computed_field) *)field_list_void);
	LEAVE;

	return (return_code);
} /* Computed_field_is_in_list */

int Computed_field_has_string_value_type(struct Computed_field *field,
	void *dummy_void)
{
	USE_PARAMETER(dummy_void);
	return (cmzn_field_get_value_type(field) == CMZN_FIELD_VALUE_TYPE_STRING);
}

int Computed_field_core::has_multiple_times()
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns 1 if any of the source fields have multiple times.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_default_has_multiple_times);
	if (field)
	{
		return_code=0;
		for (i=0;(i<field->number_of_source_fields)&&(!return_code);i++)
		{
			if (Computed_field_has_multiple_times(field->source_fields[i]))
			{
				return_code=1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_default_has_multiple_times.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_default_has_multiple_times */

int Computed_field_or_ancestor_satisfies_condition(struct Computed_field *field,
	LIST_CONDITIONAL_FUNCTION(Computed_field) *conditional_function,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns true if <field> satisfies <conditional_function> with <user_data>. If
not, recursively calls this function for each of its source fields until one
satisfies the function for a true result, or all have failed for false.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_or_ancestor_satisfies_condition);
	if (field && conditional_function)
	{
		if ((conditional_function)(field, user_data))
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
			for (i = 0; (i < field->number_of_source_fields) && (!return_code); i++)
			{
				return_code = Computed_field_or_ancestor_satisfies_condition(
					field->source_fields[i], conditional_function, user_data);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_or_ancestor_satisfies_condition.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_or_ancestor_satisfies_condition */

int Computed_field_for_each_ancestor(struct Computed_field *field,
	LIST_ITERATOR_FUNCTION(Computed_field) *iterator_function, void *user_data)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
For <field> and all of its source Computed_fields, calls <iterator_function>
with <user_data>. Iteration stops if a single iterator_function call returns 0.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_for_each_ancestor);
	if (field && iterator_function)
	{
		return_code = (iterator_function)(field, user_data);
		for (i = 0; (i < field->number_of_source_fields) && return_code; i++)
		{
			return_code = Computed_field_for_each_ancestor(
				field->source_fields[i], iterator_function, user_data);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_for_each_ancestor.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_for_each_ancestor */

// External API
int cmzn_field_assign_mesh_location(cmzn_field_id field,
	cmzn_field_cache_id cache, cmzn_element_id element,
	int number_of_chart_coordinates, const double *chart_coordinates)
{
	if (cmzn_field_cache_check(field, cache) && element && chart_coordinates &&
		(number_of_chart_coordinates >= get_FE_element_dimension(element)) &&
		(CMZN_FIELD_VALUE_TYPE_MESH_LOCATION == cmzn_field_get_value_type(field)))
	{
		MeshLocationFieldValueCache *valueCache = MeshLocationFieldValueCache::cast(field->getValueCache(*cache));
		valueCache->setMeshLocation(element, chart_coordinates);
		enum FieldAssignmentResult result = field->assign(*cache, *valueCache);
		return (result != FIELD_ASSIGNMENT_RESULT_FAIL);
	}
	return 0;
}

// External API
int cmzn_field_assign_real(cmzn_field_id field, cmzn_field_cache_id cache,
	int number_of_values, const double *values)
{
	if (cmzn_field_cache_check(field, cache) && field->isNumerical() &&
		(number_of_values >= field->number_of_components) && values)
	{
		RealFieldValueCache *valueCache = RealFieldValueCache::cast(field->getValueCache(*cache));
		valueCache->setValues(values);
		enum FieldAssignmentResult result = field->assign(*cache, *valueCache);
		return (result != FIELD_ASSIGNMENT_RESULT_FAIL);
	}
	return 0;
}

// External API
int cmzn_field_assign_string(cmzn_field_id field, cmzn_field_cache_id cache,
	const char *string_value)
{
	if (cmzn_field_cache_check(field, cache) && string_value &&
		(CMZN_FIELD_VALUE_TYPE_STRING == cmzn_field_get_value_type(field)))
	{
		StringFieldValueCache *valueCache = StringFieldValueCache::cast(field->getValueCache(*cache));
		valueCache->setString(string_value);
		enum FieldAssignmentResult result = field->assign(*cache, *valueCache);
		return (result != FIELD_ASSIGNMENT_RESULT_FAIL);
	}
	return 0;
}

// Internal function
// Note this returns 0 for undefined as well as false. Not ready to expose in
// external API until this is deemed reasonable.
// Follow older functions and change name to 'evaluates_as_true'?
int cmzn_field_evaluate_boolean(cmzn_field_id field, cmzn_field_cache_id cache)
{
	const FE_value zero_tolerance = 1e-6;
	if (cmzn_field_cache_check(field, cache) && field->isNumerical())
	{
		FieldValueCache *valueCache = field->evaluate(*cache);
		if (valueCache)
		{
			RealFieldValueCache& realValueCache = RealFieldValueCache::cast(*valueCache);
			for (int i = 0; i < field->number_of_components; ++i)
			{
				if ((realValueCache.values[i] < -zero_tolerance) ||
					(realValueCache.values[i] > zero_tolerance))
				{
					return 1;
				}
			}
		}
	}
	return 0;
}

// External API
cmzn_element_id cmzn_field_evaluate_mesh_location(cmzn_field_id field,
	cmzn_field_cache_id cache, int number_of_chart_coordinates,
	double *chart_coordinates)
{
	cmzn_element_id element = 0;
	if (cmzn_field_cache_check(field, cache) && chart_coordinates &&
		(CMZN_FIELD_VALUE_TYPE_MESH_LOCATION == cmzn_field_get_value_type(field)))
	{
		FieldValueCache *valueCache = field->evaluate(*cache);
		if (valueCache)
		{
			MeshLocationFieldValueCache& meshLocationValueCache =
				MeshLocationFieldValueCache::cast(*valueCache);
			int dimension = get_FE_element_dimension(meshLocationValueCache.element);
			if (number_of_chart_coordinates >= dimension)
			{
				for (int i = 0; i < dimension; i++)
				{
					chart_coordinates[i] = meshLocationValueCache.xi[i];
				}
				element = ACCESS(FE_element)(meshLocationValueCache.element);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_field_evaluate_mesh_location.  Invalid argument(s)");
	}
	return element;
}

// External API
// Note: no warnings if not evaluated so can be used for is_defined
int cmzn_field_evaluate_real(cmzn_field_id field, cmzn_field_cache_id cache,
	int number_of_values, double *values)
{
	if (cmzn_field_cache_check(field, cache) && (number_of_values >= field->number_of_components) && values &&
		field->core->has_numerical_components())
	{
		FieldValueCache *valueCache = field->evaluate(*cache);
		if (valueCache)
		{
			RealFieldValueCache& realValueCache = RealFieldValueCache::cast(*valueCache);
			for (int i = 0; i < field->number_of_components; ++i)
			{
				values[i] = realValueCache.values[i];
			}
			return CMZN_OK;
		}
	}
	return !CMZN_OK;
}

// Internal API
// IMPORTANT: Not yet approved for external API!
int cmzn_field_evaluate_real_with_derivatives(cmzn_field_id field,
	cmzn_field_cache_id cache, int number_of_values, double *values,
	int number_of_derivatives, double *derivatives)
{
	if (cmzn_field_cache_check(field, cache) && (number_of_values >= field->number_of_components) && values &&
		(number_of_derivatives > 0) && (number_of_derivatives <= MAXIMUM_ELEMENT_XI_DIMENSIONS) && derivatives &&
		field->core->has_numerical_components())
	{
		RealFieldValueCache *valueCache = field->evaluateWithDerivatives(*cache, number_of_derivatives);
		if (valueCache)
		{
			RealFieldValueCache& realValueCache = RealFieldValueCache::cast(*valueCache);
			if (realValueCache.derivatives_valid)
			{
				int i;
				for (i = 0; i < field->number_of_components; ++i)
				{
					values[i] = realValueCache.values[i];
				}
				int size = field->number_of_components*number_of_derivatives;
				for (i = 0; i < size; ++i)
				{
					derivatives[i] = realValueCache.derivatives[i];
				}
				return CMZN_OK;
			}
		}
	}
	return !CMZN_OK;
}

// External API
// Note: no warnings if not evaluated so can be used for is_defined
char *cmzn_field_evaluate_string(cmzn_field_id field,
	cmzn_field_cache_id cache)
{
	if (cmzn_field_cache_check(field, cache))
	{
		FieldValueCache *valueCache = field->evaluate(*cache);
		if (valueCache)
			return valueCache->getAsString();
	}
	return 0;
}

// External API
int cmzn_field_evaluate_derivative(cmzn_field_id field,
	cmzn_differential_operator_id differential_operator,
	cmzn_field_cache_id cache, int number_of_values, double *values)
{
	if (cmzn_field_cache_check(field, cache) && differential_operator &&
		(number_of_values >= field->number_of_components) && values &&
		field->core->has_numerical_components())
	{
		Field_element_xi_location *element_xi_location =
			dynamic_cast<Field_element_xi_location *>(cache->getLocation());
		if (element_xi_location)
		{
			int element_dimension = element_xi_location->get_dimension();
			if (element_dimension == differential_operator->getDimension())
			{
				FieldValueCache *valueCache = field->evaluateWithDerivatives(*cache, element_dimension);
				if (valueCache)
				{
					RealFieldValueCache& realValueCache = RealFieldValueCache::cast(*valueCache);
					if (realValueCache.derivatives_valid)
					{
						FE_value *derivative = realValueCache.derivatives + (differential_operator->getTerm() - 1);
						for (int i = 0; i < field->number_of_components; i++)
						{
							values[i] = *derivative;
							derivative += element_dimension;
						}
						return CMZN_OK;
					}
				}
			}
		}
	}
	return !CMZN_OK;
}

int cmzn_field_is_defined_at_location(cmzn_field_id field,
	cmzn_field_cache_id cache)
{
	if (cmzn_field_cache_check(field, cache))
		return field->core->is_defined_at_location(*cache);
	return 0;
}

int Computed_field_get_native_discretization_in_element(
	struct Computed_field *field,struct FE_element *element,int *number_in_xi)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
If the <field> or its source field is grid-based in <element>, returns in
<number_in_xi> the numbers of finite difference cells in each xi-direction of
<element>. Note that this number is one less than the number of grid points in
each direction. <number_in_xi> should be allocated with at least as much space
as the number of dimensions in <element>, but is assumed to have no more than
MAXIMUM_ELEMENT_XI_DIMENSIONS so that
int number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS] can be passed to this function.
Returns 0 with no errors if the field is not grid-based.
In particular, make sure all the same field types are supported here and in
Computed_field_set_values_in_[managed_]element.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_native_discretization_in_element);
	if (field && element && number_in_xi &&
		(MAXIMUM_ELEMENT_XI_DIMENSIONS >= get_FE_element_dimension(element)))
	{
		return_code =
			field->core->get_native_discretization_in_element(element, number_in_xi);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_native_discretization_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_native_discretization_in_element */

int Computed_field_core::get_native_discretization_in_element(
	struct FE_element *element,int *number_in_xi)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Inherits its result from the first source field -- if any.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_default_get_native_discretization_in_element);
	if (field&&element&&number_in_xi&&
		(MAXIMUM_ELEMENT_XI_DIMENSIONS>=get_FE_element_dimension(element)))
	{
		if (field->source_fields && (0 < field->number_of_source_fields))
		{
			return_code=Computed_field_get_native_discretization_in_element(
				field->source_fields[0],element,number_in_xi);
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_default_get_native_discretization_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_default_get_native_discretization_in_element */

int recursively_add_source_fields_to_list( struct Computed_field *field, struct LIST(Computed_field) *field_list )
{
	int return_code = 1;
	if ( field )
	{
		ADD_OBJECT_TO_LIST(Computed_field)(field,field_list);
		for (int i=0;(i<field->number_of_source_fields)&&(!return_code);i++)
		{
			recursively_add_source_fields_to_list(field->source_fields[i],field_list);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"recursively_add_sourcefields_to_list.  Invalid argument(s)");
		return_code=0;
	}

	return return_code;
}

int Computed_field_is_coordinate_field(struct Computed_field *field, void *not_in_use)
{
	USE_PARAMETER(not_in_use);
	int response = 0;
	if (field)
	{
		response = field->core->get_attribute_integer(CMZN_FIELD_ATTRIBUTE_IS_COORDINATE);
	}
	return response;
}

int Computed_field_get_domain( struct Computed_field *field, struct LIST(Computed_field) *domain_field_list )
{
	int return_code = 0;
	if (field && domain_field_list)
	{
		return_code = field->core->get_domain( domain_field_list );
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_domain.  Invalid argument(s)");
	}
	return return_code;
}

int Computed_field_is_non_linear(struct Computed_field *field)
{
	int return_code = 0;
	if (field)
	{
		return_code = field->core->is_non_linear();
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_non_linear.  Invalid argument(s)");
	}

	return return_code;
}

int Computed_field_get_number_of_components(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

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
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns an allocated string containing the name of <component_no> of <field>.
If the <field> has special component names then these are returned,
otherwise default names are made out of the character form of the
component_no+1, eg, 1 -> "2".
It is up to the calling function to deallocate the returned string.
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
LAST MODIFIED : 14 August 2006

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
LAST MODIFIED : 14 August 2006

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

const char *Computed_field_get_type_string(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns the string which identifies the type.
The calling function must not deallocate the returned string.
==============================================================================*/
{
	const char *return_string;

	ENTER(Computed_field_get_type_string);
	if (field)
	{
		if (field->core)
		{
			return_string = field->core->get_type_string();
		}
		else
		{
			return_string = (const char *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_field_get_type_string.  Missing field");
		return_string = (const char *)NULL;
	}
	LEAVE;

	return (return_string);
} /* Computed_field_get_type_string */

int Computed_field_set_command_string(struct Computed_field *field,
	const char *command_string)
/*******************************************************************************
LAST MODIFIED : 6 September 2007

DESCRIPTION :
Sets the string that will be printed for the computed fields name.
This may be different from the name when it contains characters invalid for
using as an identifier in the manager, such as spaces or punctuation.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_set_command_string);
	if (field)
	{
		if (field->command_string && (field->command_string != field->name))
		{
			DEALLOCATE(field->command_string);
		}
		field->command_string = duplicate_string(command_string);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_field_set_command_string.  "
			"Missing field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_command_string */

int Computed_field_get_native_resolution(struct Computed_field *field,
	int *dimension, int **sizes,
	struct Computed_field **texture_coordinate_field)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Gets the <dimension>, <sizes>, <minimums>, <maximums> and <texture_coordinate_field> from
the <field>. These parameters will be used in image processing.

==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_get_native_resolution);
	if (field)
	{
		return_code = field->core->get_native_resolution(dimension, sizes,
			texture_coordinate_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_native_resolution.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_get_native_resolution */

int Computed_field_core::get_native_resolution(
	int *dimension, int **sizes,
	struct Computed_field **texture_coordinate_field)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Inherits its result from the first source field that returns it-- if any.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_default_get_native_resolution);
	if (field&&dimension&&sizes&&texture_coordinate_field)
	{
		if (field->source_fields && (0 < field->number_of_source_fields))
		{
			i = 0;
			do
			{
				return_code=Computed_field_get_native_resolution(
					field->source_fields[i],dimension, sizes,
					texture_coordinate_field);
				i++;
			}
			while ((!(*sizes))	&& i < field->number_of_source_fields);
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_default_get_native_resolution.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_default_get_native_resolution */

int Computed_field_has_value_type_mesh_location(struct Computed_field *field,
	void *dummy_void)
{
	USE_PARAMETER(dummy_void);
	return (cmzn_field_get_value_type(field) == CMZN_FIELD_VALUE_TYPE_MESH_LOCATION);
}

int Computed_field_has_numerical_components(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Conditional function returning true if <field> returns numerical components.
Note that whether the numbers were integer, FE_value or double, they may be
returned as FE_value when evaluated.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_has_numerical_components);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code = field->core->has_numerical_components();
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_numerical_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_has_numerical_components */

int Computed_field_is_scalar(struct Computed_field *field,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

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
LAST MODIFIED : 14 August 2006

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
LAST MODIFIED : 14 August 2006

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
LAST MODIFIED : 14 August 2006

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
LAST MODIFIED : 14 August 2006

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
LAST MODIFIED : 14 August 2006

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

int Computed_field_has_16_components(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 21 February 2008

DESCRIPTION :
Iterator/conditional function returning true if <field> has exactly sixteen
components - useful for selecting transformation matrix.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_has_16_components);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		return_code=(16 == field->number_of_components);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_16_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_has_16_components */

int Computed_field_has_n_components(struct Computed_field *field,
	void *components_ptr_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

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

int Computed_field_has_multiple_times(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Conditional function returning true if <field> depends on time.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_has_multiple_times);
	return_code=0;
	if (field)
	{
		return_code = field->core->has_multiple_times();
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_multipletimes.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_has_multiple_times */

int Computed_field_is_orientation_scale_capable(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

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
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Conditional function returning true if the field is suitable for 3-D streamline
tracking. This means it has either 3, 6 or 9 components (with 3 components per
vector), or has a FIBRE coordinate_system, meaning it can be wrapped to produce
9-component fibre_axes.  Also now supports 2 components for use with a 2
component coordinate field.
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
		return_code=((2==field->number_of_components)||(3==field->number_of_components)||
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

int Computed_field_find_element_xi(struct Computed_field *field,
	cmzn_field_cache_id field_cache, const FE_value *values,
	int number_of_values, struct FE_element **element_address, FE_value *xi,
	cmzn_mesh_id mesh, int propagate_to_source, int find_nearest)
{
	int return_code;
	ENTER(Computed_field_find_element_xi);
	if (field && field_cache && values && (number_of_values == field->number_of_components) &&
		element_address && xi && (mesh || *element_address))
	{
		if ((!propagate_to_source) || find_nearest ||
			(!(return_code = field->core->propagate_find_element_xi(*field_cache,
				values, number_of_values, element_address, xi, mesh))))
		{
			return_code = Computed_field_perform_find_element_xi(field, field_cache,
				values, number_of_values, element_address, xi, mesh, find_nearest);
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
}

int Computed_field_is_find_element_xi_capable(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

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
		/* By doing the inversion iterations on the final computed field we
			can do this on all computed fields. */
		return_code=1;
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

int equivalent_computed_fields_at_elements(struct FE_element *element_1,
	struct FE_element *element_2)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

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
LAST MODIFIED : 14 August 2006

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
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Writes the properties of the <field> to the command window.
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
		if (NULL != (temp_string=Coordinate_system_string(&field->coordinate_system)))
		{
			display_message(INFORMATION_MESSAGE,"  coordinate_system = %s\n",
				temp_string);
			DEALLOCATE(temp_string);
		}
		display_message(INFORMATION_MESSAGE,"  field type = %s\n",
			Computed_field_get_type_string(field));
		field->core->list();
		/* write the names of the components */
		if (1<field->number_of_components)
		{
			display_message(INFORMATION_MESSAGE,"  component names:");
			for (i=0;i<field->number_of_components;i++)
			{
				if (NULL != (component_name=Computed_field_get_component_name(field,i)))
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

int list_Computed_field_name(struct Computed_field *field,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

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
			Computed_field_get_type_string(field));
		display_message(INFORMATION_MESSAGE,", %d component(s)",
			field->number_of_components);
		if (NULL != (temp_string=Coordinate_system_string(&field->coordinate_system)))
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
LAST MODIFIED : 14 August 2006

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
			&&(field->coordinate_system.type==other_computed_field->coordinate_system.type)
			/* Ignoring other coordinate_system parameters */
			&&(typeid(field->core)==typeid(other_computed_field->core))
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
			if (return_code)
			{
				return_code = field->core->compare(other_computed_field->core);
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

int Computed_field_core::list()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Default listing of source fields and source values.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_core::list);
	if (field)
	{
		if (0 < field->number_of_source_fields)
		{
			display_message(INFORMATION_MESSAGE,
				"    source fields :");
			for (i = 0 ; i < field->number_of_source_fields ; i++)
			{
				display_message(INFORMATION_MESSAGE,
					" %s", field->source_fields[i]->name);
			}
			display_message(INFORMATION_MESSAGE, "\n");
		}
		if (0 < field->number_of_source_values)
		{
			display_message(INFORMATION_MESSAGE,
				"    values :");
			for (i = 0 ; i < field->number_of_source_values ; i++)
			{
				display_message(INFORMATION_MESSAGE,
					" %g", field->source_values[i]);
			}
			display_message(INFORMATION_MESSAGE, "\n");
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_core::list.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_core::list */

char *Computed_field_core::get_command_string()
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Default listing of source fields and source values.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error, i;

	ENTER(Computed_field_get_command_string(filter));
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string, get_type_string(), &error);
		if (0 < field->number_of_source_fields)
		{
			if (1 == field->number_of_source_fields)
				append_string(&command_string, " field", &error);
			else
				append_string(&command_string, " fields", &error);
			for (i = 0 ; i < field->number_of_source_fields ; i++)
			{
				if (GET_NAME(Computed_field)(field->source_fields[i], &field_name))
				{
					append_string(&command_string, " ", &error);
					make_valid_token(&field_name);
					append_string(&command_string, field_name, &error);
					DEALLOCATE(field_name);
				}
			}
		}
		if (0 < field->number_of_source_values)
		{
			append_string(&command_string, " values", &error);
			for (i = 0 ; i < field->number_of_source_values ; i++)
			{
				sprintf(temp_string, " %g", field->source_values[i]);
				append_string(&command_string, temp_string, &error);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_core::get_command_string.  Missing field");
	}

	return (command_string);
} /* Computed_field_core::get_command_string */

int Computed_field_core::get_domain( struct LIST(Computed_field) *domain_field_list ) const
{
	int return_code = 0;

	if (field && domain_field_list)
	{
		return_code = 1;
		for (int i = 0; (i < field->number_of_source_fields) && return_code; i++)
		{
			return_code = field->source_fields[i]->core->get_domain( domain_field_list );
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_core::get_domain.  Invalid argument(s)");
	}

	return return_code;
}

int Computed_field_core::check_dependency()
{
	if (field)
	{
		if (field->manager_change_status & MANAGER_CHANGE_RESULT(Computed_field))
		{
			return 1;
		}
		for (int i = 0; i < field->number_of_source_fields; i++)
		{
			if (field->source_fields[i]->core->check_dependency())
			{
				Computed_field_dependency_change_private(field);
				return 1;
			}
		}
	}
	return 0;
}

bool Computed_field_core::is_non_linear() const
{
	if (field)
	{
		for (int i = 0; i < field->number_of_source_fields; i++)
		{
			if (field->source_fields[i]->core->is_non_linear())
			{
				return true;
			}
		}
	}
	return false;
}

int Computed_field_broadcast_field_components(
	struct cmzn_field_module *field_module,
	struct Computed_field **field_one, struct Computed_field **field_two)
/*******************************************************************************
LAST MODIFIED : 31 March 2008

DESCRIPTION :
Takes two ACCESSED fields <field_one> and <field_two> and compares their number
of components.  If they are equal then the function just returns.  If one
is a scalar field and the is not then the scalar is wrapped in a composite field
which repeats the scalar to match the non scalar number of components.  The
wrapped field will be DEACCESSED by the function but now will be accessed by
the wrapping field and an ACCESSED pointer to the wrapper field is returned
replacing the wrapped field.
If the two fields are non scalar and have different numbers of components then
nothing is done, although other shape broadcast operations could be proposed
for matrix operations.
==============================================================================*/
{
	int i, number_of_components, return_code, *source_field_numbers,
		*source_value_numbers;
	Computed_field *broadcast_wrapper, ***field_to_wrap;

	ENTER(Computed_field_broadcast_field_components);
	if (field_one && *field_one && field_two && *field_two)
	{
		if ((*field_one)->number_of_components ==
			(*field_two)->number_of_components)
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
			field_to_wrap = (Computed_field ***)NULL;
			if (1 == (*field_one)->number_of_components)
			{
				number_of_components = (*field_two)->number_of_components;
				field_to_wrap = &field_one;
			}
			else if (1 == (*field_two)->number_of_components)
			{
				number_of_components = (*field_one)->number_of_components;
				field_to_wrap = &field_two;
			}
			else
			{
				/* Do nothing at the moment */
				return_code = 1;
			}

			if (field_to_wrap)
			{
				ALLOCATE(source_field_numbers, int, number_of_components);
				ALLOCATE(source_value_numbers, int, number_of_components);
				for (i = 0 ; i < number_of_components ; i++)
				{
					/* First (and only) field */
					source_field_numbers[i] = 0;
					/* First (and only) component */
					source_value_numbers[i] = 0;
				}
				// use temporary field module for broadcast wrapper since needs different defaults
				cmzn_field_module *temp_field_module =
					cmzn_field_module_create(cmzn_field_module_get_region_internal(field_module));
				// wrapper field has same name stem as wrapped field
				cmzn_field_module_set_field_name(temp_field_module, (**field_to_wrap)->name);
				broadcast_wrapper = Computed_field_create_composite(temp_field_module,
					number_of_components,
					/*number_of_source_fields*/1, *field_to_wrap,
					0, (double *)NULL,
					source_field_numbers, source_value_numbers);
				cmzn_field_module_destroy(&temp_field_module);

				DEALLOCATE(source_field_numbers);
				DEALLOCATE(source_value_numbers);

				DEACCESS(Computed_field)(*field_to_wrap);
				*(*field_to_wrap) = broadcast_wrapper;

				return_code = 1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_broadcast_field_components.  Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_broadcast_field_components */

bool cmzn_field_is_managed(cmzn_field_id field)
{
	if (field)
	{
		return (0 != (field->attribute_flags & COMPUTED_FIELD_ATTRIBUTE_IS_MANAGED_BIT));
	}
	return 0;
}

int cmzn_field_set_managed(cmzn_field_id field, bool value)
{
	if (field)
	{
		bool old_value = cmzn_field_is_managed(field);
		if (value)
		{
			field->attribute_flags |= COMPUTED_FIELD_ATTRIBUTE_IS_MANAGED_BIT;
		}
		else
		{
			field->attribute_flags &= ~COMPUTED_FIELD_ATTRIBUTE_IS_MANAGED_BIT;
		}
		if (value != old_value)
		{
			MANAGED_OBJECT_CHANGE(Computed_field)(
				field, MANAGER_CHANGE_NOT_RESULT(Computed_field));
		}
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_field_get_attribute_integer(cmzn_field_id field,
	enum cmzn_field_attribute attribute)
{
	int value = 0;
	if (field)
	{
		switch (attribute)
		{
		case CMZN_FIELD_ATTRIBUTE_IS_MANAGED:
			value = (0 != (field->attribute_flags & COMPUTED_FIELD_ATTRIBUTE_IS_MANAGED_BIT));
			break;
		case CMZN_FIELD_ATTRIBUTE_IS_COORDINATE:
			value = field->core->get_attribute_integer(attribute);
			break;
		case CMZN_FIELD_ATTRIBUTE_NUMBER_OF_COMPONENTS:
			value = field->number_of_components;
			break;
		case CMZN_FIELD_ATTRIBUTE_NUMBER_OF_SOURCE_FIELDS:
			value = field->number_of_source_fields;
			break;
		default:
			display_message(ERROR_MESSAGE,
				"cmzn_field_get_attribute_integer.  Invalid attribute");
			break;
		}
	}
	return value;
}

int cmzn_field_set_attribute_integer(cmzn_field_id field,
	enum cmzn_field_attribute attribute, int value)
{
	int return_code = 0;
	if (field)
	{
		return_code = 1;
		int old_value = cmzn_field_get_attribute_integer(field, attribute);
		enum MANAGER_CHANGE(Computed_field) change =
			MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Computed_field);
		switch (attribute)
		{
		case CMZN_FIELD_ATTRIBUTE_IS_MANAGED:
			if (value)
			{
				field->attribute_flags |= COMPUTED_FIELD_ATTRIBUTE_IS_MANAGED_BIT;
			}
			else
			{
				field->attribute_flags &= ~COMPUTED_FIELD_ATTRIBUTE_IS_MANAGED_BIT;
			}
			change = MANAGER_CHANGE_NOT_RESULT(Computed_field);
			break;
		case CMZN_FIELD_ATTRIBUTE_IS_COORDINATE:
			return_code = field->core->set_attribute_integer(attribute, value);
			if (!return_code)
			{
				display_message(WARNING_MESSAGE,
					"cmzn_field_set_attribute_integer.  Cannot set attribute");
			}
			change = MANAGER_CHANGE_NOT_RESULT(Computed_field);
			break;
		default:
			display_message(ERROR_MESSAGE,
				"cmzn_field_set_attribute_integer.  Cannot set attribute");
			return_code = 0;
			break;
		}
		if (cmzn_field_get_attribute_integer(field, attribute) != old_value)
		{
			MANAGED_OBJECT_CHANGE(Computed_field)(field, change);
		}
	}
	return return_code;
}

double cmzn_field_get_attribute_real(cmzn_field_id field,
	enum cmzn_field_attribute attribute)
{
	double value = 0.0;
	if (field)
	{
		switch (attribute)
		{
		case CMZN_FIELD_ATTRIBUTE_COORDINATE_SYSTEM_FOCUS:
			value = field->coordinate_system.parameters.focus;
			break;
		default:
			display_message(ERROR_MESSAGE,
				"cmzn_field_get_attribute_real.  Invalid attribute");
			break;
		}
	}
	return value;
}

int cmzn_field_set_attribute_real(cmzn_field_id field,
	enum cmzn_field_attribute attribute, double value)
{
	int return_code = 0;
	if (field)
	{
		return_code = 1;
		switch (attribute)
		{
		case CMZN_FIELD_ATTRIBUTE_COORDINATE_SYSTEM_FOCUS:
			if (value > 0.0)
			{
				field->coordinate_system.parameters.focus = value;
				// copy to wrapped FE_field:
				field->core->propagate_coordinate_system();
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"cmzn_field_set_attribute_real.  Coordinate system focus must be positive");
				return_code = 0;
			}
			break;
		default:
			display_message(ERROR_MESSAGE,
				"cmzn_field_set_attribute_real.  Cannot set attribute");
			return_code = 0;
			break;
		}
		if (return_code)
		{
			MANAGED_OBJECT_CHANGE(Computed_field)(field, MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Computed_field));
		}
	}
	return return_code;
}

char *cmzn_field_get_component_name(cmzn_field_id field, int component_number)
{
	return Computed_field_get_component_name(field, component_number - 1);
}

enum cmzn_field_coordinate_system_type cmzn_field_get_coordinate_system_type(
	cmzn_field_id field)
{
	enum cmzn_field_coordinate_system_type coordinate_system_type = CMZN_FIELD_COORDINATE_SYSTEM_TYPE_INVALID;
	if (field)
	{
		switch (field->coordinate_system.type)
		{
			case RECTANGULAR_CARTESIAN:
				coordinate_system_type = CMZN_FIELD_COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN;
				break;
			case CYLINDRICAL_POLAR:
				coordinate_system_type = CMZN_FIELD_COORDINATE_SYSTEM_TYPE_CYLINDRICAL_POLAR;
				break;
			case SPHERICAL_POLAR:
				coordinate_system_type = CMZN_FIELD_COORDINATE_SYSTEM_TYPE_SPHERICAL_POLAR;
				break;
			case PROLATE_SPHEROIDAL:
				coordinate_system_type = CMZN_FIELD_COORDINATE_SYSTEM_TYPE_PROLATE_SPHEROIDAL;
				break;
			case OBLATE_SPHEROIDAL:
				coordinate_system_type = CMZN_FIELD_COORDINATE_SYSTEM_TYPE_OBLATE_SPHEROIDAL;
				break;
			case FIBRE:
				coordinate_system_type = CMZN_FIELD_COORDINATE_SYSTEM_TYPE_FIBRE;
				break;
			default:
				break;
		}
	}
	return coordinate_system_type;
}

int cmzn_field_set_coordinate_system_type(cmzn_field_id field,
	enum cmzn_field_coordinate_system_type coordinate_system_type)
{
	if (!field)
		return 0;
	enum Coordinate_system_type type = NOT_APPLICABLE;
	switch (coordinate_system_type)
	{
		case CMZN_FIELD_COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN:
			type = RECTANGULAR_CARTESIAN;
			break;
		case CMZN_FIELD_COORDINATE_SYSTEM_TYPE_CYLINDRICAL_POLAR:
			type = CYLINDRICAL_POLAR;
			break;
		case CMZN_FIELD_COORDINATE_SYSTEM_TYPE_SPHERICAL_POLAR:
			type = SPHERICAL_POLAR;
			break;
		case CMZN_FIELD_COORDINATE_SYSTEM_TYPE_PROLATE_SPHEROIDAL:
			type = PROLATE_SPHEROIDAL;
			break;
		case CMZN_FIELD_COORDINATE_SYSTEM_TYPE_OBLATE_SPHEROIDAL:
			type = OBLATE_SPHEROIDAL;
			break;
		case CMZN_FIELD_COORDINATE_SYSTEM_TYPE_FIBRE:
			type = FIBRE;
			break;
		default:
			break;
	}
	if (type != field->coordinate_system.type)
	{
		field->coordinate_system.type = type;
		// copy to wrapped FE_field:
		field->core->propagate_coordinate_system();
		MANAGED_OBJECT_CHANGE(Computed_field)(field, MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Computed_field));
	}
	return 1;
}

char *cmzn_field_get_name(cmzn_field_id field)
{
	char *name = NULL;
	GET_NAME(Computed_field)(field, &name);
	return (name);
}

int cmzn_field_set_name(struct Computed_field *field, const char *name)
{
	int return_code;

	ENTER(cmzn_field_set_name);
	if (field && is_standard_object_name(name))
	{
		return_code = 1;
		cmzn_set_cmzn_field *manager_field_list = 0;
		bool restore_changed_object_to_lists = false;
		if (field->manager)
		{
			manager_field_list = reinterpret_cast<cmzn_set_cmzn_field *>(field->manager->object_list);
			if (FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)(
				name, field->manager))
			{
				display_message(ERROR_MESSAGE, "cmzn_field_set_name.  "
					"Field named \"%s\" already exists in this field manager.",
					name);
				return_code = 0;
			}
			if (return_code)
			{
				// this temporarily removes the object from all related lists
				restore_changed_object_to_lists =
					manager_field_list->begin_identifier_change(field);
				if (!restore_changed_object_to_lists)
				{
					display_message(ERROR_MESSAGE, "cmzn_field_set_name.  "
						"Could not safely change identifier in manager");
					return_code = 0;
				}
			}
		}
		if (return_code)
		{
			char *new_name = duplicate_string(name);
			if (new_name)
			{
				/* If this has previously been allocated separately destroy it */
				if (field->command_string != field->name)
				{
					DEALLOCATE(field->command_string);
				}
				DEALLOCATE(field->name);
				field->name = new_name;
				/* Now make them point to the same memory */
				field->command_string = (char *)field->name;
			}
			else
			{
				return_code = 0;
			}
		}
		if (restore_changed_object_to_lists)
		{
			manager_field_list->end_identifier_change();
		}
		if (return_code)
		{
			// allow core type to change name of wrapped objects e.g. FE_field
			field->core->set_name(name);
		}
		if (field->manager)
		{
			if (return_code)
			{
				MANAGED_OBJECT_CHANGE(Computed_field)(field,
					MANAGER_CHANGE_IDENTIFIER(Computed_field));
			}
		}
	}
	else
	{
		if (field)
		{
			display_message(ERROR_MESSAGE,
				"cmzn_field_set_name.  Invalid field name '%s'", name);
		}
		return_code=0;
	}

	return (return_code);
}

cmzn_field_id cmzn_field_get_source_field(cmzn_field_id field, int index)
{
	cmzn_field_id source_field = 0;
	if (field && (0 < index) && (index <= field->number_of_source_fields))
	{
		source_field = ACCESS(Computed_field)(field->source_fields[index - 1]);
	}
	return source_field;
}

int Computed_field_manager_set_region(struct MANAGER(Computed_field) *manager,
	struct cmzn_region *region)
{
	return MANAGER_SET_OWNER(Computed_field)(manager, region);
}

struct cmzn_region *Computed_field_manager_get_region(
	struct MANAGER(Computed_field) *manager)
{
	return MANAGER_GET_OWNER(Computed_field)(manager);
}

const cmzn_set_cmzn_field &Computed_field_manager_get_fields(
	struct MANAGER(Computed_field) *manager)
{
	return const_cast<const cmzn_set_cmzn_field&>(
		*(reinterpret_cast<cmzn_set_cmzn_field*>(manager->object_list)));
}

struct cmzn_region *Computed_field_get_region(struct Computed_field *field)
{
	if (field)
		return MANAGER_GET_OWNER(Computed_field)(field->manager);
	return 0;
}

Computed_field *Computed_field_modify_data::get_field()
{
	return cmzn_field_module_get_replace_field(field_module);
};

cmzn_region *Computed_field_modify_data::get_region()
{
	return cmzn_field_module_get_region_internal(field_module);
};

MANAGER(Computed_field) *Computed_field_modify_data::get_field_manager()
{
	return cmzn_region_get_Computed_field_manager(get_region());
};

int Computed_field_does_not_depend_on_field(Computed_field *field, void *source_field_void)
{
	struct Computed_field *source_field = (struct Computed_field *)source_field_void;
	int return_code = 0;

	if (field && source_field && field != source_field)
	{
		return_code = field->dependsOnField(source_field);
	}

	return !return_code;
}

int Computed_field_is_not_source_field_of_others(struct Computed_field *field)
{
	int return_code = 0;

	if (field->manager)
	{
		return_code = FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
			Computed_field_does_not_depend_on_field,(void *)field,
			field->manager);
	}
	else
	{
		return_code = 1;
	}

	return return_code;
}

/***************************************************************************//**
 * Asks field to propagates changes from sub-region field, as appropriate to
 * field type.
 *
 * @param field  Parent field.
 * @param message_void  Child region field manager change message.
 * @return 1
 */
static int Computed_field_propagate_hierarchical_field_changes(
	Computed_field *field, void *message_void)
{
	if (field)
	{
		MANAGER_MESSAGE(Computed_field) *message =
			(MANAGER_MESSAGE(Computed_field) *)message_void;
		field->core->propagate_hierarchical_field_changes(message);
	}
	return 1;
}

void Computed_field_manager_propagate_hierarchical_field_changes(
	MANAGER(Computed_field) *manager, MANAGER_MESSAGE(Computed_field) *message)
{
	if (manager && message)
	{
		MANAGER_BEGIN_CACHE(Computed_field)(manager);
		FOR_EACH_OBJECT_IN_MANAGER(Computed_field)(
			Computed_field_propagate_hierarchical_field_changes, (void *)message,
			manager);
		MANAGER_END_CACHE(Computed_field)(manager);
	}
}

int Computed_field_manager_message_get_object_change_and_detail(
	struct MANAGER_MESSAGE(Computed_field) *message, struct Computed_field *field,
	const struct cmzn_field_change_detail **change_detail_address)
{
	if (message && field && change_detail_address)
	{
		int i;
		struct MANAGER_MESSAGE_OBJECT_CHANGE(Computed_field) *object_change;

		object_change = message->object_changes;
		for (i = message->number_of_changed_objects; 0 < i; i--)
		{
			if (field == object_change->object)
			{
				*change_detail_address = object_change->detail;
				return (object_change->change);
			}
			object_change++;
		}
	}
	if (change_detail_address)
	{
		*change_detail_address = NULL;
	}
	return (MANAGER_CHANGE_NONE(Computed_field));
}

class cmzn_field_attribute_conversion
{
public:
	static const char *to_string(enum cmzn_field_attribute attribute)
	{
		const char *enum_string = 0;
		switch (attribute)
		{
			case CMZN_FIELD_ATTRIBUTE_IS_MANAGED:
				enum_string = "IS_MANAGED";
				break;
			case CMZN_FIELD_ATTRIBUTE_IS_COORDINATE:
				enum_string = "IS_COORDINATE";
				break;
			case CMZN_FIELD_ATTRIBUTE_NUMBER_OF_COMPONENTS:
				enum_string = "NUMBER_OF_COMPONENTS";
				break;
			case CMZN_FIELD_ATTRIBUTE_NUMBER_OF_SOURCE_FIELDS:
				enum_string = "NUMBER_OF_SOURCE_FIELDS";
				break;
			case CMZN_FIELD_ATTRIBUTE_COORDINATE_SYSTEM_FOCUS:
				enum_string = "COORDINATE_SYSTEM_FOCUS";
				break;
			default:
				break;
		}
		return enum_string;
	}
};

enum cmzn_field_attribute cmzn_field_attribute_enum_from_string(const char *string)
{
	return string_to_enum<enum cmzn_field_attribute,	cmzn_field_attribute_conversion>(string);
}

char *cmzn_field_attribute_enum_to_string(enum cmzn_field_attribute attribute)
{
	const char *attribute_string = cmzn_field_attribute_conversion::to_string(attribute);
	return (attribute_string ? duplicate_string(attribute_string) : 0);
}

class cmzn_field_coordinate_system_type_conversion
{
public:
	static const char *to_string(enum cmzn_field_coordinate_system_type coordinate_system_type)
	{
		const char *enum_string = 0;
		switch (coordinate_system_type)
		{
			case CMZN_FIELD_COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN:
				enum_string = "RECTANGULAR_CARTESIAN";
				break;
			case CMZN_FIELD_COORDINATE_SYSTEM_TYPE_CYLINDRICAL_POLAR:
				enum_string = "CYLINDRICAL_POLAR";
				break;
			case CMZN_FIELD_COORDINATE_SYSTEM_TYPE_SPHERICAL_POLAR:
				enum_string = "SPHERICAL_POLAR";
				break;
			case CMZN_FIELD_COORDINATE_SYSTEM_TYPE_PROLATE_SPHEROIDAL:
				enum_string = "PROLATE_SPHEROIDAL";
				break;
			case CMZN_FIELD_COORDINATE_SYSTEM_TYPE_OBLATE_SPHEROIDAL:
				enum_string = "OBLATE_SPHEROIDAL";
				break;
			case CMZN_FIELD_COORDINATE_SYSTEM_TYPE_FIBRE:
				enum_string = "FIBRE";
				break;
			default:
				break;
		}
		return enum_string;
	}
};

enum cmzn_field_coordinate_system_type cmzn_field_coordinate_system_type_enum_from_string(const char *string)
{
	return string_to_enum<enum cmzn_field_coordinate_system_type, cmzn_field_coordinate_system_type_conversion>(string);
}

char *cmzn_field_coordinate_system_type_enum_to_string(enum cmzn_field_coordinate_system_type coordinate_system_type)
{
	const char *coordinate_system_type_string = cmzn_field_coordinate_system_type_conversion::to_string(coordinate_system_type);
	return (coordinate_system_type_string ? duplicate_string(coordinate_system_type_string) : 0);
}
