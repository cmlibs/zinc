/*******************************************************************************
FILE : computed_field_private.hpp

LAST MODIFIED : 31 March 2008

DESCRIPTION :
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (COMPUTED_FIELD_PRIVATE_H)
#define COMPUTED_FIELD_PRIVATE_H

/*
Computed field types
--------------------
Types used only internally to computed fields.
*/

#include "opencmiss/zinc/types/fieldparametersid.h"
#include "opencmiss/zinc/field.h"
#include "opencmiss/zinc/fieldcache.h"
#include "general/cmiss_set.hpp"
#include "computed_field/computed_field.h"
#include "computed_field/field_location.hpp"
#include "computed_field/field_cache.hpp"
#include "general/debug.h"
#include "general/manager_private.h"
#include "region/cmiss_region.hpp"

/**
 * Base class of type-specific field change details.
 */
struct cmzn_field_change_detail
{
	virtual ~cmzn_field_change_detail()
	{
	}

	/** override to clear change detail in derived type */
	virtual void clear() = 0;
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
This is the internal core which each type of cmzn_field will implement.
Separating the Computed_field_core and the public cmzn_field enables the
core object to be replaced while maintaining the same wrapper (enabling
changes to an existing cmzn_field heirarchy.  It also enables different
interfaces on the internal core to the public interface (which I am maintaining
in C).
==============================================================================*/
{
protected:
	struct cmzn_field *field;

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
	virtual bool attach_to_field(cmzn_field* parent)
	{
		if (parent)
		{
			field = parent;
			return true;
		}
		return false;
	};

	struct cmzn_field *getField()
	{
		return field;
	}

	/** only call when field member set! */
	inline void beginChange() const;

	/** only call when field member set! */
	inline void endChange() const;

	/** Get source field at index. Not bounds checked!
	 * @param index  Index from 0 to number_of_source_fields - 1
	 * @return  Non-accessed source field */
	inline cmzn_field_id getSourceField(int index) const;

	/**
	 * Override to inherit attributes such as coordinate system from source fields.
	 */
	virtual void inherit_source_field_attributes()
	{
	};

	virtual Computed_field_core *copy() = 0;

	virtual const char *get_type_string() = 0;

	virtual enum cmzn_field_type get_type()
	{
		return CMZN_FIELD_TYPE_INVALID;
	};

	// override for fields requiring specialised value caches
	virtual FieldValueCache *createValueCache(cmzn_fieldcache& /*fieldCache*/);

	virtual int clear_cache() // GRC remove
	{
		return 1;
	};

	/** @return  1 if core has equivalent definition */
	virtual int compare(Computed_field_core* other) = 0;

	/** @return  1 if other core has exactly same definition, e.g. wraps same FE_field
	 * Few fields need to override this. */
	virtual int compareExact(Computed_field_core* other)
	{
		return this->compare(other);
	}

	/** default implementation returns true if all source fields are defined at location.
	 * override if different logic is needed. */
	virtual bool is_defined_at_location(cmzn_fieldcache& cache);

	virtual int has_numerical_components()
	{
		return 1;
	};

	virtual int not_in_use()
	{
		return 1;
	};

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& valueCache) = 0;

	/** Override for real-valued fields, or return zero for non-numeric fields */
	virtual int evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative) = 0;

	/** Evaluate derivatives using finite differences. Only for real-valued fields
	 * Only implemented for element_xi derivatives & locations.
	 * @param cache  Parent cache containing location to evaluate.
	 * @param valueCache  The real field value cache to put values in.
	 * @param fieldDerivative  The field derivative operator. */
	int evaluateDerivativeFiniteDifference(cmzn_fieldcache& cache, RealFieldValueCache& valueCache, const FieldDerivative& fieldDerivative);

	/** Get the highest order of derivatives with non-zero terms for the
	 * derivative tree evaluated for fieldDerivative. For example, returns
	 * zero for a constant field, 1 if the field only has the first derivative
	 * in the tree. Overridden for field operators with potentially lower orders */
	virtual int getDerivativeTreeOrder(const FieldDerivative& fieldDerivative);

