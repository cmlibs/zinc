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

#include "general/cmiss_set.hpp"
#include "computed_field/field_location.hpp"
extern "C" {
#include "computed_field/computed_field.h"
#include "general/debug.h"
#include "region/cmiss_region.h"
}

/**
 * Argument to field modifier functions supplying region, default name,
 * coordinate system etc.
 * Previously had other parameters, but now just wraps the field_module.
 */
class Computed_field_modify_data
{
private:
	Cmiss_field_module *field_module;

public:
	Computed_field_modify_data(
		struct Cmiss_field_module *field_module) :
		field_module(field_module)
	{
	};

	~Computed_field_modify_data()
	{
	};

	Cmiss_field_module *get_field_module()
	{
		return field_module;
	};

	/**
	 * Take ownership of field reference so caller does not need
	 * to deaccess the supplied field.
	 * Sets MANAGED attribute so it is not destroyed.
	 * 
	 * @param new_field  Field to take ownership of. 
	 * @return  1 if field supplied, 0 if not.
	 */
	int update_field_and_deaccess(Computed_field *new_field)
	{
		if (new_field)
		{
			Cmiss_field_set_attribute_integer(new_field, CMISS_FIELD_ATTRIBUTE_IS_MANAGED, 1);
			DEACCESS(Computed_field)(&new_field);
			return 1;
		}
		return 0;
	};

	/**
	 * Get existing field to replace, if any.
	 */
	Computed_field *get_field();

	Cmiss_region *get_region();

	MANAGER(Computed_field) *get_field_manager();
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

/***************************************************************************//**
 * Base class of type-specific field change details.
 */
struct Cmiss_field_change_detail
{
	virtual ~Cmiss_field_change_detail()
	{
	}
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
	Computed_field_core()
		: field(NULL)
	{
	};

	virtual ~Computed_field_core()
	{
	};

	/**
	 * Last stage of construction - attach to parent field. Override to perform
	 * additional tasks after calling base class attach_to_field.
	 * NOTE: do not modify parent field in this function - use
	 * inherit_source_field_attributes() to copy default coordinate system
	 * from source fields, etc.
	 *
	 * @param parent  Field to attach core to
	 * @return  true on success, false if construction of field failed.  
	 */
	virtual bool attach_to_field(Computed_field* parent)
	{
		if (parent)
		{
			field = parent;
			return true;
		}
		return false;
	};

	struct Computed_field *getField()
	{
		return field;
	}
	
	/**
	 * Override to inherit attributes such as coordinate system from source fields.
	 */
	virtual void inherit_source_field_attributes()
	{
	};
	
	virtual Computed_field_core *copy() = 0;

	virtual const char *get_type_string() = 0;

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

	/** Override & return 1 for field types supporting the sum_square_terms API */
	virtual int supports_sum_square_terms() const
	{
		return 0;
	}

	/** Override for field types whose value is a sum of squares to get the
	 * number of terms summed. Multiply by number of components to get number of values.
	 * Can be expensive. */
	virtual int get_number_of_sum_square_terms() const
	{
		return 0;
	}

	/** Override for field types whose value is a sum of squares to get the array of
	 * individual terms PRIOR to being squared*/
	virtual int evaluate_sum_square_terms_at_location(Field_location* /*location*/,
		int /*number_of_values*/, FE_value* /* *values*/)
	{
		return 0;
	}

	virtual int set_values_at_location(Field_location* /*location*/, const FE_value * /*values*/)
	{
		return 0;
	};

	virtual int set_mesh_location_value(Field_location* /*location*/, Cmiss_element_id /*element*/, const FE_value* /*xi*/)
	{
		return 0;
	};

	virtual int set_string_at_location(Field_location* /*location*/, const char * /*string_value*/)
	{
		return 0;
	};

	virtual int get_native_discretization_in_element(struct FE_element *element,
		int *number_in_xi);

	virtual int propagate_find_element_xi(const FE_value * /*values*/, int /*number_of_values*/,
		struct FE_element ** /*element_address*/, FE_value * /*xi*/,
		FE_value /*time*/, Cmiss_mesh_id /*mesh*/)
	{
		return 0;
	};

	virtual int list();

	virtual char* get_command_string();

	virtual int has_multiple_times();

	virtual int get_native_resolution(int *dimension, int **sizes,
		struct Computed_field **texture_coordinate_field);

	/* override if field type needs to be informed when it has been added to region */
	virtual void field_is_managed(void)
	{
	}

	/* override if field is a domain */
	virtual int get_domain( struct LIST(Computed_field) *domain_field_list ) const;

	// override for customised response to propagating field changes to dependencies
	// @return  1 if field has changed directly or via dependency
	virtual int check_dependency();

