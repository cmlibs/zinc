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

- Should handle coordinate system differently. For the majority of field types
it can simply be assumed, eg. from source fields, or RC/irrelevant if scalar.
For those that do allow it to be specified, should make it a leaf option with
the gfx define field commands. Also should not allow coordinate system to be
changed if field is in use, since field may be chosen on the basis of this,
like the number of components.

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
extern "C" {
#include <math.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_find_xi.h"
#include "computed_field/computed_field_composite.h"
}
#include "computed_field/computed_field_private.hpp"
extern "C" {
#include "computed_field/computed_field_set.h"
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
#include "general/any_object_private.h"
#include "region/cmiss_region_private.h"
#include "user_interface/message.h"
}
#include <typeinfo>
#include "user_interface/process_list_or_write_command.hpp"

FULL_DECLARE_INDEXED_LIST_TYPE(Computed_field);

FULL_DECLARE_MANAGER_TYPE(Computed_field);

struct Computed_field_type_data
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Stores information defining a type of computed field so that it can be 
accessed by the rest of the program.
==============================================================================*/
{
	char *name;
	Define_Computed_field_type_function define_Computed_field_type_function;
	Computed_field_type_package *define_type_user_data;
	int access_count;
};

PROTOTYPE_OBJECT_FUNCTIONS(Computed_field_type_data);

DECLARE_LIST_TYPES(Computed_field_type_data);
PROTOTYPE_LIST_FUNCTIONS(Computed_field_type_data);

struct Computed_field_package
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Contains all information for editing and maintaining Computed_fields, including
the MANAGER(Computed_field). Holds pointer to MANAGER(FE_field) since computed
fields are based on them, and callback ID for FE_fields, since Computed_field
wrappers need to be automatically created for each FE_field.
???RC Make macro PACKAGE(Computed_field) etc.?
==============================================================================*/
{
	struct MANAGER(Computed_field) *computed_field_manager;
	struct LIST(Computed_field_type_data) *computed_field_type_list;
	Computed_field_simple_package *simple_package;
}; /* struct Computed_field_package */

/*
Module functions
----------------
*/

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Computed_field,name,char *,strcmp)

DECLARE_LOCAL_MANAGER_FUNCTIONS(Computed_field)
struct Computed_field_type_data *CREATE(Computed_field_type_data)
   (char *name, Define_Computed_field_type_function 
	define_Computed_field_type_function, 
	Computed_field_type_package *define_type_user_data)
/*******************************************************************************
LAST MODIFIED : 24 January 2007

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
LAST MODIFIED : 14 August 2006

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

int Computed_field_clear_type(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

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
/*******************************************************************************
LAST MODIFIED : 14 August 2006

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

struct Computed_field *CREATE(Computed_field)(const char *name)
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
			/* By default the name and the command_string are the same */
			field->command_string = field->name;
			/* initialise all members of computed_field */	
			field->number_of_components = 0;
			/* allowed to modify/remove from manager until disabled with
				 Computed_field_set_read_only */
			field->read_only = 0;
			field->coordinate_system.type = RECTANGULAR_CARTESIAN;
			field->component_names = (char **)NULL;

			/* values/derivatives cache and working_values */
			field->values = (FE_value *)NULL;
			field->derivatives = (FE_value *)NULL;
			field->string_cache = (char *)NULL;
			field->string_component = -1;
			field->values_valid = 0;
			field->derivatives_valid = 0;
			field->element = (struct FE_element *)NULL;
			field->node = (struct FE_node *)NULL;
			field->time = 0;
			field->coordinate_reference_field = (struct Computed_field *)NULL;
			field->field_does_not_depend_on_cached_location = 0;

			field->find_element_xi_cache = (struct Computed_field_find_element_xi_cache *)NULL;

			field->core = (Computed_field_core*)NULL;

			/* for all types of Computed_field calculated from others */
			field->source_fields = (struct Computed_field **)NULL;
			field->number_of_source_fields = 0;
			/* for all Computed_fields which use real source values */
			field->source_values = (FE_value *)NULL;
			field->number_of_source_values = 0;

			field->access_count = 0;

			field->manager = (struct MANAGER(Computed_field) *)NULL;
			field->intermediary_managed_field = 0;
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
			if (component_name=field->component_names)
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
		else if ((1 == object->access_count) &&
			object->intermediary_managed_field && object->manager)
		{
			return_code = REMOVE_OBJECT_FROM_MANAGER(Computed_field)
				(object, object->manager);
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

	ENTER(GET_NAME(object_type));
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
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Do not allow copy if:
- it creates a self-referencing field (one that depends on itself) which will
  result in an infinite loop;
- it changes the number of components of a field in use;
???RC.  Previously denied copy if read_only flag was set in the destination
field. However, this makes it impossible to modify computed fields that wrap
fe_fields when the latter changes. Hence, now leave it up to define/destroy
functions to check if read_only flag is set.
==============================================================================*/
{
	char **component_names;
	Computed_field_core *core;
	FE_value *source_values;
	int i,return_code;
	struct Computed_field **source_fields;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_field,name));
	if (source&&destination&&(source!=destination))
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
			source_fields=(struct Computed_field **)NULL;
			source_values=(FE_value *)NULL;
			core = (Computed_field_core*)NULL;
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
				/* 2. free current type-specific data */
				Computed_field_clear_type(destination);
				/* 3. establish the new type */
				destination->number_of_components=source->number_of_components;
				destination->read_only=source->read_only;
				COPY(Coordinate_system)(&destination->coordinate_system,
					&source->coordinate_system);
				
				destination->component_names = component_names;
				
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

				if (core = source->core->copy(destination))
				{
					destination->core = core;
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_field,name).  "
						"Unable to copy Computed_field_core.");
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

DECLARE_OBJECT_WITH_MANAGER_MANAGER_FUNCTIONS(Computed_field, manager)