	/** Override & return true for field types supporting the sum_square_terms API */
	virtual bool supports_sum_square_terms() const
	{
		return false;
	}

	/** Override for field types whose value is a sum of squares to get the
	 * number of terms summed. Multiply by number of components to get number of values.
	 * Can be expensive. */
	virtual int get_number_of_sum_square_terms(cmzn_fieldcache&) const
	{
		return 0;
	}

	/** Override for field types whose value is a sum of squares to get the array of
	 * individual terms PRIOR to being squared*/
	virtual int evaluate_sum_square_terms(cmzn_fieldcache&, RealFieldValueCache&,
		int /*number_of_values*/, FE_value* /* *values*/)
	{
		return 0;
	}

	virtual enum FieldAssignmentResult assign(cmzn_fieldcache& /*cache*/, MeshLocationFieldValueCache& /*valueCache*/)
	{
		return FIELD_ASSIGNMENT_RESULT_FAIL;
	}

	virtual enum FieldAssignmentResult assign(cmzn_fieldcache& /*cache*/, RealFieldValueCache& /*valueCache*/)
	{
		return FIELD_ASSIGNMENT_RESULT_FAIL;
	}

	virtual enum FieldAssignmentResult assign(cmzn_fieldcache& /*cache*/, StringFieldValueCache& /*valueCache*/)
	{
		return FIELD_ASSIGNMENT_RESULT_FAIL;
	}

	virtual int get_native_discretization_in_element(struct FE_element *element,
		int *number_in_xi);

	virtual int propagate_find_element_xi(cmzn_fieldcache&,
		const FE_value * /*values*/, int /*number_of_values*/,
		struct FE_element ** /*element_address*/, FE_value * /*xi*/,
		cmzn_mesh_id /*mesh*/)
	{
		return 0;
	};

	virtual int list();

	virtual char* get_command_string();

	/**
	 * Default component names are the component number as a string.
	 * Override for fields with stored component names, e.g. finite element.
	 * @param componentNumber  Starts at 1.
	 * @return  Allocated string name. Up to caller to free.
	 */
	virtual char *getComponentName(int componentNumber) const;

	/**
	 * Default implementation is that field component names can't be set.
	 * Override for fields with stored component names, e.g. finite element.
	 * @return CMZN_ERROR_ARGUMENT.
	 */
	virtual int setComponentName(int, const char *)
	{
		return CMZN_ERROR_ARGUMENT;
	}

	virtual int has_multiple_times();

	virtual int get_native_resolution(int *dimension, int **sizes,
		struct cmzn_field **texture_coordinate_field);

	/* override if field type needs to be informed when it has been added to region */
	virtual void fieldAddedToRegion(void)
	{
	}

	/* override if field is a domain */
	virtual int get_domain( struct LIST(cmzn_field) *domain_field_list ) const;

	/**
	 * Propagate changes to the result of this field depending on changes to its
	 * source fields and external/subjects e.g. nodes/elements.
	 * Standard behaviour:
	 * If current change status has MANAGER_CHANGE_FULL_RESULT set,
	 * then return this value immediately.
	 * If any source field has MANAGER_CHANGE_FULL_RESULT set then set this
	 * flag on current field and return it.
	 * If none of the above apply then if any source field has
	 * MANAGER_CHANGE_PARTIAL_RESULT set, then set and return this value.
	 * In all other cases return current change status of field.
	 * Override for customised dependencies on fields, external or sub-objects.
	 * @return  MANAGER_CHANGE_FULL_RESULT, MANAGER_CHANGE_PARTIAL_RESULT or
	 * MANAGER_CHANGE_NONE.
	 */
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

	/** override if field type can determine whether it is a coordinate type field */
	virtual bool isTypeCoordinate() const
	{
		return false;
	}

	/** override if field can is a coordinate type field or stores state
	 * @return CMZN_OK on success, CMZN_ERROR_ARGUMENT otherwise.
	 */
	virtual int setTypeCoordinate(bool value)
	{
		if (value)
			return CMZN_ERROR_ARGUMENT;
		return CMZN_OK;
	}

