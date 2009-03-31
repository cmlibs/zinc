/*******************************************************************************
FILE : computed_field_private.hpp

LAST MODIFIED : 31 March 2008

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
 * Portions created by the Initial Developer are Copyright (C) 2006
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

/* Used by the register_type_function, Computed_field_type_data and 
	Computed_field_add_type_to_option_table*/
typedef int (*Define_Computed_field_type_function)(
	struct Parse_state *state,void *field_void,void *computed_field_package_void);


/*
Computed field types
--------------------
Types used only internally to computed fields.
*/

#include "computed_field/field_location.hpp"
extern "C" {
#include "general/debug.h"
#include "region/cmiss_region.h"
}

/**
 * Argument to field modifier functions containing field to be modified,
 * and the region which source fields should be chosen from.
 */
class Computed_field_modify_data
{
public:
	struct Computed_field *field;
	struct Cmiss_region *region;
	
	Computed_field_modify_data(struct Computed_field *field,
		struct Cmiss_region *region) :
		field(field),
		region(region)
	{
	}
};

class Computed_field_type_package
/*******************************************************************************
LAST MODIFIED : 24 January 2007

DESCRIPTION :
The base class for each computed field classes own package.
Provides reference counting.
==============================================================================*/
{
private:
	unsigned int access_count;

public:
	void addref()
	{
		access_count++;
	}
	void removeref()
	{
		if (access_count > 1)
		{
			access_count--;
		}
		else
		{
			delete this;
		}
	}
   Computed_field_type_package()
	{
		access_count = 0;
	}

protected:
	virtual ~Computed_field_type_package()
	{
	}
};

class Computed_field_simple_package : public Computed_field_type_package
/*******************************************************************************
LAST MODIFIED : 24 January 2007

DESCRIPTION :
Minimum set of type-specific data for gfx define field commands.
Contains nothing now that field manager is extracted from region, which is
passed around as part of Computed_field_modify_data in to_be_modified argument.
==============================================================================*/
{
};

class Computed_field_core
/*******************************************************************************
LAST MODIFIED : 23 August 2006

DESCRIPTION :
This is the internal core which each type of Computed_field will implement.
Separating the Computed_field_core and the public Computed_field enables the
core object to be replaced while maintaining the same wrapper (enabling
changes to an existing Computed_field heirarchy.  It also enables different
interfaces on the internal core to the public interface (which I am maintaining
in C).
==============================================================================*/
{
protected:
	struct Computed_field *field;

public:
	Computed_field_core(Computed_field *parent = (Computed_field*)NULL)
		: field(parent)
	{
	};

	virtual ~Computed_field_core()
	{
	};

	virtual Computed_field_core *copy(Computed_field* parent) = 0;

	virtual char *get_type_string() = 0;

	virtual int clear_cache()
	{
		return 1;
	};

	virtual int compare(Computed_field_core* other) = 0;

	virtual int is_defined_at_location(Field_location* location);

	virtual int has_numerical_components()
	{
		return 1;
	};

	virtual int not_in_use()
	{
		return 1;
	};

	virtual int evaluate_cache_at_location(Field_location* location) = 0;

	virtual int set_values_at_location(Field_location* location, FE_value *values)
	{
		return 0;
	};

	virtual int get_native_discretization_in_element(struct FE_element *element,
		int *number_in_xi);

	virtual int find_element_xi(FE_value *values, int number_of_values, 
		struct FE_element **element, FE_value *xi,
		int element_dimension, struct Cmiss_region *search_region)
	{
		return 0;
	};

	virtual int list();

	virtual char* get_command_string();

	virtual int has_multiple_times();

	virtual int get_native_resolution(int *dimension, int **sizes,
		struct Computed_field **texture_coordinate_field);

	virtual int check_source_fields_manager(MANAGER(Computed_field) **manager_address);

	virtual int manage_source_fields(MANAGER(Computed_field) *manager);
}; /* class Computed_field_core */

