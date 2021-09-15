/**
 * FILE : computed_field_apply.cpp
 *
 * Implements fields for applying the function of other fields, including from
 * other regions, with argument binding.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_apply.hpp"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "region/cmiss_region.hpp"
#include "general/message.h"
#include <cstdlib>
#include <vector>


namespace {

const char Computed_field_apply_type_string[] = "apply";

const char Computed_field_argument_real_type_string[] = "argument_real";

class ArgumentRealFieldValueCache : public RealFieldValueCache
{
	static const int maxSourceFields = 5;

public:
	// use vector of distinct source field and cache values with count of each
	std::vector<cmzn_field *> sourceFields;
	std::vector<cmzn_fieldcache *> sourceCaches;
	std::vector<int> sourceCounts;
	unsigned int currentIndex;

	ArgumentRealFieldValueCache(int componentCount) :
		RealFieldValueCache(componentCount),
		sourceFields(1, nullptr),
		sourceCaches(1, nullptr),
		sourceCounts(1, 0),
		currentIndex(0)
	{
	}

	virtual ~ArgumentRealFieldValueCache()
	{
	}

	static ArgumentRealFieldValueCache* cast(FieldValueCache* valueCache)
	{
		return FIELD_VALUE_CACHE_CAST<ArgumentRealFieldValueCache*>(valueCache);
	}

	static ArgumentRealFieldValueCache& cast(FieldValueCache& valueCache)
	{
		return FIELD_VALUE_CACHE_CAST<ArgumentRealFieldValueCache&>(valueCache);
	}

	virtual void clear()
	{
		if (this->sourceFields[this->currentIndex] != 0)
		{
			display_message(ERROR_MESSAGE, "ArgumentRealFieldValueCache::clear.  Called while fields are bound. Possible concurrency issue?");
		}
		this->sourceFields.resize(1);
		this->sourceCaches.resize(1);
		this->sourceCounts.resize(1);
		this->currentIndex = 0;
		this->sourceFields[this->currentIndex] = nullptr;
		this->sourceCaches[this->currentIndex] = nullptr;
		this->sourceCounts[this->currentIndex] = 0;
		RealFieldValueCache::clear();
	}

	/** Bind the source field, pushing it onto the stack.
	 * Must pop it with unbindSourceField after evaluating.
	 * @return  Boolean true if source field changed => invalidate cache */
	bool bindSource(cmzn_field *sourceFieldIn, cmzn_fieldcache *sourceCacheIn)
	{
		// can rediscover last source field at current index for sourceCounts[0] == 0
		if (this->sourceFields[this->currentIndex] == sourceFieldIn)
		{
			++sourceCounts[this->currentIndex];
			return false;
		}
		if (this->sourceCounts[this->currentIndex] > 0)
		{
			++(this->currentIndex);
			if (this->currentIndex == this->sourceCounts.size())
			{
				unsigned int newSize = this->currentIndex*2;
				this->sourceFields.resize(newSize, nullptr);
				this->sourceCaches.resize(newSize, nullptr);
				this->sourceCounts.resize(newSize, 0);
			}
		}
		this->sourceFields[this->currentIndex] = sourceFieldIn;
		// fields depending on arguments must be evaluated in the shared extra cache
		// check gives a small performance hit, but only when bound source field changes
		if (sourceFieldIn->dependsOnArgument())
			this->sourceCaches[this->currentIndex] = sourceCacheIn->getSharedWorkingCache();
		else
			this->sourceCaches[this->currentIndex] = sourceCacheIn;
		this->sourceCounts[this->currentIndex] = 1;
		return true;
	}

	/** @return  Boolean true if source field changed => invalidate cache */
	bool unbindSource()
	{
		if (sourceCounts[this->currentIndex] > 0)
		{
			if (--(sourceCounts[this->currentIndex]) == 0)
			{
				if (this->currentIndex > 0)
				{
					--(this->currentIndex);
					return true;
				}
			}
			return false;
		}
		display_message(ERROR_MESSAGE, "ArgumentRealFieldValueCache::unbindSource:  Too many calls to unbindSource");
		return true;
	}

	/** Get bound source field and the field cache to evaluate it in.
	 * @param argumentField  The argument field being evaluated.
	 * @param cache  The cache the argument field is being evaluated in.
	 * @param sourceCache  On successful return with a sourceField, set to the non-accessed
	 * sourceCache to evaluate the source field in, updated to the current location.
	 * @return  Non-accessed bound source field or nullptr if none */
	cmzn_field *getBoundSourceField(cmzn_field *argumentField, cmzn_fieldcache& cache, cmzn_fieldcache* &sourceCache);