	/** clones and clears type-specific change detail, if any.
	 * override for classes with type-specific change detail e.g. group fields
	 * @return  change detail prior to clearing, or nullptr if none.
	 */
	virtual cmzn_field_change_detail *extract_change_detail()
	{
		return nullptr;
	}

	virtual const cmzn_field_change_detail *get_change_detail() const
	{
		return nullptr;
	}

	/** override for hierarchical fields to merge changes from sub-region fields */
	virtual void propagate_hierarchical_field_changes(MANAGER_MESSAGE(cmzn_field) *)
	{
	}

	/** override for fields wrapping other objects with coordinate system, e.g. FE_field
	 * to propagate back from field to internal object */
	virtual void propagate_coordinate_system()
	{
	}

	/**
	 * Override for hierarchical fields (e.g. group) which must remove any links
	 * to removed subregion */
	virtual void subregionRemoved(cmzn_region *)
	{
	}

	/** Override if field is not real valued */
	virtual cmzn_field_value_type get_value_type() const
	{
		return CMZN_FIELD_VALUE_TYPE_REAL;
	}

	/* overrides should return true if matching field, false if depends on something else e.g. finite element:
		virtual bool is_purely_function_of_field(cmzn_field *other_field)
		{
			return (this->field == other_field);
		}
	*/
	virtual bool is_purely_function_of_field(cmzn_field *other_field);

protected:

	// call whenever type-specific parameters are changed to notify clients
	inline void setChanged();

}; /* class Computed_field_core */

/** Flag attributes for generic fields */
enum Computed_field_attribute_flags
{
	COMPUTED_FIELD_ATTRIBUTE_IS_MANAGED_BIT = 1
	/*!< If NOT set, destroy field when only access is from region.
	 * @see cmzn_field_set_mnanaged */
};

struct cmzn_field
{
	/* the name/identifier of the cmzn_field */
	const char *name;
	// true if an automatic unique name has been given to field
	// means can rename to make way for another field with a different definition
	bool automaticName;
	/* index of field values in field cache, unique in region */
	int cache_index;
	int number_of_components;

	struct Coordinate_system coordinate_system;

	Computed_field_core* core;

	/* for all Computed_field_types calculated from others */

	/* array of computed fields this field is calculated from */
	int number_of_source_fields;
	struct cmzn_field **source_fields;
	/* array of constant values this field is calculated from */
	int number_of_source_values;
	FE_value *source_values;

	cmzn_fieldparameters *fieldparameters;

	/* after clearing in create, following to be modified only by manager */
	/* Keep a reference to the objects manager */
	struct MANAGER(cmzn_field) *manager;
	int manager_change_status;

	/** bit flag attributes. @see Computed_field_attribute_flags. */
	int attribute_flags;

	int access_count;

protected:

	cmzn_field();
	~cmzn_field();

public:

	/** @param nameIn  Optional name of new field or nullptr for none */
	static cmzn_field *create(const char *nameIn=nullptr);

	inline cmzn_field *access()
	{
		++(this->access_count);
		return this;
	}

	static void deaccess(cmzn_field*& field);

	static void reaccess(cmzn_field*& field, cmzn_field *newField)
	{
		if (newField)
			newField->access();
		if (field)
			cmzn_field::deaccess(field);
		field = newField;
	}

	/** call whenever field values have been assigned to. Clears all cached data for
	 * this field and any field that depends on it.
	 */
	void clearCaches();

	int getNumberOfComponents() const
	{
		return this->number_of_components;
	}

	cmzn_field_value_type getValueType() const
	{
		return this->core->get_value_type();
	}

	/** @return  true if this field and other field have matching basic definition,
	 * value type, number of components. Does not compare name, coordinate system etc. */
	bool compareBasicDefinition(const cmzn_field& otherField) const
	{
		return (this->number_of_components == otherField.number_of_components)
			&& (this->getValueType() == otherField.getValueType());
	}

	/** @return  true if this field and other field have matching full definition,
	 * excluding name, coordinate system */
	bool compareFullDefinition(const cmzn_field& otherField) const;