struct Computed_field
/*******************************************************************************
LAST MODIFIED : 23 August 2006

DESCRIPTION :
==============================================================================*/
{
	/* the name/identifier of the Computed_field */
	const char *name;
	/* The command string is what is printed for GET_NAME.  This is usually
		the same as the name (and just points to it) however it is separated
		out so that we can specify an string for the command_string which is not
		a valid identifier (contains spaces etc.) */
	char *command_string;
	int number_of_components;
	/* This is set for fields where the components have names other than
		the defaults of 1,2...number_of_components */
	char **component_names;

	/* if the following flag is set, the field may not be modified or destroyed
		 by the user. See Computed_field_set_read_only function */
	int read_only;
	struct Coordinate_system coordinate_system;

	/* Value cache: This should probably form another object kept here, rather
	 than explicit storage. */
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
	/* flag to say whether the values are valid (normally this will be true,
		but is the beginning of different types of values, 
		where a field may return a string_cache but not valid values */
	int values_valid;
	/* flag saying whether derivatives were calculated */
	int derivatives_valid;
	/* A string cache value */
	char *string_cache;

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
	/* last time the string cache was evaluated this is the component that was 
		requested (-1 for all components) */
	int string_component;
	/* Record what reference field the values are calculated for with
		a coordinate location. */
	struct Computed_field *coordinate_reference_field;
	/* Cache whether or not the value in the cache depends on the
		cached location (node, element or coordinate_reference_field).
		If it does not then the cache can be deemed valid if the next
		location is the same type and for coordinates, has the same
		reference field but different values. */
	int field_does_not_depend_on_cached_location;

	/* cache used when doing find_xi calculations */
	struct Computed_field_find_element_xi_cache *find_element_xi_cache;

	Computed_field_core* core;

	/* for all Computed_field_types calculated from others */

	/* array of computed fields this field is calculated from */
	int number_of_source_fields;
	struct Computed_field **source_fields;
	/* array of constant values this field is calculated from */
	int number_of_source_values;
	FE_value *source_values;

	int access_count;
	
	/* Keep a reference to the objects manager */
	struct MANAGER(Computed_field) *manager;

	/** Mode/flags controlling how this field is managed by a region. */
	enum Managed_status
	{
		/** If bit set, remove from region when unused / access_count reduced to 1 */
		REMOVE_IF_UNUSED_BIT = 4,

		/** Field is not managed by a region */
		UNMANAGED = 0,

		/** Field is managed by region with public visibility */
		MANAGED_PUBLIC = 1,

		/** Field is managed by region with private visibility */
		MANAGED_PRIVATE = 3,

		/** Field is managed by region with private visibility, and is volatile */
		MANAGED_PRIVATE_VOLATILE = MANAGED_PRIVATE + REMOVE_IF_UNUSED_BIT
	};
	Managed_status managed_status;

}; /* struct Computed_field */

/*
Computed field functions
------------------------
Functions used only internally to computed fields or the region that owns them.
*/

struct Computed_field_constructor *CREATE(Computed_field_constructor)
	(Computed_field_core *core, int number_of_components);
/*******************************************************************************
LAST MODIFIED : 9 May 2008

DESCRIPTION :
Creates a structure representing a type of computed field.
==============================================================================*/

int DESTROY(Computed_field_constructor)
	(struct Computed_field_constructor **type_address);
/*******************************************************************************
LAST MODIFIED : 9 May 2008

DESCRIPTION :
Frees memory/deaccess data at <*type_address>.
==============================================================================*/

int Computed_field_constructor_add_source_field(
	struct Computed_field_constructor *field_type,
	struct Computed_field *source_field);
/*******************************************************************************
LAST MODIFIED : 9 May 2008

DESCRIPTION :
Add the <source_field> to the list of fields referenced by <field_type>.
==============================================================================*/

int Computed_field_constructor_add_source_value(
	struct Computed_field_constructor *field_type, FE_value source_value);
/*******************************************************************************
LAST MODIFIED : 9 May 2008

DESCRIPTION :
Add the <value> to the list of values referenced by <field_type>.
==============================================================================*/

/***************************************************************************//**
 * Sets the Cmiss_region_fields object which will own this manager.
 * Private! Only to be called only from Cmiss_region object.
 * 
 * @param manager  Computed field manager.
 * @return  The owning Cmiss_region_fields object.
 */
