/**
 * FILE : computed_field_finite_element.cpp
 *
 * Evaluates stored/interpolated finite element fields.
 */
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <cassert>
#include <cmath>
#include <map>
#include "cmlibs/zinc/fieldmodule.h"
#include "cmlibs/zinc/fieldfiniteelement.h"
#include "cmlibs/zinc/mesh.h"
#include "cmlibs/zinc/status.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_coordinate.h"
#include "computed_field/computed_field_find_xi.h"
#include "computed_field/computed_field_find_xi_private.hpp"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_set.h"
#include "computed_field/fieldparametersprivate.hpp"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_discretization.h"
#include "finite_element/finite_element_field_evaluation.hpp"
#include "finite_element/finite_element_mesh.hpp"
#include "finite_element/finite_element_mesh_field_ranges.hpp"
#include "finite_element/finite_element_private.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_region_private.h"
#include "finite_element/finite_element_time.h"
#include "general/debug.h"
#include "general/enumerator_private.hpp"
#include "general/mystring.h"
#include "general/message.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/field_module.hpp"
#include "general/enumerator_conversion.hpp"
#include "mesh/mesh.hpp"
#include "mesh/mesh_group.hpp"
#include "mesh/cmiss_node_private.hpp"

#if defined (DEBUG_CODE)
/* SAB This field is useful for debugging when things don't clean up properly
	but has to be used carefully, especially as operations such as caching
	accesses the node or element being considered so you get effects like
	the first point evaluated in an element having a count one less than
	all the others */
#define COMPUTED_FIELD_ACCESS_COUNT
#endif /* defined (DEBUG_CODE) */

/*
Module types
------------
*/

namespace {

typedef std::map<cmzn_element *, FE_element_field_evaluation *> FE_element_field_evaluation_map;

const int maxCachedTimes = 3;

class TimeElementFieldEvaluationMap
{
	FE_field *feField;
	FE_value time;
	FE_element_field_evaluation_map elementFieldEvaluationMap;
	FE_element_field_evaluation *elementFieldEvaluation;  // evalution object for latest element

public:

	TimeElementFieldEvaluationMap(FE_field *feFieldIn, FE_value timeIn) :
		feField(feFieldIn),
		time(timeIn),
		elementFieldEvaluation(nullptr)
	{
	}

	~TimeElementFieldEvaluationMap()
	{
		this->clear();
	}

	void clear()
	{
		for (FE_element_field_evaluation_map::iterator iter = this->elementFieldEvaluationMap.begin();
			iter != this->elementFieldEvaluationMap.end(); ++iter)
		{
			FE_element_field_evaluation::deaccess(iter->second);
		}
		this->elementFieldEvaluationMap.clear();
		// Following was a pointer to an object just destroyed, so must clear
		this->elementFieldEvaluation = nullptr;
	}

	inline FE_value getTime() const
	{
		return this->time;
	}

	/** Get or create the FE_element_field_evaluation for evaluating field in
	 * element at time, inherited from optional topLevelElement. Uses existing
	 * values in cache if nothing changed. Caller must have ensured time match
	 * with this object and that arguments are valid.
	 * @return  Non-accessed FE_element_field_evaluation* or nullptr if failed.
	 */
	FE_element_field_evaluation *getElementFieldEvaluation(
		cmzn_element *element, cmzn_element *topLevelElement)
	{
		// can't trust cached element field values if between manager begin/end change
		// and this field has been modified.
		const bool fieldChanged = FE_field_has_cached_changes(this->feField);
		// ensure we have FE_element_field_evaluation calculated for element
		// with derivatives_calculated if requested
		if ((!this->elementFieldEvaluation) || fieldChanged ||
			(!this->elementFieldEvaluation->isForElement(element, topLevelElement)))
		{
			bool needUpdate = false;
			bool addToCache = false;
			FE_element_field_evaluation_map::iterator iter = this->elementFieldEvaluationMap.find(element);
			if (iter == this->elementFieldEvaluationMap.end())
			{
				this->elementFieldEvaluation = FE_element_field_evaluation::create();
				addToCache = true;
				needUpdate = true;
			}
			else
			{
				this->elementFieldEvaluation = iter->second;
				if (fieldChanged || (!this->elementFieldEvaluation->isForElement(element, topLevelElement)))
				{
					this->elementFieldEvaluation->clear();
					needUpdate = true;
				}
			}
			if ((this->elementFieldEvaluation) && needUpdate)
			{
				if (this->elementFieldEvaluation->calculate_values(this->feField, element, this->time, topLevelElement))
				{
					if (addToCache)
					{
						// Set a cache size limit, clearing is a simple way to only cache for recent elements
						if (1000 < this->elementFieldEvaluationMap.size())
						{
							FE_element_field_evaluation* tmpElementFieldEvaluation = this->elementFieldEvaluation;
							this->clear();
							this->elementFieldEvaluation = tmpElementFieldEvaluation;
						}
						this->elementFieldEvaluationMap[element] = this->elementFieldEvaluation;
					}
				}
				else
				{
					FE_element_field_evaluation::deaccess(this->elementFieldEvaluation);
					if (!addToCache)
					{
						this->elementFieldEvaluationMap.erase(iter);
					}
				}
			}
		}
		return this->elementFieldEvaluation;
	}

};


/** reference counted cache of FE_element_field_evaluation for recent elements to share between field caches */
class FE_element_field_evaluation_cache
{
	FE_field *feField;
	std::vector<TimeElementFieldEvaluationMap*> timeElementFieldEvaluationMaps;
	int access_count;

	FE_element_field_evaluation_cache(FE_field *feFieldIn) :
		feField(feFieldIn),
		access_count(1)
	{
	}

	~FE_element_field_evaluation_cache()
	{
		this->clear();
	}

public:
	static FE_element_field_evaluation_cache *create(FE_field *feFieldIn)
	{
		return new FE_element_field_evaluation_cache(feFieldIn);
	}

	FE_element_field_evaluation_cache *access()
	{
		++this->access_count;
		return this;
	}

	static int deaccess(FE_element_field_evaluation_cache* &evaluation_cache)
	{
		if (!evaluation_cache)
			return CMZN_ERROR_ARGUMENT;
		--(evaluation_cache->access_count);
		if (evaluation_cache->access_count <= 0)
			delete evaluation_cache;
		evaluation_cache = 0;
		return CMZN_OK;
	}

	void clear()
	{
		const int timeCount = static_cast<int>(this->timeElementFieldEvaluationMaps.size());
		for (int i = 0; i < timeCount; ++i)
		{
			delete this->timeElementFieldEvaluationMaps[i];
		}
		this->timeElementFieldEvaluationMaps.clear();
	}

	TimeElementFieldEvaluationMap *getTimeElementFieldEvaluationMap(FE_value time)
	{
		TimeElementFieldEvaluationMap *timeElementFieldEvaluationMap;
		const int timeCount = static_cast<int>(this->timeElementFieldEvaluationMaps.size());
		for (int i = 0; i < timeCount; ++i)
		{
			timeElementFieldEvaluationMap = this->timeElementFieldEvaluationMaps[i];
			if (timeElementFieldEvaluationMap->getTime() == time)
			{
				if (i != 0)
				{
					// move to the front
					for (int j = i; j > 0; --j)
					{
						this->timeElementFieldEvaluationMaps[j] = this->timeElementFieldEvaluationMaps[j - 1];
					}
					this->timeElementFieldEvaluationMaps[0] = timeElementFieldEvaluationMap;
				}
				return timeElementFieldEvaluationMap;
			}
		}
		if (timeCount == maxCachedTimes)
		{
			delete this->timeElementFieldEvaluationMaps[timeCount - 1];
			this->timeElementFieldEvaluationMaps.resize(timeCount - 1);
		}
		timeElementFieldEvaluationMap = new TimeElementFieldEvaluationMap(this->feField, time);
		this->timeElementFieldEvaluationMaps.insert(this->timeElementFieldEvaluationMaps.begin(), timeElementFieldEvaluationMap);
		return timeElementFieldEvaluationMap;
	}

	/**
	 * Establishes the FE_element_field_evaluation necessary for evaluating field in
	 * element at time, inherited from optional top_level_element. Uses existing
	 * values in cache if nothing changed.
	 * @param  Time to get parameters at. If FE_field is not time-dependent, this
	 * is ignored and time 0.0 is used.
	 * @return  Non-accessed FE_element_field_evaluation* or 0 if failed.
	 */
	inline FE_element_field_evaluation *getElementFieldEvaluation(cmzn_element *element, FE_value time,
		cmzn_element *topLevelElement)
	{
		const FE_value useTime = this->feField->isTimeDependent() ? time : 0.0;
		TimeElementFieldEvaluationMap *timeElementFieldEvaluationMap = this->getTimeElementFieldEvaluationMap(useTime);
		return timeElementFieldEvaluationMap->getElementFieldEvaluation(element, topLevelElement);
	}

};

class MultiTypeRealFieldValueCache : public RealFieldValueCache
{
public:
	// caches for evaluating different value types
	std::vector<double> double_values;
	std::vector<float> float_values;
	std::vector<int> int_values;
	std::vector<short> short_values;

	MultiTypeRealFieldValueCache(int componentCount) :
		RealFieldValueCache(componentCount),
		double_values(componentCount),
		float_values(componentCount),
		int_values(componentCount),
		short_values(componentCount)
	{
	}

	virtual ~MultiTypeRealFieldValueCache()
	{
	}

	static MultiTypeRealFieldValueCache* cast(FieldValueCache* valueCache)
	{
		return FIELD_VALUE_CACHE_CAST<MultiTypeRealFieldValueCache*>(valueCache);
	}

	static MultiTypeRealFieldValueCache& cast(FieldValueCache& valueCache)
	{
		return FIELD_VALUE_CACHE_CAST<MultiTypeRealFieldValueCache&>(valueCache);
	}

};

class FiniteElementRealFieldValueCache : public MultiTypeRealFieldValueCache
{
public:
	FE_element_field_evaluation_cache *element_field_evaluation_cache;

	/** @param parentValueCache  Optional parentValueCache to get element_field_evaluation_cache from */
	FiniteElementRealFieldValueCache(FE_field *feField, FiniteElementRealFieldValueCache *parentValueCache) :
		MultiTypeRealFieldValueCache(feField->getNumberOfComponents()),
		element_field_evaluation_cache((parentValueCache) ? parentValueCache->element_field_evaluation_cache->access()
			: FE_element_field_evaluation_cache::create(feField))
	{
	}

	virtual ~FiniteElementRealFieldValueCache()
	{
		FE_element_field_evaluation_cache::deaccess(this->element_field_evaluation_cache);
	}

	virtual void clear()
	{
		this->element_field_evaluation_cache->clear();
		RealFieldValueCache::clear();
	}

	static FiniteElementRealFieldValueCache* cast(FieldValueCache* valueCache)
	{
		return FIELD_VALUE_CACHE_CAST<FiniteElementRealFieldValueCache*>(valueCache);
	}

	static FiniteElementRealFieldValueCache& cast(FieldValueCache& valueCache)
	{
		return FIELD_VALUE_CACHE_CAST<FiniteElementRealFieldValueCache&>(valueCache);
	}
};

// FE_element_field_evaluation are needed to evaluate indexed string FE_fields on elements
class FiniteElementStringFieldValueCache : public StringFieldValueCache
{
public:
	FE_element_field_evaluation_cache *element_field_evaluation_cache;

	/** @param parentValueCache  Optional parentValueCache to get element_field_evaluation_cache from */
	FiniteElementStringFieldValueCache(FE_field *feField, FiniteElementStringFieldValueCache *parentValueCache) :
		StringFieldValueCache(),
		element_field_evaluation_cache((parentValueCache) ? parentValueCache->element_field_evaluation_cache->access()
			: FE_element_field_evaluation_cache::create(feField))
	{
	}

	virtual ~FiniteElementStringFieldValueCache()
	{
		FE_element_field_evaluation_cache::deaccess(this->element_field_evaluation_cache);
	}

	virtual void clear()
	{
		this->element_field_evaluation_cache->clear();
		StringFieldValueCache::clear();
	}

	static FiniteElementStringFieldValueCache* cast(FieldValueCache* valueCache)
	{
		return FIELD_VALUE_CACHE_CAST<FiniteElementStringFieldValueCache*>(valueCache);
	}

	static FiniteElementStringFieldValueCache& cast(FieldValueCache& valueCache)
	{
		return FIELD_VALUE_CACHE_CAST<FiniteElementStringFieldValueCache&>(valueCache);
	}

};

const char computed_field_finite_element_type_string[] = "finite_element";

class Computed_field_finite_element : public Computed_field_core
{
public:
	FE_field *fe_field;
	enum cmzn_field_type type;

public:
	Computed_field_finite_element(FE_field *fe_field) :
		Computed_field_core(),
		fe_field(ACCESS(FE_field)(fe_field))
	{
		fe_field->addWrapper();
		type = CMZN_FIELD_TYPE_INVALID;
	};

	virtual ~Computed_field_finite_element();

	virtual void inherit_source_field_attributes()
	{
		if (this->field)
		{
			this->field->setCoordinateSystem(fe_field->getCoordinateSystem(), /*notifyChange*/false);
		}
	}

	/** @param componentNumber  Component number >= 0 or negative to get all components
	  * @param versionNumber  Version number >= 0 */
	int getNodeParameters(cmzn_fieldcache& cache, int componentNumber,
		cmzn_node_value_label valueLabel, int versionNumber,
		int valuesCount, double *valuesOut);

	/** @param componentNumber  Component number >= 0 or negative to set all components
	  * @param versionNumber  Version number >= 0 */
	int setNodeParameters(cmzn_fieldcache& cache, int componentNumber,
		cmzn_node_value_label valueLabel, int versionNumber,
		int valuesCount, const double *valuesIn);

	/** @return  True if any parameters stored at location in cache. */
	bool hasParametersAtLocation(cmzn_fieldcache& cache);

	virtual bool is_purely_function_of_field(cmzn_field *other_field)
	{
		return (this->field == other_field);
	}

	/** @return  True if this finite element field result changed. */
	virtual bool isResultChanged()
	{
		int change = 0;
		CHANGE_LOG_QUERY(FE_field)(this->fe_field->get_FE_region()->fe_field_changes, this->fe_field, &change);
		// changed if any flags other than identifier changed
		if (change &= (~CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_field)))
		{
			return true;
		}
		return false;
	}

private:
	Computed_field_core *copy();

	const char *get_type_string()
	{
		return(computed_field_finite_element_type_string);
	}

	virtual enum cmzn_field_type get_type()
	{
		return type;
	}

	virtual int compare(Computed_field_core* other_field);

	virtual int compareExact(Computed_field_core *other_core);

	virtual FieldValueCache *createValueCache(cmzn_fieldcache& fieldCache)
	{
		const Value_type value_type = this->fe_field->getValueType();
		FieldValueCache *parentValueCache = (fieldCache.getParentCache()) ? this->field->getValueCache(*fieldCache.getParentCache()) : 0;
		switch (value_type)
		{
			case ELEMENT_XI_VALUE:
				return new MeshLocationFieldValueCache();
			case STRING_VALUE:
			case URL_VALUE:
				return new FiniteElementStringFieldValueCache(this->fe_field, static_cast<FiniteElementStringFieldValueCache *>(parentValueCache));
			default:
				break;
		}
		// Future: have common finite element field cache in some circumstances
		// note they must not be shared with time lookup fields as only a single time is cached
		// and performance will be poor.
		return new FiniteElementRealFieldValueCache(this->fe_field, static_cast<FiniteElementRealFieldValueCache *>(parentValueCache));
	}

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	virtual int evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative);

	virtual int getDerivativeTreeOrder(const FieldDerivative& fieldDerivative)
	{
		// assumes derivatives are evaluated in elements
		// Future:  tabulate highest order basis in use esp. if all per-element constant
		if ((this->fe_field->getValueType() == FE_VALUE_VALUE) || (this->fe_field->getValueType() == SHORT_VALUE))
		{
			if (fieldDerivative.getFieldparameters() && (fieldDerivative.getFieldparameters()->getField() == this->field))
				return fieldDerivative.getMeshOrder() + 1;  // since linear with own parameters
			return fieldDerivative.getMeshOrder();
		}
		return 0;  // all other value types are either stored in nodes only, or have zero mesh derivatives (e.g. integer)
	}

	int list();

	char* get_command_string();

	virtual char *getComponentName(int componentNumber) const
	{
		return get_FE_field_component_name(this->fe_field, componentNumber - 1);
	}

	virtual int setComponentName(int componentNumber, const char *name)
	{
		if (this->fe_field->setComponentName(componentNumber - 1, name))
		{
			return CMZN_OK;
		}
		return CMZN_ERROR_ARGUMENT;
	}

	virtual bool is_defined_at_location(cmzn_fieldcache& cache);

	int has_multiple_times();

	int has_numerical_components();

	int not_in_use();

	virtual enum FieldAssignmentResult assign(cmzn_fieldcache& cache, RealFieldValueCache& valueCache);

	virtual enum FieldAssignmentResult assign(cmzn_fieldcache& cache, MeshLocationFieldValueCache& valueCache);

	virtual enum FieldAssignmentResult assign(cmzn_fieldcache& cache, StringFieldValueCache& valueCache);

	/* Propagate the coordinate system back from field to FE_field.
	 * Called only by field API affecting coordinate system. */
	virtual void propagate_coordinate_system()
	{
		this->fe_field->setCoordinateSystem(field->coordinate_system);
	}

	int get_native_discretization_in_element(
		struct FE_element *element,int *number_in_xi);

	virtual int check_dependency()
	{
		if (field)
		{
			// propagate changes from FE_field
			CHANGE_LOG(FE_field) *fe_field_changes = FE_region_get_FE_field_changes(this->fe_field->get_FE_region());
			int change = 0;
			CHANGE_LOG_QUERY(FE_field)(fe_field_changes, this->fe_field, &change);
			if (change & CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_field))
				field->setChangedPrivate(MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Computed_field));
			else if (change & CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_field))
				field->setChangedPrivate(MANAGER_CHANGE_PARTIAL_RESULT(Computed_field));
			return field->manager_change_status;
		}
		return MANAGER_CHANGE_NONE(Computed_field);
	}

	virtual bool is_non_linear() const
	{
		return fe_field->usesNonlinearBasis();
	}

	virtual int set_name(const char *name)
	{
		return FE_region_set_FE_field_name(this->fe_field->get_FE_region(), fe_field, name);
	};

	virtual bool isTypeCoordinate() const
	{
		return (this->fe_field->get_CM_field_type() == CM_COORDINATE_FIELD);
	}

	virtual int setTypeCoordinate(bool value)
	{
		// Note that CM_field_type is an enum with 3 states
		// so can't be COORDINATE and ANATOMICAL at the same time.
		CM_field_type cm_field_type = this->fe_field->get_CM_field_type();
		if (value)
		{
			if (cm_field_type != CM_COORDINATE_FIELD)
				this->fe_field->set_CM_field_type(CM_COORDINATE_FIELD);
		}
		else
		{
			if (cm_field_type == CM_COORDINATE_FIELD)
				this->fe_field->set_CM_field_type(CM_GENERAL_FIELD);
		}
		return CMZN_OK;
	}

	virtual cmzn_field_value_type get_value_type() const
	{
		const Value_type fe_value_type = this->fe_field->getValueType();
		cmzn_field_value_type value_type = CMZN_FIELD_VALUE_TYPE_INVALID;
		switch (fe_value_type)
		{
			case ELEMENT_XI_VALUE:
				value_type = CMZN_FIELD_VALUE_TYPE_MESH_LOCATION;
				break;
			case STRING_VALUE:
			case URL_VALUE:
				value_type = CMZN_FIELD_VALUE_TYPE_STRING;
				break;
			case DOUBLE_VALUE:
			case FE_VALUE_VALUE:
			case FLT_VALUE:
			case INT_VALUE:
			case SHORT_VALUE:
				value_type = CMZN_FIELD_VALUE_TYPE_REAL;
				break;
			default:
				break;
		}
		return value_type;
	}

};

