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

/*
Computed field types
--------------------
Types used only internally to computed fields.
*/

#include "general/cmiss_set.hpp"
#include "computed_field/field_location.hpp"
#include "computed_field/field_cache.hpp"
#include "computed_field/computed_field.h"
#include "general/debug.h"
#include "region/cmiss_region.h"

/**
 * Argument to field modifier functions supplying region, default name,
 * coordinate system etc.
 * Previously had other parameters, but now just wraps the field_module.
 */
class Computed_field_modify_data
{
private:
	cmzn_field_module *field_module;

public:
	Computed_field_modify_data(
		struct cmzn_field_module *field_module) :
		field_module(field_module)
	{
	}

	~Computed_field_modify_data()
	{
	}

	cmzn_field_module *get_field_module()
	{
		return field_module;
	}

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
			cmzn_field_set_managed(new_field, true);
			DEACCESS(Computed_field)(&new_field);
			return 1;
		}
		return 0;
	}

	/**
	 * Get existing field to replace, if any.
	 */
	Computed_field *get_field();

	cmzn_region *get_region();

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
struct cmzn_field_change_detail
{
	virtual ~cmzn_field_change_detail()
	{
	}
};

/***************************************************************************//**
 * Return value for field assignment functions. Partial success is needed for
 * EDIT_MASK field which can successfully set some of the values passed.
 */
enum FieldAssignmentResult
{
	FIELD_ASSIGNMENT_RESULT_FAIL = 0,
	FIELD_ASSIGNMENT_RESULT_PARTIAL_VALUES_SET = 1,
	FIELD_ASSIGNMENT_RESULT_ALL_VALUES_SET = 2,
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

	inline cmzn_field_id getSourceField(int index);

	/**
	 * Override to inherit attributes such as coordinate system from source fields.
	 */
	virtual void inherit_source_field_attributes()
	{
	};

	virtual Computed_field_core *copy() = 0;

	virtual const char *get_type_string() = 0;

	// override for fields requiring specialised value caches
	virtual FieldValueCache *createValueCache(cmzn_field_cache& /*parentCache*/);

	virtual int clear_cache() // GRC remove
	{
		return 1;
	};

	virtual int compare(Computed_field_core* other) = 0;

	/** default implementation returns true if all source fields are defined at location.
	 * override if different logic is needed. */
	virtual bool is_defined_at_location(cmzn_field_cache& cache);

	virtual int has_numerical_components()
	{
		return 1;
	};

	virtual int not_in_use()
	{
		return 1;
	};

	virtual int evaluate(cmzn_field_cache& cache, FieldValueCache& valueCache) = 0;

	/** Override & return 1 for field types supporting the sum_square_terms API */
	virtual int supports_sum_square_terms() const
	{
		return 0;
	}

	/** Override for field types whose value is a sum of squares to get the
	 * number of terms summed. Multiply by number of components to get number of values.
	 * Can be expensive. */
	virtual int get_number_of_sum_square_terms(cmzn_field_cache&) const
	{
		return 0;
	}

	/** Override for field types whose value is a sum of squares to get the array of
	 * individual terms PRIOR to being squared*/
	virtual int evaluate_sum_square_terms(cmzn_field_cache&, RealFieldValueCache&,
		int /*number_of_values*/, FE_value* /* *values*/)
	{
		return 0;
	}

	virtual enum FieldAssignmentResult assign(cmzn_field_cache& /*cache*/, MeshLocationFieldValueCache& /*valueCache*/)
	{
		return FIELD_ASSIGNMENT_RESULT_FAIL;
	}

	virtual enum FieldAssignmentResult assign(cmzn_field_cache& /*cache*/, RealFieldValueCache& /*valueCache*/)
	{
		return FIELD_ASSIGNMENT_RESULT_FAIL;
	}

	virtual enum FieldAssignmentResult assign(cmzn_field_cache& /*cache*/, StringFieldValueCache& /*valueCache*/)
	{
		return FIELD_ASSIGNMENT_RESULT_FAIL;
	}