private:

	inline cmzn_field *getBoundSourceFieldPrivate(cmzn_fieldcache* &sourceCache)
	{
		if (sourceCounts[this->currentIndex] > 0)
		{
			sourceCache = this->sourceCaches[this->currentIndex];
			return this->sourceFields[this->currentIndex];
		}
		return nullptr;
	}

};

cmzn_field *ArgumentRealFieldValueCache::getBoundSourceField(cmzn_field *argumentField, cmzn_fieldcache& cache, cmzn_fieldcache* &sourceCache)
{
	cmzn_field *sourceField = this->getBoundSourceFieldPrivate(sourceCache);
	if (sourceField)
		return sourceField;
	// try to inherit from parent/ancestor cache (which must be for same region)
	cmzn_fieldcache *parentCache = cache.getParentCache();
	while (parentCache)
	{
		ArgumentRealFieldValueCache *argumentRealValueCache = ArgumentRealFieldValueCache::cast(argumentField->getValueCache(*parentCache));
		sourceField = argumentRealValueCache->getBoundSourceFieldPrivate(sourceCache);
		if (sourceField)
		{
			// location must have changed as evaluating on a different cache: do same for source field
			sourceCache = sourceCache->getOrCreateSharedWorkingCache();
			sourceCache->copyLocation(cache);
			return sourceField;
		}
		parentCache = parentCache->getParentCache();
	}
	return nullptr;
}

} // namespace


Computed_field_apply::~Computed_field_apply()
{
	if (other_field_manager_callback_id)
	{
		if (field && (field->number_of_source_fields > 0) && field->source_fields && this->getEvaluateField())
		{
			if (this->getEvaluateField()->manager)
				MANAGER_DEREGISTER(cmzn_field)(other_field_manager_callback_id,
					this->getEvaluateField()->manager);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"~Computed_field_apply.  cmzn_field source_fields removed before core. Can't get manager of evaluate field to end callbacks.");
		}
	}
}

/**
 * Callback for changes in the field manager owning evaluate field.
 * If this field depends on the change, propagate to this manager as a change to
 * this field.
 */