	// override if field knows its function is non-linear over its domain
	// base implementation returns true if any source fields are non_linear.
	// Overrides must call the base implementation if function is not non-linear
	// and there are source fields.
	virtual bool is_non_linear() const;

	/** called by Cmiss_field_set_name. Override to rename wrapped objects e.g. FE_field */
	virtual int set_name(const char *name)
	{
		USE_PARAMETER(name);
		return 1;
	}

	// override if attribute supported by field type
	// used only for attributes not stored in the generic field object
	virtual int get_attribute_integer(enum Cmiss_field_attribute attribute) const
	{
		USE_PARAMETER(attribute);
		return 0;
	}

	// override if attribute can be set for field type
	// used only for attributes not stored in the generic field object
	virtual int set_attribute_integer(enum Cmiss_field_attribute attribute, int value)
	{
		USE_PARAMETER(attribute);
		USE_PARAMETER(value);
		return 0;
	}

	/** clones and clears type-specific change detail, if any.
	 * override for classes with type-specific change detail e.g. group fields
	 * @return  change detail prior to clearing, or NULL if none.
	 */
	virtual Cmiss_field_change_detail *extract_change_detail()
	{
		return NULL;
	}

	virtual const Cmiss_field_change_detail *get_change_detail() const
	{
		return NULL;
	}

	/** override for hierarchical fields to merge changes from sub-region fields */
	virtual void propagate_hierarchical_field_changes(MANAGER_MESSAGE(Computed_field) *message)
	{
		USE_PARAMETER(message);
	}

	/** default implementation converts numerical values into string. Override for
	 * non-string, non-numeric type to make string version of field values.
	 */
	virtual int make_string_cache(int component_number = -1);

	/** override for MESH_LOCATION value type fields to return cached value */
	virtual Cmiss_element_id get_mesh_location_value(FE_value *xi) const
	{
		USE_PARAMETER(xi);
		return 0;
	}

	/** Override if field is not real valued */
	virtual Cmiss_field_value_type get_value_type() const
	{
		return CMISS_FIELD_VALUE_TYPE_REAL;
	}

}; /* class Computed_field_core */

/** Flag attributes for generic fields */
enum Computed_field_attribute_flags
{
	COMPUTED_FIELD_ATTRIBUTE_IS_MANAGED_BIT = 1
	/*!< If NOT set, destroy field when only access is from region.
	 * @see CMISS_FIELD_ATTRIBUTE_IS_MANAGED */
};

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
	
	/* after clearing in create, following to be modified only by manager */
	/* Keep a reference to the objects manager */
	struct MANAGER(Computed_field) *manager;
	int manager_change_status;
	
	/** bit flag attributes. @see Computed_field_attribute_flags. */
	int attribute_flags;

	inline Computed_field *access()
	{
		++access_count;
		return this;
	}

	static inline int deaccess(Computed_field **field_address)
	{
		return DEACCESS(Computed_field)(field_address);
	}

}; /* struct Computed_field */

/* Only to be used from FIND_BY_IDENTIFIER_IN_INDEXED_LIST_STL function
 * Creates a pseudo object with name identifier suitable for finding
 * objects by identifier with Cmiss_set.
 */
class Computed_field_identifier : private Computed_field
{
public:
	Computed_field_identifier(const char *name)
	{
		Computed_field::name = name;
	}

	~Computed_field_identifier()
	{
		Computed_field::name = NULL;
	}

	Computed_field *getPseudoObject()
	{
		return this;
	}
};

/** functor for ordering Cmiss_set<Computed_field> by field name */
struct Computed_field_compare_name
{
	bool operator() (const Computed_field* field1, const Computed_field* field2) const
	{
		return strcmp(field1->name, field2->name) < 0;
	}
};

typedef Cmiss_set<Computed_field *,Computed_field_compare_name> Cmiss_set_Cmiss_field;

/*
Computed field functions
------------------------
Functions used only internally to computed fields or the region that owns them.
*/

/***************************************************************************//**
 * Make a 'unique' field name by appending a number onto the stem_name until no
 * field of that name is found in the manager.
 *
 * @param first_number  First number to try. If negative (the default argument),
 * start with number of fields in manager + 1.
 * @return  Allocated string containing valid field name not used by any field
 * in manager. Caller must DEALLOCATE. NULL on failure.
 */
char *Computed_field_manager_get_unique_field_name(
	struct MANAGER(Computed_field) *manager, const char *stem_name="temp",
	const char *separator="", int first_number=-1);

/***************************************************************************//**
 * Creates a new computed field with the supplied content and for the region
 * specified by the field_module.
 * 
 * @param field_module  Specifies region to add field to default parameters.
 * @param check_source_field_regions  If set to true, require all source fields
 * to be from the same region as the field_module. If false skip this checks.
 * @param number_of_components  Number of components the new field will have.
 * @param number_of_source_fields  Size of source_fields array.
 * @param source_fields  Array of source fields for the new field.
 * @param number_of_source_values  Size of source_values array.
 * @param source_values  Array of source values for the new field.
 * @param field_core  Field type-specific data and implementation. On success,
 * ownership of field_core object passes to the new field.
 * @return  Newly created field, or NULL on failure.
 */