	/**
	 * Copy the functional definition of source field to this field.
	 * Fails if source depends on this field or is from another region.
	 * Fails if changing number of components while this field is in use.
	 * Note this copies the definition only; finite element fields defined on nodes
	 * and elements are not affected.
	 * Does not copy name of field.
	 *
	 * @source  Field to copy functional definition from.
	 * @return  Result OK on success, ERROR_ARGUMENT if fails because source
	 * depends on this field or is from another region, ERROR_INCOMPATIBLE_DATA if
	 * attempting to change number of components while field is in use, otherwise
	 * any other error.
	 */
	int copyDefinition(const cmzn_field& source);

	inline FieldValueCache *getValueCache(cmzn_fieldcache& fieldCache)
	{
		FieldValueCache *valueCache = fieldCache.getValueCache(this->cache_index);
		if (!valueCache)
		{
			valueCache = this->core->createValueCache(fieldCache);
			fieldCache.setValueCache(this->cache_index, valueCache);
		}
		return valueCache;
	}

	template <class FieldValueCacheClass>
	inline FieldAssignmentResult assign(cmzn_fieldcache& cache, FieldValueCacheClass& valueCache)
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

	inline const FieldValueCache *evaluate(cmzn_fieldcache& cache);

	/** Note: caller is responsible for ensuring field is real-valued and fieldDerivative is for this region */
	inline const DerivativeValueCache *evaluateDerivative(cmzn_fieldcache& cache, const FieldDerivative& fieldDerivative);

	/** Evaluate all derivatives from fieldDerivative down to value.
	 * Note: caller is responsible for ensuring field is real-valued and fieldDerivative is for this region */
	inline const RealFieldValueCache *evaluateDerivativeTree(cmzn_fieldcache& cache, const FieldDerivative& fieldDerivative);

	/** Apply field needs to know if a field depends on an argument.
	 * @return  true if this field is a function of an argument field directly
	 * or indirectly, otherwise false.
	 */
	bool dependsOnArgument() const
	{
		if (0 == this->number_of_source_fields)
		{
			// using fact that Argument fields have no source fields to minimise virtual function call:
			return this->core->get_type() == CMZN_FIELD_TYPE_ARGUMENT_REAL;
		}
		for (int i = 0; i < this->number_of_source_fields; ++i)
		{
			if (this->source_fields[i]->dependsOnArgument())
				return true;
		}
		return false;
	}

	/** @return  true if this field equals otherField or otherField is a source
	 * field directly or indirectly, otherwise false.
	 */
	bool dependsOnField(cmzn_field *otherField) const
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

	/** Get the highest order of derivatives with non-zero terms for the
	 * derivative tree evaluated for fieldDerivative. For example, returns
	 * zero for a constant field, 1 if the field only has the first derivative
	 * in the tree. */
	int getDerivativeTreeOrder(const FieldDerivative& fieldDerivative)
	{
		return this->core->getDerivativeTreeOrder(fieldDerivative);
	}

	/* @return  Accessed handle to existing or new field parameters for field, or nullptr if wrong type of field */
	cmzn_fieldparameters *getFieldparameters();

	/** Only to be called by ~cmzn_fieldparameters */
	void clearFieldparameters()
	{
		this->fieldparameters = nullptr;
	}

	int isNumerical()
	{
		return core->has_numerical_components();
	}

	int get_number_of_sum_square_terms(cmzn_fieldcache& cache)
	{
		return core->get_number_of_sum_square_terms(cache);
	}

	int evaluate_sum_square_terms(cmzn_fieldcache& cache, int number_of_values, FE_value *values)
	{
		if ((0 <= number_of_values) && values)
		{
			RealFieldValueCache *valueCache = RealFieldValueCache::cast(getValueCache(cache));
			return core->evaluate_sum_square_terms(cache, *valueCache, number_of_values, values);
		}
		return 0;
	}

	/**
	 * Record that field definition has changed.
	 * Notify clients if not caching changes.
	 */
	inline void setChanged();

	/**
	 * Private function for setting the change status flag and adding
	 * to the manager's changed object list without sending manager updates.
	 * Should only be called by Computed_field_core-defined check_dependency methods.
	 * @param change  A change status flag, one of OBJECT_NOT_IDENTIFIER, DEPENDENCY
	 * or PARTIAL.
	 */
	inline void setChangedPrivate(MANAGER_CHANGE(cmzn_field) change);