void Computed_field_apply::otherRegionFieldChange(
	struct MANAGER_MESSAGE(cmzn_field) *message, void *apply_field_core_void)
{
	Computed_field_apply *apply_field_core =
		reinterpret_cast<Computed_field_apply *>(apply_field_core_void);
	cmzn_field *field;
	if (message && apply_field_core && (field = apply_field_core->field) &&
		(field->number_of_source_fields > 0) && field->source_fields)
	{
		int change = MANAGER_MESSAGE_GET_OBJECT_CHANGE(cmzn_field)(message,
			apply_field_core->getEvaluateField());
		if (change & MANAGER_CHANGE_RESULT(cmzn_field))
		{
			Computed_field_dependency_changed(field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_apply::otherRegionFieldChange.  Invalid argument(s)");
	}
}

/**
 * If evaluate field is from a different manager to this field, request
 * manager messages to propagate changes to this manager.
 */
void Computed_field_apply::checkApplyFromOtherRegion(void)
{
	if (!other_field_manager_callback_id)
	{
		if (field && (field->number_of_source_fields > 0) && field->source_fields &&
			this->getEvaluateField() && this->getEvaluateField()->manager)
		{
			if (field->manager && (field->manager != this->getEvaluateField()->manager))
			{
				// apply from another region: set up manager callbacks
				other_field_manager_callback_id = MANAGER_REGISTER(cmzn_field)(
					otherRegionFieldChange, (void *)this, this->getEvaluateField()->manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_apply::checkApplyFromOtherRegion.  Invalid source_fields array.");
		}
	}
}

const char* Computed_field_apply::get_type_string()
{
	return (Computed_field_apply_type_string);
}

FieldValueCache *Computed_field_apply::createValueCache(cmzn_fieldcache& fieldCache)
{
	RealFieldValueCache *valueCache = new RealFieldValueCache(this->field->number_of_components);
	// extraCache is always created as likely to be from another region and/or binding arguments
	cmzn_region *evaluateRegion = this->getEvaluateField()->getRegion();
	if (evaluateRegion == field->getRegion())
	{
		valueCache->getOrCreateSharedExtraCache(fieldCache);
	}
	else
	{
		valueCache->getOrCreateSharedExternalExtraCache(fieldCache, evaluateRegion);
	}
	return valueCache;
}

bool Computed_field_apply::bindSourceFields(cmzn_fieldcache& cache, RealFieldValueCache &valueCache)
{
	cmzn_fieldcache& workingCache = *(valueCache.getExtraCache());
	bool result = true;
	bool bindingChange = false;
	const int numberOfSourceFields = this->field->number_of_source_fields;
	for (int i = 1; i < numberOfSourceFields; i += 2)
	{
		cmzn_field *argumentField = this->getSourceField(i);
		cmzn_field *sourceField = this->getSourceField(i + 1);
		ArgumentRealFieldValueCache *argumentRealValueCache = ArgumentRealFieldValueCache::cast(argumentField->getValueCache(workingCache));
		if (!argumentRealValueCache)
		{
			result = false;
		}
		else if (argumentRealValueCache->bindSource(sourceField, &cache))
		{
			bindingChange = true;
		}
	}
	if (!workingCache.isSameLocation(cache))
	{
		workingCache.copyLocation(cache);
	}
	else if (bindingChange)
	{
		workingCache.locationChanged();  // to force recalculation with new source fields
	}
	return result;
}

void Computed_field_apply::unbindSourceFields(cmzn_fieldcache& cache, RealFieldValueCache &valueCache)
{
	cmzn_fieldcache& workingCache = *(valueCache.getExtraCache());
	bool bindingChange = false;
	const int numberOfSourceFields = this->field->number_of_source_fields;
	for (int i = 1; i < numberOfSourceFields; i += 2)
	{
		cmzn_field *argumentField = this->getSourceField(i);
		ArgumentRealFieldValueCache *argumentRealValueCache = ArgumentRealFieldValueCache::cast(argumentField->getValueCache(workingCache));
		if ((argumentRealValueCache) && argumentRealValueCache->unbindSource())
		{
			bindingChange = true;
		}
	}
	if (bindingChange)
	{
		workingCache.locationChanged();
	}
}

int Computed_field_apply::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	// currently guaranteed to be numerical
	RealFieldValueCache &valueCache = RealFieldValueCache::cast(inValueCache);
	int result = 1;
	if (this->bindSourceFields(cache, valueCache))
	{
		cmzn_fieldcache& workingCache = *(valueCache.getExtraCache());
		const RealFieldValueCache *sourceCache = RealFieldValueCache::cast(this->getEvaluateField()->evaluate(workingCache));
		if (sourceCache)
		{
			valueCache.copyValues(*sourceCache);
		}
		else
		{
			result = 0;
		}
	}
	else
	{
		result = 0;
	}
	this->unbindSourceFields(cache, valueCache);
	return result;
}

int Computed_field_apply::evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative)
{
	// currently guaranteed to be numerical
	RealFieldValueCache& valueCache = RealFieldValueCache::cast(inValueCache);
	int result = 1;
	if (this->bindSourceFields(cache, valueCache))
	{
		cmzn_fieldcache& workingCache = *(valueCache.getExtraCache());
		const DerivativeValueCache *sourceDerivativeValueCache = this->getEvaluateField()->evaluateDerivative(workingCache, fieldDerivative);
		if (sourceDerivativeValueCache)
		{
			DerivativeValueCache& derivativeValueCache = *(valueCache.getDerivativeValueCache(fieldDerivative));
			derivativeValueCache.copyValues(*sourceDerivativeValueCache);
		}
		else
		{
			result = 0;
		}
	}
	else
	{
		result = 0;
	}
	this->unbindSourceFields(cache, valueCache);
	return result;
}

bool Computed_field_apply::is_defined_at_location(cmzn_fieldcache& cache)
{
	RealFieldValueCache& valueCache = RealFieldValueCache::cast(*(this->field->getValueCache(cache)));
	bool result = false;
	if (this->bindSourceFields(cache, valueCache))
	{
		cmzn_fieldcache& workingCache = *(valueCache.getExtraCache());
		result = this->getEvaluateField()->core->is_defined_at_location(workingCache);
	}
	this->unbindSourceFields(cache, valueCache);
	return result;
}

enum FieldAssignmentResult Computed_field_apply::assign(cmzn_fieldcache& cache, RealFieldValueCache& valueCache)
{
	FieldAssignmentResult result = FIELD_ASSIGNMENT_RESULT_FAIL;
	if (this->bindSourceFields(cache, valueCache))
	{
		cmzn_fieldcache& workingCache = *(valueCache.getExtraCache());
		RealFieldValueCache& sourceValueCache = RealFieldValueCache::cast(*(this->getEvaluateField()->getValueCache(workingCache)));
		sourceValueCache.setValues(valueCache.values);
		result = this->getEvaluateField()->assign(workingCache, sourceValueCache);
	}
	this->unbindSourceFields(cache, valueCache);
	return result;
}

int Computed_field_apply::check_dependency()
{
	if (this->field)
	{
		if (0 == (field->manager_change_status & MANAGER_CHANGE_FULL_RESULT(cmzn_field)))
		{
			// skip source fields from other regions; their changes propagate by separate callback
			bool localEvaluate = this->getEvaluateField()->getManager() == this->field->getManager();
			const int startIndex = localEvaluate ? 0 : 1;
			const int increment = localEvaluate ? 1 : 2;
			const int numberOfSourceFields = this->field->number_of_source_fields;
			for (int i = startIndex; i < numberOfSourceFields; i += increment)
			{
				int source_change_status = field->source_fields[i]->core->check_dependency();
				if (source_change_status & MANAGER_CHANGE_FULL_RESULT(cmzn_field))
				{
					field->setChangedPrivate(MANAGER_CHANGE_FULL_RESULT(cmzn_field));
					break;
				}
				else if (source_change_status & MANAGER_CHANGE_PARTIAL_RESULT(cmzn_field))
				{
					field->setChangedPrivate(MANAGER_CHANGE_PARTIAL_RESULT(cmzn_field));
				}
			}
		}
		return field->manager_change_status;
	}
	return MANAGER_CHANGE_NONE(cmzn_field);
}

int Computed_field_apply::list()
{
	display_message(INFORMATION_MESSAGE, "    Evaluate field : ");
	cmzn_region *region = this->field->getRegion();
	cmzn_region *evaluateRegion = this->getEvaluateField()->getRegion();
	if (evaluateRegion != region)
	{
		char *path = evaluateRegion->getRelativePath(region);
		display_message(INFORMATION_MESSAGE, "%s" CMZN_REGION_PATH_SEPARATOR_STRING, path);
		DEALLOCATE(path);
	}
	display_message(INFORMATION_MESSAGE, "%s\n", this->getEvaluateField()->name);
	for (int i = 0; i < this->numberOfBindings; ++i)
	{
		cmzn_field *argumentField = this->getBindArgumentField(i);
		cmzn_field *sourceField = this->getBindArgumentSourceField(argumentField);
		display_message(INFORMATION_MESSAGE, "    Bind argument field %s to source field %s", argumentField->name, sourceField->name);
	}
	return 1;
}

char *Computed_field_apply::get_command_string()
{
	char *command_string = nullptr;
	int error = 0;
	append_string(&command_string, Computed_field_apply_type_string, &error);
	append_string(&command_string, " evaluate_field ", &error);
	cmzn_region *region = this->field->getRegion();
	cmzn_region *evaluateRegion = this->getEvaluateField()->getRegion();
	if (evaluateRegion != region)
	{
		char *path = evaluateRegion->getRelativePath(region);
		append_string(&command_string, path, &error);
		DEALLOCATE(path);
		append_string(&command_string, CMZN_REGION_PATH_SEPARATOR_STRING, &error);
	}
	char *fieldName = duplicate_string(this->getEvaluateField()->name);
	if (fieldName)
	{
		make_valid_token(&fieldName);
		append_string(&command_string, fieldName, &error);
		DEALLOCATE(fieldName);
	}
	for (int i = 0; i < this->numberOfBindings; ++i)
	{
		append_string(&command_string, " bind ", &error);
		cmzn_field *argumentField = this->getBindArgumentField(i);
		fieldName = duplicate_string(argumentField->name);
		make_valid_token(&fieldName);
		append_string(&command_string, fieldName, &error);
		DEALLOCATE(fieldName);
		append_string(&command_string, " ", &error);
		cmzn_field *sourceField = this->getBindArgumentSourceField(argumentField);
		fieldName = duplicate_string(sourceField->name);
		make_valid_token(&fieldName);
		append_string(&command_string, fieldName, &error);
		DEALLOCATE(fieldName);
	}
	return command_string;
}

cmzn_field *Computed_field_apply::getBindArgumentField(int bindIndex) const
{
	if ((0 <= bindIndex) && (bindIndex < this->numberOfBindings))
	{
		return this->getSourceField(1 + bindIndex*2);
	}
	return nullptr;
}

cmzn_field *Computed_field_apply::getBindArgumentSourceField(cmzn_field *argumentField) const
{
	const int sourceFieldCount = this->field->number_of_source_fields;
	for (int i = 1; i < sourceFieldCount; i += 2)
	{
		if (this->getSourceField(i) == argumentField)
			return this->getSourceField(i + 1);
	}
	return nullptr;
}

int Computed_field_apply::setBindArgumentSourceField(cmzn_field *argumentField, cmzn_field *sourceField)
{
	cmzn_field *evaluateField = this->getEvaluateField();
	if ((!argumentField) || (argumentField->getManager() != evaluateField->getManager()) ||
		(nullptr == dynamic_cast<Computed_field_argument_real*>(reinterpret_cast<cmzn_field*>(argumentField)->core)))
	{
		display_message(ERROR_MESSAGE, "FieldApply setBindArgumentSourceField.  Argument field is invalid, or not from evaluate field's region");
		return CMZN_ERROR_ARGUMENT;
	}
	const int sourceFieldCount = this->field->number_of_source_fields;
	if (sourceField)
	{
		if (sourceField->getManager() != this->field->getManager())
		{
			display_message(ERROR_MESSAGE, "FieldApply setBindArgumentSourceField.  Source field is not from this region");
			return CMZN_ERROR_ARGUMENT;
		}
		if (sourceField->number_of_components != argumentField->number_of_components)
		{
			display_message(ERROR_MESSAGE, "FieldApply setBindArgumentSourceField.  Source field number of components does not match argument field");
			return CMZN_ERROR_ARGUMENT;
		}
		if (sourceField->dependsOnField(evaluateField))
		{
			display_message(ERROR_MESSAGE, "FieldApply setBindArgumentSourceField.  Cannot set a source field which depends on the evaluate or apply field");
			return CMZN_ERROR_ARGUMENT;
		}
		if (sourceField->dependsOnField(argumentField))
		{
			display_message(ERROR_MESSAGE, "FieldApply setBindArgumentSourceField.  Cannot set a source field which depends on the argument field");
			return CMZN_ERROR_ARGUMENT;
		}
		// following only detects dependence cycles over 2 arguments
		for (int i = 1; i < sourceFieldCount; i += 2)
		{
			cmzn_field *otherArgumentField = this->getSourceField(i);
			if (otherArgumentField != argumentField)
			{
				cmzn_field *otherSourceField = this->getSourceField(i + 1);
				if (sourceField->dependsOnField(otherArgumentField) &&
					otherSourceField->dependsOnField(argumentField))
				{
					display_message(ERROR_MESSAGE, "FieldApply setBindArgumentSourceField.  Dependence cycle detected over bound fields");
					return CMZN_ERROR_ARGUMENT;
				}
			}
		}
	}
	// first try to find argument in existing bindings
	int result = CMZN_OK;
	int i = 1;
	for (; i < sourceFieldCount; i += 2)
	{
		if (this->getSourceField(i) == argumentField)
		{
			if (sourceField)
			{
				result = this->field->setSourceField(i + 1, sourceField);
			}
			else
			{
				// remove binding
				this->field->setSourceField(i + 1, nullptr);
				this->field->setSourceField(i, nullptr);
				--(this->numberOfBindings);
			}
			return CMZN_OK;
		}
	}
	if (sourceField)
	{
		// set both argument and source fields, ensure only one notification
		result = this->field->setSourceField(i, argumentField, /*notify*/false);
		if (result == CMZN_OK)
		{
			result = this->field->setSourceField(i + 1, sourceField);
			if (result == CMZN_OK)
			{
				++(this->numberOfBindings);
			}
			else
			{
				this->field->setSourceField(i, nullptr);
				this->setChanged();
			}
		}
	}
	return result;
}

const char* Computed_field_argument_real::get_type_string()
{
	return (Computed_field_argument_real_type_string);
}

FieldValueCache *Computed_field_argument_real::createValueCache(cmzn_fieldcache& /*fieldCache*/)
{
	return new ArgumentRealFieldValueCache(this->field->number_of_components);
}

int Computed_field_argument_real::evaluate(cmzn_fieldcache& cache, FieldValueCache& inValueCache)
{
	ArgumentRealFieldValueCache &valueCache = ArgumentRealFieldValueCache::cast(inValueCache);
	cmzn_fieldcache *sourceCache = nullptr;
	cmzn_field *sourceField = valueCache.getBoundSourceField(this->field, cache, sourceCache);
	if (sourceField)
	{
		const RealFieldValueCache *sourceValueCache = RealFieldValueCache::cast(sourceField->evaluate(*sourceCache));
		if (sourceValueCache)
		{
			valueCache.copyValues(*sourceValueCache);
			return 1;
		}
	}
	return 0;
}

int Computed_field_argument_real::evaluateDerivative(cmzn_fieldcache& cache, RealFieldValueCache& inValueCache, const FieldDerivative& fieldDerivative)
{
	ArgumentRealFieldValueCache &valueCache = ArgumentRealFieldValueCache::cast(inValueCache);
	cmzn_fieldcache *sourceCache = nullptr;
	cmzn_field *sourceField = valueCache.getBoundSourceField(this->field, cache, sourceCache);
	if (sourceField)
	{
		const DerivativeValueCache *sourceDerivativeCache = sourceField->evaluateDerivative(*sourceCache, fieldDerivative);
		if (sourceDerivativeCache)
		{
			DerivativeValueCache& derivativeValueCache = *(valueCache.getDerivativeValueCache(fieldDerivative));
			derivativeValueCache.copyValues(*sourceDerivativeCache);
			return 1;
		}
	}
	return 0;
}

bool Computed_field_argument_real::is_defined_at_location(cmzn_fieldcache& cache)
{
	ArgumentRealFieldValueCache& valueCache = ArgumentRealFieldValueCache::cast(*(this->field->getValueCache(cache)));
	cmzn_fieldcache *sourceCache = nullptr;
	cmzn_field *sourceField = valueCache.getBoundSourceField(this->field, cache, sourceCache);
	if (sourceField)
	{
		return sourceField->core->is_defined_at_location(*sourceCache);
	}
	return false;
}

enum FieldAssignmentResult Computed_field_argument_real::assign(cmzn_fieldcache& cache, RealFieldValueCache& valueCache)
{
	ArgumentRealFieldValueCache& argumentValueCache = ArgumentRealFieldValueCache::cast(valueCache);
	cmzn_fieldcache *sourceCache = nullptr;
	cmzn_field *sourceField = argumentValueCache.getBoundSourceField(this->field, cache, sourceCache);
	if (sourceField)
	{
		RealFieldValueCache& sourceValueCache = RealFieldValueCache::cast(*(sourceField->getValueCache(*sourceCache)));
		sourceValueCache.setValues(valueCache.values);
		return sourceField->assign(*sourceCache, sourceValueCache);
	}
	return FIELD_ASSIGNMENT_RESULT_FAIL;
}

int Computed_field_argument_real::list()
{
	display_message(INFORMATION_MESSAGE, "    Number of components : %d\n", this->field->number_of_components);
	return 1;
}

char *Computed_field_argument_real::get_command_string()
{
	char *command_string = nullptr;
	int error = 0;
	append_string(&command_string, Computed_field_argument_real_type_string, &error);
	append_string(&command_string, " number_of_components ", &error);
	char tmp[20];
	sprintf(tmp, "%d", this->field->number_of_components);
	append_string(&command_string, tmp, &error);
	return command_string;
}

/*
Public API
----------
*/

cmzn_field_id cmzn_fieldmodule_create_field_alias(cmzn_fieldmodule_id field_module,
	cmzn_field_id original_field)
{
	return cmzn_fieldmodule_create_field_apply(field_module, original_field);
}

cmzn_field_id cmzn_fieldmodule_create_field_apply(
	cmzn_fieldmodule_id fieldmodule, cmzn_field_id evaluate_field)
{
	cmzn_field_id field = 0;
	// @TODO Generalise to non-numeric types by adding createValueCache and modifying evaluate methods
	if (fieldmodule && evaluate_field && evaluate_field->isNumerical())
	{
		field = Computed_field_create_generic(fieldmodule,
			/*check_source_field_regions*/false, evaluate_field->number_of_components,
			/*number_of_source_fields*/1, &evaluate_field,
			/*number_of_source_values*/0, nullptr,
			new Computed_field_apply());
	}
	return (field);
}

cmzn_field_apply_id cmzn_field_cast_apply(cmzn_field_id field)
{
	if (field && dynamic_cast<Computed_field_apply*>(field->core))
	{
		field->access();
		return (reinterpret_cast<cmzn_field_apply_id>(field));
	}
	return nullptr;
}

int cmzn_field_apply_destroy(cmzn_field_apply_id *apply_address)
{
	return cmzn_field_destroy(reinterpret_cast<cmzn_field_id *>(apply_address));
}

cmzn_field_id cmzn_field_apply_get_bind_argument_field(
	cmzn_field_apply_id apply_field, int bind_index)
{
	Computed_field_apply *applyCore = Computed_field_apply::coreCast(apply_field);
	if (applyCore)
	{
		cmzn_field *argumentField = applyCore->getBindArgumentField(bind_index - 1);
		if (argumentField)
			return argumentField->access();
	}
	return nullptr;
}

cmzn_field_id cmzn_field_apply_get_bind_argument_source_field(
	cmzn_field_apply_id apply_field, cmzn_field_id argument_field)
{
	Computed_field_apply *applyCore = Computed_field_apply::coreCast(apply_field);
	if (applyCore)
	{
		cmzn_field *sourceField = applyCore->getBindArgumentSourceField(argument_field);
		if (sourceField)
			return sourceField->access();
	}
	return nullptr;
}

int cmzn_field_apply_set_bind_argument_source_field(
	cmzn_field_apply_id apply_field, cmzn_field_id argument_field,
	cmzn_field_id source_field)
{
	Computed_field_apply *applyCore = Computed_field_apply::coreCast(apply_field);
	if (applyCore)
	{
		return applyCore->setBindArgumentSourceField(argument_field, source_field);
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_field_apply_get_number_of_bindings(cmzn_field_apply_id apply_field)
{
	Computed_field_apply *applyCore = Computed_field_apply::coreCast(apply_field);
	if (applyCore)
	{
		return applyCore->getNumberOfBindings();
	}
	return -1;
}

cmzn_field_id cmzn_fieldmodule_create_field_argument_real(
	cmzn_fieldmodule_id fieldmodule, int number_of_components)
{
	cmzn_field *field = nullptr;
	if (fieldmodule && (0 < number_of_components))
	{
		field = Computed_field_create_generic(fieldmodule,
			/*check_source_field_regions*/false, number_of_components,
			/*number_of_source_fields*/0, nullptr,
			/*number_of_source_values*/0, nullptr,
			new Computed_field_argument_real());
	}
	return (field);
}

cmzn_field_argument_real_id cmzn_field_cast_argument_real(cmzn_field_id field)
{
	if (field && dynamic_cast<Computed_field_argument_real*>(field->core))
	{
		field->access();
		return (reinterpret_cast<cmzn_field_argument_real_id>(field));
	}
	return nullptr;
}

int cmzn_field_argument_real_destroy(cmzn_field_argument_real_id *argument_real_address)
{
	return cmzn_field_destroy(reinterpret_cast<cmzn_field_id *>(argument_real_address));
}