Computed_field *Computed_field_create_generic(
	Cmiss_field_module *field_module, bool check_source_field_regions,
	int number_of_components,
	int number_of_source_fields, Computed_field **source_fields,
	int number_of_source_values, const double *source_values,
	Computed_field_core *field_core);

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

/***************************************************************************//**
 * Gets the Cmiss_region owning this field manager.
 * 
 * @param manager  Computed field manager.
 * @return  The owning Cmiss_region object.
 */
struct Cmiss_region *Computed_field_manager_get_region(
	struct MANAGER(Computed_field) *manager);

/***************************************************************************//**
 * Gets a reference to the set of fields in the manager.
 * Intended only for immediately iterating over. Do not keep this handle.
 *
 * @param manager  Computed field manager.
 * @return  The set of fields in the manager.
 */
const Cmiss_set_Cmiss_field &Computed_field_manager_get_fields(
	struct MANAGER(Computed_field) *manager);

/***************************************************************************//**
 * Record that field data has changed.
 * Notify clients if not caching changes.
 * 
 * @param field  The field that has changed.
 */
int Computed_field_changed(struct Computed_field *field);

/***************************************************************************//**
 * Record that external global objects this field depends on have change such
 * that this field should evaluate to different values.
 * Notify clients if not caching changes.
 * 
 * @param field  The field whose dependencies have changed.
 */
int Computed_field_dependency_changed(struct Computed_field *field);

/***************************************************************************//**
 * Private function for setting the dependency change flag and adding to the
 * manager's changed object list without sending manager updates.
 * Should only be called by code invoked by MANAGER_UPDATE().
 * @return  1 on success, 0 on failure.
 */
int Computed_field_dependency_change_private(struct Computed_field *field);

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

int Computed_field_is_defined_at_location(struct Computed_field *field,
	Field_location* location);
/*******************************************************************************
LAST MODIFIED : 9 August 2006

DESCRIPTION :
Returns 1 if the all the source fields are defined at the supplied <location>.
==============================================================================*/

/***************************************************************************//**
 * Sets coordinate system of the <field> to that of its first source field.
 */
int Computed_field_set_coordinate_system_from_sources(
	struct Computed_field *field);