	/**
	 * Record that field result has changed due to related objects changing,
	 * e.g. FE_field on nodes, elements.
	 * Notify clients if not caching changes.
	 */
	inline void setChangedRelated();

	const Coordinate_system& getCoordinateSystem()
	{
		return this->coordinate_system;
	}

	/** Checks non-numerical fields are only able to have coordinate system type NOT_APPLICABLE.
	 * @param notifyChange  Set to false to avoid change messages being sent.
	 * @return  Result OK, or ERROR_ARGUMENT if invalid coordinate system for field. */
	int setCoordinateSystem(const Coordinate_system& coordinateSystemIn, bool notifyChange=true);

	FE_value getCoordinateSystemFocus() const;

	int setCoordinateSystemFocus(FE_value focus);

	cmzn_field_coordinate_system_type getCoordinateSystemType() const;

	/** @return  Result OK on success, or ERROR_ARGUMENT if coordinate system type
	 * not valid for field e.g. non-numeric field may only have NOT_APPLICABLE set. */
	int setCoordinateSystemType(cmzn_field_coordinate_system_type coordinateSystemType);

	/** @param index  Index from 0 to number_of_source_fields - 1
	 * @param notifyChange  Set to false to avoid change messages being sent */
	void copyCoordinateSystemFromSourceField(int index, bool notifyChange=true)
	{
		if ((0 <= index) && (index < this->number_of_source_fields))
		{
			this->setCoordinateSystem(this->source_fields[index]->getCoordinateSystem(), notifyChange);
		}
	}

	const char *getName() const
	{
		return this->name;
	}

	bool hasAutomaticName() const
	{
		return this->automaticName;
	}

	/** Set new name for field which must be unique in its manager.
	 * Field must be managed to call this.
	 * @return  Result OK on success, otherwise an error code. */
	int setName(const char *nameIn);

	/** Set a standard automatic name for the field e.g. temp### that is
	 * different to its current name and unique in its manager. Mark field as
	 * having an automatic name, meaning it is permitted to be renamed further
	 * if name is needed by another different field.
	 * Field must be managed to call this.
	 * @param fieldManager  Field manager to use if field not yet managed.
	 * @return  Result OK on success, otherwise an error code. */
	int setNameAutomatic(MANAGER(cmzn_field) *fieldManager=nullptr);

	/** Set new name for field that is different to its current name and
	 * unique in its manager, name concatenates part1 and part2 and if needed a
	 * number string starting at 1.
	 * @param part1  First part of name.
	 * @param part2  Second part of name, separator to number.
	 * @param startNumber  Negative to start from number of fields in manager,
	 * 0 to try name without number first, then start from 1, or any positive
	 * number to try first.
	 * @return  Result OK on success, otherwise an error code. */
	int setNameUnique(const char *part1, const char *part2="", int startNumber=-1);

	/**
	 * Record that external global objects this field depends on have change such
	 * that this field should evaluate to different values.
	 * Notify clients if not caching changes.
	 */
	inline void dependencyChanged();

	/** Get source field at index. Not bounds checked!
	 * @param index  Index from 0 to number_of_source_fields - 1
	 * @return  Non-accessed source field */
	cmzn_field *getSourceField(int index) const
	{
		return this->source_fields[index];
	}

	/**
	 * Set the source field at the index in the field. Capable of adding one
	 * source to the end of the list.
	 * To be used with care by certain field types only!
	 * Caller must ensure this is a source field permitted to be changed, usually
	 * an optional source field added after the compulsory one.
	 * Not to be exposed directly in the public API.
	 *
	 * @param index  Index of source field, starting at 0. Must be either
	 * a valid source field index or equal to the current number of source fields
	 * to add a new source field.
	 * @param sourceField  The source field to set, or nullptr to clear. If
	 * clearing and the index is not the last, the following fields are moved
	 * down in the array.
	 * @param notifyChange  Optionally set to false to not record change to field
	 * nor notify clients. Use to reduce messages when making another change
	 * immediately afterwards.
	 * @return  CMZN_OK or other status code on failure.
	 */
	int setSourceField(int index, cmzn_field *sourceField, bool notifyChange=true);