PROTOTYPE_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Computed_field)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Computed_field requires a special version of this function mainly due to the
finite_element type which automatically wraps FE_fields. If the computed field
is not itself in use, it calls the field's optional computed_field_not_in_use
function and bases its result on that.
==============================================================================*/
{
	int return_code;

	ENTER(MANAGED_OBJECT_NOT_IN_USE(Computed_field));
	return_code = 0;
	if (manager && object)
	{
		if (!(manager->locked))
		{
			if (IS_OBJECT_IN_LIST(Computed_field)(object, manager->object_list))
			{
				if ((1 == object->access_count) ||
					((2 == object->access_count) &&
						IS_OBJECT_IN_LIST(Computed_field)(object,
							manager->message->changed_object_list)))
				{
					return_code = object->core ? object->core->not_in_use() : 1;
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"MANAGED_OBJECT_NOT_IN_USE(Computed_field).  Object is not managed");
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"MANAGED_OBJECT_NOT_IN_USE(Computed_field).  Manager is locked");
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

DECLARE_ADD_OBJECT_WITH_MANAGER_TO_MANAGER_FUNCTION(Computed_field,name,manager)

PROTOTYPE_MANAGER_MODIFY_FUNCTION(Computed_field, name)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Computed_field type needs a special versions of MANAGER_MODIFY
since changes to number_of_components are not permitted unless it is NOT_IN_USE.
==============================================================================*/
{
	int return_code;
	struct LIST_IDENTIFIER_CHANGE_DATA(Computed_field, name)
		*identifier_change_data;
	struct Computed_field *tmp_object;

	ENTER(MANAGER_MODIFY(Computed_field,name));
	if (manager && object && new_data)
	{
		if (!(manager->locked))
		{
			if (IS_OBJECT_IN_LIST(Computed_field)(object, manager->object_list))
			{
				/* can only change number_of_components if field NOT_IN_USE */
				if ((new_data->number_of_components == object->number_of_components) ||
					MANAGED_OBJECT_NOT_IN_USE(Computed_field)(object, manager))
				{
					if (tmp_object =
						FIND_BY_IDENTIFIER_IN_LIST(Computed_field, name)(
							new_data->name, manager->object_list))
					{
						if (tmp_object == object)
						{
							/* don't need to copy object over itself */
							return_code = 1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"MANAGER_MODIFY(Computed_field,name).  "
								"Source object is also in manager");
							return_code = 0;
						}
					}
					else
					{
						/* must perform IDENTIFIER_CHANGE stuff between BEGIN_CHANGE and
							 END_CHANGE calls; manager message must not be sent while object
							 is part changed and/or temporarily out of the manager! */
						MANAGER_BEGIN_CHANGE(Computed_field)(manager,
							MANAGER_CHANGE_OBJECT(Computed_field), object);
						if (identifier_change_data =
							LIST_BEGIN_IDENTIFIER_CHANGE(Computed_field, name)(object))
						{
							if (!(return_code = MANAGER_COPY_WITH_IDENTIFIER(Computed_field,
								name)(object, new_data)))
							{
								display_message(ERROR_MESSAGE,
									"MANAGER_MODIFY(Computed_field,name).  "
									"Could not copy object");
							}
							if (!LIST_END_IDENTIFIER_CHANGE(Computed_field,
								name)(&identifier_change_data))
							{
								display_message(ERROR_MESSAGE,
									"MANAGER_MODIFY(Computed_field,name).  "
									"Could not restore object to all indexed lists");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"MANAGER_MODIFY(Computed_field,name).  "
								"Could not safely change identifier in indexed lists");
							return_code = 0;
						}
						MANAGER_END_CHANGE(Computed_field)(manager);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"MANAGER_MODIFY(Computed_field,name).  "
						"Cannot change number of components while field is in use");
					return_code = 0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"MANAGER_MODIFY(Computed_field,name).  Object is not managed");
				return_code = 0;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"MANAGER_MODIFY(Computed_field,name).  Manager locked");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_MODIFY(Computed_field,name).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_MODIFY(Computed_field,name) */

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
				/* can only change number_of_components if field NOT_IN_USE */
				if ((new_data->number_of_components == object->number_of_components) ||
					MANAGED_OBJECT_NOT_IN_USE(Computed_field)(object, manager))
				{
					MANAGER_BEGIN_CHANGE(Computed_field)(manager,
						MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Computed_field), object);
					if (MANAGER_COPY_WITHOUT_IDENTIFIER(Computed_field,
						name)(object, new_data))
					{
						return_code = 1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"MANAGER_MODIFY_NOT_IDENTIFIER(Computed_field,name).  "
							"Could not copy object");
						return_code = 0;
					}
					MANAGER_END_CHANGE(Computed_field)(manager);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"MANAGER_MODIFY_NOT_IDENTIFIER(Computed_field,name).  "
						"Cannot change number of components while field is in use");
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

DECLARE_MANAGER_MODIFY_IDENTIFIER_FUNCTION(Computed_field, name, char *)
DECLARE_FIND_BY_IDENTIFIER_IN_MANAGER_FUNCTION(Computed_field, name, char *)

int Computed_field_changed(struct Computed_field *field,
	struct MANAGER(Computed_field) *computed_field_manager)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

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
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Clears any caching of elements/values of <field> and any fields it refers to.
Must call this function for each field after you have used functions
Computed_field_evaluate_in_element or Computed_field_evaluate_at_location and they
are possibly not going to be called again for some time.
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
			DESTROY(Computed_field_find_element_xi_cache)
				(&field->find_element_xi_cache);
		}
		if (field->node)
		{
			DEACCESS(FE_node)(&field->node);
		}
		if (field->coordinate_reference_field)
		{
			DEACCESS(Computed_field)(&field->coordinate_reference_field);
		}
		field->field_does_not_depend_on_cached_location = 0;
		if (field->core)
		{
			field->core->clear_cache();
		}
		if (field->string_cache)
		{
			DEALLOCATE(field->string_cache);
		}
		field->string_component = -1;
		field->derivatives_valid=0;
		field->values_valid=0;
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
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns true if <field> can be calculated in <element>. If the field depends on
any other fields, this function is recursively called for them.
???RC.  Should also ask if derivatives defined for it.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_defined_in_element);
	return_code=0;
	if (field&&element)
	{
		Field_element_xi_location location(element);
		return_code = field->core->is_defined_at_location(&location);
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

int Computed_field_is_defined_in_element_conditional(
	struct Computed_field *field,void *element_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

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

int Computed_field_is_true_in_element(struct Computed_field *field,
	struct FE_element *element,FE_value time)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns true if <field> is determined to be "true" at the centre of <element>.
This is currently that the field is defined and any of the components are non zero.
==============================================================================*/
{
	FE_value zero_tolerance = 1e-6;
	int i, number_of_xi_points, number_in_xi, return_code;
	struct FE_element_shape *shape;
	Triple *xi_points;

	ENTER(Computed_field_is_true_in_element);
	return_code=0;
	if (field&&element)
	{	
		Field_element_xi_location location(element);
		if (field->core->is_defined_at_location(&location))
		{
			number_in_xi = 1;
			get_FE_element_shape(element, &shape);
			if (FE_element_shape_get_xi_points_cell_centres(shape, &number_in_xi,
					&number_of_xi_points, &xi_points) && (number_of_xi_points > 0))
			{
				Field_element_xi_location centre_location(element, xi_points[0], 
					time, /*top_level_element*/(struct FE_element *)NULL);
				if (Computed_field_evaluate_cache_at_location(field, &centre_location))
				{
					return_code = 0;
					for (i = 0 ; (return_code == 0) && 
							  (i < field->number_of_components) ; i++)
					{
						if ((field->values[i] < -zero_tolerance) ||
							(field->values[i] > zero_tolerance))
						{
							return_code = 1;
						}
					}
				}
				else
				{
					return_code = 0;
				}
				DEALLOCATE(xi_points);
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
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_true_at_location.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_true_at_location */

int FE_element_Computed_field_is_not_true_iterator(struct FE_element *element,
	void *computed_field_conditional_data_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Iterator version of NOT Computed_field_is_true_in_element.
==============================================================================*/
{
	int return_code;
	struct Computed_field_conditional_data *data;

	ENTER(FE_element_Computed_field_is_not_true_iterator);
	if (element && (data = (struct Computed_field_conditional_data *)
		computed_field_conditional_data_void) && data->conditional_field)
	{	
		return_code = !Computed_field_is_true_in_element(data->conditional_field,
			element, data->time);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_Computed_field_is_not_true_iterator.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_Computed_field_is_not_true_iterator */

int Computed_field_is_defined_at_node(struct Computed_field *field,
	struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns true if <field> can be calculated at <node>. If the field depends on
any other fields, this function is recursively called for them.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_defined_at_node);
	return_code=0;
	if (field&&node)
	{	
		Field_node_location location(node);
		return_code = field->core->is_defined_at_location(&location);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_defined_at_location.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_defined_at_node */

int Computed_field_is_defined_at_node_conditional(struct Computed_field *field,
	void *node_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

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

int Computed_field_is_defined_at_location(struct Computed_field *field,
	Field_location* location)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns 1 if the all the source fields are defined at the supplied <location>.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_defined_at_location);
	if (field && location)
	{
		return_code = field->core->is_defined_at_location(location);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_defined_at_location.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_defined_at_location */

int Computed_field_core::is_defined_at_location(Field_location* location)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns 1 if the all the source fields are defined at the supplied <location>.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_default_is_defined_at_location);
	if (field && location)
	{
		return_code=1;
		for (i=0;(i<field->number_of_source_fields)&&return_code;i++)
		{
			return_code = Computed_field_is_defined_at_location(
				field->source_fields[i], location);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_default_is_defined_at_location.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_default_is_defined_at_location */

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

int Computed_field_is_true_at_node(struct Computed_field *field,
	struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns true if <field> is determined to be "true" at <node>.  This is currently
that the field is defined and any of the components are non zero.
==============================================================================*/
{
	FE_value zero_tolerance = 1e-6;
	int i, return_code;

	ENTER(Computed_field_is_true_at_node);
	return_code=0;
	if (field&&node)
	{	
		Field_node_location location(node, time);
		if (field->core->is_defined_at_location(&location))
		{
			if (Computed_field_evaluate_cache_at_location(field, &location))
			{
				return_code = 0;
				for (i = 0 ; (return_code == 0) && 
						  (i < field->number_of_components) ; i++)
				{
					if ((field->values[i] < -zero_tolerance) ||
						(field->values[i] > zero_tolerance))
					{
						return_code = 1;
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
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_true_at_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_true_at_node */

int FE_node_Computed_field_is_not_true_iterator(struct FE_node *node,
	void *computed_field_conditional_data_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Iterator version of NOT Computed_field_is_true_at_location.
==============================================================================*/
{
	int return_code;
	struct Computed_field_conditional_data *data;

	ENTER(FE_node_Computed_field_is_not_true_iterator);
	if (node && (data = (struct Computed_field_conditional_data *)
		computed_field_conditional_data_void) && data->conditional_field)
	{	
		return_code = !Computed_field_is_true_at_node(data->conditional_field,
			node, data->time);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_Computed_field_is_not_true_iterator.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_Computed_field_is_not_true_iterator */

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

int FE_node_has_Computed_field_defined(struct FE_node *node,void *field_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
FE_node conditional function version of Computed_field_is_defined_at_location.
==============================================================================*/
{
	int return_code;

	ENTER(FE_node_has_Computed_field_defined);
	return_code=
		Computed_field_is_defined_at_node((struct Computed_field *)field_void,node);
	LEAVE;

	return (return_code);
} /* FE_node_has_Computed_field_defined */

int Computed_field_depends_on_Computed_field(struct Computed_field *field,
	struct Computed_field *other_field)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

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

int Computed_field_depends_on_Computed_field_in_list(
	struct Computed_field *field, struct LIST(Computed_field) *field_list)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns true if <field> depends on any field in <field_list>.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_depends_on_Computed_field_in_list);
	if (field && field_list)
	{
		if (IS_OBJECT_IN_LIST(Computed_field)(field, field_list))
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
			for (i = 0; (i < field->number_of_source_fields) && (!return_code); i++)
			{
				return_code = Computed_field_depends_on_Computed_field_in_list(
					field->source_fields[i], field_list);
			}
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

int Computed_field_add_to_list_if_depends_on_list(
	struct Computed_field *field, void *field_list_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
If <field> depends on a field in <field_list> it is added to the list.
Checks to see if it is already in the list.
==============================================================================*/
{
	int return_code;
	struct LIST(Computed_field) *field_list;

	ENTER(Computed_field_add_to_list_if_depends_on_list);
	if (field && (field_list = (struct LIST(Computed_field) *)field_list_void))
	{
		if (IS_OBJECT_IN_LIST(Computed_field)(field, field_list))
		{
			return_code = 1;
		}
		else
		{
			if (Computed_field_depends_on_Computed_field_in_list(field, field_list))
			{
				return_code = ADD_OBJECT_TO_LIST(Computed_field)(field, field_list);
			}
			else
			{
				return_code = 1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_add_to_list_if_depends_on_list.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_add_to_list_if_depends_on_list */

int Computed_field_evaluate_cache_in_element(
	struct Computed_field *field,struct FE_element *element,FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

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
==============================================================================*/
{
	int cache_is_valid,element_dimension,i,number_of_derivatives,
		return_code;

	ENTER(Computed_field_evaluate_cache_in_element);
	if (field && element)
	{
		element_dimension=get_FE_element_dimension(element);
		return_code=1;
		/* clear the cache if values already cached for a node */
		if (field->node)
		{
			Computed_field_clear_cache(field);
		}
		/* Are the values and derivatives in the cache not already calculated? */
		if ((element != field->element) || (time != field->time) ||
			(calculate_derivatives && (!field->derivatives_valid)))
		{
			cache_is_valid = 0;
		}
		else
		{
			cache_is_valid = 1;
			for (i = 0; cache_is_valid && (i < element_dimension); i++)
			{
				if (field->xi[i] != xi[i])
				{
					cache_is_valid = 0;
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
			}
			field->derivatives_valid=0;
			if (field->string_cache)
			{
				DEALLOCATE(field->string_cache);
			}
			if (return_code)
			{
				/* Before we set up a better typed cache storage we are assuming 
					the evaluate will generate valid values, for those which don't
					this will be set to zero in the evaluate.  This allows the valid
					evaluation to a string, which potentially will expand to more types. */
				field->values_valid = 1;
				if (calculate_derivatives)
				{
					number_of_derivatives = element_dimension;
				}
				else
				{
					number_of_derivatives = 0;
				}
				Field_element_xi_location location(element, xi, time,
					 top_level_element, number_of_derivatives);
				return_code = field->core->evaluate_cache_at_location(&location);
					/* How to specify derivatives or not */
				if (return_code&&calculate_derivatives&&!(field->derivatives_valid))
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_evaluate_cache_in_element.  "
						"Derivatives unavailable for field %s of type %s",field->name,
						Computed_field_get_type_string(field));
					return_code=0;
				}
				if (return_code)
				{
					REACCESS(FE_element)(&field->element, element);
					field->time = time;
					for (i = 0; i < element_dimension; i++)
					{
						field->xi[i] = xi[i];
					}
					for (; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
					{
						field->xi[i] = 0.0;
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

char *Computed_field_evaluate_as_string_at_location(
	struct Computed_field *field,int component_number,
	Field_location *location)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

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
	char *return_string, tmp_string[100];
	int error, i;

	ENTER(Computed_field_evaluate_as_string_in_element);
	return_string=(char *)NULL;
	if (field&&location&&(-1<=component_number)&&
		(component_number < field->number_of_components))
	{
		if (Computed_field_evaluate_cache_at_location(field, location))
		{
			// cache may be valid but we also need to check that the cached string
			// is the string for the component number we are interested in.
			if (field->string_cache && field->string_component == component_number)
			{
				return_string = duplicate_string(field->string_cache);
			}
			else if (field->values_valid)
			{
				error = 0;
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
				field->string_component = component_number;
				field->string_cache = duplicate_string(return_string);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_evaluate_as_string_at_location.  "
					"Cache values invalid.");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_evaluate_as_string_at_location.  "
				"Unable to evaluate field at location.");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_as_string_at_location.  "
			"Invalid argument(s)");
	}
	LEAVE;

	return (return_string);
} /* Computed_field_evaluate_as_string_at_location */

char *Computed_field_evaluate_as_string_in_element(
	struct Computed_field *field,int component_number,
	struct FE_element *element,FE_value *xi,FE_value time,
	struct FE_element *top_level_element)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

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
	char *return_string;

	ENTER(Computed_field_evaluate_as_string_in_element);
	return_string=(char *)NULL;
	if (field&&element&&xi)
	{
		Field_element_xi_location location(element, xi, time, top_level_element);
		return_string = Computed_field_evaluate_as_string_at_location(
			field, component_number, &location);
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

int Computed_field_evaluate_in_element(struct Computed_field *field,
	struct FE_element *element,FE_value *xi,FE_value time, 
	struct FE_element *top_level_element,FE_value *values,FE_value *derivatives)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

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
			time,top_level_element,((FE_value *)NULL != derivatives)))
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
				for (i=field->number_of_components*get_FE_element_dimension(element);0<i;i--)
				{
					*destination = *source;
					source++;
					destination++;
				}
			}
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

int Computed_field_evaluate_cache_at_node(
	struct Computed_field *field,struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

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
{
	int return_code;

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
		if ((node != field->node) || (time != field->time))
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
			if (field->string_cache)
			{
				DEALLOCATE(field->string_cache);
			}
			if (return_code)
			{
				/* Before we set up a better typed cache storage we are assuming 
					the evaluate will generate valid values, for those which don't
					this will be set to zero in the evaluate.  This allows the valid
					evaluation to a string, which potentially will expand to more types. */
				field->values_valid = 1;
				Field_node_location location(node, time);
				return_code = field->core->evaluate_cache_at_location(&location);

				/* Store information about what is cached, or clear it if error */
				if (return_code)
				{
					REACCESS(FE_node)(&field->node, node);
					field->time = time;
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

char *Computed_field_evaluate_as_string_at_node(struct Computed_field *field,
	int component_number, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns a string describing the value/s of the <field> at the <node>. If the
field is based on an FE_field but not returning FE_values, it is asked to supply
the string. Otherwise, a string built up of comma separated values evaluated
for the field in Computed_field_evaluate_cache_at_location. The FE_value exception
is used since it is likely the values are already in the cache in most cases,
or can be used by other fields again if calculated now.
The <component_number> indicates which component to calculate.  Use -1 to 
create a string which represents all the components.
Some basic field types such as CMISS_NUMBER have special uses in this function.
It is up to the calling function to DEALLOCATE the returned string.
==============================================================================*/
{
	char *return_string;

	ENTER(Computed_field_evaluate_as_string_at_location);
	return_string=(char *)NULL;
	if (field&&node)
	{
		Field_node_location location(node, time);
		return_string = Computed_field_evaluate_as_string_at_location(
			field, component_number, &location);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_as_string_in_element.  "
			"Invalid argument(s)");
	}
	LEAVE;

	return (return_string);
} /* Computed_field_evaluate_as_string_at_location */

int Computed_field_evaluate_at_node(struct Computed_field *field,
	struct FE_node *node, FE_value time, FE_value *values)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns the <values> of <field> at <node> if it is defined there. Can verify
this in advance by calling function Computed_field_defined_at_location. Each <field>
has a cache for storing its values and derivatives, which is allocated and the
field->values array filled by a call to Computed_field_evaluate_cache_at_location,
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
		Field_node_location location(node, time);
		if (return_code=Computed_field_evaluate_cache_at_location(field, &location))
		{
			if (field->values_valid)
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
					"Computed_field_evaluate_at_node.  Field '%s' has no numerical values",
					field->name);
				return_code = 0;
			}
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

int Computed_field_evaluate_without_node(struct Computed_field *field,
	 FE_value time, FE_value *values)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns the <values> of <field> for nodal_lookup field if it is defined there. Can verify
this in advance by calling function Computed_field_defined_at_location. Each <field>
has a cache for storing its values and derivatives, which is allocated and the
field->values array filled by a call to Computed_field_evaluate_cache_at_location,
which is then copied to <values> by this function. 

The <values> array must be large enough to store as many FE_values as there are
number_of_components.
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_evaluate_without_node);
	return_code=0;
	if (field)
	{
		 Field_node_location location(NULL, time);
		 if (return_code=Computed_field_evaluate_cache_at_location(field, &location))
		 {
				/* copy values from cache to <values> */
				for (i=0;i<field->number_of_components;i++)
				{
					 values[i]=field->values[i];
				}
		 }
	}
	else
	{
		 display_message(ERROR_MESSAGE,
				"Computed_field_evaluate_without_location.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Computed_field_evaluate_without_node */

int Computed_field_set_values_at_location(struct Computed_field *field,
	Field_location* location, FE_value *values)
/*******************************************************************************
LAST MODIFIED : 31 March 2008

DESCRIPTION :
Sets the <values> of the computed <field> at <location>. Only certain computed field
types allow their values to be set. Fields that deal directly with FE_fields eg.
FINITE_ELEMENT and NODE_VALUE fall into this category, as do the various
transformations, RC_COORDINATE, RC_VECTOR, OFFSET, SCALE, etc. which convert
the values into what they expect from their source field, and then call the same
function for it. If a field has more than one source field, eg. RC_VECTOR, it
can in many cases still choose which one is actually being changed, for example,
the 'vector' field in this case - coordinates should not change. This process
continues until the actual FE_field values at the locationare changed or a field
is reached for which its calculation is not reversible, or is not supported yet.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_set_values_at_location);
	if (field && location && values)
	{
		if (location->set_values_for_location(field, values))
		{
			/* The location has stored the values so we don't need to call the
				actual field */
			return_code = 1;
		}
		else
		{
			/* Normally propagate the set_values call */
			if (!(return_code = field->core->set_values_at_location(location, values)))
			{
				display_message(ERROR_MESSAGE, "Computed_field_set_values_at_location.  "
					"Failed for field %s of type %s", field->name, field->core->get_type_string());
			}
		}
		Computed_field_clear_cache(field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_values_at_location.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_values_at_location */

int Computed_field_set_values_at_node(struct Computed_field *field,
	struct FE_node *node, FE_value time, FE_value *values)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Sets the <values> of the computed <field> at <node>. Only certain computed field
types allow their values to be set. Fields that deal directly with FE_fields eg.
FINITE_ELEMENT and NODE_VALUE fall into this category, as do the various
transformations, RC_COORDINATE, RC_VECTOR, OFFSET, SCALE, etc. which convert
the values into what they expect from their source field, and then call the same
function for it. If a field has more than one source field, eg. RC_VECTOR, it
can in many cases still choose which one is actually being changed, for example,
the 'vector' field in this case - coordinates should not change. This process
continues until the actual FE_field values at the locationare changed or a field
is reached for which its calculation is not reversible, or is not supported yet.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_set_values_at_node);
	if (field && node && values)
	{
		Field_node_location location(node, time);
		if (!(return_code = 
				field->core->set_values_at_location(&location, values)))
		{
			display_message(ERROR_MESSAGE, "Computed_field_set_values_at_node.  "
				"Failed for field %s of type %s", field->name, field->core->get_type_string());
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

int Computed_field_evaluate_at_field_coordinates(struct Computed_field *field,
	struct Computed_field *reference_field, int number_of_input_values,
	FE_value *input_values, FE_value time, FE_value *values)
/*******************************************************************************
LAST MODIFIED : 3 April 2007

DESCRIPTION :
Returns the <values> of <field> at the location of <input_values>
with respect to the <reference_field> if it is defined there.

The <values> array must be large enough to store as many FE_values as there are
number_of_components.
==============================================================================*/
{
	int i,return_code;

	ENTER(Computed_field_evaluate_at_field_coordinates);
	if (field&&reference_field&&number_of_input_values&&input_values&&values)
	{
		Field_coordinate_location location(reference_field,
			number_of_input_values, input_values, time);
		if (return_code=Computed_field_evaluate_cache_at_location(field, &location))
		{
			/* copy values from cache to <values> */
			for (i=0;i<field->number_of_components;i++)
			{
				values[i]=field->values[i];
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_at_field_coordinates.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_evaluate_at_field_coordinates */

int Computed_field_get_values_in_element(struct Computed_field *field,
	struct FE_element *element, int *number_in_xi, FE_value time,
	FE_value **values)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Companion function to Computed_field_set_values_at_location.
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
		element_dimension=get_FE_element_dimension(element);
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
						time,/*top_level_element*/(struct FE_element *)NULL,
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
	struct FE_element *element, FE_value *xi, FE_value time,
	FE_value *values)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Sets the <values> of the computed <field> at <xi> in the <element>. Only certain
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
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_set_values_in_element);
	if (field && element && xi && values)
	{
		Field_element_xi_location location(element, xi, time);
		if (!(return_code = field->core->set_values_at_location(&location, values)))
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_set_values_in_element.  "
				"Failed for field %s of type %s", field->name, field->core->get_type_string());
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

char *Computed_field_get_type_string(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns the string which identifies the type.
The calling function must not deallocate the returned string.
==============================================================================*/
{
	char *return_string;

	ENTER(Computed_field_get_type_string);
	if (field)
	{
		if (field->core)
		{
			return_string = field->core->get_type_string();
		}
		else
		{
			return_string = (char *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_field_get_type_string.  Missing field");
		return_string = (char *)NULL;
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
Inherits its result from the first source field -- if any.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_default_get_native_resolution);
	if (field&&dimension&&sizes&&texture_coordinate_field)
	{
		if (field->source_fields && (0 < field->number_of_source_fields))
		{
			return_code=Computed_field_get_native_resolution(
				field->source_fields[0],dimension, sizes, 
				texture_coordinate_field);
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

int Computed_field_is_read_only(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

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
LAST MODIFIED : 14 August 2006

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
LAST MODIFIED : 14 August 2006

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
	FE_value *xi, int element_dimension, struct Cmiss_region *search_region,
	int propagate_field, int find_nearest_location)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
This function implements the reverse of some certain computed_fields
(Computed_field_is_find_element_xi_capable) so that it tries to find an element
and xi which would evaluate to the given values.
This has been implemented so that the texture_coordinates can be used to extract
information from textures (sample_texture computed_field) and then modified and
then put back into another texture.
The <search_element_group> is the set of elements from which the chosen element
will belong or alternatively this can be NULL and the <*element> set to 
a single element to search in.
If <propagate_field> is set and the field has a find_element_xi_function, it
is called to undo its field calculation and resume the search on its source
field. This can result in less computation, but can fail if the source field
is multivalued, a common case being when it is in a polar coordinate system
since valid values may be a multiple of  2*PI out.
If <propagate_field> is not set or there is no <find_element_xi_function> this
function searches all elements in <search_element_group> trying to find a point
at which the field evaluates to the <values>.
If <propagate_field> is not set then <find_nearest_location> can be set and 
then rather than requiring an exact match the closest location in the 
<search_region> or the <*element> will be found.  If <propagate_field> is set
then the <find_nearest_location> flag is ignored.
Note a copy of the <values> array is immediately made so it will be possible to
pass in pointers to field cache values.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_find_element_xi);
	if (field && values && (number_of_values == field->number_of_components) &&
		element && xi && (search_region || *element))
	{
		if ((!propagate_field) || 
			(!(return_code = field->core->find_element_xi(values, number_of_values,
					element, xi, element_dimension, search_region))))
		{
			return_code = Computed_field_perform_find_element_xi(field,
				values, number_of_values, element, xi, element_dimension, search_region,
				find_nearest_location);
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

struct Add_type_to_option_table_data
{
	struct Option_table *option_table;
	void *field_void;
	void *computed_field_package_void;
};

static int Computed_field_add_type_to_option_table(struct Computed_field_type_data *type,
	void *add_type_to_option_table_data_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

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
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Part of the group of define_Computed_field functions. Here, we already have the
<field> to be modified and have determined the number of components and
coordinate system, and must now determine the type of computed field function
and its parameter fields and values.
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

int define_Computed_field_coordinate_system(struct Parse_state *state,
	void *field_void,void *computed_field_package_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Modifier entry function acting as an optional prerequisite for settings the
coordinate_system. That means that if the token "coordinate_system" or part
thereof is found in the current token, then the coordinate system is read. In
each case, assuming no error has occurred, control passes to the next parsing
level, defining the type of computed field function.  Then, if the
coordinate_system was not explictly stated, it is set in accordance with the type.
Function assumes that <field> is not currently managed, as it would be illegal
to modify it if it was.
==============================================================================*/
{
	char *current_token;
	struct Coordinate_system coordinate_system;
	int return_code;
	struct Computed_field *field;
	struct Option_table *option_table;

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
					option_table = CREATE(Option_table)();
					Option_table_add_entry(option_table,"coordinate_system",
						&coordinate_system, NULL, set_Coordinate_system);
					return_code=Option_table_parse(option_table,state);
					DESTROY(Option_table)(&option_table);
					if(return_code)
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
				/* Write out the help */
				option_table = CREATE(Option_table)();
				Option_table_add_entry(option_table,"[coordinate_system NAME]",
					field_void, computed_field_package_void, 
					define_Computed_field_type);
				return_code=Option_table_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
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
LAST MODIFIED : 14 August 2006

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
	char *current_token;
	int return_code;
	struct Computed_field *existing_field,*temp_field;
	struct Computed_field_package *computed_field_package;
	struct Option_table *help_option_table;

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
							if (temp_field->core && 
								temp_field->core->get_type_string())
							{
								/* add the new field to the manager */
								if (!ADD_OBJECT_TO_MANAGER(Computed_field)(temp_field,
									computed_field_package->computed_field_manager))
								{
									display_message(ERROR_MESSAGE,
										"define_Computed_field.  Unable to add field to manager");
									return_code = 0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"gfx define field:  No field type specified");
								display_parse_state_location(state);
								return_code = 0;
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
					/* Write out the help */
					if (temp_field=CREATE(Computed_field)("dummy"))
					{
						help_option_table = CREATE(Option_table)();
						/* FIELD_NAME */
						Option_table_add_entry(help_option_table,"FIELD_NAME",
							(void *)temp_field, computed_field_package_void,
							define_Computed_field_coordinate_system);
						return_code=Option_table_parse(help_option_table,state);
						DESTROY(Option_table)(&help_option_table);
						DESTROY(Computed_field)(&temp_field);
						return_code=1;
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
		if (temp_string=Coordinate_system_string(&field->coordinate_system))
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

int process_list_or_write_Computed_field_commands(struct Computed_field *field,
	 void *command_prefix_void, Process_list_or_write_command_class *process_message)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Writes the commands needed to reproduce <field> to the command window.
==============================================================================*/
{
	char *command_prefix, *command_string, *field_name, *temp_string;
	int return_code;

	ENTER(list_Computed_field_commands);
	if (field && (command_prefix = (char *)command_prefix_void))
	{
		if (field_name = duplicate_string(field->name))
		{
			make_valid_token(&field_name);
			process_message->process_command(
				 INFORMATION_MESSAGE, "%s%s", command_prefix, field_name);
			DEALLOCATE(field_name);
		}
		if (temp_string = Coordinate_system_string(&field->coordinate_system))
		{
			 process_message->process_command(
					INFORMATION_MESSAGE, " coordinate_system %s",temp_string);
			DEALLOCATE(temp_string);
		}
		if (command_string = field->core->get_command_string())
		{
			process_message->process_command(
				 INFORMATION_MESSAGE, " %s", command_string);
			DEALLOCATE(command_string);
		}
		process_message->process_command(INFORMATION_MESSAGE, ";\n");
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_commands.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	
	return (return_code);
} /* list_Computed_field_commands */

int list_Computed_field_commands(struct Computed_field *field,
	void *command_prefix_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Writes the commands needed to reproduce <field> to the command window.
==============================================================================*/
{
	 int return_code;

	 ENTER(list_Computed_field_commands);
	 if (Process_list_command_class *list_message =
			new Process_list_command_class())
	 {
			return_code = process_list_or_write_Computed_field_commands(
				 field, command_prefix_void, list_message);
			delete list_message;
	 }
	 LEAVE;

	 return (return_code);
}

int list_Computed_field_commands_if_managed_source_fields_in_list(
	struct Computed_field *field, void *list_commands_data_void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Calls list_Computed_field_commands if the field is not already in the list,
has no source fields, or all its source fields are either not managed or
already in the list. If the field is listed, it is added to the list.
Ensures field command list comes out in the order they need to be created.
Note, must be cycled through as many times as it takes till listed_fields -> 0.
Second argument is a struct List_Computed_field_commands_data.
==============================================================================*/
{
	int i, list_field, return_code;
	struct List_Computed_field_commands_data *list_commands_data;

	ENTER(list_Computed_field_commands_if_managed_source_fields_in_list);
	if (field && (list_commands_data =
		(struct List_Computed_field_commands_data *)list_commands_data_void))
	{
		return_code = 1;
		/* is the field not listed yet? */
		if (!IS_OBJECT_IN_LIST(Computed_field)(field,
			list_commands_data->computed_field_list))
		{
			list_field = 1;
			for (i = 0; list_field && (i < field->number_of_source_fields); i++)
			{
				if ((!IS_OBJECT_IN_LIST(Computed_field)(
					field->source_fields[i], list_commands_data->computed_field_list)) &&
					IS_MANAGED(Computed_field)(field->source_fields[i],
						list_commands_data->computed_field_manager))
				{
					list_field = 0;
				}
			}
			if (list_field)
			{
				/* do not list commands for read-only computed fields created
					 automatically by cmgui */
				return_code = ((field->read_only ||
					list_Computed_field_commands(field,
						(void *)list_commands_data->command_prefix)) &&
					ADD_OBJECT_TO_LIST(Computed_field)(field,
						list_commands_data->computed_field_list));
				list_commands_data->listed_fields++;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_commands_if_managed_source_fields_in_list.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_commands_if_managed_source_fields_in_list */

int write_Computed_field_commands_to_comfile(struct Computed_field *field,
	 void *command_prefix_void)
/*******************************************************************************
LAST MODIFIED : 10 August 2007

DESCRIPTION :
Writes the commands needed to reproduce <field> to the com file.
==============================================================================*/
{
	int return_code;

	ENTER(write_Computed_field_commands_to_comfile);
	if (Process_write_command_class *write_message =
		 new Process_write_command_class())
	{
		 return_code = process_list_or_write_Computed_field_commands(
				field,  command_prefix_void, write_message);
		 delete write_message;
	}
	return (return_code);
	LEAVE;

	return (return_code);
} /* write_Computed_field_commands_to_comfile */

int write_Computed_field_commands_if_managed_source_fields_in_list_to_comfile(
	 struct Computed_field *field, void *list_commands_data_void)
/*******************************************************************************
LAST MODIFIED : 10 August 2007

DESCRIPTION :
Calls list_Computed_field_commands if the field is not already in the list,
has no source fields, or all its source fields are either not managed or
already in the list. If the field is listed, it is added to the list.
Ensures field command list comes out in the order they need to be created.
Note, must be cycled through as many times as it takes till listed_fields -> 0.
Second argument is a struct List_Computed_field_commands_data.
==============================================================================*/
{
	int i, list_field, return_code;
	struct List_Computed_field_commands_data *list_commands_data;

	ENTER(write_Computed_field_commands_if_managed_source_fields_in_list_to_comfile);
	if (field && (list_commands_data =
		(struct List_Computed_field_commands_data *)list_commands_data_void))
	{
		return_code = 1;
		/* is the field not listed yet? */
		if (!IS_OBJECT_IN_LIST(Computed_field)(field,
			list_commands_data->computed_field_list))
		{
			list_field = 1;
			for (i = 0; list_field && (i < field->number_of_source_fields); i++)
			{
				if ((!IS_OBJECT_IN_LIST(Computed_field)(
					field->source_fields[i], list_commands_data->computed_field_list)) &&
					IS_MANAGED(Computed_field)(field->source_fields[i],
						list_commands_data->computed_field_manager))
				{
					list_field = 0;
				}
			}
			if (list_field)
			{
				/* do not list commands for read-only computed fields created
					 automatically by cmgui */
				return_code = ((field->read_only ||
					write_Computed_field_commands_to_comfile(field,
						 (void *)list_commands_data->command_prefix)) &&
					ADD_OBJECT_TO_LIST(Computed_field)(field,
						list_commands_data->computed_field_list));
				list_commands_data->listed_fields++;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_commands_if_managed_source_fields_in_list.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_Computed_field_commands_if_managed_source_fields_in_list_to_comfile */

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
			&&(field->read_only==other_computed_field->read_only)
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

int Computed_field_copy_type_specific_and_deaccess(
	struct Computed_field *destination, struct Computed_field *source)
/*******************************************************************************
LAST MODIFIED : 13 May 2008

DESCRIPTION :
Copy the type specific parts of <source> field to <destination>,
namely the number_of_components, the source_fields, the soure_values
and the core.  The <source> field is then DESTROYED.
==============================================================================*/
{
	FE_value *source_values;
	int i, return_code;
	Computed_field **source_fields;

	ENTER(Computed_field_copy_type_specific_and_deaccess);
	if (source && destination)
	{
		source_fields=(struct Computed_field **)NULL;
		source_values=(FE_value *)NULL;
		if (((0==source->number_of_source_fields)||ALLOCATE(source_fields,
				struct Computed_field *,source->number_of_source_fields))&&
			((0==source->number_of_source_values)||ALLOCATE(source_values,
				FE_value,source->number_of_source_values)))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(destination);
			/* 3. establish the new type */
			destination->number_of_components=source->number_of_components;
								
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

			if (destination->core = source->core->copy(destination))
			{
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_copy_type_specific_and_deaccess.  "
					"Unable to copy Computed_field_core.");
				return_code=0;
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
			display_message(ERROR_MESSAGE,
				"Computed_field_copy_type_specific_and_deaccess.  "
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
		}
		DEACCESS(Computed_field)(&source);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_copy_type_specific_and_deaccess.  Missing field");
		return_code=0;
	}

	return (return_code);	
} /* Computed_field_copy_type_specific_and_deaccess */

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
			append_string(&command_string, " fields ", &error);
			for (i = 0 ; i < field->number_of_source_fields ; i++)
			{
				if (GET_NAME(Computed_field)(field->source_fields[i], &field_name))
				{
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

struct Computed_field_package *CREATE(Computed_field_package)(void)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Creates a Computed_field_package which is used by the rest of the program to
access everything to do with computed fields. The computed_field_manager is
created as part of the package.
==============================================================================*/
{
	struct Computed_field_package *computed_field_package;

	ENTER(CREATE(Computed_field_package));
	if (ALLOCATE(computed_field_package,struct Computed_field_package,1)&&
		(computed_field_package->computed_field_manager=
			CREATE(MANAGER(Computed_field))()))
	{
		computed_field_package->computed_field_type_list =
			CREATE(LIST(Computed_field_type_data))();
		computed_field_package->simple_package =
			new Computed_field_simple_package(
				computed_field_package->computed_field_manager);
		computed_field_package->simple_package->addref();
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Computed_field_package).  Not enough memory");
		DEALLOCATE(computed_field_package);
	}
	LEAVE;

	return (computed_field_package);
} /* CREATE(Computed_field_package) */

int DESTROY(Computed_field_package)(
	struct Computed_field_package **package_address)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

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
		computed_field_package->simple_package->removeref();
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
LAST MODIFIED : 14 August 2006

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
	Computed_field_type_package *define_type_user_data)
/*******************************************************************************
LAST MODIFIED : 24 January 2007

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
	if (computed_field_package && name && define_Computed_field_type_function &&
		 define_type_user_data)
	{
		if(data = CREATE(Computed_field_type_data)(name,
			define_Computed_field_type_function, define_type_user_data))
		{
			data->define_type_user_data->addref();
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

int Computed_field_package_remove_types(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 24 January 2007

DESCRIPTION :
Unregisters each of the computed field types added.
==============================================================================*/
{
	int return_code;
	struct Computed_field_type_data *data;

	ENTER(Computed_field_package_remove_types);
	if (computed_field_package)
	{
		while(data = FIRST_OBJECT_IN_LIST_THAT(Computed_field_type_data)(
			(LIST_CONDITIONAL_FUNCTION(Computed_field_type_data) *)NULL, (void *)NULL,
			computed_field_package->computed_field_type_list))
		{
			data->define_type_user_data->removeref();

			REMOVE_OBJECT_FROM_LIST(Computed_field_type_data)(data,
				computed_field_package->computed_field_type_list);
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

Computed_field_simple_package *Computed_field_package_get_simple_package(
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 24 January 2007

DESCRIPTION :
Returns a pointer to a sharable simple type package which just contains a
function to access the Computed_field_package.
==============================================================================*/
{
	Computed_field_simple_package* return_package;

	ENTER(Computed_field_package_get_simple_package);
	if (computed_field_package)
	{
		return_package = computed_field_package->simple_package;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_package_get_simple_package.  Invalid arguments");
		return_package = (Computed_field_simple_package*)NULL;
	}
	LEAVE;

	return (return_package);
} /* Computed_field_package_get_simple_package */

int Computed_field_broadcast_field_components(
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
				broadcast_wrapper = Computed_field_create_composite(
					number_of_components,
					/*number_of_source_fields*/1, *field_to_wrap,
					0, (FE_value *)NULL,
					source_field_numbers, source_value_numbers);
				/* The name of the wrapper should be the same as the wrapped
					field so the list commands still reference the original field,
					and as this field does not go into the MANAGER it can be the same. */
				Computed_field_set_name(broadcast_wrapper, (**field_to_wrap)->name);

				DEALLOCATE(source_field_numbers);
				DEALLOCATE(source_value_numbers);

				DEACCESS(Computed_field)(*field_to_wrap);
				*(*field_to_wrap) = broadcast_wrapper;
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

int Computed_field_set_intermediary_managed_field_flag(
	struct Computed_field *field, int intermediary_managed_field_flag)
/*******************************************************************************
LAST MODIFIED : 17 April 2008

DESCRIPTION :
If this flag is set then when the ACCESS count gets to 1 a managed field
will automatically remove itself from the manager and be destroyed.
When the flag is not set, the field will only be destroyed when given a 
specific command to destroy or as the program ends.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_set_intermediary_managed_field_flag);
	if (field)
	{
		field->intermediary_managed_field = intermediary_managed_field_flag;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_field_set_intermediary_managed_field_flag.  "
			"Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_set_intermediary_managed_field_flag */

int Computed_field_set_name(
	struct Computed_field *field, const char *name)
/*******************************************************************************
LAST MODIFIED : 18 April 2008

DESCRIPTION :
Changes the name of a field.
==============================================================================*/
{
	char *new_name;
	int return_code;
	LIST_IDENTIFIER_CHANGE_DATA(Computed_field, name) *identifier_change_data;

	ENTER(Computed_field_set_name);
	if (field && name)
	{
		if (ALLOCATE(new_name, char, strlen(name)+1))
		{
			return_code = 1;
			if (field->manager)
			{
				if (FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)(
						 const_cast<char *>(name), field->manager))
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_set_name.  "
						"Field named \"%s\" already exists in this field manager.",
						name);
					return_code = 0;
				}
				if (return_code)
				{
					/* must perform IDENTIFIER_CHANGE stuff between BEGIN_CHANGE and
						END_CHANGE calls; manager message must not be sent while object
						is part changed and/or temporarily out of the manager! */
					MANAGER_BEGIN_CHANGE(Computed_field)(field->manager,
						MANAGER_CHANGE_OBJECT(Computed_field), field);
				}
			}
			if (!(identifier_change_data =
					LIST_BEGIN_IDENTIFIER_CHANGE(Computed_field, name)(field)))
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_name.  "
					"Could not safely change identifier in indexed lists");
				return_code = 0;
			}
				
			/* Actually do the change */
			if (return_code)
			{
				/* If this has previously been allocated separately destroy it */
				if (field->command_string != field->name)
				{
					DEALLOCATE(field->command_string);
				}
				DEALLOCATE(field->name);
				field->name = new_name;
				strcpy(field->name, name);
				/* Now make them point to the same memory */
				field->command_string = field->name;
			}
			
			if (!LIST_END_IDENTIFIER_CHANGE(Computed_field,
					name)(&identifier_change_data))
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_set_name  "
					"Could not restore object to all indexed lists");
			}
			if (field->manager)
			{
				MANAGER_END_CHANGE(Computed_field)(field->manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Computed_field_set_name.  "
				"Unable to ALLOCATE memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Computed_field_set_name.  "
			"Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_set_name */

PROTOTYPE_ANY_OBJECT(MANAGER(Computed_field));

DEFINE_ANY_OBJECT(MANAGER(Computed_field))

int Cmiss_region_attach_Computed_field_manager(
	struct Cmiss_region *cmiss_region,
	struct MANAGER(Computed_field) *field_manager)
/*******************************************************************************
LAST MODIFIED : 21 April 2008

DESCRIPTION :
Adds <field_manager> to the list of objects attached to <cmiss_region>.
==============================================================================*/
{
	int return_code;
	struct Any_object *any_object;

	ENTER(Cmiss_region_attach_Computed_field_manager);
	return_code = 0;
	if (cmiss_region && field_manager)
	{
		if (Cmiss_region_get_Computed_field_manager(cmiss_region))
		{
			display_message(ERROR_MESSAGE, "Cmiss_region_attach_Computed_field_manager.  "
				"Cmiss_region already has an Computed_field_manager");
		}
		else
		{
			if ((any_object = CREATE(ANY_OBJECT(manager_Computed_field))(field_manager)) &&
				Cmiss_region_private_attach_any_object(cmiss_region, any_object))
			{
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_region_attach_Computed_field_manager.  Could not attach object");
				DESTROY(Any_object)(&any_object);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_attach_Computed_field_manager.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Cmiss_region_attach_Computed_field_manager */

struct MANAGER(Computed_field) *Cmiss_region_get_Computed_field_manager(
	struct Cmiss_region *cmiss_region)
/*******************************************************************************
LAST MODIFIED : 21 April 2008

DESCRIPTION :
Currently, a Cmiss_region may have at most one Computed_field_manager.
This function returns it, or NULL if no Computed_field_manager found.
==============================================================================*/
{
	struct MANAGER(Computed_field) *field_manager;

	ENTER(Cmiss_region_get_Computed_field_manager);
	field_manager = (struct MANAGER(Computed_field) *)NULL;
	if (cmiss_region)
	{
		field_manager = FIRST_OBJECT_IN_LIST_THAT(ANY_OBJECT(manager_Computed_field))(
			(ANY_OBJECT_CONDITIONAL_FUNCTION(manager_Computed_field) *)NULL, (void *)NULL,
			Cmiss_region_private_get_any_object_list(cmiss_region));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_region_get_Computed_field_manager.  Missing Cmiss_region");
	}
	LEAVE;

	return (field_manager);
} /* Cmiss_region_get_Computed_field_manager */