int Computed_field_broadcast_field_components(
	struct Cmiss_field_module *field_module,
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

// caller must ensure valid field is passed and field->values is zero
inline int Computed_field_allocate_values_cache(Computed_field *field)
{
	/* get enough space for derivatives in highest dimension element */
	if (!(ALLOCATE(field->values, FE_value, field->number_of_components) &&
		ALLOCATE(field->derivatives, FE_value,
			MAXIMUM_ELEMENT_XI_DIMENSIONS*field->number_of_components)))
	{
		// make sure we have allocated values AND derivatives, or nothing
		DEALLOCATE(field->values);
		return 0;
	}
	for (int i = 0; i < field->number_of_components; i++)
	{
		field->values[i] = 0.0;
	}
	return 1;
}

inline int Computed_field_evaluate_cache_at_location(
	Computed_field *field, Field_location* location)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Calculates the values of <field> at <location>.
Upon successful return the node values of the <field> are stored in its cache.
==============================================================================*/
{
	 int return_code;

	ENTER(Computed_field_evaluate_cache_at_location);
#if ! defined (OPTIMISED)
	if (field && field->core && location)
	{
#endif /* ! defined (OPTIMISED) */
		return_code=1;
		if (!field->values)
		{
			return_code = Computed_field_allocate_values_cache(field);
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
	Field_location* location, const FE_value *values);
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
 * Clears values cache of field but not deeper type-specific cache.
 * Does not recurse through all source fields.
 * Caller's responsibility to ensure field argument is valid.
 */
void Cmiss_field_clear_values_cache_non_recursive(Cmiss_field *field);

/***************************************************************************//**
 * Clears values cache of field but not deeper type-specific cache.
 * Does same for all source fields.
 * Caller's responsibility to ensure field argument is valid.
 */
void Cmiss_field_clear_values_cache_recursive(Cmiss_field *field);

/***************************************************************************//**
 * Variant of Computed_field_clear_cache which does not recurse over source
 * fields.
 * Caller's responsibility to ensure field argument is valid.
 */
void Cmiss_field_clear_cache_non_recursive(Cmiss_field *field);

/***************************************************************************//**
 * Creates field module object needed to create fields in supplied region.
 * Internally: Also used to set new field default arguments prior to create.
 *
 * @param region  The owning region.
 * @return  Field module for the supplied region.
 */
struct Cmiss_field_module *Cmiss_field_module_create(struct Cmiss_region *region);

/***************************************************************************//**
 * Internal, non-accessing version of Cmiss_field_module_get_region.
 *
 * @param field_module  The field module to query.
 * @return  Non-accessed handle to owning region for field_module.
 */
struct Cmiss_region *Cmiss_field_module_get_region_internal(
	struct Cmiss_field_module *field_module);

/***************************************************************************//**
 * Gets the region this field module can create fields for.
 *
 * @param field_module  The field module to query.
 * @return  Accessed handle to owning region for field_module.
 */
struct Cmiss_region *Cmiss_field_module_get_region(
	struct Cmiss_field_module *field_module);

/***************************************************************************//**
 * Sets the name (or name stem if non-unique) of the next field to be created
 * with this field_module.
 *
 * @param field_module  The field module to create fields in.
 * @param field_name  Field name or name stem.
 * @return  Non-zero on success, 0 on failure.
 */
int Cmiss_field_module_set_field_name(struct Cmiss_field_module *field_module,
	const char *field_name);

/***************************************************************************//**
 * Gets a copy of the field name/stem set in this field_module.
 * Up to caller to DEALLOCATE.
 *
 * @param field_module  The field module to create fields in.
 * @return  Allocated copy of the name, or NULL if none.
 */
char *Cmiss_field_module_get_field_name(
	struct Cmiss_field_module *field_module);

/***************************************************************************//**
 * Sets the coordinate system to be used for subsequent fields created with
 * this field module.
 *
 * @param field_module  The field module to create fields in.
 * @param coordinate_system  The coordinate system to set.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_field_module_set_coordinate_system(
	struct Cmiss_field_module *field_module,
	struct Coordinate_system coordinate_system);

/***************************************************************************//**
 * Returns the default coordinate system set in the field_module.
 *
 * @param field_module  The field module to create fields in.
 * @return  Copy of default coordinate system.
 */
struct Coordinate_system Cmiss_field_module_get_coordinate_system(
	struct Cmiss_field_module *field_module);

/***************************************************************************//**
 * Returns true if the default coordinate system has been explicitly set. 
 *
 * @param field_module  The field module to create fields in.
 * @return  1 if coordinate system set, 0 if never set.
 */
int Cmiss_field_module_coordinate_system_is_set(
	struct Cmiss_field_module *field_module);

/***************************************************************************//**
 * Sets the replace_field that will be redefined by the next field
 * created with the field module. Cleared after next field create call.
 * Field name and coordinate system defaults are taken from supplied field.
 *
 * @param field_module  The field module to create fields in.
 * @param replace_field  Existing field to be overwritten on next create. Can
 * be NULL to clear.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_field_module_set_replace_field(
	struct Cmiss_field_module *field_module,
	struct Computed_field *replace_field);

/***************************************************************************//**
 * Gets the replace_field, if any, that will be redefined by the next field
 * created with the field module. Cleared after next field create call.
 *
 * @param field_module  The field module to create fields in.
 * @return  Existing field to be replaced which caller must deaccess, or NULL
 * if none.
 */
struct Computed_field *Cmiss_field_module_get_replace_field(
	struct Cmiss_field_module *field_module);

/***************************************************************************//**
 * Rebuild the cache values of the field. This is only used by 
 * computed_field_image to wipe out the cache when number of components changes.
 *
 * @param field  Field to be updated.
 * @return  1 if successfully rebuild cache_values, or 0
 * if none.
 */
inline int Computed_field_rebuild_cache_values(
	Computed_field *field)
{
	int return_code = 1;

	if (field)
	{
		if (field->values)
		{
			DEALLOCATE(field->values);
			DEALLOCATE(field->derivatives);
		}
	}

	return (return_code);
}

/***************************************************************************//**
 * For each hierarchical field in manager, propagates changes from sub-region
 * fields, if any.
 *
 * @param manager  Parent region field manager.
 * @param message  Child region field manager change message.
 */
void Computed_field_manager_propagate_hierarchical_field_changes(
	MANAGER(Computed_field) *manager, MANAGER_MESSAGE(Computed_field) *message);

/***************************************************************************//**
 * Same as MANAGER_MESSAGE_GET_OBJECT_CHANGE(Computed_field) but also returns
 * change_detail for field, if any.
 *
 * @param message  The field manager change message.
 * @param field  The field to query about.
 * @param change_detail_address  Address to put const change detail in.
 * @return  manager change flags for the object.
 */
int Computed_field_manager_message_get_object_change_and_detail(
	struct MANAGER_MESSAGE(Computed_field) *message, struct Computed_field *field,
	const struct Cmiss_field_change_detail **change_detail_address);

#endif /* !defined (COMPUTED_FIELD_PRIVATE_H) */