	virtual int get_native_discretization_in_element(struct FE_element *element,
		int *number_in_xi);

	virtual int propagate_find_element_xi(cmzn_field_cache&,
		const FE_value * /*values*/, int /*number_of_values*/,
		struct FE_element ** /*element_address*/, FE_value * /*xi*/,
		cmzn_mesh_id /*mesh*/)
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

	/** called by cmzn_field_set_name. Override to rename wrapped objects e.g. FE_field */
	virtual int set_name(const char *name)
	{
		USE_PARAMETER(name);
		return 1;
	}

	// override if attribute supported by field type
	// used only for attributes not stored in the generic field object
	virtual int get_attribute_integer(enum cmzn_field_attribute attribute) const
	{
		USE_PARAMETER(attribute);
		return 0;
	}

	// override if attribute can be set for field type
	// used only for attributes not stored in the generic field object
	virtual int set_attribute_integer(enum cmzn_field_attribute attribute, int value)
	{
		USE_PARAMETER(attribute);
		USE_PARAMETER(value);
		return 0;
	}

	/** clones and clears type-specific change detail, if any.
	 * override for classes with type-specific change detail e.g. group fields
	 * @return  change detail prior to clearing, or NULL if none.
	 */
	virtual cmzn_field_change_detail *extract_change_detail()
	{
		return NULL;
	}

	virtual const cmzn_field_change_detail *get_change_detail() const
	{
		return NULL;
	}

	/** override for hierarchical fields to merge changes from sub-region fields */
	virtual void propagate_hierarchical_field_changes(MANAGER_MESSAGE(Computed_field) *message)
	{
		USE_PARAMETER(message);
	}

	/** override for fields wrapping other objects with coordinate system, e.g. FE_field */
	virtual void propagate_coordinate_system()
	{
	}

	/** Override if field is not real valued */
	virtual cmzn_field_value_type get_value_type() const
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
	/* index of field values in field cache, unique in region */
	int cache_index;
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

	/** call whenever field values have been assigned to. Clears all cached data for
	 * this field and any field that depends on it.
	 */
	void clearCaches();

	inline FieldValueCache *getValueCache(cmzn_field_cache& cache)
	{
		FieldValueCache *valueCache = cache.getValueCache(cache_index);
		if (!valueCache)
		{
			valueCache = core->createValueCache(cache);
			cache.setValueCache(cache_index, valueCache);
		}
		return valueCache;
	}

	template <class FieldValueCacheClass>
	inline FieldAssignmentResult assign(cmzn_field_cache& cache, FieldValueCacheClass& valueCache)
	{
		FieldAssignmentResult result = core->assign(cache, valueCache);
		if ((result == FIELD_ASSIGNMENT_RESULT_ALL_VALUES_SET) &&
			cache.assignInCacheOnly())
		{
			valueCache.evaluationCounter = cache.getLocationCounter();
		}
		else
		{
			// ensure supplied values are never used if failed or partially set
			// or have minor numerical differences when all values set
			valueCache.resetEvaluationCounter();
		}
		return result;
	}

	inline FieldValueCache *evaluate(cmzn_field_cache& cache)
	{
		FieldValueCache *valueCache = getValueCache(cache);
		// GRC: move derivatives to a separate value cache in future
		if ((valueCache->evaluationCounter < cache.getLocationCounter()) ||
			(cache.getRequestedDerivatives() && (!valueCache->hasDerivatives())))
		{
			if (core->evaluate(cache, *valueCache))
				valueCache->evaluationCounter = cache.getLocationCounter();
			else
				valueCache = 0;
		}
		return valueCache;
	}