inline Computed_field_finite_element *cmzn_field_finite_element_core_cast(
	cmzn_field_finite_element_id finite_element_field)
{
	return (static_cast<Computed_field_finite_element*>(
		reinterpret_cast<Computed_field*>(finite_element_field)->core));
}

Computed_field_finite_element::~Computed_field_finite_element()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{

	ENTER(Computed_field_finite_element::~Computed_field_finite_element);
	if (field)
	{
		if (fe_field)
		{
			/* The following logic only removes the FE_field when it is not in
			 * use, which should be ensured in normal use by
			 * MANAGED_OBJECT_NOT_IN_USE(Computed_field).
			 * There are complications due to the merge process used when reading
			 * fields from file which appears to leave some FE_fields temporarily
			 * not in their owning FE_region when this is called.
			 * Also gfx define field commands create and destroy temporary
			 * finite_element field wrappers & we don't want to clean up the
			 * FE_field until the last wrapper is destroyed.
			 */
			const int number_of_remaining_wrappers = fe_field->removeWrapper();
			if (0 == number_of_remaining_wrappers)
			{
				struct FE_region *fe_region = this->fe_field->get_FE_region();
				if (fe_region && FE_region_contains_FE_field(fe_region, fe_field) &&
					(!FE_region_is_FE_field_in_use(fe_region, fe_field)))
				{
					if (!FE_region_remove_FE_field(fe_region, fe_field))
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_finite_element::~Computed_field_finite_element.  "
							"Destroying computed field before FE_field.");
					}
				}
			}
			DEACCESS(FE_field)(&(fe_field));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element::~Computed_field_finite_element.  "
			"Invalid arguments.");
	}
	LEAVE;

} /* Computed_field_finite_element::~Computed_field_finite_element */

Computed_field_core* Computed_field_finite_element::copy()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	Computed_field_finite_element* core =
		new Computed_field_finite_element(fe_field);

	return (core);
} /* Computed_field_finite_element::copy */

// compare basic definition required to be able to merge fields
int Computed_field_finite_element::compare(Computed_field_core *other_core)
{
	Computed_field_finite_element* other = dynamic_cast<Computed_field_finite_element*>(other_core);
	if ((other) && this->fe_field->compareBasicDefinition(other->fe_field))
	{
		return 1;
	}
	return 0;
}

// @return  1 if internal FE_field is the same object, otherwise 0.
int Computed_field_finite_element::compareExact(Computed_field_core *other_core)
{
	Computed_field_finite_element* other = dynamic_cast<Computed_field_finite_element*>(other_core);
	if ((other) && (this->fe_field == other->fe_field))
	{
		return 1;
	}
	return 0;
}

bool Computed_field_finite_element::is_defined_at_location(cmzn_fieldcache& cache)
{
	const Field_location_element_xi *element_xi_location;
	const Field_location_node *node_location;
	if ((element_xi_location = cache.get_location_element_xi()))
	{
		return FE_field_is_defined_in_element(fe_field, element_xi_location->get_element());
	}
	else if ((node_location = cache.get_location_node()))
	{
		// true and able to be evaluated only if all components have a VALUE parameter
		const FE_node_field *node_field = node_location->get_node()->getNodeField(this->fe_field);
		if (node_field)
		{
			return node_field->getValueMinimumVersionsCount(CMZN_NODE_VALUE_LABEL_VALUE) > 0;
		}
	}
	return false;
}

int Computed_field_finite_element::has_multiple_times()
{
	return this->fe_field->checkTimeDependent();
}

int Computed_field_finite_element::has_numerical_components()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_finite_element::has_numerical_components);
	if (field)
	{
		return_code=Value_type_is_numeric_simple(
			this->fe_field->getValueType());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element::has_numerical_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_finite_element::has_numerical_components */

int Computed_field_finite_element::not_in_use()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
The FE_field must also not be in use.
==============================================================================*/
{
	int return_code;
	struct FE_region *fe_region;

	ENTER(Computed_field_finite_element::not_in_use);
	if (field)
	{
		/* check the fe_field can be destroyed */
		fe_region = this->fe_field->get_FE_region();
		if (fe_region)
		{
			/* ask owning FE_region if fe_field is used in nodes and elements */
			if (FE_region_is_FE_field_in_use(fe_region, fe_field))
			{
				return_code = 0;
			}
			else
			{
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_finite_element::not_in_use.  Missing FE_region");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element::not_in_use.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_finite_element::not_in_use */

int Computed_field_finite_element::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	int return_code = 0;
	const Value_type value_type = this->fe_field->getValueType();
	const Field_location_element_xi* meshLocation;
	const Field_location_node *nodeLocation;
	switch (value_type)
	{
		case ELEMENT_XI_VALUE:
		{
			if ((nodeLocation = cache.get_location_node()))
			{
				MeshLocationFieldValueCache& meshLocationValueCache = MeshLocationFieldValueCache::cast(inValueCache);
				// can only have 1 component; can only be evaluated at node so assume node location
				cmzn_element_id element = 0;
				FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
				return_code = get_FE_nodal_element_xi_value(nodeLocation->get_node(), fe_field, /*component_number*/0,
					&element, xi) && (element);
				if (return_code)
				{
					meshLocationValueCache.setMeshLocation(element, xi);
				}
			}
		} break;
		case STRING_VALUE:
		case URL_VALUE:
		{
			return_code = 1;
			FiniteElementStringFieldValueCache& feStringValueCache = FiniteElementStringFieldValueCache::cast(inValueCache);
			if (feStringValueCache.stringValue)
			{
				DEALLOCATE(feStringValueCache.stringValue);
			}
			if ((nodeLocation = cache.get_location_node()))
			{
				// can only have 1 component
				feStringValueCache.stringValue = get_FE_nodal_value_as_string(nodeLocation->get_node(),
					fe_field, /*componentNumber*/0, CMZN_NODE_VALUE_LABEL_VALUE, /*version*/0,
					/*ignored*/cache.getTime());
				if (!feStringValueCache.stringValue)
				{
					return_code = 0;
				}
			}
			else if ((meshLocation = cache.get_location_element_xi()))
			{
				cmzn_element_id element = meshLocation->get_element();
				cmzn_element_id topLevelElement = meshLocation->get_top_level_element();
				FE_value time = meshLocation->get_time();
				const FE_value* xi = meshLocation->get_xi();
				FE_element_field_evaluation *element_field_evaluation =
					feStringValueCache.element_field_evaluation_cache->getElementFieldEvaluation(
						element, time, topLevelElement);
				if (element_field_evaluation)
				{
					return_code = element_field_evaluation->evaluate_as_string(
						/*component_number*/-1, xi, meshLocation->get_basis_function_evaluation(),
						&(feStringValueCache.stringValue));
				}
				else
				{
					return_code = 0;
				}
			}
		} break;
		default:
		{
			FiniteElementRealFieldValueCache& feValueCache = FiniteElementRealFieldValueCache::cast(inValueCache);
			if ((meshLocation = cache.get_location_element_xi()))
			{
				cmzn_element_id element = meshLocation->get_element();
				cmzn_element_id topLevelElement = meshLocation->get_top_level_element();
				const FE_value time = meshLocation->get_time();
				const FE_value* xi = meshLocation->get_xi();
				FE_element_field_evaluation *element_field_evaluation =
					feValueCache.element_field_evaluation_cache->getElementFieldEvaluation(
						element, time, topLevelElement);
				if (element_field_evaluation)
				{
					/* component number -1 = calculate all components */
					switch (value_type)
					{
						case FE_VALUE_VALUE:
						case SHORT_VALUE:
						{
							return_code = element_field_evaluation->evaluate_real(
								/*component_number*/-1, xi, meshLocation->get_basis_function_evaluation(),
								/*mesh_derivative_order*/0, /*parameter_derivative_order*/0, feValueCache.values);
						} break;
						case INT_VALUE:
						{
							int *int_values = feValueCache.int_values.data();
							return_code = element_field_evaluation->evaluate_int(
								/*component_number*/-1, xi, int_values);
							const int componentCount = field->number_of_components;
							for (int c = 0; c < componentCount; ++c)
							{
								feValueCache.values[c] = static_cast<FE_value>(int_values[c]);
							}
						} break;
						default:
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_finite_element::evaluate.  "
								"Unsupported value type %s in finite_element field",
								Value_type_string(value_type));
							return_code = 0;
						} break;
					}
				}
				else
				{
					return_code = 0;
				}
			}
			else if ((nodeLocation = cache.get_location_node()))
			{
				int result = CMZN_ERROR_GENERAL;
				const int componentCount = field->number_of_components;
				cmzn_node *node = nodeLocation->get_node();
				const FE_value time = nodeLocation->get_time();
				switch (value_type)
				{
				case DOUBLE_VALUE:
				{
					double *double_values = feValueCache.double_values.data();
					result = get_FE_nodal_double_value(node, this->fe_field, /*componentNumber=ALL*/-1,
						CMZN_NODE_VALUE_LABEL_VALUE, /*version*/0, time, double_values);
					for (int c = 0; c < componentCount; ++c)
					{
						feValueCache.values[c] = static_cast<FE_value>(double_values[c]);
					}
				} break;
				case FE_VALUE_VALUE:
				{
					result = get_FE_nodal_FE_value_value(node, fe_field, /*componentNumber=ALL*/-1,
						CMZN_NODE_VALUE_LABEL_VALUE, /*version_number*/0, time, feValueCache.values);
				} break;
				case FLT_VALUE:
				{
					float *float_values = feValueCache.float_values.data();
					result = get_FE_nodal_float_value(node, this->fe_field, /*componentNumber=ALL*/-1,
						CMZN_NODE_VALUE_LABEL_VALUE, /*version*/0, time, float_values);
					for (int c = 0; c < componentCount; ++c)
					{
						feValueCache.values[c] = static_cast<FE_value>(float_values[c]);
					}
				} break;
				case INT_VALUE:
				{
					int *int_values = feValueCache.int_values.data();
					result = get_FE_nodal_int_value(node, this->fe_field, /*componentNumber=ALL*/-1,
						CMZN_NODE_VALUE_LABEL_VALUE, /*version*/0, time, int_values);
					for (int c = 0; c < componentCount; ++c)
					{
						feValueCache.values[c] = static_cast<FE_value>(int_values[c]);
					}
				} break;
				case SHORT_VALUE:
				{
					short *short_values = feValueCache.short_values.data();
					result = get_FE_nodal_short_value(node, this->fe_field, /*componentNumber=ALL*/-1,
						CMZN_NODE_VALUE_LABEL_VALUE, /*version*/0, time, short_values);
					for (int c = 0; c < componentCount; ++c)
					{
						feValueCache.values[c] = static_cast<FE_value>(short_values[c]);
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Computed_field_finite_element::evaluate.  "
						"Unsupported value type %s in finite_element field",
						Value_type_string(value_type));
				} break;
				}
				return_code = (result == CMZN_OK) ? 1 : 0;
			}
			else
			{
				return_code = 0;
			}
		} break;
	}
	return return_code;
}

int Computed_field_finite_element::evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative)
{
	const Field_location_element_xi* meshLocation = cache.get_location_element_xi();
	const Value_type valueType = this->fe_field->getValueType();
	if (meshLocation)
	{
		FiniteElementRealFieldValueCache& feValueCache = FiniteElementRealFieldValueCache::cast(inValueCache);
		DerivativeValueCache *derivativeCache = feValueCache.getDerivativeValueCache(fieldDerivative);
		cmzn_element *element = meshLocation->get_element();
		FE_value coordinateTransformation[MAXIMUM_ELEMENT_XI_DIMENSIONS*MAXIMUM_ELEMENT_XI_DIMENSIONS];
		// derivatives w.r.t. parent element/s are evaluated on separate derivative cache; see below
		DerivativeValueCache *evaluateDerivativeCache = derivativeCache;
		cmzn_element *evaluateElement = element;
		const FE_value *evaluateXi = meshLocation->get_xi();
		FE_value parentXi[MAXIMUM_ELEMENT_XI_DIMENSIONS];  // used only if converting xi to parent

		if ((fieldDerivative.getFieldparameters()) ||
			((fieldDerivative.getMesh()) && (fieldDerivative.getMesh() != element->getMesh())))
		{
			// evaluate on field element and convert mesh derivatives as needed later
			evaluateElement = this->fe_field->getOrInheritOnElement(element,
				/*inherit_face_number*/-1, meshLocation->get_top_level_element(), coordinateTransformation);
			if (!evaluateElement)
			{
				return 0;  // field is not defined
			}
			if (evaluateElement == element)
			{
				if ((fieldDerivative.getMesh()) && (fieldDerivative.getMesh() != element->getMesh()))
				{
					return 0;  // child mesh derivatives are not valid, and conversion to parent mesh derivative is not implemented
				}
			}
			else
			{
				// convert xi to higher dimension evaluateElement
				const int elementDimension = element->getDimension();
				const int evaluateElementDimension = evaluateElement->getDimension();
				for (int j = 0; j < evaluateElementDimension; ++j)
				{
					const FE_value *row = coordinateTransformation + j * (elementDimension + 1);
					FE_value sum = row[0];
					for (int i = 0; i < elementDimension; ++i)
					{
						sum += row[i + 1] * evaluateXi[i];
					}
					parentXi[j] = sum;
				}
				evaluateXi = parentXi;
				// if only parameter derivatives, exploit current implementation which gives face element
				// same parameters as field element it inherits from
				// if field derivative mesh is same as evaluate element, already have correct derivativeCache
				if (fieldDerivative.getMesh() && (fieldDerivative.getMesh() != evaluateElement->getMesh()))
				{
					if (fieldDerivative.getMesh() != element->getMesh())
					{
						return 0;   // child mesh derivatives are not valid, and conversion to in-between mesh derivative is not implemented
					}
					// need to evaluate in derivative cache for higher dimensional xi
					const FieldDerivative *inheritFieldDerivative = (fieldDerivative.getFieldparameters()) ?
						fieldDerivative.getFieldparameters()->getFieldDerivativeMixed(evaluateElement->getMesh(),
							fieldDerivative.getMeshOrder(), fieldDerivative.getParameterOrder()) :
						evaluateElement->getMesh()->getFieldDerivative(fieldDerivative.getMeshOrder());
					if (!inheritFieldDerivative)
					{
						return 0;
					}
					Field_location_element_xi tmpMeshLocation;
					tmpMeshLocation.set_element_xi(evaluateElement, evaluateXi);
					evaluateDerivativeCache = feValueCache.getOrCreateDerivativeValueCache(*inheritFieldDerivative, tmpMeshLocation);
					if (!evaluateDerivativeCache)
					{
						return 0;
					}
				}
			}
			// current finite element field is a linear function of parameters, for this field
			if ((fieldDerivative.getFieldparameters()) &&
				((fieldDerivative.getFieldparameters()->getField() != this->field)
					|| (fieldDerivative.getParameterOrder() > 1)))
			{
				derivativeCache->zeroValues();
				return 1;
			}
		}
		FE_element_field_evaluation *element_field_evaluation =
			feValueCache.element_field_evaluation_cache->getElementFieldEvaluation(
				evaluateElement, meshLocation->get_time(),
				meshLocation->get_top_level_element());
		if (!element_field_evaluation)
		{
			return 0;  // not defined, or some error
		}
		if ((valueType == FE_VALUE_VALUE) || (valueType == SHORT_VALUE))
		{
			FE_value *evaluateDerivatives = evaluateDerivativeCache->values;
			if (!element_field_evaluation->evaluate_real(
				/*component_number*/-1, evaluateXi, meshLocation->get_basis_function_evaluation(),
				fieldDerivative.getMeshOrder(), fieldDerivative.getParameterOrder(), evaluateDerivatives))
			{
				return 0;
			}
			if (evaluateDerivativeCache != derivativeCache)
			{
				// pre-calculate map from higher to lower element derivatives of mesh order
				const int meshOrder = fieldDerivative.getMeshOrder();
				const int dstDimension = element->getDimension();
				const int srcDimension = evaluateElement->getDimension();
				int dstSize = 1;
				int srcSize = 1;
				int dstIndex[MAXIMUM_MESH_DERIVATIVE_ORDER];
				for (int i = 0; i < meshOrder; ++i)
				{
					dstSize *= dstDimension;
					srcSize *= srcDimension;
					dstIndex[i] = 0;
				}
				std::vector<FE_value> derivativeMap(dstSize*srcSize);
				FE_value *mapRow = derivativeMap.data();
				for (int d = 0; d < dstSize; ++d)
				{
					int srcIndex[MAXIMUM_MESH_DERIVATIVE_ORDER];
					for (int i = 0; i < meshOrder; ++i)
					{
						srcIndex[i] = 0;
					}
					for (int s = 0; s < srcSize; ++s)
					{
						FE_value product = 1.0;
						for (int i = 0; i < meshOrder; ++i)
						{
							product *= coordinateTransformation[srcIndex[i] * (dstDimension + 1) + dstIndex[i] + 1];
						}
						mapRow[s] = product;
						for (int i = 0; (i < meshOrder) && (++(srcIndex[i]) == srcDimension); ++i)
						{
							srcIndex[i] = 0;
						}
					}
					for (int i = 0; (i < meshOrder) && (++(dstIndex[i]) == dstDimension); ++i)
					{
						dstIndex[i] = 0;
					}
					mapRow += srcSize;
				}
				// transform all components, and inner nested derivative terms (currently only parameter derivatives)
				derivativeCache->zeroValues();
				FE_value *derivatives = derivativeCache->values;
				const int paramCount = derivativeCache->getTermCount() / fieldDerivative.getMeshTermCount();
				for (int c = 0; c < this->field->number_of_components; ++c)
				{
					FE_value *mapRow = derivativeMap.data();
					for (int d = 0; d < dstSize; ++d)
					{
						const FE_value *srcDerivatives = evaluateDerivatives + c * srcSize * paramCount;
						for (int s = 0; s < srcSize; ++s)
						{
							const FE_value scale = mapRow[s];
							if (scale != 0.0)
							{
								for (int p = 0; p < paramCount; ++p)
								{
									derivatives[p] += scale * srcDerivatives[p];
								}
							}
							srcDerivatives += paramCount;
						}
						mapRow += srcSize;
						derivatives += paramCount;
					}
				}
			}
			return 1;
		}
		else if (valueType == INT_VALUE)
		{
			// all derivatives of int values are zero as field is constant with step changes
			inValueCache.getDerivativeValueCache(fieldDerivative)->zeroValues();
			return 1;
		}
	}
	else if (((valueType == FE_VALUE_VALUE)
		|| (valueType == INT_VALUE)
		|| (valueType == DOUBLE_VALUE)
		|| (valueType == FLT_VALUE)
		|| (valueType == SHORT_VALUE)) && this->is_defined_at_location(cache))
	{
		// assume all derivatives are zero
		// future: handle derivatives w.r.t. own field parameters?
		inValueCache.getDerivativeValueCache(fieldDerivative)->zeroValues();
		return 1;
	}
	return 0;  // non-numeric
}

int Computed_field_finite_element::getNodeParameters(cmzn_fieldcache& cache, int componentNumber, 
	cmzn_node_value_label valueLabel, int versionNumber,
	int valuesCount, double *valuesOut)
{
	const Field_location_node *node_location = cache.get_location_node();
	if ((componentNumber >= this->field->number_of_components)
		|| (versionNumber < 0)
		|| (valuesCount < ((componentNumber < 0) ? this->field->number_of_components : 1))
		|| (!valuesOut)
		|| (!node_location))
	{
		display_message(ERROR_MESSAGE, "FieldFiniteElement getNodeParameters.  Invalid arguments");
		return CMZN_ERROR_ARGUMENT;
	}
	const Value_type value_type = this->fe_field->getValueType();
	if (FE_VALUE_VALUE != value_type)
	{
		display_message(ERROR_MESSAGE, "FieldFiniteElement getNodeParameters.  Not implemented for field value type");
		return CMZN_ERROR_NOT_IMPLEMENTED;
	}
	FE_node *node = node_location->get_node();
	FE_value time = node_location->get_time();
	return get_FE_nodal_FE_value_value(node, this->fe_field, componentNumber, valueLabel, versionNumber, time, valuesOut);
}

int Computed_field_finite_element::setNodeParameters(cmzn_fieldcache& cache,
	int componentNumber, cmzn_node_value_label valueLabel, int versionNumber,
	int valuesCount, const double *valuesIn)
{
	const Field_location_node *node_location = cache.get_location_node();
	if ((componentNumber >= this->field->number_of_components)
		|| (versionNumber < 0)
		|| (valuesCount < ((componentNumber < 0) ? this->field->number_of_components : 1))
		|| (!valuesIn)
		|| (!node_location))
	{
		display_message(ERROR_MESSAGE, "FieldFiniteElement setNodeParameters.  Invalid arguments");
		return CMZN_ERROR_ARGUMENT;
	}
	const Value_type value_type = this->fe_field->getValueType();
	if (FE_VALUE_VALUE != value_type)
	{
		display_message(ERROR_MESSAGE, "FieldFiniteElement setNodeParameters.  Not implemented for field value type");
		return CMZN_ERROR_NOT_IMPLEMENTED;
	}
	FE_node *node = node_location->get_node();
	FE_value time = node_location->get_time();
	return set_FE_nodal_FE_value_value(node, this->fe_field, componentNumber, valueLabel, versionNumber, time, valuesIn);
}

bool Computed_field_finite_element::hasParametersAtLocation(cmzn_fieldcache& cache)
{
	const Field_location_node *node_location = cache.get_location_node();
	if (node_location)
		return node_location->get_node()->getNodeField(this->fe_field);
	return false;
}

enum FieldAssignmentResult Computed_field_finite_element::assign(cmzn_fieldcache& cache, RealFieldValueCache& valueCache)
{
	if (cache.assignInCacheOnly())
	{
		return FIELD_ASSIGNMENT_RESULT_ALL_VALUES_SET;
	}
	FiniteElementRealFieldValueCache& feValueCache = FiniteElementRealFieldValueCache::cast(valueCache);
	FieldAssignmentResult result = FIELD_ASSIGNMENT_RESULT_ALL_VALUES_SET;
	const Value_type value_type = this->fe_field->getValueType();
	const Field_location_element_xi *element_xi_location;
	const Field_location_node *node_location;
	if ((element_xi_location = cache.get_location_element_xi()))
	{
		FE_element* element = element_xi_location->get_element();
		const FE_value* xi = element_xi_location->get_xi();

		const FE_mesh_field_data *meshFieldData = this->fe_field->getMeshFieldData(element->getMesh());
		if (!meshFieldData) // field not defined on any elements of mesh
		{
			result = FIELD_ASSIGNMENT_RESULT_FAIL;
		}
		else
		{
			const DsLabelIndex elementIndex = element->getIndex();
			FE_element_shape *element_shape = element->getElementShape();
			const int componentCount = this->field->number_of_components;
			bool elementFieldChange = false;
			for (int c = 0; c < componentCount; ++c)
			{
				const FE_mesh_field_template *mft = meshFieldData->getComponentMeshfieldtemplate(c);
				const FE_element_field_template *eft = mft->getElementfieldtemplate(elementIndex);
				if (!eft) // field not defined on element
				{
					result = FIELD_ASSIGNMENT_RESULT_FAIL;
					break;
				}
				if (eft->getParameterMappingMode() != CMZN_ELEMENTFIELDTEMPLATE_PARAMETER_MAPPING_MODE_ELEMENT)
					continue;  // ignore non-element-based components
				const int numberOfElementDOFs = eft->getNumberOfElementDOFs();
				const int *gridNumberInXi = 0;
				if ((1 == numberOfElementDOFs) || (gridNumberInXi = eft->getLegacyGridNumberInXi()))
				{
					// Ensuring the constant case is implemented for triangles etc. and no legacy grid
					int offset = 0;
					if (numberOfElementDOFs > 1)
					{
						int indices[MAXIMUM_ELEMENT_XI_DIMENSIONS];
						const int dimension = element->getMesh()->getDimension();
						if (!FE_element_shape_get_indices_for_xi_location_in_cell_corners(
							element_shape, gridNumberInXi, xi, indices))
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_finite_element::assign.  Element locations do not coincide with grid");
							result = FIELD_ASSIGNMENT_RESULT_FAIL;
							break;
						}
						else
						{
							offset = indices[dimension - 1];
							for (int i = dimension - 2; i >= 0; i--)
								offset = offset*(gridNumberInXi[i] + 1) + indices[i];
						}
					}
					switch (value_type)
					{
					case FE_VALUE_VALUE:
					{
						auto component = static_cast<FE_mesh_field_data::Component<FE_value> *>(meshFieldData->getComponentBase(c));
						FE_value *values = component->getOrCreateElementValues(elementIndex, numberOfElementDOFs);
						if (!values)
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_finite_element::assign.   Unable to get grid FE_value values");
							result = FIELD_ASSIGNMENT_RESULT_FAIL;
						}
						else
						{
                            values[offset] = valueCache.values[c];
							elementFieldChange = true;
						}
					} break;
					case INT_VALUE:
					{
						auto component = static_cast<FE_mesh_field_data::Component<int> *>(meshFieldData->getComponentBase(c));
						int *values = component->getOrCreateElementValues(elementIndex, numberOfElementDOFs);
						if (!values)
						{
							display_message(ERROR_MESSAGE,
								"Computed_field_finite_element::assign.   Unable to get grid int values");
							result = FIELD_ASSIGNMENT_RESULT_FAIL;
						}
						else
						{
                            values[offset] = static_cast<int>(valueCache.values[c]);
							result = FIELD_ASSIGNMENT_RESULT_PARTIAL_VALUES_SET; // GRC: don't know why partial for integer?
							elementFieldChange = true;
						}
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Computed_field_finite_element::assign.   Unsupported value type");
						result = FIELD_ASSIGNMENT_RESULT_FAIL;
					} break;
					}
					if (result == FIELD_ASSIGNMENT_RESULT_FAIL)
						break;
				}
				else
				{
					// not implemented for non-constant non-grid case
					result = FIELD_ASSIGNMENT_RESULT_FAIL;
					break;
				}
			}
			if (elementFieldChange)
			{
				// change notification
				element->getMesh()->elementFieldChange(elementIndex, DS_LABEL_CHANGE_TYPE_RELATED, this->fe_field);
			}
		}
	}
	else if ((node_location = cache.get_location_node()))
	{
		const int componentCount = field->number_of_components;
		cmzn_node *node = node_location->get_node();
		FE_value time = node_location->get_time();
		const FE_node_field *node_field = node->getNodeField(this->fe_field);
		if (!node_field)
		{
			return FIELD_ASSIGNMENT_RESULT_FAIL;
		}
		/* set values all versions; to set values for selected version only,
		   use COMPUTED_FIELD_NODE_VALUE instead */
		int versionsCount = 1;
		for (int c = 0; c < componentCount; ++c)
		{
			const FE_node_field_template *nft = node_field->getComponent(c);
			const int count = nft->getValueNumberOfVersions(CMZN_NODE_VALUE_LABEL_VALUE);
			if (count > versionsCount)
			{
				versionsCount = count;
			}
		}
		for (int v = 0; v < versionsCount; ++v)
		{
			int return_code = CMZN_ERROR_GENERAL;
			switch (value_type)
			{
			case DOUBLE_VALUE:
			{
				double *double_values = feValueCache.double_values.data();
				for (int c = 0; c < componentCount; ++c)
				{
					double_values[c] = static_cast<double>(feValueCache.values[c]);
				}
				return_code = set_FE_nodal_double_value(node, this->fe_field, /*componentNumber=ALL*/-1,
					CMZN_NODE_VALUE_LABEL_VALUE, v, time, double_values);
			} break;
			case FE_VALUE_VALUE:
			{
				return_code = set_FE_nodal_FE_value_value(node, fe_field,/*componentNumber=ALL*/-1,
					CMZN_NODE_VALUE_LABEL_VALUE, v, time, feValueCache.values);
			} break;
			case FLT_VALUE:
			{
				float *float_values = feValueCache.float_values.data();
				for (int c = 0; c < componentCount; ++c)
				{
					float_values[c] = static_cast<FE_value>(feValueCache.values[c]);
				}
				return_code = set_FE_nodal_float_value(node, this->fe_field, /*componentNumber=ALL*/-1,
					CMZN_NODE_VALUE_LABEL_VALUE, v, time, float_values);
			} break;
			case INT_VALUE:
			{
				int *int_values = feValueCache.int_values.data();
				for (int c = 0; c < componentCount; ++c)
				{
					int_values[c] = static_cast<FE_value>(feValueCache.values[c]);
				}
				return_code = set_FE_nodal_int_value(node, this->fe_field, /*componentNumber=ALL*/-1,
					CMZN_NODE_VALUE_LABEL_VALUE, v, time, int_values);
			} break;
			case SHORT_VALUE:
			{
				short *short_values = feValueCache.short_values.data();
				for (int c = 0; c < componentCount; ++c)
				{
					short_values[c] = static_cast<short>(feValueCache.values[c]);
				}
				return_code = set_FE_nodal_short_value(node, this->fe_field, /*componentNumber=ALL*/-1,
					CMZN_NODE_VALUE_LABEL_VALUE, v, time, short_values);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_finite_element::assign.  "
					"Unsupported value type %s in finite_element field",
					Value_type_string(value_type));
				return_code = 0;
			} break;
			}
			if (CMZN_OK != return_code)
			{
				return FIELD_ASSIGNMENT_RESULT_FAIL;
			}
		}
	}
	else
	{
		result = FIELD_ASSIGNMENT_RESULT_FAIL;
	}
	return result;
}