int Computed_field_manager_set_owner(struct MANAGER(Computed_field) *manager,
	struct Cmiss_region_fields *region_fields);

/***************************************************************************//**
 * Returns the Cmiss_region_fields object this field is packaged into, if any.
 * Private! Only to be called only by Cmiss_region object.
 * 
 * @param field  The field.
 * @return  A handle to the owning region fields object or NULL if none.
 */
struct Cmiss_region_fields *Computed_field_get_owner(struct Computed_field *field);

int Computed_field_changed(struct Computed_field *field,
	struct MANAGER(Computed_field) *computed_field_manager);
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Notifies the <computed_field_manager> that the <field> has changed.
==============================================================================*/

int Computed_field_package_add_type(
	struct Computed_field_package *computed_field_package, const char *name,
	Define_Computed_field_type_function define_Computed_field_type_function,
	Computed_field_type_package *define_type_user_data);
/*******************************************************************************
LAST MODIFIED : 24 January 2007

DESCRIPTION :
Adds the type of Computed_field described by <name> and 
<define_Computed_field_type_function> to those in the LIST held by the 
<computed_field_package>.  This type is then added to the 
define_Computed_field_type option table when parsing commands.
==============================================================================*/

Computed_field_simple_package *Computed_field_package_get_simple_package(
	struct Computed_field_package *computed_field_package);
/*******************************************************************************
LAST MODIFIED : 24 January 2007

DESCRIPTION :
Returns a pointer to a sharable simple type package which just contains a
function to access the Computed_field_package.
==============================================================================*/

int Computed_field_set_command_string(struct Computed_field *field,
	const char *command_string);
/*******************************************************************************
LAST MODIFIED : 6 September 2007

DESCRIPTION :
Sets the string that will be printed for the computed fields name.
This may be different from the name when it contains characters invalid for
using as an identifier in the manager, such as spaces or punctuation.
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

/***************************************************************************//**
 * Copy the type specific parts of <source> field to <destination>, namely the
 * number_of_components, the source_fields, the soure_values and the core. 
 * The <source> field is then DEACCESSED.
 * Chiefly used in gfx define field commands to copy a new field's contents over
 * an existing field.
 * Note: destination may not be managed since this function could destroy
 * volatile source fields, sending change messages while destination is corrupt.
 * 
 * @destination  Field being modified to have a copy of type-specific data.
 * @source  Field providing the type-specific data. Deaccessed at end.
 * @return  1 on success, 0 on failure.
 */
int Computed_field_copy_type_specific_and_deaccess(
	struct Computed_field *destination, struct Computed_field *source);

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

int Computed_field_is_defined_at_location(struct Computed_field *field,
	Field_location* location);
/*******************************************************************************
LAST MODIFIED : 9 August 2006

DESCRIPTION :
Returns 1 if the all the source fields are defined at the supplied <location>.
==============================================================================*/

int Computed_field_set_coordinate_system_from_sources(
	struct Computed_field *field);
/*******************************************************************************
LAST MODIFIED : 3 July 2000

DESCRIPTION :
Sets the coordinate system of the <field> to match that of it's sources.
==============================================================================*/

int Computed_field_broadcast_field_components(
	struct Computed_field **field_one, struct Computed_field **field_two);
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

inline int Computed_field_evaluate_cache_at_location(
	Computed_field *field, Field_location* location)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Calculates the values of <field> at <location>.
Upon successful return the node values of the <field> are stored in its cache.
==============================================================================*/
{
	 int i, return_code;

	ENTER(Computed_field_evaluate_cache_at_location);
#if ! defined (OPTIMISED)
	if (field && field->core && location)
	{
#endif /* ! defined (OPTIMISED) */
		return_code=1;

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
			if (field->values && return_code)
			{
				 for (i=0;i<(field->number_of_components);i++)
				 {
						field->values[i]=0;
				 }
			}
		}
		
		if (!location->check_cache_for_location(field))
		{
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
				return_code = field->core->evaluate_cache_at_location(location);

				if (return_code)
				{
					location->update_cache_for_location(field);
				}
				else
				{
					/* Not calling clear_cache here because that does far more
						work and propagates back down to the source fields */
					if (field->node)
					{
						DEACCESS(FE_node)(&field->node);
					}
					if (field->element)
					{
						DEACCESS(FE_element)(&field->element);
					}
					if (field->coordinate_reference_field)
					{
						DEACCESS(Computed_field)(&field->coordinate_reference_field);
					}
					field->field_does_not_depend_on_cached_location = 0;
				}
			}
		}