	/** @param numberOfDerivatives  positive number of xi dimension of element location */
	inline RealFieldValueCache *evaluateWithDerivatives(cmzn_field_cache& cache, int numberOfDerivatives)
	{
		int requestedDerivatives = cache.getRequestedDerivatives();
		cache.setRequestedDerivatives(numberOfDerivatives);
		RealFieldValueCache *valueCache = RealFieldValueCache::cast(evaluate(cache));
		cache.setRequestedDerivatives(requestedDerivatives);
		if (valueCache && valueCache->derivatives_valid)
			return valueCache;
		return 0;
	}

	inline FieldValueCache *evaluateNoDerivatives(cmzn_field_cache& cache)
	{
		int requestedDerivatives = cache.getRequestedDerivatives();
		cache.setRequestedDerivatives(0);
		FieldValueCache *valueCache = evaluate(cache);
		cache.setRequestedDerivatives(requestedDerivatives);
		return valueCache;
	}

	/** @return  true if this field equals otherField or otherField is a source
	 * field directly or indirectly, otherwise false.
	 */
	bool dependsOnField(cmzn_field *otherField)
	{
		if (this == otherField)
			return true;
		for (int i = 0; i < this->number_of_source_fields; ++i)
		{
			if (this->source_fields[i]->dependsOnField(otherField))
				return true;
		}
		return false;
	}

	int isNumerical()
	{
		return core->has_numerical_components();
	}

	int get_number_of_sum_square_terms(cmzn_field_cache& cache)
	{
		return core->get_number_of_sum_square_terms(cache);
	}

	int evaluate_sum_square_terms(cmzn_field_cache& cache, int number_of_values, FE_value *values)
	{
		if ((0 <= number_of_values) && values)
		{
			RealFieldValueCache *valueCache = RealFieldValueCache::cast(getValueCache(cache));
			return core->evaluate_sum_square_terms(cache, *valueCache, number_of_values, values);
		}
		return 0;
	}


}; /* struct Computed_field */

inline cmzn_field_id Computed_field_core::getSourceField(int index)
{
	return field->source_fields[index];
}

/* Only to be used from FIND_BY_IDENTIFIER_IN_INDEXED_LIST_STL function
 * Creates a pseudo object with name identifier suitable for finding
 * objects by identifier with cmzn_set.
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

/** functor for ordering cmzn_set<Computed_field> by field name */
struct Computed_field_compare_name
{
	bool operator() (const Computed_field* field1, const Computed_field* field2) const
	{
		return strcmp(field1->name, field2->name) < 0;
	}
};

typedef cmzn_set<Computed_field *,Computed_field_compare_name> cmzn_set_cmzn_field;

struct cmzn_field_iterator : public cmzn_set_cmzn_field::ext_iterator
{
	cmzn_field_iterator(cmzn_set_cmzn_field *container) :
		cmzn_set_cmzn_field::ext_iterator(container)
	{
	}
};

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
 * Create an iterator for the objects in the manager.
 */
cmzn_field_iterator_id Computed_field_manager_create_iterator(
	struct MANAGER(Computed_field) *manager);

/***************************************************************************//**
 * Return index of field in field cache.
 */
int cmzn_field_get_cache_index_private(cmzn_field_id field);

/***************************************************************************//**
 * Set index of field in field cache. Private; use only from cmzn_region.
 */
int cmzn_field_set_cache_index_private(cmzn_field_id field, int cache_index);

/***************************************************************************//**
 * Private function for adding field to manager; use only from cmzn_region.
 * Ensures field name is unique and informs field core that field is managed.
 */
int Computed_field_add_to_manager_private(struct Computed_field *field,
	struct MANAGER(Computed_field) *manager);

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
	cmzn_field_module *field_module, bool check_source_field_regions,
	int number_of_components,
	int number_of_source_fields, Computed_field **source_fields,
	int number_of_source_values, const double *source_values,
	Computed_field_core *field_core);

/***************************************************************************//**
 * Sets the cmzn_region object which will own this manager.
 * Private! Only to be called only from cmzn_region.
 *
 * @param manager  Computed field manager.
 * @return  The owning cmzn_region object.
 */