	/** Return owning field manager. Must check not nullptr before use as
	 * can be cleared during clean-up. */
	struct MANAGER(cmzn_field) *getManager() const
	{
		return this->manager;
	}

	/** @return  Non-accessed owning region */
	inline cmzn_region *getRegion() const;

}; /* struct cmzn_field */

inline void Computed_field_core::beginChange() const
{
	if (this->field->manager)
		MANAGER_BEGIN_CACHE(cmzn_field)(this->field->manager);
}

inline void Computed_field_core::endChange() const
{
	if (this->field->manager)
		MANAGER_END_CACHE(cmzn_field)(this->field->manager);
}

inline cmzn_field_id Computed_field_core::getSourceField(int index) const
{
	return field->source_fields[index];
}

/* Only to be used from FIND_BY_IDENTIFIER_IN_INDEXED_LIST_STL function
 * Creates a pseudo object with name identifier suitable for finding
 * objects by identifier with cmzn_set.
 */
class cmzn_field_identifier : private cmzn_field
{
public:
	cmzn_field_identifier(const char *name) :
		cmzn_field()
	{
		this->name = name;
	}

	~cmzn_field_identifier()
	{
		this->name = nullptr;
	}

	cmzn_field *getPseudoObject()
	{
		return this;
	}
};

/** functor for ordering cmzn_set<cmzn_field> by field name */
struct Computed_field_compare_name
{
	bool operator() (const cmzn_field* field1, const cmzn_field* field2) const
	{
		return strcmp(field1->name, field2->name) < 0;
	}
};

typedef cmzn_set<cmzn_field *,Computed_field_compare_name> cmzn_set_cmzn_field;

FULL_DECLARE_MANAGER_TYPE_WITH_OWNER(cmzn_field, struct cmzn_region, struct cmzn_field_change_detail *);

PROTOTYPE_MANAGER_OWNER_FUNCTIONS(cmzn_field, struct cmzn_region);

inline const FieldValueCache *cmzn_field::evaluate(cmzn_fieldcache& cache)
{
	FieldValueCache *valueCache = this->getValueCache(cache);
	if ((valueCache->evaluationCounter < cache.getLocationCounter())
		|| cache.hasRegionModifications())
	{
		if (this->core->evaluate(cache, *valueCache))
			valueCache->evaluationCounter = cache.getLocationCounter();
		else
			return nullptr;
	}
	return valueCache;
}

/** Caller is responsible for ensuring field is real-valued */
inline const DerivativeValueCache *cmzn_field::evaluateDerivative(cmzn_fieldcache& cache, const FieldDerivative& fieldDerivative)
{
	RealFieldValueCache *realValueCache = RealFieldValueCache::cast(this->getValueCache(cache));
	DerivativeValueCache *derivativeValueCache = realValueCache->getOrCreateDerivativeValueCache(fieldDerivative, cache.get_location());
	if ((derivativeValueCache->evaluationCounter < cache.getLocationCounter())
		|| cache.hasRegionModifications())
	{
		if (this->core->evaluateDerivative(cache, *realValueCache, fieldDerivative))
			derivativeValueCache->evaluationCounter = cache.getLocationCounter();
		else
			return nullptr;
	}
	return derivativeValueCache;
}

/** Caller is responsible for ensuring field is real-valued */
inline const RealFieldValueCache *cmzn_field::evaluateDerivativeTree(cmzn_fieldcache& cache, const FieldDerivative& fieldDerivative)
{
	const FieldDerivative *thisFieldDerivative = &fieldDerivative;
	do
	{
		if (!this->evaluateDerivative(cache, *thisFieldDerivative))
			return nullptr;
		thisFieldDerivative = thisFieldDerivative->getLowerDerivative();
	} while (thisFieldDerivative);
	return RealFieldValueCache::cast(this->evaluate(cache));
}

inline cmzn_region *cmzn_field::getRegion() const
{
	return MANAGER_GET_OWNER(cmzn_field)(this->manager);
}