#if ! defined (OPTIMISED)
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_cache_at_location.  Invalid argument(s)");
		return_code=0;
	}
#endif /* ! defined (OPTIMISED) */
	LEAVE;

	return (return_code);
} /* Computed_field_evaluate_cache_at_location */

int Computed_field_set_values_at_location(struct Computed_field *field,
	Field_location* location, FE_value *values);
/*******************************************************************************
LAST MODIFIED : 10 August 2006

DESCRIPTION :
Sets the <values> of the computed <field> at <location>. Only certain computed field
types allow their values to be set. Fields that deal directly with FE_fields eg.
FINITE_ELEMENT and NODE_VALUE fall into this category, as do the various
transformations, RC_COORDINATE, RC_VECTOR, OFFSET, SCALE, etc. which convert
the values into what they expect from their source field, and then call the same
function for it. If a field has more than one source field, eg. RC_VECTOR, it
can in many cases still choose which one is actually being changed, for example,
the 'vector' field in this case - coordinates should not change. This process
continues until the actual FE_field values at the node are changed or a field
is reached for which its calculation is not reversible, or is not supported yet.
==============================================================================*/

inline int Computed_field_evaluate_source_fields_cache_at_location(
	struct Computed_field *field, Field_location *location)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Calculates the cache values of each source field in <field> at <node>, if it 
is defined.
Upon successful return the node values of the source fields are stored in their
cache.
==============================================================================*/
{
	int i, return_code;

	ENTER(Computed_field_evaluate_source_fields_cache_at_location);
#if ! defined (OPTIMISED)
	/* These checks cost us a lot when evaluating thousands of fields */
	if (field && location)
	{
#endif /* ! defined (OPTIMISED) */
		return_code = 1;
		/* calculate values of source_fields */
		for (i=0;(i<field->number_of_source_fields)&&return_code;i++)
		{
			return_code=Computed_field_evaluate_cache_at_location(
				field->source_fields[i], location);
		}
#if ! defined (OPTIMISED)
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_evaluate_source_fields_cache_at_location.  "
			"Invalid argument(s)");
		return_code=0;
	}
#endif /* ! defined (OPTIMISED) */
	LEAVE;

	return (return_code);
} /* Computed_field_evaluate_source_fields_cache_at_location */

/***************************************************************************//**
 * Checks field is dependent on source fields in at most one manager.
 *
 * @param field  Field to be checked.
 * @param manager_address  Address of manager which must be initialised to the
 *   required manager if known, or NULL if unknown. On return points at the
 *   first manager used by field or any of its source fields, or NULL if none.
 * @return  1 if valid i.e. field depends on fields from at most one manager;
 *   0 if invalid combination of fields from more than one mnanager.
 */
int Computed_field_check_manager(struct Computed_field *field, 
	struct MANAGER(Computed_field) **manager_address);

/***************************************************************************//**
 * Ensures field is in the manager with the desired status.
 * If not currently managed, first recursively adds any unmanaged source fields
 * to manager with status Computed_field::MANAGED_PRIVATE_VOLATILE.
 * Assumes check_manager has been used to ensure fields are not already in
 * another manager.
 * Note: Should only be called by Cmiss_region_add_field.
 *
 * @param field  Field to be added to the manager.
 * @param manager  Computed field manager.
 * @return  1 if field successfully added to manager, 0 if not added.
 */
int Computed_field_manage(struct Computed_field *field, 
	struct MANAGER(Computed_field) *manager,
	Computed_field::Managed_status managed_status);

#endif /* !defined (COMPUTED_FIELD_PRIVATE_H) */