int Computed_field_manager_set_region(struct MANAGER(Computed_field) *manager,
	struct cmzn_region *region);

/***************************************************************************//**
 * Gets the cmzn_region owning this field manager.
 *
 * @param manager  Computed field manager.
 * @return  The owning cmzn_region object.
 */
struct cmzn_region *Computed_field_manager_get_region(
	struct MANAGER(Computed_field) *manager);

/***************************************************************************//**
 * Gets a reference to the set of fields in the manager.
 * Intended only for immediately iterating over. Do not keep this handle.
 *
 * @param manager  Computed field manager.
 * @return  The set of fields in the manager.
 */
const cmzn_set_cmzn_field &Computed_field_manager_get_fields(
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

/***************************************************************************//**
 * Sets coordinate system of the <field> to that of its first source field.
 */
int Computed_field_set_coordinate_system_from_sources(
	struct Computed_field *field);

int Computed_field_broadcast_field_components(
	struct cmzn_field_module *field_module,
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

/***************************************************************************//**
 * Creates field module object needed to create fields in supplied region.
 * Internally: Also used to set new field default arguments prior to create.
 *
 * @param region  The owning region.
 * @return  Field module for the supplied region.
 */
struct cmzn_field_module *cmzn_field_module_create(struct cmzn_region *region);

/***************************************************************************//**
 * Internal, non-accessing version of cmzn_field_module_get_region.
 *
 * @param field_module  The field module to query.
 * @return  Non-accessed handle to owning region for field_module.
 */
struct cmzn_region *cmzn_field_module_get_region_internal(
	struct cmzn_field_module *field_module);

/***************************************************************************//**
 * Sets the name (or name stem if non-unique) of the next field to be created
 * with this field_module.
 *
 * @param field_module  The field module to create fields in.
 * @param field_name  Field name or name stem.
 * @return  Non-zero on success, 0 on failure.
 */
int cmzn_field_module_set_field_name(struct cmzn_field_module *field_module,
	const char *field_name);

/***************************************************************************//**
 * Gets a copy of the field name/stem set in this field_module.
 * Up to caller to DEALLOCATE.
 *
 * @param field_module  The field module to create fields in.
 * @return  Allocated copy of the name, or NULL if none.
 */
char *cmzn_field_module_get_field_name(
	struct cmzn_field_module *field_module);

/***************************************************************************//**
 * Sets the coordinate system to be used for subsequent fields created with
 * this field module.
 *
 * @param field_module  The field module to create fields in.
 * @param coordinate_system  The coordinate system to set.
 * @return  1 on success, 0 on failure.
 */
int cmzn_field_module_set_coordinate_system(
	struct cmzn_field_module *field_module,
	struct Coordinate_system coordinate_system);

/***************************************************************************//**
 * Returns the default coordinate system set in the field_module.
 *
 * @param field_module  The field module to create fields in.
 * @return  Copy of default coordinate system.
 */
struct Coordinate_system cmzn_field_module_get_coordinate_system(
	struct cmzn_field_module *field_module);

/***************************************************************************//**
 * Returns true if the default coordinate system has been explicitly set.
 *
 * @param field_module  The field module to create fields in.
 * @return  1 if coordinate system set, 0 if never set.
 */
int cmzn_field_module_coordinate_system_is_set(
	struct cmzn_field_module *field_module);

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
int cmzn_field_module_set_replace_field(
	struct cmzn_field_module *field_module,
	struct Computed_field *replace_field);

/***************************************************************************//**
 * Gets the replace_field, if any, that will be redefined by the next field
 * created with the field module. Cleared after next field create call.
 *
 * @param field_module  The field module to create fields in.
 * @return  Existing field to be replaced which caller must deaccess, or NULL
 * if none.
 */
struct Computed_field *cmzn_field_module_get_replace_field(
	struct cmzn_field_module *field_module);

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
	const struct cmzn_field_change_detail **change_detail_address);

#endif /* !defined (COMPUTED_FIELD_PRIVATE_H) */