struct cmzn_fielditerator : public cmzn_set_cmzn_field::ext_iterator
{
private:
	cmzn_fielditerator(cmzn_set_cmzn_field *container);
	cmzn_fielditerator(const cmzn_fielditerator&);
	~cmzn_fielditerator();

public:

		static cmzn_fielditerator *create(cmzn_set_cmzn_field *container)
		{
			return static_cast<cmzn_fielditerator *>(cmzn_set_cmzn_field::ext_iterator::create(container));
		}

		cmzn_fielditerator *access()
		{
			return static_cast<cmzn_fielditerator *>(this->cmzn_set_cmzn_field::ext_iterator::access());
		}

		static int deaccess(cmzn_fielditerator* &iterator)
		{
			cmzn_set_cmzn_field::ext_iterator* baseIterator = static_cast<cmzn_set_cmzn_field::ext_iterator*>(iterator);
			iterator = 0;
			return cmzn_set_cmzn_field::ext_iterator::deaccess(baseIterator);
		}

};

/*
Computed field functions
------------------------
Functions used only internally to computed fields or the region that owns them.
*/

/**
 * Make a name by concatenating part1 and part2. While there exists a field with
 * that name, append with a number until not found, and return the string.
 *
 * @param startNumber  First number to try. If negative (the default argument),
 * start with number of fields in manager + 1. If 0 try without a number before
 * starting at 1.
 * @return  Allocated string containing valid field name not used by any field
 * in manager. Caller must DEALLOCATE. NULL on failure.
 */
char *Computed_field_manager_get_unique_field_name(
	struct MANAGER(cmzn_field) *manager, const char *part1="temp",
	const char *part2="", int startNumber =-1);

/**
 * Create an iterator for the objects in the manager.
 */
cmzn_fielditerator_id Computed_field_manager_create_iterator(
	struct MANAGER(cmzn_field) *manager);

/**
 * Create an iterator for the objects in the list.
 */
cmzn_fielditerator_id Computed_field_list_create_iterator(
	struct LIST(cmzn_field) *list);

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
int Computed_field_add_to_manager_private(struct cmzn_field *field,
	struct MANAGER(cmzn_field) *manager);

/**
 * Creates a new computed field with the supplied content and for the region
 * specified by the field module.
 *
 * @param fieldmodule  Specifies region to add field to default parameters.
 * @param check_source_field_regions  If set to true, require all source fields
 * to be from the same region as the field module. If false skip this checks.
 * @param number_of_components  Number of components the new field will have.
 * @param number_of_source_fields  Size of source_fields array.
 * @param source_fields  Array of source fields for the new field.
 * @param number_of_source_values  Size of source_values array.
 * @param source_values  Array of source values for the new field.
 * @param field_core  Field type-specific data and implementation. On success,
 * ownership of field_core object passes to the new field. On failure this is
 * destroyed.
 * @param name  Optional name for the new field. If not supplied a unique
 * automatic name temp# is created for it. If supplied and name is in use by
 * another field, creation fails.
 * @return  Newly created field, or NULL on failure.
 */
cmzn_field *Computed_field_create_generic(
	cmzn_fieldmodule *fieldmodule, bool check_source_field_regions,
	int number_of_components,
	int number_of_source_fields, cmzn_field **source_fields,
	int number_of_source_values, const double *source_values,
	Computed_field_core *field_core, const char *name=nullptr);

/***************************************************************************//**
 * Sets the cmzn_region object which will own this manager.
 * Private! Only to be called only from cmzn_region.
 *
 * @param manager  Computed field manager.
 * @return  The owning cmzn_region object.
 */
int Computed_field_manager_set_region(struct MANAGER(cmzn_field) *manager,
	struct cmzn_region *region);

/***************************************************************************//**
 * Gets the cmzn_region owning this field manager.
 *
 * @param manager  Computed field manager.
 * @return  The owning cmzn_region object.
 */
struct cmzn_region *Computed_field_manager_get_region(
	struct MANAGER(cmzn_field) *manager);

/***************************************************************************//**
 * Gets a reference to the set of fields in the manager.
 * Intended only for immediately iterating over. Do not keep this handle.
 *
 * @param manager  Computed field manager.
 * @return  The set of fields in the manager.
 */