enum FieldAssignmentResult Computed_field_finite_element::assign(cmzn_fieldcache& cache, MeshLocationFieldValueCache& valueCache)
{
	const Field_location_node *node_location = cache.get_location_node();
	if (node_location &&
		(this->fe_field->getValueType() == ELEMENT_XI_VALUE) &&
		(this->fe_field->get_FE_field_type() == GENERAL_FE_FIELD))
	{
		if (cache.assignInCacheOnly() ||
			set_FE_nodal_element_xi_value(node_location->get_node(), fe_field,
				/*component_number*/0, valueCache.element, valueCache.xi))
		{
			return FIELD_ASSIGNMENT_RESULT_ALL_VALUES_SET;
		}
	}
	return FIELD_ASSIGNMENT_RESULT_FAIL;
};

enum FieldAssignmentResult Computed_field_finite_element::assign(cmzn_fieldcache& cache, StringFieldValueCache& valueCache)
{
	const Field_location_node *node_location = cache.get_location_node();
	if (node_location &&
		(this->fe_field->getValueType() == STRING_VALUE) &&
		(this->fe_field->get_FE_field_type() == GENERAL_FE_FIELD))
	{
		if (cache.assignInCacheOnly() ||
			set_FE_nodal_string_value(node_location->get_node(),
				fe_field, /*component_number*/0, valueCache.stringValue))
		{
			return FIELD_ASSIGNMENT_RESULT_ALL_VALUES_SET;
		}
	}
	return FIELD_ASSIGNMENT_RESULT_FAIL;
}

/**
 * If the field is grid-based in element, get the number of linear grid cells
 * each xi-direction of element. Note that this is one less than the number of
 * grid points in each direction.
 * @param element  The element to get native grid discretization in.
 * @param number_in_xi  Array to receive returned numbers. Caller must ensure
 * this is no smaller than the dimension of the element.
 * @return  1 on success, 0 with no error message if the field not grid-based.
 */
int Computed_field_finite_element::get_native_discretization_in_element(
	struct FE_element *element,int *number_in_xi)
{
	if (this->field && element && element->getMesh() && number_in_xi)
	{
		const FE_mesh_field_data *meshFieldData = this->fe_field->getMeshFieldData(element->getMesh());
		if (!meshFieldData)
			return 0; // invalid element or not defined in any element of mesh
		const int componentCount = this->field->number_of_components;
		// use first grid-based field component, if any
		for (int c = 0; c < componentCount; ++c)
		{
			const FE_mesh_field_template *mft = meshFieldData->getComponentMeshfieldtemplate(c);
			const FE_element_field_template *eft = mft->getElementfieldtemplate(element->getIndex());
			if (eft)
			{
				const int *gridNumberInXi = eft->getLegacyGridNumberInXi();
				if (gridNumberInXi) // only if legacy grid; other element-based do not multiply
				{
					const int dimension = element->getMesh()->getDimension();
					for (int d = 0; d < dimension; ++d)
						number_in_xi[d] = gridNumberInXi[d];
					return 1;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element::get_native_discretization_in_element.  "
			"Invalid argument(s)");
	}
	return 0;
}

int Computed_field_finite_element::list()
{
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    fe_field : %s\n",this->fe_field->getName());
		display_message(INFORMATION_MESSAGE,"    CM field type : %s\n",
			ENUMERATOR_STRING(CM_field_type)(this->fe_field->get_CM_field_type()));
		Value_type value_type = this->fe_field->getValueType();
		display_message(INFORMATION_MESSAGE,"    Value type : %s\n",
			Value_type_string(value_type));
		if (ELEMENT_XI_VALUE == value_type)
		{
			const FE_mesh *hostMesh = this->fe_field->getElementXiHostMesh();
			display_message(INFORMATION_MESSAGE,"    host mesh : %s\n", hostMesh ? hostMesh->getName() : "unknown");
		}
		return 1;
	}
	display_message(ERROR_MESSAGE,
		"list_Computed_field_finite_element.  Invalid arguments.");
	return 0;
}

char *Computed_field_finite_element::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *component_name, temp_string[40];
	int error, i, number_of_components;