const cmzn_set_cmzn_field &Computed_field_manager_get_fields(
	struct MANAGER(cmzn_field) *manager);

inline void Computed_field_core::setChanged()
{
	this->field->setChanged();
}

inline void cmzn_field::setChanged()
{
	if ((this->manager) && (this->manager->owner))
	{
		this->manager->owner->setFieldModify();
		MANAGED_OBJECT_CHANGE(cmzn_field)(this, MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(cmzn_field));
	}
}

inline void cmzn_field::setChangedPrivate(MANAGER_CHANGE(cmzn_field) change)
{
	if ((this->manager) && (this->manager->owner))
	{
		if (this->manager_change_status == MANAGER_CHANGE_NONE(cmzn_field))
		{
			ADD_OBJECT_TO_LIST(cmzn_field)(this, this->manager->changed_object_list);
		}
		this->manager->owner->setFieldModify();
		this->manager_change_status |= change;
	}
}

inline void cmzn_field::setChangedRelated()
{
	if ((this->manager) && (this->manager->owner))
	{
		this->manager->owner->setFieldModify();
		MANAGED_OBJECT_CHANGE(cmzn_field)(this, MANAGER_CHANGE_PARTIAL_RESULT(cmzn_field));
	}
}

/**
 * Record that field data has changed.
 * Notify clients if not caching changes.
 *
 * @param field  The field that has changed.
 */
inline int Computed_field_changed(struct cmzn_field *field)
{
	if (field)
	{
		field->setChanged();
		return 1;
	}
	display_message(ERROR_MESSAGE, "Computed_field_changed.  Invalid argument(s)");
	return 0;
}

/**
 * Record that external global objects this field depends on have change such
 * that this field should evaluate to different values.
 * Notify clients if not caching changes.
 *
 * @param field  The field whose dependencies have changed.
 */
inline void cmzn_field::dependencyChanged()
{
	MANAGED_OBJECT_CHANGE(cmzn_field)(this, MANAGER_CHANGE_FULL_RESULT(cmzn_field));
}

/**
 * Takes two ACCESSED fields <field_one> and <field_two> and compares their number
 * of components.  If they are equal then the function just returns.  If one
 * is a scalar field and the other is not then the scalar is wrapped in a component
 * field repeating the scalar to match the non scalar number of components.  The
 * wrapped field will be DEACCESSED by the function but now will be accessed by
 * the wrapping field and an ACCESSED pointer to the wrapper field is returned
 * replacing the wrapped field.
 * If the two fields are non scalar and have different numbers of components then
 * nothing is done, although other shape broadcast operations could be proposed
 * for matrix operations.
 */
int Computed_field_broadcast_field_components(cmzn_fieldmodule *fieldmodule,
	cmzn_field **field_one, cmzn_field **field_two);

/**
 * For each hierarchical field in manager, propagates changes from sub-region
 * fields, if any.
 *
 * @param manager  Parent region field manager.
 * @param message  Child region field manager change message.
 */
void Computed_field_manager_propagate_hierarchical_field_changes(
	MANAGER(cmzn_field) *manager, MANAGER_MESSAGE(cmzn_field) *message);

/**
 * For each hierarchical field in manager, ensure any subfields in the removed
 * subregion are removed. Currently only applicable to group fields.
 *
 * @param manager  Parent region field manager.
 * @param subregion  The subregion being removed.
 */
void Computed_field_manager_subregion_removed(MANAGER(cmzn_field) *manager,
	cmzn_region *subregion);

/***************************************************************************//**
 * Same as MANAGER_MESSAGE_GET_OBJECT_CHANGE(cmzn_field) but also returns
 * change_detail for field, if any.
 *
 * @param message  The field manager change message.
 * @param field  The field to query about.
 * @param change_detail_address  Address to put const change detail in.
 * @return  manager change flags for the object.
 */
int Computed_field_manager_message_get_object_change_and_detail(
	struct MANAGER_MESSAGE(cmzn_field) *message, struct cmzn_field *field,
	const struct cmzn_field_change_detail **change_detail_address);

#endif /* !defined (COMPUTED_FIELD_PRIVATE_H) */