	ENTER(Computed_field_finite_element::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_finite_element_type_string, &error);
		number_of_components = get_FE_field_number_of_components(fe_field);
		sprintf(temp_string, " number_of_components %d ", number_of_components);
		append_string(&command_string, temp_string, &error);
		append_string(&command_string, ENUMERATOR_STRING(CM_field_type)(
			this->fe_field->get_CM_field_type()), &error);
		append_string(&command_string, " ", &error);
		append_string(&command_string,
			Value_type_string(this->fe_field->getValueType()), &error);
		append_string(&command_string, " component_names", &error);
		for (i = 0; i < number_of_components; i++)
		{
			component_name = get_FE_field_component_name(fe_field, i);
			if (component_name)
			{
				make_valid_token(&component_name);
				append_string(&command_string, " ", &error);
				append_string(&command_string, component_name, &error);
				DEALLOCATE(component_name);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_finite_element::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_finite_element::get_command_string */

} //namespace

cmzn_field *cmzn_fieldmodule_create_field_finite_element_wrapper(
	struct cmzn_fieldmodule *fieldmodule, struct FE_field *fe_field, const char *name)
{
	cmzn_field *field = 0;
	if ((fieldmodule) && (fe_field))
	{
		cmzn_region *region = cmzn_fieldmodule_get_region_internal(fieldmodule);
		FE_region *fe_region = region->get_FE_region();
		if (fe_field->get_FE_region() != fe_region)
		{
			display_message(ERROR_MESSAGE,
				"cmzn_fieldmodule_create_field_finite_element_wrapper.  Region mismatch");
			return 0;
		}
		/* 1. make dynamic allocations for any new type-specific data */
		field = Computed_field_create_generic(fieldmodule,
			/*check_source_field_regions*/true, fe_field->getNumberOfComponents(),
			/*number_of_source_fields*/0, NULL,
			/*number_of_source_values*/0, NULL,
			new Computed_field_finite_element(fe_field), name);
		if (field && field->core)
		{
			Computed_field_finite_element *fieldFiniteElement=
				static_cast<Computed_field_finite_element*>(field->core);
			const Value_type value_type = get_FE_field_value_type(fieldFiniteElement->fe_field);
			switch (value_type)
			{
				case ELEMENT_XI_VALUE:
					fieldFiniteElement->type = CMZN_FIELD_TYPE_STORED_MESH_LOCATION;
					break;
				case STRING_VALUE:
				case URL_VALUE:
					fieldFiniteElement->type = CMZN_FIELD_TYPE_STORED_STRING;
					break;
				case FE_VALUE_VALUE:
					fieldFiniteElement->type = CMZN_FIELD_TYPE_FINITE_ELEMENT;
					break;
				default:
					break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_fieldmodule_create_field_finite_element_wrapper.  Invalid argument(s)");
	}
	return (field);
}

cmzn_field_id cmzn_fieldmodule_create_field_finite_element_internal(
	cmzn_fieldmodule_id field_module, enum Value_type value_type, int number_of_components)
{
	cmzn_field_id field = 0;
	// cache changes to ensure FE_field not automatically wrapped already
	cmzn_fieldmodule_begin_change(field_module);
	cmzn_region *region = cmzn_fieldmodule_get_region_internal(field_module);
	FE_region *fe_region = region->get_FE_region();
	// ensure FE_field and Computed_field have same name
	char *fieldName = Computed_field_manager_get_unique_field_name(region->getFieldManager());
	FE_field *fe_field = FE_region_get_FE_field_with_general_properties(
		fe_region, fieldName, value_type, number_of_components);
	if (fe_field)
	{
		field = Computed_field_create_generic(field_module,
			/*check_source_field_regions*/true, number_of_components,
			/*number_of_source_fields*/0, NULL,
			/*number_of_source_values*/0, NULL,
			new Computed_field_finite_element(fe_field));
		FE_field::deaccess(fe_field);
		field->setName(fieldName);
	}
	DEALLOCATE(fieldName);
	cmzn_fieldmodule_end_change(field_module);
	return (field);
}

cmzn_field_id cmzn_fieldmodule_create_field_finite_element(
	cmzn_fieldmodule_id fieldmodule, int number_of_components)
{
	Computed_field *field = NULL;
	if ((fieldmodule) && (0 < number_of_components))
	{
		field = cmzn_fieldmodule_create_field_finite_element_internal(
			fieldmodule, FE_VALUE_VALUE, number_of_components);
		if (field && field->core)
		{
			Computed_field_finite_element *fieldFiniteElement=
				static_cast<Computed_field_finite_element*>(field->core);
			fieldFiniteElement->type = CMZN_FIELD_TYPE_FINITE_ELEMENT;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_fieldmodule_create_field_finite_element.  Invalid argument(s)");
	}
	return (field);
}

cmzn_field_finite_element_id cmzn_field_cast_finite_element(cmzn_field_id field)
{
	if (field)
	{
		Computed_field_finite_element* core =
			dynamic_cast<Computed_field_finite_element*>(field->core);
		if (core &&
			(core->fe_field->get_FE_field_type() == GENERAL_FE_FIELD) &&
			(core->fe_field->getValueType() == FE_VALUE_VALUE))
		{
			cmzn_field_access(field);
			return (reinterpret_cast<cmzn_field_finite_element_id>(field));
		}
	}
	return 0;
}

int cmzn_field_finite_element_destroy(
	cmzn_field_finite_element_id *finite_element_field_address)
{
	return cmzn_field_destroy(reinterpret_cast<cmzn_field_id *>(finite_element_field_address));
}

int cmzn_field_finite_element_get_node_parameters(
	cmzn_field_finite_element_id finite_element_field, cmzn_fieldcache_id cache,
	int component_number, enum cmzn_node_value_label node_value_label,
	int version_number, int values_count, double *values_out)
{
	if (finite_element_field && cache && ((component_number == -1) || (0 < component_number)))
	{
		return cmzn_field_finite_element_core_cast(finite_element_field)->getNodeParameters(*cache,
			component_number - 1, node_value_label, version_number - 1, values_count, values_out);
	}
	display_message(ERROR_MESSAGE, "FieldFiniteElement getNodeParameters.  Invalid arguments");
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_field_finite_element_set_node_parameters(
	cmzn_field_finite_element_id finite_element_field, cmzn_fieldcache_id cache,
	int component_number, enum cmzn_node_value_label node_value_label,
	int version_number, int values_count, const double *values_in)
{
	if (finite_element_field && cache && ((component_number == -1) || (0 < component_number)))
	{
		return cmzn_field_finite_element_core_cast(finite_element_field)->setNodeParameters(*cache,
			component_number - 1, node_value_label, version_number - 1, values_count, values_in);
	}
	display_message(ERROR_MESSAGE, "FieldFiniteElement setNodeParameters.  Invalid arguments");
	return CMZN_ERROR_ARGUMENT;
}

bool cmzn_field_finite_element_has_parameters_at_location(
	cmzn_field_finite_element_id finite_element_field, cmzn_fieldcache_id cache)
{
	if (finite_element_field && cache)
	{
		return cmzn_field_finite_element_core_cast(finite_element_field)->hasParametersAtLocation(*cache);
	}
	display_message(ERROR_MESSAGE, "FieldFiniteElement hasParametersAtLocation.  Invalid arguments");
	return false;
}

cmzn_field_id cmzn_fieldmodule_create_field_stored_mesh_location(
	cmzn_fieldmodule_id fieldmodule, cmzn_mesh_id mesh)
{
	if ((fieldmodule) && (mesh) && (mesh->getRegion() ==
		cmzn_fieldmodule_get_region_internal(fieldmodule)))
	{
		cmzn_fieldmodule_begin_change(fieldmodule);
		cmzn_field *field = cmzn_fieldmodule_create_field_finite_element_internal(
			fieldmodule, ELEMENT_XI_VALUE, /*number_of_components*/1);
		if (field && field->core)
		{
			auto fieldFiniteElement = static_cast<Computed_field_finite_element*>(field->core);
			fieldFiniteElement->fe_field->setElementXiHostMesh(mesh->getFeMesh());
			fieldFiniteElement->type = CMZN_FIELD_TYPE_STORED_MESH_LOCATION;
		}
		cmzn_fieldmodule_end_change(fieldmodule);
		return field;
	}
	display_message(ERROR_MESSAGE, "cmzn_fieldmodule_create_field_finite_element.  Invalid argument(s)");
	return nullptr;
}

cmzn_field_stored_mesh_location_id cmzn_field_cast_stored_mesh_location(cmzn_field_id field)
{
	if (field)
	{
		Computed_field_finite_element* core =
			dynamic_cast<Computed_field_finite_element*>(field->core);
		if (core &&
			(core->fe_field->get_FE_field_type() == GENERAL_FE_FIELD) &&
			(get_FE_field_value_type(core->fe_field) == ELEMENT_XI_VALUE))
		{
			cmzn_field_access(field);
			return (reinterpret_cast<cmzn_field_stored_mesh_location_id>(field));
		}
	}
	return 0;
}

inline Computed_field_finite_element *cmzn_field_stored_mesh_location_core_cast(
	cmzn_field_stored_mesh_location_id stored_mesh_location_field)
{
	return (static_cast<Computed_field_finite_element*>(
		reinterpret_cast<Computed_field*>(stored_mesh_location_field)->core));
}

int cmzn_field_stored_mesh_location_destroy(
	cmzn_field_stored_mesh_location_id *stored_mesh_location_field_address)
{
	return cmzn_field_destroy(
		reinterpret_cast<cmzn_field_id *>(stored_mesh_location_field_address));
}

cmzn_mesh_id cmzn_field_stored_mesh_location_get_mesh(
	cmzn_field_stored_mesh_location_id stored_mesh_location_field)
{
	if (stored_mesh_location_field)
	{
		FE_mesh *feMesh = cmzn_field_stored_mesh_location_core_cast(stored_mesh_location_field)->fe_field->getElementXiHostMesh();
		cmzn_region* region = feMesh->get_FE_region()->getRegion();
		cmzn_mesh* mesh = region->findMeshByDimension(feMesh->getDimension());
		if (mesh)
		{
			return mesh->access();
		}
	}
	return nullptr;
}

cmzn_field_id cmzn_fieldmodule_create_field_stored_string(
	cmzn_fieldmodule_id fieldmodule)
{
	cmzn_field_id field = cmzn_fieldmodule_create_field_finite_element_internal(
		fieldmodule, STRING_VALUE, /*number_of_components*/1);
	if ((field) && (field->core))
	{
		Computed_field_finite_element *fieldFiniteElement= static_cast<Computed_field_finite_element*>(
			field->core);
		fieldFiniteElement->type = CMZN_FIELD_TYPE_STORED_STRING;
	}
	return field;
}

cmzn_field_stored_string_id cmzn_field_cast_stored_string(cmzn_field_id field)
{
	if (field)
	{
		Computed_field_finite_element* core =
			dynamic_cast<Computed_field_finite_element*>(field->core);
		if (core &&
			(core->fe_field->get_FE_field_type() == GENERAL_FE_FIELD) &&
			(get_FE_field_value_type(core->fe_field) == STRING_VALUE))
		{
			cmzn_field_access(field);
			return (reinterpret_cast<cmzn_field_stored_string_id>(field));
		}
	}
	return 0;
}

int cmzn_field_stored_string_destroy(
	cmzn_field_stored_string_id *stored_string_field_address)
{
	return cmzn_field_destroy(
		reinterpret_cast<cmzn_field_id *>(stored_string_field_address));
}

int Computed_field_is_type_finite_element(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns true if <field> has the appropriate static type string.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_finite_element);
	if (field)
	{
		if (dynamic_cast<Computed_field_finite_element*>(field->core))
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
			"Computed_field_is_type_finite_element.  Missing field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_finite_element */

int Computed_field_get_type_finite_element(cmzn_field *field,
	struct FE_field **fe_field)
{
	Computed_field_finite_element* core;
	if (field && (0 != (core = dynamic_cast<Computed_field_finite_element*>(field->core))))
	{
		*fe_field = core->fe_field;
		return 1;
	}
	return 0;
}

FE_field* cmzn_field_finite_element_get_FE_field(cmzn_field* field)
{
	Computed_field_finite_element* core;
	if (field && (0 != (core = dynamic_cast<Computed_field_finite_element*>(field->core))))
	{
		return core->fe_field;
	}
	return nullptr;
}

namespace {

const char computed_field_cmiss_number_type_string[] = "cmiss_number";

class Computed_field_cmiss_number : public Computed_field_core
{
public:
	Computed_field_cmiss_number() : Computed_field_core()
	{
	};

	virtual bool is_purely_function_of_field(cmzn_field *other_field)
	{
		return (this->field == other_field);
	}

private:
	Computed_field_core *copy()
	{
		return new Computed_field_cmiss_number();
	}

	const char *get_type_string()
	{
		return(computed_field_cmiss_number_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_cmiss_number*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	virtual int evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative);

	virtual int getDerivativeTreeOrder(const FieldDerivative& fieldDerivative)
	{
		return 0;
	}

	int list();

	char* get_command_string();
};

int Computed_field_cmiss_number::evaluate(cmzn_fieldcache& cache,
	FieldValueCache& inValueCache)
{
	int return_code = 1;
	RealFieldValueCache& valueCache = RealFieldValueCache::cast(inValueCache);
	const Field_location_element_xi *element_xi_location;
	const Field_location_node *node_location;
	if ((element_xi_location = cache.get_location_element_xi()))
	{
		FE_element* element = element_xi_location->get_element();
		valueCache.values[0] = static_cast<FE_value>(get_FE_element_identifier(element));
	}
	else if ((node_location = cache.get_location_node()))
	{
		FE_node *node = node_location->get_node();
		valueCache.values[0] = static_cast<FE_value>(get_FE_node_identifier(node));
	}
	else
	{
		// Location type unknown or not implemented
		return_code = 0;
	}
	return (return_code);
}

int Computed_field_cmiss_number::evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative)
{
	// assume all derivatives are zero
	inValueCache.getDerivativeValueCache(fieldDerivative)->zeroValues();
	return 1;
}

int Computed_field_cmiss_number::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_cmiss_number);
	if (field)
	{
		/* no extra parameters */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_cmiss_number.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_cmiss_number */

char *Computed_field_cmiss_number::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string;

	ENTER(Computed_field_cmiss_number::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		/* no command options */
		command_string = duplicate_string(computed_field_cmiss_number_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cmiss_number::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_cmiss_number::get_command_string */

} //namespace

int Computed_field_is_type_cmiss_number(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 14 August 2006

DESCRIPTION :
Returns true if <field> has the appropriate static type string.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_cmiss_number);
	if (field)
	{
		if (dynamic_cast<Computed_field_cmiss_number*>(field->core))
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
			"Computed_field_is_type_cmiss_number.  Missing field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_cmiss_number */

cmzn_field *cmzn_fieldmodule_create_field_cmiss_number(
	cmzn_fieldmodule *fieldmodule)
{
	cmzn_field *field = nullptr;
	if (fieldmodule)
	{
		field = Computed_field_create_generic(fieldmodule,
			/*check_source_field_regions*/true,
			/*number_of_components*/1,
			/*number_of_source_fields*/0, NULL,
			/*number_of_source_values*/0, NULL,
			new Computed_field_cmiss_number());
	}
	return field;
}

#if defined (COMPUTED_FIELD_ACCESS_COUNT)
namespace {

const char computed_field_access_count_type_string[] = "access_count";

class Computed_field_access_count : public Computed_field_core
{
public:
	Computed_field_access_count() : Computed_field_core()
	{
	};

	virtual bool is_purely_function_of_field(cmzn_field *other_field)
	{
		return (this->field == other_field);
	}

private:
	Computed_field_core *copy()
	{
		return new Computed_field_access_count();
	}

	const char *get_type_string()
	{
		return(computed_field_access_count_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_access_count*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	virtual int evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative)
	{
		inValueCache.getDerivativeValueCache(fieldDerivative)->zeroValues();
		return 1;
	}

	virtual int getDerivativeTreeOrder(const FieldDerivative& fieldDerivative)
	{
		return 0;
	}

	int list();

	char* get_command_string();
};

int Computed_field_access_count::evaluate(cmzn_fieldcache& cache,
	FieldValueCache& inValueCache)
{
	RealFieldValueCache& valueCache = RealFieldValueCache::cast(inValueCache);
	const Field_location_element_xi *element_xi_location;
	const Field_location_node *node_location;
	if ((element_xi_location = cache.get_location_element_xi()))
	{
		FE_element* element = element_xi_location->get_element();
		valueCache.values[0] = static_cast<FE_value>(element->getAccessCount());
	}
	else if ((node_location = cache.get_location_node()))
	{
		FE_node *node = node_location->get_node();
		valueCache.values[0] = (FE_value)node->getAccessCount();
	}
	else
	{
		valueCache.values[0] = 0;
	}
	return 1;
}

int Computed_field_access_count::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_access_count);
	if (field)
	{
		/* no extra parameters */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_access_count.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_access_count */

char *Computed_field_access_count::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string;

	ENTER(Computed_field_access_count::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		/* no command options */
		command_string = duplicate_string(computed_field_access_count_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_access_count::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_access_count::get_command_string */

} //namespace

/*****************************************************************************//**
 * Creates a field which returns the element or node access count as its value.
 *
 * @experimental
 * @param field_module  Region field module which will own new field.
 * @return Newly created field
 */
cmzn_field *cmzn_fieldmodule_create_field_access_count(
	cmzn_fieldmodule *fieldmodule)
{
	cmzn_field *field = nullptr;
	if (fieldmodule)
	{
		field = Computed_field_create_generic(fieldmodule,
			/*check_source_field_regions*/true,
			/*number_of_components*/1,
			/*number_of_source_fields*/0, NULL,
			/*number_of_source_values*/0, NULL,
			new Computed_field_access_count());
	}
	return (field);
}

#endif /* defined (COMPUTED_FIELD_ACCESS_COUNT) */

namespace {

const char computed_field_node_value_type_string[] = "node_value";

class Computed_field_node_value : public Computed_field_core
{
public:
	FE_field *fe_field;
	cmzn_node_value_label nodeValueLabel;
	int versionNumber; // starting at 0

	Computed_field_node_value(cmzn_field_id finite_element_field,
			cmzn_node_value_label nodeValueLabelIn, int versionNumberIn) :
		Computed_field_core(),
		fe_field(0),
		nodeValueLabel(nodeValueLabelIn),
		versionNumber(versionNumberIn)
	{
		Computed_field_get_type_finite_element(finite_element_field, &fe_field);
		ACCESS(FE_field)(fe_field);
	};

	virtual ~Computed_field_node_value();

	cmzn_node_value_label getNodeValueLabel() const
	{
		return this->nodeValueLabel;
	}

	int setNodeValueLabel(cmzn_node_value_label nodeValueLabelIn)
	{
		if (nodeValueLabelIn != this->nodeValueLabel)
		{
			const char *s = cmzn_node_value_label_conversion::to_string(nodeValueLabelIn);
			if (!s)
			{
				display_message(ERROR_MESSAGE, "FieldNodeValue setNodeValueLabel.  Invalid node value label");
				return CMZN_ERROR_ARGUMENT;
			}
			this->nodeValueLabel = nodeValueLabelIn;
			this->field->setChanged();
		}
		return CMZN_OK;
	}

	/** @return  Version number starting at 0 */
	int getVersionNumber() const
	{
		return this->versionNumber;
	}

	/** @param versionNumberIn  Version number >= 0 */
	int setVersionNumber(int versionNumberIn)
	{
		if (versionNumberIn != this->versionNumber)
		{
			if (versionNumberIn < 0)
			{
				display_message(ERROR_MESSAGE, "FieldNodeValue setVersionNumber.  Invalid version number");
				return CMZN_ERROR_ARGUMENT;
			}
			this->versionNumber = versionNumberIn;
			this->field->setChanged();
		}
		return CMZN_OK;
	}

	virtual void inherit_source_field_attributes()
	{
		if (this->field)
		{
			this->field->setCoordinateSystem(fe_field->getCoordinateSystem(), /*notifyChange*/false);
		}
	}

	virtual bool is_purely_function_of_field(cmzn_field *other_field)
	{
		return (this->field == other_field);
	}

	/** @return  True if this finite element field result changed. */
	virtual bool isResultChanged()
	{
		int change = 0;
		CHANGE_LOG_QUERY(FE_field)(this->fe_field->get_FE_region()->fe_field_changes, this->fe_field, &change);
		// changed if any flags other than identifier changed
		if (change | (~CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_field)))
		{
			return true;
		}
		return false;
	}

private:
	Computed_field_core *copy();

	const char *get_type_string()
	{
		return(computed_field_node_value_type_string);
	}

	virtual enum cmzn_field_type get_type()
	{
		return CMZN_FIELD_TYPE_NODE_VALUE;
	}

	int compare(Computed_field_core* other_field);

	virtual FieldValueCache *createValueCache(cmzn_fieldcache& /*fieldCache*/)
	{
		return new MultiTypeRealFieldValueCache(field->number_of_components);
	}

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	virtual int evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative);

	int list();

	char* get_command_string();

	virtual char *getComponentName(int componentNumber) const
	{
		return get_FE_field_component_name(this->fe_field, componentNumber - 1);
	}

	virtual bool is_defined_at_location(cmzn_fieldcache& cache);

	int has_numerical_components();

	enum FieldAssignmentResult assign(cmzn_fieldcache& cache, RealFieldValueCache& valueCache);

	int has_multiple_times();
};

Computed_field_node_value::~Computed_field_node_value()
{
	DEACCESS(FE_field)(&(this->fe_field));
}

Computed_field_core* Computed_field_node_value::copy()
{
	return new Computed_field_node_value(this->getSourceField(0), this->nodeValueLabel, this->versionNumber);
}

int Computed_field_node_value::compare(Computed_field_core *other_core)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	Computed_field_node_value* other;
	int return_code;

	ENTER(Computed_field_node_value::compare);
	if (field && (other = dynamic_cast<Computed_field_node_value*>(other_core)))
	{
		return_code = ((this->fe_field == other->fe_field)
			&& (this->nodeValueLabel == other->nodeValueLabel)
			&& (this->versionNumber == other->versionNumber));
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_node_value::compare */

bool Computed_field_node_value::is_defined_at_location(cmzn_fieldcache& cache)
{
	const Field_location_node *node_location;
	if ((node_location = cache.get_location_node()))
	{
		cmzn_node *node = node_location->get_node();
		const FE_node_field *node_field = node->getNodeField(this->fe_field);
		if (node_field)
		{
			// defined if at least one component has version of value label defined
			const int componentCount = field->number_of_components;
			for (int c = 0; c < componentCount; ++c)
			{
				const int versionsCount = node_field->getComponent(c)->getValueNumberOfVersions(this->nodeValueLabel);
				if (this->versionNumber < versionsCount)
				{
					return true;
				}
			}
		}
	}
	return false;
}

int Computed_field_node_value::has_numerical_components()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_node_value::has_numerical_components);
	if (field)
	{
		return_code=Value_type_is_numeric_simple(
			this->fe_field->getValueType());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_node_value::has_numerical_components.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_node_value::has_numerical_components */

int Computed_field_node_value::evaluate(cmzn_fieldcache& cache,
	FieldValueCache& inValueCache)
{
	int return_code = 1;
	MultiTypeRealFieldValueCache& valueCache = MultiTypeRealFieldValueCache::cast(inValueCache);
	const Field_location_node *node_location = cache.get_location_node();
	if (node_location)
	{
		int result = CMZN_ERROR_GENERAL;
		const int componentCount = field->number_of_components;
		cmzn_node *node = node_location->get_node();
		FE_value time = node_location->get_time();
		const Value_type value_type = this->fe_field->getValueType();
		switch (value_type)
		{
		case DOUBLE_VALUE:
		{
			double *double_values = valueCache.double_values.data();
			result = get_FE_nodal_double_value(node, this->fe_field, /*componentNumber=ALL*/-1,
				this->nodeValueLabel, this->versionNumber, time, double_values);
			for (int c = 0; c < componentCount; ++c)
			{
				valueCache.values[c] = static_cast<FE_value>(double_values[c]);
			}
		} break;
		case FE_VALUE_VALUE:
		{
			result = get_FE_nodal_FE_value_value(node, fe_field,/*componentNumber=ALL*/-1,
				this->nodeValueLabel, this->versionNumber, time, valueCache.values);
		} break;
		case FLT_VALUE:
		{
			float *float_values = valueCache.float_values.data();
			result = get_FE_nodal_float_value(node, this->fe_field, /*componentNumber=ALL*/-1,
				this->nodeValueLabel, this->versionNumber, time, float_values);
			for (int c = 0; c < componentCount; ++c)
			{
				valueCache.values[c] = static_cast<FE_value>(float_values[c]);
			}
		} break;
		case INT_VALUE:
		{
			int *int_values = valueCache.int_values.data();
			result = get_FE_nodal_int_value(node, this->fe_field, /*componentNumber=ALL*/-1,
				this->nodeValueLabel, this->versionNumber, time, int_values);
			for (int c = 0; c < componentCount; ++c)
			{
				valueCache.values[c] = static_cast<FE_value>(int_values[c]);
			}
		} break;
		case SHORT_VALUE:
		{
			short *short_values = valueCache.short_values.data();
			result = get_FE_nodal_short_value(node, this->fe_field, /*componentNumber=ALL*/-1,
				this->nodeValueLabel, this->versionNumber, time, short_values);
			for (int c = 0; c < componentCount; ++c)
			{
				valueCache.values[c] = static_cast<FE_value>(short_values[c]);
			}
		} break;
		default:
		{
			display_message(ERROR_MESSAGE, "Computed_field_node_value::evaluate.  "
				"Unsupported value type %s in node_value field",
				Value_type_string(value_type));
		} break;
		}
		return_code = ((result == CMZN_OK) || (result == CMZN_WARNING_PART_DONE)) ? 1 : 0;
	}
	else
	{
		// Only valid for Field_location_node type
		return_code = 0;
	}
	return (return_code);
}

int Computed_field_node_value::evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative)
{
	const Value_type valueType = this->fe_field->getValueType();
	if (((valueType == FE_VALUE_VALUE)
		|| (valueType == INT_VALUE)
		|| (valueType == DOUBLE_VALUE)
		|| (valueType == FLT_VALUE)
		|| (valueType == SHORT_VALUE)) && this->is_defined_at_location(cache))
	{
		// assume all derivatives are zero
		// future: handle derivatives w.r.t. own field parameters?
		inValueCache.getDerivativeValueCache(fieldDerivative)->zeroValues();
		return 1;
	}
	return 0;
}

enum FieldAssignmentResult Computed_field_node_value::assign(cmzn_fieldcache& cache, RealFieldValueCache& valueCache)
{
	const Field_location_node *node_location = cache.get_location_node();
	if (node_location)
	{
		if (cache.assignInCacheOnly())
		{
			return FIELD_ASSIGNMENT_RESULT_ALL_VALUES_SET;
		}
		FiniteElementRealFieldValueCache& feValueCache = FiniteElementRealFieldValueCache::cast(valueCache);
		const int componentCount = field->number_of_components;
		cmzn_node *node = node_location->get_node();
		const FE_value time = node_location->get_time();
		const Value_type value_type = this->fe_field->getValueType();
		int return_code = CMZN_ERROR_GENERAL;
		switch (value_type)
		{
		case DOUBLE_VALUE:
		{
			double *double_values = feValueCache.double_values.data();
			for (int c = 0; c < componentCount; ++c)
			{
				double_values[c] = static_cast<double>(feValueCache.values[c]);
			}
			return_code = set_FE_nodal_double_value(node, this->fe_field, /*componentNumber=ALL*/-1,
				this->nodeValueLabel, this->versionNumber, time, double_values);
		} break;
		case FE_VALUE_VALUE:
		{
			return_code = set_FE_nodal_FE_value_value(node, fe_field,/*componentNumber=ALL*/-1,
				this->nodeValueLabel, this->versionNumber, time, feValueCache.values);
		} break;
		case FLT_VALUE:
		{
			float *float_values = feValueCache.float_values.data();
			for (int c = 0; c < componentCount; ++c)
			{
				float_values[c] = static_cast<FE_value>(feValueCache.values[c]);
			}
			return_code = set_FE_nodal_float_value(node, this->fe_field, /*componentNumber=ALL*/-1,
				this->nodeValueLabel, this->versionNumber, time, float_values);
		} break;
		case INT_VALUE:
		{
			int *int_values = feValueCache.int_values.data();
			for (int c = 0; c < componentCount; ++c)
			{
				int_values[c] = static_cast<FE_value>(feValueCache.values[c]);
			}
			return_code = set_FE_nodal_int_value(node, this->fe_field, /*componentNumber=ALL*/-1,
				this->nodeValueLabel, this->versionNumber, time, int_values);
		} break;
		case SHORT_VALUE:
		{
			short *short_values = feValueCache.short_values.data();
			for (int c = 0; c < componentCount; ++c)
			{
				short_values[c] = static_cast<short>(feValueCache.values[c]);
			}
			return_code = set_FE_nodal_short_value(node, this->fe_field, /*componentNumber=ALL*/-1,
				this->nodeValueLabel, this->versionNumber, time, short_values);
		} break;
		default:
		{
			display_message(ERROR_MESSAGE, "Computed_field_node_value::assign.  "
				"Unsupported value type %s in node_value field", Value_type_string(value_type));
		} break;
		}
		if (CMZN_OK == return_code)
		{
			return FIELD_ASSIGNMENT_RESULT_ALL_VALUES_SET;
		}
		else if (CMZN_WARNING_PART_DONE == return_code)
		{
			return FIELD_ASSIGNMENT_RESULT_PARTIAL_VALUES_SET;
		}
		return FIELD_ASSIGNMENT_RESULT_FAIL;
	}
	return FIELD_ASSIGNMENT_RESULT_FAIL;
}

int Computed_field_node_value::list()
{
	if (field)
	{
		display_message(INFORMATION_MESSAGE,"    fe_field : %s\n",this->fe_field->getName());
		display_message(INFORMATION_MESSAGE,"    nodal value type : %s\n",
			ENUMERATOR_STRING(cmzn_node_value_label)(this->nodeValueLabel));
		display_message(INFORMATION_MESSAGE,"    version : %d\n", this->versionNumber + 1);
		return 1;
	}
	display_message(ERROR_MESSAGE,
		"list_Computed_field_node_value.  Invalid arguments.");
	return 0;
}

char *Computed_field_node_value::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error;

	ENTER(Computed_field_node_value::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_node_value_type_string, &error);
		append_string(&command_string, " fe_field ", &error);
		field_name = duplicate_string(this->fe_field->getName());
		if (field_name)
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " ", &error);
		append_string(&command_string,
			ENUMERATOR_STRING(cmzn_node_value_label)(this->nodeValueLabel), &error);
		sprintf(temp_string, " version %d", this->versionNumber + 1);
		append_string(&command_string, temp_string, &error);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_node_value::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_node_value::get_command_string */

int Computed_field_node_value::has_multiple_times()
{
	return this->fe_field->checkTimeDependent();
}

} //namespace

cmzn_field_id cmzn_fieldmodule_create_field_node_value(
	cmzn_fieldmodule_id fieldmodule, cmzn_field_id source_field,
	enum cmzn_node_value_label node_value_label, int version_number)
{
	cmzn_field *field = 0;
	FE_field *fe_field = 0;
	if ((fieldmodule) && (source_field) && source_field->isNumerical()
		&& Computed_field_get_type_finite_element(source_field, &fe_field)
		&& ENUMERATOR_STRING(cmzn_node_value_label)(node_value_label)
		&& (version_number > 0))
	{
		field = Computed_field_create_generic(fieldmodule,
			/*check_source_field_regions*/true, get_FE_field_number_of_components(fe_field),
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/0, NULL,
			new Computed_field_node_value(source_field, node_value_label, version_number - 1));
	}
	return field;
}

cmzn_field_node_value_id cmzn_field_cast_node_value(cmzn_field_id field)
{
	if (field && (dynamic_cast<Computed_field_node_value*>(field->core)))
	{
		cmzn_field_access(field);
		return (reinterpret_cast<cmzn_field_node_value_id>(field));
	}
	return nullptr;
}

int cmzn_field_node_value_destroy(
	cmzn_field_node_value_id *node_value_field_address)
{
	return cmzn_field_destroy(reinterpret_cast<cmzn_field_id *>(node_value_field_address));
}

inline Computed_field_node_value *Computed_field_node_value_core_cast(
	cmzn_field_node_value *node_value_field)
{
	return static_cast<Computed_field_node_value*>(
		reinterpret_cast<Computed_field*>(node_value_field)->core);
}

enum cmzn_node_value_label cmzn_field_node_value_get_node_value_label(
	cmzn_field_node_value_id node_value_field)
{
	if (node_value_field)
	{
		Computed_field_node_value *node_value_core = Computed_field_node_value_core_cast(node_value_field);
		return node_value_core->getNodeValueLabel();
	}
	return CMZN_NODE_VALUE_LABEL_INVALID;
}

int cmzn_field_node_value_set_node_value_label(
	cmzn_field_node_value_id node_value_field,
	enum cmzn_node_value_label node_value_label)
{
	if (node_value_field)
	{
		Computed_field_node_value *node_value_core = Computed_field_node_value_core_cast(node_value_field);
		return node_value_core->setNodeValueLabel(node_value_label);
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_field_node_value_get_version_number(
	cmzn_field_node_value_id node_value_field)
{
	if (node_value_field)
	{
		Computed_field_node_value *node_value_core = Computed_field_node_value_core_cast(node_value_field);
		return node_value_core->getVersionNumber() + 1;
	}
	return 0;
}

int cmzn_field_node_value_set_version_number(
	cmzn_field_node_value_id node_value_field, int version_number)
{
	if (node_value_field)
	{
		Computed_field_node_value *node_value_core = Computed_field_node_value_core_cast(node_value_field);
		return node_value_core->setVersionNumber(version_number - 1);
	}
	return CMZN_ERROR_ARGUMENT;
}

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(cmzn_field_edge_discontinuity_measure)
{
	switch (enumerator_value)
	{
		case CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_INVALID:
			break;
		case CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_C1:
			return "measure_c1";
			break;
		case CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_G1:
			return "measure_g1";
			break;
		case CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_SURFACE_NORMAL:
			return "measure_surface_normal";
			break;
	}
	return 0;
}

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(cmzn_field_edge_discontinuity_measure)

namespace {

const char computed_field_edge_discontinuity_type_string[] = "edge_discontinuity";

class Computed_field_edge_discontinuity : public Computed_field_core
{
	cmzn_field_edge_discontinuity_measure measure;
public:
	Computed_field_edge_discontinuity() :
		Computed_field_core(),
		measure(CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_C1)
	{
	};

	virtual void inherit_source_field_attributes()
	{
		if (this->field)
		{
			this->field->copyCoordinateSystemFromSourceField(0, /*notifyChange*/false);
		}
	}

	/** @return non-accessed conditional field, if any */
	cmzn_field *getConditionalField() const
	{
		return (2 == field->number_of_source_fields) ? field->source_fields[1] : 0;
	}

	int setConditionalField(cmzn_field *conditionalField)
	{
		if ((!conditionalField) || Computed_field_is_scalar(conditionalField, 0))
			return this->field->setSourceField(1, conditionalField);
		return CMZN_ERROR_ARGUMENT;
	}

	cmzn_field_edge_discontinuity_measure getMeasure() const
	{
		return this->measure;
	}

	int setMeasure(cmzn_field_edge_discontinuity_measure measureIn)
	{
		if ((measureIn == CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_C1) ||
			(measureIn == CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_G1) ||
			((measureIn == CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_SURFACE_NORMAL) &&
			(3 == getSourceField(0)->number_of_components)))
		{
			if (measureIn != this->measure)
			{
				this->measure = measureIn;
				this->setChanged();
			}
			return CMZN_OK;
		}
		return CMZN_ERROR_ARGUMENT;
	}

	virtual bool is_purely_function_of_field(cmzn_field *other_field)
	{
		return (this->field == other_field);
	}

private:
	Computed_field_core *copy()
	{
		return new Computed_field_edge_discontinuity();
	}

	const char *get_type_string()
	{
		return (computed_field_edge_discontinuity_type_string);
	}

	virtual enum cmzn_field_type get_type()
	{
		return CMZN_FIELD_TYPE_EDGE_DISCONTINUITY;
	}

	int compare(Computed_field_core* other_core)
	{
		return (field && (0 != dynamic_cast<Computed_field_edge_discontinuity*>(other_core)));
	}

	virtual FieldValueCache *createValueCache(cmzn_fieldcache& fieldCache)
	{
		RealFieldValueCache *valueCache = new RealFieldValueCache(field->number_of_components);
		// need extra cache for evaluating at parent element locations
		valueCache->getOrCreateSharedExtraCache(fieldCache);
		return valueCache;
	}

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	virtual int evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative)
	{
		return this->evaluateDerivativeFiniteDifference(cache, inValueCache, fieldDerivative);
	}

	int list();

	char* get_command_string();

	virtual bool is_defined_at_location(cmzn_fieldcache& cache)
	{
		return (0 != field->evaluate(cache));
	}

};

int Computed_field_edge_discontinuity::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	const Field_location_element_xi* element_xi_location = cache.get_location_element_xi();
	if (!element_xi_location)
		return 0;
	cmzn_element *element = element_xi_location->get_element();
	FE_mesh *fe_mesh = element->getMesh();
	if ((!fe_mesh) || (1 != fe_mesh->getDimension()))
		return 0;
	FE_mesh *parentMesh = fe_mesh->getParentMesh();
	if (!parentMesh)
		return 0;
	const FE_value xi = *(element_xi_location->get_xi());
	cmzn_field *sourceField = getSourceField(0);
	const RealFieldValueCache *sourceValueCache;
	const FieldDerivative& fieldDerivative = *fe_mesh->getFieldDerivative(/*order*/1);
	if (this->measure == CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_SURFACE_NORMAL)
		sourceValueCache = RealFieldValueCache::cast(sourceField->evaluateDerivativeTree(cache, fieldDerivative));
	else
		sourceValueCache = RealFieldValueCache::cast(sourceField->evaluate(cache));
	if (!sourceValueCache)
		return 0;
	const FE_value *sourceValues = sourceValueCache->values;
	RealFieldValueCache& valueCache = RealFieldValueCache::cast(inValueCache);
	cmzn_fieldcache& extraCache = *valueCache.getExtraCache();
	extraCache.setTime(cache.getTime());

	cmzn_field *conditionalField = this->getConditionalField();
	const DsLabelIndex elementIndex = get_FE_element_index(element);
	const DsLabelIndex *parents = 0;
	int parentsCount = fe_mesh->getElementParents(elementIndex, parents);
	if (parentsCount < 2) // must be at least 2 parents to work.
		parentsCount = 0;
	FE_value parentXi[2];
	const int numberOfComponents = field->number_of_components;
	// Future: put in type-specific value cache to save allocations here
	RealFieldValueCache parent1SourceValueCache(numberOfComponents);
	RealFieldValueCache parent2SourceValueCache(numberOfComponents);
	RealFieldValueCache *parentSourceValueCaches[2] =
		{ &parent1SourceValueCache, &parent2SourceValueCache };
	const FieldDerivative& parentFieldDerivative = *parentMesh->getFieldDerivative(/*order*/1);
	for (int i = 0; i < 2; ++i)
		if (!(parentSourceValueCaches[i]->getOrCreateDerivativeValueCache(parentFieldDerivative, *element_xi_location)))
			return 0;
	const FE_value *elementToParentsXi[2];
	int qualifyingParents = 0;
	for (int i = 0; (i < parentsCount) && (qualifyingParents < 2); ++i)
	{
		cmzn_element *parent = parentMesh->getElement(parents[i]);
		if (!parent)
			continue;
		int face_number = parentMesh->getElementFaceNumber(parents[i], elementIndex);
		FE_element_shape *parentShape = get_FE_element_shape(parent);
		const FE_value *elementToParentXi = get_FE_element_shape_face_to_element(parentShape, face_number);
		if (!elementToParentXi)
		{
			qualifyingParents = 0;
			break;
		}
		parentXi[0] = elementToParentXi[0] + elementToParentXi[1]*xi;
		parentXi[1] = elementToParentXi[2] + elementToParentXi[3]*xi;
		extraCache.setMeshLocation(parent, parentXi);
		if (conditionalField)
		{
			const RealFieldValueCache *conditionalValueCache = RealFieldValueCache::cast(conditionalField->evaluate(extraCache));
			if ((!conditionalValueCache) || (0.0 == conditionalValueCache->values[0]))
				continue;
		}
		const RealFieldValueCache *parentSourceValueCache =
			RealFieldValueCache::cast(sourceField->evaluateDerivativeTree(extraCache, parentFieldDerivative));
		if (!parentSourceValueCache)
			continue;
		parentSourceValueCaches[qualifyingParents]->copyValuesAndDerivatives(*parentSourceValueCache);

		// face-to-element map matches either xi or (1.0 - xi), try latter and see which is closer to line value
		parentXi[0] = elementToParentXi[0] + elementToParentXi[1]*(1.0 - xi);
		parentXi[1] = elementToParentXi[2] + elementToParentXi[3]*(1.0 - xi);
		extraCache.setMeshLocation(parent, parentXi);
		parentSourceValueCache = RealFieldValueCache::cast(sourceField->evaluateDerivativeTree(extraCache, parentFieldDerivative));

		FE_value sqdiff1 = 0.0, sqdiff2 = 0.0;
		for (int n = 0; n < numberOfComponents; ++n)
		{
			FE_value diff1 = parentSourceValueCaches[qualifyingParents]->values[n] - sourceValues[n];
			sqdiff1 += diff1*diff1;
			FE_value diff2 = parentSourceValueCache->values[n] - sourceValues[n];
			sqdiff2 += diff2*diff2;
		}
		if (sqdiff2 < sqdiff1)
			parentSourceValueCaches[qualifyingParents]->copyValuesAndDerivatives(*parentSourceValueCache);
		elementToParentsXi[qualifyingParents] = elementToParentXi;
		++qualifyingParents;
	}
	if (2 == qualifyingParents)
	{
		for (int p = 0; p < 2; ++p)
		{
			RealFieldValueCache& parentSourceValueCache = *(parentSourceValueCaches[p]);
			const FE_value *parentDerivatives = parentSourceValueCache.getDerivativeValueCache(parentFieldDerivative)->values;
			const FE_value *elementToParentXi = elementToParentsXi[p];
			if (0.0 == elementToParentXi[1])
			{
				// parent lateral direction = xi1;
				if (0.0 == elementToParentXi[0]) // away from edge
					for (int n = 0; n < numberOfComponents; ++n)
						parentSourceValueCache.values[n] = parentDerivatives[n*2];
				else
					for (int n = 0; n < numberOfComponents; ++n)
						parentSourceValueCache.values[n] = -parentDerivatives[n*2];
			}
			else if (0.0 == elementToParentXi[3])
			{
				// parent lateral direction = xi2;
				if (0.0 == elementToParentXi[2]) // away from edge
					for (int n = 0; n < numberOfComponents; ++n)
						parentSourceValueCache.values[n] = parentDerivatives[n*2 + 1];
				else
					for (int n = 0; n < numberOfComponents; ++n)
						parentSourceValueCache.values[n] = -parentDerivatives[n*2 + 1];
			}
			else
			{
				// simplex - get derivative w.r.t. extra barycentric coordinate: 1 - xi1 - xi2
				for (int n = 0; n < numberOfComponents; ++n)
					parentSourceValueCache.values[n] = -parentDerivatives[n*2] - parentDerivatives[n*2 + 1];
			}
			if (this->measure == CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_SURFACE_NORMAL)
			{
				// requires source field to have 3 components
				const FE_value *dx_dxi1 = sourceValueCache->getDerivativeValueCache(fieldDerivative)->values;
				const FE_value dx_dxi2[3] =
				{
					parentSourceValueCache.values[0],
					parentSourceValueCache.values[1],
					parentSourceValueCache.values[2]
				};
				parentSourceValueCache.values[0] = dx_dxi1[1]*dx_dxi2[2] - dx_dxi2[1]*dx_dxi1[2];
				parentSourceValueCache.values[1] = dx_dxi1[2]*dx_dxi2[0] - dx_dxi2[2]*dx_dxi1[0];
				parentSourceValueCache.values[2] = dx_dxi1[0]*dx_dxi2[1] - dx_dxi2[0]*dx_dxi1[1];
			}
			if (this->measure != CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_C1)
			{
				// normalise derivatives
				FE_value sumsq = 0.0;
				for (int n = 0; n < numberOfComponents; ++n)
					sumsq += parentSourceValueCache.values[n]*parentSourceValueCache.values[n];
				if (sumsq > 0.0)
				{
					FE_value factor = sqrt(sumsq);
					for (int n = 0; n < numberOfComponents; ++n)
						parentSourceValueCache.values[n] /= factor;
				}
			}
		}
		// since the above code ensures tangents/normals for each adjacent element are in
		// opposite directions, the difference is just their sum:
		for (int n = 0; n < numberOfComponents; ++n)
			valueCache.values[n] = parent1SourceValueCache.values[n] + parent2SourceValueCache.values[n];
	}
	else
	{
		for (int n = 0; n < numberOfComponents; ++n)
			valueCache.values[n] = 0.0;
	}
	return 1;
}

int Computed_field_edge_discontinuity::list()
{
	int return_code;
	if (field)
	{
		display_message(INFORMATION_MESSAGE, "    source field : %s\n", field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE, "    measure : %s\n", ENUMERATOR_STRING(cmzn_field_edge_discontinuity_measure)(this->measure));
		cmzn_field *conditionalField = this->getConditionalField();
		if (conditionalField)
			display_message(INFORMATION_MESSAGE, "    conditional field : %s\n", conditionalField->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_edge_discontinuity.  Invalid arguments.");
		return_code = 0;
	}
	return (return_code);
}

/** Returns allocated command string for reproducing field. Includes type. */
char *Computed_field_edge_discontinuity::get_command_string()
{
	char *command_string = 0;
	if (field)
	{
		int error = 0;
		append_string(&command_string,
			computed_field_edge_discontinuity_type_string, &error);
		append_string(&command_string, " source_field ", &error);
		char *field_name = cmzn_field_get_name(field->source_fields[0]);
		make_valid_token(&field_name);
		append_string(&command_string, field_name, &error);
		DEALLOCATE(field_name);
		append_string(&command_string, " ", &error);
		append_string(&command_string, ENUMERATOR_STRING(cmzn_field_edge_discontinuity_measure)(this->measure), &error);
		cmzn_field *conditionalField = this->getConditionalField();
		if (conditionalField)
		{
			append_string(&command_string, " conditional_field ", &error);
			field_name = cmzn_field_get_name(conditionalField);
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_edge_discontinuity::get_command_string.  Invalid field");
	}
	return (command_string);
}

} //namespace

struct cmzn_field_edge_discontinuity : public Computed_field
{
	inline Computed_field_edge_discontinuity *get_core()
	{
		return static_cast<Computed_field_edge_discontinuity*>(core);
	}
};

cmzn_field_id cmzn_fieldmodule_create_field_edge_discontinuity(
	cmzn_fieldmodule_id fieldmodule, cmzn_field_id source_field)
{
	struct Computed_field *field = 0;
	if ((fieldmodule) && (source_field) && source_field->isNumerical())
	{
		field = Computed_field_create_generic(fieldmodule,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/1, &source_field,
			/*number_of_source_values*/0, NULL,
			new Computed_field_edge_discontinuity());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_fieldmodule_create_field_edge_discontinuity.  Invalid argument(s)");
	}
	return (field);
}

cmzn_field_edge_discontinuity_id cmzn_field_cast_edge_discontinuity(cmzn_field_id field)
{
	if (field && dynamic_cast<Computed_field_edge_discontinuity*>(field->core))
	{
		cmzn_field_access(field);
		return (reinterpret_cast<cmzn_field_edge_discontinuity_id>(field));
	}
	return 0;
}

int cmzn_field_edge_discontinuity_destroy(
	cmzn_field_edge_discontinuity_id *edge_discontinuity_field_address)
{
	return cmzn_field_destroy(reinterpret_cast<cmzn_field_id *>(edge_discontinuity_field_address));
}

cmzn_field_id cmzn_field_edge_discontinuity_get_conditional_field(
	cmzn_field_edge_discontinuity_id edge_discontinuity_field)
{
	if (edge_discontinuity_field)
		return cmzn_field_access(edge_discontinuity_field->get_core()->getConditionalField());
	return 0;
}

int cmzn_field_edge_discontinuity_set_conditional_field(
	cmzn_field_edge_discontinuity_id edge_discontinuity_field,
	cmzn_field_id conditional_field)
{
	if (edge_discontinuity_field)
		return edge_discontinuity_field->get_core()->setConditionalField(conditional_field);
	return CMZN_ERROR_ARGUMENT;
}

cmzn_field_edge_discontinuity_measure cmzn_field_edge_discontinuity_get_measure(
	cmzn_field_edge_discontinuity_id edge_discontinuity_field)
{
	if (edge_discontinuity_field)
		return edge_discontinuity_field->get_core()->getMeasure();
	return CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_INVALID;
}

int cmzn_field_edge_discontinuity_set_measure(
	cmzn_field_edge_discontinuity_id edge_discontinuity_field,
	cmzn_field_edge_discontinuity_measure measure)
{
	if (edge_discontinuity_field)
		return edge_discontinuity_field->get_core()->setMeasure(measure);
	return CMZN_ERROR_ARGUMENT;
}

namespace {

const char computed_field_embedded_type_string[] = "embedded";

class Computed_field_embedded : public Computed_field_core
{
public:
	Computed_field_embedded() :
		Computed_field_core()
	{
	};

	virtual void inherit_source_field_attributes()
	{
		if (this->field)
		{
			this->field->copyCoordinateSystemFromSourceField(0, /*notifyChange*/false);
		}
	}

	virtual bool is_purely_function_of_field(cmzn_field *other_field)
	{
		if (this->field == other_field)
			return true;
		return this->field->source_fields[1]->core->is_purely_function_of_field(other_field);
	}

private:
	Computed_field_core *copy();

	const char *get_type_string()
	{
		return(computed_field_embedded_type_string);
	}

	virtual enum cmzn_field_type get_type()
	{
		return CMZN_FIELD_TYPE_EMBEDDED;
	}

	int compare(Computed_field_core* other_field);

	virtual FieldValueCache *createValueCache(cmzn_fieldcache& fieldCache)
	{
		RealFieldValueCache *valueCache = new RealFieldValueCache(field->number_of_components);
		valueCache->getOrCreateSharedExtraCache(fieldCache);
		return valueCache;
	}

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	virtual int evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative);

	virtual int getDerivativeTreeOrder(const FieldDerivative& fieldDerivative)
	{
		return fieldDerivative.getProductTreeOrder(
			this->field->source_fields[0]->getDerivativeTreeOrder(fieldDerivative),
			this->field->source_fields[1]->getDerivativeTreeOrder(fieldDerivative));
	}

	int list();

	char* get_command_string();

	virtual bool is_defined_at_location(cmzn_fieldcache& cache);

	int has_numerical_components();

	virtual int check_dependency()
	{
		if (field)
		{
			if (0 == (field->manager_change_status & MANAGER_CHANGE_FULL_RESULT(Computed_field)))
			{
				// any change to result of source field is a full change to the embedded field
				int source_change_status = getSourceField(0)->core->check_dependency();
				if (source_change_status & MANAGER_CHANGE_RESULT(Computed_field))
					field->setChangedPrivate(MANAGER_CHANGE_FULL_RESULT(Computed_field));
				else
				{
					// propagate full or partial result from mesh location field
					source_change_status = getSourceField(1)->core->check_dependency();
					if (source_change_status & MANAGER_CHANGE_FULL_RESULT(Computed_field))
						field->setChangedPrivate(MANAGER_CHANGE_FULL_RESULT(Computed_field));
					else if (source_change_status & MANAGER_CHANGE_PARTIAL_RESULT(Computed_field))
						field->setChangedPrivate(MANAGER_CHANGE_PARTIAL_RESULT(Computed_field));
				}
			}
			return field->manager_change_status;
		}
		return MANAGER_CHANGE_NONE(Computed_field);
	}

};

Computed_field_core* Computed_field_embedded::copy()
{
	return new Computed_field_embedded();
}

int Computed_field_embedded::compare(Computed_field_core *other_core)
{
	return (field && (0 != dynamic_cast<Computed_field_embedded*>(other_core)));
}

bool Computed_field_embedded::is_defined_at_location(cmzn_fieldcache& cache)
{
	return (0 != field->evaluate(cache));
}

int Computed_field_embedded::has_numerical_components()
{
	return ((field) && Computed_field_has_numerical_components(
		field->source_fields[0], (void *)NULL));
}

int Computed_field_embedded::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	// assumes mesh-location valued fields all create MeshLocationFieldValueCache
	const MeshLocationFieldValueCache *meshLocationValueCache = MeshLocationFieldValueCache::cast(getSourceField(1)->evaluate(cache));
	if (meshLocationValueCache)
	{
		RealFieldValueCache& valueCache = RealFieldValueCache::cast(inValueCache);
		cmzn_fieldcache& extraCache = *valueCache.getExtraCache();
		extraCache.setMeshLocation(meshLocationValueCache->element, meshLocationValueCache->xi);
		extraCache.setTime(cache.getTime());
		const RealFieldValueCache *sourceValueCache = RealFieldValueCache::cast(getSourceField(0)->evaluate(extraCache));
		if (sourceValueCache)
		{
			valueCache.copyValues(*sourceValueCache);
			return 1;
		}
	}
	return 0;
}

// implemented for parameter derivatives of constant mesh location, used in fitting
int Computed_field_embedded::evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative)
{
	// handle xi not interpreted on the host element, or expression too complex
	if ((fieldDerivative.getMeshOrder() > 0) || (this->field->source_fields[1]->getDerivativeTreeOrder(fieldDerivative) > 0))
		return this->evaluateDerivativeFiniteDifference(cache, inValueCache, fieldDerivative);
	const Field_location_node *fieldLocationNode = cache.get_location_node();
	if ((!fieldLocationNode) || (!fieldLocationNode->get_host_element()))
		return 0;  // can only evaluate at node locations embedded in an element; see check below
	const MeshLocationFieldValueCache *meshLocationValueCache = MeshLocationFieldValueCache::cast(getSourceField(1)->evaluate(cache));
	if (!meshLocationValueCache)
		return 0;
	if (meshLocationValueCache->element != fieldLocationNode->get_host_element())
		return this->evaluateDerivativeFiniteDifference(cache, inValueCache, fieldDerivative);
	RealFieldValueCache& valueCache = RealFieldValueCache::cast(inValueCache);
	cmzn_fieldcache& extraCache = *valueCache.getExtraCache();
	extraCache.setMeshLocation(meshLocationValueCache->element, meshLocationValueCache->xi);
	extraCache.setTime(cache.getTime());
	const DerivativeValueCache *sourceDerivativeCache = getSourceField(0)->evaluateDerivative(extraCache, fieldDerivative);
	if (!sourceDerivativeCache)
		return 0;
	DerivativeValueCache *derivativeCache = inValueCache.getDerivativeValueCache(fieldDerivative);
	derivativeCache->copyValues(*sourceDerivativeCache);
	return 1;
}

int Computed_field_embedded::list()
{
	int return_code;
	if (field)
	{
		display_message(INFORMATION_MESSAGE, "    embedded location field : %s\n", field->source_fields[1]->name);
		display_message(INFORMATION_MESSAGE, "    source field : %s\n", field->source_fields[0]->name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_embedded.  Invalid arguments.");
		return_code = 0;
	}
	return (return_code);
}

/** Returns allocated command string for reproducing field. Includes type. */
char *Computed_field_embedded::get_command_string()
{
	char *command_string, *field_name;
	int error;

	ENTER(Computed_field_embedded::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		error = 0;
		append_string(&command_string,
			computed_field_embedded_type_string, &error);
		append_string(&command_string, " element_xi ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_embedded::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
}

} //namespace

cmzn_field_id cmzn_fieldmodule_create_field_embedded(
	cmzn_fieldmodule_id fieldmodule, cmzn_field_id source_field,
	cmzn_field_id embedded_location_field)
{
	struct Computed_field *field = 0;
	if ((fieldmodule) && (source_field) && source_field->isNumerical() &&
		(embedded_location_field) && (CMZN_FIELD_VALUE_TYPE_MESH_LOCATION == 
			embedded_location_field->getValueType()))
	{
		cmzn_field_id source_fields[2];
		source_fields[0] = source_field;
		source_fields[1] = embedded_location_field;
		field = Computed_field_create_generic(fieldmodule,
			/*check_source_field_regions*/true,
			source_field->number_of_components,
			/*number_of_source_fields*/2, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_embedded());
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_fieldmodule_create_field_embedded.  Invalid argument(s)");
	}
	return (field);
}

/** If the field is of type COMPUTED_FIELD_EMBEDDED, returns the fields it uses. */
int Computed_field_get_type_embedded(struct Computed_field *field,
	struct Computed_field **source_field_address,
	struct Computed_field **embedded_location_field_address)
{
	int return_code = 0;
	if (field && (0 != dynamic_cast<Computed_field_embedded*>(field->core)))
	{
		*source_field_address = field->source_fields[0];
		*embedded_location_field_address = field->source_fields[1];
		return_code = 1;
	}
	return (return_code);
}

namespace {

class Computed_field_find_mesh_location;

// Has a custom implementation of the clear() method to ensure find element xi cache
// is cleared. Stores owning field to support this; should all FieldValueCaches do this?
class FindMeshLocationFieldValueCache : public MeshLocationFieldValueCache
{
	Computed_field_find_element_xi_cache *findElementXiCache;

public:

	FindMeshLocationFieldValueCache() :
		MeshLocationFieldValueCache(),
		findElementXiCache(nullptr)
	{
	}

	~FindMeshLocationFieldValueCache()
	{
		if (this->findElementXiCache)
		{
			delete this->findElementXiCache;
			this->findElementXiCache = nullptr;
		}
	}

	virtual void clear()
	{
		if (this->findElementXiCache)
		{
			delete this->findElementXiCache;
			this->findElementXiCache = nullptr;
		}
		MeshLocationFieldValueCache::clear();
	}

	Computed_field_find_element_xi_cache *getFindElementXiCache(cmzn_field *meshField)
	{
		if (!this->findElementXiCache)
		{
			this->findElementXiCache = new Computed_field_find_element_xi_cache(meshField);
		}
		return this->findElementXiCache;
	}

	static const FindMeshLocationFieldValueCache* cast(const FieldValueCache* valueCache)
	{
		return FIELD_VALUE_CACHE_CAST<const FindMeshLocationFieldValueCache*>(valueCache);
	}

	static FindMeshLocationFieldValueCache* cast(FieldValueCache* valueCache)
	{
		return FIELD_VALUE_CACHE_CAST<FindMeshLocationFieldValueCache*>(valueCache);
	}

	static const FindMeshLocationFieldValueCache& cast(const FieldValueCache& valueCache)
	{
		return FIELD_VALUE_CACHE_CAST<const FindMeshLocationFieldValueCache&>(valueCache);
	}

	static FindMeshLocationFieldValueCache& cast(FieldValueCache& valueCache)
	{
		return FIELD_VALUE_CACHE_CAST<FindMeshLocationFieldValueCache&>(valueCache);
	}
};

const char computed_field_find_mesh_location_type_string[] = "find_mesh_location";

class Computed_field_find_mesh_location : public Computed_field_core
{
private:
	cmzn_mesh *mesh;  // mesh for storing locations in
	cmzn_mesh *searchMesh;  // mesh for search for locations e.g. subset/faces
	enum cmzn_field_find_mesh_location_search_mode searchMode;
	FeMeshFieldRangesCache *meshFieldRangesCache;
	FeMeshFieldRanges *meshFieldRanges;

	Computed_field_find_mesh_location();  // not defined

public:

	Computed_field_find_mesh_location(cmzn_mesh *mesh) :
		Computed_field_core(),
		mesh(cmzn_mesh_access(mesh)),
		searchMesh(cmzn_mesh_access(mesh)),
		searchMode(CMZN_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_EXACT),
		meshFieldRangesCache(nullptr),
		meshFieldRanges(nullptr)
	{
	};

	Computed_field_find_mesh_location(const Computed_field_find_mesh_location& source) :
		Computed_field_core(),
		mesh(cmzn_mesh_access(source.mesh)),
		searchMesh(cmzn_mesh_access(source.searchMesh)),
		searchMode(source.searchMode),
		meshFieldRangesCache(nullptr),
		meshFieldRanges(nullptr)
	{
	};

	virtual bool attach_to_field(cmzn_field *parent)
	{
		if (Computed_field_core::attach_to_field(parent))
		{
			this->updateMeshFieldRanges();
			return true;
		}
		return false;
	}

	virtual ~Computed_field_find_mesh_location()
	{
		FeMeshFieldRanges::deaccess(this->meshFieldRanges);
		FeMeshFieldRangesCache::deaccess(this->meshFieldRangesCache);
		cmzn_mesh_destroy(&this->mesh);
		cmzn_mesh_destroy(&this->searchMesh);
	}

	virtual void inherit_source_field_attributes()
	{
		if (this->field)
		{
			Coordinate_system coordinateSystem(NOT_APPLICABLE);
			this->field->setCoordinateSystem(coordinateSystem, /*notifyChange*/false);
		}
	}

	/** @return  Non-accessed field */
	cmzn_field *getSourceField()
	{
		return field->source_fields[0];
	}

	virtual enum cmzn_field_type get_type()
	{
		return CMZN_FIELD_TYPE_FIND_MESH_LOCATION;
	}

	/** @return  Non-accessed field */
	cmzn_field *getMeshField()
	{
		return this->field->source_fields[1];
	}

	/** @return  Non accessed mesh */
	cmzn_mesh *getMesh()
	{
		return this->mesh;
	}

	/** @return  Non-accessed object storing ranges of mesh field in elements of mesh */
	FeMeshFieldRanges *getMeshFieldRanges() const
	{
		return this->meshFieldRanges;
	}

	/** @return  Non-accessed search mesh */
	cmzn_mesh *getSearchMesh()
	{
		return this->searchMesh;
	}

	int setSearchMesh(cmzn_mesh *searchMeshIn)
	{
		if (searchMeshIn != this->searchMesh)
		{
			FE_mesh *feMesh = mesh->getFeMesh();
			FE_mesh *feSearchMesh = searchMeshIn->getFeMesh();
			if ((!searchMeshIn)
				|| (feSearchMesh->get_FE_region() != feMesh->get_FE_region())
				|| (feSearchMesh->getDimension() > feMesh->getDimension()))
			{
				display_message(ERROR_MESSAGE, "FieldFindMeshLocation setSearchMesh  Invalid search mesh");
				return CMZN_ERROR_ARGUMENT;
			}
			cmzn_mesh_access(searchMeshIn);
			cmzn_mesh_destroy(&this->searchMesh);
			this->searchMesh = searchMeshIn;
			this->updateMeshFieldRanges();
			this->field->setChanged();
		}
		return CMZN_OK;
	}

	enum cmzn_field_find_mesh_location_search_mode getSearchMode() const
	{
		return this->searchMode;
	}

	int setSearchMode(enum cmzn_field_find_mesh_location_search_mode searchModeIn)
	{
		if (searchModeIn != this->searchMode)
		{
			this->searchMode = searchModeIn;
			this->field->setChanged();
		}
		return CMZN_OK;
	}

	virtual bool is_purely_function_of_field(cmzn_field *other_field)
	{
		if (this->field == other_field)
			return true;
		return this->getSourceField()->core->is_purely_function_of_field(other_field);
	}

private:
	Computed_field_core *copy();

	const char *get_type_string()
	{
		return (computed_field_find_mesh_location_type_string);
	}

	int compare(Computed_field_core* other_field);

	virtual FieldValueCache *createValueCache(cmzn_fieldcache& fieldCache)
	{
		FindMeshLocationFieldValueCache *valueCache = new FindMeshLocationFieldValueCache();
		valueCache->getOrCreateSharedExtraCache(fieldCache);
		return valueCache;
	}

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	virtual int evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative)
	{
		return 0;  // non-numeric
	}

	int list();

	char* get_command_string();

	virtual bool is_defined_at_location(cmzn_fieldcache& cache);

	int has_numerical_components()
	{
		return 0;
	}

	virtual cmzn_field_value_type get_value_type() const
	{
		return CMZN_FIELD_VALUE_TYPE_MESH_LOCATION;
	}

	// if the mesh or search mesh is a group, also need to propagate changes from it
	// also, if the mesh field or mesh or search mesh group have changed, trigger re-evaluation of mesh field ranges
	virtual int check_dependency()
	{
		int return_code = Computed_field_core::check_dependency();
		// have to call check_dependency because Computed_field_core::check_dependency will not
		// call it if any earlier field is MANAGER_CHANGE_FULL_RESULT
		// must compare with MANAGER_CHANGE_RESULT as may be MANAGER_CHANGE_PARTIAL_RESULT
		cmzn_field *meshField = this->getMeshField();
		if ((meshField->manager_change_status & MANAGER_CHANGE_RESULT(Computed_field)) ||
			(meshField->core->check_dependency() & MANAGER_CHANGE_RESULT(Computed_field)) ||
			this->mesh->hasMembershipChanges() ||
			this->searchMesh->hasMembershipChanges())
		{
			this->meshFieldRangesCache->clearAllRanges();
			// this implies a full change
			this->field->setChangedPrivate(MANAGER_CHANGE_FULL_RESULT(Computed_field));
			return_code = this->field->manager_change_status;
		}
		return return_code;
	}

	// call if search mesh or mesh field are changed to get new mesh field ranges/cache
	void updateMeshFieldRanges()
	{
		// get new mesh field ranges cache and ranges before releasing old ones in case they are the same
		FeMeshFieldRangesCache *newMeshFieldRangesCache = this->searchMesh->getFeMesh()->getFeMeshFieldRangesCache(this->getMeshField());
		FeMeshFieldRanges *newMeshFieldRanges = newMeshFieldRangesCache->getMeshFieldRanges(this->searchMesh);
		FeMeshFieldRangesCache::deaccess(this->meshFieldRangesCache);
		FeMeshFieldRanges::deaccess(this->meshFieldRanges);
		this->meshFieldRangesCache = newMeshFieldRangesCache;
		this->meshFieldRanges = newMeshFieldRanges;
	}

};

Computed_field_core* Computed_field_find_mesh_location::copy()
{
	return new Computed_field_find_mesh_location(*this);
}

int Computed_field_find_mesh_location::compare(Computed_field_core *other_core)
{
	Computed_field_find_mesh_location* other;
	int return_code = 0;
	if (field && (other = dynamic_cast<Computed_field_find_mesh_location*>(other_core)))
	{
		return_code = (mesh == other->mesh);
	}
	return (return_code);
}

bool Computed_field_find_mesh_location::is_defined_at_location(cmzn_fieldcache& cache)
{
	return (0 != field->evaluate(cache));
}

int Computed_field_find_mesh_location::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	const RealFieldValueCache *sourceValueCache = RealFieldValueCache::cast(getSourceField()->evaluate(cache));
	if (!sourceValueCache)
		return 0;
	FindMeshLocationFieldValueCache& findMeshLocationValueCache = FindMeshLocationFieldValueCache::cast(inValueCache);
	findMeshLocationValueCache.clearElement();
	cmzn_fieldcache& extraCache = *findMeshLocationValueCache.getExtraCache();
	extraCache.setTime(cache.getTime());
	cmzn_element *element;
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	cmzn_field *meshField = this->getMeshField();
	Computed_field_find_element_xi_cache *findElementXiCache =
		findMeshLocationValueCache.getFindElementXiCache(meshField);
	FeMeshFieldRanges *meshFieldRanges = this->meshFieldRanges;
	// while the mesh field has changes the ranges are not used
	// changes to groups are fine: deletion of elements immediately removes them from any groups, and their ranges from mesh field ranges
	// Any elements added to groups won't have a range so will fallback to the original find xi algorithm
	// the evaluated flag is cleared when FindMeshLocation field is notified of change to either mesh field or search mesh group
	if (meshField->isResultChanged())
	{
		// mesh field ranges are invalid while mesh field has unnotified result changes
		meshFieldRanges = nullptr;
	}
	else if (!meshFieldRanges->isEvaluated())
	{
		this->meshFieldRangesCache->evaluateMeshFieldRanges(extraCache, meshFieldRanges);
	}
	if (!(Computed_field_find_element_xi(meshField, &extraCache,
		findElementXiCache, meshFieldRanges, sourceValueCache->values,
		sourceValueCache->componentCount, &element, xi, this->searchMesh,
		/*find_nearest*/(this->searchMode != CMZN_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_EXACT))
		&& (element)))
	{
		return 0;
	}
	FE_mesh *searchFeMesh = this->searchMesh->getFeMesh();
	FE_mesh *ancestorFeMesh = this->mesh->getFeMesh();
	if (searchFeMesh != ancestorFeMesh)
	{
		FE_value meshCoordinates[3];
		assert(meshField->getNumberOfComponents() <= 3);
		extraCache.setMeshLocation(element, xi);
		cmzn_field_evaluate_real(meshField, &extraCache, 3, meshCoordinates);
		cmzn_mesh_group* meshGroup = dynamic_cast<cmzn_mesh_group*>(this->mesh);
		const DsLabelsGroup* ancestorLabelsGroup = (meshGroup) ? meshGroup->getLabelsGroup() : nullptr;
		FE_value ancestorXi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
		DsLabelIndex ancestorElementIndex = searchFeMesh->convertLocationToAncestor(
			element->getIndex(), xi, meshField, meshCoordinates, &extraCache,
			ancestorFeMesh, ancestorLabelsGroup, ancestorXi);
		if (ancestorElementIndex < 0)
		{
			return 0;
		}
		element = ancestorFeMesh->getElement(ancestorElementIndex);
		for (int k = 0; k < ancestorFeMesh->getDimension(); ++k)
		{
			xi[k] = ancestorXi[k];
		}
	}
	findMeshLocationValueCache.setMeshLocation(element, xi);
	return 1;
}

int Computed_field_find_mesh_location::list()
{
	int return_code = 0;
	if (field)
	{
		display_message(INFORMATION_MESSAGE, "    search mode : ");
		if (this->searchMode == CMZN_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_NEAREST)
		{
			display_message(INFORMATION_MESSAGE, " find_nearest\n");
		}
		else
		{
			display_message(INFORMATION_MESSAGE, " find_exact\n");
		}
		display_message(INFORMATION_MESSAGE, "    mesh : ");
		char *mesh_name = cmzn_mesh_get_name(this->mesh);
		display_message(INFORMATION_MESSAGE, "%s\n", mesh_name);
		DEALLOCATE(mesh_name);

		display_message(INFORMATION_MESSAGE, "    search mesh : ");
		mesh_name = cmzn_mesh_get_name(this->searchMesh);
		display_message(INFORMATION_MESSAGE, "%s\n", mesh_name);
		DEALLOCATE(mesh_name);

		display_message(INFORMATION_MESSAGE,
			"    mesh field : %s\n", getMeshField()->name);
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n", getSourceField()->name);
		return_code = 1;
	}
	return (return_code);
}

/** Returns allocated command string for reproducing field. Includes type */
char *Computed_field_find_mesh_location::get_command_string()
{
	char *command_string = 0;
	int error = 0;
	if (field)
	{
		append_string(&command_string, computed_field_find_mesh_location_type_string, &error);

		if (this->searchMode == CMZN_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_NEAREST)
		{
			append_string(&command_string, " find_nearest", &error);
		}
		else
		{
			append_string(&command_string, " find_exact", &error);
		}

		append_string(&command_string, " mesh ", &error);
		char *mesh_name = cmzn_mesh_get_name(this->mesh);
		make_valid_token(&mesh_name);
		append_string(&command_string, mesh_name, &error);
		DEALLOCATE(mesh_name);

		append_string(&command_string, " search_mesh ", &error);
		mesh_name = cmzn_mesh_get_name(this->searchMesh);
		make_valid_token(&mesh_name);
		append_string(&command_string, mesh_name, &error);
		DEALLOCATE(mesh_name);

		char *mesh_field_name = cmzn_field_get_name(this->getMeshField());
		make_valid_token(&mesh_field_name);
		append_string(&command_string, " mesh_field ", &error);
		append_string(&command_string, mesh_field_name, &error);
		DEALLOCATE(mesh_field_name);

		char *source_field_name = cmzn_field_get_name(this->getSourceField());
		make_valid_token(&source_field_name);
		append_string(&command_string, " source_field ", &error);
		append_string(&command_string, source_field_name, &error);
		DEALLOCATE(source_field_name);
	}
	return (command_string);
}

} // namespace

cmzn_field_id cmzn_fieldmodule_create_field_find_mesh_location(
	cmzn_fieldmodule_id fieldmodule, cmzn_field_id source_field,
	cmzn_field_id mesh_field, cmzn_mesh_id mesh)
{
	cmzn_field *field = nullptr;
	if ((fieldmodule) &&
		(source_field) && source_field->isNumerical() &&
		(mesh_field) && mesh_field->isNumerical() &&
		(source_field->getNumberOfComponents() == mesh_field->getNumberOfComponents()) &&
		(mesh) && (mesh->getRegion() ==
			cmzn_fieldmodule_get_region_internal(fieldmodule)) &&
		(mesh_field->getNumberOfComponents() >= cmzn_mesh_get_dimension(mesh)))
	{
		cmzn_field_id source_fields[2];
		source_fields[0] = source_field;
		source_fields[1] = mesh_field;
		field = Computed_field_create_generic(fieldmodule,
			/*check_source_field_regions*/true,
			/*number_of_components*/1,
			/*number_of_source_fields*/2, source_fields,
			/*number_of_source_values*/0, NULL,
			new Computed_field_find_mesh_location(mesh));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_fieldmodule_create_field_find_mesh_location.  Invalid argument(s)");
	}
	return (field);
}

struct cmzn_field_find_mesh_location : private Computed_field
{
	inline Computed_field_find_mesh_location *get_core()
	{
		return static_cast<Computed_field_find_mesh_location*>(core);
	}
};

cmzn_field_find_mesh_location_id cmzn_field_cast_find_mesh_location(
	cmzn_field_id field)
{
	if (field)
	{
		if (dynamic_cast<Computed_field_find_mesh_location*>(field->core))
		{
			cmzn_field_access(field);
			return (reinterpret_cast<cmzn_field_find_mesh_location_id>(field));
		}
	}
	return 0;
}

int cmzn_field_find_mesh_location_destroy(
	cmzn_field_find_mesh_location_id *find_mesh_location_field_address)
{
	return cmzn_field_destroy(reinterpret_cast<cmzn_field_id *>(find_mesh_location_field_address));
}

cmzn_mesh_id cmzn_field_find_mesh_location_get_mesh(
	cmzn_field_find_mesh_location_id find_mesh_location_field)
{
	if (find_mesh_location_field)
		return cmzn_mesh_access(find_mesh_location_field->get_core()->getMesh());
	return nullptr;
}

cmzn_mesh_id cmzn_field_find_mesh_location_get_search_mesh(
	cmzn_field_find_mesh_location_id find_mesh_location_field)
{
	if (find_mesh_location_field)
		return cmzn_mesh_access(find_mesh_location_field->get_core()->getSearchMesh());
	return nullptr;
}

int cmzn_field_find_mesh_location_set_search_mesh(
	cmzn_field_find_mesh_location_id find_mesh_location_field,
	cmzn_mesh_id search_mesh)
{
	if (find_mesh_location_field)
		return find_mesh_location_field->get_core()->setSearchMesh(search_mesh);
	return CMZN_ERROR_ARGUMENT;
}


class cmzn_field_find_mesh_location_search_mode_conversion
{
public:
	static const char *to_string(enum cmzn_field_find_mesh_location_search_mode mode)
	{
		switch (mode)
		{
		case CMZN_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_INVALID:
			break;
		case CMZN_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_EXACT:
			return "EXACT";
			break;
		case CMZN_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_NEAREST:
			return "NEAREST";
			break;
		default:
			break;
		}
		return nullptr;
	}
};

enum cmzn_field_find_mesh_location_search_mode
	cmzn_field_find_mesh_location_search_mode_enum_from_string(const char *name)
{
	return string_to_enum<enum cmzn_field_find_mesh_location_search_mode,
		cmzn_field_find_mesh_location_search_mode_conversion>(name);
}

char *cmzn_field_find_mesh_location_search_mode_enum_to_string(
	enum cmzn_field_find_mesh_location_search_mode mode)
{
	const char *mode_string = cmzn_field_find_mesh_location_search_mode_conversion::to_string(mode);
	return (mode_string ? duplicate_string(mode_string) : 0);
}

enum cmzn_field_find_mesh_location_search_mode
	cmzn_field_find_mesh_location_get_search_mode(
		cmzn_field_find_mesh_location_id find_mesh_location_field)
{
	if (find_mesh_location_field)
		return find_mesh_location_field->get_core()->getSearchMode();
	return CMZN_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_INVALID;
}

int cmzn_field_find_mesh_location_set_search_mode(
	cmzn_field_find_mesh_location_id find_mesh_location_field,
	enum cmzn_field_find_mesh_location_search_mode search_mode)
{
	if (find_mesh_location_field)
		return find_mesh_location_field->get_core()->setSearchMode(search_mode);
	return CMZN_ERROR_ARGUMENT;
}

namespace {

const char computed_field_xi_coordinates_type_string[] = "xi_coordinates";

class Computed_field_xi_coordinates : public Computed_field_core
{
public:
	Computed_field_xi_coordinates() : Computed_field_core()
	{
	};

	virtual bool is_purely_function_of_field(cmzn_field *other_field)
	{
		return (this->field == other_field);
	}

private:
	Computed_field_core *copy()
	{
		return new Computed_field_xi_coordinates();
	}

	const char *get_type_string()
	{
		return(computed_field_xi_coordinates_type_string);
	}

	int compare(Computed_field_core* other_field)
	{
		if (dynamic_cast<Computed_field_xi_coordinates*>(other_field))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	virtual int evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative);

	virtual int getDerivativeTreeOrder(const FieldDerivative& fieldDerivative)
	{
		return fieldDerivative.getMeshOrder() ? 1 : 0;
	}

	int list();

	char* get_command_string();

	virtual bool is_defined_at_location(cmzn_fieldcache& cache);
};

bool Computed_field_xi_coordinates::is_defined_at_location(cmzn_fieldcache& cache)
{
	return (0 != cache.get_location_element_xi());
}

int Computed_field_xi_coordinates::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	const Field_location_element_xi *element_xi_location = cache.get_location_element_xi();
	if (element_xi_location)
	{
		RealFieldValueCache& valueCache = RealFieldValueCache::cast(inValueCache);
		const FE_value* xi = element_xi_location->get_xi();
		// returns xi up to the element_dimension and padded with zeroes
		const int elementDimension = element_xi_location->get_element()->getDimension();
		for (int i = 0; i < field->number_of_components; ++i)
			valueCache.values[i] = (i < elementDimension) ? xi[i] : 0.0;
		return 1;
	}
	return 0;
}

int Computed_field_xi_coordinates::evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative)
{
	DerivativeValueCache *derivativeCache = inValueCache.getDerivativeValueCache(fieldDerivative);
	if (fieldDerivative.isMeshOnly())
	{
		// directly evaluate only derivative w.r.t. mesh which owns element at location
		// first derivatives are 1.0 in the xi direction, otherwise 0.0
		// derivatives w.r.t. face xi currently handled by fallback finite difference
		const Field_location_element_xi* element_xi_location = cache.get_location_element_xi();
		if (element_xi_location)
		{
			if (element_xi_location->get_element()->getMesh() == fieldDerivative.getMesh())
			{
				FE_value *derivatives = derivativeCache->values;
				// start with all derivatives zero
				derivativeCache->zeroValues();
				// first derivatives have 1.0 on main diagonal up to element_dimension
				if (fieldDerivative.getMeshOrder() == 1)
				{
					const int elementDimension = element_xi_location->get_element_dimension();
					for (int i = 0; i < elementDimension; ++i)
						derivatives[i*(elementDimension + 1)] = 1.0;
				}
				return 1;
			}
			return this->evaluateDerivativeFiniteDifference(cache, inValueCache, fieldDerivative);  // could be a related element/face/line mesh
		}
	}
	// all other derivatives are zero
	derivativeCache->zeroValues();
	return 1;
}

int Computed_field_xi_coordinates::list()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(List_Computed_field_xi_coordinates);
	if (field)
	{
		/* no extra parameters */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_xi_coordinates.  Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_xi_coordinates */

char *Computed_field_xi_coordinates::get_command_string()
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string;

	ENTER(Computed_field_xi_coordinates::get_command_string);
	command_string = (char *)NULL;
	if (field)
	{
		/* no command options */
		command_string =
			duplicate_string(computed_field_xi_coordinates_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_xi_coordinates::get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_xi_coordinates::get_command_string */

} // namespace

int Computed_field_is_type_xi_coordinates(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 25 August 2006

DESCRIPTION :
Returns true if <field> has the appropriate static type string.
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_xi_coordinates);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		if (dynamic_cast<Computed_field_xi_coordinates*>(field->core))
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
			"Computed_field_is_type_xi_coordinates.  Missing field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_xi_coordinates */

cmzn_field *cmzn_fieldmodule_create_field_xi_coordinates(
	cmzn_fieldmodule *fieldmodule)
{
	cmzn_field *field = nullptr;
	if (fieldmodule)
	{
		field = Computed_field_create_generic(fieldmodule,
			/*check_source_field_regions*/true,
			/*number_of_components*/3,
			/*number_of_source_fields*/0, NULL,
			/*number_of_source_values*/0, NULL,
			new Computed_field_xi_coordinates());
	}
	return (field);
}

namespace {

/**
 * If <field> has a source FE_field, ensures it is in <fe_field_list>.
 * @param fe_field_list_void  Void pointer to LIST(FE_field)
 */
int cmzn_field_add_source_FE_field_to_list(cmzn_field *field, void *fe_field_list_void)
{
	int return_code = 1;
	struct LIST(FE_field) *fe_field_list = static_cast<struct LIST(FE_field) *>(fe_field_list_void);
	if (field && fe_field_list)
	{
		FE_field *fe_field = 0;
		Computed_field_finite_element *finite_element_core;
		Computed_field_node_value *node_value_core;
		if ((finite_element_core = dynamic_cast<Computed_field_finite_element*>(field->core)))
		{
			fe_field = finite_element_core->fe_field;
		}
		else if ((node_value_core = dynamic_cast<Computed_field_node_value*>(field->core)))
		{
			fe_field = node_value_core->fe_field;
		}
		if (fe_field)
		{
			if (!IS_OBJECT_IN_LIST(FE_field)(fe_field, fe_field_list))
			{
				return_code = ADD_OBJECT_TO_LIST(FE_field)(fe_field, fe_field_list);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_add_source_FE_field_to_list.  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

}

struct LIST(FE_field)
	*Computed_field_get_defining_FE_field_list(struct Computed_field *field)
{
	struct LIST(FE_field) *fe_field_list = nullptr;
	if (field)
	{
		fe_field_list = CREATE(LIST(FE_field))();
		if (fe_field_list)
		{
			if (!Computed_field_for_each_ancestor_same_region(field,
				cmzn_field_add_source_FE_field_to_list, (void *)fe_field_list))
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_get_defining_FE_field_list.  Failed");
				DESTROY(LIST(FE_field))(&fe_field_list);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_defining_FE_field_list.  Invalid argument(s)");
	}
	return (fe_field_list);
}

struct LIST(FE_field)
	*Computed_field_array_get_defining_FE_field_list(
		int number_of_fields, struct Computed_field **field_array)
{
	struct LIST(FE_field) *fe_field_list = nullptr;
	if ((0 < number_of_fields) && (field_array))
	{
		fe_field_list = Computed_field_get_defining_FE_field_list(field_array[0]);
		for (int i = 1 ; i < number_of_fields ; i++)
		{
			struct LIST(FE_field) *additional_fe_field_list =
				Computed_field_get_defining_FE_field_list(field_array[i]);
			FOR_EACH_OBJECT_IN_LIST(FE_field)(ensure_FE_field_is_in_list,
				(void *)fe_field_list, additional_fe_field_list);
			DESTROY(LIST(FE_field))(&additional_fe_field_list);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_array_get_defining_FE_field_list.  Invalid argument(s)");
	}
	return (fe_field_list);
}

int Computed_field_is_type_finite_element_iterator(
	struct Computed_field *field, void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 16 March 2007

DESCRIPTION :
Iterator/conditional function returning true if <field> is read only and a
wrapper for an FE_field.
==============================================================================*/
{
	int return_code;

	USE_PARAMETER(dummy_void);
	ENTER(Computed_field_is_type_finite_element_iterator);
	if (field)
	{
		return_code = Computed_field_is_type_finite_element(field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_finite_element_iterator.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_type_finite_element_iterator */

int Computed_field_wraps_fe_field(
	struct Computed_field *field, void *fe_field_void)
{
	int return_code = 0;

	ENTER(Computed_field_wraps_fe_field);
	struct FE_field *fe_field = (struct FE_field *)fe_field_void;
	if (field && fe_field)
	{
		Computed_field_finite_element* core =
			dynamic_cast<Computed_field_finite_element*>(field->core);
		if (core)
		{
			return_code = (fe_field == core->fe_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_wraps_fe_field.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
}

int Computed_field_has_coordinate_fe_field(struct Computed_field *field,
	void *)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Iterator/conditional function returning true if <field> is a wrapper for a
coordinate type fe_field.
==============================================================================*/
{
	if (field)
	{
		Computed_field_finite_element* core = dynamic_cast<Computed_field_finite_element*>(field->core);
		if (core)
			return core->fe_field->isTypeCoordinate();
		return 0;
	}
	display_message(ERROR_MESSAGE, "Computed_field_has_coordinate_fe_field.  Invalid argument(s)");
	return 0;
}

int Computed_field_has_element_xi_fe_field(struct Computed_field *field,
	void *dummy)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Iterator/conditional function returning true if <field> is a wrapper for an
element_xi type fe_field.
==============================================================================*/
{
	Computed_field_finite_element* core;
	int return_code;

	ENTER(Computed_field_has_element_xi_fe_field);
	USE_PARAMETER(dummy);
	if (field)
	{
		return_code = 0;
		core = dynamic_cast<Computed_field_finite_element*>(field->core);
		if (core)
		{
			return_code = (core->fe_field->getValueType() == ELEMENT_XI_VALUE);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_has_element_xi_fe_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_has_element_xi_fe_field */

int Computed_field_is_scalar_integer(struct Computed_field *field,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Returns true if <field> is a 1 integer component FINITE_ELEMENT wrapper.
==============================================================================*/
{
	Computed_field_finite_element* core;
	int return_code;

	ENTER(Computed_field_is_scalar_integer);
	USE_PARAMETER(dummy_void);
	if (field)
	{
		if((1==field->number_of_components)&&
			(core = dynamic_cast<Computed_field_finite_element*>(field->core)))
		{
			return_code = (INT_VALUE == core->fe_field->getValueType());
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_scalar_integer.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_is_scalar_integer */

int Computed_field_is_scalar_integer_grid_in_element(
	cmzn_field_id field, void *element_void)
{
	struct FE_element *element;
	if (field && (element = (struct FE_element *)element_void))
	{
		Computed_field_finite_element* core;
		if ((1 == field->number_of_components)
			&& (core = dynamic_cast<Computed_field_finite_element*>(field->core))
			&& (INT_VALUE == core->fe_field->getValueType())
			&& FE_element_field_is_grid_based(element, core->fe_field))
			return 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_scalar_integer_grid_in_element.  Invalid argument(s)");
	}
	return 0;
}

struct FE_time_sequence *Computed_field_get_FE_node_field_FE_time_sequence(
	 struct Computed_field *field, struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 22 Feb 2008

DESCRIPTION :
Returns the <fe_time_sequence> corresponding to the <node> and <field>.  If the
<node> and <field> have no time dependence then the function will return NULL.
==============================================================================*/
{
	FE_time_sequence *time_sequence;
	FE_field *fe_field;
	struct LIST(FE_field) *fe_field_list;

	ENTER(Computed_field_get_FE_node_field_FE_time_sequence);
	time_sequence = (FE_time_sequence *)NULL;
	if (field)
	{
		fe_field_list = Computed_field_get_defining_FE_field_list(field);
		if (fe_field_list)
		{
			if (NUMBER_IN_LIST(FE_field)(fe_field_list) == 1)
			{
				fe_field = FIRST_OBJECT_IN_LIST_THAT(FE_field)(
						(LIST_CONDITIONAL_FUNCTION(FE_field) *)NULL, (void *)NULL,
						fe_field_list);
				const FE_node_field *node_field = node->getNodeField(fe_field);
				if (node_field)
				{
					time_sequence = node_field->getTimeSequence();
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Computed_field_get_FE_node_field_FE_time_sequence. None or"
					"more than one FE element field is used to define this"
					"computed field, this function expects only one finite element"
					"field at the corresponding node otherwise it may contain more than"
					"one time sequence./n");
			}
			DESTROY(LIST(FE_field))(&fe_field_list);
		}
		else
		{
			display_message(ERROR_MESSAGE,
					"Computed_field_get_FE_node_field_FE_time_sequence. Cannot get the"
					"FE field list /n");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_FE_node_field_FE_time_sequence.  Invalid argument(s)");
		time_sequence = (FE_time_sequence *)NULL;
	}
	LEAVE;

	return (time_sequence);
} /* Computed_field_get_FE_node_field_FE_time_sequence */

namespace {

const char computed_field_is_exterior_type_string[] = "is_exterior";

class Computed_field_is_exterior : public Computed_field_core
{
public:
	Computed_field_is_exterior() : Computed_field_core()
	{
	};

	virtual bool is_purely_function_of_field(cmzn_field *other_field)
	{
		return (this->field == other_field);
	}

private:
	Computed_field_core *copy()
	{
		return new Computed_field_is_exterior();
	}

	const char *get_type_string()
	{
		return (computed_field_is_exterior_type_string);
	}

	virtual enum cmzn_field_type get_type()
	{
		return CMZN_FIELD_TYPE_IS_EXTERIOR;
	}

	int compare(Computed_field_core* other_field)
	{
		return (0 != dynamic_cast<Computed_field_is_exterior*>(other_field));
	}

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	virtual int evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative);

	virtual int getDerivativeTreeOrder(const FieldDerivative& fieldDerivative)
	{
		return 0;
	}

	int list()
	{
		return 1;
	}

	char* get_command_string()
	{
		return duplicate_string(computed_field_is_exterior_type_string);
	}

	bool is_defined_at_location(cmzn_fieldcache& cache)
	{
		return (0 != cache.get_location_element_xi());
	}
};

int Computed_field_is_exterior::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	const Field_location_element_xi *element_xi_location = cache.get_location_element_xi();
	if (element_xi_location)
	{
		RealFieldValueCache& valueCache = RealFieldValueCache::cast(inValueCache);
		cmzn_element* element = element_xi_location->get_element();
		FE_mesh *fe_mesh = element->getMesh();
		if (fe_mesh && fe_mesh->isElementExterior(get_FE_element_index(element)))
			valueCache.values[0] = 1.0;
		else
			valueCache.values[0] = 0.0;
		return 1;
	}
	return 0;
}

int Computed_field_is_exterior::evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative)
{
	// assume all derivatives are zero
	inValueCache.getDerivativeValueCache(fieldDerivative)->zeroValues();
	return 1;
}

} // namespace

cmzn_field_id cmzn_fieldmodule_create_field_is_exterior(
	cmzn_fieldmodule_id fieldmodule)
{
	cmzn_field *field = nullptr;
	if (fieldmodule)
	{
		field = Computed_field_create_generic(fieldmodule,
			/*check_source_field_regions*/true,
			/*number_of_components*/1,
			/*number_of_source_fields*/0, NULL,
			/*number_of_source_values*/0, NULL,
			new Computed_field_is_exterior());
	}
	return field;
}

namespace {

const char computed_field_is_on_face_type_string[] = "is_on_face";

class Computed_field_is_on_face : public Computed_field_core
{
	cmzn_element_face_type elementFaceType;

public:
	Computed_field_is_on_face(cmzn_element_face_type elementFaceTypeIn) :
		Computed_field_core(),
		elementFaceType(elementFaceTypeIn)
	{
	};

	cmzn_element_face_type getElementFaceType() const
	{
		return this->elementFaceType;
	}

	int setElementFaceType(cmzn_element_face_type elementFaceTypeIn)
	{
		if (elementFaceTypeIn != this->elementFaceType)
		{
			char *s = cmzn_element_face_type_enum_to_string(elementFaceTypeIn);
			if (!s)
			{
				display_message(ERROR_MESSAGE, "FieldIsOnFace setElementFaceType.  Invalid element face type");
				return CMZN_ERROR_ARGUMENT;
			}
			DEALLOCATE(s);
			this->elementFaceType = elementFaceTypeIn;
			this->field->setChanged();
		}
		return CMZN_OK;
	}

	virtual bool is_purely_function_of_field(cmzn_field *other_field)
	{
		return (this->field == other_field);
	}

private:
	Computed_field_core *copy()
	{
		return new Computed_field_is_on_face(this->elementFaceType);
	}

	const char *get_type_string()
	{
		return (computed_field_is_on_face_type_string);
	}

	virtual enum cmzn_field_type get_type()
	{
		return CMZN_FIELD_TYPE_IS_ON_FACE;
	}

	int compare(Computed_field_core* other_field)
	{
		Computed_field_is_on_face* other_core = dynamic_cast<Computed_field_is_on_face*>(other_field);
		return (0 != other_core) && (other_core->elementFaceType == this->elementFaceType);
	}

	virtual int evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache);

	virtual int evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative);

	virtual int getDerivativeTreeOrder(const FieldDerivative& fieldDerivative)
	{
		return 0;
	}

	int list()
	{
		display_message(INFORMATION_MESSAGE,"    face : %s\n", ENUMERATOR_STRING(cmzn_element_face_type)(this->elementFaceType));
		return 1;
	}

	char* get_command_string()
	{
		char *command_string = duplicate_string(computed_field_is_on_face_type_string);
		int error = 0;
		append_string(&command_string, " face ", &error);
		append_string(&command_string, ENUMERATOR_STRING(cmzn_element_face_type)(this->elementFaceType), &error);
		return command_string;
	}

	bool is_defined_at_location(cmzn_fieldcache& cache)
	{
		return (0 != cache.get_location_element_xi());
	}
};

int Computed_field_is_on_face::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	const Field_location_element_xi *element_xi_location = cache.get_location_element_xi();
	if (element_xi_location)
	{
		RealFieldValueCache& valueCache = RealFieldValueCache::cast(inValueCache);
		cmzn_element* element = element_xi_location->get_element();
		FE_mesh *fe_mesh = element->getMesh();
		if (fe_mesh &&
				((CMZN_ELEMENT_FACE_TYPE_ALL == this->elementFaceType) ||
				((CMZN_ELEMENT_FACE_TYPE_NO_FACE == this->elementFaceType) &&
					(fe_mesh->getElementParentOnFace(get_FE_element_index(element), CMZN_ELEMENT_FACE_TYPE_ANY_FACE) < 0)) ||
				((CMZN_ELEMENT_FACE_TYPE_NO_FACE != this->elementFaceType) &&
					(fe_mesh->getElementParentOnFace(get_FE_element_index(element), this->elementFaceType) >= 0))))
			valueCache.values[0] = 1.0;
		else
			valueCache.values[0] = 0.0;
		return 1;
	}
	return 0;
}

int Computed_field_is_on_face::evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative)
{
	// assume all derivatives are zero
	inValueCache.getDerivativeValueCache(fieldDerivative)->zeroValues();
	return 1;
}

} // namespace

cmzn_field_id cmzn_fieldmodule_create_field_is_on_face(
	cmzn_fieldmodule_id fieldmodule, enum cmzn_element_face_type face)
{
	cmzn_field *field = nullptr;
	if ((fieldmodule) && (ENUMERATOR_STRING(cmzn_element_face_type)(face)))
	{
		field = Computed_field_create_generic(fieldmodule,
			/*check_source_field_regions*/true,
			/*number_of_components*/1,
			/*number_of_source_fields*/0, NULL,
			/*number_of_source_values*/0, NULL,
			new Computed_field_is_on_face(face));
	}
	return field;
}

cmzn_field_is_on_face_id cmzn_field_cast_is_on_face(cmzn_field_id field)
{
	if (field && (dynamic_cast<Computed_field_is_on_face*>(field->core)))
	{
		cmzn_field_access(field);
		return (reinterpret_cast<cmzn_field_is_on_face_id>(field));
	}
	return nullptr;
}

int cmzn_field_is_on_face_destroy(
	cmzn_field_is_on_face_id *is_on_face_field_address)
{
	return cmzn_field_destroy(reinterpret_cast<cmzn_field_id *>(is_on_face_field_address));
}

inline Computed_field_is_on_face *Computed_field_is_on_face_core_cast(
	cmzn_field_is_on_face *is_on_face_field)
{
	return static_cast<Computed_field_is_on_face*>(
		reinterpret_cast<Computed_field*>(is_on_face_field)->core);
}

enum cmzn_element_face_type cmzn_field_is_on_face_get_element_face_type(
	cmzn_field_is_on_face_id is_on_face_field)
{
	if (is_on_face_field)
	{
		Computed_field_is_on_face *is_on_face_core = Computed_field_is_on_face_core_cast(is_on_face_field);
		return is_on_face_core->getElementFaceType();
	}
	return CMZN_ELEMENT_FACE_TYPE_INVALID;
}

int cmzn_field_is_on_face_set_element_face_type(
	cmzn_field_is_on_face_id is_on_face_field, enum cmzn_element_face_type face)
{
	if (is_on_face_field)
	{
		Computed_field_is_on_face *is_on_face_core = Computed_field_is_on_face_core_cast(is_on_face_field);
		return is_on_face_core->setElementFaceType(face);
	}
	return CMZN_ERROR_ARGUMENT;
}


class cmzn_field_edge_discontinuity_measure_conversion
{
public:
	static const char *to_string(enum cmzn_field_edge_discontinuity_measure measure)
	{
		const char *enum_string = 0;
		switch (measure)
		{
			case CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_C1:
				enum_string = "C1";
				break;
			case CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_G1:
				enum_string = "G1";
				break;
			case CMZN_FIELD_EDGE_DISCONTINUITY_MEASURE_SURFACE_NORMAL:
				enum_string = "SURFACE_NORMAL";
				break;
			default:
				break;
		}
		return enum_string;
	}
};

enum cmzn_field_edge_discontinuity_measure
cmzn_field_edge_discontinuity_measure_enum_from_string(const char *name)
{
	return string_to_enum<enum cmzn_field_edge_discontinuity_measure,
		cmzn_field_edge_discontinuity_measure_conversion>(name);
}

char *cmzn_field_edge_discontinuity_measure_enum_to_string(
	enum cmzn_field_edge_discontinuity_measure measure)
{
	const char *measure_string = cmzn_field_edge_discontinuity_measure_conversion::to_string(measure);
	return (measure_string ? duplicate_string(measure_string) : 0);
}

FE_mesh *cmzn_field_get_host_FE_mesh(cmzn_field *field)
{
	if (field && field->core && (field->getValueType() == CMZN_FIELD_VALUE_TYPE_MESH_LOCATION))
	{
		Computed_field_finite_element *fieldFiniteElement = dynamic_cast<Computed_field_finite_element*>(field->core);
		if (fieldFiniteElement)
		{
			return fieldFiniteElement->fe_field->getElementXiHostMesh();
		}
		Computed_field_find_mesh_location *fieldFindMeshLocation = dynamic_cast<Computed_field_find_mesh_location*>(field->core);
		if (fieldFindMeshLocation)
		{
			return fieldFindMeshLocation->getMesh()->getFeMesh();
		}
		// for other fields, e.g. conditional field if, use first source field host mesh
		for (int i = 0; i < field->number_of_source_fields; ++i)
		{
			if (field->source_fields[i]->getValueType() == CMZN_FIELD_VALUE_TYPE_MESH_LOCATION)
			{
				return cmzn_field_get_host_FE_mesh(field->source_fields[i]);
			}
		}
	}
	display_message(ERROR_MESSAGE, "cmzn_field_get_host_FE_mesh.  Invalid argument(s)");
	return nullptr;
}

FE_element_field_evaluation *cmzn_field_get_cache_FE_element_field_evaluation(cmzn_field *field, cmzn_fieldcache *fieldcache)
{
	Computed_field_finite_element* core;
	const Field_location_element_xi *element_xi_location;
	if (!((field) && (fieldcache)
		&& (core = dynamic_cast<Computed_field_finite_element*>(field->core))
		&& (core->fe_field->get_FE_field_type() == GENERAL_FE_FIELD)
		&& (core->fe_field->getValueType() == FE_VALUE_VALUE)
		&& (element_xi_location = fieldcache->get_location_element_xi())))
	{
		display_message(ERROR_MESSAGE, "cmzn_field_get_cache_FE_element_field_evaluation.  Invalid argument(s)");
		return nullptr;
	}
	FiniteElementRealFieldValueCache *feValueCache = FiniteElementRealFieldValueCache::cast(field->getValueCache(*fieldcache));
	if (!feValueCache)
		return nullptr;
	cmzn_element_id element = element_xi_location->get_element();
	cmzn_element_id top_level_element = element_xi_location->get_top_level_element();
	const FE_value time = element_xi_location->get_time();
	return feValueCache->element_field_evaluation_cache->getElementFieldEvaluation(element, time, top_level_element);
}
